/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

#ifndef _PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_H_
#define _PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS 42

#define NUM_OF_QWORDS_PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS 21


struct phyrx_other_receive_info_all_sigb_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t pkt_type                                                :  4, // [3:0]
                      reserved_0a                                             : 12, // [15:4]
                      cc0_num_user                                            :  8, // [23:16]
                      cc1_num_user                                            :  8; // [31:24]
             uint32_t reserved_1a                                             : 32; // [31:0]
             uint32_t cc0_2user_blk_0_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_0_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_1_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_1_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_2_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_2_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_3_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_3_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_4_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_4_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_5_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_5_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_6_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_6_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_7_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_7_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_8_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_8_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_9_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_9_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_0_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_0_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_1_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_1_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_2_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_2_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_3_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_3_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_4_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_4_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_5_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_5_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_6_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_6_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_7_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_7_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_8_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_8_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_9_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_9_63_32                                   : 32; // [31:0]
#else
             uint32_t cc1_num_user                                            :  8, // [31:24]
                      cc0_num_user                                            :  8, // [23:16]
                      reserved_0a                                             : 12, // [15:4]
                      pkt_type                                                :  4; // [3:0]
             uint32_t reserved_1a                                             : 32; // [31:0]
             uint32_t cc0_2user_blk_0_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_0_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_1_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_1_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_2_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_2_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_3_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_3_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_4_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_4_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_5_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_5_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_6_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_6_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_7_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_7_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_8_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_8_63_32                                   : 32; // [31:0]
             uint32_t cc0_2user_blk_9_31_0                                    : 32; // [31:0]
             uint32_t cc0_2user_blk_9_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_0_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_0_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_1_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_1_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_2_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_2_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_3_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_3_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_4_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_4_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_5_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_5_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_6_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_6_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_7_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_7_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_8_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_8_63_32                                   : 32; // [31:0]
             uint32_t cc1_2user_blk_9_31_0                                    : 32; // [31:0]
             uint32_t cc1_2user_blk_9_63_32                                   : 32; // [31:0]
#endif
};




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_PKT_TYPE_OFFSET                   0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_PKT_TYPE_LSB                      0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_PKT_TYPE_MSB                      3
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_PKT_TYPE_MASK                     0x000000000000000f




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_0A_OFFSET                0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_0A_LSB                   4
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_0A_MSB                   15
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_0A_MASK                  0x000000000000fff0




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_NUM_USER_OFFSET               0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_NUM_USER_LSB                  16
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_NUM_USER_MSB                  23
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_NUM_USER_MASK                 0x0000000000ff0000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_NUM_USER_OFFSET               0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_NUM_USER_LSB                  24
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_NUM_USER_MSB                  31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_NUM_USER_MASK                 0x00000000ff000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_1A_OFFSET                0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_1A_LSB                   32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_1A_MSB                   63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_RESERVED_1A_MASK                  0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_31_0_OFFSET       0x0000000000000008
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_63_32_OFFSET      0x0000000000000008
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_0_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_31_0_OFFSET       0x0000000000000010
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_63_32_OFFSET      0x0000000000000010
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_1_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_31_0_OFFSET       0x0000000000000018
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_63_32_OFFSET      0x0000000000000018
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_2_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_31_0_OFFSET       0x0000000000000020
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_63_32_OFFSET      0x0000000000000020
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_3_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_31_0_OFFSET       0x0000000000000028
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_63_32_OFFSET      0x0000000000000028
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_4_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_31_0_OFFSET       0x0000000000000030
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_63_32_OFFSET      0x0000000000000030
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_5_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_31_0_OFFSET       0x0000000000000038
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_63_32_OFFSET      0x0000000000000038
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_6_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_31_0_OFFSET       0x0000000000000040
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_63_32_OFFSET      0x0000000000000040
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_7_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_31_0_OFFSET       0x0000000000000048
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_63_32_OFFSET      0x0000000000000048
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_8_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_31_0_OFFSET       0x0000000000000050
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_63_32_OFFSET      0x0000000000000050
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC0_2USER_BLK_9_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_31_0_OFFSET       0x0000000000000058
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_63_32_OFFSET      0x0000000000000058
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_0_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_31_0_OFFSET       0x0000000000000060
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_63_32_OFFSET      0x0000000000000060
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_1_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_31_0_OFFSET       0x0000000000000068
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_63_32_OFFSET      0x0000000000000068
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_2_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_31_0_OFFSET       0x0000000000000070
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_63_32_OFFSET      0x0000000000000070
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_3_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_31_0_OFFSET       0x0000000000000078
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_63_32_OFFSET      0x0000000000000078
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_4_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_31_0_OFFSET       0x0000000000000080
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_63_32_OFFSET      0x0000000000000080
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_5_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_31_0_OFFSET       0x0000000000000088
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_63_32_OFFSET      0x0000000000000088
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_6_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_31_0_OFFSET       0x0000000000000090
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_63_32_OFFSET      0x0000000000000090
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_7_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_31_0_OFFSET       0x0000000000000098
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_63_32_OFFSET      0x0000000000000098
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_8_63_32_MASK        0xffffffff00000000




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_31_0_OFFSET       0x00000000000000a0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_31_0_LSB          0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_31_0_MSB          31
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_31_0_MASK         0x00000000ffffffff




#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_63_32_OFFSET      0x00000000000000a0
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_63_32_LSB         32
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_63_32_MSB         63
#define PHYRX_OTHER_RECEIVE_INFO_ALL_SIGB_DETAILS_CC1_2USER_BLK_9_63_32_MASK        0xffffffff00000000




#endif
