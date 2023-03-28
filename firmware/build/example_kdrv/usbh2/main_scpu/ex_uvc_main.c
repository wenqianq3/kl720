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

#include "cmsis_os2.h" // ARM::CMSIS:RTOS2:Keil RTX5

#include "kmdw_memory.h"   //for ddr_malloc
#include "kmdw_console.h"
#include "kdrv_system.h"
#include "kdrv_ddr.h"
#include "kdrv_uart.h"
#include "kdrv_cmsis_core.h"
#include "project.h"

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

// DDR heap area, FIXME
#define DDR_HEAP_BEGIN 0x83000000
#define DDR_HEAP_END 0x80000000

extern int ex_uvc_aveo_init(void);

static void user_thread(void *argument)
{
    int result = ex_uvc_aveo_init();
}

/**
 * @brief main, main dispatch function
 */
int main(void)
{
    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS

    kdrv_system_initialize(POWER_MODE, (uint32_t)WKUP_SRC, &sysclk_opt);
    kdrv_ddr_Initialize(AXI_DDR_MHZ);

    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END);
    kdrv_uart_initialize(); // for log printing
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);          // uart console initial

    /* init the test function */
    osThreadNew(user_thread, NULL, NULL);

    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while (1)
    {
    }
}
