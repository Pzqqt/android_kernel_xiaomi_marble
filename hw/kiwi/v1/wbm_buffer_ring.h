
/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */











#ifndef _WBM_BUFFER_RING_H_
#define _WBM_BUFFER_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_BUFFER_RING 2


struct wbm_buffer_ring {
	     struct   buffer_addr_info                                          buf_addr_info;
};







#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET                       0x00000000
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_31_0_LSB                          0
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MSB                          31
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MASK                         0xffffffff




#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET                      0x00000004
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_39_32_LSB                         0
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MSB                         7
#define WBM_BUFFER_RING_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MASK                        0x000000ff




#define WBM_BUFFER_RING_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET                  0x00000004
#define WBM_BUFFER_RING_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB                     8
#define WBM_BUFFER_RING_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB                     11
#define WBM_BUFFER_RING_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK                    0x00000f00




#define WBM_BUFFER_RING_BUF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET                       0x00000004
#define WBM_BUFFER_RING_BUF_ADDR_INFO_SW_BUFFER_COOKIE_LSB                          12
#define WBM_BUFFER_RING_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MSB                          31
#define WBM_BUFFER_RING_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MASK                         0xfffff000



#endif
