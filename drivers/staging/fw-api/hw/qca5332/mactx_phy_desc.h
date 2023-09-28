
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

 
 
 
 
 
 
 


#ifndef _MACTX_PHY_DESC_H_
#define _MACTX_PHY_DESC_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MACTX_PHY_DESC 4

#define NUM_OF_QWORDS_MACTX_PHY_DESC 2


struct mactx_phy_desc {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             : 16, // [15:0]
                      bf_type                                                 :  2, // [17:16]
                      wait_sifs                                               :  2, // [19:18]
                      dot11b_preamble_type                                    :  1, // [20:20]
                      pkt_type                                                :  4, // [24:21]
                      su_or_mu                                                :  2, // [26:25]
                      mu_type                                                 :  1, // [27:27]
                      bandwidth                                               :  3, // [30:28]
                      channel_capture                                         :  1; // [31:31]
             uint32_t mcs                                                     :  4, // [3:0]
                      global_ofdma_mimo_enable                                :  1, // [4:4]
                      reserved_1a                                             :  1, // [5:5]
                      stbc                                                    :  1, // [6:6]
                      dot11ax_su_extended                                     :  1, // [7:7]
                      dot11ax_trigger_frame_embedded                          :  1, // [8:8]
                      tx_pwr_shared                                           :  8, // [16:9]
                      tx_pwr_unshared                                         :  8, // [24:17]
                      measure_power                                           :  1, // [25:25]
                      tpc_glut_self_cal                                       :  1, // [26:26]
                      back_to_back_transmission_expected                      :  1, // [27:27]
                      heavy_clip_nss                                          :  3, // [30:28]
                      txbf_per_packet_no_csd_no_walsh                         :  1; // [31:31]
             uint32_t ndp                                                     :  2, // [1:0]
                      ul_flag                                                 :  1, // [2:2]
                      triggered                                               :  1, // [3:3]
                      ap_pkt_bw                                               :  3, // [6:4]
                      ru_position_start                                       :  8, // [14:7]
                      pcu_ppdu_setup_start_reason                             :  3, // [17:15]
                      tlv_source                                              :  1, // [18:18]
                      reserved_2a                                             :  2, // [20:19]
                      nss                                                     :  3, // [23:21]
                      stream_offset                                           :  3, // [26:24]
                      reserved_2b                                             :  2, // [28:27]
                      clpc_enable                                             :  1, // [29:29]
                      mu_ndp                                                  :  1, // [30:30]
                      response_expected                                       :  1; // [31:31]
             uint32_t rx_chain_mask                                           :  8, // [7:0]
                      rx_chain_mask_valid                                     :  1, // [8:8]
                      ant_sel_valid                                           :  1, // [9:9]
                      ant_sel                                                 :  1, // [10:10]
                      cp_setting                                              :  2, // [12:11]
                      he_ppdu_subtype                                         :  2, // [14:13]
                      active_channel                                          :  3, // [17:15]
                      generate_phyrx_tx_start_timing                          :  1, // [18:18]
                      ltf_size                                                :  2, // [20:19]
                      ru_size_updated_v2                                      :  4, // [24:21]
                      reserved_3c                                             :  1, // [25:25]
                      u_sig_puncture_pattern_encoding                         :  6; // [31:26]
#else
             uint32_t channel_capture                                         :  1, // [31:31]
                      bandwidth                                               :  3, // [30:28]
                      mu_type                                                 :  1, // [27:27]
                      su_or_mu                                                :  2, // [26:25]
                      pkt_type                                                :  4, // [24:21]
                      dot11b_preamble_type                                    :  1, // [20:20]
                      wait_sifs                                               :  2, // [19:18]
                      bf_type                                                 :  2, // [17:16]
                      reserved_0a                                             : 16; // [15:0]
             uint32_t txbf_per_packet_no_csd_no_walsh                         :  1, // [31:31]
                      heavy_clip_nss                                          :  3, // [30:28]
                      back_to_back_transmission_expected                      :  1, // [27:27]
                      tpc_glut_self_cal                                       :  1, // [26:26]
                      measure_power                                           :  1, // [25:25]
                      tx_pwr_unshared                                         :  8, // [24:17]
                      tx_pwr_shared                                           :  8, // [16:9]
                      dot11ax_trigger_frame_embedded                          :  1, // [8:8]
                      dot11ax_su_extended                                     :  1, // [7:7]
                      stbc                                                    :  1, // [6:6]
                      reserved_1a                                             :  1, // [5:5]
                      global_ofdma_mimo_enable                                :  1, // [4:4]
                      mcs                                                     :  4; // [3:0]
             uint32_t response_expected                                       :  1, // [31:31]
                      mu_ndp                                                  :  1, // [30:30]
                      clpc_enable                                             :  1, // [29:29]
                      reserved_2b                                             :  2, // [28:27]
                      stream_offset                                           :  3, // [26:24]
                      nss                                                     :  3, // [23:21]
                      reserved_2a                                             :  2, // [20:19]
                      tlv_source                                              :  1, // [18:18]
                      pcu_ppdu_setup_start_reason                             :  3, // [17:15]
                      ru_position_start                                       :  8, // [14:7]
                      ap_pkt_bw                                               :  3, // [6:4]
                      triggered                                               :  1, // [3:3]
                      ul_flag                                                 :  1, // [2:2]
                      ndp                                                     :  2; // [1:0]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [31:26]
                      reserved_3c                                             :  1, // [25:25]
                      ru_size_updated_v2                                      :  4, // [24:21]
                      ltf_size                                                :  2, // [20:19]
                      generate_phyrx_tx_start_timing                          :  1, // [18:18]
                      active_channel                                          :  3, // [17:15]
                      he_ppdu_subtype                                         :  2, // [14:13]
                      cp_setting                                              :  2, // [12:11]
                      ant_sel                                                 :  1, // [10:10]
                      ant_sel_valid                                           :  1, // [9:9]
                      rx_chain_mask_valid                                     :  1, // [8:8]
                      rx_chain_mask                                           :  8; // [7:0]
#endif
};


