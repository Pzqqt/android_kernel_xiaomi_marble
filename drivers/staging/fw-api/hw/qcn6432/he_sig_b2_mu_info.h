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

#ifndef _HE_SIG_B2_MU_INFO_H_
#define _HE_SIG_B2_MU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HE_SIG_B2_MU_INFO 2


struct he_sig_b2_mu_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sta_id                                                  : 11, // [10:0]
                      sta_spatial_config                                      :  4, // [14:11]
                      sta_mcs                                                 :  4, // [18:15]
                      reserved_set_to_1                                       :  1, // [19:19]
                      sta_coding                                              :  1, // [20:20]
                      reserved_0a                                             :  7, // [27:21]
                      nsts                                                    :  3, // [30:28]
                      rx_integrity_check_passed                               :  1; // [31:31]
             uint32_t user_order                                              :  8, // [7:0]
                      cc_mask                                                 :  8, // [15:8]
                      reserved_1a                                             : 16; // [31:16]
#else
             uint32_t rx_integrity_check_passed                               :  1, // [31:31]
                      nsts                                                    :  3, // [30:28]
                      reserved_0a                                             :  7, // [27:21]
                      sta_coding                                              :  1, // [20:20]
                      reserved_set_to_1                                       :  1, // [19:19]
                      sta_mcs                                                 :  4, // [18:15]
                      sta_spatial_config                                      :  4, // [14:11]
                      sta_id                                                  : 11; // [10:0]
             uint32_t reserved_1a                                             : 16, // [31:16]
                      cc_mask                                                 :  8, // [15:8]
                      user_order                                              :  8; // [7:0]
#endif
};


/* Description		STA_ID

			Identifies the STA that is addressed. Details of STA ID 
			are TBD
*/

#define HE_SIG_B2_MU_INFO_STA_ID_OFFSET                                             0x00000000
#define HE_SIG_B2_MU_INFO_STA_ID_LSB                                                0
#define HE_SIG_B2_MU_INFO_STA_ID_MSB                                                10
#define HE_SIG_B2_MU_INFO_STA_ID_MASK                                               0x000007ff


/* Description		STA_SPATIAL_CONFIG

			Number of assigned spatial streams and their corresponding
			 index. 
			Total number of spatial streams assigned for the MU-MIMO
			 allocation is also signaled. 
*/

#define HE_SIG_B2_MU_INFO_STA_SPATIAL_CONFIG_OFFSET                                 0x00000000
#define HE_SIG_B2_MU_INFO_STA_SPATIAL_CONFIG_LSB                                    11
#define HE_SIG_B2_MU_INFO_STA_SPATIAL_CONFIG_MSB                                    14
#define HE_SIG_B2_MU_INFO_STA_SPATIAL_CONFIG_MASK                                   0x00007800


/* Description		STA_MCS

			Indicates the data MCS
*/

#define HE_SIG_B2_MU_INFO_STA_MCS_OFFSET                                            0x00000000
#define HE_SIG_B2_MU_INFO_STA_MCS_LSB                                               15
#define HE_SIG_B2_MU_INFO_STA_MCS_MSB                                               18
#define HE_SIG_B2_MU_INFO_STA_MCS_MASK                                              0x00078000



#define HE_SIG_B2_MU_INFO_RESERVED_SET_TO_1_OFFSET                                  0x00000000
#define HE_SIG_B2_MU_INFO_RESERVED_SET_TO_1_LSB                                     19
#define HE_SIG_B2_MU_INFO_RESERVED_SET_TO_1_MSB                                     19
#define HE_SIG_B2_MU_INFO_RESERVED_SET_TO_1_MASK                                    0x00080000


/* Description		STA_CODING

			Distinguishes between BCC/LDPC
			
			0: BCC
			1: LDPC
			<legal all>
*/

#define HE_SIG_B2_MU_INFO_STA_CODING_OFFSET                                         0x00000000
#define HE_SIG_B2_MU_INFO_STA_CODING_LSB                                            20
#define HE_SIG_B2_MU_INFO_STA_CODING_MSB                                            20
#define HE_SIG_B2_MU_INFO_STA_CODING_MASK                                           0x00100000


/* Description		RESERVED_0A

			<legal 0>
*/

#define HE_SIG_B2_MU_INFO_RESERVED_0A_OFFSET                                        0x00000000
#define HE_SIG_B2_MU_INFO_RESERVED_0A_LSB                                           21
#define HE_SIG_B2_MU_INFO_RESERVED_0A_MSB                                           27
#define HE_SIG_B2_MU_INFO_RESERVED_0A_MASK                                          0x0fe00000


/* Description		NSTS

			MAC RX side usage only:
			Needed by RXPCU. Provided by PHY so that RXPCU does not 
			need to have the RU number decoding logic.
			
			Number of spatial streams for this user
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define HE_SIG_B2_MU_INFO_NSTS_OFFSET                                               0x00000000
#define HE_SIG_B2_MU_INFO_NSTS_LSB                                                  28
#define HE_SIG_B2_MU_INFO_NSTS_MSB                                                  30
#define HE_SIG_B2_MU_INFO_NSTS_MASK                                                 0x70000000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the CRC check of the
			 codeblock containing this HE-SIG-B user info has passed, 
			else set to 0
			
			<legal all>
*/

#define HE_SIG_B2_MU_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                          0x00000000
#define HE_SIG_B2_MU_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                             31
#define HE_SIG_B2_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                             31
#define HE_SIG_B2_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                            0x80000000


/* Description		USER_ORDER

			RX side: Set to 0
			TX side: Ordering index of the User field
			Gaps between the ordering indices of User fields indicate
			 that the microcode shall generate "unallocated RU" User
			 fields (STAID=2046) to fill the gaps.
			<legal all>
*/

#define HE_SIG_B2_MU_INFO_USER_ORDER_OFFSET                                         0x00000004
#define HE_SIG_B2_MU_INFO_USER_ORDER_LSB                                            0
#define HE_SIG_B2_MU_INFO_USER_ORDER_MSB                                            7
#define HE_SIG_B2_MU_INFO_USER_ORDER_MASK                                           0x000000ff


/* Description		CC_MASK

			RX side: Set to 0
			TX side: Indicates what content channel this User field 
			can go to
			Bit 0: content channel 0
			Bit 1: content channel 1
			The other bits are unused, but could repeat the above pattern
			 for compatibility with 'EHT_SIG_USR_MU_MIMO_INFO.'
			<legal all>
*/

#define HE_SIG_B2_MU_INFO_CC_MASK_OFFSET                                            0x00000004
#define HE_SIG_B2_MU_INFO_CC_MASK_LSB                                               8
#define HE_SIG_B2_MU_INFO_CC_MASK_MSB                                               15
#define HE_SIG_B2_MU_INFO_CC_MASK_MASK                                              0x0000ff00


/* Description		RESERVED_1A

			<legal 0>
*/

#define HE_SIG_B2_MU_INFO_RESERVED_1A_OFFSET                                        0x00000004
#define HE_SIG_B2_MU_INFO_RESERVED_1A_LSB                                           16
#define HE_SIG_B2_MU_INFO_RESERVED_1A_MSB                                           31
#define HE_SIG_B2_MU_INFO_RESERVED_1A_MASK                                          0xffff0000



#endif   // HE_SIG_B2_MU_INFO
