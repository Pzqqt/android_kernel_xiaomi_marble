
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

 
 
 
 
 
 
 


#ifndef _OFDMA_TRIGGER_DETAILS_H_
#define _OFDMA_TRIGGER_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_OFDMA_TRIGGER_DETAILS 22

#define NUM_OF_QWORDS_OFDMA_TRIGGER_DETAILS 11


struct ofdma_trigger_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ax_trigger_source                                       :  1, // [0:0]
                      rx_trigger_frame_user_source                            :  2, // [2:1]
                      received_bandwidth                                      :  3, // [5:3]
                      txop_duration_all_ones                                  :  1, // [6:6]
                      eht_trigger_response                                    :  1, // [7:7]
                      pre_rssi_comb                                           :  8, // [15:8]
                      rssi_comb                                               :  8, // [23:16]
                      rxpcu_pcie_l0_req_duration                              :  8; // [31:24]
             uint32_t he_trigger_ul_ppdu_length                               :  5, // [4:0]
                      he_trigger_ru_allocation                                :  8, // [12:5]
                      he_trigger_dl_tx_power                                  :  5, // [17:13]
                      he_trigger_ul_target_rssi                               :  5, // [22:18]
                      he_trigger_ul_mcs                                       :  2, // [24:23]
                      he_trigger_reserved                                     :  1, // [25:25]
                      bss_color                                               :  6; // [31:26]
             uint32_t trigger_type                                            :  4, // [3:0]
                      lsig_response_length                                    : 12, // [15:4]
                      cascade_indication                                      :  1, // [16:16]
                      carrier_sense                                           :  1, // [17:17]
                      bandwidth                                               :  2, // [19:18]
                      cp_ltf_size                                             :  2, // [21:20]
                      mu_mimo_ltf_mode                                        :  1, // [22:22]
                      number_of_ltfs                                          :  3, // [25:23]
                      stbc                                                    :  1, // [26:26]
                      ldpc_extra_symbol                                       :  1, // [27:27]
                      ap_tx_power_lsb_part                                    :  4; // [31:28]
             uint32_t ap_tx_power_msb_part                                    :  2, // [1:0]
                      packet_extension_a_factor                               :  2, // [3:2]
                      packet_extension_pe_disambiguity                        :  1, // [4:4]
                      spatial_reuse                                           : 16, // [20:5]
                      doppler                                                 :  1, // [21:21]
                      he_siga_reserved                                        :  9, // [30:22]
                      reserved_3b                                             :  1; // [31:31]
             uint32_t aid12                                                   : 12, // [11:0]
                      ru_allocation                                           :  9, // [20:12]
                      mcs                                                     :  4, // [24:21]
                      dcm                                                     :  1, // [25:25]
                      start_spatial_stream                                    :  3, // [28:26]
                      number_of_spatial_stream                                :  3; // [31:29]
             uint32_t target_rssi                                             :  7, // [6:0]
                      coding_type                                             :  1, // [7:7]
                      mpdu_mu_spacing_factor                                  :  2, // [9:8]
                      tid_aggregation_limit                                   :  3, // [12:10]
                      reserved_5b                                             :  1, // [13:13]
                      prefered_ac                                             :  2, // [15:14]
                      bar_control_ack_policy                                  :  1, // [16:16]
                      bar_control_multi_tid                                   :  1, // [17:17]
                      bar_control_compressed_bitmap                           :  1, // [18:18]
                      bar_control_reserved                                    :  9, // [27:19]
                      bar_control_tid_info                                    :  4; // [31:28]
             uint32_t nr0_per_tid_info_reserved                               : 12, // [11:0]
                      nr0_per_tid_info_tid_value                              :  4, // [15:12]
                      nr0_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr0_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr1_per_tid_info_reserved                               : 12, // [11:0]
                      nr1_per_tid_info_tid_value                              :  4, // [15:12]
                      nr1_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr1_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr2_per_tid_info_reserved                               : 12, // [11:0]
                      nr2_per_tid_info_tid_value                              :  4, // [15:12]
                      nr2_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr2_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr3_per_tid_info_reserved                               : 12, // [11:0]
                      nr3_per_tid_info_tid_value                              :  4, // [15:12]
                      nr3_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr3_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr4_per_tid_info_reserved                               : 12, // [11:0]
                      nr4_per_tid_info_tid_value                              :  4, // [15:12]
                      nr4_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr4_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr5_per_tid_info_reserved                               : 12, // [11:0]
                      nr5_per_tid_info_tid_value                              :  4, // [15:12]
                      nr5_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr5_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr6_per_tid_info_reserved                               : 12, // [11:0]
                      nr6_per_tid_info_tid_value                              :  4, // [15:12]
                      nr6_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr6_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t nr7_per_tid_info_reserved                               : 12, // [11:0]
                      nr7_per_tid_info_tid_value                              :  4, // [15:12]
                      nr7_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr7_start_seq_ctrl_start_seq_number                     : 12; // [31:20]
             uint32_t fb_segment_retransmission_bitmap                        :  8, // [7:0]
                      reserved_14a                                            :  2, // [9:8]
                      u_sig_puncture_pattern_encoding                         :  6, // [15:10]
                      dot11be_puncture_bitmap                                 : 16; // [31:16]
             uint32_t rx_chain_mask                                           :  8, // [7:0]
                      rx_duration_field                                       : 16, // [23:8]
                      scrambler_seed                                          :  7, // [30:24]
                      rx_chain_mask_type                                      :  1; // [31:31]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t normalized_pre_rssi_comb                                :  8, // [23:16]
                      normalized_rssi_comb                                    :  8; // [31:24]
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      response_tx_duration                                    : 16; // [31:16]
             uint32_t ranging_trigger_subtype                                 :  4, // [3:0]
                      tbr_trigger_common_info_79_68                           : 12, // [15:4]
                      tbr_trigger_sound_reserved_20_12                        :  9, // [24:16]
                      i2r_rep                                                 :  3, // [27:25]
                      tbr_trigger_sound_reserved_25_24                        :  2, // [29:28]
                      reserved_18a                                            :  1, // [30:30]
                      qos_null_only_response_tx                               :  1; // [31:31]
             uint32_t tbr_trigger_sound_sac                                   : 16, // [15:0]
                      reserved_19a                                            :  8, // [23:16]
                      u_sig_reserved2                                         :  5, // [28:24]
                      reserved_19b                                            :  3; // [31:29]
             uint32_t eht_special_aid12                                       : 12, // [11:0]
                      phy_version                                             :  3, // [14:12]
                      bandwidth_ext                                           :  2, // [16:15]
                      eht_spatial_reuse                                       :  8, // [24:17]
                      u_sig_reserved1                                         :  7; // [31:25]
             uint32_t eht_trigger_special_user_info_71_40                     : 32; // [31:0]
