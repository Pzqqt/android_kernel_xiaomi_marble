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


#ifndef _TX_MSDU_START_H_
#define _TX_MSDU_START_H_

#define NUM_OF_DWORDS_TX_MSDU_START 7

struct tx_msdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t msdu_len                                                : 14,
                      first_msdu                                              :  1,
                      last_msdu                                               :  1,
                      encap_type                                              :  2,
                      epd_en                                                  :  1,
                      da_sa_present                                           :  2,
                      ipv4_checksum_en                                        :  1,
                      udp_over_ipv4_checksum_en                               :  1,
                      udp_over_ipv6_checksum_en                               :  1,
                      tcp_over_ipv4_checksum_en                               :  1,
                      tcp_over_ipv6_checksum_en                               :  1,
                      dummy_msdu_delimitation                                 :  1,
                      reserved_0a                                             :  5;
             uint32_t tso_enable                                              :  1,
                      reserved_1a                                             :  6,
                      tcp_flag                                                :  9,
                      tcp_flag_mask                                           :  9,
                      mesh_enable                                             :  1,
                      reserved_1b                                             :  6;
             uint32_t l2_length                                               : 16,
                      ip_length                                               : 16;
             uint32_t tcp_seq_number                                          : 32;
             uint32_t ip_identification                                       : 16,
                      checksum_offset                                         : 13,
                      partial_checksum_en                                     :  1,
                      reserved_4                                              :  2;
             uint32_t payload_start_offset                                    : 14,
                      reserved_5a                                             :  2,
                      payload_end_offset                                      : 14,
                      reserved_5b                                             :  2;
             uint32_t udp_length                                              : 16,
                      reserved_6                                              : 16;
#else
             uint32_t reserved_0a                                             :  5,
                      dummy_msdu_delimitation                                 :  1,
                      tcp_over_ipv6_checksum_en                               :  1,
                      tcp_over_ipv4_checksum_en                               :  1,
                      udp_over_ipv6_checksum_en                               :  1,
                      udp_over_ipv4_checksum_en                               :  1,
                      ipv4_checksum_en                                        :  1,
                      da_sa_present                                           :  2,
                      epd_en                                                  :  1,
                      encap_type                                              :  2,
                      last_msdu                                               :  1,
                      first_msdu                                              :  1,
                      msdu_len                                                : 14;
             uint32_t reserved_1b                                             :  6,
                      mesh_enable                                             :  1,
                      tcp_flag_mask                                           :  9,
                      tcp_flag                                                :  9,
                      reserved_1a                                             :  6,
                      tso_enable                                              :  1;
             uint32_t ip_length                                               : 16,
                      l2_length                                               : 16;
             uint32_t tcp_seq_number                                          : 32;
             uint32_t reserved_4                                              :  2,
                      partial_checksum_en                                     :  1,
                      checksum_offset                                         : 13,
                      ip_identification                                       : 16;
             uint32_t reserved_5b                                             :  2,
                      payload_end_offset                                      : 14,
                      reserved_5a                                             :  2,
                      payload_start_offset                                    : 14;
             uint32_t reserved_6                                              : 16,
                      udp_length                                              : 16;
#endif
};

#define TX_MSDU_START_MSDU_LEN_OFFSET                                               0x00000000
#define TX_MSDU_START_MSDU_LEN_LSB                                                  0
#define TX_MSDU_START_MSDU_LEN_MSB                                                  13
#define TX_MSDU_START_MSDU_LEN_MASK                                                 0x00003fff

#define TX_MSDU_START_FIRST_MSDU_OFFSET                                             0x00000000
#define TX_MSDU_START_FIRST_MSDU_LSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MASK                                               0x00004000

#define TX_MSDU_START_LAST_MSDU_OFFSET                                              0x00000000
#define TX_MSDU_START_LAST_MSDU_LSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MASK                                                0x00008000

