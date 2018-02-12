/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _RX_MPDU_LINK_PTR_H_
#define _RX_MPDU_LINK_PTR_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct buffer_addr_info mpdu_link_desc_addr_info;
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_MPDU_LINK_PTR 2

struct rx_mpdu_link_ptr {
    struct            buffer_addr_info                       mpdu_link_desc_addr_info;
};

/*

struct buffer_addr_info mpdu_link_desc_addr_info
			
			Details of the physical address of an MPDU link
			descriptor
*/

#define RX_MPDU_LINK_PTR_0_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_OFFSET 0x00000000
#define RX_MPDU_LINK_PTR_0_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_LSB 0
#define RX_MPDU_LINK_PTR_0_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_MASK 0xffffffff
#define RX_MPDU_LINK_PTR_1_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_OFFSET 0x00000004
#define RX_MPDU_LINK_PTR_1_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_LSB 0
#define RX_MPDU_LINK_PTR_1_BUFFER_ADDR_INFO_MPDU_LINK_DESC_ADDR_INFO_MASK 0xffffffff


#endif // _RX_MPDU_LINK_PTR_H_
