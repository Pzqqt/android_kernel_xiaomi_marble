
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

 
 
 
 
 
 
 


#ifndef _COEX_RX_STATUS_H_
#define _COEX_RX_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_COEX_RX_STATUS 2

#define NUM_OF_QWORDS_COEX_RX_STATUS 1


struct coex_rx_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_mac_frame_status                                     :  2, // [1:0]
                      rx_with_tx_response                                     :  1, // [2:2]
                      rx_rate                                                 :  5, // [7:3]
                      rx_bw                                                   :  3, // [10:8]
                      single_mpdu                                             :  1, // [11:11]
                      filter_status                                           :  1, // [12:12]
                      ampdu                                                   :  1, // [13:13]
                      directed                                                :  1, // [14:14]
                      reserved_0                                              :  1, // [15:15]
                      rx_nss                                                  :  3, // [18:16]
                      rx_rssi                                                 :  8, // [26:19]
                      rx_type                                                 :  3, // [29:27]
                      retry_bit_setting                                       :  1, // [30:30]
                      more_data_bit_setting                                   :  1; // [31:31]
             uint32_t remain_rx_packet_time                                   : 16, // [15:0]
                      rx_remaining_fes_time                                   : 16; // [31:16]
#else
             uint32_t more_data_bit_setting                                   :  1, // [31:31]
                      retry_bit_setting                                       :  1, // [30:30]
                      rx_type                                                 :  3, // [29:27]
                      rx_rssi                                                 :  8, // [26:19]
                      rx_nss                                                  :  3, // [18:16]
                      reserved_0                                              :  1, // [15:15]
                      directed                                                :  1, // [14:14]
                      ampdu                                                   :  1, // [13:13]
                      filter_status                                           :  1, // [12:12]
                      single_mpdu                                             :  1, // [11:11]
                      rx_bw                                                   :  3, // [10:8]
                      rx_rate                                                 :  5, // [7:3]
                      rx_with_tx_response                                     :  1, // [2:2]
                      rx_mac_frame_status                                     :  2; // [1:0]
             uint32_t rx_remaining_fes_time                                   : 16, // [31:16]
                      remain_rx_packet_time                                   : 16; // [15:0]
#endif
};


/* Description		RX_MAC_FRAME_STATUS

			RXPCU send this bit as 1 when it receives the begin of a
			 frame from PHY, and it passes the address filter. RXPCUsend
			 this bit as 0 when the frame ends. (on/off bit)
			<enum 0     ppdu_start> start of PPDU reception. 
			For SU: Generated the first time the MPDU header passes 
			the address filter and is destined to this STA. 
			For MU: Generated the first time the MPDU header from any
			 user passes the address filter and is destined to this 
			STA.
			<enum 1     first_mpdu_FCS_pass> message only sent in case
			 of A-MPDU reception.
			For SU:  first time the FCS of an MPDU passes (and frame
			 is destined to this device)
			For MU:  first time the FCS of any MPDU passes (and frame
			 is destined to this device)
			
			<enum 2     ppdu_end> receive of PPDU frame reception has
			 finished 
			<enum 3 ppdu_end_due_to_phy_nap> receive of PPDU frame reception
			 has finished as it has been aborted due to PHY NAP generation
			
			<legal all>
*/

#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_OFFSET                                   0x0000000000000000
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_LSB                                      0
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_MSB                                      1
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_MASK                                     0x0000000000000003


/* Description		RX_WITH_TX_RESPONSE

			Field only valid when rx_mac_frame_status is first_mpdu_FCS_pass
			 or ppdu_end.
			
			For SU: RXPCU set this bit to indicate it is expecting the
			 TX to send a response after the receive. 
			For MU: RXPCU set this bit to indicate it is expecting that
			 at least for one of the users a response after the reception
			 needs to be generated.
			
			<legal all>
*/

#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_OFFSET                                   0x0000000000000000
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_LSB                                      2
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_MSB                                      2
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_MASK                                     0x0000000000000004


/* Description		RX_RATE

			For SU: RXPCU send the current receive rate at the beginning
			 of receive when rate is available from PHY. 
			For MU: RXPCU to use the current receive rate from the first
			 USER that triggers this TLV to be generated.
			
			 Field is always valid
			
			 <legal all>
*/