#else
             uint32_t rxpcu_pcie_l0_req_duration                              :  8, // [31:24]
                      rssi_comb                                               :  8, // [23:16]
                      pre_rssi_comb                                           :  8, // [15:8]
                      eht_trigger_response                                    :  1, // [7:7]
                      txop_duration_all_ones                                  :  1, // [6:6]
                      received_bandwidth                                      :  3, // [5:3]
                      rx_trigger_frame_user_source                            :  2, // [2:1]
                      ax_trigger_source                                       :  1; // [0:0]
             uint32_t bss_color                                               :  6, // [31:26]
                      he_trigger_reserved                                     :  1, // [25:25]
                      he_trigger_ul_mcs                                       :  2, // [24:23]
                      he_trigger_ul_target_rssi                               :  5, // [22:18]
                      he_trigger_dl_tx_power                                  :  5, // [17:13]
                      he_trigger_ru_allocation                                :  8, // [12:5]
                      he_trigger_ul_ppdu_length                               :  5; // [4:0]
             uint32_t ap_tx_power_lsb_part                                    :  4, // [31:28]
                      ldpc_extra_symbol                                       :  1, // [27:27]
                      stbc                                                    :  1, // [26:26]
                      number_of_ltfs                                          :  3, // [25:23]
                      mu_mimo_ltf_mode                                        :  1, // [22:22]
                      cp_ltf_size                                             :  2, // [21:20]
                      bandwidth                                               :  2, // [19:18]
                      carrier_sense                                           :  1, // [17:17]
                      cascade_indication                                      :  1, // [16:16]
                      lsig_response_length                                    : 12, // [15:4]
                      trigger_type                                            :  4; // [3:0]
             uint32_t reserved_3b                                             :  1, // [31:31]
                      he_siga_reserved                                        :  9, // [30:22]
                      doppler                                                 :  1, // [21:21]
                      spatial_reuse                                           : 16, // [20:5]
                      packet_extension_pe_disambiguity                        :  1, // [4:4]
                      packet_extension_a_factor                               :  2, // [3:2]
                      ap_tx_power_msb_part                                    :  2; // [1:0]
             uint32_t number_of_spatial_stream                                :  3, // [31:29]
                      start_spatial_stream                                    :  3, // [28:26]
                      dcm                                                     :  1, // [25:25]
                      mcs                                                     :  4, // [24:21]
                      ru_allocation                                           :  9, // [20:12]
                      aid12                                                   : 12; // [11:0]
             uint32_t bar_control_tid_info                                    :  4, // [31:28]
                      bar_control_reserved                                    :  9, // [27:19]
                      bar_control_compressed_bitmap                           :  1, // [18:18]
                      bar_control_multi_tid                                   :  1, // [17:17]
                      bar_control_ack_policy                                  :  1, // [16:16]
                      prefered_ac                                             :  2, // [15:14]
                      reserved_5b                                             :  1, // [13:13]
                      tid_aggregation_limit                                   :  3, // [12:10]
                      mpdu_mu_spacing_factor                                  :  2, // [9:8]
                      coding_type                                             :  1, // [7:7]
                      target_rssi                                             :  7; // [6:0]
             uint32_t nr0_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr0_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr0_per_tid_info_tid_value                              :  4, // [15:12]
                      nr0_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr1_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr1_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr1_per_tid_info_tid_value                              :  4, // [15:12]
                      nr1_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr2_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr2_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr2_per_tid_info_tid_value                              :  4, // [15:12]
                      nr2_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr3_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr3_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr3_per_tid_info_tid_value                              :  4, // [15:12]
                      nr3_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr4_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr4_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr4_per_tid_info_tid_value                              :  4, // [15:12]
                      nr4_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr5_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr5_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr5_per_tid_info_tid_value                              :  4, // [15:12]
                      nr5_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr6_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr6_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr6_per_tid_info_tid_value                              :  4, // [15:12]
                      nr6_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t nr7_start_seq_ctrl_start_seq_number                     : 12, // [31:20]
                      nr7_start_seq_ctrl_frag_number                          :  4, // [19:16]
                      nr7_per_tid_info_tid_value                              :  4, // [15:12]
                      nr7_per_tid_info_reserved                               : 12; // [11:0]
             uint32_t dot11be_puncture_bitmap                                 : 16, // [31:16]
                      u_sig_puncture_pattern_encoding                         :  6, // [15:10]
                      reserved_14a                                            :  2, // [9:8]
                      fb_segment_retransmission_bitmap                        :  8; // [7:0]
             uint32_t rx_chain_mask_type                                      :  1, // [31:31]
                      scrambler_seed                                          :  7, // [30:24]
                      rx_duration_field                                       : 16, // [23:8]
                      rx_chain_mask                                           :  8; // [7:0]
             uint32_t normalized_rssi_comb                                    :  8, // [31:24]
                      normalized_pre_rssi_comb                                :  8; // [23:16]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint32_t response_tx_duration                                    : 16, // [31:16]
                      sw_peer_id                                              : 16; // [15:0]
             uint32_t qos_null_only_response_tx                               :  1, // [31:31]
                      reserved_18a                                            :  1, // [30:30]
                      tbr_trigger_sound_reserved_25_24                        :  2, // [29:28]
                      i2r_rep                                                 :  3, // [27:25]
                      tbr_trigger_sound_reserved_20_12                        :  9, // [24:16]
                      tbr_trigger_common_info_79_68                           : 12, // [15:4]
                      ranging_trigger_subtype                                 :  4; // [3:0]
             uint32_t reserved_19b                                            :  3, // [31:29]
                      u_sig_reserved2                                         :  5, // [28:24]
                      reserved_19a                                            :  8, // [23:16]
                      tbr_trigger_sound_sac                                   : 16; // [15:0]
             uint32_t u_sig_reserved1                                         :  7, // [31:25]
                      eht_spatial_reuse                                       :  8, // [24:17]
                      bandwidth_ext                                           :  2, // [16:15]
                      phy_version                                             :  3, // [14:12]
                      eht_special_aid12                                       : 12; // [11:0]
             uint32_t eht_trigger_special_user_info_71_40                     : 32; // [31:0]
