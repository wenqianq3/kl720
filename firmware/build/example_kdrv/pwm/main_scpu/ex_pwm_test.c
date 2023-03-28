#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kmdw_console.h"
#include "kdrv_clock.h"
#include "kdrv_pwm.h"

//#define PWM_DIRECT_OUT
#define PWM_DEBUG
#ifdef PWM_DEBUG
#include "kmdw_console.h"
#define pwm_msg(fmt, ...) err_msg("[%s] \n" fmt, __func__, ##__VA_ARGS__)
#else
#define pwm_msg(fmt, ...)
#endif
void sample_pwm_test(pwm_id pwm_channel)
{
    char buf[256];
    unsigned int period, duty, polarity;

    pwm_msg("[PWM%d Test]",pwm_channel);
    pwm_msg("Please input the period(20ns)");
    kmdw_console_echo_gets(buf, sizeof(buf));
    period = atoi(buf);
    pwm_msg("Please input duty(20ns)");
    kmdw_console_echo_gets(buf, sizeof(buf));
    duty = atoi(buf);
    pwm_msg("Please input polarity(0:low, 1:high)");
    kmdw_console_echo_gets(buf, sizeof(buf));
    polarity = atoi(buf);
    pwm_msg("period = %d, duty = %d, polarity = %d", period, duty, polarity);

    kdrv_pwm_config((pwm_id)pwm_channel, (pwmpolarity)polarity, duty, period, 0);
    kdrv_pwm_enable((pwm_id)pwm_channel);
    pwm_msg("setting done!");

}

void sample_pwm_adjust_duty_test(pwm_id pwm_channel, uint16_t duty)
{
    //char buf[256];
    unsigned int period, polarity;

    pwm_msg("[PWM%d Test]",pwm_channel);
    period = 1000;
    polarity = 0;
    pwm_msg("period = %d, duty = %d, polarity = %d", period, duty, polarity);

    kdrv_pwm_config((pwm_id)pwm_channel, (pwmpolarity)polarity, duty, period, 0);
    kdrv_pwm_enable((pwm_id)pwm_channel);
    pwm_msg("setting done!");

}

void pwm_selftest(void)
{
#ifdef PWM_DIRECT_OUT
    kdrv_pwm_config(PWM_ID_1, PWM_POLARITY_NORMAL, 25, 50, 0);
    kdrv_pwm_enable(PWM_ID_1);
#else
    char buf[256];
    uint16_t duty=0;
    //TEST 1
    pwm_msg("************************************");
    sample_pwm_test(PWM_ID_1);
    pwm_msg("Enter to go next step!");
    kmdw_console_echo_gets(buf, sizeof(buf));
    kdrv_pwm_disable(PWM_ID_1);
    pwm_msg("Enter to go next step!");
    kmdw_console_echo_gets(buf, sizeof(buf));

    pwm_msg("************************************");
    sample_pwm_test(PWM_ID_2);
    pwm_msg("Enter to go next step!");
    kmdw_console_echo_gets(buf, sizeof(buf));
    kdrv_pwm_disable(PWM_ID_2);
    pwm_msg("Enter to go next step!");
    kmdw_console_echo_gets(buf, sizeof(buf));
    
    //TEST 2
    pwm_msg("************************************");
    pwm_msg("PWM duty increasing test!");
    for(duty=50; duty <= 1000; duty+=50)
    {
        sample_pwm_adjust_duty_test(PWM_ID_1, duty);
        kdrv_delay_us(300000);
    }

    //TEST 3
    pwm_msg("************************************");
    pwm_msg("PWM duty decreasing test!");
    for(duty=1000; duty >= 50; duty-=50)
    {
        sample_pwm_adjust_duty_test(PWM_ID_1, duty);
        kdrv_delay_us(300000);
    }
#endif
}



