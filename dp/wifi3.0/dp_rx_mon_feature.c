/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "hal_api_mon.h"
#include "dp_rx_mon.h"
#include "dp_internal.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "wlan_cfg.h"
#include "dp_htt.h"
#include "dp_mon_filter.h"

#ifdef WLAN_RX_PKT_CAPTURE_ENH

static inline void
dp_rx_free_msdu_list(struct msdu_list *msdu_list)
{
	qdf_nbuf_list_free(msdu_list->head);
	msdu_list->head = NULL;
	msdu_list->tail = NULL;
	msdu_list->sum_len = 0;
}

/**
 * dp_nbuf_set_data_and_len() - set nbuf data and len
 * @buf: Network buf instance
 * @data: pointer to nbuf data
 * @len: nbuf data length
 *
 * Return: none
 */
static inline void
dp_nbuf_set_data_and_len(qdf_nbuf_t buf, unsigned char *data,
			 int len)
{
	qdf_nbuf_set_data_pointer(buf, data);
	qdf_nbuf_set_len(buf, len);
	qdf_nbuf_set_tail_pointer(buf, len);
}

/*
 * dp_rx_populate_cdp_indication_mpdu_info() - Populate cdp rx indication
 * MPDU info structure
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from monitor status ring
 * @cdp_mpdu_info: cdp rx indication MPDU info structure
 * @user: user ID
 *
 * Return: none
 */
void
dp_rx_populate_cdp_indication_mpdu_info(
	struct dp_pdev *pdev,
	struct hal_rx_ppdu_info *ppdu_info,
	struct cdp_rx_indication_mpdu_info *cdp_mpdu_info,
	uint32_t user)
{
	int i;
	struct mon_rx_user_status *rx_user_status;

	cdp_mpdu_info->ppdu_id = ppdu_info->com_info.ppdu_id;
	cdp_mpdu_info->channel = ppdu_info->rx_status.chan_num;
	cdp_mpdu_info->chan_freq = ppdu_info->rx_status.chan_freq;
	cdp_mpdu_info->duration = ppdu_info->rx_status.duration;
	cdp_mpdu_info->timestamp = ppdu_info->rx_status.tsft;
	cdp_mpdu_info->bw = ppdu_info->rx_status.bw;
	if ((ppdu_info->rx_status.sgi == VHT_SGI_NYSM) &&
	    (ppdu_info->rx_status.preamble_type == HAL_RX_PKT_TYPE_11AC))
		cdp_mpdu_info->gi = CDP_SGI_0_4_US;
	else
		cdp_mpdu_info->gi = ppdu_info->rx_status.sgi;
	cdp_mpdu_info->ldpc = ppdu_info->rx_status.ldpc;
	cdp_mpdu_info->preamble = ppdu_info->rx_status.preamble_type;
	cdp_mpdu_info->ppdu_type = ppdu_info->rx_status.reception_type;
	cdp_mpdu_info->rssi_comb = ppdu_info->rx_status.rssi_comb;
	cdp_mpdu_info->nf = ppdu_info->rx_status.chan_noise_floor;

	if (ppdu_info->rx_status.reception_type == HAL_RX_TYPE_MU_OFDMA) {
		rx_user_status =  &ppdu_info->rx_user_status[user];
		cdp_mpdu_info->nss = rx_user_status->nss;
		cdp_mpdu_info->mcs = rx_user_status->mcs;
		cdp_mpdu_info->mu_ul_info_valid =
				rx_user_status->mu_ul_info_valid;
		cdp_mpdu_info->ofdma_ru_start_index =
				rx_user_status->ofdma_ru_start_index;
		cdp_mpdu_info->ofdma_ru_width =
				rx_user_status->ofdma_ru_width;

	} else {
		cdp_mpdu_info->nss = ppdu_info->rx_status.nss;
		cdp_mpdu_info->mcs = ppdu_info->rx_status.mcs;
	}
	cdp_mpdu_info->rate = ppdu_info->rx_status.rate;
	for (i = 0; i < MAX_CHAIN; i++)
		cdp_mpdu_info->per_chain_rssi[i] = ppdu_info->rx_status.rssi[i];
}

