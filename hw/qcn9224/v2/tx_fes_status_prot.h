
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_PROT_H_
#define _TX_FES_STATUS_PROT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_PROT 14

#define NUM_OF_QWORDS_TX_FES_STATUS_PROT 7


struct tx_fes_status_prot {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t success                                                 :  1,  
                      phytx_pkt_end_info_valid                                :  1,  
                      phytx_abort_request_info_valid                          :  1,  
                      reserved_0                                              : 20,  
                      pkt_type                                                :  4,  
                      dot11ax_su_extended                                     :  1,  
                      rate_mcs                                                :  4;  
             uint32_t frame_type                                              :  2,  
                      frame_subtype                                           :  4,  
                      rx_pwr_mgmt                                             :  1,  
                      status                                                  :  1,  
                      duration_field                                          : 16,  
                      reserved_1a                                             :  2,  
                      agc_cbw                                                 :  3,  
                      service_cbw                                             :  3;  
             uint32_t start_of_frame_timestamp_15_0                           : 16,  
                      start_of_frame_timestamp_31_16                          : 16;  
             uint32_t end_of_frame_timestamp_15_0                             : 16,  
                      end_of_frame_timestamp_31_16                            : 16;  
             uint32_t tx_group_delay                                          : 12,  
                      timing_status                                           :  2,  
                      dpdtrain_done                                           :  1,  
                      reserved_4                                              :  1,  
                      transmit_delay                                          : 16;  
             uint32_t tpc_dbg_info_cmn_15_0                                   : 16,  
                      tpc_dbg_info_cmn_31_16                                  : 16;  
             uint32_t tpc_dbg_info_cmn_47_32                                  : 16,  
                      tpc_dbg_info_chn1_15_0                                  : 16;  
             uint32_t tpc_dbg_info_chn1_31_16                                 : 16,  
                      tpc_dbg_info_chn1_47_32                                 : 16;  
             uint32_t tpc_dbg_info_chn1_63_48                                 : 16,  
                      tpc_dbg_info_chn1_79_64                                 : 16;  
             uint32_t tpc_dbg_info_chn2_15_0                                  : 16,  
                      tpc_dbg_info_chn2_31_16                                 : 16;  
             uint32_t tpc_dbg_info_chn2_47_32                                 : 16,  
                      tpc_dbg_info_chn2_63_48                                 : 16;  
             uint32_t tpc_dbg_info_chn2_79_64                                 : 16;  
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t phytx_tx_end_sw_info_15_0                               : 16,  
                      phytx_tx_end_sw_info_31_16                              : 16;  
             uint32_t phytx_tx_end_sw_info_47_32                              : 16,  
                      phytx_tx_end_sw_info_63_48                              : 16;  
#else
             uint32_t rate_mcs                                                :  4,  
                      dot11ax_su_extended                                     :  1,  
                      pkt_type                                                :  4,  
                      reserved_0                                              : 20,  
                      phytx_abort_request_info_valid                          :  1,  
                      phytx_pkt_end_info_valid                                :  1,  
                      success                                                 :  1;  
             uint32_t service_cbw                                             :  3,  
                      agc_cbw                                                 :  3,  
                      reserved_1a                                             :  2,  
                      duration_field                                          : 16,  
                      status                                                  :  1,  
                      rx_pwr_mgmt                                             :  1,  
                      frame_subtype                                           :  4,  
                      frame_type                                              :  2;  
             uint32_t start_of_frame_timestamp_31_16                          : 16,  
                      start_of_frame_timestamp_15_0                           : 16;  
             uint32_t end_of_frame_timestamp_31_16                            : 16,  
                      end_of_frame_timestamp_15_0                             : 16;  
             uint32_t transmit_delay                                          : 16,  
                      reserved_4                                              :  1,  
                      dpdtrain_done                                           :  1,  
                      timing_status                                           :  2,  
                      tx_group_delay                                          : 12;  
             uint32_t tpc_dbg_info_cmn_31_16                                  : 16,  
                      tpc_dbg_info_cmn_15_0                                   : 16;  
             uint32_t tpc_dbg_info_chn1_15_0                                  : 16,  
                      tpc_dbg_info_cmn_47_32                                  : 16;  
             uint32_t tpc_dbg_info_chn1_47_32                                 : 16,  
                      tpc_dbg_info_chn1_31_16                                 : 16;  
             uint32_t tpc_dbg_info_chn1_79_64                                 : 16,  
                      tpc_dbg_info_chn1_63_48                                 : 16;  
             uint32_t tpc_dbg_info_chn2_31_16                                 : 16,  
                      tpc_dbg_info_chn2_15_0                                  : 16;  
             uint32_t tpc_dbg_info_chn2_63_48                                 : 16,  
                      tpc_dbg_info_chn2_47_32                                 : 16;  
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t tpc_dbg_info_chn2_79_64                                 : 16;  
             uint32_t phytx_tx_end_sw_info_31_16                              : 16,  
                      phytx_tx_end_sw_info_15_0                               : 16;  
             uint32_t phytx_tx_end_sw_info_63_48                              : 16,  
                      phytx_tx_end_sw_info_47_32                              : 16;  
#endif
};


 

