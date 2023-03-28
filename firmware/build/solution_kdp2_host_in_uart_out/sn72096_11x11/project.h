/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
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
*  project.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _PROJECT_H_
#define _PROJECT_H_

/*=============================================================================
asic setting
=============================================================================*/
#include "membase.h"

/*=============================================================================
clock setting
=============================================================================*/
#define SCPU_MHZ                                SCPU_400
#define AXI_DDR_MHZ                             AXI_DDR_533
#define MRX1_MHZ                                MRX1_720
#define MRX0_MHZ                                MRX0_720
#define NPU_MHZ                                 NPU_700
#define DSP_MHZ                                 DSP_500
#define AUDIO_MHZ                               ADO_12p288

/*=============================================================================
board setting
=============================================================================*/
#include "board_sn72096_11x11\board.h"
#define COMM_SUPPORT_0                          COMM_SUPPORT_I2C
#define COMM_PORT_0                             COMM_PORT_ID_0
#define COMM_I2CSPEED_0                         COMM_I2CSPEED_100K
#define COMM_SPIMODE_0                          COMM_SPIMODE_MODE_0
#define FLASH_TYPE                              FLASH_TYPE_WINBOND_NAND
#define FLASH_SIZE                              FLASH_SIZE_1GBIT
#define FLASH_COMM                              FLASH_COMM_SPEED_100MHZ
#define FLASH_DRV                               FLASH_DRV_QUAD_IO_MODE

#define SUPPORT_UART                            SUPPORT_UART_CONSOLE_DISABLE


/*=============================================================================
CAM setting
=============================================================================*/
#define YES                                     1
#define NO                                      0
#define IMGSRC_IN_0                             YES
#define IMGSRC_IN_1                             YES

#define IMGSRC_0_IN_PORT                        IMG_SRC_IN_PORT_MIPI
#define IMGSRC_0_SENSORID                       SENSOR_ID_GC2145
#define IMGSRC_0_DEV_ADDR                       0x3C
#define IMGSRC_0_PORT_ID                        0x2
#define IMGSRC_0_FORMAT                         IMG_FORMAT_RGB565
#define IMGSRC_0_TYPE                           IMG_TYPE_RGB
#define IMGSRC_0_RES                            SENSOR_RES_640_480
#define IMGSRC_0_WIDTH                          640
#define IMGSRC_0_HEIGHT                         480
#define IMGSRC_0_TILE_AVG                       0
#define IMGSRC_0_MIPI_LANE                      2
#define IMGSRC_0_SUPPORT_LED                    YES//NO//
#define IMGSRC_0_LED_STRENGTH                   8

#define IMGSRC_1_IN_PORT                        IMG_SRC_IN_PORT_MIPI
#define IMGSRC_1_SENSORID                       SENSOR_ID_SC132GS
#define IMGSRC_1_DEV_ADDR                       0x30
#define IMGSRC_1_PORT_ID                        0x2
#define IMGSRC_1_FORMAT                         IMG_FORMAT_RAW8
#define IMGSRC_1_TYPE                           IMG_TYPE_IR
#define IMGSRC_1_RES                            SENSOR_RES_480_640
#define IMGSRC_1_WIDTH                          480
#define IMGSRC_1_HEIGHT                         640
#define IMGSRC_1_TILE_AVG                       1
#define IMGSRC_1_MIPI_LANE                      2
#define IMGSRC_1_SUPPORT_LED                    YES//NO//
#define IMGSRC_1_LED_STRENGTH                   8

#define CAM_ID_MAX                              2
#define LED_DELAY_INTERVAL                      3
#define RGB_LED                                 GPIO_PIN_21
#define NIR_LED                                 GPIO_PIN_22
//#define MIPI_EXAMPLE
#define IMGSRC_NUM                              CAM_ID_MAX
#define MIPI_CAM_RGB                            0
#define MIPI_CAM_NIR                            1
#define MIPI_LANE_RGB                           2
#define MIPI_LANE_NIR                           2

#define SENSOR_RES_RGB                          SENSOR_RES_640_480
#define SENSOR_RES_NIR                          SENSOR_RES_480_640
#define IMGSRC_FORMAT_RGB                       IMG_FORMAT_RGB565
#define IMGSRC_FORMAT_NIR                       IMG_FORMAT_RAW8

