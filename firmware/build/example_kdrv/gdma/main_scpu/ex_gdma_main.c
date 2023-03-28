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
#include "kdrv_uart.h"
#include "kmdw_memory.h" //for ddr_malloc
#include "kmdw_console.h"

#include <stdlib.h>
#include "kdrv_gdma.h"
#include "kdrv_timer.h"
#include "kdrv_system.h"
#include "kdrv_ddr.h"
#include "kdrv_cmsis_core.h"
#include "project.h"

#define CAT_AUX(a, b) a##b
#define CAT(a, b) CAT_AUX(a, b)

#define POWER_MODE  CAT(POWER_MODE_,       POWER_MODE_INIT)
#define WKUP_SRC    (WKUP_SRC_RTC<<17 | WKUP_SRC_EXT_BUT<<20 | WKUP_SRC_USB_SUPER_SPEED << 19 | WKUP_SRC_USB_HIGH_SPEED<<18)

typedef enum
{
    TEST_GDMA_MEMCPY,
    TEST_GDMA_MEMCPY_ASYNC,
    TEST_GDMA_TRANSFER,
    TEST_GDMA_TRANSFER_ASYNC,
    TEST_SPECIFIED_DMA_CHANNEL,
} GDMA_TEST_ITEM;

volatile static int g_dma_waiting = 1;
volatile static int g_dma_cb_sts = 0;

void dma_cb(kdrv_status_t status, void *arg)
{
    if (status != KDRV_STATUS_OK)
    {
        kmdw_printf("GDMA callback : failed !\n");
        g_dma_cb_sts = 0;
    }
    else
        g_dma_cb_sts = 1;

    g_dma_waiting = 0;
}

static int one_dma_memcpy_test(uint32_t dstAddr, uint32_t srcAddr, uint32_t size,
                               uint32_t loop, uint32_t test_item, int ch)
{

    kdrv_timer_perf_measure_start();

    memset((void *)srcAddr, 0, size);
    memset((void *)dstAddr, 0, size);

    for (int i = 0; i < size; i++)
    {
        *(uint8_t *)(srcAddr + i) = (rand() % 0XFF);
    }

    uint32_t tickdiff, tick_start, tick_end;

    if (test_item == TEST_GDMA_MEMCPY)
    {
        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (uint32_t i = 0; i < loop; i++)
        {
            kdrv_status_t dma_sts = kdrv_gdma_memcpy(dstAddr, srcAddr, size, NULL, NULL);
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("kdrv_gdma_memcpy() failed at dst 0x%x src 0x%s loop %d\n", dstAddr, srcAddr, loop);
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);
    }
    else if (test_item == TEST_GDMA_MEMCPY_ASYNC)
    {
        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (uint32_t i = 0; i < loop; i++)
        {
            g_dma_waiting = 1;
            kdrv_status_t dma_sts = kdrv_gdma_memcpy(dstAddr, srcAddr, size, dma_cb, NULL);
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("kdrv_gdma_memcpy() failed at dst 0x%x src 0x%s loop %d\n", dstAddr, srcAddr, loop);
            while (g_dma_waiting)
                ;
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);
    }
    else if (test_item == TEST_GDMA_TRANSFER)
    {
        kdrv_gdma_handle_t dma_handle = kdrv_gdma_acquire_handle();
        gdma_setting_t dma_setting;

        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.burst_size = GDMA_BURST_SIZE_16;
        dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.dma_mode = GDMA_NORMAL_MODE;
        dma_setting.dma_dst_req = 0;
        dma_setting.dma_src_req = 0;

        kdrv_gdma_configure_setting(dma_handle, &dma_setting);

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (uint32_t i = 0; i < loop; i++)
        {
            kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, dstAddr, srcAddr, size, NULL, NULL);
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("kdrv_gdma_transfer() failed at dst 0x%x src 0x%s loop %d\n", dstAddr, srcAddr, loop);
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);

        kdrv_gdma_release_handle(dma_handle);
    }
    else if (test_item == TEST_GDMA_TRANSFER_ASYNC)
    {
        kdrv_gdma_handle_t dma_handle = kdrv_gdma_acquire_handle();
        gdma_setting_t dma_setting;

        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.burst_size = GDMA_BURST_SIZE_16;
        dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.dma_mode = GDMA_NORMAL_MODE;
        dma_setting.dma_dst_req = 0;
        dma_setting.dma_src_req = 0;

        kdrv_gdma_configure_setting(dma_handle, &dma_setting);

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (uint32_t i = 0; i < loop; i++)
        {
            g_dma_waiting = 1;
            kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, dstAddr, srcAddr, size, dma_cb, NULL);
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("kdrv_gdma_transfer() failed at dst 0x%x src 0x%s loop %d\n", dstAddr, srcAddr, loop);
            while (g_dma_waiting)
                ;
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);

        kdrv_gdma_release_handle(dma_handle);
    }
    else if (test_item == TEST_SPECIFIED_DMA_CHANNEL)
    {
        kdrv_gdma_handle_t dma_handle = ch;
        gdma_setting_t dma_setting;

        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.burst_size = GDMA_BURST_SIZE_16;
        dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.dma_mode = GDMA_NORMAL_MODE;
        dma_setting.dma_dst_req = 0;
        dma_setting.dma_src_req = 0;

        kdrv_gdma_configure_setting(dma_handle, &dma_setting);

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (uint32_t i = 0; i < loop; i++)
        {
            kdrv_status_t dma_sts = kdrv_gdma_transfer(dma_handle, dstAddr, srcAddr, size, NULL, NULL);
            if (dma_sts != KDRV_STATUS_OK)
                kmdw_printf("kdrv_gdma_transfer() failed at dst 0x%x src 0x%x loop %d\n", dstAddr, srcAddr, loop);
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);
    }

    int cmp_sts = memcmp((const void *)srcAddr, (const void *)dstAddr, size);

    float speed = (float)(size * loop) / (tick_end - tick_start);
    speed = speed * 1000 / 1024;

    //kdrv_pwmtimer_close(PWMTIMER1);

    kmdw_printf("[memcpy] tid 0x%x copy 0x%x to 0x%x %d bytes x %d, spent %u us speed: %1.f MB/s ... %s\n",
                osThreadGetId(), srcAddr, dstAddr, size, loop, (tick_end - tick_start), speed, (cmp_sts == 0) ? "PASSED" : "FAILED");

    return (cmp_sts == 0);
}