#ifdef WLAN_SUPPORT_RX_FLOW_TAG
/**
 * dp_rx_mon_enh_capture_set_flow_tag() - Tags the actual nbuf with
 * cached flow tag data read from TLV
 * @pdev: pdev structure
 * @ppdu_info: ppdu info structure from monitor status ring
 * @user_id: user ID on which the PPDU is received
 * @nbuf: packet buffer on which metadata have to be updated
 *
 * Return: None
 */
void dp_rx_mon_enh_capture_set_flow_tag(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					uint32_t user_id, qdf_nbuf_t nbuf)
{
	struct dp_soc *soc = pdev->soc;
	uint16_t fse_metadata;

	if (user_id >= MAX_MU_USERS)
		return;

	if (qdf_likely(!wlan_cfg_is_rx_flow_tag_enabled(soc->wlan_cfg_ctx)))
		return;

	if (ppdu_info->rx_msdu_info[user_id].is_flow_idx_invalid)
		return;

	if (ppdu_info->rx_msdu_info[user_id].is_flow_idx_timeout)
		return;

	fse_metadata =
	  (uint16_t)ppdu_info->rx_msdu_info[user_id].fse_metadata & 0xFFFF;

	/* update the skb->cb with the user-specified tag/metadata */
	qdf_nbuf_set_rx_flow_tag(nbuf, fse_metadata);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "Setting flow tag %u for userID %u", fse_metadata, user_id);

	ppdu_info->rx_msdu_info[user_id].fse_metadata = 0;
	ppdu_info->rx_msdu_info[user_id].flow_idx = 0;
	ppdu_info->rx_msdu_info[user_id].is_flow_idx_timeout = false;
	ppdu_info->rx_msdu_info[user_id].is_flow_idx_invalid = false;
}

/**
 * dp_rx_mon_enh_capture_set_flow_tag_in_trailer - update msdu trailer
 *                                                 with flow tag
 * @nbuf: packet buffer on which metadata have to be updated
 * @trailer: pointer to rx monitor-lite trailer
 *
 * Return: None
 */
static inline void dp_rx_mon_enh_capture_set_flow_tag_in_trailer(
					qdf_nbuf_t nbuf, void *trailer)
{
	uint16_t flow_tag = qdf_nbuf_get_rx_flow_tag(nbuf);
	struct dp_rx_mon_enh_trailer_data *nbuf_trailer =
			(struct dp_rx_mon_enh_trailer_data *)trailer;

	if (!flow_tag)
		return;

	nbuf_trailer->flow_tag = flow_tag;
}
#else
void dp_rx_mon_enh_capture_set_flow_tag(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					uint32_t user_id, qdf_nbuf_t nbuf)
{
}

static inline void dp_rx_mon_enh_capture_set_flow_tag_in_trailer(
					qdf_nbuf_t nbuf, void *trailer)
{
}
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/*
 * dp_rx_mon_enh_capture_set_protocol_tag() - Tags the actual nbuf with
 * cached protocol tag data read from TLV
 * @pdev: pdev structure
 * @ppdu_info: ppdu info structure from monitor status ring
 * @user_id: user ID on which the PPDU is received
 * @nbuf: packet buffer on which metadata have to be updated
 *
 * Return: none
 */
static void
dp_rx_mon_enh_capture_set_protocol_tag(struct dp_pdev *pdev,
				       struct hal_rx_ppdu_info *ppdu_info,
				       uint32_t user_id,
				       qdf_nbuf_t nbuf)
{
	uint32_t cce_metadata = 0;
	uint16_t protocol_tag = 0;

	if (user_id >= MAX_MU_USERS)
		return;

	/**
	 * Since skb->cb is memset to 0, we can skip setting protocol tag to 0
	 * in all the error paths.
	 */

	cce_metadata = ppdu_info->rx_msdu_info[user_id].cce_metadata;

	/**
	 * Received CCE metadata should be
	 * within the valid limits
	 */
	if (qdf_unlikely((cce_metadata < RX_PROTOCOL_TAG_START_OFFSET) ||
			 (cce_metadata >= (RX_PROTOCOL_TAG_START_OFFSET
					   + RX_PROTOCOL_TAG_MAX))))
		return;

	/**
	 * The CCE metadata received is just the
	 * packet_type + RX_PROTOCOL_TAG_START_OFFSET
	 */
	cce_metadata -= RX_PROTOCOL_TAG_START_OFFSET;

	/**
	 * Update the QDF packet with the user specified tag/metadata
	 * by looking up tag value for received protocol type.
	 */
	protocol_tag = pdev->rx_proto_tag_map[cce_metadata].tag;
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "Setting ProtoID:%d Tag %u in mon nbuf",
		  cce_metadata, protocol_tag);
	qdf_nbuf_set_rx_protocol_tag(nbuf, protocol_tag);
}

