// #define ENABLE_DBG_LOG

#include <string.h>
#include "cmsis_os2.h"
#include "kmdw_power_manager.h"

#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kdrv_gdma3.h"
#include "kdrv_power.h"

#include "usbd_hal.h"

#include "model_type.h"

#include "buffer_object.h"

#include "kmdw_fifoq_manager.h"
#include "kdp2_ipc_cmd.h"
#include "kdp2_inf_app_yolo.h"
#include "kdp2_usb_companion.h"

#include "host_in_uart_out.h"

#include "kmdw_camera.h"
#include "kdrv_timer.h"
#include "kdrv_i2c.h"
#include "kdrv_dpi2ahb.h"
#include "project.h"

#include "kdrv_gpio.h"
#include "kmdw_model.h"

#define BUF_HEADER_SIZE sizeof(buf_header_t)
#define OFFSET_INF_TO_HEAD_BUF (sizeof(buf_header_t) - sizeof(kdp2_ipc_app_yolo_inf_header_t))

#define IMG_BUF_TO_HEAD_BUF(img_buf) \
    (uint32_t)((uint32_t)img_buf - BUF_HEADER_SIZE)

extern uint32_t kdrv_efuse_get_kn_number(void);

#define NUM_MIPI_INIT_BUFS 2 // ping-pong buffers

#define NUM_IMG_BUF 10
#define IMG_BUF_SIZE (1 * 1024 * 1024)
#define NUM_RESULT_BUF 10
#define RESULT_BUF_SIZE (400 * 1024)
#define CMD_BUF_SIZE (1 * 1024)

#define JTAG_MAGIC_ADDRESS 0x1FFFFFFC
#define JTAG_MAGIC_VALUE 0xFEDCBA01
#define USB_BOOT_MAGIC_HB 0xaabbccdd
#define USB_BOOT_MAGIC_LB 0x11223344

#ifdef ENABLE_DBG_LOG
#define dbg_log(__format__, ...) kmdw_printf("[kp hico mipi]"__format__, ##__VA_ARGS__)
#else
#define dbg_log(__format__, ...)
#endif

static int dbg_mipi_cnt = 0;
static uint32_t _img_buf_start_addr_backup = 0;
static osThreadId_t _uart_upd_result_tid = NULL;

// usb link status notify
static void usb_user_link_status_callback(usbd_hal_link_status_t link_status)
{
    switch (link_status)
    {
    case USBD_STATUS_DISCONNECTED:
        kmdw_printf("USB is disconnected\n");
        break;

    case USBD_STATUS_CONFIGURED:
        kmdw_printf("USB is connected\n");
        break;
    }
}

// vendor-specific control transfer setup packet notify
static bool usb_user_control_callback(usbd_hal_setup_packet_t *setup)
{
    bool ret = false;

    dbg_log("control bRequest = 0x%x\n", setup->bRequest);

    switch (setup->bRequest)
    {
    case KDP2_CONTROL_REBOOT:
    {
        dbg_log("control reboot\n");
        kdrv_power_sw_reset();
        break;
    }
    case KDP2_CONTROL_SHUTDOWN:
    {
        dbg_log("control shutdown\n");
        kmdw_power_manager_shutdown();
        break;
    }
    case KDP2_CONTROL_FIFOQ_RESET:
    {
        dbg_log("control fifoq reset\n");
        // FIXME
        ret = true;
        break;
    }

    default:
        ret = false;
        break;
    }

    return ret;
}

static uint32_t retrieve_new_buffer_for_mipi()
{
    uint32_t new_head_buf_addr = 0;
    int buf_size;

    for (int i = 0; i < NUM_IMG_BUF; i++) // MAX try, not really a for-loop
    {
        uint32_t new_inf_buf_addr = 0;

        osStatus_t sts = kmdw_fifoq_manager_image_get_free_buffer(&new_inf_buf_addr, &buf_size, 0, true);
        if (sts != osOK) {
            kmdw_printf("(ISR %d) error !! kmdw_fifoq_manager_image_get_free_buffer() failed, sts = %d\n", dbg_mipi_cnt, sts);
            return 0;
        }

        new_head_buf_addr = new_inf_buf_addr - OFFSET_INF_TO_HEAD_BUF;
        buf_header_t *new_bufHdr = (buf_header_t *)new_head_buf_addr;

        if (new_bufHdr->read_ref_count == 0)
            break; // found available one
        else
        {
            // this buf is still in use, return it back to free buf queue
            sts = kmdw_fifoq_manager_image_put_free_buffer(new_inf_buf_addr, IMG_BUF_SIZE, 0);
            if (sts != osOK)
                kmdw_printf("(ISR %d) error !! kmdw_fifoq_manager_image_put_free_buffer() failed, sts = %d\n", dbg_mipi_cnt, sts);

            new_head_buf_addr = 0;
        }
    }

    if (new_head_buf_addr == 0)
    {
        kmdw_printf("(ISR %d) error !! cannot retrieve available free buffers\n", dbg_mipi_cnt);
        return 0;
    }

    return (new_head_buf_addr + BUF_HEADER_SIZE);
}

