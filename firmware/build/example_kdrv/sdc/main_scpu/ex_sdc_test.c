/*
 * Kneron Peripheral API - SDC Test function
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "io.h"
#include "kdrv_sdc.h"
#include "kmdw_console.h"
#include "kdrv_power.h"
#include "kdrv_scu_ext.h"
#include "kdrv_status.h"
#include "kdrv_timer.h"

#define DBG_CARD_INFO           1
extern uint32_t perftimerid;
typedef signed long clock_t;
#define USE_DDR
#ifdef USE_DDR
#define TEST_LOOP               1
#define TEST_LEN                16 * 1024    //32KB test length
static uint8_t *read_buf= (uint8_t *) 0x80300000;
static uint8_t *write_buf = (uint8_t *) 0x80400000;
#else
#define TEST_LOOP               1
#define TEST_LEN                512
static uint8_t read_buf[TEST_LEN];
static uint8_t write_buf[TEST_LEN];
#endif

const int8_t *abort_type_table[] = {
    {"Asynchronous abort"},
    {"Synchronous abort"},
    {"Undefined abort type"}
};

const int8_t *transfer_speed_table[] = {
    "Normal Speed / SDR12",
    "High Speed / SDR25",
    "SDR50-100MHz",
    "SDR104-208MHz",
    "DDR50",
    "Undefined Speed"
};

const int8_t *transfer_type_table[] = {
    "ADMA",
    "SDMA",
    "PIO",
    "External DMA",
    "Undefined Transfer Type"
};

const int8_t *kdrv_status_msg[] = {
    "KDRV_STATUS_OK",
    "KDRV_STATUS_ERROR",
    "KDRV_STATUS_INVALID_PARAM",
    "KDRV_STATUS_I2C_BUS_BUSY",
    "KDRV_STATUS_I2C_DEVICE_NACK",
    "KDRV_STATUS_I2C_TIMEOUT",
    "KDRV_STATUS_USBD_INVALID_ENDPOINT",
    "KDRV_STATUS_USBD_TRANSFER_TIMEOUT",
    "KDRV_STATUS_USBD_INVALID_TRANSFER",
    "KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS",
    "KDRV_STATUS_GDMA_ERROR_NO_RESOURCE",
    "KDRV_STATUS_TIMER_ID_NOT_IN_USED",
    "KDRV_STATUS_TIMER_ID_IN_USED",
    "KDRV_STATUS_TIMER_ID_NOT_AVAILABLE",
    "KDRV_STATUS_TIMER_INVALID_TIMER_ID",
    "KDRV_STATUS_UART_TX_RX_BUSY",
    "KDRV_STATUS_UART_TIMEOUT",
    "KDRV_STATUS_SDC_CMD_ERR",
    "KDRV_STATUS_SDC_INIT_ERR",
    "KDRV_STATUS_SDC_CARD_NO_EXISTED",
    "KDRV_STATUS_SDC_CARD_TYPE_ERR",
    "KDRV_STATUS_SDC_CSD_EXT_READ_ERR",
    "KDRV_STATUS_SDC_CID_READ_ERR",
    "KDRV_STATUS_SDC_MEM_ALLOC_ERR",
    "KDRV_STATUS_SDC_READ_FAIL",
    "KDRV_STATUS_SDC_WRITE_FAIL",
    "KDRV_STATUS_SDC_TRANSFER_FAIL",
    "KDRV_STATUS_SDC_TIMEOUT",
    "KDRV_STATUS_SDC_CMD_NOT_SUPPORT",
    "KDRV_STATUS_SDC_BUS_WIDTH_NOT_SUPPORT",
    "KDRV_STATUS_SDC_BUS_WIDTH_ERR",
    "KDRV_STATUS_SDC_SPEED_MOD_ERR",
    "KDRV_STATUS_SDC_VOL_ERR",
    "KDRV_STATUS_SDC_INHIBIT_ERR",
    "KDRV_STATUS_SDC_RECOVERABLE_ERR",
    "KDRV_STATUS_SDC_ABORT_ERR",
    "KDRV_STATUS_SDC_SWITCH_ERR",
    "KDRV_STATUS_SDC_PWR_SET_ERR",
};

uint8_t *kdrv_status_msg_show(kdrv_status_t id)
{
    return (uint8_t *)kdrv_status_msg[id];
}

uint8_t *sdc_abort_type_to_str(kdrv_sdc_abort_type_e type)
{
    if (type > ABORT_UNDEFINED)
        type = ABORT_UNDEFINED;

    return (uint8_t *)abort_type_table[type];
}

/**
 * @brief sdc_transfer_type_to_str() convert kdrv_sdc_transfer_type_e to string
 */
