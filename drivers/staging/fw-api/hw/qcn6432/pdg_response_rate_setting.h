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

#ifndef _PDG_RESPONSE_RATE_SETTING_H_
#define _PDG_RESPONSE_RATE_SETTING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_PDG_RESPONSE_RATE_SETTING 7


struct pdg_response_rate_setting {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             :  1, // [0:0]
                      tx_antenna_sector_ctrl                                  : 24, // [24:1]
                      pkt_type                                                :  4, // [28:25]
                      smoothing                                               :  1, // [29:29]
                      ldpc                                                    :  1, // [30:30]
                      stbc                                                    :  1; // [31:31]
             uint32_t alt_tx_pwr                                              :  8, // [7:0]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_nss                                                 :  3, // [18:16]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_bw                                                  :  3, // [29:27]
                      stf_ltf_3db_boost                                       :  1, // [30:30]
                      force_extra_symbol                                      :  1; // [31:31]
             uint32_t alt_rate_mcs                                            :  4, // [3:0]
                      nss                                                     :  3, // [6:4]
                      dpd_enable                                              :  1, // [7:7]
                      tx_pwr                                                  :  8, // [15:8]
                      min_tx_pwr                                              :  8, // [23:16]
                      tx_chain_mask                                           :  8; // [31:24]
             uint32_t reserved_3a                                             :  8, // [7:0]
                      sgi                                                     :  2, // [9:8]
                      rate_mcs                                                :  4, // [13:10]
                      reserved_3b                                             :  2, // [15:14]
                      tx_pwr_1                                                :  8, // [23:16]
                      alt_tx_pwr_1                                            :  8; // [31:24]
             uint32_t aggregation                                             :  1, // [0:0]
                      dot11ax_bss_color_id                                    :  6, // [6:1]
                      dot11ax_spatial_reuse                                   :  4, // [10:7]
                      dot11ax_cp_ltf_size                                     :  2, // [12:11]
                      dot11ax_dcm                                             :  1, // [13:13]
                      dot11ax_doppler_indication                              :  1, // [14:14]
                      dot11ax_su_extended                                     :  1, // [15:15]
                      dot11ax_min_packet_extension                            :  2, // [17:16]
                      dot11ax_pe_nss                                          :  3, // [20:18]
                      dot11ax_pe_content                                      :  1, // [21:21]
                      dot11ax_pe_ltf_size                                     :  2, // [23:22]
                      dot11ax_chain_csd_en                                    :  1, // [24:24]
                      dot11ax_pe_chain_csd_en                                 :  1, // [25:25]
                      dot11ax_dl_ul_flag                                      :  1, // [26:26]
                      reserved_4a                                             :  5; // [31:27]
             uint32_t dot11ax_ext_ru_start_index                              :  4, // [3:0]
                      dot11ax_ext_ru_size                                     :  4, // [7:4]
                      eht_duplicate_mode                                      :  2, // [9:8]
                      he_sigb_dcm                                             :  1, // [10:10]
                      he_sigb_0_mcs                                           :  3, // [13:11]
                      num_he_sigb_sym                                         :  5, // [18:14]
                      required_response_time_source                           :  1, // [19:19]
                      reserved_5a                                             :  6, // [25:20]
                      u_sig_puncture_pattern_encoding                         :  6; // [31:26]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t required_response_time                                  : 12, // [27:16]
                      dot11be_params_placeholder                              :  4; // [31:28]
#else
             uint32_t stbc                                                    :  1, // [31:31]
                      ldpc                                                    :  1, // [30:30]
                      smoothing                                               :  1, // [29:29]
                      pkt_type                                                :  4, // [28:25]
                      tx_antenna_sector_ctrl                                  : 24, // [24:1]
                      reserved_0a                                             :  1; // [0:0]
             uint32_t force_extra_symbol                                      :  1, // [31:31]
                      stf_ltf_3db_boost                                       :  1, // [30:30]
                      alt_bw                                                  :  3, // [29:27]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_nss                                                 :  3, // [18:16]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_tx_pwr                                              :  8; // [7:0]
             uint32_t tx_chain_mask                                           :  8, // [31:24]
                      min_tx_pwr                                              :  8, // [23:16]
                      tx_pwr                                                  :  8, // [15:8]
                      dpd_enable                                              :  1, // [7:7]
                      nss                                                     :  3, // [6:4]
                      alt_rate_mcs                                            :  4; // [3:0]
             uint32_t alt_tx_pwr_1                                            :  8, // [31:24]
                      tx_pwr_1                                                :  8, // [23:16]
                      reserved_3b                                             :  2, // [15:14]
                      rate_mcs                                                :  4, // [13:10]
                      sgi                                                     :  2, // [9:8]
                      reserved_3a                                             :  8; // [7:0]
             uint32_t reserved_4a                                             :  5, // [31:27]
                      dot11ax_dl_ul_flag                                      :  1, // [26:26]
                      dot11ax_pe_chain_csd_en                                 :  1, // [25:25]
                      dot11ax_chain_csd_en                                    :  1, // [24:24]
                      dot11ax_pe_ltf_size                                     :  2, // [23:22]
                      dot11ax_pe_content                                      :  1, // [21:21]
                      dot11ax_pe_nss                                          :  3, // [20:18]
                      dot11ax_min_packet_extension                            :  2, // [17:16]
                      dot11ax_su_extended                                     :  1, // [15:15]
                      dot11ax_doppler_indication                              :  1, // [14:14]
                      dot11ax_dcm                                             :  1, // [13:13]
                      dot11ax_cp_ltf_size                                     :  2, // [12:11]
                      dot11ax_spatial_reuse                                   :  4, // [10:7]
                      dot11ax_bss_color_id                                    :  6, // [6:1]
                      aggregation                                             :  1; // [0:0]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [31:26]
                      reserved_5a                                             :  6, // [25:20]
                      required_response_time_source                           :  1, // [19:19]
                      num_he_sigb_sym                                         :  5, // [18:14]
                      he_sigb_0_mcs                                           :  3, // [13:11]
                      he_sigb_dcm                                             :  1, // [10:10]
                      eht_duplicate_mode                                      :  2, // [9:8]
                      dot11ax_ext_ru_size                                     :  4, // [7:4]
                      dot11ax_ext_ru_start_index                              :  4; // [3:0]
             uint32_t dot11be_params_placeholder                              :  4, // [31:28]
                      required_response_time                                  : 12; // [27:16]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
#endif
};


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_OFFSET                                0x00000000
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_MSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_MASK                                  0x00000001


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_OFFSET                     0x00000000
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_LSB                        1
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_MSB                        24
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_MASK                       0x01fffffe


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

