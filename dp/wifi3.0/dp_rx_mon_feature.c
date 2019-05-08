/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

	cdp_mpdu_info->ppdu_id = ppdu_info->com_info.ppdu_id;
	cdp_mpdu_info->channel = ppdu_info->rx_status.chan_num;
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
		cdp_mpdu_info->nss = ppdu_info->rx_user_status[user].nss;
		cdp_mpdu_info->mcs = ppdu_info->rx_user_status[user].mcs;
	} else {
		cdp_mpdu_info->nss = ppdu_info->rx_status.nss;
		cdp_mpdu_info->mcs = ppdu_info->rx_status.mcs;
	}
	cdp_mpdu_info->rate = ppdu_info->rx_status.rate;
	for (i = 0; i < MAX_CHAIN; i++)
		cdp_mpdu_info->per_chain_rssi[i] = ppdu_info->rx_status.rssi[i];
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

	while (!qdf_nbuf_is_queue_empty(mpdu_q)) {
		msdu_list = &pdev->msdu_list[user];
		dp_rx_free_msdu_list(msdu_list);
		mpdu_ind = &pdev->mpdu_ind[user];
		mpdu_info = &mpdu_ind->mpdu_info;

		dp_rx_populate_cdp_indication_mpdu_info(
			pdev, &pdev->ppdu_info, mpdu_info, user);

		while ((mpdu_head = qdf_nbuf_queue_remove(mpdu_q))) {

			mpdu_ind->nbuf = mpdu_head;
			mpdu_info->fcs_err =
				QDF_NBUF_CB_RX_FCS_ERR(mpdu_head);

			dp_wdi_event_handler(WDI_EVENT_RX_MPDU,
					     soc, mpdu_ind, HTT_INVALID_PEER,
					     WDI_NO_VAL, pdev->pdev_id);
		}
		user++;
		mpdu_q = &pdev->mpdu_q[user];
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_mon_enh_capture_process() - Rx enhanced capture mode
 *	processing.
 * @pdev: pdev structure
 * @tlv_status: processed TLV status
 * @status_nbuf: monitor status ring buffer
 * @ppdu_info: ppdu info structure from monitor status ring
 * @nbuf_used: nbuf need a clone
 * @rx_enh_capture_mode: Rx enhanced capture mode
 *
 * Return: none
 */
void
dp_rx_mon_enh_capture_process(struct dp_pdev *pdev, uint32_t tlv_status,
			      qdf_nbuf_t status_nbuf,
			      struct hal_rx_ppdu_info *ppdu_info,
			      bool *nbuf_used,
			      uint32_t rx_enh_capture_mode)
{
	qdf_nbuf_t nbuf;
	struct msdu_list *msdu_list;
	uint32_t user_id;
	struct dp_soc *soc;
	qdf_nbuf_t mpdu_head;

	if (rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_DISABLED)
		return;

	user_id = ppdu_info->user_id;

	if ((tlv_status == HAL_TLV_STATUS_HEADER) && (
	    (rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_MPDU_MSDU) ||
	    ((rx_enh_capture_mode == CDP_RX_ENH_CAPTURE_MPDU) &&
	    pdev->is_mpdu_hdr[user_id]))) {

		if (*nbuf_used) {
			nbuf = qdf_nbuf_clone(status_nbuf);
		} else {
			*nbuf_used = true;
			nbuf = status_nbuf;
		}

		if (!nbuf)
			return;

		dp_nbuf_set_data_and_len(nbuf, ppdu_info->data,
					 ppdu_info->hdr_len
					 - 4);
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
}

/*
 * dp_config_enh_rx_capture()- API to enable/disable enhanced rx capture
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: 0 for success. nonzero for failure.
 */
QDF_STATUS
dp_config_enh_rx_capture(struct cdp_pdev *pdev_handle, int val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (pdev->mcopy_mode || (val < CDP_RX_ENH_CAPTURE_DISABLED) ||
	    (val > CDP_RX_ENH_CAPTURE_MPDU_MSDU)) {
		dp_err("Invalid mode");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev->rx_enh_capture_mode)
		dp_reset_monitor_mode(pdev_handle);

	pdev->rx_enh_capture_mode = val;
	return dp_pdev_configure_monitor_rings(pdev);
}
#endif