/* Description		RESERVED_0A

			<legal 0>
*/

#define MACTX_PHY_DESC_RESERVED_0A_OFFSET                                           0x0000000000000000
#define MACTX_PHY_DESC_RESERVED_0A_LSB                                              0
#define MACTX_PHY_DESC_RESERVED_0A_MSB                                              15
#define MACTX_PHY_DESC_RESERVED_0A_MASK                                             0x000000000000ffff


/* Description		BF_TYPE

			<enum 0     NO_BF> Transmit a non-beamformed packet. NOTE
			 that MAC might have send MACTX_BF_PARAMS... related TLVs
			 to the PHY for this upcoming transmission, but if this 
			field indicates NO_BF, MAC_TX has for some reason decided
			 at the last moment that actual beamform transmission shall
			 not happen anymore...
			<enum 1     LEGACY_BF> Transmit a legacy beamformed packet. 
			 This means beamforming starts at the L-STF.  The possible
			 preamble formats are 11a, 11n mixed mode and 11ac.  This
			 is used to support legacy implicit beamforming.
			<enum 2     SU_BF> Transmit a single-user beamformed packet
			 starting at the HT-STF or VHT-STF. 
			<enum 3     MU_BF> Transmit a multi-user beamformed packet
			 starting at the VHT-STF. In case of an MU transmission, 
			where maybe not all users are being transmitted in a 'beamformed' 
			way, but at least one is, this e_num setting will be used
			 as well
			<legal all>
*/

#define MACTX_PHY_DESC_BF_TYPE_OFFSET                                               0x0000000000000000
#define MACTX_PHY_DESC_BF_TYPE_LSB                                                  16
#define MACTX_PHY_DESC_BF_TYPE_MSB                                                  17
#define MACTX_PHY_DESC_BF_TYPE_MASK                                                 0x0000000000030000


/* Description		WAIT_SIFS

			This bit is evaluated by the PHY TX to determine if this
			 transmission start on the air needs to be exactly SIFS 
			aligned compared to the end of the previous reception or
			 previous transmission.
			
			This feature is typically required for Triggered UL response
			 transmissions, where SIFS accuracy is really required. 
			For RTT this is also usefull, but not absolutely needed.
			
			
			
			This field is filled in by TXPCU.
			  
			<enum 0 NO_SIFS_TIMING> Transmission shall start with the
			 normal delay in PHY after receiving this notification
			<enum 1 SIFS_TIMING_DESIRED> Transmission shall be made 
			at the SIFS boundary. If shall never start before SIFS boundary, 
			but if it a little later, it is not ideal and should be 
			flagged, but transmission shall not be aborted.
			<enum 2 SIFS_TIMING_MANDATED> Transmission shall be made
			 at exactly SIFS boundary. If this notification is received
			 by the PHY after SIFS boundary already passed, the PHY 
			shall abort the transmission
			<legal 0-2>
*/

