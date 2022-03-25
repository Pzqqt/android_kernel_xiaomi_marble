/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "qdf_types.h"
#include "qdf_util.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "qdf_module.h"

#include "target_type.h"
#include "wcss_version.h"

#include "hal_be_hw_headers.h"
#include "hal_internal.h"
#include "hal_api.h"
#include "hal_flow.h"
#include "rx_flow_search_entry.h"
#include "hal_rx_flow_info.h"
#include "hal_be_api.h"
#include "tcl_entrance_from_ppe_ring.h"
#include "sw_monitor_ring.h"
#include "wcss_seq_hwioreg_umac.h"
#include "wfss_ce_reg_seq_hwioreg.h"
#include <uniform_reo_status_header.h>
#include <wbm_release_ring_tx.h>
#include <wbm_release_ring_rx.h>
#include <phyrx_location.h>

#include <hal_be_rx.h>

#define UNIFIED_RX_MPDU_START_0_RX_MPDU_INFO_RX_MPDU_INFO_DETAILS_OFFSET \
	RX_MPDU_START_RX_MPDU_INFO_DETAILS_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_MSDU_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET
#define UNIFIED_RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET
#define UNIFIED_REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	REO_DESTINATION_RING_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET
#define UNIFORM_REO_STATUS_HEADER_STATUS_HEADER \
	STATUS_HEADER_REO_STATUS_NUMBER
#define UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC \
	STATUS_HEADER_TIMESTAMP
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_MSDU_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_0_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_1_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_OFFSET
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB \
	BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK \
	BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB \
	BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK \
	BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB \
	BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK \
	BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB \
	BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK \
	BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_MASK
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_LSB
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK \
	WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET \
	WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB \
	WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB

#include "hal_be_api_mon.h"

#ifdef CONFIG_WIFI_EMULATION_WIFI_3_0
#define CMEM_REG_BASE 0x0010e000

#define CMEM_WINDOW_ADDRESS_9224 \
		((CMEM_REG_BASE >> WINDOW_SHIFT) & WINDOW_VALUE_MASK)
#endif

#define CE_WINDOW_ADDRESS_9224 \
		((CE_WFSS_CE_REG_BASE >> WINDOW_SHIFT) & WINDOW_VALUE_MASK)

#define UMAC_WINDOW_ADDRESS_9224 \
		((UMAC_BASE >> WINDOW_SHIFT) & WINDOW_VALUE_MASK)

#ifdef CONFIG_WIFI_EMULATION_WIFI_3_0
#define WINDOW_CONFIGURATION_VALUE_9224 \
		((CE_WINDOW_ADDRESS_9224 << 6) |\
		 (UMAC_WINDOW_ADDRESS_9224 << 12) | \
		 CMEM_WINDOW_ADDRESS_9224 | \
		 WINDOW_ENABLE_BIT)
#else
#define WINDOW_CONFIGURATION_VALUE_9224 \
		((CE_WINDOW_ADDRESS_9224 << 6) |\
		 (UMAC_WINDOW_ADDRESS_9224 << 12) | \
		 WINDOW_ENABLE_BIT)
#endif

/* For Berryllium sw2rxdma ring size increased to 20 bits */
#define HAL_RXDMA_MAX_RING_SIZE_BE 0xFFFFF

#ifdef CONFIG_WORD_BASED_TLV
#ifndef BIG_ENDIAN_HOST
struct rx_msdu_end_compact_qca9224 {
	uint32_t rxpcu_mpdu_filter_in_category		:  2, // [1:0]
		 sw_frame_group_id			:  7, // [8:2]
		 reserved_0				:  7, // [15:9]
		 phy_ppdu_id				: 16; // [31:16]
	uint32_t ip_hdr_chksum				: 16, // [15:0]
		 reported_mpdu_length			: 14, // [29:16]
		 reserved_1a				:  2; // [31:30]
	uint32_t key_id_octet				:  8, // [7:0]
		 cce_super_rule				:  6, // [13:8]
		 cce_classify_not_done_truncate		:  1, // [14:14]
		 cce_classify_not_done_cce_dis		:  1, // [15:15]
		 cumulative_l3_checksum			: 16; // [31:16]
	uint32_t rule_indication_31_0			: 32; // [31:0]
	uint32_t rule_indication_63_32			: 32; // [31:0]
	uint32_t da_offset				:  6, // [5:0]
		 sa_offset				:  6, // [11:6]
		 da_offset_valid			:  1, // [12:12]
		 sa_offset_valid			:  1, // [13:13]
		 reserved_5a				:  2, // [15:14]
		 l3_type				: 16; // [31:16]
	uint32_t ipv6_options_crc			: 32; // [31:0]
	uint32_t tcp_seq_number				: 32; // [31:0]
	uint32_t tcp_ack_number				: 32; // [31:0]
	uint32_t tcp_flag				:  9, // [8:0]
		 lro_eligible				:  1, // [9:9]
		 reserved_9a				:  6, // [15:10]
		 window_size				: 16; // [31:16]
	uint32_t tcp_udp_chksum				: 16, // [15:0]
		 sa_idx_timeout				:  1, // [16:16]
		 da_idx_timeout				:  1, // [17:17]
		 msdu_limit_error			:  1, // [18:18]
		 flow_idx_timeout			:  1, // [19:19]
		 flow_idx_invalid			:  1, // [20:20]
		 wifi_parser_error			:  1, // [21:21]
		 amsdu_parser_error			:  1, // [22:22]
		 sa_is_valid				:  1, // [23:23]
		 da_is_valid				:  1, // [24:24]
		 da_is_mcbc				:  1, // [25:25]
		 l3_header_padding			:  2, // [27:26]
		 first_msdu				:  1, // [28:28]
		 last_msdu				:  1, // [29:29]
		 tcp_udp_chksum_fail_copy		:  1, // [30:30]
		 ip_chksum_fail_copy			:  1; // [31:31]
	uint32_t sa_idx					: 16, // [15:0]
		 da_idx_or_sw_peer_id			: 16; // [31:16]
	uint32_t msdu_drop				:  1, // [0:0]
		 reo_destination_indication		:  5, // [5:1]
		 flow_idx				: 20, // [25:6]
		 use_ppe				:  1, // [26:26]
		 reserved_12a				:  5; // [31:27]
	uint32_t fse_metadata				: 32; // [31:0]
	uint32_t cce_metadata				: 16, // [15:0]
		 sa_sw_peer_id				: 16; // [31:16]
	uint32_t aggregation_count			:  8, // [7:0]
		 flow_aggregation_continuation		:  1, // [8:8]
		 fisa_timeout				:  1, // [9:9]
		 reserved_15a				: 22; // [31:10]
	uint32_t cumulative_l4_checksum			: 16, // [15:0]
		 cumulative_ip_length			: 16; // [31:16]
	uint32_t reserved_17a				:  6, // [5:0]
		 service_code				:  9, // [14:6]
		 priority_valid				:  1, // [15:15]
		 intra_bss				:  1, // [16:16]
		 dest_chip_id				:  2, // [18:17]
		 multicast_echo				:  1, // [19:19]
		 wds_learning_event			:  1, // [20:20]
		 wds_roaming_event			:  1, // [21:21]
		 wds_keep_alive_event			:  1, // [22:22]
		 reserved_17b				:  9; // [31:23]
	uint32_t msdu_length				: 14, // [13:0]
		 stbc					:  1, // [14:14]
		 ipsec_esp				:  1, // [15:15]
		 l3_offset				:  7, // [22:16]
		 ipsec_ah				:  1, // [23:23]
		 l4_offset				:  8; // [31:24]
	uint32_t msdu_number				:  8, // [7:0]
		 decap_format				:  2, // [9:8]
		 ipv4_proto				:  1, // [10:10]
		 ipv6_proto				:  1, // [11:11]
		 tcp_proto				:  1, // [12:12]
		 udp_proto				:  1, // [13:13]
		 ip_frag				:  1, // [14:14]
		 tcp_only_ack				:  1, // [15:15]
		 da_is_bcast_mcast			:  1, // [16:16]
		 toeplitz_hash_sel			:  2, // [18:17]
		 ip_fixed_header_valid			:  1, // [19:19]
		 ip_extn_header_valid			:  1, // [20:20]
		 tcp_udp_header_valid			:  1, // [21:21]
		 mesh_control_present			:  1, // [22:22]
		 ldpc					:  1, // [23:23]
		 ip4_protocol_ip6_next_header		:  8; // [31:24]
	uint32_t toeplitz_hash_2_or_4			: 32; // [31:0]
	uint32_t flow_id_toeplitz			: 32; // [31:0]
	uint32_t user_rssi				:  8, // [7:0]
		 pkt_type				:  4, // [11:8]
		 sgi					:  2, // [13:12]
		 rate_mcs				:  4, // [17:14]
		 receive_bandwidth			:  3, // [20:18]
		 reception_type				:  3, // [23:21]
		 mimo_ss_bitmap				:  8; // [31:24]
	uint32_t ppdu_start_timestamp_31_0		: 32; // [31:0]
	uint32_t ppdu_start_timestamp_63_32		: 32; // [31:0]
	uint32_t sw_phy_meta_data			: 32; // [31:0]
	uint32_t vlan_ctag_ci				: 16, // [15:0]
		 vlan_stag_ci				: 16; // [31:16]
	uint32_t reserved_27a				: 32; // [31:0]
	uint32_t reserved_28a				: 32; // [31:0]
	uint32_t reserved_29a				: 32; // [31:0]
	uint32_t first_mpdu				:  1, // [0:0]
		 reserved_30a				:  1, // [1:1]
		 mcast_bcast				:  1, // [2:2]
		 ast_index_not_found			:  1, // [3:3]
		 ast_index_timeout			:  1, // [4:4]
		 power_mgmt				:  1, // [5:5]
		 non_qos				:  1, // [6:6]
		 null_data				:  1, // [7:7]
		 mgmt_type				:  1, // [8:8]
		 ctrl_type				:  1, // [9:9]
		 more_data				:  1, // [10:10]
		 eosp					:  1, // [11:11]
		 a_msdu_error				:  1, // [12:12]
		 fragment_flag				:  1, // [13:13]
		 order					:  1, // [14:14]
		 cce_match				:  1, // [15:15]
		 overflow_err				:  1, // [16:16]
		 msdu_length_err			:  1, // [17:17]
		 tcp_udp_chksum_fail			:  1, // [18:18]
		 ip_chksum_fail				:  1, // [19:19]
		 sa_idx_invalid				:  1, // [20:20]
		 da_idx_invalid				:  1, // [21:21]
		 reserved_30b				:  1, // [22:22]
		 rx_in_tx_decrypt_byp			:  1, // [23:23]
		 encrypt_required			:  1, // [24:24]
		 directed				:  1, // [25:25]
		 buffer_fragment			:  1, // [26:26]
		 mpdu_length_err			:  1, // [27:27]
		 tkip_mic_err				:  1, // [28:28]
		 decrypt_err				:  1, // [29:29]
		 unencrypted_frame_err			:  1, // [30:30]
		 fcs_err				:  1; // [31:31]
	uint32_t reserved_31a				: 10, // [9:0]
		 decrypt_status_code			:  3, // [12:10]
		 rx_bitmap_not_updated			:  1, // [13:13]
		 reserved_31b				: 17, // [30:14]
		 msdu_done				:  1; // [31:31]

};

