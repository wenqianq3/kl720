
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"                    // ARM::CMSIS:RTOS2:Keil RTX5
#include "kdrv_ddr.h"
#include "kdrv_system.h"
#include <errno.h>
#include "cmsis_os2.h"
#include "regbase.h"
#include "io.h"
#include "kdrv_uart.h"
#include "kdrv_pwm.h"
#include "kdrv_timer.h"
#include "kdrv_clock.h"
#define DDRST_DEBUG
#ifdef DDRST_DEBUG
#include "kmdw_console.h"
#define ddrst_msg(fmt, ...) err_msg("[%s] " fmt, __func__, ##__VA_ARGS__)
#define ddrst_msg1(fmt, ...) err_msg("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define ddrst_msg(fmt, ...)
#define ddrst_msg1(fmt, ...)
#endif

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#define MEM_BASE (DDR_MEM_BASE)
#define MEM_SIZE (0x100000)//1MB
#define MEM_BASE4NPU (MEM_BASE)
#define PATTERN_LENGTH (17*16)
#define PATTERN1(i) ((i^0x55aa)+((i^0xaa55)<<16))

#define MOD_SZ 20
#define ITER_CNT 1//
typedef enum {
    MEMTEST86_ID_1 = 1,
    MEMTEST86_ID_2,
    MEMTEST86_ID_3,
    MEMTEST86_ID_4,
    MEMTEST86_ID_5,
    MEMTEST86_ID_6,
    MEMTEST86_ID_7,
    MEMTEST86_ID_8,
    MEMTEST86_ID_9,
    MEMTEST86_ID_10,
    MEMTEST86_ID_11,
    FARADAY_MEMTEST,
    MEMTEST_ID_MAX,

}MEMTEST_ID;

typedef int  (*ddr_cmd_func)(void);
struct ddr_cmd_info
{
    char *desc;
};
struct ddr_cmd_info ddr_cmd_array[MEMTEST_ID_MAX-1] = {
    {"MEMTEST86_ID_1    : Address test, walking ones"},
    {"MEMTEST86_ID_2    : Address test, own address"},
    {"MEMTEST86_ID_3    : Moving inversions, all ones and zeros"},
    {"MEMTEST86_ID_4    : Moving inversions, 8 bit walking ones and zeros"},
    {"MEMTEST86_ID_5    : Moving inversions, 32 bit shifting pattern"},
    {"MEMTEST86_ID_6    : Random Data"},
    {"MEMTEST86_ID_7    : Random Data Sequence"},
    {"MEMTEST86_ID_8    : Bit fade test"},
    {"MEMTEST86_ID_9    : Modulo 20 check, Random pattern"},
    {"MEMTEST86_ID_10   : Modulo 20 check, all ones and zeros"},
    {"MEMTEST86_ID_11   : Modulo 20 check, 8 bit pattern"},
    {"FARADAY_MEMTEST   : Faraday's ddr memory test"},
};

uint32_t pftimerid;

