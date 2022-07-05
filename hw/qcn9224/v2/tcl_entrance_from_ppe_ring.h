
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

 
 
 
 
 
 
 


#ifndef _TCL_ENTRANCE_FROM_PPE_RING_H_
#define _TCL_ENTRANCE_FROM_PPE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TCL_ENTRANCE_FROM_PPE_RING 8


struct tcl_entrance_from_ppe_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t buffer_addr_lo                                          : 32;  
             uint32_t buffer_addr_hi                                          :  8,  
                      drop_prec                                               :  2,  
                      fake_mac_header                                         :  1,  
                      known_ind                                               :  1,  
                      cpu_code_valid                                          :  1,  
                      tunnel_term_ind                                         :  1,  
                      tunnel_type                                             :  1,  
                      wifi_qos_flag                                           :  1,  
                      service_code                                            :  9,  
                      reserved_1b                                             :  1,  
                      int_pri                                                 :  4,  
                      more                                                    :  1,  
                      reserved_1a                                             :  1;  
             uint32_t opaque_lo                                               : 32;  
             uint32_t opaque_hi                                               : 32;  
             uint32_t src_info                                                : 16,  
                      dst_info                                                : 16;  
             uint32_t data_length                                             : 18,  
                      pool_id                                                 :  6,  
                      wifi_qos                                                :  8;  
             uint32_t data_offset                                             : 12,  
                      l4_csum_status                                          :  1,  
                      l3_csum_status                                          :  1,  
                      hash_flag                                               :  2,  
                      hash_value                                              : 16;  
             uint32_t dscp                                                    :  8,  
                      valid_toggle                                            :  1,  
                      pppoe_flag                                              :  1,  
                      svlan_flag                                              :  1,  
                      cvlan_flag                                              :  1,  
                      pid                                                     :  4,  
                      l3_offset                                               :  8,  
                      l4_offset                                               :  8;  
#else
             uint32_t buffer_addr_lo                                          : 32;  
             uint32_t reserved_1a                                             :  1,  
                      more                                                    :  1,  
                      int_pri                                                 :  4,  
                      reserved_1b                                             :  1,  
                      service_code                                            :  9,  
                      wifi_qos_flag                                           :  1,  
                      tunnel_type                                             :  1,  
                      tunnel_term_ind                                         :  1,  
                      cpu_code_valid                                          :  1,  
                      known_ind                                               :  1,  
                      fake_mac_header                                         :  1,  
                      drop_prec                                               :  2,  
                      buffer_addr_hi                                          :  8;  
             uint32_t opaque_lo                                               : 32;  
             uint32_t opaque_hi                                               : 32;  
             uint32_t dst_info                                                : 16,  
                      src_info                                                : 16;  
             uint32_t wifi_qos                                                :  8,  
                      pool_id                                                 :  6,  
                      data_length                                             : 18;  
             uint32_t hash_value                                              : 16,  
                      hash_flag                                               :  2,  
                      l3_csum_status                                          :  1,  
                      l4_csum_status                                          :  1,  
                      data_offset                                             : 12;  
             uint32_t l4_offset                                               :  8,  
                      l3_offset                                               :  8,  
                      pid                                                     :  4,  
                      cvlan_flag                                              :  1,  
                      svlan_flag                                              :  1,  
                      pppoe_flag                                              :  1,  
                      valid_toggle                                            :  1,  
                      dscp                                                    :  8;  
#endif
};


 

#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_OFFSET                            0x00000000
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_LSB                               0
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_MSB                               31
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_MASK                              0xffffffff


 

#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_OFFSET                            0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_LSB                               0
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_MSB                               7
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_MASK                              0x000000ff


 

#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_OFFSET                                 0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_LSB                                    8
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_MSB                                    9
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_MASK                                   0x00000300


 

#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_OFFSET                           0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_LSB                              10
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_MSB                              10
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_MASK                             0x00000400


 

#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_OFFSET                                 0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_LSB                                    11
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_MSB                                    11
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_MASK                                   0x00000800


 

#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_OFFSET                            0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_LSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_MSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_MASK                              0x00001000


 

#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_OFFSET                           0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_LSB                              13
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_MSB                              13
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_MASK                             0x00002000


 

#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_LSB                                  14
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_MSB                                  14
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_MASK                                 0x00004000


 

#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_OFFSET                             0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_LSB                                15
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_MSB                                15
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_MASK                               0x00008000


 

#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_OFFSET                              0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_LSB                                 16
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_MSB                                 24
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_MASK                                0x01ff0000


 

#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_LSB                                  25
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_MSB                                  25
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_MASK                                 0x02000000


 

#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_OFFSET                                   0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_LSB                                      26
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_MSB                                      29
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_MASK                                     0x3c000000


 

#define TCL_ENTRANCE_FROM_PPE_RING_MORE_OFFSET                                      0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_LSB                                         30
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_MSB                                         30
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_MASK                                        0x40000000


 

#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_LSB                                  31
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_MSB                                  31
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_MASK                                 0x80000000


 

#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_OFFSET                                 0x00000008
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_LSB                                    0
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_MASK                                   0xffffffff


 

#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_OFFSET                                 0x0000000c
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_LSB                                    0
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_MASK                                   0xffffffff


 

#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_OFFSET                                  0x00000010
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_LSB                                     0
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_MSB                                     15
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_MASK                                    0x0000ffff


 

#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_OFFSET                                  0x00000010
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_LSB                                     16
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_MSB                                     31
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_MASK                                    0xffff0000


 

#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_OFFSET                               0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_LSB                                  0
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_MSB                                  17
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_MASK                                 0x0003ffff


 

#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_OFFSET                                   0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_LSB                                      18
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_MSB                                      23
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_MASK                                     0x00fc0000


 

#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_OFFSET                                  0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_LSB                                     24
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_MSB                                     31
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_MASK                                    0xff000000


 

#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_OFFSET                               0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_LSB                                  0
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_MSB                                  11
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_MASK                                 0x00000fff


 

#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_OFFSET                            0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_LSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_MSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_MASK                              0x00001000


 

#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_OFFSET                            0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_LSB                               13
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_MSB                               13
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_MASK                              0x00002000


 

#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_OFFSET                                 0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_LSB                                    14
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_MSB                                    15
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_MASK                                   0x0000c000


 

#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_OFFSET                                0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_LSB                                   16
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_MSB                                   31
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_MASK                                  0xffff0000


 

#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_OFFSET                                      0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_LSB                                         0
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_MSB                                         7
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_MASK                                        0x000000ff


 

#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_OFFSET                              0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_LSB                                 8
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_MSB                                 8
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_MASK                                0x00000100


 

#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_LSB                                   9
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_MSB                                   9
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_MASK                                  0x00000200


 

#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_LSB                                   10
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_MSB                                   10
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_MASK                                  0x00000400


 

#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_LSB                                   11
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_MSB                                   11
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_MASK                                  0x00000800


 

#define TCL_ENTRANCE_FROM_PPE_RING_PID_OFFSET                                       0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_PID_LSB                                          12
#define TCL_ENTRANCE_FROM_PPE_RING_PID_MSB                                          15
#define TCL_ENTRANCE_FROM_PPE_RING_PID_MASK                                         0x0000f000


 

#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_OFFSET                                 0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_LSB                                    16
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_MSB                                    23
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_MASK                                   0x00ff0000


 

#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_OFFSET                                 0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_LSB                                    24
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_MASK                                   0xff000000



#endif    