#define TX_FES_STATUS_PROT_SUCCESS_OFFSET                                           0x0000000000000000
#define TX_FES_STATUS_PROT_SUCCESS_LSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MASK                                             0x0000000000000001


 

#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_LSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MASK                            0x0000000000000002


 

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                      0x0000000000000004


 

#define TX_FES_STATUS_PROT_RESERVED_0_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_PROT_RESERVED_0_LSB                                           3
#define TX_FES_STATUS_PROT_RESERVED_0_MSB                                           22
#define TX_FES_STATUS_PROT_RESERVED_0_MASK                                          0x00000000007ffff8


 

#define TX_FES_STATUS_PROT_PKT_TYPE_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_PROT_PKT_TYPE_LSB                                             23
#define TX_FES_STATUS_PROT_PKT_TYPE_MSB                                             26
#define TX_FES_STATUS_PROT_PKT_TYPE_MASK                                            0x0000000007800000


 

#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_LSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MASK                                 0x0000000008000000


 

#define TX_FES_STATUS_PROT_RATE_MCS_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_PROT_RATE_MCS_LSB                                             28
#define TX_FES_STATUS_PROT_RATE_MCS_MSB                                             31
#define TX_FES_STATUS_PROT_RATE_MCS_MASK                                            0x00000000f0000000


 

#define TX_FES_STATUS_PROT_FRAME_TYPE_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_PROT_FRAME_TYPE_LSB                                           32
#define TX_FES_STATUS_PROT_FRAME_TYPE_MSB                                           33
#define TX_FES_STATUS_PROT_FRAME_TYPE_MASK                                          0x0000000300000000


 

#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_OFFSET                                     0x0000000000000000
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_LSB                                        34
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MSB                                        37
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MASK                                       0x0000003c00000000


 

#define TX_FES_STATUS_PROT_RX_PWR_MGMT_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_LSB                                          38
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MSB                                          38
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MASK                                         0x0000004000000000


 

#define TX_FES_STATUS_PROT_STATUS_OFFSET                                            0x0000000000000000
#define TX_FES_STATUS_PROT_STATUS_LSB                                               39
#define TX_FES_STATUS_PROT_STATUS_MSB                                               39
#define TX_FES_STATUS_PROT_STATUS_MASK                                              0x0000008000000000


 

#define TX_FES_STATUS_PROT_DURATION_FIELD_OFFSET                                    0x0000000000000000
#define TX_FES_STATUS_PROT_DURATION_FIELD_LSB                                       40
#define TX_FES_STATUS_PROT_DURATION_FIELD_MSB                                       55
#define TX_FES_STATUS_PROT_DURATION_FIELD_MASK                                      0x00ffff0000000000


 

