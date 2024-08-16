
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _PHYRX_L_SIG_B_H_
#define _PHYRX_L_SIG_B_H_
#if !defined(__ASSEMBLER__)
#endif

#include "l_sig_b_info.h"
#define NUM_OF_DWORDS_PHYRX_L_SIG_B 2

#define NUM_OF_QWORDS_PHYRX_L_SIG_B 1


struct phyrx_l_sig_b {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   l_sig_b_info                                              phyrx_l_sig_b_info_details;
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   l_sig_b_info                                              phyrx_l_sig_b_info_details;
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET                        0x0000000000000000
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_LSB                           0
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_MSB                           3
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_MASK                          0x000000000000000f


 

#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_LENGTH_OFFSET                      0x0000000000000000
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_LENGTH_LSB                         4
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_LENGTH_MSB                         15
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_LENGTH_MASK                        0x000000000000fff0


 

#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RESERVED_OFFSET                    0x0000000000000000
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RESERVED_LSB                       16
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RESERVED_MSB                       30
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RESERVED_MASK                      0x000000007fff0000


 

#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_OFFSET   0x0000000000000000
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_LSB      31
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MSB      31
#define PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MASK     0x0000000080000000


 

#define PHYRX_L_SIG_B_TLV64_PADDING_OFFSET                                          0x0000000000000000
#define PHYRX_L_SIG_B_TLV64_PADDING_LSB                                             32
#define PHYRX_L_SIG_B_TLV64_PADDING_MSB                                             63
#define PHYRX_L_SIG_B_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif    