uint8_t *sdc_transfer_type_to_str(kdrv_sdc_transfer_type_e tType)
{
    if (tType > TRANS_UNKNOWN)
        tType = TRANS_UNKNOWN;

    return (uint8_t *)transfer_type_table[tType];
}

/**
 * @brief sdc_transfer_speed_to_str() convert transfer_speed to string
 */
uint8_t *sdc_transfer_speed_to_str(kdrv_sdc_bus_speed_e speed)
{
    if (speed > SPEED_RSRV)
        speed = SPEED_RSRV;

    return (uint8_t *)transfer_speed_table[speed];
}


/**
 * @brief sdc_scr_display() display scr information
 */
void sdc_scr_display(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    kmdw_printf("**************** scr register ****************\n");
    kmdw_printf("SCR Structure: %d\n", card_info->scr.scr_structure);
    kmdw_printf("SCR Memory Card - Spec. Version: %d\n", card_info->scr.sd_spec);
    kmdw_printf("Data status after erase: %d\n", card_info->scr.data_stat_after_erase);
    kmdw_printf("SD Security Support: %d\n", card_info->scr.sd_security);
    kmdw_printf("DAT Bus widths supported: %d\n", card_info->scr.sd_bus_widths);
    kmdw_printf("CMD23 supported: %d\n", card_info->scr.cmd23_support);
    kmdw_printf("CMD20 supported: %d\n", card_info->scr.cmd20_support);
    kmdw_printf("**********************************************\n");
}


/**
 * @brief sdc_ext_cid_display() display cid information
 */
void sdc_ext_cid_display(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card = dev->card_info;
    kmdw_printf("**************** CID register **************** %x\n",&card->cid_lo);
    if (card->card_type == MEMORY_CARD_TYPE_SD) {
        kmdw_printf("Manufacturer ID: 0x%02x\n", (unsigned long)(card->cid_hi >> 48));
        kmdw_printf("OEM / App ID: 0x%04x\n", (unsigned long)((card->cid_hi >> 32) & 0xFFFF));
        kmdw_printf("Product Name: 0x%02x\n",
                    (unsigned long)((((card->cid_hi) & 0xFFFFFFFF) << 8) | (card->cid_lo >> 56) & 0xFF));
        kmdw_printf("Product Revision: %d.%d\n", (card->cid_lo >> 52) & 0xF, (card->cid_lo >> 48) & 0xF);
        kmdw_printf("Product Serial No.: %u\n", (card->cid_lo >> 16) & 0xFFFFFFFF);
        kmdw_printf("reserved:0x%x\n", (unsigned long)((card->cid_lo & 0xF000) >> 12));
        kmdw_printf("Manufacturer Date: %d - %0.2d\n",
                    ((card->cid_lo & 0xFF0) >> 4) + 2000, (card->cid_lo & 0xF));
    } else {
        kmdw_printf("Manufacturer ID: 0x%02x\n", (unsigned long)(card->cid_hi >> 48));
        kmdw_printf("OEM / App ID: 0x%04x\n", (unsigned long)((card->cid_hi >> 32) & 0xFFFF));
        kmdw_printf("Product Name: 0x%02x\n",
                    (unsigned long)((((card->cid_hi) & 0xFFFFFFFF) << 16) | (card->cid_lo >> 48) & 0xFFFF));
        kmdw_printf("Product Revision: %d.%d\n", (card->cid_lo >> 44) & 0xF, (card->cid_lo >> 40) & 0xF);
        kmdw_printf("Product Serial No.: %u\n", (card->cid_lo >> 8) & 0xFFFFFFFF);
        kmdw_printf("Manufacturer Date: %d - %0.2d\n",
                    ((card->cid_lo & 0xF) + 1997), (card->cid_lo & 0xF0) >> 4);
    }
}

