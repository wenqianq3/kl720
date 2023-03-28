#include "cmsis_os2.h"

#include <stdlib.h>
#include <string.h>
#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kmdw_usbd_uvc.h"
#include "kdev_flash.h"

/* Video frame */
uint8_t *ddr_frame[2] = {NULL, NULL};
#define DEMO_FRAME_WIDTH        640
#define DEMO_FRAME_HEIGHT       480
#define DEMO_FRAME_FOTMAT       UVC_FRAME_FORMAT_YUY2

#if defined DEMO_FRAME_FOTMAT && DEMO_FRAME_FOTMAT==UVC_FRAME_FORMAT_Y
#define DEMO_FRAME_SIZE         DEMO_FRAME_WIDTH*DEMO_FRAME_HEIGHT
#elif defined DEMO_FRAME_FOTMAT && DEMO_FRAME_FOTMAT==UVC_FRAME_FORMAT_YUY2
#define DEMO_FRAME_SIZE         DEMO_FRAME_WIDTH*DEMO_FRAME_HEIGHT*2
#endif
osEventFlagsId_t _evt_id;
#define EVENT_OPENED     0x01

static void send_video_thread(void *argument)
{
    uint8_t idx = 0;
    uint8_t *frame;
wait:
    osEventFlagsWait(_evt_id, EVENT_OPENED, osFlagsWaitAny, osWaitForever);
    while(1){
        if(kmdw_usbd_get_link_status() == KMDW_USBD_UVC_OPENED){
            if(kmdw_usbd_uvc_get_current_format() == 0){
                idx = 0;
                frame = ddr_frame[idx];
                kmdw_usbd_uvc_send_frame(frame, DEMO_FRAME_SIZE, KMDW_USBD_UVC_FRAME_FLAG_SOF_EOF);
            }
            else{
                idx = 1;
                frame = ddr_frame[idx];
                kmdw_usbd_uvc_send_frame(frame, DEMO_FRAME_SIZE, KMDW_USBD_UVC_FRAME_FLAG_SOF);
                idx = 0;
                frame = ddr_frame[idx];
                kmdw_usbd_uvc_send_frame(frame, DEMO_FRAME_SIZE, KMDW_USBD_UVC_FRAME_FLAG_EOF);
            }
        }
        else{
            kmdw_printf("KMDW_USBD_UVC_CLOSED\n");
            goto wait;
        }
        osDelay(200);
    }
}

void uvc_status_cb(kmdw_usbd_uvc_link_status_t status){
    kmdw_printf("uvc status - ");
    switch(status){
        case KMDW_USBD_UVC_DISCONNECTED:
            kmdw_printf("disconnected\n");
            break;
        case KMDW_USBD_UVC_CONNECTED:
            kmdw_printf("connected\n");
            break;
        case KMDW_USBD_UVC_OPENED:
            kmdw_printf("opened\n");
            osEventFlagsSet(_evt_id, EVENT_OPENED);
            break;
        case KMDW_USBD_UVC_CLOSED:
            kmdw_printf("closed\n");
            break;
    }
}

static void read_img_from_flash(uint32_t addr_in_flash, uint8_t *ddr, uint32_t size){
    int single_transfer = 2048;
    int total = size;
    int txf_size = 0;
    uint8_t *ptr = ddr;
    uint32_t flash = addr_in_flash;
    while(total){
        txf_size = total>single_transfer ? single_transfer:total;
        kdev_flash_readdata(flash, ptr, txf_size);
        total -= txf_size;
        ptr += txf_size;
        flash += txf_size;
    }
}

int test_usbd3_uvc(void){
    _evt_id = osEventFlagsNew(NULL);
    kmdw_printf("============UVC test============\n");
    kmdw_printf("Before running this example, test images need to be programed to the NAND flash in advance.\nFollow the following instruction before runnnng this demo\n");
    kmdw_printf("If raw image(Y only) test is performed, run the flash_prog_img_y.bat, and modify the DEMO_FRAME_FOTMAT to be UVC_FRAME_FORMAT_Y\n");
    kmdw_printf("If color image(YUY2) test is performed, run the flash_prog_img_yuy2.bat, and modify the DEMO_FRAME_FOTMAT to be UVC_FRAME_FORMAT_YUY2\n");
    kmdw_printf("============UVC test============\n");
    // Reserve DDR space for two images
    ddr_frame[0] = (uint8_t *)kmdw_ddr_reserve(DEMO_FRAME_SIZE);
    ddr_frame[1] = (uint8_t *)kmdw_ddr_reserve(DEMO_FRAME_SIZE);
    
    // Init flash driver
    kdev_flash_initialize();
    // Read pre-programmed images from flash
    read_img_from_flash(0x00760000, ddr_frame[0], DEMO_FRAME_SIZE);
    read_img_from_flash(0x00960000, ddr_frame[1], DEMO_FRAME_SIZE);

    // Initialize UVC middleware
    kmdw_printf("Initializing UVC device\n");
    kmdw_usbd_uvc_callbacks_t cb = {
        .kmdw_usbd_uvc_link_status = uvc_status_cb
    };
    kmdw_usbd_uvc_config_t cfg ={
        .frame_format = DEMO_FRAME_FOTMAT,
        .frame_width = DEMO_FRAME_WIDTH,
        .frame_height = DEMO_FRAME_HEIGHT,
        .frame_byte_per_pixel = DEMO_FRAME_FOTMAT==UVC_FRAME_FORMAT_Y?1:2
    };
    kmdw_usbd_uvc_init(&cfg, &cb);
    
    osThreadNew(send_video_thread, NULL, NULL);

    return 0;
}


