/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RX_PPDU_END_USER_STATS_H_
#define _RX_PPDU_END_USER_STATS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_rxpcu_classification_overview.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct rx_rxpcu_classification_overview rxpcu_classification_details;
//	1	sta_full_aid[12:0], mcs[16:13], nss[19:17], odma_info_valid[20], ofdma_low_ru_index[27:21], reserved_1a[31:28]
//	2	ofdma_high_ru_index[6:0], reserved_2a[7], user_receive_quality[15:8], mpdu_cnt_fcs_err[25:16], wbm2rxdma_buf_source_used[26], fw2rxdma_buf_source_used[27], sw2rxdma_buf_source_used[28], reserved_2b[31:29]
//	3	mpdu_cnt_fcs_ok[8:0], frame_control_info_valid[9], qos_control_info_valid[10], ht_control_info_valid[11], data_sequence_control_info_valid[12], reserved_3a[15:13], rxdma2reo_ring_used[16], rxdma2fw_ring_used[17], rxdma2sw_ring_used[18], rxdma_release_ring_used[19], ht_control_field_pkt_type[23:20], reserved_3b[31:24]
//	4	ast_index[15:0], frame_control_field[31:16]
//	5	first_data_seq_ctrl[15:0], qos_control_field[31:16]
//	6	ht_control_field[31:0]
//	7	fcs_ok_bitmap_31_0[31:0]
//	8	fcs_ok_bitmap_63_32[31:0]
//	9	udp_msdu_count[15:0], tcp_msdu_count[31:16]
//	10	other_msdu_count[15:0], tcp_ack_msdu_count[31:16]
//	11	sw_response_reference_ptr[31:0]
//	12	received_qos_data_tid_bitmap[15:0], received_qos_data_tid_eosp_bitmap[31:16]
//	13	qosctrl_15_8_tid0[7:0], qosctrl_15_8_tid1[15:8], qosctrl_15_8_tid2[23:16], qosctrl_15_8_tid3[31:24]
//	14	qosctrl_15_8_tid4[7:0], qosctrl_15_8_tid5[15:8], qosctrl_15_8_tid6[23:16], qosctrl_15_8_tid7[31:24]
//	15	qosctrl_15_8_tid8[7:0], qosctrl_15_8_tid9[15:8], qosctrl_15_8_tid10[23:16], qosctrl_15_8_tid11[31:24]
//	16	qosctrl_15_8_tid12[7:0], qosctrl_15_8_tid13[15:8], qosctrl_15_8_tid14[23:16], qosctrl_15_8_tid15[31:24]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_PPDU_END_USER_STATS 17

struct rx_ppdu_end_user_stats {
    struct            rx_rxpcu_classification_overview                       rxpcu_classification_details;
             uint32_t sta_full_aid                    : 13, //[12:0]
                      mcs                             :  4, //[16:13]
                      nss                             :  3, //[19:17]
                      odma_info_valid                 :  1, //[20]
                      ofdma_low_ru_index              :  7, //[27:21]
                      reserved_1a                     :  4; //[31:28]
             uint32_t ofdma_high_ru_index             :  7, //[6:0]
                      reserved_2a                     :  1, //[7]
                      user_receive_quality            :  8, //[15:8]
                      mpdu_cnt_fcs_err                : 10, //[25:16]
                      wbm2rxdma_buf_source_used       :  1, //[26]
                      fw2rxdma_buf_source_used        :  1, //[27]
                      sw2rxdma_buf_source_used        :  1, //[28]
                      reserved_2b                     :  3; //[31:29]
             uint32_t mpdu_cnt_fcs_ok                 :  9, //[8:0]
                      frame_control_info_valid        :  1, //[9]
                      qos_control_info_valid          :  1, //[10]
                      ht_control_info_valid           :  1, //[11]
                      data_sequence_control_info_valid:  1, //[12]
                      reserved_3a                     :  3, //[15:13]
                      rxdma2reo_ring_used             :  1, //[16]
                      rxdma2fw_ring_used              :  1, //[17]
                      rxdma2sw_ring_used              :  1, //[18]
                      rxdma_release_ring_used         :  1, //[19]
                      ht_control_field_pkt_type       :  4, //[23:20]
                      reserved_3b                     :  8; //[31:24]
             uint32_t ast_index                       : 16, //[15:0]
                      frame_control_field             : 16; //[31:16]
             uint32_t first_data_seq_ctrl             : 16, //[15:0]
                      qos_control_field               : 16; //[31:16]
             uint32_t ht_control_field                : 32; //[31:0]
             uint32_t fcs_ok_bitmap_31_0              : 32; //[31:0]
             uint32_t fcs_ok_bitmap_63_32             : 32; //[31:0]
             uint32_t udp_msdu_count                  : 16, //[15:0]
                      tcp_msdu_count                  : 16; //[31:16]
             uint32_t other_msdu_count                : 16, //[15:0]
                      tcp_ack_msdu_count              : 16; //[31:16]
             uint32_t sw_response_reference_ptr       : 32; //[31:0]
             uint32_t received_qos_data_tid_bitmap    : 16, //[15:0]
                      received_qos_data_tid_eosp_bitmap: 16; //[31:16]
             uint32_t qosctrl_15_8_tid0               :  8, //[7:0]
                      qosctrl_15_8_tid1               :  8, //[15:8]
                      qosctrl_15_8_tid2               :  8, //[23:16]
                      qosctrl_15_8_tid3               :  8; //[31:24]
             uint32_t qosctrl_15_8_tid4               :  8, //[7:0]
                      qosctrl_15_8_tid5               :  8, //[15:8]
                      qosctrl_15_8_tid6               :  8, //[23:16]
                      qosctrl_15_8_tid7               :  8; //[31:24]
             uint32_t qosctrl_15_8_tid8               :  8, //[7:0]
                      qosctrl_15_8_tid9               :  8, //[15:8]
                      qosctrl_15_8_tid10              :  8, //[23:16]
                      qosctrl_15_8_tid11              :  8; //[31:24]
             uint32_t qosctrl_15_8_tid12              :  8, //[7:0]
                      qosctrl_15_8_tid13              :  8, //[15:8]
                      qosctrl_15_8_tid14              :  8, //[23:16]
                      qosctrl_15_8_tid15              :  8; //[31:24]
};

