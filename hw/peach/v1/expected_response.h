/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _EXPECTED_RESPONSE_H_
#define _EXPECTED_RESPONSE_H_

#define NUM_OF_DWORDS_EXPECTED_RESPONSE 5

struct expected_response {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_ad2_31_0                                             : 32;
             uint32_t tx_ad2_47_32                                            : 16,
                      expected_response_type                                  :  5,
                      response_to_response                                    :  3,
                      su_ba_user_number                                       :  1,
                      response_info_part2_required                            :  1,
                      transmitted_bssid_check_en                              :  1,
                      reserved_1                                              :  5;
             uint32_t ndp_sta_partial_aid_2_8_0                               : 11,
                      reserved_2                                              : 10,
                      ndp_sta_partial_aid1_8_0                                : 11;
             uint32_t ast_index                                               : 16,
                      capture_ack_ba_sounding                                 :  1,
                      capture_sounding_1str_20mhz                             :  1,
                      capture_sounding_1str_40mhz                             :  1,
                      capture_sounding_1str_80mhz                             :  1,
                      capture_sounding_1str_160mhz                            :  1,
                      capture_sounding_1str_240mhz                            :  1,
                      capture_sounding_1str_320mhz                            :  1,
                      reserved_3a                                             :  9;
             uint32_t fcs                                                     :  9,
                      reserved_4a                                             :  1,
                      crc                                                     :  4,
                      scrambler_seed                                          :  7,
                      reserved_4b                                             : 11;
#else
             uint32_t tx_ad2_31_0                                             : 32;
             uint32_t reserved_1                                              :  5,
                      transmitted_bssid_check_en                              :  1,
                      response_info_part2_required                            :  1,
                      su_ba_user_number                                       :  1,
                      response_to_response                                    :  3,
                      expected_response_type                                  :  5,
                      tx_ad2_47_32                                            : 16;
             uint32_t ndp_sta_partial_aid1_8_0                                : 11,
                      reserved_2                                              : 10,
                      ndp_sta_partial_aid_2_8_0                               : 11;
             uint32_t reserved_3a                                             :  9,
                      capture_sounding_1str_320mhz                            :  1,
                      capture_sounding_1str_240mhz                            :  1,
                      capture_sounding_1str_160mhz                            :  1,
                      capture_sounding_1str_80mhz                             :  1,
                      capture_sounding_1str_40mhz                             :  1,
                      capture_sounding_1str_20mhz                             :  1,
                      capture_ack_ba_sounding                                 :  1,
                      ast_index                                               : 16;
             uint32_t reserved_4b                                             : 11,
                      scrambler_seed                                          :  7,
                      crc                                                     :  4,
                      reserved_4a                                             :  1,
                      fcs                                                     :  9;
#endif
};

#define EXPECTED_RESPONSE_TX_AD2_31_0_OFFSET                                        0x00000000
#define EXPECTED_RESPONSE_TX_AD2_31_0_LSB                                           0
#define EXPECTED_RESPONSE_TX_AD2_31_0_MSB                                           31
#define EXPECTED_RESPONSE_TX_AD2_31_0_MASK                                          0xffffffff

#define EXPECTED_RESPONSE_TX_AD2_47_32_OFFSET                                       0x00000004
#define EXPECTED_RESPONSE_TX_AD2_47_32_LSB                                          0
#define EXPECTED_RESPONSE_TX_AD2_47_32_MSB                                          15
#define EXPECTED_RESPONSE_TX_AD2_47_32_MASK                                         0x0000ffff

#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_OFFSET                             0x00000004
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_LSB                                16
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_MSB                                20
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_MASK                               0x001f0000

#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_OFFSET                               0x00000004
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_LSB                                  21
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_MSB                                  23
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_MASK                                 0x00e00000

#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_OFFSET                                  0x00000004
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_LSB                                     24
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_MSB                                     24
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_MASK                                    0x01000000

#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_OFFSET                       0x00000004
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_LSB                          25
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_MSB                          25
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_MASK                         0x02000000

