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
#include "kdp2_inf_app_tof.h"
#include "kdp2_usb_companion.h"

// --- hico tof include ---
#include "hico_tof.h"

#include "kmdw_camera.h"
#include "kdrv_timer.h"
#include "kdrv_i2c.h"
#include "kdrv_dpi2ahb.h"
#include "project.h"
#include "kmdw_sensor.h"
extern uint32_t kdrv_efuse_get_kn_number(void);

#define NUM_MIPI_INIT_BUFS 2 // ping-pong buffers
#define NUM_IMAGE_BUF 5
#define IMAGE_BUF_SIZE (2 * 1024 * 1024)
#define NUM_RESULT_BUF 5
#define RESULT_BUF_SIZE (1200 * 1024)

#define JTAG_MAGIC_ADDRESS 0x1FFFFFFC
#define JTAG_MAGIC_VALUE 0xFEDCBA01
#define USB_BOOT_MAGIC_HB 0xaabbccdd
#define USB_BOOT_MAGIC_LB 0x11223344

#ifdef ENABLE_DBG_LOG
#define dbg_log(__format__, ...) kmdw_printf("[kp hico mipi]"__format__, ##__VA_ARGS__)
#else
#define dbg_log(__format__, ...)
#endif

#define FLAG_WAIT_USB_CONNECTION 0x1

static osThreadId_t _usb_cmd_tid = NULL;
uint32_t g_tof_mode = MODE_LIVE_VIEW_INF;
static uint32_t _app_job_id = 0xFFFFFFFF;

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
        osThreadFlagsSet(_usb_cmd_tid, FLAG_WAIT_USB_CONNECTION);
        break;
    }
}

// vendor-specific control transfer setup packet notify
bool usb_user_control_callback(usbd_hal_setup_packet_t *setup)
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

static cam_format _cams_fmt[2] = {0};

// image ISR callback
void image_coming_callback(uint32_t cam_idx, uint32_t img_buf, uint32_t *p_new_img)
{
    // kmdw_printf("image_coming_callback, buf 0x%p\n", img_buf);

    uint32_t new_inf_buf;
    int buf_size;
    osStatus_t sts = kmdw_fifoq_manager_image_get_free_buffer(&new_inf_buf, &buf_size, 0, true);
    if (sts != osOK)
    {
        printf("(ISR) error !! retrieving new buf failed, kmdw_fifoq_manager_image_get_free_buffer() ret = %d\n", sts);
        // Due to highest priority ISR has,
        // image input is so fast that fifoq can't provide a free buffer even force_grab is set
        *p_new_img = img_buf;
        return;
    }

    *p_new_img = new_inf_buf + sizeof(kdp2_ipc_app_tof_inf_header_t);


    kdp2_ipc_app_tof_inf_header_t *tof_header = (kdp2_ipc_app_tof_inf_header_t *)(img_buf - sizeof(kdp2_ipc_app_tof_inf_header_t));

    tof_header->header_stamp.magic_type = KDP2_MAGIC_TYPE_INFERENCE;
    tof_header->header_stamp.total_size = sizeof(kdp2_ipc_app_tof_inf_header_t) + 1542400;
    tof_header->header_stamp.job_id = _app_job_id;
    tof_header->header_stamp.status_code = KP_SUCCESS;
    tof_header->header_stamp.total_image = 1;
    tof_header->header_stamp.image_index = 0;

    kmdw_fifoq_manager_image_enqueue(1, 0, (uint32_t)tof_header, IMAGE_BUF_SIZE, 0, false);

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
    memcpy(fmtstr, &_cams_fmt[cam_idx].pixelformat, 4);

    int buf_size;
    uint32_t buf_addr[NUM_MIPI_INIT_BUFS] = {0};

    // mipi needs some buffers at initization
    for (int i = 0; i < NUM_MIPI_INIT_BUFS; i++)
    {
        kmdw_fifoq_manager_image_get_free_buffer(&buf_addr[i], &buf_size, osWaitForever, false);
        buf_addr[i] += sizeof(kdp2_ipc_app_tof_inf_header_t);
    }

    if (0 != (ret = kmdw_camera_buffer_init(cam_idx, buf_addr[0], buf_addr[1])))
        return ret;

    if (0 != (ret = kmdw_camera_start(cam_idx, image_coming_callback)))
        return ret;

    kmdw_sensor_s_stream(cam_idx, 1);

    return 0;
}

#define CMD_BUF_SIZE (1 * 1024)