static cam_format _cams_fmt[2] = {0};

// image ISR callback
void image_coming_callback(uint32_t cam_idx, uint32_t img_buf_addr, uint32_t *pNew_img_buf)
{
    dbg_mipi_cnt++;

    // retrieve a new free buf (from the inference free buffer queue)
    *pNew_img_buf = retrieve_new_buffer_for_mipi();
    if (NULL == pNew_img_buf) {
        // Due to highest priority ISR has,
        // image input is coming so fast that fifoq can't provide a free buffer even force_grab is set
        // so just reuse the same buffer
        *pNew_img_buf = img_buf_addr;
        return;
    }


    // prepare to enqueue image for inference
    uint32_t head_buf = img_buf_addr - BUF_HEADER_SIZE;

    // initialize host img header
    buf_header_t *bufHdr = (buf_header_t *)head_buf;
    bufHdr->img_buf_offset = BUF_HEADER_SIZE;
    bufHdr->read_ref_count = 0;
    bufHdr->cam_index = cam_idx;

    kdp2_ipc_app_yolo_inf_header_t *infHdr = &bufHdr->inf_header;

    infHdr->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    infHdr->header_stamp.total_size = sizeof(kdp2_ipc_app_yolo_inf_header_t) + _cams_fmt[cam_idx].width * _cams_fmt[cam_idx].height * 2;
    infHdr->header_stamp.job_id = KDP2_INF_ID_APP_YOLO;
    infHdr->header_stamp.status_code = KP_SUCCESS;
    infHdr->inf_number = cam_idx;
    infHdr->width = _cams_fmt[cam_idx].width;
    infHdr->height = _cams_fmt[cam_idx].height;
    infHdr->channel = 3;
    infHdr->model_id = KNERON_YOLOV5S_COCO80_640_640_3;

    // Add support for more image formats if needed
    if (_cams_fmt[cam_idx].pixelformat == IMG_FORMAT_RGB565)
        infHdr->image_format = KP_IMAGE_FORMAT_RGB565;
    else if (_cams_fmt[cam_idx].pixelformat == IMG_FORMAT_RAW8)
        infHdr->image_format = KP_IMAGE_FORMAT_RAW8;

    infHdr->model_normalize = KP_NORMALIZE_KNERON;

    // non-blocking inference enqueue
    osStatus_t sts = kmdw_fifoq_manager_image_enqueue(1, 0, (uint32_t)infHdr, IMG_BUF_SIZE, 0, false);
    if (sts != osOK)
        kmdw_printf("(ISR) error !! kmdw_fifoq_manager_image_enqueue() failed, sts = %d\n", sts);

    return;
}

static uint32_t camera_start(uint8_t cam_idx, uint32_t width, uint32_t height, uint32_t pixelformat)
{
    uint32_t ret;
    struct cam_capability cap;

    _cams_fmt[cam_idx].width = width;
    _cams_fmt[cam_idx].height = height;
    _cams_fmt[cam_idx].pixelformat = pixelformat;

    char fmtstr[8];
    memset(&cap, 0, sizeof(cap));

    if (0 != (ret = kmdw_camera_open(cam_idx)))
        return ret;

    if (0 != (ret = kmdw_camera_get_device_info(cam_idx, &cap)))
        return ret;

    if (0 != (ret = kmdw_camera_set_frame_format(cam_idx, &_cams_fmt[cam_idx])))
        return ret;

    if (0 != (ret = kmdw_camera_get_frame_format(cam_idx, &_cams_fmt[cam_idx])))
        return ret;

    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &_cams_fmt[cam_idx], 4);

    int buf_size;
    uint32_t mipi_buf_addr[NUM_MIPI_INIT_BUFS] = {0};

    // mipi needs some buffers at initialization
    for (int i = 0; i < NUM_MIPI_INIT_BUFS; i++)
    {
        uint32_t head_buf;
        // retrieve free hico buffer
        kmdw_fifoq_manager_image_get_free_buffer(&head_buf, &buf_size, osWaitForever, false);
        // and use its img buffer part for MIPI input
        mipi_buf_addr[i] = head_buf + sizeof(kdp2_ipc_app_yolo_inf_header_t);
    }

    if (0 != (ret = kmdw_camera_buffer_init(cam_idx, mipi_buf_addr[0], mipi_buf_addr[1])))
        return ret;

    if (0 != (ret = kmdw_camera_start(cam_idx, image_coming_callback)))
        return ret;

    return 0;
}

