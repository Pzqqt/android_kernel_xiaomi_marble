
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _TX_QUEUE_EXTENSION_H_
#define _TX_QUEUE_EXTENSION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_QUEUE_EXTENSION 14

#define NUM_OF_QWORDS_TX_QUEUE_EXTENSION 7


struct tx_queue_extension {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t frame_ctl                                               : 16,  
                      qos_ctl                                                 : 16;  
             uint32_t ampdu_flag                                              :  1,  
                      tx_notify_no_htc_override                               :  1,  
                      reserved_1a                                             :  7,  
                      checksum_tso_disable_for_frag                           :  1,  
                      key_id                                                  :  8,  
                      qos_buf_state_overwrite                                 :  1,  
                      buf_state_sta_id                                        :  1,  
                      buf_state_source                                        :  1,  
                      ht_control_overwrite_enable                             :  1,  
                      ht_control_overwrite_source                             :  4,  
                      reserved_1b                                             :  6;  
             uint32_t ul_headroom_insertion_enable                            :  1,  
                      ul_headroom_offset                                      :  5,  
                      bqrp_insertion_enable                                   :  1,  
                      bqrp_offset                                             :  5,  
                      ul_headroom_rsvd_7_6                                    :  2,  
                      bqr_rsvd_9_8                                            :  2,  
                      base_pn_63_48                                           : 16;  
             uint32_t base_pn_95_64                                           : 32;  
             uint32_t base_pn_127_96                                          : 32;  
             uint32_t ht_control_field_bw20                                   : 32;  
             uint32_t ht_control_field_bw40                                   : 32;  
             uint32_t ht_control_field_bw80                                   : 32;  
             uint32_t ht_control_field_bw160                                  : 32;  
             uint32_t ht_control_overwrite_mask                               : 32;  
             uint32_t cas_control_info                                        :  8,  
                      cas_offset                                              :  5,  
                      cas_insertion_enable                                    :  1,  
                      reserved_10a                                            :  2,  
                      ht_control_overwrite_source_for_srp                     :  4,  
                      ht_control_overwrite_source_for_bsrp                    :  4,  
                      reserved_10b                                            :  6,  
                      mpdu_hdr_len_override_en                                :  1,  
                      bar_ssn_overwrite_enable                                :  1;  
             uint32_t bar_ssn_offset                                          : 12,  
                      mpdu_hdr_len_override_val                               :  9,  
                      reserved_11a                                            : 11;  
             uint32_t ht_control_field_bw320                                  : 32;  
             uint32_t fw2sw_info                                              : 32;  
#else
             uint32_t qos_ctl                                                 : 16,  
                      frame_ctl                                               : 16;  
             uint32_t reserved_1b                                             :  6,  
                      ht_control_overwrite_source                             :  4,  
                      ht_control_overwrite_enable                             :  1,  
                      buf_state_source                                        :  1,  
                      buf_state_sta_id                                        :  1,  
                      qos_buf_state_overwrite                                 :  1,  
                      key_id                                                  :  8,  
                      checksum_tso_disable_for_frag                           :  1,  
                      reserved_1a                                             :  7,  
                      tx_notify_no_htc_override                               :  1,  
                      ampdu_flag                                              :  1;  
             uint32_t base_pn_63_48                                           : 16,  
                      bqr_rsvd_9_8                                            :  2,  
                      ul_headroom_rsvd_7_6                                    :  2,  
                      bqrp_offset                                             :  5,  
                      bqrp_insertion_enable                                   :  1,  
                      ul_headroom_offset                                      :  5,  
                      ul_headroom_insertion_enable                            :  1;  
             uint32_t base_pn_95_64                                           : 32;  
             uint32_t base_pn_127_96                                          : 32;  
             uint32_t ht_control_field_bw20                                   : 32;  
             uint32_t ht_control_field_bw40                                   : 32;  
             uint32_t ht_control_field_bw80                                   : 32;  
             uint32_t ht_control_field_bw160                                  : 32;  
             uint32_t ht_control_overwrite_mask                               : 32;  
             uint32_t bar_ssn_overwrite_enable                                :  1,  
                      mpdu_hdr_len_override_en                                :  1,  
                      reserved_10b                                            :  6,  
                      ht_control_overwrite_source_for_bsrp                    :  4,  
                      ht_control_overwrite_source_for_srp                     :  4,  
                      reserved_10a                                            :  2,  
                      cas_insertion_enable                                    :  1,  
                      cas_offset                                              :  5,  
                      cas_control_info                                        :  8;  
             uint32_t reserved_11a                                            : 11,  
                      mpdu_hdr_len_override_val                               :  9,  
                      bar_ssn_offset                                          : 12;  
             uint32_t ht_control_field_bw320                                  : 32;  
             uint32_t fw2sw_info                                              : 32;  
#endif
};


 

