
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _WBM_LINK_DESCRIPTOR_RING_H_
#define _WBM_LINK_DESCRIPTOR_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_LINK_DESCRIPTOR_RING 2


struct wbm_link_descriptor_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          desc_addr_info;
#else
             struct   buffer_addr_info                                          desc_addr_info;
#endif
};


 


 

#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET             0x00000000
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB                0
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB                31
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK               0xffffffff


 

#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET            0x00000004
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB               0
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB               7
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK              0x000000ff


 

#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET        0x00000004
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB           8
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB           11
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK          0x00000f00


 

#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET             0x00000004
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB                12
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB                31
#define WBM_LINK_DESCRIPTOR_RING_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK               0xfffff000



#endif    
