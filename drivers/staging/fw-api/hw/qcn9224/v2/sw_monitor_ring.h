
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

 
 
 
 
 
 
 


#ifndef _SW_MONITOR_RING_H_
#define _SW_MONITOR_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#include "rx_mpdu_details.h"
#define NUM_OF_DWORDS_SW_MONITOR_RING 8


struct sw_monitor_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             struct   buffer_addr_info                                          status_buff_addr_info;
             uint32_t rxdma_push_reason                                       :  2,  
                      rxdma_error_code                                        :  5,  
                      mpdu_fragment_number                                    :  4,  
                      frameless_bar                                           :  1,  
                      status_buf_count                                        :  4,  
                      end_of_ppdu                                             :  1,  
                      reserved_6a                                             : 15;  
             uint32_t phy_ppdu_id                                             : 16,  
                      reserved_7a                                             :  4,  
                      ring_id                                                 :  8,  
                      looping_count                                           :  4;  
#else
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             struct   buffer_addr_info                                          status_buff_addr_info;
             uint32_t reserved_6a                                             : 15,  
                      end_of_ppdu                                             :  1,  
                      status_buf_count                                        :  4,  
                      frameless_bar                                           :  1,  
                      mpdu_fragment_number                                    :  4,  
                      rxdma_error_code                                        :  5,  
                      rxdma_push_reason                                       :  2;  
             uint32_t looping_count                                           :  4,  
                      ring_id                                                 :  8,  
                      reserved_7a                                             :  4,  
                      phy_ppdu_id                                             : 16;  
#endif
};


 


 


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB 0
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB 31
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK 0xffffffff


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB 0
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB 7
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK 0x000000ff


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB 12
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB 31
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK 0xfffff000


 


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB 0
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB 7
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK 0x000000ff


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB 8
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB 8
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK 0x00000100


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB 9
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB 9
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK 0x00000200


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB 10
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB 10
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK 0x00000400


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB 11
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB 11
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK 0x00000800


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 12
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB 12
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB 13
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB 13
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK 0x00002000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB 14
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB 14
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK 0x00004000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB 15
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB 26
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK 0x07ff8000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB 27
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB 27
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK 0x08000000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET 0x00000008
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_LSB 28
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MSB 31
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MASK 0xf0000000


 

#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET 0x0000000c
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB 0
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB 31
#define SW_MONITOR_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK 0xffffffff


 


 

#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET               0x00000010
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_31_0_LSB                  0
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_31_0_MSB                  31
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_31_0_MASK                 0xffffffff


 

#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET              0x00000014
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_39_32_LSB                 0
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_39_32_MSB                 7
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_BUFFER_ADDR_39_32_MASK                0x000000ff


 

#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET          0x00000014
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB             8
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB             11
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK            0x00000f00


 

#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET               0x00000014
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_SW_BUFFER_COOKIE_LSB                  12
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_SW_BUFFER_COOKIE_MSB                  31
#define SW_MONITOR_RING_STATUS_BUFF_ADDR_INFO_SW_BUFFER_COOKIE_MASK                 0xfffff000


 

#define SW_MONITOR_RING_RXDMA_PUSH_REASON_OFFSET                                    0x00000018
#define SW_MONITOR_RING_RXDMA_PUSH_REASON_LSB                                       0
#define SW_MONITOR_RING_RXDMA_PUSH_REASON_MSB                                       1
#define SW_MONITOR_RING_RXDMA_PUSH_REASON_MASK                                      0x00000003


 

#define SW_MONITOR_RING_RXDMA_ERROR_CODE_OFFSET                                     0x00000018
#define SW_MONITOR_RING_RXDMA_ERROR_CODE_LSB                                        2
#define SW_MONITOR_RING_RXDMA_ERROR_CODE_MSB                                        6
#define SW_MONITOR_RING_RXDMA_ERROR_CODE_MASK                                       0x0000007c


 

#define SW_MONITOR_RING_MPDU_FRAGMENT_NUMBER_OFFSET                                 0x00000018
#define SW_MONITOR_RING_MPDU_FRAGMENT_NUMBER_LSB                                    7
#define SW_MONITOR_RING_MPDU_FRAGMENT_NUMBER_MSB                                    10
#define SW_MONITOR_RING_MPDU_FRAGMENT_NUMBER_MASK                                   0x00000780


 

#define SW_MONITOR_RING_FRAMELESS_BAR_OFFSET                                        0x00000018
#define SW_MONITOR_RING_FRAMELESS_BAR_LSB                                           11
#define SW_MONITOR_RING_FRAMELESS_BAR_MSB                                           11
#define SW_MONITOR_RING_FRAMELESS_BAR_MASK                                          0x00000800


 

#define SW_MONITOR_RING_STATUS_BUF_COUNT_OFFSET                                     0x00000018
#define SW_MONITOR_RING_STATUS_BUF_COUNT_LSB                                        12
#define SW_MONITOR_RING_STATUS_BUF_COUNT_MSB                                        15
#define SW_MONITOR_RING_STATUS_BUF_COUNT_MASK                                       0x0000f000


 

#define SW_MONITOR_RING_END_OF_PPDU_OFFSET                                          0x00000018
#define SW_MONITOR_RING_END_OF_PPDU_LSB                                             16
#define SW_MONITOR_RING_END_OF_PPDU_MSB                                             16
#define SW_MONITOR_RING_END_OF_PPDU_MASK                                            0x00010000


 

#define SW_MONITOR_RING_RESERVED_6A_OFFSET                                          0x00000018
#define SW_MONITOR_RING_RESERVED_6A_LSB                                             17
#define SW_MONITOR_RING_RESERVED_6A_MSB                                             31
#define SW_MONITOR_RING_RESERVED_6A_MASK                                            0xfffe0000


 

#define SW_MONITOR_RING_PHY_PPDU_ID_OFFSET                                          0x0000001c
#define SW_MONITOR_RING_PHY_PPDU_ID_LSB                                             0
#define SW_MONITOR_RING_PHY_PPDU_ID_MSB                                             15
#define SW_MONITOR_RING_PHY_PPDU_ID_MASK                                            0x0000ffff


 

#define SW_MONITOR_RING_RESERVED_7A_OFFSET                                          0x0000001c
#define SW_MONITOR_RING_RESERVED_7A_LSB                                             16
#define SW_MONITOR_RING_RESERVED_7A_MSB                                             19
#define SW_MONITOR_RING_RESERVED_7A_MASK                                            0x000f0000


 

#define SW_MONITOR_RING_RING_ID_OFFSET                                              0x0000001c
#define SW_MONITOR_RING_RING_ID_LSB                                                 20
#define SW_MONITOR_RING_RING_ID_MSB                                                 27
#define SW_MONITOR_RING_RING_ID_MASK                                                0x0ff00000


 

#define SW_MONITOR_RING_LOOPING_COUNT_OFFSET                                        0x0000001c
#define SW_MONITOR_RING_LOOPING_COUNT_LSB                                           28
#define SW_MONITOR_RING_LOOPING_COUNT_MSB                                           31
#define SW_MONITOR_RING_LOOPING_COUNT_MASK                                          0xf0000000



#endif    
