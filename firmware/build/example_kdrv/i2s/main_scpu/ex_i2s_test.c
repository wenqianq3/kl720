/*
 * Kneron JPEG test sample code 
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kdrv_io.h"
#include "kdrv_status.h"
#include "kdrv_timer.h"
#include "kdrv_clock.h"
#include "kmdw_console.h"
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "kdrv_i2s.h"
#include "kdrv_usbd3.h"
#include "kdrv_i2c.h"
#include "kdrv_pinmux.h"
#include "wm8731.h"
#include "project.h"

#define I2S_USE_DMA

typedef struct __attribute__((packed)) wavheader_s {
      //   RIFF Section    
      char ridd[4];              // Letters "RIFF"
      uint32_t size;              // Size of entire file less 8
      char riff_fmt[4];         // Letters "WAVE"
      
      //   Format Section    
      char fmt_sec_id[4];    // letters "fmt"
      uint32_t fmt_size;        // Size of format section less 8
      uint16_t fmt_id;          // 1=uncompressed PCM
      uint16_t num_channel;       // 1=mono,2=stereo
      uint32_t sample_rate;        // 44100, 16000, 8000 etc.
      uint32_t byte_rate;          // =SampleRate * Channels * (BitsPerSample/8)
      uint16_t block_align;        // =Channels * (BitsPerSample/8), effectivly the size of a single sample for all chans.
      uint16_t bits_per_sample;     // 8,16,24 or 32
    
      // Data Section
      char data_sec_id[4];      // The letters "data"
      uint32_t data_size;          // Size of the data that follows
}wavheader_t;
    
#define FLAG_FOR_USB_EVENT 0x100

#define ENP_CMD_BULK_OUT 0x02     // bulk-out endpoint
#define ENP_CMD_BULK_IN 0x81      // bulk-in endpoint
#define ENP_NUM 2
osEventFlagsId_t evt_id;                        // event flags id
osMutexId_t mux_id;
#define FLAGS_USB_RECEIVE       0x00000001U << 0
#define FLAGS_CMD_READY         0x00000001U << 1
#define FLAGS_CMD_DATA_READY    0x00000001U << 2
#define FLAGS_RESP_DATA_READY   0x00000001U << 3
#define FLAGS_SEND_ACK          0x00000001U << 4

uint8_t ack[512] = {0xDD, 0xCC, 0xBB, 0xAA};

int received_len = 0;
uint8_t buf[1024 * 8];

#define KHOST_CMD_PREFIX 0xAABBCCDD
#define KHOST_CMD_HEADER_LEN 16

#define CMD_ID_MEM_WRITE            0x01
#define CMD_ID_MEM_READ             0x02
#define CMD_ID_MEM_WRITE_DMA        0x03
#define CMD_ID_MEM_READ_DMA         0x04
#define CMD_ID_NCPU_RUN             0x05
#define CMD_ID_GO                   0x06
#define CMD_ID_GO_2                 0x07

typedef struct{
    uint32_t prefix;
    uint32_t cmd;
    uint32_t arg1;
    uint32_t arg2;
}cmd_header_t;

typedef struct{
    uint8_t *buf;
    uint32_t len;
}cmd_resp_t;

enum{
    KHOST_STATUS_IDLE,
    KHOST_STATUS_RX,
    KHOST_STATUS_BUSY,
    KHOST_STATUS_TX
};

typedef struct{
    uint8_t status;
    cmd_header_t cmd;
    cmd_resp_t resp;
    uint8_t is_dma;
}khost_handle_t;

volatile khost_handle_t khost_handler;

/* ============= Super-Speed descriptors ============= */

