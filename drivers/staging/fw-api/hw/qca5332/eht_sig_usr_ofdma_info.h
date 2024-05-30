
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

 
 
 
 
 
 
 


#ifndef _EHT_SIG_USR_OFDMA_INFO_H_
#define _EHT_SIG_USR_OFDMA_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_EHT_SIG_USR_OFDMA_INFO 2


struct eht_sig_usr_ofdma_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sta_id                                                  : 11, // [10:0]
                      sta_mcs                                                 :  4, // [14:11]
                      validate_0a                                             :  1, // [15:15]
                      nss                                                     :  4, // [19:16]
                      txbf                                                    :  1, // [20:20]
                      sta_coding                                              :  1, // [21:21]
                      reserved_0b                                             :  1, // [22:22]
                      rx_integrity_check_passed                               :  1, // [23:23]
                      subband80_cc_mask                                       :  8; // [31:24]
             uint32_t user_order_subband80_0                                  :  8, // [7:0]
                      user_order_subband80_1                                  :  8, // [15:8]
                      user_order_subband80_2                                  :  8, // [23:16]
                      user_order_subband80_3                                  :  8; // [31:24]
#else
             uint32_t subband80_cc_mask                                       :  8, // [31:24]
                      rx_integrity_check_passed                               :  1, // [23:23]
                      reserved_0b                                             :  1, // [22:22]
                      sta_coding                                              :  1, // [21:21]
                      txbf                                                    :  1, // [20:20]
                      nss                                                     :  4, // [19:16]
                      validate_0a                                             :  1, // [15:15]
                      sta_mcs                                                 :  4, // [14:11]
                      sta_id                                                  : 11; // [10:0]
             uint32_t user_order_subband80_3                                  :  8, // [31:24]
                      user_order_subband80_2                                  :  8, // [23:16]
                      user_order_subband80_1                                  :  8, // [15:8]
                      user_order_subband80_0                                  :  8; // [7:0]
#endif
};


/* Description		STA_ID

			Identifies the STA that is addressed. Details of STA ID 
			are TBD
*/

#define EHT_SIG_USR_OFDMA_INFO_STA_ID_OFFSET                                        0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_LSB                                           0
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_MSB                                           10
#define EHT_SIG_USR_OFDMA_INFO_STA_ID_MASK                                          0x000007ff


/* Description		STA_MCS

			Indicates the data MCS
			0 - 13: MCS 0 - 13
			14: validate
			15: MCS 0 with DCM
			<legal 0-13, 15>
*/

#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_OFFSET                                       0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_LSB                                          11
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_MSB                                          14
#define EHT_SIG_USR_OFDMA_INFO_STA_MCS_MASK                                         0x00007800


/* Description		VALIDATE_0A

			Note: spec indicates this shall be set to 1
			<legal 1>
*/

#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_OFFSET                                   0x00000000
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_LSB                                      15
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_MSB                                      15
#define EHT_SIG_USR_OFDMA_INFO_VALIDATE_0A_MASK                                     0x00008000


/* Description		NSS

			Number of spatial streams for this user
			
			The actual number of streams is 1 larger than indicated 
			in this field.
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_NSS_OFFSET                                           0x00000000
#define EHT_SIG_USR_OFDMA_INFO_NSS_LSB                                              16
#define EHT_SIG_USR_OFDMA_INFO_NSS_MSB                                              19
#define EHT_SIG_USR_OFDMA_INFO_NSS_MASK                                             0x000f0000


/* Description		TXBF

			Indicates whether beamforming is applied
			0: No beamforming
			1: beamforming
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_TXBF_OFFSET                                          0x00000000
#define EHT_SIG_USR_OFDMA_INFO_TXBF_LSB                                             20
#define EHT_SIG_USR_OFDMA_INFO_TXBF_MSB                                             20
#define EHT_SIG_USR_OFDMA_INFO_TXBF_MASK                                            0x00100000


/* Description		STA_CODING

			Distinguishes between BCC/LDPC
			
			0: BCC
			1: LDPC
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_OFFSET                                    0x00000000
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_LSB                                       21
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_MSB                                       21
#define EHT_SIG_USR_OFDMA_INFO_STA_CODING_MASK                                      0x00200000


/* Description		RESERVED_0B

			<legal 0>
*/

#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_OFFSET                                   0x00000000
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_LSB                                      22
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_MSB                                      22
#define EHT_SIG_USR_OFDMA_INFO_RESERVED_0B_MASK                                     0x00400000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the CRC check of the
			 codeblock containing this EHT-SIG user info has passed, 
			else set to 0
			
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                     0x00000000
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                        23
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                        23
#define EHT_SIG_USR_OFDMA_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                       0x00800000


/* Description		SUBBAND80_CC_MASK

			RX side: Set to 0
			TX side: Indicates what content channels of what 80 MHz 
			subbands this User field can go to
			Bit 0: lowest 80 MHz content channel 0
			Bit 1: lowest 80 MHz content channel 1
			Bit 2: 2nd lowest 80 MHz content channel 0
			...
			Bit 7: highest 80 MHz content channel 1
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_OFFSET                             0x00000000
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_LSB                                24
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_MSB                                31
#define EHT_SIG_USR_OFDMA_INFO_SUBBAND80_CC_MASK_MASK                               0xff000000


/* Description		USER_ORDER_SUBBAND80_0

			RX side: Set to 0
			TX side: Ordering index of the User field within the lowest
			 80 MHz
			Gaps between the ordering indices of User fields indicate
			 that the microcode shall generate "unallocated RU" User
			 fields (STAID=2046) to fill the gaps.
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_LSB                           0
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_MSB                           7
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_0_MASK                          0x000000ff


/* Description		USER_ORDER_SUBBAND80_1

			RX side: Set to 0
			TX side: Ordering index of the User field within the 2nd
			 lowest 80 MHz
			See 'user_order_subband80_0.'
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_LSB                           8
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_MSB                           15
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_1_MASK                          0x0000ff00


/* Description		USER_ORDER_SUBBAND80_2

			RX side: Set to 0
			TX side: Ordering index of the User field within the 2nd
			 highest 80 MHz
			See 'user_order_subband80_0.'
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_LSB                           16
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_MSB                           23
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_2_MASK                          0x00ff0000


/* Description		USER_ORDER_SUBBAND80_3

			RX side: Set to 0
			TX side: Ordering index of the User field within the highest
			 80 MHz
			See 'user_order_subband80_0.'
			<legal all>
*/

#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_OFFSET                        0x00000004
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_LSB                           24
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_MSB                           31
#define EHT_SIG_USR_OFDMA_INFO_USER_ORDER_SUBBAND80_3_MASK                          0xff000000



#endif   // EHT_SIG_USR_OFDMA_INFO
