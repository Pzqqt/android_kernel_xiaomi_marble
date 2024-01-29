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


#ifndef _TX_FES_STATUS_USER_PPDU_H_
#define _TX_FES_STATUS_USER_PPDU_H_

#define NUM_OF_DWORDS_TX_FES_STATUS_USER_PPDU 6

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

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_OFFSET                         0x00000000
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_LSB                            0
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MSB                            8
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MASK                           0x000001ff

#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_OFFSET                       0x00000000
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_LSB                          9
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MSB                          10
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MASK                         0x00000600

#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_OFFSET                    0x00000000
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_LSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MASK                      0x00000800

#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_OFFSET                              0x00000000
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_LSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MASK                                0x00001000

#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_OFFSET                         0x00000000
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_LSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MASK                           0x00002000

#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_OFFSET                           0x00000000
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_LSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MASK                             0x00004000

#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_OFFSET                                0x00000000
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_LSB                                   15
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MSB                                   23
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MASK                                  0x00ff8000

#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_OFFSET                                 0x00000000
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_LSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MASK                                   0x01000000

#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_OFFSET                0x00000000
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_LSB                   25
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MSB                   31
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MASK                  0xfe000000

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_OFFSET                         0x00000004
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_LSB                            0
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MSB                            15
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MASK                           0x0000ffff

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_OFFSET                  0x00000004
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_LSB                     16
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MSB                     16
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MASK                    0x00010000

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_OFFSET                        0x00000004
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_LSB                           17
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MSB                           25
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MASK                          0x03fe0000

#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_OFFSET                              0x00000004
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_LSB                                 26
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MSB                                 29
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MASK                                0x3c000000

#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_OFFSET                   0x00000004
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_LSB                      30
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MSB                      30
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MASK                     0x40000000

#define TX_FES_STATUS_USER_PPDU_RESERVED_1_OFFSET                                   0x00000004
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_LSB                                      31
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MSB                                      31
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MASK                                     0x80000000

#define TX_FES_STATUS_USER_PPDU_DURATION_OFFSET                                     0x00000008
#define TX_FES_STATUS_USER_PPDU_DURATION_LSB                                        0
#define TX_FES_STATUS_USER_PPDU_DURATION_MSB                                        15
#define TX_FES_STATUS_USER_PPDU_DURATION_MASK                                       0x0000ffff

#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_OFFSET                          0x00000008
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_LSB                             16
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MSB                             31
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MASK                            0xffff0000

#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_OFFSET                                   0x0000000c
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_LSB                                      0
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MSB                                      23
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MASK                                     0x00ffffff

#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_OFFSET                                0x0000000c
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_LSB                                   24
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MSB                                   31
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MASK                                  0xff000000

#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_OFFSET                         0x00000010
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_LSB                            0
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MSB                            21
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MASK                           0x003fffff

#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_OFFSET                                  0x00000010
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_LSB                                     22
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MSB                                     23
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MASK                                    0x00c00000

#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_OFFSET                    0x00000010
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_LSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MASK                      0x01000000

#define TX_FES_STATUS_USER_PPDU_NSS_OFFSET                                          0x00000010
#define TX_FES_STATUS_USER_PPDU_NSS_LSB                                             25
#define TX_FES_STATUS_USER_PPDU_NSS_MSB                                             27
#define TX_FES_STATUS_USER_PPDU_NSS_MASK                                            0x0e000000

#define TX_FES_STATUS_USER_PPDU_MCS_OFFSET                                          0x00000010
#define TX_FES_STATUS_USER_PPDU_MCS_LSB                                             28
#define TX_FES_STATUS_USER_PPDU_MCS_MSB                                             31
#define TX_FES_STATUS_USER_PPDU_MCS_MASK                                            0xf0000000

#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_OFFSET                                   0x00000014
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_LSB                                      0
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MSB                                      31
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MASK                                     0xffffffff

#endif
