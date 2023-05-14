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

#ifndef _RX_PPDU_END_USER_STATS_H_
#define _RX_PPDU_END_USER_STATS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_rxpcu_classification_overview.h"
#define NUM_OF_DWORDS_RX_PPDU_END_USER_STATS 30

#define NUM_OF_QWORDS_RX_PPDU_END_USER_STATS 15


struct rx_ppdu_end_user_stats {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_rxpcu_classification_overview                          rxpcu_classification_details;
             uint32_t sta_full_aid                                            : 13, // [12:0]
                      mcs                                                     :  4, // [16:13]
                      nss                                                     :  3, // [19:17]
                      expected_response_ack_or_ba                             :  1, // [20:20]
                      reserved_1a                                             : 11; // [31:21]
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      mpdu_cnt_fcs_err                                        : 11, // [26:16]
                      sw2rxdma0_buf_source_used                               :  1, // [27:27]
                      fw2rxdma_pmac0_buf_source_used                          :  1, // [28:28]
                      sw2rxdma1_buf_source_used                               :  1, // [29:29]
                      sw2rxdma_exception_buf_source_used                      :  1, // [30:30]
                      fw2rxdma_pmac1_buf_source_used                          :  1; // [31:31]
             uint32_t mpdu_cnt_fcs_ok                                         : 11, // [10:0]
                      frame_control_info_valid                                :  1, // [11:11]
                      qos_control_info_valid                                  :  1, // [12:12]
                      ht_control_info_valid                                   :  1, // [13:13]
                      data_sequence_control_info_valid                        :  1, // [14:14]
                      ht_control_info_null_valid                              :  1, // [15:15]
                      rxdma2fw_pmac1_ring_used                                :  1, // [16:16]
                      rxdma2reo_ring_used                                     :  1, // [17:17]
                      rxdma2fw_pmac0_ring_used                                :  1, // [18:18]
                      rxdma2sw_ring_used                                      :  1, // [19:19]
                      rxdma_release_ring_used                                 :  1, // [20:20]
                      ht_control_field_pkt_type                               :  4, // [24:21]
                      rxdma2reo_remote0_ring_used                             :  1, // [25:25]
                      rxdma2reo_remote1_ring_used                             :  1, // [26:26]
                      reserved_3b                                             :  5; // [31:27]
             uint32_t ast_index                                               : 16, // [15:0]
                      frame_control_field                                     : 16; // [31:16]
             uint32_t first_data_seq_ctrl                                     : 16, // [15:0]
                      qos_control_field                                       : 16; // [31:16]
             uint32_t ht_control_field                                        : 32; // [31:0]
             uint32_t fcs_ok_bitmap_31_0                                      : 32; // [31:0]
             uint32_t fcs_ok_bitmap_63_32                                     : 32; // [31:0]
             uint32_t udp_msdu_count                                          : 16, // [15:0]
                      tcp_msdu_count                                          : 16; // [31:16]
             uint32_t other_msdu_count                                        : 16, // [15:0]
                      tcp_ack_msdu_count                                      : 16; // [31:16]
             uint32_t sw_response_reference_ptr                               : 32; // [31:0]
             uint32_t received_qos_data_tid_bitmap                            : 16, // [15:0]
                      received_qos_data_tid_eosp_bitmap                       : 16; // [31:16]
             uint32_t qosctrl_15_8_tid0                                       :  8, // [7:0]
                      qosctrl_15_8_tid1                                       :  8, // [15:8]
                      qosctrl_15_8_tid2                                       :  8, // [23:16]
                      qosctrl_15_8_tid3                                       :  8; // [31:24]
             uint32_t qosctrl_15_8_tid4                                       :  8, // [7:0]
                      qosctrl_15_8_tid5                                       :  8, // [15:8]
                      qosctrl_15_8_tid6                                       :  8, // [23:16]
                      qosctrl_15_8_tid7                                       :  8; // [31:24]
             uint32_t qosctrl_15_8_tid8                                       :  8, // [7:0]
                      qosctrl_15_8_tid9                                       :  8, // [15:8]
                      qosctrl_15_8_tid10                                      :  8, // [23:16]
                      qosctrl_15_8_tid11                                      :  8; // [31:24]
             uint32_t qosctrl_15_8_tid12                                      :  8, // [7:0]
                      qosctrl_15_8_tid13                                      :  8, // [15:8]
                      qosctrl_15_8_tid14                                      :  8, // [23:16]
                      qosctrl_15_8_tid15                                      :  8; // [31:24]
             uint32_t mpdu_ok_byte_count                                      : 25, // [24:0]
                      ampdu_delim_ok_count_6_0                                :  7; // [31:25]
             uint32_t ampdu_delim_err_count                                   : 25, // [24:0]
                      ampdu_delim_ok_count_13_7                               :  7; // [31:25]
             uint32_t mpdu_err_byte_count                                     : 25, // [24:0]
                      ampdu_delim_ok_count_20_14                              :  7; // [31:25]
             uint32_t non_consecutive_delimiter_err                           : 16, // [15:0]
                      retried_msdu_count                                      : 16; // [31:16]
             uint32_t ht_control_null_field                                   : 32; // [31:0]
             uint32_t sw_response_reference_ptr_ext                           : 32; // [31:0]
             uint32_t corrupted_due_to_fifo_delay                             :  1, // [0:0]
                      frame_control_info_null_valid                           :  1, // [1:1]
                      frame_control_field_null                                : 16, // [17:2]
                      retried_mpdu_count                                      : 11, // [28:18]
                      reserved_23a                                            :  3; // [31:29]
             uint32_t rxpcu_mpdu_filter_in_category                           :  2, // [1:0]
                      sw_frame_group_id                                       :  7, // [8:2]
                      reserved_24a                                            :  4, // [12:9]
                      frame_control_info_mgmt_ctrl_valid                      :  1, // [13:13]
                      mac_addr_ad2_valid                                      :  1, // [14:14]
                      mcast_bcast                                             :  1, // [15:15]
                      frame_control_field_mgmt_ctrl                           : 16; // [31:16]
             uint32_t user_ppdu_len                                           : 24, // [23:0]
                      reserved_25a                                            :  8; // [31:24]
             uint32_t mac_addr_ad2_31_0                                       : 32; // [31:0]
             uint32_t mac_addr_ad2_47_32                                      : 16, // [15:0]
                      amsdu_msdu_count                                        : 16; // [31:16]
             uint32_t non_amsdu_msdu_count                                    : 16, // [15:0]
                      ucast_msdu_count                                        : 16; // [31:16]
             uint32_t bcast_msdu_count                                        : 16, // [15:0]
                      mcast_bcast_msdu_count                                  : 16; // [31:16]
#else
             struct   rx_rxpcu_classification_overview                          rxpcu_classification_details;
             uint32_t reserved_1a                                             : 11, // [31:21]
                      expected_response_ack_or_ba                             :  1, // [20:20]
                      nss                                                     :  3, // [19:17]
                      mcs                                                     :  4, // [16:13]
                      sta_full_aid                                            : 13; // [12:0]
             uint32_t fw2rxdma_pmac1_buf_source_used                          :  1, // [31:31]
                      sw2rxdma_exception_buf_source_used                      :  1, // [30:30]
                      sw2rxdma1_buf_source_used                               :  1, // [29:29]
                      fw2rxdma_pmac0_buf_source_used                          :  1, // [28:28]
                      sw2rxdma0_buf_source_used                               :  1, // [27:27]
                      mpdu_cnt_fcs_err                                        : 11, // [26:16]
                      sw_peer_id                                              : 16; // [15:0]
             uint32_t reserved_3b                                             :  5, // [31:27]
                      rxdma2reo_remote1_ring_used                             :  1, // [26:26]
                      rxdma2reo_remote0_ring_used                             :  1, // [25:25]
                      ht_control_field_pkt_type                               :  4, // [24:21]
                      rxdma_release_ring_used                                 :  1, // [20:20]
                      rxdma2sw_ring_used                                      :  1, // [19:19]
                      rxdma2fw_pmac0_ring_used                                :  1, // [18:18]
                      rxdma2reo_ring_used                                     :  1, // [17:17]
                      rxdma2fw_pmac1_ring_used                                :  1, // [16:16]
                      ht_control_info_null_valid                              :  1, // [15:15]
                      data_sequence_control_info_valid                        :  1, // [14:14]
                      ht_control_info_valid                                   :  1, // [13:13]
                      qos_control_info_valid                                  :  1, // [12:12]
                      frame_control_info_valid                                :  1, // [11:11]
                      mpdu_cnt_fcs_ok                                         : 11; // [10:0]
             uint32_t frame_control_field                                     : 16, // [31:16]
                      ast_index                                               : 16; // [15:0]
             uint32_t qos_control_field                                       : 16, // [31:16]
                      first_data_seq_ctrl                                     : 16; // [15:0]
             uint32_t ht_control_field                                        : 32; // [31:0]
             uint32_t fcs_ok_bitmap_31_0                                      : 32; // [31:0]
             uint32_t fcs_ok_bitmap_63_32                                     : 32; // [31:0]
             uint32_t tcp_msdu_count                                          : 16, // [31:16]
                      udp_msdu_count                                          : 16; // [15:0]
             uint32_t tcp_ack_msdu_count                                      : 16, // [31:16]
                      other_msdu_count                                        : 16; // [15:0]
             uint32_t sw_response_reference_ptr                               : 32; // [31:0]
             uint32_t received_qos_data_tid_eosp_bitmap                       : 16, // [31:16]
                      received_qos_data_tid_bitmap                            : 16; // [15:0]
             uint32_t qosctrl_15_8_tid3                                       :  8, // [31:24]
                      qosctrl_15_8_tid2                                       :  8, // [23:16]
                      qosctrl_15_8_tid1                                       :  8, // [15:8]
                      qosctrl_15_8_tid0                                       :  8; // [7:0]
             uint32_t qosctrl_15_8_tid7                                       :  8, // [31:24]
                      qosctrl_15_8_tid6                                       :  8, // [23:16]
                      qosctrl_15_8_tid5                                       :  8, // [15:8]
                      qosctrl_15_8_tid4                                       :  8; // [7:0]
             uint32_t qosctrl_15_8_tid11                                      :  8, // [31:24]
                      qosctrl_15_8_tid10                                      :  8, // [23:16]
                      qosctrl_15_8_tid9                                       :  8, // [15:8]
                      qosctrl_15_8_tid8                                       :  8; // [7:0]
             uint32_t qosctrl_15_8_tid15                                      :  8, // [31:24]
                      qosctrl_15_8_tid14                                      :  8, // [23:16]
                      qosctrl_15_8_tid13                                      :  8, // [15:8]
                      qosctrl_15_8_tid12                                      :  8; // [7:0]
             uint32_t ampdu_delim_ok_count_6_0                                :  7, // [31:25]
                      mpdu_ok_byte_count                                      : 25; // [24:0]
             uint32_t ampdu_delim_ok_count_13_7                               :  7, // [31:25]
                      ampdu_delim_err_count                                   : 25; // [24:0]
             uint32_t ampdu_delim_ok_count_20_14                              :  7, // [31:25]
                      mpdu_err_byte_count                                     : 25; // [24:0]
             uint32_t retried_msdu_count                                      : 16, // [31:16]
                      non_consecutive_delimiter_err                           : 16; // [15:0]
             uint32_t ht_control_null_field                                   : 32; // [31:0]
             uint32_t sw_response_reference_ptr_ext                           : 32; // [31:0]
             uint32_t reserved_23a                                            :  3, // [31:29]
                      retried_mpdu_count                                      : 11, // [28:18]
                      frame_control_field_null                                : 16, // [17:2]
                      frame_control_info_null_valid                           :  1, // [1:1]
                      corrupted_due_to_fifo_delay                             :  1; // [0:0]
             uint32_t frame_control_field_mgmt_ctrl                           : 16, // [31:16]
                      mcast_bcast                                             :  1, // [15:15]
                      mac_addr_ad2_valid                                      :  1, // [14:14]
                      frame_control_info_mgmt_ctrl_valid                      :  1, // [13:13]
                      reserved_24a                                            :  4, // [12:9]
                      sw_frame_group_id                                       :  7, // [8:2]
                      rxpcu_mpdu_filter_in_category                           :  2; // [1:0]
             uint32_t reserved_25a                                            :  8, // [31:24]
                      user_ppdu_len                                           : 24; // [23:0]
             uint32_t mac_addr_ad2_31_0                                       : 32; // [31:0]
             uint32_t amsdu_msdu_count                                        : 16, // [31:16]
                      mac_addr_ad2_47_32                                      : 16; // [15:0]
             uint32_t ucast_msdu_count                                        : 16, // [31:16]
                      non_amsdu_msdu_count                                    : 16; // [15:0]
             uint32_t mcast_bcast_msdu_count                                  : 16, // [31:16]
                      bcast_msdu_count                                        : 16; // [15:0]
#endif
};