#define TX_FES_STATUS_PROT_RESERVED_1A_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_RESERVED_1A_LSB                                          56
#define TX_FES_STATUS_PROT_RESERVED_1A_MSB                                          57
#define TX_FES_STATUS_PROT_RESERVED_1A_MASK                                         0x0300000000000000


 

#define TX_FES_STATUS_PROT_AGC_CBW_OFFSET                                           0x0000000000000000
#define TX_FES_STATUS_PROT_AGC_CBW_LSB                                              58
#define TX_FES_STATUS_PROT_AGC_CBW_MSB                                              60
#define TX_FES_STATUS_PROT_AGC_CBW_MASK                                             0x1c00000000000000


 

#define TX_FES_STATUS_PROT_SERVICE_CBW_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_SERVICE_CBW_LSB                                          61
#define TX_FES_STATUS_PROT_SERVICE_CBW_MSB                                          63
#define TX_FES_STATUS_PROT_SERVICE_CBW_MASK                                         0xe000000000000000


 

#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_LSB                        0
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_MSB                        15
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_MASK                       0x000000000000ffff


 

#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                    0x0000000000000008
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_LSB                       16
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_MSB                       31
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_MASK                      0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_LSB                          32
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_MSB                          47
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_MASK                         0x0000ffff00000000


 

#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_LSB                         48
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_MSB                         63
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_MASK                        0xffff000000000000


 

#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_OFFSET                                    0x0000000000000010
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_LSB                                       0
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_MSB                                       11
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_MASK                                      0x0000000000000fff


 

#define TX_FES_STATUS_PROT_TIMING_STATUS_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_PROT_TIMING_STATUS_LSB                                        12
#define TX_FES_STATUS_PROT_TIMING_STATUS_MSB                                        13
#define TX_FES_STATUS_PROT_TIMING_STATUS_MASK                                       0x0000000000003000


 

#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_LSB                                        14
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_MSB                                        14
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_MASK                                       0x0000000000004000


 

#define TX_FES_STATUS_PROT_RESERVED_4_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_PROT_RESERVED_4_LSB                                           15
#define TX_FES_STATUS_PROT_RESERVED_4_MSB                                           15
#define TX_FES_STATUS_PROT_RESERVED_4_MASK                                          0x0000000000008000


 

#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_OFFSET                                    0x0000000000000010
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_LSB                                       16
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_MSB                                       31
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_MASK                                      0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_OFFSET                             0x0000000000000010
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_LSB                                32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_MSB                                47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_MASK                               0x0000ffff00000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_OFFSET                            0x0000000000000010
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_LSB                               48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_MSB                               63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_MASK                              0xffff000000000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_LSB                               0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_MSB                               15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_MASK                              0x000000000000ffff


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_LSB                               16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_MSB                               31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_MASK                              0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_OFFSET                           0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_LSB                              32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_MSB                              47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_MASK                             0x0000ffff00000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_OFFSET                           0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_LSB                              48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_MSB                              63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_MASK                             0xffff000000000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_LSB                              0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_MSB                              15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_MASK                             0x000000000000ffff


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_LSB                              16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_MSB                              31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_MASK                             0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_LSB                               32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_MSB                               47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_MASK                              0x0000ffff00000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_LSB                              48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_MSB                              63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_MASK                             0xffff000000000000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_LSB                              0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_MSB                              15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_MASK                             0x000000000000ffff


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_LSB                              16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_MSB                              31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_MASK                             0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_LSB                              32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_MSB                              47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_MASK                             0x0000ffff00000000


 


 

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB  48
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB  55
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK 0x00ff000000000000


 

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET      0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB         56
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB         61
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK        0x3f00000000000000


 

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET         0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB            62
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB            63
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK           0xc000000000000000


 

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_LSB                            0
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_MSB                            15
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_MASK                           0x000000000000ffff


 

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_LSB                           16
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_MSB                           31
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_MASK                          0x00000000ffff0000


 

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_LSB                           32
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_MSB                           47
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_MASK                          0x0000ffff00000000


 

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_LSB                           48
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_MSB                           63
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_MASK                          0xffff000000000000



#endif    
