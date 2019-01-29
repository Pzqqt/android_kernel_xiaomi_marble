/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "qdf_util.h"
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "tcl_data_cmd.h"
#include "mac_tcl_reg_seq_hwioreg.h"
#include "phyrx_rssi_legacy.h"
#include "rx_msdu_start.h"
#include "tlv_tag_def.h"
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"
#include "phyrx_other_receive_info_ru_details.h"

#if defined(QCA_WIFI_QCA6290_11AX)
#define HAL_RX_MSDU_START_MIMO_SS_BITMAP(_rx_msdu_start)\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_MIMO_SS_BITMAP_OFFSET)),	\
	RX_MSDU_START_5_MIMO_SS_BITMAP_MASK,		\
	RX_MSDU_START_5_MIMO_SS_BITMAP_LSB))

/*
 * hal_rx_msdu_start_nss_get_6290(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
static uint32_t
hal_rx_msdu_start_nss_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_MSDU_START_MIMO_SS_BITMAP(msdu_start);

	return qdf_get_hweight8(mimo_ss_bitmap);
}
#else
static uint32_t
hal_rx_msdu_start_nss_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t nss;

	nss = HAL_RX_MSDU_START_NSS_GET(msdu_start);
	return nss;
}
#endif

/**
 * hal_rx_mon_hw_desc_get_mpdu_status_6290(): Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static void hal_rx_mon_hw_desc_get_mpdu_status_6290(void *hw_desc_addr,
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
#if !defined(QCA_WIFI_QCA6290_11AX)
	rs->nr_ant = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, NSS);
#endif
	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)

static uint32_t hal_get_link_desc_size_6290(void)
{
	return LINK_DESC_SIZE;
}


#ifdef QCA_WIFI_QCA6290_11AX
/*
 * hal_rx_get_tlv_6290(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static uint8_t hal_rx_get_tlv_6290(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY_0, RECEIVE_BANDWIDTH);
}
#else
static uint8_t hal_rx_get_tlv_6290(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY_35, RECEIVE_BANDWIDTH);
}
#endif

#ifdef QCA_WIFI_QCA6290_11AX
/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_6290()
 *				    - process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
static
void hal_rx_proc_phyrx_other_receive_info_tlv_6290(void *rx_tlv_hdr,
						   void *ppdu_info_handle)
{
	uint32_t tlv_tag, tlv_len;
	uint32_t temp_len, other_tlv_len, other_tlv_tag;
	void *rx_tlv = (uint8_t *)rx_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;
	void *other_tlv_hdr = NULL;
	void *other_tlv = NULL;
	uint32_t ru_details_channel_0;
	struct hal_rx_ppdu_info *ppdu_info =
		(struct hal_rx_ppdu_info *)ppdu_info_handle;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv_hdr);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv_hdr);
	temp_len = 0;

	other_tlv_hdr = rx_tlv + HAL_RX_TLV32_HDR_SIZE;

	other_tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(other_tlv_hdr);
	other_tlv_len = HAL_RX_GET_USER_TLV32_LEN(other_tlv_hdr);
	temp_len += other_tlv_len;
	other_tlv = other_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;

	switch (other_tlv_tag) {
	case WIFIPHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_E:
		ru_details_channel_0 =
			HAL_RX_GET(other_tlv,
				   PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_0,
				   RU_DETAILS_CHANNEL_0);

		qdf_mem_copy(ppdu_info->rx_status.he_RU,
			     &ru_details_channel_0,
			     sizeof(ppdu_info->rx_status.he_RU));

		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_20) {
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0;
		}
		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_40) {
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU1;
		}
		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_80) {
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU2;
		}
		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_160) {
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU3;
		}
			break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s unhandled TLV type: %d, TLV len:%d",
			  __func__, other_tlv_tag, other_tlv_len);
		break;
	}
}
#else
static
void hal_rx_proc_phyrx_other_receive_info_tlv_6290(void *rx_tlv_hdr,
						   void *ppdu_info_handle)
{
}
#endif /* QCA_WIFI_QCA6290_11AX */

