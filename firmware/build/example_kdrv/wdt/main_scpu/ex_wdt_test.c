/*
 * Kneron WDT test code driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "kdrv_io.h"
#include "kdrv_status.h"
#include "kdrv_timer.h"
#include "kdrv_clock.h"
#include "kdrv_wdt.h"
#include "kmdw_console.h"
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "system_config.h"

volatile uint32_t wd_int_flag;
static void wdt_callback(void *arg);

#define WDT_EXTCLK               (1 * 1000 * 1000)

/**
 * @brief wdt_isr(), WDT ISR function
 */
//void wdt_isr()
static void wdt_callback(void *arg)
{
    wd_int_flag = 1;
    kmdw_printf("\n\rWDT Interrupt occured\n");
}


void wdt_start(uint32_t reload_cnt)
{
    kdrv_wdt_set_auto_reload(reload_cnt);
    kdrv_wdt_reset();
    kdrv_wdt_enable();               //Enable WDT
}



/*
 * @breif watchdog isr/clock test 
 */
int wdt_isr_test(void)
{
    uint32_t j, delay_cnt;
    int result = 0;
    char buf[40];
    kdrv_timer_initialize();
    kmdw_printf("\rWatchDog Test...\n");
    kmdw_printf("Please select clock source:\n");
    kmdw_printf("(1)PCLK 1sec (2)PCLK 4sec (3)WDT_EXTCLK 1MHz 1sec (4)WDT_EXTCLK 1MHz 4sec \r\n");
    kdrv_uart_read(DRVUART_PORT0, (uint8_t *)buf, 1);
    wd_int_flag = 0;
    result = false;
    delay_cnt = 0;

    kdrv_wdt_disable();
    kdrv_wdt_sys_int_enable();
    kdrv_wdt_irq_enable();
    j = atoi(buf);
    switch(j) {
        case 1: default:
            /* PCLK 1sec */
            kdrv_wdt_set_src_clock(0);
            wdt_start(APB_CLOCK);
            break;            
        case 2:
            /* PCLK 4sec */
            kdrv_wdt_set_src_clock(0);
            wdt_start(APB_CLOCK * 4);
            break;
        case 3:
            /* WDT_EXTCLK 1MHz 1sec */
            kdrv_wdt_set_src_clock(1);
            wdt_start(WDT_EXTCLK);
            break;
        case 4: 
            /* WDT_EXTCLK 1MHz 4sec */
            kdrv_wdt_set_src_clock(1);
            wdt_start(WDT_EXTCLK * 4);
            break;
    }
    kmdw_printf("\rstart!\n");
    /* set initial status */

    while (wd_int_flag == 0) {
        kdrv_delay_us(10 * 1000);
        delay_cnt++;
        if (delay_cnt > 800) {
            /* break timeout > 8sec */
            break;
        }
    }
    
    if (wd_int_flag == 1)
        result = true;

    kdrv_wdt_disable();
    kdrv_wdt_irq_disable();

    if (result) {
        kmdw_printf("\rPass!\n");
        result = 1;
    } else {
        kmdw_printf("\rFail!\n");
        result = 0;
    }

    kmdw_printf("End WatchDog Test!\n");

    return result;
}


void wdt_reset_test()
{
    uint32_t delay_cnt = 0;
    kmdw_printf("Running\n");
    kmdw_printf("The test is PASS when CPU reset\n");
    kmdw_printf("Waiting 5sec for CPU reset\n");
    //kdrv_wdt_board_reset(APB_CLOCK * 5);
    kdrv_wdt_board_reset(5 * 1000 * 1000);
    while(1) {
        #define DLY_TIME_100MS     (100 * 1000)
        delay_cnt++;
        kdrv_delay_us(DLY_TIME_100MS);
        kmdw_printf(".");
        if (delay_cnt > (APB_CLOCK * 8/DLY_TIME_100MS)) { 
            kmdw_printf("\n\rIf you see this message \"The watchdog test is FAIL\", it presents WDT function fail\n");
            return;
        }
    }
}

/*
 * @brief board reset immediately
 */ 
void wdt_board_reset_test(void)
{
    kmdw_printf("The test is PASS when WDT CPU reset immediately\n");
    kdrv_wdt_board_reset(1);
}


/*
 * @brief wdt_selftest() wdt test function
 */
void wdt_selftest(void)
{
    int32_t item;
    char buf[80];
    uint32_t val;
    #define EXT_BOOTUP_WDT          BIT(9)
    #define EXT_BOOTUP_GLOBAL_RST   BIT(12)
    #define EXT_BOOTUP_PWR_BTN      BIT(20)

    val = readl(SCU_REG_BASE + 0x00);   // Boot-up Status register in SCU100
    if (val & EXT_BOOTUP_WDT) {         // reboot by the watchdog reset
        kmdw_printf("re-boot by the watchdog reset\n");
    } else if(val & EXT_BOOTUP_GLOBAL_RST) {
        kmdw_printf("re-boot by the hardware reset\n");
    } else if (val & EXT_BOOTUP_PWR_BTN) {
        kmdw_printf("boot up by the power button\n");
    }
    kdrv_wdt_initialize();
    // set interrupt callback for WDT interrupt
    kdrv_wdt_register_callback(wdt_callback, NULL);

    while (1) {
        kdrv_wdt_set_src_clock(0); //reset clock source to default setting for different testcase
        kmdw_printf("Choose a test item:\n");
        kmdw_printf("1: Watchdog ISR/CLK Test \n");
        kmdw_printf("2: Watchdog Reset Test \n");
        kmdw_printf("3: Reset Board \n");
        kdrv_uart_read(DRVUART_PORT0, (uint8_t *)buf, 1);
        item = atoi(buf);

        switch (item) {
        case 1:
            wdt_isr_test();
            break;
        case 2:
            wdt_reset_test();
            break;
        case 3:
            wdt_board_reset_test();
            break;          // for normal case, should never reach
        default:
            kmdw_printf("*** Non-existed item! ***\n");
            break;
        }  /* -----  end switch  ----- */
    }
}

