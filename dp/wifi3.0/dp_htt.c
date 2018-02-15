/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include "htt_stats.h"
#include "htt_ppdu_stats.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "cdp_txrx_cmn_struct.h"

#define HTT_TLV_HDR_LEN HTT_T2H_EXT_STATS_CONF_TLV_HDR_SIZE

#define HTT_HTC_PKT_POOL_INIT_SIZE 64
#define HTT_T2H_MAX_MSG_SIZE 2048

#define HTT_MSG_BUF_SIZE(msg_bytes) \
	((msg_bytes) + HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING)

#define HTT_PID_BIT_MASK 0x3

#define DP_EXT_MSG_LENGTH 2048
#define DP_HTT_SEND_HTC_PKT(soc, pkt)                            \
do {                                                             \
	if (htc_send_pkt(soc->htc_soc, &pkt->htc_pkt) ==         \
					QDF_STATUS_SUCCESS)      \
		htt_htc_misc_pkt_list_add(soc, pkt);             \
} while (0)

#define HTT_MGMT_CTRL_TLV_RESERVERD_LEN 12
/**
 * Bitmap of HTT PPDU TLV types for Default mode
 */
#define HTT_PPDU_DEFAULT_TLV_BITMAP \
	(1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV) | \
	(1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)

/**
 * Bitmap of HTT PPDU TLV types for Sniffer mode
 */
#define HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP \
	(1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV) | \
	(1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV) | \
	(1 << HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV)

#define HTT_FRAMECTRL_DATATYPE 0x08
#define HTT_PPDU_DESC_MAX_DEPTH 16

/*
 * dp_tx_stats_update() - Update per-peer statistics
 * @soc: Datapath soc handle
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 * @ack_rssi: RSSI of last ack received
 *
 * Return: None
 */
#ifdef FEATURE_PERPKT_INFO
static void dp_tx_stats_update(struct dp_soc *soc, struct dp_peer *peer,
		struct cdp_tx_completion_ppdu_user *ppdu, uint32_t ack_rssi)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	uint8_t preamble, mcs;
	uint16_t num_msdu;

	preamble = ppdu->preamble;
	mcs = ppdu->mcs;
	num_msdu = ppdu->num_msdu;

	/* If the peer statistics are already processed as part of
	 * per-MSDU completion handler, do not process these again in per-PPDU
	 * indications */
	if (soc->process_tx_status)
		return;

	DP_STATS_INC_PKT(peer, tx.comp_pkt,
			num_msdu, (ppdu->success_bytes +
				ppdu->retry_bytes + ppdu->failed_bytes));
	DP_STATS_INC(peer, tx.tx_failed, ppdu->failed_msdus);
	DP_STATS_UPD(peer, tx.tx_rate, ppdu->tx_rate);
	DP_STATS_INC(peer, tx.sgi_count[ppdu->gi], num_msdu);
	DP_STATS_INC(peer, tx.bw[ppdu->bw], num_msdu);
	DP_STATS_INC(peer, tx.nss[ppdu->nss], num_msdu);
	DP_STATS_INC(peer, tx.wme_ac_type[TID_TO_WME_AC(ppdu->tid)], num_msdu);
	DP_STATS_INCC(peer, tx.stbc, num_msdu, ppdu->stbc);
	DP_STATS_INCC(peer, tx.ldpc, num_msdu, ppdu->ldpc);
	if (!(ppdu->is_mcast))
		DP_STATS_UPD(peer, tx.last_ack_rssi, ack_rssi);

	DP_STATS_INC(peer, tx.retries,
			(ppdu->long_retries + ppdu->short_retries));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11B) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < (MAX_MCS_11B)) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
			((mcs >= (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer,
			tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
			((mcs < (MAX_MCS - 1)) && (preamble == DOT11_AX)));

	if (soc->cdp_soc.ol_ops->update_dp_stats) {
		soc->cdp_soc.ol_ops->update_dp_stats(pdev->osif_pdev,
				&peer->stats, ppdu->peer_id,
				UPDATE_PEER_STATS);

		dp_aggregate_vdev_stats(peer->vdev);
	}
}
#endif

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
 * htt_htc_misc_pkt_list_trim() - trim misc list
 * @htt_soc: HTT SOC handle
 * @level: max no. of pkts in list
 */
static void
htt_htc_misc_pkt_list_trim(struct htt_soc *soc, int level)
{
	struct dp_htt_htc_pkt_union *pkt, *next, *prev = NULL;
	int i = 0;
	qdf_nbuf_t netbuf;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	pkt = soc->htt_htc_pkt_misclist;
	while (pkt) {
		next = pkt->u.next;
		/* trim the out grown list*/
		if (++i > level) {
			netbuf =
				(qdf_nbuf_t)(pkt->u.pkt.htc_pkt.pNetBufContext);
			qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);
			qdf_nbuf_free(netbuf);
			qdf_mem_free(pkt);
			pkt = NULL;
			if (prev)
				prev->u.next = NULL;
		}
		prev = pkt;
		pkt = next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

/*
 * htt_htc_misc_pkt_list_add() - Add pkt to misc list
 * @htt_soc:	HTT SOC handle
 * @dp_htt_htc_pkt: pkt to be added to list
 */
static void
htt_htc_misc_pkt_list_add(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
				(struct dp_htt_htc_pkt_union *)pkt;
	int misclist_trim_level = htc_get_tx_queue_depth(soc->htc_soc,
							pkt->htc_pkt.Endpoint)
				+ DP_HTT_HTC_PKT_MISCLIST_SIZE;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_misclist) {
		u_pkt->u.next = soc->htt_htc_pkt_misclist;
		soc->htt_htc_pkt_misclist = u_pkt;
	} else {
		soc->htt_htc_pkt_misclist = u_pkt;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	/* only ce pipe size + tx_queue_depth could possibly be in use
	 * free older packets in the misclist
	 */
	htt_htc_misc_pkt_list_trim(soc, misclist_trim_level);
}

/*
 * htt_htc_misc_pkt_pool_free() - free pkts in misc list
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_misc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	qdf_nbuf_t netbuf;

	pkt = soc->htt_htc_pkt_misclist;

	while (pkt) {
		next = pkt->u.next;
		netbuf = (qdf_nbuf_t) (pkt->u.pkt.htc_pkt.pNetBufContext);
		qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);

		soc->stats.htc_pkt_free++;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
			 "%s: Pkt free count %d\n",
			 __func__, soc->stats.htc_pkt_free);

		qdf_nbuf_free(netbuf);
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_misclist = NULL;
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
	DP_HTT_SEND_HTC_PKT(soc, pkt);
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
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF0)) {
			htt_ring_id = HTT_HOST1_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#ifdef IPA_OFFLOAD
		} else if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF2)) {
			htt_ring_id = HTT_HOST2_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#endif
#else
		if (srng_params.ring_id ==
			(HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 +
			  (mac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
#endif
		} else if (srng_params.ring_id ==
#ifdef IPA_OFFLOAD
			 (HAL_SRNG_WMAC1_SW2RXDMA0_BUF1 +
#else
			 (HAL_SRNG_WMAC1_SW2RXDMA1_BUF +
#endif
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
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
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

	if ((htt_ring_type == HTT_SW_TO_HW_RING) ||
			(htt_ring_type == HTT_HW_TO_SW_RING))
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
		HTC_TX_PACKET_TAG_RUNTIME_PUT); /* tag for no FW response msg */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt);

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
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
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

	/*
	 * pdev_id is indexed from 0 whereas mac_id is indexed from 1
	 * SW_TO_SW and SW_TO_HW rings are unaffected by this
	 */
	if (htt_ring_type == HTT_SW_TO_SW_RING ||
			htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_RX_RING_SELECTION_CFG_PDEV_ID_SET(*msg_word,
						DP_SW2HW_MACID(pdev_id));

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
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
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
				MGMT, 0111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
				MGMT, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
				MGMT, 1001, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	/* word 3 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
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
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
	}

	/* word 4 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0001, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0010, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0011, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0100, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0101, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
				CTRL, 0110, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
				CTRL, 0111, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
				CTRL, 1000, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
				CTRL, 1001, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, MCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, UCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, NULL,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
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
				DATA, MCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
				DATA, UCAST, 1);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
				DATA, NULL, 1);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, MCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, UCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, NULL,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
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
		htt_tlv_filter->attention);
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
	/* RESERVED bit maps to header_per_msdu in htt_tlv_filter*/
	 htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, RESERVED,
		 htt_tlv_filter->header_per_msdu);

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
	DP_HTT_SEND_HTC_PKT(soc, pkt);
	return QDF_STATUS_SUCCESS;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

#if defined(CONFIG_WIN) && WDI_EVENT_ENABLE
static inline QDF_STATUS dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
					struct dp_soc *soc, qdf_nbuf_t htt_msg)

