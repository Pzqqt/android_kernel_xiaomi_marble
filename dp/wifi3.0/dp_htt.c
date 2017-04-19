/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include <htt.h>
#include <hal_api.h>
#include "dp_htt.h"
#include "dp_peer.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_rx_mon.h"

#define HTT_HTC_PKT_POOL_INIT_SIZE 64

#define HTT_MSG_BUF_SIZE(msg_bytes) \
	((msg_bytes) + HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING)

/*
 * htt_htc_pkt_alloc() - Allocate HTC packet buffer
 * @htt_soc:	HTT SOC handle
 *
 * Return: Pointer to htc packet buffer
 */
static struct dp_htt_htc_pkt *
htt_htc_pkt_alloc(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt = NULL;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_freelist) {
		pkt = soc->htt_htc_pkt_freelist;
		soc->htt_htc_pkt_freelist = soc->htt_htc_pkt_freelist->u.next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	if (pkt == NULL)
		pkt = qdf_mem_malloc(sizeof(*pkt));
	return &pkt->u.pkt; /* not actually a dereference */
}

/*
 * htt_htc_pkt_free() - Free HTC packet buffer
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_pkt_free(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
		(struct dp_htt_htc_pkt_union *)pkt;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	u_pkt->u.next = soc->htt_htc_pkt_freelist;
	soc->htt_htc_pkt_freelist = u_pkt;
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

/*
 * htt_htc_pkt_pool_free() - Free HTC packet pool
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	pkt = soc->htt_htc_pkt_freelist;
	while (pkt) {
		next = pkt->u.next;
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_freelist = NULL;
}

/*
 * htt_t2h_mac_addr_deswizzle() - Swap MAC addr bytes if FW endianess differ
 * @tgt_mac_addr:	Target MAC
 * @buffer:		Output buffer
 */
static u_int8_t *
htt_t2h_mac_addr_deswizzle(u_int8_t *tgt_mac_addr, u_int8_t *buffer)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * The host endianness is opposite of the target endianness.
	 * To make u_int32_t elements come out correctly, the target->host
	 * upload has swizzled the bytes in each u_int32_t element of the
	 * message.
	 * For byte-array message fields like the MAC address, this
	 * upload swizzling puts the bytes in the wrong order, and needs
	 * to be undone.
	 */
	buffer[0] = tgt_mac_addr[3];
	buffer[1] = tgt_mac_addr[2];
	buffer[2] = tgt_mac_addr[1];
	buffer[3] = tgt_mac_addr[0];
	buffer[4] = tgt_mac_addr[7];
	buffer[5] = tgt_mac_addr[6];
	return buffer;
#else
	/*
	 * The host endianness matches the target endianness -
	 * we can use the mac addr directly from the message buffer.
	 */
	return tgt_mac_addr;
#endif
}

/*
 * dp_htt_h2t_send_complete_free_netbuf() - Free completed buffer
 * @soc:	SOC handle
 * @status:	Completion status
 * @netbuf:	HTT buffer
 */
static void
dp_htt_h2t_send_complete_free_netbuf(
	void *soc, A_STATUS status, qdf_nbuf_t netbuf)
{
	qdf_nbuf_free(netbuf);
}

/*
 * dp_htt_h2t_send_complete() - H2T completion handler
 * @context:	Opaque context (HTT SOC handle)
 * @htc_pkt:	HTC packet
 */
