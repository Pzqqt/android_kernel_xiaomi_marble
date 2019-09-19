/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _PHYRX_HT_SIG_H_
#define _PHYRX_HT_SIG_H_
#if !defined(__ASSEMBLER__)
#endif

#include "ht_sig_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct ht_sig_info phyrx_ht_sig_info_details;
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_HT_SIG 2

struct phyrx_ht_sig {
    struct            ht_sig_info                       phyrx_ht_sig_info_details;
};

/*

struct ht_sig_info phyrx_ht_sig_info_details

			See detailed description of the STRUCT
*/


 /* EXTERNAL REFERENCE : struct ht_sig_info phyrx_ht_sig_info_details */


/* Description		PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_MCS

			Modulation Coding Scheme:

			0-7 are used for single stream

			8-15 are used for 2 streams

			16-23 are used for 3 streams

			24-31 are used for 4 streams

			32 is used for duplicate HT20 (unsupported)

			33-76 is used for unequal modulation (unsupported)

			77-127 is reserved.

			<legal 0-31>
*/
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_MCS_OFFSET          0x00000000
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_MCS_LSB             0
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_MCS_MASK            0x0000007f

/* Description		PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_CBW

			Packet bandwidth:

			<enum 0     ht_20_mhz>

			<enum 1     ht_40_mhz>

			<legal 0-1>
*/
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_CBW_OFFSET          0x00000000
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_CBW_LSB             7
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_CBW_MASK            0x00000080

/* Description		PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_LENGTH

			This is the MPDU or A-MPDU length in octets of the PPDU

			<legal all>
*/
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_LENGTH_OFFSET       0x00000000
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_LENGTH_LSB          8
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_LENGTH_MASK         0x00ffff00

/* Description		PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_0

			This field is not part of HT-SIG

			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY <legal 0>
*/
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_0_OFFSET   0x00000000
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_0_LSB      24
#define PHYRX_HT_SIG_0_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_0_MASK     0xff000000

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SMOOTHING

			Field indicates if smoothing is needed

			E_num 0     do_smoothing Unsupported setting: indicates
			smoothing is often used for beamforming


			<legal 1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SMOOTHING_OFFSET    0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SMOOTHING_LSB       0
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SMOOTHING_MASK      0x00000001

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NOT_SOUNDING

			E_num 0     sounding Unsupported setting: indicates
			sounding is used

			<enum 1     no_sounding>  Indicates no sounding is used

			<legal 1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NOT_SOUNDING_OFFSET 0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NOT_SOUNDING_LSB    1
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NOT_SOUNDING_MASK   0x00000002

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_HT_RESERVED

			Reserved: Should be set to 1 by the MAC and ignored by
			the PHY

			<legal 1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_HT_RESERVED_OFFSET  0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_HT_RESERVED_LSB     2
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_HT_RESERVED_MASK    0x00000004

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_AGGREGATION

			<enum 0     mpdu> Indicates MPDU format

			<enum 1     a_mpdu> Indicates A-MPDU format

			<legal 0-1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_AGGREGATION_OFFSET  0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_AGGREGATION_LSB     3
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_AGGREGATION_MASK    0x00000008

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_STBC

			<enum 0     no_stbc> Indicates no STBC

			<enum 1     1_str_stbc> Indicates 1 stream STBC

			E_num 2     2_str_stbc Indicates 2 stream STBC
			(Unsupported)

			<legal 0-1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_STBC_OFFSET         0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_STBC_LSB            4
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_STBC_MASK           0x00000030

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_FEC_CODING

			<enum 0     ht_bcc>  Indicates BCC coding

			<enum 1     ht_ldpc>  Indicates LDPC coding

			<legal 0-1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_FEC_CODING_OFFSET   0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_FEC_CODING_LSB      6
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_FEC_CODING_MASK     0x00000040

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SHORT_GI

			<enum 0     ht_normal_gi>  Indicates normal guard
			interval


			<legal 0-1>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SHORT_GI_OFFSET     0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SHORT_GI_LSB        7
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SHORT_GI_MASK       0x00000080

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NUM_EXT_SP_STR

			Number of extension spatial streams: (Used for TxBF)

			<enum 0     0_ext_sp_str>  No extension spatial streams

			E_num 1     1_ext_sp_str  Not supported: 1 extension
			spatial streams

			E_num 2     2_ext_sp_str  Not supported:  2 extension
			spatial streams

			<legal 0>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NUM_EXT_SP_STR_OFFSET 0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NUM_EXT_SP_STR_LSB  8
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_NUM_EXT_SP_STR_MASK 0x00000300

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_CRC

			The CRC protects the HT-SIG (HT-SIG[0][23:0] and
			HT-SIG[1][9:0]. The generator polynomial is G(D) = D8 + D2 +
			D + 1.  <legal all>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_CRC_OFFSET          0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_CRC_LSB             10
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_CRC_MASK            0x0003fc00

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SIGNAL_TAIL

			The 6 bits of tail is always set to 0 is used to flush
			the BCC encoder and decoder.  <legal 0>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SIGNAL_TAIL_OFFSET  0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SIGNAL_TAIL_LSB     18
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_SIGNAL_TAIL_MASK    0x00fc0000

/* Description		PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_1

			This field is not part of HT-SIG:

			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY.  <legal 0>
*/
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_1_OFFSET   0x00000004
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_1_LSB      24
#define PHYRX_HT_SIG_1_PHYRX_HT_SIG_INFO_DETAILS_RESERVED_1_MASK     0xff000000


#endif // _PHYRX_HT_SIG_H_