#define COEX_RX_STATUS_RX_RATE_OFFSET                                               0x0000000000000000
#define COEX_RX_STATUS_RX_RATE_LSB                                                  3
#define COEX_RX_STATUS_RX_RATE_MSB                                                  7
#define COEX_RX_STATUS_RX_RATE_MASK                                                 0x00000000000000f8


/* Description		RX_BW

			Actual RX bandwidth. Not SU or MU dependent.
			RXPCU send the current receive rate at the beginning of 
			receive. This information is from PHY.
			Field is always valid 
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define COEX_RX_STATUS_RX_BW_OFFSET                                                 0x0000000000000000
#define COEX_RX_STATUS_RX_BW_LSB                                                    8
#define COEX_RX_STATUS_RX_BW_MSB                                                    10
#define COEX_RX_STATUS_RX_BW_MASK                                                   0x0000000000000700


/* Description		SINGLE_MPDU

			For SU: Once set the Received frame is a single MPDU. This
			 can be a non-AMPDU reception or A-MPDU reception but with
			 an EOF bit set (VHT single AMPDU).
			For MU: RXPCU to base this on the first USER that triggers
			 this TLV to be generated.
			<legal all>
*/

#define COEX_RX_STATUS_SINGLE_MPDU_OFFSET                                           0x0000000000000000
#define COEX_RX_STATUS_SINGLE_MPDU_LSB                                              11
#define COEX_RX_STATUS_SINGLE_MPDU_MSB                                              11
#define COEX_RX_STATUS_SINGLE_MPDU_MASK                                             0x0000000000000800


/* Description		FILTER_STATUS

			1: LMAC is interested in receiving the full packet and forward
			 it to downstream modules. 0: LMAC is not interested in 
			receiving the packet.
			
			In HastingsPrime based on the register bit 'COEX_RX_STATUS_NOFILTERIN,' 
			Rx PCU will send this TLV for filtered-out packets as well, 
			with appropriate info in the fields filter_status, AMPDU
			 and Directed. Otherwise, and in other chips, this TLV is
			 sent only for packets filtered in, with these fields set
			  to zero.
			<legal all>
*/

#define COEX_RX_STATUS_FILTER_STATUS_OFFSET                                         0x0000000000000000
#define COEX_RX_STATUS_FILTER_STATUS_LSB                                            12
#define COEX_RX_STATUS_FILTER_STATUS_MSB                                            12
#define COEX_RX_STATUS_FILTER_STATUS_MASK                                           0x0000000000001000


/* Description		AMPDU

			1: Indicates received frame is an AMPDU0: indicates received
			 frames in not an AMPDU
			
			In HastingsPrime based on the register bit 'COEX_RX_STATUS_NOFILTERIN,' 
			Rx PCU will send this TLV for filtered-out packets as well, 
			with appropriate info in the fields filter_status, AMPDU
			 and Directed. Otherwise, and in other chips, this TLV is
			 sent only for packets filtered in, with these fields set
			 to zero.
			<legal all>
*/

#define COEX_RX_STATUS_AMPDU_OFFSET                                                 0x0000000000000000
#define COEX_RX_STATUS_AMPDU_LSB                                                    13
#define COEX_RX_STATUS_AMPDU_MSB                                                    13
#define COEX_RX_STATUS_AMPDU_MASK                                                   0x0000000000002000


/* Description		DIRECTED

			1: indicates AD1 matches our Receiver address0: indicates
			 AD1 does not match our Receiver address
			
			In HastingsPrime based on the register bit 'COEX_RX_STATUS_NOFILTERIN,' 
			Rx PCU will send this TLV for filtered-out packets as well, 
			with appropriate info in the fields filter_status, AMPDU
			 and Directed. Otherwise, and in other chips, this TLV is
			 sent only for packets filtered in, with these fields set
			 to zero.
			<legal all>
*/

#define COEX_RX_STATUS_DIRECTED_OFFSET                                              0x0000000000000000
#define COEX_RX_STATUS_DIRECTED_LSB                                                 14
#define COEX_RX_STATUS_DIRECTED_MSB                                                 14
#define COEX_RX_STATUS_DIRECTED_MASK                                                0x0000000000004000


/* Description		RESERVED_0

			<legal 0>
*/

