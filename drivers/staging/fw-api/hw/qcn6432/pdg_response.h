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

#ifndef _PDG_RESPONSE_H_
#define _PDG_RESPONSE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "pdg_response_rate_setting.h"
#define NUM_OF_DWORDS_PDG_RESPONSE 12

#define NUM_OF_QWORDS_PDG_RESPONSE 6


struct pdg_response {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   pdg_response_rate_setting                                 hw_response_rate_info;
             uint32_t hw_response_tx_duration                                 : 16, // [15:0]
                      rx_duration_field                                       : 16; // [31:16]
             uint32_t punctured_response_transmission                         :  1, // [0:0]
                      cca_subband_channel_bonding_mask                        : 16, // [16:1]
                      scrambler_seed_override                                 :  2, // [18:17]
                      response_density_valid                                  :  1, // [19:19]
                      response_density                                        :  5, // [24:20]
                      more_data                                               :  1, // [25:25]
                      duration_indication                                     :  1, // [26:26]
                      relayed_frame                                           :  1, // [27:27]
                      address_indicator                                       :  1, // [28:28]
                      bandwidth                                               :  3; // [31:29]
             uint32_t ack_id                                                  : 16, // [15:0]
                      block_ack_bitmap                                        : 16; // [31:16]
             uint32_t response_frame_type                                     :  4, // [3:0]
                      ack_id_ext                                              : 10, // [13:4]
                      ftm_en                                                  :  1, // [14:14]
                      group_id                                                :  6, // [20:15]
                      sta_partial_aid                                         : 11; // [31:21]
             uint32_t ndp_ba_start_seq_ctrl                                   : 12, // [11:0]
                      active_channel                                          :  3, // [14:12]
                      txop_duration_all_ones                                  :  1, // [15:15]
                      frame_length                                            : 16; // [31:16]
#else
             struct   pdg_response_rate_setting                                 hw_response_rate_info;
             uint32_t rx_duration_field                                       : 16, // [31:16]
                      hw_response_tx_duration                                 : 16; // [15:0]
             uint32_t bandwidth                                               :  3, // [31:29]
                      address_indicator                                       :  1, // [28:28]
                      relayed_frame                                           :  1, // [27:27]
                      duration_indication                                     :  1, // [26:26]
                      more_data                                               :  1, // [25:25]
                      response_density                                        :  5, // [24:20]
                      response_density_valid                                  :  1, // [19:19]
                      scrambler_seed_override                                 :  2, // [18:17]
                      cca_subband_channel_bonding_mask                        : 16, // [16:1]
                      punctured_response_transmission                         :  1; // [0:0]
             uint32_t block_ack_bitmap                                        : 16, // [31:16]
                      ack_id                                                  : 16; // [15:0]
             uint32_t sta_partial_aid                                         : 11, // [31:21]
                      group_id                                                :  6, // [20:15]
                      ftm_en                                                  :  1, // [14:14]
                      ack_id_ext                                              : 10, // [13:4]
                      response_frame_type                                     :  4; // [3:0]
             uint32_t frame_length                                            : 16, // [31:16]
                      txop_duration_all_ones                                  :  1, // [15:15]
                      active_channel                                          :  3, // [14:12]
                      ndp_ba_start_seq_ctrl                                   : 12; // [11:0]
#endif
};