{
	uint32_t pdev_id;
	uint32_t *msg_word = NULL;
	uint32_t msg_remain_len = 0;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);

	/*COOKIE MSB*/
	pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;

	/* stats message length + 16 size of HTT header*/
	msg_remain_len = qdf_min(htt_stats->msg_len + 16,
				(uint32_t)DP_EXT_MSG_LENGTH);

	dp_wdi_event_handler(WDI_EVENT_HTT_STATS, soc,
			msg_word,  msg_remain_len,
			WDI_NO_VAL, pdev_id);

	if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
		htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
	}
	/* Need to be freed here as WDI handler will
	 * make a copy of pkt to send data to application
	 */
	qdf_nbuf_free(htt_msg);
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
					struct dp_soc *soc, qdf_nbuf_t htt_msg)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * dp_process_htt_stat_msg(): Process the list of buffers of HTT EXT stats
 * @htt_stats: htt stats info
 *
 * The FW sends the HTT EXT STATS as a stream of T2H messages. Each T2H message
 * contains sub messages which are identified by a TLV header.
 * In this function we will process the stream of T2H messages and read all the
 * TLV contained in the message.
 *
 * THe following cases have been taken care of
 * Case 1: When the tlv_remain_length <= msg_remain_length of HTT MSG buffer
 *		In this case the buffer will contain multiple tlvs.
 * Case 2: When the tlv_remain_length > msg_remain_length of HTT MSG buffer.
 *		Only one tlv will be contained in the HTT message and this tag
 *		will extend onto the next buffer.
 * Case 3: When the buffer is the continuation of the previous message
 * Case 4: tlv length is 0. which will indicate the end of message
 *
 * return: void
 */
static inline void dp_process_htt_stat_msg(struct htt_stats_context *htt_stats,
					struct dp_soc *soc)
{
	htt_tlv_tag_t tlv_type = 0xff;
	qdf_nbuf_t htt_msg = NULL;
	uint32_t *msg_word;
	uint8_t *tlv_buf_head = NULL;
	uint8_t *tlv_buf_tail = NULL;
	uint32_t msg_remain_len = 0;
	uint32_t tlv_remain_len = 0;
	uint32_t *tlv_start;
	int cookie_val;
	int cookie_msb;
	int pdev_id;
	bool copy_stats = false;
	struct dp_pdev *pdev;

	/* Process node in the HTT message queue */
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
		!= NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		cookie_val = *(msg_word + 1);
		if (cookie_val) {
			if (dp_send_htt_stat_resp(htt_stats, soc, htt_msg)
					== QDF_STATUS_SUCCESS) {
				continue;
			}
		}
		cookie_msb = *(msg_word + 2);
		pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;
		pdev = soc->pdev_list[pdev_id];

