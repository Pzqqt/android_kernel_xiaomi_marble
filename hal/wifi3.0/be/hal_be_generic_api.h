/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_BE_GENERIC_API_H_
#define _HAL_BE_GENERIC_API_H_

#include <hal_be_hw_headers.h>
#include "hal_be_tx.h"
#include "hal_be_reo.h"
#include <hal_api_mon.h>
#include <hal_generic_api.h>

/**
 * hal_tx_comp_get_status() - TQM Release reason
 * @hal_desc: completion ring Tx status
 *
 * This function will parse the WBM completion descriptor and populate in
 * HAL structure
 *
 * Return: none
 */
static inline void
hal_tx_comp_get_status_generic_be(void *desc, void *ts1,
				  struct hal_soc *hal)
{
	uint8_t rate_stats_valid = 0;
	uint32_t rate_stats = 0;
	struct hal_tx_completion_status *ts =
		(struct hal_tx_completion_status *)ts1;

	ts->ppdu_id = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
				      TQM_STATUS_NUMBER);
	ts->ack_frame_rssi = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
					     ACK_FRAME_RSSI);
	ts->first_msdu = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
					 FIRST_MSDU);
	ts->last_msdu = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
					LAST_MSDU);
#if 0
	// TODO -  This has to be calculated form first and last msdu
	ts->msdu_part_of_amsdu = HAL_TX_DESC_GET(desc,
						 WBM2SW_COMPLETION_RING_TX,
						 MSDU_PART_OF_AMSDU);
#endif

	ts->peer_id = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
				      SW_PEER_ID);
	ts->tid = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX, TID);
	ts->transmit_cnt = HAL_TX_DESC_GET(desc, WBM2SW_COMPLETION_RING_TX,
					   TRANSMIT_COUNT);

	rate_stats = HAL_TX_DESC_GET(desc, HAL_TX_COMP, TX_RATE_STATS);

	rate_stats_valid = HAL_TX_MS(TX_RATE_STATS_INFO,
			TX_RATE_STATS_INFO_VALID, rate_stats);

	ts->valid = rate_stats_valid;

	if (rate_stats_valid) {
		ts->bw = HAL_TX_MS(TX_RATE_STATS_INFO, TRANSMIT_BW,
				rate_stats);
		ts->pkt_type = HAL_TX_MS(TX_RATE_STATS_INFO,
				TRANSMIT_PKT_TYPE, rate_stats);
		ts->stbc = HAL_TX_MS(TX_RATE_STATS_INFO,
				TRANSMIT_STBC, rate_stats);
		ts->ldpc = HAL_TX_MS(TX_RATE_STATS_INFO, TRANSMIT_LDPC,
				rate_stats);
		ts->sgi = HAL_TX_MS(TX_RATE_STATS_INFO, TRANSMIT_SGI,
				rate_stats);
		ts->mcs = HAL_TX_MS(TX_RATE_STATS_INFO, TRANSMIT_MCS,
				rate_stats);
		ts->ofdma = HAL_TX_MS(TX_RATE_STATS_INFO, OFDMA_TRANSMISSION,
				rate_stats);
		ts->tones_in_ru = HAL_TX_MS(TX_RATE_STATS_INFO, TONES_IN_RU,
				rate_stats);
	}

	ts->release_src = hal_tx_comp_get_buffer_source_generic_be(desc);
	ts->status = hal_tx_comp_get_release_reason(
					desc,
					hal_soc_to_hal_soc_handle(hal));

	ts->tsf = HAL_TX_DESC_GET(desc, UNIFIED_WBM_RELEASE_RING_6,
			TX_RATE_STATS_INFO_TX_RATE_STATS);
}

#if defined(QCA_WIFI_QCA6290_11AX_MU_UL) && defined(QCA_WIFI_QCA6290_11AX)
/**
 * hal_rx_handle_other_tlvs() - handle special TLVs like MU_UL
 * tlv_tag: Taf of the TLVs
 * rx_tlv: the pointer to the TLVs
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: true if the tlv is handled, false if not
 */
static inline bool
hal_rx_handle_other_tlvs(uint32_t tlv_tag, void *rx_tlv,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	uint32_t value;

	switch (tlv_tag) {
	case WIFIPHYRX_HE_SIG_A_MU_UL_E:
	{
		uint8_t *he_sig_a_mu_ul_info =
			(uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_A_MU_UL,
				      HE_SIG_A_MU_UL_INFO_PHYRX_HE_SIG_A_MU_UL_INFO_DETAILS);
		ppdu_info->rx_status.he_flags = 1;

		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO,
				   FORMAT_INDICATION);
		if (value == 0) {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
		} else {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_SU_FORMAT_TYPE;
		}

		/* data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_TXOP_KNOWN;

		/*data3*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info,
				   HE_SIG_A_MU_UL_INFO, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;
		/* 1 for UL and 0 for DL */
		value = 1;
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/*data4*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO,
				   SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/*data5*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info,
				   HE_SIG_A_MU_UL_INFO, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;

		/*data6*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO,
				   TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;
		return true;
	}
	default:
		return false;
	}
}
#else
static inline bool
hal_rx_handle_other_tlvs(uint32_t tlv_tag, void *rx_tlv,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	return false;
}
#endif /* QCA_WIFI_QCA6290_11AX_MU_UL && QCA_WIFI_QCA6290_11AX */

#if defined(RX_PPDU_END_USER_STATS_OFDMA_INFO_VALID_OFFSET) && \
defined(RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_OFFSET)

static inline void
hal_rx_handle_mu_ul_info(void *rx_tlv,
			 struct mon_rx_user_status *mon_rx_user_status)
{
	mon_rx_user_status->mu_ul_user_v0_word0 =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
			   SW_RESPONSE_REFERENCE_PTR);

	mon_rx_user_status->mu_ul_user_v0_word1 =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
			   SW_RESPONSE_REFERENCE_PTR_EXT);
}

static inline void
hal_rx_populate_byte_count(void *rx_tlv, void *ppduinfo,
			   struct mon_rx_user_status *mon_rx_user_status)
{
	uint32_t mpdu_ok_byte_count;
	uint32_t mpdu_err_byte_count;