/* Description		HW_RESPONSE_RATE_INFO

			All transmit rate related parameters
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_0A_OFFSET                       0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_0A_LSB                          0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_0A_MSB                          0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_0A_MASK                         0x0000000000000001


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_ANTENNA_SECTOR_CTRL_OFFSET            0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_ANTENNA_SECTOR_CTRL_LSB               1
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_ANTENNA_SECTOR_CTRL_MSB               24
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_ANTENNA_SECTOR_CTRL_MASK              0x0000000001fffffe


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

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_PKT_TYPE_OFFSET                          0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_PKT_TYPE_LSB                             25
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_PKT_TYPE_MSB                             28
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_PKT_TYPE_MASK                            0x000000001e000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SMOOTHING_OFFSET                         0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SMOOTHING_LSB                            29
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SMOOTHING_MSB                            29
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SMOOTHING_MASK                           0x0000000020000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_LDPC_OFFSET                              0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_LDPC_LSB                                 30
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_LDPC_MSB                                 30
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_LDPC_MASK                                0x0000000040000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STBC_OFFSET                              0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STBC_LSB                                 31
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STBC_MSB                                 31
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STBC_MASK                                0x0000000080000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_OFFSET                        0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_LSB                           32
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_MSB                           39
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_MASK                          0x000000ff00000000


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_MIN_TX_PWR_OFFSET                    0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_MIN_TX_PWR_LSB                       40
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_MIN_TX_PWR_MSB                       47
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_MIN_TX_PWR_MASK                      0x0000ff0000000000


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

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_NSS_OFFSET                           0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_NSS_LSB                              48
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_NSS_MSB                              50
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_NSS_MASK                             0x0007000000000000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_CHAIN_MASK_OFFSET                 0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_CHAIN_MASK_LSB                    51
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_CHAIN_MASK_MSB                    58
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_CHAIN_MASK_MASK                   0x07f8000000000000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_BW_OFFSET                            0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_BW_LSB                               59
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_BW_MSB                               61
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_BW_MASK                              0x3800000000000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STF_LTF_3DB_BOOST_OFFSET                 0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STF_LTF_3DB_BOOST_LSB                    62
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STF_LTF_3DB_BOOST_MSB                    62
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_STF_LTF_3DB_BOOST_MASK                   0x4000000000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_FORCE_EXTRA_SYMBOL_OFFSET                0x0000000000000000
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_FORCE_EXTRA_SYMBOL_LSB                   63
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_FORCE_EXTRA_SYMBOL_MSB                   63
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_FORCE_EXTRA_SYMBOL_MASK                  0x8000000000000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_RATE_MCS_OFFSET                      0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_RATE_MCS_LSB                         0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_RATE_MCS_MSB                         3
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_RATE_MCS_MASK                        0x000000000000000f


/* Description		NSS

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

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NSS_OFFSET                               0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NSS_LSB                                  4
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NSS_MSB                                  6
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NSS_MASK                                 0x0000000000000070


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DPD_ENABLE_OFFSET                        0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DPD_ENABLE_LSB                           7
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DPD_ENABLE_MSB                           7
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DPD_ENABLE_MASK                          0x0000000000000080


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_OFFSET                            0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_LSB                               8
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_MSB                               15
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_MASK                              0x000000000000ff00


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MIN_TX_PWR_OFFSET                        0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MIN_TX_PWR_LSB                           16
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MIN_TX_PWR_MSB                           23
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MIN_TX_PWR_MASK                          0x0000000000ff0000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_CHAIN_MASK_OFFSET                     0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_CHAIN_MASK_LSB                        24
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_CHAIN_MASK_MSB                        31
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_CHAIN_MASK_MASK                       0x00000000ff000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3A_OFFSET                       0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3A_LSB                          32
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3A_MSB                          39
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3A_MASK                         0x000000ff00000000


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SGI_OFFSET                               0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SGI_LSB                                  40
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SGI_MSB                                  41
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_SGI_MASK                                 0x0000030000000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RATE_MCS_OFFSET                          0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RATE_MCS_LSB                             42
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RATE_MCS_MSB                             45
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RATE_MCS_MASK                            0x00003c0000000000


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3B_OFFSET                       0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3B_LSB                          46
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3B_MSB                          47
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_3B_MASK                         0x0000c00000000000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_1_OFFSET                          0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_1_LSB                             48
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_1_MSB                             55
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_TX_PWR_1_MASK                            0x00ff000000000000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_1_OFFSET                      0x0000000000000008
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_1_LSB                         56
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_1_MSB                         63
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_ALT_TX_PWR_1_MASK                        0xff00000000000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_AGGREGATION_OFFSET                       0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_AGGREGATION_LSB                          0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_AGGREGATION_MSB                          0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_AGGREGATION_MASK                         0x0000000000000001


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_BSS_COLOR_ID_OFFSET              0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_BSS_COLOR_ID_LSB                 1
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_BSS_COLOR_ID_MSB                 6
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_BSS_COLOR_ID_MASK                0x000000000000007e


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SPATIAL_REUSE_OFFSET             0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SPATIAL_REUSE_LSB                7
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SPATIAL_REUSE_MSB                10
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SPATIAL_REUSE_MASK               0x0000000000000780


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CP_LTF_SIZE_OFFSET               0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CP_LTF_SIZE_LSB                  11
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CP_LTF_SIZE_MSB                  12
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CP_LTF_SIZE_MASK                 0x0000000000001800


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DCM_OFFSET                       0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DCM_LSB                          13
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DCM_MSB                          13
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DCM_MASK                         0x0000000000002000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DOPPLER_INDICATION_OFFSET        0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DOPPLER_INDICATION_LSB           14
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DOPPLER_INDICATION_MSB           14
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DOPPLER_INDICATION_MASK          0x0000000000004000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SU_EXTENDED_OFFSET               0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SU_EXTENDED_LSB                  15
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SU_EXTENDED_MSB                  15
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_SU_EXTENDED_MASK                 0x0000000000008000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_MIN_PACKET_EXTENSION_OFFSET      0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_MIN_PACKET_EXTENSION_LSB         16
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_MIN_PACKET_EXTENSION_MSB         17
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_MIN_PACKET_EXTENSION_MASK        0x0000000000030000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_NSS_OFFSET                    0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_NSS_LSB                       18
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_NSS_MSB                       20
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_NSS_MASK                      0x00000000001c0000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CONTENT_OFFSET                0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CONTENT_LSB                   21
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CONTENT_MSB                   21
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CONTENT_MASK                  0x0000000000200000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_LTF_SIZE_OFFSET               0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_LTF_SIZE_LSB                  22
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_LTF_SIZE_MSB                  23
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_LTF_SIZE_MASK                 0x0000000000c00000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CHAIN_CSD_EN_OFFSET              0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CHAIN_CSD_EN_LSB                 24
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CHAIN_CSD_EN_MSB                 24
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_CHAIN_CSD_EN_MASK                0x0000000001000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CHAIN_CSD_EN_OFFSET           0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CHAIN_CSD_EN_LSB              25
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CHAIN_CSD_EN_MSB              25
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_PE_CHAIN_CSD_EN_MASK             0x0000000002000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DL_UL_FLAG_OFFSET                0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DL_UL_FLAG_LSB                   26
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DL_UL_FLAG_MSB                   26
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_DL_UL_FLAG_MASK                  0x0000000004000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_4A_OFFSET                       0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_4A_LSB                          27
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_4A_MSB                          31
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_4A_MASK                         0x00000000f8000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_START_INDEX_OFFSET        0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_START_INDEX_LSB           32
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_START_INDEX_MSB           35
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_START_INDEX_MASK          0x0000000f00000000


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_SIZE_OFFSET               0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_SIZE_LSB                  36
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_SIZE_MSB                  39
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11AX_EXT_RU_SIZE_MASK                 0x000000f000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_EHT_DUPLICATE_MODE_OFFSET                0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_EHT_DUPLICATE_MODE_LSB                   40
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_EHT_DUPLICATE_MODE_MSB                   41
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_EHT_DUPLICATE_MODE_MASK                  0x0000030000000000


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_DCM_OFFSET                       0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_DCM_LSB                          42
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_DCM_MSB                          42
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_DCM_MASK                         0x0000040000000000


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_0_MCS_OFFSET                     0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_0_MCS_LSB                        43
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_0_MCS_MSB                        45
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_HE_SIGB_0_MCS_MASK                       0x0000380000000000


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NUM_HE_SIGB_SYM_OFFSET                   0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NUM_HE_SIGB_SYM_LSB                      46
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NUM_HE_SIGB_SYM_MSB                      50
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_NUM_HE_SIGB_SYM_MASK                     0x0007c00000000000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET     0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_SOURCE_LSB        51
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_SOURCE_MSB        51
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_SOURCE_MASK       0x0008000000000000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_5A_OFFSET                       0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_5A_LSB                          52
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_5A_MSB                          57
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_RESERVED_5A_MASK                         0x03f0000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET   0x0000000000000010
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB      58
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB      63
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK     0xfc00000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB    13
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB    15
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK   0x000000000000e000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_OFFSET            0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_LSB               16
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_MSB               27
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_REQUIRED_RESPONSE_TIME_MASK              0x000000000fff0000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11BE_PARAMS_PLACEHOLDER_OFFSET        0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11BE_PARAMS_PLACEHOLDER_LSB           28
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11BE_PARAMS_PLACEHOLDER_MSB           31
#define PDG_RESPONSE_HW_RESPONSE_RATE_INFO_DOT11BE_PARAMS_PLACEHOLDER_MASK          0x00000000f0000000


/* Description		HW_RESPONSE_TX_DURATION

			The amount of time the transmission of the HW response will
			 take (in us)
			
			Used for coex.....
			
			This field also represents the 'alt_hw_response_tx_duration'. 
			Note that this implies that no different duration can be
			 programmed for the default and alt setting. SW should program
			 the worst case value in the RXPCU table in case they are
			 different.
			<legal all>
*/

