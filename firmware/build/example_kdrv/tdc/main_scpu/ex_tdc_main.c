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
#include "project.h"
#include "system_init.h"
#include "driver_init.h"
#include "kdrv_uart.h"
#include "kdrv_timer.h"
#include "kmdw_console.h"
#include "kdrv_system.h"
#include "kdrv_cmsis_core.h"

#define CAT_AUX(a, b) a##b
#define CAT(a, b) CAT_AUX(a, b)

#define POWER_MODE  CAT(POWER_MODE_,       POWER_MODE_INIT)
#define WKUP_SRC    (WKUP_SRC_RTC<<17 | WKUP_SRC_EXT_BUT<<20 | WKUP_SRC_USB_SUPER_SPEED << 19 | WKUP_SRC_USB_HIGH_SPEED<<18)

sysclockopt sysclk_opt = {
    .axi_ddr    = AXI_DDR_MHZ,
    .mrx1       = MRX1_MHZ,
    .mrx0       = MRX0_MHZ,
    .npu        = NPU_MHZ,
    .dsp        = DSP_MHZ,
    .audio      = AUDIO_MHZ,
};

extern kdrv_status_t tdc_selftest(void);


/**
 * @brief main, main dispatch function
 */
int main(void)
{

    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS

#ifndef _BOARD_SN720HAPS_H_
    kdrv_system_initialize(POWER_MODE, (uint32_t)WKUP_SRC, &sysclk_opt);
#endif

    /* below is some middleware init settings */
    kdrv_uart_initialize();                                                            // for log printing
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);           // uart console initial
    tdc_selftest();

    //application is triggered in host_com.c
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    while (1) {
    }
}

