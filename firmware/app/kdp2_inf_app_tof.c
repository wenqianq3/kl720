/*
 * Kneron Application general functions
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kmdw_console.h"

#include "kmdw_inference_app.h"
#include "kmdw_memory.h"
#include "kmdw_model.h"
#include "kmdw_tof.h"
#include "kmdw_fifoq_manager.h"
#include "kdev_status.h"
#include "kmdw_sensor.h"

#include "kdp2_inf_app_yolo.h"
#include "kdp2_inf_app_tof.h" // for hico tof application flow

#include "hico_tof.h"         // for hico_mode_t definition
#include "ipc.h"
#include "tof_intf.h"

#include "model_type.h"

extern uint32_t tof_ir_bright_buf_addr;
#define TARGET_CLASS 0 // 0: person

osEventFlagsId_t g_tof_event_id;

static kp_app_yolo_post_proc_config_t post_proc_params_v5s = {
    .prob_thresh = 0.15,
    .nms_thresh = 0.5,
    .max_detection_per_class = 20,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326,
        // strides[3]
        8, 16, 32,
    },
};

static kp_app_yolo_post_proc_config_t post_proc_params_v5m = {
    .prob_thresh = 0.3,
    .nms_thresh = 0.45,
    .max_detection_per_class = 20,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326,
        // strides[3]
        8, 16, 32,
    },
};

static kp_app_yolo_post_proc_config_t post_proc_params_v3_tiny_416 = {
    .prob_thresh = 0.6,
    .nms_thresh = 0.45,
    .max_detection_per_class = 20,
    .anchor_row = 3,
    .anchor_col = 6,
    .stride_size = 3,
    .reserved_size = 0,
    .data = {
        // anchors[3][6]
        81, 82, 135, 169, 344, 319,
        23, 27, 37, 58, 81, 82,
        4, 9, 13, 24, 24, 50, // -> not used in tiny yolo v3 post-proc
        // strides[3] -> not used in tiny yolo v3 post-proc
        8, 16, 32,
    },
};

typedef struct
{
    int model_id;
    int param_size;
    void *post_proc_params;
} map_model_post_proc_t;

#define MAX_MODEL_PAIRS 4
static map_model_post_proc_t model_pp[MAX_MODEL_PAIRS] = {0}; // 4 pairs of modle-post_proc enough ?

static map_model_post_proc_t get_model_post_proc_param(int model_id)
{
    map_model_post_proc_t mapping = {0};

    // looking for model's post-proc params, if none apply some defaults
    for (int i = 0; i < MAX_MODEL_PAIRS; i++)
    {
        if (model_pp[i].model_id == model_id)
        {
            // found matched model id with post-proc params
            mapping = model_pp[i];
            break;
        }
        else if (model_pp[i].model_id == 0)
        {
            // register some default settings
            model_pp[i].model_id = model_id;
            switch (model_id)
            {
            case KNERON_YOLOV5S_COCO80_640_640_3:
            case KNERON_YOLOV5S_COCO80_480_640_3:
                model_pp[i].param_size = sizeof(post_proc_params_v5s);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v5s;
                break;
            case KNERON_YOLOV5m_COCO80_640_640_3:
            case KNERON_PERSONDETECTION_YOLOV5s_480_256_3:
            case KNERON_PERSONDETECTION_YOLOV5sParklot_480_256_3:
                model_pp[i].param_size = sizeof(post_proc_params_v5m);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v5m;
                break;
            case TINY_YOLO_V3_416_416_3:
                model_pp[i].param_size = sizeof(post_proc_params_v3_tiny_416);
                model_pp[i].post_proc_params = (void *)&post_proc_params_v3_tiny_416;
                break;
            default:
                // cannot find matched post-proc config
                break;
            }

            mapping = model_pp[i];
            break;
        }
    }

    return mapping;
}

void kdp2_app_yolo_config_post_process_parameters(int job_id, int num_input_buf, void **inf_input_buf_list)
{
    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    kdp2_ipc_app_yolo_post_proc_config_t *yolo_pp_config = (kdp2_ipc_app_yolo_post_proc_config_t *)inf_input_buf_list[0];

    if (yolo_pp_config->set_or_get == 1)
    {
        // setting post-proc configs with specified model_id
        for (int i = 0; i < MAX_MODEL_PAIRS; i++)
        {
            if (model_pp[i].model_id == yolo_pp_config->model_id || model_pp[i].model_id == 0)
            {
                model_pp[i].model_id = yolo_pp_config->model_id; // for model_pp[i].model_id == 0
                if (model_pp[i].post_proc_params == NULL || yolo_pp_config->param_size > model_pp[i].param_size)
                {
                    model_pp[i].post_proc_params = malloc(yolo_pp_config->param_size);
                    if (model_pp[i].post_proc_params == NULL)
                    {
                        kmdw_printf("[app_yolo]: error ! no memory for malloc post-proc parameters\n");
                        kmdw_inference_app_send_status_code(job_id, KP_FW_CONFIG_POST_PROC_ERROR_MALLOC_FAILED_105);
                        return; // failed return
                    }

                    model_pp[i].param_size = yolo_pp_config->param_size;
                }

                memcpy(model_pp[i].post_proc_params, (void *)yolo_pp_config->param_data, yolo_pp_config->param_size);
                kmdw_inference_app_send_status_code(job_id, KP_SUCCESS);
                return; // sucessful return
            }
        }

        kmdw_inference_app_send_status_code(job_id, KP_FW_CONFIG_POST_PROC_ERROR_NO_SPACE_106);
        return; // failed return
    }
    else
    {
        // getting post-proc configs with specified model_id
        // get a result buffer to save pp parameters
        int result_buf_size;
        kdp2_ipc_app_yolo_post_proc_config_t *return_pp_config = (kdp2_ipc_app_yolo_post_proc_config_t *)kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

        map_model_post_proc_t mapping = get_model_post_proc_param(yolo_pp_config->model_id);

        return_pp_config->header_stamp = yolo_pp_config->header_stamp;
        return_pp_config->header_stamp.status_code = KP_SUCCESS;
        return_pp_config->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_post_proc_config_t);
        return_pp_config->set_or_get = 0;
        return_pp_config->model_id = yolo_pp_config->model_id;
        return_pp_config->param_size = mapping.param_size;
        if (mapping.param_size > 0)
            memcpy((void *)return_pp_config->param_data, mapping.post_proc_params, mapping.param_size);

        // send pp params back to host SW
        kmdw_fifoq_manager_result_enqueue((void *)return_pp_config, result_buf_size, false);
    }
}

uint32_t calculate_depth_in_some_way(int img_width, int img_height,
                                     int box_x1, int box_y1, int box_x2, int box_y2, uint16_t *depth_map)
{
    int diff_x = box_x2 - box_x1;
    int diff_y = box_y2 - box_y1;

    // kmdw_printf("bounding box (%d,%d) -> (%d,%d)\n", box_x1, box_y1, box_x2, box_y2);

#if 0

    int center_x = box_x1 + diff_x / 2;
    int center_y = box_y1 + diff_y / 2;
    return depth_map[img_width * center_y + center_x];

#endif

#if 1

    int dv = 9;

    int delta_x = diff_x / dv;
    int delta_y = diff_y / dv;

    int pos_x = 0;
    int pos_y = 0;

    uint32_t sum_depth = 0;

    // samle 64 points in the area
    int total_points = (dv - 1) * (dv - 1);

    // Skip the boundary points, only sample the central ones
    for (int i = 1; i < dv; i++)
    {
        for (int j = 1; j < dv; j++)
        {
            pos_x = box_x1 + delta_x * i;
            pos_y = box_y1 + delta_y * j;

            uint16_t depth_value = depth_map[img_width * pos_y + pos_x];

            // kmdw_printf("collecting (%d,%d) depth = %d\n", pos_x, pos_y, depth_value);

            if(depth_value == 0)
            {
                total_points--;
                continue;
            }

            sum_depth += depth_value;
        }
    }

    uint32_t avg_depth = sum_depth / total_points;
    // kmdw_printf("average depth = %d\n", avg_depth);

    return avg_depth;

#endif
}

void yolo_result_callback(int status, void *inf_result_buf, int inf_result_buf_size, void *ncpu_result_buf, bounding_box_t *single_tof_box, bool *is_target_found)
{
    kdp2_app_tof_result_t *tof_result = (kdp2_app_tof_result_t *)inf_result_buf;
    kp_app_yolo_result_t *yolo_inf_output = (kp_app_yolo_result_t *)ncpu_result_buf;

    if (status != KP_SUCCESS)
    {
        kmdw_printf("yolo_result_callback() error %d\n", status);
        return;
    }

    if (yolo_inf_output->box_count > YOLO_GOOD_BOX_MAX)
    {
        kmdw_printf("error ! too many bounding boxes = %d!!!\n", yolo_inf_output->box_count);
        yolo_inf_output->box_count = YOLO_GOOD_BOX_MAX;
    }

    // kmdw_printf("@@@ yolo box = %d\n", yolo_inf_output->box_count);

    tof_result->tof_data.class_count = yolo_inf_output->class_count;
    tof_result->tof_data.box_count = yolo_inf_output->box_count;
    float target_class_area = 0;

    for (int i = 0; i < yolo_inf_output->box_count; i++)
    {
        kdp2_tof_box_t *tof_box = &tof_result->tof_data.boxes[i];
        kp_bounding_box_t *yolo_box = &yolo_inf_output->boxes[i];

        if((int)yolo_box->x2 >= TOF_IMG_WIDTH)
            yolo_box->x2 = TOF_IMG_WIDTH - 1; // WKKKAAAROUND
        if((int)yolo_box->y2 >= TOF_IMG_HEIGHT)
            yolo_box->y2 = TOF_IMG_HEIGHT -1; // WKKKAAAROUND

        memcpy(tof_box, yolo_box, sizeof(kp_bounding_box_t));

        /* average depth should calculate by dsp to speed up!*/
        tof_box->average_depth = calculate_depth_in_some_way(TOF_IMG_WIDTH, TOF_IMG_HEIGHT,
                                                             (int)yolo_box->x1, (int)yolo_box->y1,
                                                             (int)yolo_box->x2, (int)yolo_box->y2,
                                                             (uint16_t *)tof_result->depth_data);