/*

struct rx_rxpcu_classification_overview rxpcu_classification_details
			
			Details related to what RXPCU classification types of
			MPDUs have been received

sta_full_aid
			
			Consumer: FW
			
			Producer: RXPCU
			
			
			
			The full AID of this station. 
			
			
			
			<legal all>

mcs
			
			MCS of the received frame
			
			
			
			For details, refer to  MCS_TYPE description
			
			<legal all>

nss
			
			Number of spatial streams.
			
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			
			<enum 1 2_spatial_streams>2 spatial streams
			
			<enum 2 3_spatial_streams>3 spatial streams
			
			<enum 3 4_spatial_streams>4 spatial streams
			
			<enum 4 5_spatial_streams>5 spatial streams
			
			<enum 5 6_spatial_streams>6 spatial streams
			
			<enum 6 7_spatial_streams>7 spatial streams
			
			<enum 7 8_spatial_streams>8 spatial streams

odma_info_valid
			
			When set, ofdma RU related info in the following fields
			is valid
			
			<legal all>

ofdma_low_ru_index
			
			The index of the lowerest RU used by this STA.
			
			<legal all>

reserved_1a
			
			<legal 0>

ofdma_high_ru_index
			
			The index of the highest RU used by this STA.
			
			<legal all>

reserved_2a
			
			<legal 0>

user_receive_quality
			
			RSSI / EVM for this user ???
			
			
			
			Details TBD
			
			<legal all>

mpdu_cnt_fcs_err
			
			The number of MPDUs received from this STA in this PPDU
			with FCS errors
			
			<legal all>

wbm2rxdma_buf_source_used
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the wbm2rxdma_buf ring as
			source for at least one of the frames in this PPDU.

fw2rxdma_buf_source_used
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the fw2rxdma_buf ring as source
			for at least one of the frames in this PPDU.

sw2rxdma_buf_source_used
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the sw2rxdma_buf ring as source
			for at least one of the frames in this PPDU.

reserved_2b
			
			<legal 0>

mpdu_cnt_fcs_ok
			
			The number of MPDUs received from this STA in this PPDU
			with correct FCS
			
			<legal all>

frame_control_info_valid
			
			When set, the frame_control_info field contains valid
			information
			
			<legal all>

qos_control_info_valid
			
			When set, the QoS_control_info field contains valid
			information
			
			<legal all>

ht_control_info_valid
			
			When set, the HT_control_info field contains valid
			information
			
			<legal all>

data_sequence_control_info_valid
			
			When set, the First_data_seq_ctrl field contains valid
			information
			
			<legal all>

reserved_3a
			
			<legal 0>

rxdma2reo_ring_used
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA

rxdma2fw_ring_used
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA

rxdma2sw_ring_used
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA

rxdma_release_ring_used
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA

ht_control_field_pkt_type
			
			Field only valid when HT_control_info_valid  is set.
			
			
			
			Indicates what the PHY receive type was for receiving
			this frame. Can help determine if the HT_CONTROL field shall
			be interpreted as HT/VHT or HE.
			
			
			
			<enum 0 dot11a>802.11a PPDU type
			
			<enum 1 dot11b>802.11b PPDU type
			
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			
			<enum 3 dot11ac>802.11ac PPDU type
			
			<enum 4 dot11ax>802.11ax PPDU type

reserved_3b
			
			<legal 0>

ast_index
			
			This field indicates the index of the AST entry
			corresponding to this MPDU. It is provided by the GSE module
			instantiated in RXPCU.
			
			A value of 0xFFFF indicates an invalid AST index,
			meaning that No AST entry was found or NO AST search was
			performed
			
			<legal all>

frame_control_field
			
			Field only valid when Frame_control_info_valid is set.
			
			
			
			Last successfully received Frame_control field of data
			frame (excluding Data NULL/ QoS Null) for this user
			
			Mainly used to track the PM state of the transmitted
			device
			
			
			
			NOTE: only data frame info is needed, as control and
			management frames are already routed to the FW.
			
			<legal all>

first_data_seq_ctrl
			
			Field only valid when Data_sequence_control_info_valid
			is set.
			
			
			
			Sequence control field of the first data frame
			(excluding Data NULL or QoS Data null) received for this
			user with correct FCS
			
			
			
			NOTE: only data frame info is needed, as control and
			management frames are already routed to the FW.
			
			<legal all>

qos_control_field
			
			Field only valid when QoS_control_info_valid is set.
			
			
			
			Last successfully received QoS_control field of data
			frame (excluding Data NULL/ QoS Null) for this user
			
			
			
			Note that in case of multi TID, this field can only
			reflect the last properly received MPDU, and thus can not
			indicate all potentially different TIDs that had been
			received earlier. 
			
			
			
			There are however per TID fields, that will contain
			among other things all buffer status info: See
			
			QoSCtrl_15_8_tid???
			
			<legal all>

ht_control_field
			
			Field only valid when HT_control_info_valid is set.
			
			
			
			Last successfully received
			HT_CONTROL/VHT_CONTROL/HE_CONTROL  field of data frames,
			excluding Data NULL/ QoS Null, for this user. Data NULL/ QoS
			Null are excluded here because these frames are always
			already routed to the FW by RXDMA.
			
			
			
			See field HT_control_field_pkt_type in case pkt_type
			influences if this fields interpretation as HT/VHT/HE
			CONTROL
			
			<legal all>

fcs_ok_bitmap_31_0
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_63_32
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			
			
			NOTE: for users 0, 1, 2 and 3, additional bitmap info
			(up to 256 bitmap window) is provided in
			RX_PPDU_END_USER_STATS_EXT TLV
			
			<legal all>

udp_msdu_count
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain UDP frames.
			
			<legal all>

tcp_msdu_count
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain TCP frames.
			
			
			
			(Note: This does NOT include TCP-ACK)
			
			<legal all>

other_msdu_count
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain neither UDP or TCP frames.
			
			
			
			Includes Management and control frames.
			
			
			
			<legal all>

tcp_ack_msdu_count
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain TCP ack frames.
			
			<legal all>

sw_response_reference_ptr
			
			Pointer that SW uses to refer back to an expected
			response reception. Used for Rate adaptation purposes.
			
			When a reception occurrs that is not tied to an expected
			response, this field is set to 0x0
			
			<legal all>

received_qos_data_tid_bitmap
			
			Whenever a QoS Data frame is received, the bit in this
			field that corresponds to the received TID shall be set.
			
			...Bitmap[0] = TID0
			
			...Bitmap[1] = TID1
			
			Etc.
			
			<legal all>

received_qos_data_tid_eosp_bitmap
			
			Field initialized to 0
			
			For every QoS Data frame that is correctly received, the
			EOSP bit of that frame is copied over into the corresponding
			TID related field.
			
			Note that this implies that the bits here represent the
			EOSP bit status for each TID of the last MPDU received for
			that TID.
			
			
			
			received TID shall be set.
			
			...eosp_bitmap[0] = eosp of TID0
			
			...eosp_bitmap[1] = eosp of TID1
			
			Etc.
			
			<legal all>

qosctrl_15_8_tid0
			
			Field only valid when Received_qos_data_tid_bitmap[0] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID0

qosctrl_15_8_tid1
			
			Field only valid when Received_qos_data_tid_bitmap[1] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID1

qosctrl_15_8_tid2
			
			Field only valid when Received_qos_data_tid_bitmap[2] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID2

qosctrl_15_8_tid3
			
			Field only valid when Received_qos_data_tid_bitmap[3] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID3

qosctrl_15_8_tid4
			
			Field only valid when Received_qos_data_tid_bitmap[4] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID4

qosctrl_15_8_tid5
			
			Field only valid when Received_qos_data_tid_bitmap[5] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID5

qosctrl_15_8_tid6
			
			Field only valid when Received_qos_data_tid_bitmap[6] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID6

qosctrl_15_8_tid7
			
			Field only valid when Received_qos_data_tid_bitmap[7] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID7

qosctrl_15_8_tid8
			
			Field only valid when Received_qos_data_tid_bitmap[8] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID8

qosctrl_15_8_tid9
			
			Field only valid when Received_qos_data_tid_bitmap[9] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID9

qosctrl_15_8_tid10
			
			Field only valid when Received_qos_data_tid_bitmap[10]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID10

qosctrl_15_8_tid11
			
			Field only valid when Received_qos_data_tid_bitmap[11]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID11

qosctrl_15_8_tid12
			
			Field only valid when Received_qos_data_tid_bitmap[12]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID12

qosctrl_15_8_tid13
			
			Field only valid when Received_qos_data_tid_bitmap[13]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID13

qosctrl_15_8_tid14
			
			Field only valid when Received_qos_data_tid_bitmap[14]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID14

qosctrl_15_8_tid15
			
			Field only valid when Received_qos_data_tid_bitmap[15]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID15
*/

