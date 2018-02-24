/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "hal_api_mon.h"
#include "dp_rx_mon.h"
#include "wlan_cfg.h"

/**
 * dp_rx_mon_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @dp_pdev: core txrx pdev context
 * @buf_addr_info: void pointer to monitor link descriptor buf addr info
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_rx_mon_link_desc_return(struct dp_pdev *dp_pdev,
	void *buf_addr_info)
{
	struct dp_srng *dp_srng;
	void *hal_srng;
	void *hal_soc;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	void *src_srng_desc;

	hal_soc = dp_pdev->soc->hal_soc;

	dp_srng = &dp_pdev->rxdma_mon_desc_ring;
	hal_srng = dp_srng->hal_srng;

	qdf_assert(hal_srng);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, hal_srng))) {

		/* TODO */
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : \
			HAL RING Access For WBM Release SRNG Failed -- %pK\n",
			__func__, __LINE__, hal_srng);
		goto done;
	}

	src_srng_desc = hal_srng_src_get_next(hal_soc, hal_srng);

	if (qdf_likely(src_srng_desc)) {
		/* Return link descriptor through WBM ring (SW2WBM)*/
		hal_rx_mon_msdu_link_desc_set(hal_soc,
				src_srng_desc, buf_addr_info);
		status = QDF_STATUS_SUCCESS;
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d -- Monitor Link Desc WBM Release Ring Full\n",
			__func__, __LINE__);
	}
done:
	hal_srng_access_end(hal_soc, hal_srng);
	return status;
}

/**
 * dp_mon_adjust_frag_len() - MPDU and MSDU may spread across
 *				multiple nbufs. This function
 *                              is to return data length in
 *				fragmented buffer
 *
 * @total_len: pointer to remaining data length.
 * @frag_len: poiter to data length in this fragment.
*/
static inline void dp_mon_adjust_frag_len(uint32_t *total_len,
uint32_t *frag_len)
{
	if (*total_len >= (RX_BUFFER_SIZE - RX_PKT_TLVS_LEN)) {
		*frag_len = RX_BUFFER_SIZE - RX_PKT_TLVS_LEN;
		*total_len -= *frag_len;
	} else {
		*frag_len = *total_len;
		*total_len = 0;
	}
}

/**
 * dp_rx_mon_mpdu_pop() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @soc: core DP main context
 * @mac_id: mac id which is one of 3 mac_ids
 * @rxdma_dst_ring_desc: void pointer to monitor link descriptor buf addr info
 * @head_msdu: head of msdu to be popped
 * @tail_msdu: tail of msdu to be popped
 * @npackets: number of packet to be popped
 * @ppdu_id: ppdu id of processing ppdu
 * @head: head of descs list to be freed
 * @tail: tail of decs list to be freed
 * Return: number of msdu in MPDU to be popped
 */
