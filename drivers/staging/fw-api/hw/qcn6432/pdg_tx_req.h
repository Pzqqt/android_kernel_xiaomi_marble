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

#ifndef _PDG_TX_REQ_H_
#define _PDG_TX_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PDG_TX_REQ 2

#define NUM_OF_QWORDS_PDG_TX_REQ 1


struct pdg_tx_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_reason                                               :  2, // [1:0]
                      use_puncture_pattern                                    :  2, // [3:2]
                      req_bw                                                  :  3, // [6:4]
                      puncture_pattern_number                                 :  6, // [12:7]
                      reserved_0b                                             :  1, // [13:13]
                      req_paprd                                               :  1, // [14:14]
                      duration_field_boundary_valid                           :  1, // [15:15]
                      duration_field_boundary                                 : 16; // [31:16]
             uint32_t puncture_subband_mask                                   : 16, // [15:0]
                      reserved_0c                                             : 16; // [31:16]
#else
             uint32_t duration_field_boundary                                 : 16, // [31:16]
                      duration_field_boundary_valid                           :  1, // [15:15]
                      req_paprd                                               :  1, // [14:14]
                      reserved_0b                                             :  1, // [13:13]
                      puncture_pattern_number                                 :  6, // [12:7]
                      req_bw                                                  :  3, // [6:4]
                      use_puncture_pattern                                    :  2, // [3:2]
                      tx_reason                                               :  2; // [1:0]
             uint32_t reserved_0c                                             : 16, // [31:16]
                      puncture_subband_mask                                   : 16; // [15:0]
#endif
};


/* Description		TX_REASON

			<enum 0     tx_fes_protection_frame>  RTS, CTS2Self or 11h
			 protection type transmission preceding the regular PPDU
			 portion of the coming FES. 
			<enum 1     tx_fes_after_protection >  Regular PPDU transmission
			 that follows the transmission of medium protection frames:.
			
			<enum 2     tx_fes_only>  Regular PPDU transmission without
			 preceding medium protection frame exchanges. 
			
			Note: Response frame transmissions are initiated with the
			 PDG_RESPONSE TLV
			
			<legal 0-2>
*/

#define PDG_TX_REQ_TX_REASON_OFFSET                                                 0x0000000000000000
#define PDG_TX_REQ_TX_REASON_LSB                                                    0
#define PDG_TX_REQ_TX_REASON_MSB                                                    1
#define PDG_TX_REQ_TX_REASON_MASK                                                   0x0000000000000003



#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_OFFSET                                      0x0000000000000000
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_LSB                                         2
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_MSB                                         3
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_MASK                                        0x000000000000000c


/* Description		REQ_BW

			Field not valid when use_puncture_pattern is set to PUNCTURE_FROM_TX_SETUP
			
			
			The BW of the upcoming transmission.
			Note: Coex might have changed this from the original request.
			
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PDG_TX_REQ_REQ_BW_OFFSET                                                    0x0000000000000000
#define PDG_TX_REQ_REQ_BW_LSB                                                       4
#define PDG_TX_REQ_REQ_BW_MSB                                                       6
#define PDG_TX_REQ_REQ_BW_MASK                                                      0x0000000000000070


/* Description		PUNCTURE_PATTERN_NUMBER

			Field only valid when "use_puncture_pattern" is set.
			
			The pattern number in case punctured transmission is enabled
			
			<legal all>
*/

#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_OFFSET                                   0x0000000000000000
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_LSB                                      7
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_MSB                                      12
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_MASK                                     0x0000000000001f80


/* Description		RESERVED_0B

			<legal 0>
*/

#define PDG_TX_REQ_RESERVED_0B_OFFSET                                               0x0000000000000000
#define PDG_TX_REQ_RESERVED_0B_LSB                                                  13
#define PDG_TX_REQ_RESERVED_0B_MSB                                                  13
#define PDG_TX_REQ_RESERVED_0B_MASK                                                 0x0000000000002000


#define PDG_TX_REQ_REQ_PAPRD_OFFSET                                                 0x0000000000000000
#define PDG_TX_REQ_REQ_PAPRD_LSB                                                    14
#define PDG_TX_REQ_REQ_PAPRD_MSB                                                    14
#define PDG_TX_REQ_REQ_PAPRD_MASK                                                   0x0000000000004000


/* Description		DURATION_FIELD_BOUNDARY_VALID

			When set, PDG should take the 'duration_field_boundary' 
			value into account when it is calculating the TX and RX 
			boundaries for the upcoming transmission. Both RX and TX
			 should not go beyond this time duration provided.
			
			<legal all>
*/

#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_OFFSET                             0x0000000000000000
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_LSB                                15
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_MSB                                15
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_MASK                               0x0000000000008000


/* Description		DURATION_FIELD_BOUNDARY

			Field only valid when 'Duration_field_boundary_valid' is
			 set
			
			Amount of time to both TX and RX boundaries that PDG should
			 take into account for the upcoming transmission.
			<legal all>
*/

#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_OFFSET                                   0x0000000000000000
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_LSB                                      16
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_MSB                                      31
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_MASK                                     0x00000000ffff0000


/* Description		PUNCTURE_SUBBAND_MASK

			Field only valid when use_puncture_pattern is set to PUNCTURE_FROM_ALL_ALLOWED_MODES
			
			
			This mask indicates which 20 Mhz channels are actively used
			 in this transmission.
			
			Bit 0: primary 20 Mhz
			Bit 1: secondary 20 MHz
			Etc.
			<legal all>
*/

#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_OFFSET                                     0x0000000000000000
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_LSB                                        32
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_MSB                                        47
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_MASK                                       0x0000ffff00000000


/* Description		RESERVED_0C

			Reserved for future power bits: Generator should set to 
			0, consumer shall ignore <legal 0>
*/

#define PDG_TX_REQ_RESERVED_0C_OFFSET                                               0x0000000000000000
#define PDG_TX_REQ_RESERVED_0C_LSB                                                  48
#define PDG_TX_REQ_RESERVED_0C_MSB                                                  63
#define PDG_TX_REQ_RESERVED_0C_MASK                                                 0xffff000000000000



#endif   // PDG_TX_REQ