#define TX_MSDU_START_ENCAP_TYPE_OFFSET                                             0x00000000
#define TX_MSDU_START_ENCAP_TYPE_LSB                                                16
#define TX_MSDU_START_ENCAP_TYPE_MSB                                                17
#define TX_MSDU_START_ENCAP_TYPE_MASK                                               0x00030000

#define TX_MSDU_START_EPD_EN_OFFSET                                                 0x00000000
#define TX_MSDU_START_EPD_EN_LSB                                                    18
#define TX_MSDU_START_EPD_EN_MSB                                                    18
#define TX_MSDU_START_EPD_EN_MASK                                                   0x00040000

#define TX_MSDU_START_DA_SA_PRESENT_OFFSET                                          0x00000000
#define TX_MSDU_START_DA_SA_PRESENT_LSB                                             19
#define TX_MSDU_START_DA_SA_PRESENT_MSB                                             20
#define TX_MSDU_START_DA_SA_PRESENT_MASK                                            0x00180000

#define TX_MSDU_START_IPV4_CHECKSUM_EN_OFFSET                                       0x00000000
#define TX_MSDU_START_IPV4_CHECKSUM_EN_LSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MASK                                         0x00200000

#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x00000000
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_LSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MASK                                0x00400000

#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x00000000
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_LSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MASK                                0x00800000

#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x00000000
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_LSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MASK                                0x01000000

#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x00000000
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_LSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MASK                                0x02000000

#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_OFFSET                                0x00000000
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_LSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MASK                                  0x04000000

#define TX_MSDU_START_RESERVED_0A_OFFSET                                            0x00000000
#define TX_MSDU_START_RESERVED_0A_LSB                                               27
#define TX_MSDU_START_RESERVED_0A_MSB                                               31
#define TX_MSDU_START_RESERVED_0A_MASK                                              0xf8000000

#define TX_MSDU_START_TSO_ENABLE_OFFSET                                             0x00000004
#define TX_MSDU_START_TSO_ENABLE_LSB                                                0
#define TX_MSDU_START_TSO_ENABLE_MSB                                                0
#define TX_MSDU_START_TSO_ENABLE_MASK                                               0x00000001

#define TX_MSDU_START_RESERVED_1A_OFFSET                                            0x00000004
#define TX_MSDU_START_RESERVED_1A_LSB                                               1
#define TX_MSDU_START_RESERVED_1A_MSB                                               6
#define TX_MSDU_START_RESERVED_1A_MASK                                              0x0000007e

#define TX_MSDU_START_TCP_FLAG_OFFSET                                               0x00000004
#define TX_MSDU_START_TCP_FLAG_LSB                                                  7
#define TX_MSDU_START_TCP_FLAG_MSB                                                  15
#define TX_MSDU_START_TCP_FLAG_MASK                                                 0x0000ff80

#define TX_MSDU_START_TCP_FLAG_MASK_OFFSET                                          0x00000004
#define TX_MSDU_START_TCP_FLAG_MASK_LSB                                             16
#define TX_MSDU_START_TCP_FLAG_MASK_MSB                                             24
#define TX_MSDU_START_TCP_FLAG_MASK_MASK                                            0x01ff0000

#define TX_MSDU_START_MESH_ENABLE_OFFSET                                            0x00000004
#define TX_MSDU_START_MESH_ENABLE_LSB                                               25
#define TX_MSDU_START_MESH_ENABLE_MSB                                               25
#define TX_MSDU_START_MESH_ENABLE_MASK                                              0x02000000

#define TX_MSDU_START_RESERVED_1B_OFFSET                                            0x00000004
#define TX_MSDU_START_RESERVED_1B_LSB                                               26
#define TX_MSDU_START_RESERVED_1B_MSB                                               31
#define TX_MSDU_START_RESERVED_1B_MASK                                              0xfc000000

