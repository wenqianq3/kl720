/* Copyright (c) 2021 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  main.c
*
*  Description:
*  ------------
*
*
******************************************************************************/


/******************************************************************************
Head Block of The File
******************************************************************************/
#include "cmsis_os2.h"

#include "system_init.h"
#include "driver_init.h"
#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kmdw_system.h"
#include "kmdw_tdc.h"
#include "kmdw_errandserv.h"
#include "kmdw_model.h"
#include "scpu_log.h"
#include "version.h"

#include "kcomm.h"
#include "kapp_ops.h"
#include "kapp_center_app_ops.h"

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/

/******************************************************************************
Declaration of data structure
******************************************************************************/

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
void console_entry(void);

static int sdk_main(void)
{
    sys_initialize();
    drv_initialize();
    osKernelInitialize();

    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END); // FIXME
    kmdw_console_queue_init();                // uart console
    kmdw_console_set_log_level_scpu(LOG_DBG);

    kmdw_printf(">> Start running KL720 companion mode ...\n");

    load_ncpu_fw(1);
    kmdw_model_init();
    kmdw_errandserv_start(); // a service thread for running errand tasks (bottom-half)
#ifdef TDC_ENABLE
    kmdw_tdc_start();
#endif

    kmdw_printf("SDK v%u.%u.%u-build.%03u\n", (uint8_t)(IMG_FW_VERSION>>24), (uint8_t)(IMG_FW_VERSION>>16), (uint8_t)(IMG_FW_VERSION>>8), (uint8_t)(IMG_FW_VERSION));

    return 0;
}

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
int main(void)
{
    /* SDK main init */
    sdk_main();


    /* Console entry */
    console_entry();

    /* Start RTOS Kernel */
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    while(1);
}