#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_OFFSET                         0x00000004
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_LSB                            26
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_MSB                            26
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_MASK                           0x04000000

#define EXPECTED_RESPONSE_RESERVED_1_OFFSET                                         0x00000004
#define EXPECTED_RESPONSE_RESERVED_1_LSB                                            27
#define EXPECTED_RESPONSE_RESERVED_1_MSB                                            31
#define EXPECTED_RESPONSE_RESERVED_1_MASK                                           0xf8000000

#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_OFFSET                          0x00000008
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_LSB                             0
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_MSB                             10
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_MASK                            0x000007ff

#define EXPECTED_RESPONSE_RESERVED_2_OFFSET                                         0x00000008
#define EXPECTED_RESPONSE_RESERVED_2_LSB                                            11
#define EXPECTED_RESPONSE_RESERVED_2_MSB                                            20
#define EXPECTED_RESPONSE_RESERVED_2_MASK                                           0x001ff800

#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_OFFSET                           0x00000008
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_LSB                              21
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_MSB                              31
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_MASK                             0xffe00000

#define EXPECTED_RESPONSE_AST_INDEX_OFFSET                                          0x0000000c
#define EXPECTED_RESPONSE_AST_INDEX_LSB                                             0
#define EXPECTED_RESPONSE_AST_INDEX_MSB                                             15
#define EXPECTED_RESPONSE_AST_INDEX_MASK                                            0x0000ffff

#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_OFFSET                            0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_LSB                               16
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_MSB                               16
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_MASK                              0x00010000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_OFFSET                        0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_LSB                           17
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_MSB                           17
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_MASK                          0x00020000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_OFFSET                        0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_LSB                           18
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_MSB                           18
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_MASK                          0x00040000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_OFFSET                        0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_LSB                           19
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_MSB                           19
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_MASK                          0x00080000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_OFFSET                       0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_LSB                          20
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_MSB                          20
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_MASK                         0x00100000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_OFFSET                       0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_LSB                          21
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_MSB                          21
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_MASK                         0x00200000

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_OFFSET                       0x0000000c
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_LSB                          22
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_MSB                          22
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_MASK                         0x00400000

#define EXPECTED_RESPONSE_RESERVED_3A_OFFSET                                        0x0000000c
#define EXPECTED_RESPONSE_RESERVED_3A_LSB                                           23
#define EXPECTED_RESPONSE_RESERVED_3A_MSB                                           31
#define EXPECTED_RESPONSE_RESERVED_3A_MASK                                          0xff800000

#define EXPECTED_RESPONSE_FCS_OFFSET                                                0x00000010
#define EXPECTED_RESPONSE_FCS_LSB                                                   0
#define EXPECTED_RESPONSE_FCS_MSB                                                   8
#define EXPECTED_RESPONSE_FCS_MASK                                                  0x000001ff

#define EXPECTED_RESPONSE_RESERVED_4A_OFFSET                                        0x00000010
#define EXPECTED_RESPONSE_RESERVED_4A_LSB                                           9
#define EXPECTED_RESPONSE_RESERVED_4A_MSB                                           9
#define EXPECTED_RESPONSE_RESERVED_4A_MASK                                          0x00000200

#define EXPECTED_RESPONSE_CRC_OFFSET                                                0x00000010
#define EXPECTED_RESPONSE_CRC_LSB                                                   10
#define EXPECTED_RESPONSE_CRC_MSB                                                   13
#define EXPECTED_RESPONSE_CRC_MASK                                                  0x00003c00

#define EXPECTED_RESPONSE_SCRAMBLER_SEED_OFFSET                                     0x00000010
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_LSB                                        14
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_MSB                                        20
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_MASK                                       0x001fc000

#define EXPECTED_RESPONSE_RESERVED_4B_OFFSET                                        0x00000010
#define EXPECTED_RESPONSE_RESERVED_4B_LSB                                           21
#define EXPECTED_RESPONSE_RESERVED_4B_MSB                                           31
#define EXPECTED_RESPONSE_RESERVED_4B_MASK                                          0xffe00000

#endif