struct rx_mpdu_start_compact_qca9224 {
	struct rxpt_classify_info rxpt_classify_info_details;
	uint32_t rx_reo_queue_desc_addr_31_0		: 32; // [31:0]
	uint32_t rx_reo_queue_desc_addr_39_32		:  8, // [7:0]
		 receive_queue_number			: 16, // [23:8]
		 pre_delim_err_warning			:  1, // [24:24]
		 first_delim_err			:  1, // [25:25]
		 reserved_2a				:  6; // [31:26]
	uint32_t pn_31_0				: 32; // [31:0]
	uint32_t pn_63_32				: 32; // [31:0]
	uint32_t pn_95_64				: 32; // [31:0]
	uint32_t pn_127_96				: 32; // [31:0]
	uint32_t epd_en					:  1, // [0:0]
		 all_frames_shall_be_encrypted		:  1, // [1:1]
		 encrypt_type				:  4, // [5:2]
		 wep_key_width_for_variable_key		:  2, // [7:6]
		 mesh_sta				:  2, // [9:8]
		 bssid_hit				:  1, // [10:10]
		 bssid_number				:  4, // [14:11]
		 tid					:  4, // [18:15]
		 reserved_7a				: 13; // [31:19]
	uint32_t peer_meta_data				: 32; // [31:0]
	uint32_t rxpcu_mpdu_filter_in_category		:  2, // [1:0]
		 sw_frame_group_id			:  7, // [8:2]
		 ndp_frame				:  1, // [9:9]
		 phy_err				:  1, // [10:10]
		 phy_err_during_mpdu_header		:  1, // [11:11]
		 protocol_version_err			:  1, // [12:12]
		 ast_based_lookup_valid			:  1, // [13:13]
		 ranging				:  1, // [14:14]
		 reserved_9a				:  1, // [15:15]
		 phy_ppdu_id				: 16; // [31:16]
	uint32_t ast_index				: 16, // [15:0]
		 sw_peer_id				: 16; // [31:16]
	uint32_t mpdu_frame_control_valid		:  1, // [0:0]
		 mpdu_duration_valid			:  1, // [1:1]
		 mac_addr_ad1_valid			:  1, // [2:2]
		 mac_addr_ad2_valid			:  1, // [3:3]
		 mac_addr_ad3_valid			:  1, // [4:4]
		 mac_addr_ad4_valid			:  1, // [5:5]
		 mpdu_sequence_control_valid		:  1, // [6:6]
		 mpdu_qos_control_valid			:  1, // [7:7]
		 mpdu_ht_control_valid			:  1, // [8:8]
		 frame_encryption_info_valid		:  1, // [9:9]
		 mpdu_fragment_number			:  4, // [13:10]
		 more_fragment_flag			:  1, // [14:14]
		 reserved_11a				:  1, // [15:15]
		 fr_ds					:  1, // [16:16]
		 to_ds					:  1, // [17:17]
		 encrypted				:  1, // [18:18]
		 mpdu_retry				:  1, // [19:19]
		 mpdu_sequence_number			: 12; // [31:20]
	uint32_t key_id_octet				:  8, // [7:0]
		 new_peer_entry				:  1, // [8:8]
		 decrypt_needed				:  1, // [9:9]
		 decap_type				:  2, // [11:10]
		 rx_insert_vlan_c_tag_padding		:  1, // [12:12]
		 rx_insert_vlan_s_tag_padding		:  1, // [13:13]
		 strip_vlan_c_tag_decap			:  1, // [14:14]
		 strip_vlan_s_tag_decap			:  1, // [15:15]
		 pre_delim_count			: 12, // [27:16]
		 ampdu_flag				:  1, // [28:28]
		 bar_frame				:  1, // [29:29]
		 raw_mpdu				:  1, // [30:30]
		 reserved_12				:  1; // [31:31]
	uint32_t mpdu_length				: 14, // [13:0]
		 first_mpdu				:  1, // [14:14]
		 mcast_bcast				:  1, // [15:15]
		 ast_index_not_found			:  1, // [16:16]
		 ast_index_timeout			:  1, // [17:17]
		 power_mgmt				:  1, // [18:18]
		 non_qos				:  1, // [19:19]
		 null_data				:  1, // [20:20]
		 mgmt_type				:  1, // [21:21]
		 ctrl_type				:  1, // [22:22]
		 more_data				:  1, // [23:23]
		 eosp					:  1, // [24:24]
		 fragment_flag				:  1, // [25:25]
		 order					:  1, // [26:26]
		 u_apsd_trigger				:  1, // [27:27]
		 encrypt_required			:  1, // [28:28]
		 directed				:  1, // [29:29]
		 amsdu_present				:  1, // [30:30]
		 reserved_13				:  1; // [31:31]
	uint32_t mpdu_frame_control_field		: 16, // [15:0]
		 mpdu_duration_field			: 16; // [31:16]
	uint32_t mac_addr_ad1_31_0			: 32; // [31:0]
	uint32_t mac_addr_ad1_47_32			: 16, // [15:0]
		 mac_addr_ad2_15_0			: 16; // [31:16]
	uint32_t mac_addr_ad2_47_16			: 32; // [31:0]
	uint32_t mac_addr_ad3_31_0			: 32; // [31:0]
	uint32_t mac_addr_ad3_47_32			: 16, // [15:0]
		 mpdu_sequence_control_field		: 16; // [31:16]
	uint32_t mac_addr_ad4_31_0			: 32; // [31:0]
	uint32_t mac_addr_ad4_47_32			: 16, // [15:0]
		 mpdu_qos_control_field			: 16; // [31:16]
	uint32_t mpdu_ht_control_field			: 32; // [31:0]
	uint32_t vdev_id				:  8, // [7:0]
		 service_code				:  9, // [16:8]
		 priority_valid				:  1, // [17:17]
		 src_info				: 12, // [29:18]
		 reserved_23a				:  1, // [30:30]
		 multi_link_addr_ad1_ad2_valid		:  1; // [31:31]
	uint32_t multi_link_addr_ad1_31_0		: 32; // [31:0]
	uint32_t multi_link_addr_ad1_47_32		: 16, // [15:0]
		 multi_link_addr_ad2_15_0		: 16; // [31:16]
	uint32_t multi_link_addr_ad2_47_16		: 32; // [31:0]
	uint32_t reserved_27a				: 32; // [31:0]
	uint32_t reserved_28a				: 32; // [31:0]
	uint32_t reserved_29a				: 32; // [31:0]
};
#else
struct rx_msdu_end_compact_qca9224 {
	uint32_t phy_ppdu_id                            : 16, // [31:16]
		 reserved_0                             :  7, // [15:9]
		 sw_frame_group_id                      :  7, // [8:2]
		 rxpcu_mpdu_filter_in_category          :  2; // [1:0]
	uint32_t reserved_1a                            :  2, // [31:30]
		 reported_mpdu_length                   : 14, // [29:16]
		 ip_hdr_chksum                          : 16; // [15:0]
	uint32_t cumulative_l3_checksum                 : 16, // [31:16]
		 cce_classify_not_done_cce_dis          :  1, // [15:15]
		 cce_classify_not_done_truncate         :  1, // [14:14]
		 cce_super_rule                         :  6, // [13:8]
		 key_id_octet                           :  8; // [7:0]
	uint32_t rule_indication_31_0                   : 32; // [31:0]
	uint32_t rule_indication_63_32                  : 32; // [31:0]
	uint32_t l3_type                                : 16, // [31:16]
		 reserved_5a                            :  2, // [15:14]
		 sa_offset_valid                        :  1, // [13:13]
		 da_offset_valid                        :  1, // [12:12]
		 sa_offset                              :  6, // [11:6]
		 da_offset                              :  6; // [5:0]
	uint32_t ipv6_options_crc                       : 32; // [31:0]
	uint32_t tcp_seq_number                         : 32; // [31:0]
	uint32_t tcp_ack_number                         : 32; // [31:0]
	uint32_t window_size                            : 16, // [31:16]
		 reserved_9a                            :  6, // [15:10]
		 lro_eligible                           :  1, // [9:9]
		 tcp_flag                               :  9; // [8:0]
	uint32_t ip_chksum_fail_copy                    :  1, // [31:31]
		 tcp_udp_chksum_fail_copy               :  1, // [30:30]
		 last_msdu                              :  1, // [29:29]
		 first_msdu                             :  1, // [28:28]
		 l3_header_padding                      :  2, // [27:26]
		 da_is_mcbc                             :  1, // [25:25]
		 da_is_valid                            :  1, // [24:24]
		 sa_is_valid                            :  1, // [23:23]
		 amsdu_parser_error                     :  1, // [22:22]
		 wifi_parser_error                      :  1, // [21:21]
		 flow_idx_invalid                       :  1, // [20:20]
		 flow_idx_timeout                       :  1, // [19:19]
		 msdu_limit_error                       :  1, // [18:18]
		 da_idx_timeout                         :  1, // [17:17]
		 sa_idx_timeout                         :  1, // [16:16]
		 tcp_udp_chksum                         : 16; // [15:0]
	uint32_t da_idx_or_sw_peer_id                   : 16, // [31:16]
		 sa_idx                                 : 16; // [15:0]
	uint32_t reserved_12a                           :  5, // [31:27]
		 use_ppe                                :  1, // [26:26]
		 flow_idx                               : 20, // [25:6]
		 reo_destination_indication             :  5, // [5:1]
		 msdu_drop                              :  1; // [0:0]
	uint32_t fse_metadata                           : 32; // [31:0]
	uint32_t sa_sw_peer_id                          : 16, // [31:16]
		 cce_metadata                           : 16; // [15:0]
	uint32_t reserved_15a                           : 22, // [31:10]
		 fisa_timeout                           :  1, // [9:9]
		 flow_aggregation_continuation          :  1, // [8:8]
		 aggregation_count                      :  8; // [7:0]
	uint32_t cumulative_ip_length                   : 16, // [31:16]
		 cumulative_l4_checksum                 : 16; // [15:0]
	uint32_t reserved_17b                           :  9, // [31:23]
		 wds_keep_alive_event                   :  1, // [22:22]
		 wds_roaming_event                      :  1, // [21:21]
		 wds_learning_event                     :  1, // [20:20]
		 multicast_echo                         :  1, // [19:19]
		 dest_chip_id                           :  2, // [18:17]
		 intra_bss                              :  1, // [16:16]
		 priority_valid                         :  1, // [15:15]
		 service_code                           :  9, // [14:6]
		 reserved_17a                           :  6; // [5:0]
	uint32_t l4_offset                              :  8, // [31:24]
		 ipsec_ah                               :  1, // [23:23]
		 l3_offset                              :  7, // [22:16]
		 ipsec_esp                              :  1, // [15:15]
		 stbc                                   :  1, // [14:14]
		 msdu_length                            : 14; // [13:0]
	uint32_t ip4_protocol_ip6_next_header           :  8, // [31:24]
		 ldpc                                   :  1, // [23:23]
		 mesh_control_present                   :  1, // [22:22]
		 tcp_udp_header_valid                   :  1, // [21:21]
		 ip_extn_header_valid                   :  1, // [20:20]
		 ip_fixed_header_valid                  :  1, // [19:19]
		 toeplitz_hash_sel                      :  2, // [18:17]
		 da_is_bcast_mcast                      :  1, // [16:16]
		 tcp_only_ack                           :  1, // [15:15]
		 ip_frag                                :  1, // [14:14]
		 udp_proto                              :  1, // [13:13]
		 tcp_proto                              :  1, // [12:12]
		 ipv6_proto                             :  1, // [11:11]
		 ipv4_proto                             :  1, // [10:10]
		 decap_format                           :  2, // [9:8]
		 msdu_number                            :  8; // [7:0]
	uint32_t toeplitz_hash_2_or_4                   : 32; // [31:0]
	uint32_t flow_id_toeplitz                       : 32; // [31:0]
	uint32_t mimo_ss_bitmap                         :  8, // [31:24]
		 reception_type                         :  3, // [23:21]
		 receive_bandwidth                      :  3, // [20:18]
		 rate_mcs                               :  4, // [17:14]
		 sgi                                    :  2, // [13:12]
		 pkt_type                               :  4, // [11:8]
		 user_rssi                              :  8; // [7:0]
	uint32_t ppdu_start_timestamp_31_0              : 32; // [31:0]
	uint32_t ppdu_start_timestamp_63_32             : 32; // [31:0]
	uint32_t sw_phy_meta_data                       : 32; // [31:0]
	uint32_t vlan_stag_ci                           : 16, // [31:16]
		 vlan_ctag_ci                           : 16; // [15:0]
	uint32_t reserved_27a                           : 32; // [31:0]
	uint32_t reserved_28a                           : 32; // [31:0]
	uint32_t reserved_29a                           : 32; // [31:0]
	uint32_t fcs_err                                :  1, // [31:31]
		 unencrypted_frame_err                  :  1, // [30:30]
		 decrypt_err                            :  1, // [29:29]
		 tkip_mic_err                           :  1, // [28:28]
		 mpdu_length_err                        :  1, // [27:27]
		 buffer_fragment                        :  1, // [26:26]
		 directed                               :  1, // [25:25]
		 encrypt_required                       :  1, // [24:24]
		 rx_in_tx_decrypt_byp                   :  1, // [23:23]
		 reserved_30b                           :  1, // [22:22]
		 da_idx_invalid                         :  1, // [21:21]
		 sa_idx_invalid                         :  1, // [20:20]
		 ip_chksum_fail                         :  1, // [19:19]
		 tcp_udp_chksum_fail                    :  1, // [18:18]
		 msdu_length_err                        :  1, // [17:17]
		 overflow_err                           :  1, // [16:16]
		 cce_match                              :  1, // [15:15]
		 order                                  :  1, // [14:14]
		 fragment_flag                          :  1, // [13:13]
		 a_msdu_error                           :  1, // [12:12]
		 eosp                                   :  1, // [11:11]
		 more_data                              :  1, // [10:10]
		 ctrl_type                              :  1, // [9:9]
		 mgmt_type                              :  1, // [8:8]
		 null_data                              :  1, // [7:7]
		 non_qos                                :  1, // [6:6]
		 power_mgmt                             :  1, // [5:5]
		 ast_index_timeout                      :  1, // [4:4]
		 ast_index_not_found                    :  1, // [3:3]
		 mcast_bcast                            :  1, // [2:2]
		 reserved_30a                           :  1, // [1:1]
		 first_mpdu                             :  1; // [0:0]
	uint32_t msdu_done                              :  1, // [31:31]
		 reserved_31b                           : 17, // [30:14]
		 rx_bitmap_not_updated                  :  1, // [13:13]
		 decrypt_status_code                    :  3, // [12:10]
		 reserved_31a                           : 10; // [9:0]
};

