
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _REO_ENTRANCE_RING_H_
#define _REO_ENTRANCE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_mpdu_details.h"
#define NUM_OF_DWORDS_REO_ENTRANCE_RING 8


struct reo_entrance_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8,  
                      rounded_mpdu_byte_count                                 : 14,  
                      reo_destination_indication                              :  5,  
                      frameless_bar                                           :  1,  
                      reserved_5a                                             :  4;  
             uint32_t rxdma_push_reason                                       :  2,  
                      rxdma_error_code                                        :  5,  
                      mpdu_fragment_number                                    :  4,  
                      sw_exception                                            :  1,  
                      sw_exception_mpdu_delink                                :  1,  
                      sw_exception_destination_ring_valid                     :  1,  
                      sw_exception_destination_ring                           :  5,  
                      mpdu_sequence_number                                    : 12,  
                      reserved_6a                                             :  1;  
             uint32_t phy_ppdu_id                                             : 16,  
                      src_link_id                                             :  3,  
                      reserved_7a                                             :  1,  
                      ring_id                                                 :  8,  
                      looping_count                                           :  4;  
#else
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t reserved_5a                                             :  4,  
                      frameless_bar                                           :  1,  
                      reo_destination_indication                              :  5,  
                      rounded_mpdu_byte_count                                 : 14,  
                      rx_reo_queue_desc_addr_39_32                            :  8;  
             uint32_t reserved_6a                                             :  1,  
                      mpdu_sequence_number                                    : 12,  
                      sw_exception_destination_ring                           :  5,  
                      sw_exception_destination_ring_valid                     :  1,  
                      sw_exception_mpdu_delink                                :  1,  
                      sw_exception                                            :  1,  
                      mpdu_fragment_number                                    :  4,  
                      rxdma_error_code                                        :  5,  
                      rxdma_push_reason                                       :  2;  
             uint32_t looping_count                                           :  4,  
                      ring_id                                                 :  8,  
                      reserved_7a                                             :  1,  
                      src_link_id                                             :  3,  
                      phy_ppdu_id                                             : 16;  
#endif
};


 


 


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK 0xffffffff


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB 7
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK 0x000000ff


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK 0xfffff000


 


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB 7
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK 0x000000ff


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK 0x00000100


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB 9
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB 9
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK 0x00000200


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB 10
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB 10
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK 0x00000400


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK 0x00000800


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB 13
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB 13
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK 0x00002000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB 14
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB 14
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK 0x00004000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB 15
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB 26
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK 0x07ff8000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB 27
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB 27
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK 0x08000000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_LSB 28
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MASK 0xf0000000


 

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET 0x0000000c
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK 0xffffffff


 

#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                        0x00000010
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                           0
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                           31
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                          0xffffffff


 

#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                       0x00000014
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                          0
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                          7
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                         0x000000ff


 

#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_OFFSET                            0x00000014
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_LSB                               8
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_MSB                               21
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_MASK                              0x003fff00


 

#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_OFFSET                         0x00000014
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_LSB                            22
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_MSB                            26
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_MASK                           0x07c00000


 

#define REO_ENTRANCE_RING_FRAMELESS_BAR_OFFSET                                      0x00000014
#define REO_ENTRANCE_RING_FRAMELESS_BAR_LSB                                         27
#define REO_ENTRANCE_RING_FRAMELESS_BAR_MSB                                         27
#define REO_ENTRANCE_RING_FRAMELESS_BAR_MASK                                        0x08000000


 

#define REO_ENTRANCE_RING_RESERVED_5A_OFFSET                                        0x00000014
#define REO_ENTRANCE_RING_RESERVED_5A_LSB                                           28
#define REO_ENTRANCE_RING_RESERVED_5A_MSB                                           31
#define REO_ENTRANCE_RING_RESERVED_5A_MASK                                          0xf0000000


 

#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_OFFSET                                  0x00000018
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_LSB                                     0
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_MSB                                     1
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_MASK                                    0x00000003


 

#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_OFFSET                                   0x00000018
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_LSB                                      2
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_MSB                                      6
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_MASK                                     0x0000007c


 

#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_OFFSET                               0x00000018
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_LSB                                  7
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_MSB                                  10
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_MASK                                 0x00000780


 

#define REO_ENTRANCE_RING_SW_EXCEPTION_OFFSET                                       0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_LSB                                          11
#define REO_ENTRANCE_RING_SW_EXCEPTION_MSB                                          11
#define REO_ENTRANCE_RING_SW_EXCEPTION_MASK                                         0x00000800


 

#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_OFFSET                           0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_LSB                              12
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_MSB                              12
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_MASK                             0x00001000


 

#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_OFFSET                0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_LSB                   13
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_MSB                   13
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_MASK                  0x00002000


 

#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_OFFSET                      0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_LSB                         14
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_MSB                         18
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_MASK                        0x0007c000


 

#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_OFFSET                               0x00000018
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_LSB                                  19
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_MSB                                  30
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_MASK                                 0x7ff80000


 

#define REO_ENTRANCE_RING_RESERVED_6A_OFFSET                                        0x00000018
#define REO_ENTRANCE_RING_RESERVED_6A_LSB                                           31
#define REO_ENTRANCE_RING_RESERVED_6A_MSB                                           31
#define REO_ENTRANCE_RING_RESERVED_6A_MASK                                          0x80000000


 

#define REO_ENTRANCE_RING_PHY_PPDU_ID_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_PHY_PPDU_ID_LSB                                           0
#define REO_ENTRANCE_RING_PHY_PPDU_ID_MSB                                           15
#define REO_ENTRANCE_RING_PHY_PPDU_ID_MASK                                          0x0000ffff


 

#define REO_ENTRANCE_RING_SRC_LINK_ID_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_SRC_LINK_ID_LSB                                           16
#define REO_ENTRANCE_RING_SRC_LINK_ID_MSB                                           18
#define REO_ENTRANCE_RING_SRC_LINK_ID_MASK                                          0x00070000


 

#define REO_ENTRANCE_RING_RESERVED_7A_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_RESERVED_7A_LSB                                           19
#define REO_ENTRANCE_RING_RESERVED_7A_MSB                                           19
#define REO_ENTRANCE_RING_RESERVED_7A_MASK                                          0x00080000


 

#define REO_ENTRANCE_RING_RING_ID_OFFSET                                            0x0000001c
#define REO_ENTRANCE_RING_RING_ID_LSB                                               20
#define REO_ENTRANCE_RING_RING_ID_MSB                                               27
#define REO_ENTRANCE_RING_RING_ID_MASK                                              0x0ff00000


 

#define REO_ENTRANCE_RING_LOOPING_COUNT_OFFSET                                      0x0000001c
#define REO_ENTRANCE_RING_LOOPING_COUNT_LSB                                         28
#define REO_ENTRANCE_RING_LOOPING_COUNT_MSB                                         31
#define REO_ENTRANCE_RING_LOOPING_COUNT_MASK                                        0xf0000000



#endif    