static kdrv_usbd3_device_descriptor_t ss_device_descriptor =
    {
        .bLength = 18,           // 18 bytes
        .bDescriptorType = 0x01, // Type : Device Descriptor
        .bcdUSB = 0x300,         // USB 3.0
        .bDeviceClass = 0x00,    // Device class, 0x0: defined by the interface descriptors
        .bDeviceSubClass = 0x00, // Device sub-class
        .bDeviceProtocol = 0x00, // Device protocol
        .bMaxPacketSize0 = 0x9,  // Maxpacket size for EP0 : 2^9
        .idVendor = 0x3231,      // Vendor ID
        .idProduct = 0x0200,     // Product ID
        .bcdDevice = 0x0001,     // Device release number
        .iManufacturer = 0x00,   // Manufacture string index, FIXME
        .iProduct = 0x00,        // Product string index, FIXME
        .iSerialNumber = 0x0,    // Serial number string index
        .bNumConfigurations = 1, // Number of configurations, FIXME
};

#define SS_CONFIG_TOTOL_SIZE (9 + 9 + ENP_NUM * (sizeof(kdrv_usbd3_endpoint_descriptor_t) + sizeof(kdrv_usbd3_endpoint_companion_descriptor_t)))
static kdrv_usbd3_config_descriptor_t ss_config_descriptor =
    {
        .bLength = 9,                         // 9 bytes
        .bDescriptorType = 0x02,              // Type: Configuration Descriptor
        .wTotalLength = SS_CONFIG_TOTOL_SIZE, // total bytes including config/interface/endpoint/companion descriptors
        .bNumInterfaces = 1,                  // Number of interfaces
        .bConfigurationValue = 0x1,           // Configuration number
        .iConfiguration = 0x0,                // No String Descriptor
        .bmAttributes = 0xC0,                 // Self-powered, no Remote wakeup
        .MaxPower = 0x0,                      // 0
};

static kdrv_usbd3_interface_descriptor_t ss_interface_descriptor =
    {
        .bLength = 9,             // 9 bytes
        .bDescriptorType = 0x04,  // Inteface Descriptor
        .bInterfaceNumber = 0x0,  // Interface Number
        .bAlternateSetting = 0x0, //
        .bNumEndpoints = ENP_NUM,
        .bInterfaceClass = 0xFF,   // Vendor specific
        .bInterfaceSubClass = 0x0, //
        .bInterfaceProtocol = 0x0, //
        .iInterface = 0x0,         // Interface descriptor string index
};

static kdrv_usbd3_endpoint_descriptor_t ss_enpoint_1_descriptor =
    {
        .bLength = 7,                     // 7 bytes
        .bDescriptorType = 0x05,          // Endpoint Descriptor
        .bEndpointAddress = ENP_CMD_BULK_OUT, // endpoint address
        .bmAttributes = 0x02,             // TransferType = Bulk
        .wMaxPacketSize = 1024,           // max 1024 bytes
        .bInterval = 0x00,                // never NAKs
};

static kdrv_usbd3_endpoint_companion_descriptor_t ss_enpoint_1_companion_descriptor =
    {
        .bLength = 6,            // 6 bytes
        .bDescriptorType = 0x30, // Endpoint Companion Descriptor
        .bMaxBurst = 0,          // burst size = 1
        .bmAttributes = 0x0,     // no stream
        .wBytesPerInterval = 0,  // 0 for bulk
};

static kdrv_usbd3_endpoint_descriptor_t ss_enpoint_2_descriptor =
    {
        .bLength = 7,                    // 7 bytes
        .bDescriptorType = 0x05,         // Endpoint Descriptor
        .bEndpointAddress = ENP_CMD_BULK_IN, // endpoint address
        .bmAttributes = 0x02,            // TransferType = Bulk
        .wMaxPacketSize = 1024,          // max 1024 bytes
        .bInterval = 0x00,               // never NAKs
};

static kdrv_usbd3_endpoint_companion_descriptor_t ss_enpoint_2_companion_descriptor =
    {
        .bLength = 6,            // 6 bytes
        .bDescriptorType = 0x30, // Endpoint Companion Descriptor
        .bMaxBurst = 0,          // burst size = 1
        .bmAttributes = 0x0,     // no stream
        .wBytesPerInterval = 0,  // 0 for bulk
};