static inline uint32_t
dp_rx_mon_mpdu_pop(struct dp_soc *soc, uint32_t mac_id,
	void *rxdma_dst_ring_desc, qdf_nbuf_t *head_msdu,
	qdf_nbuf_t *tail_msdu, uint32_t *npackets, uint32_t *ppdu_id,
	union dp_rx_desc_list_elem_t **head,
	union dp_rx_desc_list_elem_t **tail)
{
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];
	void *rx_desc_tlv;
	void *rx_msdu_link_desc;
	qdf_nbuf_t msdu;
	qdf_nbuf_t last;
	struct hal_rx_msdu_list msdu_list;
	uint16_t num_msdus;
	uint32_t rx_buf_size, rx_pkt_offset;
	struct hal_buf_info buf_info;
	void *p_buf_addr_info;
	void *p_last_buf_addr_info;
	uint32_t rx_bufs_used = 0;
	uint32_t msdu_ppdu_id, msdu_cnt, last_ppdu_id;
	uint32_t msdu_len;
	uint8_t *data;
	uint32_t i;
	uint32_t total_frag_len = 0, frag_len = 0;
	bool is_frag, is_first_msdu;
	bool check_ppdu_id = true;

	msdu_len = 0;
	msdu = 0;
	last_ppdu_id = dp_pdev->ppdu_info.com_info.last_ppdu_id;

	last = NULL;

	hal_rx_reo_ent_buf_paddr_get(rxdma_dst_ring_desc, &buf_info,
		&p_last_buf_addr_info, &msdu_cnt);

	if ((hal_rx_reo_ent_rxdma_push_reason_get(rxdma_dst_ring_desc) ==
		HAL_RX_WBM_RXDMA_PSH_RSN_ERROR) &&
		qdf_unlikely(hal_rx_reo_ent_rxdma_error_code_get(
		rxdma_dst_ring_desc) == HAL_RXDMA_ERR_FLUSH_REQUEST)) {
		check_ppdu_id = false;
	}

	is_frag = false;
	is_first_msdu = true;

	do {
		rx_msdu_link_desc =
			dp_rx_cookie_2_mon_link_desc_va(dp_pdev, &buf_info);

		qdf_assert(rx_msdu_link_desc);

		hal_rx_msdu_list_get(rx_msdu_link_desc, &msdu_list, &num_msdus);

		for (i = 0; i < num_msdus; i++) {
			uint32_t l2_hdr_offset;
			struct dp_rx_desc *rx_desc =
				dp_rx_cookie_2_va_mon_buf(soc,
					msdu_list.sw_cookie[i]);

			qdf_assert(rx_desc);
			msdu = rx_desc->nbuf;

			if (rx_desc->unmapped == 0) {
				qdf_nbuf_unmap_single(soc->osdev, msdu,
						      QDF_DMA_FROM_DEVICE);
				rx_desc->unmapped = 1;
			}

			data = qdf_nbuf_data(msdu);

			rx_desc_tlv = HAL_RX_MON_DEST_GET_DESC(data);

			QDF_TRACE(QDF_MODULE_ID_DP,
				QDF_TRACE_LEVEL_DEBUG,
				"[%s] i=%d, ppdu_id=%x, "
				"last_ppdu_id=%x num_msdus = %u\n",
				__func__, i, *ppdu_id,
				last_ppdu_id, num_msdus);

			if (qdf_likely(check_ppdu_id)) {
				if (is_first_msdu) {
					msdu_ppdu_id =
					HAL_RX_MON_HW_DESC_GET_PPDUID_GET(
						rx_desc_tlv);
					is_first_msdu = false;
				}

				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_DEBUG,
					"[%s] msdu_ppdu_id=%x\n",
					__func__, msdu_ppdu_id);

				if (*ppdu_id > msdu_ppdu_id)
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_WARN,
						"[%s][%d] ppdu_id=%d "
						"msdu_ppdu_id=%d\n",
						__func__, __LINE__, *ppdu_id,
						msdu_ppdu_id);

				if ((*ppdu_id < msdu_ppdu_id) && (*ppdu_id >
					last_ppdu_id)) {
					*ppdu_id = msdu_ppdu_id;
					return rx_bufs_used;
				}
			}

			if (hal_rx_desc_is_first_msdu(rx_desc_tlv))
				hal_rx_mon_hw_desc_get_mpdu_status(rx_desc_tlv,
					&(dp_pdev->ppdu_info.rx_status));


			if (msdu_list.msdu_info[i].msdu_flags &
				HAL_MSDU_F_MSDU_CONTINUATION) {
				if (!is_frag) {
					total_frag_len =
					msdu_list.msdu_info[i].msdu_len;
					is_frag = true;
				}
				dp_mon_adjust_frag_len(
					&total_frag_len, &frag_len);
			} else {
				if (is_frag) {
					dp_mon_adjust_frag_len(
						&total_frag_len, &frag_len);
				} else {
					frag_len =
					msdu_list.msdu_info[i].msdu_len;
				}
				is_frag = false;
				msdu_cnt--;
			}
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "%s total_len %u frag_len %u flags %u",
				  __func__, total_frag_len, frag_len,
				  msdu_list.msdu_info[i].msdu_flags);
			msdu_len += frag_len;

			rx_pkt_offset = HAL_RX_MON_HW_RX_DESC_SIZE();
			/*
			 * HW structures call this L3 header padding
			 * -- even though this is actually the offset
			 * from the buffer beginning where the L2
			 * header begins.
			 */
			l2_hdr_offset =
			hal_rx_msdu_end_l3_hdr_padding_get(data);

			rx_buf_size = rx_pkt_offset + l2_hdr_offset
					+ frag_len;

			qdf_nbuf_set_pktlen(msdu, rx_buf_size);

#if 0
			/* Disble it.see packet on msdu done set to 0 */
			/*
			 * Check if DMA completed -- msdu_done is the
			 * last bit to be written
			 */
			if (!hal_rx_attn_msdu_done_get(rx_desc_tlv)) {

				QDF_TRACE(QDF_MODULE_ID_DP,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s:%d: Pkt Desc\n",
					  __func__, __LINE__);

				QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					rx_desc_tlv, 128);

				qdf_assert_always(0);
			}