#define RX_PPDU_END_USER_STATS_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_LSB 24
#define RX_PPDU_END_USER_STATS_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_MASK 0xffffffff

/* Description		RX_PPDU_END_USER_STATS_1_STA_FULL_AID
			
			Consumer: FW
			
			Producer: RXPCU
			
			
			
			The full AID of this station. 
			
			
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_1_STA_FULL_AID_OFFSET                 0x00000004
#define RX_PPDU_END_USER_STATS_1_STA_FULL_AID_LSB                    0
#define RX_PPDU_END_USER_STATS_1_STA_FULL_AID_MASK                   0x00001fff

/* Description		RX_PPDU_END_USER_STATS_1_MCS
			
			MCS of the received frame
			
			
			
			For details, refer to  MCS_TYPE description
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_1_MCS_OFFSET                          0x00000004
#define RX_PPDU_END_USER_STATS_1_MCS_LSB                             13
#define RX_PPDU_END_USER_STATS_1_MCS_MASK                            0x0001e000

/* Description		RX_PPDU_END_USER_STATS_1_NSS
			
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
#define RX_PPDU_END_USER_STATS_1_NSS_OFFSET                          0x00000004
#define RX_PPDU_END_USER_STATS_1_NSS_LSB                             17
#define RX_PPDU_END_USER_STATS_1_NSS_MASK                            0x000e0000

/* Description		RX_PPDU_END_USER_STATS_1_ODMA_INFO_VALID
			
			When set, ofdma RU related info in the following fields
			is valid
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_1_ODMA_INFO_VALID_OFFSET              0x00000004
#define RX_PPDU_END_USER_STATS_1_ODMA_INFO_VALID_LSB                 20
#define RX_PPDU_END_USER_STATS_1_ODMA_INFO_VALID_MASK                0x00100000

/* Description		RX_PPDU_END_USER_STATS_1_OFDMA_LOW_RU_INDEX
			
			The index of the lowerest RU used by this STA.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_1_OFDMA_LOW_RU_INDEX_OFFSET           0x00000004
#define RX_PPDU_END_USER_STATS_1_OFDMA_LOW_RU_INDEX_LSB              21
#define RX_PPDU_END_USER_STATS_1_OFDMA_LOW_RU_INDEX_MASK             0x0fe00000

/* Description		RX_PPDU_END_USER_STATS_1_RESERVED_1A
			
			<legal 0>
*/
#define RX_PPDU_END_USER_STATS_1_RESERVED_1A_OFFSET                  0x00000004
#define RX_PPDU_END_USER_STATS_1_RESERVED_1A_LSB                     28
#define RX_PPDU_END_USER_STATS_1_RESERVED_1A_MASK                    0xf0000000

