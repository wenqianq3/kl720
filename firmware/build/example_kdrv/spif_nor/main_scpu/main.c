#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
#include "base.h"
#include "system_init.h"
#include "driver_init.h"
#include "project.h"

// driver headers
#include "kdrv_uart.h"

// middleware headers
#include "kmdw_console.h"
#include "kmdw_memory.h"
#include "kdev_flash.h"
#include "kdrv_spif.h"
#include "kdev_flash_winbond.h"
#include "kdrv_cmsis_core.h"

#define readl(addr)				(*(volatile unsigned int *)(addr))
#define writel(val, addr)		(*(volatile unsigned int *)(addr) = (val))
#define inw(port)				readl(port)
#define outw(port, val)			writel(val, port)
extern void SystemCoreClockUpdate(void);

#define INF_IMAGE_BUFFER_SIZE (1 * 1024 * 1024)
#define INF_RESULT_BUFFER_SIZE (4 * 1024)
#define QUEUE_NUM 10

#define DDR1_DST    DDR_HEAP_END
#define FLASH_SRC   0x00
#define LENGTH      (1024*1024*5)
#define DDR2_DST    (DDR1_DST+LENGTH)

#undef err_msg
#define err_msg kmdw_printf
#define _get_min(x,y) ( x < y ? x: y )

extern void kdev_flash_4Bytes_ctrl(uint8_t enable);
extern void kdev_flash_write_control(uint8_t  enable);
extern spi_flash_t flash_info;
extern kdev_status_t kdev_flash_read_flash_id(void);
extern bool kdev_flash_is_4byte_address(void);

int _kdp_memxfer_flash_to_ddr(uint32_t dst, uint32_t src, size_t bytes, uint8_t mode)
{
    int32_t total_lens;
    int32_t access_byte;
    uint32_t write_addr;
    uint32_t read_data;
    int32_t rx_fifo_depth;
    
    if ((bytes & 0x3) > 0) return -1;
    
    total_lens = bytes;
    write_addr = dst;
    rx_fifo_depth = (int32_t)kdrv_spif_rxfifo_depth();
    //read from flash
    //write to ddr

#if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    if (kdev_flash_is_4byte_address())
    {
        switch(mode)
        {
            case 0xEC:
                kdrv_spif_set_commands(src, SPI020_EC_CMD1, total_lens, SPI020_EC_CMD3);
            break;
            case 0x6C:
                kdrv_spif_set_commands(src, SPI020_6C_CMD1, total_lens, SPI020_6C_CMD3);
            break;
            case 0x13:
            default:
                kdrv_spif_set_commands(src, SPI020_13_CMD1, total_lens, SPI020_13_CMD3);
            break;
        }
    }
    else
#endif
    {
        switch(mode)
        {
            case 0xEC:
                kdrv_spif_set_commands(src, SPI020_EB_CMD1, total_lens, SPI020_EB_CMD3);
            break;
            case 0x6C:
                kdrv_spif_set_commands(src, SPI020_6B_CMD1, total_lens, SPI020_6B_CMD3);
            break;
            case 0x13:
            default:
                kdrv_spif_set_commands(src, SPI020_03_CMD1, total_lens, SPI020_03_CMD3);
            break;
        }
    }

    while (total_lens > 0)
    {        
        kdrv_spif_wait_rx_full();

        access_byte = _get_min(total_lens, rx_fifo_depth);
        total_lens -= access_byte;   

        while (access_byte > 0)
        {
            read_data = regSPIF_data->dw.kdrv_spif_dp;//u32Lib_LeRead32((unsigned char* )SPI020REG_DATAPORT);
            *(volatile unsigned int *)(write_addr) = read_data;//outw(write_addr, read_data);
            write_addr += 4;
            access_byte -= 4;
        }
    }    
#ifndef MIXING_MODE_OPEN_RENDERER
    kdrv_spif_wait_command_complete();/* wait for command complete */
#endif    

#if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
#endif    

    return 0;
}

