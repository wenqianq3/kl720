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
*  board_sn72096_11x11.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _BOARD_SN72096_11X11_H_
#define _BOARD_SN72096_11X11_H_

/******************************************************************************
Head Block of The File
******************************************************************************/
// Sec 0: Comment block of the file

// Sec 1: Include File

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define YES                             1
#define NO                              0

//System clock
//SCPU
#define SCPU_400                        0
//AXI_DDR
#define AXI_DDR_200                     0
#define AXI_DDR_333                     1
#define AXI_DDR_400                     2
#define AXI_DDR_466                     3
#define AXI_DDR_533                     4
//MRX1
#define MRX1_720                        0
//MRX0
#define MRX0_720                        0
#define MRX0_1320                       1
//NPU
#define NPU_200                         0
#define NPU_250                         1
#define NPU_300                         2
#define NPU_350                         3
#define NPU_400                         4
#define NPU_500                         5
#define NPU_600                         6
#define NPU_650                         7
#define NPU_700                         8
//DSP
#define DSP_200                         0
#define DSP_300                         1
#define DSP_400                         2
#define DSP_500                         3
//Audio
#define ADO_12p288                      0

#define IMG_SRC_IN_PORT_NONE            0
#define IMG_SRC_IN_PORT_MIPI            1
#define IMG_SRC_IN_PORT_DPI             2  //DVP port1
#define IMG_SRC_IN_PORT_UVC             3

#define SENSOR_ID_GC2145                0
#define SENSOR_ID_SC132GS               1
#define SENSOR_ID_HMX2056               2
#define SENSOR_ID_OV9286                3
#define SENSOR_ID_HMXRICA               4
#define SENSOR_ID_SC200AI               5
#define SENSOR_ID_IRS2877C              6
#define SENSOR_ID_IRS0224               7
#define SENSOR_ID_MAX                   8
#define SENSOR_ID_EXTERN                0xFE
#define SENSOR_ID_NONE                  0xFF

#define IMG_FORMAT_RGB565               0
#define IMG_FORMAT_RAW10                1
#define IMG_FORMAT_RAW8                 2
#define IMG_FORMAT_YCBCR                3

#define IMG_TYPE_RGB                    0
#define IMG_TYPE_IR                     1
#define IMG_TYPE_TOF                    2

#define SENSOR_RES_640_480              0
#define SENSOR_RES_480_640              1
#define SENSOR_RES_480_272              2
#define SENSOR_RES_272_480              3

#define IMG_MIPILANE_NUM_2              1
#define IMG_MIPILANE_NUM_1              2

#define SENSOR_COM_I2C                  0
#define SENSOR_COM_SPI                  1

#define SENSOR_I2CSPEED_100K            0
#define SENSOR_I2CSPEED_200K            1
#define SENSOR_I2CSPEED_400K            2
#define SENSOR_I2CSPEED_1000K           3

//CSIRX
#define CSIRX_VSTU_LINE                 0
#define CSIRX_VSTU_PIXEL                1

#define CSIRX_MCR_LSB                   0
#define CSIRX_MCR_MSB                   1


#define CSIRX_VSTR_IRS2877C             0x50    //0x1
#define CSIRX_VSTER_IRS2877C            0x0     //0x8
#define CSIRX_HSTR_IRS2877C             0x8     //0x8
#define CSIRX_PFTR_IRS2877C             0x15    //0x30
#define CSIRX_SETTLE_CNT_IRS2877C       0xA     //0xB//0x7

#define CSIRX_VSTR_GC2145               0x5
#define CSIRX_VSTER_GC2145              0x8
#define CSIRX_HSTR_GC2145               0x8
#define CSIRX_PFTR_GC2145               0x30
#define CSIRX_SETTLE_CNT_GC2145         0x7

#define CSIRX_VSTR_SC132GS              0x2
#define CSIRX_VSTER_SC132GS             0x9
#define CSIRX_HSTR_SC132GS              0x8
#define CSIRX_PFTR_SC132GS              0xFF
#define CSIRX_SETTLE_CNT_SC132GS        0xA

#define CSIRX_VSTR_SC200AI              0x5
#define CSIRX_VSTER_SC200AI             0x8
#define CSIRX_HSTR_SC200AI              0x8
#define CSIRX_PFTR_SC200AI              0x30
#define CSIRX_SETTLE_CNT_SC200AI        0x3

/* D2A Packet type register */
#define D2A_PT_YUV422                   0x1E
#define D2A_PT_RGB565                   0x22
#define D2A_PT_RGB888                   0x24
#define D2A_PT_RAW8                     0x2A
#define D2A_PT_RAW10                    0x2B
#define D2A_PT_RAW12                    0x2C
#define D2A_PT_RAW14                    0x2D
#define D2A_PT_RAW16                    0x2E

#define D2A_SRC_CSIRX                   0
#define D2A_SRC_EXT_DPI                 1

#define D2A_DA_LSB                      0
#define D2A_DA_MSB                      1

#define D2A_POLARITY_L                  0
#define D2A_POLARITY_H                  1

#define D2A_FT_IRS2877C                 0x100
#define D2A_FT_GC2145                   0x180
#define D2A_FT_SC132GS                  0x180
#define D2A_FT_EXTERN                   0x380

#define D2A_TILE_AVG_SIZE128            0
#define D2A_TILE_AVG_SIZE64             1
#define D2A_TILE_AVG_SIZE32             2

