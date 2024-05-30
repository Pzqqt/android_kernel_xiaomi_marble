
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

 
 
 
 
 
 
 


#ifndef _TX_CBF_INFO_H_
#define _TX_CBF_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_CBF_INFO 16

#define NUM_OF_QWORDS_TX_CBF_INFO 8


struct tx_cbf_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      pre_cbf_duration                                        : 16; // [31:16]
             uint32_t brpoll_info_valid                                       :  1, // [0:0]
                      trigger_brpoll_info_valid                               :  1, // [1:1]
                      npda_info_11ac_valid                                    :  1, // [2:2]
                      npda_info_11ax_valid                                    :  1, // [3:3]
                      dot11ax_su_extended                                     :  1, // [4:4]
                      bandwidth                                               :  3, // [7:5]
                      brpoll_info                                             :  8, // [15:8]
                      cbf_response_table_base_index                           :  8, // [23:16]
                      peer_index                                              :  3, // [26:24]
                      pkt_type                                                :  4, // [30:27]
                      txop_duration_all_ones                                  :  1; // [31:31]
             uint32_t trigger_brpoll_common_info_15_0                         : 16, // [15:0]
                      trigger_brpoll_common_info_31_16                        : 16; // [31:16]
             uint32_t trigger_brpoll_user_info_15_0                           : 16, // [15:0]
                      trigger_brpoll_user_info_31_16                          : 16; // [31:16]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr1_47_32                                             : 16, // [15:0]
                      addr2_15_0                                              : 16; // [31:16]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t addr3_31_0                                              : 32; // [31:0]
             uint32_t addr3_47_32                                             : 16, // [15:0]
                      sta_partial_aid                                         : 11, // [26:16]
                      reserved_8a                                             :  4, // [30:27]
                      cbf_resp_pwr_mgmt                                       :  1; // [31:31]
             uint32_t group_id                                                :  6, // [5:0]
                      rssi_comb                                               :  8, // [13:6]
                      reserved_9a                                             :  2, // [15:14]
                      vht_ndpa_sta_info                                       : 16; // [31:16]
             uint32_t he_eht_sta_info_15_0                                    : 16, // [15:0]
                      he_eht_sta_info_31_16                                   : 16; // [31:16]
             uint32_t dot11ax_received_format_indication                      :  1, // [0:0]
                      dot11ax_received_dl_ul_flag                             :  1, // [1:1]
                      dot11ax_received_bss_color_id                           :  6, // [7:2]
                      dot11ax_received_spatial_reuse                          :  4, // [11:8]
                      dot11ax_received_cp_size                                :  2, // [13:12]
                      dot11ax_received_ltf_size                               :  2, // [15:14]
                      dot11ax_received_coding                                 :  1, // [16:16]
                      dot11ax_received_dcm                                    :  1, // [17:17]
                      dot11ax_received_doppler_indication                     :  1, // [18:18]
                      dot11ax_received_ext_ru_size                            :  4, // [22:19]
                      dot11ax_dl_ul_flag                                      :  1, // [23:23]
                      reserved_11a                                            :  8; // [31:24]
             uint32_t sw_response_frame_length                                : 16, // [15:0]
                      sw_response_tlv_from_crypto                             :  1, // [16:16]
                      wait_sifs_config_valid                                  :  1, // [17:17]
                      wait_sifs                                               :  2, // [19:18]
                      ranging                                                 :  1, // [20:20]
                      secure                                                  :  1, // [21:21]
                      tb_ranging_response_required                            :  2, // [23:22]
                      reserved_12a                                            :  2, // [25:24]
                      u_sig_puncture_pattern_encoding                         :  6; // [31:26]
             uint32_t dot11be_puncture_bitmap                                 : 16, // [15:0]
                      dot11be_response                                        :  1, // [16:16]
                      punctured_response                                      :  1, // [17:17]
                      npda_info_11be_valid                                    :  1, // [18:18]
                      eht_duplicate_mode                                      :  2, // [20:19]
                      reserved_13a                                            : 11; // [31:21]
             uint32_t eht_sta_info_39_32                                      :  8, // [7:0]
                      reserved_14a                                            : 24; // [31:8]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t pre_cbf_duration                                        : 16, // [31:16]
                      sw_peer_id                                              : 16; // [15:0]
             uint32_t txop_duration_all_ones                                  :  1, // [31:31]
                      pkt_type                                                :  4, // [30:27]
                      peer_index                                              :  3, // [26:24]
                      cbf_response_table_base_index                           :  8, // [23:16]
                      brpoll_info                                             :  8, // [15:8]
                      bandwidth                                               :  3, // [7:5]
                      dot11ax_su_extended                                     :  1, // [4:4]
                      npda_info_11ax_valid                                    :  1, // [3:3]
                      npda_info_11ac_valid                                    :  1, // [2:2]
                      trigger_brpoll_info_valid                               :  1, // [1:1]
                      brpoll_info_valid                                       :  1; // [0:0]
             uint32_t trigger_brpoll_common_info_31_16                        : 16, // [31:16]
                      trigger_brpoll_common_info_15_0                         : 16; // [15:0]
             uint32_t trigger_brpoll_user_info_31_16                          : 16, // [31:16]
                      trigger_brpoll_user_info_15_0                           : 16; // [15:0]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr2_15_0                                              : 16, // [31:16]
                      addr1_47_32                                             : 16; // [15:0]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t addr3_31_0                                              : 32; // [31:0]
             uint32_t cbf_resp_pwr_mgmt                                       :  1, // [31:31]
                      reserved_8a                                             :  4, // [30:27]
                      sta_partial_aid                                         : 11, // [26:16]
                      addr3_47_32                                             : 16; // [15:0]
             uint32_t vht_ndpa_sta_info                                       : 16, // [31:16]
                      reserved_9a                                             :  2, // [15:14]
                      rssi_comb                                               :  8, // [13:6]
                      group_id                                                :  6; // [5:0]
             uint32_t he_eht_sta_info_31_16                                   : 16, // [31:16]
                      he_eht_sta_info_15_0                                    : 16; // [15:0]
             uint32_t reserved_11a                                            :  8, // [31:24]
                      dot11ax_dl_ul_flag                                      :  1, // [23:23]
                      dot11ax_received_ext_ru_size                            :  4, // [22:19]
                      dot11ax_received_doppler_indication                     :  1, // [18:18]
                      dot11ax_received_dcm                                    :  1, // [17:17]
                      dot11ax_received_coding                                 :  1, // [16:16]
                      dot11ax_received_ltf_size                               :  2, // [15:14]
                      dot11ax_received_cp_size                                :  2, // [13:12]
                      dot11ax_received_spatial_reuse                          :  4, // [11:8]
                      dot11ax_received_bss_color_id                           :  6, // [7:2]
                      dot11ax_received_dl_ul_flag                             :  1, // [1:1]
                      dot11ax_received_format_indication                      :  1; // [0:0]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [31:26]
                      reserved_12a                                            :  2, // [25:24]
                      tb_ranging_response_required                            :  2, // [23:22]
                      secure                                                  :  1, // [21:21]
                      ranging                                                 :  1, // [20:20]
                      wait_sifs                                               :  2, // [19:18]
                      wait_sifs_config_valid                                  :  1, // [17:17]
                      sw_response_tlv_from_crypto                             :  1, // [16:16]
                      sw_response_frame_length                                : 16; // [15:0]
             uint32_t reserved_13a                                            : 11, // [31:21]
                      eht_duplicate_mode                                      :  2, // [20:19]
                      npda_info_11be_valid                                    :  1, // [18:18]
                      punctured_response                                      :  1, // [17:17]
                      dot11be_response                                        :  1, // [16:16]
                      dot11be_puncture_bitmap                                 : 16; // [15:0]
             uint32_t reserved_14a                                            : 24, // [31:8]
                      eht_sta_info_39_32                                      :  8; // [7:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		SW_PEER_ID

			An identifier indicating from which AP this CBF is being
			 requested. Helps in crosschecking that the MAC and PHY 
			are still in sync on what is stored in the cbf_mem_index
			 location.
			<legal all>