#define MACTX_PHY_DESC_WAIT_SIFS_OFFSET                                             0x0000000000000000
#define MACTX_PHY_DESC_WAIT_SIFS_LSB                                                18
#define MACTX_PHY_DESC_WAIT_SIFS_MSB                                                19
#define MACTX_PHY_DESC_WAIT_SIFS_MASK                                               0x00000000000c0000


/* Description		DOT11B_PREAMBLE_TYPE

			Valid for 802.11b packets only.
			<enum 0     short_preamble>
			<enum 1     long_preamble>
			<legal all>
*/

#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_OFFSET                                  0x0000000000000000
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_LSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MASK                                    0x0000000000100000


/* Description		PKT_TYPE

			Packet type:
			
			Note: in case of 11ax, see field he_ppdu_subtype for additional
			 info...
			
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

#define MACTX_PHY_DESC_PKT_TYPE_OFFSET                                              0x0000000000000000
#define MACTX_PHY_DESC_PKT_TYPE_LSB                                                 21
#define MACTX_PHY_DESC_PKT_TYPE_MSB                                                 24
#define MACTX_PHY_DESC_PKT_TYPE_MASK                                                0x0000000001e00000


/* Description		SU_OR_MU

			Type of transmission:
			
			For 11ax:
			<enum 0 SU_transmission> 
			11ax:
			This setting is used for the following preamble type of 
			transmissions:
			11ax HE_SU PPDU
			11ax HE_EXT_SU PPDU
			11ax HE_TRIG PPDU
			Note that the above implies all single user transmissions
			
			
			11ac and other pkt_types:
			Single user transmission
			
			<enum 1 MU_transmission> 
			11ax:
			This setting is used for the following preamble type of 
			transmissions:
			11ax HE_MU
			Note that this type of transmission implies multiple users
			
			
			For 11ac:
			Multi-user transmission
			
			<enum 2 MU_SU_transmission> 
			11ax:
			This setting is used for the following preamble type of 
			transmissions:
			11ax HE_MU
			Note that this type of transmission implies a SINGLE user, 
			but using HE_MU preamble type...
			
			11ac and other pkt_types:
			Reserved
			
			<legal 0-2>
*/

#define MACTX_PHY_DESC_SU_OR_MU_OFFSET                                              0x0000000000000000
#define MACTX_PHY_DESC_SU_OR_MU_LSB                                                 25
#define MACTX_PHY_DESC_SU_OR_MU_MSB                                                 26
#define MACTX_PHY_DESC_SU_OR_MU_MASK                                                0x0000000006000000


/* Description		MU_TYPE

			Field only valid when 
			SU_or_MU == MU_transmission or
			SU_or_MU == MU_SU_transmission
			<enum 0 MU_MIMO_Transmission>
			<enum 1 MU_OFDMA_Transmission> Note that within the RUs, 
			there might still be MU-MIMO...
			<legal all>
*/

#define MACTX_PHY_DESC_MU_TYPE_OFFSET                                               0x0000000000000000
#define MACTX_PHY_DESC_MU_TYPE_LSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MASK                                                 0x0000000008000000


/* Description		BANDWIDTH

			Packet bandwidth:
			
			The physical bandwidth that this device will be transmitting
			 in.
			
			Note that for 11ax Trigger response transmissions (when 
			Field triggered == is_triggered), this bandwith is min(AP_pkt_bw, 
			STA_ch_bw)
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define MACTX_PHY_DESC_BANDWIDTH_OFFSET                                             0x0000000000000000
#define MACTX_PHY_DESC_BANDWIDTH_LSB                                                28
#define MACTX_PHY_DESC_BANDWIDTH_MSB                                                30
#define MACTX_PHY_DESC_BANDWIDTH_MASK                                               0x0000000070000000


/* Description		CHANNEL_CAPTURE

			Indicates that the PHY should be armed to capture the channel
			 on the next received packet. This channel estimate is passed
			 to the MAC if the packet is successfully received. 
			<legal 0-1>
			This field is not applicable for 11ah  since implicit beamforming
			 is not supported  
*/

#define MACTX_PHY_DESC_CHANNEL_CAPTURE_OFFSET                                       0x0000000000000000
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_LSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MASK                                         0x0000000080000000


