
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _MACTX_VHT_SIG_B_SU20_H_
#define _MACTX_VHT_SIG_B_SU20_H_
#if !defined(__ASSEMBLER__)
#endif

#include "vht_sig_b_su20_info.h"
#define NUM_OF_DWORDS_MACTX_VHT_SIG_B_SU20 2

#define NUM_OF_QWORDS_MACTX_VHT_SIG_B_SU20 1


struct mactx_vht_sig_b_su20 {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   vht_sig_b_su20_info                                       mactx_vht_sig_b_su20_info_details;
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   vht_sig_b_su20_info                                       mactx_vht_sig_b_su20_info_details;
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_LENGTH_OFFSET        0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_LENGTH_LSB           0
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_LENGTH_MSB           16
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_LENGTH_MASK          0x000000000001ffff


 

#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_VHTB_RESERVED_OFFSET 0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_VHTB_RESERVED_LSB    17
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_VHTB_RESERVED_MSB    19
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_VHTB_RESERVED_MASK   0x00000000000e0000


 

#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_TAIL_OFFSET          0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_TAIL_LSB             20
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_TAIL_MSB             25
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_TAIL_MASK            0x0000000003f00000


 

#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RESERVED_OFFSET      0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RESERVED_LSB         26
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RESERVED_MSB         30
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RESERVED_MASK        0x000000007c000000


 

#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RX_NDP_OFFSET        0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RX_NDP_LSB           31
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RX_NDP_MSB           31
#define MACTX_VHT_SIG_B_SU20_MACTX_VHT_SIG_B_SU20_INFO_DETAILS_RX_NDP_MASK          0x0000000080000000


 

#define MACTX_VHT_SIG_B_SU20_TLV64_PADDING_OFFSET                                   0x0000000000000000
#define MACTX_VHT_SIG_B_SU20_TLV64_PADDING_LSB                                      32
#define MACTX_VHT_SIG_B_SU20_TLV64_PADDING_MSB                                      63
#define MACTX_VHT_SIG_B_SU20_TLV64_PADDING_MASK                                     0xffffffff00000000



#endif    