*/

#define TX_CBF_INFO_SW_PEER_ID_OFFSET                                               0x0000000000000000
#define TX_CBF_INFO_SW_PEER_ID_LSB                                                  0
#define TX_CBF_INFO_SW_PEER_ID_MSB                                                  15
#define TX_CBF_INFO_SW_PEER_ID_MASK                                                 0x000000000000ffff


/* Description		PRE_CBF_DURATION

			NPDA_duration_field - SIFS - NDP_pkt_time or BRPOLL_duration_field. 
			 The cbf_duration_field = pre_cbf_duration - cbf_pkt_time
			
			
			This will be the pre-NDP duration or pre-LMR duration in
			 case of .11az ranging (field Ranging below is set).
*/

#define TX_CBF_INFO_PRE_CBF_DURATION_OFFSET                                         0x0000000000000000
#define TX_CBF_INFO_PRE_CBF_DURATION_LSB                                            16
#define TX_CBF_INFO_PRE_CBF_DURATION_MSB                                            31
#define TX_CBF_INFO_PRE_CBF_DURATION_MASK                                           0x00000000ffff0000


/* Description		BRPOLL_INFO_VALID

			When set, legacy type brpoll info is valid. TXPCU will have
			 to trigger the PDG for response transmission
			
			It will not be clear here what the PHY's response format
			 will be. Could be 11ac or 11ax. MAC is not 'remembering' 
			the format type, but PHY will know.
			
			MAC will get to know based on the field Cbf_response_type
			 in the PHYRX_CBF_READ_REQUEST_ACK TLV.
			
			<legal all>
*/

