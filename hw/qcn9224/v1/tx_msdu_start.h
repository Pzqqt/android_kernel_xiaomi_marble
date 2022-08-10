
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MSDU_START 8

#define NUM_OF_QWORDS_TX_MSDU_START 4


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
             uint32_t tlv64_padding                                           : 32;  
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
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define TX_MSDU_START_MSDU_LEN_OFFSET                                               0x0000000000000000
#define TX_MSDU_START_MSDU_LEN_LSB                                                  0
#define TX_MSDU_START_MSDU_LEN_MSB                                                  13
#define TX_MSDU_START_MSDU_LEN_MASK                                                 0x0000000000003fff


 

#define TX_MSDU_START_FIRST_MSDU_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_FIRST_MSDU_LSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MASK                                               0x0000000000004000


 

#define TX_MSDU_START_LAST_MSDU_OFFSET                                              0x0000000000000000
#define TX_MSDU_START_LAST_MSDU_LSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MASK                                                0x0000000000008000


 

#define TX_MSDU_START_ENCAP_TYPE_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_ENCAP_TYPE_LSB                                                16
#define TX_MSDU_START_ENCAP_TYPE_MSB                                                17
#define TX_MSDU_START_ENCAP_TYPE_MASK                                               0x0000000000030000


 

#define TX_MSDU_START_EPD_EN_OFFSET                                                 0x0000000000000000
#define TX_MSDU_START_EPD_EN_LSB                                                    18
#define TX_MSDU_START_EPD_EN_MSB                                                    18
#define TX_MSDU_START_EPD_EN_MASK                                                   0x0000000000040000


 

#define TX_MSDU_START_DA_SA_PRESENT_OFFSET                                          0x0000000000000000
#define TX_MSDU_START_DA_SA_PRESENT_LSB                                             19
#define TX_MSDU_START_DA_SA_PRESENT_MSB                                             20
#define TX_MSDU_START_DA_SA_PRESENT_MASK                                            0x0000000000180000


 

#define TX_MSDU_START_IPV4_CHECKSUM_EN_OFFSET                                       0x0000000000000000
#define TX_MSDU_START_IPV4_CHECKSUM_EN_LSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MASK                                         0x0000000000200000


 

#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_LSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MASK                                0x0000000000400000


 

#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_LSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MASK                                0x0000000000800000


 

#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_LSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MASK                                0x0000000001000000


 

#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_LSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MASK                                0x0000000002000000


 

#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_OFFSET                                0x0000000000000000
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_LSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MASK                                  0x0000000004000000


 

#define TX_MSDU_START_RESERVED_0A_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_0A_LSB                                               27
#define TX_MSDU_START_RESERVED_0A_MSB                                               31
#define TX_MSDU_START_RESERVED_0A_MASK                                              0x00000000f8000000


 

#define TX_MSDU_START_TSO_ENABLE_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_TSO_ENABLE_LSB                                                32
#define TX_MSDU_START_TSO_ENABLE_MSB                                                32
#define TX_MSDU_START_TSO_ENABLE_MASK                                               0x0000000100000000


 

#define TX_MSDU_START_RESERVED_1A_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_1A_LSB                                               33
#define TX_MSDU_START_RESERVED_1A_MSB                                               38
#define TX_MSDU_START_RESERVED_1A_MASK                                              0x0000007e00000000


 

#define TX_MSDU_START_TCP_FLAG_OFFSET                                               0x0000000000000000
#define TX_MSDU_START_TCP_FLAG_LSB                                                  39
#define TX_MSDU_START_TCP_FLAG_MSB                                                  47
#define TX_MSDU_START_TCP_FLAG_MASK                                                 0x0000ff8000000000


 

#define TX_MSDU_START_TCP_FLAG_MASK_OFFSET                                          0x0000000000000000
#define TX_MSDU_START_TCP_FLAG_MASK_LSB                                             48
#define TX_MSDU_START_TCP_FLAG_MASK_MSB                                             56
#define TX_MSDU_START_TCP_FLAG_MASK_MASK                                            0x01ff000000000000


 

#define TX_MSDU_START_MESH_ENABLE_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_MESH_ENABLE_LSB                                               57
#define TX_MSDU_START_MESH_ENABLE_MSB                                               57
#define TX_MSDU_START_MESH_ENABLE_MASK                                              0x0200000000000000


 

#define TX_MSDU_START_RESERVED_1B_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_1B_LSB                                               58
#define TX_MSDU_START_RESERVED_1B_MSB                                               63
#define TX_MSDU_START_RESERVED_1B_MASK                                              0xfc00000000000000


 

