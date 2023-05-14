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

#ifndef _EHT_SIG_USR_SU_INFO_H_
#define _EHT_SIG_USR_SU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_EHT_SIG_USR_SU_INFO 1


struct eht_sig_usr_su_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sta_id                                                  : 11, // [10:0]
                      sta_mcs                                                 :  4, // [14:11]
                      validate_0a                                             :  1, // [15:15]
                      nss                                                     :  4, // [19:16]
                      txbf                                                    :  1, // [20:20]
                      sta_coding                                              :  1, // [21:21]
                      reserved_0b                                             :  9, // [30:22]
                      rx_integrity_check_passed                               :  1; // [31:31]
#else
             uint32_t rx_integrity_check_passed                               :  1, // [31:31]
                      reserved_0b                                             :  9, // [30:22]
                      sta_coding                                              :  1, // [21:21]
                      txbf                                                    :  1, // [20:20]
                      nss                                                     :  4, // [19:16]
                      validate_0a                                             :  1, // [15:15]
                      sta_mcs                                                 :  4, // [14:11]
                      sta_id                                                  : 11; // [10:0]
#endif
};


/* Description		STA_ID

			Identifies the STA that is addressed. Details of STA ID 
			are TBD
*/

#define EHT_SIG_USR_SU_INFO_STA_ID_OFFSET                                           0x00000000
#define EHT_SIG_USR_SU_INFO_STA_ID_LSB                                              0
#define EHT_SIG_USR_SU_INFO_STA_ID_MSB                                              10
#define EHT_SIG_USR_SU_INFO_STA_ID_MASK                                             0x000007ff


/* Description		STA_MCS

			Indicates the data MCS
			0 - 13: MCS 0 - 13
			14: MCS 0 with DCM and 2x duplicate
			15: MCS 0 with DCM
			<legal all>
*/

#define EHT_SIG_USR_SU_INFO_STA_MCS_OFFSET                                          0x00000000
#define EHT_SIG_USR_SU_INFO_STA_MCS_LSB                                             11
#define EHT_SIG_USR_SU_INFO_STA_MCS_MSB                                             14
#define EHT_SIG_USR_SU_INFO_STA_MCS_MASK                                            0x00007800


/* Description		VALIDATE_0A

			Note: spec indicates this shall be set to 1
			<legal 1>
*/

#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_OFFSET                                      0x00000000
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_LSB                                         15
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_MSB                                         15
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_MASK                                        0x00008000


/* Description		NSS

			Number of spatial streams for this user
			
			The actual number of streams is 1 larger than indicated 
			in this field.
			<legal all>
*/

#define EHT_SIG_USR_SU_INFO_NSS_OFFSET                                              0x00000000
#define EHT_SIG_USR_SU_INFO_NSS_LSB                                                 16
#define EHT_SIG_USR_SU_INFO_NSS_MSB                                                 19
#define EHT_SIG_USR_SU_INFO_NSS_MASK                                                0x000f0000


/* Description		TXBF

			Indicates whether beamforming is applied
			0: No beamforming
			1: beamforming
			<legal all>
*/

#define EHT_SIG_USR_SU_INFO_TXBF_OFFSET                                             0x00000000
#define EHT_SIG_USR_SU_INFO_TXBF_LSB                                                20
#define EHT_SIG_USR_SU_INFO_TXBF_MSB                                                20
#define EHT_SIG_USR_SU_INFO_TXBF_MASK                                               0x00100000


/* Description		STA_CODING

			Distinguishes between BCC/LDPC
			
			0: BCC
			1: LDPC
			<legal all>
*/

#define EHT_SIG_USR_SU_INFO_STA_CODING_OFFSET                                       0x00000000
#define EHT_SIG_USR_SU_INFO_STA_CODING_LSB                                          21
#define EHT_SIG_USR_SU_INFO_STA_CODING_MSB                                          21
#define EHT_SIG_USR_SU_INFO_STA_CODING_MASK                                         0x00200000


/* Description		RESERVED_0B

			<legal 0>
*/

#define EHT_SIG_USR_SU_INFO_RESERVED_0B_OFFSET                                      0x00000000
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_LSB                                         22
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_MSB                                         30
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_MASK                                        0x7fc00000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the CRC check of the
			 codeblock containing this EHT-SIG user info has passed, 
			else set to 0
			
			<legal all>
*/

#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                        0x00000000
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                           31
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                           31
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                          0x80000000



#endif   // EHT_SIG_USR_SU_INFO