#define TX_CBF_INFO_BRPOLL_INFO_VALID_OFFSET                                        0x0000000000000000
#define TX_CBF_INFO_BRPOLL_INFO_VALID_LSB                                           32
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MSB                                           32
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MASK                                          0x0000000100000000


/* Description		TRIGGER_BRPOLL_INFO_VALID

			When set with Ranging = 0, trigger based brpoll info is 
			valid.
			When set with Ranging = 1, .11az sounding trigger info is
			 valid for trigger-based ranging (TBR).
			This also implies that RXPCU has already triggered the PDG
			 for response transmission
			<legal all>
*/

#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_OFFSET                                0x0000000000000000
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_LSB                                   33
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MSB                                   33
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MASK                                  0x0000000200000000


/* Description		NPDA_INFO_11AC_VALID

			When set, 11ac_NDPA info is valid.
			TXPCU will have to trigger the PDG for response transmission
			 
			
			PHY's response will be be in 11ac format
			<legal all>
*/

#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_OFFSET                                     0x0000000000000000
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_LSB                                        34
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MSB                                        34
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MASK                                       0x0000000400000000


/* Description		NPDA_INFO_11AX_VALID

			When set, 11ax_NDPA info is valid.
			TXPCU will have to trigger the PDG for response transmission
			 
			
			PHY's response will be be in 11ax format
			
			There is a separate Npda_info_11be_valid field near the 
			end of this TLV.
			<legal all>
*/

#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_OFFSET                                     0x0000000000000000
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_LSB                                        35
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MSB                                        35
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MASK                                       0x0000000800000000


/* Description		DOT11AX_SU_EXTENDED

			When set, frame was received in 11ax or 11be extended range
			 format 
*/

#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_OFFSET                                      0x0000000000000000
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_LSB                                         36
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MSB                                         36
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MASK                                        0x0000001000000000


/* Description		BANDWIDTH

			Field only valid when Brpoll_info_valid , Npda_info_11ac_valid
			  or Npda_info_11ax_valid is set.
			
			The bandwidth that TXPCU uses to select the final response
			 table entry. That entry will contain all response info 
			for the CBF frame.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_CBF_INFO_BANDWIDTH_OFFSET                                                0x0000000000000000
#define TX_CBF_INFO_BANDWIDTH_LSB                                                   37
#define TX_CBF_INFO_BANDWIDTH_MSB                                                   39
#define TX_CBF_INFO_BANDWIDTH_MASK                                                  0x000000e000000000


/* Description		BRPOLL_INFO

			Field only valid when Brpoll_info_valid is set.
			
			Feedback Segment retransmission feedback field from the 
			BRPOLL frame.
			<legal all>
*/

#define TX_CBF_INFO_BRPOLL_INFO_OFFSET                                              0x0000000000000000
#define TX_CBF_INFO_BRPOLL_INFO_LSB                                                 40
#define TX_CBF_INFO_BRPOLL_INFO_MSB                                                 47
#define TX_CBF_INFO_BRPOLL_INFO_MASK                                                0x0000ff0000000000


/* Description		CBF_RESPONSE_TABLE_BASE_INDEX

			Field only valid when Brpoll_info_valid or
			Npda_info_11ac_valid  or Npda_info_11ax_valid is set.
			
			When set to 0, use the register based lookup for determining
			 the CBF response rates.
			
			When > 0, TXPCU shall use this response table index for 
			the 20 MHz response, and higher BW responses are in the 
			subsequent response table entries
			
			This will be the LMR response table base index in case of
			 .11az ranging (field Ranging below is set).
			
			<legal all>
*/

#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_OFFSET                            0x0000000000000000
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_LSB                               48
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MSB                               55
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MASK                              0x00ff000000000000