struct rx_mpdu_start_compact_qca9224 {
	struct   rxpt_classify_info                 rxpt_classify_info_details;
	uint32_t rx_reo_queue_desc_addr_31_0            : 32; // [31:0]
	uint32_t reserved_2a                            :  6, // [31:26]
		 first_delim_err                        :  1, // [25:25]
		 pre_delim_err_warning                  :  1, // [24:24]
		 receive_queue_number                   : 16, // [23:8]
		 rx_reo_queue_desc_addr_39_32           :  8; // [7:0]
	uint32_t pn_31_0                                : 32; // [31:0]
	uint32_t pn_63_32                               : 32; // [31:0]
	uint32_t pn_95_64                               : 32; // [31:0]
	uint32_t pn_127_96                              : 32; // [31:0]
	uint32_t reserved_7a                            : 13, // [31:19]
		 tid                                    :  4, // [18:15]
		 bssid_number                           :  4, // [14:11]
		 bssid_hit                              :  1, // [10:10]
		 mesh_sta                               :  2, // [9:8]
		 wep_key_width_for_variable_key         :  2, // [7:6]
		 encrypt_type                           :  4, // [5:2]
		 all_frames_shall_be_encrypted          :  1, // [1:1]
		 epd_en                                 :  1; // [0:0]
	uint32_t peer_meta_data                         : 32; // [31:0]
	uint32_t phy_ppdu_id                            : 16, // [31:16]
		 reserved_9a                            :  1, // [15:15]
		 ranging                                :  1, // [14:14]
		 ast_based_lookup_valid                 :  1, // [13:13]
		 protocol_version_err                   :  1, // [12:12]
		 phy_err_during_mpdu_header             :  1, // [11:11]
		 phy_err                                :  1, // [10:10]
		 ndp_frame                              :  1, // [9:9]
		 sw_frame_group_id                      :  7, // [8:2]
		 rxpcu_mpdu_filter_in_category          :  2; // [1:0]
	uint32_t sw_peer_id                             : 16, // [31:16]
		 ast_index                              : 16; // [15:0]
	uint32_t mpdu_sequence_number                   : 12, // [31:20]
		 mpdu_retry                             :  1, // [19:19]
		 encrypted                              :  1, // [18:18]
		 to_ds                                  :  1, // [17:17]
		 fr_ds                                  :  1, // [16:16]
		 reserved_11a                           :  1, // [15:15]
		 more_fragment_flag                     :  1, // [14:14]
		 mpdu_fragment_number                   :  4, // [13:10]
		 frame_encryption_info_valid            :  1, // [9:9]
		 mpdu_ht_control_valid                  :  1, // [8:8]
		 mpdu_qos_control_valid                 :  1, // [7:7]
		 mpdu_sequence_control_valid            :  1, // [6:6]
		 mac_addr_ad4_valid                     :  1, // [5:5]
		 mac_addr_ad3_valid                     :  1, // [4:4]
		 mac_addr_ad2_valid                     :  1, // [3:3]
		 mac_addr_ad1_valid                     :  1, // [2:2]
		 mpdu_duration_valid                    :  1, // [1:1]
		 mpdu_frame_control_valid               :  1; // [0:0]
	uint32_t reserved_12                            :  1, // [31:31]
		 raw_mpdu                               :  1, // [30:30]
		 bar_frame                              :  1, // [29:29]
		 ampdu_flag                             :  1, // [28:28]
		 pre_delim_count                        : 12, // [27:16]
		 strip_vlan_s_tag_decap                 :  1, // [15:15]
		 strip_vlan_c_tag_decap                 :  1, // [14:14]
		 rx_insert_vlan_s_tag_padding           :  1, // [13:13]
		 rx_insert_vlan_c_tag_padding           :  1, // [12:12]
		 decap_type                             :  2, // [11:10]
		 decrypt_needed                         :  1, // [9:9]
		 new_peer_entry                         :  1, // [8:8]
		 key_id_octet                           :  8; // [7:0]
	uint32_t reserved_13                            :  1, // [31:31]
		 amsdu_present                          :  1, // [30:30]
		 directed                               :  1, // [29:29]
		 encrypt_required                       :  1, // [28:28]
		 u_apsd_trigger                         :  1, // [27:27]
		 order                                  :  1, // [26:26]
		 fragment_flag                          :  1, // [25:25]
		 eosp                                   :  1, // [24:24]
		 more_data                              :  1, // [23:23]
		 ctrl_type                              :  1, // [22:22]
		 mgmt_type                              :  1, // [21:21]
		 null_data                              :  1, // [20:20]
		 non_qos                                :  1, // [19:19]
		 power_mgmt                             :  1, // [18:18]
		 ast_index_timeout                      :  1, // [17:17]
		 ast_index_not_found                    :  1, // [16:16]
		 mcast_bcast                            :  1, // [15:15]
		 first_mpdu                             :  1, // [14:14]
		 mpdu_length                            : 14; // [13:0]
	uint32_t mpdu_duration_field                    : 16, // [31:16]
		 mpdu_frame_control_field               : 16; // [15:0]
	uint32_t mac_addr_ad1_31_0                      : 32; // [31:0]
	uint32_t mac_addr_ad2_15_0                      : 16, // [31:16]
		 mac_addr_ad1_47_32                     : 16; // [15:0]
	uint32_t mac_addr_ad2_47_16                     : 32; // [31:0]
	uint32_t mac_addr_ad3_31_0                      : 32; // [31:0]
	uint32_t mpdu_sequence_control_field            : 16, // [31:16]
		 mac_addr_ad3_47_32                     : 16; // [15:0]
	uint32_t mac_addr_ad4_31_0                      : 32; // [31:0]
	uint32_t mpdu_qos_control_field                 : 16, // [31:16]
		 mac_addr_ad4_47_32                     : 16; // [15:0]
	uint32_t mpdu_ht_control_field                  : 32; // [31:0]
	uint32_t multi_link_addr_ad1_ad2_valid          :  1, // [31:31]
		 reserved_23a                           :  1, // [30:30]
		 src_info                               : 12, // [29:18]
		 priority_valid                         :  1, // [17:17]
		 service_code                           :  9, // [16:8]
		 vdev_id                                :  8; // [7:0]
	uint32_t multi_link_addr_ad1_31_0               : 32; // [31:0]
	uint32_t multi_link_addr_ad2_15_0               : 16, // [31:16]
		 multi_link_addr_ad1_47_32              : 16; // [15:0]
	uint32_t multi_link_addr_ad2_47_16              : 32; // [31:0]
	uint32_t reserved_27a                           : 32; // [31:0]
	uint32_t reserved_28a                           : 32; // [31:0]
	uint32_t reserved_29a                           : 32; // [31:0]
};
#endif /* BIG_ENDIAN_HOST */

