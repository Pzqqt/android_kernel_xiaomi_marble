
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

 
 
 
 
 
 
 


#ifndef _RX_FLOW_SEARCH_ENTRY_H_
#define _RX_FLOW_SEARCH_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FLOW_SEARCH_ENTRY 16


struct rx_flow_search_entry {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t src_ip_127_96                                           : 32;  
             uint32_t src_ip_95_64                                            : 32;  
             uint32_t src_ip_63_32                                            : 32;  
             uint32_t src_ip_31_0                                             : 32;  
             uint32_t dest_ip_127_96                                          : 32;  
             uint32_t dest_ip_95_64                                           : 32;  
             uint32_t dest_ip_63_32                                           : 32;  
             uint32_t dest_ip_31_0                                            : 32;  
             uint32_t src_port                                                : 16,  
                      dest_port                                               : 16;  
             uint32_t l4_protocol                                             :  8,  
                      valid                                                   :  1,  
                      reserved_9                                              :  4,  
                      service_code                                            :  9,  
                      priority_valid                                          :  1,  
                      use_ppe                                                 :  1,  
                      reo_destination_indication                              :  5,  
                      msdu_drop                                               :  1,  
                      reo_destination_handler                                 :  2;  
             uint32_t metadata                                                : 32;  
             uint32_t aggregation_count                                       :  7,  
                      lro_eligible                                            :  1,  
                      msdu_count                                              : 24;  
             uint32_t msdu_byte_count                                         : 32;  
             uint32_t timestamp                                               : 32;  
             uint32_t cumulative_ip_length_pmac1                              : 16,  
                      cumulative_ip_length                                    : 16;  
             uint32_t tcp_sequence_number                                     : 32;  
#else
             uint32_t src_ip_127_96                                           : 32;  
             uint32_t src_ip_95_64                                            : 32;  
             uint32_t src_ip_63_32                                            : 32;  
             uint32_t src_ip_31_0                                             : 32;  
             uint32_t dest_ip_127_96                                          : 32;  
             uint32_t dest_ip_95_64                                           : 32;  
             uint32_t dest_ip_63_32                                           : 32;  
             uint32_t dest_ip_31_0                                            : 32;  
             uint32_t dest_port                                               : 16,  
                      src_port                                                : 16;  
             uint32_t reo_destination_handler                                 :  2,  
                      msdu_drop                                               :  1,  
                      reo_destination_indication                              :  5,  
                      use_ppe                                                 :  1,  
                      priority_valid                                          :  1,  
                      service_code                                            :  9,  
                      reserved_9                                              :  4,  
                      valid                                                   :  1,  
                      l4_protocol                                             :  8;  
             uint32_t metadata                                                : 32;  
             uint32_t msdu_count                                              : 24,  
                      lro_eligible                                            :  1,  
                      aggregation_count                                       :  7;  
             uint32_t msdu_byte_count                                         : 32;  
             uint32_t timestamp                                               : 32;  
             uint32_t cumulative_ip_length                                    : 16,  
                      cumulative_ip_length_pmac1                              : 16;  
             uint32_t tcp_sequence_number                                     : 32;  
#endif
};


 

#define RX_FLOW_SEARCH_ENTRY_SRC_IP_127_96_OFFSET                                   0x00000000
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_127_96_LSB                                      0
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_127_96_MSB                                      31
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_127_96_MASK                                     0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_SRC_IP_95_64_OFFSET                                    0x00000004
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_95_64_LSB                                       0
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_95_64_MSB                                       31
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_95_64_MASK                                      0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_SRC_IP_63_32_OFFSET                                    0x00000008
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_63_32_LSB                                       0
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_63_32_MSB                                       31
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_63_32_MASK                                      0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_SRC_IP_31_0_OFFSET                                     0x0000000c
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_31_0_LSB                                        0
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_31_0_MSB                                        31
#define RX_FLOW_SEARCH_ENTRY_SRC_IP_31_0_MASK                                       0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_DEST_IP_127_96_OFFSET                                  0x00000010
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_127_96_LSB                                     0
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_127_96_MSB                                     31
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_127_96_MASK                                    0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_DEST_IP_95_64_OFFSET                                   0x00000014
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_95_64_LSB                                      0
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_95_64_MSB                                      31
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_95_64_MASK                                     0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_DEST_IP_63_32_OFFSET                                   0x00000018
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_63_32_LSB                                      0
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_63_32_MSB                                      31
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_63_32_MASK                                     0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_DEST_IP_31_0_OFFSET                                    0x0000001c
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_31_0_LSB                                       0
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_31_0_MSB                                       31
#define RX_FLOW_SEARCH_ENTRY_DEST_IP_31_0_MASK                                      0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_SRC_PORT_OFFSET                                        0x00000020
#define RX_FLOW_SEARCH_ENTRY_SRC_PORT_LSB                                           0
#define RX_FLOW_SEARCH_ENTRY_SRC_PORT_MSB                                           15
#define RX_FLOW_SEARCH_ENTRY_SRC_PORT_MASK                                          0x0000ffff


 