/* Description		PEER_INDEX

			Field only valid when Brpoll_info_valid or
			Npda_info_11ac_valid  or Npda_info_11ax_valid is set.
			
			
			Indicates the CBF peer index to be used by TxPCU to determine
			 the look-up table index for CBF response frames. RxPCU 
			populate this field from the peer_entry. 
			<legal 0-7>
*/

#define TX_CBF_INFO_PEER_INDEX_OFFSET                                               0x0000000000000000
#define TX_CBF_INFO_PEER_INDEX_LSB                                                  56
#define TX_CBF_INFO_PEER_INDEX_MSB                                                  58
#define TX_CBF_INFO_PEER_INDEX_MASK                                                 0x0700000000000000


/* Description		PKT_TYPE

			Received Packet type:
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

#define TX_CBF_INFO_PKT_TYPE_OFFSET                                                 0x0000000000000000
#define TX_CBF_INFO_PKT_TYPE_LSB                                                    59
#define TX_CBF_INFO_PKT_TYPE_MSB                                                    62
#define TX_CBF_INFO_PKT_TYPE_MASK                                                   0x7800000000000000


/* Description		TXOP_DURATION_ALL_ONES

			When set, either the TXOP_DURATION of the received frame
			 was set to all 1s or there is a BSS color collision. The
			 TXOP_DURATION of the transmit response should be forced
			 to all 1s.
			
			<legal all>
*/

#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_OFFSET                                   0x0000000000000000
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_LSB                                      63
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MSB                                      63
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MASK                                     0x8000000000000000


/* Description		TRIGGER_BRPOLL_COMMON_INFO_15_0

			Field only valid when Trigger_Brpoll_info_valid  is set.
			
			
			Trigger based BRPOLL or .11az sounding (TBR) request info... 
			bits [15:0] 
			
			This is the variable common info field from the trigger 
			related to the BTPOLL. For field definition see IEEE spec
			
			
			Note: final IEEE field might not need all these bits. If
			 so, the extra bits become reserved fields.
			<legal all>
*/

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_OFFSET                          0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_LSB                             0
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MSB                             15
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MASK                            0x000000000000ffff


/* Description		TRIGGER_BRPOLL_COMMON_INFO_31_16

			Field only valid when Trigger_Brpoll_info_valid  is set.
			
			
			Trigger based BRPOLL or .11az sounding (TBR) request info... 
			bits [31:15] 
			
			This is the variable common info field from the trigger 
			related to the BTPOLL. For field definition see IEEE spec
			
			
			Note: final IEEE field might not need all these bits. If
			 so, the extra bits become reserved fields.
			<legal all>
*/

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_OFFSET                         0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_LSB                            16
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MSB                            31
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MASK                           0x00000000ffff0000


/* Description		TRIGGER_BRPOLL_USER_INFO_15_0

			Field only valid when Trigger_Brpoll_info_valid  is set.
			
			
			BRPOLL or .11az sounding (TBR) trigger Type dependent User
			 information bits [15:0] 
			
			This is the variable user info field from the trigger related
			 to the BTPOLL. 
			
			For field definition see IEEE spec
			
			Note: final IEEE field might not need all these bits. If
			 so, the extra bits become reserved fields.
			<legal all>
*/

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_OFFSET                            0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_LSB                               32
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MSB                               47
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MASK                              0x0000ffff00000000


/* Description		TRIGGER_BRPOLL_USER_INFO_31_16

			Field only valid when Trigger_Brpoll_info_valid  is set.
			
			
			BRPOLL or .11az sounding (TBR) trigger Type dependent User
			 information bits [31:16] 
			
			This is the variable user info field from the trigger related
			 to the BTPOLL. 
			
			For field definition see IEEE spec
			
			Note: final IEEE field might not need all these bits. If
			 so, the extra bits become reserved fields.
			<legal all>
*/

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_OFFSET                           0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_LSB                              48
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MSB                              63
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MASK                             0xffff000000000000


/* Description		ADDR1_31_0

			CBF address1[31:0]
*/

#define TX_CBF_INFO_ADDR1_31_0_OFFSET                                               0x0000000000000010
#define TX_CBF_INFO_ADDR1_31_0_LSB                                                  0
#define TX_CBF_INFO_ADDR1_31_0_MSB                                                  31
#define TX_CBF_INFO_ADDR1_31_0_MASK                                                 0x00000000ffffffff


/* Description		ADDR1_47_32

			CBF address1[47:32]
*/

