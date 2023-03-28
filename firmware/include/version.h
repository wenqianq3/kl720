#ifndef __VERSION_H__
#define __VERSION_H__

//Image FW version format: 0xAABBCCDD, AA=major, BB=minor, CC=patch, DD=rd_reserve
#define IMG_FW_MAJOR        0x02
#define IMG_FW_MINOR        0x01
#define IMG_FW_PATCH        0x00
#define IMG_FW_RD_VER       0x00

#define IMG_ID              "SCPU"  //must be 4 bytes
#define IMG_FW_VERSION      ((IMG_FW_MAJOR<<24)+(IMG_FW_MINOR<<16)+(IMG_FW_PATCH<<8)+(IMG_FW_RD_VER))
#define IMG_FW_BUILD        105
#define IMG_FLAG            0xFFFFFFFF

#endif