	mpdu_ok_byte_count = HAL_RX_GET(rx_tlv,
					RX_PPDU_END_USER_STATS,
					MPDU_OK_BYTE_COUNT);
	mpdu_err_byte_count = HAL_RX_GET(rx_tlv,
					 RX_PPDU_END_USER_STATS,
					 MPDU_ERR_BYTE_COUNT);

	mon_rx_user_status->mpdu_ok_byte_count = mpdu_ok_byte_count;
	mon_rx_user_status->mpdu_err_byte_count = mpdu_err_byte_count;
}
#else
static inline void
hal_rx_handle_mu_ul_info(void *rx_tlv,
			 struct mon_rx_user_status *mon_rx_user_status)
{
}

static inline void
hal_rx_populate_byte_count(void *rx_tlv, void *ppduinfo,
			   struct mon_rx_user_status *mon_rx_user_status)
{
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	/* HKV1: doesn't support mpdu byte count */
	mon_rx_user_status->mpdu_ok_byte_count = ppdu_info->rx_status.ppdu_len;
	mon_rx_user_status->mpdu_err_byte_count = 0;
}
#endif

static inline void
hal_rx_populate_mu_user_info(void *rx_tlv, void *ppduinfo, uint32_t user_id,
			     struct mon_rx_user_status *mon_rx_user_status)
{
	struct mon_rx_info *mon_rx_info;
	struct mon_rx_user_info *mon_rx_user_info;
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	mon_rx_info = &ppdu_info->rx_info;
	mon_rx_user_info = &ppdu_info->rx_user_info[user_id];
	mon_rx_user_info->qos_control_info_valid =
		mon_rx_info->qos_control_info_valid;
	mon_rx_user_info->qos_control =  mon_rx_info->qos_control;

	mon_rx_user_status->ast_index = ppdu_info->rx_status.ast_index;
	mon_rx_user_status->tid = ppdu_info->rx_status.tid;
	mon_rx_user_status->tcp_msdu_count =
		ppdu_info->rx_status.tcp_msdu_count;
	mon_rx_user_status->udp_msdu_count =
		ppdu_info->rx_status.udp_msdu_count;
	mon_rx_user_status->other_msdu_count =
		ppdu_info->rx_status.other_msdu_count;
	mon_rx_user_status->frame_control = ppdu_info->rx_status.frame_control;
	mon_rx_user_status->frame_control_info_valid =
		ppdu_info->rx_status.frame_control_info_valid;
	mon_rx_user_status->data_sequence_control_info_valid =
		ppdu_info->rx_status.data_sequence_control_info_valid;
	mon_rx_user_status->first_data_seq_ctrl =
		ppdu_info->rx_status.first_data_seq_ctrl;
	mon_rx_user_status->preamble_type = ppdu_info->rx_status.preamble_type;
	mon_rx_user_status->ht_flags = ppdu_info->rx_status.ht_flags;
	mon_rx_user_status->rtap_flags = ppdu_info->rx_status.rtap_flags;
	mon_rx_user_status->vht_flags = ppdu_info->rx_status.vht_flags;
	mon_rx_user_status->he_flags = ppdu_info->rx_status.he_flags;
	mon_rx_user_status->rs_flags = ppdu_info->rx_status.rs_flags;

	mon_rx_user_status->mpdu_cnt_fcs_ok =
		ppdu_info->com_info.mpdu_cnt_fcs_ok;
	mon_rx_user_status->mpdu_cnt_fcs_err =
		ppdu_info->com_info.mpdu_cnt_fcs_err;
	qdf_mem_copy(&mon_rx_user_status->mpdu_fcs_ok_bitmap,
		     &ppdu_info->com_info.mpdu_fcs_ok_bitmap,
		     HAL_RX_NUM_WORDS_PER_PPDU_BITMAP *
		     sizeof(ppdu_info->com_info.mpdu_fcs_ok_bitmap[0]));

	hal_rx_populate_byte_count(rx_tlv, ppdu_info, mon_rx_user_status);
}

#define HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(chain, \
					ppdu_info, rssi_info_tlv) \
	{						\
	ppdu_info->rx_status.rssi_chain[chain][0] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_PRI20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][1] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][2] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT40_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][3] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT40_HIGH20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][4] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT80_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][5] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT80_LOW_HIGH20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][6] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT80_HIGH_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][7] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO,\
				   RSSI_EXT80_HIGH20_CHAIN##chain); \
	}						\