#define TX_CBF_INFO_ADDR1_47_32_OFFSET                                              0x0000000000000010
#define TX_CBF_INFO_ADDR1_47_32_LSB                                                 32
#define TX_CBF_INFO_ADDR1_47_32_MSB                                                 47
#define TX_CBF_INFO_ADDR1_47_32_MASK                                                0x0000ffff00000000


/* Description		ADDR2_15_0

			CBF address2[15:0]
*/

#define TX_CBF_INFO_ADDR2_15_0_OFFSET                                               0x0000000000000010
#define TX_CBF_INFO_ADDR2_15_0_LSB                                                  48
#define TX_CBF_INFO_ADDR2_15_0_MSB                                                  63
#define TX_CBF_INFO_ADDR2_15_0_MASK                                                 0xffff000000000000


/* Description		ADDR2_47_16

			CBF address2[47:16]
*/

#define TX_CBF_INFO_ADDR2_47_16_OFFSET                                              0x0000000000000018
#define TX_CBF_INFO_ADDR2_47_16_LSB                                                 0
#define TX_CBF_INFO_ADDR2_47_16_MSB                                                 31
#define TX_CBF_INFO_ADDR2_47_16_MASK                                                0x00000000ffffffff


/* Description		ADDR3_31_0

			CBF address3[31:0]
*/

#define TX_CBF_INFO_ADDR3_31_0_OFFSET                                               0x0000000000000018
#define TX_CBF_INFO_ADDR3_31_0_LSB                                                  32
#define TX_CBF_INFO_ADDR3_31_0_MSB                                                  63
#define TX_CBF_INFO_ADDR3_31_0_MASK                                                 0xffffffff00000000


/* Description		ADDR3_47_32

			CBF address3[47:16]
*/

#define TX_CBF_INFO_ADDR3_47_32_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_ADDR3_47_32_LSB                                                 0
#define TX_CBF_INFO_ADDR3_47_32_MSB                                                 15
#define TX_CBF_INFO_ADDR3_47_32_MASK                                                0x000000000000ffff


/* Description		STA_PARTIAL_AID

			Partial AID field
*/

#define TX_CBF_INFO_STA_PARTIAL_AID_OFFSET                                          0x0000000000000020
#define TX_CBF_INFO_STA_PARTIAL_AID_LSB                                             16
#define TX_CBF_INFO_STA_PARTIAL_AID_MSB                                             26
#define TX_CBF_INFO_STA_PARTIAL_AID_MASK                                            0x0000000007ff0000


/* Description		RESERVED_8A

			<legal 0>
*/

#define TX_CBF_INFO_RESERVED_8A_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_RESERVED_8A_LSB                                                 27
#define TX_CBF_INFO_RESERVED_8A_MSB                                                 30
#define TX_CBF_INFO_RESERVED_8A_MASK                                                0x0000000078000000


/* Description		CBF_RESP_PWR_MGMT

			Power management bit of the response CBF frame or LMR frame
			 in case of .11az ranging (field Ranging below is set).
*/

#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_OFFSET                                        0x0000000000000020
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_LSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MASK                                          0x0000000080000000


/* Description		GROUP_ID

			Group ID field
*/

#define TX_CBF_INFO_GROUP_ID_OFFSET                                                 0x0000000000000020
#define TX_CBF_INFO_GROUP_ID_LSB                                                    32
#define TX_CBF_INFO_GROUP_ID_MSB                                                    37
#define TX_CBF_INFO_GROUP_ID_MASK                                                   0x0000003f00000000


/* Description		RSSI_COMB

			The combined RSSI of the legacy STF of RX PPDU of all active
			 chains and bandwidths.
*/

#define TX_CBF_INFO_RSSI_COMB_OFFSET                                                0x0000000000000020
#define TX_CBF_INFO_RSSI_COMB_LSB                                                   38
#define TX_CBF_INFO_RSSI_COMB_MSB                                                   45
#define TX_CBF_INFO_RSSI_COMB_MASK                                                  0x00003fc000000000


/* Description		RESERVED_9A

			Bit 14: force_extra_symbol:
			
			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
			
			Not supported in Hamilton v1
			<legal 0-1>
*/

#define TX_CBF_INFO_RESERVED_9A_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_RESERVED_9A_LSB                                                 46
#define TX_CBF_INFO_RESERVED_9A_MSB                                                 47
#define TX_CBF_INFO_RESERVED_9A_MASK                                                0x0000c00000000000