#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_OFFSET                                   0x00000000
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_LSB                                      25
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_MSB                                      28
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_MASK                                     0x1e000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_OFFSET                                  0x00000000
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_LSB                                     29
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_MSB                                     29
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_MASK                                    0x20000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PDG_RESPONSE_RATE_SETTING_LDPC_OFFSET                                       0x00000000
#define PDG_RESPONSE_RATE_SETTING_LDPC_LSB                                          30
#define PDG_RESPONSE_RATE_SETTING_LDPC_MSB                                          30
#define PDG_RESPONSE_RATE_SETTING_LDPC_MASK                                         0x40000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PDG_RESPONSE_RATE_SETTING_STBC_OFFSET                                       0x00000000
#define PDG_RESPONSE_RATE_SETTING_STBC_LSB                                          31
#define PDG_RESPONSE_RATE_SETTING_STBC_MSB                                          31
#define PDG_RESPONSE_RATE_SETTING_STBC_MASK                                         0x80000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_OFFSET                                 0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_LSB                                    0
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_MSB                                    7
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_MASK                                   0x000000ff


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_OFFSET                             0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_LSB                                8
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_MSB                                15
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_MASK                               0x0000ff00


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

#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_OFFSET                                    0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_LSB                                       16
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_MSB                                       18
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_MASK                                      0x00070000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_OFFSET                          0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_LSB                             19
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_MSB                             26
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_MASK                            0x07f80000


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

#define PDG_RESPONSE_RATE_SETTING_ALT_BW_OFFSET                                     0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_LSB                                        27
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_MSB                                        29
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_MASK                                       0x38000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_OFFSET                          0x00000004
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_LSB                             30
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_MSB                             30
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_MASK                            0x40000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_OFFSET                         0x00000004
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_LSB                            31
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_MSB                            31
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_MASK                           0x80000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_OFFSET                               0x00000008
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_LSB                                  0
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_MSB                                  3
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_MASK                                 0x0000000f


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

#define PDG_RESPONSE_RATE_SETTING_NSS_OFFSET                                        0x00000008
#define PDG_RESPONSE_RATE_SETTING_NSS_LSB                                           4
#define PDG_RESPONSE_RATE_SETTING_NSS_MSB                                           6
#define PDG_RESPONSE_RATE_SETTING_NSS_MASK                                          0x00000070


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

