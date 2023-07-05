
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

 
 
 
 
 
 
 


#ifndef _RX_PPDU_START_H_
#define _RX_PPDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_PPDU_START 6

#define NUM_OF_QWORDS_RX_PPDU_START 3


struct rx_ppdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16, // [15:0]
                      preamble_time_to_rxframe                                :  8, // [23:16]
                      reserved_0a                                             :  8; // [31:24]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t rxframe_assert_timestamp                                : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0a                                             :  8, // [31:24]
                      preamble_time_to_rxframe                                :  8, // [23:16]
                      phy_ppdu_id                                             : 16; // [15:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t rxframe_assert_timestamp                                : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_PPDU_START_PHY_PPDU_ID_OFFSET                                            0x0000000000000000
#define RX_PPDU_START_PHY_PPDU_ID_LSB                                               0
#define RX_PPDU_START_PHY_PPDU_ID_MSB                                               15
#define RX_PPDU_START_PHY_PPDU_ID_MASK                                              0x000000000000ffff


/* Description		PREAMBLE_TIME_TO_RXFRAME

			The amount of time (in us) of the frame being put on the
			 medium, and PHY raising rx_frame
			
			From  'PHYRX_RSSI_LEGACY. Preamble_time_to_rx_frame'
			
			<legal all>
*/

#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_OFFSET                               0x0000000000000000
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_LSB                                  16
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_MSB                                  23
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_MASK                                 0x0000000000ff0000


/* Description		RESERVED_0A

			Reserved
			<legal 0>
*/

#define RX_PPDU_START_RESERVED_0A_OFFSET                                            0x0000000000000000
#define RX_PPDU_START_RESERVED_0A_LSB                                               24
#define RX_PPDU_START_RESERVED_0A_MSB                                               31
#define RX_PPDU_START_RESERVED_0A_MASK                                              0x00000000ff000000


/* Description		SW_PHY_META_DATA

			SW programmed Meta data provided by the PHY.
			
			Can be used for SW to indicate the channel the device is
			 on.
			
			From  'PHYRX_RSSI_LEGACY.Sw_phy_meta_data'
*/

#define RX_PPDU_START_SW_PHY_META_DATA_OFFSET                                       0x0000000000000000
#define RX_PPDU_START_SW_PHY_META_DATA_LSB                                          32
#define RX_PPDU_START_SW_PHY_META_DATA_MSB                                          63
#define RX_PPDU_START_SW_PHY_META_DATA_MASK                                         0xffffffff00000000


/* Description		PPDU_START_TIMESTAMP_31_0

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, lower 32 bits.
			
			The timestamp is captured by the PHY and given to the MAC
			 in 'PHYRX_RSSI_LEGACY.ppdu_start_timestamp_*.'
			<legal all>
*/

#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_OFFSET                              0x0000000000000008
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_LSB                                 0
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_MSB                                 31
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_MASK                                0x00000000ffffffff


/* Description		PPDU_START_TIMESTAMP_63_32

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, upper 32 bits.
			
			The timestamp is captured by the PHY and given to the MAC
			 in 'PHYRX_RSSI_LEGACY.ppdu_start_timestamp_*.'
			<legal all>
*/

#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_OFFSET                             0x0000000000000008
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_LSB                                32
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_MSB                                63
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_MASK                               0xffffffff00000000


/* Description		RXFRAME_ASSERT_TIMESTAMP

			MAC timer Timestamp that indicates when PHY asserted the
			 'rx_frame' signal for the reception of this PPDU
			<legal all>
*/

#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_OFFSET                               0x0000000000000010
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_LSB                                  0
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_MSB                                  31
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_MASK                                 0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_PPDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000010
#define RX_PPDU_START_TLV64_PADDING_LSB                                             32
#define RX_PPDU_START_TLV64_PADDING_MSB                                             63
#define RX_PPDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif   // RX_PPDU_START
