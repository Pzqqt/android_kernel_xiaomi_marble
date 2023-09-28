
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

 
 
 
 
 
 
 


#ifndef _VHT_SIG_B_SU80_INFO_H_
#define _VHT_SIG_B_SU80_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_SU80_INFO 4


struct vht_sig_b_su80_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 21, // [20:0]
                      vhtb_reserved                                           :  2, // [22:21]
                      tail                                                    :  6, // [28:23]
                      reserved_0                                              :  2, // [30:29]
                      rx_ndp                                                  :  1; // [31:31]
             uint32_t length_copy_a                                           : 21, // [20:0]
                      vhtb_reserved_copy_a                                    :  2, // [22:21]
                      tail_copy_a                                             :  6, // [28:23]
                      reserved_1                                              :  2, // [30:29]
                      rx_ndp_copy_a                                           :  1; // [31:31]
             uint32_t length_copy_b                                           : 21, // [20:0]
                      vhtb_reserved_copy_b                                    :  2, // [22:21]
                      tail_copy_b                                             :  6, // [28:23]
                      reserved_2                                              :  2, // [30:29]
                      rx_ndp_copy_b                                           :  1; // [31:31]
             uint32_t length_copy_c                                           : 21, // [20:0]
                      vhtb_reserved_copy_c                                    :  2, // [22:21]
                      tail_copy_c                                             :  6, // [28:23]
                      reserved_3                                              :  2, // [30:29]
                      rx_ndp_copy_c                                           :  1; // [31:31]
#else
             uint32_t rx_ndp                                                  :  1, // [31:31]
                      reserved_0                                              :  2, // [30:29]
                      tail                                                    :  6, // [28:23]
                      vhtb_reserved                                           :  2, // [22:21]
                      length                                                  : 21; // [20:0]
             uint32_t rx_ndp_copy_a                                           :  1, // [31:31]
                      reserved_1                                              :  2, // [30:29]
                      tail_copy_a                                             :  6, // [28:23]
                      vhtb_reserved_copy_a                                    :  2, // [22:21]
                      length_copy_a                                           : 21; // [20:0]
             uint32_t rx_ndp_copy_b                                           :  1, // [31:31]
                      reserved_2                                              :  2, // [30:29]
                      tail_copy_b                                             :  6, // [28:23]
                      vhtb_reserved_copy_b                                    :  2, // [22:21]
                      length_copy_b                                           : 21; // [20:0]
             uint32_t rx_ndp_copy_c                                           :  1, // [31:31]
                      reserved_3                                              :  2, // [30:29]
                      tail_copy_c                                             :  6, // [28:23]
                      vhtb_reserved_copy_c                                    :  2, // [22:21]
                      length_copy_c                                           : 21; // [20:0]
#endif
};


/* Description		LENGTH

			VHT-SIG-B Length (in units of 4 octets) = ceiling (LENGTH/4)
			
			<legal all>
*/

#define VHT_SIG_B_SU80_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_SU80_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_SU80_INFO_LENGTH_MSB                                              20
#define VHT_SIG_B_SU80_INFO_LENGTH_MASK                                             0x001fffff


/* Description		VHTB_RESERVED

			Reserved:  Set  to all ones for non-NDP frames and ignored
			 on receive  <legal 1,3>
*/

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_OFFSET                                    0x00000000
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_LSB                                       21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_MSB                                       22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_MASK                                      0x00600000


/* Description		TAIL

			Used to terminate the trellis of the convolutional decoder.
			
			Set to 0.  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_SU80_INFO_TAIL_LSB                                                23
#define VHT_SIG_B_SU80_INFO_TAIL_MSB                                                28
#define VHT_SIG_B_SU80_INFO_TAIL_MASK                                               0x1f800000


/* Description		RESERVED_0

			Not part of VHT-SIG-B.
			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_B_SU80_INFO_RESERVED_0_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_0_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_0_MASK                                         0x60000000


/* Description		RX_NDP

			Not part of VHT-SIG-B.
			Used to identify received NDP frame
			<legal 0,1>
*/

#define VHT_SIG_B_SU80_INFO_RX_NDP_OFFSET                                           0x00000000
#define VHT_SIG_B_SU80_INFO_RX_NDP_LSB                                              31
#define VHT_SIG_B_SU80_INFO_RX_NDP_MSB                                              31
#define VHT_SIG_B_SU80_INFO_RX_NDP_MASK                                             0x80000000


/* Description		LENGTH_COPY_A

			Same as "length" <legal all>
*/

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_OFFSET                                    0x00000004
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_MASK                                      0x001fffff


/* Description		VHTB_RESERVED_COPY_A

			Same as "vhtb_reserved"  <legal 1,3>
*/

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_OFFSET                             0x00000004
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_MASK                               0x00600000


/* Description		TAIL_COPY_A

			Same as "tail"  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_OFFSET                                      0x00000004
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_MASK                                        0x1f800000


/* Description		RESERVED_1

			Reserved: Set to 0 and ignored on receive  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_B_SU80_INFO_RESERVED_1_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_1_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_1_MASK                                         0x60000000


/* Description		RX_NDP_COPY_A

			Not part of VHT-SIG-B.
			Used to identify received NDP frame
			<legal 0,1>
*/

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_OFFSET                                    0x00000004
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_MASK                                      0x80000000


/* Description		LENGTH_COPY_B

			Same as "length" <legal all>
*/

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_OFFSET                                    0x00000008
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_MASK                                      0x001fffff


/* Description		VHTB_RESERVED_COPY_B

			Same as "vhtb_reserved"  <legal 1,3>
*/

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_OFFSET                             0x00000008
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_MASK                               0x00600000


/* Description		TAIL_COPY_B

			Same as "tail"  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_OFFSET                                      0x00000008
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_MASK                                        0x1f800000


/* Description		RESERVED_2

			Reserved: Set to 0 and ignored on receive <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_RESERVED_2_OFFSET                                       0x00000008
#define VHT_SIG_B_SU80_INFO_RESERVED_2_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_2_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_2_MASK                                         0x60000000


/* Description		RX_NDP_COPY_B

			Not part of VHT-SIG-B.
			Used to identify received NDP frame
			<legal 0,1>
*/

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_OFFSET                                    0x00000008
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_MASK                                      0x80000000


/* Description		LENGTH_COPY_C

			Same as "length" <legal all>
*/

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_OFFSET                                    0x0000000c
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_MASK                                      0x001fffff


/* Description		VHTB_RESERVED_COPY_C

			Same as "vhtb_reserved"  <legal 1,3>
*/

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_OFFSET                             0x0000000c
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_MASK                               0x00600000


/* Description		TAIL_COPY_C

			Same as "tail"  <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_OFFSET                                      0x0000000c
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_MASK                                        0x1f800000


/* Description		RESERVED_3

			Reserved: Set to 0 and ignored on receive <legal 0>
*/

#define VHT_SIG_B_SU80_INFO_RESERVED_3_OFFSET                                       0x0000000c
#define VHT_SIG_B_SU80_INFO_RESERVED_3_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_3_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_3_MASK                                         0x60000000


/* Description		RX_NDP_COPY_C

			Not part of VHT-SIG-B.
			Used to identify received NDP frame
			<legal 0,1>
*/

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_OFFSET                                    0x0000000c
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_MASK                                      0x80000000



#endif   // VHT_SIG_B_SU80_INFO