#define PDG_RESPONSE_HW_RESPONSE_TX_DURATION_OFFSET                                 0x0000000000000018
#define PDG_RESPONSE_HW_RESPONSE_TX_DURATION_LSB                                    32
#define PDG_RESPONSE_HW_RESPONSE_TX_DURATION_MSB                                    47
#define PDG_RESPONSE_HW_RESPONSE_TX_DURATION_MASK                                   0x0000ffff00000000


/* Description		RX_DURATION_FIELD

			The duration field in the received frame.
			PDG uses this field to calculate what the duration field
			 value should be in the response frame.
			This is returned to the TX PCU
			
			This field is used in 11ah mode as well
			<legal all>
*/

#define PDG_RESPONSE_RX_DURATION_FIELD_OFFSET                                       0x0000000000000018
#define PDG_RESPONSE_RX_DURATION_FIELD_LSB                                          48
#define PDG_RESPONSE_RX_DURATION_FIELD_MSB                                          63
#define PDG_RESPONSE_RX_DURATION_FIELD_MASK                                         0xffff000000000000


/* Description		PUNCTURED_RESPONSE_TRANSMISSION

			When set, this response frame will be transmitted using 
			a puncture transmit pattern that is indicated in the cca_subband_channel_bonding_mask
			 field.
			
			Typically used in the Response to response transmissions.
			
			<legal all>
*/

