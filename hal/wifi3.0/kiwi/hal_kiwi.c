/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "hal_api.h"
#include "target_type.h"
#include "wcss_version.h"
#include "qdf_module.h"
#include "hal_flow.h"
#include "rx_flow_search_entry.h"
#include "hal_rx_flow_info.h"
#include "hal_be_api.h"
#include "reo_destination_ring_with_pn.h"

#include <hal_be_rx.h>

#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_OFFSET
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MASK
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_LSB
#define UNIFIED_PHYRX_HT_SIG_0_HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_B_0_L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_A_0_L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_A_PHYRX_L_SIG_A_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_VHT_SIG_A_0_VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_VHT_SIG_A_PHYRX_VHT_SIG_A_INFO_DETAILS_BANDWIDTH_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_SU_0_HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_SU_PHYRX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_MU_DL_0_HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_MU_DL_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_DL_UL_FLAG_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B1_MU_0_HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_MU_0_HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_MU_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_STA_ID_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0_HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_OFDMA_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_OFFSET

#define UNIFIED_PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_RSSI_LEGACY_19_RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET
#define UNIFIED_RX_MPDU_START_0_RX_MPDU_INFO_RX_MPDU_INFO_DETAILS_OFFSET \
	RX_MPDU_START_0_RX_MPDU_INFO_DETAILS_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC \
	UNIFORM_REO_STATUS_HEADER_STATUS_HEADER
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
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
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET \
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB \
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB

#include "hal_kiwi_tx.h"
#include "hal_kiwi_rx.h"

#include "hal_be_rx_tlv.h"

#include <hal_generic_api.h>
#include <hal_be_generic_api.h>
#include "hal_be_api_mon.h"

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)

static uint32_t hal_get_link_desc_size_kiwi(void)
{
	return LINK_DESC_SIZE;
}