int _kdp_memxfer_ddr_to_flash(uint32_t dst, uint32_t src, size_t bytes)
{
#define BLOCK_SIZE 256
#define SECTOR_ERASE_SIZE 4096
#define SECTOR64_ERASE_SIZE  0x10000

    int32_t total_lens;
    uint32_t erase_dst_addr;     
    uint32_t write_dst_addr;    
    int32_t write_len, write_len2;        
    int32_t access_byte;       
    uint32_t read_addr;
    uint32_t write_data;
    int32_t tx_fifo_depth;

    if ((dst & 0x00000FFF) > 0) return -1;
    if ((src & 0x3) > 0) return -1;
    if ((bytes & 0x3) > 0) return -1;

    //erase flash
    total_lens = bytes;    
    erase_dst_addr = dst;

    #if FLASH_4BYTES_CMD_EN
    //kdev_flash_4Bytes_ctrl(1);
    #endif

    while (total_lens > 0) 
    {
        if ((total_lens >= SECTOR64_ERASE_SIZE) && ((erase_dst_addr & 0x0FFFF) == 0)) {  // use 64KB erase if possible
            kdev_flash_64kErase(erase_dst_addr);
            total_lens -= SECTOR64_ERASE_SIZE;
            erase_dst_addr += SECTOR64_ERASE_SIZE;
        }
        else {  // use 4K erase
            kdev_flash_write_control(1);
            #if FLASH_4BYTES_CMD_EN
            kdev_flash_4Bytes_ctrl(1);
            kdrv_spif_set_commands(erase_dst_addr, SPI020_21_CMD1, SPI020_21_CMD2, SPI020_21_CMD3);
            #else
            kdrv_spif_set_commands(erase_dst_addr, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
            #endif

            kdrv_spif_check_quad_status_till_ready();

            #if FLASH_4BYTES_CMD_EN
            kdev_flash_4Bytes_ctrl(0);
            #endif
            total_lens -= SECTOR_ERASE_SIZE;
            erase_dst_addr += SECTOR_ERASE_SIZE;
        }
        if (total_lens <= 0)
            break;
    }
    
    total_lens = bytes;
    write_dst_addr = dst;
    read_addr = src;
    tx_fifo_depth = (int32_t)kdrv_spif_txfifo_depth();    

    while (total_lens > 0) {
        kdev_flash_write_control(1);
        
        write_len = _get_min(total_lens, BLOCK_SIZE);
        #if FLASH_4BYTES_CMD_EN
        kdev_flash_4Bytes_ctrl(1);        
        kdrv_spif_set_commands(write_dst_addr, SPI020_12_CMD1, write_len, SPI020_12_CMD3);
        #else
        kdrv_spif_set_commands(write_dst_addr, SPI020_02_CMD1, write_len, SPI020_02_CMD3);        
        #endif        
        
        write_dst_addr += write_len;
        write_len2 = write_len;
        
        while(write_len2 > 0)
        {
            kdrv_spif_wait_tx_empty();
            access_byte = _get_min(write_len2, tx_fifo_depth);
            write_len2 -= access_byte;
            while(access_byte > 0)
            {
                write_data = inw(read_addr);                
                regSPIF_data->dw.kdrv_spif_dp = write_data;
                read_addr += 4;
                access_byte -= 4;
            }
        }        
        
        kdrv_spif_check_quad_status_till_ready();
    #if FLASH_4BYTES_CMD_EN
        kdev_flash_4Bytes_ctrl(0);
    #endif
        total_lens -= write_len;
    }

//#if FLASH_4BYTES_CMD_EN
//    kdev_flash_4Bytes_ctrl(0);
//#endif        
//    
    return 0;
}

void mySPItest(void)
{
    uint32_t RDSR1=0; //05h
    uint32_t RDSR2=0; //35h
    uint32_t RDCR=0; //15h
    uint32_t reg;
    uint32_t m,k, err, Qmode = 0x13, block_erase=0;
    char s[48];
    char Ch;
	int mA;
    int flash_op_driving;
    uint8_t manufacturer_ID = 0;
    uint8_t buf[2];
    uint32_t size=0;
    //uint32_t st_tick, sp_tick;
    uint32_t block = 0;
    uint32_t block_addr = 0;
    
    //config to 25MHz and single mode
    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort = 1;//vLib_LeWrite32(SPI020REG_CONTROL, SPI020_ABORT);
    do
    {
        //if((u32Lib_LeRead32(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort == 0)
            break;
    }while(1);
 
    /* Set control register */
    reg = regSPIF_ctrl->st.dw.kdrv_spif_cr;//u32Lib_LeRead32(SPI020REG_CONTROL); // 0x80
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8; // System:200MHz, DIV_8:25MHz
    regSPIF_ctrl->st.dw.kdrv_spif_cr = reg;//vLib_LeWrite32(SPI020REG_CONTROL, reg);
    regSPIF_ctrl->st.bf.kdrv_spif_cr.XIP_port_sel = 0; /* make sure it's in Command slave port */

    /*16mA*/
	//mA = 0; //4mA
	//mA = 1; //8mA
	//mA = 2; //12mA
	mA = 3; //16mA
    for(uint32_t addr=0x30080200; addr<=0x30080214; addr+=4)
    {
        reg = inw(addr);                //SPI IO control
        reg &= ~0x00600000;             //clear bit21, bit22
        reg |= (mA<<21);        //select driving strength sun1023
        outw(addr, reg);
    }

    kdev_flash_read_flash_id();
    err_msg("read_spi_flash_id 0x%X OK!\n",flash_info.flash_id);
    err_msg("read_spi_flash_manufacture 0x%X OK!\n",flash_info.manufacturer);

    kdev_flash_4Bytes_ctrl(1);

    //full fill buffer by 0x99 to clear previous data
    /*int cnt = LENGTH / 2048;
    for(int i=0; i<cnt; i++)
    {
        memset((void*)(DDR1_DST+(i*2048)), i+0x80, 2048);
        //memset((void*)(DDR1_DST+LENGTH/2), 0x36, LENGTH/2);
        //read data by 25MHz and Quad mode as a golden sample
    }
    _kdp_memxfer_ddr_to_flash(FLASH_SRC,DDR1_DST,LENGTH);*/
    memset((void*)DDR1_DST, 0x55, LENGTH);
    _kdp_memxfer_flash_to_ddr(DDR1_DST, FLASH_SRC, LENGTH, Qmode);

//start testing    
/*******************************************************************************/

    regSPIF_ctrl->st.bf.kdrv_spif_cr.abort = 1;//vLib_LeWrite32(SPI020REG_CONTROL, SPI020_ABORT);
    do
    {
        //if((u32Lib_LeRead32(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
        if(regSPIF_ctrl->st.bf.kdrv_spif_cr.abort == 0)
            break;
    }while(1);

    /* Set control register */
    reg = regSPIF_ctrl->st.dw.kdrv_spif_cr;//u32Lib_LeRead32(SPI020REG_CONTROL); // 0x80
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);

    //input clock
    while(1)
    {
        err_msg("\nSPI speed (5)50Mhz, (2)25MHz, (1)100MHz: ");
        Ch = kmdw_console_getc();
        err_msg("%c", Ch);
        if(Ch == '5')
        {
            reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4; // System:200MHz, DIV_4:50MHz
            strcpy(s, " 50MHz_");
            break;
        }
        else if(Ch == '2')
        {
            reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8; // System:200MHz, DIV_8:25MHz
            strcpy(s, " 25MHz_");
            break;
        }
        else if(Ch == '1')
        {
            reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2; // System:200MHz, DIV_2:100MHz
            strcpy(s, "100MHz_");
            break;
        }
        
        err_msg("\r");
    }
    regSPIF_ctrl->st.dw.kdrv_spif_cr = reg;//vLib_LeWrite32(SPI020REG_CONTROL, reg);
    kdev_flash_4Bytes_ctrl(1);
    err_msg("\r\n");

    //input CLK driving
    while(1)
    {
        err_msg("CLK pin driving (0)4mA, (1)8mA, (2)12mA, (3)16mA: ");
        Ch = kmdw_console_getc();
        err_msg("%c", Ch);
        if((Ch == '0') || (Ch == '1') || (Ch == '2') || (Ch == '3'))
        {
            uint32_t addr=0x30080204;       //SPI_CLK IO control Register
            reg = inw(addr);
            reg &= ~0x00600000;             //clear bit21, bit22
            reg |= ((Ch - 0x30)<<21);        //select driving strength
            outw(addr, reg);

            switch(Ch)
            {
                case '0': strcpy(&s[7], "CLK04mA_");
                    break;
                case '1': strcpy(&s[7], "CLK08mA_");
                    break;
                case '2': strcpy(&s[7], "CLK12mA_");
                    break;
                case '3': strcpy(&s[7], "CLK16mA_");
                    break;
            }
            
            break;
        }
        err_msg("\r");
    }
    err_msg("\r\n");

    //input DAT driving
    while(1)
    {
        err_msg("DATA pin driving (0)4mA, (1)8mA, (2)12mA, (3)16mA: ");
        Ch = kmdw_console_getc();
        err_msg("%c", Ch);
        if((Ch == '0') || (Ch == '1') || (Ch == '2') || (Ch == '3'))
        {
            for(uint32_t addr=0x30080208; addr<=0x30080214; addr+=4)
            {
                reg = inw(addr);                //SPI IO control
                reg &= ~0x00600000;             //clear bit21, bit22
                reg |= ((Ch - 0x30)<<21);        //select driving strength
                outw(addr, reg);
            }

            switch(Ch)
            {
                case '0': strcpy(&s[15], "DAT04mA_");
                    break;
                case '1': strcpy(&s[15], "DAT08mA_");
                    break;
                case '2': strcpy(&s[15], "DAT12mA_");
                    break;
                case '3': strcpy(&s[15], "DAT16mA_");
                    break;
            }
            
            break;
        }
        err_msg("\r");
    }
    err_msg("\r\n");
retest:
    //input single/quad mode
    while(1)
    {
//sun1009       err_msg("(i)quad io mode, (o)quad output, (s)single mode: ");
        err_msg("(1)quad io mode, (2)quad output, (3)single mode, (4)single block erase: ");

        Ch = kmdw_console_getc();
        err_msg("%c", Ch);
        block_erase = 0;
        if(Ch == '1')
        {
            Qmode = 0xEC;
            strcpy(&s[23], "quadio");
            break;
        }
        else if(Ch == '2')
        {
            Qmode = 0x6C;
            strcpy(&s[23], "quad-o");
            break;
        }
        else if(Ch == '3')
        {
            Qmode = 0x13;
            strcpy(&s[23], "single");
            break;
        }
        else if(Ch == '4')
        {
            Qmode = 0x13;
            block_erase = 0xDC;
            strcpy(&s[23], "single");
            break;
        }
        err_msg("\r");
    }
    err_msg("\r\n");
    
    //Flash Output Driver Strength
    manufacturer_ID = flash_info.manufacturer;
    while(1)
    {
        size = flash_info.flash_size;
        if(manufacturer_ID==FLASH_MXIC_DEV)
        {
            err_msg("Flash Output Driver Strength(1)30 Ohms, (2)45 Ohms, (3)90 Ohms, (4)15 Ohms :");
        }
        else
        {
            err_msg("Flash Output Driver Strength(1)100%%, (2)75%%, (3)50%%, (4)25%% :");
        }

        Ch = kmdw_console_getc();
        err_msg("%c\n", Ch);
        flash_op_driving = Ch - '0';   
        //err_msg("%d\n", flash_op_driving);
        if( flash_op_driving > 0 && flash_op_driving <= 4 )
        {
            strcpy(&s[29], " flash_op_dr");
            s[41] = Ch;

            err_msg("kdev_flash_write_control done\n");
            err_msg("SPI020_05_CMD %4X %4X %4X %4X\n",SPI020_05_CMD0_w , SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w);
            kdrv_spif_set_commands( SPI020_05_CMD0_w , SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
            kdrv_spif_read_data( &RDSR1, 0x01 );
            err_msg("SPI020_05_CMD1 buf[0]=0x%2X\n", RDSR1 );
            kdrv_spif_wait_command_complete();

            kdrv_spif_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3 );
            kdrv_spif_read_data( &RDCR, 0x01 );
            err_msg("SPI020_15_CMD1 buf[1]=0x%2X\n", RDCR );
            kdrv_spif_wait_command_complete();
            
            kdev_flash_write_control(1);
            err_msg("manufacturer_ID 0x%2X\n",manufacturer_ID);
            switch(manufacturer_ID)
            {
                case FLASH_ZBIT_DEV:
                case FLASH_WB_DEV:
                case FLASH_GD_DEV:
                    if( (RDCR  & BIT2) )
                    {
                        err_msg("WPS is enabled!!\n", RDCR );
                    }
                    //enable volatile bit
                    kdrv_spif_set_commands(SPI020_50_CMD0, SPI020_50_CMD1, SPI020_50_CMD2, SPI020_50_CMD3);
                    kdrv_spif_wait_command_complete();
                    err_msg("SPI020_50_CMD0 done\n");
#if 1
                    err_msg("original driving strength (%d) \n", ((RDCR&0x60)>>5)+1 );
                    err_msg("RDCR (0x%4X) \n", RDCR );
                    RDCR  &= ~0x60;
                    err_msg("RDCR&~0x60 (0x%4X) \n", RDCR );
                    RDCR  |= (flash_op_driving-1) << 5;
                    err_msg("RDCR|flash_op_driving (0x%4X) \n", RDCR );
                    err_msg("New driving strength (%d) \n", ((RDCR&0x60)>>5)+1 );
                    kdrv_spif_set_commands(SPI020_11_CMD0, SPI020_11_CMD1, SPI020_11_CMD2, SPI020_11_CMD3 );
                    kdrv_spif_write_data((uint8_t*)(&RDCR), 1);
                    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
#else
                    err_msg("original driving strength (%d) \n", ((RDCR&0x60)>>5)+1 );
                    err_msg("RDCR (0x%4X) \n", RDCR );
                    RDCR  &= ~0x60;
                    err_msg("RDCR&~0x60 (0x%4X) \n", RDCR );
                    RDCR  |= (flash_op_driving-1) << 5;
                    err_msg("RDCR|flash_op_driving (0x%4X) \n", RDCR );
                    err_msg("New driving strength (%d) \n", ((RDCR&0x60)>>5)+1 );
                    kdrv_spif_set_commands(((uint8_t)RDCR), 0x01000001, 0, SPI020_11_CMD3 );
                    //kdrv_spif_write_data((uint8_t*)&RDCR, 1);
                    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
#endif
                    //Qmode
                    if(( Qmode == 0xEC )||( Qmode == 0x6C ))
                    {
                        err_msg("FLASH_WB/GD/ZBIT QE need to enable!! \n");
                        kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
                        kdrv_spif_read_data( &RDSR2, 0x01 );
                        err_msg("SPI020_35_CMD1 buf[1]=0x%2X\n", RDSR2 );
                        kdrv_spif_wait_command_complete();
                        
                        if(!(RDSR2 & BIT1))
                        {
                            RDSR2 |= BIT1;
                            kdrv_spif_set_commands(SPI020_31_CMD0, SPI020_31_CMD1, SPI020_31_CMD2, SPI020_31_CMD3 );
                            kdrv_spif_write_data((uint8_t*)(&RDSR2), 1);
                            kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
                            err_msg("FLASH_WB/GD/ZBIT Set QE OK!! \n");

                            err_msg("read FLASH_WB/GD/ZBIT status-2!! \n");
                            kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
                            kdrv_spif_read_data( &RDSR2, 0x01 );
                            err_msg("SPI020_35_CMD1 buf[1]=0x%2X\n", RDSR2 );
                            kdrv_spif_wait_command_complete();
                        }
                    }
                    break;
                case FLASH_MXIC_DEV:
                    kdev_flash_write_control(1);
                    err_msg("original driving strength (%d) \n", (RDCR&0x03)+1 );
                    RDCR  &= ~0x07;
                    if(size == 0x10000)
                    {
                        switch(flash_op_driving)
                        {
                            case 1:
                                RDCR  |= 7;
                                break;
                            case 2:
                                RDCR  |= 3;
                                break;
                            case 3:
                                RDCR  |= 1;
                                break;
                            case 4:
                                RDCR  |= 6;
                                break;
                        }
                    }
                    else
                    {
                        RDCR  |= (flash_op_driving-1);
                    }
                    //Qmode
                    if(( Qmode == 0xEC )||( Qmode == 0x6C ))
                    {
                        err_msg("FLASH_MXIC_DEV QE enabled \n");
                        RDSR1 |= BIT6; //QE enabled
                    }
                    else
                    {
                        err_msg("FLASH_MXIC_DEV QE disabled \n");
                        RDSR1 &= ~BIT6; //QE disabled
                    }
                    buf[0] = (uint8_t)RDSR1;
                    buf[1] = (uint8_t)RDCR;
                    err_msg("New driving strength (%d) \n", (RDCR&0x03)+1 );
                    err_msg("New driving strength (%4X %4X) \n", buf[0], buf[1] );
                    
                    kdrv_spif_set_commands(SPI020_01_CMD0, SPI020_01_CMD1, SPI020_01_CMD2, SPI020_01_CMD3 );
                    kdrv_spif_write_data(buf, 2);
                    err_msg("spi020_check_status_til_ready\n");
                    kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
                    break;
                default:
                    err_msg("CAN'T support driving strength change!!");
                    break;
            }
            kdrv_spif_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3 );
            kdrv_spif_read_data( &RDCR, 0x01 );
            err_msg("SPI020_15_CMD1 buf[0]=0x%2X\n", RDCR );
            kdrv_spif_wait_command_complete();

            break;
        }
    }
    s[42] = '\0';
    err_msg("%s\r\n",s);
    kdev_flash_write_control(0);
    //mytime.hour = 0;
    //mytime.min = 0;
    //mytime.sec = 0;
    //rtc_init(&mytime, NULL);
    err = 0;
    k=0;
    char buff[5];
    int while_count=100;
    if(block_erase)
        goto block_erase_test;
    
    while(1)
    {
        err_msg("How many time you would like to test? enter the number 0..10000(0 means endless) :");
        kmdw_console_echo_gets(buff, sizeof(buff));
        //err_msg("%s\n", buff);
        while_count = atoi(buff);
        //err_msg("%d\n", while_count);
        if((while_count>=0)&&(while_count<=10000))
            break;
        else
            err_msg("%d is out of range!\n", while_count);
    }
    while_count += 1;
    while(while_count)
    {
        memset((void*)DDR2_DST, 0x99, LENGTH);
        _kdp_memxfer_flash_to_ddr(DDR2_DST, FLASH_SRC, LENGTH, Qmode);
        for(m=0;m<LENGTH;m+=0x10)
        {
            //rtc_get_date_time(NULL, &mytime);
            if (0 != memcmp((const void*)(DDR2_DST+m), (const void*)(DDR1_DST+m), 0x10))
            {
                err_msg("0x%08X %08X %08X %08X %08X  base at 25MHz_single mode\n",
                        m,
                        inw(DDR1_DST+m+0x00),
                        inw(DDR1_DST+m+0x04),
                        inw(DDR1_DST+m+0x08),
                        inw(DDR1_DST+m+0x0C));
                
                err_msg("0x%08X %08X %08X %08X %08X  read at %s mode\n",
                        m,
                        inw(DDR2_DST+m+0x00),
                        inw(DDR2_DST+m+0x04),
                        inw(DDR2_DST+m+0x08),
                        inw(DDR2_DST+m+0x0C),
                        s);
                err++;
            }
            else
            {
                //err_msg("[%02d:%02d:%02d]%s...0x%08X\r", mytime.hour, mytime.min, mytime.sec, s, m);
            }
        }
        //err_msg("[%02d:%02d:%02d]%s...0x%08X  done %d err=%d\n", mytime.hour, mytime.min, mytime.sec, s, m, ++k, err);
        err_msg("done %d err=%d\n",++k, err);
        if(while_count>1)
        {
            while_count --;
            if(while_count == 1)
                break;
            //else
            //    kdrv_spif_wait_command_complete();/* wait for command complete */
        }
    }
    goto retest;