/* Description		MCS

			In case of  SU_or_MU == SU_transmission
			
			Note that this includes trigger response transmission
			
			The MCS to be used for the upcoming transmission. It must
			 match the 4-bit MCS value that is sent in the appropriate
			 signal field for the given packet type, except that EHT
			 BPSK with DCM and/or duplicate is encoded as '0.'
			
			In case of .11ba (WUR), this field is filled according to
			 what is on the MAC side defined as "MCS_TYPE".
			
			In case of  SU_or_MU == MU_transmission
			.11ac: highest MCS of all users
			.11ax or .11be: highest 4-bit MCS field in all the HE_SIG_B
			 or EHT_SIG TLVs that MAC S/W informs to MAC H/W. Actual
			 highest 4-bit MCS to be sent to PHY might be lower after
			 MAC H/W computation.
			
			For details, refer to  the SIG field, related to this pkt_type.
			
			(Note that this is slightly different then what is on the
			 MAC side defined as "MCS_TYPE". For this reason, the 'legal
			 values' here are NOT defined as MCS_TYPE)
			<legal all>
*/

#define MACTX_PHY_DESC_MCS_OFFSET                                                   0x0000000000000000
#define MACTX_PHY_DESC_MCS_LSB                                                      32
#define MACTX_PHY_DESC_MCS_MSB                                                      35
#define MACTX_PHY_DESC_MCS_MASK                                                     0x0000000f00000000


/* Description		GLOBAL_OFDMA_MIMO_ENABLE

			When set, this transmission contains at least 1 user for
			 which MU-MIMO is enabled in its RU.
			After per-BW/puncture pattern user disabling, in case of
			 pure OFDMA, PDG will clear this bit, but full BW MU-MIMO
			 is still possible with this bit set.
			<legal all>
*/

#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_OFFSET                              0x0000000000000000
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_LSB                                 36
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MSB                                 36
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MASK                                0x0000001000000000


/* Description		RESERVED_1A

*/

#define MACTX_PHY_DESC_RESERVED_1A_OFFSET                                           0x0000000000000000
#define MACTX_PHY_DESC_RESERVED_1A_LSB                                              37
#define MACTX_PHY_DESC_RESERVED_1A_MSB                                              37
#define MACTX_PHY_DESC_RESERVED_1A_MASK                                             0x0000002000000000


/* Description		STBC

			When set, this transmission is based on stbc rates.
*/

#define MACTX_PHY_DESC_STBC_OFFSET                                                  0x0000000000000000
#define MACTX_PHY_DESC_STBC_LSB                                                     38
#define MACTX_PHY_DESC_STBC_MSB                                                     38
#define MACTX_PHY_DESC_STBC_MASK                                                    0x0000004000000000


/* Description		DOT11AX_SU_EXTENDED

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be
			
			When set, the 11ax or 11be transmission is extended range
			 SU
*/

#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_OFFSET                                   0x0000000000000000
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_LSB                                      39
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MSB                                      39
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MASK                                     0x0000008000000000


/* Description		DOT11AX_TRIGGER_FRAME_EMBEDDED

			When set, there is an 11ax trigger frame OR 11be trigger
			 frame embedded in this transmission. PHY shall latch the
			 transmit BW of this transmission and use it to select the
			 'MACTX_UPLINK_COMMON/USER...' TLVs parameters belonging
			 to this BW. Note that these 'MACTX_UPLINK_COMMON/USER...' 
			might already have been received by the PHY, or will come
			 in later.
			<legal all>
*/

#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_OFFSET                        0x0000000000000000
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_LSB                           40
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MSB                           40
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MASK                          0x0000010000000000


/* Description		TX_PWR_SHARED

			Transmit Power (signed value) in units of 0.25 dBm 
			<legal all>
*/

#define MACTX_PHY_DESC_TX_PWR_SHARED_OFFSET                                         0x0000000000000000
#define MACTX_PHY_DESC_TX_PWR_SHARED_LSB                                            41
#define MACTX_PHY_DESC_TX_PWR_SHARED_MSB                                            48
#define MACTX_PHY_DESC_TX_PWR_SHARED_MASK                                           0x0001fe0000000000


/* Description		TX_PWR_UNSHARED

			Transmit Power (signed value) in units of 0.25 dBm <legal
			 all>
*/

#define MACTX_PHY_DESC_TX_PWR_UNSHARED_OFFSET                                       0x0000000000000000
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_LSB                                          49
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MSB                                          56
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MASK                                         0x01fe000000000000


/* Description		MEASURE_POWER

			This field enables the TPC to use power measurement for 
			current packet in CLPC updates.
			<enum 0     measure_dis> TPC will not latch power measurement
			 result for current packet
			<enum 1     measure_en> TPC will latch power measurement
			 result for current packet
			 <legal all>
*/