#define CSIRX_0_TCN                             100
#define CSIRX_0_HRTV                            100
#define CSIRX_0_VSTU                            CSIRX_VSTU_PIXEL
#define CSIRX_0_MCR                             CSIRX_MCR_LSB
#define CSIRX_0_VSTR                            CSIRX_VSTR_GC2145
#define CSIRX_0_VSTER                           CSIRX_VSTER_GC2145
#define CSIRX_0_HSTR                            CSIRX_HSTR_GC2145
#define CSIRX_0_PFTR                            CSIRX_PFTR_GC2145
#define CSIRX_0_SETTLE_CNT                      CSIRX_SETTLE_CNT_GC2145

#define CSIRX_1_TCN                             100
#define CSIRX_1_HRTV                            100
#define CSIRX_1_VSTU                            CSIRX_VSTU_PIXEL
#define CSIRX_1_MCR                             CSIRX_MCR_LSB
#define CSIRX_1_VSTR                            CSIRX_VSTR_SC132GS
#define CSIRX_1_VSTER                           CSIRX_VSTER_SC132GS
#define CSIRX_1_HSTR                            CSIRX_HSTR_SC132GS
#define CSIRX_1_PFTR                            CSIRX_PFTR_SC132GS
#define CSIRX_1_SETTLE_CNT                      CSIRX_SETTLE_CNT_SC132GS

#define D2A_0_INPUT_SRC                         D2A_SRC_CSIRX
#define D2A_0_FIFO_THRE                         296//D2A_FT_GC2145
#define D2A_0_DROP_FRAME_NUM                    0
#define D2A_0_PACKET_TYPE                       D2A_PT_RGB565
#define D2A_0_DATA_ALIGN                        D2A_DA_LSB
#define D2A_0_VSYNC_PL                          D2A_VSYNC_PL_GC2145
#define D2A_0_HSYNC_PL                          D2A_HSYNC_PL_GC2145
#define D2A_0_TILE_AVG_EN                       IMGSRC_0_TILE_AVG
#define D2A_0_TILE_AVG_SIZE                     D2A_TILE_AVG_SIZE128

#define D2A_1_INPUT_SRC                         D2A_SRC_CSIRX
#define D2A_1_FIFO_THRE                         107//D2A_FT_SC132GS
#define D2A_1_DROP_FRAME_NUM                    0
#define D2A_1_PACKET_TYPE                       D2A_PT_RAW8
#define D2A_1_DATA_ALIGN                        D2A_DA_LSB
#define D2A_1_VSYNC_PL                          D2A_POLARITY_L
#define D2A_1_HSYNC_PL                          D2A_POLARITY_L
#define D2A_1_TILE_AVG_EN                       IMGSRC_1_TILE_AVG
#define D2A_1_TILE_AVG_SIZE                     D2A_TILE_AVG_SIZE128

/*=============================================================================
COMM setting
=============================================================================*/
#define UART_NUM                                1
#define MSG_PORT                                COMM_PORT_ID_0
#define MSG_PORT_BAUDRATE                       COMM_UART_BAUDRATE_115200

#define I2C_NUM                                 1
#define I2C_ATTR_ARRAY                          {\
                                                {COMM_PORT_ID_2,               COMM_I2CSPEED_100K,            0x30, COMM_I2CMODE_MASTER} };