/*
 * dp_rx_mon_enh_capture_tag_protocol_type() - Support protocol tagging
 * for packets captured in enhanced capture mode
 * @pdev: pdev structure
 * @ppdu_info: ppdu info structure from monitor status ring
 * @user_id: user ID on which the PPDU is received
 * @nbuf: packet buffer on which tag should be updated
 *
 * Return: none
 */
static void
dp_rx_mon_enh_capture_tag_protocol_type(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					uint32_t user_id, qdf_nbuf_t nbuf)
{
	/**
	 * Since skb->cb is memset to 0, we can skip setting protocol tag to 0
	 * in all the error paths.
	 */
	if (!pdev->is_rx_protocol_tagging_enabled)
		return;

	/**
	 * It is assumed that we have already received RX Header/ MSDU
	 * Start TLV for this MSDU.
	 */
	dp_rx_mon_enh_capture_set_protocol_tag(pdev, ppdu_info,
					       user_id, nbuf);

	/* Reset MSDU tag variables on completion of every MSDU tag */
	ppdu_info->rx_msdu_info[user_id].cce_metadata = 0;
}

/*
 * dp_rx_mon_enh_capture_set_protocol_tag_in_trailer - update msdu trailer
 *                                                     with protocol tag
 * @nbuf: packet buffer on which metadata have to be updated
 * @trailer: pointer to rx monitor-lite trailer
 *
 * Return: void
 */
static inline
void dp_rx_mon_enh_capture_set_protocol_tag_in_trailer(qdf_nbuf_t nbuf,
						       void *trailer)
{
	uint16_t protocol_tag = qdf_nbuf_get_rx_protocol_tag(nbuf);
	struct dp_rx_mon_enh_trailer_data *nbuf_trailer =
			(struct dp_rx_mon_enh_trailer_data *)trailer;

	if (protocol_tag != 0)
		nbuf_trailer->protocol_tag = protocol_tag;
}

#else
static void
dp_rx_mon_enh_capture_tag_protocol_type(struct dp_pdev *pdev,
					struct hal_rx_ppdu_info *ppdu_info,
					uint32_t user_id, qdf_nbuf_t nbuf)
{
}

static void
dp_rx_mon_enh_capture_set_protocol_tag(struct dp_pdev *pdev,
				       struct hal_rx_ppdu_info *ppdu_info,
				       uint32_t user_id, qdf_nbuf_t nbuf)
{
}