/**
 * hal_rx_dump_msdu_end_tlv_kiwi: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
#ifdef QCA_WIFI_KIWI_V2
static void hal_rx_dump_msdu_end_tlv_kiwi(void *msduend,
					  uint8_t dbg_level)
{
	struct rx_msdu_end *msdu_end = (struct rx_msdu_end *)msduend;

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
			"rx_msdu_end tlv (1/5)- "
			"rxpcu_mpdu_filter_in_category :%x "
			"sw_frame_group_id :%x "
			"reserved_0 :%x "
			"phy_ppdu_id :%x "
			"ip_hdr_chksum :%x "
			"reported_mpdu_length :%x "
			"reserved_1a :%x "
			"reserved_2a :%x "
			"cce_super_rule :%x "
			"cce_classify_not_done_truncate :%x "
			"cce_classify_not_done_cce_dis :%x "
			"cumulative_l3_checksum :%x "
			"rule_indication_31_0 :%x "
			"ipv6_options_crc :%x "
			"da_offset :%x "
			"sa_offset :%x "
			"da_offset_valid :%x "
			"sa_offset_valid :%x "
			"reserved_5a :%x "
			"l3_type :%x",
			msdu_end->rxpcu_mpdu_filter_in_category,
			msdu_end->sw_frame_group_id,
			msdu_end->reserved_0,
			msdu_end->phy_ppdu_id,
			msdu_end->ip_hdr_chksum,
			msdu_end->reported_mpdu_length,
			msdu_end->reserved_1a,
			msdu_end->reserved_2a,
			msdu_end->cce_super_rule,
			msdu_end->cce_classify_not_done_truncate,
			msdu_end->cce_classify_not_done_cce_dis,
			msdu_end->cumulative_l3_checksum,
			msdu_end->rule_indication_31_0,
			msdu_end->ipv6_options_crc,
			msdu_end->da_offset,
			msdu_end->sa_offset,
			msdu_end->da_offset_valid,
			msdu_end->sa_offset_valid,
			msdu_end->reserved_5a,
			msdu_end->l3_type);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
			"rx_msdu_end tlv (2/5)- "
			"rule_indication_63_32 :%x "
			"tcp_seq_number :%x "
			"tcp_ack_number :%x "
			"tcp_flag :%x "
			"lro_eligible :%x "
			"reserved_9a :%x "
			"window_size :%x "
			"sa_sw_peer_id :%x "
			"sa_idx_timeout :%x "
			"da_idx_timeout :%x "
			"to_ds :%x "
			"tid :%x "
			"sa_is_valid :%x "
			"da_is_valid :%x "
			"da_is_mcbc :%x "
			"l3_header_padding :%x "
			"first_msdu :%x "
			"last_msdu :%x "
			"fr_ds :%x "
			"ip_chksum_fail_copy :%x "
			"sa_idx :%x "
			"da_idx_or_sw_peer_id :%x",
			msdu_end->rule_indication_63_32,
			msdu_end->tcp_seq_number,
			msdu_end->tcp_ack_number,
			msdu_end->tcp_flag,
			msdu_end->lro_eligible,
			msdu_end->reserved_9a,
			msdu_end->window_size,
			msdu_end->sa_sw_peer_id,
			msdu_end->sa_idx_timeout,
			msdu_end->da_idx_timeout,
			msdu_end->to_ds,
			msdu_end->tid,
			msdu_end->sa_is_valid,
			msdu_end->da_is_valid,
			msdu_end->da_is_mcbc,
			msdu_end->l3_header_padding,
			msdu_end->first_msdu,
			msdu_end->last_msdu,
			msdu_end->fr_ds,
			msdu_end->ip_chksum_fail_copy,
			msdu_end->sa_idx,
			msdu_end->da_idx_or_sw_peer_id);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
			"rx_msdu_end tlv (3/5)- "
			"msdu_drop :%x "
			"reo_destination_indication :%x "
			"flow_idx :%x "
			"use_ppe :%x "
			"__reserved_g_0003 :%x "
			"vlan_ctag_stripped :%x "
			"vlan_stag_stripped :%x "
			"fragment_flag :%x "
			"fse_metadata :%x "
			"cce_metadata :%x "
			"tcp_udp_chksum :%x "
			"aggregation_count :%x "
			"flow_aggregation_continuation :%x "
			"fisa_timeout :%x "
			"tcp_udp_chksum_fail_copy :%x "
			"msdu_limit_error :%x "
			"flow_idx_timeout :%x "
			"flow_idx_invalid :%x "
			"cce_match :%x "
			"amsdu_parser_error :%x "
			"cumulative_ip_length :%x "
			"key_id_octet :%x "
			"reserved_16a :%x "
			"reserved_17a :%x "
			"service_code :%x "
			"priority_valid :%x "
			"intra_bss :%x "
			"dest_chip_id :%x "
			"multicast_echo :%x "
			"wds_learning_event :%x "
			"wds_roaming_event :%x "
			"wds_keep_alive_event :%x "
			"reserved_17b :%x",
			msdu_end->msdu_drop,
			msdu_end->reo_destination_indication,
			msdu_end->flow_idx,
			msdu_end->use_ppe,
			msdu_end->__reserved_g_0003,
			msdu_end->vlan_ctag_stripped,
			msdu_end->vlan_stag_stripped,
			msdu_end->fragment_flag,
			msdu_end->fse_metadata,
			msdu_end->cce_metadata,
			msdu_end->tcp_udp_chksum,
			msdu_end->aggregation_count,
			msdu_end->flow_aggregation_continuation,
			msdu_end->fisa_timeout,
			msdu_end->tcp_udp_chksum_fail_copy,
			msdu_end->msdu_limit_error,
			msdu_end->flow_idx_timeout,
			msdu_end->flow_idx_invalid,
			msdu_end->cce_match,
			msdu_end->amsdu_parser_error,
			msdu_end->cumulative_ip_length,
			msdu_end->key_id_octet,
			msdu_end->reserved_16a,
			msdu_end->reserved_17a,
			msdu_end->service_code,
			msdu_end->priority_valid,
			msdu_end->intra_bss,
			msdu_end->dest_chip_id,
			msdu_end->multicast_echo,
			msdu_end->wds_learning_event,
			msdu_end->wds_roaming_event,
			msdu_end->wds_keep_alive_event,
			msdu_end->reserved_17b);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
			"rx_msdu_end tlv (4/5)- "
			"msdu_length :%x "
			"stbc :%x "
			"ipsec_esp :%x "
			"l3_offset :%x "
			"ipsec_ah :%x "
			"l4_offset :%x "
			"msdu_number :%x "
			"decap_format :%x "
			"ipv4_proto :%x "
			"ipv6_proto :%x "
			"tcp_proto :%x "
			"udp_proto :%x "
			"ip_frag :%x "
			"tcp_only_ack :%x "
			"da_is_bcast_mcast :%x "
			"toeplitz_hash_sel :%x "
			"ip_fixed_header_valid :%x "
			"ip_extn_header_valid :%x "
			"tcp_udp_header_valid :%x "
			"mesh_control_present :%x "
			"ldpc :%x "
			"ip4_protocol_ip6_next_header :%x "
			"vlan_ctag_ci :%x "
			"vlan_stag_ci :%x "
			"peer_meta_data :%x "
			"user_rssi :%x "
			"pkt_type :%x "
			"sgi :%x "
			"rate_mcs :%x "
			"receive_bandwidth :%x "
			"reception_type :%x "
			"mimo_ss_bitmap :%x "
			"msdu_done_copy :%x "
			"flow_id_toeplitz :%x",
			msdu_end->msdu_length,
			msdu_end->stbc,
			msdu_end->ipsec_esp,
			msdu_end->l3_offset,
			msdu_end->ipsec_ah,
			msdu_end->l4_offset,
			msdu_end->msdu_number,
			msdu_end->decap_format,
			msdu_end->ipv4_proto,
			msdu_end->ipv6_proto,
			msdu_end->tcp_proto,
			msdu_end->udp_proto,
			msdu_end->ip_frag,
			msdu_end->tcp_only_ack,
			msdu_end->da_is_bcast_mcast,
			msdu_end->toeplitz_hash_sel,
			msdu_end->ip_fixed_header_valid,
			msdu_end->ip_extn_header_valid,
			msdu_end->tcp_udp_header_valid,
			msdu_end->mesh_control_present,
			msdu_end->ldpc,
			msdu_end->ip4_protocol_ip6_next_header,
			msdu_end->vlan_ctag_ci,
			msdu_end->vlan_stag_ci,
			msdu_end->peer_meta_data,
			msdu_end->user_rssi,
			msdu_end->pkt_type,
			msdu_end->sgi,
			msdu_end->rate_mcs,
			msdu_end->receive_bandwidth,
			msdu_end->reception_type,
			msdu_end->mimo_ss_bitmap,
			msdu_end->msdu_done_copy,
			msdu_end->flow_id_toeplitz);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
			"rx_msdu_end tlv (5/5)- "
			"ppdu_start_timestamp_63_32 :%x "
			"sw_phy_meta_data :%x "
			"ppdu_start_timestamp_31_0 :%x "
			"toeplitz_hash_2_or_4 :%x "
			"reserved_28a :%x "
			"sa_15_0 :%x "
			"sa_47_16 :%x "
			"first_mpdu :%x "
			"reserved_30a :%x "
			"mcast_bcast :%x "
			"ast_index_not_found :%x "
			"ast_index_timeout :%x "
			"power_mgmt :%x "
			"non_qos :%x "
			"null_data :%x "
			"mgmt_type :%x "
			"ctrl_type :%x "
			"more_data :%x "
			"eosp :%x "
			"a_msdu_error :%x "
			"reserved_30b :%x "
			"order :%x "
			"wifi_parser_error :%x "
			"overflow_err :%x "
			"msdu_length_err :%x "
			"tcp_udp_chksum_fail :%x "
			"ip_chksum_fail :%x "
			"sa_idx_invalid :%x "
			"da_idx_invalid :%x "
			"amsdu_addr_mismatch :%x "
			"rx_in_tx_decrypt_byp :%x "
			"encrypt_required :%x "
			"directed :%x "
			"buffer_fragment :%x "
			"mpdu_length_err :%x "
			"tkip_mic_err :%x "
			"decrypt_err :%x "
			"unencrypted_frame_err :%x "
			"fcs_err :%x "
			"reserved_31a :%x "
			"decrypt_status_code :%x "
			"rx_bitmap_not_updated :%x "
			"reserved_31b :%x "
			"msdu_done :%x",
			msdu_end->ppdu_start_timestamp_63_32,
			msdu_end->sw_phy_meta_data,
			msdu_end->ppdu_start_timestamp_31_0,
			msdu_end->toeplitz_hash_2_or_4,
			msdu_end->reserved_28a,
			msdu_end->sa_15_0,
			msdu_end->sa_47_16,
			msdu_end->first_mpdu,
			msdu_end->reserved_30a,
			msdu_end->mcast_bcast,
			msdu_end->ast_index_not_found,
			msdu_end->ast_index_timeout,
			msdu_end->power_mgmt,
			msdu_end->non_qos,
			msdu_end->null_data,
			msdu_end->mgmt_type,
			msdu_end->ctrl_type,
			msdu_end->more_data,
			msdu_end->eosp,
			msdu_end->a_msdu_error,
			msdu_end->reserved_30b,
			msdu_end->order,
			msdu_end->wifi_parser_error,
			msdu_end->overflow_err,
			msdu_end->msdu_length_err,
			msdu_end->tcp_udp_chksum_fail,
			msdu_end->ip_chksum_fail,
			msdu_end->sa_idx_invalid,
			msdu_end->da_idx_invalid,
			msdu_end->amsdu_addr_mismatch,
			msdu_end->rx_in_tx_decrypt_byp,
			msdu_end->encrypt_required,
			msdu_end->directed,
			msdu_end->buffer_fragment,
			msdu_end->mpdu_length_err,
			msdu_end->tkip_mic_err,
			msdu_end->decrypt_err,
			msdu_end->unencrypted_frame_err,
			msdu_end->fcs_err,
			msdu_end->reserved_31a,
			msdu_end->decrypt_status_code,
			msdu_end->rx_bitmap_not_updated,
			msdu_end->reserved_31b,
			msdu_end->msdu_done);
}
#else
static void hal_rx_dump_msdu_end_tlv_kiwi(void *msduend,
					  uint8_t dbg_level)
{
	struct rx_msdu_end *msdu_end = (struct rx_msdu_end *)msduend;

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (1/7)- "
		       "rxpcu_mpdu_filter_in_category :%x"
		       "sw_frame_group_id :%x"
		       "reserved_0 :%x"
		       "phy_ppdu_id :%x"
		       "ip_hdr_chksum:%x"
		       "reported_mpdu_length :%x"
		       "reserved_1a :%x"
		       "key_id_octet :%x"
		       "cce_super_rule :%x"
		       "cce_classify_not_done_truncate :%x"
		       "cce_classify_not_done_cce_dis:%x"
		       "cumulative_l3_checksum :%x"
		       "rule_indication_31_0 :%x"
		       "rule_indication_63_32:%x"
		       "da_offset :%x"
		       "sa_offset :%x"
		       "da_offset_valid :%x"
		       "sa_offset_valid :%x"
		       "reserved_5a :%x"
		       "l3_type :%x",
			msdu_end->rxpcu_mpdu_filter_in_category,
			msdu_end->sw_frame_group_id,
			msdu_end->reserved_0,
			msdu_end->phy_ppdu_id,
			msdu_end->ip_hdr_chksum,
			msdu_end->reported_mpdu_length,
			msdu_end->reserved_1a,
			msdu_end->key_id_octet,
			msdu_end->cce_super_rule,
			msdu_end->cce_classify_not_done_truncate,
			msdu_end->cce_classify_not_done_cce_dis,
			msdu_end->cumulative_l3_checksum,
			msdu_end->rule_indication_31_0,
			msdu_end->rule_indication_63_32,
			msdu_end->da_offset,
			msdu_end->sa_offset,
			msdu_end->da_offset_valid,
			msdu_end->sa_offset_valid,
			msdu_end->reserved_5a,
			msdu_end->l3_type);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (2/7)- "
		       "ipv6_options_crc :%x"
		       "tcp_seq_number :%x"
		       "tcp_ack_number :%x"
		       "tcp_flag :%x"
		       "lro_eligible :%x"
		       "reserved_9a :%x"
		       "window_size :%x"
		       "tcp_udp_chksum :%x"
		       "sa_idx_timeout :%x"
		       "da_idx_timeout :%x"
		       "msdu_limit_error :%x"
		       "flow_idx_timeout :%x"
		       "flow_idx_invalid :%x"
		       "wifi_parser_error :%x"
		       "amsdu_parser_error :%x"
		       "sa_is_valid :%x"
		       "da_is_valid :%x"
		       "da_is_mcbc :%x"
		       "l3_header_padding :%x"
		       "first_msdu :%x"
		       "last_msdu :%x",
		       msdu_end->ipv6_options_crc,
		       msdu_end->tcp_seq_number,
		       msdu_end->tcp_ack_number,
		       msdu_end->tcp_flag,
		       msdu_end->lro_eligible,
		       msdu_end->reserved_9a,
		       msdu_end->window_size,
		       msdu_end->tcp_udp_chksum,
		       msdu_end->sa_idx_timeout,
		       msdu_end->da_idx_timeout,
		       msdu_end->msdu_limit_error,
		       msdu_end->flow_idx_timeout,
		       msdu_end->flow_idx_invalid,
		       msdu_end->wifi_parser_error,
		       msdu_end->amsdu_parser_error,
		       msdu_end->sa_is_valid,
		       msdu_end->da_is_valid,
		       msdu_end->da_is_mcbc,
		       msdu_end->l3_header_padding,
		       msdu_end->first_msdu,
		       msdu_end->last_msdu);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (3/7)"
		       "tcp_udp_chksum_fail_copy :%x"
		       "ip_chksum_fail_copy :%x"
		       "sa_idx :%x"
		       "da_idx_or_sw_peer_id :%x"
		       "msdu_drop :%x"
		       "reo_destination_indication :%x"
		       "flow_idx :%x"
		       "reserved_12a :%x"
		       "fse_metadata :%x"
		       "cce_metadata :%x"
		       "sa_sw_peer_id:%x"
		       "aggregation_count :%x"
		       "flow_aggregation_continuation:%x"
		       "fisa_timeout :%x"
		       "reserved_15a :%x"
		       "cumulative_l4_checksum :%x"
		       "cumulative_ip_length :%x"
		       "service_code :%x"
		       "priority_valid :%x",
		       msdu_end->tcp_udp_chksum_fail_copy,
		       msdu_end->ip_chksum_fail_copy,
		       msdu_end->sa_idx,
		       msdu_end->da_idx_or_sw_peer_id,
		       msdu_end->msdu_drop,
		       msdu_end->reo_destination_indication,
		       msdu_end->flow_idx,
		       msdu_end->reserved_12a,
		       msdu_end->fse_metadata,
		       msdu_end->cce_metadata,
		       msdu_end->sa_sw_peer_id,
		       msdu_end->aggregation_count,
		       msdu_end->flow_aggregation_continuation,
		       msdu_end->fisa_timeout,
		       msdu_end->reserved_15a,
		       msdu_end->cumulative_l4_checksum,
		       msdu_end->cumulative_ip_length,
		       msdu_end->service_code,
		       msdu_end->priority_valid);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (4/7)"
		       "reserved_17a :%x"
		       "msdu_length :%x"
		       "ipsec_esp :%x"
		       "l3_offset :%x"
		       "ipsec_ah :%x"
		       "l4_offset :%x"
		       "msdu_number :%x"
		       "decap_format :%x"
		       "ipv4_proto :%x"
		       "ipv6_proto :%x"
		       "tcp_proto :%x"
		       "udp_proto :%x"
		       "ip_frag :%x"
		       "tcp_only_ack :%x"
		       "da_is_bcast_mcast :%x"
		       "toeplitz_hash_sel :%x"
		       "ip_fixed_header_valid:%x"
		       "ip_extn_header_valid :%x"
		       "tcp_udp_header_valid :%x",
		       msdu_end->reserved_17a,
		       msdu_end->msdu_length,
		       msdu_end->ipsec_esp,
		       msdu_end->l3_offset,
		       msdu_end->ipsec_ah,
		       msdu_end->l4_offset,
		       msdu_end->msdu_number,
		       msdu_end->decap_format,
		       msdu_end->ipv4_proto,
		       msdu_end->ipv6_proto,
		       msdu_end->tcp_proto,
		       msdu_end->udp_proto,
		       msdu_end->ip_frag,
		       msdu_end->tcp_only_ack,
		       msdu_end->da_is_bcast_mcast,
		       msdu_end->toeplitz_hash_sel,
		       msdu_end->ip_fixed_header_valid,
		       msdu_end->ip_extn_header_valid,
		       msdu_end->tcp_udp_header_valid);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (5/7)"
		       "mesh_control_present :%x"
		       "ldpc :%x"
		       "ip4_protocol_ip6_next_header :%x"
		       "toeplitz_hash_2_or_4 :%x"
		       "flow_id_toeplitz :%x"
		       "user_rssi :%x"
		       "pkt_type :%x"
		       "stbc :%x"
		       "sgi :%x"
		       "rate_mcs :%x"
		       "receive_bandwidth :%x"
		       "reception_type :%x"
		       "mimo_ss_bitmap :%x"
		       "ppdu_start_timestamp_31_0 :%x"
		       "ppdu_start_timestamp_63_32 :%x"
		       "sw_phy_meta_data :%x"
		       "vlan_ctag_ci :%x"
		       "vlan_stag_ci :%x"
		       "first_mpdu :%x"
		       "reserved_30a :%x"
		       "mcast_bcast :%x",
		       msdu_end->mesh_control_present,
		       msdu_end->ldpc,
		       msdu_end->ip4_protocol_ip6_next_header,
		       msdu_end->toeplitz_hash_2_or_4,
		       msdu_end->flow_id_toeplitz,
		       msdu_end->user_rssi,
		       msdu_end->pkt_type,
		       msdu_end->stbc,
		       msdu_end->sgi,
		       msdu_end->rate_mcs,
		       msdu_end->receive_bandwidth,
		       msdu_end->reception_type,
		       msdu_end->mimo_ss_bitmap,
		       msdu_end->ppdu_start_timestamp_31_0,
		       msdu_end->ppdu_start_timestamp_63_32,
		       msdu_end->sw_phy_meta_data,
		       msdu_end->vlan_ctag_ci,
		       msdu_end->vlan_stag_ci,
		       msdu_end->first_mpdu,
		       msdu_end->reserved_30a,
		       msdu_end->mcast_bcast);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (6/7)"
		       "ast_index_not_found :%x"
		       "ast_index_timeout :%x"
		       "power_mgmt :%x"
		       "non_qos :%x"
		       "null_data :%x"
		       "mgmt_type :%x"
		       "ctrl_type :%x"
		       "more_data :%x"
		       "eosp :%x"
		       "a_msdu_error :%x"
		       "fragment_flag:%x"
		       "order:%x"
		       "cce_match :%x"
		       "overflow_err :%x"
		       "msdu_length_err :%x"
		       "tcp_udp_chksum_fail :%x"
		       "ip_chksum_fail :%x"
		       "sa_idx_invalid :%x"
		       "da_idx_invalid :%x"
		       "reserved_30b :%x",
		       msdu_end->ast_index_not_found,
		       msdu_end->ast_index_timeout,
		       msdu_end->power_mgmt,
		       msdu_end->non_qos,
		       msdu_end->null_data,
		       msdu_end->mgmt_type,
		       msdu_end->ctrl_type,
		       msdu_end->more_data,
		       msdu_end->eosp,
		       msdu_end->a_msdu_error,
		       msdu_end->fragment_flag,
		       msdu_end->order,
		       msdu_end->cce_match,
		       msdu_end->overflow_err,
		       msdu_end->msdu_length_err,
		       msdu_end->tcp_udp_chksum_fail,
		       msdu_end->ip_chksum_fail,
		       msdu_end->sa_idx_invalid,
		       msdu_end->da_idx_invalid,
		       msdu_end->reserved_30b);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_msdu_end tlv (7/7)"
		       "rx_in_tx_decrypt_byp :%x"
		       "encrypt_required :%x"
		       "directed :%x"
		       "buffer_fragment :%x"
		       "mpdu_length_err :%x"
		       "tkip_mic_err :%x"
		       "decrypt_err :%x"
		       "unencrypted_frame_err:%x"
		       "fcs_err :%x"
		       "reserved_31a :%x"
		       "decrypt_status_code :%x"
		       "rx_bitmap_not_updated:%x"
		       "reserved_31b :%x"
		       "msdu_done :%x",
		       msdu_end->rx_in_tx_decrypt_byp,
		       msdu_end->encrypt_required,
		       msdu_end->directed,
		       msdu_end->buffer_fragment,
		       msdu_end->mpdu_length_err,
		       msdu_end->tkip_mic_err,
		       msdu_end->decrypt_err,
		       msdu_end->unencrypted_frame_err,
		       msdu_end->fcs_err,
		       msdu_end->reserved_31a,
		       msdu_end->decrypt_status_code,
		       msdu_end->rx_bitmap_not_updated,
		       msdu_end->reserved_31b,
		       msdu_end->msdu_done);
}
#endif

/**
 * hal_rx_dump_pkt_hdr_tlv: dump RX pkt header TLV in hex format
 * @ pkt_hdr_tlv: pointer the pkt_hdr_tlv in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_hdr_tlv_kiwi(struct rx_pkt_tlvs *pkt_tlvs,
						uint8_t dbg_level)
{
	struct rx_pkt_hdr_tlv *pkt_hdr_tlv = &pkt_tlvs->pkt_hdr_tlv;

	hal_verbose_debug("\n---------------\n"
			  "rx_pkt_hdr_tlv\n"
			  "---------------\n"
			  "phy_ppdu_id %lld ",
			  pkt_hdr_tlv->phy_ppdu_id);

	hal_verbose_hex_dump(pkt_hdr_tlv->rx_pkt_hdr,
			     sizeof(pkt_hdr_tlv->rx_pkt_hdr));
}

/**
 * hal_rx_dump_mpdu_start_tlv_generic_be: dump RX mpdu_start TLV in structured
 *			       human readable format.
 * @mpdu_start: pointer the rx_attention TLV in pkt.
 * @dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_mpdu_start_tlv_kiwi(void *mpdustart,
						   uint8_t dbg_level)
{
	struct rx_mpdu_start *mpdu_start = (struct rx_mpdu_start *)mpdustart;
	struct rx_mpdu_info *mpdu_info =
		(struct rx_mpdu_info *)&mpdu_start->rx_mpdu_info_details;

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_mpdu_start tlv (1/5) - "
		       "rx_reo_queue_desc_addr_31_0 :%x"
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
		       "bssid_hit:%x "
		       "bssid_number:%x "
		       "tid:%x "
		       "reserved_7a:%x "
		       "peer_meta_data:%x ",
		       mpdu_info->rx_reo_queue_desc_addr_31_0,
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
		       mpdu_info->bssid_hit,
		       mpdu_info->bssid_number,
		       mpdu_info->tid,
		       mpdu_info->reserved_7a,
		       mpdu_info->peer_meta_data);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_mpdu_start tlv (2/5) - "
		       "rxpcu_mpdu_filter_in_category  :%x"
		       "sw_frame_group_id:%x "
		       "ndp_frame:%x "
		       "phy_err:%x "
		       "phy_err_during_mpdu_header  :%x"
		       "protocol_version_err:%x "
		       "ast_based_lookup_valid:%x "
		       "reserved_9a:%x "
		       "phy_ppdu_id:%x "
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
		       mpdu_info->rxpcu_mpdu_filter_in_category,
		       mpdu_info->sw_frame_group_id,
		       mpdu_info->ndp_frame,
		       mpdu_info->phy_err,
		       mpdu_info->phy_err_during_mpdu_header,
		       mpdu_info->protocol_version_err,
		       mpdu_info->ast_based_lookup_valid,
		       mpdu_info->reserved_9a,
		       mpdu_info->phy_ppdu_id,
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

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_mpdu_start tlv (3/5) - "
		       "mpdu_fragment_number:%x "
		       "more_fragment_flag:%x "
		       "reserved_11a:%x "
		       "fr_ds:%x "
		       "to_ds:%x "
		       "encrypted:%x "
		       "mpdu_retry:%x "
		       "mpdu_sequence_number:%x "
		       "key_id_octet:%x "
		       "new_peer_entry:%x "
		       "decrypt_needed:%x "
		       "decap_type:%x "
		       "rx_insert_vlan_c_tag_padding :%x"
		       "rx_insert_vlan_s_tag_padding :%x"
		       "strip_vlan_c_tag_decap:%x "
		       "strip_vlan_s_tag_decap:%x "
		       "pre_delim_count:%x "
		       "ampdu_flag:%x "
		       "bar_frame:%x "
		       "raw_mpdu:%x "
		       "reserved_12:%x "
		       "mpdu_length:%x ",
		       mpdu_info->mpdu_fragment_number,
		       mpdu_info->more_fragment_flag,
		       mpdu_info->reserved_11a,
		       mpdu_info->fr_ds,
		       mpdu_info->to_ds,
		       mpdu_info->encrypted,
		       mpdu_info->mpdu_retry,
		       mpdu_info->mpdu_sequence_number,
		       mpdu_info->key_id_octet,
		       mpdu_info->new_peer_entry,
		       mpdu_info->decrypt_needed,
		       mpdu_info->decap_type,
		       mpdu_info->rx_insert_vlan_c_tag_padding,
		       mpdu_info->rx_insert_vlan_s_tag_padding,
		       mpdu_info->strip_vlan_c_tag_decap,
		       mpdu_info->strip_vlan_s_tag_decap,
		       mpdu_info->pre_delim_count,
		       mpdu_info->ampdu_flag,
		       mpdu_info->bar_frame,
		       mpdu_info->raw_mpdu,
		       mpdu_info->reserved_12,
		       mpdu_info->mpdu_length);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
		       "rx_mpdu_start tlv (4/5) - "
		       "mpdu_length:%x "
		       "first_mpdu:%x "
		       "mcast_bcast:%x "
		       "ast_index_not_found:%x "
		       "ast_index_timeout:%x "
		       "power_mgmt:%x "
		       "non_qos:%x "
		       "null_data:%x "
		       "mgmt_type:%x "
		       "ctrl_type:%x "
		       "more_data:%x "
		       "eosp:%x "
		       "fragment_flag:%x "
		       "order:%x "
		       "u_apsd_trigger:%x "
		       "encrypt_required:%x "
		       "directed:%x "
		       "amsdu_present:%x "
		       "reserved_13:%x "
		       "mpdu_frame_control_field:%x "
		       "mpdu_duration_field:%x ",
		       mpdu_info->mpdu_length,
		       mpdu_info->first_mpdu,
		       mpdu_info->mcast_bcast,
		       mpdu_info->ast_index_not_found,
		       mpdu_info->ast_index_timeout,
		       mpdu_info->power_mgmt,
		       mpdu_info->non_qos,
		       mpdu_info->null_data,
		       mpdu_info->mgmt_type,
		       mpdu_info->ctrl_type,
		       mpdu_info->more_data,
		       mpdu_info->eosp,
		       mpdu_info->fragment_flag,
		       mpdu_info->order,
		       mpdu_info->u_apsd_trigger,
		       mpdu_info->encrypt_required,
		       mpdu_info->directed,
		       mpdu_info->amsdu_present,
		       mpdu_info->reserved_13,
		       mpdu_info->mpdu_frame_control_field,
		       mpdu_info->mpdu_duration_field);

	__QDF_TRACE_RL(dbg_level, QDF_MODULE_ID_HAL,
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
		       "mpdu_qos_control_field:%x "
		       "mpdu_ht_control_field:%x "
		       "vdev_id:%x "
		       "service_code:%x "
		       "priority_valid:%x "
		       "reserved_23a:%x ",
		       mpdu_info->mac_addr_ad1_31_0,
		       mpdu_info->mac_addr_ad1_47_32,
		       mpdu_info->mac_addr_ad2_15_0,
		       mpdu_info->mac_addr_ad2_47_16,
		       mpdu_info->mac_addr_ad3_31_0,
		       mpdu_info->mac_addr_ad3_47_32,
		       mpdu_info->mpdu_sequence_control_field,
		       mpdu_info->mac_addr_ad4_31_0,
		       mpdu_info->mac_addr_ad4_47_32,
		       mpdu_info->mpdu_qos_control_field,
		       mpdu_info->mpdu_ht_control_field,
		       mpdu_info->vdev_id,
		       mpdu_info->service_code,
		       mpdu_info->priority_valid,
		       mpdu_info->reserved_23a);
}

/**
 * hal_rx_dump_pkt_tlvs_kiwi(): API to print RX Pkt TLVS for kiwi
 * @hal_soc_hdl: hal_soc handle
 * @buf: pointer the pkt buffer
 * @dbg_level: log level
 *
 * Return: void
 */