#endif
};


/* Description		AX_TRIGGER_SOURCE

			<enum 0 11ax_trigger_frame>
			<enum 1 he_control_based_trigger>
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_OFFSET                              0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_LSB                                 0
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MSB                                 0
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MASK                                0x0000000000000001


/* Description		RX_TRIGGER_FRAME_USER_SOURCE

			Field not really needed by PDG, but is there for debugging
			 purposes to be put in event.
			
			<enum 0 dot11ax_direct_trigger_frame>
			<enum 1 dot11ax_wildcard_trigger_frame> wildcard trigger
			 for associated STAs
			<enum 2 dot11ax_usassoc_wildcard_trigger_frame> wildcard
			 trigger for unassociated STAs
			
			<legal 0-2>
*/

#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_OFFSET                   0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_LSB                      1
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_MSB                      2
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_MASK                     0x0000000000000006


/* Description		RECEIVED_BANDWIDTH

			Received Packet bandwidth of the trigger frame.
			
			Note that this is not the BW indicated within the trigger
			 frame itself.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_OFFSET                             0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_LSB                                3
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_MSB                                5
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_MASK                               0x0000000000000038


/* Description		TXOP_DURATION_ALL_ONES

			When set, TXOP_DURATION of the received frame was set to
			 all 1s.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_OFFSET                         0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_LSB                            6
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_MSB                            6
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_MASK                           0x0000000000000040


/* Description		EHT_TRIGGER_RESPONSE

			0: Trigger expects an HE TB PPDU Tx response.
			1: Trigger expects an EHT TB PPDU Tx response.
			<legal 0>
*/

#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_OFFSET                           0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_LSB                              7
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_MSB                              7
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_MASK                             0x0000000000000080


/* Description		PRE_RSSI_COMB

			Combined pre_rssi of all chains. Based on primary channel
			 RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_OFFSET                                  0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_LSB                                     8
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_MSB                                     15
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_MASK                                    0x000000000000ff00


/* Description		RSSI_COMB

			Combined rssi of all chains. Based on primary channel RSSI.
			
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_OFFSET                                      0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_LSB                                         16
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_MSB                                         23
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_MASK                                        0x0000000000ff0000


/* Description		RXPCU_PCIE_L0_REQ_DURATION

			RXPCU fills the duration in µs for which it has asserted
			 the 'L0 request' signal to PCIe when it generates this 
			TLV. This may be capped by either the max. PCIe L1SS exit
			 latency (~75 µs) or the max. value possible for this field.
			
			
			This is filled as zero if ILP is unsupported (e.g. in Maple
			 and Spruce) or disabled.
			
			PDG uses this to fill Qos_null_only_response_tx.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_OFFSET                     0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_LSB                        24
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_MSB                        31
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_MASK                       0x00000000ff000000


/* Description		HE_TRIGGER_UL_PPDU_LENGTH

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			length of the HE trigger-based PPDU response.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_OFFSET                      0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_LSB                         32
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_MSB                         36
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_MASK                        0x0000001f00000000


/* Description		HE_TRIGGER_RU_ALLOCATION

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			RU allocation for HE based trigger
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_OFFSET                       0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_LSB                          37
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_MSB                          44
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_MASK                         0x00001fe000000000


/* Description		HE_TRIGGER_DL_TX_POWER

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			Downlink TX power
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_OFFSET                         0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_LSB                            45
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_MSB                            49
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_MASK                           0x0003e00000000000


/* Description		HE_TRIGGER_UL_TARGET_RSSI

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			Ul target RSSI
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_OFFSET                      0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_LSB                         50
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_MSB                         54
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_MASK                        0x007c000000000000


/* Description		HE_TRIGGER_UL_MCS

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			UL MCS
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_OFFSET                              0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_LSB                                 55
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_MSB                                 56
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_MASK                                0x0180000000000000


/* Description		HE_TRIGGER_RESERVED

			Field only valid when ax_trigger_source = he_control_based_trigger
			
			
			Field extracted from the HE control field.
			Reserved field
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_OFFSET                            0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_LSB                               57
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_MSB                               57
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_MASK                              0x0200000000000000


/* Description		BSS_COLOR

			The BSS color of the AP
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_OFFSET                                      0x0000000000000000
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_LSB                                         58
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_MSB                                         63
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_MASK                                        0xfc00000000000000


/* Description		TRIGGER_TYPE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Indicates what kind of response is required to the received
			 OFDMA trigger...
			
			Field not really needed by PDG, but is there for debugging
			 purposes to be put in event.
			
			<enum 0 ax_trigger_basic> TXPCU sends back whatever SW has
			 programmed...for the basic response..
			<enum 1 ax_trigger_brpoll>  TXPCU is only allowed to send
			 CBF frame(s) back
			<enum 2 ax_trigger_mu_bar> TXPCU shall first send BA info, 
			and optionally followed with data. No info from SCH is expected
			
			<enum 3 ax_trigger_mu_rts> TXPCU shall only send CTS back. 
			No info from SCH is expected
			<enum 4 ax_trigger_buffer_size> Also known as the BSRP trigger. 
			TXPCU sends back whatever SW has programmed...for the basic
			 response..
			<enum 5 ax_trigger_gcr_mu_bar>
			<enum 6 ax_trigger_BQRP> Bandwidth Query Report Poll
			<enum 7 ax_trigger_NDP_fb_report_poll> NDP feedback report
			 Poll
			<enum 8 ax_tb_ranging_trigger> ranging Trigger Frame of 
			subvariant indicated by Ranging_Trigger_Subtype
			<enum 9 ax_trigger_reserved_9>
			<enum 10 ax_trigger_reserved_10>
			<enum 11 ax_trigger_reserved_11>
			<enum 12 ax_trigger_reserved_12>
			<enum 13 ax_trigger_reserved_13>
			<enum 14 ax_trigger_reserved_14>
			<enum 15 ax_trigger_reserved_15>
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_OFFSET                                   0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_LSB                                      0
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_MSB                                      3
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_MASK                                     0x000000000000000f


/* Description		LSIG_RESPONSE_LENGTH

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Indicates the value of the L-SIG Length field of the HE 
			trigger-based PPDU that is the response to the Trigger frame
			 
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_OFFSET                           0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_LSB                              4
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MSB                              15
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MASK                             0x000000000000fff0


/* Description		CASCADE_INDICATION

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			When set to 1, then a subsequent Trigger frame follows the
			 current Trigger frame.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_OFFSET                             0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_LSB                                16
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_MSB                                16
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_MASK                               0x0000000000010000


/* Description		CARRIER_SENSE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Need to sense the energy before transmit when CS=1 if allocated
			 channel is not available do not transmit . If CS=0 no need
			 to check for idle channel.  For region based restrict ignore
			 this bit and always check channel before transmit.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_OFFSET                                  0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_LSB                                     17
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_MSB                                     17
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_MASK                                    0x0000000000020000


/* Description		BANDWIDTH

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Indicates the bandwidth in the HE-SIG-A/U-SIG of the HE/EHT
			 Trigger based PPDU
			
			Also see field Bandwidth_ext that determines 320 MHz bandwidth
			 for EHT.
			
			<enum 0 HE_SIG_A_BW20> 20 Mhz 
			<enum 1 HE_SIG_A_BW40> 40 Mhz 
			<enum 2 HE_SIG_A_BW80> 80 Mhz 
			<enum 3 HE_SIG_A_BW160> 160 MHz or 80+80 MHz
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_OFFSET                                      0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_LSB                                         18
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_MSB                                         19
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_MASK                                        0x00000000000c0000


/* Description		CP_LTF_SIZE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 Trig_OneX_LTF_1_6CP> 1xLTF + 1.6 us CP 
			<enum 1 Trig_TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP 
			<enum 2 Trig_FourX_LTF_3_2CP> 4x LTF + 3.2 µs CP 
			
			<legal 0-2>
*/

