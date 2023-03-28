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
#include <cmsis_os2.h>
#include "kdrv_pinmux.h"
#include "kdrv_timer.h"
#include "kdrv_i2c.h"
#include "kdrv_i2s.h"
#include "kdrv_gdma.h"

static uint32_t pinmux_array[PIN_NUM] = PINMUX_ARRAY;
static i2c_attr_context i2c_ctx[I2C_NUM] = I2C_ATTR_ARRAY;
static kdrv_i2s_attr_context i2s_ctx[I2S_NUM] = {\
                                                {I2S_PORT, COMM_I2S_MODE_MASTER_STEREO, COMM_I2S_AUDIO_FREQ_32K, COMM_I2S_MSB_FIRST, 16, 16, COMM_I2S_PAD_BACK , 0} };
 //Function
void drv_initialize(void)
{
    uint32_t ret = 0;
    kdrv_uart_initialize();  
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);           // uart console initial
    kdrv_pinmux_initialize(PIN_NUM, pinmux_array);
    kdrv_timer_initialize();
    kdrv_timer_perf_measure_start();
    ret = kdrv_ddr_Initialize(AXI_DDR_MHZ);
    if(ret != 0)
    {
        while(1)
        {};
    }
 
    // init i2c
    kdrv_i2c_initialize(I2C_NUM, i2c_ctx);
    
    // int dma
    kdrv_gdma_initialize();
        
    // init i2s
    kdrv_i2s_init(I2S_NUM, i2s_ctx);
}