#endif
			rx_bufs_used++;

			QDF_TRACE(QDF_MODULE_ID_DP,
					  QDF_TRACE_LEVEL_DEBUG,
					  "%s: rx_pkt_offset=%d, l2_hdr_offset=%d, msdu_len=%d, addr=%p skb->len %lu",
					  __func__, rx_pkt_offset, l2_hdr_offset,
					  msdu_list.msdu_info[i].msdu_len,
					  qdf_nbuf_data(msdu), qdf_nbuf_len(msdu));

			if (*head_msdu == NULL)
				*head_msdu = msdu;
			else
				qdf_nbuf_set_next(last, msdu);

			last = msdu;
			dp_rx_add_to_free_desc_list(head,
				tail, rx_desc);
		}

		hal_rx_mon_next_link_desc_get(rx_msdu_link_desc, &buf_info,
			&p_buf_addr_info);

		if (dp_rx_mon_link_desc_return(dp_pdev, p_last_buf_addr_info)
			!= QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"dp_rx_mon_link_desc_return failed\n");
		}
		p_last_buf_addr_info = p_buf_addr_info;

	} while (buf_info.paddr && msdu_cnt);

	qdf_nbuf_set_next(last, NULL);

	*tail_msdu = msdu;

	dp_pdev->ppdu_info.rx_status.ppdu_len += msdu_len;

	return rx_bufs_used;

}

static inline
void dp_rx_msdus_set_payload(qdf_nbuf_t msdu)
{
	uint8_t *data;
	uint32_t rx_pkt_offset, l2_hdr_offset;

	data = qdf_nbuf_data(msdu);
	rx_pkt_offset = HAL_RX_MON_HW_RX_DESC_SIZE();
	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(data);
	qdf_nbuf_pull_head(msdu, rx_pkt_offset + l2_hdr_offset);
}

static inline
qdf_nbuf_t dp_rx_mon_restitch_mpdu_from_msdus(struct dp_soc *soc,
	uint32_t mac_id, qdf_nbuf_t head_msdu, qdf_nbuf_t last_msdu,
	struct cdp_mon_status *rx_status)
{
	qdf_nbuf_t msdu, mpdu_buf, prev_buf, msdu_orig, head_frag_list;
	uint32_t decap_format, wifi_hdr_len, sec_hdr_len, msdu_llc_len,
		mpdu_buf_len, decap_hdr_pull_bytes, frag_list_sum_len, dir,
		is_amsdu, is_first_frag, amsdu_pad;
	void *rx_desc;
	char *hdr_desc;
	unsigned char *dest;
	struct ieee80211_frame *wh;
	struct ieee80211_qoscntl *qos;
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];
	head_frag_list = NULL;

	/* The nbuf has been pulled just beyond the status and points to the
	   * payload
	*/

	msdu_orig = head_msdu;

	rx_desc = qdf_nbuf_data(msdu_orig);

	if (HAL_RX_DESC_GET_MPDU_LENGTH_ERR(rx_desc)) {
		/* It looks like there is some issue on MPDU len err */
		/* Need further investigate if drop the packet */
		/* return NULL; */
	}

	rx_desc = qdf_nbuf_data(last_msdu);

	rx_status->cdp_rs_fcs_err = HAL_RX_DESC_GET_MPDU_FCS_ERR(rx_desc);
	dp_pdev->ppdu_info.rx_status.rs_fcs_err =
		HAL_RX_DESC_GET_MPDU_FCS_ERR(rx_desc);

	/* Fill out the rx_status from the PPDU start and end fields */
	/*   HAL_RX_GET_PPDU_STATUS(soc, mac_id, rx_status); */

	rx_desc = qdf_nbuf_data(head_msdu);

	decap_format = HAL_RX_DESC_GET_DECAP_FORMAT(rx_desc);

	/* Easy case - The MSDU status indicates that this is a non-decapped
	 * packet in RAW mode.
	*/
	if (decap_format == HAL_HW_RX_DECAP_FORMAT_RAW) {
		/* Note that this path might suffer from headroom unavailabilty
		 * - but the RX status is usually enough
		 */

		dp_rx_msdus_set_payload(head_msdu);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "[%s][%d] decap format raw head %pK head->next %pK last_msdu %pK last_msdu->next %pK",
				  __func__, __LINE__, head_msdu, head_msdu->next,
				  last_msdu, last_msdu->next);

		mpdu_buf = head_msdu;

		if (!mpdu_buf)
			goto mpdu_stitch_fail;

		prev_buf = mpdu_buf;

		frag_list_sum_len = 0;
		msdu = qdf_nbuf_next(head_msdu);
		is_first_frag = 1;

		while (msdu) {

			dp_rx_msdus_set_payload(msdu);

			if (is_first_frag) {
				is_first_frag = 0;
				head_frag_list  = msdu;
			}

			frag_list_sum_len += qdf_nbuf_len(msdu);

			/* Maintain the linking of the cloned MSDUS */
			qdf_nbuf_set_next_ext(prev_buf, msdu);

			/* Move to the next */
			prev_buf = msdu;
			msdu = qdf_nbuf_next(msdu);
		}

		qdf_nbuf_trim_tail(prev_buf, HAL_RX_FCS_LEN);

		/* If there were more fragments to this RAW frame */
		if (head_frag_list) {
			frag_list_sum_len -= HAL_RX_FCS_LEN;
			qdf_nbuf_append_ext_list(mpdu_buf, head_frag_list,
				frag_list_sum_len);
			qdf_nbuf_set_next(mpdu_buf, NULL);
		}

		goto mpdu_stitch_done;
	}

	/* Decap mode:
	 * Calculate the amount of header in decapped packet to knock off based
	 * on the decap type and the corresponding number of raw bytes to copy
	 * status header
	 */
	rx_desc = qdf_nbuf_data(head_msdu);

	hdr_desc = HAL_RX_DESC_GET_80211_HDR(rx_desc);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "[%s][%d] decap format not raw",
		  __func__, __LINE__);


	/* Base size */
	wifi_hdr_len = sizeof(struct ieee80211_frame);
	wh = (struct ieee80211_frame *)hdr_desc;

	dir = wh->i_fc[1] & IEEE80211_FC1_DIR_MASK;

	if (dir == IEEE80211_FC1_DIR_DSTODS)
		wifi_hdr_len += 6;

	is_amsdu = 0;
	if (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS) {
		qos = (struct ieee80211_qoscntl *)
			(hdr_desc + wifi_hdr_len);
		wifi_hdr_len += 2;

		is_amsdu = (qos->i_qos[0] & IEEE80211_QOS_AMSDU);
	}

	/*Calculate security header length based on 'Protected'
	 * and 'EXT_IV' flag
	 * */
	if (wh->i_fc[1] & IEEE80211_FC1_WEP) {
		char *iv = (char *)wh + wifi_hdr_len;

		if (iv[3] & KEY_EXTIV)
			sec_hdr_len = 8;
		else
			sec_hdr_len = 4;
	} else {
		sec_hdr_len = 0;
	}
	wifi_hdr_len += sec_hdr_len;

	/* MSDU related stuff LLC - AMSDU subframe header etc */
	msdu_llc_len = is_amsdu ? (14 + 8) : 8;

	mpdu_buf_len = wifi_hdr_len + msdu_llc_len;

	/* "Decap" header to remove from MSDU buffer */
	decap_hdr_pull_bytes = 14;

	/* Allocate a new nbuf for holding the 802.11 header retrieved from the
	 * status of the now decapped first msdu. Leave enough headroom for
	 * accomodating any radio-tap /prism like PHY header
	 */