static kdrv_usbd3_SS_descriptors_t ss_all_descriptor =
    {
        .dev_descp = &ss_device_descriptor,
        .config_descp = &ss_config_descriptor,
        .intf_descp = &ss_interface_descriptor,
        .enp_descp[0] = &ss_enpoint_1_descriptor,
        .enp_descp[1] = &ss_enpoint_2_descriptor,
        .enp_cmpn_descp[0] = &ss_enpoint_1_companion_descriptor,
        .enp_cmpn_descp[1] = &ss_enpoint_2_companion_descriptor,
};

#define USB_MANUFACTURER_STRING {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0}
#define USB_PRODUCT_STRING {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0,' ',0,'K',0,'L',0,'7',0,'2',0,'0',0}
#define USB_SERIAL_STRING {'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0}

kdrv_usbd3_prd_string_descriptor_t str_desc_mun =
{
    .bLength = (2 + 12),
    .bDescriptorType = 0x03,
    .bString = USB_MANUFACTURER_STRING,
};

kdrv_usbd3_prd_string_descriptor_t str_desc_prd =
{
    .bLength = (2 + 24),
    .bDescriptorType = 0x03,
    .bString = USB_PRODUCT_STRING,
};

kdrv_usbd3_prd_string_descriptor_t str_desc_serial =
{
    .bLength = (2 + 16),
    .bDescriptorType = 0x03,
    .bString = USB_SERIAL_STRING,
};

kdrv_usbd3_string_descriptor_t str_desc =
{
    .bLength = 4,
    .bDescriptorType = 0x03,
    .bLanguageID = 0x0409,
    .desc[0] = &str_desc_mun,
    .desc[1] = &str_desc_prd,
    .desc[2] = &str_desc_serial,
};


static kdrv_status_t send_ack(){
    kdrv_status_t status;
    status = kdrv_usbd3_bulk_send(ENP_CMD_BULK_IN, (uint32_t *)ack, 4, 1000);
    return status;
}

static uint8_t is_in_DMA_range(uint32_t addr){
    uint8_t ret = 0;
#ifndef _BOARD_SN530HAPS_H_
    if(addr >= DDR_MEM_PHY_BASE && addr < DDR_MEM_PHY_BASE + DDR_MEM_PHY_SIZE){
        ret = 1;
    }
#endif
    return ret;
}

static void reset_khost_handler(){
    khost_handler.cmd.cmd = 0;
    khost_handler.cmd.arg1 = 0;
    khost_handler.cmd.arg2 = 0;
    khost_handler.is_dma = 0;
}