static void
dp_htt_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	void (*send_complete_part2)(
		void *soc, A_STATUS status, qdf_nbuf_t msdu);
	struct htt_soc *soc =  (struct htt_soc *) context;
	struct dp_htt_htc_pkt *htt_pkt;
	qdf_nbuf_t netbuf;

	send_complete_part2 = htc_pkt->pPktContext;

	htt_pkt = container_of(htc_pkt, struct dp_htt_htc_pkt, htc_pkt);

	/* process (free or keep) the netbuf that held the message */
	netbuf = (qdf_nbuf_t) htc_pkt->pNetBufContext;
	/*
	 * adf sendcomplete is required for windows only
	 */
	/* qdf_nbuf_set_sendcompleteflag(netbuf, TRUE); */
	if (send_complete_part2 != NULL) {
		send_complete_part2(
			htt_pkt->soc_ctxt, htc_pkt->Status, netbuf);
	}
	/* free the htt_htc_pkt / HTC_PACKET object */
	htt_htc_pkt_free(soc, htt_pkt);
}

/*
 * htt_h2t_ver_req_msg() - Send HTT version request message to target
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
static int htt_h2t_ver_req_msg(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_VER_REQ_BYTES),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_VER_REQ_BYTES) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for HTT_H2T_MSG_TYPE_VERSION_REQ msg\n",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *) qdf_nbuf_data(msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_VERSION_REQ);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}
	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf, qdf_nbuf_data(msg),
		qdf_nbuf_len(msg), soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	htc_send_pkt(soc->htc_soc, &pkt->htc_pkt);
	return 0;
}

/*
 * htt_srng_setup() - Send SRNG setup message to target
 * @htt_soc:	HTT SOC handle
 * @mac_id:	MAC Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 *
 * Return: 0 on success; error code on failure
 */
int htt_srng_setup(void *htt_soc, int mac_id, void *hal_srng,
	int hal_ring_type)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	struct hal_srng_params srng_params;
	qdf_dma_addr_t hp_addr, tp_addr;
	uint32_t ring_entry_size =
		hal_srng_get_entrysize(soc->hal_soc, hal_ring_type);
	int htt_ring_type, htt_ring_id;

	/* Sizes should be set in 4-byte words */
	ring_entry_size = ring_entry_size >> 2;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_SRING_SETUP_SZ),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg)
		goto fail0;

	hal_get_srng_params(soc->hal_soc, hal_srng, &srng_params);
	hp_addr = hal_srng_get_hp_addr(soc->hal_soc, hal_srng);
	tp_addr = hal_srng_get_tp_addr(soc->hal_soc, hal_srng);

	switch (hal_ring_type) {
	case RXDMA_BUF:
#ifdef QCA_HOST2FW_RXBUF_RING
		if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF)) {
			htt_ring_id = HTT_HOST1_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#else
		if (srng_params.ring_id ==
			(HAL_SRNG_WMAC1_SW2RXDMA0_BUF +
			  (mac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
#endif
		} else if (srng_params.ring_id ==
			 (HAL_SRNG_WMAC1_SW2RXDMA1_BUF +
			  (mac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
		} else {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
					   "%s: Ring %d currently not supported\n",
					   __func__, srng_params.ring_id);
			goto fail1;
		}

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: ring_type %d ring_id %d\n",
			 __func__, hal_ring_type, srng_params.ring_id);
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: hp_addr 0x%llx tp_addr 0x%llx\n",
			 __func__, (uint64_t)hp_addr, (uint64_t)tp_addr);
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: htt_ring_id %d\n", __func__, htt_ring_id);
		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported\n", __func__);
			goto fail1;
	}

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_SRING_SETUP_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for SRING_SETUP msg\n",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_SRING_SETUP);

	if (htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word,
			 DP_SW2HW_MACID(mac_id));
	else
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word, mac_id);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: mac_id %d\n", __func__, mac_id);
	HTT_SRING_SETUP_RING_TYPE_SET(*msg_word, htt_ring_type);
	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_SRING_SETUP_RING_ID_SET(*msg_word, htt_ring_id);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_LO_SET(*msg_word,
		srng_params.ring_base_paddr & 0xffffffff);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)srng_params.ring_base_paddr >> 32);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_ENTRY_SIZE_SET(*msg_word, ring_entry_size);
	HTT_SRING_SETUP_RING_SIZE_SET(*msg_word,
		(ring_entry_size * srng_params.num_entries));
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: entry_size %d\n", __func__,
			 ring_entry_size);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: num_entries %d\n", __func__,
			 srng_params.num_entries);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: ring_size %d\n", __func__,
			 (ring_entry_size * srng_params.num_entries));
	if (htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_SRING_SETUP_RING_MISC_CFG_FLAG_LOOPCOUNT_DISABLE_SET(
						*msg_word, 1);
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_MSI_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_TLV_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_DATA_TLV_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_HOST_FW_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_RING_PTR_SWAP));

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		hp_addr & 0xffffffff);

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)hp_addr >> 32);

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		tp_addr & 0xffffffff);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)tp_addr >> 32);

	/* word 8 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_LO_SET(*msg_word,
		srng_params.msi_addr & 0xffffffff);

	/* word 9 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_HI_SET(*msg_word,
		(uint64_t)(srng_params.msi_addr) >> 32);

	/* word 10 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_DATA_SET(*msg_word,
		srng_params.msi_data);

	/* word 11 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_INTR_BATCH_COUNTER_TH_SET(*msg_word,
		srng_params.intr_batch_cntr_thres_entries *
		ring_entry_size);
	HTT_SRING_SETUP_INTR_TIMER_TH_SET(*msg_word,
		srng_params.intr_timer_thres_us >> 3);

	/* word 12 */
	msg_word++;
	*msg_word = 0;
	if (srng_params.flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		/* TODO: Setting low threshold to 1/8th of ring size - see
		 * if this needs to be configurable
		 */
		HTT_SRING_SETUP_INTR_LOW_TH_SET(*msg_word,
			srng_params.low_threshold);
	}
	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt)
		goto fail1;

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	htc_send_pkt(soc->htc_soc, &pkt->htc_pkt);

	return QDF_STATUS_SUCCESS;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

