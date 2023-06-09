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

/**@addtogroup  TASK_HANDLER
 * @{
 * @brief       Kneron System init
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef _TASK_HANDLER_H
#define _TASK_HANDLER_H
#include "cmsis_os2.h"
#include "project.h"
#define USB_HOST
/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list
typedef struct
{
    //parameters for creating tasks
    const char caName[8]; //now, len=8

    osThreadId_t        *tTaskHandle;
    osThreadFunc_t      fpEntry;
    const uint32_t      dwStackSize;
    osPriority_t        dwPriority;

    //parameters for creating queue
    osMessageQueueId_t  *tQueueHandle;
    const uint32_t      tQmsg_count;
    const uint32_t      tQmsg_size;
}T_S_KneronTask;

osThreadId_t task_tdc_handle;
osThreadId_t task_log_handle;
osThreadId_t task_errand_handle;
//osThreadId_t task_scpulog_handle;
osThreadId_t task_infdata_handle;
osThreadId_t task_infcb_handle;

osThreadId_t task_usb_cmd_handle;
osThreadId_t task_usb_image_handle;
osThreadId_t task_usb_result_handle;
osThreadId_t task_buf_mgr_handle;

osMessageQueueId_t qTdcMgrSched;
osMessageQueueId_t qLogMgrSched;
osMessageQueueId_t qErrandMgrSched;
//osMessageQueueId_t qScpulogMgrSched;

extern void kmdw_tdc_monitor_thread(void *argument);
extern void logger_thread(void *arg);
extern void errand_thread(void *arg);
//extern void scpu_log_buf_thread(void *argument);
extern void kmdw_inference_image_dispatcher_thread(void *argument);
extern void kmdw_inference_result_handler_callback_thread(void *argument);

extern void kdp2_hico_mipi_usb_cmd_thread(void *arg);
extern void kdp2_hico_mipi_usb_img_send_back_thread(void *arg);
extern void kdp2_hico_mipi_usb_result_thread(void *arg);
extern void kdp2_fifoq_manager_enqueue_image_thread(void *arg);

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable
T_S_KneronTask g_atKneronTaskPool[]=
{
// TaskName         TaskHandle                  TaskFuncEntry                                   TaskStack   TaskPriority            QueueHandle         Queue      Queue
//                                                                                                                                                      MsgCount   MsgSize
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  {"LogTask",       &task_log_handle,           logger_thread,                                  2048,       osPriorityHigh,         &qLogMgrSched,      16,        16   },
#ifndef _BOARD_SN720HAPS_H_
#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
  {"TdcTask",       &task_tdc_handle,           kmdw_tdc_monitor_thread,                        2048,       osPriorityNormal,       &qTdcMgrSched,      16,        16   },
#endif
#endif
  {"Errand",        &task_errand_handle,        errand_thread,                                  2048,       osPriorityBelowNormal,  &qErrandMgrSched,   10,        16   },
//{"Scpu_log",      &task_scpulog_handle,       scpu_log_buf_thread,                            1024,       osPriorityBelowNormal,  &qScpulogMgrSched,  16,        16   },
  {"Infdata",       &task_infdata_handle,       kmdw_inference_image_dispatcher_thread,         4096,       osPriorityHigh,         NULL,                0,        0    },
  {"Infcb",         &task_infcb_handle,         kmdw_inference_result_handler_callback_thread,  2048,       osPriorityHigh,         NULL,                0,        0    },

//
//Follow above format to add your TASK here
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  {"buf_mgr",       &task_buf_mgr_handle,       kdp2_fifoq_manager_enqueue_image_thread,        1024,       osPriorityNormal,       NULL,                0,        0    },

//Application specific threads
  {"usbcmd",        &task_usb_cmd_handle,       kdp2_hico_mipi_usb_cmd_thread,                  2048,       osPriorityNormal,       NULL,                0,        0    },
  {"usbimg",        &task_usb_image_handle,     kdp2_hico_mipi_usb_img_send_back_thread,        2048,       osPriorityNormal,       NULL,                0,        0    },
  {"usbrslt",       &task_usb_result_handle,    kdp2_hico_mipi_usb_result_thread,               2048,       osPriorityNormal,       NULL,                0,        0    },

//end of table, don't remove it
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  {NULL,NULL,NULL,0,0,NULL,0,0}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
};

#endif