#if ENABLE_TOF_AEC
        if(yolo_box->class_num == TARGET_CLASS){
            float area = (yolo_box->x2 - yolo_box->x1) * (yolo_box->y2 - yolo_box->y1);
            if(area > target_class_area){
                target_class_area = area;
                memcpy(single_tof_box, tof_box, sizeof(bounding_box_t));
            }
            *is_target_found = true;
#endif

        }
    }

    tof_result->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    tof_result->header_stamp.status_code = KP_SUCCESS;
    tof_result->header_stamp.total_size = sizeof(kdp2_app_tof_result_t);
    tof_result->header_stamp.job_id = KDP2_INF_ID_APP_TOF;

    // send output result buffer back to host SW
    kmdw_fifoq_manager_result_enqueue((void *)inf_result_buf, inf_result_buf_size, false);
}

void kdp2_app_tof_init_once()
{
    static bool is_init = false;

    if (is_init)
        return;

    kmdw_tof_config_init();

    is_init = true;
}

void kdp2_app_tof_ir_aec_init_once(uint32_t nir_output_buf, int32_t src_width, uint32_t src_height)
{
    static bool is_init = false;

    if (is_init)
        return;

    kmdw_tof_ir_bright_init(nir_output_buf, src_width, src_height);

    is_init = true;
}