#define HAL_RX_PPDU_UPDATE_RSSI(ppdu_info, rssi_info_tlv) \
	{HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(0, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(1, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(2, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(3, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(4, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(5, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(6, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(7, ppdu_info, rssi_info_tlv)} \

static inline uint32_t
hal_rx_update_rssi_chain(struct hal_rx_ppdu_info *ppdu_info,
			 uint8_t *rssi_info_tlv)
{
	// TODO - Find all these registers for wcn7850
#if 0
	HAL_RX_PPDU_UPDATE_RSSI(ppdu_info, rssi_info_tlv)
#endif
	return 0;
}

#ifdef WLAN_TX_PKT_CAPTURE_ENH
static inline void
hal_get_qos_control(void *rx_tlv,
		    struct hal_rx_ppdu_info *ppdu_info)
{
	ppdu_info->rx_info.qos_control_info_valid =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
			   QOS_CONTROL_INFO_VALID);

	if (ppdu_info->rx_info.qos_control_info_valid)
		ppdu_info->rx_info.qos_control =
			HAL_RX_GET(rx_tlv,
				   RX_PPDU_END_USER_STATS,
				   QOS_CONTROL_FIELD);
}

static inline void
hal_get_mac_addr1(uint8_t *rx_mpdu_start,
		  struct hal_rx_ppdu_info *ppdu_info)
{
	if ((ppdu_info->sw_frame_group_id
	     == HAL_MPDU_SW_FRAME_GROUP_MGMT_PROBE_REQ) ||
	    (ppdu_info->sw_frame_group_id ==
	     HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS)) {
		ppdu_info->rx_info.mac_addr1_valid =
				HAL_RX_GET_MAC_ADDR1_VALID(rx_mpdu_start);

		*(uint32_t *)&ppdu_info->rx_info.mac_addr1[0] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO,
				   MAC_ADDR_AD1_31_0);
		if (ppdu_info->sw_frame_group_id ==
		    HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS) {
			*(uint32_t *)&ppdu_info->rx_info.mac_addr1[4] =
				HAL_RX_GET(rx_mpdu_start,
					   RX_MPDU_INFO,
					   MAC_ADDR_AD1_47_32);
		}
	}
}
#else
static inline void
hal_get_qos_control(void *rx_tlv,
		    struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline void
hal_get_mac_addr1(uint8_t *rx_mpdu_start,
		  struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

/**
 * hal_rx_status_get_tlv_info() - process receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: HAL_TLV_STATUS_PPDU_NOT_DONE or HAL_TLV_STATUS_PPDU_DONE from tlv
 */
static inline uint32_t
hal_rx_status_get_tlv_info_generic_be(void *rx_tlv_hdr, void *ppduinfo,
				      hal_soc_handle_t hal_soc_hdl,
				      qdf_nbuf_t nbuf)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;
	uint32_t tlv_tag, user_id, tlv_len, value;
	uint8_t group_id = 0;
	uint8_t he_dcm = 0;
	uint8_t he_stbc = 0;
	uint16_t he_gi = 0;
	uint16_t he_ltf = 0;
	void *rx_tlv;
	bool unhandled = false;
	struct mon_rx_user_status *mon_rx_user_status;
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv_hdr);
	user_id = HAL_RX_GET_USER_TLV32_USERID(rx_tlv_hdr);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv_hdr);

	rx_tlv = (uint8_t *)rx_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;

	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			   rx_tlv, tlv_len);

	switch (tlv_tag) {
	case WIFIRX_PPDU_START_E:
	{
		if (qdf_unlikely(ppdu_info->com_info.last_ppdu_id ==
		    HAL_RX_GET(rx_tlv, RX_PPDU_START, PHY_PPDU_ID)))
			hal_err("Matching ppdu_id(%u) detected",
				 ppdu_info->com_info.last_ppdu_id);

		/* Reset ppdu_info before processing the ppdu */
		qdf_mem_zero(ppdu_info,
			     sizeof(struct hal_rx_ppdu_info));

		ppdu_info->com_info.last_ppdu_id =
			ppdu_info->com_info.ppdu_id =
				HAL_RX_GET(rx_tlv, RX_PPDU_START,
					PHY_PPDU_ID);

		/* channel number is set in PHY meta data */
		ppdu_info->rx_status.chan_num =
			(HAL_RX_GET(rx_tlv, RX_PPDU_START,
				SW_PHY_META_DATA) & 0x0000FFFF);
		ppdu_info->rx_status.chan_freq =
			(HAL_RX_GET(rx_tlv, RX_PPDU_START,
				SW_PHY_META_DATA) & 0xFFFF0000) >> 16;
		if (ppdu_info->rx_status.chan_num &&
		    ppdu_info->rx_status.chan_freq) {
			ppdu_info->rx_status.chan_freq =
				hal_rx_radiotap_num_to_freq(
				ppdu_info->rx_status.chan_num,
				 ppdu_info->rx_status.chan_freq);
		}
#ifdef DP_BE_NOTYET_WAR
		// TODO -  timestamp is changed to 64-bit for wcn7850
		ppdu_info->com_info.ppdu_timestamp =
			HAL_RX_GET(rx_tlv, RX_PPDU_START,
				PPDU_START_TIMESTAMP);
#endif
		ppdu_info->rx_status.ppdu_timestamp =
			ppdu_info->com_info.ppdu_timestamp;
		ppdu_info->rx_state = HAL_RX_MON_PPDU_START;

		break;
	}

	case WIFIRX_PPDU_START_USER_INFO_E:
		break;

	case WIFIRX_PPDU_END_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] ppdu_end_e len=%d",
				__func__, __LINE__, tlv_len);
		/* This is followed by sub-TLVs of PPDU_END */
		ppdu_info->rx_state = HAL_RX_MON_PPDU_END;
		break;

	case WIFIPHYRX_PKT_END_E:
		hal_rx_get_rtt_info(hal_soc_hdl, rx_tlv, ppdu_info);
		break;

	case WIFIRXPCU_PPDU_END_INFO_E:
		ppdu_info->rx_status.rx_antenna =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO, RX_ANTENNA);
		ppdu_info->rx_status.tsft =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO,
				WB_TIMESTAMP_UPPER_32);
		ppdu_info->rx_status.tsft = (ppdu_info->rx_status.tsft << 32) |
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO,
				WB_TIMESTAMP_LOWER_32);
		ppdu_info->rx_status.duration =
			HAL_RX_GET(rx_tlv, UNIFIED_RXPCU_PPDU_END_INFO_8,
				RX_PPDU_DURATION);
		hal_rx_get_bb_info(hal_soc_hdl, rx_tlv, ppdu_info);
		break;

	/*
	 * WIFIRX_PPDU_END_USER_STATS_E comes for each user received.
	 * for MU, based on num users we see this tlv that many times.
	 */
	case WIFIRX_PPDU_END_USER_STATS_E:
	{
		unsigned long tid = 0;
		uint16_t seq = 0;

		ppdu_info->rx_status.ast_index =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
						AST_INDEX);

		tid = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
				RECEIVED_QOS_DATA_TID_BITMAP);
		ppdu_info->rx_status.tid = qdf_find_first_bit(&tid,
							      sizeof(tid) * 8);

		if (ppdu_info->rx_status.tid == (sizeof(tid) * 8))
			ppdu_info->rx_status.tid = HAL_TID_INVALID;

		ppdu_info->rx_status.tcp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					TCP_MSDU_COUNT) +
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					TCP_ACK_MSDU_COUNT);
		ppdu_info->rx_status.udp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
						UDP_MSDU_COUNT);
		ppdu_info->rx_status.other_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					OTHER_MSDU_COUNT);

		if (ppdu_info->sw_frame_group_id
		    != HAL_MPDU_SW_FRAME_GROUP_NULL_DATA) {
			ppdu_info->rx_status.frame_control_info_valid =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					   FRAME_CONTROL_INFO_VALID);

			if (ppdu_info->rx_status.frame_control_info_valid)
				ppdu_info->rx_status.frame_control =
					HAL_RX_GET(rx_tlv,
						   RX_PPDU_END_USER_STATS,
						   FRAME_CONTROL_FIELD);

			hal_get_qos_control(rx_tlv, ppdu_info);
		}

		ppdu_info->rx_status.data_sequence_control_info_valid =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
				   DATA_SEQUENCE_CONTROL_INFO_VALID);

		seq = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
				 FIRST_DATA_SEQ_CTRL);
		if (ppdu_info->rx_status.data_sequence_control_info_valid)
			ppdu_info->rx_status.first_data_seq_ctrl = seq;

		ppdu_info->rx_status.preamble_type =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
						HT_CONTROL_FIELD_PKT_TYPE);
		switch (ppdu_info->rx_status.preamble_type) {
		case HAL_RX_PKT_TYPE_11N:
			ppdu_info->rx_status.ht_flags = 1;
			ppdu_info->rx_status.rtap_flags |= HT_SGI_PRESENT;
			break;
		case HAL_RX_PKT_TYPE_11AC:
			ppdu_info->rx_status.vht_flags = 1;
			break;
		case HAL_RX_PKT_TYPE_11AX:
			ppdu_info->rx_status.he_flags = 1;
			break;
		default:
			break;
		}

		ppdu_info->com_info.mpdu_cnt_fcs_ok =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					MPDU_CNT_FCS_OK);
		ppdu_info->com_info.mpdu_cnt_fcs_err =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					MPDU_CNT_FCS_ERR);
		if ((ppdu_info->com_info.mpdu_cnt_fcs_ok |
			ppdu_info->com_info.mpdu_cnt_fcs_err) > 1)
			ppdu_info->rx_status.rs_flags |= IEEE80211_AMPDU_FLAG;
		else
			ppdu_info->rx_status.rs_flags &=
				(~IEEE80211_AMPDU_FLAG);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[0] =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					   FCS_OK_BITMAP_31_0);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[1] =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS,
					   FCS_OK_BITMAP_63_32);

		if (user_id < HAL_MAX_UL_MU_USERS) {
			mon_rx_user_status =
				&ppdu_info->rx_user_status[user_id];

			hal_rx_handle_mu_ul_info(rx_tlv, mon_rx_user_status);

			ppdu_info->com_info.num_users++;

			hal_rx_populate_mu_user_info(rx_tlv, ppdu_info,
						     user_id,
						     mon_rx_user_status);
		}
		break;
	}

	case WIFIRX_PPDU_END_USER_STATS_EXT_E:
		ppdu_info->com_info.mpdu_fcs_ok_bitmap[2] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				   FCS_OK_BITMAP_95_64);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[3] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				    FCS_OK_BITMAP_127_96);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[4] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				   FCS_OK_BITMAP_159_128);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[5] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				    FCS_OK_BITMAP_191_160);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[6] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				   FCS_OK_BITMAP_223_192);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[7] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT,
				    FCS_OK_BITMAP_255_224);
		break;

	case WIFIRX_PPDU_END_STATUS_DONE_E:
		return HAL_TLV_STATUS_PPDU_DONE;

	case WIFIDUMMY_E:
		return HAL_TLV_STATUS_BUF_DONE;

	case WIFIPHYRX_HT_SIG_E:
	{
		uint8_t *ht_sig_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_HT_SIG_0,
				HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS);
		value = HAL_RX_GET(ht_sig_info, HT_SIG_INFO,
				FEC_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		ppdu_info->rx_status.mcs = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO, MCS);
		ppdu_info->rx_status.ht_mcs = ppdu_info->rx_status.mcs;
		ppdu_info->rx_status.bw = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO, CBW);
		ppdu_info->rx_status.sgi = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO, SHORT_GI);
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		ppdu_info->rx_status.nss = ((ppdu_info->rx_status.mcs) >>
				HT_SIG_SU_NSS_SHIFT) + 1;
		ppdu_info->rx_status.mcs &= ((1 << HT_SIG_SU_NSS_SHIFT) - 1);
		break;
	}

	case WIFIPHYRX_L_SIG_B_E:
	{
		uint8_t *l_sig_b_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_L_SIG_B_0,
				L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_b_info, L_SIG_B_INFO, RATE);
		ppdu_info->rx_status.l_sig_b_info = *((uint32_t *)l_sig_b_info);
		switch (value) {
		case 1:
			ppdu_info->rx_status.rate = HAL_11B_RATE_3MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS3;
			break;
		case 2:
			ppdu_info->rx_status.rate = HAL_11B_RATE_2MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS2;
			break;
		case 3:
			ppdu_info->rx_status.rate = HAL_11B_RATE_1MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS1;
			break;
		case 4:
			ppdu_info->rx_status.rate = HAL_11B_RATE_0MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS0;
			break;
		case 5:
			ppdu_info->rx_status.rate = HAL_11B_RATE_6MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS6;
			break;
		case 6:
			ppdu_info->rx_status.rate = HAL_11B_RATE_5MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS5;
			break;
		case 7:
			ppdu_info->rx_status.rate = HAL_11B_RATE_4MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS4;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.cck_flag = 1;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
	break;
	}

	case WIFIPHYRX_L_SIG_A_E:
	{
		uint8_t *l_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_L_SIG_A_0,
				L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_a_info, L_SIG_A_INFO, RATE);
		ppdu_info->rx_status.l_sig_a_info = *((uint32_t *)l_sig_a_info);
		switch (value) {
		case 8:
			ppdu_info->rx_status.rate = HAL_11A_RATE_0MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS0;
			break;
		case 9:
			ppdu_info->rx_status.rate = HAL_11A_RATE_1MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS1;
			break;
		case 10:
			ppdu_info->rx_status.rate = HAL_11A_RATE_2MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS2;
			break;
		case 11:
			ppdu_info->rx_status.rate = HAL_11A_RATE_3MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS3;
			break;
		case 12:
			ppdu_info->rx_status.rate = HAL_11A_RATE_4MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS4;
			break;
		case 13:
			ppdu_info->rx_status.rate = HAL_11A_RATE_5MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS5;
			break;
		case 14:
			ppdu_info->rx_status.rate = HAL_11A_RATE_6MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS6;
			break;
		case 15:
			ppdu_info->rx_status.rate = HAL_11A_RATE_7MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS7;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.ofdm_flag = 1;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
	break;
	}

	case WIFIPHYRX_VHT_SIG_A_E:
	{
		uint8_t *vht_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_VHT_SIG_A_0,
				VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO,
				SU_MU_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		group_id = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO, GROUP_ID);
		ppdu_info->rx_status.vht_flag_values5 = group_id;
		ppdu_info->rx_status.mcs = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO, MCS);
		ppdu_info->rx_status.sgi = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO, GI_SETTING);

		switch (hal->target_type) {
		case TARGET_TYPE_QCA8074:
		case TARGET_TYPE_QCA8074V2:
		case TARGET_TYPE_QCA6018:
		case TARGET_TYPE_QCA5018:
		case TARGET_TYPE_QCN9000:
		case TARGET_TYPE_QCN6122:
#ifdef QCA_WIFI_QCA6390
		case TARGET_TYPE_QCA6390:
#endif
			ppdu_info->rx_status.is_stbc =
				HAL_RX_GET(vht_sig_a_info,
					   VHT_SIG_A_INFO, STBC);
			value =  HAL_RX_GET(vht_sig_a_info,
					    VHT_SIG_A_INFO, N_STS);
			value = value & VHT_SIG_SU_NSS_MASK;
			if (ppdu_info->rx_status.is_stbc && (value > 0))
				value = ((value + 1) >> 1) - 1;
			ppdu_info->rx_status.nss =
				((value & VHT_SIG_SU_NSS_MASK) + 1);

			break;
		case TARGET_TYPE_QCA6290:
#if !defined(QCA_WIFI_QCA6290_11AX)
			ppdu_info->rx_status.is_stbc =
				HAL_RX_GET(vht_sig_a_info,
					   VHT_SIG_A_INFO, STBC);
			value =  HAL_RX_GET(vht_sig_a_info,
					    VHT_SIG_A_INFO, N_STS);
			value = value & VHT_SIG_SU_NSS_MASK;
			if (ppdu_info->rx_status.is_stbc && (value > 0))
				value = ((value + 1) >> 1) - 1;
			ppdu_info->rx_status.nss =
				((value & VHT_SIG_SU_NSS_MASK) + 1);
#else
			ppdu_info->rx_status.nss = 0;
#endif
			break;
		case TARGET_TYPE_QCA6490:
		case TARGET_TYPE_QCA6750:
		case TARGET_TYPE_WCN7850:
			ppdu_info->rx_status.nss = 0;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.vht_flag_values3[0] =
				(((ppdu_info->rx_status.mcs) << 4)
				| ppdu_info->rx_status.nss);
		ppdu_info->rx_status.bw = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO, BANDWIDTH);
		ppdu_info->rx_status.vht_flag_values2 =
			ppdu_info->rx_status.bw;
		ppdu_info->rx_status.vht_flag_values4 =
			HAL_RX_GET(vht_sig_a_info,
				  VHT_SIG_A_INFO, SU_MU_CODING);

		ppdu_info->rx_status.beamformed = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO, BEAMFORMED);
		if (group_id == 0 || group_id == 63)
			ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		else
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_MIMO;

		break;
	}
	case WIFIPHYRX_HE_SIG_A_SU_E:
	{
		uint8_t *he_sig_a_su_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_A_SU_0,
			HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS);
		ppdu_info->rx_status.he_flags = 1;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
			FORMAT_INDICATION);
		if (value == 0) {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
		} else {
			ppdu_info->rx_status.he_data1 =
				 QDF_MON_STATUS_HE_SU_FORMAT_TYPE;
		}

		/* data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_BEAM_CHANGE_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_DCM_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN |
			QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 =
			QDF_MON_STATUS_HE_GI_KNOWN;
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_TXBF_KNOWN |
			QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN |
			QDF_MON_STATUS_TXOP_KNOWN |
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN |
			QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN |
			QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN;

		/* data3 */
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, BEAM_CHANGE);
		value = value << QDF_MON_STATUS_BEAM_CHANGE_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, DL_UL_FLAG);
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, TRANSMIT_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, DCM);
		he_dcm = value;
		value = value << QDF_MON_STATUS_DCM_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO,
				LDPC_EXTRA_SYMBOL);
		value = value << QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, STBC);
		he_stbc = value;
		value = value << QDF_MON_STATUS_STBC_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* data4 */
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
							SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/* data5 */
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO, CP_LTF_SIZE);
		switch (value) {
		case 0:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_1_X;
				break;
		case 1:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_2_X;
				break;
		case 2:
				he_gi = HE_GI_1_6;
				he_ltf = HE_LTF_2_X;
				break;
		case 3:
				if (he_dcm && he_stbc) {
					he_gi = HE_GI_0_8;
					he_ltf = HE_LTF_4_X;
				} else {
					he_gi = HE_GI_3_2;
					he_ltf = HE_LTF_4_X;
				}
				break;
		}
		ppdu_info->rx_status.sgi = he_gi;
		ppdu_info->rx_status.ltf_size = he_ltf;
		hal_get_radiotap_he_gi_ltf(&he_gi, &he_ltf);
		value = he_gi << QDF_MON_STATUS_GI_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = he_ltf << QDF_MON_STATUS_HE_LTF_SIZE_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO, NSTS);
		value = (value << QDF_MON_STATUS_HE_LTF_SYM_SHIFT);
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
						PACKET_EXTENSION_A_FACTOR);
		value = value << QDF_MON_STATUS_PRE_FEC_PAD_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO, TXBF);
		value = value << QDF_MON_STATUS_TXBF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
					PACKET_EXTENSION_PE_DISAMBIGUITY);
		value = value << QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/* data6 */
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO, NSTS);
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 = value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
							DOPPLER_INDICATION);
		value = value << QDF_MON_STATUS_DOPPLER_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO,
							TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		ppdu_info->rx_status.beamformed = HAL_RX_GET(he_sig_a_su_info,
					HE_SIG_A_SU_INFO, TXBF);
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		break;
	}
	case WIFIPHYRX_HE_SIG_A_MU_DL_E:
	{
		uint8_t *he_sig_a_mu_dl_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_A_MU_DL_0,
			HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS);

		ppdu_info->rx_status.he_mu_flags = 1;

		/* HE Flags */
		/*data1*/
		ppdu_info->rx_status.he_data1 =
					QDF_MON_STATUS_HE_MU_FORMAT_TYPE;
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 =
			QDF_MON_STATUS_HE_GI_KNOWN;
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN |
			QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN |
			QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN |
			QDF_MON_STATUS_TXOP_KNOWN |
			QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN;

		/*data3*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, DL_UL_FLAG);
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO,
				LDPC_EXTRA_SYMBOL);
		value = value << QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, STBC);
		he_stbc = value;
		value = value << QDF_MON_STATUS_STBC_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/*data4*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO,
							SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/*data5*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, CP_LTF_SIZE);
		switch (value) {
		case 0:
			he_gi = HE_GI_0_8;
			he_ltf = HE_LTF_4_X;
			break;
		case 1:
			he_gi = HE_GI_0_8;
			he_ltf = HE_LTF_2_X;
			break;
		case 2:
			he_gi = HE_GI_1_6;
			he_ltf = HE_LTF_2_X;
			break;
		case 3:
			he_gi = HE_GI_3_2;
			he_ltf = HE_LTF_4_X;
			break;
		}
		ppdu_info->rx_status.sgi = he_gi;
		ppdu_info->rx_status.ltf_size = he_ltf;
		hal_get_radiotap_he_gi_ltf(&he_gi, &he_ltf);
		value = he_gi << QDF_MON_STATUS_GI_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = he_ltf << QDF_MON_STATUS_HE_LTF_SIZE_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				   HE_SIG_A_MU_DL_INFO, NUM_LTF_SYMBOLS);
		value = (value << QDF_MON_STATUS_HE_LTF_SYM_SHIFT);
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO,
				   PACKET_EXTENSION_A_FACTOR);
		value = value << QDF_MON_STATUS_PRE_FEC_PAD_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO,
				   PACKET_EXTENSION_PE_DISAMBIGUITY);
		value = value << QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/*data6*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO,
							DOPPLER_INDICATION);
		value = value << QDF_MON_STATUS_DOPPLER_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO,
							TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		/* HE-MU Flags */
		/* HE-MU-flags1 */
		ppdu_info->rx_status.he_flags1 =
			QDF_MON_STATUS_SIG_B_MCS_KNOWN |
			QDF_MON_STATUS_SIG_B_DCM_KNOWN |
			QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_1_KNOWN |
			QDF_MON_STATUS_SIG_B_SYM_NUM_KNOWN |
			QDF_MON_STATUS_RU_0_KNOWN;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, MCS_OF_SIG_B);
		ppdu_info->rx_status.he_flags1 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, DCM_OF_SIG_B);
		value = value << QDF_MON_STATUS_DCM_FLAG_1_SHIFT;
		ppdu_info->rx_status.he_flags1 |= value;

		/* HE-MU-flags2 */
		ppdu_info->rx_status.he_flags2 =
			QDF_MON_STATUS_BW_KNOWN;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, TRANSMIT_BW);
		ppdu_info->rx_status.he_flags2 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, COMP_MODE_SIG_B);
		value = value << QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_2_SHIFT;
		ppdu_info->rx_status.he_flags2 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO, NUM_SIG_B_SYMBOLS);
		value = value - 1;
		value = value << QDF_MON_STATUS_NUM_SIG_B_SYMBOLS_SHIFT;
		ppdu_info->rx_status.he_flags2 |= value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_MIMO;
		break;
	}
	case WIFIPHYRX_HE_SIG_B1_MU_E:
	{
		uint8_t *he_sig_b1_mu_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B1_MU_0,
			HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS);

		ppdu_info->rx_status.he_sig_b_common_known |=
			QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0;
		/* TODO: Check on the availability of other fields in
		 * sig_b_common
		 */

		value = HAL_RX_GET(he_sig_b1_mu_info,
				HE_SIG_B1_MU_INFO, RU_ALLOCATION);
		ppdu_info->rx_status.he_RU[0] = value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_MIMO;
		break;
	}
	case WIFIPHYRX_HE_SIG_B2_MU_E:
	{
		uint8_t *he_sig_b2_mu_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B2_MU_0,
			HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS);
		/*
		 * Not all "HE" fields can be updated from
		 * WIFIPHYRX_HE_SIG_A_MU_DL_E TLV. Use WIFIPHYRX_HE_SIG_B2_MU_E
		 * to populate rest of the "HE" fields for MU scenarios.
		 */

		/* HE-data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN;

		/* HE-data2 */

		/* HE-data3 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO, STA_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO, STA_CODING);
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* HE-data4 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO, STA_ID);
		value = value << QDF_MON_STATUS_STA_ID_SHIFT;
		ppdu_info->rx_status.he_data4 |= value;

		/* HE-data5 */

		/* HE-data6 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				   HE_SIG_B2_MU_INFO, NSTS);
		/* value n indicates n+1 spatial streams */
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 |= value;

		break;
	}
	case WIFIPHYRX_HE_SIG_B2_OFDMA_E:
	{
		uint8_t *he_sig_b2_ofdma_info =
		(uint8_t *)rx_tlv +
		HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0,
		HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS);

		/*
		 * Not all "HE" fields can be updated from
		 * WIFIPHYRX_HE_SIG_A_MU_DL_E TLV. Use WIFIPHYRX_HE_SIG_B2_MU_E
		 * to populate rest of "HE" fields for MU OFDMA scenarios.
		 */

		/* HE-data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_DCM_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN;

		/* HE-data2 */
		ppdu_info->rx_status.he_data2 |=
					QDF_MON_STATUS_TXBF_KNOWN;

		/* HE-data3 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO, STA_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO, STA_DCM);
		he_dcm = value;
		value = value << QDF_MON_STATUS_DCM_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO, STA_CODING);
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* HE-data4 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO, STA_ID);
		value = value << QDF_MON_STATUS_STA_ID_SHIFT;
		ppdu_info->rx_status.he_data4 |= value;

		/* HE-data5 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				   HE_SIG_B2_OFDMA_INFO, TXBF);
		value = value << QDF_MON_STATUS_TXBF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/* HE-data6 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				   HE_SIG_B2_OFDMA_INFO, NSTS);
		/* value n indicates n+1 spatial streams */
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 |= value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_OFDMA;
		break;
	}
	case WIFIPHYRX_RSSI_LEGACY_E:
	{
		uint8_t reception_type;
		int8_t rssi_value;
		uint8_t *rssi_info_tlv = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_RSSI_LEGACY_19,
				RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS);

		ppdu_info->rx_status.rssi_comb = HAL_RX_GET(rx_tlv,
			PHYRX_RSSI_LEGACY, RSSI_COMB);
		ppdu_info->rx_status.bw = hal->ops->hal_rx_get_tlv(rx_tlv);
		ppdu_info->rx_status.he_re = 0;

		reception_type = HAL_RX_GET(rx_tlv,
					    PHYRX_RSSI_LEGACY,
					    RECEPTION_TYPE);
		switch (reception_type) {
		case QDF_RECEPTION_TYPE_ULOFMDA:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_OFDMA;
			ppdu_info->rx_status.ulofdma_flag = 1;
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
			break;
		case QDF_RECEPTION_TYPE_ULMIMO:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_MIMO;
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_MU_FORMAT_TYPE;
			break;
		default:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_SU;
			break;
		}
		hal_rx_update_rssi_chain(ppdu_info, rssi_info_tlv);
		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO, RSSI_PRI20_CHAIN0);
		ppdu_info->rx_status.rssi[0] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN0: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO, RSSI_PRI20_CHAIN1);
		ppdu_info->rx_status.rssi[1] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN1: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO, RSSI_PRI20_CHAIN2);
		ppdu_info->rx_status.rssi[2] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN2: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO, RSSI_PRI20_CHAIN3);
		ppdu_info->rx_status.rssi[3] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN3: %d\n", rssi_value);