// this thread receive the command from host SW by USB, only support updating FW or model to flash
void kdp2_host_recv_usb_cmd_thread(void *arg)
{
    dbg_log("[%s] start !\n", __FUNCTION__);

    while (USBD_STATUS_CONFIGURED != usbd_hal_get_link_status())
        osDelay(1);

    uint32_t cmd_buf = kmdw_ddr_reserve(CMD_BUF_SIZE);

    while (1)
    {
        uint32_t txLen = CMD_BUF_SIZE;
        kdrv_status_t usb_sts = usbd_hal_bulk_receive(KDP2_USB_ENDPOINT_DATA_OUT, (void *)cmd_buf, &txLen, osWaitForever);
        if (usb_sts != KDRV_STATUS_OK) // KDRV_STATUS_USBD_TRANSFER_TERMINATED or KDRV_STATUS_USBD_TRANSFER_DISCONNECTED
        {
            dbg_log("[%s] bulk receive is terminated, sts %d\n", __FUNCTION__, usb_sts);
            continue;
        }

        dbg_log("[%s] usb recv addr 0x%x len %d\n", __FUNCTION__, (void *)cmd_buf, txLen);

        kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)cmd_buf;

        if ((header_stamp->magic_type & 0xFFFF) == KDP_MSG_HDR_CMD) // very special case for old arch. fw update
        {
            // handle legendary kdp commands, should be as few as possible
            dbg_log("handle legendary kdp command = 0x%x\n", header_stamp->job_id);
            kdp2_cmd_handle_legend_kdp_command(cmd_buf);
        }
        else
        {
            dbg_log("error ! buffer begin with incorrect magic_type 0x%x, txLen %d\n", __FUNCTION__, header_stamp->magic_type, txLen);
        }
    }
}

enum {
    UART_CMD_START_YOLOV5S_INF_RGB = 0,
    UART_CMD_START_YOLOV5S_INF_NIR,
    UART_CMD_START_YOLOV5S_INF_RGB_NIR,
    UART_CMD_STOP_YOLOV5S_INF,
    UART_CMD_QUIT,
    UART_CMD_NUM
};

