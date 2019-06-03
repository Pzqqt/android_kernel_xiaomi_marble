/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef _PHYRX_HE_SIG_B2_OFDMA_H_
#define _PHYRX_HE_SIG_B2_OFDMA_H_
#if !defined(__ASSEMBLER__)
#endif

#include "he_sig_b2_ofdma_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct he_sig_b2_ofdma_info phyrx_he_sig_b2_ofdma_info_details;
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_HE_SIG_B2_OFDMA 1

struct phyrx_he_sig_b2_ofdma {
    struct            he_sig_b2_ofdma_info                       phyrx_he_sig_b2_ofdma_info_details;
};

/*

struct he_sig_b2_ofdma_info phyrx_he_sig_b2_ofdma_info_details
			
			See detailed description of the STRUCT
*/


 /* EXTERNAL REFERENCE : struct he_sig_b2_ofdma_info phyrx_he_sig_b2_ofdma_info_details */ 


/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID
			
			Identifies the STA that is addressed. Details of STA ID
			are TBD
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_LSB 0
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_MASK 0x000007ff

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_NSTS
			
			MAC RX side usage only:
			
			
			
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
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_NSTS_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_NSTS_LSB 11
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_NSTS_MASK 0x00003800

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_TXBF
			
			Indicates whether beamforming is applied
			
			0: No beamforming
			
			1: beamforming
			
			<legal all>
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_TXBF_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_TXBF_LSB 14
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_TXBF_MASK 0x00004000

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_MCS
			
			Indicates the data MCS
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_MCS_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_MCS_LSB 15
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_MCS_MASK 0x00078000

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_DCM
			
			
			0: No DCM
			
			1:DCM
			
			<legal all>
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_DCM_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_DCM_LSB 19
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_DCM_MASK 0x00080000

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_CODING
			
			Distinguishes between BCC/LDPC
			
			
			
			0: BCC
			
			1: LDPC
			
			<legal all>
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_CODING_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_CODING_LSB 20
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_CODING_MASK 0x00100000

/* Description		PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_RESERVED_0
			
			<legal 0>
*/
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_RESERVED_0_OFFSET 0x00000000
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_RESERVED_0_LSB 21
#define PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_RESERVED_0_MASK 0xffe00000


#endif // _PHYRX_HE_SIG_B2_OFDMA_H_
