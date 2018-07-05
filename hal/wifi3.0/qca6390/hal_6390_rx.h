/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define HAL_RX_MSDU_START_MIMO_SS_BITMAP(_rx_msdu_start)\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_MIMO_SS_BITMAP_OFFSET)),	\
	RX_MSDU_START_5_MIMO_SS_BITMAP_MASK,		\
	RX_MSDU_START_5_MIMO_SS_BITMAP_LSB))

/*
 * hal_rx_msdu_start_nss_get_6390(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
uint32_t
hal_rx_msdu_start_nss_get_6390(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_MSDU_START_MIMO_SS_BITMAP(msdu_start);

	return qdf_get_hweight8(mimo_ss_bitmap);
}

qdf_export_symbol(hal_rx_msdu_start_nss_get_6390);
/**
 * hal_rx_mon_hw_desc_get_mpdu_status_6390(): Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
void hal_rx_mon_hw_desc_get_mpdu_status_6390(void *hw_desc_addr,
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

	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, PKT_TYPE);
	switch (reg_value) {
	case HAL_RX_PKT_TYPE_11N:
		rs->ht_flags = 1;
		break;
	case HAL_RX_PKT_TYPE_11AC:
		rs->vht_flags = 1;
		reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5,
				       RECEIVE_BANDWIDTH);
		rs->vht_flag_values2 = reg_value;
		break;
	case HAL_RX_PKT_TYPE_11AX:
		rs->he_flags = 1;
		break;
	default:
		break;
	}
	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

qdf_export_symbol(hal_rx_mon_hw_desc_get_mpdu_status_6390);

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)

uint32_t hal_get_link_desc_size_6390(void)
{
	return LINK_DESC_SIZE;
}

qdf_export_symbol(hal_get_link_desc_size_6390);

/*
 * hal_rx_get_tlv_6390(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
uint8_t hal_rx_get_tlv_6390(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY_0, RECEIVE_BANDWIDTH);
}

qdf_export_symbol(hal_rx_get_tlv_6390);

/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_6390()
 *				    - process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
void hal_rx_proc_phyrx_other_receive_info_tlv_6390(void *rx_tlv_hdr,
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

		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_20)
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0;

		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_40)
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU1;

		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_80)
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU2;

		if (ppdu_info->rx_status.bw >= HAL_FULL_RX_BW_160)
			ppdu_info->rx_status.he_sig_b_common_known |=
				QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU3;
			break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s unhandled TLV type: %d, TLV len:%d",
			  __func__, other_tlv_tag, other_tlv_len);
		break;
	}
}

qdf_export_symbol(hal_rx_proc_phyrx_other_receive_info_tlv_6390);

/**
 * hal_rx_dump_msdu_start_tlv_6390() : dump RX msdu_start TLV in structured
 *			     human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
void hal_rx_dump_msdu_start_tlv_6390(void *msdustart,   uint8_t dbg_level)
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
			msdu_start->ppdu_start_timestamp,
			msdu_start->sw_phy_meta_data);
}

qdf_export_symbol(hal_rx_dump_msdu_start_tlv_6390);

/*
 * Get tid from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_TID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_3_TID_OFFSET)),		\
		RX_MPDU_INFO_3_TID_MASK,		\
		RX_MPDU_INFO_3_TID_LSB))

uint32_t hal_rx_mpdu_start_tid_get_6390(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t tid;

	tid = HAL_RX_MPDU_INFO_TID_GET(&mpdu_start->rx_mpdu_info_details);

	return tid;
}

qdf_export_symbol(hal_rx_mpdu_start_tid_get_6390);

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
uint32_t hal_rx_msdu_start_reception_type_get_6390(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t reception_type;

	reception_type = HAL_RX_MSDU_START_RECEPTION_TYPE_GET(msdu_start);

	return reception_type;
}

qdf_export_symbol(hal_rx_msdu_start_reception_type_get_6390);