static void hal_rx_dump_pkt_tlvs_kiwi(hal_soc_handle_t hal_soc_hdl,
				      uint8_t *buf, uint8_t dbg_level)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct rx_mpdu_start *mpdu_start =
				&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	hal_rx_dump_msdu_end_tlv_kiwi(msdu_end, dbg_level);
	hal_rx_dump_mpdu_start_tlv_kiwi(mpdu_start, dbg_level);
	hal_rx_dump_pkt_hdr_tlv_kiwi(pkt_tlvs, dbg_level);
}

/**
 * hal_rx_tlv_populate_mpdu_desc_info_kiwi() - Populate the local mpdu_desc_info
 *			elements from the rx tlvs
 * @buf: start address of rx tlvs [Validated by caller]
 * @mpdu_desc_info_hdl: Buffer to populate the mpdu_dsc_info
 *			[To be validated by caller]
 *
 * Return: None
 */
static void
hal_rx_tlv_populate_mpdu_desc_info_kiwi(uint8_t *buf,
					void *mpdu_desc_info_hdl)
{
	struct hal_rx_mpdu_desc_info *mpdu_desc_info =
		(struct hal_rx_mpdu_desc_info *)mpdu_desc_info_hdl;
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
					&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	mpdu_desc_info->mpdu_seq = mpdu_info->mpdu_sequence_number;
	mpdu_desc_info->mpdu_flags = hal_rx_get_mpdu_flags((uint32_t *)
							    mpdu_info);
	mpdu_desc_info->peer_meta_data = mpdu_info->peer_meta_data;
	mpdu_desc_info->bar_frame = mpdu_info->bar_frame;
}

