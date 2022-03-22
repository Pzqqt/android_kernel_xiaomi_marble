
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

 
 
 
 
 
 
 


#ifndef _TX_PEER_ENTRY_H_
#define _TX_PEER_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_PEER_ENTRY 18

#define NUM_OF_QWORDS_TX_PEER_ENTRY 9


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
             uint32_t multi_link_addr_ad1_31_0                                : 32;  
             uint32_t multi_link_addr_ad1_47_32                               : 16,  
                      multi_link_addr_ad2_15_0                                : 16;  
             uint32_t multi_link_addr_ad2_47_16                               : 32;  
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
             uint32_t multi_link_addr_ad1_31_0                                : 32;  
             uint32_t multi_link_addr_ad2_15_0                                : 16,  
                      multi_link_addr_ad1_47_32                               : 16;  
             uint32_t multi_link_addr_ad2_47_16                               : 32;  
             uint32_t sw_peer_id                                              : 16,  
                      reserved_17a                                            : 15,  
                      multi_link_addr_crypto_enable                           :  1;  
#endif
};


 

#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_OFFSET                                        0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_LSB                                           0
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MSB                                           31
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MASK                                          0x00000000ffffffff


 

#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_OFFSET                                       0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_LSB                                          32
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MSB                                          47
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MASK                                         0x0000ffff00000000


 

#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_OFFSET                                        0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_LSB                                           48
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MSB                                           63
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MASK                                          0xffff000000000000


 

#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_OFFSET                                       0x0000000000000008
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_LSB                                          0
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MSB                                          31
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MASK                                         0x00000000ffffffff


 

#define TX_PEER_ENTRY_USE_AD_B_OFFSET                                               0x0000000000000008
#define TX_PEER_ENTRY_USE_AD_B_LSB                                                  32
#define TX_PEER_ENTRY_USE_AD_B_MSB                                                  32
#define TX_PEER_ENTRY_USE_AD_B_MASK                                                 0x0000000100000000


 

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_OFFSET                                0x0000000000000008
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_LSB                                   33
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MSB                                   33
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MASK                                  0x0000000200000000


 

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_OFFSET                                0x0000000000000008
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_LSB                                   34
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MSB                                   34
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MASK                                  0x0000000400000000


 

#define TX_PEER_ENTRY_VLAN_LLC_MODE_OFFSET                                          0x0000000000000008
#define TX_PEER_ENTRY_VLAN_LLC_MODE_LSB                                             35
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MSB                                             35
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MASK                                            0x0000000800000000


 

#define TX_PEER_ENTRY_KEY_TYPE_OFFSET                                               0x0000000000000008
#define TX_PEER_ENTRY_KEY_TYPE_LSB                                                  36
#define TX_PEER_ENTRY_KEY_TYPE_MSB                                                  39
#define TX_PEER_ENTRY_KEY_TYPE_MASK                                                 0x000000f000000000


 

#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_OFFSET                                     0x0000000000000008
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_LSB                                        40
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MSB                                        42
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MASK                                       0x0000070000000000


 

#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_OFFSET                                    0x0000000000000008
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_LSB                                       43
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MSB                                       43
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MASK                                      0x0000080000000000


 

#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_OFFSET                                    0x0000000000000008
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_LSB                                       44
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MSB                                       44
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MASK                                      0x0000100000000000


 

#define TX_PEER_ENTRY_EPD_OUTPUT_OFFSET                                             0x0000000000000008
#define TX_PEER_ENTRY_EPD_OUTPUT_LSB                                                45
#define TX_PEER_ENTRY_EPD_OUTPUT_MSB                                                45
#define TX_PEER_ENTRY_EPD_OUTPUT_MASK                                               0x0000200000000000


 

#define TX_PEER_ENTRY_WDS_OFFSET                                                    0x0000000000000008
#define TX_PEER_ENTRY_WDS_LSB                                                       46
#define TX_PEER_ENTRY_WDS_MSB                                                       46
#define TX_PEER_ENTRY_WDS_MASK                                                      0x0000400000000000


 

#define TX_PEER_ENTRY_INSERT_OR_STRIP_OFFSET                                        0x0000000000000008
#define TX_PEER_ENTRY_INSERT_OR_STRIP_LSB                                           47
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MSB                                           47
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MASK                                          0x0000800000000000


 

