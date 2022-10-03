
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

 
 
 
 
 
 
 


#ifndef _COEX_TX_REQ_H_
#define _COEX_TX_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_COEX_TX_REQ 4

#define NUM_OF_QWORDS_COEX_TX_REQ 2


struct coex_tx_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_pwr                                                  :  8, // [7:0]
                      min_tx_pwr                                              :  8, // [15:8]
                      nss                                                     :  3, // [18:16]
                      tx_chain_mask                                           :  8, // [26:19]
                      bw                                                      :  3, // [29:27]
                      reserved_0                                              :  2; // [31:30]
             uint32_t alt_tx_pwr                                              :  8, // [7:0]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_nss                                                 :  3, // [18:16]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_bw                                                  :  3, // [29:27]
                      reserved_1                                              :  2; // [31:30]
             uint32_t tx_pwr_1                                                :  8, // [7:0]
                      alt_tx_pwr_1                                            :  8, // [15:8]
                      wlan_request_duration                                   : 16; // [31:16]
             uint32_t wlan_pkt_type                                           :  4, // [3:0]
                      coex_tx_reason                                          :  2, // [5:4]
                      response_frame_type                                     :  5, // [10:6]
                      wlan_low_priority_slicing_allowed                       :  1, // [11:11]
                      wlan_high_priority_slicing_allowed                      :  1, // [12:12]
                      sch_tx_burst_ongoing                                    :  1, // [13:13]
                      coex_tx_priority                                        :  4, // [17:14]
                      reserved_3a                                             : 14; // [31:18]
#else
             uint32_t reserved_0                                              :  2, // [31:30]
                      bw                                                      :  3, // [29:27]
                      tx_chain_mask                                           :  8, // [26:19]
                      nss                                                     :  3, // [18:16]
                      min_tx_pwr                                              :  8, // [15:8]
                      tx_pwr                                                  :  8; // [7:0]
             uint32_t reserved_1                                              :  2, // [31:30]
                      alt_bw                                                  :  3, // [29:27]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_nss                                                 :  3, // [18:16]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_tx_pwr                                              :  8; // [7:0]
             uint32_t wlan_request_duration                                   : 16, // [31:16]
                      alt_tx_pwr_1                                            :  8, // [15:8]
                      tx_pwr_1                                                :  8; // [7:0]
             uint32_t reserved_3a                                             : 14, // [31:18]
                      coex_tx_priority                                        :  4, // [17:14]
                      sch_tx_burst_ongoing                                    :  1, // [13:13]
                      wlan_high_priority_slicing_allowed                      :  1, // [12:12]
                      wlan_low_priority_slicing_allowed                       :  1, // [11:11]
                      response_frame_type                                     :  5, // [10:6]
                      coex_tx_reason                                          :  2, // [5:4]
                      wlan_pkt_type                                           :  4; // [3:0]
#endif
};


/* Description		TX_PWR

			Default (desired) transmit parameter
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_REQ_TX_PWR_OFFSET                                                   0x0000000000000000
#define COEX_TX_REQ_TX_PWR_LSB                                                      0
#define COEX_TX_REQ_TX_PWR_MSB                                                      7
#define COEX_TX_REQ_TX_PWR_MASK                                                     0x00000000000000ff


/* Description		MIN_TX_PWR

			Default (desired) transmit parameter
			
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_REQ_MIN_TX_PWR_OFFSET                                               0x0000000000000000
#define COEX_TX_REQ_MIN_TX_PWR_LSB                                                  8
#define COEX_TX_REQ_MIN_TX_PWR_MSB                                                  15
#define COEX_TX_REQ_MIN_TX_PWR_MASK                                                 0x000000000000ff00


/* Description		NSS

			Default (desired) transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define COEX_TX_REQ_NSS_OFFSET                                                      0x0000000000000000
#define COEX_TX_REQ_NSS_LSB                                                         16
#define COEX_TX_REQ_NSS_MSB                                                         18
#define COEX_TX_REQ_NSS_MASK                                                        0x0000000000070000


/* Description		TX_CHAIN_MASK

			Default (desired) transmit parameter
			
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define COEX_TX_REQ_TX_CHAIN_MASK_OFFSET                                            0x0000000000000000
#define COEX_TX_REQ_TX_CHAIN_MASK_LSB                                               19
#define COEX_TX_REQ_TX_CHAIN_MASK_MSB                                               26
#define COEX_TX_REQ_TX_CHAIN_MASK_MASK                                              0x0000000007f80000


/* Description		BW

			Default (desired) transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define COEX_TX_REQ_BW_OFFSET                                                       0x0000000000000000
#define COEX_TX_REQ_BW_LSB                                                          27
#define COEX_TX_REQ_BW_MSB                                                          29
#define COEX_TX_REQ_BW_MASK                                                         0x0000000038000000


/* Description		RESERVED_0

			<legal 0>
*/

