
/* Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _RXPT_CLASSIFY_INFO_H_
#define _RXPT_CLASSIFY_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RXPT_CLASSIFY_INFO 1


struct rxpt_classify_info {
#ifndef BIG_ENDIAN_HOST
             uint32_t reo_destination_indication                              :  5,  
                      lmac_peer_id_msb                                        :  2,  
                      use_flow_id_toeplitz_clfy                               :  1,  
                      pkt_selection_fp_ucast_data                             :  1,  
                      pkt_selection_fp_mcast_data                             :  1,  
                      pkt_selection_fp_1000                                   :  1,  
                      rxdma0_source_ring_selection                            :  3,  
                      rxdma0_destination_ring_selection                       :  3,  
                      mcast_echo_drop_enable                                  :  1,  
                      wds_learning_detect_en                                  :  1,  
                      intrabss_check_en                                       :  1,  
                      use_ppe                                                 :  1,  
                      ppe_routing_enable                                      :  1,  
                      reserved_0b                                             : 10;  
#else
             uint32_t reserved_0b                                             : 10,  
                      ppe_routing_enable                                      :  1,  
                      use_ppe                                                 :  1,  
                      intrabss_check_en                                       :  1,  
                      wds_learning_detect_en                                  :  1,  
                      mcast_echo_drop_enable                                  :  1,  
                      rxdma0_destination_ring_selection                       :  3,  
                      rxdma0_source_ring_selection                            :  3,  
                      pkt_selection_fp_1000                                   :  1,  
                      pkt_selection_fp_mcast_data                             :  1,  
                      pkt_selection_fp_ucast_data                             :  1,  
                      use_flow_id_toeplitz_clfy                               :  1,  
                      lmac_peer_id_msb                                        :  2,  
                      reo_destination_indication                              :  5;  
#endif
};


 

#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_OFFSET                        0x00000000
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_LSB                           0
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_MSB                           4
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_MASK                          0x0000001f


 

#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_OFFSET                                  0x00000000
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_LSB                                     5
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_MSB                                     6
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_MASK                                    0x00000060


 

#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET                         0x00000000
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_LSB                            7
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_MSB                            7
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_MASK                           0x00000080


 

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_OFFSET                       0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_LSB                          8
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_MSB                          8
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_MASK                         0x00000100


 

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_OFFSET                       0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_LSB                          9
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_MSB                          9
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_MASK                         0x00000200


 

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_OFFSET                             0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_LSB                                10
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_MSB                                10
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_MASK                               0x00000400


 

#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_LSB                         11
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_MSB                         13
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_MASK                        0x00003800


 

#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_OFFSET                 0x00000000
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_LSB                    14
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_MSB                    16
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_MASK                   0x0001c000


 

#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_OFFSET                            0x00000000
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_LSB                               17
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_MSB                               17
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_MASK                              0x00020000


 

#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_OFFSET                            0x00000000
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_LSB                               18
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_MSB                               18
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_MASK                              0x00040000


 

#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_OFFSET                                 0x00000000
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_LSB                                    19
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_MSB                                    19
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_MASK                                   0x00080000


 

#define RXPT_CLASSIFY_INFO_USE_PPE_OFFSET                                           0x00000000
#define RXPT_CLASSIFY_INFO_USE_PPE_LSB                                              20
#define RXPT_CLASSIFY_INFO_USE_PPE_MSB                                              20
#define RXPT_CLASSIFY_INFO_USE_PPE_MASK                                             0x00100000


 

#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_OFFSET                                0x00000000
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_LSB                                   21
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_MSB                                   21
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_MASK                                  0x00200000


 

#define RXPT_CLASSIFY_INFO_RESERVED_0B_OFFSET                                       0x00000000
#define RXPT_CLASSIFY_INFO_RESERVED_0B_LSB                                          22
#define RXPT_CLASSIFY_INFO_RESERVED_0B_MSB                                          31
#define RXPT_CLASSIFY_INFO_RESERVED_0B_MASK                                         0xffc00000



#endif    