#define MACTX_PHY_DESC_MEASURE_POWER_OFFSET                                         0x0000000000000000
#define MACTX_PHY_DESC_MEASURE_POWER_LSB                                            57
#define MACTX_PHY_DESC_MEASURE_POWER_MSB                                            57
#define MACTX_PHY_DESC_MEASURE_POWER_MASK                                           0x0200000000000000


/* Description		TPC_GLUT_SELF_CAL

			Setting related to transmit power control calibration.
			<legal all>
*/

#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_OFFSET                                     0x0000000000000000
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_LSB                                        58
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MSB                                        58
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MASK                                       0x0400000000000000


/* Description		BACK_TO_BACK_TRANSMISSION_EXPECTED

			When set, the next transmission is expected to follow this
			 one in SIFS time (without any response reception in between).
			
			
			For example used when transmitting beacons followed by the
			 broadcast or multicast frames
			<legal all>
*/

#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_OFFSET                    0x0000000000000000
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_LSB                       59
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MSB                       59
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MASK                      0x0800000000000000


/* Description		HEAVY_CLIP_NSS

			Number of active spatial streams in current packet. This
			 parameter is used by the heavy clip function in the transmitter. 
			In case of MU PPDU, this is total Nss of all users. 
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_OFFSET                                        0x0000000000000000
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_LSB                                           60
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MSB                                           62
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MASK                                          0x7000000000000000


/* Description		TXBF_PER_PACKET_NO_CSD_NO_WALSH

			This is a global switch that is applied to beamformed packets
			
			
			If set, no_csd and no_walsh is applied to steering packet.
			
*/

#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_OFFSET                       0x0000000000000000
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_LSB                          63
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MSB                          63
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MASK                         0x8000000000000000


/* Description		NDP

			When not "0", upcoming transmission is one of the indicated
			 NDP types.
			
			<enum 0 no_ndp>No NDP transmission
			<enum 1 beamforming_ndp>Beamforming NDP
			<enum 2 he_ranging_ndp>11az NDP (HE Ranging NDP)
			<enum 3 he_feedback_ndp>Short TB (HE Feedback NDP)
*/

#define MACTX_PHY_DESC_NDP_OFFSET                                                   0x0000000000000008
#define MACTX_PHY_DESC_NDP_LSB                                                      0
#define MACTX_PHY_DESC_NDP_MSB                                                      1
#define MACTX_PHY_DESC_NDP_MASK                                                     0x0000000000000003


/* Description		UL_FLAG

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be
			
			
			Used for HE_SIGB
			<enum 1     uplink>
			<enum 0     downlink>
			<legal all>
*/

#define MACTX_PHY_DESC_UL_FLAG_OFFSET                                               0x0000000000000008
#define MACTX_PHY_DESC_UL_FLAG_LSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MASK                                                 0x0000000000000004


/* Description		TRIGGERED

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be
			
			
			Denotes whether it's a triggered uplink transmission
			
			Must be set for HE-TB NDPs used in Secure Ranging NDPs (11az) 
			and Short-NDP (HE TB Feedback NDP).
			
			<enum 0     non_trigerred>
			<enum 1     is_triggered>
			<legal all>
*/

#define MACTX_PHY_DESC_TRIGGERED_OFFSET                                             0x0000000000000008
#define MACTX_PHY_DESC_TRIGGERED_LSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MASK                                               0x0000000000000008


/* Description		AP_PKT_BW

			Field only valid when triggered == is_triggered
			
			This indicates the total bandwidth of the UL_TRIG packet
			 as indicated in the Trigger Frame.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define MACTX_PHY_DESC_AP_PKT_BW_OFFSET                                             0x0000000000000008
#define MACTX_PHY_DESC_AP_PKT_BW_LSB                                                4
#define MACTX_PHY_DESC_AP_PKT_BW_MSB                                                6
#define MACTX_PHY_DESC_AP_PKT_BW_MASK                                               0x0000000000000070


/* Description		RU_POSITION_START

			Field only valid when triggered == is_triggered
			
			
			This field indicates the start basic (26 tone) RU number
			 assigned to this user 
			
			RU Numbering is based only on the order in which the RUs
			 are allocated over the available BW, starting from 0 and
			 in increasing frequency order and not primary-secondary
			 order.
			
			The RU number within 80 MHz is available from the RU allocation
			 information in the trigger. For 160 MHz transmissions, 
			the trigger RU allocation only mentions primary/secondary
			 80 MHz. PDG needs to convert this to lower/higher 80 MHz.
			
			
			If in 'PCU_PPDU_SETUP_START'/'MACTX_PRE_PHY_DESC,' CCA_Subband_channel_bonding_mask
			 bit 0 is mapped to any of bits 4 - 7 of Freq_Subband_channel_bonding_mask, 
			then the primary 80 MHz is the higher 80 MHz and the secondary
			 80 MHz is the lower one.
			Otherwise (if CCA_Subband_channel_bonding_mask bit 0 is 
			mapped to any of bits 0 - 3 of Freq_Subband_channel_bonding_mask, 
			then the primary 80 MHz is the lower 80 MHz and the secondary
			 80 MHz is the higher one.
			
			Note: this type of encoding decouples the formatting of 
			the trigger from from how info between MAC-PHY is exchanged
			
			<legal 0- 147>
*/