#define MAX_MONITOR_HEADER (512)
	mpdu_buf = qdf_nbuf_alloc(soc->osdev,
			MAX_MONITOR_HEADER + mpdu_buf_len,
			MAX_MONITOR_HEADER, 4, FALSE);

	if (!mpdu_buf)
		goto mpdu_stitch_done;

	/* Copy the MPDU related header and enc headers into the first buffer
	 * - Note that there can be a 2 byte pad between heaader and enc header
	 */

	prev_buf = mpdu_buf;
	dest = qdf_nbuf_put_tail(prev_buf, wifi_hdr_len);
	if (!dest)
		goto mpdu_stitch_fail;

	qdf_mem_copy(dest, hdr_desc, wifi_hdr_len);
	hdr_desc += wifi_hdr_len;

#if 0
	dest = qdf_nbuf_put_tail(prev_buf, sec_hdr_len);
	adf_os_mem_copy(dest, hdr_desc, sec_hdr_len);
	hdr_desc += sec_hdr_len;
#endif

	/* The first LLC len is copied into the MPDU buffer */
	frag_list_sum_len = 0;
	frag_list_sum_len -= msdu_llc_len;

	msdu_orig = head_msdu;
	is_first_frag = 1;
	amsdu_pad = 0;

	while (msdu_orig) {

		/* TODO: intra AMSDU padding - do we need it ??? */

		msdu = msdu_orig;

		if (is_first_frag) {
			head_frag_list  = msdu;
		} else {
			/* Reload the hdr ptr only on non-first MSDUs */
			rx_desc = qdf_nbuf_data(msdu_orig);
			hdr_desc = HAL_RX_DESC_GET_80211_HDR(rx_desc);
		}

		/* Copy this buffers MSDU related status into the prev buffer */

		if (is_first_frag) {
			is_first_frag = 0;
		}

		dest = qdf_nbuf_put_tail(prev_buf,
				msdu_llc_len + amsdu_pad);

		if (!dest)
			goto mpdu_stitch_fail;

		dest += amsdu_pad;
		qdf_mem_copy(dest, hdr_desc, msdu_llc_len);

		dp_rx_msdus_set_payload(msdu);

		/* Push the MSDU buffer beyond the decap header */
		qdf_nbuf_pull_head(msdu, decap_hdr_pull_bytes);
		frag_list_sum_len += msdu_llc_len + qdf_nbuf_len(msdu)
			+ amsdu_pad;

		/* Set up intra-AMSDU pad to be added to start of next buffer -
		 * AMSDU pad is 4 byte pad on AMSDU subframe */
		amsdu_pad = (msdu_llc_len + qdf_nbuf_len(msdu)) & 0x3;
		amsdu_pad = amsdu_pad ? (4 - amsdu_pad) : 0;

		/* TODO FIXME How do we handle MSDUs that have fraglist - Should
		 * probably iterate all the frags cloning them along the way and
		 * and also updating the prev_buf pointer
		 */

		/* Move to the next */
		prev_buf = msdu;
		msdu_orig = qdf_nbuf_next(msdu_orig);

	}