uint32_t static ddr_sram_rw_verify(void)
{
    int i, j, k, len, test_loop;
    k=0;
    int test1_err_cnt = 0;
    int test2_err_cnt = 0;
    int test3_err_cnt = 0;
    test_loop = 50;
    ddrst_msg("Test mem 1\n");
    for (k = 0; k < test_loop; k++)
    {
        for (i = 0; i < MEM_SIZE / 4; i++)
        {
            outw(MEM_BASE+4 * i, PATTERN1(i));
        }
        len = 0;
        for (i=0; i < MEM_SIZE / 4; i++)
        {
            j = inw(MEM_BASE+4*i);
            if (j != PATTERN1(i))
            {
                test1_err_cnt++;
                ddrst_msg("%d> Test mem 1 failed ev=%08x, av=%08x\n", i, PATTERN1(i), j);
                len++;
                if(len > 10)
                {
                    ddrst_msg("%d> exit\n");
                    return (1);
                }
            }
        }
        ddrst_msg("Test loop %d/%d\n", k, test_loop);
    }

    ddrst_msg("Test mem 2\n");
    for (k = 0; k < test_loop; k++)
    {
        for (i = 0; i < MEM_SIZE / 4; i++)
        {
            outw(MEM_BASE + 4 * i, (i << 16) + ((i^0xffff)));
        }
        len = 0;
        for(i = 0; i < MEM_SIZE / 4; i++)
        {
            j = inw(MEM_BASE + 4 * i);
            if ( j != ((i << 16) + (i^0xffff)))
            {
                test2_err_cnt++;
                ddrst_msg("Test mem 2 failed ev=%08x, av=%08x\n", (i<<16)+((i^0xffff)), j);
                len++;
                if(len>10)
                {
                    ddrst_msg("%d> exit\n");
                    return (1);
                }
            }
        }
        ddrst_msg("Test loop %d/%d\n", k, test_loop);
    }

    ddrst_msg("Test mem 3\n");
    for(k = 0; k < test_loop; k++)
    {
        for(i = 0; i < MEM_SIZE / 4; i++)
        {
            outw(MEM_BASE + 4 * i, 0);
        }
        len=0;
        for(i = 0; i < MEM_SIZE / 4; i++)
        {
            j = inw(MEM_BASE + 4 * i);
            if ( j != (0))
            {
                test3_err_cnt++;
                ddrst_msg("Test mem 3 failed ev=%08x, av=%08x\n", 0, j);
                len++;
                if (len > 10)
                {
                    ddrst_msg("%d> exit\n");
                    return (1);
                }
            }
        }
        ddrst_msg("Test loop %d/%d\n", k, test_loop);
    }
    ddrst_msg("End of test\n");
    ddrst_msg("Test result mem1_test %d/%d, mem2_test=%d/%d, mem3_test=%d/%d\n",
            test_loop - test1_err_cnt, test_loop,
            test_loop - test2_err_cnt, test_loop,
            test_loop - test3_err_cnt, test_loop);
    return 0;
}

uint32_t roundup(uint32_t value, uint32_t mask)
{
    return (value + mask) & ~mask;
}

/*
 * Memory address test, walking ones
 */
uint32_t ddr_addr_tst1()
{
    int32_t i, j, k;
    volatile uint32_t *p, *pt, *end, *start;
    uint32_t mask, bank, p1;

    /* Test the global address bits */
    for (p1=0, j=0; j<2; j++)
    {
        /* Set pattern in our lowest multiple of 0x20000 */
        start = (uint32_t *)MEM_BASE;
        end = (uint32_t *)(MEM_BASE + MEM_SIZE -  4);
        p = (uint32_t *)roundup((uint32_t)start, 0x1ffff);

        *p = p1;
        p1 = ~p1;
        for (i=0; i<100; i++)
        {
            mask = 4;
            do{
                pt = (uint32_t *)((uint32_t)p | mask);
                if (pt == p)
                {
                    mask = mask << 1;
                    continue;
                }
                if (pt >= end)
                {
                    break;
                }
                *pt = p1;
                //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), ~p1);
                if ((*p) != ~p1)
                {
                    ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), ~p1);
                    return 1;
                }
                mask = mask << 1;
            } while(mask);
        }
    }

    /* Now check the address bits in each bank */
    /* If we have more than 8mb of memory then the bank size must be */
    /* bigger than 256k.  If so use 1mb for the bank size. */
    if ((uint32_t)end > (0x800000 >> 12))
    {
        bank = 0x100000;
    }
    else
    {
        bank = 0x40000;
    }
    for (p1=0, k=0; k<2; k++)
    {
        p = start;
        /* Force start address to be a multiple of 256k */
        p = (uint32_t *)roundup((uint32_t)p, bank - 1);

        /* Redundant checks for overflow */
        while (p < end && p >= start && p != 0)
        {
            *p = p1;
            p1 = ~p1;
            for (i=0; i<50; i++)
            {
                mask = 4;
                do{
                    pt = (uint32_t *)((uint32_t)p | mask);
                    if (pt == p)
                    {
                        mask = mask << 1;
                        continue;
                    }
                    if (pt >= end)
                    {
                        break;
                    }
                    *pt = p1;
                    //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), ~p1);
                    if ((*p) != ~p1)
                    {
                        ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), ~p1);
                        return 1;
                    }
                    mask = mask << 1;
                } while(mask);
            }
            if (p + bank > p)
            {
                p += bank;
            }
            else
            {
                p = end;
            }
            p1 = ~p1;
        }
        p1 = ~p1;
    }
    return 0;
}