static void test_dma_functions()
{

    kmdw_printf("\n================= [%s context] =================\n\n", osThreadGetId() == 0 ? "Non-OS" : "OS");

    const uint32_t size_begin = 16;
    const uint32_t count_begin = 10240;

    static uint32_t dstAddr = 0;
    static uint32_t srcAddr = 0;

    int total_pass = 0;
    int total_fail = 0;

    if (dstAddr == 0)
    {
        dstAddr = kmdw_ddr_reserve(4 * 1024 * 1024);
        srcAddr = kmdw_ddr_reserve(4 * 1024 * 1024);
    }

    kmdw_printf("dstAddr = 0x%x, srcAddr = 0x%x\n", dstAddr, srcAddr);

    {
        kmdw_printf("\n");
        kmdw_printf("[ 8/16/32 bit address x 1/2/4 divisible length tests ]\n");
        kmdw_printf("\n");

        int pass_count = 0;
        int fail_count = 0;

        for (int i = 0; i < 2048; i++)
            *(uint8_t *)(srcAddr + i) = 1 + i % 0xff;

        for (int i = 0; i < 4; i++)
        {
            uint32_t src = srcAddr + i;
            for (int j = 0; j < 4; j++)
            {
                uint32_t dst = dstAddr + j;
                for (int k = 0; k < 4; k++)
                {
                    uint32_t num = 1024 + k;
                    memset((void *)dstAddr, 0, 2048);
                    kdrv_gdma_memcpy(dst, src, num, NULL, NULL);
                    int cmp = memcmp((void *)dst, (void *)src, num);
                    if (cmp == 0)
                    {
                        kmdw_printf("copy %d bytes from 0x%x to 0x%x ... PASSED\n", num, src, dst);
                        pass_count++;
                    }
                    else
                    {
                        kmdw_printf("copy %d bytes from 0x%x to 0x%x ... FAILED\n", num, src, dst);
                        fail_count++;
                    }
                }
            }
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_memcpy() tests ]\n");

        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\nAaddress and transfer size are not divisiable by 4:\n");

        for (int i = 0; i < 4; i++)
            pass_count += one_dma_memcpy_test(dstAddr + i, srcAddr + i, (2 * 1024) + i, 50, TEST_GDMA_MEMCPY, 0);

        kmdw_printf("\nDifferent transfer size:\n");

        uint32_t size = size_begin;
        uint32_t count = count_begin;
        for (int i = 0; i < 11; i++)
        {
            if (one_dma_memcpy_test(dstAddr, srcAddr, size, count, TEST_GDMA_MEMCPY, 0))
                pass_count++;
            else
                fail_count++;
            size *= 2;
            count /= 2;
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_memcpy() async tests ]\n");

        kmdw_printf("\nDifferent transfer size:\n");

        uint32_t size = size_begin;
        uint32_t count = count_begin;

        for (int i = 0; i < 11; i++)
        {
            if (one_dma_memcpy_test(dstAddr, srcAddr, size, count, TEST_GDMA_MEMCPY_ASYNC, 0))
                pass_count++;
            else
                fail_count++;
            size *= 2;
            count /= 2;
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_transfer() tests ]\n");

        kmdw_printf("\nDifferent transfer size:\n");

        uint32_t size = size_begin;
        uint32_t count = count_begin;

        for (int i = 0; i < 11; i++)
        {
            if (one_dma_memcpy_test(dstAddr, srcAddr, size, count, TEST_GDMA_TRANSFER, 0))
                pass_count++;
            else
                fail_count++;
            size *= 2;
            count /= 2;
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_transfer() async tests ]\n");

        kmdw_printf("\nDifferent transfer size:\n");

        uint32_t size = size_begin;
        uint32_t count = count_begin;

        for (int i = 0; i < 11; i++)
        {
            if (one_dma_memcpy_test(dstAddr, srcAddr, size, count, TEST_GDMA_TRANSFER_ASYNC, 0))
                pass_count++;
            else
                fail_count++;

            size *= 2;
            count /= 2;
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_transfer() test all DMA channels ]\n\n");

        uint32_t size = 1024;
        uint32_t count = 50;

        for (int i = 0; i < 4; i++)
        {
            kmdw_printf("DMA CH %d: ", i);
            if (one_dma_memcpy_test(dstAddr, srcAddr, size, count, TEST_SPECIFIED_DMA_CHANNEL, i))
                pass_count++;
            else
                fail_count++;
        }

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    {
        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_allocate_copy_list() tests ]\n\n");

        int pass_count = 0;
        int fail_count = 0;

        kdrv_gdma_handle_t dma_handle = kdrv_gdma_acquire_handle();

        gdma_setting_t dma_setting;

        dma_setting.dst_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.src_width = GDMA_TXFER_WIDTH_32_BITS;
        dma_setting.burst_size = GDMA_BURST_SIZE_32;
        dma_setting.dst_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.src_addr_ctrl = GDMA_INCREMENT_ADDRESS;
        dma_setting.dma_mode = GDMA_NORMAL_MODE;

        kdrv_gdma_configure_setting(dma_handle, &dma_setting);

        int img_width = 1280;
        int img_height = 720;
        int cropping_x = 100;
        int cropping_y = 100;
        int cropping_width = 600;
        int cropping_height = 600;

        int num = cropping_height;
        static uint32_t *dst_set = 0;
        static uint32_t *src_set = 0;
        static uint32_t *size_set = 0;

        if (dst_set == 0)
        {
            dst_set = (uint32_t *)kmdw_ddr_reserve(num * 4);
            src_set = (uint32_t *)kmdw_ddr_reserve(num * 4);
            size_set = (uint32_t *)kmdw_ddr_reserve(num * 4);
        }

        for (int i = 0; i < num; i++)
        {
            src_set[i] = srcAddr + img_width * (cropping_y + i) + cropping_x;
            dst_set[i] = dstAddr + img_width * (i) + 0;
            size_set[i] = cropping_width;
        }

        kdrv_gdma_copy_list_t copy_list = kdrv_gdma_allocate_copy_list(dma_handle, dst_set, src_set, size_set, num);

        for (int i = 0; i < img_width * img_height / 4; i++)
        {
            ((uint32_t *)srcAddr)[i] = i;
        }

        memset((void *)dstAddr, 0x0, 1 * img_width * img_height);

        kdrv_timer_perf_measure_start();
        
        int loop = 3;

        uint32_t tickdiff, tick_start, tick_end;

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (int i = 0; i < loop; i++)
        {
            kdrv_status_t dma_sts = kdrv_gdma_transfer_copy_list(dma_handle, copy_list, NULL, NULL);
            if (dma_sts != KDRV_STATUS_OK)
            {
                kmdw_printf("kdrv_gdma_allocate_copy_list() failed\n");
                fail_count++;
            }
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);

        kdrv_gdma_free_copy_list(copy_list);
        kdrv_gdma_release_handle(dma_handle);

        float speed = (float)(cropping_width * cropping_height * loop) / (tick_end - tick_start);
        speed = speed * 1000 / 1024;

        if (fail_count == 0)
            pass_count++;

        if (pass_count > 0)
        {
            for (int i = 0; i < num; i++)
            {
                int cmp = memcmp((void *)src_set[i], (void *)dst_set[i], cropping_width);
                if (cmp != 0)
                {
                    kmdw_printf("memcmp() failed at src 0x%x dst 0x%x size %d\n", src_set[i], dst_set[i], size_set[i]);
                    fail_count++;
                    pass_count--;
                    break;
                }
            }
        }

        kmdw_printf("[memcpy] tid 0x%x copy_list %d bytes x %d, spent %u us speed: %1.f MB/s ... %s\n",
                    osThreadGetId(), (cropping_width * cropping_height), loop, (tick_end - tick_start), speed, (pass_count == 1) ? "PASSED" : "FAILED");

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;

        //kdrv_pwmtimer_close(PWMTIMER1);
    }

    {
        kmdw_printf("\n");
        kmdw_printf("[ kdrv_gdma_memcpy_cropping() tests ]\n\n");

        int pass_count = 0;
        int fail_count = 0;

        int img_width = 1280;
        int img_height = 720;
        int cropping_x = 60;
        int cropping_y = 120;
        int cropping_width = 800;
        int cropping_height = 500;

        gdma_cropping_descriptor_t cropping;
        cropping.start_src_addr = srcAddr + img_width * cropping_y + cropping_x;
        cropping.start_dst_addr = dstAddr;
        cropping.txfer_bytes = cropping_width;
        cropping.src_offset = img_width;
        cropping.dst_offset = img_width;
        cropping.num_txfer = cropping_height;

        memset((void *)dstAddr, 0x0, 1 * img_width * img_height);

        kdrv_timer_perf_measure_start();

        int loop = 3;

        uint32_t tickdiff, tick_start, tick_end;

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_start);

        for (int i = 0; i < loop; i++)
        {
            kdrv_status_t dma_sts = kdrv_gdma_memcpy_cropping(&cropping, NULL, NULL);
            if (dma_sts != KDRV_STATUS_OK)
            {
                kmdw_printf("kdrv_gdma_memcpy_cropping() failed\n");
                fail_count++;
            }
        }

        kdrv_timer_perf_measure_get_us(&tickdiff, &tick_end);

        float speed = (float)(cropping_width * cropping_height * loop) / (tick_end - tick_start);
        speed = speed * 1000 / 1024;

        if (fail_count == 0)
            pass_count++;

        if (pass_count > 0)
        {
            for (int i = 0; i < cropping_height; i++)
            {
                uint32_t src = cropping.start_src_addr + i * cropping.src_offset;
                uint32_t dst = cropping.start_dst_addr + i * cropping.dst_offset;
                int cmp = memcmp((void *)src, (void *)dst, cropping_width);
                if (cmp != 0)
                {
                    kmdw_printf("memcmp() failed at src 0x%x dst 0x%x size %d\n", src, dst, cropping_width);
                    fail_count++;
                    pass_count--;
                    break;
                }
            }
        }

        kmdw_printf("[memcpy] tid 0x%x copy_list %d bytes x %d, spent %u us speed: %1.f MB/s ... %s\n",
                    osThreadGetId(), (cropping_width * cropping_height), loop, (tick_end - tick_start), speed, (pass_count == 1) ? "PASSED" : "FAILED");

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;

        //kdrv_pwmtimer_close(PWMTIMER1);
    }

    {
        int pass_count = 0;
        int fail_count = 0;

        kmdw_printf("\n");
        kmdw_printf("[ DMA abort tests ]\n\n");

        kmdw_printf("doing dma memcpy 4MB and aborting it ...\n");

        g_dma_waiting = 1;
        kdrv_status_t dma_sts = kdrv_gdma_memcpy(dstAddr, srcAddr, 4 * 1024 * 1024, dma_cb, NULL);
        if (dma_sts != KDRV_STATUS_OK)
            kmdw_printf("kdrv_gdma_memcpy() failed at dst 0x%x src 0x%s\n", dstAddr, srcAddr);

        kdrv_gdma_abort_transfer(0);

        while (g_dma_waiting)
            ;

        kmdw_printf("[memcpy] aborting test ... %s\n", (g_dma_cb_sts == 0) ? "PASSED" : "FAILED");

        if (g_dma_cb_sts == 0)
            pass_count++;
        else
            fail_count++;

        kmdw_printf("\n");
        kmdw_printf("doing dma memcpy 4MB and not aborting it ...\n");

        g_dma_waiting = 1;
        dma_sts = kdrv_gdma_memcpy(dstAddr, srcAddr, 4 * 1024 * 1024, dma_cb, NULL);
        if (dma_sts != KDRV_STATUS_OK)
            kmdw_printf("kdrv_gdma_memcpy() failed at dst 0x%x src 0x%s\n", dstAddr, srcAddr);

        while (g_dma_waiting)
            ;

        kmdw_printf("[memcpy] dma copy w/o aborting test ... %s\n", (g_dma_cb_sts == 1) ? "PASSED" : "FAILED");

        if (g_dma_cb_sts == 1)
            pass_count++;
        else
            fail_count++;

        kmdw_printf("Passed: %d, Failed: %d\n", pass_count, fail_count);

        if (fail_count == 0)
            total_pass++;
        else
            total_fail++;
    }

    kmdw_printf("\nAll GDMA tests are done, total items passed: %d, total failed: %d\n", total_pass, total_fail);
}