static inline
void dp_rx_mon_enh_capture_set_protocol_tag_in_trailer(qdf_nbuf_t nbuf,
						       void *trailer)
{
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

/*
 * dp_rx_mon_enh_capture_update_trailer() - Update trailer with custom data
 * @pdev: pdev structure
 * @nbuf: packet buffer on which metadata have to be updated
 *
 * Return: return number of bytes updated in the tail
 */
static inline
uint16_t dp_rx_mon_enh_capture_update_trailer(struct dp_pdev *pdev,
					      qdf_nbuf_t nbuf)
{
	uint64_t trailer;
	uint8_t  *dest;
	struct dp_soc *soc = pdev->soc;
	struct dp_rx_mon_enh_trailer_data *nbuf_trailer =
			(struct dp_rx_mon_enh_trailer_data *)&trailer;

	if (qdf_unlikely(qdf_nbuf_len(nbuf) < sizeof(trailer)))
		return 0;

	trailer = RX_MON_CAP_ENH_TRAILER;

	if (wlan_cfg_is_rx_mon_protocol_flow_tag_enabled(soc->wlan_cfg_ctx)) {
		dp_rx_mon_enh_capture_set_protocol_tag_in_trailer(nbuf,
								  nbuf_trailer);
		dp_rx_mon_enh_capture_set_flow_tag_in_trailer(nbuf,
							      nbuf_trailer);
	}

	/**
	 * Overwrite last 8 bytes of data with trailer. This is ok since we
	 * do not care about the data in this debug mode.
	 */
	qdf_nbuf_trim_tail(nbuf, sizeof(trailer));
	dest = qdf_nbuf_put_tail(nbuf, sizeof(trailer));
	if (qdf_likely(dest)) {
		qdf_mem_copy(dest, &trailer, sizeof(trailer));
	} else {
		dp_err("Unable to add tail room");
		return 0;
	}

	return sizeof(trailer);
}

/**
 * dp_rx_enh_capture_is_peer_enabled() - Is peer based enh capture enabled.
 * @soc: core txrx main context
 * @ppdu_info: Structure for rx ppdu info
 * @user_id: user id for MU Rx packet
 *
 * Return: none
 */
static inline bool
dp_rx_enh_capture_is_peer_enabled(struct dp_soc *soc,
				  struct hal_rx_ppdu_info *ppdu_info,
				  uint32_t user_id)
{
	struct dp_peer *peer;
	struct dp_ast_entry *ast_entry;
	uint32_t ast_index;

	ast_index = ppdu_info->rx_user_status[user_id].ast_index;
	if (ast_index < wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
		ast_entry = soc->ast_table[ast_index];
		if (ast_entry) {
			peer = ast_entry->peer;
			if (peer && (peer->peer_id != HTT_INVALID_PEER))
				return peer->rx_cap_enabled;
		}
	}
	return false;
}
/*
 * dp_rx_handle_enh_capture() - Deliver Rx enhanced capture data
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from monitor status ring
 *
 * Return: QDF status
 */
QDF_STATUS
dp_rx_handle_enh_capture(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	qdf_nbuf_t  mpdu_head;
	uint32_t user;
	qdf_nbuf_queue_t *mpdu_q;
	struct cdp_rx_indication_mpdu *mpdu_ind;
	struct cdp_rx_indication_mpdu_info *mpdu_info;
	struct msdu_list *msdu_list;

	user = 0;
	mpdu_q = &pdev->mpdu_q[user];

	while (!qdf_nbuf_is_queue_empty(mpdu_q) && user < MAX_MU_USERS) {
		msdu_list = &pdev->msdu_list[user];
		dp_rx_free_msdu_list(msdu_list);
		pdev->is_mpdu_hdr[user] = true;


		if (pdev->rx_enh_capture_peer &&
		    !dp_rx_enh_capture_is_peer_enabled(
				soc, ppdu_info, user)) {
			qdf_nbuf_queue_free(mpdu_q);
		} else {
			mpdu_ind = &pdev->mpdu_ind;
			mpdu_info = &mpdu_ind->mpdu_info;
			dp_rx_populate_cdp_indication_mpdu_info(
				pdev, &pdev->ppdu_info, mpdu_info, user);

			while ((mpdu_head = qdf_nbuf_queue_remove(mpdu_q))) {

				mpdu_ind->nbuf = mpdu_head;
				mpdu_info->fcs_err =
					QDF_NBUF_CB_RX_FCS_ERR(mpdu_head);

				dp_wdi_event_handler(WDI_EVENT_RX_MPDU,
						     soc, mpdu_ind,
						     HTT_INVALID_PEER,
						     WDI_NO_VAL,
						     pdev->pdev_id);
			}
		}
		user++;
		mpdu_q = &pdev->mpdu_q[user];
	}
	return QDF_STATUS_SUCCESS;
}
/*
 * dp_rx_mon_enh_capture_process() - Rx enhanced capture mode
 * processing.
 * @pdev: pdev structure
 * @tlv_status: processed TLV status
 * @status_nbuf: monitor status ring buffer
 * @ppdu_info: ppdu info structure from monitor status ring
 * @nbuf_used: nbuf need a clone
 *
 * Return: none
 */
void
dp_rx_mon_enh_capture_process(struct dp_pdev *pdev, uint32_t tlv_status,
			      qdf_nbuf_t status_nbuf,
			      struct hal_rx_ppdu_info *ppdu_info,
			      bool *nbuf_used)
{
	qdf_nbuf_t nbuf;
	struct msdu_list *msdu_list;
	uint32_t user_id;
	struct dp_soc *soc;
	qdf_nbuf_t mpdu_head;

	if (pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_DISABLED)
		return;

	user_id = ppdu_info->user_id;

	if (user_id >= MAX_MU_USERS)
		return;

	if ((tlv_status == HAL_TLV_STATUS_HEADER) && (
	    (pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_MPDU_MSDU) ||
	    ((pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_MPDU) &&
	    pdev->is_mpdu_hdr[user_id]))) {

		if (*nbuf_used) {
			nbuf = qdf_nbuf_clone(status_nbuf);
		} else {
			*nbuf_used = true;
			nbuf = status_nbuf;
		}

		if (!nbuf)
			return;

		/* Truncate 4 bytes containing PPDU ID */
		dp_nbuf_set_data_and_len(nbuf, ppdu_info->data,
					 ppdu_info->hdr_len - 4);

		if (pdev->is_mpdu_hdr[user_id]) {
			soc = pdev->soc;
			mpdu_head = qdf_nbuf_alloc(soc->osdev,
				RX_ENH_CB_BUF_SIZE + RX_ENH_CB_BUF_RESERVATION,
				RX_ENH_CB_BUF_RESERVATION,
				RX_ENH_CB_BUF_ALIGNMENT,
				FALSE);

			if (mpdu_head == NULL)
				return;

			qdf_nbuf_queue_add(&pdev->mpdu_q[user_id],
					   mpdu_head);
			pdev->is_mpdu_hdr[user_id] = false;
		}
		msdu_list = &pdev->msdu_list[user_id];
		if (!msdu_list->head)
			msdu_list->head = nbuf;
		else
			msdu_list->tail->next = nbuf;
		msdu_list->tail = nbuf;
		msdu_list->sum_len += qdf_nbuf_len(nbuf);
	}

	if (tlv_status == HAL_TLV_STATUS_MPDU_END) {
		msdu_list = &pdev->msdu_list[user_id];
		mpdu_head = qdf_nbuf_queue_last(&pdev->mpdu_q[user_id]);

		if (mpdu_head) {
			qdf_nbuf_append_ext_list(mpdu_head,
						 msdu_list->head,
						 msdu_list->sum_len);
			msdu_list->head = NULL;
			msdu_list->tail = NULL;
			msdu_list->sum_len = 0;
			QDF_NBUF_CB_RX_FCS_ERR(mpdu_head)
					       =  ppdu_info->fcs_err;
		} else {
			dp_rx_free_msdu_list(msdu_list);
		}
		pdev->is_mpdu_hdr[user_id] = true;
	}

	/* Tag the MSDU/MPDU if a cce_metadata is valid */
	if ((tlv_status == HAL_TLV_STATUS_MSDU_END) &&
	    (pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_MPDU_MSDU)) {
		bool is_rx_mon_protocol_flow_tag_en;
		/**
		 * Proceed only if this is a data frame.
		 * We could also rx probes, etc.
		 */
		if (!(ppdu_info->nac_info.fc_valid &&
		      (IEEE80211_FC0_TYPE_DATA ==
		       (ppdu_info->nac_info.frame_control &
			IEEE80211_FC0_TYPE_MASK))))
			return;

		msdu_list = &pdev->msdu_list[user_id];
		qdf_assert_always(msdu_list->head);

		/**
		 * Directly move the last MSDU and fetch the same.
		 * The earlier MSDUs should already be tagged as the
		 * packets are tagged at the end of every RX
		 * MSDU.
		 */
		nbuf = msdu_list->tail;

		is_rx_mon_protocol_flow_tag_en =
		    wlan_cfg_is_rx_mon_protocol_flow_tag_enabled(
					pdev->soc->wlan_cfg_ctx);

		if (is_rx_mon_protocol_flow_tag_en) {
			 /* Set the protocol tag value from CCE metadata */
			dp_rx_mon_enh_capture_tag_protocol_type(pdev, ppdu_info,
								user_id, nbuf);
			/* Set the flow tag from FSE metadata */
			dp_rx_mon_enh_capture_set_flow_tag(pdev, ppdu_info,
							   user_id, nbuf);
		}
		if (!pdev->is_rx_enh_capture_trailer_enabled)
			return;
		/**
		 * Update necessary information in trailer (for debug purpose)
		 */
		dp_rx_mon_enh_capture_update_trailer(pdev, nbuf);
	}
}

/*
 * dp_config_enh_rx_capture()- API to enable/disable enhanced rx capture
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: 0 for success. nonzero for failure.
 */
QDF_STATUS
dp_config_enh_rx_capture(struct dp_pdev *pdev, uint32_t val)
{
	uint8_t rx_cap_mode = (val & CDP_RX_ENH_CAPTURE_MODE_MASK);
	uint32_t rx_enh_capture_peer;
	bool is_mpdu_hdr = false;
	uint8_t user_id;
	enum dp_mon_filter_action action = DP_MON_FILTER_SET;

	rx_enh_capture_peer =
		(val & CDP_RX_ENH_CAPTURE_PEER_MASK)
		>> CDP_RX_ENH_CAPTURE_PEER_LSB;

	if (pdev->mcopy_mode || (rx_cap_mode < CDP_RX_ENH_CAPTURE_DISABLED) ||
	    (rx_cap_mode > CDP_RX_ENH_CAPTURE_MPDU_MSDU)) {
		dp_err("Invalid mode: %d", rx_cap_mode);
		return QDF_STATUS_E_INVAL;
	}

	if (rx_enh_capture_peer > CDP_RX_ENH_CAPTURE_PEER_ENABLED) {
		dp_err("Invalid peer filter %d", rx_enh_capture_peer);
		return QDF_STATUS_E_INVAL;
	}

	if ((pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_DISABLED) &&
			(rx_cap_mode == CDP_RX_ENH_CAPTURE_DISABLED)) {
		dp_err("Rx capture is already disabled %d", rx_cap_mode);
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * Store the monitor vdev if present. The monitor vdev will be restored
	 * when the Rx enhance capture mode will be disabled.
	 */
	if (pdev->rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_DISABLED &&
	    rx_cap_mode != CDP_RX_ENH_CAPTURE_DISABLED) {
		pdev->rx_enh_monitor_vdev = pdev->monitor_vdev;
	}

	/*
	 * Disable the monitor mode and re-enable it later if enhance capture
	 * gets enabled later.
	 */
	dp_reset_monitor_mode((struct cdp_soc_t *)pdev->soc, pdev->pdev_id, 0);

	if (pdev->rx_enh_capture_mode != CDP_RX_ENH_CAPTURE_DISABLED &&
	    rx_cap_mode == CDP_RX_ENH_CAPTURE_DISABLED) {
		pdev->monitor_vdev = pdev->rx_enh_monitor_vdev;
		pdev->rx_enh_monitor_vdev = NULL;
		action = DP_MON_FILTER_CLEAR;
	}

	pdev->rx_enh_capture_mode = rx_cap_mode;
	pdev->rx_enh_capture_peer = rx_enh_capture_peer;

	if (rx_cap_mode != CDP_RX_ENH_CAPTURE_DISABLED)
		is_mpdu_hdr = true;

	for (user_id = 0; user_id < MAX_MU_USERS; user_id++)
		pdev->is_mpdu_hdr[user_id] = is_mpdu_hdr;

	/* Use a bit from val to enable MSDU trailer for internal debug use */
	pdev->is_rx_enh_capture_trailer_enabled =
		(val & RX_ENH_CAPTURE_TRAILER_ENABLE_MASK) ? true : false;

	/*
	 * Restore the monitor filters if previously monitor mode was enabled.
	 */
	if (pdev->monitor_vdev) {
		pdev->monitor_configured = true;
		dp_mon_filter_setup_mon_mode(pdev);
	}

	/*
	 * Clear up the monitor mode filters if the monitor mode is enabled.
	 * Resotre the monitor mode filters once the Rx enhance capture is
	 * disabled.
	 */
	if (action == DP_MON_FILTER_SET)
		dp_mon_filter_setup_rx_enh_capture(pdev);
	else
		dp_mon_filter_reset_rx_enh_capture(pdev);

	return dp_mon_filter_update(pdev);
}

QDF_STATUS
dp_peer_set_rx_capture_enabled(struct dp_pdev *pdev, struct dp_peer *peer,
			       bool value, uint8_t *mac_addr)
{
	if (!peer) {
		dp_err("Invalid Peer");
		if (value)
			return QDF_STATUS_E_FAILURE;
		return QDF_STATUS_SUCCESS;
	}

	peer->rx_cap_enabled = value;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_RX_PKT_CAPTURE_ENH */
