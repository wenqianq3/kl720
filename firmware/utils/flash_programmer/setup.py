import os
import sys
import time
import serial
import pdb
import words
import struct
import logging
import json
import crcmod
#import usb.core
#import usb.util
#import usb.backend.libusb1

from optparse import OptionParser
import serial
import xmodem
import pyprind
from binascii import b2a_hex, a2b_hex

 


__DEBUG__ = 1


# USB Vendor ID / Product ID
VENDOR_ID = 0x3231
PRODUCT_ID = 0x0200
USB_MAX_BYTES = 8192
DATA_BLOCK_SIZE = 8192
ACK_PACKET_SIZE = 8
UART_MAX_BYTES = 4096

INTF_USB = 0
INTF_UART = 1
INTF_I2C = 2
INTF_SPI = 3

#################### PACKET LAYER ENABLE/DISABLE ###################
PACKET = 0    # 1-> enable packet layer, 0 -> disable packet layer
IMAGE_TRF = 0 # 0-> skip transfer, 1-> use USB, 2-> use UART
HIGH_BAUD = 1 # 1 -> HAPS or EVB, 0 -> ZC706 or Host (YG's App)

# COM Port ID


COM_ID = 5 # COM5
UART_BLOCK = 0x1000
act_intf = INTF_UART

if HIGH_BAUD == 1:
    BAUDRATE_HI = 921600 #921600
    BAUDRATE = 115200
else:
    BAUDRATE_HI = 115200
    BAUDRATE = 115200
# dut host interface (mozart)
dut_intf = INTF_I2C

ser = None

CMD_NONE = 0
CMD_MEM_READ = 1
CMD_MEM_WRITE= 2
CMD_DATA = 3
CMD_ACK = 4
CMD_STS_CLR = 5
CMD_MEM_CLR = 6
CMD_CHK_ERR = 7
CMD_TEST_ECHO = 8
CMD_USB_WRITE = 9

CMD_DEMO_SET_MODEL = 0x10
CMD_DEMO_SET_IMAGES = 0x11
CMD_DEMO_RUN_ONCE = 0x12
CMD_DEMO_RUN = 0x13
CMD_DEMO_STOP = 0x14
CMD_DEMO_RESULT_LEN = 0x15
CMD_DEMO_RESULT = 0x16
CMD_DEMO_FD = 0x17            # for development, remove later
CMD_DEMO_LM = 0x18            # for development 
CMD_DEMO_LD = 0x19            # for development
CMD_DEMO_FR = 0x1A            # for development

CMD_DEMO_INFERENCE = 100
CMD_DEMO_REG1 = 101
CMD_DEMO_REG2 = 102
CMD_DEMO_REG3 = 103
CMD_DEMO_REG4 = 104
CMD_DEMO_REG5 = 105
CMD_DEMO_ADDUSER = 106
CMD_DEMO_DELUSER = 107

CMD_SFID_START = 0x108      # SFID application
CMD_SFID_NEW_USER = 0x109
CMD_SFID_ADD_DB = 0x10A
CMD_SFID_DELETE_DB = 0x10B
CMD_SEND_IMAGE = 0x10C


CMD_FLASH_INFO = 0x1000
CMD_FLASH_CHIP_ERASE = 0x1001
CMD_FLASH_SECTOR_ERASE = 0x1002
CMD_FLASH_READ = 0x1003
CMD_FLASH_WRITE = 0x1004



# model type, for development of FD/FR, remove later
MODEL_TYPE_FD = 1
MODEL_TYPE_LM = 2
MODEL_TYPE_LD = 3
MODEL_TYPE_FR = 4
# remove later, for testing of inference/register
MODEL_TYPE_INFERENCE = 100
MODEL_TYPE_REG1 = 101
MODEL_TYPE_REG2 = 102
MODEL_TYPE_REG3 = 103
MODEL_TYPE_REG4 = 104
MODEL_TYPE_REG5 = 105
MODEL_TYPE_ADDUSER = 106
MODEL_TYPE_DELUSER = 107

# Messge HDR formart
MSG_HDR_FMT = "<HHIII" # Header(2b), checksum(2b), Cmd(4b), Addr(4b), Len(4b)
MSG_PKT_HDR_FMT = "<HHHHII" # Preamble(2b), Psize(2b), Cmd(2b), Csize(2b), Addr(4b), Len(4b)
PKTX_PAMB = 0xA583      # Packet TX Preamble
PKRX_PAMB = 0x8A35      # Packet RX Preamble
MSG_HDR_SIZE = 16       # 2+2+2+2+4+4
MSG_DAT_IDX = 16        # message body will be stored from offset 16

PKT_WR_FLAG = 0x8000
PKT_CRC_FLAG = 0x4000

# SET_MODEL and SET_IMAGES format
CMD_DEMO_SET_TWELVE_FMT = "<IIIIIIIIIIII"   # arg1(4b), arg2(4b), arg3(4b), arg4(4b), arg5(4b), arg6(4b), arg7(4b), arg8(4b), arg9(4b), arg10(4b), arg11(4b), arg12(4b)
CMD_DEMO_SET_EIGHT_FMT = "<IIIIIIII"   # arg1(4b), arg2(4b), arg3(4b), arg4(4b), arg5(4b), arg6(4b), arg7(4b), arg8(4b)
CMD_DEMO_SET_SIX_FMT = "<IIIIII"   # arg1(4b), arg2(4b), arg3(4b), arg4(4b), arg5(4b), arg6(4b)
CMD_DEMO_SET_TWO_FMT = "<II"   # arg1(4b), arg2(4b)

# APP Manager RETURN CODES
KDP_APP_OK = 0
KDP_APP_UNKNOWN_ERR = 1

KDP_APP_FDR_WRONG_USAGE = 2
KDP_APP_FDR_NO_FACE = 3
KDP_APP_FDR_BAD_POSE = 4
KDP_APP_FDR_NO_ALIVE = 5
KDP_APP_FDR_FR_FAIL = 6