/*
 * Memory address test, own address
 */
uint32_t ddr_addr_tst2()
{
    int32_t  done;
    uint32_t *p, *pe, *end, *start;

    /* Write each address with it's own address */
    start = (uint32_t *)(MEM_BASE  + 0x2000000);
    end = (uint32_t *)(MEM_BASE + 0x2000000 + MEM_SIZE -  4);
    pe = (uint32_t *)end;
    p = start;
    done = 0;
    do{
        for (; p <= pe; p++)
        {
            *p = (uint32_t)p;
        }
        if (p >= pe)
        {
            done++;
        }
    } while (!done);

    /* Each address should have its own address */
    pe = end;
    p = start;
    done = 0;
    do{
        for (; p <= pe; p++)
        {
            //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), (uint32_t)p);
            if((*p) != (uint32_t)p)
            {
                ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), (uint32_t)p);
                return 1;
            }
        }
        if (p >= pe)
        {
            done++;
        }
    } while (!done);
    return 0;
}

/*
 * Test all of memory using a "half moving inversions" algorithm using random
 * numbers and their complment as the data pattern. Since we are not able to
 * produce random numbers in reverse order testing is only done in the forward
 * direction.
 */
 extern kdrv_status_t kdrv_timer_perf_get_instant(uint32_t* TimerId, uint32_t* instant, uint32_t* time);
 extern kdrv_status_t kdrv_timer_perf_open(uint32_t* TimerId);
uint32_t ddr_movinvr()
{
    int32_t i, done;
    uint32_t *p;
    uint32_t *pe;
    uint32_t *start,*end;
    uint32_t num;
    uint32_t random_number;
    uint32_t perftime, time;
    /* Initialize memory with initial sequence of random numbers.  */
    start = (uint32_t *)MEM_BASE;
    end = (uint32_t *)(MEM_BASE + MEM_SIZE -  4);

    srand(kdrv_timer_perf_get_instant(&pftimerid, &perftime, &time));
    random_number = rand();

    pe = end;
    p = start;
    done = 0;
    do {
        for (; p <= pe; p++)
        {
            *p = random_number;
        }
        if (p >= end)
        {
            done++;
        }
    } while (!done);

    /* Do moving inversions test. Check for initial pattern and then
    * write the complement for each memory location.
    */
    for (i=0; i<2; i++) {
        pe = end;
        p = start;
        done = 0;
        do {
            for (; p <= pe; p++)
            {
                num = random_number;
                if (i)
                {
                    num = ~num;
                }
                //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), num);
                if ((*p) != num)
                {
                    ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), num);
                    return 1;
                }
                *p = ~num;
            }
            if (p >= end)
            {
                done++;
            }
        } while (!done);
    }
    return 0;
}

/*
 * Test all of memory using a "moving inversions" algorithm using the
 * pattern in p1 and it's complement in p2.
 */
uint32_t ddr_movinv1 (uint32_t iter, uint32_t p1, uint32_t p2)
{
    int32_t i, done;
    uint32_t *p, *pe, *start, *end;

    start = (uint32_t *)(MEM_BASE + 0x4000000);
    end = (uint32_t *)(MEM_BASE + 0x4000000 + MEM_SIZE -  4);

    /* Initialize memory with the initial pattern.  */
    pe = end;
    p = start;
    done = 0;
    do{
        for (; p <= pe; p++)
        {
            *p = p1;
        }
        if (p >= pe)
        {
            done++;
        }
    } while (!done);

    /* Do moving inversions test. Check for initial pattern and then
    * write the complement for each memory location. Test from bottom
    * up and then from the top down.  */
    for (i=0; i<iter; i++)
    {
        pe = end;
        p = start;
        done = 0;
        do{
            for (; p <= pe; p++)
            {
                //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), p1);
                if ((*p) != p1)
                {
                    ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), p1);
                    return 1;
                }
                *p = p2;
            }
            if (p >= end)
            {
                done++;
            }
        } while (!done);

        pe = start;
        p = end;
        done = 0;
        do
        {
            do
            {
                //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), p2);
                if ((*p) != p2)
                {
                    ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), p2);
                    return 1;
                }
                *p = p1;
            } while (--p >= pe);

            if (p <= end)
            {
                done++;
            }
        } while (!done);
    }
    return 0;
}