#if 0
	/* Add in the trailer section - encryption trailer + FCS */
	qdf_nbuf_put_tail(prev_buf, HAL_RX_FCS_LEN);
	frag_list_sum_len += HAL_RX_FCS_LEN;
#endif

	/* TODO: Convert this to suitable adf routines */
	qdf_nbuf_append_ext_list(mpdu_buf, head_frag_list,
			frag_list_sum_len);

mpdu_stitch_done:
	/* Check if this buffer contains the PPDU end status for TSF */
	/* Need revist this code to see where we can get tsf timestamp */
#if 0
	/* PPDU end TLV will be retrived from monitor status ring */
	last_mpdu =
		(*(((u_int32_t *)&rx_desc->attention)) &
		RX_ATTENTION_0_LAST_MPDU_MASK) >>
		RX_ATTENTION_0_LAST_MPDU_LSB;

	if (last_mpdu)
		rx_status->rs_tstamp.tsf = rx_desc->ppdu_end.tsf_timestamp;

#endif
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s %d mpdu_buf %pK mpdu_buf->len %u",
		  __func__, __LINE__,
		  mpdu_buf, mpdu_buf->len);
	return mpdu_buf;

mpdu_stitch_fail:
	if ((mpdu_buf) && (decap_format != HAL_HW_RX_DECAP_FORMAT_RAW)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s mpdu_stitch_fail mpdu_buf %pK",
			  __func__, mpdu_buf);
		/* Free the head buffer */
		qdf_nbuf_free(mpdu_buf);
	}
	return NULL;
}

/**
 * dp_rx_extract_radiotap_info(): Extract and populate information in
 *				struct mon_rx_status type
 * @rx_status: Receive status
 * @mon_rx_status: Monitor mode status
 *
 * Returns: None
 */
static inline
void dp_rx_extract_radiotap_info(struct cdp_mon_status *rx_status,
				struct mon_rx_status *rx_mon_status)
{
	rx_mon_status->tsft = rx_status->cdp_rs_tstamp.cdp_tsf;
	rx_mon_status->chan_freq = rx_status->rs_freq;
	rx_mon_status->chan_num = rx_status->rs_channel;
	rx_mon_status->chan_flags = rx_status->rs_flags;
	rx_mon_status->rate = rx_status->rs_datarate;
	/* TODO: rx_mon_status->ant_signal_db */
	/* TODO: rx_mon_status->nr_ant */
	rx_mon_status->mcs = rx_status->cdf_rs_rate_mcs;
	rx_mon_status->is_stbc = rx_status->cdp_rs_stbc;
	rx_mon_status->sgi = rx_status->cdp_rs_sgi;
	/* TODO: rx_mon_status->ldpc */
	/* TODO: rx_mon_status->beamformed */
	/* TODO: rx_mon_status->vht_flags */
	/* TODO: rx_mon_status->vht_flag_values1 */
}

