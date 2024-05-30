
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

 
 
 
 
 
 
 


#ifndef _RESPONSE_END_STATUS_H_
#define _RESPONSE_END_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_RESPONSE_END_STATUS 22

#define NUM_OF_QWORDS_RESPONSE_END_STATUS 11


struct response_end_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t coex_bt_tx_while_wlan_tx                                :  1, // [0:0]
                      coex_wan_tx_while_wlan_tx                               :  1, // [1:1]
                      coex_wlan_tx_while_wlan_tx                              :  1, // [2:2]
                      global_data_underflow_warning                           :  1, // [3:3]
                      response_transmit_status                                :  4, // [7:4]
                      phytx_pkt_end_info_valid                                :  1, // [8:8]
                      phytx_abort_request_info_valid                          :  1, // [9:9]
                      generated_response                                      :  3, // [12:10]
                      mba_user_count                                          :  7, // [19:13]
                      mba_fake_bitmap_count                                   :  7, // [26:20]
                      coex_based_tx_bw                                        :  3, // [29:27]
                      trig_response_related                                   :  1, // [30:30]
                      dpdtrain_done                                           :  1; // [31:31]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t cbf_segment_request_mask                                :  8, // [23:16]
                      cbf_segment_sent_mask                                   :  8; // [31:24]
             uint32_t underflow_mpdu_count                                    :  9, // [8:0]
                      data_underflow_warning                                  :  2, // [10:9]
                      phy_tx_gain_setting                                     :  8, // [18:11]
                      timing_status                                           :  2, // [20:19]
                      only_null_delim_sent                                    :  1, // [21:21]
                      brp_info_valid                                          :  1, // [22:22]
                      reserved_2a                                             :  9; // [31:23]
             uint32_t mu_response_bitmap_31_0                                 : 32; // [31:0]
             uint32_t mu_response_bitmap_36_32                                :  5, // [4:0]
                      reserved_4a                                             : 11, // [15:5]
                      transmit_delay                                          : 16; // [31:16]
             uint32_t start_of_frame_timestamp_15_0                           : 16, // [15:0]
                      start_of_frame_timestamp_31_16                          : 16; // [31:16]
             uint32_t end_of_frame_timestamp_15_0                             : 16, // [15:0]
                      end_of_frame_timestamp_31_16                            : 16; // [31:16]
             uint32_t tx_group_delay                                          : 12, // [11:0]
                      reserved_7a                                             :  4, // [15:12]
                      tpc_dbg_info_cmn_15_0                                   : 16; // [31:16]
             uint32_t tpc_dbg_info_31_16                                      : 16, // [15:0]
                      tpc_dbg_info_47_32                                      : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_15_0                                  : 16, // [15:0]
                      tpc_dbg_info_chn1_31_16                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_47_32                                 : 16, // [15:0]
                      tpc_dbg_info_chn1_63_48                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_79_64                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_15_0                                  : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_31_16                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_47_32                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_63_48                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_79_64                                 : 16; // [31:16]
             uint32_t phytx_tx_end_sw_info_15_0                               : 16, // [15:0]
                      phytx_tx_end_sw_info_31_16                              : 16; // [31:16]
             uint32_t phytx_tx_end_sw_info_47_32                              : 16, // [15:0]
                      phytx_tx_end_sw_info_63_48                              : 16; // [31:16]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr1_47_32                                             : 16, // [15:0]
                      addr2_15_0                                              : 16; // [31:16]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t addr3_31_0                                              : 32; // [31:0]
             uint32_t addr3_47_32                                             : 16, // [15:0]
                      ranging                                                 :  1, // [16:16]
                      secure                                                  :  1, // [17:17]
                      ranging_ftm_frame_sent                                  :  1, // [18:18]
                      reserved_20a                                            : 13; // [31:19]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t dpdtrain_done                                           :  1, // [31:31]
                      trig_response_related                                   :  1, // [30:30]
                      coex_based_tx_bw                                        :  3, // [29:27]
                      mba_fake_bitmap_count                                   :  7, // [26:20]
                      mba_user_count                                          :  7, // [19:13]
                      generated_response                                      :  3, // [12:10]
                      phytx_abort_request_info_valid                          :  1, // [9:9]
                      phytx_pkt_end_info_valid                                :  1, // [8:8]
                      response_transmit_status                                :  4, // [7:4]
                      global_data_underflow_warning                           :  1, // [3:3]
                      coex_wlan_tx_while_wlan_tx                              :  1, // [2:2]
                      coex_wan_tx_while_wlan_tx                               :  1, // [1:1]
                      coex_bt_tx_while_wlan_tx                                :  1; // [0:0]
             uint32_t cbf_segment_sent_mask                                   :  8, // [31:24]
                      cbf_segment_request_mask                                :  8; // [23:16]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t reserved_2a                                             :  9, // [31:23]
                      brp_info_valid                                          :  1, // [22:22]
                      only_null_delim_sent                                    :  1, // [21:21]
                      timing_status                                           :  2, // [20:19]
                      phy_tx_gain_setting                                     :  8, // [18:11]
                      data_underflow_warning                                  :  2, // [10:9]
                      underflow_mpdu_count                                    :  9; // [8:0]
             uint32_t mu_response_bitmap_31_0                                 : 32; // [31:0]
             uint32_t transmit_delay                                          : 16, // [31:16]
                      reserved_4a                                             : 11, // [15:5]
                      mu_response_bitmap_36_32                                :  5; // [4:0]
             uint32_t start_of_frame_timestamp_31_16                          : 16, // [31:16]
                      start_of_frame_timestamp_15_0                           : 16; // [15:0]
             uint32_t end_of_frame_timestamp_31_16                            : 16, // [31:16]
                      end_of_frame_timestamp_15_0                             : 16; // [15:0]
             uint32_t tpc_dbg_info_cmn_15_0                                   : 16, // [31:16]
                      reserved_7a                                             :  4, // [15:12]
                      tx_group_delay                                          : 12; // [11:0]
             uint32_t tpc_dbg_info_47_32                                      : 16, // [31:16]
                      tpc_dbg_info_31_16                                      : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_31_16                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_15_0                                  : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_63_48                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_47_32                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_15_0                                  : 16, // [31:16]
                      tpc_dbg_info_chn1_79_64                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_47_32                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_31_16                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_79_64                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_63_48                                 : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_31_16                              : 16, // [31:16]
                      phytx_tx_end_sw_info_15_0                               : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_63_48                              : 16, // [31:16]
                      phytx_tx_end_sw_info_47_32                              : 16; // [15:0]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr2_15_0                                              : 16, // [31:16]
                      addr1_47_32                                             : 16; // [15:0]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t addr3_31_0                                              : 32; // [31:0]
             uint32_t reserved_20a                                            : 13, // [31:19]
                      ranging_ftm_frame_sent                                  :  1, // [18:18]
                      secure                                                  :  1, // [17:17]
                      ranging                                                 :  1, // [16:16]
                      addr3_47_32                                             : 16; // [15:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		COEX_BT_TX_WHILE_WLAN_TX

			When set, a BT tx coex event started while wlan was in the
			 middle of response transmission.
			
			Field set when coex_status_broadcast TLV received with bt
			 tx activity set and WLAN tx ongoing.
			<legal all>
