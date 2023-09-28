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

#ifndef _VHT_SIG_B_MU160_INFO_H_
#define _VHT_SIG_B_MU160_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_MU160_INFO 8


struct vht_sig_b_mu160_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 19, // [18:0]
                      mcs                                                     :  4, // [22:19]
                      tail                                                    :  6, // [28:23]
                      reserved_0                                              :  3; // [31:29]
             uint32_t length_copy_a                                           : 19, // [18:0]
                      mcs_copy_a                                              :  4, // [22:19]
                      tail_copy_a                                             :  6, // [28:23]
                      reserved_1                                              :  3; // [31:29]
             uint32_t length_copy_b                                           : 19, // [18:0]
                      mcs_copy_b                                              :  4, // [22:19]
                      tail_copy_b                                             :  6, // [28:23]
                      reserved_2                                              :  3; // [31:29]
             uint32_t length_copy_c                                           : 19, // [18:0]
                      mcs_copy_c                                              :  4, // [22:19]
                      tail_copy_c                                             :  6, // [28:23]
                      reserved_3                                              :  3; // [31:29]
             uint32_t length_copy_d                                           : 19, // [18:0]
                      mcs_copy_d                                              :  4, // [22:19]
                      tail_copy_d                                             :  6, // [28:23]
                      reserved_4                                              :  3; // [31:29]
             uint32_t length_copy_e                                           : 19, // [18:0]
                      mcs_copy_e                                              :  4, // [22:19]
                      tail_copy_e                                             :  6, // [28:23]
                      reserved_5                                              :  3; // [31:29]
             uint32_t length_copy_f                                           : 19, // [18:0]
                      mcs_copy_f                                              :  4, // [22:19]
                      tail_copy_f                                             :  6, // [28:23]
                      mu_user_number                                          :  3; // [31:29]
             uint32_t length_copy_g                                           : 19, // [18:0]
                      mcs_copy_g                                              :  4, // [22:19]
                      tail_copy_g                                             :  6, // [28:23]
                      reserved_7                                              :  3; // [31:29]
#else
             uint32_t reserved_0                                              :  3, // [31:29]
                      tail                                                    :  6, // [28:23]
                      mcs                                                     :  4, // [22:19]
                      length                                                  : 19; // [18:0]
             uint32_t reserved_1                                              :  3, // [31:29]
                      tail_copy_a                                             :  6, // [28:23]
                      mcs_copy_a                                              :  4, // [22:19]
                      length_copy_a                                           : 19; // [18:0]
             uint32_t reserved_2                                              :  3, // [31:29]
                      tail_copy_b                                             :  6, // [28:23]
                      mcs_copy_b                                              :  4, // [22:19]
                      length_copy_b                                           : 19; // [18:0]
             uint32_t reserved_3                                              :  3, // [31:29]
                      tail_copy_c                                             :  6, // [28:23]
                      mcs_copy_c                                              :  4, // [22:19]
                      length_copy_c                                           : 19; // [18:0]
             uint32_t reserved_4                                              :  3, // [31:29]
                      tail_copy_d                                             :  6, // [28:23]
                      mcs_copy_d                                              :  4, // [22:19]
                      length_copy_d                                           : 19; // [18:0]
             uint32_t reserved_5                                              :  3, // [31:29]
                      tail_copy_e                                             :  6, // [28:23]
                      mcs_copy_e                                              :  4, // [22:19]
                      length_copy_e                                           : 19; // [18:0]
             uint32_t mu_user_number                                          :  3, // [31:29]
                      tail_copy_f                                             :  6, // [28:23]
                      mcs_copy_f                                              :  4, // [22:19]
                      length_copy_f                                           : 19; // [18:0]
             uint32_t reserved_7                                              :  3, // [31:29]
                      tail_copy_g                                             :  6, // [28:23]
                      mcs_copy_g                                              :  4, // [22:19]
                      length_copy_g                                           : 19; // [18:0]
#endif
};