/*
 * htt_h2t_rx_ring_cfg() - Send SRNG packet and TLV filter
 * config message to target
 * @htt_soc:	HTT SOC handle
 * @pdev_id:	PDEV Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 * @ring_buf_size:	SRNG buffer size
 * @htt_tlv_filter:	Rx SRNG TLV and filter setting
 * Return: 0 on success; error code on failure
 */
int htt_h2t_rx_ring_cfg(void *htt_soc, int pdev_id, void *hal_srng,
	int hal_ring_type, int ring_buf_size,
	struct htt_rx_ring_tlv_filter *htt_tlv_filter)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	struct hal_srng_params srng_params;
	uint32_t htt_ring_type, htt_ring_id;
	uint32_t tlv_filter;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_RX_RING_SELECTION_CFG_SZ),
	/* reserve room for the HTC header */
	HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg)
		goto fail0;

	hal_get_srng_params(soc->hal_soc, hal_srng, &srng_params);

	switch (hal_ring_type) {
	case RXDMA_BUF:
#if QCA_HOST2FW_RXBUF_RING
		htt_ring_id = HTT_HOST1_TO_FW_RXBUF_RING;
		htt_ring_type = HTT_SW_TO_SW_RING;
#else
		htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
#endif
		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported\n", __func__);
		goto fail1;
	}

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_RX_RING_SELECTION_CFG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for RX Ring Cfg msg\n",
			__func__);
		goto fail1; /* failure */
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_RING_SELECTION_CFG);
	HTT_RX_RING_SELECTION_CFG_PDEV_ID_SET(*msg_word, pdev_id);
	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_RX_RING_SELECTION_CFG_RING_ID_SET(*msg_word, htt_ring_id);

	HTT_RX_RING_SELECTION_CFG_STATUS_TLV_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));

	HTT_RX_RING_SELECTION_CFG_PKT_TLV_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_DATA_TLV_SWAP));

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_RX_RING_SELECTION_CFG_RING_BUFFER_SIZE_SET(*msg_word,
		ring_buf_size);

	/* word 2 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0001, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 1001, 1);
	}

	if (htt_tlv_filter->enable_md) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0001, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 1001, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0001, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 1001, 1);
	}
	/* word 3 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT,	1110, 1);
	}

	if (htt_tlv_filter->enable_md) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MD,
			MGMT, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MD,
			MGMT, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MD,
			MGMT, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MD,
			MGMT, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MD,
			MGMT, 1110, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1110, 1);
	}

	/* word 4 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1001, 1);
	}

	if (htt_tlv_filter->enable_md) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1001, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1001, 1);
	}

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->enable_fp) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, MCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, UCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, NULL, 1);
	}

	if (htt_tlv_filter->enable_md) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, MCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, UCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, NULL, 1);
	}
	if (htt_tlv_filter->enable_mo) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, MCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, UCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, NULL, 1);
	}

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	tlv_filter = 0;
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_START,
		htt_tlv_filter->mpdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_START,
		htt_tlv_filter->msdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET,
		htt_tlv_filter->packet);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_END,
		htt_tlv_filter->msdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_END,
		htt_tlv_filter->mpdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET_HEADER,
		htt_tlv_filter->packet_header);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, ATTENTION,
		htt_tlv_filter->ppdu_end_status_done);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_START,
		htt_tlv_filter->ppdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END,
		htt_tlv_filter->ppdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_USER_STATS,
		htt_tlv_filter->ppdu_end_user_stats);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter,
		PPDU_END_USER_STATS_EXT,
		htt_tlv_filter->ppdu_end_user_stats_ext);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_STATUS_DONE,
		htt_tlv_filter->ppdu_end_status_done);

	HTT_RX_RING_SELECTION_CFG_TLV_FILTER_IN_FLAG_SET(*msg_word, tlv_filter);

	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt)
		goto fail1;

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	htc_send_pkt(soc->htc_soc, &pkt->htc_pkt);
	return QDF_STATUS_SUCCESS;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

/*
 * htt_soc_attach_target() - SOC level HTT setup
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
int htt_soc_attach_target(void *htt_soc)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	return htt_h2t_ver_req_msg(soc);
}


/*
 * dp_htt_t2h_msg_handler() - Generic Target to host Msg/event handler
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 */
static void dp_htt_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct htt_soc *soc = (struct htt_soc *) context;
	qdf_nbuf_t htt_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	u_int32_t *msg_word;
	enum htt_t2h_msg_type msg_type;

	/* check for successful message reception */
	if (pkt->Status != A_OK) {
		if (pkt->Status != A_ECANCELED)
			soc->stats.htc_err_cnt++;

		qdf_nbuf_free(htt_t2h_msg);
		return;
	}

	/* TODO: Check if we should pop the HTC/HTT header alignment padding */

	msg_word = (u_int32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	switch (msg_type) {
	case HTT_T2H_MSG_TYPE_PEER_MAP:
		{
			u_int8_t mac_addr_deswizzle_buf[HTT_MAC_ADDR_LEN];
			u_int8_t *peer_mac_addr;
			u_int16_t peer_id;
			u_int16_t hw_peer_id;
			u_int8_t vdev_id;

			peer_id = HTT_RX_PEER_MAP_PEER_ID_GET(*msg_word);
			hw_peer_id =
				HTT_RX_PEER_MAP_HW_PEER_ID_GET(*(msg_word+2));
			vdev_id = HTT_RX_PEER_MAP_VDEV_ID_GET(*msg_word);
			peer_mac_addr = htt_t2h_mac_addr_deswizzle(
				(u_int8_t *) (msg_word+1),
				&mac_addr_deswizzle_buf[0]);
			QDF_TRACE(QDF_MODULE_ID_TXRX,
				QDF_TRACE_LEVEL_INFO,
				"HTT_T2H_MSG_TYPE_PEER_MAP msg for peer id %d vdev id %d n",
				peer_id, vdev_id);

			dp_rx_peer_map_handler(soc->dp_soc, peer_id, hw_peer_id,
						vdev_id, peer_mac_addr);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP:
		{
			u_int16_t peer_id;
			peer_id = HTT_RX_PEER_UNMAP_PEER_ID_GET(*msg_word);

			dp_rx_peer_unmap_handler(soc->dp_soc, peer_id);
			break;
		}
	case HTT_T2H_MSG_TYPE_SEC_IND:
		{
			u_int16_t peer_id;
			enum htt_sec_type sec_type;
			int is_unicast;

			peer_id = HTT_SEC_IND_PEER_ID_GET(*msg_word);
			sec_type = HTT_SEC_IND_SEC_TYPE_GET(*msg_word);
			is_unicast = HTT_SEC_IND_UNICAST_GET(*msg_word);
			/* point to the first part of the Michael key */
			msg_word++;
			dp_rx_sec_ind_handler(
				soc->dp_soc, peer_id, sec_type, is_unicast,
				msg_word, msg_word + 2);
			break;
		}
#ifdef notyet
#ifndef REMOVE_PKT_LOG
	case HTT_T2H_MSG_TYPE_PKTLOG:
		{
			u_int32_t *pl_hdr;
			pl_hdr = (msg_word + 1);
			wdi_event_handler(WDI_EVENT_OFFLOAD_ALL, soc->dp_soc,
				pl_hdr, HTT_INVALID_PEER, WDI_NO_VAL);
			break;
		}
#endif
#endif /* notyet */
	case HTT_T2H_MSG_TYPE_VERSION_CONF:
		{
			soc->tgt_ver.major = HTT_VER_CONF_MAJOR_GET(*msg_word);
			soc->tgt_ver.minor = HTT_VER_CONF_MINOR_GET(*msg_word);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
				"target uses HTT version %d.%d; host uses %d.%d\n",
				soc->tgt_ver.major, soc->tgt_ver.minor,
				HTT_CURRENT_VERSION_MAJOR,
				HTT_CURRENT_VERSION_MINOR);
			if (soc->tgt_ver.major != HTT_CURRENT_VERSION_MAJOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"*** Incompatible host/target HTT versions!\n");
			}
			/* abort if the target is incompatible with the host */
			qdf_assert(soc->tgt_ver.major ==
				HTT_CURRENT_VERSION_MAJOR);
			if (soc->tgt_ver.minor != HTT_CURRENT_VERSION_MINOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_WARN,
					"*** Warning: host/target HTT versions"
					" are different, though compatible!\n");
			}
			break;
		}
	case HTT_T2H_MSG_TYPE_RX_ADDBA:
		{
			uint16_t peer_id;
			uint8_t tid;
			uint8_t win_sz;
			uint16_t status;
			struct dp_peer *peer;

			/*
			 * Update REO Queue Desc with new values
			 */
			peer_id = HTT_RX_ADDBA_PEER_ID_GET(*msg_word);
			tid = HTT_RX_ADDBA_TID_GET(*msg_word);
			win_sz = HTT_RX_ADDBA_WIN_SIZE_GET(*msg_word);
			peer = dp_peer_find_by_id(soc->dp_soc, peer_id);

			/*
			 * Window size needs to be incremented by 1
			 * since fw needs to represent a value of 256
			 * using just 8 bits
			 */
			if (peer) {
				status = dp_addba_requestprocess_wifi3(peer,
						0, tid, 0, win_sz + 1, 0xffff);
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_INFO,
					FL("PeerID %d BAW %d TID %d stat %d\n"),
					peer_id, win_sz, tid, status);

			} else {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					FL("Peer not found peer id %d\n"),
					peer_id);
			}
			break;
		}


	default:
		break;
	};

	/* Free the indication buffer */
	qdf_nbuf_free(htt_t2h_msg);
}

