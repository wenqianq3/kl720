#ifndef KDP2_INF_APP_TOF_H
#define KDP2_INF_APP_TOF_H

#include <stdint.h>
#include "kp_struct.h"

#define KDP2_INF_ID_APP_TOF 500

#define TOF_RAW_WIDTH 640
#define TOF_RAW_HEIGHT 240
#define TOF_IMG_WIDTH TOF_RAW_WIDTH
#define TOF_IMG_HEIGHT TOF_RAW_HEIGHT * 2
#define TOF_IMG_CHANNEL 1
#define YOLO_GOOD_BOX_MAX 100 /**< maximum number of bounding boxes for Yolo models */

#define TOF_BRIGHT_UPPER_THRESHOLD  (90)
#define TOF_BRIGHT_LOWER_THRESHOLD  (50)
#define TOF_BRIGHT_MIDDLE_THRESHOLD (TOF_BRIGHT_UPPER_THRESHOLD + TOF_BRIGHT_LOWER_THRESHOLD) / 2

#define ENABLE_TOF_AEC 1 /** enable tof aec to adjust the exposure time, for better locating the target object */

typedef struct
{
    float x1;          /**< top-left corner: x */
    float y1;          /**< top-left corner: y */
    float x2;          /**< bottom-right corner: x */
    float y2;          /**< bottom-right corner: y */
    float score;       /**< probability score */
    int32_t class_num; /**< class # (of many) with highest probability */
    uint32_t average_depth;
} __attribute__((aligned(4))) kdp2_tof_box_t;

typedef struct
{
    uint32_t class_count;                    /**< total class count */
    uint32_t box_count;                      /**< boxes of all classes */
    kdp2_tof_box_t boxes[YOLO_GOOD_BOX_MAX]; /**< box information */
} __attribute__((aligned(4))) kdp2_app_tof_data_t;

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint8_t paddings[40];                       /**< used for align 64 bytes */
    uint8_t raw_data[];

} __attribute__((aligned(4))) kdp2_ipc_app_tof_inf_header_t;

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp; // JID_HICO_TOF_RESULT
    uint8_t paddings[40];                       /**< used for align 64 bytes */
    uint8_t nir_data[640 * 480];
    uint8_t depth_data[640 * 480 * 2];
    kdp2_app_tof_data_t tof_data;

} __attribute__((aligned(4))) kdp2_app_tof_result_t;

typedef struct kdp2_app_tof_aec_struct {
    uint32_t  recog_err_not_return;
    uint32_t tof_cur_exp_time;
    uint8_t tof_aec_flag;  // 0:Ok; 1:Brightness error; 2: AEC is setting
    uint8_t tof_cur_bright;
    uint32_t last_frame_setting;
} kdp2_app_tof_aec_t;

void kdp2_app_tof_inference(int job_id, int num_input_buf, void **inf_input_buf_list);
void kdp2_app_tof_ir_aec_init_once(uint32_t nir_output_buf, int32_t src_width, uint32_t src_height);
uint32_t tof_aec_tuning(uint8_t rslt_bright);
void tof_aec_search(uint8_t rslt_bright);

#endif