/* TLV struct for word based Tlv */
typedef struct rx_mpdu_start_compact_qca9224 hal_rx_mpdu_start_t;
typedef struct rx_msdu_end_compact_qca9224 hal_rx_msdu_end_t;
#endif /* CONFIG_WORD_BASED_TLV */

#include "hal_9224_rx.h"
#include "hal_9224_tx.h"
#include "hal_be_rx_tlv.h"
#include <hal_be_generic_api.h>

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)
/**
 * hal_get_link_desc_size_9224(): API to get the link desc size
 *
 * Return: uint32_t
 */
static uint32_t hal_get_link_desc_size_9224(void)
{
	return LINK_DESC_SIZE;
}

/**
 * hal_rx_get_tlv_9224(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static uint8_t hal_rx_get_tlv_9224(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY, RECEIVE_BANDWIDTH);
}

/**
 * hal_rx_wbm_err_msdu_continuation_get_9224 () - API to check if WBM
 * msdu continuation bit is set
 *
 *@wbm_desc: wbm release ring descriptor
 *
 * Return: true if msdu continuation bit is set.
 */
uint8_t hal_rx_wbm_err_msdu_continuation_get_9224(void *wbm_desc)
{
	uint32_t comp_desc = *(uint32_t *)(((uint8_t *)wbm_desc) +
	WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET);

	return (comp_desc &
	WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK) >>
	WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB;
}

/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_9224(): API to get tlv info
 *
 * Return: uint32_t
 */
static inline
void hal_rx_proc_phyrx_other_receive_info_tlv_9224(void *rx_tlv_hdr,
						   void *ppdu_info_hdl)
{
	uint32_t tlv_tag, tlv_len;
	uint32_t temp_len, other_tlv_len, other_tlv_tag;
	void *rx_tlv = (uint8_t *)rx_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;
	void *other_tlv_hdr = NULL;
	void *other_tlv = NULL;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv_hdr);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv_hdr);
	temp_len = 0;

	other_tlv_hdr = rx_tlv + HAL_RX_TLV32_HDR_SIZE;
	other_tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(other_tlv_hdr);
	other_tlv_len = HAL_RX_GET_USER_TLV32_LEN(other_tlv_hdr);

	temp_len += other_tlv_len;
	other_tlv = other_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;

	switch (other_tlv_tag) {
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s unhandled TLV type: %d, TLV len:%d",
			  __func__, other_tlv_tag, other_tlv_len);
	break;
	}
}

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
static inline
void hal_rx_get_bb_info_9224(void *rx_tlv, void *ppdu_info_hdl)
{
	struct hal_rx_ppdu_info *ppdu_info  = ppdu_info_hdl;

	ppdu_info->cfr_info.bb_captured_channel =
		HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO, BB_CAPTURED_CHANNEL);

	ppdu_info->cfr_info.bb_captured_timeout =
		HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO, BB_CAPTURED_TIMEOUT);

	ppdu_info->cfr_info.bb_captured_reason =
		HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO, BB_CAPTURED_REASON);
}

static inline
void hal_rx_get_rtt_info_9224(void *rx_tlv, void *ppdu_info_hdl)
{
	struct hal_rx_ppdu_info *ppdu_info  = ppdu_info_hdl;

	ppdu_info->cfr_info.rx_location_info_valid =
	HAL_RX_GET(rx_tlv, PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   RX_LOCATION_INFO_VALID);

	ppdu_info->cfr_info.rtt_che_buffer_pointer_low32 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   RTT_CHE_BUFFER_POINTER_LOW32);

	ppdu_info->cfr_info.rtt_che_buffer_pointer_high8 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   RTT_CHE_BUFFER_POINTER_HIGH8);

	ppdu_info->cfr_info.chan_capture_status =
	HAL_GET_RX_LOCATION_INFO_CHAN_CAPTURE_STATUS(rx_tlv);

	ppdu_info->cfr_info.rx_start_ts =
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   RX_START_TS);

	ppdu_info->cfr_info.rtt_cfo_measurement = (int16_t)
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   RTT_CFO_MEASUREMENT);

	ppdu_info->cfr_info.agc_gain_info0 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   GAIN_CHAIN0);

	ppdu_info->cfr_info.agc_gain_info0 |=
	(((uint32_t)HAL_RX_GET(rx_tlv,
		    PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		    GAIN_CHAIN1)) << 16);

	ppdu_info->cfr_info.agc_gain_info1 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		   GAIN_CHAIN2);

	ppdu_info->cfr_info.agc_gain_info1 |=
	(((uint32_t)HAL_RX_GET(rx_tlv,
		    PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS,
		    GAIN_CHAIN3)) << 16);

	ppdu_info->cfr_info.agc_gain_info2 = 0;

	ppdu_info->cfr_info.agc_gain_info3 = 0;
}
#endif

