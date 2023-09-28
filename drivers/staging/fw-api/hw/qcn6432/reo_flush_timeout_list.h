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

#ifndef _REO_FLUSH_TIMEOUT_LIST_H_
#define _REO_FLUSH_TIMEOUT_LIST_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_TIMEOUT_LIST 10

#define NUM_OF_QWORDS_REO_FLUSH_TIMEOUT_LIST 5


struct reo_flush_timeout_list {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t ac_timout_list                                          :  2, // [1:0]
                      reserved_1                                              : 30; // [31:2]
             uint32_t minimum_release_desc_count                              : 16, // [15:0]
                      minimum_forward_buf_count                               : 16; // [31:16]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t reserved_1                                              : 30, // [31:2]
                      ac_timout_list                                          :  2; // [1:0]
             uint32_t minimum_forward_buf_count                               : 16, // [31:16]
                      minimum_release_desc_count                              : 16; // [15:0]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		CMD_HEADER

			Consumer: REO
			Producer: SW
			
			Details for command execution tracking purposes.
*/


/* Description		REO_CMD_NUMBER

			Consumer: REO/SW/DEBUG
			Producer: SW 
			
			This number can be used by SW to track, identify and link
			 the created commands with the command statusses
			
			
			<legal all> 
*/

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_OFFSET                     0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_LSB                        0
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_MSB                        15
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_MASK                       0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_LSB                   16
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_MSB                   16
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_MASK                  0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_OFFSET                        0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_LSB                           17
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_MSB                           31
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_MASK                          0x00000000fffe0000


/* Description		AC_TIMOUT_LIST

			Consumer: REO
			Producer: SW
			
			The AC_timeout list to be used for this command
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_OFFSET                                0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_LSB                                   32
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_MSB                                   33
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_MASK                                  0x0000000300000000


/* Description		RESERVED_1

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_OFFSET                                    0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_LSB                                       34
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_MSB                                       63
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_MASK                                      0xfffffffc00000000


/* Description		MINIMUM_RELEASE_DESC_COUNT

			Consumer: REO
			Producer: SW
			
			The minimum number of link descriptors requested to be released. 
			If set to 0, only buffer release counts seems to be important... 
			When set to very high value, likely the entire timeout list
			 will be exhausted before this count is reached or maybe
			 this count will not get reached. REO however will stop 
			here as it can not do anything else.
			
			When both this field and field Minimum_forward_buf_count
			 are > 0, REO needs to meet both requirements. When both
			 entries are 0 (which should be a programming error), REO
			 does not need to do anything.
			
			Note that this includes counts of MPDU link Desc as well
			 as MSDU link Desc. Where the count of MSDU link Desc is
			 not known to REO it's approximated by deriving from MSDU
			 count
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_OFFSET                    0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_LSB                       0
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_MSB                       15
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_MASK                      0x000000000000ffff


/* Description		MINIMUM_FORWARD_BUF_COUNT

			Consumer: REO
			Producer: SW
			
			The minimum number of buffer descriptors requested to be
			 passed on to the REO destination rings. 
			
			If set to 0, only descriptor release counts seems to be 
			important... 
			
			When set to very high value, likely the entire timeout list
			 will be exhausted before this count is reached or maybe
			 this count will not get reached. REO however will stop 
			here as it can not do anything else.
			
			Note that REO does not know the exact buffer count. This
			 can be approximated by using the MSDU_COUNT
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_OFFSET                     0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_LSB                        16
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_MSB                        31
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_MASK                       0x00000000ffff0000


/* Description		RESERVED_3A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_OFFSET                                   0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_LSB                                      32
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_MSB                                      63
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_MASK                                     0xffffffff00000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_OFFSET                                   0x0000000000000010
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_MASK                                     0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_OFFSET                                   0x0000000000000010
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_LSB                                      32
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_MSB                                      63
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_MASK                                     0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_OFFSET                                   0x0000000000000018
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_MASK                                     0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_OFFSET                                   0x0000000000000018
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_LSB                                      32
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_MSB                                      63
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_MASK                                     0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_OFFSET                                   0x0000000000000020
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_MASK                                     0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_TLV64_PADDING_OFFSET                                 0x0000000000000020
#define REO_FLUSH_TIMEOUT_LIST_TLV64_PADDING_LSB                                    32
#define REO_FLUSH_TIMEOUT_LIST_TLV64_PADDING_MSB                                    63
#define REO_FLUSH_TIMEOUT_LIST_TLV64_PADDING_MASK                                   0xffffffff00000000



#endif   // REO_FLUSH_TIMEOUT_LIST