		if (cookie_msb >> 2) {
			copy_stats = true;
		}
		/* read 5th word */
		msg_word = msg_word + 4;
		msg_remain_len = qdf_min(htt_stats->msg_len,
				(uint32_t) DP_EXT_MSG_LENGTH);
		/* Keep processing the node till node length is 0 */
		while (msg_remain_len) {
			/*
			 * if message is not a continuation of previous message
			 * read the tlv type and tlv length
			 */
			if (!tlv_buf_head) {
				tlv_type = HTT_STATS_TLV_TAG_GET(
						*msg_word);
				tlv_remain_len = HTT_STATS_TLV_LENGTH_GET(
						*msg_word);
			}

			if (tlv_remain_len == 0) {
				msg_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

				goto error;
			}

			if (!tlv_buf_head)
				tlv_remain_len += HTT_TLV_HDR_LEN;

			if ((tlv_remain_len <= msg_remain_len)) {
				/* Case 3 */
				if (tlv_buf_head) {
					qdf_mem_copy(tlv_buf_tail,
							(uint8_t *)msg_word,
							tlv_remain_len);
					tlv_start = (uint32_t *)tlv_buf_head;
				} else {
					/* Case 1 */
					tlv_start = msg_word;
				}

				if (copy_stats)
					dp_htt_stats_copy_tag(pdev, tlv_type, tlv_start);
				else
					dp_htt_stats_print_tag(tlv_type, tlv_start);

				msg_remain_len -= tlv_remain_len;

				msg_word = (uint32_t *)
					(((uint8_t *)msg_word) +
					tlv_remain_len);

				tlv_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

			} else { /* tlv_remain_len > msg_remain_len */
				/* Case 2 & 3 */
				if (!tlv_buf_head) {
					tlv_buf_head = qdf_mem_malloc(
							tlv_remain_len);

					if (!tlv_buf_head) {
						QDF_TRACE(QDF_MODULE_ID_TXRX,
								QDF_TRACE_LEVEL_ERROR,
								"Alloc failed");
						goto error;
					}

					tlv_buf_tail = tlv_buf_head;
				}

				qdf_mem_copy(tlv_buf_tail, (uint8_t *)msg_word,
						msg_remain_len);
				tlv_remain_len -= msg_remain_len;
				tlv_buf_tail += msg_remain_len;
			}
		}

		if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
			htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
		}

		qdf_nbuf_free(htt_msg);
	}
	return;

error:
	qdf_nbuf_free(htt_msg);
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
			!= NULL)
		qdf_nbuf_free(htt_msg);
}

void htt_t2h_stats_handler(void *context)
{
	struct dp_soc *soc = (struct dp_soc *)context;
	struct htt_stats_context htt_stats;
	uint32_t length;
	uint32_t *msg_word;
	qdf_nbuf_t htt_msg = NULL;
	uint8_t done;
	uint8_t rem_stats;

	if (!soc || !qdf_atomic_read(&soc->cmn_init_done)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"soc: 0x%pK, init_done: %d", soc,
			qdf_atomic_read(&soc->cmn_init_done));
		return;
	}

	qdf_mem_zero(&htt_stats, sizeof(htt_stats));
	qdf_nbuf_queue_init(&htt_stats.msg);

	/* pull one completed stats from soc->htt_stats_msg and process */
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	if (!soc->htt_stats.num_stats) {
		qdf_spin_unlock_bh(&soc->htt_stats.lock);
		return;
	}
	while ((htt_msg = qdf_nbuf_queue_remove(&soc->htt_stats.msg)) != NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		msg_word = msg_word + HTT_T2H_EXT_STATS_TLV_START_OFFSET;
		length = HTT_T2H_EXT_STATS_CONF_TLV_LENGTH_GET(*msg_word);
		done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);
		qdf_nbuf_queue_add(&htt_stats.msg, htt_msg);
		/*
		 * HTT EXT stats response comes as stream of TLVs which span over
		 * multiple T2H messages.
		 * The first message will carry length of the response.
		 * For rest of the messages length will be zero.
		 */
		if (length)
			htt_stats.msg_len = length;
		/*
		 * Done bit signifies that this is the last T2H buffer in the
		 * stream of HTT EXT STATS message
		 */
		if (done)
			break;
	}
	rem_stats = --soc->htt_stats.num_stats;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	dp_process_htt_stat_msg(&htt_stats, soc);
	/* If there are more stats to process, schedule stats work again */
	if (rem_stats)
		qdf_sched_work(0, &soc->htt_stats.work);
}

/*
 * dp_get_ppdu_info_user_index: Find and allocate a per-user descriptor for a PPDU,
 * if a new peer id arrives in a PPDU
 * pdev: DP pdev handle
 * @peer_id : peer unique identifier
 * @ppdu_info: per ppdu tlv structure
 *
 * return:user index to be populated
 */
#ifdef FEATURE_PERPKT_INFO
static uint8_t dp_get_ppdu_info_user_index(struct dp_pdev *pdev,
						uint16_t peer_id,
						struct ppdu_info *ppdu_info)
{
	uint8_t user_index = 0;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	while ((user_index + 1) <= ppdu_info->last_user) {
		ppdu_user_desc = &ppdu_desc->user[user_index];
		if (ppdu_user_desc->peer_id != peer_id) {
			user_index++;
			continue;
		} else {
			/* Max users possible is 8 so user array index should
			 * not exceed 7
			 */
			qdf_assert_always(user_index <= CDP_MU_MAX_USER_INDEX);
			return user_index;
		}
	}

	ppdu_info->last_user++;
	/* Max users possible is 8 so last user should not exceed 8 */
	qdf_assert_always(ppdu_info->last_user <= CDP_MU_MAX_USERS);
	return ppdu_info->last_user - 1;
}

