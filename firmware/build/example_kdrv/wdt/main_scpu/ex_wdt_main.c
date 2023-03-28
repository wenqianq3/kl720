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
#include "kdrv_cmsis_core.h"

void wdt_selftest(void);


/**
 * @brief main, main dispatch function
 */
int main(void)
{
    sys_initialize();                 // primary system init
    drv_initialize();
    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS
    /* below is some primiary system init settings */

    /* below is some middleware init settings */
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);           // uart console initial

    wdt_selftest();
    //application is triggered in host_com.c
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    while (1) {
    }
}

