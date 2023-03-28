/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/
 //Include
#include "project.h"
#include "kdrv_uart.h"
#include "kdrv_ddr.h"
#include "kdrv_pinmux.h"
#include "kdrv_i2c.h"
#include "kdrv_mipicsirx.h"
#include "kdrv_dpi2ahb.h"
#include "kdev_flash.h"
#include "kmdw_camera.h"
//Function 
static uint32_t pinmux_array[PIN_NUM] = PINMUX_ARRAY;
static i2c_attr_context i2c_ctx[I2C_NUM] = I2C_ATTR_ARRAY;
extern kmdw_cam_context cam_ctx[KDP_CAM_NUM];
__weak kdrv_status_t kdrv_csirx_initialize(uint32_t csirx_idx)
{
    return KDRV_STATUS_OK;
}
void drv_initialize(void)
{
    kdrv_uart_initialize();
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);                // uart console
#if !defined(_BOARD_SN720HAPS_H_)
    kdrv_pinmux_initialize(PIN_NUM, pinmux_array);
    kdrv_ddr_Initialize(AXI_DDR_MHZ);
    kdrv_i2c_initialize(I2C_NUM, i2c_ctx);
    for(uint32_t cam_id = 0; cam_id < CAM_ID_MAX ; cam_id++)
    {
        if(cam_ctx[cam_id].cam_input_type!= IMG_SRC_IN_PORT_NONE)
        {
            kdrv_csirx_initialize(cam_id);
            kdrv_dpi2ahb_initialize(cam_id);
        }
    }
    //kdev_flash_initialize();
#endif
}

