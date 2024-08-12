
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_REO_QUEUE_REFERENCE_H_
#define _RX_REO_QUEUE_REFERENCE_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_REO_QUEUE_REFERENCE 2


struct rx_reo_queue_reference {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8,  
                      reserved_1                                              :  8,  
                      receive_queue_number                                    : 16;  
#else
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t receive_queue_number                                    : 16,  
                      reserved_1                                              :  8,  
                      rx_reo_queue_desc_addr_39_32                            :  8;  
#endif
};


 

#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                   0x00000000
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                      0
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                      31
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                     0xffffffff


 

#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                  0x00000004
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                     0
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                     7
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                    0x000000ff


 

#define RX_REO_QUEUE_REFERENCE_RESERVED_1_OFFSET                                    0x00000004
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_LSB                                       8
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_MSB                                       15
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_MASK                                      0x0000ff00


 

#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_OFFSET                          0x00000004
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_LSB                             16
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_MSB                             31
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_MASK                            0xffff0000



#endif    