/*=============================================================================
Pinmux setting
=============================================================================*/
#define PIN_NUM                                 92
#define KDRV_PIN_X_SPI_CS_N_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_CS_N
#define KDRV_PIN_X_SPI_CLK_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_CLK
#define KDRV_PIN_X_SPI_DO_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_DO
#define KDRV_PIN_X_SPI_DI_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_DI
#define KDRV_PIN_X_SPI_WP_N_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_WP_N
#define KDRV_PIN_X_SPI_HOLD_N_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SPI_HOLD_N
#define KDRV_PIN_X_I2C0_CLK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C0_CLK
#define KDRV_PIN_X_I2C0_DATA_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C0_DATA
#define KDRV_PIN_X_I2C1_CLK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C1_CLK
#define KDRV_PIN_X_I2C1_DATA_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C1_DATA
#define KDRV_PIN_X_I2C2_CLK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C2_CLK
#define KDRV_PIN_X_I2C2_DATA_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C2_DATA
#define KDRV_PIN_X_SSP0_CLK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SSP0_CLK
#define KDRV_PIN_X_SSP0_CS0_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SSP0_CS0
#define KDRV_PIN_X_SSP0_CS1_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SSP0_CS1
#define KDRV_PIN_X_SSP0_DI_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SSP0_DI
#define KDRV_PIN_X_SSP0_DO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SSP0_DO
#define KDRV_PIN_X_SSP1_CLK_REG                 PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DPI_DATAO[13]
#define KDRV_PIN_X_SSP1_CS_REG                  PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DPI_DATAO[14]
#define KDRV_PIN_X_SSP1_DI_REG                  PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DPI_DATAO[15]
#define KDRV_PIN_X_SSP1_DO_REG                  PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DPI_DATAO[16]
#define KDRV_PIN_X_SSP1_DCX_REG                 PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DPI_DATAO[17]
#define KDRV_PIN_X_JTAG_TRSTN_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TRST_N
#define KDRV_PIN_X_JTAG_TDI_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TDI
#define KDRV_PIN_X_JTAG_TMS_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TMS
#define KDRV_PIN_X_JTAG_TCK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TCK
#define KDRV_PIN_X_UART0_TX_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART0_TX
#define KDRV_PIN_X_UART0_RX_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART0_RX
#define KDRV_PIN_X_DSP_TRSTN_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:DSP_JTAG_TRST_N
#define KDRV_PIN_X_DSP_TDI_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:DSP_JTAG_TDI
#define KDRV_PIN_X_DSP_TDO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TDO
#define KDRV_PIN_X_DSP_TMS_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TMS
#define KDRV_PIN_X_DSP_TCK_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TCK
#define KDRV_PIN_X_TRACE_CLK_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_CLK
#define KDRV_PIN_X_TRACE_DATA0_REG              PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[0]
#define KDRV_PIN_X_TRACE_DATA1_REG              PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[1]
#define KDRV_PIN_X_TRACE_DATA2_REG              PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[2]
#define KDRV_PIN_X_TRACE_DATA3_REG              PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[3]
#define KDRV_PIN_X_UART1_RI_REG                 PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART1_RI
#define KDRV_PIN_X_SD1_D3_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_D3
#define KDRV_PIN_X_SD1_D2_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_D2
#define KDRV_PIN_X_SD1_D1_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_D1
#define KDRV_PIN_X_SD1_D0_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_D0
#define KDRV_PIN_X_SD1_CMD_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_CMD
#define KDRV_PIN_X_SD1_CLK_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD1_CLK
#define KDRV_PIN_X_SD0_D3_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_D3
#define KDRV_PIN_X_SD0_D2_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_D2
#define KDRV_PIN_X_SD0_D1_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_D1
#define KDRV_PIN_X_SD0_D0_REG                   PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_D0
#define KDRV_PIN_X_SD0_CMD_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_CMD
#define KDRV_PIN_X_SD0_CLK_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_CLK
#define KDRV_PIN_X_SD0_CARD_PWN_REG             PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_CARD_PWREN
#define KDRV_PIN_X_SD0_CARD_DET_REG             PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:SD0_CARD_DET
#define KDRV_PIN_X_JTAG_TDO_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TDO
#define KDRV_PIN_X_PWM0_REG                     PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:PWM0
#define KDRV_PIN_X_PWM1_REG                     PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400003   Pinmode:PWM1
#define KDRV_PIN_X_DPI_PCLKI_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_PCLKI
#define KDRV_PIN_X_DPI_VSI_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_VSI
#define KDRV_PIN_X_DPI_HSI_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_HSI
#define KDRV_PIN_X_DPI_DEI_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DEI
#define KDRV_PIN_X_DPI_DATAI0_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[0]
#define KDRV_PIN_X_DPI_DATAI1_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[1]
#define KDRV_PIN_X_DPI_DATAI2_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[2]
#define KDRV_PIN_X_DPI_DATAI3_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[3]
#define KDRV_PIN_X_DPI_DATAI4_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[4]
#define KDRV_PIN_X_DPI_DATAI5_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[5]
#define KDRV_PIN_X_DPI_DATAI6_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[6]
#define KDRV_PIN_X_DPI_DATAI7_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[7]
#define KDRV_PIN_X_DPI_DATAI8_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[8]
#define KDRV_PIN_X_DPI_DATAI9_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[9]
#define KDRV_PIN_X_DPI_DATAI10_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[10]
#define KDRV_PIN_X_DPI_DATAI11_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[11]
#define KDRV_PIN_X_DPI_DATAI12_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[12]
#define KDRV_PIN_X_DPI_DATAI13_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[13]
#define KDRV_PIN_X_DPI_DATAI14_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[14]
#define KDRV_PIN_X_DPI_DATAI15_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAI[15]
#define KDRV_PIN_X_DPI_PCLKO_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_PCLKO
#define KDRV_PIN_X_DPI_VSO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_VSO
#define KDRV_PIN_X_DPI_HSO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_HSO
#define KDRV_PIN_X_DPI_DEO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DEO
#define KDRV_PIN_X_DPI_DATAO0_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DSP_DATAO[0]
#define KDRV_PIN_X_DPI_DATAO1_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DSP_DATAO[1]
#define KDRV_PIN_X_DPI_DATAO2_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DSP_DATAO[2]
#define KDRV_PIN_X_DPI_DATAO3_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:DSP_DATAO[3]
#define KDRV_PIN_X_DPI_DATAO4_REG               PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:UART1_TX
#define KDRV_PIN_X_DPI_DATAO5_REG               PIN_MODE_1 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400001   Pinmode:UART1_RX
#define KDRV_PIN_X_DPI_DATAO6_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[6]
#define KDRV_PIN_X_DPI_DATAO7_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[7]
#define KDRV_PIN_X_DPI_DATAO8_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[8]
#define KDRV_PIN_X_DPI_DATAO9_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[9]
#define KDRV_PIN_X_DPI_DATAO10_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[10]
#define KDRV_PIN_X_PI_DATAO11_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[11]
#define PINMUX_ARRAY                            {KDRV_PIN_X_SPI_CS_N_REG,        KDRV_PIN_X_SPI_CLK_REG,         KDRV_PIN_X_SPI_DO_REG,          KDRV_PIN_X_SPI_DI_REG,          KDRV_PIN_X_SPI_WP_N_REG,\
                                                 KDRV_PIN_X_SPI_HOLD_N_REG,      KDRV_PIN_X_I2C0_CLK_REG,        KDRV_PIN_X_I2C0_DATA_REG,       KDRV_PIN_X_I2C1_CLK_REG,        KDRV_PIN_X_I2C1_DATA_REG,\
                                                 KDRV_PIN_X_I2C2_CLK_REG,        KDRV_PIN_X_I2C2_DATA_REG,       KDRV_PIN_X_SSP0_CLK_REG,        KDRV_PIN_X_SSP0_CS0_REG,        KDRV_PIN_X_SSP0_CS1_REG,\
                                                 KDRV_PIN_X_SSP0_DI_REG,         KDRV_PIN_X_SSP0_DO_REG,         KDRV_PIN_X_SSP1_CLK_REG,        KDRV_PIN_X_SSP1_CS_REG,         KDRV_PIN_X_SSP1_DI_REG,\
                                                 KDRV_PIN_X_SSP1_DO_REG,         KDRV_PIN_X_SSP1_DCX_REG,        KDRV_PIN_X_JTAG_TRSTN_REG,      KDRV_PIN_X_JTAG_TDI_REG,        KDRV_PIN_X_JTAG_TMS_REG,\
                                                 KDRV_PIN_X_JTAG_TCK_REG,        KDRV_PIN_X_UART0_TX_REG,        KDRV_PIN_X_UART0_RX_REG,        KDRV_PIN_X_DSP_TRSTN_REG,       KDRV_PIN_X_DSP_TDI_REG,\
                                                 KDRV_PIN_X_DSP_TDO_REG,         KDRV_PIN_X_DSP_TMS_REG,         KDRV_PIN_X_DSP_TCK_REG,         KDRV_PIN_X_TRACE_CLK_REG,       KDRV_PIN_X_TRACE_DATA0_REG,\
                                                 KDRV_PIN_X_TRACE_DATA1_REG,     KDRV_PIN_X_TRACE_DATA2_REG,     KDRV_PIN_X_TRACE_DATA3_REG,     KDRV_PIN_X_UART1_RI_REG,        KDRV_PIN_X_SD1_D3_REG,\
                                                 KDRV_PIN_X_SD1_D2_REG,          KDRV_PIN_X_SD1_D1_REG,          KDRV_PIN_X_SD1_D0_REG,          KDRV_PIN_X_SD1_CMD_REG,         KDRV_PIN_X_SD1_CLK_REG,\
                                                 KDRV_PIN_X_SD0_D3_REG,          KDRV_PIN_X_SD0_D2_REG,          KDRV_PIN_X_SD0_D1_REG,          KDRV_PIN_X_SD0_D0_REG,          KDRV_PIN_X_SD0_CMD_REG,\
                                                 KDRV_PIN_X_SD0_CLK_REG,         KDRV_PIN_X_SD0_CARD_PWN_REG,    KDRV_PIN_X_SD0_CARD_DET_REG,    KDRV_PIN_X_JTAG_TDO_REG,        KDRV_PIN_X_PWM0_REG,\
                                                 KDRV_PIN_X_PWM1_REG,            KDRV_PIN_X_DPI_PCLKI_REG,       KDRV_PIN_X_DPI_VSI_REG,         KDRV_PIN_X_DPI_HSI_REG,         KDRV_PIN_X_DPI_DEI_REG,\
                                                 KDRV_PIN_X_DPI_DATAI0_REG,      KDRV_PIN_X_DPI_DATAI1_REG,      KDRV_PIN_X_DPI_DATAI2_REG,      KDRV_PIN_X_DPI_DATAI3_REG,      KDRV_PIN_X_DPI_DATAI4_REG,\
                                                 KDRV_PIN_X_DPI_DATAI5_REG,      KDRV_PIN_X_DPI_DATAI6_REG,      KDRV_PIN_X_DPI_DATAI7_REG,      KDRV_PIN_X_DPI_DATAI8_REG,      KDRV_PIN_X_DPI_DATAI9_REG,\
                                                 KDRV_PIN_X_DPI_DATAI10_REG,     KDRV_PIN_X_DPI_DATAI11_REG,     KDRV_PIN_X_DPI_DATAI12_REG,     KDRV_PIN_X_DPI_DATAI13_REG,     KDRV_PIN_X_DPI_DATAI14_REG,\
                                                 KDRV_PIN_X_DPI_DATAI15_REG,     KDRV_PIN_X_DPI_PCLKO_REG,       KDRV_PIN_X_DPI_VSO_REG,         KDRV_PIN_X_DPI_HSO_REG,         KDRV_PIN_X_DPI_DEO_REG,\
                                                 KDRV_PIN_X_DPI_DATAO0_REG,      KDRV_PIN_X_DPI_DATAO1_REG,      KDRV_PIN_X_DPI_DATAO2_REG,      KDRV_PIN_X_DPI_DATAO3_REG,      KDRV_PIN_X_DPI_DATAO4_REG,\
                                                 KDRV_PIN_X_DPI_DATAO5_REG,      KDRV_PIN_X_DPI_DATAO6_REG,      KDRV_PIN_X_DPI_DATAO7_REG,      KDRV_PIN_X_DPI_DATAO8_REG,      KDRV_PIN_X_DPI_DATAO9_REG,\
                                                 KDRV_PIN_X_DPI_DATAO10_REG,     KDRV_PIN_X_PI_DATAO11_REG};


