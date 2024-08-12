
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _UNIFORM_DESCRIPTOR_HEADER_H_
#define _UNIFORM_DESCRIPTOR_HEADER_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_UNIFORM_DESCRIPTOR_HEADER 1


struct uniform_descriptor_header {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t owner                                                   :  4,  
                      buffer_type                                             :  4,  
                      tx_mpdu_queue_number                                    : 20,  
                      reserved_0a                                             :  4;  
#else
             uint32_t reserved_0a                                             :  4,  
                      tx_mpdu_queue_number                                    : 20,  
                      buffer_type                                             :  4,  
                      owner                                                   :  4;  
#endif
};


 

#define UNIFORM_DESCRIPTOR_HEADER_OWNER_OFFSET                                      0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_LSB                                         0
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MSB                                         3
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MASK                                        0x0000000f


 

#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB                                   4
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MSB                                   7
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK                                  0x000000f0


 

#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_OFFSET                       0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_LSB                          8
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MSB                          27
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MASK                         0x0fffff00


 

#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_LSB                                   28
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MSB                                   31
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MASK                                  0xf0000000



#endif    
