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

#ifndef _RX_PREAMBLE_H_
#define _RX_PREAMBLE_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_PREAMBLE 2

#define NUM_OF_QWORDS_RX_PREAMBLE 1


struct rx_preamble {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t num_users                                               :  6, // [5:0]
                      pkt_type                                                :  4, // [9:6]
                      direction                                               :  1, // [10:10]
                      reserved_0a                                             : 21; // [31:11]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0a                                             : 21, // [31:11]
                      direction                                               :  1, // [10:10]
                      pkt_type                                                :  4, // [9:6]
                      num_users                                               :  6; // [5:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		NUM_USERS

			The number of users in the receiving OFDMA frame.
*/

#define RX_PREAMBLE_NUM_USERS_OFFSET                                                0x0000000000000000
#define RX_PREAMBLE_NUM_USERS_LSB                                                   0
#define RX_PREAMBLE_NUM_USERS_MSB                                                   5
#define RX_PREAMBLE_NUM_USERS_MASK                                                  0x000000000000003f


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define RX_PREAMBLE_PKT_TYPE_OFFSET                                                 0x0000000000000000
#define RX_PREAMBLE_PKT_TYPE_LSB                                                    6
#define RX_PREAMBLE_PKT_TYPE_MSB                                                    9
#define RX_PREAMBLE_PKT_TYPE_MASK                                                   0x00000000000003c0


/* Description		DIRECTION

			Field only valid in case of pkt_type = dot11ax
			
			<enum 0     direction_uplink_reception>
			<enum 1     direction_downlink_reception>
			<legal all>
*/

#define RX_PREAMBLE_DIRECTION_OFFSET                                                0x0000000000000000
#define RX_PREAMBLE_DIRECTION_LSB                                                   10
#define RX_PREAMBLE_DIRECTION_MSB                                                   10
#define RX_PREAMBLE_DIRECTION_MASK                                                  0x0000000000000400


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_PREAMBLE_RESERVED_0A_OFFSET                                              0x0000000000000000
#define RX_PREAMBLE_RESERVED_0A_LSB                                                 11
#define RX_PREAMBLE_RESERVED_0A_MSB                                                 31
#define RX_PREAMBLE_RESERVED_0A_MASK                                                0x00000000fffff800


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_PREAMBLE_TLV64_PADDING_OFFSET                                            0x0000000000000000
#define RX_PREAMBLE_TLV64_PADDING_LSB                                               32
#define RX_PREAMBLE_TLV64_PADDING_MSB                                               63
#define RX_PREAMBLE_TLV64_PADDING_MASK                                              0xffffffff00000000



#endif   // RX_PREAMBLE