/*
 * dp_process_ppdu_stats_common_tlv: Process htt_ppdu_stats_common_tlv
 * pdev: DP pdev handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_common_tlv(struct dp_pdev *pdev,
		uint32_t *tag_buf, struct ppdu_info *ppdu_info)
{
	uint16_t frame_type;
	uint16_t freq;
	struct dp_soc *soc = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf += 2;
	ppdu_desc->num_users =
		HTT_PPDU_STATS_COMMON_TLV_NUM_USERS_GET(*tag_buf);
	tag_buf++;
	frame_type = HTT_PPDU_STATS_COMMON_TLV_FRM_TYPE_GET(*tag_buf);

	if ((frame_type == HTT_STATS_FTYPE_TIDQ_DATA_SU) ||
			(frame_type == HTT_STATS_FTYPE_TIDQ_DATA_MU))
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_DATA;
	else
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;

	tag_buf += 2;
	ppdu_desc->tx_duration = *tag_buf;
	tag_buf += 3;
	ppdu_desc->ppdu_start_timestamp = *tag_buf;

	ppdu_desc->ppdu_end_timestamp = ppdu_desc->ppdu_start_timestamp +
					ppdu_desc->tx_duration;

	tag_buf++;

	freq = HTT_PPDU_STATS_COMMON_TLV_CHAN_MHZ_GET(*tag_buf);
	if (freq != ppdu_desc->channel) {
		soc = pdev->soc;
		ppdu_desc->channel = freq;
		if (soc && soc->cdp_soc.ol_ops->freq_to_channel)
			pdev->operating_channel =
		soc->cdp_soc.ol_ops->freq_to_channel(pdev->osif_pdev, freq);
	}

	ppdu_desc->phy_mode = HTT_PPDU_STATS_COMMON_TLV_PHY_MODE_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_user_common_tlv: Process ppdu_stats_user_common
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_COMMON_TLV_SW_PEER_ID_GET(*tag_buf);
	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];

	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;

	if (HTT_PPDU_STATS_USER_COMMON_TLV_MCAST_GET(*tag_buf)) {
		ppdu_user_desc->is_mcast = true;
		ppdu_user_desc->mpdu_tried_mcast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
	} else {
		ppdu_user_desc->mpdu_tried_ucast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
	}

	tag_buf++;

	ppdu_user_desc->qos_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_QOS_CTRL_GET(*tag_buf);
	ppdu_user_desc->frame_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_FRAME_CTRL_GET(*tag_buf);
	ppdu_desc->frame_ctrl = ppdu_user_desc->frame_ctrl;
}


/**
 * dp_process_ppdu_stats_user_rate_tlv() - Process htt_ppdu_stats_user_rate_tlv
 * @pdev: DP pdev handle
 * @tag_buf: T2H message buffer carrying the user rate TLV
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_rate_tlv(struct dp_pdev *pdev,
		uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_RATE_TLV_SW_PEER_ID_GET(*tag_buf);
	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_RATE_TLV_TID_NUM_GET(*tag_buf);

	qdf_mem_copy(ppdu_user_desc->mac_addr, peer->mac_addr.raw,
			DP_MAC_ADDR_LEN);

	tag_buf += 2;

	ppdu_user_desc->ru_tones = (HTT_PPDU_STATS_USER_RATE_TLV_RU_END_GET(*tag_buf) -
			HTT_PPDU_STATS_USER_RATE_TLV_RU_START_GET(*tag_buf)) + 1;

	tag_buf += 2;

	ppdu_user_desc->ppdu_type =
		HTT_PPDU_STATS_USER_RATE_TLV_PPDU_TYPE_GET(*tag_buf);

	tag_buf++;
	ppdu_user_desc->tx_rate = *tag_buf;

	ppdu_user_desc->ltf_size =
		HTT_PPDU_STATS_USER_RATE_TLV_LTF_SIZE_GET(*tag_buf);
	ppdu_user_desc->stbc =
		HTT_PPDU_STATS_USER_RATE_TLV_STBC_GET(*tag_buf);
	ppdu_user_desc->he_re =
		HTT_PPDU_STATS_USER_RATE_TLV_HE_RE_GET(*tag_buf);
	ppdu_user_desc->txbf =
		HTT_PPDU_STATS_USER_RATE_TLV_TXBF_GET(*tag_buf);
	ppdu_user_desc->bw =
		HTT_PPDU_STATS_USER_RATE_TLV_BW_GET(*tag_buf);
	ppdu_user_desc->nss = HTT_PPDU_STATS_USER_RATE_TLV_NSS_GET(*tag_buf);
	ppdu_user_desc->mcs = HTT_PPDU_STATS_USER_RATE_TLV_MCS_GET(*tag_buf);
	ppdu_user_desc->preamble =
		HTT_PPDU_STATS_USER_RATE_TLV_PREAMBLE_GET(*tag_buf);
	ppdu_user_desc->gi = HTT_PPDU_STATS_USER_RATE_TLV_GI_GET(*tag_buf);
	ppdu_user_desc->dcm = HTT_PPDU_STATS_USER_RATE_TLV_DCM_GET(*tag_buf);
	ppdu_user_desc->ldpc = HTT_PPDU_STATS_USER_RATE_TLV_LDPC_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	struct dp_peer *peer;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
					CDP_BA_64_BIT_MAP_SIZE_DWORDS);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	struct dp_peer *peer;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
					CDP_BA_256_BIT_MAP_SIZE_DWORDS);
}

/*
 * dp_process_ppdu_stats_user_cmpltn_common_tlv: Process
 * htt_ppdu_stats_user_cmpltn_common_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_cmpltn_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_cmpltn_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	htt_ppdu_stats_user_cmpltn_common_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_cmpltn_common_tlv *)tag_buf;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SW_PEER_ID_GET(*tag_buf);
	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->completion_status =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_COMPLETION_STATUS_GET(
				*tag_buf);

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_TID_NUM_GET(*tag_buf);


	tag_buf++;
	ppdu_desc->ack_rssi = dp_stats_buf->ack_rssi;

	tag_buf++;

	ppdu_user_desc->mpdu_success =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPDU_SUCCESS_GET(*tag_buf);

	tag_buf++;

	ppdu_user_desc->long_retries =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_LONG_RETRY_GET(*tag_buf);

	ppdu_user_desc->short_retries =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SHORT_RETRY_GET(*tag_buf);
	ppdu_user_desc->retry_msdus =
		ppdu_user_desc->long_retries + ppdu_user_desc->short_retries;

	ppdu_user_desc->is_ampdu =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_IS_AMPDU_GET(*tag_buf);
	ppdu_info->is_ampdu = ppdu_user_desc->is_ampdu;

}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	struct dp_peer *peer;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
			CDP_BA_64_BIT_MAP_SIZE_DWORDS);
}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	struct dp_peer *peer;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
			CDP_BA_256_BIT_MAP_SIZE_DWORDS);
}

/*
 * dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv: Process
 * htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * pdev: DP PDE handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf += 2;
	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_SW_PEER_ID_GET(*tag_buf);


	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer)
		return;

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;
	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_TID_NUM_GET(*tag_buf);
	ppdu_user_desc->num_mpdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MPDU_GET(*tag_buf);

	ppdu_user_desc->num_msdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MSDU_GET(*tag_buf);

	ppdu_user_desc->success_msdus = ppdu_user_desc->num_msdu;

	tag_buf += 2;
	ppdu_user_desc->success_bytes = *tag_buf;

}

/*
 * dp_process_ppdu_stats_user_common_array_tlv: Process
 * htt_ppdu_stats_user_common_array_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_array_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint32_t peer_id;
	struct dp_peer *peer;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct htt_tx_ppdu_stats_info *dp_stats_buf;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	dp_stats_buf = (struct htt_tx_ppdu_stats_info *)tag_buf;
	tag_buf += 3;
	peer_id =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_PEERID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid peer");
		return;
	}

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];

	ppdu_user_desc->retry_bytes = dp_stats_buf->tx_retry_bytes;
	ppdu_user_desc->failed_bytes = dp_stats_buf->tx_failed_bytes;

	tag_buf++;

	ppdu_user_desc->success_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_SUCC_MSDUS_GET(*tag_buf);
	ppdu_user_desc->retry_bytes =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_RETRY_MSDUS_GET(*tag_buf);
	tag_buf++;
	ppdu_user_desc->failed_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_FAILED_MSDUS_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_flush_tlv: Process
 * htt_ppdu_stats_flush_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_flush_tlv
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_flush_tlv(struct dp_pdev *pdev,
						uint32_t *tag_buf)
{
	uint32_t peer_id;
	uint32_t drop_reason;
	uint8_t tid;
	uint32_t num_msdu;
	struct dp_peer *peer;

	tag_buf++;
	drop_reason = *tag_buf;

	tag_buf++;
	num_msdu = HTT_PPDU_STATS_FLUSH_TLV_NUM_MSDU_GET(*tag_buf);

	tag_buf++;
	peer_id =
		HTT_PPDU_STATS_FLUSH_TLV_SW_PEER_ID_GET(*tag_buf);

	peer = dp_peer_find_by_id(pdev->soc, peer_id);
	if (!peer)
		return;

	tid = HTT_PPDU_STATS_FLUSH_TLV_TID_NUM_GET(*tag_buf);

	if (drop_reason == HTT_FLUSH_EXCESS_RETRIES) {
		DP_STATS_INC(peer, tx.excess_retries_per_ac[TID_TO_WME_AC(tid)],
					num_msdu);
	}
}

/*
 * dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv: Process
 * htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @length: tlv_length
 *
 * return:void
 */