/**
 * @brief sdc_ext_csd_display() display csd information
 */
void sdc_ext_csd_display(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;
    kmdw_printf("**************** Extended CSD register ****************\n");
    kmdw_printf("Ext-CSD:  Max Speed %d Hz.\n", card_info->max_dtr);
    // Below information is EMBEDDED_MMC
    kmdw_printf(" boot_info[228]:0x%x\n", card_info->ext_csd_mmc.boot_info);
    /* Some eMMC occupy byte 227 as second byte for BOOT_SIZE_MULTI */
    kmdw_printf(" boot_size_mult[226]:0x%x.\n", (card_info->ext_csd_mmc.reserved6 << 8 |
                card_info->ext_csd_mmc.boot_size_mult));
    kmdw_printf(" sec_count[215-212]: 0x%08x.\n", card_info->ext_csd_mmc.sec_count);
    kmdw_printf(" hs_timing[185]: 0x%x.\n", card_info->ext_csd_mmc.hs_timing);
    kmdw_printf(" bus_width[183]: 0x%x.\n", card_info->ext_csd_mmc.bus_width);
    kmdw_printf(" partition_conf[179]: 0x%x.\n", card_info->ext_csd_mmc.partition_conf);
    kmdw_printf(" boot_config_port[178]: 0x%x.\n", card_info->ext_csd_mmc.boot_config_prot);
    kmdw_printf(" boot_bus_width[177]: 0x%x.\n", card_info->ext_csd_mmc.boot_bus_width);
    kmdw_printf(" boot_bus_wp[173]: 0x%x.\n", card_info->ext_csd_mmc.boot_wp);
    kmdw_printf(" partition_setting_completed[155]: 0x%x.\n",
                card_info->ext_csd_mmc.partition_setting_completed);

    kmdw_printf("**********************************************\n");

    /* The block number of MMC card, which capacity is more than 2GB, shall be fetched from Ext-CSD. */
    if (card_info->ext_csd_mmc.ext_csd_rev >= 2) {
        kmdw_printf(" MMC Ext CSD version %d\n", card_info->ext_csd_mmc.ext_csd_rev);
        kmdw_printf(" Ext-CSD: block number %d\n", card_info->num_of_blks, card_info->max_dtr);
    } else {
        kmdw_printf(" MMC CSD Version 1.%d\n", card_info->csd_mmc.csd_structure);
        kmdw_printf(" CSD: block number %d\n", card_info->num_of_blks, card_info->max_dtr);
    }
    kmdw_printf("**********************************************\n");
}


/**
 * @brief sdc_sd_info_display() display sd card information
 */
void sdc_sd_info_display(kdrv_sdc_res_t *dev)
{
    volatile kdrv_sdc_sdcard_info_t *card_info = dev->card_info;

    kmdw_printf("**************** SD Information  ****************\n");
    kmdw_printf("* Bus width: %d.\n", card_info->bus_width);
    kmdw_printf("* Transfer speed: %s.\n", sdc_transfer_speed_to_str(card_info->speed));
    kmdw_printf("* Transfer type: %s.\n", sdc_transfer_type_to_str(card_info->flow_set.use_dma));
    kmdw_printf("* Auto Command: %d.\n", card_info->flow_set.auto_cmd);
    kmdw_printf("* Infinite Test Mode: %d.\n", dev->infinite_mode);
    kmdw_printf("* Abort: %s.\n\n", sdc_abort_type_to_str(card_info->flow_set.sync_abort));
}


/**
 * @brief dump_data, dump data information
 *
 * @param start_addr start address for data information
 * @param size size for data output
 * @return N/A
 */