/**
 * hal_rx_dump_mpdu_start_tlv_9224: dump RX mpdu_start TLV in structured
 *			       human readable format.
 * @mpdu_start: pointer the rx_attention TLV in pkt.
 * @dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_mpdu_start_tlv_9224(void *mpdustart,
						   uint8_t dbg_level)
{
#ifdef CONFIG_WORD_BASED_TLV
	struct rx_mpdu_start_compact_qca9224 *mpdu_info =
		(struct rx_mpdu_start_compact_qca9224 *)mpdustart;
#else
	struct rx_mpdu_start *mpdu_start = (struct rx_mpdu_start *)mpdustart;
	struct rx_mpdu_info *mpdu_info =
		(struct rx_mpdu_info *)&mpdu_start->rx_mpdu_info_details;
#endif
	QDF_TRACE(dbg_level, QDF_MODULE_ID_HAL,
		  "rx_mpdu_start tlv (1/5) - "
		  "rx_reo_queue_desc_addr_39_32 :%x"
		  "receive_queue_number:%x "
		  "pre_delim_err_warning:%x "
		  "first_delim_err:%x "
		  "reserved_2a:%x "
		  "pn_31_0:%x "
		  "pn_63_32:%x "
		  "pn_95_64:%x "
		  "pn_127_96:%x "
		  "epd_en:%x "
		  "all_frames_shall_be_encrypted  :%x"
		  "encrypt_type:%x "
		  "wep_key_width_for_variable_key :%x"
		  "mesh_sta:%x "
		  "bssid_hit:%x "
		  "bssid_number:%x "
		  "tid:%x "
		  "reserved_7a:%x ",
		  mpdu_info->rx_reo_queue_desc_addr_39_32,
		  mpdu_info->receive_queue_number,
		  mpdu_info->pre_delim_err_warning,
		  mpdu_info->first_delim_err,
		  mpdu_info->reserved_2a,
		  mpdu_info->pn_31_0,
		  mpdu_info->pn_63_32,
		  mpdu_info->pn_95_64,
		  mpdu_info->pn_127_96,
		  mpdu_info->epd_en,
		  mpdu_info->all_frames_shall_be_encrypted,
		  mpdu_info->encrypt_type,
		  mpdu_info->wep_key_width_for_variable_key,
		  mpdu_info->mesh_sta,
		  mpdu_info->bssid_hit,
		  mpdu_info->bssid_number,
		  mpdu_info->tid,
		  mpdu_info->reserved_7a);

	QDF_TRACE(dbg_level, QDF_MODULE_ID_HAL,
		  "rx_mpdu_start tlv (2/5) - "
		  "ast_index:%x "
		  "sw_peer_id:%x "
		  "mpdu_frame_control_valid:%x "
		  "mpdu_duration_valid:%x "
		  "mac_addr_ad1_valid:%x "
		  "mac_addr_ad2_valid:%x "
		  "mac_addr_ad3_valid:%x "
		  "mac_addr_ad4_valid:%x "
		  "mpdu_sequence_control_valid :%x"
		  "mpdu_qos_control_valid:%x "
		  "mpdu_ht_control_valid:%x "
		  "frame_encryption_info_valid :%x",
		  mpdu_info->ast_index,
		  mpdu_info->sw_peer_id,
		  mpdu_info->mpdu_frame_control_valid,
		  mpdu_info->mpdu_duration_valid,
		  mpdu_info->mac_addr_ad1_valid,
		  mpdu_info->mac_addr_ad2_valid,
		  mpdu_info->mac_addr_ad3_valid,
		  mpdu_info->mac_addr_ad4_valid,
		  mpdu_info->mpdu_sequence_control_valid,
		  mpdu_info->mpdu_qos_control_valid,
		  mpdu_info->mpdu_ht_control_valid,
		  mpdu_info->frame_encryption_info_valid);

	QDF_TRACE(dbg_level, QDF_MODULE_ID_HAL,
		  "rx_mpdu_start tlv (3/5) - "
		  "mpdu_fragment_number:%x "
		  "more_fragment_flag:%x "
		  "reserved_11a:%x "
		  "fr_ds:%x "
		  "to_ds:%x "
		  "encrypted:%x "
		  "mpdu_retry:%x "
		  "mpdu_sequence_number:%x ",
		  mpdu_info->mpdu_fragment_number,
		  mpdu_info->more_fragment_flag,
		  mpdu_info->reserved_11a,
		  mpdu_info->fr_ds,
		  mpdu_info->to_ds,
		  mpdu_info->encrypted,
		  mpdu_info->mpdu_retry,
		  mpdu_info->mpdu_sequence_number);

	QDF_TRACE(dbg_level, QDF_MODULE_ID_HAL,
		  "rx_mpdu_start tlv (4/5) - "
		  "mpdu_frame_control_field:%x "
		  "mpdu_duration_field:%x ",
		  mpdu_info->mpdu_frame_control_field,
		  mpdu_info->mpdu_duration_field);

	QDF_TRACE(dbg_level, QDF_MODULE_ID_HAL,
		  "rx_mpdu_start tlv (5/5) - "
		  "mac_addr_ad1_31_0:%x "
		  "mac_addr_ad1_47_32:%x "
		  "mac_addr_ad2_15_0:%x "
		  "mac_addr_ad2_47_16:%x "
		  "mac_addr_ad3_31_0:%x "
		  "mac_addr_ad3_47_32:%x "
		  "mpdu_sequence_control_field :%x"
		  "mac_addr_ad4_31_0:%x "
		  "mac_addr_ad4_47_32:%x "
		  "mpdu_qos_control_field:%x ",
		  mpdu_info->mac_addr_ad1_31_0,
		  mpdu_info->mac_addr_ad1_47_32,
		  mpdu_info->mac_addr_ad2_15_0,
		  mpdu_info->mac_addr_ad2_47_16,
		  mpdu_info->mac_addr_ad3_31_0,
		  mpdu_info->mac_addr_ad3_47_32,
		  mpdu_info->mpdu_sequence_control_field,
		  mpdu_info->mac_addr_ad4_31_0,
		  mpdu_info->mac_addr_ad4_47_32,
		  mpdu_info->mpdu_qos_control_field);
}

/**
 * hal_rx_dump_msdu_end_tlv_9224: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_end_tlv_9224(void *msduend,
					  uint8_t dbg_level)
{
#ifdef CONFIG_WORD_BASED_TLV
	struct rx_msdu_end_compact_qca9224 *msdu_end =
		(struct rx_msdu_end_compact_qca9224 *)msduend;
#else
	struct rx_msdu_end *msdu_end =
		(struct rx_msdu_end *)msduend;
#endif
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
		  "rx_msdu_end tlv - "
		  "key_id_octet: %d "
		  "cce_super_rule: %d "
		  "cce_classify_not_done_truncat: %d "
		  "cce_classify_not_done_cce_dis: %d "
		  "rule_indication_31_0: %d "
		  "tcp_udp_chksum: %d "
		  "sa_idx_timeout: %d "
		  "da_idx_timeout: %d "
		  "msdu_limit_error: %d "
		  "flow_idx_timeout: %d "
		  "flow_idx_invalid: %d "
		  "wifi_parser_error: %d "
		  "sa_is_valid: %d "
		  "da_is_valid: %d "
		  "da_is_mcbc: %d "
		  "l3_header_padding: %d "
		  "first_msdu: %d "
		  "last_msdu: %d "
		  "sa_idx: %d "
		  "msdu_drop: %d "
		  "reo_destination_indication: %d "
		  "flow_idx: %d "
		  "fse_metadata: %d "
		  "cce_metadata: %d "
		  "sa_sw_peer_id: %d ",
		  msdu_end->key_id_octet,
		  msdu_end->cce_super_rule,
		  msdu_end->cce_classify_not_done_truncate,
		  msdu_end->cce_classify_not_done_cce_dis,
		  msdu_end->rule_indication_31_0,
		  msdu_end->tcp_udp_chksum,
		  msdu_end->sa_idx_timeout,
		  msdu_end->da_idx_timeout,
		  msdu_end->msdu_limit_error,
		  msdu_end->flow_idx_timeout,
		  msdu_end->flow_idx_invalid,
		  msdu_end->wifi_parser_error,
		  msdu_end->sa_is_valid,
		  msdu_end->da_is_valid,
		  msdu_end->da_is_mcbc,
		  msdu_end->l3_header_padding,
		  msdu_end->first_msdu,
		  msdu_end->last_msdu,
		  msdu_end->sa_idx,
		  msdu_end->msdu_drop,
		  msdu_end->reo_destination_indication,
		  msdu_end->flow_idx,
		  msdu_end->fse_metadata,
		  msdu_end->cce_metadata,
		  msdu_end->sa_sw_peer_id);
}

/**
 * hal_reo_status_get_header_9224 - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_9224(hal_ring_desc_t ring_desc,
					   int b, void *h1)
{
	uint64_t *d = (uint64_t *)ring_desc;
	uint64_t val1 = 0;
	struct hal_reo_status_header *h =
			(struct hal_reo_status_header *)h1;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	d += HAL_GET_NUM_QWORDS(sizeof(struct tlv_32_hdr));

	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_GET_QUEUE_STATS_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_QUEUE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_CACHE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UNBLOCK_CACHE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_TIMEOUT_LIST_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_QW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS,
		  STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UPDATE_RX_REO_QUEUE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->cmd_num =
		HAL_GET_FIELD(
			      UNIFORM_REO_STATUS_HEADER, REO_STATUS_NUMBER,
			      val1);
	h->exec_time =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER,
			      CMD_EXECUTION_TIME, val1);
	h->status =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER,
			      REO_CMD_EXECUTION_STATUS, val1);
	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_GET_QUEUE_STATS_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_QUEUE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_CACHE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UNBLOCK_CACHE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_TIMEOUT_LIST_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_QW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS,
		  STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UPDATE_RX_REO_QUEUE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->tstamp =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER, TIMESTAMP, val1);
}

static
void *hal_rx_msdu0_buffer_addr_lsb_9224(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_9224(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_9224(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_9224(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

/**
 * hal_reo_config_9224(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static void
hal_reo_config_9224(struct hal_soc *soc,
		    uint32_t reg_val,
		    struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_9224() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_9224(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_9224 - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_9224(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_get_window_address_9224(): Function to get hp/tp address
 * @hal_soc: Pointer to hal_soc
 * @addr: address offset of register
 *
 * Return: modified address offset of register
 */

static inline qdf_iomem_t hal_get_window_address_9224(struct hal_soc *hal_soc,
						      qdf_iomem_t addr)
{
	uint32_t offset = addr - hal_soc->dev_base_addr;
	qdf_iomem_t new_offset;

	/*
	 * If offset lies within DP register range, use 3rd window to write
	 * into DP region.
	 */
	if ((offset ^ UMAC_BASE) < WINDOW_RANGE_MASK) {
		new_offset = (hal_soc->dev_base_addr + (3 * WINDOW_START) +
			  (offset & WINDOW_RANGE_MASK));
	/*
	 * If offset lies within CE register range, use 2nd window to write
	 * into CE region.
	 */
	} else if ((offset ^ CE_WFSS_CE_REG_BASE) < WINDOW_RANGE_MASK) {
		new_offset = (hal_soc->dev_base_addr + (2 * WINDOW_START) +
			  (offset & WINDOW_RANGE_MASK));
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: ERROR: Accessing Wrong register\n", __func__);
		qdf_assert_always(0);
		return 0;
	}
	return new_offset;
}

static inline void hal_write_window_register(struct hal_soc *hal_soc)
{
	/* Write value into window configuration register */
	qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_REG_ADDRESS,
		      WINDOW_CONFIGURATION_VALUE_9224);
}

static
void hal_compute_reo_remap_ix2_ix3_9224(uint32_t *ring, uint32_t num_rings,
					uint32_t *remap1, uint32_t *remap2)
{
	switch (num_rings) {
	case 1:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[0], 17) |
				HAL_REO_REMAP_IX2(ring[0], 18) |
				HAL_REO_REMAP_IX2(ring[0], 19) |
				HAL_REO_REMAP_IX2(ring[0], 20) |
				HAL_REO_REMAP_IX2(ring[0], 21) |
				HAL_REO_REMAP_IX2(ring[0], 22) |
				HAL_REO_REMAP_IX2(ring[0], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[0], 24) |
				HAL_REO_REMAP_IX3(ring[0], 25) |
				HAL_REO_REMAP_IX3(ring[0], 26) |
				HAL_REO_REMAP_IX3(ring[0], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[0], 29) |
				HAL_REO_REMAP_IX3(ring[0], 30) |
				HAL_REO_REMAP_IX3(ring[0], 31);
		break;
	case 2:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[0], 17) |
				HAL_REO_REMAP_IX2(ring[1], 18) |
				HAL_REO_REMAP_IX2(ring[1], 19) |
				HAL_REO_REMAP_IX2(ring[0], 20) |
				HAL_REO_REMAP_IX2(ring[0], 21) |
				HAL_REO_REMAP_IX2(ring[1], 22) |
				HAL_REO_REMAP_IX2(ring[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[0], 24) |
				HAL_REO_REMAP_IX3(ring[0], 25) |
				HAL_REO_REMAP_IX3(ring[1], 26) |
				HAL_REO_REMAP_IX3(ring[1], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[0], 29) |
				HAL_REO_REMAP_IX3(ring[1], 30) |
				HAL_REO_REMAP_IX3(ring[1], 31);
		break;
	case 3:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[0], 19) |
				HAL_REO_REMAP_IX2(ring[1], 20) |
				HAL_REO_REMAP_IX2(ring[2], 21) |
				HAL_REO_REMAP_IX2(ring[0], 22) |
				HAL_REO_REMAP_IX2(ring[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[2], 24) |
				HAL_REO_REMAP_IX3(ring[0], 25) |
				HAL_REO_REMAP_IX3(ring[1], 26) |
				HAL_REO_REMAP_IX3(ring[2], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[0], 31);
		break;
	case 4:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[3], 19) |
				HAL_REO_REMAP_IX2(ring[0], 20) |
				HAL_REO_REMAP_IX2(ring[1], 21) |
				HAL_REO_REMAP_IX2(ring[2], 22) |
				HAL_REO_REMAP_IX2(ring[3], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[0], 24) |
				HAL_REO_REMAP_IX3(ring[1], 25) |
				HAL_REO_REMAP_IX3(ring[2], 26) |
				HAL_REO_REMAP_IX3(ring[3], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[3], 31);
		break;
	}
}