#ifdef DP_BE_NOTYET_WAR
		// TODO - this is not preset for wcn7850
		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO, RSSI_PRI20_CHAIN4);
		ppdu_info->rx_status.rssi[4] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN4: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO,
					RSSI_PRI20_CHAIN5);
		ppdu_info->rx_status.rssi[5] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN5: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO,
					RSSI_PRI20_CHAIN6);
		ppdu_info->rx_status.rssi[6] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN6: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO,
					RSSI_PRI20_CHAIN7);
		ppdu_info->rx_status.rssi[7] = rssi_value;
#endif
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN7: %d\n", rssi_value);
		break;
	}
	case WIFIPHYRX_OTHER_RECEIVE_INFO_E:
		hal_rx_proc_phyrx_other_receive_info_tlv(hal, rx_tlv_hdr,
								ppdu_info);
		break;
	case WIFIRX_HEADER_E:
	{
		struct hal_rx_ppdu_common_info *com_info = &ppdu_info->com_info;

		if (ppdu_info->fcs_ok_cnt >=
		    HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER) {
			hal_err("Number of MPDUs(%d) per status buff exceeded",
				ppdu_info->fcs_ok_cnt);
			break;
		}

		/* Update first_msdu_payload for every mpdu and increment
		 * com_info->mpdu_cnt for every WIFIRX_HEADER_E TLV
		 */
		ppdu_info->ppdu_msdu_info[ppdu_info->fcs_ok_cnt].first_msdu_payload =
			rx_tlv;
		ppdu_info->ppdu_msdu_info[ppdu_info->fcs_ok_cnt].payload_len = tlv_len;
		ppdu_info->msdu_info.first_msdu_payload = rx_tlv;
		ppdu_info->msdu_info.payload_len = tlv_len;
		ppdu_info->user_id = user_id;
		ppdu_info->hdr_len = tlv_len;
		ppdu_info->data = rx_tlv;
		ppdu_info->data += 4;

		/* for every RX_HEADER TLV increment mpdu_cnt */
		com_info->mpdu_cnt++;
		return HAL_TLV_STATUS_HEADER;
	}
	case WIFIRX_MPDU_START_E:
	{
		uint8_t *rx_mpdu_start = (uint8_t *)rx_tlv;
		uint32_t ppdu_id = HAL_RX_GET_PPDU_ID(rx_tlv);
		uint8_t filter_category = 0;

		ppdu_info->nac_info.fc_valid =
				HAL_RX_GET_FC_VALID(rx_tlv);

		ppdu_info->nac_info.to_ds_flag =
				HAL_RX_GET_TO_DS_FLAG(rx_tlv);

		ppdu_info->nac_info.frame_control =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO,
				   MPDU_FRAME_CONTROL_FIELD);

		ppdu_info->sw_frame_group_id =
			HAL_RX_GET_SW_FRAME_GROUP_ID(rx_tlv);

		if (ppdu_info->sw_frame_group_id ==
		    HAL_MPDU_SW_FRAME_GROUP_NULL_DATA) {
			ppdu_info->rx_status.frame_control_info_valid =
				ppdu_info->nac_info.fc_valid;
			ppdu_info->rx_status.frame_control =
				ppdu_info->nac_info.frame_control;
		}

		hal_get_mac_addr1(rx_mpdu_start,
				  ppdu_info);

		ppdu_info->nac_info.mac_addr2_valid =
				HAL_RX_TLV_MPDU_MAC_ADDR_AD2_VALID_GET(rx_tlv);

		*(uint16_t *)&ppdu_info->nac_info.mac_addr2[0] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO,
				   MAC_ADDR_AD2_15_0);

		*(uint32_t *)&ppdu_info->nac_info.mac_addr2[2] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO,
				   MAC_ADDR_AD2_47_16);

		if (ppdu_info->rx_status.prev_ppdu_id != ppdu_id) {
			ppdu_info->rx_status.prev_ppdu_id = ppdu_id;
			ppdu_info->rx_status.ppdu_len =
				HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO,
					   MPDU_LENGTH);
		} else {
			ppdu_info->rx_status.ppdu_len +=
				HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO,
					   MPDU_LENGTH);
		}

		filter_category =
				HAL_RX_GET_FILTER_CATEGORY(rx_tlv);

		if (filter_category == 0)
			ppdu_info->rx_status.rxpcu_filter_pass = 1;
		else if (filter_category == 1)
			ppdu_info->rx_status.monitor_direct_used = 1;

		ppdu_info->nac_info.mcast_bcast =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO,
				   MCAST_BCAST);
		break;
	}
	case WIFIRX_MPDU_END_E:
		ppdu_info->user_id = user_id;
		ppdu_info->fcs_err =
			HAL_RX_GET(rx_tlv, RX_MPDU_END,
				   FCS_ERR);
		return HAL_TLV_STATUS_MPDU_END;
	case WIFIRX_MSDU_END_E:
		if (user_id < HAL_MAX_UL_MU_USERS) {
			ppdu_info->rx_msdu_info[user_id].cce_metadata =
				HAL_RX_TLV_CCE_METADATA_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].fse_metadata =
				HAL_RX_TLV_FSE_METADATA_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].is_flow_idx_timeout =
				HAL_RX_TLV_FLOW_IDX_TIMEOUT_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].is_flow_idx_invalid =
				HAL_RX_TLV_FLOW_IDX_INVALID_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].flow_idx =
				HAL_RX_TLV_FLOW_IDX_GET(rx_tlv);
		}
		return HAL_TLV_STATUS_MSDU_END;
	case 0:
		return HAL_TLV_STATUS_PPDU_DONE;

	default:
		if (hal_rx_handle_other_tlvs(tlv_tag, rx_tlv, ppdu_info))
			unhandled = false;
		else
			unhandled = true;
		break;
	}

	if (!unhandled)
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s TLV type: %d, TLV len:%d %s",
			  __func__, tlv_tag, tlv_len,
			  unhandled == true ? "unhandled" : "");

	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				rx_tlv, tlv_len);

	return HAL_TLV_STATUS_PPDU_NOT_DONE;
}