/* Description		VHT_NDPA_STA_INFO

			Field only valid when Npda_info_11ac_valid is set
			
			The complete (RAW) STA INFO field that MAC extracted from
			 the VHT NDPA frame.
			
			Put here for backup reasons in case last moment fields got
			 added that PHY needs to be able to interpret
			
			This field contains 
			{
			VHT STA_INFO.NC_INDEX[2:0],
			VHT STA_INFO.FEEDBACK_TYPE,
			VHT STA_INFO.AID12[11:0]
			}
			<legal all>
*/

#define TX_CBF_INFO_VHT_NDPA_STA_INFO_OFFSET                                        0x0000000000000020
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_LSB                                           48
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MSB                                           63
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MASK                                          0xffff000000000000


/* Description		HE_EHT_STA_INFO_15_0

			Field only valid when Npda_info_11ax_valid or Npda_info_11be_valid
			 is set
			
			The first 16 bits of the RAW HE or EHT STA INFO field in
			 the NDPA frame
			
			Put here for backup reasons in case last moment fields got
			 added that PHY needs to be able to interpret
			<legal all>
*/

#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_OFFSET                                     0x0000000000000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_LSB                                        0
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MSB                                        15
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MASK                                       0x000000000000ffff


/* Description		HE_EHT_STA_INFO_31_16

			Field only valid when Npda_info_11ax_valid or Npda_info_11be_valid
			 is set
			
			The second 16 bits of the RAW HE or EHT STA INFO field in
			 the NDPA frame
			
			Put here for backup reasons in case last moment fields got
			 added that PHY needs to be able to interpret
			
			There is an EHT_STA_INFO_39_32 field near the end of this
			 TLV.
			<legal all>
*/

#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_OFFSET                                    0x0000000000000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_LSB                                       16
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MSB                                       31
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MASK                                      0x00000000ffff0000


/* Description		DOT11AX_RECEIVED_FORMAT_INDICATION

			This field is only valid for pkt_type == 11ax
			
			Format_Indication from the received frame.
			
			<enum 0 HE_SIGA_FORMAT_HE_TRIG>
			<enum 1 HE_SIGA_FORMAT_SU_OR_EXT_SU>
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET                       0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB                          32
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB                          32
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK                         0x0000000100000000


/* Description		DOT11AX_RECEIVED_DL_UL_FLAG

			This field is only valid for pkt_type == 11ax
			
			DL_UL_flag from the received frame
			
			Differentiates between DL and UL transmission 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                              0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                                 33
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                                 33
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                                0x0000000200000000


/* Description		DOT11AX_RECEIVED_BSS_COLOR_ID

			This field is only valid for pkt_type == 11ax
			
			BSS_color_id from the received frame
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET                            0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                               34
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                               39
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                              0x000000fc00000000


/* Description		DOT11AX_RECEIVED_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			Spatial reuse from the received frame
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET                           0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                              40
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                              43
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK                             0x00000f0000000000


/* Description		DOT11AX_RECEIVED_CP_SIZE

			This field is only valid for pkt_type == 11ax
			
			CP size of the received frame
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			
			<legal 0 - 3>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                                 0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                                    44
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                                    45
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                                   0x0000300000000000


/* Description		DOT11AX_RECEIVED_LTF_SIZE

			This field is only valid for pkt_type == 11ax
			
			LTF size of the received frame
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                                0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                                   46
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                                   47
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                                  0x0000c00000000000


/* Description		DOT11AX_RECEIVED_CODING

			This field is only valid for pkt_type == 11ax
			
			Coding from the received frame
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_OFFSET                                  0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_LSB                                     48
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MSB                                     48
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MASK                                    0x0001000000000000


/* Description		DOT11AX_RECEIVED_DCM

			This field is only valid for pkt_type == 11ax
			
			DCM from the received frame
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_OFFSET                                     0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_LSB                                        49
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MSB                                        49
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MASK                                       0x0002000000000000


/* Description		DOT11AX_RECEIVED_DOPPLER_INDICATION

			This field is only valid for pkt_type == 11ax
			
			Doppler_indication from the received frame
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET                      0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB                         50
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB                         50
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK                        0x0004000000000000


/* Description		DOT11AX_RECEIVED_EXT_RU_SIZE

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be AND dot11ax_su_extended is set
			The number of (basic) RUs in this extended range reception
			
			
			RXPCU gets this from the received HE_SIG_A
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET                             0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                                51
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                                54
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                               0x0078000000000000


/* Description		DOT11AX_DL_UL_FLAG

			This field is only valid for pkt_type == 11ax
			
			DL_UL_flag to be used for response frame sent to this device.
			
			
			Differentiates between DL and UL transmission 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			Note: this setting can also come from response look-up table
			 in TXPCU...
			The selection is SW programmable
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_OFFSET                                       0x0000000000000028
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_LSB                                          55
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MSB                                          55
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MASK                                         0x0080000000000000


/* Description		RESERVED_11A

			<legal 0>
*/

