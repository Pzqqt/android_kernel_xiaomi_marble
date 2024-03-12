
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _TX_FLUSH_REQ_H_
#define _TX_FLUSH_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FLUSH_REQ 2

#define NUM_OF_QWORDS_TX_FLUSH_REQ 1


struct tx_flush_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t flush_req_reason                                        :  8,  
                      phytx_abort_reason                                      :  8,  
                      flush_req_user_number_or_link_id                        :  6,  
                      mlo_abort_reason                                        :  5,  
                      reserved_0a                                             :  5;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t reserved_0a                                             :  5,  
                      mlo_abort_reason                                        :  5,  
                      flush_req_user_number_or_link_id                        :  6,  
                      phytx_abort_reason                                      :  8,  
                      flush_req_reason                                        :  8;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define TX_FLUSH_REQ_FLUSH_REQ_REASON_OFFSET                                        0x0000000000000000
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_LSB                                           0
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MSB                                           7
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MASK                                          0x00000000000000ff


 

#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_OFFSET                                      0x0000000000000000
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_LSB                                         8
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MSB                                         15
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MASK                                        0x000000000000ff00


 

#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_OFFSET                        0x0000000000000000
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_LSB                           16
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MSB                           21
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MASK                          0x00000000003f0000


 

#define TX_FLUSH_REQ_MLO_ABORT_REASON_OFFSET                                        0x0000000000000000
#define TX_FLUSH_REQ_MLO_ABORT_REASON_LSB                                           22
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MSB                                           26
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MASK                                          0x0000000007c00000


 

#define TX_FLUSH_REQ_RESERVED_0A_OFFSET                                             0x0000000000000000
#define TX_FLUSH_REQ_RESERVED_0A_LSB                                                27
#define TX_FLUSH_REQ_RESERVED_0A_MSB                                                31
#define TX_FLUSH_REQ_RESERVED_0A_MASK                                               0x00000000f8000000


 

#define TX_FLUSH_REQ_TLV64_PADDING_OFFSET                                           0x0000000000000000
#define TX_FLUSH_REQ_TLV64_PADDING_LSB                                              32
#define TX_FLUSH_REQ_TLV64_PADDING_MSB                                              63
#define TX_FLUSH_REQ_TLV64_PADDING_MASK                                             0xffffffff00000000



#endif    
