
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_START_PPDU_H_
#define _TX_FES_STATUS_START_PPDU_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_START_PPDU 4

#define NUM_OF_QWORDS_TX_FES_STATUS_START_PPDU 2


struct tx_fes_status_start_ppdu {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_timestamp_lower_32                                 : 32;  
             uint32_t ppdu_timestamp_upper_32                                 : 32;  
             uint32_t subband_mask                                            : 16,  
                      ndp_frame                                               :  2,  
                      reserved_2b                                             :  2,  
                      coex_based_tx_bw                                        :  3,  
                      coex_based_ant_mask                                     :  8,  
                      reserved_2c                                             :  1;  
             uint32_t coex_based_tx_pwr_shared_ant                            :  8,  
                      coex_based_tx_pwr_ant                                   :  8,  
                      concurrent_bt_tx                                        :  1,  
                      concurrent_wlan_tx                                      :  1,  
                      concurrent_wan_tx                                       :  1,  
                      concurrent_wan_rx                                       :  1,  
                      coex_pwr_reduction_bt                                   :  1,  
                      coex_pwr_reduction_wlan                                 :  1,  
                      coex_pwr_reduction_wan                                  :  1,  
                      coex_result_alt_based                                   :  1,  
                      request_packet_bw                                       :  3,  
                      response_type                                           :  5;  
#else
             uint32_t ppdu_timestamp_lower_32                                 : 32;  
             uint32_t ppdu_timestamp_upper_32                                 : 32;  
             uint32_t reserved_2c                                             :  1,  
                      coex_based_ant_mask                                     :  8,  
                      coex_based_tx_bw                                        :  3,  
                      reserved_2b                                             :  2,  
                      ndp_frame                                               :  2,  
                      subband_mask                                            : 16;  
             uint32_t response_type                                           :  5,  
                      request_packet_bw                                       :  3,  
                      coex_result_alt_based                                   :  1,  
                      coex_pwr_reduction_wan                                  :  1,  
                      coex_pwr_reduction_wlan                                 :  1,  
                      coex_pwr_reduction_bt                                   :  1,  
                      concurrent_wan_rx                                       :  1,  
                      concurrent_wan_tx                                       :  1,  
                      concurrent_wlan_tx                                      :  1,  
                      concurrent_bt_tx                                        :  1,  
                      coex_based_tx_pwr_ant                                   :  8,  
                      coex_based_tx_pwr_shared_ant                            :  8;  
#endif
};


 

#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_LSB                        0
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_MSB                        31
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_MASK                       0x00000000ffffffff


 

#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_LSB                        32
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_MSB                        63
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_MASK                       0xffffffff00000000


 

#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_OFFSET                                0x0000000000000008
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_LSB                                   0
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_MSB                                   15
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_MASK                                  0x000000000000ffff


 

#define TX_FES_STATUS_START_PPDU_NDP_FRAME_OFFSET                                   0x0000000000000008
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_LSB                                      16
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_MSB                                      17
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_MASK                                     0x0000000000030000


 

#define TX_FES_STATUS_START_PPDU_RESERVED_2B_OFFSET                                 0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_LSB                                    18
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_MSB                                    19
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_MASK                                   0x00000000000c0000


 

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_OFFSET                            0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_LSB                               20
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_MSB                               22
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_MASK                              0x0000000000700000


 

#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_OFFSET                         0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_LSB                            23
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_MSB                            30
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_MASK                           0x000000007f800000


 

#define TX_FES_STATUS_START_PPDU_RESERVED_2C_OFFSET                                 0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_LSB                                    31
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_MSB                                    31
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_MASK                                   0x0000000080000000


 

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_OFFSET                0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_LSB                   32
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_MSB                   39
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_MASK                  0x000000ff00000000


 

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_LSB                          40
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_MSB                          47
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_MASK                         0x0000ff0000000000


 

#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_OFFSET                            0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_LSB                               48
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_MSB                               48
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_MASK                              0x0001000000000000


 

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_OFFSET                          0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_LSB                             49
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_MSB                             49
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_MASK                            0x0002000000000000


 

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_LSB                              50
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_MSB                              50
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_MASK                             0x0004000000000000


 

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_LSB                              51
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_MSB                              51
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_MASK                             0x0008000000000000


 

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_LSB                          52
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_MSB                          52
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_MASK                         0x0010000000000000


 

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_LSB                        53
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_MSB                        53
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_MASK                       0x0020000000000000


 

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_LSB                         54
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_MSB                         54
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_MASK                        0x0040000000000000


 

#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_LSB                          55
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_MSB                          55
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_MASK                         0x0080000000000000


 

#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_LSB                              56
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_MSB                              58
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_MASK                             0x0700000000000000


 

#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_OFFSET                               0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_LSB                                  59
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_MSB                                  63
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_MASK                                 0xf800000000000000



#endif    