#define PDG_RESPONSE_PUNCTURED_RESPONSE_TRANSMISSION_OFFSET                         0x0000000000000020
#define PDG_RESPONSE_PUNCTURED_RESPONSE_TRANSMISSION_LSB                            0
#define PDG_RESPONSE_PUNCTURED_RESPONSE_TRANSMISSION_MSB                            0
#define PDG_RESPONSE_PUNCTURED_RESPONSE_TRANSMISSION_MASK                           0x0000000000000001


/* Description		CCA_SUBBAND_CHANNEL_BONDING_MASK

			Field only valid when 'Punctured_response_transmission' 
			is set
			
			Indicates which 20 Mhz channels will be used for the transmission.
			
			
			Bit 0: primary 20 Mhz
			Bit 1: secondary 20 MHz.
			Etc.
			
			<legal 1-65535>
*/

#define PDG_RESPONSE_CCA_SUBBAND_CHANNEL_BONDING_MASK_OFFSET                        0x0000000000000020
#define PDG_RESPONSE_CCA_SUBBAND_CHANNEL_BONDING_MASK_LSB                           1
#define PDG_RESPONSE_CCA_SUBBAND_CHANNEL_BONDING_MASK_MSB                           16
#define PDG_RESPONSE_CCA_SUBBAND_CHANNEL_BONDING_MASK_MASK                          0x000000000001fffe


/* Description		SCRAMBLER_SEED_OVERRIDE

			Used in dynamic BW RTS-CTS, BAR -BA, etc. kind of exchanges.
			
			
			0: PDG will use all 7 bits of the scrambler seed.
			1: PDG will override bits [6:5] of the  scrambler_seed  
			with BW information. 
			2: PDG will override bits [6:5] and bit [3] of the  scrambler_seed
			  with BW information for .11be dynamic BW procedure.
			
			<legal 0-2>
*/

#define PDG_RESPONSE_SCRAMBLER_SEED_OVERRIDE_OFFSET                                 0x0000000000000020
#define PDG_RESPONSE_SCRAMBLER_SEED_OVERRIDE_LSB                                    17
#define PDG_RESPONSE_SCRAMBLER_SEED_OVERRIDE_MSB                                    18
#define PDG_RESPONSE_SCRAMBLER_SEED_OVERRIDE_MASK                                   0x0000000000060000


