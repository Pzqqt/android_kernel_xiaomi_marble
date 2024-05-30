
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 
 
 
 
 
 
 


#ifndef _WBM_RELEASE_RING_H_
#define _WBM_RELEASE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_RELEASE_RING 8


struct wbm_release_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t release_source_module                                   :  3,  
                      reserved_2a                                             :  3,  
                      buffer_or_desc_type                                     :  3,  
                      reserved_2b                                             : 22,  
                      wbm_internal_error                                      :  1;  
             uint32_t reserved_3a                                             : 32;  
             uint32_t reserved_4a                                             : 32;  
             uint32_t reserved_5a                                             : 32;  
             uint32_t reserved_6a                                             : 32;  
             uint32_t reserved_7a                                             : 28,  
                      looping_count                                           :  4;  
#else
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t wbm_internal_error                                      :  1,  
                      reserved_2b                                             : 22,  
                      buffer_or_desc_type                                     :  3,  
                      reserved_2a                                             :  3,  
                      release_source_module                                   :  3;  
             uint32_t reserved_3a                                             : 32;  
             uint32_t reserved_4a                                             : 32;  
             uint32_t reserved_5a                                             : 32;  
             uint32_t reserved_6a                                             : 32;  
             uint32_t looping_count                                           :  4,  
                      reserved_7a                                             : 28;  
#endif
};


 


 

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET    0x00000000
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB       0
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB       31
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK      0xffffffff


 

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET   0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB      0
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB      7
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK     0x000000ff


 

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB  8
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB  11
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


 

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET    0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB       12
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB       31
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK      0xfffff000


 

#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_OFFSET                               0x00000008
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_LSB                                  0
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_MSB                                  2
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_MASK                                 0x00000007


 

#define WBM_RELEASE_RING_RESERVED_2A_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RESERVED_2A_LSB                                            3
#define WBM_RELEASE_RING_RESERVED_2A_MSB                                            5
#define WBM_RELEASE_RING_RESERVED_2A_MASK                                           0x00000038


 

#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_LSB                                    6
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_MSB                                    8
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_MASK                                   0x000001c0


 

#define WBM_RELEASE_RING_RESERVED_2B_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RESERVED_2B_LSB                                            9
#define WBM_RELEASE_RING_RESERVED_2B_MSB                                            30
#define WBM_RELEASE_RING_RESERVED_2B_MASK                                           0x7ffffe00


 

#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_OFFSET                                  0x00000008
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_LSB                                     31
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_MSB                                     31
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_MASK                                    0x80000000


 

#define WBM_RELEASE_RING_RESERVED_3A_OFFSET                                         0x0000000c
#define WBM_RELEASE_RING_RESERVED_3A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_3A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_3A_MASK                                           0xffffffff


 

#define WBM_RELEASE_RING_RESERVED_4A_OFFSET                                         0x00000010
#define WBM_RELEASE_RING_RESERVED_4A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_4A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_4A_MASK                                           0xffffffff


 

#define WBM_RELEASE_RING_RESERVED_5A_OFFSET                                         0x00000014
#define WBM_RELEASE_RING_RESERVED_5A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_5A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_5A_MASK                                           0xffffffff


 

#define WBM_RELEASE_RING_RESERVED_6A_OFFSET                                         0x00000018
#define WBM_RELEASE_RING_RESERVED_6A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_6A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_6A_MASK                                           0xffffffff


 

#define WBM_RELEASE_RING_RESERVED_7A_OFFSET                                         0x0000001c
#define WBM_RELEASE_RING_RESERVED_7A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_7A_MSB                                            27
#define WBM_RELEASE_RING_RESERVED_7A_MASK                                           0x0fffffff


 

#define WBM_RELEASE_RING_LOOPING_COUNT_OFFSET                                       0x0000001c
#define WBM_RELEASE_RING_LOOPING_COUNT_LSB                                          28
#define WBM_RELEASE_RING_LOOPING_COUNT_MSB                                          31
#define WBM_RELEASE_RING_LOOPING_COUNT_MASK                                         0xf0000000



#endif    