#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_OFFSET                                    0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_LSB                                       20
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_MSB                                       21
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_MASK                                      0x0000000000300000


/* Description		MU_MIMO_LTF_MODE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			MU MIMO LTF mode field indicates the mode in which pilots
			 are allocated
			
			Must be set to 0 for HE-Ranging NDPs (11az) or Short-NDP
			
			
			0: Single-stream pilot
			1: Mask LTF sequence of each spatial stream by a distinct
			 orthogonal code
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_OFFSET                               0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_LSB                                  22
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_MSB                                  22
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_MASK                                 0x0000000000400000


/* Description		NUMBER_OF_LTFS

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			includes the total number of LTFs the STA must include in
			 the response TRIG PPDU
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_OFFSET                                 0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_LSB                                    23
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_MSB                                    25
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_MASK                                   0x0000000003800000


/* Description		STBC

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			indicates whether STBS is used (for all STAs)
			It is set to 1 if STBC encoding is used and set to 0 otherwise.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_STBC_OFFSET                                           0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_STBC_LSB                                              26
#define OFDMA_TRIGGER_DETAILS_STBC_MSB                                              26
#define OFDMA_TRIGGER_DETAILS_STBC_MASK                                             0x0000000004000000


/* Description		LDPC_EXTRA_SYMBOL

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			indicates the status of LDPC Extra Symbol. It is set to 
			1 when LDPC extra symbol is present and set to 0 otherwise
			 
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_OFFSET                              0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_LSB                                 27
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_MSB                                 27
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_MASK                                0x0000000008000000


/* Description		AP_TX_POWER_LSB_PART

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Bits [3:0] of the ap_tx_power
			
			indicates the combined average power per 20 MHz bandwidth
			 of all transmit antennas used to transmit the trigger frame
			 at the HE AP. The resolution for the transmit power reported
			 in the Common Info field is 1dB
			
			Values 0 to 61 maps to -20 dBm to 40 dBm
			Other values are reserved.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_OFFSET                           0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_LSB                              28
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_MSB                              31
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_MASK                             0x00000000f0000000


/* Description		AP_TX_POWER_MSB_PART

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Bits [5:4] of the ap_tx_power
			See description above
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_OFFSET                           0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_LSB                              32
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_MSB                              33
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_MASK                             0x0000000300000000


/* Description		PACKET_EXTENSION_A_FACTOR

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			the packet extension duration of the trigger-based PPDU 
			response with these two bits indicating the "a-factor" 
			
			<enum 0 a_factor_4>
			<enum 1 a_factor_1>
			<enum 2 a_factor_2>
			<enum 3 a_factor_3>
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_OFFSET                      0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_LSB                         34
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_MSB                         35
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_MASK                        0x0000000c00000000


/* Description		PACKET_EXTENSION_PE_DISAMBIGUITY

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			the packet extension duration of the trigger-based PPDU 
			response with this bit indicating the PE-Disambiguity 
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET               0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                  36
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                  36
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                 0x0000001000000000


/* Description		SPATIAL_REUSE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			indicates the value of the Spatial Reuse in the HE-SIGA 
			of the HE_TRIG PPDU transmitted as a response to the Trigger
			 frame 
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_OFFSET                                  0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_LSB                                     37
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_MSB                                     52
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_MASK                                    0x001fffe000000000


/* Description		DOPPLER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			TODO: add description
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_DOPPLER_OFFSET                                        0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_DOPPLER_LSB                                           53
#define OFDMA_TRIGGER_DETAILS_DOPPLER_MSB                                           53
#define OFDMA_TRIGGER_DETAILS_DOPPLER_MASK                                          0x0020000000000000


/* Description		HE_SIGA_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			indicates the values of the reserved bits in the HE-SIGA
			 of the HE_TRIG PPDU transmitted as a response to the Trigger
			 frame
			
			In case of an EHT AP, bits [23:22] indicate the bits [55:54] 
			of the Trigger 'Common Info' called 'Special User Info Field
			 Present' and 'HE/EHT P160.' These are used along with Reserved_18a
			 to determine the presence of the EHT 'Special User Info' 
			field and EHT_trigger_response.
*/

