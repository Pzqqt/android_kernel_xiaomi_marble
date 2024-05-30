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
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   service_info                                              mactx_service_info_details;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		MACTX_SERVICE_INFO_DETAILS

			See detailed description of the STRUCT.
			
			In case of EHT, instead of 'SERVICE_INFO' the STRUCT 'EHT_SERVICE_INFO' 
			is used. See detailed description of the STRUCT.
*/


/* Description		SCRAMBLER_SEED

			This field provides the 7-bit seed for the data scrambler. 
			 <legal all>
*/

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_OFFSET              0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_LSB                 0
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_MSB                 6
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SCRAMBLER_SEED_MASK                0x000000000000007f


/* Description		RESERVED

			Reserved. Set to 0 by sender and ignored by receiver.  <legal
			 0>
*/

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_OFFSET                    0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_LSB                       7
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_MSB                       7
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_MASK                      0x0000000000000080


/* Description		SIG_B_CRC_USER

			In case of vht transmission: vht_sig_b_crc_user
			<legal all>
*/

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_OFFSET              0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_LSB                 8
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_MSB                 15
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_SIG_B_CRC_USER_MASK                0x000000000000ff00


/* Description		RESERVED_1

			<legal 0>
*/

#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_OFFSET                  0x0000000000000000
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_LSB                     16
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_MSB                     31
#define MACTX_SERVICE_MACTX_SERVICE_INFO_DETAILS_RESERVED_1_MASK                    0x00000000ffff0000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define MACTX_SERVICE_TLV64_PADDING_OFFSET                                          0x0000000000000000
#define MACTX_SERVICE_TLV64_PADDING_LSB                                             32
#define MACTX_SERVICE_TLV64_PADDING_MSB                                             63
#define MACTX_SERVICE_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif   // MACTX_SERVICE