/* Description		RESPONSE_DENSITY_VALID

			When set, field Response_density has valid info. TXPCU sets
			 this for multi segment CBF response generation.
			<legal all>
*/

#define PDG_RESPONSE_RESPONSE_DENSITY_VALID_OFFSET                                  0x0000000000000020
#define PDG_RESPONSE_RESPONSE_DENSITY_VALID_LSB                                     19
#define PDG_RESPONSE_RESPONSE_DENSITY_VALID_MSB                                     19
#define PDG_RESPONSE_RESPONSE_DENSITY_VALID_MASK                                    0x0000000000080000


/* Description		RESPONSE_DENSITY

			Field only valid when Response_density_valid is set.
			When Response_density_valid is NOT set, this field is set
			 to 0
			
			The MPDU density is required for the response frame (in 
			us). PDG will translate this value into minimum number of
			 words per MPDU and give this back to TXPCU in TLV PCU_PPDU_SETUP_USER
			 field min_mpdu_spacing
			
			TXPCU gets this value from a register.
			<legal all>
*/

#define PDG_RESPONSE_RESPONSE_DENSITY_OFFSET                                        0x0000000000000020
#define PDG_RESPONSE_RESPONSE_DENSITY_LSB                                           20
#define PDG_RESPONSE_RESPONSE_DENSITY_MSB                                           24
#define PDG_RESPONSE_RESPONSE_DENSITY_MASK                                          0x0000000001f00000


/* Description		MORE_DATA

			This setting is used for 
			NDP ACK response frames
			NDP Modified ACK response frames 
			The value of this field comes from a register programming. 
			The register resides in TxPCU and is programmed by SW within
			 SIFS response time when responding with NDP ACK or NDP 
			Modified ACK. 
			<legal all>
*/

#define PDG_RESPONSE_MORE_DATA_OFFSET                                               0x0000000000000020
#define PDG_RESPONSE_MORE_DATA_LSB                                                  25
#define PDG_RESPONSE_MORE_DATA_MSB                                                  25
#define PDG_RESPONSE_MORE_DATA_MASK                                                 0x0000000002000000


/* Description		DURATION_INDICATION

			This setting is used for 
			NDP ACK response frames 
			NDP Modified ACK response frames 
			The value of this field comes from a register programming. 
			The register resides in TxPCU and is programmed by SW within
			 SIFS response time when responding with NDP ACK or NDP 
			Modified ACK.
			<legal all>
*/

#define PDG_RESPONSE_DURATION_INDICATION_OFFSET                                     0x0000000000000020
#define PDG_RESPONSE_DURATION_INDICATION_LSB                                        26
#define PDG_RESPONSE_DURATION_INDICATION_MSB                                        26
#define PDG_RESPONSE_DURATION_INDICATION_MASK                                       0x0000000004000000


/* Description		RELAYED_FRAME

			This setting is used to fill the field in the SIG preamble
			 for
			NDP ACK response frame
			This feature is not supported and TxPCU should program this
			 field to Zero. PDG will ignore this field. 
			<legal all>
*/

#define PDG_RESPONSE_RELAYED_FRAME_OFFSET                                           0x0000000000000020
#define PDG_RESPONSE_RELAYED_FRAME_LSB                                              27
#define PDG_RESPONSE_RELAYED_FRAME_MSB                                              27
#define PDG_RESPONSE_RELAYED_FRAME_MASK                                             0x0000000008000000


/* Description		ADDRESS_INDICATOR

			This bit is used to fill the address_indicator field in 
			the SIG preamble of NDP CTS response frame. 
			
			This feature is not supported and TxPCU should program this
			 field to Zero. PDG will use this field to populate the 
			NDP response frame
			<legal all>
*/

#define PDG_RESPONSE_ADDRESS_INDICATOR_OFFSET                                       0x0000000000000020
#define PDG_RESPONSE_ADDRESS_INDICATOR_LSB                                          28
#define PDG_RESPONSE_ADDRESS_INDICATOR_MSB                                          28
#define PDG_RESPONSE_ADDRESS_INDICATOR_MASK                                         0x0000000010000000


