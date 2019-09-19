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

#ifndef _PHYRX_L_SIG_A_H_
#define _PHYRX_L_SIG_A_H_
#if !defined(__ASSEMBLER__)
#endif

#include "l_sig_a_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct l_sig_a_info phyrx_l_sig_a_info_details;
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_L_SIG_A 1

struct phyrx_l_sig_a {
    struct            l_sig_a_info                       phyrx_l_sig_a_info_details;
};

/*

struct l_sig_a_info phyrx_l_sig_a_info_details

			See detailed description of the STRUCT
*/


 /* EXTERNAL REFERENCE : struct l_sig_a_info phyrx_l_sig_a_info_details */


/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RATE

			This format is originally defined for OFDM as a 4 bit
			field but the 5th bit was added to indicate 11b formatted
			frames.  In the standard bit [4] is specified as reserved.
			For 11b frames this L-SIG is transformed in the PHY into the
			11b preamble format.  The following are the rates:

			<enum 8     ofdm_48_mbps> 64-QAM 2/3 (48 Mbps)

			<enum 9     ofdm_24_mbps> 16-QAM 1/2 (24 Mbps)

			<enum 10     ofdm_12_mbps> QPSK 1/2 (12 Mbps)

			<enum 11     ofdm_6_mbps> BPSK 1/2 (6 Mbps)

			<enum 12     ofdm_54_mbps> 64-QAM 3/4 (54 Mbps)

			<enum 13     ofdm_36_mbps> 16-QAM 3/4 (36 Mbps)

			<enum 14     ofdm_18_mbps> QPSK 1/2 (18 Mbps)

			<enum 15     ofdm_9_mbps> BPSK 3/4 (9 Mbps)

			<legal 8-15>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RATE_OFFSET       0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RATE_LSB          0
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RATE_MASK         0x0000000f

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LSIG_RESERVED

			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY

			<legal 0>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LSIG_RESERVED_OFFSET 0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LSIG_RESERVED_LSB 4
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LSIG_RESERVED_MASK 0x00000010

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LENGTH

			The length indicates the number of octets in this MPDU.
			Note that when using mixed mode 11n preamble this length
			provides the spoofed length for the PPDU.  This length
			provides part of the information to derive the actually PPDU
			length.  For legacy OFDM and 11B frames the maximum length
			is

			<legal all>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LENGTH_OFFSET     0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LENGTH_LSB        5
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_LENGTH_MASK       0x0001ffe0

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PARITY

			11a/n/ac TX: This field provides even parity over the
			first 18 bits of the signal field which means that the sum
			of 1s in the signal field will always be even on

			11a/n/ac RX: this field contains the received parity
			field from the L-SIG symbol for the current packet.

			<legal 0-1>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PARITY_OFFSET     0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PARITY_LSB        17
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PARITY_MASK       0x00020000

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_TAIL

			The 6 bits of tail is always set to 0 is used to flush
			the BCC encoder and decoder.  <legal 0>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_TAIL_OFFSET       0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_TAIL_LSB          18
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_TAIL_MASK         0x00fc0000

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PKT_TYPE

			Only used on the RX side.

			Note: This is not really part of L-SIG



			Packet type:

			<enum 0 dot11a>802.11a PPDU type

			<enum 1 dot11b>802.11b PPDU type

			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type

			<enum 3 dot11ac>802.11ac PPDU type

			<enum 4 dot11ax>802.11ax PPDU type

			<enum 5 dot11ba>802.11ba (WUR) PPDU type
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PKT_TYPE_OFFSET   0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PKT_TYPE_LSB      24
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_PKT_TYPE_MASK     0x0f000000

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_CAPTURED_IMPLICIT_SOUNDING

			Only used on the RX side.

			Note: This is not really part of L-SIG



			This indicates that the PHY has captured implicit
			sounding.
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_CAPTURED_IMPLICIT_SOUNDING_OFFSET 0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_CAPTURED_IMPLICIT_SOUNDING_LSB 28
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_CAPTURED_IMPLICIT_SOUNDING_MASK 0x10000000

/* Description		PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RESERVED

			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RESERVED_OFFSET   0x00000000
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RESERVED_LSB      29
#define PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RESERVED_MASK     0xe0000000


#endif // _PHYRX_L_SIG_A_H_