#define MACTX_PHY_DESC_RU_POSITION_START_OFFSET                                     0x0000000000000008
#define MACTX_PHY_DESC_RU_POSITION_START_LSB                                        7
#define MACTX_PHY_DESC_RU_POSITION_START_MSB                                        14
#define MACTX_PHY_DESC_RU_POSITION_START_MASK                                       0x0000000000007f80


/* Description		PCU_PPDU_SETUP_START_REASON

			PDG shall fill this with the value it fills in the setup_start_reason
			 in 'PCU_PPDU_SETUP_START.' It indicates what triggered 
			the PDG to start Tx setup.
			Used for debugging purposes.
			
			<enum 0     fes_protection_frame>  RTS or CTS-to-self transmission
			 preceding the regular PPDU portion of the coming FES. The
			 transmit is initiated by PDG_TX_REQ TLV from TXPCU
			<enum 1     fes_after_protection >  Regular PPDU transmission
			 that follows the transmission of medium protection: Either
			 RTS - CTS exchanges or CTS to self. The transmit is initiated
			 by PDG_TX_REQ TLV from TXPCU 
			<enum 2     fes_only>  Regular PPDU transmission without
			 preceding medium protection frame exchanges. The transmit
			 is initiated by PDG_TX_REQ TLV from TXPCU
			<enum 3     response_frame>  response frame transmission. 
			The transmit is initiated by PDG_RESPONSE TLV from TXPCU
			
			<enum 4     trig_response_frame>  11ax triggered response
			 frame transmission. The transmit is initiated by PDG_TRIG_RESPONSE
			 TLV from TXPCU
			<enum 5     dynamic_protection_fes_only> Regular PPDU transmission
			 without preceding medium protection frame exchanges, because
			 the dynamic medium protection constraints were not satisfied. 
			The transmit is initiated by PDG_TX_REQ TLV from TXPCU.
			
			<legal 0-5>
*/

#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_OFFSET                           0x0000000000000008
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_LSB                              15
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MSB                              17
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MASK                             0x0000000000038000


/* Description		TLV_SOURCE

			<enum 1 phy_desc_from_pdg> This MACTX_PHY_DESC TLV is generated
			 by PDG.
			<enum 0 phy_desc_from_fw> PDG is in bypass mode and this
			 MACTX_PHY_DESC TLV is queued by firmware.
			<legal all>
*/

#define MACTX_PHY_DESC_TLV_SOURCE_OFFSET                                            0x0000000000000008
#define MACTX_PHY_DESC_TLV_SOURCE_LSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MASK                                              0x0000000000040000


/* Description		RESERVED_2A

			<legal 0>
*/

#define MACTX_PHY_DESC_RESERVED_2A_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_2A_LSB                                              19
#define MACTX_PHY_DESC_RESERVED_2A_MSB                                              20
#define MACTX_PHY_DESC_RESERVED_2A_MASK                                             0x0000000000180000


/* Description		NSS

			Field only valid when triggered == is_triggered
			
			Number of Spatial Streams occupied by the User
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define MACTX_PHY_DESC_NSS_OFFSET                                                   0x0000000000000008
#define MACTX_PHY_DESC_NSS_LSB                                                      21
#define MACTX_PHY_DESC_NSS_MSB                                                      23
#define MACTX_PHY_DESC_NSS_MASK                                                     0x0000000000e00000


/* Description		STREAM_OFFSET

			Field only valid when triggered == is_triggered
			
			Specify Stream-offset of the user for HE_TB Ranging NDP 
			or Short-NDP
			
			Stream Offset from which the User occupies the Streams
*/

