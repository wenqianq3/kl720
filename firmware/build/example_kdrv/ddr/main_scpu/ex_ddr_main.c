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

#include "cmsis_os2.h"                    // ARM::CMSIS:RTOS2:Keil RTX5
#include "kdrv_system.h"

#include "system_init.h"
#include "driver_init.h"
#include "kdrv_uart.h"
#include "kdrv_ddr.h"
#include "kdrv_cmsis_core.h"
#include "project.h"

#define DDR_TEST_DBG
#ifdef DDR_TEST_DBG
#include "kmdw_console.h"
#define ddr_test_dbg(__format__, ...) kmdw_printf(__format__, ##__VA_ARGS__)
#else
#define ddr_test_dbg(__format__, ...)
#endif
extern void ddr_selftest(void);
/**
 * @brief main, main dispatch function
 */
int main(void)
{

    SystemCoreClockUpdate(); // System Initialization

    /* below is some primiary system init settings */
    sys_initialize();                 // primary system init
    drv_initialize();

	kmdw_console_queue_init();
    kmdw_console_set_log_level_scpu(LOG_PROFILE);
#ifdef DDR_INIT_PRINT_LOG
    uint32_t ret;
    ret = kdrv_ddr_Initialize(AXI_DDR_MHZ);
    if(ret != 0)
    {
        ddr_test_dbg("\n!!!!!!!!!!!!!!!!!!!!!!!!! DDR init fail ret = %d!!!!!!!!!!!!!!!!!!!!!!!!!\n", ret);
        while(1)
        {};
    }
#endif
#if defined(DO_SELFTEST) && DO_SELFTEST == 1
    ddr_selftest();
#endif
    while (1)
    {
    }
}