QDF_STATUS dp_rx_mon_deliver(struct dp_soc *soc, uint32_t mac_id,
	qdf_nbuf_t head_msdu, qdf_nbuf_t tail_msdu)
{
	struct dp_pdev *pdev = soc->pdev_list[mac_id];
	struct cdp_mon_status *rs = &pdev->rx_mon_recv_status;
	qdf_nbuf_t mon_skb, skb_next;
	qdf_nbuf_t mon_mpdu = NULL;

	if ((pdev->monitor_vdev == NULL) ||
		(pdev->monitor_vdev->osif_rx_mon == NULL)) {
		goto mon_deliver_fail;
	}

	/* restitch mon MPDU for delivery via monitor interface */
	mon_mpdu = dp_rx_mon_restitch_mpdu_from_msdus(soc, mac_id, head_msdu,
				tail_msdu, rs);

	if (mon_mpdu && pdev->monitor_vdev && pdev->monitor_vdev->osif_vdev) {
		qdf_nbuf_update_radiotap(&(pdev->ppdu_info.rx_status),
			mon_mpdu, sizeof(struct rx_pkt_tlvs));
		pdev->monitor_vdev->osif_rx_mon(
				pdev->monitor_vdev->osif_vdev, mon_mpdu, NULL);
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "[%s][%d] mon_mpdu=%p pdev->monitor_vdev %p osif_vdev %p",
				  __func__, __LINE__, mon_mpdu, pdev->monitor_vdev,
				  pdev->monitor_vdev->osif_vdev);
		goto mon_deliver_fail;
	}

	return QDF_STATUS_SUCCESS;

mon_deliver_fail:
	mon_skb = head_msdu;
	while (mon_skb) {
		skb_next = qdf_nbuf_next(mon_skb);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "[%s][%d] mon_skb=%p len %u", __func__, __LINE__,
				  mon_skb, mon_skb->len);

		qdf_nbuf_free(mon_skb);
		mon_skb = skb_next;
	}
	return QDF_STATUS_E_INVAL;
}

/**
* dp_rx_mon_dest_process() - Brain of the Rx processing functionality
*	Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
* @soc: core txrx main contex
* @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
* @quota: No. of units (packets) that can be serviced in one shot.
*
* This function implements the core of Rx functionality. This is
* expected to handle only non-error frames.
*
* Return: none
*/
void dp_rx_mon_dest_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = soc->pdev_list[mac_id];
	uint8_t pdev_id;
	void *hal_soc;
	void *rxdma_dst_ring_desc;
	void *mon_dst_srng;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	uint32_t ppdu_id;
	uint32_t rx_bufs_used;

	pdev_id = pdev->pdev_id;
	mon_dst_srng = pdev->rxdma_mon_dst_ring.hal_srng;

	if (!mon_dst_srng || !hal_srng_initialized(mon_dst_srng)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring Init Failed -- %pK\n",
			__func__, __LINE__, mon_dst_srng);
		return;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(hal_srng_access_start(hal_soc, mon_dst_srng))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Destination Ring access Failed -- %pK\n",
			__func__, __LINE__, mon_dst_srng);
		return;
	}

	ppdu_id = pdev->ppdu_info.com_info.ppdu_id;
	rx_bufs_used = 0;
	while (qdf_likely(rxdma_dst_ring_desc =
		hal_srng_dst_peek(hal_soc, mon_dst_srng))) {
		qdf_nbuf_t head_msdu, tail_msdu;
		uint32_t npackets;
		head_msdu = (qdf_nbuf_t) NULL;
		tail_msdu = (qdf_nbuf_t) NULL;

		rx_bufs_used += dp_rx_mon_mpdu_pop(soc, mac_id,
					rxdma_dst_ring_desc,
					&head_msdu, &tail_msdu,
					&npackets, &ppdu_id,
					&head, &tail);

		if (ppdu_id != pdev->ppdu_info.com_info.ppdu_id) {
			pdev->mon_ppdu_status = DP_PPDU_STATUS_START;
			pdev->ppdu_info.com_info.last_ppdu_id =
				pdev->ppdu_info.com_info.ppdu_id;
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
					  "%s %d ppdu_id %x != ppdu_info.com_info .ppdu_id %x",
					  __func__, __LINE__,
					  ppdu_id, pdev->ppdu_info.com_info.ppdu_id);
			break;
		}

		dp_rx_mon_deliver(soc, mac_id, head_msdu, tail_msdu);

		rxdma_dst_ring_desc = hal_srng_dst_get_next(hal_soc,
			mon_dst_srng);
	}

	hal_srng_access_end(hal_soc, mon_dst_srng);

	if (rx_bufs_used) {
		dp_rx_buffers_replenish(soc, pdev_id,
			&pdev->rxdma_mon_buf_ring, &soc->rx_desc_mon[pdev_id],
			rx_bufs_used, &head, &tail, HAL_RX_BUF_RBM_SW3_BM);
	}
}