#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_OFFSET                               0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_LSB                                  54
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_MSB                                  62
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_MASK                                 0x7fc0000000000000


/* Description		RESERVED_3B

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Common trigger info
			
			Reserved bit 63 in the Trigger 'Common Info'
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_OFFSET                                    0x0000000000000008
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_LSB                                       63
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_MSB                                       63
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_MASK                                      0x8000000000000000


/* Description		AID12

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			The AID12 subfield of the Per User Info field indicates 
			the LSB 12 bits of the AID of the STA allocated the RU to
			 transmit the MPDU(s) in the HE trigger-based PPDU
			
			Note strictly needed, but added here for debugging purposes.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_AID12_OFFSET                                          0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_AID12_LSB                                             0
#define OFDMA_TRIGGER_DETAILS_AID12_MSB                                             11
#define OFDMA_TRIGGER_DETAILS_AID12_MASK                                            0x0000000000000fff


/* Description		RU_ALLOCATION

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			trigger based RU definition
			
			If EHT_trigger_response = 0, only lower 8 bits are valid.
			
			If EHT_trigger_response = 1, all 9 bits re valid.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_OFFSET                                  0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_LSB                                     12
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_MSB                                     20
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_MASK                                    0x00000000001ff000


/* Description		MCS

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			indicates the MCS of the HE trigger-based PPDU response 
			of the STA identified by User Identifier field
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_MCS_OFFSET                                            0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_MCS_LSB                                               21
#define OFDMA_TRIGGER_DETAILS_MCS_MSB                                               24
#define OFDMA_TRIGGER_DETAILS_MCS_MASK                                              0x0000000001e00000


/* Description		DCM

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			indicates dual carrier modulation of the HE trigger-based
			 PPDU response of the STA identified by User Identifier 
			subfield. A value of 1 indicates that the HE trigger-based
			 PPDU response shall use DCM.
			Set to 0 to indicate that DCM shall not be used
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_DCM_OFFSET                                            0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_DCM_LSB                                               25
#define OFDMA_TRIGGER_DETAILS_DCM_MSB                                               25
#define OFDMA_TRIGGER_DETAILS_DCM_MASK                                              0x0000000002000000


/* Description		START_SPATIAL_STREAM

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			Indicates the starting spatial stream, STARTING_SS_NUM, 
			and is set to STARTING_SS_NUM  - 1 of the HE trigger-based
			 PPDU response of the STA identified by User Identifier 
			field. 
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_OFFSET                           0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_LSB                              26
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_MSB                              28
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_MASK                             0x000000001c000000


/* Description		NUMBER_OF_SPATIAL_STREAM

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			indicates the number of spatial streams, NUM_SS and is set
			 to NUM_SS - 1, of the HE trigger-based PPDU response of
			 the STA identified by User Identifier field.
			
			In case of EHT_trigger_response=1, RXPCU fills the MSB of
			 STARTING_SS_NUM in bit 31. If this is set, it will cause
			 PDG to indicate to PHY > 4-stream transmission resulting
			 in an abort in EHT R1 chips.
			
			TODO: Cleanup for EHT R2 chips
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_OFFSET                       0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_LSB                          29
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_MSB                          31
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_MASK                         0x00000000e0000000


/* Description		TARGET_RSSI

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			Indicates the target received signal power of the the HE
			 trigger-based PPDU response. The resolution for the Target
			 RSSI in the Per User Info field is 1dB 
			
			Values 0 to 90 maps to -110 dBm to -20 dBm
			Other values are reserved.
			
			Value 127 indicates to the STA to transmit an HE triggerbased
			 PPDU response at its maximum transmit power for the assigned
			 MCS. If Trigger_type = ax_tb_ranging_trigger and Ranging_Trigger_Subtype
			 = TF_Sound or TF_Secure_Sound, value 127 indicates to the
			 STA to transmit an HE TB-ranging NDP response at its maximum
			 transmit power for MCS 0.
			
			Used for power control algorithm
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_OFFSET                                    0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_LSB                                       32
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_MSB                                       38
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_MASK                                      0x0000007f00000000


/* Description		CODING_TYPE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			indicates the code type of the HE trigger-based PPDU response
			 of the STA identified by User Identifier subfield. 
			0: BCC
			1: LDPC
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_OFFSET                                    0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_LSB                                       39
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_MSB                                       39
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_MASK                                      0x0000008000000000


/* Description		MPDU_MU_SPACING_FACTOR

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Basic trigger variant user info
			
			<enum 0 Spacing_multiplier_is_1>
			<enum 1 Spacing_multiplier_is_2>
			<enum 2 Spacing_multiplier_is_4>
			<enum 3 Spacing_multiplier_is_8>
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_OFFSET                         0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_LSB                            40
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_MSB                            41
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_MASK                           0x0000030000000000


/* Description		TID_AGGREGATION_LIMIT

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Basic trigger variant user info
			
			indicates the of the number of TIDs that can be aggregated
			 by a STA in a multi-TID A-MPDU carried in the responding
			 Trigger-based PPDU 
			
			
			Napier AX and Hastings: 
			TXPCU will also evaluate this field, when trigger type is
			 Basic trigger. In that case, when this field is 0, TXPCU
			 will not send any data from user 0, but will immediately
			 go to user 1, which has the QoSNULL data frames...
			TODO: change for Hawkeye 2.0
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_OFFSET                          0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_LSB                             42
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_MSB                             44
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_MASK                            0x00001c0000000000


/* Description		RESERVED_5B

			<legal 0>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_OFFSET                                    0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_LSB                                       45
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_MSB                                       45
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_MASK                                      0x0000200000000000


/* Description		PREFERED_AC

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Basic trigger variant user info
			
			<enum 0 Prefered_ac_is_BK>
			<enum 1 Prefered_ac_is_BE>
			<enum 2 Prefered_ac_is_VI>
			<enum 3 Prefered_ac_is_VO>
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_OFFSET                                    0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_LSB                                       46
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_MSB                                       47
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_MASK                                      0x0000c00000000000


/* Description		BAR_CONTROL_ACK_POLICY

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Bar control field ack policy extracted from the trigger 
			frame
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_OFFSET                         0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_LSB                            48
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_MSB                            48
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_MASK                           0x0001000000000000


/* Description		BAR_CONTROL_MULTI_TID

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Bar control field multi_tid extracted from the trigger frame
			
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_OFFSET                          0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_LSB                             49
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_MSB                             49
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_MASK                            0x0002000000000000


/* Description		BAR_CONTROL_COMPRESSED_BITMAP

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Bar control field compressed bitmap extracted from the trigger
			 frame
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_OFFSET                  0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_LSB                     50
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_MSB                     50
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_MASK                    0x0004000000000000


/* Description		BAR_CONTROL_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Bar control field reserved part extracted from the trigger
			 frame
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_OFFSET                           0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_LSB                              51
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_MSB                              59
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_MASK                             0x0ff8000000000000


/* Description		BAR_CONTROL_TID_INFO

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Bar control field tid info extracted from the trigger frame
			
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_OFFSET                           0x0000000000000010
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_LSB                              60
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_MSB                              63
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_MASK                             0xf000000000000000


/* Description		NR0_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=0
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_MASK                        0x0000000000000fff


/* Description		NR0_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=0
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_MASK                       0x000000000000f000


/* Description		NR0_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=0
			
			OR
			
			Field only valid if the BAR control type indicates Basic
			 Block ACK request
			
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x00000000000f0000


/* Description		NR0_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=0
			
			OR
			
			Field valid if the BAR control type indicates Basic Block
			 ACK request
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0x00000000fff00000


/* Description		NR1_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=1
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_LSB                         32
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_MSB                         43
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_MASK                        0x00000fff00000000


/* Description		NR1_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=1
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_LSB                        44
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_MSB                        47
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_MASK                       0x0000f00000000000


/* Description		NR1_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=1
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_LSB                    48
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_MSB                    51
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f000000000000


/* Description		NR1_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=1
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000018
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               52
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               63
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff0000000000000


/* Description		NR2_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=2
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_MASK                        0x0000000000000fff


/* Description		NR2_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=2
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_MASK                       0x000000000000f000


/* Description		NR2_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=2
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x00000000000f0000


/* Description		NR2_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=2
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0x00000000fff00000


/* Description		NR3_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=3
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_LSB                         32
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_MSB                         43
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_MASK                        0x00000fff00000000


/* Description		NR3_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=3
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_LSB                        44
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_MSB                        47
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_MASK                       0x0000f00000000000


/* Description		NR3_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=3
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_LSB                    48
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_MSB                    51
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f000000000000


/* Description		NR3_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=3
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000020
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               52
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               63
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff0000000000000


/* Description		NR4_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=4
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_MASK                        0x0000000000000fff


/* Description		NR4_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=4
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_MASK                       0x000000000000f000


/* Description		NR4_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=4
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x00000000000f0000


/* Description		NR4_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=4
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0x00000000fff00000


/* Description		NR5_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=5
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_LSB                         32
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_MSB                         43
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_MASK                        0x00000fff00000000


/* Description		NR5_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=5
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_LSB                        44
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_MSB                        47
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_MASK                       0x0000f00000000000


/* Description		NR5_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=5
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_LSB                    48
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_MSB                    51
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f000000000000


/* Description		NR5_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=5
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000028
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               52
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               63
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff0000000000000


/* Description		NR6_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=6
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_MASK                        0x0000000000000fff


/* Description		NR6_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=6
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_MASK                       0x000000000000f000


/* Description		NR6_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=6
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x00000000000f0000


/* Description		NR6_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=6
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0x00000000fff00000


/* Description		NR7_PER_TID_INFO_RESERVED

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=7
			
			Per TID info, field "Reserved
			Field"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_OFFSET                      0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_LSB                         32
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_MSB                         43
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_MASK                        0x00000fff00000000


/* Description		NR7_PER_TID_INFO_TID_VALUE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=7
			
			Per TID info, field "TID value"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_LSB                        44
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_MSB                        47
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_MASK                       0x0000f00000000000


/* Description		NR7_START_SEQ_CTRL_FRAG_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field only valid if the BAR control type indicates Multi-TID
			 and BAR_control_TID_info >=7
			
			Start Sequence control, subfield fragment
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_LSB                    48
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_MSB                    51
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f000000000000


/* Description		NR7_START_SEQ_CTRL_START_SEQ_NUMBER

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			MU-BAR trigger variant user info
			
			Field valid if the BAR control type indicates Multi-TID 
			and BAR_control_TID_info >=7
			
			Start Sequence control, subfield Start sequence number
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000000000000030
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               52
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               63
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff0000000000000


/* Description		FB_SEGMENT_RETRANSMISSION_BITMAP

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			Beamforming_report_poll trigger variant user info
			
			Segment information field extracted from the trigger frame
			
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_OFFSET               0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_LSB                  0
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_MSB                  7
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_MASK                 0x00000000000000ff


/* Description		RESERVED_14A

			<legal 0>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_OFFSET                                   0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_LSB                                      8
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_MSB                                      9
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_MASK                                     0x0000000000000300


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			This field is only valid if the trigger was received in 
			an EHT PPDU.
			
			The 6-bit value used in U-SIG and/or EHT-SIG Common field
			 for the puncture pattern
			<legal 0-29>
*/

