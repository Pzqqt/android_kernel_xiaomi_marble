/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET \
	RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_OFFSET
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK \
	RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_MASK
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB \
	RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_LSB
#define UNIFIED_PHYRX_HT_SIG_0_HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_0_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_B_0_L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_0_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_A_0_L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_A_0_PHYRX_L_SIG_A_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_VHT_SIG_A_0_VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_VHT_SIG_A_0_PHYRX_VHT_SIG_A_INFO_DETAILS_BANDWIDTH_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_SU_0_HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_SU_0_PHYRX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_MU_DL_0_HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_MU_DL_0_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_DL_UL_FLAG_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B1_MU_0_HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B1_MU_0_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_MU_0_HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_MU_0_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_STA_ID_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0_HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_OFDMA_0_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_OFFSET

#define UNIFIED_PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_RSSI_LEGACY_19_RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET
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
	TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_1_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB \
	BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK \
	BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB \
	BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK \
	BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB \
	BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK \
	BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB \
	BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK \
	BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK \
	WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET \
	WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB \
	WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB

#include "hal_6490_tx.h"
#include "hal_6490_rx.h"
#include <hal_generic_api.h>
#include <hal_wbm.h>

/*
 * hal_rx_msdu_start_nss_get_6490(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
static uint32_t
hal_rx_msdu_start_nss_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_MSDU_START_MIMO_SS_BITMAP(msdu_start);

	return qdf_get_hweight8(mimo_ss_bitmap);
}

/**
 * hal_rx_mon_hw_desc_get_mpdu_status_6490(): Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static void hal_rx_mon_hw_desc_get_mpdu_status_6490(void *hw_desc_addr,
						    struct mon_rx_status *rs)
{
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;
	uint32_t reg_value;
	const uint32_t sgi_hw_to_cdp[] = {
		CDP_SGI_0_8_US,
		CDP_SGI_0_4_US,
		CDP_SGI_1_6_US,
		CDP_SGI_3_2_US,
	};

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;

	HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs);

	rs->ant_signal_db = HAL_RX_GET(rx_msdu_start,
				RX_MSDU_START_5, USER_RSSI);
	rs->is_stbc = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, STBC);

	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, SGI);
	rs->sgi = sgi_hw_to_cdp[reg_value];

	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)

static uint32_t hal_get_link_desc_size_6490(void)
{
	return LINK_DESC_SIZE;
}

/*
 * hal_rx_get_tlv_6490(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static uint8_t hal_rx_get_tlv_6490(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY_0, RECEIVE_BANDWIDTH);
}

/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_6490()
 *				    - process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
static
void hal_rx_proc_phyrx_other_receive_info_tlv_6490(void *rx_tlv_hdr,
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
 * hal_rx_dump_msdu_start_tlv_6490() : dump RX msdu_start TLV in structured
 *			     human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_start_tlv_6490(void *msdustart, uint8_t dbg_level)
{
	struct rx_msdu_start *msdu_start = (struct rx_msdu_start *)msdustart;

	hal_verbose_debug(
			  "rx_msdu_start tlv (1/2) - "
			  "rxpcu_mpdu_filter_in_category: %x "
			  "sw_frame_group_id: %x "
			  "phy_ppdu_id: %x "
			  "msdu_length: %x "
			  "ipsec_esp: %x "
			  "l3_offset: %x "
			  "ipsec_ah: %x "
			  "l4_offset: %x "
			  "msdu_number: %x "
			  "decap_format: %x "
			  "ipv4_proto: %x "
			  "ipv6_proto: %x "
			  "tcp_proto: %x "
			  "udp_proto: %x "
			  "ip_frag: %x "
			  "tcp_only_ack: %x "
			  "da_is_bcast_mcast: %x "
			  "ip4_protocol_ip6_next_header: %x "
			  "toeplitz_hash_2_or_4: %x "
			  "flow_id_toeplitz: %x "
			  "user_rssi: %x "
			  "pkt_type: %x "
			  "stbc: %x "
			  "sgi: %x "
			  "rate_mcs: %x "
			  "receive_bandwidth: %x "
			  "reception_type: %x "
			  "ppdu_start_timestamp: %u ",
			  msdu_start->rxpcu_mpdu_filter_in_category,
			  msdu_start->sw_frame_group_id,
			  msdu_start->phy_ppdu_id,
			  msdu_start->msdu_length,
			  msdu_start->ipsec_esp,
			  msdu_start->l3_offset,
			  msdu_start->ipsec_ah,
			  msdu_start->l4_offset,
			  msdu_start->msdu_number,
			  msdu_start->decap_format,
			  msdu_start->ipv4_proto,
			  msdu_start->ipv6_proto,
			  msdu_start->tcp_proto,
			  msdu_start->udp_proto,
			  msdu_start->ip_frag,
			  msdu_start->tcp_only_ack,
			  msdu_start->da_is_bcast_mcast,
			  msdu_start->ip4_protocol_ip6_next_header,
			  msdu_start->toeplitz_hash_2_or_4,
			  msdu_start->flow_id_toeplitz,
			  msdu_start->user_rssi,
			  msdu_start->pkt_type,
			  msdu_start->stbc,
			  msdu_start->sgi,
			  msdu_start->rate_mcs,
			  msdu_start->receive_bandwidth,
			  msdu_start->reception_type,
			  msdu_start->ppdu_start_timestamp);

	hal_verbose_debug(
			  "rx_msdu_start tlv (2/2) - "
			  "sw_phy_meta_data: %x ",
			  msdu_start->sw_phy_meta_data);
}

/**
 * hal_rx_dump_msdu_end_tlv_6490: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_end_tlv_6490(void *msduend,
					  uint8_t dbg_level)
{
	struct rx_msdu_end *msdu_end = (struct rx_msdu_end *)msduend;

	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
		  "rx_msdu_end tlv (1/2) - "
		  "rxpcu_mpdu_filter_in_category: %x "
		  "sw_frame_group_id: %x "
		  "phy_ppdu_id: %x "
		  "ip_hdr_chksum: %x "
		  "tcp_udp_chksum: %x "
		  "key_id_octet: %x "
		  "cce_super_rule: %x "
		  "cce_classify_not_done_truncat: %x "
		  "cce_classify_not_done_cce_dis: %x "
		  "ext_wapi_pn_63_48: %x "
		  "ext_wapi_pn_95_64: %x "
		  "ext_wapi_pn_127_96: %x "
		  "reported_mpdu_length: %x "
		  "first_msdu: %x "
		  "last_msdu: %x "
		  "sa_idx_timeout: %x "
		  "da_idx_timeout: %x "
		  "msdu_limit_error: %x "
		  "flow_idx_timeout: %x "
		  "flow_idx_invalid: %x "
		  "wifi_parser_error: %x "
		  "amsdu_parser_error: %x",
		  msdu_end->rxpcu_mpdu_filter_in_category,
		  msdu_end->sw_frame_group_id,
		  msdu_end->phy_ppdu_id,
		  msdu_end->ip_hdr_chksum,
		  msdu_end->tcp_udp_chksum,
		  msdu_end->key_id_octet,
		  msdu_end->cce_super_rule,
		  msdu_end->cce_classify_not_done_truncate,
		  msdu_end->cce_classify_not_done_cce_dis,
		  msdu_end->ext_wapi_pn_63_48,
		  msdu_end->ext_wapi_pn_95_64,
		  msdu_end->ext_wapi_pn_127_96,
		  msdu_end->reported_mpdu_length,
		  msdu_end->first_msdu,
		  msdu_end->last_msdu,
		  msdu_end->sa_idx_timeout,
		  msdu_end->da_idx_timeout,
		  msdu_end->msdu_limit_error,
		  msdu_end->flow_idx_timeout,
		  msdu_end->flow_idx_invalid,
		  msdu_end->wifi_parser_error,
		  msdu_end->amsdu_parser_error);

	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
		  "rx_msdu_end tlv (2/2)- "
		  "sa_is_valid: %x "
		  "da_is_valid: %x "
		  "da_is_mcbc: %x "
		  "l3_header_padding: %x "
		  "ipv6_options_crc: %x "
		  "tcp_seq_number: %x "
		  "tcp_ack_number: %x "
		  "tcp_flag: %x "
		  "lro_eligible: %x "
		  "window_size: %x "
		  "da_offset: %x "
		  "sa_offset: %x "
		  "da_offset_valid: %x "
		  "sa_offset_valid: %x "
		  "rule_indication_31_0: %x "
		  "rule_indication_63_32: %x "
		  "sa_idx: %x "
		  "da_idx: %x "
		  "msdu_drop: %x "
		  "reo_destination_indication: %x "
		  "flow_idx: %x "
		  "fse_metadata: %x "
		  "cce_metadata: %x "
		  "sa_sw_peer_id: %x ",
		  msdu_end->sa_is_valid,
		  msdu_end->da_is_valid,
		  msdu_end->da_is_mcbc,
		  msdu_end->l3_header_padding,
		  msdu_end->ipv6_options_crc,
		  msdu_end->tcp_seq_number,
		  msdu_end->tcp_ack_number,
		  msdu_end->tcp_flag,
		  msdu_end->lro_eligible,
		  msdu_end->window_size,
		  msdu_end->da_offset,
		  msdu_end->sa_offset,
		  msdu_end->da_offset_valid,
		  msdu_end->sa_offset_valid,
		  msdu_end->rule_indication_31_0,
		  msdu_end->rule_indication_63_32,
		  msdu_end->sa_idx,
		  msdu_end->da_idx_or_sw_peer_id,
		  msdu_end->msdu_drop,
		  msdu_end->reo_destination_indication,
		  msdu_end->flow_idx,
		  msdu_end->fse_metadata,
		  msdu_end->cce_metadata,
		  msdu_end->sa_sw_peer_id);
}

/*
 * Get tid from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_TID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_7_TID_OFFSET)),		\
		RX_MPDU_INFO_7_TID_MASK,		\
		RX_MPDU_INFO_7_TID_LSB))

static uint32_t hal_rx_mpdu_start_tid_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t tid;

	tid = HAL_RX_MPDU_INFO_TID_GET(&mpdu_start->rx_mpdu_info_details);

	return tid;
}

#define HAL_RX_MSDU_START_RECEPTION_TYPE_GET(_rx_msdu_start) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),	\
	RX_MSDU_START_5_RECEPTION_TYPE_OFFSET)),	\
	RX_MSDU_START_5_RECEPTION_TYPE_MASK,		\
	RX_MSDU_START_5_RECEPTION_TYPE_LSB))

/*
 * hal_rx_msdu_start_reception_type_get(): API to get the reception type
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(reception_type)
 */
