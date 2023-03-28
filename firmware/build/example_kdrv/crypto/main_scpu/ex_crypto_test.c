/*
 * Kneron Crypto AES/SHA test sample code 
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kdrv_io.h"
#include "kdrv_status.h"
#include "kdrv_timer.h"
#include "kdrv_clock.h"
#include "kmdw_console.h"
#include "kdrv_cmsis_core.h"
#include "regbase.h"
#include "kdrv_aes.h"
#include "kdrv_hash.h"
/**
 * @brief crypto_test_main crypto test program
 */
uint32_t crypto_selftest(void)
{
    uint32_t ret, i;

    const uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73,
                            0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07,
                            0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14,
                            0xdf, 0xf4
                          };
    const uint8_t cypher_data[]  = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e,
                            0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6, 0x9c, 0xfc, 0x4e, 0x96,
                            0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70,
                            0x2c, 0x7d, 0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf,
                            0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61, 0xb2, 0xeb,
                            0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07,
                            0x8c, 0x6a, 0x9d, 0x1b
                          };
    const uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                            0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
                          };
    const uint8_t plain_data[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
                            0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57,
                            0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf,
                            0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
                            0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f,
                            0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b,
                            0xe6, 0x6c, 0x37, 0x10
                          };

    const uint8_t msg [] = "Kneron is a trusted long-term partner for industries and companies where state-of-the-art electronic skills are required. We take pride in offering the best solutions and services possible, answering challenges set by applications and industries or by legislative bodies and standards.";
    uint8_t expected_dig[] = "\xa6\xe0\xce\x77\x4e\x26\xf8\xbd\x67\xde\x2c\x79\x9d\x47\x53\x44\x39\x47\x5d\x30\x70\x9f\x16\x16\xc2\x20\x1e\xf\x30\x39\xfc\xaa";

    uint8_t out[64]    = {0};
    block_t key_blk    = block_t_convert(key, sizeof(key));
    block_t iv_blk     = block_t_convert(iv,  sizeof(iv));
    block_t cipher_blk = block_t_convert(cypher_data,  sizeof(cypher_data));
    block_t plain_blk  = block_t_convert(plain_data,  sizeof(plain_data));    
    block_t out_blk    = block_t_convert(out, sizeof(out));
    block_t msg_blk    = block_t_convert(msg, strlen((char *) msg));

    //=========================================================================
    kdrv_aes_initialize();
    kmdw_printf("\nAES 256 CBC decrypt test: ");
    memset(out, 0, sizeof(out));
    ret = kdrv_aes_cbc_decrypt(&key_blk, &iv_blk, &cipher_blk, &out_blk);

    if (ret != KDRV_STATUS_OK) {
        kmdw_printf("AES decrypt failed, return code = 0x%x\n", ret);
        return ret;
    }

    if (memcmp((char*) plain_data, (char*) out, sizeof(out)) != 0) {
        kmdw_printf("AES decrypted data not match expected plain text\n");
        return KDRV_STATUS_ERROR;
    } else
        kmdw_printf("AES decrypt test, PASS.\n");

    //=========================================================================
    kmdw_printf("\nAES 256 CBC encrypt test: ");
    memset(out, 0, sizeof(out));
    ret = kdrv_aes_cbc_encrypt(&key_blk, &iv_blk, &plain_blk, &out_blk);

    if (ret != KDRV_STATUS_OK) {
        kmdw_printf("AES encrypt failed, return code = 0x%x\n", ret);
        return ret;
    }

    if (memcmp((char*) cypher_data, (char*) out, sizeof(out)) != 0) {
        kmdw_printf("AES encrypt data not match expected plain text\n");
        return KDRV_STATUS_ERROR;
    } else
        kmdw_printf("AES encrypt test, PASS.\n");

    //=========================================================================
    kmdw_printf("\nSHA 256 mode start test:\n");

    kdrv_hash_initialize();

    //ret = sx_hash_blk(e_SHA256, msg_blk, out_blk);
    ret = kdrv_hash_blk(e_SHA256, msg_blk, BLK_LITARRAY(out));

    if (ret != KDRV_STATUS_OK) {
        kmdw_printf("sx_hash_blk failed, return code = 0x%x\n", ret);
        return ret;
    }
    kmdw_printf("out data:\n");
    for (i=0; i < 32; i++) 
        kmdw_printf("%x ", out[i]);
    kmdw_printf("\n");
    kmdw_printf("expected data:\n");
    for (i=0; i < 32; i++) 
        kmdw_printf("%x ", expected_dig[i]);
    kmdw_printf("\n");

    if (memcmp((char*) expected_dig, (char*) out, 32) != 0) {
        kmdw_printf("SHA256 data not match expected dig\n");
        return KDRV_STATUS_ERROR;
    } else
        kmdw_printf("SHA256 Pass.\n");
    return KDRV_STATUS_OK;
}


