/*
 * Kneron TDC test code driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "kdrv_io.h"
#include "kdrv_status.h"
#include "kdrv_timer.h"
#include "kdrv_clock.h"
#include "kmdw_console.h"
#include "kdrv_tdc.h"
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "system_config.h"

static volatile uint8_t tdc_int_cnt;
static void tdc_callback(void *arg);
uint32_t tdc_intsts = 0; 
bool tdc_int_done = false;

#define TDC_CHDONE_THRD     BIT(8)
#define TDC_UNDER_THRD      BIT(16)
#define TDC_UPPER_THRD      BIT(24)

void tdc_callback(void *arg)
{
    tdc_int_cnt += 1;
    tdc_intsts = kdrv_tdc_int_status_read();
    tdc_int_done = true;
}

#define DLY_TIME_100MS     (200 * 1000) //FIX ME (10 * 100)

kdrv_status_t tdc_single_scan_test(void)
{
    uint32_t delay_cnt = 0;
    tdc_int_cnt = 0;
    kdrv_tdc_enable(TDC_SCAN_SINGLE, TDC_AVG_1); 
    while(1) {
        delay_cnt++;
        kdrv_delay_us(DLY_TIME_100MS);
        if (delay_cnt > 10) { 
            switch(tdc_int_cnt) {
            case 0:
                kmdw_printf("Error, TDC ISR fail.\n");
                return KDRV_STATUS_ERROR;
            case 1:
                kmdw_printf("TDC single scan pass. Temperature=%f\n", kdrv_tdc_get_temp());
                return KDRV_STATUS_OK;
            case 2: default:
                kmdw_printf("Error, TDC single scan fail.\n");
                return KDRV_STATUS_ERROR;
            }
        }
    }
}

kdrv_status_t tdc_continue_thrd_test(void)
{
    #define TDC_TEST_LOOP   100
    #define TDC_TIMEOUT     20
    #define TEMP_HTHRD      60
    #define TEMP_LTHRD      25
    uint32_t delay_cnt = 0;
    float temp = 0;
    kdrv_tdc_set_thrd(TEMP_LTHRD, TEMP_HTHRD);
    kdrv_tdc_set_thrd_enflag(1, 1);
    kdrv_tdc_enable(TDC_SCAN_CONTINUE, TDC_AVG_64); 
    kdrv_delay_us(DLY_TIME_100MS);
    kmdw_printf("==============================================================\n");
    kmdw_printf("Start Continue scan, High/Low temperature alarm test (%d times)\n",TDC_TEST_LOOP);
    kmdw_printf("Set Low/High Temperatur Threshold(%3.2f, %3.2f)\n", (float)TEMP_LTHRD, (float)TEMP_HTHRD);
    kmdw_printf("Please use hair dryer and Refrigerantn to test\n");

    while(1) {
        kdrv_delay_us(DLY_TIME_100MS);
        if (tdc_int_done) {
            tdc_int_done = false;
            temp = kdrv_tdc_get_temp();
            if (tdc_intsts & TDC_CHDONE_THRD) {
                kmdw_printf("Detect TDC channel done Temperature[%3.2f].\n", temp);
            }    
            if (tdc_intsts & TDC_UNDER_THRD) {
                kmdw_printf("Detect TDC under Threshold[%3.2f].\n", temp);
            }
            if (tdc_intsts & TDC_UPPER_THRD) {
                kmdw_printf("Detect TDC upper Threshold[%3.2f].\n", temp);
            }
        }
        if (delay_cnt > TDC_TIMEOUT) {
            if (tdc_int_cnt == 0) {
                kmdw_printf("ERROR, TDC timeout.\n");
                return KDRV_STATUS_TIMEOUT;
            }
        }
        if (tdc_int_cnt >= TDC_TEST_LOOP && delay_cnt >= TDC_TEST_LOOP) { 
            kmdw_printf("TDC Temperature test, PASS.\n", temp);
            kmdw_printf("Test done!!\n");
            return KDRV_STATUS_OK;
        }
        delay_cnt++;
    }
}


/*
 * @brief tdc_selftest() tdc test function
 */
kdrv_status_t tdc_selftest(void)
{
    kmdw_printf("Start TDC(Temperature to Digital Converter).\n");
    kdrv_tdc_initialize(0);
    kdrv_tdc_register_callback(tdc_callback, NULL);

    kmdw_printf("Start TDC single scan verification.\n");
    if (tdc_single_scan_test() != KDRV_STATUS_OK)
        return KDRV_STATUS_ERROR;
    kmdw_printf("Start TDC continue scan/ISR Done/High threshold/Low threshold verification.\n");
    kdrv_status_t ret = tdc_continue_thrd_test();
    return ret;
}