/* Description		RX_PPDU_END_USER_STATS_2_OFDMA_HIGH_RU_INDEX
			
			The index of the highest RU used by this STA.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_2_OFDMA_HIGH_RU_INDEX_OFFSET          0x00000008
#define RX_PPDU_END_USER_STATS_2_OFDMA_HIGH_RU_INDEX_LSB             0
#define RX_PPDU_END_USER_STATS_2_OFDMA_HIGH_RU_INDEX_MASK            0x0000007f

/* Description		RX_PPDU_END_USER_STATS_2_RESERVED_2A
			
			<legal 0>
*/
#define RX_PPDU_END_USER_STATS_2_RESERVED_2A_OFFSET                  0x00000008
#define RX_PPDU_END_USER_STATS_2_RESERVED_2A_LSB                     7
#define RX_PPDU_END_USER_STATS_2_RESERVED_2A_MASK                    0x00000080

/* Description		RX_PPDU_END_USER_STATS_2_USER_RECEIVE_QUALITY
			
			RSSI / EVM for this user ???
			
			
			
			Details TBD
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_2_USER_RECEIVE_QUALITY_OFFSET         0x00000008
#define RX_PPDU_END_USER_STATS_2_USER_RECEIVE_QUALITY_LSB            8
#define RX_PPDU_END_USER_STATS_2_USER_RECEIVE_QUALITY_MASK           0x0000ff00

/* Description		RX_PPDU_END_USER_STATS_2_MPDU_CNT_FCS_ERR
			
			The number of MPDUs received from this STA in this PPDU
			with FCS errors
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_2_MPDU_CNT_FCS_ERR_OFFSET             0x00000008
#define RX_PPDU_END_USER_STATS_2_MPDU_CNT_FCS_ERR_LSB                16
#define RX_PPDU_END_USER_STATS_2_MPDU_CNT_FCS_ERR_MASK               0x03ff0000

/* Description		RX_PPDU_END_USER_STATS_2_WBM2RXDMA_BUF_SOURCE_USED
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the wbm2rxdma_buf ring as
			source for at least one of the frames in this PPDU.
*/
#define RX_PPDU_END_USER_STATS_2_WBM2RXDMA_BUF_SOURCE_USED_OFFSET    0x00000008
#define RX_PPDU_END_USER_STATS_2_WBM2RXDMA_BUF_SOURCE_USED_LSB       26
#define RX_PPDU_END_USER_STATS_2_WBM2RXDMA_BUF_SOURCE_USED_MASK      0x04000000