/**
 * hal_tx_set_pcp_tid_map_generic_be() - Configure default PCP to TID map table
 * @soc: HAL SoC context
 * @map: PCP-TID mapping table
 *
 * PCP are mapped to 8 TID values using TID values programmed
 * in one set of mapping registers PCP_TID_MAP_<0 to 6>
 * The mapping register has TID mapping for 8 PCP values
 *
 * Return: none
 */
static void hal_tx_set_pcp_tid_map_generic_be(struct hal_soc *soc, uint8_t *map)
{
	uint32_t addr, value;

	addr = HWIO_TCL_R0_PCP_TID_MAP_ADDR(
				MAC_TCL_REG_REG_BASE);

	value = (map[0] |
		(map[1] << HWIO_TCL_R0_PCP_TID_MAP_PCP_1_SHFT) |
		(map[2] << HWIO_TCL_R0_PCP_TID_MAP_PCP_2_SHFT) |
		(map[3] << HWIO_TCL_R0_PCP_TID_MAP_PCP_3_SHFT) |
		(map[4] << HWIO_TCL_R0_PCP_TID_MAP_PCP_4_SHFT) |
		(map[5] << HWIO_TCL_R0_PCP_TID_MAP_PCP_5_SHFT) |
		(map[6] << HWIO_TCL_R0_PCP_TID_MAP_PCP_6_SHFT) |
		(map[7] << HWIO_TCL_R0_PCP_TID_MAP_PCP_7_SHFT));

	HAL_REG_WRITE(soc, addr, (value & HWIO_TCL_R0_PCP_TID_MAP_RMSK));
}