/* Description		RXPCU_CLASSIFICATION_DETAILS

			Details related to what RXPCU classification types of MPDUs
			 have been received
*/


/* Description		FILTER_PASS_MPDUS

			When set, at least one Filter Pass MPDU has been received. 
			FCS might or might not have been passing.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_LSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MASK  0x0000000000000001


/* Description		FILTER_PASS_MPDUS_FCS_OK

			When set, at least one Filter Pass MPDU has been received
			 that has a correct FCS.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_LSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MASK 0x0000000000000002


/* Description		MONITOR_DIRECT_MPDUS

			When set, at least one Monitor Direct MPDU has been received. 
			FCS might or might not have been passing
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_LSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MASK 0x0000000000000004


/* Description		MONITOR_DIRECT_MPDUS_FCS_OK

			When set, at least one Monitor Direct MPDU has been received
			 that has a correct FCS.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_LSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MASK 0x0000000000000008


/* Description		MONITOR_OTHER_MPDUS

			When set, at least one Monitor Direct MPDU has been received. 
			FCS might or might not have been passing.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_LSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MASK 0x0000000000000010


/* Description		MONITOR_OTHER_MPDUS_FCS_OK

			When set, at least one Monitor Direct MPDU has been received
			 that has a correct FCS.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_LSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MASK 0x0000000000000020


/* Description		PHYRX_ABORT_RECEIVED

			When set, PPDU reception was aborted by the PHY
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_LSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MASK 0x0000000000000040


/* Description		FILTER_PASS_MONITOR_OVRD_MPDUS

			When set, at least one 'Filter Pass Monitor Override' MPDU
			 has been received. FCS might or might not have been passing.
			
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_LSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MASK 0x0000000000000080


/* Description		FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK

			When set, at least one 'Filter Pass Monitor Override' MPDU
			 has been received that has a correct FCS.
			
			For MU UL, in  TLVs RX_PPDU_END and RX_PPDU_END_STATUS_DONE, 
			this field is the "OR of all the users.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_LSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MASK 0x0000000000000100


/* Description		RESERVED_0

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_OFFSET       0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_LSB          9
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MSB          15
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MASK         0x000000000000fe00


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_OFFSET      0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_LSB         16
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MSB         31
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MASK        0x00000000ffff0000


/* Description		STA_FULL_AID

			Consumer: FW
			Producer: RXPCU
			
			The full AID of this station. 
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_STA_FULL_AID_OFFSET                                  0x0000000000000000
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_LSB                                     32
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MSB                                     44
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MASK                                    0x00001fff00000000


/* Description		MCS

			MCS of the received frame
			
			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MCS_OFFSET                                           0x0000000000000000
#define RX_PPDU_END_USER_STATS_MCS_LSB                                              45
#define RX_PPDU_END_USER_STATS_MCS_MSB                                              48
#define RX_PPDU_END_USER_STATS_MCS_MASK                                             0x0001e00000000000


/* Description		NSS

			Number of spatial streams.
			
			NOTE: RXPCU derives this from the 'Mimo_ss_bitmap'
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define RX_PPDU_END_USER_STATS_NSS_OFFSET                                           0x0000000000000000
#define RX_PPDU_END_USER_STATS_NSS_LSB                                              49
#define RX_PPDU_END_USER_STATS_NSS_MSB                                              51
#define RX_PPDU_END_USER_STATS_NSS_MASK                                             0x000e000000000000


/* Description		EXPECTED_RESPONSE_ACK_OR_BA

			When set, it indicates an Ack or BA matching 'EXPECTED_RESPONSE' 
			from TXPCU
*/

