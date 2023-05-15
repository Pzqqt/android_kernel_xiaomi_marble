/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PHYTX_PPDU_HEADER_INFO_REQUEST_H_
#define _PHYTX_PPDU_HEADER_INFO_REQUEST_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_PHYTX_PPDU_HEADER_INFO_REQUEST 2

#define NUM_OF_DWORDS_PHYTX_PPDU_HEADER_INFO_REQUEST 1


struct phytx_ppdu_header_info_request {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t request_type                                            :  5, // [4:0]
                      reserved                                                : 11; // [15:5]
             uint16_t tlv32_padding                                           : 16; // [15:0]
#else
             uint16_t reserved                                                : 11, // [15:5]
                      request_type                                            :  5; // [4:0]
             uint16_t tlv32_padding                                           : 16; // [15:0]
#endif
};


/* Description		REQUEST_TYPE

			Reason for the request by PHY 
			<enum 0 request_L_SIG_B> 
			<enum 1  request_L_SIG_A>
			<enum 2 request_USER_DESC> 
			<enum 3  request_HT_SIG>
			<enum 4  request_VHT_SIG_A>
			<enum 5  request_VHT_SIG_B >
			<enum 6 request_TX_SERVICE>
			<enum 7 request_HE_SIG_A>
			<enum 8 request_HE_SIG_B>
			<enum 9 request_U_SIG>
			<enum 10 request_EHT_SIG>
			
			<legal 0-10>
*/

#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_OFFSET                          0x00000000
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_LSB                             0
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_MSB                             4
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_MASK                            0x0000001f


/* Description		RESERVED

			<legal 0>
*/

#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_OFFSET                              0x00000000
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_LSB                                 5
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_MSB                                 15
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_MASK                                0x0000ffe0


/* Description		TLV32_PADDING

			Automatic WORD padding inserted while converting TLV16 to
			 TLV32 for 64 bit ARCH
			<legal 0>
*/

#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_OFFSET                         0x00000002
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_LSB                            0
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_MSB                            15
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_MASK                           0x0000ffff



#endif   // PHYTX_PPDU_HEADER_INFO_REQUEST