#define TX_MSDU_START_L2_LENGTH_OFFSET                                              0x0000000000000008
#define TX_MSDU_START_L2_LENGTH_LSB                                                 0
#define TX_MSDU_START_L2_LENGTH_MSB                                                 15
#define TX_MSDU_START_L2_LENGTH_MASK                                                0x000000000000ffff


 

#define TX_MSDU_START_IP_LENGTH_OFFSET                                              0x0000000000000008
#define TX_MSDU_START_IP_LENGTH_LSB                                                 16
#define TX_MSDU_START_IP_LENGTH_MSB                                                 31
#define TX_MSDU_START_IP_LENGTH_MASK                                                0x00000000ffff0000


 

#define TX_MSDU_START_TCP_SEQ_NUMBER_OFFSET                                         0x0000000000000008
#define TX_MSDU_START_TCP_SEQ_NUMBER_LSB                                            32
#define TX_MSDU_START_TCP_SEQ_NUMBER_MSB                                            63
#define TX_MSDU_START_TCP_SEQ_NUMBER_MASK                                           0xffffffff00000000


 

#define TX_MSDU_START_IP_IDENTIFICATION_OFFSET                                      0x0000000000000010
#define TX_MSDU_START_IP_IDENTIFICATION_LSB                                         0
#define TX_MSDU_START_IP_IDENTIFICATION_MSB                                         15
#define TX_MSDU_START_IP_IDENTIFICATION_MASK                                        0x000000000000ffff


 

#define TX_MSDU_START_CHECKSUM_OFFSET_OFFSET                                        0x0000000000000010
#define TX_MSDU_START_CHECKSUM_OFFSET_LSB                                           16
#define TX_MSDU_START_CHECKSUM_OFFSET_MSB                                           28
#define TX_MSDU_START_CHECKSUM_OFFSET_MASK                                          0x000000001fff0000


 

#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_OFFSET                                    0x0000000000000010
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_LSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MASK                                      0x0000000020000000


 

#define TX_MSDU_START_RESERVED_4_OFFSET                                             0x0000000000000010
#define TX_MSDU_START_RESERVED_4_LSB                                                30
#define TX_MSDU_START_RESERVED_4_MSB                                                31
#define TX_MSDU_START_RESERVED_4_MASK                                               0x00000000c0000000


 

#define TX_MSDU_START_PAYLOAD_START_OFFSET_OFFSET                                   0x0000000000000010
#define TX_MSDU_START_PAYLOAD_START_OFFSET_LSB                                      32
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MSB                                      45
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MASK                                     0x00003fff00000000


 

#define TX_MSDU_START_RESERVED_5A_OFFSET                                            0x0000000000000010
#define TX_MSDU_START_RESERVED_5A_LSB                                               46
#define TX_MSDU_START_RESERVED_5A_MSB                                               47
#define TX_MSDU_START_RESERVED_5A_MASK                                              0x0000c00000000000


 

#define TX_MSDU_START_PAYLOAD_END_OFFSET_OFFSET                                     0x0000000000000010
#define TX_MSDU_START_PAYLOAD_END_OFFSET_LSB                                        48
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MSB                                        61
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MASK                                       0x3fff000000000000


 

#define TX_MSDU_START_RESERVED_5B_OFFSET                                            0x0000000000000010
#define TX_MSDU_START_RESERVED_5B_LSB                                               62
#define TX_MSDU_START_RESERVED_5B_MSB                                               63
#define TX_MSDU_START_RESERVED_5B_MASK                                              0xc000000000000000


 

#define TX_MSDU_START_UDP_LENGTH_OFFSET                                             0x0000000000000018
#define TX_MSDU_START_UDP_LENGTH_LSB                                                0
#define TX_MSDU_START_UDP_LENGTH_MSB                                                15
#define TX_MSDU_START_UDP_LENGTH_MASK                                               0x000000000000ffff


 

#define TX_MSDU_START_RESERVED_6_OFFSET                                             0x0000000000000018
#define TX_MSDU_START_RESERVED_6_LSB                                                16
#define TX_MSDU_START_RESERVED_6_MSB                                                31
#define TX_MSDU_START_RESERVED_6_MASK                                               0x00000000ffff0000


 

#define TX_MSDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000018
#define TX_MSDU_START_TLV64_PADDING_LSB                                             32
#define TX_MSDU_START_TLV64_PADDING_MSB                                             63
#define TX_MSDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif    
