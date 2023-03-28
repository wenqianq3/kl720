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

#include "system_init.h"
#include "driver_init.h"
#include "device_init.h"
#include "kmdw_console.h"
#include "kmdw_system.h"
#include "kdrv_cmsis_core.h"

void wdt_selftest(void);
#define scpu_msg(fmt, ...) kmdw_printf("[SCPU] " fmt, ##__VA_ARGS__)
#define ncpu_msg(fmt, ...) kmdw_printf("[NCPU] " fmt, ##__VA_ARGS__)
void kmdw_model_refresh_models(){};

/**
 * @brief main, main dispatch function
 */
    
typedef struct {
	uint32_t flag;
	uint32_t addr;
	uint32_t len;
} msg_t;

#define MSG_FLAG_NONE		0
#define MSG_FLAG_ACTIVE		1

#define NCPU_MSG_FLAG_ADDR     0x83000000
#define NCPU_MSG_BUF_ADDR      0x83000010
#define NCPU_MSG_BUF_LEN       0x1024    
    
int main(void)
{
    uint8_t buf[256];
    uint32_t cnt = 0;
    msg_t *msg;
    msg = (msg_t*) NCPU_MSG_FLAG_ADDR;
    sys_initialize();                 // primary system init
    drv_initialize();
    dev_initialize();
    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS
    scpu_msg("** Secure Boot Demo program ! **\r\n");
    scpu_msg(" This message send out from SCPU \r\n");
    scpu_msg(" Load ncpu firmware and wait for ncpu to send out the message!!  \r\n");
    msg->flag = 0;    
    memset((void*) NCPU_MSG_BUF_ADDR, 0, NCPU_MSG_BUF_LEN);
    load_ncpu_fw(1);
    while (1) {
        if(msg->flag == 1) {
            memcpy((void*)buf, (void*) msg->addr, msg->len);
            ncpu_msg("%s", buf);
            msg->flag = 0;
            cnt++;
            if (cnt >= 3)
                break;
        }
    }
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

}