/**
 * hal_rx_flow_setup_fse_9224() - Setup a flow search entry in HW FST
 * @fst: Pointer to the Rx Flow Search Table
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
static void *
hal_rx_flow_setup_fse_9224(uint8_t *rx_fst, uint32_t table_offset,
			   uint8_t *rx_flow)
{
	struct hal_rx_fst *fst = (struct hal_rx_fst *)rx_fst;
	struct hal_rx_flow *flow = (struct hal_rx_flow *)rx_flow;
	uint8_t *fse;
	bool fse_valid;

	if (table_offset >= fst->max_entries) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "HAL FSE table offset %u exceeds max entries %u",
			  table_offset, fst->max_entries);
		return NULL;
	}

	fse = (uint8_t *)fst->base_vaddr +
			(table_offset * HAL_RX_FST_ENTRY_SIZE);

	fse_valid = HAL_GET_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID);

	if (fse_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "HAL FSE %pK already valid", fse);
		return NULL;
	}

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_127_96,
			       qdf_htonl(flow->tuple_info.src_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64,
			       qdf_htonl(flow->tuple_info.src_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32,
			       qdf_htonl(flow->tuple_info.src_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0,
			       qdf_htonl(flow->tuple_info.src_ip_31_0));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96,
			       qdf_htonl(flow->tuple_info.dest_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64,
			       qdf_htonl(flow->tuple_info.dest_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32,
			       qdf_htonl(flow->tuple_info.dest_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0,
			       qdf_htonl(flow->tuple_info.dest_ip_31_0));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_PORT,
			       (flow->tuple_info.dest_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_PORT,
			       (flow->tuple_info.src_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL,
			       flow->tuple_info.l4_protocol);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER,
			       flow->reo_destination_handler);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, VALID, 1);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, METADATA);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, METADATA) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, METADATA,
			       flow->fse_metadata);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_INDICATION);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_INDICATION) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY,
			       REO_DESTINATION_INDICATION,
			       flow->reo_destination_indication);

	/* Reset all the other fields in FSE */
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, RESERVED_9);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_DROP);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_BYTE_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, TIMESTAMP);

	return fse;
}

#ifndef NO_RX_PKT_HDR_TLV
/**
 * hal_rx_dump_pkt_hdr_tlv: dump RX pkt header TLV in hex format
 * @ pkt_hdr_tlv: pointer the pkt_hdr_tlv in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_hdr_tlv_9224(struct rx_pkt_tlvs *pkt_tlvs,
						uint8_t dbg_level)
{
	struct rx_pkt_hdr_tlv *pkt_hdr_tlv = &pkt_tlvs->pkt_hdr_tlv;

	hal_verbose_debug("\n---------------\n"
			  "rx_pkt_hdr_tlv\n"
			  "---------------\n"
			  "phy_ppdu_id %llu ",
			  pkt_hdr_tlv->phy_ppdu_id);

	hal_verbose_hex_dump(pkt_hdr_tlv->rx_pkt_hdr,
			     sizeof(pkt_hdr_tlv->rx_pkt_hdr));
}
#else
/**
 * hal_rx_dump_pkt_hdr_tlv: dump RX pkt header TLV in hex format
 * @ pkt_hdr_tlv: pointer the pkt_hdr_tlv in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_hdr_tlv_9224(struct rx_pkt_tlvs *pkt_tlvs,
						uint8_t dbg_level)
{
}
#endif

/**
 * hal_rx_dump_pkt_tlvs_9224(): API to print RX Pkt TLVS QCN9224
 * @hal_soc_hdl: hal_soc handle
 * @buf: pointer the pkt buffer
 * @dbg_level: log level
 *
 * Return: void
 */
#ifdef CONFIG_WORD_BASED_TLV
static void hal_rx_dump_pkt_tlvs_9224(hal_soc_handle_t hal_soc_hdl,
				      uint8_t *buf, uint8_t dbg_level)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end_compact_qca9224 *msdu_end =
					&pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct rx_mpdu_start_compact_qca9224 *mpdu_start =
				&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	hal_rx_dump_msdu_end_tlv_9224(msdu_end, dbg_level);
	hal_rx_dump_mpdu_start_tlv_9224(mpdu_start, dbg_level);
	hal_rx_dump_pkt_hdr_tlv_9224(pkt_tlvs, dbg_level);
}
#else
static void hal_rx_dump_pkt_tlvs_9224(hal_soc_handle_t hal_soc_hdl,
				      uint8_t *buf, uint8_t dbg_level)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct rx_mpdu_start *mpdu_start =
				&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	hal_rx_dump_msdu_end_tlv_9224(msdu_end, dbg_level);
	hal_rx_dump_mpdu_start_tlv_9224(mpdu_start, dbg_level);
	hal_rx_dump_pkt_hdr_tlv_9224(pkt_tlvs, dbg_level);
}
#endif

#define HAL_NUM_TCL_BANKS_9224 48

/**
 * hal_cmem_write_9224() - function for CMEM buffer writing
 * @hal_soc_hdl: HAL SOC handle
 * @offset: CMEM address
 * @value: value to write
 *
 * Return: None.
 */
static void hal_cmem_write_9224(hal_soc_handle_t hal_soc_hdl,
				uint32_t offset,
				uint32_t value)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	pld_reg_write(hal->qdf_dev->dev, offset, value);
}

/**
 * hal_tx_get_num_tcl_banks_9224() - Get number of banks in target
 *
 * Returns: number of bank
 */
static uint8_t hal_tx_get_num_tcl_banks_9224(void)
{
	return HAL_NUM_TCL_BANKS_9224;
}

static void hal_reo_setup_9224(struct hal_soc *soc, void *reoparams)
{
	uint32_t reg_val;
	struct hal_reo_params *reo_params = (struct hal_reo_params *)reoparams;

	reg_val = HAL_REG_READ(soc, HWIO_REO_R0_GENERAL_ENABLE_ADDR(
		REO_REG_REG_BASE));

	hal_reo_config_9224(soc, reg_val, reo_params);
	/* Other ring enable bits and REO_ENABLE will be set by FW */

	/* TODO: Setup destination ring mapping if enabled */

	/* TODO: Error destination ring setting is left to default.
	 * Default setting is to send all errors to release ring.
	 */

	/* Set the reo descriptor swap bits in case of BIG endian platform */
	hal_setup_reo_swap(soc);

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(REO_REG_REG_BASE),
		      HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000);

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_VO_REO_TIMEOUT_MS * 1000));

	/*
	 * When hash based routing is enabled, routing of the rx packet
	 * is done based on the following value: 1 _ _ _ _ The last 4
	 * bits are based on hash[3:0]. This means the possible values
	 * are 0x10 to 0x1f. This value is used to look-up the
	 * ring ID configured in Destination_Ring_Ctrl_IX_* register.
	 * The Destination_Ring_Ctrl_IX_2 and Destination_Ring_Ctrl_IX_3
	 * registers need to be configured to set-up the 16 entries to
	 * map the hash values to a ring number. There are 3 bits per
	 * hash entry  which are mapped as follows:
	 * 0: TCL, 1:SW1, 2:SW2, * 3:SW3, 4:SW4, 5:Release, 6:FW(WIFI),
	 * 7: NOT_USED.
	 */
	if (reo_params->rx_hash_enabled) {
		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR
			      (REO_REG_REG_BASE), reo_params->remap0);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR(
				       REO_REG_REG_BASE)));

		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR
			      (REO_REG_REG_BASE), reo_params->remap1);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
				       REO_REG_REG_BASE)));

		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR
			      (REO_REG_REG_BASE), reo_params->remap2);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
				       REO_REG_REG_BASE)));
	}

	/* TODO: Check if the following registers shoould be setup by host:
	 * AGING_CONTROL
	 * HIGH_MEMORY_THRESHOLD
	 * GLOBAL_LINK_DESC_COUNT_THRESH_IX_0[1,2]
	 * GLOBAL_LINK_DESC_COUNT_CTRL
	 */
}