/*
 * dp_htt_h2t_full() - Send full handler (called from HTC)
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 *
 * Return: enum htc_send_full_action
 */
static enum htc_send_full_action
dp_htt_h2t_full(void *context, HTC_PACKET *pkt)
{
	return HTC_SEND_FULL_KEEP;
}

/*
 * htt_htc_soc_attach() - Register SOC level HTT instance with HTC
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
static int
htt_htc_soc_attach(struct htt_soc *soc)
{
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp response;
	A_STATUS status;

	qdf_mem_set(&connect, sizeof(connect), 0);
	qdf_mem_set(&response, sizeof(response), 0);

	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = soc;
	connect.EpCallbacks.EpTxComplete = dp_htt_h2t_send_complete;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = dp_htt_t2h_msg_handler;

	/* rx buffers currently are provided by HIF, not by EpRecvRefill */
	connect.EpCallbacks.EpRecvRefill = NULL;

	/* N/A, fill is done by HIF */
	connect.EpCallbacks.RecvRefillWaterMark = 1;

	connect.EpCallbacks.EpSendFull = dp_htt_h2t_full;
	/*
	 * Specify how deep to let a queue get before htc_send_pkt will
	 * call the EpSendFull function due to excessive send queue depth.
	 */
	connect.MaxSendQueueDepth = DP_HTT_MAX_SEND_QUEUE_DEPTH;

	/* disable flow control for HTT data message service */
	connect.ConnectionFlags |= HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;

	/* connect to control service */
	connect.service_id = HTT_DATA_MSG_SVC;

	status = htc_connect_service(soc->htc_soc, &connect, &response);

	if (status != A_OK)
		return QDF_STATUS_E_FAILURE;

	soc->htc_endpoint = response.Endpoint;

	return 0; /* success */
}