static
uint32_t hal_rx_msdu_start_reception_type_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t reception_type;

	reception_type = HAL_RX_MSDU_START_RECEPTION_TYPE_GET(msdu_start);

	return reception_type;
}

#define HAL_RX_MSDU_END_DA_IDX_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_OFFSET)),	\
		RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_MASK,	\
		RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_LSB))

/**
 * hal_rx_msdu_end_da_idx_get_6490: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static uint16_t hal_rx_msdu_end_da_idx_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t da_idx;

	da_idx = HAL_RX_MSDU_END_DA_IDX_GET(msdu_end);

	return da_idx;
}
/**
 * hal_rx_get_rx_fragment_number_6490(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static
uint8_t hal_rx_get_rx_fragment_number_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	/* Return first 4 bits as fragment number */
	return (HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info) &
		DOT11_SEQ_FRAG_MASK);
}

/**
 * hal_rx_msdu_end_da_is_mcbc_get_6490(): API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_mcbc
 */
static uint8_t
hal_rx_msdu_end_da_is_mcbc_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_DA_IS_MCBC_GET(msdu_end);
}

/**
 * hal_rx_msdu_end_sa_is_valid_get_6490(): API to get_6490 the
 * sa_is_valid bit from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static uint8_t
hal_rx_msdu_end_sa_is_valid_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t sa_is_valid;

	sa_is_valid = HAL_RX_MSDU_END_SA_IS_VALID_GET(msdu_end);

	return sa_is_valid;
}

/**
 * hal_rx_msdu_end_sa_idx_get_6490(): API to get_6490 the
 * sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static
uint16_t hal_rx_msdu_end_sa_idx_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t sa_idx;

	sa_idx = HAL_RX_MSDU_END_SA_IDX_GET(msdu_end);

	return sa_idx;
}

/**
 * hal_rx_desc_is_first_msdu_6490() - Check if first msdu
 *
 * @hal_soc_hdl: hal_soc handle
 * @hw_desc_addr: hardware descriptor address
 *
 * Return: 0 - success/ non-zero failure
 */