#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_OFFSET                   0x0000000000000000
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_LSB                      52
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MSB                      52
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MASK                     0x0010000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RESERVED_1A_OFFSET                                   0x0000000000000000
#define RX_PPDU_END_USER_STATS_RESERVED_1A_LSB                                      53
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MSB                                      63
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MASK                                     0xffe0000000000000


/* Description		SW_PEER_ID

			This field indicates a unique peer identifier, set from 
			the field 'sw_peer_id' in the AST entry corresponding to
			 this MPDU. It is provided by RXPCU.
			A value of 0xFFFF indicates no AST entry was found or no
			 AST search was performed.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_SW_PEER_ID_OFFSET                                    0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_LSB                                       0
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MSB                                       15
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MASK                                      0x000000000000ffff


/* Description		MPDU_CNT_FCS_ERR

			The number of MPDUs received from this STA in this PPDU 
			with FCS errors
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_OFFSET                              0x0000000000000008
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_LSB                                 16
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MSB                                 26
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MASK                                0x0000000007ff0000


/* Description		SW2RXDMA0_BUF_SOURCE_USED

			Field filled in by RXDMA
			
			When set, RXDMA has used the sw2rxdma0 buffer ring as source
			 for at least one of the frames in this PPDU.
*/

#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_LSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MASK                       0x0000000008000000