#define TX_CBF_INFO_RESERVED_11A_OFFSET                                             0x0000000000000028
#define TX_CBF_INFO_RESERVED_11A_LSB                                                56
#define TX_CBF_INFO_RESERVED_11A_MSB                                                63
#define TX_CBF_INFO_RESERVED_11A_MASK                                               0xff00000000000000


/* Description		SW_RESPONSE_FRAME_LENGTH

			Field only valid when SW_Response_tlv_from_crypto is set
			
			
			This is the size of the frame (in bytes) that SW will generate
			 as the response frame. In those scenarios where TXPCU needs
			 to indicate a frame_length in the PDG_RESPONSE TLV, this
			 will be the value that TXPCU needs to use.
			
			Note that this length value  includes the FCS.
			<legal all>
*/

#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                                 0x0000000000000030
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                                    0
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                                    15
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                                   0x000000000000ffff


/* Description		SW_RESPONSE_TLV_FROM_CRYPTO

			Field can only be set by MAC mitigation logic
			
			The idea is here that normally TXPCU generates the response
			 frame.
			But as a backup scenario, in case of a last moment some 
			CBF frame BA format change happens or there is some other
			 issue, the CBF frame could be fully generated in the MAC
			 micro CPU and pushed into TXPCU through the Crypto - TXPCU
			 TLV interface.
			
			From TXPCU perspective, all interaction with PDG remains
			 exactly the same, accept that the frame length is now coming
			 from field SW_Response_frame_length and the response frame
			 is pushed into TXPCU over the CRYPTO - TXPCU TLV interface
			
			
			When set, this feature kick in
			When clear, this feature is not enabled
			<legal all>
*/

#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                              0x0000000000000030
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                                0x0000000000010000


/* Description		WAIT_SIFS_CONFIG_VALID

			When set, TXPCU shall follow the wait_sifs configuration.
			
			
			<legal all>
*/

#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                                   0x0000000000000030
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_LSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MASK                                     0x0000000000020000


/* Description		WAIT_SIFS

			Indicates to the TXPCU how precise the SIFS the response
			 timing shall be...
			
			The configuration for this is coming from SW programmable
			 registers in RXPCU, where RXPCU shall allow SW to program
			 different settings for the following scenarios: BRPOLL, 
			NDPA-NDP, 11ax trigger frame based BRPOLL
			
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

#define TX_CBF_INFO_WAIT_SIFS_OFFSET                                                0x0000000000000030
#define TX_CBF_INFO_WAIT_SIFS_LSB                                                   18
#define TX_CBF_INFO_WAIT_SIFS_MSB                                                   19
#define TX_CBF_INFO_WAIT_SIFS_MASK                                                  0x00000000000c0000


/* Description		RANGING

			0: This TLV is generated for Tx CBF generation.
			1: TLV is generated due to an active ranging session (.11az).
			
			<legal all>
*/

#define TX_CBF_INFO_RANGING_OFFSET                                                  0x0000000000000030
#define TX_CBF_INFO_RANGING_LSB                                                     20
#define TX_CBF_INFO_RANGING_MSB                                                     20
#define TX_CBF_INFO_RANGING_MASK                                                    0x0000000000100000


/* Description		SECURE

			Field only valid if Ranging is set to 1.
			0: Current ranging session is non-secure.
			1: Current ranging session is secure.
			<legal all>
*/

#define TX_CBF_INFO_SECURE_OFFSET                                                   0x0000000000000030
#define TX_CBF_INFO_SECURE_LSB                                                      21
#define TX_CBF_INFO_SECURE_MSB                                                      21
#define TX_CBF_INFO_SECURE_MASK                                                     0x0000000000200000


/* Description		TB_RANGING_RESPONSE_REQUIRED

			Field only valid in case of TB Ranging
			<enum 0 No_TB_Ranging_Resp>
			<enum 1 CTS2S_Resp_to_TF_poll > DO NOT USE.
			<enum 2 LMR_Resp_to_TF_report> DO NOT USE.
			<enum 3 NDP_Resp_to_TF_sound> TXPCU to generate TB ranging
			 NDP in response 
			<legal 0-3>
*/