/* Description		RX_PPDU_END_USER_STATS_2_FW2RXDMA_BUF_SOURCE_USED
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the fw2rxdma_buf ring as source
			for at least one of the frames in this PPDU.
*/
#define RX_PPDU_END_USER_STATS_2_FW2RXDMA_BUF_SOURCE_USED_OFFSET     0x00000008
#define RX_PPDU_END_USER_STATS_2_FW2RXDMA_BUF_SOURCE_USED_LSB        27
#define RX_PPDU_END_USER_STATS_2_FW2RXDMA_BUF_SOURCE_USED_MASK       0x08000000

/* Description		RX_PPDU_END_USER_STATS_2_SW2RXDMA_BUF_SOURCE_USED
			
			Field filled in by RXDMA
			
			
			
			When set, RXDMA has used the sw2rxdma_buf ring as source
			for at least one of the frames in this PPDU.
*/
#define RX_PPDU_END_USER_STATS_2_SW2RXDMA_BUF_SOURCE_USED_OFFSET     0x00000008
#define RX_PPDU_END_USER_STATS_2_SW2RXDMA_BUF_SOURCE_USED_LSB        28
#define RX_PPDU_END_USER_STATS_2_SW2RXDMA_BUF_SOURCE_USED_MASK       0x10000000

/* Description		RX_PPDU_END_USER_STATS_2_RESERVED_2B
			
			<legal 0>
*/
#define RX_PPDU_END_USER_STATS_2_RESERVED_2B_OFFSET                  0x00000008
#define RX_PPDU_END_USER_STATS_2_RESERVED_2B_LSB                     29
#define RX_PPDU_END_USER_STATS_2_RESERVED_2B_MASK                    0xe0000000

/* Description		RX_PPDU_END_USER_STATS_3_MPDU_CNT_FCS_OK
			
			The number of MPDUs received from this STA in this PPDU
			with correct FCS
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_3_MPDU_CNT_FCS_OK_OFFSET              0x0000000c
#define RX_PPDU_END_USER_STATS_3_MPDU_CNT_FCS_OK_LSB                 0
#define RX_PPDU_END_USER_STATS_3_MPDU_CNT_FCS_OK_MASK                0x000001ff

/* Description		RX_PPDU_END_USER_STATS_3_FRAME_CONTROL_INFO_VALID
			
			When set, the frame_control_info field contains valid
			information
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_3_FRAME_CONTROL_INFO_VALID_OFFSET     0x0000000c
#define RX_PPDU_END_USER_STATS_3_FRAME_CONTROL_INFO_VALID_LSB        9
#define RX_PPDU_END_USER_STATS_3_FRAME_CONTROL_INFO_VALID_MASK       0x00000200

/* Description		RX_PPDU_END_USER_STATS_3_QOS_CONTROL_INFO_VALID
			
			When set, the QoS_control_info field contains valid
			information
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_3_QOS_CONTROL_INFO_VALID_OFFSET       0x0000000c
#define RX_PPDU_END_USER_STATS_3_QOS_CONTROL_INFO_VALID_LSB          10
#define RX_PPDU_END_USER_STATS_3_QOS_CONTROL_INFO_VALID_MASK         0x00000400

/* Description		RX_PPDU_END_USER_STATS_3_HT_CONTROL_INFO_VALID
			
			When set, the HT_control_info field contains valid
			information
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_INFO_VALID_OFFSET        0x0000000c
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_INFO_VALID_LSB           11
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_INFO_VALID_MASK          0x00000800

/* Description		RX_PPDU_END_USER_STATS_3_DATA_SEQUENCE_CONTROL_INFO_VALID
			
			When set, the First_data_seq_ctrl field contains valid
			information
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_3_DATA_SEQUENCE_CONTROL_INFO_VALID_OFFSET 0x0000000c
#define RX_PPDU_END_USER_STATS_3_DATA_SEQUENCE_CONTROL_INFO_VALID_LSB 12
#define RX_PPDU_END_USER_STATS_3_DATA_SEQUENCE_CONTROL_INFO_VALID_MASK 0x00001000

/* Description		RX_PPDU_END_USER_STATS_3_RESERVED_3A
			
			<legal 0>
*/
#define RX_PPDU_END_USER_STATS_3_RESERVED_3A_OFFSET                  0x0000000c
#define RX_PPDU_END_USER_STATS_3_RESERVED_3A_LSB                     13
#define RX_PPDU_END_USER_STATS_3_RESERVED_3A_MASK                    0x0000e000

/* Description		RX_PPDU_END_USER_STATS_3_RXDMA2REO_RING_USED
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA
*/
#define RX_PPDU_END_USER_STATS_3_RXDMA2REO_RING_USED_OFFSET          0x0000000c
#define RX_PPDU_END_USER_STATS_3_RXDMA2REO_RING_USED_LSB             16
#define RX_PPDU_END_USER_STATS_3_RXDMA2REO_RING_USED_MASK            0x00010000

