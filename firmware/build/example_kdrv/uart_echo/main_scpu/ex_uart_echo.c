/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  ex_hello_world.c
*
*  Description:
*  ------------
*  Basic UART loopback example
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "kdrv_cmsis_core.h"
#include "kdrv_uart.h"
#include "kdrv_system.h"
#include "project.h"

/**
 * @brief main, main dispatch function
 */
int main(void) 
{
    uint8_t rx_buf[8] = {0};
    SystemCoreClockUpdate();            // System Initialization 
    
    // System init
#ifdef KL720
    sysclockopt sysclk_opt = {
        .axi_ddr    = AXI_DDR_MHZ,
        .mrx1       = MRX1_MHZ,
        .mrx0       = MRX0_MHZ,
        .npu        = NPU_MHZ,
        .dsp        = DSP_MHZ,
        .audio      = AUDIO_MHZ,
    };
    #define WKUP_SRC    (WKUP_SRC_RTC<<17 | WKUP_SRC_EXT_BUT<<20 | WKUP_SRC_USB_SUPER_SPEED << 19 | WKUP_SRC_USB_HIGH_SPEED<<18)
    kdrv_system_initialize(POWER_MODE_ALL_AXI_ON, WKUP_SRC, &sysclk_opt);
#endif

#ifdef KL520
    kdrv_system_init();
#endif
    
    // Setup UART handle
    kdrv_uart_handle_t handle;
    if(kdrv_uart_open(&handle, MSG_PORT, UART_MODE_SYNC_RX | UART_MODE_SYNC_TX, NULL) != KDRV_STATUS_OK){
        // UART open error
        return -1;
    }
    
    // Configure UART
    kdrv_uart_config_t cfg;
    cfg.baudrate = BAUD_115200;
    cfg.data_bits = 8;
    cfg.frame_length = 0;
    cfg.stop_bits = 1;
    cfg.parity_mode = PARITY_NONE;
    
    if(kdrv_uart_configure(handle, UART_CTRL_CONFIG, (void *)&cfg) != KDRV_STATUS_OK){
        // UART configure error
        return -1;
    }
    
    // UART write
    char msg[] = "Hello World!\n";
    kdrv_uart_write(handle, (void *)msg, strlen(msg));

    while(1) {
        // Loopback example
        kdrv_status_t status;
        status = kdrv_uart_read(handle, rx_buf, 1);
        if(status == KDRV_STATUS_OK){
             kdrv_uart_write(handle, rx_buf, 1);
        }
    }
}