static QDF_STATUS
dp_rx_pdev_mon_buf_attach(struct dp_pdev *pdev) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	struct dp_srng *rxdma_srng;
	uint32_t rxdma_entries;
	struct rx_desc_pool *rx_desc_pool;
	QDF_STATUS status;

	rxdma_srng = &pdev->rxdma_mon_buf_ring;

	rxdma_entries = rxdma_srng->alloc_size/hal_srng_get_entrysize(
				soc->hal_soc,
				RXDMA_MONITOR_BUF);

	rx_desc_pool = &soc->rx_desc_mon[pdev_id];

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
			  "%s: Mon RX Desc Pool[%d] allocation size=%d"
			  , __func__, pdev_id, rxdma_entries*3);

	status = dp_rx_desc_pool_alloc(soc, pdev_id,
			rxdma_entries*3, rx_desc_pool);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_rx_desc_pool_alloc() failed \n", __func__);
		return status;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
			  "%s: Mon RX Buffers Replenish pdev_id=%d",
			  __func__, pdev_id);

	status = dp_rx_buffers_replenish(soc, pdev_id, rxdma_srng, rx_desc_pool,
			rxdma_entries, &desc_list, &tail,
			HAL_RX_BUF_RBM_SW3_BM);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_rx_buffers_replenish() failed \n", __func__);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_rx_pdev_mon_buf_detach(struct dp_pdev *pdev) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_mon[pdev_id];
	if (rx_desc_pool->pool_size != 0) {
		dp_rx_desc_pool_free(soc, pdev_id, rx_desc_pool);
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * Allocate and setup link descriptor pool that will be used by HW for
 * various link and queue descriptors and managed by WBM
 */
static int dp_mon_link_desc_pool_setup(struct dp_soc *soc, uint32_t mac_id)
{
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];
	int link_desc_size = hal_get_link_desc_size(soc->hal_soc);
	int link_desc_align = hal_get_link_desc_align(soc->hal_soc);
	uint32_t max_alloc_size = wlan_cfg_max_alloc_size(soc->wlan_cfg_ctx);
	uint32_t total_link_descs, total_mem_size;
	uint32_t num_link_desc_banks;
	uint32_t last_bank_size = 0;
	uint32_t entry_size, num_entries;
	void *mon_desc_srng;
	uint32_t num_replenish_buf;
	struct dp_srng *dp_srng;
	int i;

	dp_srng = &dp_pdev->rxdma_mon_desc_ring;

	num_entries = dp_srng->alloc_size/hal_srng_get_entrysize(
					soc->hal_soc, RXDMA_MONITOR_DESC);

	/* Round up to power of 2 */
	total_link_descs = 1;
	while (total_link_descs < num_entries)
		total_link_descs <<= 1;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"%s: total_link_descs: %u, link_desc_size: %d\n",
		__func__, total_link_descs, link_desc_size);

	total_mem_size =  total_link_descs * link_desc_size;

	total_mem_size += link_desc_align;

	if (total_mem_size <= max_alloc_size) {
		num_link_desc_banks = 0;
		last_bank_size = total_mem_size;
	} else {
		num_link_desc_banks = (total_mem_size) /
			(max_alloc_size - link_desc_align);
		last_bank_size = total_mem_size %
			(max_alloc_size - link_desc_align);
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		"%s: total_mem_size: %d, num_link_desc_banks: %u, \
		max_alloc_size: %d last_bank_size: %d\n",
		__func__, total_mem_size, num_link_desc_banks, max_alloc_size,
		last_bank_size);

	for (i = 0; i < num_link_desc_banks; i++) {
		dp_pdev->link_desc_banks[i].base_vaddr_unaligned =
		qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
			max_alloc_size,
			&(dp_pdev->link_desc_banks[i].base_paddr_unaligned));

		if (!dp_pdev->link_desc_banks[i].base_vaddr_unaligned) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Link desc memory allocation failed\n",
				__func__);
			goto fail;
		}
		dp_pdev->link_desc_banks[i].size = max_alloc_size;

		dp_pdev->link_desc_banks[i].base_vaddr =
			(void *)((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		dp_pdev->link_desc_banks[i].base_paddr =
			(unsigned long)
			(dp_pdev->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr) -
			 (unsigned long)
			 (dp_pdev->link_desc_banks[i].base_vaddr_unaligned));
	}

	if (last_bank_size) {
		/* Allocate last bank in case total memory required is not exact
		 * multiple of max_alloc_size
		 */
		dp_pdev->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev,
			soc->osdev->dev, last_bank_size,
			&(dp_pdev->link_desc_banks[i].base_paddr_unaligned));

		if (dp_pdev->link_desc_banks[i].base_vaddr_unaligned == NULL) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: allocation failed for mon link desc pool\n",
				__func__);
			goto fail;
		}
		dp_pdev->link_desc_banks[i].size = last_bank_size;

		dp_pdev->link_desc_banks[i].base_vaddr =
			(void *)((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		dp_pdev->link_desc_banks[i].base_paddr =
			(unsigned long)
			(dp_pdev->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr) -
			(unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned));
	}

	/* Allocate and setup link descriptor idle list for HW internal use */
	entry_size = hal_srng_get_entrysize(soc->hal_soc, RXDMA_MONITOR_DESC);
	total_mem_size = entry_size * total_link_descs;

	mon_desc_srng = dp_pdev->rxdma_mon_desc_ring.hal_srng;

	num_replenish_buf = 0;

	if (total_mem_size <= max_alloc_size) {
		void *desc;

		hal_srng_access_start_unlocked(soc->hal_soc, mon_desc_srng);

		for (i = 0; i < MAX_MON_LINK_DESC_BANKS &&
				dp_pdev->link_desc_banks[i].base_paddr; i++) {
			uint32_t num_entries =
			(dp_pdev->link_desc_banks[i].size -
			(unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr) -
			(unsigned long)
			(dp_pdev->link_desc_banks[i].base_vaddr_unaligned))
			/ link_desc_size;
			unsigned long paddr =
				(unsigned long)
				(dp_pdev->link_desc_banks[i].base_paddr);
			unsigned long vaddr =
				(unsigned long)
				(dp_pdev->link_desc_banks[i].base_vaddr);

			while (num_entries && (desc =
				hal_srng_src_get_next(soc->hal_soc,
					mon_desc_srng))) {

				hal_set_link_desc_addr(desc, i, paddr);
				num_entries--;
				num_replenish_buf++;
				paddr += link_desc_size;
				vaddr += link_desc_size;
			}
		}
		hal_srng_access_end_unlocked(soc->hal_soc, mon_desc_srng);
	} else {
		qdf_assert(0);
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		"%s: successfully replenished %d buffer\n",
		__func__, num_replenish_buf);

	return 0;