block_erase_test:
    while(1)
    {
        err_msg("\nWhich block do you would want to do block erase test? enter the number 0..507 :\n");
        kmdw_console_echo_gets(buff, sizeof(buff));
        //err_msg("%s\n", buff);
        while_count = atoi(buff);
        if((while_count<0)||(while_count>507))
        {
            err_msg("Block number is out of range, please enter again!!\n");
        }
        else
        {
            err_msg("Test Block %d ~ %d !\n", while_count,while_count+4);
            break;
        }
    }
    
    for(k=0; k<5; k++)
    {
        block = while_count+k;
        block_addr = block*0x10000;
        //st_tick = kdrv_current_t1_tick();
        err_msg("\nblock_erase_test is erasing block%d, address=0x%X....",block, block_addr);
        kdev_flash_64kErase(block_addr);
        //sp_tick = kdrv_current_t1_tick();
        //err_msg("\ntotal time for erasing block%d is: %d....", block, sp_tick - st_tick);
    }
    err_msg("\nblock_erase_test is done....\n");
//exit:
    err_msg("Press 'RESET' to leave or run the test again!!\n");
    kdev_flash_4Bytes_ctrl(0);
    
	//reset chip
	kdrv_spif_set_commands( SPI020_66_CMD0 , SPI020_66_CMD1, SPI020_66_CMD2, SPI020_66_CMD3 );
	kdrv_spif_wait_command_complete();
	kdrv_spif_set_commands( SPI020_99_CMD0 , SPI020_99_CMD1, SPI020_99_CMD2, SPI020_99_CMD3 );
	kdrv_spif_wait_command_complete();
}

int main(void)
{
    SystemCoreClockUpdate(); // System Initialization
    osKernelInitialize();    // Initialize CMSIS-RTOS
    sys_initialize();
    drv_initialize();

    kdrv_uart_initialize();
    kdrv_uart_console_init(MSG_PORT, MSG_PORT_BAUDRATE);           // uart console initial
    kmdw_ddr_init(DDR_HEAP_BEGIN, DDR_HEAP_END); // FIXME

    kmdw_printf("start SPI testing!!!\n");
    mySPItest();
    while (1)
    {
    }
}
