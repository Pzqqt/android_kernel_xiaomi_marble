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

#ifndef _TX_RATE_STATS_INFO_H_
#define _TX_RATE_STATS_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_RATE_STATS_INFO 2


struct tx_rate_stats_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_rate_stats_info_valid                                :  1, // [0:0]
                      transmit_bw                                             :  3, // [3:1]
                      transmit_pkt_type                                       :  4, // [7:4]
                      transmit_stbc                                           :  1, // [8:8]
                      transmit_ldpc                                           :  1, // [9:9]
                      transmit_sgi                                            :  2, // [11:10]
                      transmit_mcs                                            :  4, // [15:12]
                      ofdma_transmission                                      :  1, // [16:16]
                      tones_in_ru                                             : 12, // [28:17]
                      transmit_nss                                            :  3; // [31:29]
             uint32_t ppdu_transmission_tsf                                   : 32; // [31:0]
#else
             uint32_t transmit_nss                                            :  3, // [31:29]
                      tones_in_ru                                             : 12, // [28:17]
                      ofdma_transmission                                      :  1, // [16:16]
                      transmit_mcs                                            :  4, // [15:12]
                      transmit_sgi                                            :  2, // [11:10]
                      transmit_ldpc                                           :  1, // [9:9]
                      transmit_stbc                                           :  1, // [8:8]
                      transmit_pkt_type                                       :  4, // [7:4]
                      transmit_bw                                             :  3, // [3:1]
                      tx_rate_stats_info_valid                                :  1; // [0:0]
             uint32_t ppdu_transmission_tsf                                   : 32; // [31:0]
#endif
};


/* Description		TX_RATE_STATS_INFO_VALID

			When set all other fields in this STRUCT contain valid info.
			
			
			When clear, none of the other fields contain valid info.
			
			<legal all>
*/

#define TX_RATE_STATS_INFO_TX_RATE_STATS_INFO_VALID_OFFSET                          0x00000000
#define TX_RATE_STATS_INFO_TX_RATE_STATS_INFO_VALID_LSB                             0
#define TX_RATE_STATS_INFO_TX_RATE_STATS_INFO_VALID_MSB                             0
#define TX_RATE_STATS_INFO_TX_RATE_STATS_INFO_VALID_MASK                            0x00000001


/* Description		TRANSMIT_BW

			Field only valid when Tx_rate_stats_info_valid is set
			
			Indicates the BW of the upcoming transmission that shall
			 likely start in about 3 -4 us on the medium
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_RATE_STATS_INFO_TRANSMIT_BW_OFFSET                                       0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_BW_LSB                                          1
#define TX_RATE_STATS_INFO_TRANSMIT_BW_MSB                                          3
#define TX_RATE_STATS_INFO_TRANSMIT_BW_MASK                                         0x0000000e


/* Description		TRANSMIT_PKT_TYPE

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			The packet type
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

#define TX_RATE_STATS_INFO_TRANSMIT_PKT_TYPE_OFFSET                                 0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_PKT_TYPE_LSB                                    4
#define TX_RATE_STATS_INFO_TRANSMIT_PKT_TYPE_MSB                                    7
#define TX_RATE_STATS_INFO_TRANSMIT_PKT_TYPE_MASK                                   0x000000f0


/* Description		TRANSMIT_STBC

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			When set, STBC transmission rate was used.
*/

#define TX_RATE_STATS_INFO_TRANSMIT_STBC_OFFSET                                     0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_STBC_LSB                                        8
#define TX_RATE_STATS_INFO_TRANSMIT_STBC_MSB                                        8
#define TX_RATE_STATS_INFO_TRANSMIT_STBC_MASK                                       0x00000100


/* Description		TRANSMIT_LDPC

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			When set, use LDPC transmission rates
*/

#define TX_RATE_STATS_INFO_TRANSMIT_LDPC_OFFSET                                     0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_LDPC_LSB                                        9
#define TX_RATE_STATS_INFO_TRANSMIT_LDPC_MSB                                        9
#define TX_RATE_STATS_INFO_TRANSMIT_LDPC_MASK                                       0x00000200


/* Description		TRANSMIT_SGI

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			Specify the right GI for HE-Ranging NDPs (11az)/Short NDP.
			
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			<legal 0 - 3>
*/

#define TX_RATE_STATS_INFO_TRANSMIT_SGI_OFFSET                                      0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_SGI_LSB                                         10
#define TX_RATE_STATS_INFO_TRANSMIT_SGI_MSB                                         11
#define TX_RATE_STATS_INFO_TRANSMIT_SGI_MASK                                        0x00000c00


/* Description		TRANSMIT_MCS

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define TX_RATE_STATS_INFO_TRANSMIT_MCS_OFFSET                                      0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_MCS_LSB                                         12
#define TX_RATE_STATS_INFO_TRANSMIT_MCS_MSB                                         15
#define TX_RATE_STATS_INFO_TRANSMIT_MCS_MASK                                        0x0000f000


/* Description		OFDMA_TRANSMISSION

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			
			Set when the transmission was an OFDMA transmission (DL 
			or UL).
			<legal all>
*/

#define TX_RATE_STATS_INFO_OFDMA_TRANSMISSION_OFFSET                                0x00000000
#define TX_RATE_STATS_INFO_OFDMA_TRANSMISSION_LSB                                   16
#define TX_RATE_STATS_INFO_OFDMA_TRANSMISSION_MSB                                   16
#define TX_RATE_STATS_INFO_OFDMA_TRANSMISSION_MASK                                  0x00010000


/* Description		TONES_IN_RU

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			The number of tones in the RU used.
			<legal all>
*/

#define TX_RATE_STATS_INFO_TONES_IN_RU_OFFSET                                       0x00000000
#define TX_RATE_STATS_INFO_TONES_IN_RU_LSB                                          17
#define TX_RATE_STATS_INFO_TONES_IN_RU_MSB                                          28
#define TX_RATE_STATS_INFO_TONES_IN_RU_MASK                                         0x1ffe0000


/* Description		TRANSMIT_NSS

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG
			Not valid when in SW transmit mode
			
			The number of spatial streams used in the transmission
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define TX_RATE_STATS_INFO_TRANSMIT_NSS_OFFSET                                      0x00000000
#define TX_RATE_STATS_INFO_TRANSMIT_NSS_LSB                                         29
#define TX_RATE_STATS_INFO_TRANSMIT_NSS_MSB                                         31
#define TX_RATE_STATS_INFO_TRANSMIT_NSS_MASK                                        0xe0000000


/* Description		PPDU_TRANSMISSION_TSF

			Field only valid when Tx_rate_stats_info_valid is set
			
			Based on a HWSCH configuration register setting, this field
			 either contains:
			
			Lower 32 bits of the TSF, snapshot of this value when transmission
			 of the PPDU containing the frame finished.
			OR
			Lower 32 bits of the TSF, snapshot of this value when transmission
			 of the PPDU containing the frame started
			
			<legal all>
*/

#define TX_RATE_STATS_INFO_PPDU_TRANSMISSION_TSF_OFFSET                             0x00000004
#define TX_RATE_STATS_INFO_PPDU_TRANSMISSION_TSF_LSB                                0
#define TX_RATE_STATS_INFO_PPDU_TRANSMISSION_TSF_MSB                                31
#define TX_RATE_STATS_INFO_PPDU_TRANSMISSION_TSF_MASK                               0xffffffff



#endif   // TX_RATE_STATS_INFO