static void test_gdma_thread(void *arg)
{
    test_dma_functions();
}

// DDR heap area, ~8MB, FIXME
#define DDR_HEAP_BEGIN 0x82000000
#define DDR_HEAP_END 0x80000000

sysclockopt sysclk_opt = {
    .axi_ddr    = AXI_DDR_MHZ,
    .mrx1       = MRX1_MHZ,
    .mrx0       = MRX0_MHZ,
    .npu        = NPU_MHZ,
    .dsp        = DSP_MHZ,
    .audio      = AUDIO_MHZ,
};

/**
 * @brief main, main dispatch function
 */
int main(void)
{
    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS

    kdrv_system_initialize(POWER_MODE, (uint32_t)WKUP_SRC, &sysclk_opt);
    kdrv_uart_initialize(); // for log printing
	kdrv_uart_initialize();
	kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);           // uart console initial

    kdrv_ddr_Initialize(AXI_DDR_MHZ);

    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END);

    // init GDMA resources
    kdrv_gdma_initialize();
    kdrv_timer_initialize();
    // test GDMA functions in non-OS context
    test_dma_functions();

    // create a thread to test GDMA functions in OS context
    osThreadNew(test_gdma_thread, NULL, NULL);

    //application is triggered in host_com.c
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while (1)
    {
    }
}