static kdp2_app_tof_aec_t g_tof_aec_variables = { 0 };
static int8_t need_aec_count = 0;
uint32_t tof_aec_tuning(uint8_t rslt_bright)
{

    kdp2_app_tof_aec_t *vars = &g_tof_aec_variables;

    vars->tof_aec_flag = 0;
    dbg_msg("vars->last_frame_setting: %d\n", vars->last_frame_setting);
    if ((rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD) || (rslt_bright < TOF_BRIGHT_LOWER_THRESHOLD))
    {
        vars->tof_aec_flag = 1;
        if(rslt_bright < TOF_BRIGHT_LOWER_THRESHOLD)
        {
            if (vars->tof_cur_exp_time < 1480)
                need_aec_count--;
            else
                vars->tof_aec_flag = 0;
        }
        else if(rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD)
        {
            if (vars->tof_cur_exp_time > 100)
                need_aec_count++;
            else
                vars->tof_aec_flag = 0;
        }

        int8_t aec_bad_cnt = 1;
        
        if ((need_aec_count >= aec_bad_cnt) || (need_aec_count <= (0 - aec_bad_cnt)))
        {
            uint32_t new_exp_time;
            float multi = (float)(vars->tof_cur_exp_time)/(float)(rslt_bright);
            
            if (rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD)
            {
                uint32_t change = (uint32_t)((float)(rslt_bright - TOF_BRIGHT_MIDDLE_THRESHOLD) * multi);
                new_exp_time = (vars->tof_cur_exp_time > change) ? (vars->tof_cur_exp_time - change) : 0;
            }
            else
            {
                new_exp_time = vars->tof_cur_exp_time + (uint32_t)((float)(TOF_BRIGHT_MIDDLE_THRESHOLD - rslt_bright) * multi);
            }

            if (new_exp_time > 1480)
                new_exp_time = 1480;
            else if (new_exp_time < 100)
                new_exp_time = 100;

            if (vars->tof_cur_exp_time !=  new_exp_time && vars->last_frame_setting == 0)
            {
                kmdw_printf("[AEC] tof set_exp_time %d\n", new_exp_time);
                if (KDEV_STATUS_OK == kmdw_sensor_set_exp_time(0, new_exp_time & 0x0000FFFF))
                {
                    vars->tof_cur_exp_time = new_exp_time;
                    vars->tof_aec_flag = 2;
                    vars->last_frame_setting = 2;
                }
            }
            else
            {
                if(vars->last_frame_setting > 0)
                    vars->last_frame_setting = vars->last_frame_setting - 1;
                vars->tof_aec_flag = 0;
            }
            need_aec_count = 0;
        }
    }
    else
    {
        if(vars->last_frame_setting > 0)
            vars->last_frame_setting = vars->last_frame_setting - 1;
        vars->tof_cur_bright = rslt_bright;
        need_aec_count = 0;
    }

    if (2 == vars->tof_aec_flag)
        return 1;
    else
        return 0;
}