/**
 * hal_rx_dump_msdu_start_tlv_6290() : dump RX msdu_start TLV in structured
 *			     human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_start_tlv_6290(void *msdustart,
					    uint8_t dbg_level)
{
	struct rx_msdu_start *msdu_start = (struct rx_msdu_start *)msdustart;

	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_msdu_start tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"phy_ppdu_id: %d "
			"msdu_length: %d "
			"ipsec_esp: %d "
			"l3_offset: %d "
			"ipsec_ah: %d "
			"l4_offset: %d "
			"msdu_number: %d "
			"decap_format: %d "
			"ipv4_proto: %d "
			"ipv6_proto: %d "
			"tcp_proto: %d "
			"udp_proto: %d "
			"ip_frag: %d "
			"tcp_only_ack: %d "
			"da_is_bcast_mcast: %d "
			"ip4_protocol_ip6_next_header: %d "
			"toeplitz_hash_2_or_4: %d "
			"flow_id_toeplitz: %d "
			"user_rssi: %d "
			"pkt_type: %d "
			"stbc: %d "
			"sgi: %d "
			"rate_mcs: %d "
			"receive_bandwidth: %d "
			"reception_type: %d "
#if !defined(QCA_WIFI_QCA6290_11AX)
			"toeplitz_hash: %d "
			"nss: %d "
#endif
			"ppdu_start_timestamp: %d "
			"sw_phy_meta_data: %d ",
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
#if !defined(QCA_WIFI_QCA6290_11AX)
			msdu_start->toeplitz_hash,
			msdu_start->nss,
#endif
			msdu_start->ppdu_start_timestamp,
			msdu_start->sw_phy_meta_data);
}

/**
 * hal_rx_dump_msdu_end_tlv_6290: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_end_tlv_6290(void *msduend,
					  uint8_t dbg_level)
{
	struct rx_msdu_end *msdu_end = (struct rx_msdu_end *)msduend;

	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_msdu_end tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"phy_ppdu_id: %d "
			"ip_hdr_chksum: %d "
			"tcp_udp_chksum: %d "
			"key_id_octet: %d "
			"cce_super_rule: %d "
			"cce_classify_not_done_truncat: %d "
			"cce_classify_not_done_cce_dis: %d "
			"ext_wapi_pn_63_48: %d "
			"ext_wapi_pn_95_64: %d "
			"ext_wapi_pn_127_96: %d "
			"reported_mpdu_length: %d "
			"first_msdu: %d "
			"last_msdu: %d "
			"sa_idx_timeout: %d "
			"da_idx_timeout: %d "
			"msdu_limit_error: %d "
			"flow_idx_timeout: %d "
			"flow_idx_invalid: %d "
			"wifi_parser_error: %d "
			"amsdu_parser_error: %d "
			"sa_is_valid: %d "
			"da_is_valid: %d "
			"da_is_mcbc: %d "
			"l3_header_padding: %d "
			"ipv6_options_crc: %d "
			"tcp_seq_number: %d "
			"tcp_ack_number: %d "
			"tcp_flag: %d "
			"lro_eligible: %d "
			"window_size: %d "
			"da_offset: %d "
			"sa_offset: %d "
			"da_offset_valid: %d "
			"sa_offset_valid: %d "
			"rule_indication_31_0: %d "
			"rule_indication_63_32: %d "
			"sa_idx: %d "
			"da_idx: %d "
			"msdu_drop: %d "
			"reo_destination_indication: %d "
			"flow_idx: %d "
			"fse_metadata: %d "
			"cce_metadata: %d "
			"sa_sw_peer_id: %d ",
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
			msdu_end->amsdu_parser_error,
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
			msdu_end->da_idx,
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
		RX_MPDU_INFO_3_TID_OFFSET)),		\
		RX_MPDU_INFO_3_TID_MASK,		\
		RX_MPDU_INFO_3_TID_LSB))

static uint32_t hal_rx_mpdu_start_tid_get_6290(uint8_t *buf)
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
static uint32_t hal_rx_msdu_start_reception_type_get_6290(uint8_t *buf)
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
		RX_MSDU_END_13_DA_IDX_OFFSET)),		\
		RX_MSDU_END_13_DA_IDX_MASK,		\
		RX_MSDU_END_13_DA_IDX_LSB))

/**
 * hal_rx_msdu_end_da_idx_get_6290: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static uint16_t hal_rx_msdu_end_da_idx_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t da_idx;

	da_idx = HAL_RX_MSDU_END_DA_IDX_GET(msdu_end);

	return da_idx;
}