/* Description		BANDWIDTH

			Packet bandwidth:
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PDG_RESPONSE_BANDWIDTH_OFFSET                                               0x0000000000000020
#define PDG_RESPONSE_BANDWIDTH_LSB                                                  29
#define PDG_RESPONSE_BANDWIDTH_MSB                                                  31
#define PDG_RESPONSE_BANDWIDTH_MASK                                                 0x00000000e0000000


/* Description		ACK_ID

			ACK_ID in NDP_ACK frames, NDP Modified ACK frames 
			
			For BW > 1MHz 
			      [15:0] = ack_id 
			
			For BW = 1MHz 
			
			     [8:0] = ack_id 
			     [15:9] = Reserved 
			For NDP BA
			If BW=1MHz
			     [1:0] = Block ACK ID
			     [15:2] = Reserved 
			
			If BW>1MHz 
			     [5:0] = Block ACK ID
			     [15:2] = Reserved
			<legal all>
*/

#define PDG_RESPONSE_ACK_ID_OFFSET                                                  0x0000000000000020
#define PDG_RESPONSE_ACK_ID_LSB                                                     32
#define PDG_RESPONSE_ACK_ID_MSB                                                     47
#define PDG_RESPONSE_ACK_ID_MASK                                                    0x0000ffff00000000


/* Description		BLOCK_ACK_BITMAP

			Block Ack bitmap field for generating the NDP BA frames 
			in 1MHz and >= 2MHz 
			<legal all>
*/

#define PDG_RESPONSE_BLOCK_ACK_BITMAP_OFFSET                                        0x0000000000000020
#define PDG_RESPONSE_BLOCK_ACK_BITMAP_LSB                                           48
#define PDG_RESPONSE_BLOCK_ACK_BITMAP_MSB                                           63
#define PDG_RESPONSE_BLOCK_ACK_BITMAP_MASK                                          0xffff000000000000


/* Description		RESPONSE_FRAME_TYPE

			Coex related field
			<enum 0 Non_11ah_ACK >  
			<enum 1 Non_11ah_BA >  also used for M-BA
			<enum 2 Non_11ah_CTS > 
			<enum 3 AH_NDP_CTS> 
			<enum 4 AH_NDP_ACK>
			<enum 5 AH_NDP_BA>
			<enum 6 AH_NDP_MOD_ACK>
			<enum 7 AH_Normal_ACK>
			<enum 8 AH_Normal_BA>
			<enum 9  RTT_ACK>
			<enum 10 CBF_RESPONSE>
			<enum 11 MBA> This can be a multi STA BA or multi TID BA
			
			<enum 12 Ranging_NDP>
			<enum 13 LMR_RESPONSE> Ranging NDP response followed by 
			LMR response for Rx ranging NDPA followed by NDP
			
			<legal 0-13>
*/

#define PDG_RESPONSE_RESPONSE_FRAME_TYPE_OFFSET                                     0x0000000000000028
#define PDG_RESPONSE_RESPONSE_FRAME_TYPE_LSB                                        0
#define PDG_RESPONSE_RESPONSE_FRAME_TYPE_MSB                                        3
#define PDG_RESPONSE_RESPONSE_FRAME_TYPE_MASK                                       0x000000000000000f


/* Description		ACK_ID_EXT

			This is populated by TxPCU from the RX_RESPONSE_REQUIRED_INFO.ack_id_ext. 
			
*/

#define PDG_RESPONSE_ACK_ID_EXT_OFFSET                                              0x0000000000000028
#define PDG_RESPONSE_ACK_ID_EXT_LSB                                                 4
#define PDG_RESPONSE_ACK_ID_EXT_MSB                                                 13
#define PDG_RESPONSE_ACK_ID_EXT_MASK                                                0x0000000000003ff0


/* Description		FTM_EN

			This field is set to 1 if the response packet is either 
			an FTM_1 or an FTM_2 packet or an HE-Ranging NDP (11az).
			
			
			0: non-FTM frame
			1: FTM or HE-Randing NDP Frame
			<legal all>
*/

#define PDG_RESPONSE_FTM_EN_OFFSET                                                  0x0000000000000028
#define PDG_RESPONSE_FTM_EN_LSB                                                     14
#define PDG_RESPONSE_FTM_EN_MSB                                                     14
#define PDG_RESPONSE_FTM_EN_MASK                                                    0x0000000000004000