/**
 * hal_reo_status_get_header_kiwi - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_kiwi(hal_ring_desc_t ring_desc, int b,
					   void *h1)
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
void *hal_rx_msdu0_buffer_addr_lsb_kiwi(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_kiwi(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_kiwi(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_kiwi(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

/*
 * hal_rx_get_tlv_kiwi(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static uint8_t hal_rx_get_tlv_kiwi(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY, RECEIVE_BANDWIDTH);
}

/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_kiwi()
 *				    - process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
static
void hal_rx_proc_phyrx_other_receive_info_tlv_kiwi(void *rx_tlv_hdr,
						   void *ppdu_info_handle)
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

/**
 * hal_reo_config_kiwi(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static
void hal_reo_config_kiwi(struct hal_soc *soc,
			 uint32_t reg_val,
			 struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_kiwi() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_kiwi(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_kiwi - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_kiwi(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_get_window_address_kiwi(): Function to get hp/tp address
 * @hal_soc: Pointer to hal_soc
 * @addr: address offset of register
 *
 * Return: modified address offset of register
 */
static inline qdf_iomem_t hal_get_window_address_kiwi(struct hal_soc *hal_soc,
						      qdf_iomem_t addr)
{
	return addr;
}

/**
 * hal_reo_set_err_dst_remap_kiwi(): Function to set REO error destination
 *				     ring remap register
 * @hal_soc: Pointer to hal_soc
 *
 * Return: none.
 */
