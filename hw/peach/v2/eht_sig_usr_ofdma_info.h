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


#ifndef _EHT_SIG_USR_OFDMA_INFO_H_
#define _EHT_SIG_USR_OFDMA_INFO_H_

#define NUM_OF_DWORDS_EHT_SIG_USR_OFDMA_INFO 2

struct eht_sig_usr_ofdma_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sta_id                                                  : 11,
                      sta_mcs                                                 :  4,
                      validate_0a                                             :  1,
                      nss                                                     :  4,
                      txbf                                                    :  1,
                      sta_coding                                              :  1,
                      reserved_0b                                             :  1,
                      rx_integrity_check_passed                               :  1,
                      subband80_cc_mask                                       :  8;
             uint32_t user_order_subband80_0                                  :  8,
                      user_order_subband80_1                                  :  8,
                      user_order_subband80_2                                  :  8,
                      user_order_subband80_3                                  :  8;
#else
             uint32_t subband80_cc_mask                                       :  8,
                      rx_integrity_check_passed                               :  1,
                      reserved_0b                                             :  1,
                      sta_coding                                              :  1,
                      txbf                                                    :  1,
                      nss                                                     :  4,
                      validate_0a                                             :  1,
                      sta_mcs                                                 :  4,
                      sta_id                                                  : 11;
             uint32_t user_order_subband80_3                                  :  8,
                      user_order_subband80_2                                  :  8,
                      user_order_subband80_1                                  :  8,
                      user_order_subband80_0                                  :  8;
#endif
};

#define EHT_SIG_USR_OFDMA_INFO_STA_ID_OFFSET                                        0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_LSB                                           0
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_MSB                                           10
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_MASK                                          0x000007ff

#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_OFFSET                                       0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_LSB                                          11
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_MSB                                          14
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_MASK                                         0x00007800

#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_OFFSET                                   0x00000000
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_LSB                                      15
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_MSB                                      15
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_MASK                                     0x00008000

#define EHT_SIG_USR_OFDMA_INFO_NSS_OFFSET                                           0x00000000
#define EHT_SIG_USR_OFDMA_INFO_NSS_LSB                                              16
#define EHT_SIG_USR_OFDMA_INFO_NSS_MSB                                              19
#define EHT_SIG_USR_OFDMA_INFO_NSS_MASK                                             0x000f0000

#define EHT_SIG_USR_OFDMA_INFO_TXBF_OFFSET                                          0x00000000
#define EHT_SIG_USR_OFDMA_INFO_TXBF_LSB                                             20
#define EHT_SIG_USR_OFDMA_INFO_TXBF_MSB                                             20
#define EHT_SIG_USR_OFDMA_INFO_TXBF_MASK                                            0x00100000

#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_OFFSET                                    0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_LSB                                       21
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_MSB                                       21
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_MASK                                      0x00200000

#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_OFFSET                                   0x00000000
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_LSB                                      22
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_MSB                                      22
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_MASK                                     0x00400000

#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                     0x00000000
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                        23
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                        23
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                       0x00800000

#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_OFFSET                             0x00000000
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_LSB                                24
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_MSB                                31
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_MASK                               0xff000000

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_LSB                           0
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_MSB                           7
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_MASK                          0x000000ff

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_LSB                           8
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_MSB                           15
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_MASK                          0x0000ff00

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_LSB                           16
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_MSB                           23
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_MASK                          0x00ff0000

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_LSB                           24
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_MSB                           31
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_MASK                          0xff000000

#endif
