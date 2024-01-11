/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef _U_SIG_EHT_TB_INFO_H_
#define _U_SIG_EHT_TB_INFO_H_

#define NUM_OF_DWORDS_U_SIG_EHT_TB_INFO 2

struct u_sig_eht_tb_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_version                                             :  3,
                      transmit_bw                                             :  3,
                      dl_ul_flag                                              :  1,
                      bss_color_id                                            :  6,
                      txop_duration                                           :  7,
                      disregard_0a                                            :  6,
                      reserved_0c                                             :  6;
             uint32_t eht_ppdu_sig_cmn_type                                   :  2,
                      validate_1a                                             :  1,
                      spatial_reuse                                           :  8,
                      disregard_1b                                            :  5,
                      crc                                                     :  4,
                      tail                                                    :  6,
                      reserved_1c                                             :  5,
                      rx_integrity_check_passed                               :  1;
#else
             uint32_t reserved_0c                                             :  6,
                      disregard_0a                                            :  6,
                      txop_duration                                           :  7,
                      bss_color_id                                            :  6,
                      dl_ul_flag                                              :  1,
                      transmit_bw                                             :  3,
                      phy_version                                             :  3;
             uint32_t rx_integrity_check_passed                               :  1,
                      reserved_1c                                             :  5,
                      tail                                                    :  6,
                      crc                                                     :  4,
                      disregard_1b                                            :  5,
                      spatial_reuse                                           :  8,
                      validate_1a                                             :  1,
                      eht_ppdu_sig_cmn_type                                   :  2;
#endif
};

#define U_SIG_EHT_TB_INFO_PHY_VERSION_OFFSET                                        0x00000000
#define U_SIG_EHT_TB_INFO_PHY_VERSION_LSB                                           0
#define U_SIG_EHT_TB_INFO_PHY_VERSION_MSB                                           2
#define U_SIG_EHT_TB_INFO_PHY_VERSION_MASK                                          0x00000007

#define U_SIG_EHT_TB_INFO_TRANSMIT_BW_OFFSET                                        0x00000000
#define U_SIG_EHT_TB_INFO_TRANSMIT_BW_LSB                                           3
#define U_SIG_EHT_TB_INFO_TRANSMIT_BW_MSB                                           5
#define U_SIG_EHT_TB_INFO_TRANSMIT_BW_MASK                                          0x00000038

#define U_SIG_EHT_TB_INFO_DL_UL_FLAG_OFFSET                                         0x00000000
#define U_SIG_EHT_TB_INFO_DL_UL_FLAG_LSB                                            6
#define U_SIG_EHT_TB_INFO_DL_UL_FLAG_MSB                                            6
#define U_SIG_EHT_TB_INFO_DL_UL_FLAG_MASK                                           0x00000040

#define U_SIG_EHT_TB_INFO_BSS_COLOR_ID_OFFSET                                       0x00000000
#define U_SIG_EHT_TB_INFO_BSS_COLOR_ID_LSB                                          7
#define U_SIG_EHT_TB_INFO_BSS_COLOR_ID_MSB                                          12
#define U_SIG_EHT_TB_INFO_BSS_COLOR_ID_MASK                                         0x00001f80

#define U_SIG_EHT_TB_INFO_TXOP_DURATION_OFFSET                                      0x00000000
#define U_SIG_EHT_TB_INFO_TXOP_DURATION_LSB                                         13
#define U_SIG_EHT_TB_INFO_TXOP_DURATION_MSB                                         19
#define U_SIG_EHT_TB_INFO_TXOP_DURATION_MASK                                        0x000fe000

#define U_SIG_EHT_TB_INFO_DISREGARD_0A_OFFSET                                       0x00000000
#define U_SIG_EHT_TB_INFO_DISREGARD_0A_LSB                                          20
#define U_SIG_EHT_TB_INFO_DISREGARD_0A_MSB                                          25
#define U_SIG_EHT_TB_INFO_DISREGARD_0A_MASK                                         0x03f00000

#define U_SIG_EHT_TB_INFO_RESERVED_0C_OFFSET                                        0x00000000
#define U_SIG_EHT_TB_INFO_RESERVED_0C_LSB                                           26
#define U_SIG_EHT_TB_INFO_RESERVED_0C_MSB                                           31
#define U_SIG_EHT_TB_INFO_RESERVED_0C_MASK                                          0xfc000000

#define U_SIG_EHT_TB_INFO_EHT_PPDU_SIG_CMN_TYPE_OFFSET                              0x00000004
#define U_SIG_EHT_TB_INFO_EHT_PPDU_SIG_CMN_TYPE_LSB                                 0
#define U_SIG_EHT_TB_INFO_EHT_PPDU_SIG_CMN_TYPE_MSB                                 1
#define U_SIG_EHT_TB_INFO_EHT_PPDU_SIG_CMN_TYPE_MASK                                0x00000003

#define U_SIG_EHT_TB_INFO_VALIDATE_1A_OFFSET                                        0x00000004
#define U_SIG_EHT_TB_INFO_VALIDATE_1A_LSB                                           2
#define U_SIG_EHT_TB_INFO_VALIDATE_1A_MSB                                           2
#define U_SIG_EHT_TB_INFO_VALIDATE_1A_MASK                                          0x00000004

#define U_SIG_EHT_TB_INFO_SPATIAL_REUSE_OFFSET                                      0x00000004
#define U_SIG_EHT_TB_INFO_SPATIAL_REUSE_LSB                                         3
#define U_SIG_EHT_TB_INFO_SPATIAL_REUSE_MSB                                         10
#define U_SIG_EHT_TB_INFO_SPATIAL_REUSE_MASK                                        0x000007f8

#define U_SIG_EHT_TB_INFO_DISREGARD_1B_OFFSET                                       0x00000004
#define U_SIG_EHT_TB_INFO_DISREGARD_1B_LSB                                          11
#define U_SIG_EHT_TB_INFO_DISREGARD_1B_MSB                                          15
#define U_SIG_EHT_TB_INFO_DISREGARD_1B_MASK                                         0x0000f800

#define U_SIG_EHT_TB_INFO_CRC_OFFSET                                                0x00000004
#define U_SIG_EHT_TB_INFO_CRC_LSB                                                   16
#define U_SIG_EHT_TB_INFO_CRC_MSB                                                   19
#define U_SIG_EHT_TB_INFO_CRC_MASK                                                  0x000f0000

#define U_SIG_EHT_TB_INFO_TAIL_OFFSET                                               0x00000004
#define U_SIG_EHT_TB_INFO_TAIL_LSB                                                  20
#define U_SIG_EHT_TB_INFO_TAIL_MSB                                                  25
#define U_SIG_EHT_TB_INFO_TAIL_MASK                                                 0x03f00000

#define U_SIG_EHT_TB_INFO_RESERVED_1C_OFFSET                                        0x00000004
#define U_SIG_EHT_TB_INFO_RESERVED_1C_LSB                                           26
#define U_SIG_EHT_TB_INFO_RESERVED_1C_MSB                                           30
#define U_SIG_EHT_TB_INFO_RESERVED_1C_MASK                                          0x7c000000

#define U_SIG_EHT_TB_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                          0x00000004
#define U_SIG_EHT_TB_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                             31
#define U_SIG_EHT_TB_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                             31
#define U_SIG_EHT_TB_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                            0x80000000

#endif