*/

#define RESPONSE_END_STATUS_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                         0x0000000000000000
#define RESPONSE_END_STATUS_COEX_BT_TX_WHILE_WLAN_TX_LSB                            0
#define RESPONSE_END_STATUS_COEX_BT_TX_WHILE_WLAN_TX_MSB                            0
#define RESPONSE_END_STATUS_COEX_BT_TX_WHILE_WLAN_TX_MASK                           0x0000000000000001


/* Description		COEX_WAN_TX_WHILE_WLAN_TX

			When set, a WAN tx coex event started while wlan was in 
			the middle of response transmission.
			
			Field set when coex_status_broadcast TLV received with WAN
			 tx activity set and WLAN tx ongoing
			<legal all>
*/

#define RESPONSE_END_STATUS_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                        0x0000000000000000
#define RESPONSE_END_STATUS_COEX_WAN_TX_WHILE_WLAN_TX_LSB                           1
#define RESPONSE_END_STATUS_COEX_WAN_TX_WHILE_WLAN_TX_MSB                           1
#define RESPONSE_END_STATUS_COEX_WAN_TX_WHILE_WLAN_TX_MASK                          0x0000000000000002


/* Description		COEX_WLAN_TX_WHILE_WLAN_TX

			When set, a WLAN tx coex event started while wlan was in
			 the middle of response transmission.
			
			Field set when coex_status_broadcast TLV received with WLAN
			 tx activity set and WLAN tx ongoing
			<legal all>
*/

#define RESPONSE_END_STATUS_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                       0x0000000000000000
#define RESPONSE_END_STATUS_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                          2
#define RESPONSE_END_STATUS_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                          2
#define RESPONSE_END_STATUS_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                         0x0000000000000004


/* Description		GLOBAL_DATA_UNDERFLOW_WARNING

			Consumer: SCH/SW
			Producer: TXPCU
			
			When set, during response transmission a data underflow 
			occurred for one or more users.<legal all>
*/