/*
 * htt_soc_attach() - SOC level HTT initialization
 * @dp_soc:	Opaque Data path SOC handle
 * @osif_soc:	Opaque OSIF SOC handle
 * @htc_soc:	SOC level HTC handle
 * @hal_soc:	Opaque HAL SOC handle
 * @osdev:	QDF device
 *
 * Return: HTT handle on success; NULL on failure
 */
void *
htt_soc_attach(void *dp_soc, void *osif_soc, HTC_HANDLE htc_soc,
	void *hal_soc, qdf_device_t osdev)
{
	struct htt_soc *soc;
	int i;

	soc = qdf_mem_malloc(sizeof(*soc));

	if (!soc)
		goto fail1;

	soc->osdev = osdev;
	soc->osif_soc = osif_soc;
	soc->dp_soc = dp_soc;
	soc->htc_soc = htc_soc;
	soc->hal_soc = hal_soc;

	/* TODO: See if any NSS related context is requred in htt_soc */

	soc->htt_htc_pkt_freelist = NULL;

	if (htt_htc_soc_attach(soc))
		goto fail2;

	/* TODO: See if any Rx data specific intialization is required. For
	 * MCL use cases, the data will be received as single packet and
	 * should not required any descriptor or reorder handling
	 */

	HTT_TX_MUTEX_INIT(&soc->htt_tx_mutex);

	/* pre-allocate some HTC_PACKET objects */
	for (i = 0; i < HTT_HTC_PKT_POOL_INIT_SIZE; i++) {
		struct dp_htt_htc_pkt_union *pkt;
		pkt = qdf_mem_malloc(sizeof(*pkt));
		if (!pkt)
			break;

		htt_htc_pkt_free(soc, &pkt->u.pkt);
	}

	return soc;

fail2:
	qdf_mem_free(soc);

fail1:
	return NULL;
}


/*
 * htt_soc_detach() - Detach SOC level HTT
 * @htt_soc:	HTT SOC handle
 */
void
htt_soc_detach(void *htt_soc)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	htt_htc_pkt_pool_free(soc);
	HTT_TX_MUTEX_DESTROY(&soc->htt_tx_mutex);
	qdf_mem_free(soc);
}