static void dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv(
	struct dp_pdev *pdev, uint32_t *tag_buf, uint32_t length)
{
	htt_ppdu_stats_tx_mgmtctrl_payload_tlv *dp_stats_buf;
	qdf_nbuf_t nbuf;
	uint32_t payload_size;

	if ((!pdev->tx_sniffer_enable) && (!pdev->mcopy_mode))
		return;

	payload_size = length - HTT_MGMT_CTRL_TLV_RESERVERD_LEN;
	nbuf = NULL;
	dp_stats_buf = (htt_ppdu_stats_tx_mgmtctrl_payload_tlv *)tag_buf;


	nbuf = qdf_nbuf_alloc(pdev->soc->osdev, payload_size, 0, 4, true);

	if (!nbuf) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Nbuf Allocation failed for Mgmt. payload");
		qdf_assert(0);
		return;
	}

	qdf_nbuf_put_tail(nbuf, payload_size);
	qdf_mem_copy(qdf_nbuf_data(nbuf), dp_stats_buf->payload, payload_size);

	dp_wdi_event_handler(WDI_EVENT_TX_MGMT_CTRL, pdev->soc,
		nbuf, HTT_INVALID_PEER,
		WDI_NO_VAL, pdev->pdev_id);
}

/**
 * dp_process_ppdu_tag(): Function to process the PPDU TLVs
 * @pdev: DP pdev handle
 * @tag_buf: TLV buffer
 * @tlv_len: length of tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return: void
 */
static void dp_process_ppdu_tag(struct dp_pdev *pdev, uint32_t *tag_buf,
		uint32_t tlv_len, struct ppdu_info *ppdu_info)
{
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	switch (tlv_type) {
	case HTT_PPDU_STATS_COMMON_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_common_tlv));
		dp_process_ppdu_stats_common_tlv(pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_user_common_tlv));
		dp_process_ppdu_stats_user_common_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_RATE_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_user_rate_tlv));
		dp_process_ppdu_stats_user_rate_tlv(pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv));
		dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_256_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv));
		dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV:
		qdf_assert_always(tlv_len ==
				sizeof(htt_ppdu_stats_user_cmpltn_common_tlv));
		dp_process_ppdu_stats_user_cmpltn_common_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV:
		qdf_assert_always(tlv_len ==
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv));
		dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV:
		qdf_assert_always(tlv_len ==
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv));
		dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV:
		qdf_assert_always(tlv_len ==
			sizeof(htt_ppdu_stats_user_compltn_ack_ba_status_tlv));
		dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_ARRAY_TLV:
		qdf_assert_always(tlv_len ==
			sizeof(htt_ppdu_stats_usr_common_array_tlv_v));
		dp_process_ppdu_stats_user_common_array_tlv(
				pdev, tag_buf, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV:
		qdf_assert_always(tlv_len ==
			sizeof(htt_ppdu_stats_flush_tlv));
		dp_process_ppdu_stats_user_compltn_flush_tlv(
				pdev, tag_buf);
		break;
	case HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV:
		dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv(
				pdev, tag_buf, tlv_len);
		break;
	default:
		break;
	}
}