volatile uint32_t *aaa;
volatile uint32_t bbb;
volatile uint32_t temp3;
uint32_t ddr_movinv32(int iter, uint32_t p1, uint32_t lb, uint32_t hb, int sval, int off)
{
    int k=0, n=0, done;
    uint32_t *p, *pe, *start, *end, pat = 0, p3;
    p3 = sval << 31;

    /* Initialize memory with the initial pattern.  */

    start = (uint32_t *)0x80000000;//MEM_BASE;//
    end = (uint32_t *)(0x80000000 + MEM_SIZE -  4);//(MEM_BASE + MEM_SIZE -  4);//(
    //ddrst_msg("Fill in 1 polling Start\n");
    pe = end;
    p = start;
    done = 0;
    k = off;
    pat = p1;
    do{
        while (p <= pe)
        {
            *p = pat;

            //if (*p != pat)
            //{
            //    __NOP;
            //}
            if (++k >= 32)
            {
                pat = lb;
                k = 0;
            }
            else
            {
                pat = pat << 1;
                pat |= sval;
            }
            p++;
        }
        if (p > end)
        {
            done++;
        }
    } while (!done);
    //__NOP;
    pe = end;
    /* Do moving inversions test. Check for initial pattern and then
    * write the complement for each memory location. Test from bottom
    * up and then from the top down.  */
    p = start;
    pe = end;
    done = 0;
    k = off;
    pat = p1;
    do
    {
        while (1)
        {
            //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), pat);
            if (*p != pat)
            {
                return 1;
            }
            *p = ~pat;

            if (++k >= 32)
            {
                pat = lb;
                k = 0;
            }
            else
            {
                pat = pat << 1;
                pat |= sval;
            }
            if (p >= pe)
                break;
            p++;
        }
        if (p >= end)
        {
            done++;
        }
    } while (!done);

    if (--k < 0)
    {
        k = 31;
    }
    for (pat = lb, n = 0; n < k; n++)
    {
        pat = pat << 1;
        pat |= sval;
    }

    k++;
    p = end;
    pe = start;
    done = 0;
    do{
        while(1)
        {
            //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), ~pat);
            if ((*p) != ~pat)
            {
                return 1;
            }
            *p = pat;
            if (--k <= 0)
            {
                pat = hb;
                k = 32;
            }
            else
            {
                pat = pat >> 1;
                pat |= p3;
            }
            if (p <= pe)
                break;
            p--;
        };
        if (p <= end)
        {
            done++;
        }
    } while (!done);
    return 0;
}

/*
 * Test all of memory using modulo X access pattern.
 */
uint32_t ddr_modtst(uint32_t offset, uint32_t iter, uint32_t p1, uint32_t p2)
{
    uint32_t k, l, done;
    uint32_t *p;
    uint32_t *pe;
    uint32_t *start, *end;

    /* Write every nth location with pattern */
    start = (uint32_t *)MEM_BASE;
    end = (uint32_t *)(MEM_BASE + MEM_SIZE -  4);
    end -= MOD_SZ;  /* adjust the ending address */
    pe = end;
    p = start+offset;
    done = 0;
    do{
        /* Check for overflow */
        for (; p <= pe; p += MOD_SZ)
        {
            *p = p1;
        }
        if(p>=pe)
        {
            done++;
        }
    } while (!done);

    /* Write the rest of memory "iter" times with the pattern complement */
    for (l=0; l<iter; l++)
    {
        pe = end;
        p = start;
        done = 0;
        k = 0;
        do{
            for (; p <= pe; p++)
            {
                if (k != offset)
                {
                    *p = p2;
                }
                if (++k > MOD_SZ-1)
                {
                    k = 0;
                }
            }

            if (p >= pe)
            {
                done++;
            }
        } while (!done);
    }

    /* Now check every nth location */
    pe = end;
    p = start+offset;
    done = 0;
    end -= MOD_SZ;/* adjust the ending address */
    do{
        for (; p <= pe; p += MOD_SZ)
        {
            //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), p1);
            if ((*p) != p1)
            {
                ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), p1);
                return 1;
            }
        }
        if (p >= pe)
        {
            done++;
        }
    } while (!done);
    return 0;
}


/*
 * Test memory for bit fade, fill memory with pattern.
 */
