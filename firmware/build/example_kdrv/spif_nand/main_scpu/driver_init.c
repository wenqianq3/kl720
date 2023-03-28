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
#include "kdrv_uart.h"
#include "kdrv_ddr.h"
#include "kdrv_pinmux.h"
#include "kdev_flash_nand.h"

static uint32_t pinmux_array[PIN_NUM] = PINMUX_ARRAY;
 //Function 
void drv_initialize(void)
{
    kdrv_uart_initialize();
#if !defined(_BOARD_SN720HAPS_H_)
    kdrv_pinmux_initialize(PIN_NUM, pinmux_array);
    kdrv_ddr_Initialize(AXI_DDR_MHZ);
    kdev_flash_initialize();
#endif
}