#define COEX_TX_REQ_RESERVED_0_OFFSET                                               0x0000000000000000
#define COEX_TX_REQ_RESERVED_0_LSB                                                  30
#define COEX_TX_REQ_RESERVED_0_MSB                                                  31
#define COEX_TX_REQ_RESERVED_0_MASK                                                 0x00000000c0000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_REQ_ALT_TX_PWR_OFFSET                                               0x0000000000000000
#define COEX_TX_REQ_ALT_TX_PWR_LSB                                                  32
#define COEX_TX_REQ_ALT_TX_PWR_MSB                                                  39
#define COEX_TX_REQ_ALT_TX_PWR_MASK                                                 0x000000ff00000000


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_REQ_ALT_MIN_TX_PWR_OFFSET                                           0x0000000000000000
#define COEX_TX_REQ_ALT_MIN_TX_PWR_LSB                                              40
#define COEX_TX_REQ_ALT_MIN_TX_PWR_MSB                                              47
#define COEX_TX_REQ_ALT_MIN_TX_PWR_MASK                                             0x0000ff0000000000


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define COEX_TX_REQ_ALT_NSS_OFFSET                                                  0x0000000000000000
#define COEX_TX_REQ_ALT_NSS_LSB                                                     48
#define COEX_TX_REQ_ALT_NSS_MSB                                                     50
#define COEX_TX_REQ_ALT_NSS_MASK                                                    0x0007000000000000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			
			<legal 1-255>
*/

#define COEX_TX_REQ_ALT_TX_CHAIN_MASK_OFFSET                                        0x0000000000000000
#define COEX_TX_REQ_ALT_TX_CHAIN_MASK_LSB                                           51
#define COEX_TX_REQ_ALT_TX_CHAIN_MASK_MSB                                           58
#define COEX_TX_REQ_ALT_TX_CHAIN_MASK_MASK                                          0x07f8000000000000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter.
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define COEX_TX_REQ_ALT_BW_OFFSET                                                   0x0000000000000000
#define COEX_TX_REQ_ALT_BW_LSB                                                      59
#define COEX_TX_REQ_ALT_BW_MSB                                                      61
#define COEX_TX_REQ_ALT_BW_MASK                                                     0x3800000000000000


/* Description		RESERVED_1

			<legal 0>
*/

#define COEX_TX_REQ_RESERVED_1_OFFSET                                               0x0000000000000000
#define COEX_TX_REQ_RESERVED_1_LSB                                                  62
#define COEX_TX_REQ_RESERVED_1_MSB                                                  63
#define COEX_TX_REQ_RESERVED_1_MASK                                                 0xc000000000000000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define COEX_TX_REQ_TX_PWR_1_OFFSET                                                 0x0000000000000008
#define COEX_TX_REQ_TX_PWR_1_LSB                                                    0
#define COEX_TX_REQ_TX_PWR_1_MSB                                                    7
#define COEX_TX_REQ_TX_PWR_1_MASK                                                   0x00000000000000ff


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define COEX_TX_REQ_ALT_TX_PWR_1_OFFSET                                             0x0000000000000008
#define COEX_TX_REQ_ALT_TX_PWR_1_LSB                                                8
#define COEX_TX_REQ_ALT_TX_PWR_1_MSB                                                15
#define COEX_TX_REQ_ALT_TX_PWR_1_MASK                                               0x000000000000ff00