/*=============================================================================
GPIO setting
=============================================================================*/
#define GPIO_NUM                                5
#define GPIO_ATTR_ARRAY                         {\
                                                {GPIO_PIN_18,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_19,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_20,   GPIO_DIR_OUTPUT | GPIO_OUT_HIGH},\
                                                {GPIO_PIN_21,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_22,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW} };
                                                //GPIO_PIN_18,  TRACE0 NIR_PWDN Default Low
                                                //GPIO_PIN_19,  TRACE1 RGB_PWDN Default Low
                                                //GPIO_PIN_20,  TRACE1 RGB_PWDN Default High
/*=============================================================================
fw setting
=============================================================================*/
#define SYS_STACK_SIZE      1024*10
#define SYS_HEAP_SIZE       1024*24

//available memory size for allocation in RTX
#define OS_DYNAMIC_MEM_SIZE         (1024*32)

/*=============================================================================
DDR configuration
=============================================================================*/
#define KDP_DDR_BASE                    DDR_MEM_BASE

/**
 * @defgroup Definition of snapshot image address and size
 *
 * @{
 */
#define KDP_DDR_SNAPSHOT_RGB_IMG_SIZE       0x96000     /* 640x480x2(RGB565) */
#define KDP_DDR_SNAPSHOT_NIR_IMG_SIZE       0x4B000     /* 480x640x1(RAW8) */
#define KDP_DDR_SNAPSHOT_RGB_IMG_ADDR       0x82FFE000
#define KDP_DDR_SNAPSHOT_NIR_IMG_ADDR       0x6383B600


