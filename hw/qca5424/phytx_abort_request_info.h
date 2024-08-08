
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _PHYTX_ABORT_REQUEST_INFO_H_
#define _PHYTX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_PHYTX_ABORT_REQUEST_INFO 1


struct phytx_abort_request_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t phytx_abort_reason                                      :  8,  
                      user_number                                             :  6,  
                      reserved                                                :  2;  
#else
             uint16_t reserved                                                :  2,  
                      user_number                                             :  6,  
                      phytx_abort_reason                                      :  8;  
#endif
};


 

#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_OFFSET                          0x00000000
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_LSB                             0
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_MSB                             7
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_MASK                            0x000000ff


 

#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_OFFSET                                 0x00000000
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_LSB                                    8
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_MSB                                    13
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_MASK                                   0x00003f00


 

#define PHYTX_ABORT_REQUEST_INFO_RESERVED_OFFSET                                    0x00000000
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_LSB                                       14
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_MSB                                       15
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_MASK                                      0x0000c000



#endif    