/* Description		RX_PPDU_END_USER_STATS_3_RXDMA2FW_RING_USED
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA
*/
#define RX_PPDU_END_USER_STATS_3_RXDMA2FW_RING_USED_OFFSET           0x0000000c
#define RX_PPDU_END_USER_STATS_3_RXDMA2FW_RING_USED_LSB              17
#define RX_PPDU_END_USER_STATS_3_RXDMA2FW_RING_USED_MASK             0x00020000

/* Description		RX_PPDU_END_USER_STATS_3_RXDMA2SW_RING_USED
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA
*/
#define RX_PPDU_END_USER_STATS_3_RXDMA2SW_RING_USED_OFFSET           0x0000000c
#define RX_PPDU_END_USER_STATS_3_RXDMA2SW_RING_USED_LSB              18
#define RX_PPDU_END_USER_STATS_3_RXDMA2SW_RING_USED_MASK             0x00040000

/* Description		RX_PPDU_END_USER_STATS_3_RXDMA_RELEASE_RING_USED
			
			Field filled in by RXDMA
			
			
			
			Set when at least one frame during this PPDU got pushed
			to this ring by RXDMA
*/
#define RX_PPDU_END_USER_STATS_3_RXDMA_RELEASE_RING_USED_OFFSET      0x0000000c
#define RX_PPDU_END_USER_STATS_3_RXDMA_RELEASE_RING_USED_LSB         19
#define RX_PPDU_END_USER_STATS_3_RXDMA_RELEASE_RING_USED_MASK        0x00080000

/* Description		RX_PPDU_END_USER_STATS_3_HT_CONTROL_FIELD_PKT_TYPE
			
			Field only valid when HT_control_info_valid  is set.
			
			
			
			Indicates what the PHY receive type was for receiving
			this frame. Can help determine if the HT_CONTROL field shall
			be interpreted as HT/VHT or HE.
			
			
			
			<enum 0 dot11a>802.11a PPDU type
			
			<enum 1 dot11b>802.11b PPDU type
			
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			
			<enum 3 dot11ac>802.11ac PPDU type
			
			<enum 4 dot11ax>802.11ax PPDU type
*/
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_FIELD_PKT_TYPE_OFFSET    0x0000000c
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_FIELD_PKT_TYPE_LSB       20
#define RX_PPDU_END_USER_STATS_3_HT_CONTROL_FIELD_PKT_TYPE_MASK      0x00f00000

/* Description		RX_PPDU_END_USER_STATS_3_RESERVED_3B
			
			<legal 0>
*/
#define RX_PPDU_END_USER_STATS_3_RESERVED_3B_OFFSET                  0x0000000c
#define RX_PPDU_END_USER_STATS_3_RESERVED_3B_LSB                     24
#define RX_PPDU_END_USER_STATS_3_RESERVED_3B_MASK                    0xff000000