#define RX_FLOW_SEARCH_ENTRY_DEST_PORT_OFFSET                                       0x00000020
#define RX_FLOW_SEARCH_ENTRY_DEST_PORT_LSB                                          16
#define RX_FLOW_SEARCH_ENTRY_DEST_PORT_MSB                                          31
#define RX_FLOW_SEARCH_ENTRY_DEST_PORT_MASK                                         0xffff0000


 

#define RX_FLOW_SEARCH_ENTRY_L4_PROTOCOL_OFFSET                                     0x00000024
#define RX_FLOW_SEARCH_ENTRY_L4_PROTOCOL_LSB                                        0
#define RX_FLOW_SEARCH_ENTRY_L4_PROTOCOL_MSB                                        7
#define RX_FLOW_SEARCH_ENTRY_L4_PROTOCOL_MASK                                       0x000000ff


 

#define RX_FLOW_SEARCH_ENTRY_VALID_OFFSET                                           0x00000024
#define RX_FLOW_SEARCH_ENTRY_VALID_LSB                                              8
#define RX_FLOW_SEARCH_ENTRY_VALID_MSB                                              8
#define RX_FLOW_SEARCH_ENTRY_VALID_MASK                                             0x00000100


 

#define RX_FLOW_SEARCH_ENTRY_RESERVED_9_OFFSET                                      0x00000024
#define RX_FLOW_SEARCH_ENTRY_RESERVED_9_LSB                                         9
#define RX_FLOW_SEARCH_ENTRY_RESERVED_9_MSB                                         12
#define RX_FLOW_SEARCH_ENTRY_RESERVED_9_MASK                                        0x00001e00


 

#define RX_FLOW_SEARCH_ENTRY_SERVICE_CODE_OFFSET                                    0x00000024
#define RX_FLOW_SEARCH_ENTRY_SERVICE_CODE_LSB                                       13
#define RX_FLOW_SEARCH_ENTRY_SERVICE_CODE_MSB                                       21
#define RX_FLOW_SEARCH_ENTRY_SERVICE_CODE_MASK                                      0x003fe000


 

#define RX_FLOW_SEARCH_ENTRY_PRIORITY_VALID_OFFSET                                  0x00000024
#define RX_FLOW_SEARCH_ENTRY_PRIORITY_VALID_LSB                                     22
#define RX_FLOW_SEARCH_ENTRY_PRIORITY_VALID_MSB                                     22
#define RX_FLOW_SEARCH_ENTRY_PRIORITY_VALID_MASK                                    0x00400000


 

#define RX_FLOW_SEARCH_ENTRY_USE_PPE_OFFSET                                         0x00000024
#define RX_FLOW_SEARCH_ENTRY_USE_PPE_LSB                                            23
#define RX_FLOW_SEARCH_ENTRY_USE_PPE_MSB                                            23
#define RX_FLOW_SEARCH_ENTRY_USE_PPE_MASK                                           0x00800000


 

#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_INDICATION_OFFSET                      0x00000024
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_INDICATION_LSB                         24
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_INDICATION_MSB                         28
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_INDICATION_MASK                        0x1f000000


 

