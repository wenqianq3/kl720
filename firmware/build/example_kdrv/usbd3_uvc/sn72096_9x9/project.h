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
#include "board.h"
#define FLASH_TYPE                              FLASH_TYPE_WINBOND_NAND
#define FLASH_SIZE                              FLASH_SIZE_1GBIT
#define FLASH_COMM                              FLASH_COMM_SPEED_25MHZ
#define FLASH_DRV                               FLASH_DRV_NORMAL_MODE

#define SUPPORT_UART                            SUPPORT_UART_CONSOLE_ENABLE
#define SUPPORT_I2C                             SUPPORT_I2C_DISABLE
#define SUPPORT_SPI                             SUPPORT_SPI_DISABLE
#define SUPPORT_I2S                             SUPPORT_I2S_DISABLE
/*=============================================================================
COMM setting
=============================================================================*/
#if ((defined SUPPORT_UART) && (SUPPORT_UART == 1))
    #define UART_NUM                                1
    #define MSG_PORT                                COMM_UART_0
    #define MSG_PORT_BAUDRATE                       COMM_UART_BAUDRATE_115200
#endif

#if ((defined SUPPORT_I2C) && (SUPPORT_I2C == 1))
    #define I2C_NUM                                 1
    #define I2C_PORT                                COMM_I2C_0
    #define I2C_ATTR_ARRAY                          {\
                                                    {I2C_PORT,               COMM_I2C_SPEED_400K,            0x1B} };
#endif

#if ((defined SUPPORT_I2S) && (SUPPORT_I2S == 1))
    #define I2S_NUM                                 1
    #define I2S_PORT                                COMM_I2S_0
    #define I2S_ATTR_ARRAY                          {\
                                                    {I2S_PORT, COMM_I2S_MODE_MASTER_STEREO, COMM_I2S_MSB_FIRST, 16, 16, COMM_I2S_PAD_BACK, COMM_I2S_AUDIO_FREQ_32K, 0} };
    #define I2S_PORT_1                              COM_NOT_USED                              
#endif
                                                    
#if ((defined SUPPORT_SPI) && (SUPPORT_SPI == 1))
    #define SPI_NUM                                 0
    #define SPI_PORT                                COM_NOT_USED
    #define SPI_PORT_1                              COM_NOT_USED                                                
#endif
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
#define KDRV_PIN_X_I2C0_CLK_REG                 PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_0
#define KDRV_PIN_X_I2C0_DATA_REG                PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_1
#define KDRV_PIN_X_I2C1_CLK_REG                 PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_2
#define KDRV_PIN_X_I2C1_DATA_REG                PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_3
#define KDRV_PIN_X_I2C2_CLK_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C2_CLK
#define KDRV_PIN_X_I2C2_DATA_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:I2C2_DATA
#define KDRV_PIN_X_SSP0_CLK_REG                 PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:SSP0_CLK
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
#define KDRV_PIN_X_DSP_TRSTN_REG                PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_10
#define KDRV_PIN_X_DSP_TDI_REG                  PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_11
#define KDRV_PIN_X_DSP_TDO_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TDO
#define KDRV_PIN_X_DSP_TMS_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TMS
#define KDRV_PIN_X_DSP_TCK_REG                  PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DSP_JTAG_TCK
#define KDRV_PIN_X_UART0_TX_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART0_TX
#define KDRV_PIN_X_UART0_RX_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART0_RX
#define KDRV_PIN_X_TRACE_CLK_REG                PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_CLK
#define KDRV_PIN_X_TRACE_DATA0_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[0]
#define KDRV_PIN_X_TRACE_DATA1_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[1]
#define KDRV_PIN_X_TRACE_DATA2_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[2]
#define KDRV_PIN_X_TRACE_DATA3_REG              PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_TRACE_DATA[3]
#define KDRV_PIN_X_UART1_RI_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:UART1_RI
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
#define KDRV_PIN_X_SD0_CARD_PWN_REG             PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_23
#define KDRV_PIN_X_SD0_CARD_DET_REG             PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_24
#define KDRV_PIN_X_JTAG_TDO_REG                 PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:MCU_JTAG_TDO
#define KDRV_PIN_X_PWM0_REG                     PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_26
#define KDRV_PIN_X_PWM1_REG                     PIN_MODE_7 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400007   Pinmode:GPIO_PIN_27
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
#define KDRV_PIN_X_DPI_DATAO0_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[0]
#define KDRV_PIN_X_DPI_DATAO1_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[1]
#define KDRV_PIN_X_DPI_DATAO2_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[2]
#define KDRV_PIN_X_DPI_DATAO3_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[3]
#define KDRV_PIN_X_DPI_DATAO4_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[4]
#define KDRV_PIN_X_DPI_DATAO5_REG               PIN_MODE_0 | (PIN_PULL_NONE << 16) | (PIN_DRIVING_12MA << 21)           //0x00400000   Pinmode:DPI_DATAO[5]
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
                                                 KDRV_PIN_X_JTAG_TCK_REG,        KDRV_PIN_X_DSP_TRSTN_REG,       KDRV_PIN_X_DSP_TDI_REG,         KDRV_PIN_X_DSP_TDO_REG,         KDRV_PIN_X_DSP_TMS_REG,\
                                                 KDRV_PIN_X_DSP_TCK_REG,         KDRV_PIN_X_UART0_TX_REG,        KDRV_PIN_X_UART0_RX_REG,        KDRV_PIN_X_TRACE_CLK_REG,       KDRV_PIN_X_TRACE_DATA0_REG,\
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
#define GPIO_NUM                                10
#define GPIO_ST_OUT1                            GPIO_PIN_28
#define GPIO_ST_OUT2                            GPIO_PIN_29
#define GPIO_ST_IN1                             GPIO_PIN_30
#define GPIO_ST_IN2                             GPIO_PIN_31
#define GPIO_ATTR_ARRAY                         {\
                                                {GPIO_PIN_0,    GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_1,    GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_2,    GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_3,    GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_10,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_11,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_23,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_24,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_26,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW},\
                                                {GPIO_PIN_27,   GPIO_DIR_OUTPUT | GPIO_OUT_LOW} };

/*=============================================================================
fw setting
=============================================================================*/
#define SYS_STACK_SIZE      1024*4
#define SYS_HEAP_SIZE       1024*32

//available memory size for allocation in RTX
#define OS_DYNAMIC_MEM_SIZE         (1024*8)

/*=============================================================================
mdw setting
=============================================================================*/




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
misc setting
=============================================================================*/









/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-->critical setting<--
Below setting is for RD tuning or testing.
**Don't touch anything if you don't know what you are doing**
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#if ((defined SUPPORT_I2S) && (SUPPORT_I2S == 1) && (defined SUPPORT_SPI) && (SUPPORT_SPI == 1))
    #if ((I2S_PORT == SPI_PORT) && (I2S_PORT != COM_NOT_USED))
        #error "SPI AND I2S are mutually exclusive. Check and adjust the port assignment"
    #elif ((I2S_PORT == SPI_PORT_1) && (I2S_PORT != COM_NOT_USED))
        #error "SPI AND I2S are mutually exclusive. Check and adjust the port assignment"
    #elif ((I2S_PORT_1 == SPI_PORT) && (I2S_PORT_1 != COM_NOT_USED))
        #error "SPI AND I2S are mutually exclusive. Check and adjust the port assignment"
    #elif ((I2S_PORT_1 == SPI_PORT_1) && (I2S_PORT_1 != COM_NOT_USED))
        #error "SPI AND I2S are mutually exclusive. Check and adjust the port assignment"
    #endif
#endif

#endif //_PROJECT_H_
