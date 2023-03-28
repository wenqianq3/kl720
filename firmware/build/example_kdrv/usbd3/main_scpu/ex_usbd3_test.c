#include "cmsis_os2.h"

#include <stdlib.h>
#include <string.h>

#include "kdrv_usbd3.h"

#include "kmdw_console.h"
#include "kmdw_memory.h"

#define KNERON_VID 0x3231
#define KNERON_KL720_PID 0x0200

#define ENP_BULK_OUT 0x01
#define ENP_BULK_IN 0x82
#define ENP_INTERRUPT_IN 0x83
#define ENP_NUM 3

/* ============= High-Speed descriptors ============= */

static kdrv_usbd3_device_descriptor_t hs_device_descriptor =
    {
        .bLength = 18,                 // 18 bytes
        .bDescriptorType = 0x01,       // Type : Device Descriptor
        .bcdUSB = 0x210,               // USB 2.10
        .bDeviceClass = 0x00,          // Device class, 0x0: defined by the interface descriptors
        .bDeviceSubClass = 0x00,       // Device sub-class
        .bDeviceProtocol = 0x00,       // Device protocol
        .bMaxPacketSize0 = 0x40,       // Max EP0 packet size: 64 bytes
        .idVendor = KNERON_VID,        // Vendor ID
        .idProduct = KNERON_KL720_PID, // Product ID
        .bcdDevice = 0x0001,           // Device release number
        .iManufacturer = 0x00,         // Manufacture string index, FIXME
        .iProduct = 0x00,              // Product string index, FIXME
        .iSerialNumber = 0x0,          // Serial number string index
        .bNumConfigurations = 1,       // Number of configurations, FIXME
};

#define HS_CONFIG_TOTOL_SIZE (9 + 9 + ENP_NUM * sizeof(kdrv_usbd3_endpoint_descriptor_t))
static kdrv_usbd3_config_descriptor_t hs_config_descriptor =
    {
        .bLength = 9,                         // 9 bytes
        .bDescriptorType = 0x02,              // Type: Configuration Descriptor
        .wTotalLength = HS_CONFIG_TOTOL_SIZE, // total bytes including config/interface/endpoint descriptors
        .bNumInterfaces = 1,                  // Number of interfaces
        .bConfigurationValue = 0x1,           // Configuration number
        .iConfiguration = 0x0,                // No String Descriptor
        .bmAttributes = 0xC0,                 // Self-powered, no Remote wakeup
        .MaxPower = 0xFA,                     // 500 mA
};

static kdrv_usbd3_interface_descriptor_t hs_interface_descriptor =
    {
        .bLength = 9,              // 9 bytes
        .bDescriptorType = 0x04,   // Inteface Descriptor
        .bInterfaceNumber = 0x0,   // Interface Number
        .bAlternateSetting = 0x0,  //
        .bNumEndpoints = ENP_NUM,  // number of endpoints
        .bInterfaceClass = 0xFF,   // Vendor specific
        .bInterfaceSubClass = 0x0, //
        .bInterfaceProtocol = 0x0, //
        .iInterface = 0x0,         // Interface descriptor string index
};

static kdrv_usbd3_endpoint_descriptor_t hs_enpoint_1_descriptor =
    {
        .bLength = 7,                     // 7 bytes
        .bDescriptorType = 0x05,          // Endpoint Descriptor
        .bEndpointAddress = ENP_BULK_OUT, // endpoint address
        .bmAttributes = 0x02,             // TransferType = Bulk
        .wMaxPacketSize = 512,            // max 512 bytes
        .bInterval = 0x00,                // never NAKs
};

static kdrv_usbd3_endpoint_descriptor_t hs_enpoint_2_descriptor =
    {
        .bLength = 7,                    // 7 bytes
        .bDescriptorType = 0x05,         // Endpoint Descriptor
        .bEndpointAddress = ENP_BULK_IN, // endpoint address
        .bmAttributes = 0x02,            // TransferType = Bulk
        .wMaxPacketSize = 512,           // max 1024 bytes
        .bInterval = 0x00,               // never NAKs
};

static kdrv_usbd3_endpoint_descriptor_t hs_enpoint_3_descriptor =
    {
        .bLength = 0x07,                      // 7 bytes
        .bDescriptorType = 0x05,              // Endpoint Descriptor
        .bEndpointAddress = ENP_INTERRUPT_IN, // endpoint address
        .bmAttributes = 0x03,                 // TransferType = Interrupt
        .wMaxPacketSize = 1024,               // max 1024 bytes
        .bInterval = 8,                       // 1 ms
};

static kdrv_usbd3_HS_descriptors_t hs_all_descriptor =
    {
        .dev_descp = &hs_device_descriptor,
        .config_descp = &hs_config_descriptor,
        .intf_descp = &hs_interface_descriptor,
        .enp_descp[0] = &hs_enpoint_1_descriptor,
        .enp_descp[1] = &hs_enpoint_2_descriptor,
        .enp_descp[2] = &hs_enpoint_3_descriptor,
        .qual_descp = NULL,
};

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
        .bEndpointAddress = ENP_BULK_OUT, // endpoint address
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
        .bEndpointAddress = ENP_BULK_IN, // endpoint address
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