#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                   10
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                   15
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                  0x000000000000fc00


/* Description		DOT11BE_PUNCTURE_BITMAP

			This field is only valid if the trigger was received in 
			an EHT PPDU.
			
			The bitmap of 20 MHz sub-bands valid in the EHT PPDU reception
			
			
			RXPCU gets this from the received U-SIG and/or EHT-SIG via
			 PHY microcode.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_OFFSET                        0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_LSB                           16
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_MSB                           31
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_MASK                          0x00000000ffff0000


/* Description		RX_CHAIN_MASK

			Description dependent on the setting of field Rx_chain_mask_type.
			
			
			The chain mask at the start of the reception of this frame
			 when Rx_chain_mask_type is set to 1'b0. In this mode used
			 in 11ax TPC calculations for UL OFDMA/MIMO and has to be
			 in sync with the rssi_comb value as this is also used by
			 the MAC for the TPC calculations.
			
			
			The final rx chain mask used for the frame reception when
			 Rx_chain_mask_type is set to 1'b1
			
			each bit is one antenna
			0: the chain is NOT used
			1: the chain is used
			
			Supports up to 8 chains
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_OFFSET                                  0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_LSB                                     32
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_MSB                                     39
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_MASK                                    0x000000ff00000000


/* Description		RX_DURATION_FIELD

			The duration field embedded in the received trigger frame.
			
			PDG uses this field to calculate what the duration field
			 value should be in the response frame.
			This is returned to the TX PCU
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_OFFSET                              0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_LSB                                 40
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_MSB                                 55
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_MASK                                0x00ffff0000000000


/* Description		SCRAMBLER_SEED

			This field provides the 7-bit seed for the data scrambler. 
			
			Used in response generation to MU-RTS trigger, where CTS
			 needs to have the same scrambler seed as the RTS
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_OFFSET                                 0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_LSB                                    56
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_MSB                                    62
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_MASK                                   0x7f00000000000000


/* Description		RX_CHAIN_MASK_TYPE

			Indicates if the field rx_chain_mask represents the mask
			 at start of reception (on which the Rssi_comb value is 
			based), or the setting used during the remainder of the 
			reception
			
			1'b0: rxtd.listen_pri80_mask 
			1'b1: Final receive mask
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_OFFSET                             0x0000000000000038
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_LSB                                63
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_MSB                                63
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_MASK                               0x8000000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
			
			Hamilton v1 filled 'Bss_color' in bits [5:0] and 'Qos_null_only_response_tx' 
			in bit [6] here.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET          0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB             0
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB             9
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK            0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET       0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB          10
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB          10
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK         0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET    0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB       11
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB       11
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK      0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET    0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB       12
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB       12
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK      0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET              0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB                 13
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB                 15
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK                0x000000000000e000


/* Description		NORMALIZED_PRE_RSSI_COMB

			Combined pre_rssi of all chains, but "normalized" back to
			 a single chain. This avoids PDG from having to evaluate
			 this in combination with receive chain mask and perform
			 all kinds of pre-processing algorithms.
			
			Based on primary channel RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_OFFSET                       0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_LSB                          16
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_MSB                          23
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_MASK                         0x0000000000ff0000


/* Description		NORMALIZED_RSSI_COMB

			Combined rssi of all chains, but "normalized" back to a 
			single chain. This avoids PDG from having to evaluate this
			 in combination with receive chain mask and perform all 
			kinds of pre-processing algorithms.
			
			Based on primary channel RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_OFFSET                           0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_LSB                              24
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_MSB                              31
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_MASK                             0x00000000ff000000


/* Description		SW_PEER_ID

			Used by the PHY to correlated received trigger frames with
			 an AP and calculate long term statistics for this AP
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_OFFSET                                     0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_LSB                                        32
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_MSB                                        47
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_MASK                                       0x0000ffff00000000


/* Description		RESPONSE_TX_DURATION

			Field filled in by PDG based on the value that is given 
			in field response_Length in the RECEIVED_TRIGGER_INFO TLV
			
			
			The amount of time the transmission of the HW response shall
			 take (in us)
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_OFFSET                           0x0000000000000040
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_LSB                              48
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_MSB                              63
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_MASK                             0xffff000000000000


/* Description		RANGING_TRIGGER_SUBTYPE

			Field only valid if  Trigger_type = ax_tb_ranging_trigger
			
			
			Indicates the Trigger subtype for the current ranging TF
			
			
			<enum 0 TF_Poll>
			<enum 1 TF_Sound>
			<enum 2 TF_Secure_Sound>
			<enum 3 TF_Report>
			
			Hamilton v1 did not include this (and any subsequent) word.
			
			<legal 0-3>
*/

