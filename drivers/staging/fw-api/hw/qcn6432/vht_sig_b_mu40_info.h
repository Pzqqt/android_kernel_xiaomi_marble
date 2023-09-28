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

#ifndef _VHT_SIG_B_MU40_INFO_H_
#define _VHT_SIG_B_MU40_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_MU40_INFO 2


struct vht_sig_b_mu40_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 17, // [16:0]
                      mcs                                                     :  4, // [20:17]
                      tail                                                    :  6, // [26:21]
                      reserved_0                                              :  2, // [28:27]
                      mu_user_number                                          :  3; // [31:29]
             uint32_t length_copy                                             : 17, // [16:0]
                      mcs_copy                                                :  4, // [20:17]
                      tail_copy                                               :  6, // [26:21]
                      reserved_1                                              :  5; // [31:27]
#else
             uint32_t mu_user_number                                          :  3, // [31:29]
                      reserved_0                                              :  2, // [28:27]
                      tail                                                    :  6, // [26:21]
                      mcs                                                     :  4, // [20:17]
                      length                                                  : 17; // [16:0]
             uint32_t reserved_1                                              :  5, // [31:27]
                      tail_copy                                               :  6, // [26:21]
                      mcs_copy                                                :  4, // [20:17]
                      length_copy                                             : 17; // [16:0]
#endif
};


/* Description		LENGTH

			VHT-SIG-B Length (in units of 4 octets) = ceiling (LENGTH/4) <legal
			 all>
*/

#define VHT_SIG_B_MU40_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_MU40_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_MU40_INFO_LENGTH_MSB                                              16
#define VHT_SIG_B_MU40_INFO_LENGTH_MASK                                             0x0001ffff


/* Description		MCS

			Modulation as described in vht_sig_a mcs field 
			<legal 0-11>
*/

#define VHT_SIG_B_MU40_INFO_MCS_OFFSET                                              0x00000000
#define VHT_SIG_B_MU40_INFO_MCS_LSB                                                 17
#define VHT_SIG_B_MU40_INFO_MCS_MSB                                                 20
#define VHT_SIG_B_MU40_INFO_MCS_MASK                                                0x001e0000


/* Description		TAIL

			Used to terminate the trellis of the convolutional decoder.
			
			Set to 0.  
			<legal 0>
*/

#define VHT_SIG_B_MU40_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_MU40_INFO_TAIL_LSB                                                21
#define VHT_SIG_B_MU40_INFO_TAIL_MSB                                                26
#define VHT_SIG_B_MU40_INFO_TAIL_MASK                                               0x07e00000


/* Description		RESERVED_0

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU40_INFO_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_B_MU40_INFO_RESERVED_0_LSB                                          27
#define VHT_SIG_B_MU40_INFO_RESERVED_0_MSB                                          28
#define VHT_SIG_B_MU40_INFO_RESERVED_0_MASK                                         0x18000000


/* Description		MU_USER_NUMBER

			Not part of VHT-SIG-B.
			Mapping from user number (BFer hardware specific) to mu_user_number. 
			The reader is directed to the previous chapter (User Number) 
			for a definition of the terms user and mu_user.   <legal
			 0-3>
*/

#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_OFFSET                                   0x00000000
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_LSB                                      29
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_MSB                                      31
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_MASK                                     0xe0000000


/* Description		LENGTH_COPY

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_OFFSET                                      0x00000004
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_LSB                                         0
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_MSB                                         16
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_MASK                                        0x0001ffff


/* Description		MCS_COPY

			Same as "mcs". This field is not valid for RX packets. <legal
			 0-11>
*/

#define VHT_SIG_B_MU40_INFO_MCS_COPY_OFFSET                                         0x00000004
#define VHT_SIG_B_MU40_INFO_MCS_COPY_LSB                                            17
#define VHT_SIG_B_MU40_INFO_MCS_COPY_MSB                                            20
#define VHT_SIG_B_MU40_INFO_MCS_COPY_MASK                                           0x001e0000


/* Description		TAIL_COPY

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU40_INFO_TAIL_COPY_OFFSET                                        0x00000004
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_LSB                                           21
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_MSB                                           26
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_MASK                                          0x07e00000


/* Description		RESERVED_1

			<legal 0>
*/

#define VHT_SIG_B_MU40_INFO_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_B_MU40_INFO_RESERVED_1_LSB                                          27
#define VHT_SIG_B_MU40_INFO_RESERVED_1_MSB                                          31
#define VHT_SIG_B_MU40_INFO_RESERVED_1_MASK                                         0xf8000000



#endif   // VHT_SIG_B_MU40_INFO