static void
hal_reo_set_err_dst_remap_kiwi(void *hal_soc)
{
	/*
	 * Set REO error 2k jump (error code 5) / OOR (error code 7)
	 * frame routed to REO2SW0 ring.
	 */
	uint32_t dst_remap_ix0 =
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 0) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 1) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 2) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 3) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 4) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 5) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 6) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 7);

	uint32_t dst_remap_ix1 =
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 14) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 13) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 12) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 11) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 10) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 9) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 8);

		HAL_REG_WRITE(hal_soc,
			      HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(
			      REO_REG_REG_BASE),
			      dst_remap_ix0);

		hal_info("HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0 0x%x",
			 HAL_REG_READ(
			 hal_soc,
			 HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(
			 REO_REG_REG_BASE)));

		HAL_REG_WRITE(hal_soc,
			      HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(
			      REO_REG_REG_BASE),
			      dst_remap_ix1);

		hal_info("HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1 0x%x",
			 HAL_REG_READ(
			 hal_soc,
			 HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(
			 REO_REG_REG_BASE)));
}

/**
 * hal_reo_enable_pn_in_dest_kiwi() - Set the REO register to enable previous PN
 *				for OOR and 2K-jump frames
 * @hal_soc: HAL SoC handle
 *
 * Return: 1, since the register is set.
 */
static uint8_t hal_reo_enable_pn_in_dest_kiwi(void *hal_soc)
{
	HAL_REG_WRITE(hal_soc, HWIO_REO_R0_PN_IN_DEST_ADDR(REO_REG_REG_BASE),
		      1);
	return 1;
}

/**
 * hal_rx_flow_setup_fse_kiwi() - Setup a flow search entry in HW FST
 * @fst: Pointer to the Rx Flow Search Table
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Flow table entry fields are updated in host byte order, little endian order.
 *
 * Return: Success/Failure
 */
