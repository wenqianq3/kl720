/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/
 //Include
#include "project.h"
#include "kdrv_gpio.h"
#include "kdrv_pinmux.h"
#include "kdrv_uart.h"

static uint32_t pinmux_array[PIN_NUM] = PINMUX_ARRAY;
gpio_attr_context gpio_attr_ctx[GPIO_NUM] = GPIO_ATTR_ARRAY;
 //Function
void drv_initialize(void)
{    
    kdrv_gpio_initialize(GPIO_NUM, gpio_attr_ctx);
    kdrv_pinmux_initialize(PIN_NUM, pinmux_array);
    kdrv_uart_initialize();
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);                 // uart console 
}