#define RESPONSE_END_STATUS_GLOBAL_DATA_UNDERFLOW_WARNING_OFFSET                    0x0000000000000000
#define RESPONSE_END_STATUS_GLOBAL_DATA_UNDERFLOW_WARNING_LSB                       3
#define RESPONSE_END_STATUS_GLOBAL_DATA_UNDERFLOW_WARNING_MSB                       3
#define RESPONSE_END_STATUS_GLOBAL_DATA_UNDERFLOW_WARNING_MASK                      0x0000000000000008


/* Description		RESPONSE_TRANSMIT_STATUS

			<enum 0 response_ok> Successful transmission of the selfgen
			 response frame
			<enum 1 response_coex_soft_abort> Set if transmission is
			 terminated because of the coex soft abort.
			<enum 2 response_phy_err> Set if transmission is terminated
			 because PHY generated an abort request
			<enum 3 response_flush_received> Set if transmission is 
			terminated because RXPCU received a flush request
			<enum 4 response_other_err> Set if transmission is terminated
			 because of other errors within the RXPCU
			<legal 0-4>
*/

#define RESPONSE_END_STATUS_RESPONSE_TRANSMIT_STATUS_OFFSET                         0x0000000000000000
#define RESPONSE_END_STATUS_RESPONSE_TRANSMIT_STATUS_LSB                            4
#define RESPONSE_END_STATUS_RESPONSE_TRANSMIT_STATUS_MSB                            7
#define RESPONSE_END_STATUS_RESPONSE_TRANSMIT_STATUS_MASK                           0x00000000000000f0


/* Description		PHYTX_PKT_END_INFO_VALID

			All the fields originating from PHYTX_PKT_END TLV contain
			 valid info
			
			Note that when "trig_response_related" is set, this bit 
			will often not be set as the trigger response contents might
			 have come from a scheduling command which is not reported
			 as part of the 'response' transmission.
*/

#define RESPONSE_END_STATUS_PHYTX_PKT_END_INFO_VALID_OFFSET                         0x0000000000000000
#define RESPONSE_END_STATUS_PHYTX_PKT_END_INFO_VALID_LSB                            8
#define RESPONSE_END_STATUS_PHYTX_PKT_END_INFO_VALID_MSB                            8
#define RESPONSE_END_STATUS_PHYTX_PKT_END_INFO_VALID_MASK                           0x0000000000000100


/* Description		PHYTX_ABORT_REQUEST_INFO_VALID

			Field Phytx_abort_request_info_details contains valid info
			
*/

#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                   0x0000000000000000
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                      9
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                      9
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                     0x0000000000000200


/* Description		GENERATED_RESPONSE

			The generated response frame
			
			<enum 0 selfgen_ACK> TXPCU generated an ACK response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 1 selfgen_CTS> TXPCU generated an CTS response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 2 selfgen_BA> TXPCU generated a BA response. Note 
			that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 3 selfgen_MBA> TXPCU generated an M BA response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 4 selfgen_CBF> TXPCU generated a CBF response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 5 selfgen_other_trig_response>
			TXPCU generated a trigger related response of a type not
			 specified above. Note that in this case bit trig_response_related
			 will be set as well.
			This e-num will also be used when TXPCU has been programmed
			 to overwrite it's own self gen response generation, and
			 wait for the response to come from SCH..
			Also applicable for basic trigger response. 
			
			<enum 6 selfgen_NDP_LMR> TXPCU generated a self-gen NDP 
			followed by a self-gen LMR for the ranging NDPA followed
			 by NDP received by RXPCU.
			
			<legal 0-6>
*/

#define RESPONSE_END_STATUS_GENERATED_RESPONSE_OFFSET                               0x0000000000000000
#define RESPONSE_END_STATUS_GENERATED_RESPONSE_LSB                                  10
#define RESPONSE_END_STATUS_GENERATED_RESPONSE_MSB                                  12
#define RESPONSE_END_STATUS_GENERATED_RESPONSE_MASK                                 0x0000000000001c00


/* Description		MBA_USER_COUNT

			Field only valid in case of selfgen_MBA
			
			The number of users included in the generated MBA 
			
			Note that this value will be the same as in TLV/field: RESPONSE_START_STATUS.response_STA_count
			
			
			<legal all>
*/

#define RESPONSE_END_STATUS_MBA_USER_COUNT_OFFSET                                   0x0000000000000000
#define RESPONSE_END_STATUS_MBA_USER_COUNT_LSB                                      13
#define RESPONSE_END_STATUS_MBA_USER_COUNT_MSB                                      19
#define RESPONSE_END_STATUS_MBA_USER_COUNT_MASK                                     0x00000000000fe000


/* Description		MBA_FAKE_BITMAP_COUNT

			Field only valid in case of MU OFDMA selfgen_MBA
			
			The number of users for which RXPCU did not have a bitmap, 
			and thus provided a 'fake bitmap'
			<legal all>
*/