/**
 * hal_tx_update_pcp_tid_generic_be() - Update the pcp tid map table with
 *					value received from user-space
 * @soc: HAL SoC context
 * @pcp: pcp value
 * @tid : tid value
 *
 * Return: void
 */
static void
hal_tx_update_pcp_tid_generic_be(struct hal_soc *soc,
				 uint8_t pcp, uint8_t tid)
{
	uint32_t addr, value, regval;

	addr = HWIO_TCL_R0_PCP_TID_MAP_ADDR(
				MAC_TCL_REG_REG_BASE);

	value = (uint32_t)tid << (HAL_TX_BITS_PER_TID * pcp);

	/* Read back previous PCP TID config and update
	 * with new config.
	 */
	regval = HAL_REG_READ(soc, addr);
	regval &= ~(HAL_TX_TID_BITS_MASK << (HAL_TX_BITS_PER_TID * pcp));
	regval |= value;

	HAL_REG_WRITE(soc, addr,
		      (regval & HWIO_TCL_R0_PCP_TID_MAP_RMSK));
}

/**
 * hal_tx_update_tidmap_prty_generic_be() - Update the tid map priority
 * @soc: HAL SoC context
 * @val: priority value
 *
 * Return: void
 */
static
void hal_tx_update_tidmap_prty_generic_be(struct hal_soc *soc, uint8_t value)
{
	uint32_t addr;

	addr = HWIO_TCL_R0_TID_MAP_PRTY_ADDR(
				MAC_TCL_REG_REG_BASE);

	HAL_REG_WRITE(soc, addr,
		      (value & HWIO_TCL_R0_TID_MAP_PRTY_RMSK));
}

/**
 * hal_rx_get_tlv_size_generic_be() - Get rx packet tlv size
 * @rx_pkt_tlv_size: TLV size for regular RX packets
 * @rx_mon_pkt_tlv_size: TLV size for monitor mode packets
 *
 * Return: size of rx pkt tlv before the actual data
 */
static void hal_rx_get_tlv_size_generic_be(uint16_t *rx_pkt_tlv_size,
					   uint16_t *rx_mon_pkt_tlv_size)
{
	*rx_pkt_tlv_size = RX_PKT_TLVS_LEN;
	/* For now mon pkt tlv is same as rx pkt tlv */
	*rx_mon_pkt_tlv_size = RX_PKT_TLVS_LEN;
}
#endif /* _HAL_BE_GENERIC_API_H_ */
