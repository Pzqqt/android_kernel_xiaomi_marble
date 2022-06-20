
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

 
 
 
 
 
 
 


#ifndef _RX_START_PARAM_H_
#define _RX_START_PARAM_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_START_PARAM 2

#define NUM_OF_QWORDS_RX_START_PARAM 1


struct rx_start_param {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t pkt_type                                                :  4, // [3:0]
                      reserved_0a                                             : 12, // [15:4]
                      remaining_rx_time                                       : 16; // [31:16]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t remaining_rx_time                                       : 16, // [31:16]
                      reserved_0a                                             : 12, // [15:4]
                      pkt_type                                                :  4; // [3:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


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

#define RX_START_PARAM_PKT_TYPE_OFFSET                                              0x0000000000000000
#define RX_START_PARAM_PKT_TYPE_LSB                                                 0
#define RX_START_PARAM_PKT_TYPE_MSB                                                 3
#define RX_START_PARAM_PKT_TYPE_MASK                                                0x000000000000000f


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_START_PARAM_RESERVED_0A_OFFSET                                           0x0000000000000000
#define RX_START_PARAM_RESERVED_0A_LSB                                              4
#define RX_START_PARAM_RESERVED_0A_MSB                                              15
#define RX_START_PARAM_RESERVED_0A_MASK                                             0x000000000000fff0


/* Description		REMAINING_RX_TIME

			Remaining time (in us) for the current frame in the medium.
			
			(received from PHY in TLV: PHYRX_COMMON_USER_INFO.Receive_duration)
			
			<legal all>
*/

#define RX_START_PARAM_REMAINING_RX_TIME_OFFSET                                     0x0000000000000000
#define RX_START_PARAM_REMAINING_RX_TIME_LSB                                        16
#define RX_START_PARAM_REMAINING_RX_TIME_MSB                                        31
#define RX_START_PARAM_REMAINING_RX_TIME_MASK                                       0x00000000ffff0000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_START_PARAM_TLV64_PADDING_OFFSET                                         0x0000000000000000
#define RX_START_PARAM_TLV64_PADDING_LSB                                            32
#define RX_START_PARAM_TLV64_PADDING_MSB                                            63
#define RX_START_PARAM_TLV64_PADDING_MASK                                           0xffffffff00000000



#endif   // RX_START_PARAM