#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET                             0x0000000000000030
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                                22
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                                23
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                               0x0000000000c00000


/* Description		RESERVED_12A

			<legal 0>
*/

#define TX_CBF_INFO_RESERVED_12A_OFFSET                                             0x0000000000000030
#define TX_CBF_INFO_RESERVED_12A_LSB                                                24
#define TX_CBF_INFO_RESERVED_12A_MSB                                                25
#define TX_CBF_INFO_RESERVED_12A_MASK                                               0x0000000003000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			This field is only valid if Punctured_response is set
			
			The 6-bit value used in U-SIG and/or EHT-SIG Common field
			 for the puncture pattern
			<legal 0-29>
*/

#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                          0x0000000000000030
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                             26
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                             31
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                            0x00000000fc000000


/* Description		DOT11BE_PUNCTURE_BITMAP

			This field is only valid if Punctured_response is set
			
			The bitmap of 20 MHz sub-bands valid in this EHT reception
			
			
			RXPCU gets this from the received U-SIG and/or EHT-SIG via
			 PHY microcode.
			
			<legal all>
*/

#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                                  0x0000000000000030
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                                     32
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                                     47
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                                    0x0000ffff00000000


/* Description		DOT11BE_RESPONSE

			Indicates that the peer supports .11be response protocols, 
			e.g. .11be BW indication in scrambler seed, .11be dynamic
			 BW procedure, punctured response, etc.
*/

#define TX_CBF_INFO_DOT11BE_RESPONSE_OFFSET                                         0x0000000000000030
#define TX_CBF_INFO_DOT11BE_RESPONSE_LSB                                            48
#define TX_CBF_INFO_DOT11BE_RESPONSE_MSB                                            48
#define TX_CBF_INFO_DOT11BE_RESPONSE_MASK                                           0x0001000000000000


/* Description		PUNCTURED_RESPONSE

			Field only valid if Dot11be_response is set
			
			Indicates that the response shall use preamble puncturing
			
*/

#define TX_CBF_INFO_PUNCTURED_RESPONSE_OFFSET                                       0x0000000000000030
#define TX_CBF_INFO_PUNCTURED_RESPONSE_LSB                                          49
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MSB                                          49
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MASK                                         0x0002000000000000


/* Description		NPDA_INFO_11BE_VALID

			When set, 11be_NDPA info is valid.
			TXPCU will have to trigger the PDG for response transmission
			 .
			
			PHY's response will be in 11be format.
			<legal all>
*/

#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_OFFSET                                     0x0000000000000030
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_LSB                                        50
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MSB                                        50
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MASK                                       0x0004000000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define TX_CBF_INFO_EHT_DUPLICATE_MODE_OFFSET                                       0x0000000000000030
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_LSB                                          51
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MSB                                          52
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MASK                                         0x0018000000000000


/* Description		RESERVED_13A

			<legal 0>
*/

#define TX_CBF_INFO_RESERVED_13A_OFFSET                                             0x0000000000000030
#define TX_CBF_INFO_RESERVED_13A_LSB                                                53
#define TX_CBF_INFO_RESERVED_13A_MSB                                                63
#define TX_CBF_INFO_RESERVED_13A_MASK                                               0xffe0000000000000


/* Description		EHT_STA_INFO_39_32

			Field only valid when Npda_info_11be_valid is set
			
			The fifth 8 bits of the RAW EHT STA INFO field in the NDPA
			 frame
*/

#define TX_CBF_INFO_EHT_STA_INFO_39_32_OFFSET                                       0x0000000000000038
#define TX_CBF_INFO_EHT_STA_INFO_39_32_LSB                                          0
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MSB                                          7
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MASK                                         0x00000000000000ff


/* Description		RESERVED_14A

			Can be used for future expansion
			<legal 0>
*/

#define TX_CBF_INFO_RESERVED_14A_OFFSET                                             0x0000000000000038
#define TX_CBF_INFO_RESERVED_14A_LSB                                                8
#define TX_CBF_INFO_RESERVED_14A_MSB                                                31
#define TX_CBF_INFO_RESERVED_14A_MASK                                               0x00000000ffffff00


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define TX_CBF_INFO_TLV64_PADDING_OFFSET                                            0x0000000000000038
#define TX_CBF_INFO_TLV64_PADDING_LSB                                               32
#define TX_CBF_INFO_TLV64_PADDING_MSB                                               63
#define TX_CBF_INFO_TLV64_PADDING_MASK                                              0xffffffff00000000



#endif   // TX_CBF_INFO