#define MACTX_PHY_DESC_STREAM_OFFSET_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_STREAM_OFFSET_LSB                                            24
#define MACTX_PHY_DESC_STREAM_OFFSET_MSB                                            26
#define MACTX_PHY_DESC_STREAM_OFFSET_MASK                                           0x0000000007000000


/* Description		RESERVED_2B

			<legal 0>
*/

#define MACTX_PHY_DESC_RESERVED_2B_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_2B_LSB                                              27
#define MACTX_PHY_DESC_RESERVED_2B_MSB                                              28
#define MACTX_PHY_DESC_RESERVED_2B_MASK                                             0x0000000018000000


/* Description		CLPC_ENABLE

			This field enables closed-loop TPC operation by enabling
			 CLPC adjustment of DAC gain for the next packet.
			<enum 0     clpc_off> TPC error update disabled
			<enum 1     clpc_on> TPC error will be applied to DAC gain
			 setting for the next packet
			<legal 0-1>
*/

#define MACTX_PHY_DESC_CLPC_ENABLE_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_CLPC_ENABLE_LSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MASK                                             0x0000000020000000


/* Description		MU_NDP

			If set indicates that this packet is an NDP used for MU 
			channel estimation.  This bit will be used by the TPC to
			 signal that the analog gain settings can be updated. The
			 analog gain settings will not change for subsequent MU 
			data packets.
*/

#define MACTX_PHY_DESC_MU_NDP_OFFSET                                                0x0000000000000008
#define MACTX_PHY_DESC_MU_NDP_LSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MASK                                                  0x0000000040000000


/* Description		RESPONSE_EXPECTED

			When set, a response frame in SIFS time is expected after
			 this transmission.
			<legal all>
*/

#define MACTX_PHY_DESC_RESPONSE_EXPECTED_OFFSET                                     0x0000000000000008
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_LSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MASK                                       0x0000000080000000


/* Description		RX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define MACTX_PHY_DESC_RX_CHAIN_MASK_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_RX_CHAIN_MASK_LSB                                            32
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MSB                                            39
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MASK                                           0x000000ff00000000


/* Description		RX_CHAIN_MASK_VALID

			Indicates rx_chain_mask field is valid. 
			<enum 0 RX_CHAIN_MASK_IS_NOT_VALID>
			<enum 1 RX_CHAIN_MASK_IS_VALID>
			<legal all>
*/

#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_OFFSET                                   0x0000000000000008
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_LSB                                      40
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MSB                                      40
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MASK                                     0x0000010000000000


/* Description		ANT_SEL_VALID

			Field only valid when ant_sel_valid is set.
			
			TX Antenna select valid
			<enum 0 ANT_SEL_IS_NOT_VALID>
			<enum 1 ANT_SEL_IS_VALID>
			<legal all>
*/

#define MACTX_PHY_DESC_ANT_SEL_VALID_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_ANT_SEL_VALID_LSB                                            41
#define MACTX_PHY_DESC_ANT_SEL_VALID_MSB                                            41
#define MACTX_PHY_DESC_ANT_SEL_VALID_MASK                                           0x0000020000000000


/* Description		ANT_SEL

			Field only valid when ant_sel_valid is set.
			
			Antenna select for TX antenna diversity.
			<enum 0 ANTENNA_0>
			<enum 1 ANTENNA_1>
			<legal all>
*/

#define MACTX_PHY_DESC_ANT_SEL_OFFSET                                               0x0000000000000008
#define MACTX_PHY_DESC_ANT_SEL_LSB                                                  42
#define MACTX_PHY_DESC_ANT_SEL_MSB                                                  42
#define MACTX_PHY_DESC_ANT_SEL_MASK                                                 0x0000040000000000


/* Description		CP_SETTING

			Field only valid when pkt type is HT, VHT or HE.
			
			Specify the right CP for HE-Ranging NDPs (11az)/Short NDP
			
			
			<enum 0     gi_0_8_us > Legacy normal GI
			<enum 1     gi_0_4_us > Legacy short GI
			<enum 2     gi_1_6_us > HE related GI
			<enum 3     gi_3_2_us > HE related GI
			<legal 0 - 3>
*/

#define MACTX_PHY_DESC_CP_SETTING_OFFSET                                            0x0000000000000008
#define MACTX_PHY_DESC_CP_SETTING_LSB                                               43
#define MACTX_PHY_DESC_CP_SETTING_MSB                                               44
#define MACTX_PHY_DESC_CP_SETTING_MASK                                              0x0000180000000000


