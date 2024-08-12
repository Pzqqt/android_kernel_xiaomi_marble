
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _BUFFER_ADDR_INFO_H_
#define _BUFFER_ADDR_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_BUFFER_ADDR_INFO 2


struct buffer_addr_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t buffer_addr_31_0                                        : 32;  
             uint32_t buffer_addr_39_32                                       :  8,  
                      return_buffer_manager                                   :  4,  
                      sw_buffer_cookie                                        : 20;  
#else
             uint32_t buffer_addr_31_0                                        : 32;  
             uint32_t sw_buffer_cookie                                        : 20,  
                      return_buffer_manager                                   :  4,  
                      buffer_addr_39_32                                       :  8;  
#endif
};


 

#define BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET                                    0x00000000
#define BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB                                       0
#define BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MSB                                       31
#define BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK                                      0xffffffff


 

#define BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET                                   0x00000004
#define BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB                                      0
#define BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MSB                                      7
#define BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK                                     0x000000ff


 

#define BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET                               0x00000004
#define BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB                                  8
#define BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB                                  11
#define BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK                                 0x00000f00


 

#define BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET                                    0x00000004
#define BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_LSB                                       12
#define BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_MSB                                       31
#define BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_MASK                                      0xfffff000



#endif    