/* Description		WLAN_REQUEST_DURATION

			The amount of time PDG might use for the upcoming transmission
			 and corresponding reception if there is one...
			<legal all>
*/

#define COEX_TX_REQ_WLAN_REQUEST_DURATION_OFFSET                                    0x0000000000000008
#define COEX_TX_REQ_WLAN_REQUEST_DURATION_LSB                                       16
#define COEX_TX_REQ_WLAN_REQUEST_DURATION_MSB                                       31
#define COEX_TX_REQ_WLAN_REQUEST_DURATION_MASK                                      0x00000000ffff0000


/* Description		WLAN_PKT_TYPE

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

#define COEX_TX_REQ_WLAN_PKT_TYPE_OFFSET                                            0x0000000000000008
#define COEX_TX_REQ_WLAN_PKT_TYPE_LSB                                               32
#define COEX_TX_REQ_WLAN_PKT_TYPE_MSB                                               35
#define COEX_TX_REQ_WLAN_PKT_TYPE_MASK                                              0x0000000f00000000


/* Description		COEX_TX_REASON

			<enum 0     cxc_fes_protection_frame>  RTS, CTS2Self or 
			11h protection type transmission preceding the regular PPDU
			 portion of the coming FES. 
			<enum 1     cxc_fes_after_protection >  Regular PPDU transmission
			 that follows the transmission of medium protection frames:.
			
			<enum 2     cxc_fes_only>  Regular PPDU transmission without
			 preceding medium protection frame exchanges. 
			
			<enum 3 cxc_response_frame>  
			HW generated response frame.
			Details of the response frame type provided in field: Response_frame_type
			
			
			<legal 0-3>
*/

#define COEX_TX_REQ_COEX_TX_REASON_OFFSET                                           0x0000000000000008
#define COEX_TX_REQ_COEX_TX_REASON_LSB                                              36
#define COEX_TX_REQ_COEX_TX_REASON_MSB                                              37
#define COEX_TX_REQ_COEX_TX_REASON_MASK                                             0x0000003000000000


/* Description		RESPONSE_FRAME_TYPE

			Coex related field
			<enum 0 Resp_Non_11ah_ACK >  
			<enum 1 Resp_Non_11ah_BA >
			<enum 2 Resp_Non_11ah_CTS > 
			<enum 3 Resp_AH_NDP_CTS> 
			<enum 4 Resp_AH_NDP_ACK>
			<enum 5 Resp_AH_NDP_BA>
			<enum 6 Resp_AH_NDP_MOD_ACK>
			<enum 7 Resp_AH_Normal_ACK>
			<enum 8 Resp_AH_Normal_BA>
			<enum 9  Resp_RTT_ACK>
			<enum 10 Resp_CBF_RESPONSE>
			<enum 11 Resp_MBA>
			<enum 12 Resp_Ranging_NDP>
			<enum 13 Resp_LMR_RESPONSE>
			<enum 14 Resp_TRIGGER_RESPONSE_BASIC>
			<enum 15 Resp_TRIGGER_RESPONSE_BUF_SIZE>
			<enum 16 Resp_TRIGGER_RESPONSE_BRPOLL>
			<enum 17 Resp_TRIGGER_RESPONSE_CTS>
			<enum 18 Resp_TRIGGER_RESPONSE_OTHER>
			
			<legal 0-18>
*/