/* Description		LENGTH

			VHT-SIG-B Length (in units of 4 octets) = ceiling (LENGTH/4) 
			 <legal all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_OFFSET                                          0x00000000
#define VHT_SIG_B_MU160_INFO_LENGTH_LSB                                             0
#define VHT_SIG_B_MU160_INFO_LENGTH_MSB                                             18
#define VHT_SIG_B_MU160_INFO_LENGTH_MASK                                            0x0007ffff


/* Description		MCS

			Modulation as described in vht_sig_a mcs field  <legal 0-11>
			
*/

#define VHT_SIG_B_MU160_INFO_MCS_OFFSET                                             0x00000000
#define VHT_SIG_B_MU160_INFO_MCS_LSB                                                19
#define VHT_SIG_B_MU160_INFO_MCS_MSB                                                22
#define VHT_SIG_B_MU160_INFO_MCS_MASK                                               0x00780000


/* Description		TAIL

			Used to terminate the trellis of the convolutional decoder.
			
			Set to 0.  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_OFFSET                                            0x00000000
#define VHT_SIG_B_MU160_INFO_TAIL_LSB                                               23
#define VHT_SIG_B_MU160_INFO_TAIL_MSB                                               28
#define VHT_SIG_B_MU160_INFO_TAIL_MASK                                              0x1f800000


/* Description		RESERVED_0

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_0_OFFSET                                      0x00000000
#define VHT_SIG_B_MU160_INFO_RESERVED_0_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_0_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_0_MASK                                        0xe0000000


/* Description		LENGTH_COPY_A

			Same as "length". This field is not valid for RX packets
			 <legal all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_A_OFFSET                                   0x00000004
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_A_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_A_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_A_MASK                                     0x0007ffff


/* Description		MCS_COPY_A

			Same as "mcs". This field is not valid for RX packets  <legal
			 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_A_OFFSET                                      0x00000004
#define VHT_SIG_B_MU160_INFO_MCS_COPY_A_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_A_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_A_MASK                                        0x00780000


/* Description		TAIL_COPY_A

			Same as "tail". This field is not valid for RX packets  <legal
			 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_A_OFFSET                                     0x00000004
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_A_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_A_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_A_MASK                                       0x1f800000


/* Description		RESERVED_1

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_1_OFFSET                                      0x00000004
#define VHT_SIG_B_MU160_INFO_RESERVED_1_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_1_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_1_MASK                                        0xe0000000


/* Description		LENGTH_COPY_B

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_B_OFFSET                                   0x00000008
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_B_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_B_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_B_MASK                                     0x0007ffff


/* Description		MCS_COPY_B

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_B_OFFSET                                      0x00000008
#define VHT_SIG_B_MU160_INFO_MCS_COPY_B_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_B_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_B_MASK                                        0x00780000


/* Description		TAIL_COPY_B

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_B_OFFSET                                     0x00000008
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_B_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_B_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_B_MASK                                       0x1f800000


/* Description		RESERVED_2

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_2_OFFSET                                      0x00000008
#define VHT_SIG_B_MU160_INFO_RESERVED_2_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_2_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_2_MASK                                        0xe0000000


/* Description		LENGTH_COPY_C

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_C_OFFSET                                   0x0000000c
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_C_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_C_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_C_MASK                                     0x0007ffff


/* Description		MCS_COPY_C

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_C_OFFSET                                      0x0000000c
#define VHT_SIG_B_MU160_INFO_MCS_COPY_C_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_C_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_C_MASK                                        0x00780000


/* Description		TAIL_COPY_C

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_C_OFFSET                                     0x0000000c
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_C_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_C_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_C_MASK                                       0x1f800000


/* Description		RESERVED_3

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_3_OFFSET                                      0x0000000c
#define VHT_SIG_B_MU160_INFO_RESERVED_3_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_3_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_3_MASK                                        0xe0000000


/* Description		LENGTH_COPY_D

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_D_OFFSET                                   0x00000010
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_D_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_D_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_D_MASK                                     0x0007ffff


/* Description		MCS_COPY_D

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_D_OFFSET                                      0x00000010
#define VHT_SIG_B_MU160_INFO_MCS_COPY_D_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_D_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_D_MASK                                        0x00780000


/* Description		TAIL_COPY_D

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_D_OFFSET                                     0x00000010
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_D_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_D_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_D_MASK                                       0x1f800000


/* Description		RESERVED_4

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_4_OFFSET                                      0x00000010
#define VHT_SIG_B_MU160_INFO_RESERVED_4_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_4_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_4_MASK                                        0xe0000000


/* Description		LENGTH_COPY_E

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_E_OFFSET                                   0x00000014
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_E_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_E_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_E_MASK                                     0x0007ffff


/* Description		MCS_COPY_E

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_E_OFFSET                                      0x00000014
#define VHT_SIG_B_MU160_INFO_MCS_COPY_E_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_E_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_E_MASK                                        0x00780000


/* Description		TAIL_COPY_E

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_E_OFFSET                                     0x00000014
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_E_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_E_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_E_MASK                                       0x1f800000


/* Description		RESERVED_5

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_5_OFFSET                                      0x00000014
#define VHT_SIG_B_MU160_INFO_RESERVED_5_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_5_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_5_MASK                                        0xe0000000


/* Description		LENGTH_COPY_F

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_F_OFFSET                                   0x00000018
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_F_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_F_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_F_MASK                                     0x0007ffff


/* Description		MCS_COPY_F

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_F_OFFSET                                      0x00000018
#define VHT_SIG_B_MU160_INFO_MCS_COPY_F_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_F_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_F_MASK                                        0x00780000


/* Description		TAIL_COPY_F

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_F_OFFSET                                     0x00000018
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_F_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_F_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_F_MASK                                       0x1f800000


/* Description		MU_USER_NUMBER

			Not part of VHT-SIG-B.
			Mapping from user number (BFer hardware specific) to mu_user_number. 
			The reader is directed to the previous chapter (User Number) 
			for a definition of the terms user and mu_user.   <legal
			 0-3>
*/