#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_OFFSET                        0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_LSB                           0
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MSB                           3
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MASK                          0x000000000000000f


/* Description		TBR_TRIGGER_COMMON_INFO_79_68

			Field only valid if Trigger_type = ax_tb_ranging_trigger
			
			
			Ranging trigger variant common info
			
			Includes fields "Reserved," "Token," "Sounding Dialog Token
			 Number"
			
			If the Trigger Dependent Common Info sub-field is less than
			 16 bits, the upper bits are set to 0.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_OFFSET                  0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_LSB                     4
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_MSB                     15
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_MASK                    0x000000000000fff0


/* Description		TBR_TRIGGER_SOUND_RESERVED_20_12

			Field only valid if Trigger_type = ax_tb_ranging_trigger
			 and Ranging_Trigger_Subtype = TF_Sound or TF_Secure_Sound
			
			
			Ranging trigger variant sounding/secure sounding sub-variant
			 user info bits [20:12]
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_OFFSET               0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_LSB                  16
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_MSB                  24
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_MASK                 0x0000000001ff0000


/* Description		I2R_REP

			Field only valid if Trigger_type = ax_tb_ranging_trigger
			 and Ranging_Trigger_Subtype = TF_Sound or TF_Secure_Sound
			
			
			Ranging trigger variant sounding/secure sounding sub-variant
			 user info field "I2R Rep"
			
			PDG uses this to to populate Nrep in 'MACTX_11AZ_USER_DESC_PER_USER.'
			
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_I2R_REP_OFFSET                                        0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_I2R_REP_LSB                                           25
#define OFDMA_TRIGGER_DETAILS_I2R_REP_MSB                                           27
#define OFDMA_TRIGGER_DETAILS_I2R_REP_MASK                                          0x000000000e000000


/* Description		TBR_TRIGGER_SOUND_RESERVED_25_24

			Field only valid if Trigger_type = ax_tb_ranging_trigger
			 and Ranging_Trigger_Subtype = TF_Sound or TF_Secure_Sound
			
			
			Ranging trigger variant sounding/secure sounding sub-variant
			 user info bits [25:24]
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_OFFSET               0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_LSB                  28
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_MSB                  29
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_MASK                 0x0000000030000000


/* Description		RESERVED_18A

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			User trigger info
			
			Reserved bit 39 in the Trigger 'User Info'
			
			In case of an EHT AP, the bit 39 of the Trigger 'User Info' 
			called 'PS160' is used along with HE_SIGA_Reserved to determine
			 EHT_trigger_response. In case of EHT, 'PS160' is also included
			 in the MSB of field RU_allocation.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_OFFSET                                   0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_LSB                                      30
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_MSB                                      30
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_MASK                                     0x0000000040000000


/* Description		QOS_NULL_ONLY_RESPONSE_TX

			Field filled in by PDG based on Rxpcu_PCIe_L0_req_duration
			
			
			If based on the duration for which RXPCU has asserted the
			 'L0 request' signal to PCIe and the PCIe L1SS exit + MAC
			 + PHY Tx latencies, PDG determines that null delimiters
			 + a programmable minimum MPDU size cannot fit the trigger
			 response, PDG sets this bit.
			
			HWSCH uses this bit to determine whether to select only 
			the 'SCHEDULER_CMD' with Trig_resp_qos_null_only set, i.e. 
			which transmit only QoS Nulls.
			
			This is filled as zero if ILP is unsupported (e.g. in Maple
			 and Spruce) or disabled.
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_OFFSET                      0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_LSB                         31
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_MSB                         31
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_MASK                        0x0000000080000000


/* Description		TBR_TRIGGER_SOUND_SAC

			Field only valid if Trigger_type = ax_tb_ranging_trigger
			 and Ranging_Trigger_Subtype = TF_Secure_Sound
			
			Ranging trigger variant secure sounding sub-variant user
			 info field "SAC"
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_OFFSET                          0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_LSB                             32
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_MSB                             47
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_MASK                            0x0000ffff00000000


/* Description		RESERVED_19A

			<legal 0>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_OFFSET                                   0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_LSB                                      48
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_MSB                                      55
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_MASK                                     0x00ff000000000000


/* Description		U_SIG_RESERVED2

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			Indicates the values of the 5 'disregard' bits [41:37] in
			 the U-SIG of the EHT_TRIG PPDU transmitted as a response
			 to the Trigger frame
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_OFFSET                                0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_LSB                                   56
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_MSB                                   60
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_MASK                                  0x1f00000000000000


/* Description		RESERVED_19B

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			Reserved bits in the Trigger
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_OFFSET                                   0x0000000000000048
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_LSB                                      61
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_MSB                                      63
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_MASK                                     0xe000000000000000


/* Description		EHT_SPECIAL_AID12

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			The AID12 subfield of the Special User Info field should
			 be '2007' for EHT R1 triggers.
			
			Note strictly needed, but added here for debugging purposes.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_OFFSET                              0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_LSB                                 0
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_MSB                                 11
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_MASK                                0x0000000000000fff


/* Description		PHY_VERSION

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			The PHY version should be '0' for EHT R1 triggers.
			
			Note strictly needed, but added here for debugging purposes.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_OFFSET                                    0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_LSB                                       12
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_MSB                                       14
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_MASK                                      0x0000000000007000


/* Description		BANDWIDTH_EXT

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			
			
			This along with the field Bandwidth determines the HE-SIG-A/U-SIG
			 BW value for the HE/EHT Trigger-based PPDU.
			
			Bandwidth/Bandwidth_ext:
			0/0: 20 MHz
			1/0: 40 MHz
			2/0: 80 MHz
			3/1: 160 MHz
			3/2: 320 MHz channelization 1
			3/3: 320 MHz channelization 2
			All other cominations are reserved.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_OFFSET                                  0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_LSB                                     15
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_MSB                                     16
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_MASK                                    0x0000000000018000


/* Description		EHT_SPATIAL_REUSE

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			Indicates the value of the Spatial Reuse in the U-SIG of
			 the EHT_TRIG PPDU transmitted as a response to the Trigger
			 frame 
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_OFFSET                              0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_LSB                                 17
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_MSB                                 24
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_MASK                                0x0000000001fe0000


/* Description		U_SIG_RESERVED1

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Special User Info
			
			Indicates the values of the 6 'disregard' bits [25:20] and
			 1 'validate' bit [28] in the U-SIG of the EHT_TRIG PPDU
			 transmitted as a response to the Trigger frame
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_OFFSET                                0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_LSB                                   25
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_MSB                                   31
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_MASK                                  0x00000000fe000000


/* Description		EHT_TRIGGER_SPECIAL_USER_INFO_71_40

			Field only valid when ax_trigger_source = 11ax_trigger_frame
			
			
			EHT Trigger Dependent field in Special User Info
			
			If the Trigger Dependent User Info sub-field is less than
			 32 bits, the upper bits are set to 0.
			
			<legal all>
*/

#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_OFFSET            0x0000000000000050
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_LSB               32
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_MSB               63
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_MASK              0xffffffff00000000



#endif   // OFDMA_TRIGGER_DETAILS