/* Description		HE_PPDU_SUBTYPE

			The subtype of HE transmission:
			
			Specify as HE-SU for HE-SU Ranging NDP in 11az ;
			Specify as HE-TB for HE-TB Ranging NDP in 11az ; 
			Specify as HE-TB for Short -NDP
			Re-use the same for EHT PPDU types also
			<enum 0 he_subtype_SU>
			<enum 1 he_subtype_TRIG>
			<enum 2 he_subtype_MU>
			<enum 3 he_subtype_EXT_SU>
			
			<legal all>
*/

#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_OFFSET                                       0x0000000000000008
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_LSB                                          45
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MSB                                          46
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MASK                                         0x0000600000000000


/* Description		ACTIVE_CHANNEL

			Field only valid when triggered == non_trigerred
			In case of a triggered response transmission, this field
			 will always be set to 0
			
			This field indicates the active frequency band when the 
			packet bandwidth is less than the channel bandwidth. For
			 non 11ax packets this is same as the primary channel
			<legal all>
*/

#define MACTX_PHY_DESC_ACTIVE_CHANNEL_OFFSET                                        0x0000000000000008
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_LSB                                           47
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MSB                                           49
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MASK                                          0x0003800000000000


/* Description		GENERATE_PHYRX_TX_START_TIMING

			When set, PHY shall generate the PHYRX_TX_START_TIMING TLV
			 at the earliest opportunity during the preamble transmission
			
			<legal all>
*/

#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_OFFSET                        0x0000000000000008
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_LSB                           50
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MSB                           50
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MASK                          0x0004000000000000


/* Description		LTF_SIZE

			Field only valid when pkt type is HE.
			
			Ltf size
			
			Specify right LTF-size for HE-Ranging NDPs (11az)/Short-NDP
			
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define MACTX_PHY_DESC_LTF_SIZE_OFFSET                                              0x0000000000000008
#define MACTX_PHY_DESC_LTF_SIZE_LSB                                                 51
#define MACTX_PHY_DESC_LTF_SIZE_MSB                                                 52
#define MACTX_PHY_DESC_LTF_SIZE_MASK                                                0x0018000000000000


/* Description		RU_SIZE_UPDATED_V2

			Field only valid for pkt_type == 11ax or 11be and 
			SU_or_MU == SU_transmission or 
			SU_or_MU == MU_SU_transmission
			
			The RU size of the upcoming transmission.
			
			PHY uses this info to apply different min/max BO if payload
			 bandwidth is less than 10MHz
			
			In case of HE extended range transmission, e-num 2 (10MHz) 
			or e-num 7 (20MHz) are used.
			
			In case of trig transmission or OFDMA single user or MU-MIMO
			 single user transmission, if the ru_size allocated to the
			 user is the fullBW (with respect to AP_bw) ru size then
			 the e-num 7 is used.
			For all other cases, e-nums corresponding to the ru size
			 allocated to the user is used.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242><enum 4 RU_484><enum 5 RU_996><enum 6 RU_1992>
			
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			Note that for an MU-RTS trigger, the response will also 
			go out in legacy CTS rate... and thus e-num 7 will be used.
			
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			
			<enum 10 RU_MULTI_LARGE> HW will use per-user sub-band-mask
			 to infer the actual RU-size for Multi-large-RU/SU-Puncturing
			
			
			<enum 11 RU_78> multi small RU
			<enum 12 RU_132> multi small RU
			
			
			
			NOTE: See the table following this TLV definition that explains
			 the relationship between this field and the RU size allocated
			 to users.
			
			<legal all>
*/

#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_OFFSET                                    0x0000000000000008
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_LSB                                       53
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MSB                                       56
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MASK                                      0x01e0000000000000


/* Description		RESERVED_3C

			<legal 0>
*/

#define MACTX_PHY_DESC_RESERVED_3C_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_3C_LSB                                              57
#define MACTX_PHY_DESC_RESERVED_3C_MSB                                              57
#define MACTX_PHY_DESC_RESERVED_3C_MASK                                             0x0200000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			Field only valid for pkt_type == 11be
			
			The 6-bit value to be used in U-SIG and/or EHT-SIG Common
			 field for the puncture pattern
			<legal 0-29>
*/

#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                       0x0000000000000008
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                          58
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                          63
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                         0xfc00000000000000



#endif   // MACTX_PHY_DESC