#define COEX_RX_STATUS_RESERVED_0_OFFSET                                            0x0000000000000000
#define COEX_RX_STATUS_RESERVED_0_LSB                                               15
#define COEX_RX_STATUS_RESERVED_0_MSB                                               15
#define COEX_RX_STATUS_RESERVED_0_MASK                                              0x0000000000008000


/* Description		RX_NSS

			For SU: Number of spatial streams in the reception. Field
			 is always valid
			For MU: RXPCU to base this on the first USER that triggers
			 this TLV to be generated.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define COEX_RX_STATUS_RX_NSS_OFFSET                                                0x0000000000000000
#define COEX_RX_STATUS_RX_NSS_LSB                                                   16
#define COEX_RX_STATUS_RX_NSS_MSB                                                   18
#define COEX_RX_STATUS_RX_NSS_MASK                                                  0x0000000000070000


/* Description		RX_RSSI

			RXPCU send the current receive RSSI (from the PHYRX_RSSI_LEGACY
			 TLV) at the beginning of reception. This is information
			 is from PHY and is not SU or MU dependent. 
			Field is always valid 
			<legal all>
*/

#define COEX_RX_STATUS_RX_RSSI_OFFSET                                               0x0000000000000000
#define COEX_RX_STATUS_RX_RSSI_LSB                                                  19
#define COEX_RX_STATUS_RX_RSSI_MSB                                                  26
#define COEX_RX_STATUS_RX_RSSI_MASK                                                 0x0000000007f80000


/* Description		RX_TYPE

			For SU:  RXPCU send the current receive packet type. Field
			 is always valid.This info is from MAC.
			For MU: RXPCU to base this on the first USER that triggers
			 this TLV to be generated.
			
			<enum 0     data >
			<enum 1     management>
			<enum 2     beacon>
			<enum 3     control> For reception of RTS frame
			<enum 4     control_response>  For reception of CTS, ACK
			 or BA frames
			<enum 5     others> 
			<legal 0-5>
*/

#define COEX_RX_STATUS_RX_TYPE_OFFSET                                               0x0000000000000000
#define COEX_RX_STATUS_RX_TYPE_LSB                                                  27
#define COEX_RX_STATUS_RX_TYPE_MSB                                                  29
#define COEX_RX_STATUS_RX_TYPE_MASK                                                 0x0000000038000000


/* Description		RETRY_BIT_SETTING

			For SU: Value of the retry bit in the frame control field
			 of the first MPDU MAC header that passes the RxPCU frame
			 filter
			For MU: RXPCU to base this on the first USER that triggers
			 this TLV to be generated.
			
			<legal all>
*/

#define COEX_RX_STATUS_RETRY_BIT_SETTING_OFFSET                                     0x0000000000000000
#define COEX_RX_STATUS_RETRY_BIT_SETTING_LSB                                        30
#define COEX_RX_STATUS_RETRY_BIT_SETTING_MSB                                        30
#define COEX_RX_STATUS_RETRY_BIT_SETTING_MASK                                       0x0000000040000000


/* Description		MORE_DATA_BIT_SETTING

			For SU: Value of the more data bit in the frame control 
			field of the first MPDU MAC header that passes the RxPCU
			 frame filter
			For MU: RXPCU to base this on the first USER that triggers
			 this TLV to be generated.
			
			<legal all>
*/

#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_OFFSET                                 0x0000000000000000
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_LSB                                    31
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_MSB                                    31
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_MASK                                   0x0000000080000000


/* Description		REMAIN_RX_PACKET_TIME

			HWSCH sends current remaining rx PPDU frame time. This time
			 covers the entire rx_frame. This information is not in 
			the L-SIG and we expect to get it from PHY at the start 
			of the reception. 
			This is not SU or MU dependent.
			<legal all>
*/

#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_OFFSET                                 0x0000000000000000
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_LSB                                    32
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_MSB                                    47
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_MASK                                   0x0000ffff00000000


/* Description		RX_REMAINING_FES_TIME

			RXPCU sends the remaining time FES time the moment a frame
			 with proper FCS is received. The time indicated is the 
			remaining rx packet time with the duration field value added. 
			As long as no frame with valid FCS is received, this field
			 should be set equal to 'remain_rx_packet_time'
			This is not SU or MU dependent.
			<legal all>
*/

#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_OFFSET                                 0x0000000000000000
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_LSB                                    48
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_MSB                                    63
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_MASK                                   0xffff000000000000



#endif   // COEX_RX_STATUS