KDP_APP_DB_NO_SPACE = 7
KDP_APP_DB_ALREADY_SAVED = 8
KDP_APP_DB_DEL_NOT_VALID = 9
KDP_APP_DB_NO_MATCH = 10
KDP_APP_DB_REG_FIRST =11
KDP_APP_DB_USER_NOT_REG = 12
KDP_APP_DB_DEL_FAIL = 13

# Host_com RETURN CODES
MSG_APP_BAD_IMAGE = 0x100
MSG_APP_BAD_INDEX = 0x101
MSG_APP_UID_EXIST = 0x102
MSG_APP_UID_DIFF = 0x103
MSG_APP_IDX_FIRST = 0x104
MSG_APP_IDX_MISSING = 0x105
MSG_APP_DB_NO_USER = 0x106
MSG_APP_DB_FAIL = 0x107

##########################################################################
# Device control
##########################################################################
def LOG(__LEVEL__, fmt):
    print(fmt)


##########################################################################
# Device control
##########################################################################
def dev_init():
    if act_intf == INTF_UART:
        ''' Uart interface '''
        global ser
        '''os.system("TASKKILL /F /IM ttermpro.exe")
        os.system("TASKKILL /F /IM putty.exe")
        time.sleep(1)
        p = ['COM%s' % (i + 1) for i in range(256)]
        rlt = []
        for port in p:
            try:
                s = serial.Serial(port)
                s.close()
                rlt.append(port)
            except (OSError, serial.SerialException):
                pass
            #else:
            #    try:
            #        u = serial.Serial(port)
            #        DBGPRINT('connect %s' %port)
            #        setup_intf('uart', u, 3, BAUDRATE, timeout=5)
            #        u.flushInput()
            #        u.flushOutput()
            #    except:
            #        DBGPRINT('Uart port open fail')
        print(rlt)'''
        ser = serial.Serial()
        try:
            setup_intf('uart', ser, COM_ID, BAUDRATE, timeout=5)
            ser.flushInput()
            ser.flushOutput()
        except:
            DBGPRINT('Uart port open fail')
            sys.exit(-1)

    ''' USB interface '''
    global kneron_dev
    global ep_out
    global ep_in

    if(IMAGE_TRF == 1):
        backend = usb.backend.libusb1.get_backend(find_library=lambda x: "C:\Windows\System32\libusb0.dll")
        kneron_dev = usb.core.find(idVendor=VENDOR_ID, idProduct=PRODUCT_ID, backend=backend)
        if kneron_dev is None:
            sys.exit(-1)
        #print('kneron usb is open...')
        kneron_dev.set_configuration()
        cfg = kneron_dev.get_active_configuration()
        #print(cfg)
        interface = cfg[(0,0)]
        #print(interface)
        interface_number = cfg[(0, 0)].bInterfaceNumber
        #print(interface_number)
        alternate_setting = usb.control.get_interface(kneron_dev, interface_number)
        #print(alternate_setting)  
        usb_interface = usb.util.find_descriptor(cfg, bInterfaceNumber=interface_number, bAlternateSetting=alternate_setting)
        #print(usb_interface)
        ep_out = usb.util.find_descriptor(usb_interface, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
        if ep_out is None:
            sys.exit(-1)
        #print(ep_out) 
        ep_in = usb.util.find_descriptor(usb_interface, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
        if ep_in is None:
            sys.exit(-1)

def dev_close():
    if act_intf == INTF_UART:
        global ser
        intf_close()
    else:
        ''' USB interface '''
        #TODO: Wait for mozart usb

def dev_flush():
    global ser
    ser.flushInput()
    ser.flushOutput()

##########################################################################
# Setpu active interface
##########################################################################
def setup_intf(interface, dev, com = 0, baudrate = 115200, timeout=5.0):
    global intf_read
    global intf_write
    global intf_open
    global intf_close
    act_intf = words.__WORDS__[interface]
    if interface == 'usb':
        intf_read = usb_read
        intf_write = usb_write
        intf_open = usb_open
        intf_clsoe = usb_close
    elif interface == 'uart':
        intf_read = uart_read
        intf_write = uart_write
        intf_open = uart_open
        intf_close = uart_close
        uart_open(dev, com, baudrate, timeout)
    else:
        print("Error device interface")

##########################################################################
# Uart interface
##########################################################################
def uart_open(ser, com, baudrate, timeout):
    if os.name == 'nt':
        ser.port = 'COM' + str(com)
    else:
        ser.port = '/dev/ttyUSB' + str(com)
    ser.timeout = timeout
    ser.baudrate = baudrate
    ser.open()

def uart_write(buf, size):
#    print('->uart write: %d' %size)
#    mem_dump(buf)
    global ser
    ser.write(buf)

def uart_read(size):
    global ser
#    print('->uart read: %d' %size)
    try:
        buf = bytearray(ser.read(size))
    except:
        print('Uart read fail')
#    mem_dump(buf)
    if (len(buf) != size):
        print('Uart read fail, expected=%x, read=%x' %(size, len(buf)))
    return buf

def uart_close():
    global ser
    ser.close()


##########################################################################
# USB interface
##########################################################################
def check_generic_ack(buf):
    preamble = buf[0] + buf[1] << 8
    pkt_size = buf[2] + buf[3] << 8
    cmd = buf[4] + buf[5] >> 8
    size = buf[6] + buf[7] >> 8
    if (preamble != PKRX_PAMB):
        return False
    if (pkt_size & 0x0FFF != 0):
        return False
    if (size != 0):  # must be zero msg payload
        return False
    if (cmd == CMD_ACK):
        return True
    else:
        return False

def RecDataACKFromUSB():
#  print("RecDataACKFromUSB")
    ackusb = bytes(b"none")
    if (ep_in is None):
        print("ep_in is not open")
        return
    temp = ep_in.read(ACK_PACKET_SIZE, timeout=1000)
    ackusb = bytes(temp)
#    print(ackusb)
    while ((ackusb[0] != 0x35) or (ackusb[1] != 0x8A)): # look for preamble
        temp =  ep_in.read(ACK_PACKET_SIZE, timeout=1000)
        print("ACK Retry", temp)
        ackusb = bytes(temp)
#    print(ackusb[0:4])
#    print(len(ackusb))
#    print(".......")
    return 0 

def usb_write_data(buf, length, offset):
    if(ep_out is None):
        print("ep_out is not open")
        return
    num = (length - offset) // USB_MAX_BYTES
    leftover = (length - offset) % USB_MAX_BYTES

    for x in range(num):
#    print(buf[offset:offset+USB_MAX_BYTES])
        RecDataACKFromUSB() # wait for receiver ready
        ep_out.write(buf[offset:offset+USB_MAX_BYTES])
        offset += USB_MAX_BYTES
#        print("+", end='', flush=True)
    if (leftover != 0):
#        print(buf[offset:])
        RecDataACKFromUSB()  # wait for acknowledge first
        ep_out.write(buf[offset:])
    return

##########################################################################
# UART interface Raw Data Transfer
##########################################################################
def uart_write_data(buf, length, offset):
    num = (length - offset) // UART_MAX_BYTES
    leftover = (length - offset) % UART_MAX_BYTES

    for x in range(num):
        RecDataACKFromUART() # wait for receiver ready
        bdata = bytearray(buf[offset:offset+UART_MAX_BYTES])
        intf_write(bdata, len(bdata))
        offset += UART_MAX_BYTES

    if (leftover != 0):
        RecDataACKFromUART()  # wait for acknowledge first
        bdata = bytearray(buf[offset:offset+leftover])
        intf_write(bdata, len(bdata))
    return

def RecDataACKFromUART():
    ackuart = bytes(b"none")
    temp = intf_read(ACK_PACKET_SIZE)
    ackuart = bytes(temp)
#    print(ackuart)
    # look for the Data ACK
    while ((ackuart[0] != 0x35) or (ackuart[1] != 0x8A) or (ackuart[2] != 4)):
        intf_read(ACK_PACKET_SIZE)
        print("ACK Retry", temp)
        ackuart = bytes(temp)
    return

##########################################################################
# AND / OR
##########################################################################
def bit_set():
    ''' '''

def bit_clear():
    ''' '''


##########################################################################
# Register read / write
##########################################################################
def reg_read(addr):
    buf = mem_block_read(addr, 4)
    val = (buf[3] << 24) +  (buf[2] << 16) + (buf[1] << 8) + buf[0]
    return val

def reg_write(addr, value):
    buf = word2blist(value)
    mem_block_write(addr, buf, len(buf))

def word2blist(value):
    buf = []
    buf.append((value >> 0) & 0xFF)
    buf.append((value >> 8) & 0xFF)
    buf.append((value >> 16) & 0xFF)
    buf.append((value >> 24) & 0xFF)
    return buf


def wlist2blist(wlist):
    buf = []
    for value in wlist:
        buf.append((value >> 0) & 0xFF)
        buf.append((value >> 8) & 0xFF)
        buf.append((value >> 16) & 0xFF)
        buf.append((value >> 24) & 0xFF)
    return buf

def blist2wlist(blist):
    buf = []
    for i in range(len(blist) // 4):
        #value = (blist[i * 4 + 0] << 24) + (blist[i * 4 + 1] << 16) + (blist[i * 4 + 2] << 8) + (blist[i * 4 + 3])
        value = (blist[i * 4 + 0] ) + (blist[i * 4 + 1] << 8) + (blist[i * 4 + 2] << 16) + (blist[i * 4 + 3] << 24)
        buf.append(value)
    return buf


##########################################################################
# Flash READ/WRITE/COMPARE
##########################################################################
def flash_read(addr, size):
    return(mem_read(addr, size, CMD_FLASH_READ, True))

def flash_write(addr , buf, size, buf_max = UART_BLOCK):
    mem_write(addr, buf, size, CMD_FLASH_WRITE, buf_max, True)
    
def flash_chip_erase():
    msg_cmd_general(CMD_FLASH_CHIP_ERASE, 0.2)

def flash_sector_erase(addr):
    data = pack_data(CMD_FLASH_SECTOR_ERASE, addr, 1, None)
    intf_write(data, MSG_HDR_SIZE)
    time.sleep(0.1)
    if not msg_wait_ack():
	    print('Flase erase: wait ack fail')    

def showdebuginfo():
    msg_cmd_general(CMD_FLASH_INFO, 0.2)

def flash_sector_erase_64K(addr):
    data = pack_data(CMD_FLASH_SECTOR_ERASE_64K, addr, 1, None)
    intf_write(data, MSG_HDR_SIZE)
    time.sleep(0.1)
    if not msg_wait_ack():
	    print('Flase erase: wait ack fail')

##########################################################################
# USB MEMORY WRITE
##########################################################################
def usb_mem_write(addr, buf, size):
    print('---------------- USB -----------------')
    print('Start ADDR=0x%X usb memory Write' %addr)
    data = pack_data(CMD_USB_WRITE, addr, size, None, 1) #use crc
    intf_write(data, len(data))
    #### start usb upload #####
    usb_write_data(buf, size, 0) # start at the beginning of file
    # now get the response
    if (msg_wait_ack(CMD_USB_WRITE) != True):
        print('USB Write, wait ack fail')

    return ''

##########################################################################
# MEMORY READ/WRITE/COMPARE
##########################################################################
def mem_write(addr , buf, size, cmd = CMD_MEM_WRITE, buf_max = UART_BLOCK, dbg = False):
    #print('--------------------------------------')
    #print('Start ADDR=0x%X memory R/W test' %addr)
    wloop = 0
    percent = 0
    unit = 0
    total_range = 100
    #print('#################buf_max %d'%(buf_max))
    if (size % buf_max == 0):
        wloop = (size // buf_max)
    else:
        wloop = (size // buf_max) + 1
    
    if(wloop < total_range):
        total_range = wloop - 1
    if(total_range == 0):
        total_range = 1
    #print('%d = (%d // %d)' %((wloop // total_range), wloop, total_range))
    if (wloop % total_range == 0):
        unit = (wloop // total_range) - 1
    else:
        unit = (wloop // total_range)
    if(unit == 0):
        unit = 1
    for i in range(wloop):
        if i == wloop - 1:
            wbuf = buf[i * buf_max : size]
        else:
            wbuf = buf[i * buf_max : (i + 1) * buf_max]
        mem_block_write(addr + buf_max * i, wbuf, len(wbuf), cmd)
        #if dbg:
            #print('Write status [%d/%d]'%(i, wloop))
        if( percent != (i // unit) ):
            percent = i // unit
            if( percent > total_range):
                percent = total_range
            #print("\r" + "In progress",percent,"%",end = "",flush=True)
            if(total_range == 100):
                print("\r" + '[%s%s] %d%%' % ('*'*percent,'-'*(total_range-percent), percent),end = "",flush=True)
            else:
                print("\r" + '[%s%s] %d' % ('*'*percent,'-'*(total_range-percent), percent) + "/" + '%d' %total_range, end = "",flush=True)
    print("")


def mem_read(addr, size, cmd = CMD_MEM_READ, dbg = False):
    buf_max = UART_BLOCK
    rloop = 0
    rlen = 0
    percent = 0
    unit = 0
    total_range = 100
    buf = []
    #print('#################buf_max %d'%(buf_max))
    if (size % buf_max == 0):
        rloop = (size // buf_max)
    else:
        rloop = (size // buf_max) + 1

    if(rloop < total_range):
        total_range = rloop - 1
    if(total_range == 0):
        total_range = 1
    #print('%d = (%d // %d)' %((rloop // total_range), rloop, total_range))
    if (rloop % total_range == 0):
        unit = (rloop // total_range) - 1
    else:
        unit = (rloop // total_range)
    if(unit == 0):
        unit = 1
    for i in range(rloop):
        if i == rloop - 1:
            rlen = size - i * buf_max
        else:
            rlen = buf_max
        sbuf = mem_block_read(addr + buf_max * i, rlen, cmd)
        buf.extend(sbuf)
        #if dbg:
        #    print('Read status [%d/%d]'%(i, rloop))
        if( percent != (i // unit) ):
            percent = i // unit
            if( percent > total_range):
                percent = total_range
            #print("\r" + "In progress",percent,"%",end = "",flush=True)
            if(total_range == 100):
                print("\r" + '[%s%s] %d%%' % ('*'*percent,'-'*(total_range-percent), percent),end = "",flush=True)
            else:
                print("\r" + '[%s%s] %d' % ('*'*percent,'-'*(total_range-percent), percent) + "/" + '%d' %total_range, end = "",flush=True)
    print("")
    return buf



def mem_dump(buf, displayLen = 0):
    ''' Memory dump '''
    if buf == None or len(buf) == 0:
        return
    pLen = len(buf)
    if(pLen > 1):
        print('Lenght=%d' %pLen)
    #Display index address
    aStr = "====="
    for i in range(16):
        if displayLen == 0:
            aStr += ' ' +('%02X' %i)
        else:
            aStr += ' ' +('%03X' %i)
    print(aStr)
    #Display data
    aStr = ""
    if(pLen % 16):
        loop = (pLen // 16)+1
    else:
        loop = pLen // 16;
    for i in range(loop):
        aStr = ('%04X:' %(i * 16))
        for j in range(16):
            if((i*16+j) < pLen):
                if displayLen==0:
                    aStr += ' ' +('%02X' %buf[i*16+j])
                else:
                    aStr += ' ' +('%03X' %buf[i*16+j])
        print(aStr)


def mem_block_read(addr, size, cmd = CMD_MEM_READ):
    assert size <= UART_BLOCK, 'block buffer size must less than UART_BLOCK(default:0x100)'
    data = pack_data(cmd, addr, size, None)
    intf_write(data, MSG_HDR_SIZE)
#    time.sleep(0.1)
    buf = intf_read(size + MSG_HDR_SIZE)
    if (len(buf) != (size + MSG_HDR_SIZE)):
        print("****** Not receiving enough bytes ******")
    else:        
        hdr, chk, cal_chk, cmd, addr, size, data = unpack_data (buf)
        if (chk != cal_chk):
            print('Error: intf data read fail. CRC error!')
        return list(data)
    return ''


def mem_block_write(addr, buf, size, cmd = CMD_MEM_WRITE):
    assert size <= UART_BLOCK, 'block buffer size must less than UART_BLOCK(def:0x100)'
    cnt = 1
    RETRY_MAX = 3
    #print('#######mem_block_write##########addr %d size %d'%(addr, size))
    while(cnt <= RETRY_MAX):
        bdata = bytearray(buf)
        data = pack_data(cmd, addr, size, bytearray(buf))
        intf_write(data, len(data))
#        time.sleep(0.1)
        if msg_wait_ack():
            break
        else:
            print('Write, wait ack fail, retry %d/%d' % (cnt, RETRY_MAX))
            cnt = cnt + 1
        time.sleep(0.5)

def mem_clr(addr = 0x60000000, len = 0):
    ''' DDR/SRAM/NPUSRAM memory clear '''
    data = pack_data(CMD_MEM_CLR, addr, len, None)
    intf_write(data, MSG_HDR_SIZE)
    time.sleep(1)
    cnt = 1
    RETRY_MAX = 3
    while(cnt <= RETRY_MAX):
        if msg_wait_ack():
            return
        else:
            print('ERR: Memory clear ack fail, retry %d/%d' % (cnt, RETRY_MAX))
            cnt = cnt + 1
            time.sleep(0.5)

def set_model(input_addr, input_len, output_addr, output_len, buf_addr, buf_len, cmd_addr, cmd_len, weight_addr, weight_len, setup_addr, setup_len):
    buf = bytearray(struct.pack(CMD_DEMO_SET_TWELVE_FMT, input_addr, input_len, output_addr, output_len, buf_addr, buf_len, cmd_addr, cmd_len, weight_addr, weight_len, setup_addr, setup_len))
    data = pack_data(CMD_DEMO_SET_MODEL, setup_addr, 48, bytearray(buf))
    intf_write(data, len(data))
    time.sleep(0.1)
    if not msg_wait_ack():
	    print('set_model: wait ack fail')

def set_image(image_addr, image_len, col, row, ch, format):
    buf = bytearray(struct.pack(CMD_DEMO_SET_SIX_FMT, image_addr, image_len, col, row, ch, format))
    data = pack_data(CMD_DEMO_SET_IMAGES, 0, 24, bytearray(buf))
    intf_write(data, len(data))
    time.sleep(0.1)
    if not msg_wait_ack():
	    print('set_images: wait ack fail')
		
def demo_run_once():
    data = pack_data(CMD_DEMO_RUN_ONCE, 0, 0, None)
    intf_write(data, len(data))
    time.sleep(0.1)
    if not msg_wait_ack():
	    print('set_images: wait ack fail')

def mem_wrc():
    ''' Memory write, read and compare '''


def pack_data(cmd, addr, size, buf):
    crc_flag = 1 # always use crc
    if (buf):
        mSize = len(buf) + 8   # there is always address & size
    else:
        mSize = 8
#    print("packet_data: address is %x" %(addr))
    pSize = mSize + 4  # add MSG header itself
    pSize = pSize | PKT_WR_FLAG # we are the host
    # if CRC is used, append it at the end, so increase pSize by 4
    if (crc_flag):
        pSize = pSize + 4
        pSize = pSize | PKT_CRC_FLAG
    if (PACKET):
        nbuf = bytearray(struct.pack(MSG_PKT_HDR_FMT, PKTX_PAMB, pSize, cmd, mSize, addr, size))
    else:
        nbuf = bytearray(struct.pack(MSG_HDR_FMT, PKTX_PAMB, 0x00, cmd, addr, size))
    if (buf != None):
        nbuf = nbuf + buf
#    print("address sent -> %02x %02x %02x %02x" %(nbuf[11], nbuf[10], nbuf[9], nbuf[8]))
    if (PACKET):
        if (crc_flag):
            crc16 =  gen_crc16(nbuf[0:len(nbuf)], len(nbuf))
#            print('crc is %x' %(crc16))
#            print(bytes([crc16 & 0xFF]), bytes([crc16 >> 8]))
            nbuf = nbuf + bytes([crc16 & 0xFF]) + bytes([crc16 >> 8]) + bytes([0]) + bytes([0])
    else:
        crc16 =  gen_crc16(nbuf[4:len(nbuf)], len(nbuf) - 4)
        #print('get CRC16 %X' %(crc16))
        nbuf[2] = crc16 & 0xFF
        nbuf[3] = crc16 >> 8
    return nbuf

def unpack_data(buf):
    #assert len(buf) < 16, "Packet header error"
    chk = 0
    cal_chk = 0
    hdrmsg = buf[0:16]
    if (PACKET):
        preamble, pSize, cmd, mSize, addr, size = struct.unpack(MSG_PKT_HDR_FMT, hdrmsg)
        if (pSize & PKT_CRC_FLAG):
            crc_bytes = intf_read(4)
            chk = int.from_bytes(crc_bytes, byteorder='little', signed=False)
            cal_chk = gen_crc16(buf[0:len(buf)], len(buf))
        if (mSize < 8):
            return (preamble, chk, cal_chk, cmd, addr, size, None)
        else:
            return (preamble, chk, cal_chk, cmd, addr, size, buf[16 : len(buf)])
    else:
        hdr, chk, cmd, addr, size = struct.unpack(MSG_HDR_FMT, hdrmsg)
        cal_chk = gen_crc16(buf[4:len(buf)], len(buf) - 4)
        if (len(buf) < 16):
            return (hdr, chk, cal_chk, cmd, addr, size, None)
        else:
            return (hdr, chk, cal_chk, cmd, addr, size, buf[16 : len(buf)])


##########################################################################
# Msg Command
##########################################################################
def msg_wait_ack():
    buf = intf_read(MSG_HDR_SIZE)
    hdr, chk, cal_chk, cmd, addr, size, data = unpack_data (buf)
    if (PACKET == 0):
        if cmd == CMD_ACK:
            return True
        else:
            return False
    # else we are using packet layer
    if (cmd & 0x8000):  # check is it a proper response code?
        rsp = cmd & 0x0FFF # get the original command
        if (rsp == CMD_ACK):
            return True
        elif (rsp == CMD_MEM_READ):
            return True
        elif (rsp == CMD_MEM_WRITE):
            return True
        elif (rsp == CMD_MEM_CLR):
            return True
        elif (rsp == CMD_TEST_ECHO):
            return True
        else:
            return False
    else:
        return False

def msg_wait_result(resp, len):
    buf = intf_read(MSG_HDR_SIZE + len)
    hdr, chk, cal_chk, cmd, addr, size, data = unpack_data (buf)
    if (PACKET):
        expected_response = resp | 0x8000
    else:
        expected_response = resp
    if cmd == expected_response:
        print('*** result rcvd: %x [%d]' %(cmd, len))
    else:
        print('*** result rcvd: %x NOT %x' %(cmd, resp))
    return list(data)


def msg_cmd_general(cmd, wait_time):
    ''' send run_once command '''
    data = pack_data(cmd, 0, 0, None)
    intf_write(data, MSG_HDR_SIZE)
    time.sleep(wait_time)
    cnt = 1
    RETRY_MAX = 3
    while(cnt <= RETRY_MAX):
        if msg_wait_ack():
            return
        else:
            print('ERR: CMD ACK FAIL, retry %d/%d' % (cnt, RETRY_MAX))
            cnt = cnt + 1
            time.sleep(0.5)

def msg_cmd_run_once():
    msg_cmd_general(CMD_DEMO_RUN_ONCE, 0.2)

def msg_cmd_run():
    msg_cmd_general(CMD_DEMO_RUN, 0.2)

# for development of FD/FR
def msg_cmd_run_single_model(model_type):
    if model_type == MODEL_TYPE_FD:
        msg_cmd_general(CMD_DEMO_FD, 0.2)
    elif model_type == MODEL_TYPE_LM:
        msg_cmd_general(CMD_DEMO_LM, 0.2)
    elif model_type == MODEL_TYPE_LD:
        msg_cmd_general(CMD_DEMO_LD, 0.2)
    elif model_type == MODEL_TYPE_FR:
        msg_cmd_general(CMD_DEMO_FR, 0.2)
    #for develpment of inference/register
    elif model_type == MODEL_TYPE_INFERENCE:
        msg_cmd_general(CMD_DEMO_INFERENCE, 10.0)
    elif model_type == MODEL_TYPE_REG1:
        msg_cmd_general(CMD_DEMO_REG1, 0.2)
    elif model_type == MODEL_TYPE_REG2:
        msg_cmd_general(CMD_DEMO_REG2, 0.2)
    elif model_type == MODEL_TYPE_REG3:
        msg_cmd_general(CMD_DEMO_REG3, 0.2)
    elif model_type == MODEL_TYPE_REG4:
        msg_cmd_general(CMD_DEMO_REG4, 0.2)
    elif model_type == MODEL_TYPE_REG5:
        msg_cmd_general(CMD_DEMO_REG5, 0.2)
    elif model_type == MODEL_TYPE_ADDUSER:
        msg_cmd_general(CMD_DEMO_ADDUSER, 0.2)
    elif model_type == MODEL_TYPE_DELUSER:
        msg_cmd_general(CMD_DEMO_DELUSER, 0.2)
    else:
        print('ERR: invalid model type')

def msg_cmd_stop():
    msg_cmd_general(CMD_DEMO_STOP, 0.2)

def msg_get_result_len():
    buf = msg_wait_result(CMD_DEMO_RESULT_LEN, 4)
    val = (buf[3] << 24) +  (buf[2] << 16) + (buf[1] << 8) + buf[0]
    return val

def msg_get_result(len):
    return msg_wait_result(CMD_DEMO_RESULT, len)

##########################################################################
# SFID Application
##########################################################################
def msg_sfid_response(cmd_sent):
    buf = intf_read(MSG_HDR_SIZE)
#    print(buf)
    hdr, chk, cal_chk, cmd, addr, size, data = unpack_data (buf)
    #PACKET is always be used
    if (cmd == (cmd_sent | 0x8000)):
        if (cmd_sent == CMD_SFID_START):
            print("Start SFID: Image Size = %d" %(size))
        elif (cmd_sent == CMD_SEND_IMAGE):
            if (addr == KDP_APP_OK):
                mode = size & 0xFFFF
                indx = size >> 16
                if (mode == 0):
                    print("Image Processed: Okay-> UserID  = %d" %(indx))
                else:
                    print("Image Processed: Okay-> FmIndx = %d" %(indx))
            elif (addr == KDP_APP_DB_NO_MATCH):
                print("Image Processed: NO MATCH")
            elif (addr == MSG_APP_UID_EXIST):
                print("Image: User ID already in DB")
            elif (addr == MSG_APP_IDX_FIRST):
                print("Image Processed: Index must be 1 for new session")
            elif (addr == MSG_APP_UID_DIFF):
                print("Image Processed: User ID change not allowed")
            elif (addr == MSG_APP_IDX_MISSING):
                print("Image Processed: User ID not related to sessionh")
            elif (addr == MSG_APP_BAD_IMAGE):
                print("Image Processed: no result")
            elif (addr == MSG_APP_BAD_INDEX):
                print("Image Processed: Invalid User ID or FM Index")
            elif (addr == MSG_APP_DB_FAIL):
                print("Image Processed: DB Error")
            else:
                print("Image Processed: undefined %x, %x" %(addr, size))

        elif (cmd_sent == CMD_SFID_ADD_DB):
            if (addr == KDP_APP_OK):
                print("Add DB: Okay.")
            elif (addr == MSG_APP_UID_EXIST):
                print("Add DB: User ID already in DB")
            elif (addr == MSG_APP_UID_DIFF):
                print("Add DB: User ID change not allowed")
            elif (addr == MSG_APP_IDX_MISSING):
                print("Add DB: User ID not related to sessionh")
            elif (addr == MSG_APP_DB_FAIL):
                print("Add DB: DB Add failed")
            else:
                print("Image Processed: undefined %x, %x" %(addr, size))

        elif (cmd_sent == CMD_SFID_DELETE_DB):
            if (addr == KDP_APP_OK):
                print("Delete DB: Okay.")
            elif (addr == MSG_APP_DB_NO_USER):
                print("Delete DB: User ID not found in DB")
            elif (addr == MSG_APP_DB_FAIL):
                print("Delete DB: DB Internal Fault")
            else:
                print("Delete DB: undefined %x, %x" %(addr, size))

        else:
            print("SFID %x response: %x, %x" %(cmd, addr, size))

    else:
        print("BAD SFID RESPONSE: expect %x, got %x" %(cmd_sent|0x8000, cmd))

# short ack packet is only used for PACKET = 1
def msg_sfid_short_ack_response(cmd_sent):
    buf = intf_read(8) # short command ack message is just header
    if ((buf[3] << 8) & (PKT_CRC_FLAG)):
        crc = intf_read(4)  # need to do crc check <<<<<
    if  ((buf[0] != PKRX_PAMB & 0xFF) or (buf[1] != (PKRX_PAMB >> 8))):
        print("BAD Preamble for Short ACK")
    elif (buf[3] != (PKT_CRC_FLAG >> 8)):
        print("BAD pSize for Short ACK")
    else:
        cmd = buf[4] + buf[5]*256 
        if (cmd == cmd_sent):
            if(cmd == CMD_SFID_NEW_USER):
                print("New User short ACK")
            else:
                print("SFID short ACK: %x" %(cmd))
        else:
            print("SFID wrong ACK: received %x, expected %x " %(cmd, cmd_sent))
    return

def msg_sfid_cmd(cmd, parm1, parm2):
    data = pack_data(cmd, parm1, parm2, None)
#    print(data)
    intf_write(data, len(data))
    if (cmd == CMD_SFID_START):
        print("SFID Start fd/fr/db")
    elif (cmd == CMD_SFID_NEW_USER):
        print("SFID New User, id = %d, fmx = %d" %(parm1, parm2))
    elif (cmd == CMD_SFID_ADD_DB):
        print("SFID Add DB, id = %d" %(parm1))
    elif (cmd == CMD_SFID_DELETE_DB):
        print("SFID Delete DB, id = %d" %(parm1))
    elif (cmd == CMD_SEND_IMAGE):
        print("SFID Send Image, size = %d, mode = %d" %(parm1, parm2))
    else:
        print("Unknown Command [%04x], (%04x, %04x)" %(cmd, parm1, parm2))

    if (cmd == CMD_SFID_NEW_USER):
        msg_sfid_short_ack_response(CMD_SFID_NEW_USER)
    elif (cmd != CMD_SEND_IMAGE): # need to wait for this response msg separately
        msg_sfid_response(cmd)

def msg_sfid_send_image(file_name):
    image_file = './image.bin'
    image_data = []
    image_data = file_read_binary(image_file)
    image_size = len(image_data)

    msg_sfid_cmd(CMD_SEND_IMAGE, image_size, IMAGE_TRF) # 0->none, 1->USB , 2-> UART
    if (IMAGE_TRF == 2):
        uart_write_data(image_data, image_size, 0)  # start from beginning of file
    elif (IMAGE_TRF == 1):
        usb_write_data(image_data, image_size, 0)  # start from beginning of file
    else:
        # no image transfer will take place, skip this step
        image_size = 0
    msg_sfid_response(CMD_SEND_IMAGE)

def send_at_command(command):
    ser.write(bytes(command+"\r", encoding='ascii'))

def getc(size, timeout=1):
    return ser.read(size)

def putc_user(data, timeout=1):
    return ser.write(data)

def erase_readline():
    response = ser.readline().strip()
    #print(response)
    #if(response == b'ERASE'):
    if b'ERA' in response:
        return True
    else:
        return False
def xmodem_send_bin(flash_type):
    count = 0
    timeout = 10
    print(flash_type)
    print("Please press reset button!!")
    while(timeout):
        timeout -= 1
        response = ser.readline().strip()
        if(response == b''):
            print("Please press reset button!!")
        if(response == b'\x00'):
            print("Reset done!!")
        elif(response != b''):
            count += 1
            print(response)
        if(count >= 6):
            break        
    if(timeout == 0):
        return False
    print("xmodem_send 4 start")
    send_at_command("4")
    print("xmodem_send 4 done")
    time.sleep(1)

    ser.flushInput()
    if(flash_type):
        image_file = './utils_flash_programmer_1212_nor.bin'
        print("utils_flash_programmer_1212_nor.bin!!")
    else:
        image_file = './utils_flash_programmer_1212_nand.bin'
        print("utils_flash_programmer_1212_nand.bin!!")
    print("xmodem_send bin file start")
    stream = open(image_file, 'rb')

    m = xmodem.XMODEM(getc, putc_user)

    print("xmodem_sending ... ")
    ret = m.send(stream)

    if(ret):
        print("xmodem_send bin file done!!")
    else:
        print("xmodem_send bin file FAIL!!!! Please reset the target and start over!!!!")
        return ret
    
    time.sleep(1)

    ser.flush()
    ser.flushInput()
    ser.close()
    time.sleep(0.2)
    print("change baudrate to 921600... ")
    setup_intf('uart', ser, COM_ID, BAUDRATE_HI, timeout=5)
    ser.flushInput()
    ser.flushOutput()
    print("change baudrate to 921600 done")
    time.sleep(0.2)
    return ret

##########################################################################
# NPU control
##########################################################################
def npu_reset():
    ''' 0xC238_004C[2], NPU reset control register. set 0 : reset, 1: release '''
    val = reg_read(0xC238004C)
    reg_write(0xC238004C, val & 0xFFFFFFFB)
    reg_write(0xC238004C, val | 0x04)




##########################################################################
# Utils
##########################################################################
def gen_crc16(buf, size):
    crc16_func = crcmod.predefined.mkPredefinedCrcFun('crc-16')
    # the size is implied by buf
    if (len(buf) != size):
        print("crc error, buffer is", len(buf), "size is", size)
    out = crc16_func(buf)
    return out


##########################################################################
# Digital test pattern parsing
##########################################################################
def pattern_file_parsing(file):
    CMD_STAGE_IDX = 0
    IN_WAIT = 1
    IN_STAGE_IDX = 2
    OUT_WAIT = 3
    OUT_STAGE_IDX = 4
    cmd_addr = 0
    input_addr = 0
    output_addr = 0
    line = 0
    stage = 0
    cmd = []
    input = []
    output = []
    with open(file, 'r') as f1:
        filelist = f1.readlines()
        for x in filelist:
            x = x.strip()
            line += 1
            if (x ==''):
                continue
            elif (x == 'xxxxxxxx'):
                if stage == CMD_STAGE_IDX or stage == IN_STAGE_IDX or stage == OUT_STAGE_IDX:
                    stage = stage + 1
                continue
            else:
                if stage == IN_WAIT or stage == OUT_WAIT:
                    stage += 1
                if stage == CMD_STAGE_IDX:
                    if not cmd:
                        cmd_addr = line
                    cmd.append(int(x, 16))
                elif stage == IN_STAGE_IDX:
                    if not input:
                        input_addr = line
                    input.append(int(x, 16))
                elif stage == OUT_STAGE_IDX:
                    if not output:
                        output_addr = line
                    output.append(int(x, 16))
    # dump
    rtl_test_dump('golden_dump.log', cmd, cmd_addr, input, input_addr, output, output_addr)
    '''
    with open('golden_dump.log', 'w') as fo:
        line = 0
        for x in cmd:
            fo.write('%08x\n' %x)
        for x in range(input_addr - len(cmd) - 1):
            fo.write('xxxxxxxx\n')
        for x in input:
            fo.write('%08x\n' %x)
        for x in range(output_addr - (input_addr + len(input))):
            fo.write('xxxxxxxx\n')
        for x in output:
            fo.write('%08x\n' %x)
    '''
    return (cmd, cmd_addr, input, input_addr, output, output_addr)


def rtl_test_dump(file, cmd, cmd_addr, input, input_addr, output, output_addr):
    with open(file, 'w') as fo:
        line = 0
        for x in cmd:
            fo.write('%08x\n' %x)
        for x in range(input_addr - len(cmd) - 1):
            fo.write('xxxxxxxx\n')
        for x in input:
            fo.write('%08x\n' %x)
        for x in range(output_addr - (input_addr + len(input))):
            fo.write('xxxxxxxx\n')
        for x in output:
            fo.write('%08x\n' %x)

def mem_rtl_test_dump(file, cmd_addr, cmd_len, input_addr, input_len, output_addr, output_len):
    '''
    cmd_addr: memory command address (ex: 0x20200000(SdRAM) address)
    input_addr: memory input address
    output_addr: memory output address
    '''
    with open(file, 'w') as fo:
        line = 0
        cmd = mem_read(cmd_addr, cmd_len)
        input = mem_read(input_addr, input_len)
        for x in cmd:
            fo.write('%08x\n' %x)
        for x in range(input_addr - len(cmd) - 1):
            fo.write('xxxxxxxx\n')
        for x in input:
            fo.write('%08x\n' %x)
        for x in range(output_addr - (input_addr + len(input))):
            fo.write('xxxxxxxx\n')
        for x in output:
            fo.write('%08x\n' %x)

def inst_hex_load(file):
    cmd = []
    with open(file, 'r') as f1:
        filelist = f1.readlines()
        for x in filelist:
            x = x.strip()
            if (x ==''):
                continue
            else:
                cmd.append(int(x, 16))
    f1.close()
    return cmd

def nmem_hex_load(file):
    zero_cnt = 0
    val = 0
    data = []
    with open(file, 'r') as f1:
        filelist = f1.readlines()
        for line in filelist:
            line = line.strip()
            if (line ==''):
                continue
            line = line.replace('x', '0')
            for j in range(len(line)//8, 0, -1):
                val = line[(j-1) * 8 : j * 8]
                val = int(val, 16)
                data.append(val)
                if val == 0:
                    zero_cnt += 1
    f1.close()
    return (zero_cnt, data)

def nmem_valstr_mask(valstr, maskstr):
    for i in range(len(maskstr)):
        if maskstr[i] == 'x':
            #valstr[i] = 'x'
            valstr = valstr[:i] + 'x' + valstr[i+1:]
    return valstr





def sys_sram_hex_weight_load(file):
    ''' Parsing weight and input data '''
    ''' @cnt = 1, data format: the input data is follow weight '''
    ''' @cnt = 2, data format: weight_block then input block '''
    linenum = 0
    at_cnt = 0
    weight_addr = 0
    weight = []
    input_addr = 0
    out_addr = 0
    out = []
    input = []
    with open(file, 'r') as f1:
        filelist = f1.readlines()
        for line in filelist:
            line = line.strip()
            if (line ==''):
                linenum += 1
                continue
            elif (line[0] == '@'):
                at_cnt += 1
                line = line[1 : len(line)]
                pos = line.find('/')
                if (pos != -1):
                    line = line[0 : pos]
                line = line.strip()
                if at_cnt == 1:
                    weight_addr = int(line, 16)
                elif at_cnt == 2:
                    input_addr = int(line, 16)
                else:
                    out_addr = int(line, 16)
            else:
                line = line.replace('x', '0')
                if at_cnt == 1:
                    weight.append(int(line, 16))
                elif at_cnt == 2:
                    input.append(int(line, 16))
                else:
                    out.append(int(line, 16))
            linenum += 1
    f1.close()
    return (weight_addr, weight, input_addr, input)

##########################################################################
# Get ROI information
##########################################################################
def get_roi_info(roi_file):
    BANK_NUM = 68
    info = { "sys": 0 } # cut from line 0 => no cut
    for i in range(BANK_NUM):
        key = "%02d" % i
        info[key] = None

    if os.path.isfile(roi_file):
        roi_info = json.load(open(roi_file))
        for key, val in roi_info.items():
            info[key] = val

    return info


##########################################################################
# File I/O
##########################################################################
def file_read_binary(src):
     ''' Load depth file '''
     with open(src, "rb") as f:
         data = f.read()
         return data

def file_cmp(file1, file2):
    l1 = l2 = True
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        while l1 and l2:
            l1 = f1.readline()
            l2 = f2.readline()
            if l1 != l2:
                return False
    return True

def file_cmp_binary(file1, file2):
    data1 = file_read_binary(file1)
    data2 = file_read_binary(file2)
    return (data1 == data2)

##########################################################################
# Debug
##########################################################################
def DBGPRINT(format, *args):
    """ print debug message """
    if __DEBUG__ == 1:
        print (format % args)


