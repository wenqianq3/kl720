/*
 * Kneron HICO (Host IN Companion OUT) Console Application
 *
 * Copyright (C) 2021 Kneron, Inc. All rights reserved.
 *
 */

#include <cmsis_os2.h>
#include <string.h>
#include <stdlib.h>
#include "base.h"
#include "kmdw_console.h"
#include "kmdw_power_manager.h"
#include "kapp_hico.h"
#include "model_type.h"

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

typedef void  (*console_cmd_func)(void);

struct console_cmd_info
{
    char *desc;
    console_cmd_func func;
};

static osThreadId_t tid_console;
static int started;
struct kapp_hico_cfg_s hico_cfg_s = {0};
static uint32_t model_id = KNERON_YOLOV5S_640_640_3;

static void kapp_start_hico(void)
{
    int rc;

    if (!started) {
        hico_cfg_s.app_id = 0;
        hico_cfg_s.model_id = model_id;

        rc = 0;//kapp_hico_start(&hico_cfg_s);
        if (!rc)
            started = 1;
    }
}

static void kapp_stop_hico(void)
{
    if (started) {
        //kapp_hico_stop();
        started = !started;
    }
}

static void kapp_quit(void)
{
    DSG("Bye bye !!");
    kmdw_power_manager_shutdown();
}

struct console_cmd_info app_idle_mode_array[] = {
    {"Start HICO",              kapp_start_hico},
    {"Stop HICO",               kapp_stop_hico},
    {"Quit",                    kapp_quit},
};

static void power_btn_handler(int released)
{
    if (released) {
        if (started)
            kapp_stop_hico();
        else
            kapp_start_hico();
    }
}

void console_thread(void *arg)
{
    int id = 0;
    int i = 0;
    uint32_t idle_cmd_size = ARRAY_SIZE(app_idle_mode_array);
    char buf[64];
    console_cmd_func cmd_func;

    //kapp_hico_init();

    //power_button_register(power_btn_handler);
    
    while(1)
    {
        if (id)
            goto cmd_prompt;

        DSG("\n === Menu === ");
        for (i = 0; i < idle_cmd_size; ++i)
        { 
            sprintf(buf, "(%2d) %s", i + 1, app_idle_mode_array[i].desc);
            DSG("%s", buf);
        }

cmd_prompt:
        DSG_NOLF(" command >> ");
        kmdw_console_echo_gets(buf, sizeof(buf));
        id = atoi(strtok(buf, " \r\n\t"));
        if (id > 0 && id <= idle_cmd_size) {
            cmd_func = app_idle_mode_array[id - 1].func;
            if (cmd_func) {
                cmd_func();    
            }
        } else {
            if (id) {
                model_id = id;
                err_msg("model_id is changed to %d\n", id);
            }
            continue;
        }
    }
}

void console_entry(void)
{
    tid_console = osThreadNew(console_thread, NULL, NULL);
    osThreadSetPriority(tid_console, osPriorityBelowNormal);
}