/* Description		GROUP_ID

			Specifies the Group ID of response frames transmitted at
			 VHT rates for MU transmissions. This filed applies to both
			 non-11ah and 11ah modes.
*/

#define PDG_RESPONSE_GROUP_ID_OFFSET                                                0x0000000000000028
#define PDG_RESPONSE_GROUP_ID_LSB                                                   15
#define PDG_RESPONSE_GROUP_ID_MSB                                                   20
#define PDG_RESPONSE_GROUP_ID_MASK                                                  0x00000000001f8000


/* Description		STA_PARTIAL_AID

			In 11AH mode of Operation:
			
			This field is used to populate the ID field in the SIG PPDUs
			 of BW>1MHz and non-NDP frames.  For example, the use case
			 would be in a Speed Frame Exchange, we may be generating
			 the SIG PPDU in response and this field is needed to populate
			 the ID field in the SIGA preamble . This value is based
			 on the Table provided by 9.17b section of the Draft P802.11ah_D1.1
			 Specification
			
			In 11AH mode of Operation: 
			
			This field is also used to populate the field of RA/Parial_BSSID
			 in the NDP CTS response frames In non-11AH mode:
			
			In non-11AH mode of Operation:
			
			Specifies the partial AID of response frames transmitted
			 at VHT rates.
			<legal all>
*/

#define PDG_RESPONSE_STA_PARTIAL_AID_OFFSET                                         0x0000000000000028
#define PDG_RESPONSE_STA_PARTIAL_AID_LSB                                            21
#define PDG_RESPONSE_STA_PARTIAL_AID_MSB                                            31
#define PDG_RESPONSE_STA_PARTIAL_AID_MASK                                           0x00000000ffe00000


/* Description		NDP_BA_START_SEQ_CTRL

			Starting Sequence Control  - Sequence number of the first
			 MPDU in the frame soliciting the Block Ack. 
*/

#define PDG_RESPONSE_NDP_BA_START_SEQ_CTRL_OFFSET                                   0x0000000000000028
#define PDG_RESPONSE_NDP_BA_START_SEQ_CTRL_LSB                                      32
#define PDG_RESPONSE_NDP_BA_START_SEQ_CTRL_MSB                                      43
#define PDG_RESPONSE_NDP_BA_START_SEQ_CTRL_MASK                                     0x00000fff00000000


/* Description		ACTIVE_CHANNEL

			This field indicates the active frequency band when the 
			packet bandwidth is less than the channel bandwidth. For
			 non 11ax packets this is same as the primary channel
			<legal all>
*/

#define PDG_RESPONSE_ACTIVE_CHANNEL_OFFSET                                          0x0000000000000028
#define PDG_RESPONSE_ACTIVE_CHANNEL_LSB                                             44
#define PDG_RESPONSE_ACTIVE_CHANNEL_MSB                                             46
#define PDG_RESPONSE_ACTIVE_CHANNEL_MASK                                            0x0000700000000000


/* Description		TXOP_DURATION_ALL_ONES

			When set, either the TXOP_DURATION of the received frame
			 was set to all 1s or there is a BSS color collision. The
			 TXOP_DURATION of the transmit response should be forced
			 to all 1s.
			
			<legal all>
*/

#define PDG_RESPONSE_TXOP_DURATION_ALL_ONES_OFFSET                                  0x0000000000000028
#define PDG_RESPONSE_TXOP_DURATION_ALL_ONES_LSB                                     47
#define PDG_RESPONSE_TXOP_DURATION_ALL_ONES_MSB                                     47
#define PDG_RESPONSE_TXOP_DURATION_ALL_ONES_MASK                                    0x0000800000000000


/* Description		FRAME_LENGTH

			The response frame length in bytes
			(This includes the FCS field)
			<legal all>
*/

#define PDG_RESPONSE_FRAME_LENGTH_OFFSET                                            0x0000000000000028
#define PDG_RESPONSE_FRAME_LENGTH_LSB                                               48
#define PDG_RESPONSE_FRAME_LENGTH_MSB                                               63
#define PDG_RESPONSE_FRAME_LENGTH_MASK                                              0xffff000000000000



#endif   // PDG_RESPONSE