#define RESPONSE_END_STATUS_MBA_FAKE_BITMAP_COUNT_OFFSET                            0x0000000000000000
#define RESPONSE_END_STATUS_MBA_FAKE_BITMAP_COUNT_LSB                               20
#define RESPONSE_END_STATUS_MBA_FAKE_BITMAP_COUNT_MSB                               26
#define RESPONSE_END_STATUS_MBA_FAKE_BITMAP_COUNT_MASK                              0x0000000007f00000


/* Description		COEX_BASED_TX_BW

			This is the transmit bandwidth value
			that is granted by Coex for the response frame
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RESPONSE_END_STATUS_COEX_BASED_TX_BW_OFFSET                                 0x0000000000000000
#define RESPONSE_END_STATUS_COEX_BASED_TX_BW_LSB                                    27
#define RESPONSE_END_STATUS_COEX_BASED_TX_BW_MSB                                    29
#define RESPONSE_END_STATUS_COEX_BASED_TX_BW_MASK                                   0x0000000038000000


/* Description		TRIG_RESPONSE_RELATED

			When set, this TLV is generated by TXPCU in the context 
			of a response transmission to a received trigger frame.
			
			<legal all>
*/

#define RESPONSE_END_STATUS_TRIG_RESPONSE_RELATED_OFFSET                            0x0000000000000000
#define RESPONSE_END_STATUS_TRIG_RESPONSE_RELATED_LSB                               30
#define RESPONSE_END_STATUS_TRIG_RESPONSE_RELATED_MSB                               30
#define RESPONSE_END_STATUS_TRIG_RESPONSE_RELATED_MASK                              0x0000000040000000


/* Description		DPDTRAIN_DONE

			Field only valid when PHYTX_PKT_END_info_valid is set
			
			For DPD Training packets, this bit is set to indicate that
			 DPD Training was successfully run to completion.  Also 
			reused by Implicit BF Calibration Packets. This bit is intended
			 for debug purposes.
			<legal all>
*/

#define RESPONSE_END_STATUS_DPDTRAIN_DONE_OFFSET                                    0x0000000000000000
#define RESPONSE_END_STATUS_DPDTRAIN_DONE_LSB                                       31
#define RESPONSE_END_STATUS_DPDTRAIN_DONE_MSB                                       31
#define RESPONSE_END_STATUS_DPDTRAIN_DONE_MASK                                      0x0000000080000000


/* Description		PHYTX_ABORT_REQUEST_INFO_DETAILS

			Field only valid when PHYTX_ABORT_REQUEST_info_valid is 
			set
			
			The reason why PHYTX is requested an abort
*/


/* Description		PHYTX_ABORT_REASON

			Reason for early termination of TX packet by the PHY 
			
			<enum_type PHYTX_ABORT_ENUM>
*/

#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000000
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB 32
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB 39
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK 0x000000ff00000000


/* Description		USER_NUMBER

			For some errors, the user for which this error was detected
			 can be indicated in this field.
			<legal 0-36>
*/

#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET     0x0000000000000000
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB        40
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB        45
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK       0x00003f0000000000


/* Description		RESERVED

			<legal 0>
*/

#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET        0x0000000000000000
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB           46
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB           47
#define RESPONSE_END_STATUS_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK          0x0000c00000000000


/* Description		CBF_SEGMENT_REQUEST_MASK

			Field only valid when brp_info_valid is set.
			
			Field equal to the 'Feedback Segment Retransmission Bitmap' 
			from the Beamform Report Poll frame OR Beamform Report Poll
			 Trigger frame
			
			Bit 0 represents segment 0
			Bit 1 represents segment 1
			Etc.
			
			1'b1: Segment is requested
			1'b0: Segment is NOT requested
			
			<legal all>
*/

#define RESPONSE_END_STATUS_CBF_SEGMENT_REQUEST_MASK_OFFSET                         0x0000000000000000
#define RESPONSE_END_STATUS_CBF_SEGMENT_REQUEST_MASK_LSB                            48
#define RESPONSE_END_STATUS_CBF_SEGMENT_REQUEST_MASK_MSB                            55
#define RESPONSE_END_STATUS_CBF_SEGMENT_REQUEST_MASK_MASK                           0x00ff000000000000


/* Description		CBF_SEGMENT_SENT_MASK

			Field only valid when brp_info_valid is set.
			
			Bit 0 represents segment 0
			Bit 1 represents segment 1
			Etc.
			
			1'b1: Segment is sent
			1'b0: Segment is not sent
			
			<legal all>
*/