static uint32_t hal_rx_desc_is_first_msdu_6490(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_tlvs = (struct rx_pkt_tlvs *)hw_desc_addr;
	struct rx_msdu_end *msdu_end = &rx_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_GET(msdu_end, RX_MSDU_END_10, FIRST_MSDU);
}

/**
 * hal_rx_msdu_end_l3_hdr_padding_get_6490(): API to get_6490 the
 * l3_header padding from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static uint32_t hal_rx_msdu_end_l3_hdr_padding_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);

	return l3_header_padding;
}

/*
 * @ hal_rx_encryption_info_valid_6490: Returns encryption type.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: encryption type
 */
static uint32_t hal_rx_encryption_info_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t encryption_info = HAL_RX_MPDU_ENCRYPTION_INFO_VALID(mpdu_info);

	return encryption_info;
}

/*
 * @ hal_rx_print_pn_6490: Prints the PN of rx packet.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: void
 */
static void hal_rx_print_pn_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	uint32_t pn_31_0 = HAL_RX_MPDU_PN_31_0_GET(mpdu_info);
	uint32_t pn_63_32 = HAL_RX_MPDU_PN_63_32_GET(mpdu_info);
	uint32_t pn_95_64 = HAL_RX_MPDU_PN_95_64_GET(mpdu_info);
	uint32_t pn_127_96 = HAL_RX_MPDU_PN_127_96_GET(mpdu_info);

	hal_debug("PN number pn_127_96 0x%x pn_95_64 0x%x pn_63_32 0x%x pn_31_0 0x%x ",
		  pn_127_96, pn_95_64, pn_63_32, pn_31_0);
}

