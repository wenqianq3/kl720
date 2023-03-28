/**
 * @file        hico_tof.h
 * @brief       macros and data structure for hico application of tof camera
 * @version     0.1
 * @date        2021-10-27
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#define JID_START_HICO_TOF 1001 // job ID to tell FW that this customized command attends to configure camera and hico mipi parameters

typedef enum
{
    // MODE_SELF_TEST = 0, add this one ?
    MODE_NONE = 0,
    MODE_LIVE_VIEW = 1,     // only show image, not doing inference
    MODE_LIVE_VIEW_INF = 2, // show image with the inference result
} hico_mode_t;

// FIXME: Add resolution and inference model type as the future work ??
// need to know how to adjust the resolution of the camera
// currently model type is yolo v5s only
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t mode;       // hico_mode_t
    uint32_t app_job_id; // job id for application
} __attribute__((aligned(4))) hico_tof_config_command_t;

typedef struct
{
    uint32_t img_width;  // in pixel
    uint32_t img_height; // in pixel
    uint32_t img_format; // kp_image_format_t
} __attribute__((aligned(4))) hico_tof_camera_settings_t;

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    hico_tof_camera_settings_t cam_setting;
} __attribute__((aligned(4))) hico_tof_config_response_t;