#define RESPONSE_END_STATUS_CBF_SEGMENT_SENT_MASK_OFFSET                            0x0000000000000000
#define RESPONSE_END_STATUS_CBF_SEGMENT_SENT_MASK_LSB                               56
#define RESPONSE_END_STATUS_CBF_SEGMENT_SENT_MASK_MSB                               63
#define RESPONSE_END_STATUS_CBF_SEGMENT_SENT_MASK_MASK                              0xff00000000000000


/* Description		UNDERFLOW_MPDU_COUNT

			The MPDU count transmitted when the first underrun condition
			 was detected
			<legal 0-256>
*/

#define RESPONSE_END_STATUS_UNDERFLOW_MPDU_COUNT_OFFSET                             0x0000000000000008
#define RESPONSE_END_STATUS_UNDERFLOW_MPDU_COUNT_LSB                                0
#define RESPONSE_END_STATUS_UNDERFLOW_MPDU_COUNT_MSB                                8
#define RESPONSE_END_STATUS_UNDERFLOW_MPDU_COUNT_MASK                               0x00000000000001ff


/* Description		DATA_UNDERFLOW_WARNING

			Mac data underflow warning 
			
			<enum 0 no_data_underrun> No data underflow
			<enum 1 data_underrun_between_mpdu> PCU experienced data
			 underflow in between MPDUs
			<enum 2 data_underrun_within_mpdu> PCU experienced data 
			underflow within an MPDU
			<legal 0-2>
*/

#define RESPONSE_END_STATUS_DATA_UNDERFLOW_WARNING_OFFSET                           0x0000000000000008
#define RESPONSE_END_STATUS_DATA_UNDERFLOW_WARNING_LSB                              9
#define RESPONSE_END_STATUS_DATA_UNDERFLOW_WARNING_MSB                              10
#define RESPONSE_END_STATUS_DATA_UNDERFLOW_WARNING_MASK                             0x0000000000000600


/* Description		PHY_TX_GAIN_SETTING

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			The gain setting that the PHY used for this last PPDU transmission
			
*/

#define RESPONSE_END_STATUS_PHY_TX_GAIN_SETTING_OFFSET                              0x0000000000000008
#define RESPONSE_END_STATUS_PHY_TX_GAIN_SETTING_LSB                                 11
#define RESPONSE_END_STATUS_PHY_TX_GAIN_SETTING_MSB                                 18
#define RESPONSE_END_STATUS_PHY_TX_GAIN_SETTING_MASK                                0x000000000007f800


/* Description		TIMING_STATUS

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			<enum 0 No_tx_timing_request> The MAC did not request for
			 the transmission to start at a particular time
			<enum 1 successful_tx_timing > MAC did request for transmission
			 to start at a particular time and PHY was able to do so.
			
			<enum 2 tx_timing_not_honoured> PHY was not able to honour
			 the requested transmit time by the MAC. The transmission
			 started later, and field transmit_delay indicates how much
			 later.
			<legal 0-2>
*/

#define RESPONSE_END_STATUS_TIMING_STATUS_OFFSET                                    0x0000000000000008
#define RESPONSE_END_STATUS_TIMING_STATUS_LSB                                       19
#define RESPONSE_END_STATUS_TIMING_STATUS_MSB                                       20
#define RESPONSE_END_STATUS_TIMING_STATUS_MASK                                      0x0000000000180000


/* Description		ONLY_NULL_DELIM_SENT

			Field only valid when "trig_response_related" is set.
			
			When set, TXPCU only sent NULL delimiters to the PHY for
			 the entire duration of the trigger response time.
			
			Note that SCH does not evaluate this field. It is only for
			 SW to look at.
			
			Setting this bit can only happen when a trigger is received, 
			and either the trigger allocated an incorrectly small duration, 
			or SW had not programmed a response scheduler command in
			 time to respond, which may not comply with the 11ax IEEE
			 spec.
			
			<legal all>
*/

#define RESPONSE_END_STATUS_ONLY_NULL_DELIM_SENT_OFFSET                             0x0000000000000008
#define RESPONSE_END_STATUS_ONLY_NULL_DELIM_SENT_LSB                                21
#define RESPONSE_END_STATUS_ONLY_NULL_DELIM_SENT_MSB                                21
#define RESPONSE_END_STATUS_ONLY_NULL_DELIM_SENT_MASK                               0x0000000000200000


/* Description		BRP_INFO_VALID

			When set, TXPCU sent CBF segments.
			
			Fields cbf_segment_request_mask and cbf_segment_sent_mask
			 contain valid info.
			
			<legal all>
*/

#define RESPONSE_END_STATUS_BRP_INFO_VALID_OFFSET                                   0x0000000000000008
#define RESPONSE_END_STATUS_BRP_INFO_VALID_LSB                                      22
#define RESPONSE_END_STATUS_BRP_INFO_VALID_MSB                                      22
#define RESPONSE_END_STATUS_BRP_INFO_VALID_MASK                                     0x0000000000400000