/**
 * hal_rx_msdu_end_first_msdu_get_6490: API to get first msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: first_msdu
 */
static uint8_t hal_rx_msdu_end_first_msdu_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t first_msdu;

	first_msdu = HAL_RX_MSDU_END_FIRST_MSDU_GET(msdu_end);

	return first_msdu;
}

/**
 * hal_rx_msdu_end_da_is_valid_get_6490: API to check if da is valid
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_valid
 */
static uint8_t hal_rx_msdu_end_da_is_valid_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t da_is_valid;

	da_is_valid = HAL_RX_MSDU_END_DA_IS_VALID_GET(msdu_end);

	return da_is_valid;
}

/**
 * hal_rx_msdu_end_last_msdu_get_6490: API to get last msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: last_msdu
 */
static uint8_t hal_rx_msdu_end_last_msdu_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t last_msdu;

	last_msdu = HAL_RX_MSDU_END_LAST_MSDU_GET(msdu_end);

	return last_msdu;
}

/*
 * hal_rx_get_mpdu_mac_ad4_valid_6490(): Retrieves if mpdu 4th addr is valid
 *
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static bool hal_rx_get_mpdu_mac_ad4_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	bool ad4_valid = 0;

	ad4_valid = HAL_RX_MPDU_GET_MAC_AD4_VALID(rx_mpdu_info);

	return ad4_valid;
}

/**
 * hal_rx_mpdu_start_sw_peer_id_get_6490: Retrieve sw peer_id
 * @buf: network buffer
 *
 * Return: sw peer_id
 */
static uint32_t hal_rx_mpdu_start_sw_peer_id_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	return HAL_RX_MPDU_INFO_SW_PEER_ID_GET(
		&mpdu_start->rx_mpdu_info_details);
}

/**
 * hal_rx_mpdu_get_to_ds_6490(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */
static uint32_t hal_rx_mpdu_get_to_ds_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_TODS(mpdu_info);
}

