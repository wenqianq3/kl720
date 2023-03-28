# keywords (in lower case)
__WORDS__ = \
{
    'usb'     :   '0x00',
    'uart'    :   '0x01',
    'i2c'     :   '0x02',
    'spi'     :   '0x03',
}


__CMDS__ = \
{
    '0'       :   'CMD_NONE',
    '1'       :   'CMD_MEM_READ',
    '2'       :   'CMD_MEM_WRITE',
    '3'       :   'CMD_DATA',    
    '4'       :   'CMD_ACK',
    '5'       :   'CMD_STS_CLR',
    '6'       :   'CMD_CHK_ERR',
    '7'       :   'CMD_TEST_ECHO',    
}

__MEM_SCPU_ADDR__ = \
{
    'SIRAM'   :   0x10100000,
    'SDRAM'   :   0x10200000,
    'NIRAM'   :   0x28000000,
    'NDRAM'   :   0x2FFF0000,
    'NPURAM'  :   0x30000000,
    'DDR'     :   0x60000000,
}

__MEM_NCPU_ADDR__ = \
{
    'SIRAM'   :   0x10100000,
    'SDRAM'   :   0x20200000,
    'NIRAM'   :   0x00000000,
    'NDRAM'   :   0x0FFF0000,
    'NPURAM'  :   0x30000000,
    'DDR'     :   0x60000000,
}


__MEM_SIZE__ = \
{
    'SIRAM'   :   0x00018000,       #96KB
    'SDRAM'   :   0x00018000,       #96KB    
    'NIRAM'   :   0x00010000,       #64KB
    'NDRAM'   :   0x00010000,       #64KB
    'NPURAM'  :   0x00080000,       #512KB
    'DDR'     :   0x02000000,       #32MB
}

__MODEL_ID__ = \
{
    'TinyYolo'    :   1,  
    'ResNet50'    :   2,     
    'MobileNetV2' :   3,  
}

def get_mem_name(dict, value):
    keys = dict.keys()
    for i in dict:
        if(dict[i] == value):
            return i
    return None