/* Description		FW2RXDMA_PMAC0_BUF_SOURCE_USED

			Field filled in by RXDMA
			
			When set, RXDMA has used the fw2rxdma buffer ring for PMAC0
			 as source for at least one of the frames in this PPDU.
*/

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_OFFSET                0x0000000000000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_LSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MASK                  0x0000000010000000


/* Description		SW2RXDMA1_BUF_SOURCE_USED

			Field filled in by RXDMA
			
			When set, RXDMA has used the sw2rxdma1 buffer ring as source
			 for at least one of the frames in this PPDU.
*/

#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_LSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MASK                       0x0000000020000000


/* Description		SW2RXDMA_EXCEPTION_BUF_SOURCE_USED

			Field filled in by RXDMA
			
			When set, RXDMA has used the sw2rxdma_exception buffer ring
			 as source for at least one of the frames in this PPDU.
*/

#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_OFFSET            0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_LSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MASK              0x0000000040000000


/* Description		FW2RXDMA_PMAC1_BUF_SOURCE_USED

			Field filled in by RXDMA
			
			When set, RXDMA has used the fw2rxdma buffer ring for PMAC1
			 as source for at least one of the frames in this PPDU.
*/

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_OFFSET                0x0000000000000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_LSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MASK                  0x0000000080000000


/* Description		MPDU_CNT_FCS_OK

			The number of MPDUs received from this STA in this PPDU 
			with correct FCS
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_OFFSET                               0x0000000000000008
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_LSB                                  32
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MSB                                  42
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MASK                                 0x000007ff00000000


/* Description		FRAME_CONTROL_INFO_VALID

			When set, the frame_control_info field contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_LSB                         43
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MSB                         43
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MASK                        0x0000080000000000


/* Description		QOS_CONTROL_INFO_VALID

			When set, the QoS_control_info field contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_OFFSET                        0x0000000000000008
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_LSB                           44
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MSB                           44
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MASK                          0x0000100000000000


/* Description		HT_CONTROL_INFO_VALID

			When set, the HT_control_field contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_OFFSET                         0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_LSB                            45
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MSB                            45
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MASK                           0x0000200000000000


/* Description		DATA_SEQUENCE_CONTROL_INFO_VALID

			When set, the First_data_seq_ctrl field contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_OFFSET              0x0000000000000008
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_LSB                 46
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MSB                 46
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MASK                0x0000400000000000


/* Description		HT_CONTROL_INFO_NULL_VALID

			When set, the HT_control_NULL_field contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_OFFSET                    0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_LSB                       47
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MSB                       47
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MASK                      0x0000800000000000


/* Description		RXDMA2FW_PMAC1_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_LSB                         48
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MSB                         48
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MASK                        0x0001000000000000


/* Description		RXDMA2REO_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_OFFSET                           0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_LSB                              49
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MSB                              49
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MASK                             0x0002000000000000


/* Description		RXDMA2FW_PMAC0_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_LSB                         50
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MSB                         50
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MASK                        0x0004000000000000


/* Description		RXDMA2SW_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_OFFSET                            0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_LSB                               51
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MSB                               51
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MASK                              0x0008000000000000


/* Description		RXDMA_RELEASE_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_OFFSET                       0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_LSB                          52
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MSB                          52
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MASK                         0x0010000000000000


/* Description		HT_CONTROL_FIELD_PKT_TYPE

			Field only valid when HT_control_info_valid or HT_control_info_NULL_valid
			    is set.
			
			Indicates what the PHY receive type was for receiving this
			 frame. Can help determine if the HT_CONTROL field shall
			 be interpreted as HT/VHT or HE.
			
			NOTE: later on in the 11ax IEEE spec a bit within the HT
			 control field was introduced that explicitly indicated 
			how to interpret the HT control field.... As HT, VHT, or
			 HE.
			
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

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_LSB                        53
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MSB                        56
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MASK                       0x01e0000000000000


/* Description		RXDMA2REO_REMOTE0_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_OFFSET                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_LSB                      57
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MSB                      57
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MASK                     0x0200000000000000


/* Description		RXDMA2REO_REMOTE1_RING_USED

			Field filled in by RXDMA
			
			Set when at least one frame during this PPDU got pushed 
			to this ring by RXDMA
*/

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_OFFSET                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_LSB                      58
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MSB                      58
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MASK                     0x0400000000000000


