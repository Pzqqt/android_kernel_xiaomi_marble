
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _PHYRX_HE_SIG_B1_MU_H_
#define _PHYRX_HE_SIG_B1_MU_H_
#if !defined(__ASSEMBLER__)
#endif

#include "he_sig_b1_mu_info.h"
#define NUM_OF_DWORDS_PHYRX_HE_SIG_B1_MU 2

#define NUM_OF_QWORDS_PHYRX_HE_SIG_B1_MU 1


struct phyrx_he_sig_b1_mu {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   he_sig_b1_mu_info                                         phyrx_he_sig_b1_mu_info_details;
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   he_sig_b1_mu_info                                         phyrx_he_sig_b1_mu_info_details;
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_OFFSET     0x0000000000000000
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_LSB        0
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_MSB        7
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_MASK       0x00000000000000ff


 

#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RESERVED_0_OFFSET        0x0000000000000000
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RESERVED_0_LSB           8
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RESERVED_0_MSB           30
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RESERVED_0_MASK          0x000000007fffff00


 

#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_OFFSET 0x0000000000000000
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_LSB 31
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MSB 31
#define PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MASK 0x0000000080000000


 

#define PHYRX_HE_SIG_B1_MU_TLV64_PADDING_OFFSET                                     0x0000000000000000
#define PHYRX_HE_SIG_B1_MU_TLV64_PADDING_LSB                                        32
#define PHYRX_HE_SIG_B1_MU_TLV64_PADDING_MSB                                        63
#define PHYRX_HE_SIG_B1_MU_TLV64_PADDING_MASK                                       0xffffffff00000000



#endif    
