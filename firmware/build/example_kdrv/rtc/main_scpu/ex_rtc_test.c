#include "kdrv_rtc.h"
#include "kdrv_scu.h"
#include "regbase.h"
#include "membase.h"
#include "kdrv_power.h"
#include "kdrv_cmsis_core.h"
#define RTC_DEBUG
#ifdef RTC_DEBUG
#include "kmdw_console.h"
#define rtc_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define rtc_msg(fmt, ...)
#endif

#define NAP_TIME_1                  2
#define NAP_TIME_2                  60
#define FLAG_SYSTEM_RESET           BIT0
#define FLAG_SYSTEM_NAP             BIT1
#define FLAG_SYSTEM_NAP2            BIT2
#define FLAG_SYSTEM_SLEEP           BIT3
#define FLAG_SYSTEM_DEEP_SLEEP      BIT4
#define FLAG_SYSTEM_TIMER           BIT5
#define FLAG_SYSTEM_SHUTDOWN        BIT6
#define FLAG_SYSTEM_ERROR           BIT8
#define FLAG_SYSTEM_PWRBTN_FALL     BIT16
#define FLAG_SYSTEM_PWRBTN_RISE     BIT17

static void rtc_selftest_init(void)
{
    NVIC_DisableIRQ(SYS_IRQn);

    kdrv_rtc_initialize();
    regSCU->dw.int_sts = 0xffffffff;
    /* Enable PWR button interrupt and wakeup */
    regSCU->dw.int_en |= SCU_INT_WAKEUP | SCU_INT_RTC_SEC | SCU_INT_RTC_PERIODIC | SCU_INT_RTC_ALARM;
    /* Enable nap alarm interrupt */
    uint32_t nap_time = 5;
    kdrv_rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
    //kdrv_rtc_periodic_enable(PERIODIC_SEC_INT);
    //kdrv_rtc_sec_enable();
    NVIC_ClearPendingIRQ(SYS_IRQn);
    NVIC_EnableIRQ(SYS_IRQn);
}
void rtc_selftest()
{
    //Notice: Please make sure the X_OSCL_SEL pin status. 
    //        This pin is used to select scu_clk source.
    //        0: 32.768KHz form OSC pad. 
    //        1: 100KHz from internal OSC.
    //        If need accurate RTC timing, please set X_OSCL_SEL pin as 0. 
    while(1)
    {
        rtc_selftest_init();
        __WFI();
        rtc_msg("Leave WFI\n");
    }
}