void ddr_bit_fade_fill(uint32_t p1)
{
    int done;
    uint32_t *p, *pe;
    uint32_t *start,*end;

    /* Initialize memory with the initial pattern.  */
    {
        start = (uint32_t *)MEM_BASE;
        end = (uint32_t *)(MEM_BASE + MEM_SIZE -  4);

        pe = end;
        p = start;
        done = 0;
        do
        {
            for (; p < pe;)
            {
                *p = p1;
                p++;
            }
            if (pe >= end)
            {
                pe = end;
                done++;
            }
        } while (!done);
    }
}

uint32_t ddr_bit_fade_chk(uint32_t p1)
{
    uint32_t done;
    uint32_t *p, *pe;
    uint32_t *start,*end;

    /* Make sure that nothing changed while sleeping */
    start = (uint32_t *)MEM_BASE;
    end = (uint32_t *)(MEM_BASE + MEM_SIZE -  4);
    pe = end;
    p = start;
    done = 0;
    do{
        for (; p < pe;)
        {
            //ddrst_msg1("== <0x%x> <0x%x>\n", (*p), p1);
            if ((*p) != p1)
            {
                ddrst_msg("Fail test case adrr (0x%x) <0x%x> <0x%x>\n", p, (*p), p1);
                return 1;
            }
            p++;
        }
        if (pe >= end)
        {
            pe = end;
            done++;
        }
    } while (!done);
    return 0;
}

/* Sleep for ms */
void ddr_sleep(uint32_t ms)
{
    //kdrv_clock_disable(CLK_NPU);
    //kdrv_clock_disable(CLK_NCPU);
    //kdrv_ddr_self_refresh_enter();
    //kdrv_pwmtimer_delay_ms(ms);
    //kdrv_ddr_self_refresh_exit();
    //kdrv_clock_enable(CLK_NCPU);
    //kdrv_clock_enable(CLK_NPU);
}