void tof_aec_search(uint8_t rslt_bright)
{
    kdp2_app_tof_aec_t *vars = &g_tof_aec_variables;
    dbg_msg("vars->last_frame_setting: %d\n", vars->last_frame_setting);
    uint32_t new_exp_time = vars->tof_cur_exp_time;

    if ((rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD) || (rslt_bright < TOF_BRIGHT_LOWER_THRESHOLD))
    {
        vars->tof_aec_flag = 1;
        if(rslt_bright < TOF_BRIGHT_LOWER_THRESHOLD)
        {
            if (vars->tof_cur_exp_time < 1480)
                need_aec_count--;
            else
                vars->tof_aec_flag = 0;
        }
        else if(rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD)
        {
            if (vars->tof_cur_exp_time > 100)
                need_aec_count++;
            else
                vars->tof_aec_flag = 0;
        }
        int8_t aec_bad_cnt = 1;
        if ((need_aec_count >= aec_bad_cnt) || (need_aec_count <= (0 - aec_bad_cnt)))
        {
            if (rslt_bright < TOF_BRIGHT_LOWER_THRESHOLD)
                new_exp_time = 1480;
            else if (rslt_bright > TOF_BRIGHT_UPPER_THRESHOLD)
                new_exp_time = 100;

            if (vars->tof_cur_exp_time !=  new_exp_time && vars->last_frame_setting == 0)
            {
                kmdw_printf("[AEC] tof set_exp_time %d\n", new_exp_time);
                if (KDEV_STATUS_OK == kmdw_sensor_set_exp_time(0, new_exp_time & 0x0000FFFF))
                {
                    vars->tof_cur_exp_time = new_exp_time;
                    vars->tof_aec_flag = 2;
                    vars->last_frame_setting = 2;
                }
            }
            else
            {
                if(vars->last_frame_setting > 0)
                    vars->last_frame_setting = vars->last_frame_setting - 1;
                vars->tof_aec_flag = 0;
            }
        }
    }else{
        vars->tof_aec_flag = 0;
    }

}

#define TOF_RAW_INPUT_SIZE ((TOF_RAW_WIDTH * TOF_RAW_HEIGHT * 5 + TOF_RAW_WIDTH * 5) * 2) // FIXME
extern uint32_t g_tof_mode; // get tof mode from customized command