#define RX_FLOW_SEARCH_ENTRY_MSDU_DROP_OFFSET                                       0x00000024
#define RX_FLOW_SEARCH_ENTRY_MSDU_DROP_LSB                                          29
#define RX_FLOW_SEARCH_ENTRY_MSDU_DROP_MSB                                          29
#define RX_FLOW_SEARCH_ENTRY_MSDU_DROP_MASK                                         0x20000000


 

#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_HANDLER_OFFSET                         0x00000024
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_HANDLER_LSB                            30
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_HANDLER_MSB                            31
#define RX_FLOW_SEARCH_ENTRY_REO_DESTINATION_HANDLER_MASK                           0xc0000000


 

#define RX_FLOW_SEARCH_ENTRY_METADATA_OFFSET                                        0x00000028
#define RX_FLOW_SEARCH_ENTRY_METADATA_LSB                                           0
#define RX_FLOW_SEARCH_ENTRY_METADATA_MSB                                           31
#define RX_FLOW_SEARCH_ENTRY_METADATA_MASK                                          0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_AGGREGATION_COUNT_OFFSET                               0x0000002c
#define RX_FLOW_SEARCH_ENTRY_AGGREGATION_COUNT_LSB                                  0
#define RX_FLOW_SEARCH_ENTRY_AGGREGATION_COUNT_MSB                                  6
#define RX_FLOW_SEARCH_ENTRY_AGGREGATION_COUNT_MASK                                 0x0000007f


 

#define RX_FLOW_SEARCH_ENTRY_LRO_ELIGIBLE_OFFSET                                    0x0000002c
#define RX_FLOW_SEARCH_ENTRY_LRO_ELIGIBLE_LSB                                       7
#define RX_FLOW_SEARCH_ENTRY_LRO_ELIGIBLE_MSB                                       7
#define RX_FLOW_SEARCH_ENTRY_LRO_ELIGIBLE_MASK                                      0x00000080


 

#define RX_FLOW_SEARCH_ENTRY_MSDU_COUNT_OFFSET                                      0x0000002c
#define RX_FLOW_SEARCH_ENTRY_MSDU_COUNT_LSB                                         8
#define RX_FLOW_SEARCH_ENTRY_MSDU_COUNT_MSB                                         31
#define RX_FLOW_SEARCH_ENTRY_MSDU_COUNT_MASK                                        0xffffff00


 

#define RX_FLOW_SEARCH_ENTRY_MSDU_BYTE_COUNT_OFFSET                                 0x00000030
#define RX_FLOW_SEARCH_ENTRY_MSDU_BYTE_COUNT_LSB                                    0
#define RX_FLOW_SEARCH_ENTRY_MSDU_BYTE_COUNT_MSB                                    31
#define RX_FLOW_SEARCH_ENTRY_MSDU_BYTE_COUNT_MASK                                   0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_TIMESTAMP_OFFSET                                       0x00000034
#define RX_FLOW_SEARCH_ENTRY_TIMESTAMP_LSB                                          0
#define RX_FLOW_SEARCH_ENTRY_TIMESTAMP_MSB                                          31
#define RX_FLOW_SEARCH_ENTRY_TIMESTAMP_MASK                                         0xffffffff


 

#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_PMAC1_OFFSET                      0x00000038
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_PMAC1_LSB                         0
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_PMAC1_MSB                         15
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_PMAC1_MASK                        0x0000ffff


 

#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_OFFSET                            0x00000038
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_LSB                               16
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_MSB                               31
#define RX_FLOW_SEARCH_ENTRY_CUMULATIVE_IP_LENGTH_MASK                              0xffff0000


 

#define RX_FLOW_SEARCH_ENTRY_TCP_SEQUENCE_NUMBER_OFFSET                             0x0000003c
#define RX_FLOW_SEARCH_ENTRY_TCP_SEQUENCE_NUMBER_LSB                                0
#define RX_FLOW_SEARCH_ENTRY_TCP_SEQUENCE_NUMBER_MSB                                31
#define RX_FLOW_SEARCH_ENTRY_TCP_SEQUENCE_NUMBER_MASK                               0xffffffff



#endif    
