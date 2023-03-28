/**
 * @file        hico_mipi.h
 * @brief       macros and data structure for hico application of mipi camera
 * @version     0.1
 * @date        2021-07-02
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

#define JID_START_HICO_MIPI 1000 // job ID to tell FW that this customized command attends to configure camera and hico mipi parameters

#define JID_CAM_0_IMAGE 2000 // job ID for camera sensor 0 image
#define JID_CAM_1_IMAGE 2001 // job ID for camera sensor 1 image
#define JID_CAM_2_IMAGE 2002 // job ID for camera sensor 2 image
#define JID_CAM_3_IMAGE 2003 // job ID for camera sensor 3 image

#define JID_CAM_0_INF_RESULT 3000 // job ID for camera sensor 0 inference result
#define JID_CAM_1_INF_RESULT 3001 // job ID for camera sensor 1 inference result
#define JID_CAM_2_INF_RESULT 3002 // job ID for camera sensor 2 inference result
#define JID_CAM_3_INF_RESULT 3003 // job ID for camera sensor 3 inference result

#define KDP2_INF_ID_APP_YOLO 11 // job ID for configuring yolo application

typedef enum
{
    // MODE_SELF_TEST = 0, add this one ?
    MODE_NONE = 0,
    MODE_LIVE_VIEW = 1,     // only show image, not doing inference
    MODE_LIVE_VIEW_INF = 2, // show image with the inference result
} hico_mode_t;

typedef enum
{
    CAM_SENSOR_NONE = 0x0,
    CAM_SENSOR_0 = 0x1,
    CAM_SENSOR_1 = 0x2,
    CAM_SENSOR_2 = 0x4,
    CAM_SENSOR_3 = 0x8,
} sensor_selection_t;

// FIXME: Add resolution and inference model type as the future work ??
// need to know how to adjust the resolution of the camera
// currently model type is yolo v5s only
typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t mode;       // hico_mode_t
    uint32_t sensor_sel; // sensor_selection_t bit fields
    uint32_t app_job_id; // job id for application
} __attribute__((aligned(4))) hico_mipi_config_command_t;

typedef struct
{
    uint32_t img_width;  // in pixel
    uint32_t img_height; // in pixel
    uint32_t img_format; // kp_image_format_t
} __attribute__((aligned(4))) hico_mipi_camera_settings_t;

#define MAX_NUM_SENSOR 4

typedef struct
{
    /* header stamp is necessary for data transfer between host and device */
    kp_inference_header_stamp_t header_stamp;
    uint32_t num_cam_sensors;
    hico_mipi_camera_settings_t cam_settings[MAX_NUM_SENSOR];
} __attribute__((aligned(4))) hico_mipi_config_response_t;