#define COEX_TX_REQ_RESPONSE_FRAME_TYPE_OFFSET                                      0x0000000000000008
#define COEX_TX_REQ_RESPONSE_FRAME_TYPE_LSB                                         38
#define COEX_TX_REQ_RESPONSE_FRAME_TYPE_MSB                                         42
#define COEX_TX_REQ_RESPONSE_FRAME_TYPE_MASK                                        0x000007c000000000


/* Description		WLAN_LOW_PRIORITY_SLICING_ALLOWED

			When set, COEX is allowed to invoke 'tx slicing' algorithms
			 when WLAN tx is low priority when compared to BT activity, 
			to get to more optimal throughput. Value 0 will disable 
			this feature
			<legal all>
*/

#define COEX_TX_REQ_WLAN_LOW_PRIORITY_SLICING_ALLOWED_OFFSET                        0x0000000000000008
#define COEX_TX_REQ_WLAN_LOW_PRIORITY_SLICING_ALLOWED_LSB                           43
#define COEX_TX_REQ_WLAN_LOW_PRIORITY_SLICING_ALLOWED_MSB                           43
#define COEX_TX_REQ_WLAN_LOW_PRIORITY_SLICING_ALLOWED_MASK                          0x0000080000000000


/* Description		WLAN_HIGH_PRIORITY_SLICING_ALLOWED

			When set, COEX is allowed to invoke 'tx slicing' algorithms
			 when WLAN tx is high priority when compared to BT activity, 
			to get to more optimal throughput. Value 0 will disable 
			this feature.
			<legal all>
*/

#define COEX_TX_REQ_WLAN_HIGH_PRIORITY_SLICING_ALLOWED_OFFSET                       0x0000000000000008
#define COEX_TX_REQ_WLAN_HIGH_PRIORITY_SLICING_ALLOWED_LSB                          44
#define COEX_TX_REQ_WLAN_HIGH_PRIORITY_SLICING_ALLOWED_MSB                          44
#define COEX_TX_REQ_WLAN_HIGH_PRIORITY_SLICING_ALLOWED_MASK                         0x0000100000000000


/* Description		SCH_TX_BURST_ONGOING

			0: No action
			1: The next scheduling command needs to start at SIFS time
			 after finishing the frame transmissions in this command. 
			This allows for SIFS based bursting
			<legal all>
*/

#define COEX_TX_REQ_SCH_TX_BURST_ONGOING_OFFSET                                     0x0000000000000008
#define COEX_TX_REQ_SCH_TX_BURST_ONGOING_LSB                                        45
#define COEX_TX_REQ_SCH_TX_BURST_ONGOING_MSB                                        45
#define COEX_TX_REQ_SCH_TX_BURST_ONGOING_MASK                                       0x0000200000000000


/* Description		COEX_TX_PRIORITY

			Transmit priority. Used for Coex weight table look up in
			 case of regular FES transmission. This value is typically
			 programmed in relationship to the backoff engine. In case
			 of self_gen tx,  the value comes from a programmable register
			 in the TXPCU. For BA and ACK packets, this is related to
			 AC of the incoming frame. .  
			
			For a request type of "fes", the field is copied over from
			 the scheduling command TLV.
			<legal all>
*/

#define COEX_TX_REQ_COEX_TX_PRIORITY_OFFSET                                         0x0000000000000008
#define COEX_TX_REQ_COEX_TX_PRIORITY_LSB                                            46
#define COEX_TX_REQ_COEX_TX_PRIORITY_MSB                                            49
#define COEX_TX_REQ_COEX_TX_PRIORITY_MASK                                           0x0003c00000000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define COEX_TX_REQ_RESERVED_3A_OFFSET                                              0x0000000000000008
#define COEX_TX_REQ_RESERVED_3A_LSB                                                 50
#define COEX_TX_REQ_RESERVED_3A_MSB                                                 63
#define COEX_TX_REQ_RESERVED_3A_MASK                                                0xfffc000000000000



#endif   // COEX_TX_REQ