static void bulk_out_thread(void *argument)
{
    uint32_t blen = 1024;
    uint32_t inc = 0;
    kdrv_status_t status;
    uint32_t flags;
    kdrv_usbd3_set_enable(true);

    kmdw_printf("waiting for device being configured\n");

    while (!(kdrv_usbd3_get_link_status()==USBD3_STATUS_CONFIGURED))
        osDelay(500);

    kmdw_printf("device is enumerated & configured\n");
    khost_handler.is_dma = 0;
    osEventFlagsSet(evt_id, FLAGS_USB_RECEIVE);
    uint8_t done = 1;
    uint8_t *recv_buf = buf;
    while (1)
    {
        if(done){
            flags = osEventFlagsWait(evt_id, FLAGS_USB_RECEIVE, osFlagsWaitAny, osWaitForever);
            osEventFlagsClear(evt_id, flags);
            inc = 0;
        }
        
        if(khost_handler.status == KHOST_STATUS_IDLE){
            reset_khost_handler();
        }
        
        if(khost_handler.is_dma){
            recv_buf = (uint8_t *)khost_handler.cmd.arg1;
        }
        else{
            recv_buf = buf;
        }
        
        uint32_t rxLen = blen;
        status = kdrv_usbd3_bulk_receive(ENP_CMD_BULK_OUT, (uint32_t *)(recv_buf+inc), &rxLen, 0);
        done = 0;
        if (status == KDRV_STATUS_OK)
        {
            //kmdw_printf("endpoint 0x%x received %d bytes\n", ENP_CMD_BULK_OUT, rxLen);
            received_len = rxLen;
            if(khost_handler.status == KHOST_STATUS_IDLE && rxLen >= KHOST_CMD_HEADER_LEN){
                cmd_header_t *p = (cmd_header_t *)recv_buf;
                if(p->prefix == KHOST_CMD_PREFIX){ // command valid
                    khost_handler.cmd.cmd = p->cmd;
                    khost_handler.cmd.arg1 = p->arg1;
                    khost_handler.cmd.arg2 = p->arg2;
                    khost_handler.is_dma = 0;
                    inc = 0;
                    done = 1;
                    osEventFlagsSet(evt_id, FLAGS_CMD_READY);
                }
            }
            else if(khost_handler.status == KHOST_STATUS_RX){
                //kmdw_printf("KHOST_STATUS_RX %d bytes\n", rxLen);
                switch(khost_handler.cmd.cmd){
                    case CMD_ID_MEM_WRITE: 
                        inc += rxLen;
                        if(inc >= khost_handler.cmd.arg2){
                            //kmdw_printf("write done\n");
                            done = 1;
                            osEventFlagsSet(evt_id, FLAGS_CMD_DATA_READY);
                        }
                        break;
                }
            }
            else{
                //kmdw_printf("others\n");
            }
        }
        else
        {
            kmdw_printf("kdrv_usbd_bulk_receive(enp:0x%x) failed, error: %d\n", ENP_CMD_BULK_OUT, status);
        }
    }
}

static void bulk_in_thread(void *argument)
{
    kdrv_status_t status;
    uint32_t sendBytes = 0;
    uint32_t flags;
    uint8_t *out;
    while (1)
    {
        flags = osEventFlagsWait(evt_id, FLAGS_RESP_DATA_READY|FLAGS_SEND_ACK, osFlagsWaitAny, osWaitForever);
        osEventFlagsClear(evt_id, flags);
        if(flags == FLAGS_RESP_DATA_READY){
            sendBytes = khost_handler.resp.len;
            if(!khost_handler.is_dma){
                memcpy(buf, khost_handler.resp.buf, sendBytes);
                out = buf;
            }
            else{
                out = khost_handler.resp.buf;
            }
            status = kdrv_usbd3_bulk_send(ENP_CMD_BULK_IN, (uint32_t *)out, sendBytes, 5000);
            if (status == KDRV_STATUS_OK){
                //kmdw_printf("endpoint 0x%x sent %d bytes\n", ENP_CMD_BULK_IN, tx_cnt);
            }
            else{
                kmdw_printf("kdrv_usbd_bulk_send(enp:0x%x) failed, error: %d %d\n", ENP_CMD_BULK_IN, status, sendBytes);
            }
            khost_handler.status = KHOST_STATUS_IDLE;
            osEventFlagsSet(evt_id, FLAGS_USB_RECEIVE);
        }
        else if(flags == FLAGS_SEND_ACK){
            send_ack();
            switch(khost_handler.cmd.cmd){
                 case CMD_ID_MEM_WRITE:
                     osEventFlagsSet(evt_id, FLAGS_USB_RECEIVE);
                     break;
                 case CMD_ID_MEM_READ:
                     osEventFlagsSet(evt_id, FLAGS_RESP_DATA_READY);
                     break;
                 case CMD_ID_GO:
                 case CMD_ID_GO_2:
                     osEventFlagsSet(evt_id, FLAGS_USB_RECEIVE);
                     break;
            }
        }
    }
}

void gdma_xfer_callback(kdrv_status_t status, void *arg){

}

static void wm8731_init_dac(uint8_t mode);
static void wm8731_init_adc(uint8_t mode);