/* Description		RX_PPDU_END_USER_STATS_4_AST_INDEX
			
			This field indicates the index of the AST entry
			corresponding to this MPDU. It is provided by the GSE module
			instantiated in RXPCU.
			
			A value of 0xFFFF indicates an invalid AST index,
			meaning that No AST entry was found or NO AST search was
			performed
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_4_AST_INDEX_OFFSET                    0x00000010
#define RX_PPDU_END_USER_STATS_4_AST_INDEX_LSB                       0
#define RX_PPDU_END_USER_STATS_4_AST_INDEX_MASK                      0x0000ffff

/* Description		RX_PPDU_END_USER_STATS_4_FRAME_CONTROL_FIELD
			
			Field only valid when Frame_control_info_valid is set.
			
			
			
			Last successfully received Frame_control field of data
			frame (excluding Data NULL/ QoS Null) for this user
			
			Mainly used to track the PM state of the transmitted
			device
			
			
			
			NOTE: only data frame info is needed, as control and
			management frames are already routed to the FW.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_4_FRAME_CONTROL_FIELD_OFFSET          0x00000010
#define RX_PPDU_END_USER_STATS_4_FRAME_CONTROL_FIELD_LSB             16
#define RX_PPDU_END_USER_STATS_4_FRAME_CONTROL_FIELD_MASK            0xffff0000

/* Description		RX_PPDU_END_USER_STATS_5_FIRST_DATA_SEQ_CTRL
			
			Field only valid when Data_sequence_control_info_valid
			is set.
			
			
			
			Sequence control field of the first data frame
			(excluding Data NULL or QoS Data null) received for this
			user with correct FCS
			
			
			
			NOTE: only data frame info is needed, as control and
			management frames are already routed to the FW.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_5_FIRST_DATA_SEQ_CTRL_OFFSET          0x00000014
#define RX_PPDU_END_USER_STATS_5_FIRST_DATA_SEQ_CTRL_LSB             0
#define RX_PPDU_END_USER_STATS_5_FIRST_DATA_SEQ_CTRL_MASK            0x0000ffff

/* Description		RX_PPDU_END_USER_STATS_5_QOS_CONTROL_FIELD
			
			Field only valid when QoS_control_info_valid is set.
			
			
			
			Last successfully received QoS_control field of data
			frame (excluding Data NULL/ QoS Null) for this user
			
			
			
			Note that in case of multi TID, this field can only
			reflect the last properly received MPDU, and thus can not
			indicate all potentially different TIDs that had been
			received earlier. 
			
			
			
			There are however per TID fields, that will contain
			among other things all buffer status info: See
			
			QoSCtrl_15_8_tid???
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_5_QOS_CONTROL_FIELD_OFFSET            0x00000014
#define RX_PPDU_END_USER_STATS_5_QOS_CONTROL_FIELD_LSB               16
#define RX_PPDU_END_USER_STATS_5_QOS_CONTROL_FIELD_MASK              0xffff0000

/* Description		RX_PPDU_END_USER_STATS_6_HT_CONTROL_FIELD
			
			Field only valid when HT_control_info_valid is set.
			
			
			
			Last successfully received
			HT_CONTROL/VHT_CONTROL/HE_CONTROL  field of data frames,
			excluding Data NULL/ QoS Null, for this user. Data NULL/ QoS
			Null are excluded here because these frames are always
			already routed to the FW by RXDMA.
			
			
			
			See field HT_control_field_pkt_type in case pkt_type
			influences if this fields interpretation as HT/VHT/HE
			CONTROL
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_6_HT_CONTROL_FIELD_OFFSET             0x00000018
#define RX_PPDU_END_USER_STATS_6_HT_CONTROL_FIELD_LSB                0
#define RX_PPDU_END_USER_STATS_6_HT_CONTROL_FIELD_MASK               0xffffffff

/* Description		RX_PPDU_END_USER_STATS_7_FCS_OK_BITMAP_31_0
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_7_FCS_OK_BITMAP_31_0_OFFSET           0x0000001c
#define RX_PPDU_END_USER_STATS_7_FCS_OK_BITMAP_31_0_LSB              0
#define RX_PPDU_END_USER_STATS_7_FCS_OK_BITMAP_31_0_MASK             0xffffffff

/* Description		RX_PPDU_END_USER_STATS_8_FCS_OK_BITMAP_63_32
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			
			
			NOTE: for users 0, 1, 2 and 3, additional bitmap info
			(up to 256 bitmap window) is provided in
			RX_PPDU_END_USER_STATS_EXT TLV
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_8_FCS_OK_BITMAP_63_32_OFFSET          0x00000020
#define RX_PPDU_END_USER_STATS_8_FCS_OK_BITMAP_63_32_LSB             0
#define RX_PPDU_END_USER_STATS_8_FCS_OK_BITMAP_63_32_MASK            0xffffffff

/* Description		RX_PPDU_END_USER_STATS_9_UDP_MSDU_COUNT
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain UDP frames.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_9_UDP_MSDU_COUNT_OFFSET               0x00000024
#define RX_PPDU_END_USER_STATS_9_UDP_MSDU_COUNT_LSB                  0
#define RX_PPDU_END_USER_STATS_9_UDP_MSDU_COUNT_MASK                 0x0000ffff

/* Description		RX_PPDU_END_USER_STATS_9_TCP_MSDU_COUNT
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain TCP frames.
			
			
			
			(Note: This does NOT include TCP-ACK)
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_9_TCP_MSDU_COUNT_OFFSET               0x00000024
#define RX_PPDU_END_USER_STATS_9_TCP_MSDU_COUNT_LSB                  16
#define RX_PPDU_END_USER_STATS_9_TCP_MSDU_COUNT_MASK                 0xffff0000

/* Description		RX_PPDU_END_USER_STATS_10_OTHER_MSDU_COUNT
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain neither UDP or TCP frames.
			
			
			
			Includes Management and control frames.
			
			
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_10_OTHER_MSDU_COUNT_OFFSET            0x00000028
#define RX_PPDU_END_USER_STATS_10_OTHER_MSDU_COUNT_LSB               0
#define RX_PPDU_END_USER_STATS_10_OTHER_MSDU_COUNT_MASK              0x0000ffff

/* Description		RX_PPDU_END_USER_STATS_10_TCP_ACK_MSDU_COUNT
			
			Field filled in by RX OLE
			
			Set to 0 by RXPCU
			
			
			
			The number of MSDUs that are part of MPDUs without FCS
			error, that contain TCP ack frames.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_10_TCP_ACK_MSDU_COUNT_OFFSET          0x00000028
#define RX_PPDU_END_USER_STATS_10_TCP_ACK_MSDU_COUNT_LSB             16
#define RX_PPDU_END_USER_STATS_10_TCP_ACK_MSDU_COUNT_MASK            0xffff0000

/* Description		RX_PPDU_END_USER_STATS_11_SW_RESPONSE_REFERENCE_PTR
			
			Pointer that SW uses to refer back to an expected
			response reception. Used for Rate adaptation purposes.
			
			When a reception occurrs that is not tied to an expected
			response, this field is set to 0x0
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_11_SW_RESPONSE_REFERENCE_PTR_OFFSET   0x0000002c
#define RX_PPDU_END_USER_STATS_11_SW_RESPONSE_REFERENCE_PTR_LSB      0
#define RX_PPDU_END_USER_STATS_11_SW_RESPONSE_REFERENCE_PTR_MASK     0xffffffff

/* Description		RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_BITMAP
			
			Whenever a QoS Data frame is received, the bit in this
			field that corresponds to the received TID shall be set.
			
			...Bitmap[0] = TID0
			
			...Bitmap[1] = TID1
			
			Etc.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_BITMAP_OFFSET 0x00000030
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_BITMAP_LSB   0
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_BITMAP_MASK  0x0000ffff

/* Description		RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_EOSP_BITMAP
			
			Field initialized to 0
			
			For every QoS Data frame that is correctly received, the
			EOSP bit of that frame is copied over into the corresponding
			TID related field.
			
			Note that this implies that the bits here represent the
			EOSP bit status for each TID of the last MPDU received for
			that TID.
			
			
			
			received TID shall be set.
			
			...eosp_bitmap[0] = eosp of TID0
			
			...eosp_bitmap[1] = eosp of TID1
			
			Etc.
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_OFFSET 0x00000030
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_LSB 16
#define RX_PPDU_END_USER_STATS_12_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MASK 0xffff0000

/* Description		RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID0
			
			Field only valid when Received_qos_data_tid_bitmap[0] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID0
*/
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID0_OFFSET           0x00000034
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID0_LSB              0
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID0_MASK             0x000000ff