/*
 * hal_rx_mpdu_get_fr_ds_6490(): API to get the from ds info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(fr_ds)
 */
static uint32_t hal_rx_mpdu_get_fr_ds_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_FROMDS(mpdu_info);
}

/*
 * hal_rx_get_mpdu_frame_control_valid_6490(): Retrieves mpdu
 * frame control valid
 *
 * @nbuf: Network buffer
 * Returns: value of frame control valid field
 */
static uint8_t hal_rx_get_mpdu_frame_control_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(rx_mpdu_info);
}

/*
 * hal_rx_mpdu_get_addr1_6490(): API to check get address1 of the mpdu
 *
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr1_6490(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr1 {
		uint32_t ad1_31_0;
		uint16_t ad1_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr1 *addr = (struct hal_addr1 *)mac_addr;
	uint32_t mac_addr_ad1_valid;

	mac_addr_ad1_valid = HAL_RX_MPDU_MAC_ADDR_AD1_VALID_GET(mpdu_info);

	if (mac_addr_ad1_valid) {
		addr->ad1_31_0 = HAL_RX_MPDU_AD1_31_0_GET(mpdu_info);
		addr->ad1_47_32 = HAL_RX_MPDU_AD1_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr2_6490(): API to check get address2 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr2_6490(uint8_t *buf,
					     uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr2 {
		uint16_t ad2_15_0;
		uint32_t ad2_47_16;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr2 *addr = (struct hal_addr2 *)mac_addr;
	uint32_t mac_addr_ad2_valid;

	mac_addr_ad2_valid = HAL_RX_MPDU_MAC_ADDR_AD2_VALID_GET(mpdu_info);

	if (mac_addr_ad2_valid) {
		addr->ad2_15_0 = HAL_RX_MPDU_AD2_15_0_GET(mpdu_info);
		addr->ad2_47_16 = HAL_RX_MPDU_AD2_47_16_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr3_6490(): API to get address3 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr3_6490(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr3 {
		uint32_t ad3_31_0;
		uint16_t ad3_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr3 *addr = (struct hal_addr3 *)mac_addr;
	uint32_t mac_addr_ad3_valid;

	mac_addr_ad3_valid = HAL_RX_MPDU_MAC_ADDR_AD3_VALID_GET(mpdu_info);

	if (mac_addr_ad3_valid) {
		addr->ad3_31_0 = HAL_RX_MPDU_AD3_31_0_GET(mpdu_info);
		addr->ad3_47_32 = HAL_RX_MPDU_AD3_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr4_6490(): API to get address4 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr4_6490(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr4 {
		uint32_t ad4_31_0;
		uint16_t ad4_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr4 *addr = (struct hal_addr4 *)mac_addr;
	uint32_t mac_addr_ad4_valid;

	mac_addr_ad4_valid = HAL_RX_MPDU_MAC_ADDR_AD4_VALID_GET(mpdu_info);

	if (mac_addr_ad4_valid) {
		addr->ad4_31_0 = HAL_RX_MPDU_AD4_31_0_GET(mpdu_info);
		addr->ad4_47_32 = HAL_RX_MPDU_AD4_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_get_mpdu_sequence_control_valid_6490(): Get mpdu
 * sequence control valid
 *
 * @nbuf: Network buffer
 * Returns: value of sequence control valid field
 */
static uint8_t hal_rx_get_mpdu_sequence_control_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(rx_mpdu_info);
}

/**
 * hal_rx_is_unicast_6490: check packet is unicast frame or not.
 *
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: true on unicast.
 */
static bool hal_rx_is_unicast_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
		&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t grp_id;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;

	grp_id = (_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			   RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_OFFSET)),
			  RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_MASK,
			  RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_LSB));

	return (HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA == grp_id) ? true : false;
}

/**
 * hal_rx_tid_get_6490: get tid based on qos control valid.
 * @hal_soc_hdl: hal_soc handle
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: tid
 */
static uint32_t hal_rx_tid_get_6490(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
	&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;
	uint8_t qos_control_valid =
		(_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			  RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_OFFSET)),
			 RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_MASK,
			 RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_LSB));

	if (qos_control_valid)
		return hal_rx_mpdu_start_tid_get_6490(buf);

	return HAL_RX_NON_QOS_TID;
}

