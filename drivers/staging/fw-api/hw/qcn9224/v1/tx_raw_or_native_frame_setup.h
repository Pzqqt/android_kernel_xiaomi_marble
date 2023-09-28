
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

 
 
 
 
 
 
 


#ifndef _TX_RAW_OR_NATIVE_FRAME_SETUP_H_
#define _TX_RAW_OR_NATIVE_FRAME_SETUP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_RAW_OR_NATIVE_FRAME_SETUP 2

#define NUM_OF_QWORDS_TX_RAW_OR_NATIVE_FRAME_SETUP 1


struct tx_raw_or_native_frame_setup {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t fc_to_ds_mask                                           :  1,  
                      fc_from_ds_mask                                         :  1,  
                      fc_more_frag_mask                                       :  1,  
                      fc_retry_mask                                           :  1,  
                      fc_pwr_mgt_mask                                         :  1,  
                      fc_more_data_mask                                       :  1,  
                      fc_prot_frame_mask                                      :  1,  
                      fc_order_mask                                           :  1,  
                      duration_field_mask                                     :  1,  
                      sequence_control_mask                                   :  1,  
                      qc_tid_mask                                             :  1,  
                      qc_eosp_mask                                            :  1,  
                      qc_ack_policy_mask                                      :  1,  
                      qc_amsdu_mask                                           :  1,  
                      reserved_0a                                             :  1,  
                      qc_15to8_mask                                           :  1,  
                      iv_mask                                                 :  1,  
                      fc_to_ds_setting                                        :  1,  
                      fc_from_ds_setting                                      :  1,  
                      fc_more_frag_setting                                    :  1,  
                      fc_retry_setting                                        :  2,  
                      fc_pwr_mgt_setting                                      :  1,  
                      fc_more_data_setting                                    :  2,  
                      fc_prot_frame_setting                                   :  2,  
                      fc_order_setting                                        :  1,  
                      qc_tid_setting                                          :  4;  
             uint32_t qc_eosp_setting                                         :  2,  
                      qc_ack_policy_setting                                   :  2,  
                      qc_amsdu_setting                                        :  1,  
                      qc_15to8_setting                                        :  8,  
                      mlo_addr_override                                       :  1,  
                      mlo_ignore_addr3_override                               :  1,  
                      sequence_control_source                                 :  1,  
                      fragment_number                                         :  4,  
                      sequence_number                                         : 12;  
#else
             uint32_t qc_tid_setting                                          :  4,  
                      fc_order_setting                                        :  1,  
                      fc_prot_frame_setting                                   :  2,  
                      fc_more_data_setting                                    :  2,  
                      fc_pwr_mgt_setting                                      :  1,  
                      fc_retry_setting                                        :  2,  
                      fc_more_frag_setting                                    :  1,  
                      fc_from_ds_setting                                      :  1,  
                      fc_to_ds_setting                                        :  1,  
                      iv_mask                                                 :  1,  
                      qc_15to8_mask                                           :  1,  
                      reserved_0a                                             :  1,  
                      qc_amsdu_mask                                           :  1,  
                      qc_ack_policy_mask                                      :  1,  
                      qc_eosp_mask                                            :  1,  
                      qc_tid_mask                                             :  1,  
                      sequence_control_mask                                   :  1,  
                      duration_field_mask                                     :  1,  
                      fc_order_mask                                           :  1,  
                      fc_prot_frame_mask                                      :  1,  
                      fc_more_data_mask                                       :  1,  
                      fc_pwr_mgt_mask                                         :  1,  
                      fc_retry_mask                                           :  1,  
                      fc_more_frag_mask                                       :  1,  
                      fc_from_ds_mask                                         :  1,  
                      fc_to_ds_mask                                           :  1;  
             uint32_t sequence_number                                         : 12,  
                      fragment_number                                         :  4,  
                      sequence_control_source                                 :  1,  
                      mlo_ignore_addr3_override                               :  1,  
                      mlo_addr_override                                       :  1,  
                      qc_15to8_setting                                        :  8,  
                      qc_amsdu_setting                                        :  1,  
                      qc_ack_policy_setting                                   :  2,  
                      qc_eosp_setting                                         :  2;  
#endif
};


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_LSB                              0
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_MSB                              0
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_MASK                             0x0000000000000001


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_LSB                            1
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_MSB                            1
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_MASK                           0x0000000000000002


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_LSB                          2
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_MSB                          2
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_MASK                         0x0000000000000004


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_LSB                              3
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_MSB                              3
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_MASK                             0x0000000000000008


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_LSB                            4
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_MSB                            4
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_MASK                           0x0000000000000010


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_LSB                          5
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_MSB                          5
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_MASK                         0x0000000000000020


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_LSB                         6
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_MSB                         6
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_MASK                        0x0000000000000040


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_LSB                              7
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_MSB                              7
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_MASK                             0x0000000000000080


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_OFFSET                     0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_LSB                        8
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_MSB                        8
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_MASK                       0x0000000000000100


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_LSB                      9
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_MSB                      9
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_MASK                     0x0000000000000200


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_OFFSET                             0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_LSB                                10
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_MSB                                10
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_MASK                               0x0000000000000400


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_OFFSET                            0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_LSB                               11
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_MSB                               11
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_MASK                              0x0000000000000800


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_LSB                         12
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_MSB                         12
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_MASK                        0x0000000000001000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_LSB                              13
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_MSB                              13
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_MASK                             0x0000000000002000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_OFFSET                             0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_LSB                                14
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_MSB                                14
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_MASK                               0x0000000000004000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_LSB                              15
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_MSB                              15
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_MASK                             0x0000000000008000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_OFFSET                                 0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_LSB                                    16
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_MSB                                    16
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_MASK                                   0x0000000000010000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_LSB                           17
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_MSB                           17
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_MASK                          0x0000000000020000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_LSB                         18
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_MSB                         18
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_MASK                        0x0000000000040000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_OFFSET                    0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_LSB                       19
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_MSB                       19
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_MASK                      0x0000000000080000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_LSB                           20
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_MSB                           21
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_MASK                          0x0000000000300000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_LSB                         22
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_MSB                         22
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_MASK                        0x0000000000400000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_OFFSET                    0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_LSB                       23
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_MSB                       24
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_MASK                      0x0000000001800000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_LSB                      25
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_MSB                      26
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_MASK                     0x0000000006000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_LSB                           27
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_MSB                           27
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_MASK                          0x0000000008000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_OFFSET                          0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_LSB                             28
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_MSB                             31
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_MASK                            0x00000000f0000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_LSB                            32
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_MSB                            33
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_MASK                           0x0000000300000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_LSB                      34
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_MSB                      35
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_MASK                     0x0000000c00000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_LSB                           36
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_MSB                           36
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_MASK                          0x0000001000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_LSB                           37
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_MSB                           44
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_MASK                          0x00001fe000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_LSB                          45
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_MSB                          45
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_MASK                         0x0000200000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_OFFSET               0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_LSB                  46
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_MSB                  46
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_MASK                 0x0000400000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_OFFSET                 0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_LSB                    47
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_MSB                    47
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_MASK                   0x0000800000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_LSB                            48
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_MSB                            51
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_MASK                           0x000f000000000000


 

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_LSB                            52
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_MSB                            63
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_MASK                           0xfff0000000000000



#endif    