/*=============================================================================
Flash configuration
=============================================================================*/
/* Flash table */
#define FLASH_FW_SCPU0_ADDR                 0x00040000
#define FLASH_FW_NCPU0_ADDR                 0x00060000
#define FLASH_FW_CFG0_ADDR                  0x00260000
#define FLASH_FW_SCPU1_ADDR                 0x00280000
#define FLASH_FW_NCPU1_ADDR                 0x002A0000
#define FLASH_FW_CFG1_ADDR                  0x004A0000
#define FLASH_FID_MAP_ADDR                  0x004C0000
#define FLASH_MODEL_FW_INFO_ADDR            0x00540000
#define FLASH_MDDEL_ALL_ADDR                0x00560000
#define FLASH_MINI_BLOCK_SIZE               (128 * 1024)


/*=============================================================================
mdw setting
=============================================================================*/
/* scpu/ncpu image size */
#define SCPU_IMAGE_SIZE                     SiRAM_MEM_SIZE
#define NCPU_IMAGE_IRAM_SIZE                NiRAM_MEM_SIZE
#define NCPU_IMAGE_DRAM_SIZE                NdRAM_MEM_SIZE
#define NCPU_IMAGE_DDR_SIZE                 NCPU_FW_DDR_SIZE
#define NCPU_IMAGE_SIZE                     NCPU_FW_SIZE