static void *
hal_rx_flow_setup_fse_kiwi(uint8_t *rx_fst, uint32_t table_offset,
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
			       (flow->tuple_info.src_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64,
			       (flow->tuple_info.src_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32,
			       (flow->tuple_info.src_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0,
			       (flow->tuple_info.src_ip_31_0));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96,
			       (flow->tuple_info.dest_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64,
			       (flow->tuple_info.dest_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32,
			       (flow->tuple_info.dest_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0,
			       (flow->tuple_info.dest_ip_31_0));

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
			       (flow->fse_metadata));

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

static
void hal_compute_reo_remap_ix2_ix3_kiwi(uint32_t *ring_map,
					uint32_t num_rings, uint32_t *remap1,
					uint32_t *remap2)
{

	switch (num_rings) {
	/* should we have all the different possible ring configs */
	default:
	case 3:
		*remap1 = HAL_REO_REMAP_IX2(ring_map[0], 16) |
			  HAL_REO_REMAP_IX2(ring_map[1], 17) |
			  HAL_REO_REMAP_IX2(ring_map[2], 18) |
			  HAL_REO_REMAP_IX2(ring_map[0], 19) |
			  HAL_REO_REMAP_IX2(ring_map[1], 20) |
			  HAL_REO_REMAP_IX2(ring_map[2], 21) |
			  HAL_REO_REMAP_IX2(ring_map[0], 22) |
			  HAL_REO_REMAP_IX2(ring_map[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring_map[2], 24) |
			  HAL_REO_REMAP_IX3(ring_map[0], 25) |
			  HAL_REO_REMAP_IX3(ring_map[1], 26) |
			  HAL_REO_REMAP_IX3(ring_map[2], 27) |
			  HAL_REO_REMAP_IX3(ring_map[0], 28) |
			  HAL_REO_REMAP_IX3(ring_map[1], 29) |
			  HAL_REO_REMAP_IX3(ring_map[2], 30) |
			  HAL_REO_REMAP_IX3(ring_map[0], 31);
		break;
	case 4:
		*remap1 = HAL_REO_REMAP_IX2(ring_map[0], 16) |
			  HAL_REO_REMAP_IX2(ring_map[1], 17) |
			  HAL_REO_REMAP_IX2(ring_map[2], 18) |
			  HAL_REO_REMAP_IX2(ring_map[3], 19) |
			  HAL_REO_REMAP_IX2(ring_map[0], 20) |
			  HAL_REO_REMAP_IX2(ring_map[1], 21) |
			  HAL_REO_REMAP_IX2(ring_map[2], 22) |
			  HAL_REO_REMAP_IX2(ring_map[3], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring_map[0], 24) |
			  HAL_REO_REMAP_IX3(ring_map[1], 25) |
			  HAL_REO_REMAP_IX3(ring_map[2], 26) |
			  HAL_REO_REMAP_IX3(ring_map[3], 27) |
			  HAL_REO_REMAP_IX3(ring_map[0], 28) |
			  HAL_REO_REMAP_IX3(ring_map[1], 29) |
			  HAL_REO_REMAP_IX3(ring_map[2], 30) |
			  HAL_REO_REMAP_IX3(ring_map[3], 31);
		break;
	case 6:
		*remap1 = HAL_REO_REMAP_IX2(ring_map[0], 16) |
			  HAL_REO_REMAP_IX2(ring_map[1], 17) |
			  HAL_REO_REMAP_IX2(ring_map[2], 18) |
			  HAL_REO_REMAP_IX2(ring_map[3], 19) |
			  HAL_REO_REMAP_IX2(ring_map[4], 20) |
			  HAL_REO_REMAP_IX2(ring_map[5], 21) |
			  HAL_REO_REMAP_IX2(ring_map[0], 22) |
			  HAL_REO_REMAP_IX2(ring_map[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring_map[2], 24) |
			  HAL_REO_REMAP_IX3(ring_map[3], 25) |
			  HAL_REO_REMAP_IX3(ring_map[4], 26) |
			  HAL_REO_REMAP_IX3(ring_map[5], 27) |
			  HAL_REO_REMAP_IX3(ring_map[0], 28) |
			  HAL_REO_REMAP_IX3(ring_map[1], 29) |
			  HAL_REO_REMAP_IX3(ring_map[2], 30) |
			  HAL_REO_REMAP_IX3(ring_map[3], 31);
		break;
	case 8:
		*remap1 = HAL_REO_REMAP_IX2(ring_map[0], 16) |
			  HAL_REO_REMAP_IX2(ring_map[1], 17) |
			  HAL_REO_REMAP_IX2(ring_map[2], 18) |
			  HAL_REO_REMAP_IX2(ring_map[3], 19) |
			  HAL_REO_REMAP_IX2(ring_map[4], 20) |
			  HAL_REO_REMAP_IX2(ring_map[5], 21) |
			  HAL_REO_REMAP_IX2(ring_map[6], 22) |
			  HAL_REO_REMAP_IX2(ring_map[7], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring_map[0], 24) |
			  HAL_REO_REMAP_IX3(ring_map[1], 25) |
			  HAL_REO_REMAP_IX3(ring_map[2], 26) |
			  HAL_REO_REMAP_IX3(ring_map[3], 27) |
			  HAL_REO_REMAP_IX3(ring_map[4], 28) |
			  HAL_REO_REMAP_IX3(ring_map[5], 29) |
			  HAL_REO_REMAP_IX3(ring_map[6], 30) |
			  HAL_REO_REMAP_IX3(ring_map[7], 31);
		break;
	}
}

/* NUM TCL Bank registers in KIWI */
#define HAL_NUM_TCL_BANKS_KIWI 8

/**
 * hal_tx_get_num_tcl_banks_kiwi() - Get number of banks in target
 *
 * Returns: number of bank
 */
static uint8_t hal_tx_get_num_tcl_banks_kiwi(void)
{
	return HAL_NUM_TCL_BANKS_KIWI;
}

/**
 * hal_rx_reo_prev_pn_get_kiwi() - Get the previous PN from the REO ring desc.
 * @ring_desc: REO ring descriptor [To be validated by caller ]
 * @prev_pn: Buffer where the previous PN is to be populated.
 *		[To be validated by caller]
 *
 * Return: None
 */
static void hal_rx_reo_prev_pn_get_kiwi(void *ring_desc,
					uint64_t *prev_pn)
{
	struct reo_destination_ring_with_pn *reo_desc =
		(struct reo_destination_ring_with_pn *)ring_desc;

	*prev_pn = reo_desc->prev_pn_23_0;
	*prev_pn |= ((uint64_t)reo_desc->prev_pn_55_24 << 24);
}

/**
 * hal_cmem_write_kiwi() - function for CMEM buffer writing
 * @hal_soc_hdl: HAL SOC handle
 * @offset: CMEM address
 * @value: value to write
 *
 * Return: None.
 */
static inline void hal_cmem_write_kiwi(hal_soc_handle_t hal_soc_hdl,
				       uint32_t offset,
				       uint32_t value)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	hal_write32_mb(hal, offset, value);
}

/**
 * hal_get_idle_link_bm_id_kiwi() - Get idle link BM id from chid_id
 * @chip_id: mlo chip_id
 *
 * Returns: RBM ID
 */
static uint8_t hal_get_idle_link_bm_id_kiwi(uint8_t chip_id)
{
	return WBM_IDLE_DESC_LIST;
}

#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
/**
 * hal_get_first_wow_wakeup_packet_kiwi(): Function to get if the buffer
 * is the first one that wakes up host from WoW.
 *
 * @buf: network buffer
 *
 * Dummy function for KIWI
 *
 * Returns: 1 to indicate it is first packet received that wakes up host from
 *	    WoW. Otherwise 0
 */
static inline uint8_t hal_get_first_wow_wakeup_packet_kiwi(uint8_t *buf)
{
	return 0;
}
#endif

static void hal_hw_txrx_ops_attach_kiwi(struct hal_soc *hal_soc)
{
	/* init and setup */
	hal_soc->ops->hal_srng_dst_hw_init = hal_srng_dst_hw_init_generic;
	hal_soc->ops->hal_srng_src_hw_init = hal_srng_src_hw_init_generic;
	hal_soc->ops->hal_get_hw_hptp = hal_get_hw_hptp_generic;
	hal_soc->ops->hal_get_window_address = hal_get_window_address_kiwi;
	hal_soc->ops->hal_reo_set_err_dst_remap =
						hal_reo_set_err_dst_remap_kiwi;
	hal_soc->ops->hal_reo_enable_pn_in_dest =
						hal_reo_enable_pn_in_dest_kiwi;

	/* tx */
	hal_soc->ops->hal_tx_set_dscp_tid_map = hal_tx_set_dscp_tid_map_kiwi;
	hal_soc->ops->hal_tx_update_dscp_tid = hal_tx_update_dscp_tid_kiwi;
	hal_soc->ops->hal_tx_comp_get_status =
					hal_tx_comp_get_status_generic_be;
	hal_soc->ops->hal_tx_init_cmd_credit_ring =
					hal_tx_init_cmd_credit_ring_kiwi;

	/* rx */
	hal_soc->ops->hal_rx_msdu_start_nss_get = hal_rx_tlv_nss_get_be;
	hal_soc->ops->hal_rx_mon_hw_desc_get_mpdu_status =
		hal_rx_mon_hw_desc_get_mpdu_status_be;
	hal_soc->ops->hal_rx_get_tlv = hal_rx_get_tlv_kiwi;
	hal_soc->ops->hal_rx_pkt_hdr_get = hal_rx_pkt_hdr_get_be;
	hal_soc->ops->hal_rx_proc_phyrx_other_receive_info_tlv =
		hal_rx_proc_phyrx_other_receive_info_tlv_kiwi;

	hal_soc->ops->hal_rx_dump_msdu_end_tlv = hal_rx_dump_msdu_end_tlv_kiwi;
	hal_soc->ops->hal_rx_dump_mpdu_start_tlv =
					hal_rx_dump_mpdu_start_tlv_kiwi;
	hal_soc->ops->hal_rx_dump_pkt_tlvs = hal_rx_dump_pkt_tlvs_kiwi;

	hal_soc->ops->hal_get_link_desc_size = hal_get_link_desc_size_kiwi;
	hal_soc->ops->hal_rx_mpdu_start_tid_get = hal_rx_tlv_tid_get_be;
	hal_soc->ops->hal_rx_msdu_start_reception_type_get =
		hal_rx_tlv_reception_type_get_be;
	hal_soc->ops->hal_rx_msdu_end_da_idx_get =
					hal_rx_msdu_end_da_idx_get_be;
	hal_soc->ops->hal_rx_msdu_desc_info_get_ptr =
					hal_rx_msdu_desc_info_get_ptr_kiwi;
	hal_soc->ops->hal_rx_link_desc_msdu0_ptr =
					hal_rx_link_desc_msdu0_ptr_kiwi;
	hal_soc->ops->hal_reo_status_get_header =
					hal_reo_status_get_header_kiwi;
	hal_soc->ops->hal_rx_status_get_tlv_info =
					hal_rx_status_get_tlv_info_wrapper_be;
	hal_soc->ops->hal_rx_wbm_err_info_get =
					hal_rx_wbm_err_info_get_generic_be;
	hal_soc->ops->hal_rx_priv_info_set_in_tlv =
					hal_rx_priv_info_set_in_tlv_be;
	hal_soc->ops->hal_rx_priv_info_get_from_tlv =
					hal_rx_priv_info_get_from_tlv_be;

	hal_soc->ops->hal_tx_set_pcp_tid_map =
					hal_tx_set_pcp_tid_map_generic_be;
	hal_soc->ops->hal_tx_update_pcp_tid_map =
					hal_tx_update_pcp_tid_generic_be;
	hal_soc->ops->hal_tx_set_tidmap_prty =
					hal_tx_update_tidmap_prty_generic_be;
	hal_soc->ops->hal_rx_get_rx_fragment_number =
					hal_rx_get_rx_fragment_number_be;
	hal_soc->ops->hal_rx_msdu_end_da_is_mcbc_get =
					hal_rx_tlv_da_is_mcbc_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_is_valid_get =
		hal_rx_tlv_sa_is_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_idx_get = hal_rx_tlv_sa_idx_get_be,
	hal_soc->ops->hal_rx_desc_is_first_msdu =
					hal_rx_desc_is_first_msdu_be;
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
	hal_soc->ops->hal_rx_msdu0_buffer_addr_lsb =
					hal_rx_msdu0_buffer_addr_lsb_kiwi;
	hal_soc->ops->hal_rx_msdu_desc_info_ptr_get =
					hal_rx_msdu_desc_info_ptr_get_kiwi;
	hal_soc->ops->hal_ent_mpdu_desc_info = hal_ent_mpdu_desc_info_kiwi;
	hal_soc->ops->hal_dst_mpdu_desc_info = hal_dst_mpdu_desc_info_kiwi;
	hal_soc->ops->hal_rx_get_fc_valid = hal_rx_get_fc_valid_be;
	hal_soc->ops->hal_rx_get_to_ds_flag = hal_rx_get_to_ds_flag_be;
	hal_soc->ops->hal_rx_get_mac_addr2_valid =
					hal_rx_get_mac_addr2_valid_be;
	hal_soc->ops->hal_rx_get_filter_category =
					hal_rx_get_filter_category_be;
	hal_soc->ops->hal_rx_get_ppdu_id = hal_rx_get_ppdu_id_be;
	hal_soc->ops->hal_reo_config = hal_reo_config_kiwi;
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
	hal_soc->ops->hal_rx_tlv_get_tcp_chksum =
					hal_rx_tlv_get_tcp_chksum_be;
	hal_soc->ops->hal_rx_get_rx_sequence = hal_rx_get_rx_sequence_be;
#if defined(QCA_WIFI_KIWI) && defined(WLAN_CFR_ENABLE) && \
	defined(WLAN_ENH_CFR_ENABLE)
	hal_soc->ops->hal_rx_get_bb_info = hal_rx_get_bb_info_kiwi;
	hal_soc->ops->hal_rx_get_rtt_info = hal_rx_get_rtt_info_kiwi;
#else
	hal_soc->ops->hal_rx_get_bb_info = NULL;
	hal_soc->ops->hal_rx_get_rtt_info = NULL;
#endif
	/* rx - msdu end fast path info fields */
	hal_soc->ops->hal_rx_msdu_packet_metadata_get =
		hal_rx_msdu_packet_metadata_get_generic_be;
	hal_soc->ops->hal_rx_get_fisa_cumulative_l4_checksum =
		hal_rx_get_fisa_cumulative_l4_checksum_be;
	hal_soc->ops->hal_rx_get_fisa_cumulative_ip_length =
		hal_rx_get_fisa_cumulative_ip_length_be;
	hal_soc->ops->hal_rx_get_udp_proto = hal_rx_get_udp_proto_be;
	hal_soc->ops->hal_rx_get_fisa_flow_agg_continuation =
		hal_rx_get_flow_agg_continuation_be;
	hal_soc->ops->hal_rx_get_fisa_flow_agg_count =
					hal_rx_get_flow_agg_count_be;
	hal_soc->ops->hal_rx_get_fisa_timeout = hal_rx_get_fisa_timeout_be;
	hal_soc->ops->hal_rx_mpdu_start_tlv_tag_valid =
		hal_rx_mpdu_start_tlv_tag_valid_be;
	hal_soc->ops->hal_rx_reo_prev_pn_get = hal_rx_reo_prev_pn_get_kiwi;

	/* rx - TLV struct offsets */
	hal_soc->ops->hal_rx_msdu_end_offset_get =
					hal_rx_msdu_end_offset_get_generic;
	hal_soc->ops->hal_rx_mpdu_start_offset_get =
					hal_rx_mpdu_start_offset_get_generic;
	hal_soc->ops->hal_rx_pkt_tlv_offset_get =
					hal_rx_pkt_tlv_offset_get_generic;
	hal_soc->ops->hal_rx_flow_setup_fse = hal_rx_flow_setup_fse_kiwi;
	hal_soc->ops->hal_compute_reo_remap_ix2_ix3 =
					hal_compute_reo_remap_ix2_ix3_kiwi;
	hal_soc->ops->hal_rx_flow_setup_cmem_fse = NULL;
	hal_soc->ops->hal_rx_flow_get_cmem_fse_ts = NULL;
	hal_soc->ops->hal_rx_flow_get_cmem_fse = NULL;
	hal_soc->ops->hal_cmem_write = hal_cmem_write_kiwi;
	hal_soc->ops->hal_rx_msdu_get_reo_destination_indication =
		hal_rx_msdu_get_reo_destination_indication_be;
	hal_soc->ops->hal_tx_get_num_tcl_banks = hal_tx_get_num_tcl_banks_kiwi;
	hal_soc->ops->hal_rx_get_tlv_size = hal_rx_get_tlv_size_generic_be;
	hal_soc->ops->hal_rx_msdu_is_wlan_mcast =
					hal_rx_msdu_is_wlan_mcast_generic_be;
	hal_soc->ops->hal_rx_tlv_bw_get =
					hal_rx_tlv_bw_get_be;
	hal_soc->ops->hal_rx_tlv_get_is_decrypted =
						hal_rx_tlv_get_is_decrypted_be;
	hal_soc->ops->hal_rx_tlv_mic_err_get = hal_rx_tlv_mic_err_get_be;
	hal_soc->ops->hal_rx_tlv_get_pkt_type = hal_rx_tlv_get_pkt_type_be;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_be;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_be;
	hal_soc->ops->hal_rx_tlv_mpdu_len_err_get =
					hal_rx_tlv_mpdu_len_err_get_be;
	hal_soc->ops->hal_rx_tlv_mpdu_fcs_err_get =
					hal_rx_tlv_mpdu_fcs_err_get_be;

	hal_soc->ops->hal_rx_tlv_first_mpdu_get = hal_rx_tlv_first_mpdu_get_be;
	hal_soc->ops->hal_rx_tlv_decrypt_err_get =
					hal_rx_tlv_decrypt_err_get_be;
	hal_soc->ops->hal_rx_tlv_rate_mcs_get = hal_rx_tlv_rate_mcs_get_be;
	hal_soc->ops->hal_rx_tlv_sgi_get = hal_rx_tlv_sgi_get_be;
	hal_soc->ops->hal_rx_tlv_decap_format_get =
					hal_rx_tlv_decap_format_get_be;
	hal_soc->ops->hal_rx_tlv_get_offload_info =
					hal_rx_tlv_get_offload_info_be;
	hal_soc->ops->hal_rx_tlv_phy_ppdu_id_get =
					hal_rx_attn_phy_ppdu_id_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_done_get = hal_rx_tlv_msdu_done_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_get =
					hal_rx_msdu_start_msdu_len_get_be;
	hal_soc->ops->hal_rx_get_frame_ctrl_field =
					hal_rx_get_frame_ctrl_field_be;
	hal_soc->ops->hal_rx_get_proto_params = hal_rx_get_proto_params_be;
	hal_soc->ops->hal_rx_get_l3_l4_offsets = hal_rx_get_l3_l4_offsets_be;
	hal_soc->ops->hal_rx_tlv_csum_err_get = hal_rx_tlv_csum_err_get_be;
	hal_soc->ops->hal_rx_mpdu_info_ampdu_flag_get =
					hal_rx_mpdu_info_ampdu_flag_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_set =
					hal_rx_msdu_start_msdu_len_set_be;
	hal_soc->ops->hal_rx_tlv_populate_mpdu_desc_info =
				hal_rx_tlv_populate_mpdu_desc_info_kiwi;
	hal_soc->ops->hal_rx_tlv_get_pn_num =
				hal_rx_tlv_get_pn_num_be;
	hal_soc->ops->hal_get_reo_ent_desc_qdesc_addr =
				hal_get_reo_ent_desc_qdesc_addr_be;
	hal_soc->ops->hal_rx_get_qdesc_addr =
				hal_rx_get_qdesc_addr_be;
	hal_soc->ops->hal_set_reo_ent_desc_reo_dest_ind =
				hal_set_reo_ent_desc_reo_dest_ind_be;
	hal_soc->ops->hal_get_idle_link_bm_id = hal_get_idle_link_bm_id_kiwi;
	hal_soc->ops->hal_compute_reo_remap_ix0 = NULL;
#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
	hal_soc->ops->hal_get_first_wow_wakeup_packet =
		hal_get_first_wow_wakeup_packet_kiwi;
#endif
};

struct hal_hw_srng_config hw_srng_table_kiwi[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 8,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.nf_irq_support = true,
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
		/* Designating REO2SW0 ring as exception ring. */
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
		.max_rings = 1,
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
		.reg_size = {},
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
		.max_size =
			HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
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
		.max_rings = 5,
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
	{ /* TCL_CMD */
		.start_ring_id = HAL_SRNG_SW2TCL_CMD,
#ifndef WLAN_DP_DISABLE_TCL_CMD_CRED_SRNG
		.max_rings = 1,
#else
		.max_rings = 0,
#endif
		.entry_size = sizeof(struct tcl_gse_cmd) >> 2,
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
#ifndef WLAN_DP_DISABLE_TCL_CMD_CRED_SRNG
		.max_rings = 1,
#else
		.max_rings = 0,
#endif
		/* confirm that TLV header is needed */
		.entry_size = sizeof(struct tcl_status_ring) >> 2,
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
		.max_rings = 12,
		.entry_size = sizeof(struct ce_src_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R2_SRC_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_SRC_WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		SOC_CE_0_SRC_WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		SOC_CE_1_SRC_WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		SOC_CE_0_SRC_WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST */
		.start_ring_id = HAL_SRNG_CE_0_DST,
		.max_rings = 12,
		.entry_size = 8 >> 2,
		/*TODO: entry_size above should actually be
		 * sizeof(struct ce_dst_desc) >> 2, but couldn't find definition
		 * of struct ce_dst_desc in HW header files
		 */
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
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
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
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
		.nf_irq_support = true,
		.reg_start = {
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		.reg_size = {
		HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
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
		.max_rings = 2,
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
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
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
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_STATUS */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF,
		.max_rings = 1,
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
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW1,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DESC */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_DESC,
		.max_rings = 1,
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
	{ /* DIR_BUF_RX_DMA_SRC */
		.start_ring_id = HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING,
		/*
		 * one ring is for spectral scan
		 * the other is for cfr
		 */
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
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#endif
	{ /* REO2PPE */ 0},
	{ /* PPE2TCL */ 0},
	{ /* PPE_RELEASE */ 0},
	{ /* TX_MONITOR_BUF */ 0},
	{ /* TX_MONITOR_DST */ 0},
	{ /* SW2RXDMA_NEW */ 0},
};

/**
 * hal_srng_hw_reg_offset_init_kiwi() - Initialize the HW srng reg offset
 *				applicable only for KIWI
 * @hal_soc: HAL Soc handle
 *
 * Return: None
 */
static inline void hal_srng_hw_reg_offset_init_kiwi(struct hal_soc *hal_soc)
{
	int32_t *hw_reg_offset = hal_soc->hal_hw_reg_offset;

	hw_reg_offset[DST_MSI2_BASE_LSB] = REG_OFFSET(DST, MSI2_BASE_LSB),
	hw_reg_offset[DST_MSI2_BASE_MSB] = REG_OFFSET(DST, MSI2_BASE_MSB),
	hw_reg_offset[DST_MSI2_DATA] = REG_OFFSET(DST, MSI2_DATA),
	hw_reg_offset[DST_PRODUCER_INT2_SETUP] =
					REG_OFFSET(DST, PRODUCER_INT2_SETUP);
}

/**
 * hal_kiwi_attach() - Attach kiwi target specific hal_soc ops,
 *			  offset and srng table
 */
void hal_kiwi_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_kiwi;

	hal_srng_hw_reg_offset_init_generic(hal_soc);
	hal_srng_hw_reg_offset_init_kiwi(hal_soc);
	hal_hw_txrx_default_ops_attach_be(hal_soc);
	hal_hw_txrx_ops_attach_kiwi(hal_soc);
}
