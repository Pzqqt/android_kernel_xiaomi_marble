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

#ifndef _RXPCU_EARLY_RX_INDICATION_H_
#define _RXPCU_EARLY_RX_INDICATION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RXPCU_EARLY_RX_INDICATION 2

#define NUM_OF_QWORDS_RXPCU_EARLY_RX_INDICATION 1


struct rxpcu_early_rx_indication {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t pkt_type                                                :  4, // [3:0]
                      dot11ax_su_extended                                     :  1, // [4:4]
                      rate_mcs                                                :  4, // [8:5]
                      dot11ax_received_ext_ru_size                            :  4, // [12:9]
                      reserved_0a                                             : 19; // [31:13]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0a                                             : 19, // [31:13]
                      dot11ax_received_ext_ru_size                            :  4, // [12:9]
                      rate_mcs                                                :  4, // [8:5]
                      dot11ax_su_extended                                     :  1, // [4:4]
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

#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_OFFSET                                   0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_LSB                                      0
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_MSB                                      3
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_MASK                                     0x000000000000000f


/* Description		DOT11AX_SU_EXTENDED

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be
			
			When set, the 11ax or 11be reception was an extended range
			 SU 
*/

#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_OFFSET                        0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_LSB                           4
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_MSB                           4
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_MASK                          0x0000000000000010


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_OFFSET                                   0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_LSB                                      5
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_MSB                                      8
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_MASK                                     0x00000000000001e0


/* Description		DOT11AX_RECEIVED_EXT_RU_SIZE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be AND dot11ax_su_extended is set
			The number of (basic) RUs in this extended range reception
			
			
			RXPCU gets this from the received HE_SIG_A
			
			<legal all>
*/

#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET               0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                  9
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                  12
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                 0x0000000000001e00


/* Description		RESERVED_0A

			<legal 0>
*/

#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_OFFSET                                0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_LSB                                   13
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_MSB                                   31
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_MASK                                  0x00000000ffffe000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_OFFSET                              0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_LSB                                 32
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_MSB                                 63
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_MASK                                0xffffffff00000000



#endif   // RXPCU_EARLY_RX_INDICATION