#define DDR_SCATTER_SPACE_BASE              0x85E00000
#define DDR_SCATTER_SPACE_SIZE              0x00000000      //reserve ? MBytes for ARM scatter
                                                            //If you don't need any code/variables in DDR section,
                                                            //leave it as zero

#define DDR_HEAP_BEGIN                      0x88000000      // 128MB from base 0x80000000
#define DDR_HEAP_END                        (DDR_SCATTER_SPACE_BASE+DDR_SCATTER_SPACE_SIZE)

// Size limit of all_models.bin
#define DDR_MEM_MODEL_MAX_SIZE              ((DDR_HEAP_END - DDR_MEM_BASE) & ~0x000FFFFF)

/** Reserve for fw_info.bin */
// #define KDP_DDR_MODEL_INFO_TEMP             (DDR_HEAP_END - 0x1000) // 4KB

/*=============================================================================
app setting
=============================================================================*/



/*=============================================================================
power setting
=============================================================================*/
#define WKUP_SRC_RTC                        1
#define WKUP_SRC_EXT_BUT                    1
#define WKUP_SRC_USB_HIGH_SPEED             1
#define WKUP_SRC_USB_SUPER_SPEED            1
#define POWER_MODE_INIT                     ALL_AXI_ON

/*=============================================================================
Temperature setting
=============================================================================*/
#define TDC_DEGREE_CELSIUS_MONITOR          1		// If enabled, TDC driver would periodically print the current temperature every +-5 degree celsius via UART

#if ((defined TDC_DEGREE_CELSIUS_MONITOR) && (TDC_DEGREE_CELSIUS_MONITOR == 1))
#define TDC_SW_PROTECTION                   1		// Slow down the process on SCPU
#define TDC_HW_PROTECTION                   0		// Slow down the process on NCPU
#define TDC_DEGREE_CELSIUS_DANGEROUS        105	    // If the temperature is higher than this threshold, slow down the process via SW or HW solution
#define TDC_DEGREE_CELSIUS_SAFE             95	    // If the temperature is lower than this threshold, SCPU and NCPU keep in full function
#endif

/*=============================================================================
misc setting
=============================================================================*/









/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-->critical setting<--
Below setting is for RD tuning or testing.
**Don't touch anything if you don't know what you are doing**
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/




#endif //_PROJECT_H_