#define TX_QUEUE_EXTENSION_FRAME_CTL_OFFSET                                         0x0000000000000000
#define TX_QUEUE_EXTENSION_FRAME_CTL_LSB                                            0
#define TX_QUEUE_EXTENSION_FRAME_CTL_MSB                                            15
#define TX_QUEUE_EXTENSION_FRAME_CTL_MASK                                           0x000000000000ffff


 

#define TX_QUEUE_EXTENSION_QOS_CTL_OFFSET                                           0x0000000000000000
#define TX_QUEUE_EXTENSION_QOS_CTL_LSB                                              16
#define TX_QUEUE_EXTENSION_QOS_CTL_MSB                                              31
#define TX_QUEUE_EXTENSION_QOS_CTL_MASK                                             0x00000000ffff0000


 

#define TX_QUEUE_EXTENSION_AMPDU_FLAG_OFFSET                                        0x0000000000000000
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_LSB                                           32
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_MSB                                           32
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_MASK                                          0x0000000100000000


 

#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_OFFSET                         0x0000000000000000
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_LSB                            33
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_MSB                            33
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_MASK                           0x0000000200000000


 

#define TX_QUEUE_EXTENSION_RESERVED_1A_OFFSET                                       0x0000000000000000
#define TX_QUEUE_EXTENSION_RESERVED_1A_LSB                                          34
#define TX_QUEUE_EXTENSION_RESERVED_1A_MSB                                          40
#define TX_QUEUE_EXTENSION_RESERVED_1A_MASK                                         0x000001fc00000000


 

#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_OFFSET                     0x0000000000000000
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_LSB                        41
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_MSB                        41
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_MASK                       0x0000020000000000


 

#define TX_QUEUE_EXTENSION_KEY_ID_OFFSET                                            0x0000000000000000
#define TX_QUEUE_EXTENSION_KEY_ID_LSB                                               42
#define TX_QUEUE_EXTENSION_KEY_ID_MSB                                               49
#define TX_QUEUE_EXTENSION_KEY_ID_MASK                                              0x0003fc0000000000


 

#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_OFFSET                           0x0000000000000000
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_LSB                              50
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_MSB                              50
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_MASK                             0x0004000000000000


 

#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_OFFSET                                  0x0000000000000000
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_LSB                                     51
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_MSB                                     51
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_MASK                                    0x0008000000000000


 

#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_OFFSET                                  0x0000000000000000
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_LSB                                     52
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_MSB                                     52
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_MASK                                    0x0010000000000000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_OFFSET                       0x0000000000000000
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_LSB                          53
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_MSB                          53
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_MASK                         0x0020000000000000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_OFFSET                       0x0000000000000000
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_LSB                          54
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_MSB                          57
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_MASK                         0x03c0000000000000


 

#define TX_QUEUE_EXTENSION_RESERVED_1B_OFFSET                                       0x0000000000000000
#define TX_QUEUE_EXTENSION_RESERVED_1B_LSB                                          58
#define TX_QUEUE_EXTENSION_RESERVED_1B_MSB                                          63
#define TX_QUEUE_EXTENSION_RESERVED_1B_MASK                                         0xfc00000000000000


 

#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_OFFSET                      0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_LSB                         0
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_MSB                         0
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_MASK                        0x0000000000000001


 

#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_OFFSET                                0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_LSB                                   1
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_MSB                                   5
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_MASK                                  0x000000000000003e


 

#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_OFFSET                             0x0000000000000008
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_LSB                                6
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_MSB                                6
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_MASK                               0x0000000000000040


 

#define TX_QUEUE_EXTENSION_BQRP_OFFSET_OFFSET                                       0x0000000000000008
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_LSB                                          7
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_MSB                                          11
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_MASK                                         0x0000000000000f80


 

#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_OFFSET                              0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_LSB                                 12
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_MSB                                 13
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_MASK                                0x0000000000003000


 