static kdrv_usbd3_endpoint_descriptor_t ss_enpoint_3_descriptor =
    {
        .bLength = 7,                         // 7 bytes
        .bDescriptorType = 0x05,              // Endpoint Descriptor
        .bEndpointAddress = ENP_INTERRUPT_IN, // endpoint address
        .bmAttributes = 0x03,                 // TransferType = Interrupt
        .wMaxPacketSize = 1024,               // max 1024 bytes
        .bInterval = 1,                       // 1 ms
};

static kdrv_usbd3_endpoint_companion_descriptor_t ss_enpoint_3_companion_descriptor =
    {
        .bLength = 6,            // 6 bytes
        .bDescriptorType = 0x30, // Endpoint Companion Descriptor
        .bMaxBurst = 0,          // burst size = 1
        .bmAttributes = 0x0,     // no stream
        .wBytesPerInterval = 0,  // 0 for interrupt
};

static kdrv_usbd3_SS_descriptors_t ss_all_descriptor =
    {
        .dev_descp = &ss_device_descriptor,
        .config_descp = &ss_config_descriptor,
        .intf_descp = &ss_interface_descriptor,
        .enp_descp[0] = &ss_enpoint_1_descriptor,
        .enp_descp[1] = &ss_enpoint_2_descriptor,
        .enp_descp[2] = &ss_enpoint_3_descriptor,
        .enp_cmpn_descp[0] = &ss_enpoint_1_companion_descriptor,
        .enp_cmpn_descp[1] = &ss_enpoint_2_companion_descriptor,
        .enp_cmpn_descp[2] = &ss_enpoint_3_companion_descriptor,
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

#define TFLAG_DEV_CONFIGURE 0x1000

static osThreadId_t this_tid = 0;

static void link_status_callback(kdrv_usbd3_link_status_t link_status)
{
    switch (link_status)
    {
    case USBD3_STATUS_DISCONNECTED:
        kmdw_printf("device is diconnected\n");
        break;

    case USBD3_STATUS_CONFIGURED:

        if (kdrv_usbd3_get_link_speed() == USBD3_HIGH_SPEED)
            kmdw_printf("running at High-Speed\n");
        else if (kdrv_usbd3_get_link_speed() == USBD3_SUPER_SPEED)
            kmdw_printf("running at Super-Speed\n");
        else
        {
            kmdw_printf("running at NO-Speed\n");
            return;
        }

        osThreadFlagsSet(this_tid, TFLAG_DEV_CONFIGURE);
        break;
    }
}

static bool user_cx_callback(kdrv_usbd3_setup_packet_t *setup)
{
    return true;
}

#define BUF_SIZE (128 * 1024 * 1024)

int test_usbd3_loopback()
{
    uint32_t buffer = 0x80000000;

    if (buffer == 0)
    {
        kmdw_printf("memory allocation failed, test cannot begin ... FAILED\n");
        return 0;
    }

    this_tid = osThreadGetId();

    kmdw_printf("initializing USBD3 ...\n");
    kdrv_usbd3_initialize(&hs_all_descriptor, &ss_all_descriptor, &str_desc, link_status_callback, user_cx_callback);

    kdrv_usbd3_set_enable(true);

    kmdw_printf("waiting for connection ...\n");

    osThreadFlagsWait(TFLAG_DEV_CONFIGURE, osFlagsWaitAny, osWaitForever);

    kmdw_printf("connection is established ...\n");

    kmdw_printf("==============================\n");
    kmdw_printf("DDR loopback read/write test\n");
    kmdw_printf("buffer address 0x%x\n", buffer);
    kmdw_printf("buffer size %d\n", BUF_SIZE);
    kmdw_printf("ready !!\n");

    uint32_t count = 0;
    kdrv_status_t usb_sts;
    while (1)
    {
        uint32_t target_addr = 0x0;

        int sel = (count % 7);

        target_addr = 0x80000000 + sel * 0x1000000;

        uint32_t txfer_len = BUF_SIZE;

        usb_sts = kdrv_usbd3_bulk_receive(ENP_BULK_OUT, (void *)target_addr, &txfer_len, 5000);
        if (usb_sts != KDRV_STATUS_OK)
        {
            kmdw_printf("kdrv_usbd3_bulk_receive() failed, usb_sts = %d\n", usb_sts);
            if (usb_sts == KDRV_STATUS_TIMEOUT)
            {
                kmdw_printf("kdrv_usbd3_bulk_receive timeout, continue...\n");
                continue;
            }
            else
            {
                break;
            }
        }

        kmdw_printf("%d: received %d bytes\n", count, txfer_len);

        usb_sts = kdrv_usbd3_bulk_send(ENP_BULK_IN, (void *)target_addr, txfer_len, 5000);
        if (usb_sts != KDRV_STATUS_OK)
        {
            kmdw_printf("kdrv_usbd3_bulk_send() failed, usb_sts = %d\n", usb_sts);
            if (usb_sts == KDRV_STATUS_TIMEOUT)
            {
                kmdw_printf("kdrv_usbd3_bulk_send timeout, continue...\n");
                continue;
            }
            else
            {
                break;
            }
        }

        kmdw_printf("%d: sent %d bytes\n", count, txfer_len);
        count++;
    }

    kmdw_printf("loopback transfer is terminated\n");

    return 1;
}