/**
 * dp_ppdu_desc_deliver(): Function to deliver Tx PPDU status descriptor
 * to upper layer
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
static
void dp_ppdu_desc_deliver(struct dp_pdev *pdev,
			      struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	struct dp_peer *peer = NULL;
	qdf_nbuf_t nbuf;
	uint16_t i;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(ppdu_info->nbuf);

	ppdu_desc->num_users = ppdu_info->last_user;
	ppdu_desc->ppdu_id = ppdu_info->ppdu_id;

	for (i = 0; i < ppdu_desc->num_users; i++) {


		ppdu_desc->num_mpdu += ppdu_desc->user[i].num_mpdu;
		ppdu_desc->num_msdu += ppdu_desc->user[i].num_msdu;

		if (ppdu_desc->user[i].tid < CDP_DATA_TID_MAX) {
			peer = dp_peer_find_by_id(pdev->soc,
					ppdu_desc->user[i].peer_id);
			/**
			 * This check is to make sure peer is not deleted
			 * after processing the TLVs.
			 */
			if (!peer)
				continue;

			dp_tx_stats_update(pdev->soc, peer,
					&ppdu_desc->user[i],
					ppdu_desc->ack_rssi);
		}
	}

	/*
	 * Remove from the list
	 */
	TAILQ_REMOVE(&pdev->ppdu_info_list, ppdu_info, ppdu_info_list_elem);
	nbuf = ppdu_info->nbuf;
	pdev->list_depth--;
	qdf_mem_free(ppdu_info);

	qdf_assert_always(nbuf);

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(nbuf);

	/**
	 * Deliver PPDU stats only for valid (acked) data frames if
	 * sniffer mode is not enabled.
	 * If sniffer mode is enabled, PPDU stats for all frames
	 * including mgmt/control frames should be delivered to upper layer
	 */
	if (pdev->tx_sniffer_enable || pdev->mcopy_mode) {
		dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC, pdev->soc,
				nbuf, HTT_INVALID_PEER,
				WDI_NO_VAL, pdev->pdev_id);
	} else {
		if (ppdu_desc->num_mpdu != 0 && ppdu_desc->num_users != 0 &&
				ppdu_desc->frame_ctrl & HTT_FRAMECTRL_DATATYPE) {

			dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC,
					pdev->soc, nbuf, HTT_INVALID_PEER,
					WDI_NO_VAL, pdev->pdev_id);
		} else
			qdf_nbuf_free(nbuf);
	}
	return;
}

/**
 * dp_get_ppdu_desc(): Function to allocate new PPDU status
 * desc for new ppdu id
 * @pdev: DP pdev handle
 * @ppdu_id: PPDU unique identifier
 * @tlv_type: TLV type received
 *
 * return: ppdu_info per ppdu tlv structure
 */
static
struct ppdu_info *dp_get_ppdu_desc(struct dp_pdev *pdev, uint32_t ppdu_id,
			uint8_t tlv_type)
{
	struct ppdu_info *ppdu_info = NULL;

	/*
	 * Find ppdu_id node exists or not
	 */
	TAILQ_FOREACH(ppdu_info, &pdev->ppdu_info_list, ppdu_info_list_elem) {

		if (ppdu_info && (ppdu_info->ppdu_id == ppdu_id)) {
			break;
		}
	}

	if (ppdu_info) {
		/**
		 * if we get tlv_type that is already been processed for ppdu,
		 * that means we got a new ppdu with same ppdu id.
		 * Hence Flush the older ppdu
		 */
		if (ppdu_info->tlv_bitmap & (1 << tlv_type))
			dp_ppdu_desc_deliver(pdev, ppdu_info);
		else
			return ppdu_info;
	}

	/**
	 * Flush the head ppdu descriptor if ppdu desc list reaches max
	 * threshold
	 */
	if (pdev->list_depth > HTT_PPDU_DESC_MAX_DEPTH) {
		ppdu_info = TAILQ_FIRST(&pdev->ppdu_info_list);
		dp_ppdu_desc_deliver(pdev, ppdu_info);
	}

	/*
	 * Allocate new ppdu_info node
	 */
	ppdu_info = qdf_mem_malloc(sizeof(struct ppdu_info));
	if (!ppdu_info)
		return NULL;

	ppdu_info->nbuf = qdf_nbuf_alloc(pdev->soc->osdev,
			sizeof(struct cdp_tx_completion_ppdu), 0, 4,
			TRUE);
	if (!ppdu_info->nbuf) {
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	qdf_mem_zero(qdf_nbuf_data(ppdu_info->nbuf),
			sizeof(struct cdp_tx_completion_ppdu));

	if (qdf_nbuf_put_tail(ppdu_info->nbuf,
			sizeof(struct cdp_tx_completion_ppdu)) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"No tailroom for HTT PPDU");
		qdf_nbuf_free(ppdu_info->nbuf);
		ppdu_info->nbuf = NULL;
		ppdu_info->last_user = 0;
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	/**
	 * No lock is needed because all PPDU TLVs are processed in
	 * same context and this list is updated in same context
	 */
	TAILQ_INSERT_TAIL(&pdev->ppdu_info_list, ppdu_info,
			ppdu_info_list_elem);
	pdev->list_depth++;
	return ppdu_info;
}

/**
 * dp_htt_process_tlv(): Function to process each PPDU TLVs
 * @pdev: DP pdev handle
 * @htt_t2h_msg: HTT target to host message
 *
 * return: ppdu_info per ppdu tlv structure
 */

static struct ppdu_info *dp_htt_process_tlv(struct dp_pdev *pdev,
		qdf_nbuf_t htt_t2h_msg)
{
	uint32_t length;
	uint32_t ppdu_id;
	uint8_t tlv_type;
	uint32_t tlv_length, tlv_bitmap_expected;
	uint8_t *tlv_buf;
	struct ppdu_info *ppdu_info = NULL;

	uint32_t *msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);

	length = HTT_T2H_PPDU_STATS_PAYLOAD_SIZE_GET(*msg_word);

	msg_word = msg_word + 1;
	ppdu_id = HTT_T2H_PPDU_STATS_PPDU_ID_GET(*msg_word);


	msg_word = msg_word + 3;
	while (length > 0) {
		tlv_buf = (uint8_t *)msg_word;
		tlv_type = HTT_STATS_TLV_TAG_GET(*msg_word);
		tlv_length = HTT_STATS_TLV_LENGTH_GET(*msg_word);
		if (qdf_likely(tlv_type < CDP_PPDU_STATS_MAX_TAG))
			pdev->stats.ppdu_stats_counter[tlv_type]++;

		if (tlv_length == 0)
			break;

		tlv_length += HTT_TLV_HDR_LEN;

		/**
		 * Not allocating separate ppdu descriptor for MGMT Payload
		 * TLV as this is sent as separate WDI indication and it
		 * doesn't contain any ppdu information
		 */
		if (tlv_type == HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV) {
			dp_process_ppdu_tag(pdev, msg_word, tlv_length, NULL);
			msg_word = (uint32_t *)((uint8_t *)tlv_buf + tlv_length);
			length -= (tlv_length);
			continue;
		}

		ppdu_info = dp_get_ppdu_desc(pdev, ppdu_id, tlv_type);
		if (!ppdu_info)
			return NULL;
		ppdu_info->ppdu_id = ppdu_id;
		ppdu_info->tlv_bitmap |= (1 << tlv_type);

		dp_process_ppdu_tag(pdev, msg_word, tlv_length, ppdu_info);

		/**
		 * Increment pdev level tlv count to monitor
		 * missing TLVs
		 */
		pdev->tlv_count++;
		ppdu_info->last_tlv_cnt = pdev->tlv_count;

		msg_word = (uint32_t *)((uint8_t *)tlv_buf + tlv_length);
		length -= (tlv_length);
	}

	pdev->last_ppdu_id = ppdu_id;

	tlv_bitmap_expected = HTT_PPDU_DEFAULT_TLV_BITMAP;

	if (pdev->tx_sniffer_enable || pdev->mcopy_mode) {
		if (ppdu_info->is_ampdu)
			tlv_bitmap_expected = HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP;
	}

	/**
	 * Once all the TLVs for a given PPDU has been processed,
	 * return PPDU status to be delivered to higher layer
	 */
	if (ppdu_info->tlv_bitmap == tlv_bitmap_expected)
		return ppdu_info;

	return NULL;
}
#endif /* FEATURE_PERPKT_INFO */