/**
 * hal_rx_hw_desc_get_ppduid_get_6490(): retrieve ppdu id
 * @hw_desc_addr: hw addr
 *
 * Return: ppdu id
 */
static uint32_t hal_rx_hw_desc_get_ppduid_get_6490(void *hw_desc_addr)
{
	struct rx_mpdu_info *rx_mpdu_info;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_mpdu_info =
		&rx_desc->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	return HAL_RX_GET(rx_mpdu_info, RX_MPDU_INFO_9, PHY_PPDU_ID);
}

/**
 * hal_reo_status_get_header_6490 - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_6490(uint32_t *d, int b, void *h1)
{
	uint32_t val1 = 0;
	struct hal_reo_status_header *h =
			(struct hal_reo_status_header *)h1;

	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0,
		  STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->cmd_num =
		HAL_GET_FIELD(
			      UNIFORM_REO_STATUS_HEADER_0, REO_STATUS_NUMBER,
			      val1);
	h->exec_time =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_0,
			      CMD_EXECUTION_TIME, val1);
	h->status =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_0,
			      REO_CMD_EXECUTION_STATUS, val1);
	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1,
		  STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->tstamp =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_1, TIMESTAMP, val1);
}

/**
 * hal_tx_desc_set_mesh_en_6490 - Set mesh_enable flag in Tx descriptor
 * @desc: Handle to Tx Descriptor
 * @en:   For raw WiFi frames, this indicates transmission to a mesh STA,
 *        enabling the interpretation of the 'Mesh Control Present' bit
 *        (bit 8) of QoS Control (otherwise this bit is ignored),
 *        For native WiFi frames, this indicates that a 'Mesh Control' field
 *        is present between the header and the LLC.
 *
 * Return: void
 */
static inline
void hal_tx_desc_set_mesh_en_6490(void *desc, uint8_t en)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5, MESH_ENABLE) |=
		HAL_TX_SM(TCL_DATA_CMD_5, MESH_ENABLE, en);
}

static
void *hal_rx_msdu0_buffer_addr_lsb_6490(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_6490(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_6490(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_6490(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

static
uint8_t hal_rx_get_fc_valid_6490(uint8_t *buf)
{
	return HAL_RX_GET_FC_VALID(buf);
}

static uint8_t hal_rx_get_to_ds_flag_6490(uint8_t *buf)
{
	return HAL_RX_GET_TO_DS_FLAG(buf);
}

static uint8_t hal_rx_get_mac_addr2_valid_6490(uint8_t *buf)
{
	return HAL_RX_GET_MAC_ADDR2_VALID(buf);
}

static uint8_t hal_rx_get_filter_category_6490(uint8_t *buf)
{
	return HAL_RX_GET_FILTER_CATEGORY(buf);
}

static uint32_t
hal_rx_get_ppdu_id_6490(uint8_t *buf)
{
	return HAL_RX_GET_PPDU_ID(buf);
}

/**
 * hal_reo_config_6490(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static
void hal_reo_config_6490(struct hal_soc *soc,
			 uint32_t reg_val,
			 struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_6490() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_6490(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_6490 - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_6490(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_rx_msdu_flow_idx_get_6490: API to get flow index
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index value from MSDU END TLV
 */
static inline uint32_t hal_rx_msdu_flow_idx_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_invalid_6490: API to get flow index invalid
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index invalid value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_invalid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_timeout_6490: API to get flow index timeout
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index timeout value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_timeout_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(msdu_end);
}

/**
 * hal_rx_msdu_fse_metadata_get_6490: API to get FSE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fse metadata value from MSDU END TLV
 */
static uint32_t hal_rx_msdu_fse_metadata_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FSE_METADATA_GET(msdu_end);
}

/**
 * hal_rx_msdu_cce_metadata_get_6490: API to get CCE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: cce_metadata
 */
static uint16_t
hal_rx_msdu_cce_metadata_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_CCE_METADATA_GET(msdu_end);
}

/**
 * hal_rx_tlv_get_tcp_chksum_6490() - API to get tcp checksum
 * @buf: rx_tlv_hdr
 *
 * Return: tcp checksum
 */
static uint16_t
hal_rx_tlv_get_tcp_chksum_6490(uint8_t *buf)
{
	return HAL_RX_TLV_GET_TCP_CHKSUM(buf);
}

/**
 * hal_rx_get_rx_sequence_6490(): Function to retrieve rx sequence number
 *
 * @nbuf: Network buffer
 * Returns: rx sequence number
 */
static
uint16_t hal_rx_get_rx_sequence_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info);
}

