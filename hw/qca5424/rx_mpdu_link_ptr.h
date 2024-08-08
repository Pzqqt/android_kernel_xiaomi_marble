
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_MPDU_LINK_PTR_H_
#define _RX_MPDU_LINK_PTR_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_RX_MPDU_LINK_PTR 2


struct rx_mpdu_link_ptr {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          mpdu_link_desc_addr_info;
#else
             struct   buffer_addr_info                                          mpdu_link_desc_addr_info;
#endif
};


 


 

#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET           0x00000000
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB              0
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB              31
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK             0xffffffff


 

#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET          0x00000004
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB             0
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB             7
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK            0x000000ff


 

#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET      0x00000004
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB         8
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB         11
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK        0x00000f00


 

#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET           0x00000004
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB              12
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB              31
#define RX_MPDU_LINK_PTR_MPDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK             0xfffff000



#endif    
