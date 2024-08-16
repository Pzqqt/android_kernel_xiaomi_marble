
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _MACRX_ABORT_REQUEST_INFO_H_
#define _MACRX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_MACRX_ABORT_REQUEST_INFO 1


struct macrx_abort_request_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t macrx_abort_reason                                      :  8,  
                      reserved_0                                              :  8;  
#else
             uint16_t reserved_0                                              :  8,  
                      macrx_abort_reason                                      :  8;  
#endif
};


 

#define MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REASON_OFFSET                          0x00000000
#define MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REASON_LSB                             0
#define MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REASON_MSB                             7
#define MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REASON_MASK                            0x000000ff


 

#define MACRX_ABORT_REQUEST_INFO_RESERVED_0_OFFSET                                  0x00000000
#define MACRX_ABORT_REQUEST_INFO_RESERVED_0_LSB                                     8
#define MACRX_ABORT_REQUEST_INFO_RESERVED_0_MSB                                     15
#define MACRX_ABORT_REQUEST_INFO_RESERVED_0_MASK                                    0x0000ff00



#endif    