static void cmd_process_thread(void *argument)
{
    uint32_t flags;
    wavheader_t *header;
    
#ifndef I2S_USE_DMA
    uint16_t *ptr;
    uint32_t inc;
#else
    uint32_t *ptr2;
#endif

    uint32_t data_size = 0;
    while (1)
    {
        flags = osEventFlagsWait(evt_id, FLAGS_CMD_READY|FLAGS_CMD_DATA_READY, osFlagsWaitAny, osWaitForever);
        osEventFlagsClear(evt_id, flags);
        //kmdw_printf("cmd_process_thread 0x%x\n",khost_handler.cmd.cmd);
        if(flags == FLAGS_CMD_READY){
            switch(khost_handler.cmd.cmd){
                case CMD_ID_MEM_WRITE:
                    //kmdw_printf("CMD_ID_MEM_WRITE 0x%X 0x%X\n", khost_handler.cmd.arg1, khost_handler.cmd.arg2);
                    khost_handler.status = KHOST_STATUS_RX;
                    if(is_in_DMA_range(khost_handler.cmd.arg1)){
                        khost_handler.is_dma = 1;
                    }
                    osEventFlagsSet(evt_id, FLAGS_SEND_ACK);
                    break;
                case CMD_ID_MEM_READ:
                    khost_handler.resp.buf = (uint8_t *)khost_handler.cmd.arg1;
                    khost_handler.resp.len = khost_handler.cmd.arg2;
                    //kmdw_printf("CMD_ID_MEM_READ 0x%X 0x%X\n", khost_handler.resp.buf, khost_handler.resp.len);
                    if(is_in_DMA_range(khost_handler.cmd.arg1)){
                        khost_handler.is_dma = 1;
                    }
                    khost_handler.status = KHOST_STATUS_TX;
                    osEventFlagsSet(evt_id, FLAGS_SEND_ACK);
                    break;
                case CMD_ID_GO:
                    osEventFlagsSet(evt_id, FLAGS_SEND_ACK);

                    wm8731_init_dac(0);
                    header = (wavheader_t *)(0x80000000);
                    kmdw_printf("wav file info:\n");
                    kmdw_printf("channel number :%d\n", header->num_channel);
                    kmdw_printf("sample rate :%d\n", header->sample_rate);
                    kmdw_printf("byte rate :%d\n", header->byte_rate);
                    kmdw_printf("bits per sample :%d\n", header->bits_per_sample);
                    kmdw_printf("data size :%d\n", header->data_size);
                
                    kdrv_i2s_enable_tx((kdrv_i2s_instance_t)I2S_PORT, 1);    
                    data_size = header->data_size >> 1;
                    kmdw_printf("play start\n");
#ifndef I2S_USE_DMA
                    ptr = (uint16_t *)(0x80000000 + sizeof(wavheader_t));
                    inc = 0;
                    while(1){
                        uint32_t data = *(ptr + inc);
                        if(kdrv_i2s_send((kdrv_i2s_instance_t)I2S_PORT, data) == KDRV_STATUS_OK){
                            inc++;
                        }
                        if(inc >= data_size){
                            break;
                        }
                    }
                 
#else
                    ptr2 = (uint32_t *)(0x80000000 + sizeof(wavheader_t));
                    kdrv_gdma_handle_t g_handle;
                    g_handle = kdrv_gdma_acquire_handle();
                    kdrv_i2s_dma_send((kdrv_i2s_instance_t)I2S_PORT, g_handle, ptr2, header->data_size, NULL);
                    kdrv_i2s_enable_tx((kdrv_i2s_instance_t)I2S_PORT, 0);
                    kdrv_gdma_release_handle(g_handle);
#endif                

                    kmdw_printf("play done\n");
                    break;
                case CMD_ID_GO_2:
                    osEventFlagsSet(evt_id, FLAGS_SEND_ACK);
                    kmdw_printf("Start Record for 10 seconds...\n");
                    wm8731_init_adc(0);

                    kdrv_i2s_enable_rx((kdrv_i2s_instance_t)I2S_PORT, 1);
                    data_size = (32 * 1024 * 2 * 2) * 10; //32khz * 2byte * 2channel * 10 seconds
                    ptr2 = (uint32_t *)(0x80000000);
                    kdrv_gdma_handle_t g_handle2;
                    g_handle2 = kdrv_gdma_acquire_handle();
                    kdrv_i2s_dma_read((kdrv_i2s_instance_t)I2S_PORT, g_handle2, ptr2, data_size, NULL);
                    kdrv_i2s_enable_rx((kdrv_i2s_instance_t)I2S_PORT, 0);
                    kdrv_gdma_release_handle(g_handle2);
                    kmdw_printf("record to the record.wav in the tool folder done\n");
                    break;
                default:
                    break;
            }
            
        }
        else if(flags == FLAGS_CMD_DATA_READY){
            switch(khost_handler.cmd.cmd){
                case CMD_ID_MEM_WRITE:
                    if(!khost_handler.is_dma){
                        memcpy((void *)(khost_handler.cmd.arg1), buf, khost_handler.cmd.arg2);
                    }
                    khost_handler.status = KHOST_STATUS_IDLE;
                    osEventFlagsSet(evt_id, FLAGS_SEND_ACK);
                    break;
            }
        }
    }
}