#define TX_PEER_ENTRY_SW_FILTER_ID_OFFSET                                           0x0000000000000008
#define TX_PEER_ENTRY_SW_FILTER_ID_LSB                                              48
#define TX_PEER_ENTRY_SW_FILTER_ID_MSB                                              63
#define TX_PEER_ENTRY_SW_FILTER_ID_MASK                                             0xffff000000000000


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_OFFSET                                      0x0000000000000010
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_LSB                                         0
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MSB                                         31
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MASK                                        0x00000000ffffffff


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_OFFSET                                     0x0000000000000010
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_LSB                                        32
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MSB                                        63
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MASK                                       0xffffffff00000000


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_OFFSET                                     0x0000000000000018
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_LSB                                        0
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MSB                                        31
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MASK                                       0x00000000ffffffff


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_OFFSET                                    0x0000000000000018
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_LSB                                       32
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MSB                                       63
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MASK                                      0xffffffff00000000


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_OFFSET                                   0x0000000000000020
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MASK                                     0x00000000ffffffff


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_OFFSET                                   0x0000000000000020
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_LSB                                      32
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MSB                                      63
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MASK                                     0xffffffff00000000


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_OFFSET                                   0x0000000000000028
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MASK                                     0x00000000ffffffff


 

#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_OFFSET                                   0x0000000000000028
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_LSB                                      32
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MSB                                      63
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MASK                                     0xffffffff00000000


 

#define TX_PEER_ENTRY_STA_PARTIAL_AID_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_STA_PARTIAL_AID_LSB                                           0
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MSB                                           10
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MASK                                          0x00000000000007ff


 

#define TX_PEER_ENTRY_TRANSMIT_VIF_OFFSET                                           0x0000000000000030
#define TX_PEER_ENTRY_TRANSMIT_VIF_LSB                                              11
#define TX_PEER_ENTRY_TRANSMIT_VIF_MSB                                              14
#define TX_PEER_ENTRY_TRANSMIT_VIF_MASK                                             0x0000000000007800


 

#define TX_PEER_ENTRY_BLOCK_THIS_USER_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_BLOCK_THIS_USER_LSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MASK                                          0x0000000000008000


 

#define TX_PEER_ENTRY_MESH_AMSDU_MODE_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_LSB                                           16
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MSB                                           17
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MASK                                          0x0000000000030000


 

#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_LSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MASK                                    0x0000000000040000


 

#define TX_PEER_ENTRY_DL_UL_DIRECTION_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_DL_UL_DIRECTION_LSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MASK                                          0x0000000000080000


 

#define TX_PEER_ENTRY_RESERVED_12_OFFSET                                            0x0000000000000030
#define TX_PEER_ENTRY_RESERVED_12_LSB                                               20
#define TX_PEER_ENTRY_RESERVED_12_MSB                                               31
#define TX_PEER_ENTRY_RESERVED_12_MASK                                              0x00000000fff00000


 

#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_LSB                                     32
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MSB                                     47
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MASK                                    0x0000ffff00000000


 

#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_LSB                                     48
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MSB                                     63
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MASK                                    0xffff000000000000


 

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_OFFSET                               0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_LSB                                  0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_MSB                                  31
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_MASK                                 0x00000000ffffffff


 

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_OFFSET                              0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_LSB                                 32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_MSB                                 47
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_MASK                                0x0000ffff00000000


 

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_OFFSET                               0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_LSB                                  48
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_MSB                                  63
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_MASK                                 0xffff000000000000


 

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_OFFSET                              0x0000000000000040
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_LSB                                 0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_MSB                                 31
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_MASK                                0x00000000ffffffff


 

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_OFFSET                          0x0000000000000040
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_LSB                             32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MSB                             32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MASK                            0x0000000100000000


 

#define TX_PEER_ENTRY_RESERVED_17A_OFFSET                                           0x0000000000000040
#define TX_PEER_ENTRY_RESERVED_17A_LSB                                              33
#define TX_PEER_ENTRY_RESERVED_17A_MSB                                              47
#define TX_PEER_ENTRY_RESERVED_17A_MASK                                             0x0000fffe00000000


 

#define TX_PEER_ENTRY_SW_PEER_ID_OFFSET                                             0x0000000000000040
#define TX_PEER_ENTRY_SW_PEER_ID_LSB                                                48
#define TX_PEER_ENTRY_SW_PEER_ID_MSB                                                63
#define TX_PEER_ENTRY_SW_PEER_ID_MASK                                               0xffff000000000000



#endif    
