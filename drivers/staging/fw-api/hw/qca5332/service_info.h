
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

 
 
 
 
 
 
 


#ifndef _SERVICE_INFO_H_
#define _SERVICE_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_SERVICE_INFO 1


struct service_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t scrambler_seed                                          :  7, // [6:0]
                      reserved                                                :  1, // [7:7]
                      sig_b_crc_user                                          :  8, // [15:8]
                      reserved_1                                              : 16; // [31:16]
#else
             uint32_t reserved_1                                              : 16, // [31:16]
                      sig_b_crc_user                                          :  8, // [15:8]
                      reserved                                                :  1, // [7:7]
                      scrambler_seed                                          :  7; // [6:0]
#endif
};


/* Description		SCRAMBLER_SEED

			This field provides the 7-bit seed for the data scrambler. 
			 <legal all>
*/

#define SERVICE_INFO_SCRAMBLER_SEED_OFFSET                                          0x00000000
#define SERVICE_INFO_SCRAMBLER_SEED_LSB                                             0
#define SERVICE_INFO_SCRAMBLER_SEED_MSB                                             6
#define SERVICE_INFO_SCRAMBLER_SEED_MASK                                            0x0000007f


/* Description		RESERVED

			Reserved. Set to 0 by sender and ignored by receiver.  <legal
			 0>
*/

#define SERVICE_INFO_RESERVED_OFFSET                                                0x00000000
#define SERVICE_INFO_RESERVED_LSB                                                   7
#define SERVICE_INFO_RESERVED_MSB                                                   7
#define SERVICE_INFO_RESERVED_MASK                                                  0x00000080


/* Description		SIG_B_CRC_USER

			In case of vht transmission: vht_sig_b_crc_user
			<legal all>
*/

#define SERVICE_INFO_SIG_B_CRC_USER_OFFSET                                          0x00000000
#define SERVICE_INFO_SIG_B_CRC_USER_LSB                                             8
#define SERVICE_INFO_SIG_B_CRC_USER_MSB                                             15
#define SERVICE_INFO_SIG_B_CRC_USER_MASK                                            0x0000ff00


/* Description		RESERVED_1

			<legal 0>
*/

#define SERVICE_INFO_RESERVED_1_OFFSET                                              0x00000000
#define SERVICE_INFO_RESERVED_1_LSB                                                 16
#define SERVICE_INFO_RESERVED_1_MSB                                                 31
#define SERVICE_INFO_RESERVED_1_MASK                                                0xffff0000



#endif   // SERVICE_INFO