// this thread receive the command from host SW
void kdp2_hico_tof_usb_cmd_thread(void *arg)
{
    dbg_log("[%s] start !\n", __FUNCTION__);

    _usb_cmd_tid = osThreadGetId();

    // wait until usb connection is established
    osThreadFlagsWait(FLAG_WAIT_USB_CONNECTION, osFlagsWaitAny, osWaitForever);

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

        if (header_stamp->magic_type == KDP2_MAGIC_TYPE_COMMAND) // standard KDP2 commands
        {
            // borrow image fifo to handle KDP2 commands as well
            dbg_log("[%s] handle kdp2 command = 0x%x\n", __FUNCTION__, header_stamp->job_id);
            // handle kdp2 commands ...
            kdp2_cmd_handle_kp_command(cmd_buf);
            continue;
        }
        else if (header_stamp->magic_type == KDP2_MAGIC_TYPE_CUSTOMIZED) // customized commands
        {
            // customized command to configure mipi camera
            if (header_stamp->job_id == JID_START_HICO_TOF)
            {
                dbg_log("[%s] configuration command received\n", __FUNCTION__);

                hico_tof_config_response_t hico_tof_resp;

                hico_tof_resp.header_stamp.magic_type = KDP2_MAGIC_TYPE_CUSTOMIZED;
                hico_tof_resp.header_stamp.job_id = JID_START_HICO_TOF;
                hico_tof_resp.header_stamp.status_code = KP_SUCCESS;
                hico_tof_resp.header_stamp.total_size = sizeof(hico_tof_resp);

                hico_tof_config_command_t *hico_tof_cmd = (hico_tof_config_command_t *)cmd_buf;
                g_tof_mode = hico_tof_cmd->mode;
                _app_job_id = hico_tof_cmd->app_job_id;

                // Configure the mipi camera
                if (0 == camera_start(0, IMGSRC_0_WIDTH, IMGSRC_0_HEIGHT, IMGSRC_0_FORMAT))
                {
                    hico_tof_resp.cam_setting.img_width = 640;
                    hico_tof_resp.cam_setting.img_height = 480;
                    hico_tof_resp.cam_setting.img_format = KP_IMAGE_FORMAT_RAW8;
                }
                else
                {
                    hico_tof_resp.header_stamp.status_code = KP_ERROR_OTHER_99; // FIXME, give it a specific error code
                }

                usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)&hico_tof_resp, sizeof(hico_tof_resp), 1000);
            }
            else
            {
                dbg_log("error !!! wrong job_id = %d\n", __FUNCTION__, header_stamp->job_id);
            }
        }
        else if ((header_stamp->magic_type & 0xFFFF) == KDP_MSG_HDR_CMD) // very speical case for old arch. fw update
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

// this thread sends inference result to host
void kdp2_hico_tof_usb_result_thread(void *arg)
{
    dbg_log("[%s] start !\n", __FUNCTION__);

    while (1)
    {
        uint32_t result_buf_addr;
        int result_buf_length;

        // get result data from result fifo queue with blocking wait
        kmdw_fifoq_manager_result_dequeue(&result_buf_addr, &result_buf_length, osWaitForever);

        kp_inference_header_stamp_t *header_stamp = (kp_inference_header_stamp_t *)result_buf_addr;

        // send result to the host SW, blocking wait
        // dbg_log("[result] send usb data size %d\n", header_stamp->total_size);
        kdrv_status_t usb_sts = usbd_hal_bulk_send(KDP2_USB_ENDPOINT_DATA_IN, (void *)result_buf_addr, header_stamp->total_size, osWaitForever);
        if (usb_sts != KDRV_STATUS_OK) // KDRV_STATUS_USBD_TRANSFER_TERMINATED or KDRV_STATUS_USBD_TRANSFER_DISCONNECTED
        {
            dbg_log("error ! usbd_hal_bulk_send() ret = %d \n", usb_sts);
        }

        // return free buf back to queue
        kmdw_fifoq_manager_result_put_free_buffer(result_buf_addr, result_buf_length, osWaitForever);
    }
}

////////////////////////////////////////////////////////////

// KDP2 Inference Interface for HICO TOF code
// image input
// image + inference output
int kdp2_hico_tof_init()
{
    kmdw_printf("Running KDP2 HICO TOF\n");

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

    // HICO Mode
    uint16_t bcdDevice = KP_KDP2_FW_HICO_MODE;

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
    uint32_t buf_addr = kmdw_ddr_reserve(NUM_IMAGE_BUF * IMAGE_BUF_SIZE + NUM_RESULT_BUF * RESULT_BUF_SIZE);
    if (buf_addr == 0)
    {
        dbg_log("error !!! kmdw_ddr_reserve() failed for image/result buffers\n");
        return -1;
    }

    // queue buffers into image free-queue
    for (uint32_t i = 0; i < NUM_IMAGE_BUF; i++)
    {
        kmdw_fifoq_manager_image_put_free_buffer(buf_addr, IMAGE_BUF_SIZE, osWaitForever);
        buf_addr += IMAGE_BUF_SIZE;
    }

    // queue buffers into result free-queue
    for (uint32_t i = 0; i < NUM_RESULT_BUF; i++)
    {
        kmdw_fifoq_manager_result_put_free_buffer(buf_addr, RESULT_BUF_SIZE, osWaitForever);
        buf_addr += RESULT_BUF_SIZE;
    }

    kmdw_fifoq_manager_store_fifoq_config(NUM_IMAGE_BUF, IMAGE_BUF_SIZE, NUM_RESULT_BUF, RESULT_BUF_SIZE);

    return 0;
}