static void hal_hw_txrx_ops_attach_qcn9224(struct hal_soc *hal_soc)
{
	/* init and setup */
	hal_soc->ops->hal_srng_dst_hw_init = hal_srng_dst_hw_init_generic;
	hal_soc->ops->hal_srng_src_hw_init = hal_srng_src_hw_init_generic;
	hal_soc->ops->hal_get_hw_hptp = hal_get_hw_hptp_generic;
	hal_soc->ops->hal_get_window_address = hal_get_window_address_9224;
	hal_soc->ops->hal_cmem_write = hal_cmem_write_9224;

	/* tx */
	hal_soc->ops->hal_tx_set_dscp_tid_map = hal_tx_set_dscp_tid_map_9224;
	hal_soc->ops->hal_tx_update_dscp_tid = hal_tx_update_dscp_tid_9224;
	hal_soc->ops->hal_tx_comp_get_status =
					hal_tx_comp_get_status_generic_be;
	hal_soc->ops->hal_tx_init_cmd_credit_ring =
					hal_tx_init_cmd_credit_ring_9224;

	/* rx */
	hal_soc->ops->hal_rx_msdu_start_nss_get = hal_rx_tlv_nss_get_be;
	hal_soc->ops->hal_rx_mon_hw_desc_get_mpdu_status =
		hal_rx_mon_hw_desc_get_mpdu_status_be;
	hal_soc->ops->hal_rx_get_tlv = hal_rx_get_tlv_9224;
	hal_soc->ops->hal_rx_proc_phyrx_other_receive_info_tlv =
				hal_rx_proc_phyrx_other_receive_info_tlv_9224;

	hal_soc->ops->hal_rx_dump_msdu_end_tlv = hal_rx_dump_msdu_end_tlv_9224;
	hal_soc->ops->hal_rx_dump_mpdu_start_tlv =
					hal_rx_dump_mpdu_start_tlv_9224;
	hal_soc->ops->hal_rx_dump_pkt_tlvs = hal_rx_dump_pkt_tlvs_9224;

	hal_soc->ops->hal_get_link_desc_size = hal_get_link_desc_size_9224;
	hal_soc->ops->hal_rx_mpdu_start_tid_get = hal_rx_tlv_tid_get_be;
	hal_soc->ops->hal_rx_msdu_start_reception_type_get =
					hal_rx_tlv_reception_type_get_be;
	hal_soc->ops->hal_rx_msdu_end_da_idx_get =
					hal_rx_msdu_end_da_idx_get_be;
	hal_soc->ops->hal_rx_msdu_desc_info_get_ptr =
					hal_rx_msdu_desc_info_get_ptr_9224;
	hal_soc->ops->hal_rx_link_desc_msdu0_ptr =
					hal_rx_link_desc_msdu0_ptr_9224;
	hal_soc->ops->hal_reo_status_get_header =
					hal_reo_status_get_header_9224;
	hal_soc->ops->hal_rx_status_get_tlv_info =
					hal_rx_status_get_tlv_info_generic_be;
	hal_soc->ops->hal_rx_wbm_err_info_get =
					hal_rx_wbm_err_info_get_generic_be;
	hal_soc->ops->hal_tx_set_pcp_tid_map =
					hal_tx_set_pcp_tid_map_generic_be;
	hal_soc->ops->hal_tx_update_pcp_tid_map =
					hal_tx_update_pcp_tid_generic_be;
	hal_soc->ops->hal_tx_set_tidmap_prty =
					hal_tx_update_tidmap_prty_generic_be;
	hal_soc->ops->hal_rx_get_rx_fragment_number =
					hal_rx_get_rx_fragment_number_be,
	hal_soc->ops->hal_rx_msdu_end_da_is_mcbc_get =
					hal_rx_tlv_da_is_mcbc_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_is_valid_get =
					hal_rx_tlv_sa_is_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_idx_get = hal_rx_tlv_sa_idx_get_be;
	hal_soc->ops->hal_rx_desc_is_first_msdu = hal_rx_desc_is_first_msdu_be;
	hal_soc->ops->hal_rx_msdu_end_l3_hdr_padding_get =
		hal_rx_tlv_l3_hdr_padding_get_be;
	hal_soc->ops->hal_rx_encryption_info_valid =
					hal_rx_encryption_info_valid_be;
	hal_soc->ops->hal_rx_print_pn = hal_rx_print_pn_be;
	hal_soc->ops->hal_rx_msdu_end_first_msdu_get =
					hal_rx_tlv_first_msdu_get_be;
	hal_soc->ops->hal_rx_msdu_end_da_is_valid_get =
					hal_rx_tlv_da_is_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_last_msdu_get =
					hal_rx_tlv_last_msdu_get_be;
	hal_soc->ops->hal_rx_get_mpdu_mac_ad4_valid =
					hal_rx_get_mpdu_mac_ad4_valid_be;
	hal_soc->ops->hal_rx_mpdu_start_sw_peer_id_get =
		hal_rx_mpdu_start_sw_peer_id_get_be;
	hal_soc->ops->hal_rx_mpdu_peer_meta_data_get =
		hal_rx_mpdu_peer_meta_data_get_be;
	hal_soc->ops->hal_rx_mpdu_get_to_ds = hal_rx_mpdu_get_to_ds_be;
	hal_soc->ops->hal_rx_mpdu_get_fr_ds = hal_rx_mpdu_get_fr_ds_be;
	hal_soc->ops->hal_rx_get_mpdu_frame_control_valid =
		hal_rx_get_mpdu_frame_control_valid_be;
	hal_soc->ops->hal_rx_mpdu_get_addr1 = hal_rx_mpdu_get_addr1_be;
	hal_soc->ops->hal_rx_mpdu_get_addr2 = hal_rx_mpdu_get_addr2_be;
	hal_soc->ops->hal_rx_mpdu_get_addr3 = hal_rx_mpdu_get_addr3_be;
	hal_soc->ops->hal_rx_mpdu_get_addr4 = hal_rx_mpdu_get_addr4_be;
	hal_soc->ops->hal_rx_get_mpdu_sequence_control_valid =
		hal_rx_get_mpdu_sequence_control_valid_be;
	hal_soc->ops->hal_rx_is_unicast = hal_rx_is_unicast_be;
	hal_soc->ops->hal_rx_tid_get = hal_rx_tid_get_be;
	hal_soc->ops->hal_rx_hw_desc_get_ppduid_get =
		hal_rx_hw_desc_get_ppduid_get_be;
	hal_soc->ops->hal_rx_mpdu_start_mpdu_qos_control_valid_get =
		hal_rx_mpdu_start_mpdu_qos_control_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_sw_peer_id_get =
					hal_rx_msdu_end_sa_sw_peer_id_get_be;
	hal_soc->ops->hal_rx_msdu0_buffer_addr_lsb =
					hal_rx_msdu0_buffer_addr_lsb_9224;
	hal_soc->ops->hal_rx_msdu_desc_info_ptr_get =
					hal_rx_msdu_desc_info_ptr_get_9224;
	hal_soc->ops->hal_ent_mpdu_desc_info = hal_ent_mpdu_desc_info_9224;
	hal_soc->ops->hal_dst_mpdu_desc_info = hal_dst_mpdu_desc_info_9224;
	hal_soc->ops->hal_rx_get_fc_valid = hal_rx_get_fc_valid_be;
	hal_soc->ops->hal_rx_get_to_ds_flag = hal_rx_get_to_ds_flag_be;
	hal_soc->ops->hal_rx_get_mac_addr2_valid =
						hal_rx_get_mac_addr2_valid_be;
	hal_soc->ops->hal_rx_get_filter_category =
						hal_rx_get_filter_category_be;
	hal_soc->ops->hal_rx_get_ppdu_id = hal_rx_get_ppdu_id_be;
	hal_soc->ops->hal_reo_config = hal_reo_config_9224;
	hal_soc->ops->hal_rx_msdu_flow_idx_get = hal_rx_msdu_flow_idx_get_be;
	hal_soc->ops->hal_rx_msdu_flow_idx_invalid =
					hal_rx_msdu_flow_idx_invalid_be;
	hal_soc->ops->hal_rx_msdu_flow_idx_timeout =
					hal_rx_msdu_flow_idx_timeout_be;
	hal_soc->ops->hal_rx_msdu_fse_metadata_get =
					hal_rx_msdu_fse_metadata_get_be;
	hal_soc->ops->hal_rx_msdu_cce_match_get =
					hal_rx_msdu_cce_match_get_be;
	hal_soc->ops->hal_rx_msdu_cce_metadata_get =
					hal_rx_msdu_cce_metadata_get_be;
	hal_soc->ops->hal_rx_msdu_get_flow_params =
					hal_rx_msdu_get_flow_params_be;
	hal_soc->ops->hal_rx_tlv_get_tcp_chksum = hal_rx_tlv_get_tcp_chksum_be;
	hal_soc->ops->hal_rx_get_rx_sequence = hal_rx_get_rx_sequence_be;
#if defined(QCA_WIFI_QCA9224) && defined(WLAN_CFR_ENABLE) && \
	defined(WLAN_ENH_CFR_ENABLE)
	hal_soc->ops->hal_rx_get_bb_info = hal_rx_get_bb_info_9224;
	hal_soc->ops->hal_rx_get_rtt_info = hal_rx_get_rtt_info_9224;
#else
	hal_soc->ops->hal_rx_get_bb_info = NULL;
	hal_soc->ops->hal_rx_get_rtt_info = NULL;
#endif
	/* rx - msdu fast path info fields */
	hal_soc->ops->hal_rx_msdu_packet_metadata_get =
				hal_rx_msdu_packet_metadata_get_generic_be;
	hal_soc->ops->hal_rx_mpdu_start_tlv_tag_valid =
				hal_rx_mpdu_start_tlv_tag_valid_be;
	hal_soc->ops->hal_rx_wbm_err_msdu_continuation_get =
				hal_rx_wbm_err_msdu_continuation_get_9224;

	/* rx - TLV struct offsets */
	hal_soc->ops->hal_rx_msdu_end_offset_get =
		hal_rx_msdu_end_offset_get_generic;
	hal_soc->ops->hal_rx_mpdu_start_offset_get =
					hal_rx_mpdu_start_offset_get_generic;
#ifndef NO_RX_PKT_HDR_TLV
	hal_soc->ops->hal_rx_pkt_tlv_offset_get =
					hal_rx_pkt_tlv_offset_get_generic;
#endif
	hal_soc->ops->hal_rx_flow_setup_fse = hal_rx_flow_setup_fse_9224;
	hal_soc->ops->hal_compute_reo_remap_ix2_ix3 =
					hal_compute_reo_remap_ix2_ix3_9224;

	hal_soc->ops->hal_rx_msdu_get_reo_destination_indication =
				hal_rx_msdu_get_reo_destination_indication_be;
	hal_soc->ops->hal_rx_get_tlv_size = hal_rx_get_tlv_size_generic_be;
	hal_soc->ops->hal_rx_msdu_is_wlan_mcast =
					hal_rx_msdu_is_wlan_mcast_generic_be;
	hal_soc->ops->hal_tx_get_num_tcl_banks = hal_tx_get_num_tcl_banks_9224;
	hal_soc->ops->hal_rx_tlv_decap_format_get =
					hal_rx_tlv_decap_format_get_be;
#ifdef RECEIVE_OFFLOAD
	hal_soc->ops->hal_rx_tlv_get_offload_info =
					hal_rx_tlv_get_offload_info_be;
	hal_soc->ops->hal_rx_get_proto_params = hal_rx_get_proto_params_be;
	hal_soc->ops->hal_rx_get_l3_l4_offsets = hal_rx_get_l3_l4_offsets_be;
#endif
	hal_soc->ops->hal_rx_tlv_phy_ppdu_id_get =
					hal_rx_attn_phy_ppdu_id_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_done_get = hal_rx_tlv_msdu_done_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_get =
					hal_rx_msdu_start_msdu_len_get_be;
	hal_soc->ops->hal_rx_get_frame_ctrl_field =
					hal_rx_get_frame_ctrl_field_be;
	hal_soc->ops->hal_rx_tlv_csum_err_get = hal_rx_tlv_csum_err_get_be;
	hal_soc->ops->hal_rx_mpdu_info_ampdu_flag_get =
					hal_rx_mpdu_info_ampdu_flag_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_set =
					hal_rx_msdu_start_msdu_len_set_be;
	hal_soc->ops->hal_rx_tlv_sgi_get = hal_rx_tlv_sgi_get_be;
	hal_soc->ops->hal_rx_tlv_rate_mcs_get = hal_rx_tlv_rate_mcs_get_be;
	hal_soc->ops->hal_rx_tlv_bw_get = hal_rx_tlv_bw_get_be;
	hal_soc->ops->hal_rx_tlv_get_pkt_type = hal_rx_tlv_get_pkt_type_be;
	hal_soc->ops->hal_rx_tlv_mic_err_get = hal_rx_tlv_mic_err_get_be;
	hal_soc->ops->hal_rx_tlv_decrypt_err_get =
					hal_rx_tlv_decrypt_err_get_be;
	hal_soc->ops->hal_rx_tlv_first_mpdu_get = hal_rx_tlv_first_mpdu_get_be;
	hal_soc->ops->hal_rx_tlv_get_is_decrypted =
					hal_rx_tlv_get_is_decrypted_be;
	hal_soc->ops->hal_rx_msdu_get_keyid = hal_rx_msdu_get_keyid_be;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_be;
	hal_soc->ops->hal_rx_priv_info_set_in_tlv =
			hal_rx_priv_info_set_in_tlv_be;
	hal_soc->ops->hal_rx_priv_info_get_from_tlv =
			hal_rx_priv_info_get_from_tlv_be;
	hal_soc->ops->hal_rx_pkt_hdr_get = hal_rx_pkt_hdr_get_be;
	hal_soc->ops->hal_reo_setup = hal_reo_setup_9224;
	hal_soc->ops->hal_compute_reo_remap_ix0 = NULL;
};