#define TX_MSDU_START_L2_LENGTH_OFFSET                                              0x00000008
#define TX_MSDU_START_L2_LENGTH_LSB                                                 0
#define TX_MSDU_START_L2_LENGTH_MSB                                                 15
#define TX_MSDU_START_L2_LENGTH_MASK                                                0x0000ffff

#define TX_MSDU_START_IP_LENGTH_OFFSET                                              0x00000008
#define TX_MSDU_START_IP_LENGTH_LSB                                                 16
#define TX_MSDU_START_IP_LENGTH_MSB                                                 31
#define TX_MSDU_START_IP_LENGTH_MASK                                                0xffff0000

#define TX_MSDU_START_TCP_SEQ_NUMBER_OFFSET                                         0x0000000c
#define TX_MSDU_START_TCP_SEQ_NUMBER_LSB                                            0
#define TX_MSDU_START_TCP_SEQ_NUMBER_MSB                                            31
#define TX_MSDU_START_TCP_SEQ_NUMBER_MASK                                           0xffffffff

#define TX_MSDU_START_IP_IDENTIFICATION_OFFSET                                      0x00000010
#define TX_MSDU_START_IP_IDENTIFICATION_LSB                                         0
#define TX_MSDU_START_IP_IDENTIFICATION_MSB                                         15
#define TX_MSDU_START_IP_IDENTIFICATION_MASK                                        0x0000ffff

#define TX_MSDU_START_CHECKSUM_OFFSET_OFFSET                                        0x00000010
#define TX_MSDU_START_CHECKSUM_OFFSET_LSB                                           16
#define TX_MSDU_START_CHECKSUM_OFFSET_MSB                                           28
#define TX_MSDU_START_CHECKSUM_OFFSET_MASK                                          0x1fff0000

#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_OFFSET                                    0x00000010
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_LSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MASK                                      0x20000000

#define TX_MSDU_START_RESERVED_4_OFFSET                                             0x00000010
#define TX_MSDU_START_RESERVED_4_LSB                                                30
#define TX_MSDU_START_RESERVED_4_MSB                                                31
#define TX_MSDU_START_RESERVED_4_MASK                                               0xc0000000

#define TX_MSDU_START_PAYLOAD_START_OFFSET_OFFSET                                   0x00000014
#define TX_MSDU_START_PAYLOAD_START_OFFSET_LSB                                      0
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MSB                                      13
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MASK                                     0x00003fff

#define TX_MSDU_START_RESERVED_5A_OFFSET                                            0x00000014
#define TX_MSDU_START_RESERVED_5A_LSB                                               14
#define TX_MSDU_START_RESERVED_5A_MSB                                               15
#define TX_MSDU_START_RESERVED_5A_MASK                                              0x0000c000

#define TX_MSDU_START_PAYLOAD_END_OFFSET_OFFSET                                     0x00000014
#define TX_MSDU_START_PAYLOAD_END_OFFSET_LSB                                        16
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MSB                                        29
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MASK                                       0x3fff0000

#define TX_MSDU_START_RESERVED_5B_OFFSET                                            0x00000014
#define TX_MSDU_START_RESERVED_5B_LSB                                               30
#define TX_MSDU_START_RESERVED_5B_MSB                                               31
#define TX_MSDU_START_RESERVED_5B_MASK                                              0xc0000000

#define TX_MSDU_START_UDP_LENGTH_OFFSET                                             0x00000018
#define TX_MSDU_START_UDP_LENGTH_LSB                                                0
#define TX_MSDU_START_UDP_LENGTH_MSB                                                15
#define TX_MSDU_START_UDP_LENGTH_MASK                                               0x0000ffff

#define TX_MSDU_START_RESERVED_6_OFFSET                                             0x00000018
#define TX_MSDU_START_RESERVED_6_LSB                                                16
#define TX_MSDU_START_RESERVED_6_MSB                                                31
#define TX_MSDU_START_RESERVED_6_MASK                                               0xffff0000

#endif
