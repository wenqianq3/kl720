
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cmsis_os2.h"
#include "kdrv_timer.h"
#define TIMER_DEBUG
#ifdef TIMER_DEBUG
#include "kmdw_console.h"
#define timer_msg(fmt, ...) kmdw_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define timer_msg(fmt, ...)
#endif
extern uint32_t perftimerid;


#define TESTINTERVAL        5
#define TESTLOOPCNT         1000

#define TIMER_ST_STATUS_OK       0x0000
#define TIMER_ST_STATUS_ER_1     0x0001
#define TIMER_ST_STATUS_ER_2     0x0002
uint32_t Times[5];
uint32_t loopcnt;
uint32_t timer_selftest_execute()
{
    uint32_t perftime, perftime2;
    uint32_t i;
    uint32_t status;
    status = TIMER_ST_STATUS_OK;
    kdrv_timer_perf_measure_start();
    timer_msg("Start to do Timer IP validation =======================\n");
    for(i=0 ; i < 2 ; i++)
    {
        loopcnt = 0;
        while(loopcnt < TESTLOOPCNT)
        {
            if (i == 0)
                kdrv_timer_delay_ms(TESTINTERVAL);
            else
                kdrv_timer_delay_ms_long(TESTINTERVAL);
            kdrv_timer_perf_measure_get_us(&perftime, &perftime2);
            Times[0] = perftime;
            Times[1] = perftime2;
            {
                if(loopcnt != 0)
                {
                    Times[2] += Times[0];
                }
                loopcnt++;
            }
        }
        Times[2] /= TESTLOOPCNT;
        if(i == 0)
            timer_msg("CLK SOURCE : PCKL =================================\n");
        else
            timer_msg("CLK SOURCE : EXTCLK ===============================\n");
        if(Times[2] > (TESTINTERVAL*1000 + 50))
        {
            status |= TIMER_ST_STATUS_ER_1;
            timer_msg("Timer IP test fail, time measurement too high, Loop %d, Average = %d, Target = %d\n", i, Times[2], (TESTINTERVAL*1000));
        }
        else if(Times[2] < (TESTINTERVAL*1000 - 50))
        {
            status |= TIMER_ST_STATUS_ER_2;
            timer_msg("Timer IP test fail, time measurement too low, Loop %d, Average = %d, Target = %d\n", i, Times[2], (TESTINTERVAL*1000));
        }
        else
        {
            timer_msg("Timer IP test PASS, Loop %d, Average = %d, Target = %d\n", i, Times[2], (TESTINTERVAL*1000));
        }
        timer_msg("====================================================\n");
    }
    return status;
}

void timer_selftest(void)
{
    timer_selftest_execute();
}