#define D2A_VSYNC_PL_IRS2877C           D2A_POLARITY_L
#define D2A_VSYNC_PL_GC2145             D2A_POLARITY_L
#define D2A_VSYNC_PL_SC132GS            D2A_POLARITY_L
#define D2A_VSYNC_PL_EXT                D2A_POLARITY_H

#define D2A_HSYNC_PL_IRS2877C           D2A_POLARITY_L
#define D2A_HSYNC_PL_GC2145             D2A_POLARITY_L
#define D2A_HSYNC_PL_SC132GS            D2A_POLARITY_L
#define D2A_HSYNC_PL_EXT                D2A_POLARITY_L
//Display
#define DISPLAY_DEVICE_LCDC             0
#define DISPLAY_DEVICE_LCM              1
#define DISPLAY_DEVICE_NONE             2

#define DISPLAY_PANEL_MZT_480X272       0
#define DISPLAY_PANEL_ST7789_240X320    1
#define DISPLAY_PANEL_NONE              2


#define CFG_AI_3D_LIVENESS_IN_NONE      0
#define CFG_AI_3D_LIVENESS_IN_SCPU      1
#define CFG_AI_3D_LIVENESS_IN_NCPU      2

//Protocal
#define COMM_SUPPORT_I2C                0
#define COMM_SUPPORT_SPI                1
#define COMM_SUPPORT_UART               2
#define COMM_SUPPORT_I2S                3

#define COMM_PORT_ID_0                  0
#define COMM_PORT_ID_1                  1
#define COMM_PORT_ID_2                  2
#define COMM_PORT_ID_3                  3

#define COMM_I2CSPEED_100K              0
#define COMM_I2CSPEED_200K              1
#define COMM_I2CSPEED_400K              2
#define COMM_I2CSPEED_1000K             3

#define COMM_I2CMODE_SLAVE              0
#define COMM_I2CMODE_MASTER             1

#define COMM_SPIMODE_MODE_0             0
#define COMM_SPIMODE_MODE_1             1
#define COMM_SPIMODE_MODE_2             2
#define COMM_SPIMODE_MODE_3             3

#define COMM_UART_BAUDRATE_1200         0
#define COMM_UART_BAUDRATE_2400         1
#define COMM_UART_BAUDRATE_4800         2
#define COMM_UART_BAUDRATE_9600         3
#define COMM_UART_BAUDRATE_14400        4
#define COMM_UART_BAUDRATE_19200        5
#define COMM_UART_BAUDRATE_38400        6
#define COMM_UART_BAUDRATE_57600        7
#define COMM_UART_BAUDRATE_115200       8
#define COMM_UART_BAUDRATE_460800       9
#define COMM_UART_BAUDRATE_921600       10
//flash
//flash manufacturer
#define FLASH_TYPE_NULL                 0X00
#define FLASH_TYPE_WINBOND_NOR          0X01
#define FLASH_TYPE_WINBOND_NAND         0X02
#define FLASH_TYPE_MXIC_NOR             0X11
#define FLASH_TYPE_MXIC_NAND            0X12
#define FLASH_TYPE_GIGADEVICE_NOR       0X21
#define FLASH_TYPE_GIGADEVICE_NAND      0X22

//flash SIZE
#define FLASH_SIZE_64MBIT               1   //8MBYTES
#define FLASH_SIZE_128MBIT              2   //16MBYTES
#define FLASH_SIZE_256MBIT              3   //32MBYTES
#define FLASH_SIZE_512MBIT              4   //64MBYTES
#define FLASH_SIZE_1GBIT                5   //128MBYTES
#define FLASH_SIZE_2GBIT                6   //256MBYTES
#define FLASH_SIZE_4GBIT                7   //512MBYTES

//speed(25MHZ/50MHZ/100MHZ)
#define FLASH_COMM_SPEED_25MHZ          1
#define FLASH_COMM_SPEED_50MHZ          2
#define FLASH_COMM_SPEED_100MHZ         3

//IO pin DRIVING STRENGTH
#define FLASH_DRV_NORMAL_MODE           1
#define FLASH_DRV_DUAL_IO_MODE          2
#define FLASH_DRV_DUAL_OUTPUT_MODE      3
#define FLASH_DRV_QUAD_IO_MODE          4
#define FLASH_DRV_QUAD_OUTPUT_MODE      5

//others
#define SUPPORT_ADC_DISABLE             0
#define SUPPORT_ADC_ENABLE              1
#define SUPPORT_DDR_DISABLE             0
#define SUPPORT_DDR_ENABLE              1
#define SUPPORT_GDMA_DISABLE            0
#define SUPPORT_GDMA_ENABLE             1
#define SUPPORT_GPIO_DISABLE            0
#define SUPPORT_GPIO_ENABLE             1
#define SUPPORT_PWM_DISABLE             0
#define SUPPORT_PWM_ENABLE              1
#define SUPPORT_SDC_DISABLE             0
#define SUPPORT_SDC_ENABLE              1
#define SUPPORT_TIMER_DISABLE           0
#define SUPPORT_TIMER_ENABLE            1
#define SUPPORT_USB_DISABLE             0
#define SUPPORT_USB_ENABLE              1
#define SUPPORT_WDT_DISABLE             0
#define SUPPORT_WDT_ENABLE              1
#define SUPPORT_UART_CONSOLE_DISABLE    0
#define SUPPORT_UART_CONSOLE_ENABLE     1
/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/




#endif //_BOARD_SN72096_11X11_H_

