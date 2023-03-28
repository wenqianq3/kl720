#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"

#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kmdw_usbh2.h"
#include "kmdw_uvc2.h"

//#define PRINTF_FPS_ONLY

static osThreadId_t this_tid = 0;
static kmdw_usbh2_pipe_t isoch_pipe;

/*----------------------------------------------------------------------------
*      Thread for usb host examples
*---------------------------------------------------------------------------*/

#define TFLAG_UVC_CONFIGURE 0x20

#define NUM_FRAME 3
#define FRMAE_SIZE (640 * 480 * 2) // VGA YUV420

int total_test_result = 0;

static int collected_frame = 0;

static void uvc_frame_cb(uint32_t *frame_ptr, uint32_t frame_size)
{
    if (frame_size != FRMAE_SIZE)
    {
        kmdw_printf("[USBH2 test_2] error frame detected: %d frame_ptr 0x%p frame_size %u\n", collected_frame, frame_ptr, frame_size);
        kmdw_uvc2_queue_frame(isoch_pipe, frame_ptr, FRMAE_SIZE);
        return;
    }

    collected_frame++;

    kmdw_uvc2_queue_frame(isoch_pipe, frame_ptr, FRMAE_SIZE);

#ifndef PRINTF_FPS_ONLY
    kmdw_printf("[USBH2 test_2] %d frame_ptr 0x%p frame_size %u\n", collected_frame, frame_ptr, frame_size);
#endif
}

static void device_configured_callback(const kmdw_usbh2_device_descriptor_t *ptr_dev_desc, const kmdw_usbh2_configuration_descriptor_t *ptr_cfg_desc)
{

    osThreadFlagsSet(this_tid, TFLAG_UVC_CONFIGURE);

    if (ptr_dev_desc->idVendor != 0x1871 || ptr_dev_desc->idProduct != 0x0142)
    {
        kmdw_printf("[USBH2 test_2] this is not AVEO USB camera, cannot configure it!\n");
        return;
    }

    kmdw_printf("[USBH2 test_2] configuring the AVEO USB camera ...\n");

    // interface 1, alternate 5
    isoch_pipe = kmdw_uvc2_isoch_create(0x83, 0x13FC, 1, uvc_frame_cb);

    // allocate some frame buffers
    uint32_t all_frame_buf = kmdw_ddr_reserve(NUM_FRAME * FRMAE_SIZE);

    // queue frames into UVC middleware
    for (int i = 0; i < NUM_FRAME; i++)
        kmdw_uvc2_queue_frame(isoch_pipe, (uint32_t *)(all_frame_buf + i * FRMAE_SIZE), FRMAE_SIZE);

    // set class interface 1 altr 0 for VideoStreaming interface
    kmdw_usbh2_set_interface(1, 0);

    kmdw_uvc2_probe_commit_control_t uvc_ctrl;

    uvc_ctrl.bmHint = 0x0001;
    uvc_ctrl.bFormatIndex = 1;
    uvc_ctrl.bFrameIndex = 1;
    uvc_ctrl.dwFrameInterval = 333333;
    uvc_ctrl.wKeyFrameRate = 0;
    uvc_ctrl.wPFrameRate = 0;
    uvc_ctrl.wCompQuality = 0;
    uvc_ctrl.wCompWindowSize = 0;
    uvc_ctrl.wDelay = 0;
    uvc_ctrl.dwMaxVideoFrameSize = 0;
    uvc_ctrl.dwMaxPayloadTransferSize = 0;

    // VideoStreaming request - UVC_SET_CUR - Probe Control
    kmdw_uvc2_vs_control(UVC_SET_CUR, UVC_VS_PROBE_CONTROL, &uvc_ctrl);

    // VideoStreaming request - UVC_GET_CUR - Probe Control
    kmdw_uvc2_vs_control(UVC_GET_CUR, UVC_VS_PROBE_CONTROL, &uvc_ctrl);

    // VideoStreaming request - UVC_SET_CUR - Commit Control
    kmdw_uvc2_vs_control(UVC_SET_CUR, UVC_VS_COMMIT_CONTROL, &uvc_ctrl);

    // set class interface 1 altr 5 to start video streaming
    kmdw_usbh2_set_interface(1, 5);

    kmdw_uvc2_isoch_start(isoch_pipe);

    return;
}

static void device_disconnected_callback(void)
{
}

int ex_uvc_aveo_init(void)
{
    kmdw_printf("[USBH2 test_2] initializing USBH2 ...\n");

    this_tid = osThreadGetId();

    kmdw_printf("[USBH2 test_2] please connect the AVEO USB camera to the USB2 host port ...\n");

    // init USB host through usbh2 middleware
    kmdw_usbh2_status_t usb_status = kmdw_usbh2_initialize(device_configured_callback, device_disconnected_callback);
    if (usb_status != USBH_OK)
    {
        kmdw_printf("[USBH2 test_2] USBH2 middleware initialization ... FAILED\n");
        return 0;
    }

    osThreadFlagsWait(TFLAG_UVC_CONFIGURE, osFlagsWaitAny, osWaitForever);

    while (1)
    {
        int f0 = collected_frame;
        osDelay(5000);
        int f1 = collected_frame;
        float fps = (float)(f1 - f0) / 5;
        kmdw_printf("fps = %.2f\n", fps);
    }
}
