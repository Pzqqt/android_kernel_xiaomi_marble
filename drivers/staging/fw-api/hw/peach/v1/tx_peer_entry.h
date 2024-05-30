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


#ifndef _TX_PEER_ENTRY_H_
#define _TX_PEER_ENTRY_H_

#define NUM_OF_DWORDS_TX_PEER_ENTRY 18

struct tx_peer_entry {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mac_addr_a_31_0                                         : 32;
             uint32_t mac_addr_a_47_32                                        : 16,
                      mac_addr_b_15_0                                         : 16;
             uint32_t mac_addr_b_47_16                                        : 32;
             uint32_t use_ad_b                                                :  1,
                      strip_insert_vlan_inner                                 :  1,
                      strip_insert_vlan_outer                                 :  1,
                      vlan_llc_mode                                           :  1,
                      key_type                                                :  4,
                      a_msdu_wds_ad3_ad4                                      :  3,
                      ignore_hard_filters                                     :  1,
                      ignore_soft_filters                                     :  1,
                      epd_output                                              :  1,
                      wds                                                     :  1,
                      insert_or_strip                                         :  1,
                      sw_filter_id                                            : 16;
             uint32_t temporal_key_31_0                                       : 32;
             uint32_t temporal_key_63_32                                      : 32;
             uint32_t temporal_key_95_64                                      : 32;
             uint32_t temporal_key_127_96                                     : 32;
             uint32_t temporal_key_159_128                                    : 32;
             uint32_t temporal_key_191_160                                    : 32;
             uint32_t temporal_key_223_192                                    : 32;
             uint32_t temporal_key_255_224                                    : 32;
             uint32_t sta_partial_aid                                         : 11,
                      transmit_vif                                            :  4,
                      block_this_user                                         :  1,
                      mesh_amsdu_mode                                         :  2,
                      use_qos_alt_mute_mask                                   :  1,
                      dl_ul_direction                                         :  1,
                      reserved_12                                             : 12;
             uint32_t insert_vlan_outer_tci                                   : 16,
                      insert_vlan_inner_tci                                   : 16;
             uint32_t __reserved_g_0007                                       : 32;
             uint32_t __reserved_g_0008                                       : 16,
                      __reserved_g_0009                                       : 16;
             uint32_t __reserved_g_0010                                       : 32;
             uint32_t multi_link_addr_crypto_enable                           :  1,
                      reserved_17a                                            : 15,
                      sw_peer_id                                              : 16;
#else
             uint32_t mac_addr_a_31_0                                         : 32;
             uint32_t mac_addr_b_15_0                                         : 16,
                      mac_addr_a_47_32                                        : 16;
             uint32_t mac_addr_b_47_16                                        : 32;
             uint32_t sw_filter_id                                            : 16,
                      insert_or_strip                                         :  1,
                      wds                                                     :  1,
                      epd_output                                              :  1,
                      ignore_soft_filters                                     :  1,
                      ignore_hard_filters                                     :  1,
                      a_msdu_wds_ad3_ad4                                      :  3,
                      key_type                                                :  4,
                      vlan_llc_mode                                           :  1,
                      strip_insert_vlan_outer                                 :  1,
                      strip_insert_vlan_inner                                 :  1,
                      use_ad_b                                                :  1;
             uint32_t temporal_key_31_0                                       : 32;
             uint32_t temporal_key_63_32                                      : 32;
             uint32_t temporal_key_95_64                                      : 32;
             uint32_t temporal_key_127_96                                     : 32;
             uint32_t temporal_key_159_128                                    : 32;
             uint32_t temporal_key_191_160                                    : 32;
             uint32_t temporal_key_223_192                                    : 32;
             uint32_t temporal_key_255_224                                    : 32;
             uint32_t reserved_12                                             : 12,
                      dl_ul_direction                                         :  1,
                      use_qos_alt_mute_mask                                   :  1,
                      mesh_amsdu_mode                                         :  2,
                      block_this_user                                         :  1,
                      transmit_vif                                            :  4,
                      sta_partial_aid                                         : 11;
             uint32_t insert_vlan_inner_tci                                   : 16,
                      insert_vlan_outer_tci                                   : 16;
             uint32_t __reserved_g_0007                                       : 32;
             uint32_t __reserved_g_0009                                       : 16,
                      __reserved_g_0008                                       : 16;
             uint32_t __reserved_g_0010                                       : 32;
             uint32_t sw_peer_id                                              : 16,
                      reserved_17a                                            : 15,
                      multi_link_addr_crypto_enable                           :  1;
#endif
};

#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_OFFSET                                        0x00000000
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_LSB                                           0
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MSB                                           31
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MASK                                          0xffffffff

#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_OFFSET                                       0x00000004
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_LSB                                          0
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MSB                                          15
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MASK                                         0x0000ffff

#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_OFFSET                                        0x00000004
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_LSB                                           16
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MSB                                           31
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MASK                                          0xffff0000

#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_OFFSET                                       0x00000008
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_LSB                                          0
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MSB                                          31
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MASK                                         0xffffffff

#define TX_PEER_ENTRY_USE_AD_B_OFFSET                                               0x0000000c
#define TX_PEER_ENTRY_USE_AD_B_LSB                                                  0
#define TX_PEER_ENTRY_USE_AD_B_MSB                                                  0
#define TX_PEER_ENTRY_USE_AD_B_MASK                                                 0x00000001

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_OFFSET                                0x0000000c
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_LSB                                   1
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MSB                                   1
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MASK                                  0x00000002

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_OFFSET                                0x0000000c
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_LSB                                   2
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MSB                                   2
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MASK                                  0x00000004

