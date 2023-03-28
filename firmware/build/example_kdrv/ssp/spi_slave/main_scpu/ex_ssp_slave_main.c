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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h" // ARM::CMSIS:RTOS2:Keil RTX5

#include "kdrv_system.h"
#include "kdrv_uart.h"
#include "kdrv_ddr.h"
#include "kdrv_pinmux.h"
#include "kdrv_gpio.h"
#include "kdrv_ssp.h"
#include "kdrv_gdma.h"

#include "kmdw_memory.h" //for ddr_malloc
#include "kmdw_console.h"
#include "kmdw_ssp.h"
#include "project.h"

#define DEBUG_SPI_PORT SSP_SPI_PORT1
uint8_t rx_buffer[16]={};
uint8_t rx_index=0;
extern struct st_ssp_spi driver_ssp_ctx;
extern void kdrv_ssp_rx_polling_receive_all( struct st_ssp_spi *stspi );

static osThreadId_t tid_spi_com;
static void kmdw_ssp_spi1_callback(uint32_t event)
{
    if (event & ARM_SPI_EVENT_RECEIVE_COMPLETE)
    {
        osThreadFlagsSet(tid_spi_com, FLAG_SPI1_RX_DONE);
    }

    if (event & ARM_SPI_EVENT_SEND_COMPLETE)
    {
        osThreadFlagsSet(tid_spi_com, FLAG_SPI1_TX_DONE);
    }

    if (event & ARM_SPI_EVENT_RX_TIMEOUT)
    {
        osThreadFlagsSet(tid_spi_com, FLAG_SPI1_RX_TIMEOUT);
    }

}

static void ssp_spi_slave_thread(void *argument)
{
    err_msg(" ssp_spi_slave_test : Init\n");
    if( kmdw_ssp_api_spi_init(DEBUG_SPI_PORT, e_spi_init_slave, kmdw_ssp_spi1_callback) != 1 )
    {
        err_msg(" ssp_spi_slave_test : error!!\n");
        return;
    }
    regSSP0_ctrl(driver_ssp_ctx.port_no)->st.bf.kdrv_ssp_sspcr0.LBM=0; //please set to 0 if not test in loop back mode
    kmdw_ssp_api_spi_init(DEBUG_SPI_PORT, e_spi_txrx_reinit, NULL);
    kmdw_ssp_api_spi_enable(DEBUG_SPI_PORT, &driver_ssp_ctx);

    memset((void *)driver_ssp_ctx.Rx_buffer, 0x55, 300);
    err_msg(" ssp_spi_slave_test : Start\n");
    driver_ssp_ctx.pre_size = 40;
    while(1)
    {
        if(driver_ssp_ctx.interrupt_en == 0x00)
        {
            kdrv_ssp_rx_polling_receive_all(&driver_ssp_ctx);
            /*nsize = driver_ssp_ctx.Rx_buffer_index;
            if(nsize)
            {
                //ssp_write_word(SSP_REG_BASE_S, &driver_ssp_ctx.Rx_buffer ,k);
                err_msg("\nrx data ");
                for(int i=0; i<nsize; i++)
                {
                    err_msg(" 0x%X",*( driver_ssp_ctx.Rx_buffer+ i ));
                }
                err_msg("\n");
            }*/
        }
        if(driver_ssp_ctx.interrupt_en == 0x04)
        {
            int32_t flags;
            flags = osThreadFlagsWait(FLAG_SPI1_RX_DONE, osFlagsWaitAny, osWaitForever);
            osThreadFlagsClear(flags);
            err_msg("\nrx data ");
            for(int i=0; i<driver_ssp_ctx.pre_size; i++)
            {
                err_msg(" 0x%X",*( driver_ssp_ctx.Rx_buffer+ i ));
            }
            err_msg("\n");
        }
        if(driver_ssp_ctx.interrupt_en == 0x10)
        {
            uint8_t nsize = kdrv_ssp_rxfifo_valid_entries( driver_ssp_ctx.port_no );
            if(nsize) {
            kdrv_gdma_handle_t ch = kdrv_gdma_acquire_handle();
            gdma_setting_t dma_settings;
            dma_settings.burst_size = GDMA_BURST_SIZE_1;
            dma_settings.dma_mode = GDMA_HW_HANDSHAKE_MODE;
            dma_settings.dst_width = GDMA_TXFER_WIDTH_8_BITS;
            dma_settings.src_width = GDMA_TXFER_WIDTH_8_BITS;
            dma_settings.dma_dst_req = GDMA_HW_REQ_NONE;
            dma_settings.dma_src_req = GDMA_HW_REQ_SSP1_RX;
            dma_settings.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
            dma_settings.src_addr_ctrl = GDMA_FIXED_ADDRESS;
            kdrv_gdma_configure_setting(ch, (gdma_setting_t *) &dma_settings);
            kdrv_gdma_transfer(ch,(uint32_t)(driver_ssp_ctx.Rx_buffer+driver_ssp_ctx.Rx_tempbuffer_index), (uint32_t)(&regSSP0_ctrl(driver_ssp_ctx.port_no)->st.dw.kdrv_ssp_txrxdr), nsize, NULL, NULL);
            kdrv_gdma_release_handle(ch);
            driver_ssp_ctx.Rx_tempbuffer_index += nsize;
            }
        }
    }
}
/**
 * @brief main, main dispatch function
 */
#define CAT_AUX(a, b) a##b
#define CAT(a, b) CAT_AUX(a, b)

#define POWER_MODE      CAT(POWER_MODE_,       POWER_MODE_INIT)
#define WKUP_SRC        (WKUP_SRC_RTC<<17 | WKUP_SRC_EXT_BUT<<20 | WKUP_SRC_USB_SUPER_SPEED << 19 | WKUP_SRC_USB_HIGH_SPEED<<18)
static uint32_t pinmux_array[PIN_NUM] = PINMUX_ARRAY;
static gpio_attr_context gpio_attr_ctx[GPIO_NUM] = GPIO_ATTR_ARRAY;

sysclockopt sysclk_opt = {
    .axi_ddr    = AXI_DDR_MHZ,
    .mrx1       = MRX1_MHZ,
    .mrx0       = MRX0_MHZ,
    .npu        = NPU_MHZ,
    .dsp        = DSP_MHZ,
    .audio      = AUDIO_MHZ,
};

int main(void)
{
    osKernelInitialize();    // Initialize CMSIS-RTOS

    // driver initialization
    kdrv_system_initialize(POWER_MODE, (uint32_t)WKUP_SRC, &sysclk_opt);
    kdrv_gpio_initialize(GPIO_NUM, gpio_attr_ctx);
    kdrv_pinmux_initialize(PIN_NUM, pinmux_array);
    kdrv_uart_initialize();
    kdrv_ddr_Initialize(AXI_DDR_MHZ);
    kdrv_gdma_initialize();

    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END);
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);                // uart console
    kmdw_console_queue_init();                // uart console
    kmdw_console_set_log_level_scpu(LOG_DBG);

    kmdw_printf( "SPI slave test start....\n" );
    tid_spi_com = osThreadNew(ssp_spi_slave_thread, NULL, NULL);;

    //application is triggered in host_com.c
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while (1)
    {
    }
}