/* Description		RESERVED_3B

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RESERVED_3B_OFFSET                                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RESERVED_3B_LSB                                      59
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MSB                                      63
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MASK                                     0xf800000000000000


/* Description		AST_INDEX

			This field indicates the index of the AST entry corresponding
			 to this MPDU. It is provided by the GSE module instantiated
			 in RXPCU.
			A value of 0xFFFF indicates an invalid AST index, meaning
			 that No AST entry was found or NO AST search was performed
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AST_INDEX_OFFSET                                     0x0000000000000010
#define RX_PPDU_END_USER_STATS_AST_INDEX_LSB                                        0
#define RX_PPDU_END_USER_STATS_AST_INDEX_MSB                                        15
#define RX_PPDU_END_USER_STATS_AST_INDEX_MASK                                       0x000000000000ffff


/* Description		FRAME_CONTROL_FIELD

			Field only valid when Frame_control_info_valid is set.
			
			Last successfully received Frame_control field of data frame
			 (excluding Data NULL/ QoS Null) for this user
			Mainly used to track the PM state of the transmitted device
			
			
			NOTE: only data frame info is needed, as control and management
			 frames are already routed to the FW.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_OFFSET                           0x0000000000000010
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_LSB                              16
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MSB                              31
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MASK                             0x00000000ffff0000


/* Description		FIRST_DATA_SEQ_CTRL

			Field only valid when Data_sequence_control_info_valid is
			 set.
			
			Sequence control field of the first data frame (excluding
			 Data NULL or QoS Data null) received for this user with
			 correct FCS
			
			NOTE: only data frame info is needed, as control and management
			 frames are already routed to the FW.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_OFFSET                           0x0000000000000010
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_LSB                              32
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MSB                              47
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MASK                             0x0000ffff00000000


/* Description		QOS_CONTROL_FIELD

			Field only valid when QoS_control_info_valid is set.
			
			Last successfully received QoS_control field of data frame
			 (excluding Data NULL/ QoS Null) for this user
			
			Note that in case of multi TID, this field can only reflect
			 the last properly received MPDU, and thus can not indicate
			 all potentially different TIDs that had been received earlier. 
			
			
			There are however per TID fields, that will contain among
			 other things all buffer status info: See
			QoSCtrl_15_8_tid???
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_OFFSET                             0x0000000000000010
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_LSB                                48
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MSB                                63
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MASK                               0xffff000000000000


/* Description		HT_CONTROL_FIELD

			Field only valid when HT_control_info_valid is set.
			
			Last successfully received HT_CONTROL/VHT_CONTROL/HE_CONTROL
			  field of data frames, excluding QoS Null frames for this
			 user. 
			
			NOTE: HT control fields  from QoS Null frames are captured
			 in field HT_control_NULL_field
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_OFFSET                              0x0000000000000018
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_LSB                                 0
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MSB                                 31
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MASK                                0x00000000ffffffff


/* Description		FCS_OK_BITMAP_31_0

			Bitmap indicates in order of received MPDUs, which MPDUs
			 had an passing FCS or had an error.
			1: FCS OK
			0: FCS error
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_OFFSET                            0x0000000000000018
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_LSB                               32
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MSB                               63
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MASK                              0xffffffff00000000


/* Description		FCS_OK_BITMAP_63_32

			Bitmap indicates in order of received MPDUs, which MPDUs
			 had an passing FCS or had an error.
			1: FCS OK
			0: FCS error
			
			NOTE: for users 0, 1, 2 and 3, additional bitmap info (up
			 to 256 bitmap window) is provided in RX_PPDU_END_USER_STATS_EXT
			 TLV
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_OFFSET                           0x0000000000000020
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_LSB                              0
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MSB                              31
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MASK                             0x00000000ffffffff


/* Description		UDP_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that contain UDP frames.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_OFFSET                                0x0000000000000020
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_LSB                                   32
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MSB                                   47
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MASK                                  0x0000ffff00000000


/* Description		TCP_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that contain TCP frames.
			
			(Note: This does NOT include TCP-ACK)
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_OFFSET                                0x0000000000000020
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_LSB                                   48
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MSB                                   63
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MASK                                  0xffff000000000000


/* Description		OTHER_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that contain neither UDP or TCP frames.
			
			Includes Management and control frames.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_OFFSET                              0x0000000000000028
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_LSB                                 0
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MSB                                 15
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MASK                                0x000000000000ffff


/* Description		TCP_ACK_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that contain TCP ack frames.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_OFFSET                            0x0000000000000028
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MASK                              0x00000000ffff0000


/* Description		SW_RESPONSE_REFERENCE_PTR

			Pointer that SW uses to refer back to an expected response
			 reception. Used for Rate adaptation purposes.
			When a reception occurs that is not tied to an expected 
			response, this field is set to 0x0.
			
			Note: further on in this TLV there is also the field: Sw_response_reference_ptr_ext.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_OFFSET                     0x0000000000000028
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_LSB                        32
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MSB                        63
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MASK                       0xffffffff00000000


/* Description		RECEIVED_QOS_DATA_TID_BITMAP

			Whenever a frame is received that contains a QoS control
			 field (that includes QoS Data and/or QoS Null), the bit
			 in this field that corresponds to the received TID shall
			 be set.
			...Bitmap[0] = TID0
			...Bitmap[1] = TID1
			Etc.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_OFFSET                  0x0000000000000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_LSB                     0
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MSB                     15
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MASK                    0x000000000000ffff


/* Description		RECEIVED_QOS_DATA_TID_EOSP_BITMAP

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

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_OFFSET             0x0000000000000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_LSB                16
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MSB                31
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MASK               0x00000000ffff0000


/* Description		QOSCTRL_15_8_TID0

			Field only valid when Received_qos_data_tid_bitmap[0] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 0
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_LSB                                32
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MSB                                39
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MASK                               0x000000ff00000000


/* Description		QOSCTRL_15_8_TID1

			Field only valid when Received_qos_data_tid_bitmap[1] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 1
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_LSB                                40
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MSB                                47
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MASK                               0x0000ff0000000000


/* Description		QOSCTRL_15_8_TID2

			Field only valid when Received_qos_data_tid_bitmap[2] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 2
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_LSB                                48
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MSB                                55
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MASK                               0x00ff000000000000


/* Description		QOSCTRL_15_8_TID3

			Field only valid when Received_qos_data_tid_bitmap[3] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 3
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_LSB                                56
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MSB                                63
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MASK                               0xff00000000000000


/* Description		QOSCTRL_15_8_TID4

			Field only valid when Received_qos_data_tid_bitmap[4] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 4
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_LSB                                0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MSB                                7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MASK                               0x00000000000000ff


/* Description		QOSCTRL_15_8_TID5

			Field only valid when Received_qos_data_tid_bitmap[5] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 5
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_LSB                                8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MSB                                15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MASK                               0x000000000000ff00


/* Description		QOSCTRL_15_8_TID6

			Field only valid when Received_qos_data_tid_bitmap[6] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 6
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_LSB                                16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MSB                                23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MASK                               0x0000000000ff0000


/* Description		QOSCTRL_15_8_TID7

			Field only valid when Received_qos_data_tid_bitmap[7] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 7
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_LSB                                24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MSB                                31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MASK                               0x00000000ff000000


/* Description		QOSCTRL_15_8_TID8

			Field only valid when Received_qos_data_tid_bitmap[8] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 8
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_LSB                                32
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MSB                                39
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MASK                               0x000000ff00000000


/* Description		QOSCTRL_15_8_TID9

			Field only valid when Received_qos_data_tid_bitmap[9] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 9
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_LSB                                40
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MSB                                47
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MASK                               0x0000ff0000000000


/* Description		QOSCTRL_15_8_TID10

			Field only valid when Received_qos_data_tid_bitmap[10] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 10
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_OFFSET                            0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_LSB                               48
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MSB                               55
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MASK                              0x00ff000000000000


/* Description		QOSCTRL_15_8_TID11

			Field only valid when Received_qos_data_tid_bitmap[11] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 11
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_OFFSET                            0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_LSB                               56
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MSB                               63
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MASK                              0xff00000000000000


/* Description		QOSCTRL_15_8_TID12

			Field only valid when Received_qos_data_tid_bitmap[12] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 12
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_LSB                               0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MSB                               7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MASK                              0x00000000000000ff


/* Description		QOSCTRL_15_8_TID13

			Field only valid when Received_qos_data_tid_bitmap[13] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 13
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_LSB                               8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MSB                               15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MASK                              0x000000000000ff00


/* Description		QOSCTRL_15_8_TID14

			Field only valid when Received_qos_data_tid_bitmap[14] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 14
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_LSB                               16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MSB                               23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MASK                              0x0000000000ff0000


/* Description		QOSCTRL_15_8_TID15

			Field only valid when Received_qos_data_tid_bitmap[15] is
			 set
			
			QoS control field bits 15-8 of the last properly received
			 MPDU with a QoS control field embedded, with  TID == 15
			
*/

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_LSB                               24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MSB                               31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MASK                              0x00000000ff000000