/**
 * hal_get_window_address_6490(): Function to get hp/tp address
 * @hal_soc: Pointer to hal_soc
 * @addr: address offset of register
 *
 * Return: modified address offset of register
 */
static inline qdf_iomem_t hal_get_window_address_6490(struct hal_soc *hal_soc,
						      qdf_iomem_t addr)
{
	return addr;
}

struct hal_hw_txrx_ops qca6490_hal_hw_txrx_ops = {
	/* init and setup */
	hal_srng_dst_hw_init_generic,
	hal_srng_src_hw_init_generic,
	hal_get_hw_hptp_generic,
	hal_reo_setup_generic,
	hal_setup_link_idle_list_generic,
	hal_get_window_address_6490,

	/* tx */
	hal_tx_desc_set_dscp_tid_table_id_6490,
	hal_tx_set_dscp_tid_map_6490,
	hal_tx_update_dscp_tid_6490,
	hal_tx_desc_set_lmac_id_6490,
	hal_tx_desc_set_buf_addr_generic,
	hal_tx_desc_set_search_type_generic,
	hal_tx_desc_set_search_index_generic,
	hal_tx_desc_set_cache_set_num_generic,
	hal_tx_comp_get_status_generic,
	hal_tx_comp_get_release_reason_generic,
	hal_get_wbm_internal_error_generic,
	hal_tx_desc_set_mesh_en_6490,

	/* rx */
	hal_rx_msdu_start_nss_get_6490,
	hal_rx_mon_hw_desc_get_mpdu_status_6490,
	hal_rx_get_tlv_6490,
	hal_rx_proc_phyrx_other_receive_info_tlv_6490,
	hal_rx_dump_msdu_start_tlv_6490,
	hal_rx_dump_msdu_end_tlv_6490,
	hal_get_link_desc_size_6490,
	hal_rx_mpdu_start_tid_get_6490,
	hal_rx_msdu_start_reception_type_get_6490,
	hal_rx_msdu_end_da_idx_get_6490,
	hal_rx_msdu_desc_info_get_ptr_6490,
	hal_rx_link_desc_msdu0_ptr_6490,
	hal_reo_status_get_header_6490,
	hal_rx_status_get_tlv_info_generic,
	hal_rx_wbm_err_info_get_generic,
	hal_rx_dump_mpdu_start_tlv_generic,

