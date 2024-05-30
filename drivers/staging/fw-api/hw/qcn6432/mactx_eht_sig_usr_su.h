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

#ifndef _MACTX_EHT_SIG_USR_SU_H_
#define _MACTX_EHT_SIG_USR_SU_H_
#if !defined(__ASSEMBLER__)
#endif

#include "eht_sig_usr_su_info.h"
#define NUM_OF_DWORDS_MACTX_EHT_SIG_USR_SU 2

#define NUM_OF_QWORDS_MACTX_EHT_SIG_USR_SU 1


struct mactx_eht_sig_usr_su {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   eht_sig_usr_su_info                                       mactx_eht_sig_usr_su_info_details;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   eht_sig_usr_su_info                                       mactx_eht_sig_usr_su_info_details;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		MACTX_EHT_SIG_USR_SU_INFO_DETAILS

			See detailed description of the STRUCT
*/


/* Description		STA_ID

			Identifies the STA that is addressed. Details of STA ID 
			are TBD
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_ID_OFFSET        0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_ID_LSB           0
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_ID_MSB           10
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_ID_MASK          0x00000000000007ff


/* Description		STA_MCS

			Indicates the data MCS
			0 - 13: MCS 0 - 13
			14: MCS 0 with DCM and 2x duplicate
			15: MCS 0 with DCM
			<legal all>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_MCS_OFFSET       0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_MCS_LSB          11
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_MCS_MSB          14
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_MCS_MASK         0x0000000000007800


/* Description		VALIDATE_0A

			Note: spec indicates this shall be set to 1
			<legal 1>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_VALIDATE_0A_OFFSET   0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_VALIDATE_0A_LSB      15
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_VALIDATE_0A_MSB      15
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_VALIDATE_0A_MASK     0x0000000000008000


/* Description		NSS

			Number of spatial streams for this user
			
			The actual number of streams is 1 larger than indicated 
			in this field.
			<legal all>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_NSS_OFFSET           0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_NSS_LSB              16
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_NSS_MSB              19
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_NSS_MASK             0x00000000000f0000


/* Description		TXBF

			Indicates whether beamforming is applied
			0: No beamforming
			1: beamforming
			<legal all>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_TXBF_OFFSET          0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_TXBF_LSB             20
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_TXBF_MSB             20
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_TXBF_MASK            0x0000000000100000


/* Description		STA_CODING

			Distinguishes between BCC/LDPC
			
			0: BCC
			1: LDPC
			<legal all>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_CODING_OFFSET    0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_CODING_LSB       21
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_CODING_MSB       21
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_STA_CODING_MASK      0x0000000000200000


/* Description		RESERVED_0B

			<legal 0>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RESERVED_0B_OFFSET   0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RESERVED_0B_LSB      22
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RESERVED_0B_MSB      30
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RESERVED_0B_MASK     0x000000007fc00000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the CRC check of the
			 codeblock containing this EHT-SIG user info has passed, 
			else set to 0
			
			<legal all>
*/

#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_OFFSET 0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_LSB 31
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MSB 31
#define MACTX_EHT_SIG_USR_SU_MACTX_EHT_SIG_USR_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MASK 0x0000000080000000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define MACTX_EHT_SIG_USR_SU_TLV64_PADDING_OFFSET                                   0x0000000000000000
#define MACTX_EHT_SIG_USR_SU_TLV64_PADDING_LSB                                      32
#define MACTX_EHT_SIG_USR_SU_TLV64_PADDING_MSB                                      63
#define MACTX_EHT_SIG_USR_SU_TLV64_PADDING_MASK                                     0xffffffff00000000



#endif   // MACTX_EHT_SIG_USR_SU