/**
 * dp_txrx_ppdu_stats_handler() - Function to process HTT PPDU stats from FW
 * @soc: DP SOC handle
 * @pdev_id: pdev id
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
#if defined(WDI_EVENT_ENABLE)
#ifdef FEATURE_PERPKT_INFO
static void dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
		uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{
	struct dp_pdev *pdev = soc->pdev_list[pdev_id];
	struct ppdu_info *ppdu_info = NULL;

	if (!pdev->enhanced_stats_en && !pdev->tx_sniffer_enable &&
			!pdev->mcopy_mode)
		return;

	ppdu_info = dp_htt_process_tlv(pdev, htt_t2h_msg);
	if (ppdu_info)
		dp_ppdu_desc_deliver(pdev, ppdu_info);
}
#else
static void dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
		uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{

}
#endif
#endif

/**
 * dp_txrx_fw_stats_handler() - Function to process HTT EXT stats
 * @soc: DP SOC handle
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
static inline void dp_txrx_fw_stats_handler(struct dp_soc *soc,
		qdf_nbuf_t htt_t2h_msg)
{
	uint8_t done;
	qdf_nbuf_t msg_copy;
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_word = msg_word + 3;
	done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);

	/*
	 * HTT EXT stats response comes as stream of TLVs which span over
	 * multiple T2H messages.
	 * The first message will carry length of the response.
	 * For rest of the messages length will be zero.
	 *
	 * Clone the T2H message buffer and store it in a list to process
	 * it later.
	 *
	 * The original T2H message buffers gets freed in the T2H HTT event
	 * handler
	 */
	msg_copy = qdf_nbuf_clone(htt_t2h_msg);

	if (!msg_copy) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
				"T2H messge clone failed for HTT EXT STATS");
		goto error;
	}

	qdf_spin_lock_bh(&soc->htt_stats.lock);
	qdf_nbuf_queue_add(&soc->htt_stats.msg, msg_copy);
	/*
	 * Done bit signifies that this is the last T2H buffer in the stream of
	 * HTT EXT STATS message
	 */
	if (done) {
		soc->htt_stats.num_stats++;
		qdf_sched_work(0, &soc->htt_stats.work);
	}
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	return;

error:
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	while ((msg_copy = qdf_nbuf_queue_remove(&soc->htt_stats.msg))
			!= NULL) {
		qdf_nbuf_free(msg_copy);
	}
	soc->htt_stats.num_stats = 0;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);
	return;

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


#if defined(WDI_EVENT_ENABLE) && !defined(REMOVE_PKT_LOG)
/*
 * dp_ppdu_stats_ind_handler() - PPDU stats msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 * @htt_t2h_msg: HTT msg nbuf
 *
 * Return: None
 */
static void
dp_ppdu_stats_ind_handler(struct htt_soc *soc,
				uint32_t *msg_word,
				qdf_nbuf_t htt_t2h_msg)
{
	u_int8_t pdev_id;
	qdf_nbuf_set_pktlen(htt_t2h_msg, HTT_T2H_MAX_MSG_SIZE);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"received HTT_T2H_MSG_TYPE_PPDU_STATS_IND\n");
	pdev_id = HTT_T2H_PPDU_STATS_PDEV_ID_GET(*msg_word);
	pdev_id = DP_HW2SW_MACID(pdev_id);
	dp_txrx_ppdu_stats_handler(soc->dp_soc, pdev_id,
				  htt_t2h_msg);
	dp_wdi_event_handler(WDI_EVENT_LITE_T2H, soc->dp_soc,
		htt_t2h_msg, HTT_INVALID_PEER, WDI_NO_VAL,
		pdev_id);
}
#else
dp_ppdu_stats_ind_handler(struct htt_soc *soc,
				qdf_nbuf_t htt_t2h_msg)
{
}
#endif

#if defined(WDI_EVENT_ENABLE) && \
		!defined(REMOVE_PKT_LOG) && defined(CONFIG_WIN)