uint32_t ddr_selftest_execute()
{
    uint32_t ret;
    uint32_t i, iter, p1, p2, p0;
    uint32_t j;
    //uint32_t rc = 1;
    //int32_t cmd_size = ARRAY_SIZE(ddr_cmd_array);
    char buf[256];
    iter = ITER_CNT;
    uint32_t random_number;
    uint32_t perftime,time;
    kdrv_timer_perf_open(&pftimerid);
    kdrv_timer_perf_reset(&pftimerid);
    kdrv_timer_perf_set(&pftimerid);
    while(1)
    {
        static int32_t id = 1;//MEMTEST86_ID_5;
        ddrst_msg("========================================================================== \n");
        sprintf(buf, "Do memtest case %s", ddr_cmd_array[id-1].desc);
        ddrst_msg("%s\n", buf);
        ddrst_msg("========================================================================== \n");
        switch(id)
        {
            case MEMTEST86_ID_1:
                /* Address test, walking ones (test #0) */
                ret = ddr_addr_tst1();
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d> \n", id);
                    return (uint32_t)MEMTEST86_ID_1;
                }
                break;
            case MEMTEST86_ID_2:
                /* Address test, own address (test #1, 2) */
                ret = ddr_addr_tst2();
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d> \n", id);
                    return (uint32_t)MEMTEST86_ID_2;
                }
                break;
            case MEMTEST86_ID_3:
                /* Moving inversions, all ones and zeros (tests #3, 4) */
                p1 = 0;
                p2 = ~p1;
                ret = ddr_movinv1(iter, p1, p2);
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d_1> \n", id);
                    return (uint32_t)MEMTEST86_ID_3;
                }
                ret = ddr_movinv1(iter, p2, p1);
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d_2> \n", id);
                    return (uint32_t)MEMTEST86_ID_3;
                }
                break;
            case MEMTEST86_ID_4:
                /* Moving inversions, 8 bit walking ones and zeros (test #5) */
                p0 = 0x80;
                for (i=0; i<8; i++, p0=p0>>1)
                {
                    p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
                    p2 = ~p1;
                    ret = ddr_movinv1(iter,p1,p2);

                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }

                    /* Switch patterns */
                    ret = ddr_movinv1(iter,p2,p1);
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }
                }
                break;
            case MEMTEST86_ID_5:
                /* Moving inversions, 32 bit shifting pattern (test #8) */
                for (i=0, p1=1; p1; p1=p1<<1, i++)
                {
                    ret = ddr_movinv32(iter,p1, 1, 0x80000000, 0, i);
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }
                    ret = ddr_movinv32(iter,~p1, 0xfffffffe,0x7fffffff, 1, i);
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }
                }
                break;
            case MEMTEST86_ID_6:
                /* Random Data (test #6) */
                for (i=0; i < iter; i++)
                {
                    srand(kdrv_timer_perf_get_instant(&pftimerid, &perftime, &time));
                    random_number = rand();
                    p1 = random_number;
                    p2 = ~p1;
                    ddr_movinv1(2,p1,p2);
                }
                break;
            case MEMTEST86_ID_7:
                /* Random Data Sequence (test #9) */
                for (i=0; i < iter; i++) {
                    ret = ddr_movinvr();
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }
                }
                break;
            case MEMTEST86_ID_8:
                /* Bit fade test, fill (test #11) */
                p1 = 0;
                ddr_bit_fade_fill(p1);
                ddr_sleep(1000);
                ret = ddr_bit_fade_chk(p1);
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d> \n", id);
                    return ret;
                }
                p1 = ~p1;
                ddr_bit_fade_fill(p1);
                ddr_sleep(1000);
                ret = ddr_bit_fade_chk(p1);
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d> \n", id);
                    return ret;
                }
                break;
            case MEMTEST86_ID_9:
                /* Modulo 20 check, Random pattern (test #10) */
                for (j=0; j<iter; j++)
                {
                    p1 = random_number;
                    for (i=0; i<MOD_SZ; i++) {
                        p2 = ~p1;
                        ret = ddr_modtst(i, 2, p1, p2);
                        if (ret != 0)
                        {
                            ddrst_msg("Fail test case <%d> \n", id);
                            return ret;
                        }

                        /* Switch patterns */
                        ret = ddr_modtst(i, 2, p2, p1);
                        if (ret != 0)
                        {
                            ddrst_msg("Fail test case <%d> \n", id);
                            return ret;
                        }
                    }
                }
                break;
            case MEMTEST86_ID_10:
                /* Modulo 20 check, all ones and zeros*/
                p1 = 0;
                for (i=0; i<MOD_SZ; i++) {
                    p2 = ~p1;
                    ret = ddr_modtst(i, 2, p1, p2);
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }

                    /* Switch patterns */
                    ret = ddr_modtst(i, 2, p2, p1);
                    if (ret != 0)
                    {
                        ddrst_msg("Fail test case <%d> \n", id);
                        return ret;
                    }
                }
                break;
            case MEMTEST86_ID_11:
                /* Modulo 20 check, 8 bit pattern*/
                p0 = 0x80;
                for (j=0; j<8; j++, p0=p0>>1) {
                    p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
                    for (i=0; i<MOD_SZ; i++) {
                        p2 = ~p1;
                        ret = ddr_modtst(i, 2, p1, p2);
                        if (ret != 0)
                        {
                            ddrst_msg("Fail test case <%d> \n", id);
                            return ret;
                        }

                        /* Switch patterns */
                        ret = ddr_modtst(i, 2, p2, p1);
                        if (ret != 0)
                        {
                            ddrst_msg("Fail test case <%d> \n", id);
                            return ret;
                        }
                    }
                }
                break;
            case FARADAY_MEMTEST:
                ret = ddr_sram_rw_verify();
                if (ret != 0)
                {
                    ddrst_msg("Fail test case <%d> \n", id);
                    return ret;
                }
                break;
            default:
                break;
        }
        if(id > 0 && id < MEMTEST_ID_MAX)
            ddrst_msg("Pass test case <%d> \n", id);
        
        id += 1;
        if(id >= MEMTEST_ID_MAX)
            return 0;
    }
}
#include "kdrv_pwm.h"
void ddr_selftest(void)
{
    uint32_t ret;
    kdrv_pwm_config(PWM_ID_1, PWM_POLARITY_NORMAL, 25, 50, 0);
    kdrv_pwm_enable(PWM_ID_1);
    ret = ddr_selftest_execute();
    if(ret != 0)
    {
        ddrst_msg("ddr self test FAIL <%d> \n", ret);
    }
    else
    {
        ddrst_msg("ddr selt test Pass\n");
    }
}
