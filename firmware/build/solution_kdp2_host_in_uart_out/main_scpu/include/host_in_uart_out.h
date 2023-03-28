/**
 * @file        host_in_uart_out.h
 * @brief       macros and data structure for host application of mipi camera in and uart out
 * @version     0.1
 * @date        2021-09-30
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

#pragma once

/*
  === host image buffer layout ===

head_buf -> ---------------------------------------
            | buf_header_t                        | BUF_HEADER_SIZE
img_buf  -> |--------------------------------------
            |                                     |
            |                                     |
            |                                     |
            |             image body              |
            |                                     |
            |                                     |
            |                                     |
            |-------------------------------------|

*/

/**
 * @brief describe the specific data (especially reference count) required by host mode only
 */
typedef struct
{
    uint32_t img_buf_offset; // image buffer offset from this header
    uint32_t read_ref_count; // reference count, [NOT USED] in host in uart out scenario, but add to sync with other host mode scenarios
    uint32_t cam_index;      // sensor_selection_e
    kdp2_ipc_app_yolo_inf_header_t inf_header;
} __attribute__((aligned(4))) buf_header_t;