/* Description		RESERVED_2A

			<legal 0>
*/

#define RESPONSE_END_STATUS_RESERVED_2A_OFFSET                                      0x0000000000000008
#define RESPONSE_END_STATUS_RESERVED_2A_LSB                                         23
#define RESPONSE_END_STATUS_RESERVED_2A_MSB                                         31
#define RESPONSE_END_STATUS_RESERVED_2A_MASK                                        0x00000000ff800000


/* Description		MU_RESPONSE_BITMAP_31_0

			Bit 0 represents user 0
			Bit 1 represents user 1
			...
			When set, at least 1 MPDU from this user has been properly
			 received => FCS OK
			
			TODO: remove these
			Field can not be filled in with the self generated response
			
*/

#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_31_0_OFFSET                          0x0000000000000008
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_31_0_LSB                             32
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_31_0_MSB                             63
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_31_0_MASK                            0xffffffff00000000


/* Description		MU_RESPONSE_BITMAP_36_32

			Bit 0 represents user 32
			Bit 1 represents user 33
			...
			When set, at least 1 MPDU from this user has been properly
			 received => FCS OK
			TODO: remove these
			Field can not be filled in with the self generated response
			
			Note: Received_response already goes to SW, so probably 
			no need to copy this bitmap info to TX_FES_STATUS TLV.
*/

#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_36_32_OFFSET                         0x0000000000000010
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_36_32_LSB                            0
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_36_32_MSB                            4
#define RESPONSE_END_STATUS_MU_RESPONSE_BITMAP_36_32_MASK                           0x000000000000001f


/* Description		RESERVED_4A

			<legal 0>
*/

#define RESPONSE_END_STATUS_RESERVED_4A_OFFSET                                      0x0000000000000010
#define RESPONSE_END_STATUS_RESERVED_4A_LSB                                         5
#define RESPONSE_END_STATUS_RESERVED_4A_MSB                                         15
#define RESPONSE_END_STATUS_RESERVED_4A_MASK                                        0x000000000000ffe0


/* Description		TRANSMIT_DELAY

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			The number of 480 MHz clock cycles that the transmission
			 started after the actual requested transmit start time.
			
			Value saturates at 0xFFFF
			<legal all>
*/

#define RESPONSE_END_STATUS_TRANSMIT_DELAY_OFFSET                                   0x0000000000000010
#define RESPONSE_END_STATUS_TRANSMIT_DELAY_LSB                                      16
#define RESPONSE_END_STATUS_TRANSMIT_DELAY_MSB                                      31
#define RESPONSE_END_STATUS_TRANSMIT_DELAY_MASK                                     0x00000000ffff0000


/* Description		START_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all> 
*/

#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                    0x0000000000000010
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_15_0_LSB                       32
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_15_0_MSB                       47
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_15_0_MASK                      0x0000ffff00000000


/* Description		START_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all>
*/

#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                   0x0000000000000010
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_31_16_LSB                      48
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_31_16_MSB                      63
#define RESPONSE_END_STATUS_START_OF_FRAME_TIMESTAMP_31_16_MASK                     0xffff000000000000


/* Description		END_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all> 
*/

#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                      0x0000000000000018
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_15_0_LSB                         0
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_15_0_MSB                         15
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_15_0_MASK                        0x000000000000ffff


/* Description		END_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all>
*/

#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                     0x0000000000000018
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_31_16_LSB                        16
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_31_16_MSB                        31
#define RESPONSE_END_STATUS_END_OF_FRAME_TIMESTAMP_31_16_MASK                       0x00000000ffff0000


/* Description		TX_GROUP_DELAY

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Group delay on TxTD+PHYRF path for this PPDU (packet BW 
			dependent), useful for RTT
			
			Unit is 960MHz cycles.
			<legal all>
*/

#define RESPONSE_END_STATUS_TX_GROUP_DELAY_OFFSET                                   0x0000000000000018
#define RESPONSE_END_STATUS_TX_GROUP_DELAY_LSB                                      32
#define RESPONSE_END_STATUS_TX_GROUP_DELAY_MSB                                      43
#define RESPONSE_END_STATUS_TX_GROUP_DELAY_MASK                                     0x00000fff00000000


/* Description		RESERVED_7A

			<legal 0>
*/

#define RESPONSE_END_STATUS_RESERVED_7A_OFFSET                                      0x0000000000000018
#define RESPONSE_END_STATUS_RESERVED_7A_LSB                                         44
#define RESPONSE_END_STATUS_RESERVED_7A_MSB                                         47
#define RESPONSE_END_STATUS_RESERVED_7A_MASK                                        0x0000f00000000000