void kdp2_app_tof_inference(int job_id, int num_input_buf, void **inf_input_buf_list)
{
    kdp2_app_tof_init_once();

    if (1 != num_input_buf) {
        kmdw_inference_app_send_status_code(job_id, KP_FW_WRONG_INPUT_BUFFER_COUNT_110);
        return;
    }

    // input TOF RAW data
    kdp2_ipc_app_tof_inf_header_t *tof_raw_header = (kdp2_ipc_app_tof_inf_header_t *)inf_input_buf_list[0];
    void *tof_raw_buf = (void *)tof_raw_header->raw_data;

    // get a new result buffer and arrange layout
    int result_buf_size;
    kdp2_app_tof_result_t *tof_result = (kdp2_app_tof_result_t *)kmdw_fifoq_manager_result_get_free_buffer(&result_buf_size);

    void *nir_output_buf = (void *)tof_result->nir_data;
    void *depth_output_buf = (void *)tof_result->depth_data;

    kdp2_app_tof_data_t *tof_data = &tof_result->tof_data;
    tof_data->box_count = 0;

    // temporary yolo output buf
    kp_app_yolo_result_t *yolo_inf_output = (kp_app_yolo_result_t *)((uint32_t)tof_result + sizeof(kdp2_app_tof_result_t));

    // decode TOF raw data
    int tof_decode_status = kmdw_tof_decode((uint32_t)tof_raw_buf, TOF_RAW_INPUT_SIZE, (uint32_t)depth_output_buf, (uint32_t)nir_output_buf);
    if(tof_decode_status != 0)
    {
        kmdw_printf("kmdw_tof_decode() failed, err = %d\n", tof_decode_status);
        kmdw_inference_app_send_status_code(job_id, KP_FW_INFERENCE_ERROR_101);
        return;
    }

    if (MODE_LIVE_VIEW == g_tof_mode)
    {
        tof_result->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
        tof_result->header_stamp.status_code = KP_SUCCESS;
        tof_result->header_stamp.total_size = sizeof(kdp2_app_tof_result_t);
        tof_result->header_stamp.job_id = KDP2_INF_ID_APP_TOF;

        // send output result buffer back to host SW
        kmdw_fifoq_manager_result_enqueue((void *)tof_result, result_buf_size, false);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////

    // config image preprocessing and model settings
    kmdw_inference_app_config_t inf_config;
    memset(&inf_config, 0, sizeof(kmdw_inference_app_config_t)); // for safety let default 'bool' to 'false'

    // image buffer address should be just after the header
    inf_config.num_image = 1;

    inf_config.image_list[0].image_buf = (void *)nir_output_buf;
    inf_config.image_list[0].image_width = TOF_IMG_WIDTH;
    inf_config.image_list[0].image_height = TOF_IMG_HEIGHT;
    inf_config.image_list[0].image_channel = TOF_IMG_CHANNEL;
    inf_config.image_list[0].image_format = KP_IMAGE_FORMAT_RAW8;
    inf_config.image_list[0].image_norm = KP_NORMALIZE_KNERON;
    inf_config.image_list[0].image_resize = KP_RESIZE_ENABLE;   // enable resize
    inf_config.image_list[0].image_padding = KP_PADDING_CORNER; // enable padding on corner
    inf_config.model_id = KNERON_YOLOV5S_COCO80_640_640_3;
    inf_config.enable_parallel = false;                   // only works for single model and post-process in ncpu
    inf_config.inf_result_buf = tof_result;               // for callback
    inf_config.inf_result_buf_size = result_buf_size;     //
    inf_config.ncpu_result_buf = (void *)yolo_inf_output; // give result buffer for ncpu/npu, callback will carry it

    map_model_post_proc_t mapping = get_model_post_proc_param(inf_config.model_id);
    inf_config.user_define_data = mapping.post_proc_params; // FIXME: if NULL what happen ?

    int status = kmdw_inference_app_execute(&inf_config);

    bounding_box_t *single_tof_box = (bounding_box_t*) tof_ir_bright_buf_addr;
    bool is_target_found = false;
    // process yolo data and calculate depth
    // pass single_tof_box for aec
    yolo_result_callback(status, tof_result, result_buf_size, yolo_inf_output, single_tof_box, &is_target_found);

#if ENABLE_TOF_AEC
    // calculate the average ir sum of the single_tof_box
    kdp2_app_tof_ir_aec_init_once((uint32_t)nir_output_buf, TOF_IMG_WIDTH, TOF_IMG_HEIGHT);

    kmdw_tof_ir_bright_aec((uint32_t)nir_output_buf, TOF_IMG_WIDTH, TOF_IMG_HEIGHT, single_tof_box, is_target_found);

    ir_bright_result_t* result = model_ir_bright_get_rslt();

    dbg_msg("result: %d is_target_found: %d\n", result->rslt_bright, is_target_found);
    if(is_target_found){
        // fine tune the exposure time based on average ir brightness of the target
        tof_aec_tuning(result->rslt_bright);
    }else{
        // update the exposure time based on average ir brightness of the full image
        tof_aec_search(result->rslt_bright);
    }

#endif

    osDelay(5); // workaround for DSP
}