fail:
	for (i = 0; i < MAX_MON_LINK_DESC_BANKS; i++) {
		if (dp_pdev->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
			dp_pdev->link_desc_banks[i].size,
			dp_pdev->link_desc_banks[i].base_vaddr_unaligned,
			dp_pdev->link_desc_banks[i].base_paddr_unaligned, 0);
			dp_pdev->link_desc_banks[i].base_vaddr_unaligned = NULL;
		}
	}
	return QDF_STATUS_E_FAILURE;
}

/*
 * Free link descriptor pool that was setup HW
 */
static void dp_mon_link_desc_pool_cleanup(struct dp_soc *soc, uint32_t mac_id)
{
	struct dp_pdev *dp_pdev = soc->pdev_list[mac_id];
	int i;

	for (i = 0; i < MAX_MON_LINK_DESC_BANKS; i++) {
		if (dp_pdev->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
			dp_pdev->link_desc_banks[i].size,
			dp_pdev->link_desc_banks[i].base_vaddr_unaligned,
			dp_pdev->link_desc_banks[i].base_paddr_unaligned, 0);
			dp_pdev->link_desc_banks[i].base_vaddr_unaligned = NULL;
		}
	}
}

/**
 * dp_rx_pdev_mon_attach() - attach DP RX for monitor mode
 * @pdev: core txrx pdev context
 *
 * This function will attach a DP RX for monitor mode instance into
 * the main device (SOC) context. Will allocate dp rx resource and
 * initialize resources.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */

QDF_STATUS
dp_rx_pdev_mon_attach(struct dp_pdev *pdev) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	QDF_STATUS status;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
			"%s: pdev attach id=%d\n", __func__, pdev_id);

	status = dp_rx_pdev_mon_buf_attach(pdev);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_rx_pdev_mon_buf_attach() failed \n", __func__);
		return status;
	}

	status = dp_rx_pdev_mon_status_attach(pdev);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_rx_pdev_mon_status_attach() failed \n",
			__func__);
		return status;
	}

	status = dp_mon_link_desc_pool_setup(soc, pdev_id);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_mon_link_desc_pool_setup() failed \n",
			__func__);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
/**
 * dp_rx_pdev_mon_detach() - detach dp rx for monitor mode
 * @pdev: core txrx pdev context
 *
 * This function will detach DP RX for monitor mode from
 * main device context. will free DP Rx resources for
 * monitor mode
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS
dp_rx_pdev_mon_detach(struct dp_pdev *pdev) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;

	dp_mon_link_desc_pool_cleanup(soc, pdev_id);
	dp_rx_pdev_mon_status_detach(pdev);
	dp_rx_pdev_mon_buf_detach(pdev);

	return QDF_STATUS_SUCCESS;
}