#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_OFFSET                                 0x00000008
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_LSB                                    7
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_MSB                                    7
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_MASK                                   0x00000080


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_TX_PWR_OFFSET                                     0x00000008
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_LSB                                        8
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_MSB                                        15
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_MASK                                       0x0000ff00


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_OFFSET                                 0x00000008
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_LSB                                    16
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_MSB                                    23
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_MASK                                   0x00ff0000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_OFFSET                              0x00000008
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_LSB                                 24
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_MSB                                 31
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_MASK                                0xff000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_OFFSET                                0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_MSB                                   7
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_MASK                                  0x000000ff


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

#define PDG_RESPONSE_RATE_SETTING_SGI_OFFSET                                        0x0000000c
#define PDG_RESPONSE_RATE_SETTING_SGI_LSB                                           8
#define PDG_RESPONSE_RATE_SETTING_SGI_MSB                                           9
#define PDG_RESPONSE_RATE_SETTING_SGI_MASK                                          0x00000300


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_OFFSET                                   0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_LSB                                      10
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_MSB                                      13
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_MASK                                     0x00003c00


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_OFFSET                                0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_LSB                                   14
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_MSB                                   15
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_MASK                                  0x0000c000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_OFFSET                                   0x0000000c
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_LSB                                      16
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_MSB                                      23
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_MASK                                     0x00ff0000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_OFFSET                               0x0000000c
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_LSB                                  24
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_MSB                                  31
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_MASK                                 0xff000000


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

#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_MSB                                   0
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_MASK                                  0x00000001


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_OFFSET                       0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_LSB                          1
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_MSB                          6
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_MASK                         0x0000007e


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_OFFSET                      0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_LSB                         7
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_MSB                         10
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_MASK                        0x00000780


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

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_LSB                           11
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_MSB                           12
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_MASK                          0x00001800


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_LSB                                   13
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_MSB                                   13
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_MASK                                  0x00002000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_OFFSET                 0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_LSB                    14
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_MSB                    14
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_MASK                   0x00004000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_LSB                           15
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_MSB                           15
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_MASK                          0x00008000


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

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_OFFSET               0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_LSB                  16
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_MSB                  17
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_MASK                 0x00030000


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

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_OFFSET                             0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_LSB                                18
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_MSB                                20
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_MASK                               0x001c0000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_OFFSET                         0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_LSB                            21
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_MSB                            21
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_MASK                           0x00200000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_LSB                           22
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_MSB                           23
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_MASK                          0x00c00000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_OFFSET                       0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_LSB                          24
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_MSB                          24
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_MASK                         0x01000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_OFFSET                    0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_LSB                       25
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_MSB                       25
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_MASK                      0x02000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_OFFSET                         0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_LSB                            26
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_MSB                            26
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_MASK                           0x04000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_LSB                                   27
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_MSB                                   31
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_MASK                                  0xf8000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_OFFSET                 0x00000014
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_LSB                    0
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_MSB                    3
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_MASK                   0x0000000f


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

#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_OFFSET                        0x00000014
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_LSB                           4
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_MSB                           7
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_MASK                          0x000000f0


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_OFFSET                         0x00000014
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_LSB                            8
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_MSB                            9
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_MASK                           0x00000300


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_OFFSET                                0x00000014
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_LSB                                   10
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_MSB                                   10
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_MASK                                  0x00000400


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_OFFSET                              0x00000014
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_LSB                                 11
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_MSB                                 13
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_MASK                                0x00003800


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_OFFSET                            0x00000014
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_LSB                               14
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_MSB                               18
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_MASK                              0x0007c000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET              0x00000014
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_LSB                 19
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_MSB                 19
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_MASK                0x00080000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_OFFSET                                0x00000014
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_LSB                                   20
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_MSB                                   25
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_MASK                                  0x03f00000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET            0x00000014
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB               26
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB               31
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK              0xfc000000


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

#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET      0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB         0
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB         9
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK        0x000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET   0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB      10
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB      10
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK     0x00000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB   11
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB   11
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK  0x00000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB   12
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB   12
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK  0x00001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET          0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB             13
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB             15
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK            0x0000e000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_OFFSET                     0x00000018
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_LSB                        16
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_MSB                        27
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_MASK                       0x0fff0000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_OFFSET                 0x00000018
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_LSB                    28
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_MSB                    31
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_MASK                   0xf0000000



#endif   // PDG_RESPONSE_RATE_SETTING