void dump_data(uint32_t *pp, uint32_t start_addr, uint32_t size)
{
    uint32_t i=0;
    do {
        if((i%4)==0) {
            kmdw_printf("\n");
            kmdw_printf("[0x%08x]:",start_addr);
            start_addr+=16;
        }
        kmdw_printf(" 0x%08x", *(pp++));
        i++;
    } while((pp!=NULL)&&(i<size));
    kmdw_printf("\n");
}

/**
 * @brief sdc_rw_stress_test, sdc r/w stress test
 */
#define ABS(x, y)       (x > y) ? (x-y) : (y-x)
kdrv_status_t sdc_rw_stress_test(kdrv_sdc_dev_e dev_id, uint32_t test_size, uint32_t test_loop)
{
    int32_t i, j;
    clock_t read_time, write_time;
    uint32_t st_tick, diff;
    for (j = 0; j < test_loop; j ++) {
        memset(read_buf, 0, test_size);
        memset(write_buf, 0, test_size);

        for(i = 0; i < test_size; i++) {
            //*(write_buf + i) = i + start_idx;
            *(write_buf + i) = rand();
        }
       
        kdrv_timer_perf_get_instant(&perftimerid, &diff, &st_tick);
        kdrv_sdc_write(dev_id, write_buf, 0, test_size);
        kdrv_timer_perf_get_instant(&perftimerid, &diff, &st_tick);
       
        write_time = diff;

        kdrv_timer_perf_get_instant(&perftimerid, &diff, &st_tick);
        kdrv_sdc_read(dev_id, read_buf, 0, test_size);
        kdrv_timer_perf_get_instant(&perftimerid, &diff, &st_tick);
        read_time = diff;

        if (memcmp(read_buf, write_buf, test_size) != 0) {
            kmdw_printf("%d.KB SDC Read/Write compare FAIL.(%dB R:%d us, W:%d us)!\r\n",
                        j,test_size, read_time, write_time);
            return KDRV_STATUS_ERROR;
        } else {
            kmdw_printf("%d.SDC Read/Write compare PASS.(%dB R:%d us, W:%d us)!\r\n",
                        j,test_size, read_time, write_time);
        }
    }
    return KDRV_STATUS_OK;
}

/**
 * @brief sdc test main function, this test code include SDC0 and SDC1 verification.
 */
kdrv_status_t sdc_selftest(void)
{
    kdrv_status_t err;
    kdrv_sdc_dev_e dev_id; 
    /* test program for SDC0 & SDC1 */ 
    for (dev_id = KDRV_SDC0_DEV; dev_id <= KDRV_SDC1_DEV; dev_id++) {
        kmdw_printf("== SDC%d Start Verification ==\r\n", (uint8_t)dev_id);
        if ((err = kdrv_sdc_initialize(dev_id)) != KDRV_STATUS_OK) {
            kmdw_printf(" kdrv_sdc_initialzie fail and exits [%s] !\n", kdrv_status_msg_show(err));
            return KDRV_STATUS_ERROR                                                       ;
        }

        memset(read_buf, 0, TEST_LEN);
        memset(write_buf, 0, TEST_LEN);

        /* card scan */
        if ((err = kdrv_sdc_dev_scan(dev_id)) == KDRV_STATUS_OK) {
            #if DBG_CARD_INFO  // for card information display            
            kdrv_sdc_res_t *dev;
            dev = kdrv_sdc_get_dev(dev_id);
            sdc_ext_cid_display(dev);
            if (dev->card_info->card_type == MEMORY_CARD_TYPE_SD) {
                kmdw_printf("** Get SD Card **\n");
                sdc_scr_display(dev);
            } else if (dev->card_info->card_type == MEMORY_CARD_TYPE_MMC) {
                kmdw_printf("** Get eMMC/MMC Card **\n");
                sdc_ext_csd_display(dev);
            }
            #endif
        }
        if (sdc_rw_stress_test(dev_id, 1024, 5) == KDRV_STATUS_OK)
            kmdw_printf("** SDC eMMC/MMC card R/W test PASS **\n");
        else
            kmdw_printf("** SDC eMMC/MMC Card R/W test FAIL **\n");
        kdrv_sdc_uninitialize(dev_id);    
    }
    return KDRV_STATUS_OK;
}

