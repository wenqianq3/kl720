/*
 * Kneron Application initialization
 *
 * Copyright (C) 2022 Kneron, Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include "cmsis_os2.h"

// inference core
#include "kp_struct.h"
#include "kmdw_console.h"
#include "kmdw_inference_app.h"

// inference app
#include "kdp2_inf_app_tof.h"

// inference client
extern int kdp2_hico_tof_init(void);

#define MAX_IMAGE_COUNT   10          /**< MAX inference input  queue slot count */
#define MAX_RESULT_COUNT  10          /**< MAX inference output queue slot count */


/**
 * @brief To register AI applications
 * @param[in] num_input_buf number of data inputs in list
 * @param[in] inf_input_buf_list list of data input for inference task
 * @return N/A
 * @note Add a switch case item for a new inf_app application
 */
static void _app_func(int num_input_buf, void **inf_input_buf_list);


void _app_func(int num_input_buf, void **inf_input_buf_list)
{
    // check header stamp
    if (0 >= num_input_buf) {
        kmdw_printf("No input buffer for app function\n");
        return;
    }

    void *first_inf_input_buf = inf_input_buf_list[0];
    kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)first_inf_input_buf;
    uint32_t job_id = header_stamp->job_id;

    switch (job_id) {
    case KDP2_INF_ID_APP_TOF:
        kdp2_app_tof_inference(job_id, num_input_buf, inf_input_buf_list);
        break;
    default:
        kmdw_inference_app_send_status_code(job_id, KP_FW_ERROR_UNKNOWN_APP);
        break;
    }
}


void app_initialize(void)
{
    kmdw_printf(">> Start running KL720 KDP2 HICO(MIPI-in, USB-out) mode ...\n");

    /* initialize inference app */
    /* register APP functions */
    /* specify depth of inference queues */
    kmdw_inference_app_init(_app_func, MAX_IMAGE_COUNT, MAX_RESULT_COUNT);

    /* HICO tof init */
    kdp2_hico_tof_init();

    return;
}
