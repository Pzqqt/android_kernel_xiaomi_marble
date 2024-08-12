
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_PREAMBLE_H_
#define _RX_PREAMBLE_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_PREAMBLE 2

#define NUM_OF_QWORDS_RX_PREAMBLE 1


struct rx_preamble {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t num_users                                               :  6,  
                      pkt_type                                                :  4,  
                      direction                                               :  1,  
                      reserved_0a                                             : 21;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t reserved_0a                                             : 21,  
                      direction                                               :  1,  
                      pkt_type                                                :  4,  
                      num_users                                               :  6;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define RX_PREAMBLE_NUM_USERS_OFFSET                                                0x0000000000000000
#define RX_PREAMBLE_NUM_USERS_LSB                                                   0
#define RX_PREAMBLE_NUM_USERS_MSB                                                   5
#define RX_PREAMBLE_NUM_USERS_MASK                                                  0x000000000000003f


 

#define RX_PREAMBLE_PKT_TYPE_OFFSET                                                 0x0000000000000000
#define RX_PREAMBLE_PKT_TYPE_LSB                                                    6
#define RX_PREAMBLE_PKT_TYPE_MSB                                                    9
#define RX_PREAMBLE_PKT_TYPE_MASK                                                   0x00000000000003c0


 

#define RX_PREAMBLE_DIRECTION_OFFSET                                                0x0000000000000000
#define RX_PREAMBLE_DIRECTION_LSB                                                   10
#define RX_PREAMBLE_DIRECTION_MSB                                                   10
#define RX_PREAMBLE_DIRECTION_MASK                                                  0x0000000000000400


 

#define RX_PREAMBLE_RESERVED_0A_OFFSET                                              0x0000000000000000
#define RX_PREAMBLE_RESERVED_0A_LSB                                                 11
#define RX_PREAMBLE_RESERVED_0A_MSB                                                 31
#define RX_PREAMBLE_RESERVED_0A_MASK                                                0x00000000fffff800


 

#define RX_PREAMBLE_TLV64_PADDING_OFFSET                                            0x0000000000000000
#define RX_PREAMBLE_TLV64_PADDING_LSB                                               32
#define RX_PREAMBLE_TLV64_PADDING_MSB                                               63
#define RX_PREAMBLE_TLV64_PADDING_MASK                                              0xffffffff00000000



#endif    