// this thread receive the command from UART or power button ?
void kdp2_host_recv_uart_cmd_thread(void *arg)
{
    int id = -1;
    char buf[64];
    bool cam_started[2] = {false};
    bool quit_thread = false;

    // Implement it if needed ? Press power button to toggle inference
    // power_button_register(power_btn_handler);

    while(1)
    {
        if (-1 != id)
            goto cmd_prompt;

        DSG("\n === Menu === ");
        DSG("( 0) Start YoloV5s Inference with RGB camera");
        DSG("( 1) Start YoloV5s Inference with NIR camera");
        DSG("( 2) Start YoloV5s Inference with RGB and NIR camera");
        DSG("( 3) Stop Inference");
        DSG("( 4) Quit");

cmd_prompt:
        DSG_NOLF(" command >> ");
        kmdw_console_echo_gets(buf, sizeof(buf));

        id = atoi(strtok(buf, " \r\n\t"));

        if (id >= 0 && id < UART_CMD_NUM)
        {
            // FIXME: add mechanism of releasing buffers used by mipi driver to mipi driver and kmdw_camera levels
            // following code is just a workaround
            if (UART_CMD_START_YOLOV5S_INF_RGB <= id && id <= UART_CMD_START_YOLOV5S_INF_RGB_NIR)
            {
                if (true == cam_started[0] || true == cam_started[1])
                {
                    DSG("Please stop the current running camera first");
                    goto cmd_prompt;
                }

                // Wait until all image buffers are not used
                while (osThreadBlocked != osThreadGetState(_uart_upd_result_tid))
                    osDelay(1);

                uint32_t temp_buf_addr;
                int temp_buf_size;

                // clean up image buffers
                while (osErrorResource != kmdw_fifoq_manager_image_get_free_buffer(&temp_buf_addr, &temp_buf_size, 0, true));

                uint32_t img_buf_addr = _img_buf_start_addr_backup;

                // requeue buffers into image free-queue
                for (uint32_t i = 0; i < NUM_IMG_BUF; i++)
                {
                    buf_header_t *bufHdr = (buf_header_t *)(img_buf_addr);
                    bufHdr->read_ref_count = 0;

                    kmdw_fifoq_manager_image_put_free_buffer((uint32_t)&bufHdr->inf_header, IMG_BUF_SIZE, osWaitForever);
                    img_buf_addr += IMG_BUF_SIZE; // next one
                }
            }

            switch(id)
            {
            case UART_CMD_START_YOLOV5S_INF_RGB:
                if (0 != camera_start(0, IMGSRC_0_WIDTH, IMGSRC_0_HEIGHT, IMGSRC_0_FORMAT))
                {
                    DSG("Error ! RGB camera start failed !");
                }
                else
                    cam_started[0] = true;
                break;
            case UART_CMD_START_YOLOV5S_INF_NIR:
                if (0 != camera_start(1, IMGSRC_1_WIDTH, IMGSRC_1_HEIGHT, IMGSRC_1_FORMAT))
                {
                    DSG("Error ! NIR camera start failed !");
                }
                else
                    cam_started[1] = true;
                break;
            case UART_CMD_START_YOLOV5S_INF_RGB_NIR:
                if (0 != camera_start(0, IMGSRC_0_WIDTH, IMGSRC_0_HEIGHT, IMGSRC_0_FORMAT) || 0 != camera_start(1, IMGSRC_1_WIDTH, IMGSRC_1_HEIGHT, IMGSRC_1_FORMAT))
                {
                    DSG("Error ! RGB or NIR camera start failed !");
                }
                else
                    cam_started[0] = cam_started[1] = true;
                break;
            case UART_CMD_STOP_YOLOV5S_INF:
            case UART_CMD_QUIT:
                if (true == cam_started[0])
                {
                    if (KMDW_STATUS_OK != kmdw_camera_stop(0))
                    {
                        DSG("Error ! RGB camera stop failed !");
                    }
                    if (KMDW_STATUS_OK != kmdw_camera_close(0))
                    {
                        DSG("Error ! RGB camera close failed !");
                    }
                    cam_started[0] = false;
                    kdrv_gpio_write_pin(RGB_LED, false); // switch off the LED
                }
                if (true == cam_started[1])
                {
                    if (KMDW_STATUS_OK != kmdw_camera_stop(1))
                    {
                        DSG("Error ! NIR camera stop failed !");
                    }
                    if (KMDW_STATUS_OK != kmdw_camera_close(1))
                    {
                        DSG("Error ! NIR camera close failed !");
                    }
                    cam_started[1] = false;
                }

                if (id == UART_CMD_QUIT)
                {
                    DSG("Quit");
                    quit_thread = true;
                }
                break;
            }
        }
        else {
            if (id)
                err_msg("Invalid command: %d\n", id);
            continue;
        }

        if (quit_thread)
            break;
    }
}

// this thread print inference result to uart. show inference result on local display if there is one !
void kdp2_host_update_result_thread(void *arg)
{
    dbg_log("[%s] start !\n", __FUNCTION__);

    _uart_upd_result_tid = osThreadGetId();

    while (1)
    {
        uint32_t result_buf_addr;
        int result_buf_length;

        // get result data from result fifo queue, blocking wait
        kmdw_fifoq_manager_result_dequeue(&result_buf_addr, &result_buf_length, osWaitForever);

        // then send inference result
        kdp2_ipc_app_yolo_result_t *yolo_result = (kdp2_ipc_app_yolo_result_t *)result_buf_addr;

        // get yolo result and print to uart
        if (yolo_result->inf_number == 0)
            kmdw_printf("RGB image inference result :\n");
        else if (yolo_result->inf_number == 1)
            kmdw_printf("NIR image inference result :\n");
        kmdw_printf("box count : %d\n", yolo_result->yolo_data.box_count);
        for (int i = 0; i < yolo_result->yolo_data.box_count; i++)
        {
            kmdw_printf("Box %d (x1, y1, x2, y2, score, class) = %.1f, %.1f, %.1f, %.1f, %f, %d\n",
                i,
                yolo_result->yolo_data.boxes[i].x1, yolo_result->yolo_data.boxes[i].y1,
                yolo_result->yolo_data.boxes[i].x2, yolo_result->yolo_data.boxes[i].y2,
                yolo_result->yolo_data.boxes[i].score, yolo_result->yolo_data.boxes[i].class_num);
        }

        // return free buf back to queue
        kmdw_fifoq_manager_result_put_free_buffer(result_buf_addr, result_buf_length, osWaitForever);
    }
}