#define TX_PEER_ENTRY_VLAN_LLC_MODE_OFFSET                                          0x0000000c
#define TX_PEER_ENTRY_VLAN_LLC_MODE_LSB                                             3
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MSB                                             3
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MASK                                            0x00000008

#define TX_PEER_ENTRY_KEY_TYPE_OFFSET                                               0x0000000c
#define TX_PEER_ENTRY_KEY_TYPE_LSB                                                  4
#define TX_PEER_ENTRY_KEY_TYPE_MSB                                                  7
#define TX_PEER_ENTRY_KEY_TYPE_MASK                                                 0x000000f0

#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_OFFSET                                     0x0000000c
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_LSB                                        8
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MSB                                        10
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MASK                                       0x00000700

#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_OFFSET                                    0x0000000c
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_LSB                                       11
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MSB                                       11
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MASK                                      0x00000800

#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_OFFSET                                    0x0000000c
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_LSB                                       12
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MSB                                       12
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MASK                                      0x00001000

#define TX_PEER_ENTRY_EPD_OUTPUT_OFFSET                                             0x0000000c
#define TX_PEER_ENTRY_EPD_OUTPUT_LSB                                                13
#define TX_PEER_ENTRY_EPD_OUTPUT_MSB                                                13
#define TX_PEER_ENTRY_EPD_OUTPUT_MASK                                               0x00002000

#define TX_PEER_ENTRY_WDS_OFFSET                                                    0x0000000c
#define TX_PEER_ENTRY_WDS_LSB                                                       14
#define TX_PEER_ENTRY_WDS_MSB                                                       14
#define TX_PEER_ENTRY_WDS_MASK                                                      0x00004000

#define TX_PEER_ENTRY_INSERT_OR_STRIP_OFFSET                                        0x0000000c
#define TX_PEER_ENTRY_INSERT_OR_STRIP_LSB                                           15
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MSB                                           15
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MASK                                          0x00008000

#define TX_PEER_ENTRY_SW_FILTER_ID_OFFSET                                           0x0000000c
#define TX_PEER_ENTRY_SW_FILTER_ID_LSB                                              16
#define TX_PEER_ENTRY_SW_FILTER_ID_MSB                                              31
#define TX_PEER_ENTRY_SW_FILTER_ID_MASK                                             0xffff0000

#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_OFFSET                                      0x00000010
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_LSB                                         0
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MSB                                         31
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MASK                                        0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_OFFSET                                     0x00000014
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_LSB                                        0
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MSB                                        31
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MASK                                       0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_OFFSET                                     0x00000018
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_LSB                                        0
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MSB                                        31
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MASK                                       0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_OFFSET                                    0x0000001c
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_LSB                                       0
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MSB                                       31
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MASK                                      0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_OFFSET                                   0x00000020
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MASK                                     0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_OFFSET                                   0x00000024
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MASK                                     0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_OFFSET                                   0x00000028
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MASK                                     0xffffffff

#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_OFFSET                                   0x0000002c
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MASK                                     0xffffffff

#define TX_PEER_ENTRY_STA_PARTIAL_AID_OFFSET                                        0x00000030
#define TX_PEER_ENTRY_STA_PARTIAL_AID_LSB                                           0
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MSB                                           10
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MASK                                          0x000007ff

#define TX_PEER_ENTRY_TRANSMIT_VIF_OFFSET                                           0x00000030
#define TX_PEER_ENTRY_TRANSMIT_VIF_LSB                                              11
#define TX_PEER_ENTRY_TRANSMIT_VIF_MSB                                              14
#define TX_PEER_ENTRY_TRANSMIT_VIF_MASK                                             0x00007800

#define TX_PEER_ENTRY_BLOCK_THIS_USER_OFFSET                                        0x00000030
#define TX_PEER_ENTRY_BLOCK_THIS_USER_LSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MASK                                          0x00008000

#define TX_PEER_ENTRY_MESH_AMSDU_MODE_OFFSET                                        0x00000030
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_LSB                                           16
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MSB                                           17
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MASK                                          0x00030000

#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_OFFSET                                  0x00000030
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_LSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MASK                                    0x00040000

#define TX_PEER_ENTRY_DL_UL_DIRECTION_OFFSET                                        0x00000030
#define TX_PEER_ENTRY_DL_UL_DIRECTION_LSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MASK                                          0x00080000

#define TX_PEER_ENTRY_RESERVED_12_OFFSET                                            0x00000030
#define TX_PEER_ENTRY_RESERVED_12_LSB                                               20
#define TX_PEER_ENTRY_RESERVED_12_MSB                                               31
#define TX_PEER_ENTRY_RESERVED_12_MASK                                              0xfff00000

#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_OFFSET                                  0x00000034
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_LSB                                     0
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MSB                                     15
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MASK                                    0x0000ffff

#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_OFFSET                                  0x00000034
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_LSB                                     16
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MSB                                     31
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MASK                                    0xffff0000

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_OFFSET                          0x00000044
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_LSB                             0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MSB                             0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MASK                            0x00000001

#define TX_PEER_ENTRY_RESERVED_17A_OFFSET                                           0x00000044
#define TX_PEER_ENTRY_RESERVED_17A_LSB                                              1
#define TX_PEER_ENTRY_RESERVED_17A_MSB                                              15
#define TX_PEER_ENTRY_RESERVED_17A_MASK                                             0x0000fffe

#define TX_PEER_ENTRY_SW_PEER_ID_OFFSET                                             0x00000044
#define TX_PEER_ENTRY_SW_PEER_ID_LSB                                                16
#define TX_PEER_ENTRY_SW_PEER_ID_MSB                                                31
#define TX_PEER_ENTRY_SW_PEER_ID_MASK                                               0xffff0000

#endif