#define VHT_SIG_B_MU160_INFO_MU_USER_NUMBER_OFFSET                                  0x00000018
#define VHT_SIG_B_MU160_INFO_MU_USER_NUMBER_LSB                                     29
#define VHT_SIG_B_MU160_INFO_MU_USER_NUMBER_MSB                                     31
#define VHT_SIG_B_MU160_INFO_MU_USER_NUMBER_MASK                                    0xe0000000


/* Description		LENGTH_COPY_G

			Same as "length". This field is not valid for RX packets. <legal
			 all>
*/

#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_G_OFFSET                                   0x0000001c
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_G_LSB                                      0
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_G_MSB                                      18
#define VHT_SIG_B_MU160_INFO_LENGTH_COPY_G_MASK                                     0x0007ffff


/* Description		MCS_COPY_G

			Same as "mcs". This field is not valid for RX packets.  
			
			<legal 0-11>
*/

#define VHT_SIG_B_MU160_INFO_MCS_COPY_G_OFFSET                                      0x0000001c
#define VHT_SIG_B_MU160_INFO_MCS_COPY_G_LSB                                         19
#define VHT_SIG_B_MU160_INFO_MCS_COPY_G_MSB                                         22
#define VHT_SIG_B_MU160_INFO_MCS_COPY_G_MASK                                        0x00780000


/* Description		TAIL_COPY_G

			Same as "tail". This field is not valid for RX packets. 
			 <legal 0>
*/

#define VHT_SIG_B_MU160_INFO_TAIL_COPY_G_OFFSET                                     0x0000001c
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_G_LSB                                        23
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_G_MSB                                        28
#define VHT_SIG_B_MU160_INFO_TAIL_COPY_G_MASK                                       0x1f800000


/* Description		RESERVED_7

			<legal 0>
*/

#define VHT_SIG_B_MU160_INFO_RESERVED_7_OFFSET                                      0x0000001c
#define VHT_SIG_B_MU160_INFO_RESERVED_7_LSB                                         29
#define VHT_SIG_B_MU160_INFO_RESERVED_7_MSB                                         31
#define VHT_SIG_B_MU160_INFO_RESERVED_7_MASK                                        0xe0000000



#endif   // VHT_SIG_B_MU160_INFO