/* Description		TPC_DBG_INFO_CMN_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CMN_15_0_OFFSET                            0x0000000000000018
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CMN_15_0_LSB                               48
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CMN_15_0_MSB                               63
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CMN_15_0_MASK                              0xffff000000000000


/* Description		TPC_DBG_INFO_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_31_16_OFFSET                               0x0000000000000020
#define RESPONSE_END_STATUS_TPC_DBG_INFO_31_16_LSB                                  0
#define RESPONSE_END_STATUS_TPC_DBG_INFO_31_16_MSB                                  15
#define RESPONSE_END_STATUS_TPC_DBG_INFO_31_16_MASK                                 0x000000000000ffff


/* Description		TPC_DBG_INFO_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug infothat PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_47_32_OFFSET                               0x0000000000000020
#define RESPONSE_END_STATUS_TPC_DBG_INFO_47_32_LSB                                  16
#define RESPONSE_END_STATUS_TPC_DBG_INFO_47_32_MSB                                  31
#define RESPONSE_END_STATUS_TPC_DBG_INFO_47_32_MASK                                 0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN1_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_15_0_OFFSET                           0x0000000000000020
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_15_0_LSB                              32
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_15_0_MSB                              47
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_15_0_MASK                             0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN1_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_31_16_OFFSET                          0x0000000000000020
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_31_16_LSB                             48
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_31_16_MSB                             63
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_31_16_MASK                            0xffff000000000000


/* Description		TPC_DBG_INFO_CHN1_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_47_32_OFFSET                          0x0000000000000028
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_47_32_LSB                             0
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_47_32_MSB                             15
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_47_32_MASK                            0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN1_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_63_48_OFFSET                          0x0000000000000028
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_63_48_LSB                             16
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_63_48_MSB                             31
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_63_48_MASK                            0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN1_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_79_64_OFFSET                          0x0000000000000028
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_79_64_LSB                             32
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_79_64_MSB                             47
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN1_79_64_MASK                            0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN2_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_15_0_OFFSET                           0x0000000000000028
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_15_0_LSB                              48
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_15_0_MSB                              63
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_15_0_MASK                             0xffff000000000000


/* Description		TPC_DBG_INFO_CHN2_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_31_16_OFFSET                          0x0000000000000030
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_31_16_LSB                             0
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_31_16_MSB                             15
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_31_16_MASK                            0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN2_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_47_32_OFFSET                          0x0000000000000030
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_47_32_LSB                             16
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_47_32_MSB                             31
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_47_32_MASK                            0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN2_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_63_48_OFFSET                          0x0000000000000030
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_63_48_LSB                             32
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_63_48_MSB                             47
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_63_48_MASK                            0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN2_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_79_64_OFFSET                          0x0000000000000030
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_79_64_LSB                             48
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_79_64_MSB                             63
#define RESPONSE_END_STATUS_TPC_DBG_INFO_CHN2_79_64_MASK                            0xffff000000000000


/* Description		PHYTX_TX_END_SW_INFO_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_15_0_OFFSET                        0x0000000000000038
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_15_0_LSB                           0
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_15_0_MSB                           15
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_15_0_MASK                          0x000000000000ffff


/* Description		PHYTX_TX_END_SW_INFO_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_31_16_OFFSET                       0x0000000000000038
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_31_16_LSB                          16
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_31_16_MSB                          31
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_31_16_MASK                         0x00000000ffff0000


/* Description		PHYTX_TX_END_SW_INFO_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_47_32_OFFSET                       0x0000000000000038
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_47_32_LSB                          32
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_47_32_MSB                          47
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_47_32_MASK                         0x0000ffff00000000


/* Description		PHYTX_TX_END_SW_INFO_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_63_48_OFFSET                       0x0000000000000038
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_63_48_LSB                          48
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_63_48_MSB                          63
#define RESPONSE_END_STATUS_PHYTX_TX_END_SW_INFO_63_48_MASK                         0xffff000000000000


/* Description		ADDR1_31_0

			To be copied over from RESPONSE_REQUIRED_INFO or TX_CBF_INFO
			
*/

#define RESPONSE_END_STATUS_ADDR1_31_0_OFFSET                                       0x0000000000000040
#define RESPONSE_END_STATUS_ADDR1_31_0_LSB                                          0
#define RESPONSE_END_STATUS_ADDR1_31_0_MSB                                          31
#define RESPONSE_END_STATUS_ADDR1_31_0_MASK                                         0x00000000ffffffff


/* Description		ADDR1_47_32

			To be copied over from RESPONSE_REQUIRED_INFO or TX_CBF_INFO
			
*/

