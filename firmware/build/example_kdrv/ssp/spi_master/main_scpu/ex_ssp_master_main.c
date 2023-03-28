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

#define DEBUG_SPI_PORT SSP_SPI_PORT0
uint8_t rx_buffer[16]={};
uint8_t rx_index=0;
extern struct st_ssp_spi driver_ssp_ctx;
extern void kdrv_ssp_rx_polling_receive_all( struct st_ssp_spi *stspi );
#define min(a, b)   (((a)<(b))?(a):(b))

uint8_t ssp_spi_master_test(void)
{
    uint8_t	tx_temp_size = min(kdrv_ssp_txfifo_depth( DEBUG_SPI_PORT ),kdrv_ssp_rxfifo_depth( DEBUG_SPI_PORT ));
    //uint16_t	ncount = 0;
    uint16_t i = 0;
    uint8_t  temp_buffer[tx_temp_size];
    uint16_t temp_buffer_index = 0;

    for( i=0; i< tx_temp_size; i++ )
    {
        temp_buffer[i] = i;
        temp_buffer_index++;
    }

    err_msg(" ssp_spi_master_test : Init\n");
    if( kmdw_ssp_api_spi_init(DEBUG_SPI_PORT, e_spi_init_master,NULL) != 1 )
    {
        return 0;
    }
    regSSP0_ctrl(driver_ssp_ctx.port_no)->st.bf.kdrv_ssp_sspcr0.LBM=1; //please set to 0 if not test in loop back mode

    err_msg(" ssp_spi_master_test : Start\n");
    if(temp_buffer_index > driver_ssp_ctx.target_Rxfifo_depth)
        temp_buffer_index = driver_ssp_ctx.target_Rxfifo_depth;
    while(1)
    {
        //control GPIO
        kdrv_ssp_spi_CS_set(chip_select_pin, SPI_CS_LOW);

		kmdw_ssp_api_spi_write_tx_buff( temp_buffer, temp_buffer_index );
        kmdw_ssp_api_spi_transfer(DEBUG_SPI_PORT);

        //kdrv_delay_us( 30 );
        while(!kmdw_ssp_api_spi_transfer_checks(DEBUG_SPI_PORT));

        //control GPIO
        kdrv_ssp_spi_CS_set(chip_select_pin, SPI_CS_HI);
        
        //uint8_t nsize = kdrv_ssp_rxfifo_valid_entries( driver_ssp_ctx.port_no );
        //if(nsize)
        {
            driver_ssp_ctx.Rx_buffer_index=0;
            kdrv_ssp_rx_polling_receive_all(&driver_ssp_ctx);
            uint32_t k = driver_ssp_ctx.Rx_buffer_index;
            if(k == temp_buffer_index)
            {
                //ssp_write_word(SSP_REG_BASE_S, &driver_ssp_ctx.Rx_buffer ,k);
                kmdw_printf("\nrx data ");
                for(uint32_t i=0; i<k; i++)
                {
                    kmdw_printf(" 0x%X",*( driver_ssp_ctx.Rx_buffer+ i ));
                }
                kmdw_printf("\n");
                if ( memcmp((const void *)temp_buffer, (const void *)driver_ssp_ctx.Rx_buffer, tx_temp_size) != 0)
                    kmdw_printf("SPI MASTER LOOPBACK TEST FAIL!\r\n");
                else
                    kmdw_printf("SPI MASTER LOOPBACK TEST PASS.\r\n");
            }
            else 
            {
                kmdw_printf("Only receive %d bytes.\r\n", k);
                kmdw_printf("\nrx data ");
                for(uint32_t i=0; i<k; i++)
                {
                    kmdw_printf(" 0x%X",*( driver_ssp_ctx.Rx_buffer+ i ));
                }
                kmdw_printf("\n");
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

    kmdw_printf( "SPI master test start....\n" );
    ssp_spi_master_test();

    while (1)
    {
    }
}