/* Description		MPDU_OK_BYTE_COUNT

			The number of bytes received within an MPDU for this user
			 with correct FCS. This includes the FCS field
			
			NOTE:
			The sum of the four fields.....
			Mpdu_ok_byte_count +
			mpdu_err_byte_count +
			(Ampdu_delim_ok_count x 4) + (Ampdu_delim_err_count x 4)
			
			.....is the total number of bytes that were received for
			 this user from the PHY.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_LSB                               32
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MSB                               56
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MASK                              0x01ffffff00000000


/* Description		AMPDU_DELIM_OK_COUNT_6_0

			Number of AMPDU delimiter received with correct structure
			
			LSB 7 bits from this counter
			
			Note that this is a delimiter count and not byte count. 
			To get to the number of bytes occupied by these delimiters, 
			multiply this number by 4
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_OFFSET                      0x0000000000000040
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_LSB                         57
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MSB                         63
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MASK                        0xfe00000000000000


/* Description		AMPDU_DELIM_ERR_COUNT

			The number of MPDU delimiter errors counted for this user.
			
			
			Note that this is a delimiter count and not byte count. 
			To get to the number of bytes occupied by these delimiters, 
			multiply this number by 4
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_OFFSET                         0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_LSB                            0
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MSB                            24
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MASK                           0x0000000001ffffff


/* Description		AMPDU_DELIM_OK_COUNT_13_7

			Number of AMPDU delimiters received with correct structure
			
			Bits 13-7 from this counter
			
			Note that this is a delimiter count and not byte count. 
			To get to the number of bytes occupied by these delimiters, 
			multiply this number by 4
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_OFFSET                     0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_LSB                        25
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MSB                        31
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MASK                       0x00000000fe000000


/* Description		MPDU_ERR_BYTE_COUNT

			The number of bytes belonging to MPDUs with an FCS error. 
			This includes the FCS field.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_OFFSET                           0x0000000000000048
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_LSB                              32
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MSB                              56
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MASK                             0x01ffffff00000000


/* Description		AMPDU_DELIM_OK_COUNT_20_14

			Number of AMPDU delimiters received with correct structure
			
			Bits 20-14 from this counter
			
			Note that this is a delimiter count and not byte count. 
			To get to the number of bytes occupied by these delimiters, 
			multiply this number by 4
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_OFFSET                    0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_LSB                       57
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MSB                       63
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MASK                      0xfe00000000000000


/* Description		NON_CONSECUTIVE_DELIMITER_ERR

			The number of times an MPDU delimiter error is detected 
			that is not immediately preceded by another MPDU delimiter
			 also with FCS error.
			
			The counter saturates at 0xFFFF
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_OFFSET                 0x0000000000000050
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_LSB                    0
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MSB                    15
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MASK                   0x000000000000ffff


/* Description		RETRIED_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that have the retry bit set.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_OFFSET                            0x0000000000000050
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MASK                              0x00000000ffff0000


/* Description		HT_CONTROL_NULL_FIELD

			Field only valid when HT_control_info_NULL_valid is set.
			
			
			Last successfully received HT_CONTROL/VHT_CONTROL/HE_CONTROL
			  field from QoS Null frame for this user. 
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_OFFSET                         0x0000000000000050
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_LSB                            32
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MSB                            63
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MASK                           0xffffffff00000000


/* Description		SW_RESPONSE_REFERENCE_PTR_EXT

			Extended Pointer info that SW uses to refer back to an expected
			 response transmission. Used for Rate adaptation purposes.
			
			When a reception occurs that is not tied to an expected 
			response, this field is set to 0x0.
			
			Note: earlier on in this TLV there is also the field: Sw_response_reference_ptr.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_OFFSET                 0x0000000000000058
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_LSB                    0
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MSB                    31
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MASK                   0x00000000ffffffff


/* Description		CORRUPTED_DUE_TO_FIFO_DELAY

			Set if Rx PCU avoided a hang due to SFM delays by writing
			 a corrupted 'RX_PPDU_END_USER_STATS*' and/or 'RX_PPDU_END.'
			
*/

