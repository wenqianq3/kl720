/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    main.c
 *      Purpose: RTX for Kneron
 *
 *---------------------------------------------------------------------------*/

#include <string.h>
#include "cmsis_os2.h"
#include "project.h"

#include "system_init.h"
#include "driver_init.h"
#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kmdw_system.h"
#include "kmdw_tdc.h"
#include "kmdw_errandserv.h"
#include "kmdw_model.h"
#include "version.h"
#include "kmdw_camera.h"
#include "kdrv_timer.h"
#include "kdrv_i2c.h"

//#include "kcomm.h"
//#include "kapp_ops.h"
//#include "kapp_center_app_ops.h"

typedef struct myCamera_ctx {
    bool init;
    enum camera_state state;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
} myCamera_exp_ctx;

myCamera_exp_ctx m_kdp_cam_exp_ctx[IMGSRC_NUM + 2] = {
    {false, CAMERA_STATE_IDLE, 0, 0, 0},
    {false, CAMERA_STATE_IDLE, 0, 0, 0},
};

static uint32_t myCamera_example_setting(uint32_t cam_idx, uint32_t width, uint32_t height, uint32_t pixelformat)
{
    m_kdp_cam_exp_ctx[cam_idx].init = true;
    m_kdp_cam_exp_ctx[cam_idx].width = width;
    m_kdp_cam_exp_ctx[cam_idx].height = height;
    m_kdp_cam_exp_ctx[cam_idx].pixelformat = pixelformat;
    return 0;
}

static void myCamera_image_callback(uint32_t cam_idx, uint32_t img_buf, uint32_t *p_new_img)
{
    kmdw_printf("image complete : cam_idx %d, img_buf 0x%p\n", cam_idx, img_buf);
    *p_new_img = img_buf;
}

static uint32_t myCamera_example_open(uint8_t cam_idx, uint32_t width, uint32_t height, uint32_t pixelformat)
{
    uint32_t ret;

    struct cam_capability cap;
    cam_format fmt;

    char fmtstr[8];
    memset(&cap, 0, sizeof(cap));
    memset(&fmt, 0, sizeof(fmt));

    myCamera_example_setting(cam_idx, width, height, pixelformat);

    if (0 != (ret = kmdw_camera_open(cam_idx)))
        return ret;

    if (0 != (ret = kmdw_camera_get_device_info(cam_idx, &cap)))
        return ret;

    fmt.width = m_kdp_cam_exp_ctx[cam_idx].width;
    fmt.height = m_kdp_cam_exp_ctx[cam_idx].height;
    fmt.pixelformat = m_kdp_cam_exp_ctx[cam_idx].pixelformat;
    kmdw_printf("[%s] cam_idx:%d, width:%d, height:%d,  pixelformat: %d\n", __func__, cam_idx, fmt.width, fmt.height, fmt.pixelformat);
    if (0 != (ret = kmdw_camera_set_frame_format(cam_idx, (cam_format *)&fmt)))
        return ret;

    if (0 != (ret = kmdw_camera_get_frame_format(cam_idx, &fmt)))
        return ret;

    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.pixelformat, 4);

    // two buffers for MIPI camera
    uint32_t buffer_addr[2] = {0};
    buffer_addr[0] = kmdw_ddr_reserve(1024*1024);
    buffer_addr[1] = kmdw_ddr_reserve(1024*1024);

    if (0 != (ret = kmdw_camera_buffer_init(cam_idx, buffer_addr[0], buffer_addr[1])))
        return ret;

    m_kdp_cam_exp_ctx[cam_idx].state = CAMERA_STATE_INITED;
    return 0;
}

static uint32_t myCamera_example_start(uint32_t cam_idx)
{
    uint32_t ret;

    if (0 != (ret = kmdw_camera_start(cam_idx, myCamera_image_callback))) {
        return ret;
    }

    m_kdp_cam_exp_ctx[cam_idx].state = CAMERA_STATE_RUNNING;
    return 0;
}
void myCameratest()
{
#if (IMGSRC_IN_0)
    myCamera_example_open(0, IMGSRC_0_WIDTH, IMGSRC_0_HEIGHT, IMGSRC_0_FORMAT);
    myCamera_example_start(0);
#endif
#if (IMGSRC_IN_1)
    myCamera_example_open(1, IMGSRC_1_WIDTH, IMGSRC_1_HEIGHT, IMGSRC_1_FORMAT);
    myCamera_example_start(1);
#endif
    kmdw_printf("============================================================================================\n");
    kmdw_printf("Camera example :\n");
    kmdw_printf("Please use keil command window to input save image cmd to store image from rgb/nir sensor.\n");
    kmdw_printf("    ex: save rgb.hex 0x62f42800, 0x62f42800+(640*480*2)\n");
    kmdw_printf("        save nir.hex 0x62979250, 0x62979250+(640*480)\n");
    kmdw_printf("Then use thirdparty's tool to verify image.\n");
    kmdw_printf("rgb sensor : size 640*480*2, with color bar image \n");
    kmdw_printf("nir sensor : size 480*640,   with gray bar image\n");
    kmdw_printf("============================================================================================\n");
}

static int sdk_main(void)
{
    sys_initialize();
    drv_initialize();
    osKernelInitialize();

    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END); // FIXME
    kmdw_console_queue_init();                // uart console
    kmdw_console_set_log_level_scpu(LOG_DBG);

    kmdw_printf(">> Start running KL720 companion mode ...\n");

    kmdw_printf("SDK v%u.%u.%u-build.%03u\n", (uint8_t)(IMG_FW_VERSION>>24), (uint8_t)(IMG_FW_VERSION>>16), (uint8_t)(IMG_FW_VERSION>>8), (uint8_t)(IMG_FW_VERSION));

    return 0;
}

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
int main(void)
{
    /* SDK main init */
    sdk_main();

    kmdw_camera_init();
    myCameratest();

    //kmdw_camera_set_inc(1, 1);
    while (1)
    {
        kdrv_timer_delay_ms(1000);
        kmdw_printf(">> Working ...\n");
        //kmdw_camera_get_exp_time(1);
    }
}
