
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _MACTX_SERVICE_H_
#define _MACTX_SERVICE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "service_info.h"
#define NUM_OF_DWORDS_MACTX_SERVICE 2

#define NUM_OF_QWORDS_MACTX_SERVICE 1


struct mactx_service {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   service_info                                              mactx_service_info_details;
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   service_info                                              mactx_service_info_details;
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_OFFSET              0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_LSB                 0
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_MSB                 6
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_MASK                0x000000000000007f


 

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_OFFSET                    0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_LSB                       7
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_MSB                       7
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_MASK                      0x0000000000000080


 

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_OFFSET              0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_LSB                 8
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_MSB                 15
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_MASK                0x000000000000ff00


 

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_OFFSET                  0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_LSB                     16
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_MSB                     31
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_MASK                    0x00000000ffff0000


 

#define MACTX_SERVICE_TLV64_PADDING_OFFSET                                          0x0000000000000000
#define MACTX_SERVICE_TLV64_PADDING_LSB                                             32
#define MACTX_SERVICE_TLV64_PADDING_MSB                                             63
#define MACTX_SERVICE_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif    