/*
 * dp_pktlog_msg_handler() - Pktlog msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 *
 * Return: None
 */
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
				uint32_t *msg_word)
{
	uint8_t pdev_id;
	uint32_t *pl_hdr;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"received HTT_T2H_MSG_TYPE_PKTLOG\n");
	pdev_id = HTT_T2H_PKTLOG_PDEV_ID_GET(*msg_word);
	pdev_id = DP_HW2SW_MACID(pdev_id);
	pl_hdr = (msg_word + 1);
	dp_wdi_event_handler(WDI_EVENT_OFFLOAD_ALL, soc->dp_soc,
		pl_hdr, HTT_INVALID_PEER, WDI_NO_VAL,
		pdev_id);
}
#else
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
				uint32_t *msg_word)
{
}
#endif

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
	if (pkt->Status != QDF_STATUS_SUCCESS) {
		if (pkt->Status != QDF_STATUS_E_CANCELED)
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

	case HTT_T2H_MSG_TYPE_PPDU_STATS_IND:
		{
			dp_ppdu_stats_ind_handler(soc, msg_word, htt_t2h_msg);
			break;
		}

	case HTT_T2H_MSG_TYPE_PKTLOG:
		{
			dp_pktlog_msg_handler(soc, msg_word);
			break;
		}

	case HTT_T2H_MSG_TYPE_VERSION_CONF:
		{
			htc_pm_runtime_put(soc->htc_soc);
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
	case HTT_T2H_MSG_TYPE_EXT_STATS_CONF:
		{
			dp_txrx_fw_stats_handler(soc->dp_soc, htt_t2h_msg);
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
 * dp_htt_hif_t2h_hp_callback() - HIF callback for high priority T2H messages
 * @context:	Opaque context (HTT SOC handle)
 * @nbuf:	nbuf containing T2H message
 * @pipe_id:	HIF pipe ID
 *
 * Return: QDF_STATUS
 *
 * TODO: Temporary change to bypass HTC connection for this new HIF pipe, which
 * will be used for packet log and other high-priority HTT messsages. Proper
 * HTC connection to be added later once required FW changes are available
 */
static QDF_STATUS
dp_htt_hif_t2h_hp_callback (void *context, qdf_nbuf_t nbuf, uint8_t pipe_id)
{
	A_STATUS rc = QDF_STATUS_SUCCESS;
	HTC_PACKET htc_pkt;

	qdf_assert_always(pipe_id == DP_HTT_T2H_HP_PIPE);
	qdf_mem_zero(&htc_pkt, sizeof(htc_pkt));
	htc_pkt.Status = QDF_STATUS_SUCCESS;
	htc_pkt.pPktContext = (void *)nbuf;
	dp_htt_t2h_msg_handler(context, &htc_pkt);

	return rc;
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
	struct dp_soc *dpsoc = soc->dp_soc;

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

	hif_save_htc_htt_config_endpoint(dpsoc->hif_handle, soc->htc_endpoint);
	dp_hif_update_pipe_callback(soc->dp_soc, (void *)soc,
		dp_htt_hif_t2h_hp_callback, DP_HTT_T2H_HP_PIPE);

	return 0; /* success */
}

/*
 * htt_soc_attach() - SOC level HTT initialization
 * @dp_soc:	Opaque Data path SOC handle
 * @ctrl_psoc:	Opaque ctrl SOC handle
 * @htc_soc:	SOC level HTC handle
 * @hal_soc:	Opaque HAL SOC handle
 * @osdev:	QDF device
 *
 * Return: HTT handle on success; NULL on failure
 */
void *
htt_soc_attach(void *dp_soc, void *ctrl_psoc, HTC_HANDLE htc_soc,
	void *hal_soc, qdf_device_t osdev)
{
	struct htt_soc *soc;
	int i;

	soc = qdf_mem_malloc(sizeof(*soc));

	if (!soc)
		goto fail1;

	soc->osdev = osdev;
	soc->ctrl_psoc = ctrl_psoc;
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

	htt_htc_misc_pkt_pool_free(soc);
	htt_htc_pkt_pool_free(soc);
	HTT_TX_MUTEX_DESTROY(&soc->htt_tx_mutex);
	qdf_mem_free(soc);
}

/*
 * dp_get_pdev_mask_for_channel_id() - Retrieve pdev_id mask based on channel
 * information
 * @pdev - DP PDEV Handle
 * @channel - frequency
 *
 * Return - Pdev_id mask
 */
static inline
uint8_t dp_get_pdev_mask_for_channel_id(struct dp_pdev *pdev, uint8_t channel)
{
	uint8_t pdev_mask = 0;

	if (!channel)
		return 1 << (pdev->pdev_id + 1);

	else if (channel && WLAN_CHAN_IS_5GHZ(channel))
		pdev_mask = 0;

	else if (channel && WLAN_CHAN_IS_2GHZ(channel))
		pdev_mask = 1;

	return 1 << (pdev_mask + 1);
}

/**
 * dp_h2t_ext_stats_msg_send(): function to contruct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @config_param_0: extra configuration parameters
 * @config_param_1: extra configuration parameters
 * @config_param_2: extra configuration parameters
 * @config_param_3: extra configuration parameters
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_ext_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint32_t config_param_0,
		uint32_t config_param_1, uint32_t config_param_2,
		uint32_t config_param_3, int cookie_val, int cookie_msb,
		uint8_t channel)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask = 0;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_EXT_STATS_REQ_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	pdev_mask = dp_get_pdev_mask_for_channel_id(pdev, channel);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_EXT_STATS_REQ_MSG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Failed to expand head for HTT_EXT_STATS");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		"-----%s:%d----\n cookie <-> %d\n config_param_0 %u\n"
		"config_param_1 %u\n config_param_2 %u\n"
		"config_param_4 %u\n -------------\n",
		__func__, __LINE__, cookie_val, config_param_0,
		config_param_1, config_param_2,	config_param_3);

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_EXT_STATS_REQ);
	HTT_H2T_EXT_STATS_REQ_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_EXT_STATS_REQ_STATS_TYPE_SET(*msg_word, stats_type_upload_mask);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_0);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_1);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_2);

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_3);

	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, 0);

	/* word 5 */
	msg_word++;

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_val);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	/*Using last 2 bits for pdev_id */
	cookie_msb = ((cookie_msb << 2) | pdev->pdev_id);
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_msb);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt);
	return 0;
}

/* This macro will revert once proper HTT header will define for
 * HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in htt.h file
 * */
#if defined(WDI_EVENT_ENABLE)
/**
 * dp_h2t_cfg_stats_msg_send(): function to construct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @mac_id: Mac id number
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_PPDU_STATS_CFG_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, true);

	if (!msg) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Fail to allocate HTT_H2T_PPDU_STATS_CFG_MSG_SZ msg buffer\n");
		qdf_assert(0);
		return QDF_STATUS_E_NOMEM;
	}

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	pdev_mask = 1 << DP_SW2HW_MACID(mac_id);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_PPDU_STATS_CFG_MSG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Failed to expand head for HTT_CFG_STATS\n");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_PPDU_STATS_CFG);
	HTT_H2T_PPDU_STATS_CFG_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_PPDU_STATS_CFG_TLV_BITMASK_SET(*msg_word,
			stats_type_upload_mask);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Fail to allocate dp_htt_htc_pkt buffer\n");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt);
	return 0;
}
#endif
