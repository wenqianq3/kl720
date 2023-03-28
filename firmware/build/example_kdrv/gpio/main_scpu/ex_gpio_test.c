
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cmsis_os2.h"
#include "kdrv_gpio.h"
#include "kdrv_pinmux.h"
#include "project.h"
#include "kdrv_clock.h"
#define GPIO_DEBUG
#ifdef GPIO_DEBUG
#include "kmdw_console.h"
#define gpio_msg(fmt, ...) kmdw_printf("[%s] \n" fmt, __func__, ##__VA_ARGS__)
#else
#define gpio_msg(fmt, ...)
#endif

#define TESTINTERVAL            500
#define GPIO_ST_LOOPMAX         10
#define GPIO_ST_STATUS_OK       0x0000
#define GPIO_ST_STATUS_ER_1     0x0001
#define GPIO_ST_STATUS_ER_2     0x0002
#define GPIO_ST_STATUS_ER_3     0x0004
#define GPIO_ST_STATUS_ER_4     0x0008
#define GPIO_ST_STATUS_ER_5     0x0010
#define GPIO_ST_STATUS_ER_6     0x0020
#define GPIO_ST_STATUS_ER_7     0x0040
#define GPIO_ST_STATUS_ER_8     0x0080
#define GPIO_ST_STATUS_ER_9     0x0100
#define GPIO_ST_STATUS_ER_10    0x0200
#define GPIO_ST_STATUS_ER_11    0x0400
#define GPIO_ST_STATUS_ER_12    0x0800
#define GPIO_ST_STATUS_ER_13    0x1000

static void gpio_callback(kdrv_gpio_pin_t pin, void *arg);
extern gpio_attr_context gpio_attr_ctx[GPIO_NUM];
void gpio_selftest_thread(void *argument);
uint32_t gpio_self_test_cnt;
uint32_t startleveltrigger = 0;
void gpio_selftest_init(void)
{
    kdrv_pinmux_config(KDRV_PIN_X_DPI_DATAO8,   PIN_MODE_7, PIN_PULL_NONE,  PIN_DRIVING_8MA);    // as GPIO 28
    kdrv_pinmux_config(KDRV_PIN_X_DPI_DATAO9,   PIN_MODE_7, PIN_PULL_NONE,  PIN_DRIVING_8MA);    // as GPIO 29
    kdrv_pinmux_config(KDRV_PIN_X_DPI_DATAO10,  PIN_MODE_7, PIN_PULL_UP,    PIN_DRIVING_8MA);    // as GPIO 30
    kdrv_pinmux_config(KDRV_PIN_X_DPI_DATAO11,  PIN_MODE_7, PIN_PULL_UP,    PIN_DRIVING_8MA);    // as GPIO 31

    gpio_msg("setting GPIO_ST_OUT1 : Output Low");
    gpio_msg("setting GPIO_ST_OUT2 : Output Low");

    // set GPIO_ST_OUT1/GPIO_ST_OUT2 as output low in the beginning
    kdrv_gpio_set_attribute(GPIO_ST_OUT1, GPIO_DIR_OUTPUT);
    kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
    kdrv_gpio_set_attribute(GPIO_ST_OUT2, GPIO_DIR_OUTPUT);
    kdrv_gpio_write_pin(GPIO_ST_OUT2, false);

    gpio_msg("setting GPIO_ST_IN1 : Interrupt source");

    /* set GPIO_ST_IN1 as interrupt input */
    {
        // first disable interrupt in case of wrong condition
        kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);

        // set pin attributes as desired
        kdrv_gpio_set_attribute(GPIO_ST_IN1, (GPIO_DIR_INPUT | GPIO_INT_EDGE_FALLING));

        // enable internal hardware debounce with clock rate
        kdrv_gpio_set_debounce(GPIO_ST_IN1, true /* 1 for enable */, 1000);

        // at last enable interrupt after all settings done
        kdrv_gpio_set_interrupt(GPIO_ST_IN1, true);
    }

    gpio_msg("setting GPIO_ST_IN2 : Interrupt source");

    /* set GPIO_ST_IN2 as interrupt input, pull-high, debounce */
    {
        // first disable interrupt in case of wrong condition
        kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);

        // set pin attributes as desired
        kdrv_gpio_set_attribute(GPIO_ST_IN2, (GPIO_DIR_INPUT | GPIO_INT_EDGE_BOTH));

        // enable internal hardware debounce with clock rate
        kdrv_gpio_set_debounce(GPIO_ST_IN2, true /* 1 for enable */, 1000);

        // at last enable interrupt after all settings done
        kdrv_gpio_set_interrupt(GPIO_ST_IN2, true);
    }

    // set interrupt callback for GPIO interrupt
    kdrv_gpio_register_callback(gpio_callback, NULL);

    /* exit here then let kernel get started */
}

