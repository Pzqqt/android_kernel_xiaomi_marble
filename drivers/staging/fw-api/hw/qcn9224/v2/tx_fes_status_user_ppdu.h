
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_USER_PPDU_H_
#define _TX_FES_STATUS_USER_PPDU_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_USER_PPDU 6

#define NUM_OF_QWORDS_TX_FES_STATUS_USER_PPDU 3


struct tx_fes_status_user_ppdu {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t underflow_mpdu_count                                    :  9,  
                      data_underflow_warning                                  :  2,  
                      bw_drop_underflow_warning                               :  1,  
                      qc_eosp_setting                                         :  1,  
                      fc_more_data_setting                                    :  1,  
                      fc_pwr_mgt_setting                                      :  1,  
                      mpdu_tx_count                                           :  9,  
                      user_blocked                                            :  1,  
                      pre_trig_response_delim_count                           :  7;  
             uint32_t underflow_byte_count                                    : 16,  
                      coex_abort_mpdu_count_valid                             :  1,  
                      coex_abort_mpdu_count                                   :  9,  
                      transmitted_tid                                         :  4,  
                      txdma_dropped_mpdu_warning                              :  1,  
                      reserved_1                                              :  1;  
             uint32_t duration                                                : 16,  
                      num_eof_delim_added                                     : 16;  
             uint32_t psdu_octet                                              : 24,  
                      qos_buf_state                                           :  8;  
             uint32_t num_null_delim_added                                    : 22,  
                      reserved_4a                                             :  2,  
                      cv_corr_user_valid_in_phy                               :  1,  
                      nss                                                     :  3,  
                      mcs                                                     :  4;  
             uint32_t ht_control                                              : 32;  
#else
             uint32_t pre_trig_response_delim_count                           :  7,  
                      user_blocked                                            :  1,  
                      mpdu_tx_count                                           :  9,  
                      fc_pwr_mgt_setting                                      :  1,  
                      fc_more_data_setting                                    :  1,  
                      qc_eosp_setting                                         :  1,  
                      bw_drop_underflow_warning                               :  1,  
                      data_underflow_warning                                  :  2,  
                      underflow_mpdu_count                                    :  9;  
             uint32_t reserved_1                                              :  1,  
                      txdma_dropped_mpdu_warning                              :  1,  
                      transmitted_tid                                         :  4,  
                      coex_abort_mpdu_count                                   :  9,  
                      coex_abort_mpdu_count_valid                             :  1,  
                      underflow_byte_count                                    : 16;  
             uint32_t num_eof_delim_added                                     : 16,  
                      duration                                                : 16;  
             uint32_t qos_buf_state                                           :  8,  
                      psdu_octet                                              : 24;  
             uint32_t mcs                                                     :  4,  
                      nss                                                     :  3,  
                      cv_corr_user_valid_in_phy                               :  1,  
                      reserved_4a                                             :  2,  
                      num_null_delim_added                                    : 22;  
             uint32_t ht_control                                              : 32;  
#endif
};


 

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_LSB                            0
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MSB                            8
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MASK                           0x00000000000001ff


 

#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_OFFSET                       0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_LSB                          9
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MSB                          10
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MASK                         0x0000000000000600


 

#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_LSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MASK                      0x0000000000000800


 

#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_LSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MASK                                0x0000000000001000


 

#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_LSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MASK                           0x0000000000002000


 

#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_LSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MASK                             0x0000000000004000


 

#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_OFFSET                                0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_LSB                                   15
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MSB                                   23
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MASK                                  0x0000000000ff8000


 

#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_LSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MASK                                   0x0000000001000000


 

#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_OFFSET                0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_LSB                   25
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MSB                   31
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MASK                  0x00000000fe000000


 

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_LSB                            32
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MSB                            47
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MASK                           0x0000ffff00000000


 

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_OFFSET                  0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_LSB                     48
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MSB                     48
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MASK                    0x0001000000000000


 

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_OFFSET                        0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_LSB                           49
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MSB                           57
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MASK                          0x03fe000000000000


 

#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_LSB                                 58
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MSB                                 61
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MASK                                0x3c00000000000000


 

#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_OFFSET                   0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_LSB                      62
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MSB                      62
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MASK                     0x4000000000000000


 

#define TX_FES_STATUS_USER_PPDU_RESERVED_1_OFFSET                                   0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_LSB                                      63
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MSB                                      63
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MASK                                     0x8000000000000000


 

#define TX_FES_STATUS_USER_PPDU_DURATION_OFFSET                                     0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_DURATION_LSB                                        0
#define TX_FES_STATUS_USER_PPDU_DURATION_MSB                                        15
#define TX_FES_STATUS_USER_PPDU_DURATION_MASK                                       0x000000000000ffff


 

#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_OFFSET                          0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_LSB                             16
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MSB                             31
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MASK                            0x00000000ffff0000


 

#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_OFFSET                                   0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_LSB                                      32
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MSB                                      55
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MASK                                     0x00ffffff00000000


 

#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_OFFSET                                0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_LSB                                   56
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MSB                                   63
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MASK                                  0xff00000000000000


 

#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_OFFSET                         0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_LSB                            0
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MSB                            21
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MASK                           0x00000000003fffff


 

#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_OFFSET                                  0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_LSB                                     22
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MSB                                     23
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MASK                                    0x0000000000c00000


 

#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_OFFSET                    0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_LSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MASK                      0x0000000001000000


 

#define TX_FES_STATUS_USER_PPDU_NSS_OFFSET                                          0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_NSS_LSB                                             25
#define TX_FES_STATUS_USER_PPDU_NSS_MSB                                             27
#define TX_FES_STATUS_USER_PPDU_NSS_MASK                                            0x000000000e000000


 

#define TX_FES_STATUS_USER_PPDU_MCS_OFFSET                                          0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_MCS_LSB                                             28
#define TX_FES_STATUS_USER_PPDU_MCS_MSB                                             31
#define TX_FES_STATUS_USER_PPDU_MCS_MASK                                            0x00000000f0000000


 

#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_OFFSET                                   0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_LSB                                      32
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MSB                                      63
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MASK                                     0xffffffff00000000



#endif    