////////////////////////////////////////////////////////////

// KDP2 Inference Interface for Host-in-uart-out code
// image input
// inference output (bounding box info in text) with UART
int kdp2_host_in_uart_out_init()
{
    // retrieve real serial number here from efuse
    // then convert it to hex string format
    uint32_t uid = kdrv_efuse_get_kn_number();

    int32_t sidx = 0;
    uint8_t kn_num_string[32] = {0};
    for (int i = 7; i >= 0; i--)
    {
        uint32_t hex = (uid >> i * 4) & 0xF;
        kn_num_string[sidx] = (hex < 10) ? '0' + hex : 'A' + (hex - 10);
        sidx += 2;
    }

    // Host Mode
    uint16_t bcdDevice = KP_KDP2_FW_HOST_MODE;

    if (*((uint32_t *)JTAG_MAGIC_ADDRESS) == JTAG_MAGIC_VALUE)
    {
        kmdw_printf("FW is running in JTAG mode\n");
        bcdDevice |= KP_KDP2_FW_JTAG_TYPE;
    }
    else
    {
        uint32_t magic_lb, magic_hb;
        magic_lb = (*(uint32_t *)(DDR_MAGIC_BASE));
        magic_hb = (*(uint32_t *)(DDR_MAGIC_BASE + 0x04));

        if ((magic_lb == USB_BOOT_MAGIC_LB) && (magic_hb == USB_BOOT_MAGIC_HB))
        {
            kmdw_printf("KDP2 FW is running in usb-boot mode\n");
            bcdDevice |= KP_KDP2_FW_USB_TYPE;
        }
        else
        {
            kmdw_printf("KDP2 FW is running in flash-boot mode\n");
            bcdDevice |= KP_KDP2_FW_FLASH_TYPE;
        }
    }

    usbd_hal_initialize(kn_num_string, bcdDevice, usb_user_link_status_callback, usb_user_control_callback);
    usbd_hal_set_enable(true);

    // wow ! fifoq can also handle command
    kdp2_cmd_handler_initialize();

    /* Allocate memory for image and result buffers */
    uint32_t img_buf_addr = kmdw_ddr_reserve(NUM_IMG_BUF * IMG_BUF_SIZE);
    if (img_buf_addr == 0)
    {
        dbg_log("error !!! kmdw_ddr_reserve() failed for image/result buffers\n");
        return -1;
    }

    // back up image buffer start address for reopening the camera
    // FIXME: this is just a workaround
    _img_buf_start_addr_backup = img_buf_addr;

    // queue buffers into image free-queue
    for (uint32_t i = 0; i < NUM_IMG_BUF; i++)
    {
        buf_header_t *bufHdr = (buf_header_t *)(img_buf_addr);
        bufHdr->read_ref_count = 0;

        kmdw_fifoq_manager_image_put_free_buffer((uint32_t)&bufHdr->inf_header, IMG_BUF_SIZE, osWaitForever);
        img_buf_addr += IMG_BUF_SIZE; // next one
    }

    uint32_t result_buf_addr = kmdw_ddr_reserve(NUM_RESULT_BUF * RESULT_BUF_SIZE);

    // queue buffers into result free-queue
    for (uint32_t i = 0; i < NUM_RESULT_BUF; i++)
    {
        kmdw_fifoq_manager_result_put_free_buffer(result_buf_addr, RESULT_BUF_SIZE, osWaitForever);
        result_buf_addr += RESULT_BUF_SIZE;
    }

    kmdw_fifoq_manager_store_fifoq_config(NUM_IMG_BUF, IMG_BUF_SIZE, NUM_RESULT_BUF, RESULT_BUF_SIZE);

    // load model from flash
    int32_t load_model_sts = kmdw_model_load_model(-1);
    if (0 < load_model_sts)
    {
        dbg_log("error !!! kmdw_model_load_model() failed for loading model from flash\n");
        return -1;
    }

    return 0;
}