void usbd_sync_example_init(void)
{
    khost_handler.status = KHOST_STATUS_IDLE;
    
    // create threads to handle usb transfers
    evt_id = osEventFlagsNew(NULL);
    mux_id = osMutexNew(NULL);
    osThreadNew(bulk_out_thread, NULL, NULL);
    osThreadNew(bulk_in_thread, NULL, NULL);
    osThreadNew(cmd_process_thread, NULL, NULL);
    
    kdrv_usbd3_initialize(NULL, &ss_all_descriptor, &str_desc, NULL, NULL);

    kdrv_usbd3_set_enable(true);
}

/**
 * @brief i2s test program
 */
uint32_t i2s_selftest(void)
{
    kmdw_printf("=====i2s test=====\n");
    kmdw_printf("Execute the wav.py from the tool folder to transfer wav file\n");
 
    // init wm8731
    wm8731_set_i2c_address(WM8731_SLAVE_ADDRESS_LOW);
    wm8731_reset();

   // init usb for send/receive data
    usbd_sync_example_init();
    return KDRV_STATUS_OK;
}

/*
WM7831 as master:
BCLK is always 64 x 48kHz = 3072 Khz
The desired sampling rate is adjusted by adding padding bits to achieve corresponding RCLK value.
For example, the desired sampling is 32Khz then the corresponding RCLK is 32KHz, so data bits is then 48 bits.
If the valid data bits is set to 16 bits, the remaing 32 bits are padding bits.
*/
static void wm8731_init_dac(uint8_t mode){
    wm8731_reset();
    wm8731_set_master_or_slave_mode(mode); // 1 for master mode
    wm8731_dac_select(1); //endcode
    wm8731_set_sampling_control(WM8731_SAMPLE_32K); // normal mode, 32k sampling rate
    uint16_t val = WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_I2S;
    wm8731_set_digital_audio_data_bit_length(val);
    val = WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_IWL_16_BIT;
    wm8731_set_digital_audio_data_bit_length(val);
    wm8731_set_dac_soft_mute(0);
    wm8731_power_mode_dac();
    wm8731_set_active(1);
}

static void wm8731_init_adc(uint8_t mode){
    wm8731_reset();
    wm8731_set_master_or_slave_mode(mode); // 1 for master mode
    wm8731_adc_input_select(1); //select mic
    wm8731_set_sampling_control(WM8731_SAMPLE_32K); // normal mode, 32k sampling rate
    uint16_t val = WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_I2S;
    wm8731_set_digital_audio_data_bit_length(val);
    val = WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_IWL_16_BIT;
    wm8731_set_digital_audio_data_bit_length(val);
    wm8731_enable_adc_high_pass_filter(1);
    wm8731_set_mic_mute(0);
    wm8731_power_mode_adc_mic();
    wm8731_set_active(1);
}