static void gpio_callback(kdrv_gpio_pin_t pin, void *arg)
{
    bool value;
    if (pin == GPIO_ST_IN1)
    {
        gpio_self_test_cnt++;
        kdrv_gpio_read_pin(GPIO_ST_IN1, &value);
        if(startleveltrigger)
            kdrv_gpio_write_pin(GPIO_ST_OUT1, !value);
    }
    else if (pin == GPIO_ST_IN2)
    {
        gpio_self_test_cnt++;
        kdrv_gpio_read_pin(GPIO_ST_IN2, &value);
        if(startleveltrigger)
            kdrv_gpio_write_pin(GPIO_ST_OUT2, !value);
    }
}

uint32_t gpio_selftest_execute()
{
    uint32_t status;
    uint32_t loopcnt = 0;

    gpio_selftest_init();
    gpio_msg("========================================");
    gpio_msg("Start check generic IO test");
    gpio_msg("========================================");
    loopcnt = 0;
    status = GPIO_ST_STATUS_OK;
    startleveltrigger = 0;
    kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
    kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
    kdrv_gpio_set_attribute(GPIO_ST_IN1, (GPIO_DIR_INPUT));
    for (; loopcnt++ < GPIO_ST_LOOPMAX ;)
    {
        kdrv_delay_us(5000);
        {
            bool value;
            gpio_msg("Selft test : gneric IO loopcnt %d",loopcnt);
            kdrv_gpio_write_pin(GPIO_ST_OUT1, true);
            kdrv_delay_us(1000);
            kdrv_gpio_read_pin(GPIO_ST_IN1, &value);
            if(value == 0)
            {
                status |= GPIO_ST_STATUS_ER_1;
            }
            kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
            kdrv_delay_us(1000);
            kdrv_gpio_read_pin(GPIO_ST_IN1, &value);
            if(value == 1)
            {
                status |= GPIO_ST_STATUS_ER_2;
            }
        }
    }
    if((status & (GPIO_ST_STATUS_ER_1|GPIO_ST_STATUS_ER_2)) != GPIO_ST_STATUS_OK)
    {
        gpio_msg("GPIO IP self test: generic IO function validation FAIL, Error ID 0x%x ", status);
    }
    else
    {
        gpio_msg("GPIO IP self test: generic IO function validation PASS");
    }


    gpio_msg("========================================");
    gpio_msg("Start check edge interrupt");
    gpio_msg("========================================");
    loopcnt = 0;
    startleveltrigger = 0;
    for (; loopcnt++ < GPIO_ST_LOOPMAX ;)
    {
        kdrv_delay_us(5000);
        {
            gpio_msg("Selft test : edge interrupt loopcnt %d",loopcnt);
            kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
            kdrv_gpio_write_pin(GPIO_ST_OUT2, false);
            kdrv_gpio_set_attribute(GPIO_ST_IN1, (GPIO_DIR_INPUT | GPIO_INT_EDGE_FALLING));
            kdrv_gpio_set_attribute(GPIO_ST_IN2, (GPIO_DIR_INPUT | GPIO_INT_EDGE_BOTH));
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, true);
            kdrv_gpio_set_interrupt(GPIO_ST_IN2, true);
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT1, true);
            kdrv_delay_us(1000);
            if(gpio_self_test_cnt != 0)
            {
                status |= GPIO_ST_STATUS_ER_3;
            }
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
            kdrv_delay_us(5000);
            if(gpio_self_test_cnt == 0)
            {
                status |= GPIO_ST_STATUS_ER_4;
            }

            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT2, true);
            kdrv_delay_us(5000);
            if(gpio_self_test_cnt == 0)
            {
                status |= GPIO_ST_STATUS_ER_5;
            }

            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT2, false);
            kdrv_delay_us(5000);
            if(gpio_self_test_cnt == 0)
            {
                status |= GPIO_ST_STATUS_ER_6;
            }
        }
    }
    if((status & (GPIO_ST_STATUS_ER_3|GPIO_ST_STATUS_ER_4|GPIO_ST_STATUS_ER_5|GPIO_ST_STATUS_ER_6)) != GPIO_ST_STATUS_OK)
    {
        gpio_msg("GPIO IP self test: IC edge interrupt function validation FAIL, Error ID 0x%x ", status);
    }
    else
    {
        gpio_msg("GPIO IP self test: IC edge interrupt function validation PASS");
    }

    gpio_msg("========================================");
    gpio_msg("Start check level interrupt");
    gpio_msg("========================================");
    loopcnt = 0;
    startleveltrigger = 1;
    kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
    kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
    kdrv_gpio_set_attribute(GPIO_ST_IN1, (GPIO_DIR_INPUT | GPIO_INT_LEVEL_HIGH));

    kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);
    kdrv_gpio_write_pin(GPIO_ST_OUT2, true);
    kdrv_gpio_set_attribute(GPIO_ST_IN2, (GPIO_DIR_INPUT | GPIO_INT_LEVEL_LOW));
    for (; loopcnt++ < GPIO_ST_LOOPMAX ;)
    {
        kdrv_delay_us(5000);
        {
            gpio_msg("Selft test : level interrupt loopcnt %d",loopcnt);
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT1, true);
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, true);
            kdrv_delay_us(1000);
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
            if(gpio_self_test_cnt == 0)
            {
                status |= GPIO_ST_STATUS_ER_7;
            }
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT1, false);
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, true);
            kdrv_delay_us(1000);
            kdrv_gpio_set_interrupt(GPIO_ST_IN1, false);
            if(gpio_self_test_cnt != 0)
            {
                status |= GPIO_ST_STATUS_ER_8;
            }

            kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT2, false);
            kdrv_gpio_set_interrupt(GPIO_ST_IN2, true);
            kdrv_delay_us(1000);
            kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);
            if(gpio_self_test_cnt == 0)
            {
                status |= GPIO_ST_STATUS_ER_9;
            }

            kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);
            gpio_self_test_cnt = 0;
            kdrv_gpio_write_pin(GPIO_ST_OUT2, true);
            kdrv_gpio_set_interrupt(GPIO_ST_IN2, true);
            kdrv_delay_us(1000);
            kdrv_gpio_set_interrupt(GPIO_ST_IN2, false);
            if(gpio_self_test_cnt != 0)
            {
                status |= GPIO_ST_STATUS_ER_10;
            }
        }
    }
    if((status & (GPIO_ST_STATUS_ER_7|GPIO_ST_STATUS_ER_8|GPIO_ST_STATUS_ER_9|GPIO_ST_STATUS_ER_10)) != GPIO_ST_STATUS_OK)
    {
        gpio_msg("GPIO IP self test: IC level interrupt function validation FAIL, Error ID 0x%x ", status);
    }
    else
    {
        gpio_msg("GPIO IP self test: IC level interrupt function validation PASS");
    }

    gpio_msg("========================================");
    gpio_msg("Start IC internal circuit IO test");
    gpio_msg("========================================");
    for(uint32_t i=0; i<GPIO_NUM; i++)
    {
        bool value;
        kdrv_gpio_write_pin((kdrv_gpio_pin_t)gpio_attr_ctx[i].gpio_pin, true);
        kdrv_delay_us(5);
        kdrv_gpio_read_pin((kdrv_gpio_pin_t)gpio_attr_ctx[i].gpio_pin, &value);
        if(value != 1)
        {
            gpio_msg("pin %d selft test fail pin value as %d, expect as 1", i, value);
            status |= GPIO_ST_STATUS_ER_11;
        }
        else
        {
            gpio_msg("pin %d selft test PASS pin value as %d, expect as 1", i, value);
        }

    }
    if((status &  GPIO_ST_STATUS_ER_11)!= GPIO_ST_STATUS_OK)
    {
        gpio_msg("GPIO IP self test: IC internal signal validation FAIL, Error ID 0x%x ", status);
    }
    else
    {
        gpio_msg("GPIO IP self test: IC internal signal validation PASS");
    }


    return status;
}

void gpio_selftest_thread(void *argument)
{
    uint32_t status;
    status = gpio_selftest_execute();
    gpio_msg("\n\n========================================");
    if(status != GPIO_ST_STATUS_OK)
    {
        gpio_msg("GPIO IP self test: FAIL, Error ID 0x%x ", status);
    }
    else
    {
        gpio_msg("GPIO IP self test: PASS");
    }
    gpio_msg("========================================");
}

void gpio_selftest(void)
{
    gpio_selftest_execute();
}