	hal_tx_set_pcp_tid_map_generic,
	hal_tx_update_pcp_tid_generic,
	hal_tx_update_tidmap_prty_generic,
	hal_rx_get_rx_fragment_number_6490,
	hal_rx_msdu_end_da_is_mcbc_get_6490,
	hal_rx_msdu_end_sa_is_valid_get_6490,
	hal_rx_msdu_end_sa_idx_get_6490,
	hal_rx_desc_is_first_msdu_6490,
	hal_rx_msdu_end_l3_hdr_padding_get_6490,
	hal_rx_encryption_info_valid_6490,
	hal_rx_print_pn_6490,
	hal_rx_msdu_end_first_msdu_get_6490,
	hal_rx_msdu_end_da_is_valid_get_6490,
	hal_rx_msdu_end_last_msdu_get_6490,
	hal_rx_get_mpdu_mac_ad4_valid_6490,
	hal_rx_mpdu_start_sw_peer_id_get_6490,
	hal_rx_mpdu_get_to_ds_6490,
	hal_rx_mpdu_get_fr_ds_6490,
	hal_rx_get_mpdu_frame_control_valid_6490,
	hal_rx_mpdu_get_addr1_6490,
	hal_rx_mpdu_get_addr2_6490,
	hal_rx_mpdu_get_addr3_6490,
	hal_rx_mpdu_get_addr4_6490,
	hal_rx_get_mpdu_sequence_control_valid_6490,
	hal_rx_is_unicast_6490,
	hal_rx_tid_get_6490,
	hal_rx_hw_desc_get_ppduid_get_6490,
	NULL,
	NULL,
	hal_rx_msdu0_buffer_addr_lsb_6490,
	hal_rx_msdu_desc_info_ptr_get_6490,
	hal_ent_mpdu_desc_info_6490,
	hal_dst_mpdu_desc_info_6490,
	hal_rx_get_fc_valid_6490,
	hal_rx_get_to_ds_flag_6490,
	hal_rx_get_mac_addr2_valid_6490,
	hal_rx_get_filter_category_6490,
	hal_rx_get_ppdu_id_6490,
	hal_reo_config_6490,
	hal_rx_msdu_flow_idx_get_6490,
	hal_rx_msdu_flow_idx_invalid_6490,
	hal_rx_msdu_flow_idx_timeout_6490,
	hal_rx_msdu_fse_metadata_get_6490,
	hal_rx_msdu_cce_metadata_get_6490,
	NULL,
	hal_rx_tlv_get_tcp_chksum_6490,
	hal_rx_get_rx_sequence_6490,
	NULL,
	NULL,
};

struct hal_hw_srng_config hw_srng_table_6490[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 4,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2SW1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
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
		/* Designating REO2TCL ring as exception ring. This ring is
		 * similar to other REO2SW rings though it is named as REO2TCL.
		 * Any of theREO2SW rings can be used as exception ring.
		 */
		.start_ring_id = HAL_SRNG_REO2TCL,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2TCL_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2TCL_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_REINJECT */
		.start_ring_id = HAL_SRNG_SW2REO,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_SW2REO_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
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
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_CMD_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
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
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
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
		.max_rings = 3,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_data_cmd)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
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
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_gse_cmd)) >> 2,
		.lmac_ring =  FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL_CREDIT_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
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
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
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
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
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
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
			/* TODO: check destination status ring registers */
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
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
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		.max_rings = 4,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		.reg_size = {
		HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		.max_rings = 1,
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
};

int32_t hal_hw_reg_offset_qca6490[] = {
	/* dst */
	REG_OFFSET(DST, HP),
	REG_OFFSET(DST, TP),
	REG_OFFSET(DST, ID),
	REG_OFFSET(DST, MISC),
	REG_OFFSET(DST, HP_ADDR_LSB),
	REG_OFFSET(DST, HP_ADDR_MSB),
	REG_OFFSET(DST, MSI1_BASE_LSB),
	REG_OFFSET(DST, MSI1_BASE_MSB),
	REG_OFFSET(DST, MSI1_DATA),
	REG_OFFSET(DST, BASE_LSB),
	REG_OFFSET(DST, BASE_MSB),
	REG_OFFSET(DST, PRODUCER_INT_SETUP),
	/* src */
	REG_OFFSET(SRC, HP),
	REG_OFFSET(SRC, TP),
	REG_OFFSET(SRC, ID),
	REG_OFFSET(SRC, MISC),
	REG_OFFSET(SRC, TP_ADDR_LSB),
	REG_OFFSET(SRC, TP_ADDR_MSB),
	REG_OFFSET(SRC, MSI1_BASE_LSB),
	REG_OFFSET(SRC, MSI1_BASE_MSB),
	REG_OFFSET(SRC, MSI1_DATA),
	REG_OFFSET(SRC, BASE_LSB),
	REG_OFFSET(SRC, BASE_MSB),
	REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX0),
	REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX1),
};

/**
 * hal_qca6490_attach() - Attach 6490 target specific hal_soc ops,
 *			  offset and srng table
 */
void hal_qca6490_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_6490;
	hal_soc->hal_hw_reg_offset = hal_hw_reg_offset_qca6490;
	hal_soc->ops = &qca6490_hal_hw_txrx_ops;
}