#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_OFFSET                                      0x0000000000000008
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_LSB                                         14
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_MSB                                         15
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_MASK                                        0x000000000000c000


 

#define TX_QUEUE_EXTENSION_BASE_PN_63_48_OFFSET                                     0x0000000000000008
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_LSB                                        16
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_MSB                                        31
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_MASK                                       0x00000000ffff0000


 

#define TX_QUEUE_EXTENSION_BASE_PN_95_64_OFFSET                                     0x0000000000000008
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_LSB                                        32
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_MSB                                        63
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_MASK                                       0xffffffff00000000


 

#define TX_QUEUE_EXTENSION_BASE_PN_127_96_OFFSET                                    0x0000000000000010
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_LSB                                       0
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_MSB                                       31
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_MASK                                      0x00000000ffffffff


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_OFFSET                             0x0000000000000010
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_LSB                                32
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_MSB                                63
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_MASK                               0xffffffff00000000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_OFFSET                             0x0000000000000018
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_LSB                                0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_MSB                                31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_MASK                               0x00000000ffffffff


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_OFFSET                             0x0000000000000018
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_LSB                                32
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_MSB                                63
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_MASK                               0xffffffff00000000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_OFFSET                            0x0000000000000020
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_LSB                               0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_MSB                               31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_MASK                              0x00000000ffffffff


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_OFFSET                         0x0000000000000020
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_LSB                            32
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_MSB                            63
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_MASK                           0xffffffff00000000


 

#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_OFFSET                                  0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_LSB                                     0
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_MSB                                     7
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_MASK                                    0x00000000000000ff


 

#define TX_QUEUE_EXTENSION_CAS_OFFSET_OFFSET                                        0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_OFFSET_LSB                                           8
#define TX_QUEUE_EXTENSION_CAS_OFFSET_MSB                                           12
#define TX_QUEUE_EXTENSION_CAS_OFFSET_MASK                                          0x0000000000001f00


 

#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_OFFSET                              0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_LSB                                 13
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_MSB                                 13
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_MASK                                0x0000000000002000


 

#define TX_QUEUE_EXTENSION_RESERVED_10A_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_10A_LSB                                         14
#define TX_QUEUE_EXTENSION_RESERVED_10A_MSB                                         15
#define TX_QUEUE_EXTENSION_RESERVED_10A_MASK                                        0x000000000000c000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_OFFSET               0x0000000000000028
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_LSB                  16
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_MSB                  19
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_MASK                 0x00000000000f0000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_OFFSET              0x0000000000000028
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_LSB                 20
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_MSB                 23
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_MASK                0x0000000000f00000


 

#define TX_QUEUE_EXTENSION_RESERVED_10B_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_10B_LSB                                         24
#define TX_QUEUE_EXTENSION_RESERVED_10B_MSB                                         29
#define TX_QUEUE_EXTENSION_RESERVED_10B_MASK                                        0x000000003f000000


 

#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_OFFSET                          0x0000000000000028
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_LSB                             30
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_MSB                             30
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_MASK                            0x0000000040000000


 

#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_OFFSET                          0x0000000000000028
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_LSB                             31
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_MSB                             31
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_MASK                            0x0000000080000000


 

#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_OFFSET                                    0x0000000000000028
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_LSB                                       32
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_MSB                                       43
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_MASK                                      0x00000fff00000000


 

#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_OFFSET                         0x0000000000000028
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_LSB                            44
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_MSB                            52
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_MASK                           0x001ff00000000000


 

#define TX_QUEUE_EXTENSION_RESERVED_11A_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_11A_LSB                                         53
#define TX_QUEUE_EXTENSION_RESERVED_11A_MSB                                         63
#define TX_QUEUE_EXTENSION_RESERVED_11A_MASK                                        0xffe0000000000000


 

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_OFFSET                            0x0000000000000030
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_LSB                               0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_MSB                               31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_MASK                              0x00000000ffffffff


 

#define TX_QUEUE_EXTENSION_FW2SW_INFO_OFFSET                                        0x0000000000000030
#define TX_QUEUE_EXTENSION_FW2SW_INFO_LSB                                           32
#define TX_QUEUE_EXTENSION_FW2SW_INFO_MSB                                           63
#define TX_QUEUE_EXTENSION_FW2SW_INFO_MASK                                          0xffffffff00000000



#endif    