struct hal_hw_srng_config hw_srng_table_9224[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 8,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO2SW1_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		.reg_size = {
			HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(0) -
				HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(0),
			HWIO_REO_R2_REO2SW2_RING_HP_ADDR(0) -
				HWIO_REO_R2_REO2SW1_RING_HP_ADDR(0),
		},
		.max_size =
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_EXCEPTION */
		/* Designating REO2SW0 ring as exception ring. This ring is
		 * similar to other REO2SW rings though it is named as REO2SW0.
		 * Any of theREO2SW rings can be used as exception ring.
		 */
		.start_ring_id = HAL_SRNG_REO2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW0_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO2SW0_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_REINJECT */
		.start_ring_id = HAL_SRNG_SW2REO,
		.max_rings = 4,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_SW2REO_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {
			HWIO_REO_R0_SW2REO1_RING_BASE_LSB_ADDR(0) -
				HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(0),
			HWIO_REO_R2_SW2REO1_RING_HP_ADDR(0) -
				HWIO_REO_R2_SW2REO_RING_HP_ADDR(0)
		},
		.max_size = HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_BMSK >>
				HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_CMD */
		.start_ring_id = HAL_SRNG_REO_CMD,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_REO_CMD_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO_CMD_RING_HP_ADDR(
				REO_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_STATUS */
		.start_ring_id = HAL_SRNG_REO_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats_status)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO_STATUS_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(
				REO_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_DATA */
		.start_ring_id = HAL_SRNG_SW2TCL1,
		.max_rings = 6,
		.entry_size = sizeof(struct tcl_data_cmd) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		.reg_size = {
			HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(0) -
				HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(0),
			HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(0) -
				HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(0),
		},
		.max_size =
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_CMD/CREDIT */
	  /* qca8074v2 and qcn9224 uses this ring for data commands */
		.start_ring_id = HAL_SRNG_SW2TCL_CMD,
		.max_rings = 1,
		.entry_size = sizeof(struct tcl_data_cmd) >> 2,
		.lmac_ring =  FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_SW2TCL_CREDIT_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_STATUS */
		.start_ring_id = HAL_SRNG_TCL_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_status_ring)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_SRC */
		.start_ring_id = HAL_SRNG_CE_0_SRC,
		.max_rings = 16,
		.entry_size = sizeof(struct ce_src_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_LSB_ADDR(
				WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE),
		HWIO_WFSS_CE_CHANNEL_SRC_R2_SRC_RING_HP_ADDR(
				WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE),
		},
		.reg_size = {
		WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST */
		.start_ring_id = HAL_SRNG_CE_0_DST,
		.max_rings = 16,
		.entry_size = 8 >> 2,
		/*TODO: entry_size above should actually be
		 * sizeof(struct ce_dst_desc) >> 2, but couldn't find definition
		 * of struct ce_dst_desc in HW header files
		 */
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE),
		},
		.reg_size = {
		WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 16,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR(
				WFSS_CE_0_CHANNEL_DST_REG_REG_BASE),
		HWIO_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR(
				WFSS_CE_0_CHANNEL_DST_REG_REG_BASE),
		},
		/* TODO: check destination status ring registers */
		.reg_size = {
		WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
	HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM_IDLE_LINK */
		.start_ring_id = HAL_SRNG_WBM_IDLE_LINK,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_link_descriptor_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* SW2WBM_RELEASE */
		.start_ring_id = HAL_SRNG_WBM_SW_RELEASE,
		.max_rings = 2,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		.reg_size = {
		HWIO_WBM_R0_SW1_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_SW1_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE)
		},
		.max_size =
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM2SW_RELEASE */
		.start_ring_id = HAL_SRNG_WBM2SW0_RELEASE,
		.max_rings = 8,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(
				WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(
				WBM_REG_REG_BASE),
		},
		.reg_size = {
		HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(
				WBM_REG_REG_BASE) -
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(
				WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(
				WBM_REG_REG_BASE) -
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(
				WBM_REG_REG_BASE),
		},
		.max_size =
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* RXDMA_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA0_BUF0,
#ifdef IPA_OFFLOAD
		.max_rings = 3,
#else
		.max_rings = 3,
#endif
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW0,
		.max_rings = 0,
		.entry_size = 0/*sizeof(struct reo_entrance_ring) >> 2*/,
		.lmac_ring =  TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA2_BUF,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
	{ /* RXDMA_MONITOR_STATUS */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF,
		.max_rings = 0,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXMON2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct sw_monitor_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
	{ /* RXDMA_MONITOR_DESC */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_DESC,
		.max_rings = 0,
		.entry_size = 0/*sizeof(struct sw_monitor_ring) >> 2*/,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},

	{ /* DIR_BUF_RX_DMA_SRC */
		.start_ring_id = HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING,
		/* one ring for spectral and one ring for cfr */
		.max_rings = 2,
		.entry_size = 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#ifdef WLAN_FEATURE_CIF_CFR
	{ /* WIFI_POS_SRC */
		.start_ring_id = HAL_SRNG_WIFI_POS_SRC_DMA_RING,
		.max_rings = 1,
		.entry_size = sizeof(wmi_oem_dma_buf_release_entry)  >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
#endif
	{ /* REO2PPE */
		.start_ring_id = HAL_SRNG_REO2PPE,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2PPE_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO2PPE_RING_HP_ADDR(
				REO_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_REO_R0_REO2PPE_RING_BASE_LSB_RING_BASE_ADDR_LSB_BMSK >>
		HWIO_REO_R0_REO2PPE_RING_BASE_LSB_RING_BASE_ADDR_LSB_SHFT,
	},
	{ /* PPE2TCL */
		.start_ring_id = HAL_SRNG_PPE2TCL1,
		.max_rings = 1,
		.entry_size = sizeof(struct tcl_entrance_from_ppe_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_PPE2TCL1_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_PPE2TCL1_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* PPE_RELEASE */
		.start_ring_id = HAL_SRNG_WBM_PPE_RELEASE,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_PPE_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_PPE_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		.reg_size = {},
		.max_size =
		HWIO_WBM_R0_PPE_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_PPE_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TX_MONITOR_BUF */
		.start_ring_id = HAL_SRNG_SW2TXMON_BUF0,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
	{ /* TX_MONITOR_DST */
		.start_ring_id = HAL_SRNG_WMAC1_TXMON2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct sw_monitor_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
	{ /* SW2RXDMA */
		.start_ring_id = HAL_SRNG_SW2RXDMA_BUF0,
		.max_rings = 3,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring =  TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE_BE,
	},
};

/**
 * hal_srng_hw_reg_offset_init_qcn9224() - Initialize the HW srng reg offset
 *				applicable only for QCN9224
 * @hal_soc: HAL Soc handle
 *
 * Return: None
 */
static inline void hal_srng_hw_reg_offset_init_qcn9224(struct hal_soc *hal_soc)
{
	int32_t *hw_reg_offset = hal_soc->hal_hw_reg_offset;

	hw_reg_offset[DST_MSI2_BASE_LSB] = REG_OFFSET(DST, MSI2_BASE_LSB),
	hw_reg_offset[DST_MSI2_BASE_MSB] = REG_OFFSET(DST, MSI2_BASE_MSB),
	hw_reg_offset[DST_MSI2_DATA] = REG_OFFSET(DST, MSI2_DATA),
	hw_reg_offset[DST_PRODUCER_INT2_SETUP] =
					REG_OFFSET(DST, PRODUCER_INT2_SETUP);
}

/**
 * hal_qcn9224_attach()- Attach 9224 target specific hal_soc ops,
 *			  offset and srng table
 * Return: void
 */
void hal_qcn9224_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_9224;

	hal_srng_hw_reg_offset_init_generic(hal_soc);
	hal_srng_hw_reg_offset_init_qcn9224(hal_soc);

	hal_hw_txrx_default_ops_attach_be(hal_soc);
	hal_hw_txrx_ops_attach_qcn9224(hal_soc);
	if (hal_soc->static_window_map)
		hal_write_window_register(hal_soc);
	hal_soc->dmac_cmn_src_rxbuf_ring = true;
}
