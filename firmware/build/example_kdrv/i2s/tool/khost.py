from ctypes import*
import pdb
import struct

KL720_PID = 0x200
KL530_PID = 0x530
CMD_ID_MEM_WRITE = 0x01
CMD_ID_MEM_READ = 0x02
CMD_ID_MEM_WRITE_DMA = 0x03
CMD_ID_MEM_READ_DMA = 0x04
CMD_ID_NCPU_RUN = 0x05
CMD_ID_GO = 0x06
'''
   For Debug Memory Write command:
   cmd_id = CMD_ID_MEM_WRITE, arg1 = WRITE_ADDRESS, arg2 = WRITE_SIZE

   For Debug Memory Read command:
   cmd_id = CMD_ID_MEM_READ, arg1 = READ_ADDRESS, arg2 = READ_SIZE
'''
USB_BUF_MAX = 2 * 1024 * 1024
USB_ONE_TIME_XFR = 8192
class khost_t():
    EP_IN = 0x81
    EP_OUT = 0x02
    def __init__(self):
        self.usb = cdll.LoadLibrary("./khost_lib_mingw_x64.dll")
        #self.usb = cdll.LoadLibrary("../../../khost_lib/windows_prebuilt/khost_lib_x64.dll")
        ''' Function type initialize '''
        self.usb.khost_ll_connect_device.restype = c_ulonglong
        self.usb.khost_ll_connect_device.argtypes = [c_ulonglong]
        self.usb.khost_ll_disconnect_device.argtypes = [c_ulonglong]
        self.usb.khost_ll_read_data.argtypes = [c_ulonglong, c_ulonglong, c_void_p, c_longlong, c_longlong]
        self.usb.khost_ll_read_data.restype = c_ulonglong
        self.usb.khost_ll_write_data.argtypes = [c_ulonglong, c_ulonglong, c_void_p, c_longlong, c_longlong]
        self.uid = c_ulonglong(0)
        self.cmd_prefix = 0xAABBCCDD

    def is_connect(self):
        if self.uid != 0:
            return True
        else:
            return False

    def connect(self):
        self.uid = self.usb.khost_ll_connect_device(KL720_PID)
        return self.uid

    def disconnect(self):
        if self.uid != 0:
            self.usb.khost_ll_disconnect_device(self.uid)
            self.uid = 0
    
    # Read data directly from the specific USB endpoint
    def read_data(self, ep, size):
        rx_data = []
        cbuf = create_string_buffer(size)
        self.usb.khost_ll_read_data(self.uid, ep, cbuf, size, 5000)
        rx_data.extend(list(cbuf.raw))
        return rx_data[:size]

    # Write data directly to the specific USB endpoint
    def write_data(self, ep, buf, size):
        wbuf = create_string_buffer(bytes(buf[::]))
        self.usb.khost_ll_write_data(self.uid, ep, wbuf, size, 5000)


    def mem_read(self, addr, size):
        # short term solution, usb buffer has USB_ONE_TIME_XFR limitation.
        rx_data = []
        total_size = size
        idx = 0
        while total_size > 0:
            rx_cnt = total_size
            if rx_cnt > USB_ONE_TIME_XFR:
                rx_cnt = USB_ONE_TIME_XFR
            status = self.cmd_header_write(CMD_ID_MEM_READ, addr+idx, rx_cnt)
            idx += rx_cnt
            total_size -= rx_cnt
            if status == 0:
                resp = self.read_data(self.EP_IN, rx_cnt)
                rx_data.extend(resp)
        return rx_data

    def mem_write(self, addr, buf, size):
        total_size = size
        idx = 0
        while total_size > 0:
            tx_cnt = total_size
            if tx_cnt > USB_ONE_TIME_XFR:
                tx_cnt = USB_ONE_TIME_XFR
            self.cmd_write(CMD_ID_MEM_WRITE, addr+idx, tx_cnt, buf[idx:idx+tx_cnt], tx_cnt)
            idx += tx_cnt
            total_size -= tx_cnt
            resp = self.read_data(self.EP_IN, 4)
            resp_int, = struct.unpack('I', bytes(resp))
            if resp_int != self.cmd_prefix:
                print('no ack')
                return -1
        #sleep(0.05)
         
    def cmd_header_write(self, cmd, arg1, arg2):
        cmd_header = struct.pack('4I', self.cmd_prefix, cmd, arg1, arg2)
        self.write_data(self.EP_OUT, cmd_header, len(cmd_header))
        resp = self.read_data(self.EP_IN, 4)
        resp_int, = struct.unpack('I', bytes(resp))
        if resp_int == self.cmd_prefix:
            return 0
        else:
            print('header no ack')
            print(resp)
            return -1
            
    def cmd_write(self, cmd, arg1, arg2, buf, size):
        x = self.cmd_header_write(cmd, arg1, arg2)
        if x == 0:
            self.write_data(self.EP_OUT, buf, size)
    
    def reg_write(self, addr, val):
        val_buf = struct.pack('I', val)
        self.cmd_write(CMD_ID_MEM_WRITE, addr, 4, val_buf, 4)
        resp = self.read_data(self.EP_IN, 4)
        resp_int, = struct.unpack('I', bytes(resp))
        if resp_int == self.cmd_prefix:
            return 0
        else:
            print('no ack')
            return -1
        
    def reg_read(self, addr):
        status = self.cmd_header_write(CMD_ID_MEM_READ, addr, 4)
        if status == 0:
            resp = self.read_data(self.EP_IN, 4)
            int_val, = struct.unpack('I', bytes(resp))
            return (int_val)
        
            