#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_OFFSET                   0x0000000000000058
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_LSB                      32
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MSB                      32
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MASK                     0x0000000100000000


/* Description		FRAME_CONTROL_INFO_NULL_VALID

			When set, Frame_control_field_null contains valid information
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_OFFSET                 0x0000000000000058
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_LSB                    33
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MSB                    33
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MASK                   0x0000000200000000


/* Description		FRAME_CONTROL_FIELD_NULL

			Field only valid when Frame_control_info_null_valid is set.
			
			 
			Last successfully received Frame_control field of Data Null/QoS
			 Null for this user, mainly used to track the PM state of
			 the transmitted device
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_OFFSET                      0x0000000000000058
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_LSB                         34
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MSB                         49
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MASK                        0x0003fffc00000000


/* Description		RETRIED_MPDU_COUNT

			Field filled in by RXPCU
			
			The number of MPDUs without FCS error, that have the retry
			 bit set.
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_OFFSET                            0x0000000000000058
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_LSB                               50
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MSB                               60
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MASK                              0x1ffc000000000000


/* Description		RESERVED_23A

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RESERVED_23A_OFFSET                                  0x0000000000000058
#define RX_PPDU_END_USER_STATS_RESERVED_23A_LSB                                     61
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MSB                                     63
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MASK                                    0xe000000000000000


/* Description		RXPCU_MPDU_FILTER_IN_CATEGORY

			Field indicates what the reason was that the last successfully
			 received MPDU was allowed to come into the receive path
			 by RXPCU.
			<enum 0 rxpcu_filter_pass> The last MPDU passed the normal
			 frame filter programming of rxpcu
			<enum 1 rxpcu_monitor_client> The last MPDU did NOT pass
			 the regular frame filter and would have been dropped, were
			 it not for the frame fitting into the 'monitor_client' 
			category.
			<enum 2 rxpcu_monitor_other> The last MPDU did NOT pass 
			the regular frame filter and also did not pass the rxpcu_monitor_client
			 filter. It would have been dropped accept that it did pass
			 the 'monitor_other' category.
			<enum 3 rxpcu_filter_pass_monitor_ovrd> The last MPDU passed
			 the normal frame filter programming of RXPCU but additionally
			 fit into the 'monitor_override_client' category.
			
			<legal 0-3>
*/

#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                 0x0000000000000060
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                    0
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                    1
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                   0x0000000000000003


/* Description		SW_FRAME_GROUP_ID

			SW processes frames based on certain classifications. This
			 field indicates to what sw classification the last successfully
			 received MPDU is mapped.
			The classification is given in priority order
			
			<enum 0 sw_frame_group_NDP_frame> 
			
			<enum 1 sw_frame_group_Multicast_data> 
			<enum 2 sw_frame_group_Unicast_data> 
			<enum 3 sw_frame_group_Null_data > This includes mpdus of
			 type Data Null.
			<enum 38 sw_frame_group_QoS_Null_data> This includes QoS
			 Null frames except in UL MU or TB PPDUs.
			<enum 39 sw_frame_group_QoS_Null_data_TB> This includes 
			QoS Null frames in UL MU or TB PPDUs.
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			<enum 5 sw_frame_group_mgmt_0001 > 
			<enum 6 sw_frame_group_mgmt_0010 > 
			<enum 7 sw_frame_group_mgmt_0011 > 
			<enum 8 sw_frame_group_mgmt_0100 > 
			<enum 9 sw_frame_group_mgmt_0101 > 
			<enum 10 sw_frame_group_mgmt_0110 > 
			<enum 11 sw_frame_group_mgmt_0111 > 
			<enum 12 sw_frame_group_mgmt_1000 > 
			<enum 13 sw_frame_group_mgmt_1001 > 
			<enum 14 sw_frame_group_mgmt_1010 > 
			<enum 15 sw_frame_group_mgmt_1011 > 
			<enum 16 sw_frame_group_mgmt_1100 > 
			<enum 17 sw_frame_group_mgmt_1101 > 
			<enum 18 sw_frame_group_mgmt_1110 > 
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			<enum 21 sw_frame_group_ctrl_0001 > 
			<enum 22 sw_frame_group_ctrl_0010 > 
			<enum 23 sw_frame_group_ctrl_0011 > 
			<enum 24 sw_frame_group_ctrl_0100 > 
			<enum 25 sw_frame_group_ctrl_0101 > 
			<enum 26 sw_frame_group_ctrl_0110 > 
			<enum 27 sw_frame_group_ctrl_0111 > 
			<enum 28 sw_frame_group_ctrl_1000 > 
			<enum 29 sw_frame_group_ctrl_1001 > 
			<enum 30 sw_frame_group_ctrl_1010 > 
			<enum 31 sw_frame_group_ctrl_1011 > 
			<enum 32 sw_frame_group_ctrl_1100 > 
			<enum 33 sw_frame_group_ctrl_1101 > 
			<enum 34 sw_frame_group_ctrl_1110 > 
			<enum 35 sw_frame_group_ctrl_1111 > 
			
			<enum 36 sw_frame_group_unsupported> This covers type 3 
			and protocol version != 0
			
			<enum 37 sw_frame_group_phy_error> PHY reported an error
			
			
			<legal 0-39>
*/