#define RESPONSE_END_STATUS_ADDR1_47_32_OFFSET                                      0x0000000000000040
#define RESPONSE_END_STATUS_ADDR1_47_32_LSB                                         32
#define RESPONSE_END_STATUS_ADDR1_47_32_MSB                                         47
#define RESPONSE_END_STATUS_ADDR1_47_32_MASK                                        0x0000ffff00000000


/* Description		ADDR2_15_0

			To be copied over from RESPONSE_REQUIRED_INFO or TX_CBF_INFO
			
*/

#define RESPONSE_END_STATUS_ADDR2_15_0_OFFSET                                       0x0000000000000040
#define RESPONSE_END_STATUS_ADDR2_15_0_LSB                                          48
#define RESPONSE_END_STATUS_ADDR2_15_0_MSB                                          63
#define RESPONSE_END_STATUS_ADDR2_15_0_MASK                                         0xffff000000000000


/* Description		ADDR2_47_16

			To be copied over from RESPONSE_REQUIRED_INFO or TX_CBF_INFO
			
*/

#define RESPONSE_END_STATUS_ADDR2_47_16_OFFSET                                      0x0000000000000048
#define RESPONSE_END_STATUS_ADDR2_47_16_LSB                                         0
#define RESPONSE_END_STATUS_ADDR2_47_16_MSB                                         31
#define RESPONSE_END_STATUS_ADDR2_47_16_MASK                                        0x00000000ffffffff


/* Description		ADDR3_31_0

			To be copied over from TX_CBF_INFO
*/

#define RESPONSE_END_STATUS_ADDR3_31_0_OFFSET                                       0x0000000000000048
#define RESPONSE_END_STATUS_ADDR3_31_0_LSB                                          32
#define RESPONSE_END_STATUS_ADDR3_31_0_MSB                                          63
#define RESPONSE_END_STATUS_ADDR3_31_0_MASK                                         0xffffffff00000000


/* Description		ADDR3_47_32

			To be copied over from TX_CBF_INFO
*/

#define RESPONSE_END_STATUS_ADDR3_47_32_OFFSET                                      0x0000000000000050
#define RESPONSE_END_STATUS_ADDR3_47_32_LSB                                         0
#define RESPONSE_END_STATUS_ADDR3_47_32_MSB                                         15
#define RESPONSE_END_STATUS_ADDR3_47_32_MASK                                        0x000000000000ffff


/* Description		RANGING

			To be copied over from TX_CBF_INFO: Set to 1 if the status
			 is generated due to an active ranging session (.11az)
*/

#define RESPONSE_END_STATUS_RANGING_OFFSET                                          0x0000000000000050
#define RESPONSE_END_STATUS_RANGING_LSB                                             16
#define RESPONSE_END_STATUS_RANGING_MSB                                             16
#define RESPONSE_END_STATUS_RANGING_MASK                                            0x0000000000010000


/* Description		SECURE

			To be copied over from TX_CBF_INFO: Only valid if Ranging
			 is set to 1, this indicates if the current ranging session
			 is secure.
*/

#define RESPONSE_END_STATUS_SECURE_OFFSET                                           0x0000000000000050
#define RESPONSE_END_STATUS_SECURE_LSB                                              17
#define RESPONSE_END_STATUS_SECURE_MSB                                              17
#define RESPONSE_END_STATUS_SECURE_MASK                                             0x0000000000020000


/* Description		RANGING_FTM_FRAME_SENT

			Only valid if Ranging is set to 1
			
			TXPCU sets this bit if an FTM frame aggregated with an LMR
			 was sent.
*/

#define RESPONSE_END_STATUS_RANGING_FTM_FRAME_SENT_OFFSET                           0x0000000000000050
#define RESPONSE_END_STATUS_RANGING_FTM_FRAME_SENT_LSB                              18
#define RESPONSE_END_STATUS_RANGING_FTM_FRAME_SENT_MSB                              18
#define RESPONSE_END_STATUS_RANGING_FTM_FRAME_SENT_MASK                             0x0000000000040000


/* Description		RESERVED_20A

			<legal 0>
*/

#define RESPONSE_END_STATUS_RESERVED_20A_OFFSET                                     0x0000000000000050
#define RESPONSE_END_STATUS_RESERVED_20A_LSB                                        19
#define RESPONSE_END_STATUS_RESERVED_20A_MSB                                        31
#define RESPONSE_END_STATUS_RESERVED_20A_MASK                                       0x00000000fff80000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RESPONSE_END_STATUS_TLV64_PADDING_OFFSET                                    0x0000000000000050
#define RESPONSE_END_STATUS_TLV64_PADDING_LSB                                       32
#define RESPONSE_END_STATUS_TLV64_PADDING_MSB                                       63
#define RESPONSE_END_STATUS_TLV64_PADDING_MASK                                      0xffffffff00000000



#endif   // RESPONSE_END_STATUS