/* Description		RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID1
			
			Field only valid when Received_qos_data_tid_bitmap[1] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID1
*/
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID1_OFFSET           0x00000034
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID1_LSB              8
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID1_MASK             0x0000ff00

/* Description		RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID2
			
			Field only valid when Received_qos_data_tid_bitmap[2] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID2
*/
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID2_OFFSET           0x00000034
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID2_LSB              16
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID2_MASK             0x00ff0000

/* Description		RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID3
			
			Field only valid when Received_qos_data_tid_bitmap[3] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID3
*/
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID3_OFFSET           0x00000034
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID3_LSB              24
#define RX_PPDU_END_USER_STATS_13_QOSCTRL_15_8_TID3_MASK             0xff000000

/* Description		RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID4
			
			Field only valid when Received_qos_data_tid_bitmap[4] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID4
*/
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID4_OFFSET           0x00000038
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID4_LSB              0
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID4_MASK             0x000000ff

/* Description		RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID5
			
			Field only valid when Received_qos_data_tid_bitmap[5] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID5
*/
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID5_OFFSET           0x00000038
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID5_LSB              8
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID5_MASK             0x0000ff00

/* Description		RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID6
			
			Field only valid when Received_qos_data_tid_bitmap[6] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID6
*/
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID6_OFFSET           0x00000038
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID6_LSB              16
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID6_MASK             0x00ff0000

/* Description		RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID7
			
			Field only valid when Received_qos_data_tid_bitmap[7] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID7
*/
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID7_OFFSET           0x00000038
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID7_LSB              24
#define RX_PPDU_END_USER_STATS_14_QOSCTRL_15_8_TID7_MASK             0xff000000

/* Description		RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID8
			
			Field only valid when Received_qos_data_tid_bitmap[8] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID8
*/
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID8_OFFSET           0x0000003c
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID8_LSB              0
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID8_MASK             0x000000ff

/* Description		RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID9
			
			Field only valid when Received_qos_data_tid_bitmap[9] is
			set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID9
*/
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID9_OFFSET           0x0000003c
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID9_LSB              8
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID9_MASK             0x0000ff00

/* Description		RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID10
			
			Field only valid when Received_qos_data_tid_bitmap[10]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID10
*/
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID10_OFFSET          0x0000003c
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID10_LSB             16
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID10_MASK            0x00ff0000

/* Description		RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID11
			
			Field only valid when Received_qos_data_tid_bitmap[11]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID11
*/
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID11_OFFSET          0x0000003c
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID11_LSB             24
#define RX_PPDU_END_USER_STATS_15_QOSCTRL_15_8_TID11_MASK            0xff000000

/* Description		RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID12
			
			Field only valid when Received_qos_data_tid_bitmap[12]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID12
*/
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID12_OFFSET          0x00000040
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID12_LSB             0
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID12_MASK            0x000000ff

/* Description		RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID13
			
			Field only valid when Received_qos_data_tid_bitmap[13]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID13
*/
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID13_OFFSET          0x00000040
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID13_LSB             8
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID13_MASK            0x0000ff00

/* Description		RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID14
			
			Field only valid when Received_qos_data_tid_bitmap[14]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID14
*/
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID14_OFFSET          0x00000040
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID14_LSB             16
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID14_MASK            0x00ff0000

/* Description		RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID15
			
			Field only valid when Received_qos_data_tid_bitmap[15]
			is set
			
			
			
			QoS control field bits 15-8 of the last properly
			received MPDU with TID15
*/
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID15_OFFSET          0x00000040
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID15_LSB             24
#define RX_PPDU_END_USER_STATS_16_QOSCTRL_15_8_TID15_MASK            0xff000000


#endif // _RX_PPDU_END_USER_STATS_H_