#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_OFFSET                             0x0000000000000060
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_LSB                                2
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_MSB                                8
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_MASK                               0x00000000000001fc


/* Description		RESERVED_24A

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RESERVED_24A_OFFSET                                  0x0000000000000060
#define RX_PPDU_END_USER_STATS_RESERVED_24A_LSB                                     9
#define RX_PPDU_END_USER_STATS_RESERVED_24A_MSB                                     12
#define RX_PPDU_END_USER_STATS_RESERVED_24A_MASK                                    0x0000000000001e00


/* Description		FRAME_CONTROL_INFO_MGMT_CTRL_VALID

			When set, Frame_control_field_mgmt_ctrl contains valid information.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_OFFSET            0x0000000000000060
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_LSB               13
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_MSB               13
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_MASK              0x0000000000002000


/* Description		MAC_ADDR_AD2_VALID

			When set, the fields mac_addr_ad2_... contain valid information.
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_OFFSET                            0x0000000000000060
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_LSB                               14
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_MSB                               14
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_MASK                              0x0000000000004000


/* Description		MCAST_BCAST

			Multicast / broadcast indicator
			
			Only set when the MAC address 1 bit 0 is set indicating 
			mcast/bcast and the BSSID matches one of the BSSID registers, 
			for the last successfully received MPDU
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MCAST_BCAST_OFFSET                                   0x0000000000000060
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_LSB                                      15
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSB                                      15
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MASK                                     0x0000000000008000


/* Description		FRAME_CONTROL_FIELD_MGMT_CTRL

			Field only valid when Frame_control_info_mgmt_ctrl_valid
			 is set
			
			Last successfully received 'Frame control' field of control
			 or management frames for this user, mainly used in Rx monitor
			 mode
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_OFFSET                 0x0000000000000060
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_LSB                    16
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_MSB                    31
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_MASK                   0x00000000ffff0000


/* Description		USER_PPDU_LEN

			The sum of the mpdu_length fields of all the 'RX_MPDU_START' 
			TLVs generated for this user  for this PPDU
*/

#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_OFFSET                                 0x0000000000000060
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_LSB                                    32
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_MSB                                    55
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_MASK                                   0x00ffffff00000000


/* Description		RESERVED_25A

			<legal 0>
*/

#define RX_PPDU_END_USER_STATS_RESERVED_25A_OFFSET                                  0x0000000000000060
#define RX_PPDU_END_USER_STATS_RESERVED_25A_LSB                                     56
#define RX_PPDU_END_USER_STATS_RESERVED_25A_MSB                                     63
#define RX_PPDU_END_USER_STATS_RESERVED_25A_MASK                                    0xff00000000000000


/* Description		MAC_ADDR_AD2_31_0

			Field only valid when mac_addr_ad2_valid is set
			
			The least significant 4 bytes of the last successfully received
			 frame's MAC Address AD2
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_OFFSET                             0x0000000000000068
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_LSB                                0
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_MSB                                31
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_MASK                               0x00000000ffffffff


/* Description		MAC_ADDR_AD2_47_32

			Field only valid when mac_addr_ad2_valid is set
			
			The 2 most significant bytes of the last successfully received
			 frame's MAC Address AD2
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_OFFSET                            0x0000000000000068
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_LSB                               32
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_MSB                               47
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_MASK                              0x0000ffff00000000


/* Description		AMSDU_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of A-MSDUs that are part
			 of MPDUs without FCS error
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_OFFSET                              0x0000000000000068
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_LSB                                 48
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_MSB                                 63
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_MASK                                0xffff000000000000


/* Description		NON_AMSDU_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are not part of A-MSDUs that are
			 part of MPDUs without FCS error
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_OFFSET                          0x0000000000000070
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_LSB                             0
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_MSB                             15
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_MASK                            0x000000000000ffff


/* Description		UCAST_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			that are directed to a unicast destination address
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_OFFSET                              0x0000000000000070
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_LSB                                 16
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_MSB                                 31
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_MASK                                0x00000000ffff0000


/* Description		BCAST_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			whose destination addresses are broadcast (0xFFFF_FFFF_FFFF)
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_OFFSET                              0x0000000000000070
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_LSB                                 32
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_MSB                                 47
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_MASK                                0x0000ffff00000000


/* Description		MCAST_BCAST_MSDU_COUNT

			Field filled in by RX OLE
			Set to 0 by RXPCU
			
			The number of MSDUs that are part of MPDUs without FCS error, 
			whose destination addresses are either multicast or broadcast
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_OFFSET                        0x0000000000000070
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_LSB                           48
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_MSB                           63
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_MASK                          0xffff000000000000



#endif   // RX_PPDU_END_USER_STATS