#####################
##  NPU Registers  ##
#####################
#register attribute
NPU_BASE = 0x30FF0000
NPU_REGS = \
{ 
    'ADDR_NPU_CODE'         : NPU_BASE + 0x0000,
    'ADDR_NPU_CLEN'         : NPU_BASE + 0x0004,
    'ADDR_NPU_RUN'          : NPU_BASE + 0x0008,
    'ADDR_NPU_ELEN'         : NPU_BASE + 0x000C,
    'ADDR_NPU_VER'          : NPU_BASE + 0x0010,
    'ADDR_NPU_MODE'         : NPU_BASE + 0x0014,
    'ADDR_NPU_DMA'          : NPU_BASE + 0x0018,
    'ADDR_NPU_RDMA0_SRC0'   : NPU_BASE + 0x001C,
    'ADDR_NPU_RDMA0_SRC1'   : NPU_BASE + 0x0020,
    'ADDR_NPU_RDMA0_SRC2'   : NPU_BASE + 0x0024,
    'ADDR_NPU_RDMA0_DST'    : NPU_BASE + 0x0028,
    'ADDR_NPU_RDMA0_BLK'    : NPU_BASE + 0x002C,
    'ADDR_NPU_RDMA1_SRC0'   : NPU_BASE + 0x0030,
    'ADDR_NPU_RDMA1_SRC1'   : NPU_BASE + 0x0034,
    'ADDR_NPU_RDMA1_SRC2'   : NPU_BASE + 0x0038,
    'ADDR_NPU_RDMA1_DST'    : NPU_BASE + 0x003C,
    'ADDR_NPU_RDMA1_BLK'    : NPU_BASE + 0x0040,
    'ADDR_NPU_RDMA2_SRC0'   : NPU_BASE + 0x0044,
    'ADDR_NPU_RDMA2_SRC1'   : NPU_BASE + 0x0048,
    'ADDR_NPU_RDMA2_SRC2'   : NPU_BASE + 0x004C,
    'ADDR_NPU_RDMA2_DST'    : NPU_BASE + 0x0050,
    'ADDR_NPU_RDMA2_BLK'    : NPU_BASE + 0x0054,
    'ADDR_NPU_GETW0'        : NPU_BASE + 0x0058,
    'ADDR_NPU_GETW1'        : NPU_BASE + 0x005C,
    'ADDR_NPU_GETW2'        : NPU_BASE + 0x0060,
    'ADDR_NPU_WDMA0_SRC'    : NPU_BASE + 0x0064,
    'ADDR_NPU_WDMA0_DST0'   : NPU_BASE + 0x0068,
    'ADDR_NPU_WDMA0_DST1'   : NPU_BASE + 0x006C,
    'ADDR_NPU_WDMA0_DST2'   : NPU_BASE + 0x0070,
    'ADDR_NPU_WDMA0_BLK'    : NPU_BASE + 0x0074,
    'ADDR_NPU_WDMA1_SRC'    : NPU_BASE + 0x0078,
    'ADDR_NPU_WDMA1_DST0'   : NPU_BASE + 0x007C,
    'ADDR_NPU_WDMA1_DST1'   : NPU_BASE + 0x0080,
    'ADDR_NPU_WDMA1_DST2'   : NPU_BASE + 0x0084,
    'ADDR_NPU_WDMA1_BLK'    : NPU_BASE + 0x0088,
    'ADDR_NPU_NMEM_FM0'     : NPU_BASE + 0x008C,
    'ADDR_NPU_NMEM_FM1'     : NPU_BASE + 0x0090,
    'ADDR_NPU_NMEM_FM2'     : NPU_BASE + 0x0094,
    'ADDR_NPU_NMEM_PS0'     : NPU_BASE + 0x0098,
    'ADDR_NPU_NMEM_PS1'     : NPU_BASE + 0x009C,
    'ADDR_NPU_NMEM_PS2'     : NPU_BASE + 0x00A0,
    'ADDR_NPU_NMEM_ST0'     : NPU_BASE + 0x00A4,
    'ADDR_NPU_NMEM_ST1'     : NPU_BASE + 0x00A8,
    'ADDR_NPU_NMEM_ST2'     : NPU_BASE + 0x00AC,
    'ADDR_NPU_NMEM_RDMA0'   : NPU_BASE + 0x00B0,
    'ADDR_NPU_NMEM_RDMA1'   : NPU_BASE + 0x00B4,
    'ADDR_NPU_NMEM_RDMA2'   : NPU_BASE + 0x00B8,
    'ADDR_NPU_NMEM_WDMA0'   : NPU_BASE + 0x00BC,
    'ADDR_NPU_NMEM_WDMA1'   : NPU_BASE + 0x00C0,
    'ADDR_NPU_NMEM_WDMA2'   : NPU_BASE + 0x00C4,
    'ADDR_NPU_NMEM_WT'      : NPU_BASE + 0x00C8,
    'ADDR_NPU_NMEM_LB'      : NPU_BASE + 0x00CC,
    'ADDR_NPU_CONV'         : NPU_BASE + 0x00D0,
    'ADDR_NPU_FMAP0'        : NPU_BASE + 0x00D4,
    'ADDR_NPU_FMAP1'        : NPU_BASE + 0x00D8,
    'ADDR_NPU_ZPAD'         : NPU_BASE + 0x00DC,
    'ADDR_NPU_NEXT0'        : NPU_BASE + 0x00E0,
    'ADDR_NPU_NEXT1'        : NPU_BASE + 0x00E4,
    'ADDR_NPU_NEXT2'        : NPU_BASE + 0x00E8,
    'ADDR_NPU_LWT'          : NPU_BASE + 0x00EC,
    'ADDR_NPU_ACC'          : NPU_BASE + 0x00F0,
    'ADDR_NPU_PSUM'         : NPU_BASE + 0x00F4,
    'ADDR_NPU_CUSTOM'       : NPU_BASE + 0x00F8,
    'ADDR_NPU_STORE'        : NPU_BASE + 0x00FC,
    'ADDR_NPU_TRIM'         : NPU_BASE + 0x0100,
    'ADDR_NPU_RESHAPE'      : NPU_BASE + 0x0104,
    'ADDR_NPU_PCONV'        : NPU_BASE + 0x0108,
    'ADDR_NPU_BN'           : NPU_BASE + 0x010C,
    'ADDR_NPU_RELU'         : NPU_BASE + 0x0110,
    'ADDR_NPU_RELU6'        : NPU_BASE + 0x0114,
    'ADDR_NPU_POOL'         : NPU_BASE + 0x0118,
    'ADDR_NPU_GAP'          : NPU_BASE + 0x011C,
    'ADDR_NPU_FORMAT'       : NPU_BASE + 0x0120,
    'ADDR_NPU_RESIZE'       : NPU_BASE + 0x0124,
    'ADDR_NPU_RESIZE_SRC'   : NPU_BASE + 0x0128,
    'ADDR_NPU_RESIZE_DST'   : NPU_BASE + 0x012C,
    'ADDR_NPU_RESIZE_RATIO' : NPU_BASE + 0x0130,
    'ADDR_NPU_CHAN'         : NPU_BASE + 0x0134,
    'ADDR_NPU_INTEN'        : NPU_BASE + 0x0138,
    'ADDR_NPU_INT'          : NPU_BASE + 0x013C,
    'ADDR_NPU_DBG0'         : NPU_BASE + 0x0140,
    'ADDR_NPU_DBG1'         : NPU_BASE + 0x0144,
    'ADDR_NPU_REGM'         : NPU_BASE + 0x0148,
    'ADDR_NPU_DUMMY_0'      : NPU_BASE + 0x014C,
    'ADDR_NPU_DUMMY_1'      : NPU_BASE + 0x0150,
    'ADDR_NPU_DUMMY_2'      : NPU_BASE + 0x0154,
}



