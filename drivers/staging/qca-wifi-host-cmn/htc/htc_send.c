/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "htc_debug.h"
#include "htc_internal.h"
#include "htc_credit_history.h"
#include <qdf_mem.h>            /* qdf_mem_malloc */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include "qdf_module.h"

/* #define USB_HIF_SINGLE_PIPE_DATA_SCHED */
/* #ifdef USB_HIF_SINGLE_PIPE_DATA_SCHED */
#define DATA_EP_SIZE 4
/* #endif */
#define HTC_DATA_RESOURCE_THRS 256
#define HTC_DATA_MINDESC_PERPACKET 2

/* maximum number of requeue attempts before print */
#define MAX_REQUEUE_WARN 5

enum HTC_SEND_QUEUE_RESULT {
	HTC_SEND_QUEUE_OK = 0,  /* packet was queued */
	HTC_SEND_QUEUE_DROP = 1, /* this packet should be dropped */
};

#ifndef DEBUG_CREDIT
#define DEBUG_CREDIT 0
#endif

#if DEBUG_CREDIT
/* bit mask to enable debug certain endpoint */
static unsigned int ep_debug_mask =
	(1 << ENDPOINT_0) | (1 << ENDPOINT_1) | (1 << ENDPOINT_2);
#endif

#ifdef QCA_WIFI_EMULATION
#define HTC_EMULATION_DELAY_IN_MS 20
/**
 * htc_add_delay(): Adds a delay in before proceeding, only for emulation
 *
 * Return: None
 */
static inline void htc_add_emulation_delay(void)
{
	qdf_mdelay(HTC_EMULATION_DELAY_IN_MS);
}
#else
static inline void htc_add_emulation_delay(void)
{
}
#endif

void htc_dump_counter_info(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	if (!target)
		return;

	AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("\n%s: ce_send_cnt = %d, TX_comp_cnt = %d\n",
			 __func__, target->ce_send_cnt, target->TX_comp_cnt));
}

int htc_get_tx_queue_depth(HTC_HANDLE htc_handle, HTC_ENDPOINT_ID endpoint_id)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);
	HTC_ENDPOINT *endpoint = &target->endpoint[endpoint_id];

	return HTC_PACKET_QUEUE_DEPTH(&endpoint->TxQueue);
}
qdf_export_symbol(htc_get_tx_queue_depth);

void htc_get_control_endpoint_tx_host_credits(HTC_HANDLE HTCHandle,
					      int *credits)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	int i;

	if (!credits || !target) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("%s: invalid args", __func__));
		return;
	}

	*credits = 0;
	LOCK_HTC_TX(target);
	for (i = 0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		if (pEndpoint->service_id == WMI_CONTROL_SVC) {
			*credits = pEndpoint->TxCredits;
			break;
		}
	}
	UNLOCK_HTC_TX(target);
}

static inline void restore_tx_packet(HTC_TARGET *target, HTC_PACKET *pPacket)
{
	qdf_nbuf_t netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);

	if (pPacket->PktInfo.AsTx.Flags & HTC_TX_PACKET_FLAG_FIXUP_NETBUF) {
		qdf_nbuf_unmap(target->osdev, netbuf, QDF_DMA_TO_DEVICE);
		pPacket->PktInfo.AsTx.Flags &= ~HTC_TX_PACKET_FLAG_FIXUP_NETBUF;
	}
	if (pPacket->PktInfo.AsTx.Flags &
		HTC_TX_PACKET_FLAG_HTC_HEADER_IN_NETBUF_DATA) {
		qdf_nbuf_pull_head(netbuf, sizeof(HTC_FRAME_HDR));
		pPacket->PktInfo.AsTx.Flags &=
			~HTC_TX_PACKET_FLAG_HTC_HEADER_IN_NETBUF_DATA;
	}
}

static void send_packet_completion(HTC_TARGET *target, HTC_PACKET *pPacket)
{
	HTC_ENDPOINT *pEndpoint = &target->endpoint[pPacket->Endpoint];
	HTC_EP_SEND_PKT_COMPLETE EpTxComplete;

	if ((pPacket->PktInfo.AsTx.Flags & HTC_TX_PACKET_FLAG_FIXUP_NETBUF) &&
	    (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)))
		target->nbuf_nfc_unmap_count++;

	restore_tx_packet(target, pPacket);

	/*
	 * In case of SSR, we cannot call the upper layer completion
	 * callbacks, hence just free the nbuf and HTC packet here.
	 */
	if (target->hif_dev && hif_get_target_status(target->hif_dev)) {
		htc_free_control_tx_packet(target, pPacket);
		return;
	}

	/* do completion */
	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("HTC calling ep %d send complete callback on packet %pK\n",
			 pEndpoint->Id, pPacket));

	EpTxComplete = pEndpoint->EpCallBacks.EpTxComplete;
	if (EpTxComplete)
		EpTxComplete(pEndpoint->EpCallBacks.pContext, pPacket);
	else
		qdf_nbuf_free(pPacket->pPktContext);


}

#ifdef FEATURE_RUNTIME_PM
/**
 * log_packet_info() - Log HTC packet information
 *
 * @target: handle of HTC context
 * @pPacket: handle of HTC packet
 *
 * Return: None
 */
static void log_packet_info(HTC_TARGET *target, HTC_PACKET *pPacket)
{
	HTC_ENDPOINT *pEndpoint = &target->endpoint[pPacket->Endpoint];
	HTC_EP_LOG_PKT ep_log_pkt;
	qdf_nbuf_t netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);

	ep_log_pkt = pEndpoint->EpCallBacks.ep_log_pkt;
	if (ep_log_pkt) {
		qdf_nbuf_pull_head(netbuf, sizeof(HTC_FRAME_HDR));
		ep_log_pkt(pEndpoint->EpCallBacks.pContext, pPacket);
		qdf_nbuf_push_head(netbuf, sizeof(HTC_FRAME_HDR));
	}
}

/**
 * htc_inc_runtime_cnt: Increment htc runtime count
 * @target: handle of HTC context
 *
 * Return: None
 */
static inline
void htc_inc_runtime_cnt(HTC_TARGET *target)
{
	qdf_atomic_inc(&target->htc_runtime_cnt);
}
#else
static void log_packet_info(HTC_TARGET *target, HTC_PACKET *pPacket)
{
}

static inline
void htc_inc_runtime_cnt(HTC_TARGET *target)
{
}
#endif

void htc_send_complete_check_cleanup(void *context)
{
	HTC_ENDPOINT *pEndpoint = (HTC_ENDPOINT *) context;

	htc_send_complete_check(pEndpoint, 1);
}

HTC_PACKET *allocate_htc_bundle_packet(HTC_TARGET *target)
{
	HTC_PACKET *pPacket;
	HTC_PACKET_QUEUE *pQueueSave;
	qdf_nbuf_t netbuf;

	LOCK_HTC_TX(target);
	if (!target->pBundleFreeList) {
		UNLOCK_HTC_TX(target);
		netbuf = qdf_nbuf_alloc(NULL,
					target->MaxMsgsPerHTCBundle *
					target->TargetCreditSize, 0, 4, false);
		AR_DEBUG_ASSERT(netbuf);
		if (!netbuf)
			return NULL;
		pPacket = qdf_mem_malloc(sizeof(HTC_PACKET));
		AR_DEBUG_ASSERT(pPacket);
		if (!pPacket) {
			qdf_nbuf_free(netbuf);
			return NULL;
		}
		pQueueSave = qdf_mem_malloc(sizeof(HTC_PACKET_QUEUE));
		AR_DEBUG_ASSERT(pQueueSave);
		if (!pQueueSave) {
			qdf_nbuf_free(netbuf);
			qdf_mem_free(pPacket);
			return NULL;
		}
		INIT_HTC_PACKET_QUEUE(pQueueSave);
		pPacket->pContext = pQueueSave;
		SET_HTC_PACKET_NET_BUF_CONTEXT(pPacket, netbuf);
		pPacket->pBuffer = qdf_nbuf_data(netbuf);
		pPacket->BufferLength = qdf_nbuf_len(netbuf);

		/* store the original head room so that we can restore this
		 * when we "free" the packet.
		 * free packet puts the packet back on the free list
		 */
		pPacket->netbufOrigHeadRoom = qdf_nbuf_headroom(netbuf);
		return pPacket;
	}
	/* already done malloc - restore from free list */
	pPacket = target->pBundleFreeList;
	AR_DEBUG_ASSERT(pPacket);
	if (!pPacket) {
		UNLOCK_HTC_TX(target);
		return NULL;
	}
	target->pBundleFreeList = (HTC_PACKET *) pPacket->ListLink.pNext;
	UNLOCK_HTC_TX(target);
	pPacket->ListLink.pNext = NULL;

	return pPacket;
}

void free_htc_bundle_packet(HTC_TARGET *target, HTC_PACKET *pPacket)
{
	uint32_t curentHeadRoom;
	qdf_nbuf_t netbuf;
	HTC_PACKET_QUEUE *pQueueSave;

	netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
	AR_DEBUG_ASSERT(netbuf);
	if (!netbuf) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("\n%s: Invalid netbuf in HTC Packet\n",
				__func__));
		return;
	}
	/* HIF adds data to the headroom section of the nbuf, restore thei
	 * original size. If this is not done, headroom keeps shrinking with
	 * every HIF send and eventually HIF ends up doing another malloc big
	 * enough to store the data + its header
	 */

	curentHeadRoom = qdf_nbuf_headroom(netbuf);
	qdf_nbuf_pull_head(netbuf,
			   pPacket->netbufOrigHeadRoom - curentHeadRoom);
	qdf_nbuf_trim_tail(netbuf, qdf_nbuf_len(netbuf));

	/* restore the pBuffer pointer. HIF changes this */
	pPacket->pBuffer = qdf_nbuf_data(netbuf);
	pPacket->BufferLength = qdf_nbuf_len(netbuf);

	/* restore queue */
	pQueueSave = (HTC_PACKET_QUEUE *) pPacket->pContext;
	if (qdf_unlikely(!pQueueSave)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("\n%s: Invalid pQueueSave in HTC Packet\n",
				__func__));
		AR_DEBUG_ASSERT(pQueueSave);
	} else
		INIT_HTC_PACKET_QUEUE(pQueueSave);

	LOCK_HTC_TX(target);
	if (!target->pBundleFreeList) {
		target->pBundleFreeList = pPacket;
		pPacket->ListLink.pNext = NULL;
	} else {
		pPacket->ListLink.pNext = (DL_LIST *) target->pBundleFreeList;
		target->pBundleFreeList = pPacket;
	}
	UNLOCK_HTC_TX(target);
}

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)

/**
 * htc_send_update_tx_bundle_stats() - update tx bundle stats depends
 *				on max bundle size
 * @target: hif context
 * @data_len: tx data len
 * @TxCreditSize: endpoint tx credit size
 *
 * Return: None
 */
static inline void
htc_send_update_tx_bundle_stats(HTC_TARGET *target,
				qdf_size_t data_len,
				int TxCreditSize)
{
	int index = ((data_len + TxCreditSize - 1) / TxCreditSize) - 1;

	if (index < HTC_MAX_MSG_PER_BUNDLE_TX)
		target->tx_bundle_stats[index]++;
}

/**
 * htc_issue_tx_bundle_stats_inc() - increment in tx bundle stats
 *				on max bundle size
 * @target: hif context
 *
 * Return: None
 */
static inline void
htc_issue_tx_bundle_stats_inc(HTC_TARGET *target)
{
	target->tx_bundle_stats[0]++;
}
#else

static inline void
htc_send_update_tx_bundle_stats(HTC_TARGET *target,
				qdf_size_t data_len,
				int TxCreditSize)
{
}

static inline void
htc_issue_tx_bundle_stats_inc(HTC_TARGET *target)
{
}
#endif

#if defined(HIF_USB) || defined(HIF_SDIO)
#ifdef ENABLE_BUNDLE_TX
static QDF_STATUS htc_send_bundled_netbuf(HTC_TARGET *target,
					HTC_ENDPOINT *pEndpoint,
					unsigned char *pBundleBuffer,
					HTC_PACKET *pPacketTx)
{
	qdf_size_t data_len;
	QDF_STATUS status;
	qdf_nbuf_t bundleBuf;
	uint32_t data_attr = 0;

	bundleBuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacketTx);
	data_len = pBundleBuffer - qdf_nbuf_data(bundleBuf);
	qdf_nbuf_put_tail(bundleBuf, data_len);
	SET_HTC_PACKET_INFO_TX(pPacketTx,
			       target,
			       pBundleBuffer,
			       data_len,
			       pEndpoint->Id, HTC_TX_PACKET_TAG_BUNDLED);
	LOCK_HTC_TX(target);
	HTC_PACKET_ENQUEUE(&pEndpoint->TxLookupQueue, pPacketTx);
	pEndpoint->ul_outstanding_cnt++;
	UNLOCK_HTC_TX(target);
#if DEBUG_BUNDLE
	qdf_print(" Send bundle EP%d buffer size:0x%x, total:0x%x, count:%d.",
		  pEndpoint->Id,
		  pEndpoint->TxCreditSize,
		  data_len, data_len / pEndpoint->TxCreditSize);
#endif

	htc_send_update_tx_bundle_stats(target, data_len,
					pEndpoint->TxCreditSize);

	status = hif_send_head(target->hif_dev,
			       pEndpoint->UL_PipeID,
			       pEndpoint->Id, data_len,
			       bundleBuf, data_attr);
	if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
		HTC_PACKET_QUEUE requeue;

		qdf_print("hif_send_head failed(len=%zu).", data_len);
		INIT_HTC_PACKET_QUEUE(&requeue);
		LOCK_HTC_TX(target);
		pEndpoint->ul_outstanding_cnt--;
		HTC_PACKET_REMOVE(&pEndpoint->TxLookupQueue, pPacketTx);

		if (pPacketTx->PktInfo.AsTx.Tag == HTC_TX_PACKET_TAG_BUNDLED) {
			HTC_PACKET *temp_packet;
			HTC_PACKET_QUEUE *packet_queue =
				(HTC_PACKET_QUEUE *)pPacketTx->pContext;

			HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(packet_queue,
							      temp_packet) {
				HTC_PACKET_ENQUEUE(&requeue, temp_packet);
			} HTC_PACKET_QUEUE_ITERATE_END;

			UNLOCK_HTC_TX(target);
			free_htc_bundle_packet(target, pPacketTx);
			LOCK_HTC_TX(target);

		} else {
			HTC_PACKET_ENQUEUE(&requeue, pPacketTx);
		}

		HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&pEndpoint->TxQueue,
						  &requeue);
		UNLOCK_HTC_TX(target);
	}
	return status;
}

#ifdef QCA_TX_PADDING_CREDIT_SUPPORT
#define SDIO_BLOCK_SIZE		512
static int htc_tx_pad_credit_avail(HTC_ENDPOINT *ep)
{
	int ret = 0;

	if (!ep || !ep->EpCallBacks.pContext ||
	    !ep->EpCallBacks.ep_padding_credit_update)
		return 1;

	ret = ep->EpCallBacks.ep_padding_credit_update(ep->EpCallBacks.pContext,
						       0);

	if (ret < 2)
		AR_DEBUG_PRINTF(ATH_DEBUG_INFO, ("%s ret %d\n", __func__, ret));

	return ret;
}

static bool htc_handle_extra_tx_credit(HTC_ENDPOINT *ep,
				       HTC_PACKET *p_last_htc_pkt,
				       unsigned char *p_last_pkt_bundle_buffer,
				       unsigned char **p_bundle_buffer,
				       int tot_data_len)
{
	bool extra_tx_credit = FALSE;
	HTC_FRAME_HDR *p_htc_hdr;
	int first_buf_bundled_len = 0, last_buf_len = 0;
	int sdio_pad = 0, free_space = 0;
	int (*update_ep_padding_credit)(void *, int);

	update_ep_padding_credit = ep->EpCallBacks.ep_padding_credit_update;

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("%s Tot data_len = %d\n", __func__, tot_data_len));

	if (!p_last_htc_pkt)
		return extra_tx_credit;

	last_buf_len = (p_last_htc_pkt->ActualLength + HTC_HDR_LENGTH);
	if (tot_data_len != last_buf_len) {
		first_buf_bundled_len = tot_data_len - ep->TxCreditSize;
		free_space = tot_data_len -
					(first_buf_bundled_len + last_buf_len);
	} else {
		free_space = ep->TxCreditSize - tot_data_len;
	}

	sdio_pad = SDIO_BLOCK_SIZE - ((first_buf_bundled_len + last_buf_len) %
			SDIO_BLOCK_SIZE);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("%s first_buf_bundled_len = %d last_buf_len = %d\n",
			__func__, first_buf_bundled_len, last_buf_len));

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("%s sdio_pad = %d free_space = %d\n", __func__,
			sdio_pad, free_space));

	if (sdio_pad <= free_space) {
		if (p_bundle_buffer && *p_bundle_buffer) {
			/* Align Tx bundled buf to avoid a extra Padding buf */
			*p_bundle_buffer -= (free_space - sdio_pad);
		}
	} else {
		/* Extra Padding Buffer needed, consume extra tx credit */
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
				("%s Used a Tx credit for Padding Buffer\n",
				 __func__));
		p_htc_hdr = (HTC_FRAME_HDR *)(p_last_pkt_bundle_buffer);
		p_htc_hdr->Flags |= HTC_FLAGS_PADDING_CHECK;
		extra_tx_credit = TRUE;
		if (ep->EpCallBacks.ep_padding_credit_update) {
			/* Decrement 1 credit at host,
			 * due to extra tx credit consumed by padding buffer
			 */
			update_ep_padding_credit(ep->EpCallBacks.pContext, -1);
		}
	}
	return extra_tx_credit;
}
#else
static int htc_tx_pad_credit_avail(HTC_ENDPOINT *ep)
{
	return 1;
}

static bool htc_handle_extra_tx_credit(HTC_ENDPOINT *ep,
				       HTC_PACKET *p_last_htc_pkt,
				       unsigned char *p_last_pkt_bundle_buffer,
				       unsigned char **p_bundle_buffer,
				       int tot_data_len)
{
	return FALSE;
}
#endif

/**
 * htc_issue_packets_bundle() - HTC function to send bundle packets from a queue
 * @target: HTC target on which packets need to be sent
 * @pEndpoint: logical endpoint on which packets needs to be sent
 * @pPktQueue: HTC packet queue containing the list of packets to be sent
 *
 * Return: void
 */
static void htc_issue_packets_bundle(HTC_TARGET *target,
				     HTC_ENDPOINT *pEndpoint,
				     HTC_PACKET_QUEUE *pPktQueue)
{
	int i, frag_count, nbytes;
	qdf_nbuf_t netbuf, bundleBuf;
	unsigned char *pBundleBuffer = NULL;
	HTC_PACKET *pPacket = NULL, *pPacketTx = NULL;
	HTC_FRAME_HDR *pHtcHdr;
	int last_credit_pad = 0;
	int creditPad, creditRemainder, transferLength, bundlesSpaceRemaining =
		0;
	HTC_PACKET_QUEUE *pQueueSave = NULL;
	HTC_PACKET *p_last_htc_pkt = NULL;
	unsigned char *p_last_pkt_bundle_buffer = NULL;

	bundlesSpaceRemaining =
		target->MaxMsgsPerHTCBundle * pEndpoint->TxCreditSize;
	pPacketTx = allocate_htc_bundle_packet(target);
	if (!pPacketTx) {
		/* good time to panic */
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("allocate_htc_bundle_packet failed\n"));
		AR_DEBUG_ASSERT(false);
		return;
	}
	bundleBuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacketTx);
	pBundleBuffer = qdf_nbuf_data(bundleBuf);
	pQueueSave = (HTC_PACKET_QUEUE *) pPacketTx->pContext;
	while (1) {
		if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
			if (htc_tx_pad_credit_avail(pEndpoint) < 1)
				break;
		}
		pPacket = htc_packet_dequeue(pPktQueue);
		if (!pPacket)
			break;
		creditPad = 0;
		transferLength = pPacket->ActualLength + HTC_HDR_LENGTH;
		creditRemainder = transferLength % pEndpoint->TxCreditSize;
		if (creditRemainder != 0) {
			if (transferLength < pEndpoint->TxCreditSize) {
				creditPad = pEndpoint->TxCreditSize -
					    transferLength;
			} else {
				creditPad = creditRemainder;
			}
			transferLength += creditPad;
		}

		if (bundlesSpaceRemaining < transferLength) {
			htc_handle_extra_tx_credit(pEndpoint, p_last_htc_pkt,
						   p_last_pkt_bundle_buffer,
						   &pBundleBuffer,
						   pBundleBuffer -
						   qdf_nbuf_data(bundleBuf));

			/* send out previous buffer */
			htc_send_bundled_netbuf(target, pEndpoint,
						pBundleBuffer - last_credit_pad,
						pPacketTx);
			/* One packet has been dequeued from sending queue when enter
			 * this loop, so need to add 1 back for this checking.
			 */
			if ((HTC_PACKET_QUEUE_DEPTH(pPktQueue) + 1) <
			    HTC_MIN_MSG_PER_BUNDLE) {
				HTC_PACKET_ENQUEUE_TO_HEAD(pPktQueue, pPacket);
				return;
			}
			bundlesSpaceRemaining =
				target->MaxMsgsPerHTCBundle *
				pEndpoint->TxCreditSize;
			pPacketTx = allocate_htc_bundle_packet(target);
			if (!pPacketTx) {
				HTC_PACKET_ENQUEUE_TO_HEAD(pPktQueue, pPacket);
				/* good time to panic */
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("allocate_htc_bundle_packet failed\n"));
				AR_DEBUG_ASSERT(false);
				return;
			}
			bundleBuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacketTx);
			pBundleBuffer = qdf_nbuf_data(bundleBuf);
			pQueueSave = (HTC_PACKET_QUEUE *) pPacketTx->pContext;
		}

		p_last_htc_pkt = pPacket;
		p_last_pkt_bundle_buffer = pBundleBuffer;

		bundlesSpaceRemaining -= transferLength;
		netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);

		if (hif_get_bus_type(target->hif_dev) != QDF_BUS_TYPE_USB) {
			pHtcHdr = (HTC_FRAME_HDR *)qdf_nbuf_get_frag_vaddr(
								netbuf, 0);
			HTC_WRITE32(pHtcHdr,
				SM(pPacket->ActualLength,
				HTC_FRAME_HDR_PAYLOADLEN) |
				SM(pPacket->PktInfo.AsTx.SendFlags |
				HTC_FLAGS_SEND_BUNDLE,
				HTC_FRAME_HDR_FLAGS) |
				SM(pPacket->Endpoint,
				HTC_FRAME_HDR_ENDPOINTID));
			HTC_WRITE32((uint32_t *) pHtcHdr + 1,
				SM(pPacket->PktInfo.AsTx.SeqNo,
				HTC_FRAME_HDR_CONTROLBYTES1) | SM(creditPad,
				HTC_FRAME_HDR_RESERVED));
			pHtcHdr->reserved = creditPad;
		}
		frag_count = qdf_nbuf_get_num_frags(netbuf);
		nbytes = pPacket->ActualLength + HTC_HDR_LENGTH;
		for (i = 0; i < frag_count && nbytes > 0; i++) {
			int frag_len = qdf_nbuf_get_frag_len(netbuf, i);
			unsigned char *frag_addr =
				qdf_nbuf_get_frag_vaddr(netbuf, i);
			if (frag_len > nbytes)
				frag_len = nbytes;
			qdf_mem_copy(pBundleBuffer, frag_addr, frag_len);
			nbytes -= frag_len;
			pBundleBuffer += frag_len;
		}
		HTC_PACKET_ENQUEUE(pQueueSave, pPacket);
		pBundleBuffer += creditPad;

		/* last one can't be packed. */
		if (hif_get_bus_type(target->hif_dev) == QDF_BUS_TYPE_USB)
			last_credit_pad = creditPad;
	}
	/* send out remaining buffer */
	if (pBundleBuffer != qdf_nbuf_data(bundleBuf)) {
		htc_handle_extra_tx_credit(pEndpoint, p_last_htc_pkt,
					   p_last_pkt_bundle_buffer,
					   &pBundleBuffer,
					   pBundleBuffer -
					   qdf_nbuf_data(bundleBuf));

		htc_send_bundled_netbuf(target, pEndpoint,
					pBundleBuffer - last_credit_pad,
					pPacketTx);
	} else {
		free_htc_bundle_packet(target, pPacketTx);
	}
}
#endif /* ENABLE_BUNDLE_TX */
#else
static int htc_tx_pad_credit_avail(HTC_ENDPOINT *ep)
{
	return 1;
}

bool htc_handle_extra_tx_credit(HTC_ENDPOINT *ep,
				HTC_PACKET *p_last_htc_pkt,
				unsigned char *p_last_pkt_bundle_buffer,
				unsigned char **p_bundle_buffer,
				int tot_data_len);
bool htc_handle_extra_tx_credit(HTC_ENDPOINT *ep,
				HTC_PACKET *p_last_htc_pkt,
				unsigned char *p_last_pkt_bundle_buffer,
				unsigned char **p_bundle_buffer,
				int tot_data_len)
{
	return FALSE;
}

static void htc_issue_packets_bundle(HTC_TARGET *target,
				     HTC_ENDPOINT *pEndpoint,
				     HTC_PACKET_QUEUE *pPktQueue)
{
}
#endif

/**
 * htc_issue_packets() - HTC function to send packets from a queue
 * @target: HTC target on which packets need to be sent
 * @pEndpoint: logical endpoint on which packets needs to be sent
 * @pPktQueue: HTC packet queue containing the list of packets to be sent
 *
 * Return: QDF_STATUS_SUCCESS on success and error QDF status on failure
 */
static QDF_STATUS htc_issue_packets(HTC_TARGET *target,
				  HTC_ENDPOINT *pEndpoint,
				  HTC_PACKET_QUEUE *pPktQueue)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_nbuf_t netbuf;
	HTC_PACKET *pPacket = NULL;
	uint16_t payloadLen;
	HTC_FRAME_HDR *pHtcHdr;
	uint32_t data_attr = 0;
	enum qdf_bus_type bus_type;
	QDF_STATUS ret;
	bool rt_put = false;
	bool used_extra_tx_credit = false;
	uint8_t *buf = NULL;
	int (*update_ep_padding_credit)(void *, int);
	void *ctx = NULL;
	bool rt_put_in_resp;
	int32_t sys_state = HIF_SYSTEM_PM_STATE_ON;

	update_ep_padding_credit =
			pEndpoint->EpCallBacks.ep_padding_credit_update;

	bus_type = hif_get_bus_type(target->hif_dev);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+htc_issue_packets: Queue: %pK, Pkts %d\n", pPktQueue,
			 HTC_PACKET_QUEUE_DEPTH(pPktQueue)));
	while (true) {
		rt_put_in_resp = false;
		if (HTC_TX_BUNDLE_ENABLED(target) &&
		    HTC_PACKET_QUEUE_DEPTH(pPktQueue) >=
		    HTC_MIN_MSG_PER_BUNDLE) {
			switch (bus_type) {
			case QDF_BUS_TYPE_SDIO:
				if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint))
					break;
				if (update_ep_padding_credit) {
					if (htc_tx_pad_credit_avail
					    (pEndpoint) < 1)
						break;
				}
				fallthrough;
			case QDF_BUS_TYPE_USB:
				htc_issue_packets_bundle(target,
							pEndpoint,
							pPktQueue);
				break;
			default:
				break;
			}
		}
		/* if not bundling or there was a packet that could not be
		 * placed in a bundle, and send it by normal way
		 */
		if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
			if (htc_tx_pad_credit_avail(pEndpoint) < 1) {
				status = QDF_STATUS_E_FAILURE;
				break;
			}
		}

		pPacket = htc_packet_dequeue(pPktQueue);
		if (!pPacket) {
			/* local queue is fully drained */
			break;
		}

		netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
		AR_DEBUG_ASSERT(netbuf);
		/* Non-credit enabled endpoints have been mapped and setup by
		 * now, so no need to revisit the HTC headers
		 */
		if (IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {

			payloadLen = pPacket->ActualLength;
			/* setup HTC frame header */

			pHtcHdr = (HTC_FRAME_HDR *)
				qdf_nbuf_get_frag_vaddr(netbuf, 0);
			if (qdf_unlikely(!pHtcHdr)) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("%s Invalid pHtcHdr\n",
						 __func__));
				AR_DEBUG_ASSERT(pHtcHdr);
				status = QDF_STATUS_E_FAILURE;
				break;
			}

			HTC_WRITE32(pHtcHdr,
					SM(payloadLen,
						HTC_FRAME_HDR_PAYLOADLEN) |
					SM(pPacket->PktInfo.AsTx.SendFlags,
						HTC_FRAME_HDR_FLAGS) |
					SM(pPacket->Endpoint,
						HTC_FRAME_HDR_ENDPOINTID));
			HTC_WRITE32(((uint32_t *) pHtcHdr) + 1,
				    SM(pPacket->PktInfo.AsTx.SeqNo,
				       HTC_FRAME_HDR_CONTROLBYTES1));

			/*
			 * Now that the HTC frame header has been added, the
			 * netbuf can be mapped.  This only applies to non-data
			 * frames, since data frames were already mapped as they
			 * entered into the driver.
			 */
			ret = qdf_nbuf_map(target->osdev,
				GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket),
				QDF_DMA_TO_DEVICE);
			if (ret != QDF_STATUS_SUCCESS) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s nbuf Map Fail Endpnt %pK\n",
					__func__, pEndpoint));
				HTC_PACKET_ENQUEUE_TO_HEAD(pPktQueue, pPacket);
				status = QDF_STATUS_E_FAILURE;
				break;
			}
			pPacket->PktInfo.AsTx.Flags |=
				HTC_TX_PACKET_FLAG_FIXUP_NETBUF;
		}

		if (!pEndpoint->async_update) {
			LOCK_HTC_TX(target);
		}
		/* store in look up queue to match completions */
		HTC_PACKET_ENQUEUE(&pEndpoint->TxLookupQueue, pPacket);
		INC_HTC_EP_STAT(pEndpoint, TxIssued, 1);
		pEndpoint->ul_outstanding_cnt++;
		if (!pEndpoint->async_update) {
			UNLOCK_HTC_TX(target);
			hif_send_complete_check(target->hif_dev,
					pEndpoint->UL_PipeID, false);
		}

		if (pPacket->PktInfo.AsTx.Tag == HTC_TX_PACKET_SYSTEM_SUSPEND) {
			sys_state = hif_system_pm_get_state(target->hif_dev);
			hif_system_pm_set_state_suspending(target->hif_dev);
		}

		htc_packet_set_magic_cookie(pPacket, HTC_PACKET_MAGIC_COOKIE);
		/*
		 * For HTT messages without a response from fw,
		 *   do the runtime put here.
		 * otherwise runtime put will be done when the fw response comes
		 */
		if (pPacket->PktInfo.AsTx.Tag == HTC_TX_PACKET_TAG_RUNTIME_PUT) {
			rt_put = true;
			hif_pm_runtime_update_stats(
					target->hif_dev, RTPM_ID_HTC,
					HIF_PM_HTC_STATS_GET_HTT_NO_RESPONSE);
		} else if (pPacket->PktInfo.AsTx.Tag ==
			 HTC_TX_PACKET_TAG_RTPM_PUT_RC) {
			rt_put_in_resp = true;
			htc_inc_runtime_cnt(target);
			hif_pm_runtime_update_stats(
					target->hif_dev, RTPM_ID_HTC,
					HIF_PM_HTC_STATS_GET_HTT_RESPONSE);
		}

#if DEBUG_BUNDLE
		qdf_print(" Send single EP%d buffer size:0x%x, total:0x%x.",
			  pEndpoint->Id,
			  pEndpoint->TxCreditSize,
			  HTC_HDR_LENGTH + pPacket->ActualLength);
#endif
		buf = (uint8_t *)qdf_nbuf_get_frag_vaddr(netbuf, 0);
		used_extra_tx_credit =
			htc_handle_extra_tx_credit(pEndpoint, pPacket, buf,
						   NULL, pPacket->ActualLength +
						   HTC_HDR_LENGTH);

		status = hif_send_head(target->hif_dev,
				       pEndpoint->UL_PipeID, pEndpoint->Id,
				       HTC_HDR_LENGTH + pPacket->ActualLength,
				       netbuf, data_attr);

		if (status != QDF_STATUS_SUCCESS) {
			if (rt_put_in_resp)
				htc_dec_return_runtime_cnt((void *)target);

			if (pPacket->PktInfo.AsTx.Tag ==
			    HTC_TX_PACKET_SYSTEM_SUSPEND)
				__hif_system_pm_set_state(target->hif_dev,
							  sys_state);

			if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
				if (used_extra_tx_credit) {
					ctx = pEndpoint->EpCallBacks.pContext;
					update_ep_padding_credit(ctx, 1);
				}
			}
		}

		htc_issue_tx_bundle_stats_inc(target);

		target->ce_send_cnt++;
		pEndpoint->htc_send_cnt++;

		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			if (status != QDF_STATUS_E_RESOURCES) {
				/* TODO : if more than 1 endpoint maps to the
				 * same PipeID it is possible to run out of
				 * resources in the HIF layer. Don't emit the
				 * error
				 */
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("hif_send Failed status:%d\n",
						 status));
			} else {
				if (target->htc_pkt_dbg) {
					if (pEndpoint->num_requeues_warn >
						MAX_REQUEUE_WARN) {
						hif_print_napi_stats(target->hif_dev);
					}
				}
			}

			/* only unmap if we mapped in this function */
			if (IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
				qdf_nbuf_unmap(target->osdev,
					GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket),
					QDF_DMA_TO_DEVICE);
				pPacket->PktInfo.AsTx.Flags &=
					~HTC_TX_PACKET_FLAG_FIXUP_NETBUF;
			}

			if (!pEndpoint->async_update) {
				LOCK_HTC_TX(target);
			}
			target->ce_send_cnt--;
			pEndpoint->htc_send_cnt--;
			pEndpoint->ul_outstanding_cnt--;
			HTC_PACKET_REMOVE(&pEndpoint->TxLookupQueue, pPacket);
			htc_packet_set_magic_cookie(pPacket, 0);
			/* put it back into the callers queue */
			HTC_PACKET_ENQUEUE_TO_HEAD(pPktQueue, pPacket);
			/* reclaim credits */
			HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(pPktQueue,
							      pPacket) {
			    pEndpoint->TxCredits +=
				pPacket->PktInfo.AsTx.CreditsUsed;
			} HTC_PACKET_QUEUE_ITERATE_END;
			if (!pEndpoint->async_update) {
				UNLOCK_HTC_TX(target);
			}
			break;
		}
		if (rt_put) {
			hif_pm_runtime_put(target->hif_dev,
					   RTPM_ID_HTC);
			hif_pm_runtime_update_stats(
					target->hif_dev, RTPM_ID_HTC,
					HIF_PM_HTC_STATS_PUT_HTT_NO_RESPONSE);
			rt_put = false;
		}
	}

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
		if (((status == QDF_STATUS_E_RESOURCES) &&
		     (pEndpoint->num_requeues_warn > MAX_REQUEUE_WARN)) ||
		     (status != QDF_STATUS_E_RESOURCES)) {
			QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO,
				  "failed pkt:0x%pK status:%d endpoint:%d",
				  pPacket, status, pEndpoint->Id);
		}

	}

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-htc_issue_packets\n"));

	return status;
}

#ifdef FEATURE_RUNTIME_PM
/**
 * extract_htc_pm_packets(): move pm packets from endpoint into queue
 * @endpoint: which enpoint to extract packets from
 * @queue: a queue to store extracted packets in.
 *
 * remove pm packets from the endpoint's tx queue.
 * queue them into a queue
 */
static void extract_htc_pm_packets(HTC_ENDPOINT *endpoint,
				HTC_PACKET_QUEUE *queue)
{
	HTC_PACKET *packet;

	/* only WMI endpoint has power management packets */
	if (endpoint->service_id != WMI_CONTROL_SVC)
		return;

	ITERATE_OVER_LIST_ALLOW_REMOVE(&endpoint->TxQueue.QueueHead, packet,
			HTC_PACKET, ListLink) {
		if (packet->PktInfo.AsTx.Tag == HTC_TX_PACKET_TAG_AUTO_PM) {
			HTC_PACKET_REMOVE(&endpoint->TxQueue, packet);
			HTC_PACKET_ENQUEUE(queue, packet);
		}
	} ITERATE_END
}

/**
 * queue_htc_pm_packets(): queue pm packets with priority
 * @endpoint: enpoint to queue packets to
 * @queue: queue of pm packets to enque
 *
 * suspend resume packets get special treatment & priority.
 * need to queue them at the front of the queue.
 */
static void queue_htc_pm_packets(HTC_ENDPOINT *endpoint,
				 HTC_PACKET_QUEUE *queue)
{
	if (endpoint->service_id != WMI_CONTROL_SVC)
		return;

	HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&endpoint->TxQueue, queue);
}
#else
static void extract_htc_pm_packets(HTC_ENDPOINT *endpoint,
		HTC_PACKET_QUEUE *queue)
{}

static void queue_htc_pm_packets(HTC_ENDPOINT *endpoint,
		HTC_PACKET_QUEUE *queue)
{}
#endif

/**
 * htc_send_pkts_rtpm_dbgid_get() - get runtime pm dbgid by service_id
 * @service_id: service for endpoint
 *
 * For service_id HTT_DATA_MSG_SVC, HTT message donot have a tx complete
 * from CE level, so they need runtime put which only can happen in fw
 * response. runtime put will happens at 2 ways.
 *    1 if packet tag HTC_TX_PACKET_TAG_RUNTIME_PUT, runtime put
 *      will be just in htc_issue_packets. as such pkt doesn't have
 *      a response from fw.
 *    2 other pkt must have a response from fw, it will be handled
 *      by fw response using htc_pm_runtime_put.
 *
 * For other service_id, they have tx_completion from CE, so they will be
 * handled in htc_tx_completion_handler, except packet tag as
 * HTC_TX_PACKET_TAG_AUTO_PM, pm related wmi cmd don't need a runtime
 * put/get.
 *
 *
 * Return: rtpm_dbgid to trace who use it
 */
static wlan_rtpm_dbgid
htc_send_pkts_rtpm_dbgid_get(HTC_SERVICE_ID service_id)
{
	wlan_rtpm_dbgid rtpm_dbgid;

	if (service_id == HTT_DATA_MSG_SVC)
		rtpm_dbgid = RTPM_ID_HTC;
	else
		rtpm_dbgid = RTPM_ID_WMI;

	return rtpm_dbgid;
}

#ifdef SYSTEM_PM_CHECK
/**
 * extract_htc_system_resume_pkts(): Move system pm resume packets from endpoint
 *  into queue
 * @endpoint: which enpoint to extract packets from
 * @queue: a queue to store extracted packets in.
 *
 * Remove pm packets from the endpoint's tx queue and enqueue
 * them into a queue
 */
static void extract_htc_system_resume_pkts(HTC_ENDPOINT *endpoint,
					   HTC_PACKET_QUEUE *queue)
{
	HTC_PACKET *packet;

	/* only WMI endpoint has power management packets */
	if (endpoint->service_id != WMI_CONTROL_SVC)
		return;

	ITERATE_OVER_LIST_ALLOW_REMOVE(&endpoint->TxQueue.QueueHead, packet,
				       HTC_PACKET, ListLink) {
		if (packet->PktInfo.AsTx.Tag == HTC_TX_PACKET_SYSTEM_RESUME) {
			HTC_PACKET_REMOVE(&endpoint->TxQueue, packet);
			HTC_PACKET_ENQUEUE(queue, packet);
		}
	} ITERATE_END
}
#else
static inline
void extract_htc_system_resume_pkts(HTC_ENDPOINT *endpoint,
				    HTC_PACKET_QUEUE *queue)
{
}
#endif

/**
 * get_htc_send_packets_credit_based() - get packets based on available credits
 * @target: HTC target on which packets need to be sent
 * @pEndpoint: logical endpoint on which packets needs to be sent
 * @pQueue: HTC packet queue containing the list of packets to be sent
 *
 * Get HTC send packets from TX queue on an endpoint based on available credits.
 * The function moves the packets from TX queue of the endpoint to pQueue.
 *
 * Return: None
 */
static void get_htc_send_packets_credit_based(HTC_TARGET *target,
					      HTC_ENDPOINT *pEndpoint,
					      HTC_PACKET_QUEUE *pQueue)
{
	int creditsRequired;
	int remainder;
	uint8_t sendFlags;
	HTC_PACKET *pPacket;
	unsigned int transferLength;
	HTC_PACKET_QUEUE *tx_queue;
	HTC_PACKET_QUEUE pm_queue;
	bool do_pm_get = false;
	wlan_rtpm_dbgid rtpm_dbgid = 0;
	int ret;
	HTC_PACKET_QUEUE sys_pm_queue;
	bool sys_pm_check = false;

	/*** NOTE : the TX lock is held when this function is called ***/
	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+get_htc_send_packets_credit_based\n"));

	INIT_HTC_PACKET_QUEUE(&pm_queue);
	extract_htc_pm_packets(pEndpoint, &pm_queue);
	if (HTC_QUEUE_EMPTY(&pm_queue)) {
		do_pm_get = true;

		INIT_HTC_PACKET_QUEUE(&sys_pm_queue);
		extract_htc_system_resume_pkts(pEndpoint, &sys_pm_queue);
		if (HTC_QUEUE_EMPTY(&sys_pm_queue)) {
			tx_queue = &pEndpoint->TxQueue;
			sys_pm_check = true;
		} else {
			tx_queue = &sys_pm_queue;
		}
	} else {
		tx_queue = &pm_queue;
	}

	/* loop until we can grab as many packets out of the queue as we can */
	while (true) {
		if (do_pm_get) {
			rtpm_dbgid =
				htc_send_pkts_rtpm_dbgid_get(
					pEndpoint->service_id);
			ret = hif_pm_runtime_get(target->hif_dev,
						 rtpm_dbgid, false);
			if (ret) {
				/* bus suspended, runtime resume issued */
				QDF_ASSERT(HTC_PACKET_QUEUE_DEPTH(pQueue) == 0);
				if (ret == -EAGAIN) {
					pPacket = htc_get_pkt_at_head(tx_queue);
					if (!pPacket)
						break;
					log_packet_info(target, pPacket);
				}
				break;
			}
		}

		sendFlags = 0;
		/* get packet at head, but don't remove it */
		pPacket = htc_get_pkt_at_head(tx_queue);
		if (!pPacket) {
			if (do_pm_get)
				hif_pm_runtime_put(target->hif_dev,
						   rtpm_dbgid);
			break;
		}

		if (sys_pm_check &&
		    hif_system_pm_state_check(target->hif_dev)) {
			if (do_pm_get)
				hif_pm_runtime_put(target->hif_dev, rtpm_dbgid);
			break;
		}

		AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
				(" Got head packet:%pK , Queue Depth: %d\n",
				 pPacket,
				 HTC_PACKET_QUEUE_DEPTH(tx_queue)));

		transferLength = pPacket->ActualLength + HTC_HDR_LENGTH;

		if (transferLength <= pEndpoint->TxCreditSize) {
			creditsRequired = 1;
		} else {
			/* figure out how many credits this message requires */
			creditsRequired =
				transferLength / pEndpoint->TxCreditSize;
			remainder = transferLength % pEndpoint->TxCreditSize;

			if (remainder)
				creditsRequired++;
		}

		AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
				(" Credits Required:%d   Got:%d\n",
				 creditsRequired, pEndpoint->TxCredits));

		if (pEndpoint->Id == ENDPOINT_0) {
			/*
			 * endpoint 0 is special, it always has a credit and
			 * does not require credit based flow control
			 */
			creditsRequired = 0;
		} else {

			if (pEndpoint->TxCredits < creditsRequired) {
#if DEBUG_CREDIT
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("EP%d,No Credit now.%d < %d\n",
						 pEndpoint->Id,
						 pEndpoint->TxCredits,
						 creditsRequired));
#endif
				if (do_pm_get)
					hif_pm_runtime_put(target->hif_dev,
							   rtpm_dbgid);
				break;
			}

			pEndpoint->TxCredits -= creditsRequired;
			INC_HTC_EP_STAT(pEndpoint, TxCreditsConsummed,
					creditsRequired);

			/* check if we need credits back from the target */
			if (pEndpoint->TxCredits <=
			    pEndpoint->TxCreditsPerMaxMsg) {
				/* tell the target we need credits ASAP! */
				sendFlags |= HTC_FLAGS_NEED_CREDIT_UPDATE;
				if (pEndpoint->service_id == WMI_CONTROL_SVC) {
					htc_credit_record(HTC_REQUEST_CREDIT,
							  pEndpoint->TxCredits,
							  HTC_PACKET_QUEUE_DEPTH
							  (tx_queue));
					hif_latency_detect_credit_record_time(
						HIF_REQUEST_CREDIT,
						target->hif_dev);
				}
				INC_HTC_EP_STAT(pEndpoint,
						TxCreditLowIndications, 1);
#if DEBUG_CREDIT
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						(" EP%d Needs Credits\n",
						 pEndpoint->Id));
#endif
			}
		}

		/* now we can fully dequeue */
		pPacket = htc_packet_dequeue(tx_queue);
		if (pPacket) {
			/* save the number of credits this packet consumed */
			pPacket->PktInfo.AsTx.CreditsUsed = creditsRequired;
			/* save send flags */
			pPacket->PktInfo.AsTx.SendFlags = sendFlags;

			/* queue this packet into the caller's queue */
			HTC_PACKET_ENQUEUE(pQueue, pPacket);
		}
	}

	if (!HTC_QUEUE_EMPTY(&pm_queue))
		queue_htc_pm_packets(pEndpoint, &pm_queue);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("-get_htc_send_packets_credit_based\n"));

}

static void get_htc_send_packets(HTC_TARGET *target,
				 HTC_ENDPOINT *pEndpoint,
				 HTC_PACKET_QUEUE *pQueue, int Resources)
{

	HTC_PACKET *pPacket;
	HTC_PACKET_QUEUE *tx_queue;
	HTC_PACKET_QUEUE pm_queue;
	bool do_pm_get = false;
	wlan_rtpm_dbgid rtpm_dbgid = 0;
	int ret;

	/*** NOTE : the TX lock is held when this function is called ***/
	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+get_htc_send_packets %d resources\n", Resources));

	INIT_HTC_PACKET_QUEUE(&pm_queue);
	extract_htc_pm_packets(pEndpoint, &pm_queue);
	if (HTC_QUEUE_EMPTY(&pm_queue)) {
		tx_queue = &pEndpoint->TxQueue;
		do_pm_get = true;
	} else {
		tx_queue = &pm_queue;
	}

	/* loop until we can grab as many packets out of the queue as we can */
	while (Resources > 0) {
		int num_frags;

		if (do_pm_get) {
			rtpm_dbgid =
				htc_send_pkts_rtpm_dbgid_get(
					pEndpoint->service_id);
			ret = hif_pm_runtime_get(target->hif_dev,
						 rtpm_dbgid, false);
			if (ret) {
				/* bus suspended, runtime resume issued */
				QDF_ASSERT(HTC_PACKET_QUEUE_DEPTH(pQueue) == 0);
				if (ret == -EAGAIN) {
					pPacket = htc_get_pkt_at_head(tx_queue);
					if (!pPacket)
						break;
					log_packet_info(target, pPacket);
				}
				break;
			}
			hif_pm_runtime_update_stats(
					target->hif_dev, rtpm_dbgid,
					HIF_PM_HTC_STATS_GET_HTT_FETCH_PKTS);
		}

		ret = hif_system_pm_state_check(target->hif_dev);
		if (ret) {
			if (do_pm_get) {
				hif_pm_runtime_put(target->hif_dev, rtpm_dbgid);
				hif_pm_runtime_update_stats(
					target->hif_dev, rtpm_dbgid,
					HIF_PM_HTC_STATS_PUT_HTT_FETCH_PKTS);
			}
			break;
		}

		pPacket = htc_packet_dequeue(tx_queue);
		if (!pPacket) {
			if (do_pm_get) {
				hif_pm_runtime_put(target->hif_dev, rtpm_dbgid);
				hif_pm_runtime_update_stats(
					target->hif_dev, rtpm_dbgid,
					HIF_PM_HTC_STATS_PUT_HTT_FETCH_PKTS);
			}
			break;
		}

		AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
				(" Got packet:%pK , New Queue Depth: %d\n",
				 pPacket,
				 HTC_PACKET_QUEUE_DEPTH(tx_queue)));
		/* For non-credit path the sequence number is already embedded
		 * in the constructed HTC header
		 */
		pPacket->PktInfo.AsTx.SendFlags = 0;
		pPacket->PktInfo.AsTx.CreditsUsed = 0;
		/* queue this packet into the caller's queue */
		HTC_PACKET_ENQUEUE(pQueue, pPacket);

		/*
		 * FIX THIS:
		 * For now, avoid calling qdf_nbuf_get_num_frags before calling
		 * qdf_nbuf_map, because the MacOS version of qdf_nbuf_t doesn't
		 * support qdf_nbuf_get_num_frags until after qdf_nbuf_map has
		 * been done.
		 * Assume that the non-data netbufs, i.e. WMI message netbufs,
		 * consist of a single fragment.
		 */
		/* WMI messages are in a single-fragment network buf */
		num_frags =
			(pPacket->PktInfo.AsTx.
			 Flags & HTC_TX_PACKET_FLAG_FIXUP_NETBUF) ? 1 :
			qdf_nbuf_get_num_frags(GET_HTC_PACKET_NET_BUF_CONTEXT
						       (pPacket));
		Resources -= num_frags;
	}

	if (!HTC_QUEUE_EMPTY(&pm_queue))
		queue_htc_pm_packets(pEndpoint, &pm_queue);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-get_htc_send_packets\n"));

}

/**
 * htc_try_send() - Send packets in a queue on an endpoint
 * @target: HTC target on which packets need to be sent
 * @pEndpoint: logical endpoint on which packets needs to be sent
 * @pCallersSendQueue: packet queue containing the list of packets to be sent
 *
 * Return: enum HTC_SEND_QUEUE_RESULT indicates whether the packet was queued to
 *         be sent or the packet should be dropped by the upper layer
 */
static enum HTC_SEND_QUEUE_RESULT htc_try_send(HTC_TARGET *target,
					  HTC_ENDPOINT *pEndpoint,
					  HTC_PACKET_QUEUE *pCallersSendQueue)
{
	/* temp queue to hold packets at various stages */
	HTC_PACKET_QUEUE sendQueue;
	HTC_PACKET *pPacket;
	int tx_resources;
	int overflow;
	enum HTC_SEND_QUEUE_RESULT result = HTC_SEND_QUEUE_OK;
	QDF_STATUS status;

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("+htc_try_send (Queue:%pK Depth:%d)\n",
					 pCallersSendQueue,
					 (pCallersSendQueue ==
					  NULL) ? 0 :
					 HTC_PACKET_QUEUE_DEPTH
						 (pCallersSendQueue)));

	/* init the local send queue */
	INIT_HTC_PACKET_QUEUE(&sendQueue);

	do {

		/* caller didn't provide a queue, just wants us to check
		 * queues and send
		 */
		if (!pCallersSendQueue)
			break;

		if (HTC_QUEUE_EMPTY(pCallersSendQueue)) {
			/* empty queue */
			OL_ATH_HTC_PKT_ERROR_COUNT_INCR(target,
							HTC_PKT_Q_EMPTY);
			result = HTC_SEND_QUEUE_DROP;
			break;
		}

		if (HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue) >=
		    pEndpoint->MaxTxQueueDepth) {
			/* we've already overflowed */
			overflow = HTC_PACKET_QUEUE_DEPTH(pCallersSendQueue);
		} else {
			/* figure out how much we will overflow by */
			overflow = HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue);
			overflow += HTC_PACKET_QUEUE_DEPTH(pCallersSendQueue);
			/* get how much we will overflow the TX queue by */
			overflow -= pEndpoint->MaxTxQueueDepth;
		}

		/* if overflow is negative or zero, we are okay */
		if (overflow > 0) {
			AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
					("Endpoint %d, TX queue will overflow :%d , Tx Depth:%d, Max:%d\n",
					 pEndpoint->Id, overflow,
					 HTC_PACKET_QUEUE_DEPTH(&pEndpoint->
								TxQueue),
					 pEndpoint->MaxTxQueueDepth));
		}
		if ((overflow <= 0)
		    || (!pEndpoint->EpCallBacks.EpSendFull)) {
			/* all packets will fit or caller did not provide send
			 * full indication handler
			 * just move all of them to local sendQueue object
			 */
			HTC_PACKET_QUEUE_TRANSFER_TO_TAIL(&sendQueue,
							  pCallersSendQueue);
		} else {
			int i;
			int goodPkts =
				HTC_PACKET_QUEUE_DEPTH(pCallersSendQueue) -
				overflow;

			A_ASSERT(goodPkts >= 0);
			/* we have overflowed and callback is provided. Dequeue
			 * all non-overflow packets into the sendqueue
			 */
			for (i = 0; i < goodPkts; i++) {
				/* pop off caller's queue */
				pPacket = htc_packet_dequeue(pCallersSendQueue);
				A_ASSERT(pPacket);
				if (pPacket)
					/* insert into local queue */
					HTC_PACKET_ENQUEUE(&sendQueue,
							   pPacket);
			}

			/* the caller's queue has all the packets that won't fit
			 * walk through the caller's queue and indicate each one
			 * to the send full handler
			 */
			ITERATE_OVER_LIST_ALLOW_REMOVE(&pCallersSendQueue->
						       QueueHead, pPacket,
						       HTC_PACKET, ListLink) {

				AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
						("Indicating overflowed TX packet: %pK\n",
						 pPacket));
				/*
				 * Remove headroom reserved for HTC_FRAME_HDR
				 * before giving the packet back to the user via
				 * the EpSendFull callback.
				 */
				qdf_nbuf_pull_head
					(GET_HTC_PACKET_NET_BUF_CONTEXT
						(pPacket),
					sizeof(HTC_FRAME_HDR));
				pPacket->PktInfo.AsTx.Flags &=
					~HTC_TX_PACKET_FLAG_HTC_HEADER_IN_NETBUF_DATA;

				if (pEndpoint->EpCallBacks.
				    EpSendFull(pEndpoint->EpCallBacks.pContext,
					       pPacket) == HTC_SEND_FULL_DROP) {
					/* callback wants the packet dropped */
					INC_HTC_EP_STAT(pEndpoint, TxDropped,
							1);
					/* leave this one in the caller's queue
					 * for cleanup
					 */
				} else {
					/* callback wants to keep this packet,
					 * remove from caller's queue
					 */
					HTC_PACKET_REMOVE(pCallersSendQueue,
							  pPacket);
					/* put it in the send queue
					 * add HTC_FRAME_HDR space reservation
					 * again
					 */
					qdf_nbuf_push_head
						(GET_HTC_PACKET_NET_BUF_CONTEXT
							(pPacket),
						sizeof(HTC_FRAME_HDR));
					pPacket->PktInfo.AsTx.Flags |=
						HTC_TX_PACKET_FLAG_HTC_HEADER_IN_NETBUF_DATA;

					HTC_PACKET_ENQUEUE(&sendQueue, pPacket);
				}

			}
			ITERATE_END;

			if (HTC_QUEUE_EMPTY(&sendQueue)) {
				/* no packets made it in, caller will cleanup */
				OL_ATH_HTC_PKT_ERROR_COUNT_INCR(target,
							HTC_SEND_Q_EMPTY);
				result = HTC_SEND_QUEUE_DROP;
				break;
			}
		}

	} while (false);

	if (result != HTC_SEND_QUEUE_OK) {
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-htc_try_send: %d\n",
			result));
		return result;
	}

	LOCK_HTC_TX(target);

	if (!HTC_QUEUE_EMPTY(&sendQueue)) {
		if (target->is_nodrop_pkt) {
			/*
			 * nodrop pkts have higher priority than normal pkts,
			 * insert nodrop pkt to head for proper
			 * start/termination of test.
			 */
			HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&pEndpoint->TxQueue,
					&sendQueue);
			target->is_nodrop_pkt = false;
		} else {
			/* transfer packets to tail */
			HTC_PACKET_QUEUE_TRANSFER_TO_TAIL(&pEndpoint->TxQueue,
					&sendQueue);
			A_ASSERT(HTC_QUEUE_EMPTY(&sendQueue));
			INIT_HTC_PACKET_QUEUE(&sendQueue);
		}
	}

	/* increment tx processing count on entry */
	if (qdf_atomic_inc_return(&pEndpoint->TxProcessCount) > 1) {
		/* another thread or task is draining the TX queues on this
		 * endpoint that thread will reset the tx processing count when
		 * the queue is drained
		 */
		qdf_atomic_dec(&pEndpoint->TxProcessCount);
		UNLOCK_HTC_TX(target);
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-htc_try_send (busy)\n"));
		return HTC_SEND_QUEUE_OK;
	}

	/***** beyond this point only 1 thread may enter ******/

	/* now drain the endpoint TX queue for transmission as long as we have
	 * enough transmit resources
	 */
	if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
		tx_resources =
			hif_get_free_queue_number(target->hif_dev,
						  pEndpoint->UL_PipeID);
	} else {
		tx_resources = 0;
	}

	while (true) {

		if (HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue) == 0)
			break;

		if (pEndpoint->async_update &&
			(!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) &&
			(!tx_resources))
			break;

		if (IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
#if DEBUG_CREDIT
			int cred = pEndpoint->TxCredits;
#endif
			/* credit based mechanism provides flow control based on
			 * target transmit resource availability, we assume that
			 * the HIF layer will always have bus resources greater
			 * than target transmit resources
			 */
			get_htc_send_packets_credit_based(target, pEndpoint,
							  &sendQueue);
#if DEBUG_CREDIT
			if (ep_debug_mask & (1 << pEndpoint->Id)) {
				if (cred - pEndpoint->TxCredits > 0) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						(" <HTC> Decrease EP%d %d - %d = %d credits.\n",
							 pEndpoint->Id, cred,
							 cred -
							 pEndpoint->TxCredits,
							 pEndpoint->TxCredits));
				}
			}
#endif
		} else {

		/*
		* Header and payload belongs to the different fragments and
		* consume 2 resource for one HTC package but USB combine into
		* one transfer.And one WMI message only consumes one single
		* resource.
		*/
			if (HTC_TX_BUNDLE_ENABLED(target) && tx_resources &&
				hif_get_bus_type(target->hif_dev) ==
						QDF_BUS_TYPE_USB) {
				if (pEndpoint->service_id ==
					WMI_CONTROL_SVC)
					tx_resources =
					    HTC_MAX_MSG_PER_BUNDLE_TX;
				else
					tx_resources =
					    (HTC_MAX_MSG_PER_BUNDLE_TX * 2);
			}
			/* get all the packets for this endpoint that we can for
			 * this pass
			 */
			get_htc_send_packets(target, pEndpoint, &sendQueue,
					     tx_resources);
		}

		if (HTC_PACKET_QUEUE_DEPTH(&sendQueue) == 0) {
			/* didn't get any packets due to a lack of resources or
			 * TX queue was drained
			 */
			break;
		}

		if (!pEndpoint->async_update)
			UNLOCK_HTC_TX(target);

		/* send what we can */
		status = htc_issue_packets(target, pEndpoint, &sendQueue);
		if (status) {
			int i;
			wlan_rtpm_dbgid rtpm_dbgid;

			result = HTC_SEND_QUEUE_DROP;

			switch (status) {
			case  QDF_STATUS_E_RESOURCES:
				if (pEndpoint->num_requeues_warn <= MAX_REQUEUE_WARN) {
					pEndpoint->num_requeues_warn++;
					pEndpoint->total_num_requeues++;
					break;
				} else {
					pEndpoint->total_num_requeues++;
					pEndpoint->num_requeues_warn = 0;
				}
				fallthrough;
			default:
				QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO,
					  "htc_issue_packets, failed status:%d"
					  "endpoint:%d, put it back to head of"
					  "callersSendQueue", result, pEndpoint->Id);
				break;
			}

			rtpm_dbgid =
				htc_send_pkts_rtpm_dbgid_get(
					pEndpoint->service_id);
			for (i = HTC_PACKET_QUEUE_DEPTH(&sendQueue); i > 0; i--) {
				hif_pm_runtime_put(target->hif_dev,
						   rtpm_dbgid);
				hif_pm_runtime_update_stats(
					target->hif_dev, rtpm_dbgid,
					HIF_PM_HTC_STATS_PUT_HTT_ERROR);
			}

			if (!pEndpoint->async_update) {
				LOCK_HTC_TX(target);
			}
			HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&pEndpoint->TxQueue,
							  &sendQueue);
			break;
		}  else {
			if (pEndpoint->num_requeues_warn)
				pEndpoint->num_requeues_warn = 0;
		}

		if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
			tx_resources =
				hif_get_free_queue_number(target->hif_dev,
							  pEndpoint->UL_PipeID);
		}

		if (!pEndpoint->async_update) {
			LOCK_HTC_TX(target);
		}

	}

	/* done with this endpoint, we can clear the count */
	qdf_atomic_init(&pEndpoint->TxProcessCount);

	UNLOCK_HTC_TX(target);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-htc_try_send:\n"));

	return HTC_SEND_QUEUE_OK;
}

#ifdef USB_HIF_SINGLE_PIPE_DATA_SCHED
static uint16_t htc_send_pkts_sched_check(HTC_HANDLE HTCHandle,
					  HTC_ENDPOINT_ID id)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	HTC_ENDPOINT_ID eid;
	HTC_PACKET_QUEUE *pTxQueue;
	uint16_t resources;
	uint16_t acQueueStatus[DATA_EP_SIZE] = { 0, 0, 0, 0 };

	if (id < ENDPOINT_2 || id > ENDPOINT_5)
		return 1;

	for (eid = ENDPOINT_2; eid <= ENDPOINT_5; eid++) {
		pEndpoint = &target->endpoint[eid];
		pTxQueue = &pEndpoint->TxQueue;

		if (HTC_QUEUE_EMPTY(pTxQueue))
			acQueueStatus[eid - 2] = 1;
	}

	switch (id) {
	case ENDPOINT_2:        /* BE */
		return acQueueStatus[0] && acQueueStatus[2]
			&& acQueueStatus[3];
	case ENDPOINT_3:        /* BK */
		return acQueueStatus[0] && acQueueStatus[1] && acQueueStatus[2]
			&& acQueueStatus[3];
	case ENDPOINT_4:        /* VI */
		return acQueueStatus[2] && acQueueStatus[3];
	case ENDPOINT_5:        /* VO */
		return acQueueStatus[3];
	default:
		return 0;
	}

}

static A_STATUS htc_send_pkts_sched_queue(HTC_TARGET *target,
					  HTC_PACKET_QUEUE *pPktQueue,
					  HTC_ENDPOINT_ID eid)
{
	HTC_ENDPOINT *pEndpoint;
	HTC_PACKET_QUEUE *pTxQueue;
	HTC_PACKET *pPacket;
	int goodPkts;

	pEndpoint = &target->endpoint[eid];
	pTxQueue = &pEndpoint->TxQueue;

	LOCK_HTC_TX(target);

	goodPkts =
		pEndpoint->MaxTxQueueDepth -
		HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue);

	if (goodPkts > 0) {
		while (!HTC_QUEUE_EMPTY(pPktQueue)) {
			pPacket = htc_packet_dequeue(pPktQueue);
			HTC_PACKET_ENQUEUE(pTxQueue, pPacket);
			goodPkts--;

			if (goodPkts <= 0)
				break;
		}
	}

	if (HTC_PACKET_QUEUE_DEPTH(pPktQueue)) {
		ITERATE_OVER_LIST_ALLOW_REMOVE(&pPktQueue->QueueHead, pPacket,
					       HTC_PACKET, ListLink) {

			if (pEndpoint->EpCallBacks.
			    EpSendFull(pEndpoint->EpCallBacks.pContext,
				       pPacket) == HTC_SEND_FULL_DROP) {
				INC_HTC_EP_STAT(pEndpoint, TxDropped, 1);
			} else {
				HTC_PACKET_REMOVE(pPktQueue, pPacket);
				HTC_PACKET_ENQUEUE(pTxQueue, pPacket);
			}
		}
		ITERATE_END;
	}

	UNLOCK_HTC_TX(target);

	return A_OK;
}

#endif

static inline QDF_STATUS __htc_send_pkt(HTC_HANDLE HTCHandle,
				HTC_PACKET *pPacket)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	HTC_PACKET_QUEUE pPktQueue;
	qdf_nbuf_t netbuf;
	HTC_FRAME_HDR *htc_hdr;
	QDF_STATUS status;

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+__htc_send_pkt\n"));

	/* get packet at head to figure out which endpoint these packets will
	 * go into
	 */
	if (!pPacket) {
		OL_ATH_HTC_PKT_ERROR_COUNT_INCR(target, GET_HTC_PKT_Q_FAIL);
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-__htc_send_pkt\n"));
		return QDF_STATUS_E_INVAL;
	}

	if ((pPacket->Endpoint >= ENDPOINT_MAX) ||
	    (pPacket->Endpoint <= ENDPOINT_UNUSED)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("%s endpoint is invalid\n", __func__));
		AR_DEBUG_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}
	pEndpoint = &target->endpoint[pPacket->Endpoint];

	if (!pEndpoint->service_id) {
		AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("%s service_id is invalid\n",
								__func__));
		return QDF_STATUS_E_INVAL;
	}

#ifdef HTC_EP_STAT_PROFILING
	LOCK_HTC_TX(target);
	INC_HTC_EP_STAT(pEndpoint, TxPosted, 1);
	UNLOCK_HTC_TX(target);
#endif

	/* provide room in each packet's netbuf for the HTC frame header */
	netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
	AR_DEBUG_ASSERT(netbuf);
	if (!netbuf)
		return QDF_STATUS_E_INVAL;

	qdf_nbuf_push_head(netbuf, sizeof(HTC_FRAME_HDR));
	pPacket->PktInfo.AsTx.Flags |=
		HTC_TX_PACKET_FLAG_HTC_HEADER_IN_NETBUF_DATA;
	/* setup HTC frame header */
	htc_hdr = (HTC_FRAME_HDR *)qdf_nbuf_get_frag_vaddr(netbuf, 0);
	AR_DEBUG_ASSERT(htc_hdr);
	if (!htc_hdr)
		return QDF_STATUS_E_INVAL;

	HTC_WRITE32(htc_hdr,
		    SM(pPacket->ActualLength,
		       HTC_FRAME_HDR_PAYLOADLEN) |
		    SM(pPacket->Endpoint,
		       HTC_FRAME_HDR_ENDPOINTID));
	LOCK_HTC_TX(target);

	pPacket->PktInfo.AsTx.SeqNo = pEndpoint->SeqNo;
	pEndpoint->SeqNo++;

	HTC_WRITE32(((uint32_t *)htc_hdr) + 1,
		    SM(pPacket->PktInfo.AsTx.SeqNo,
		       HTC_FRAME_HDR_CONTROLBYTES1));

	UNLOCK_HTC_TX(target);

	/*
	 * For flow control enabled endpoints mapping is done in
	 * htc_issue_packets and for non flow control enabled endpoints
	 * its done here.
	 */
	if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
		pPacket->PktInfo.AsTx.Flags |= HTC_TX_PACKET_FLAG_FIXUP_NETBUF;
		status = qdf_nbuf_map(target->osdev,
				      GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket),
				      QDF_DMA_TO_DEVICE);
		if (status == QDF_STATUS_SUCCESS) {
			target->nbuf_nfc_map_count++;
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: nbuf map failed, endpoint %pK, seq_no. %d\n",
					 __func__, pEndpoint, pEndpoint->SeqNo));
			return status;
		}
	}

	INIT_HTC_PACKET_QUEUE_AND_ADD(&pPktQueue, pPacket);
#ifdef USB_HIF_SINGLE_PIPE_DATA_SCHED
	if (!htc_send_pkts_sched_check(HTCHandle, pEndpoint->Id))
		htc_send_pkts_sched_queue(HTCHandle, &pPktQueue, pEndpoint->Id);
	else
		htc_try_send(target, pEndpoint, &pPktQueue);
#else
	htc_try_send(target, pEndpoint, &pPktQueue);
#endif

	/* do completion on any packets that couldn't get in */
	while (!HTC_QUEUE_EMPTY(&pPktQueue)) {
		pPacket = htc_packet_dequeue(&pPktQueue);

		if (HTC_STOPPING(target))
			pPacket->Status = QDF_STATUS_E_CANCELED;
		else
			pPacket->Status = QDF_STATUS_E_RESOURCES;

		send_packet_completion(target, pPacket);
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-__htc_send_pkt\n"));

	return QDF_STATUS_SUCCESS;
}

/* HTC API - htc_send_pkt */
QDF_STATUS htc_send_pkt(HTC_HANDLE htc_handle, HTC_PACKET *htc_packet)
{
	if (!htc_handle) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: HTCHandle is NULL \n", __func__));
		return QDF_STATUS_E_FAILURE;
	}

	if (!htc_packet) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: pPacket is NULL \n", __func__));
		return QDF_STATUS_E_FAILURE;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+-htc_send_pkt: Enter endPointId: %d, buffer: %pK, length: %d\n",
			 htc_packet->Endpoint, htc_packet->pBuffer,
			 htc_packet->ActualLength));
	return __htc_send_pkt(htc_handle, htc_packet);
}
qdf_export_symbol(htc_send_pkt);

#ifdef ATH_11AC_TXCOMPACT
/**
 * htc_send_data_pkt() - send single data packet on an endpoint
 * @HTCHandle: pointer to HTC handle
 * @netbuf: network buffer containing the data to be sent
 * @ActualLength: length of data that needs to be transmitted
 *
 * Return: QDF_STATUS_SUCCESS for success or an appropriate QDF_STATUS error
 */
QDF_STATUS htc_send_data_pkt(HTC_HANDLE htc_hdl, qdf_nbuf_t netbuf, int ep_id,
			     int actual_length)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_hdl);
	HTC_ENDPOINT *pEndpoint;
	HTC_FRAME_HDR *p_htc_hdr;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int tx_resources;
	uint32_t data_attr = 0;
	int htc_payload_len = actual_length;
	wlan_rtpm_dbgid rtpm_dbgid;

	pEndpoint = &target->endpoint[ep_id];

	tx_resources = hif_get_free_queue_number(target->hif_dev,
						 pEndpoint->UL_PipeID);

	if (tx_resources < HTC_DATA_RESOURCE_THRS) {
		if (pEndpoint->ul_is_polled) {
			hif_send_complete_check(pEndpoint->target->hif_dev,
						pEndpoint->UL_PipeID, 1);
			tx_resources =
				hif_get_free_queue_number(target->hif_dev,
							  pEndpoint->UL_PipeID);
		}
		if (tx_resources < HTC_DATA_MINDESC_PERPACKET)
			return QDF_STATUS_E_FAILURE;
	}

	rtpm_dbgid =
		htc_send_pkts_rtpm_dbgid_get(pEndpoint->service_id);
	if (hif_pm_runtime_get(target->hif_dev, rtpm_dbgid, false))
		return QDF_STATUS_E_FAILURE;

	p_htc_hdr = (HTC_FRAME_HDR *)qdf_nbuf_get_frag_vaddr(netbuf, 0);
	AR_DEBUG_ASSERT(p_htc_hdr);

	data_attr = qdf_nbuf_data_attr_get(netbuf);

	if (target->htc_hdr_length_check)
		htc_payload_len = actual_length - HTC_HEADER_LEN;

	HTC_WRITE32(p_htc_hdr, SM(htc_payload_len, HTC_FRAME_HDR_PAYLOADLEN)
		    | SM(ep_id, HTC_FRAME_HDR_ENDPOINTID));
	/*
	 * If the HIF pipe for the data endpoint is polled rather than
	 * interrupt-driven, this is a good point to check whether any
	 * data previously sent through the HIF pipe have finished being
	 * sent.
	 * Since this may result in callbacks to htc_tx_completion_handler,
	 * which can take the HTC tx lock, make the hif_send_complete_check
	 * call before acquiring the HTC tx lock.
	 * Call hif_send_complete_check directly, rather than calling
	 * htc_send_complete_check, and call the PollTimerStart separately
	 * after calling hif_send_head, so the timer will be started to
	 * check for completion of the new outstanding download (in the
	 * unexpected event that other polling calls don't catch it).
	 */

	LOCK_HTC_TX(target);

	HTC_WRITE32(((uint32_t *)p_htc_hdr) + 1,
		    SM(pEndpoint->SeqNo, HTC_FRAME_HDR_CONTROLBYTES1));

	pEndpoint->SeqNo++;

	QDF_NBUF_UPDATE_TX_PKT_COUNT(netbuf, QDF_NBUF_TX_PKT_HTC);
	DPTRACE(qdf_dp_trace(netbuf, QDF_DP_TRACE_HTC_PACKET_PTR_RECORD,
		QDF_TRACE_DEFAULT_PDEV_ID, qdf_nbuf_data_addr(netbuf),
		sizeof(qdf_nbuf_data(netbuf)), QDF_TX));
	status = hif_send_head(target->hif_dev,
			       pEndpoint->UL_PipeID,
			       pEndpoint->Id, actual_length, netbuf, data_attr);

	UNLOCK_HTC_TX(target);
	return status;
}
#else                           /*ATH_11AC_TXCOMPACT */

/**
 * htc_send_data_pkt() - htc_send_data_pkt
 * @HTCHandle: pointer to HTC handle
 * @pPacket: pointer to HTC_PACKET
 * @more_data: indicates whether more data is to follow
 *
 * Return: QDF_STATUS_SUCCESS for success or an appropriate QDF_STATUS error
 */
QDF_STATUS htc_send_data_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket,
			   uint8_t more_data)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	HTC_FRAME_HDR *pHtcHdr;
	HTC_PACKET_QUEUE sendQueue;
	qdf_nbuf_t netbuf = NULL;
	int tx_resources;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t data_attr = 0;
	bool used_extra_tx_credit = false;

	if (pPacket) {
		if ((pPacket->Endpoint >= ENDPOINT_MAX) ||
		   (pPacket->Endpoint <= ENDPOINT_UNUSED)) {
			AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
				("%s endpoint is invalid\n", __func__));
			AR_DEBUG_ASSERT(0);
			return QDF_STATUS_E_INVAL;
		}
		pEndpoint = &target->endpoint[pPacket->Endpoint];

		/* add HTC_FRAME_HDR in the initial fragment */
		netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
		pHtcHdr = (HTC_FRAME_HDR *) qdf_nbuf_get_frag_vaddr(netbuf, 0);
		AR_DEBUG_ASSERT(pHtcHdr);

		HTC_WRITE32(pHtcHdr,
				SM(pPacket->ActualLength,
				       HTC_FRAME_HDR_PAYLOADLEN) |
				SM(pPacket->PktInfo.AsTx.SendFlags,
					HTC_FRAME_HDR_FLAGS) |
				SM(pPacket->Endpoint,
					HTC_FRAME_HDR_ENDPOINTID));
		/*
		 * If the HIF pipe for the data endpoint is polled rather than
		 * interrupt-driven, this is a good point to check whether any
		 * data previously sent through the HIF pipe have finished being
		 * sent. Since this may result in callbacks to
		 * htc_tx_completion_handler, which can take the HTC tx lock,
		 * make the hif_send_complete_check call before acquiring the
		 * HTC tx lock.
		 * Call hif_send_complete_check directly, rather than calling
		 * htc_send_complete_check, and call the PollTimerStart
		 * separately after calling hif_send_head, so the timer will be
		 * started to check for completion of the new outstanding
		 * download (in the unexpected event that other polling calls
		 * don't catch it).
		 */
		if (pEndpoint->ul_is_polled) {
			htc_send_complete_poll_timer_stop(pEndpoint);
			hif_send_complete_check(pEndpoint->target->hif_dev,
						pEndpoint->UL_PipeID, 0);
		}

		LOCK_HTC_TX(target);

		pPacket->PktInfo.AsTx.SeqNo = pEndpoint->SeqNo;
		pEndpoint->SeqNo++;

		HTC_WRITE32(((uint32_t *) pHtcHdr) + 1,
			    SM(pPacket->PktInfo.AsTx.SeqNo,
			       HTC_FRAME_HDR_CONTROLBYTES1));

		/* append new packet to pEndpoint->TxQueue */
		HTC_PACKET_ENQUEUE(&pEndpoint->TxQueue, pPacket);
		if (HTC_TX_BUNDLE_ENABLED(target) && (more_data)) {
			UNLOCK_HTC_TX(target);
			return QDF_STATUS_SUCCESS;
		}

		QDF_NBUF_UPDATE_TX_PKT_COUNT(netbuf, QDF_NBUF_TX_PKT_HTC);
		DPTRACE(qdf_dp_trace(netbuf, QDF_DP_TRACE_HTC_PACKET_PTR_RECORD,
			  QDF_TRACE_DEFAULT_PDEV_ID, qdf_nbuf_data_addr(netbuf),
				sizeof(qdf_nbuf_data(netbuf)), QDF_TX));
	} else {
		LOCK_HTC_TX(target);
		pEndpoint = &target->endpoint[1];
	}

	/* increment tx processing count on entry */
	qdf_atomic_inc(&pEndpoint->TxProcessCount);
	if (qdf_atomic_read(&pEndpoint->TxProcessCount) > 1) {
		/*
		 * Another thread or task is draining the TX queues on this
		 * endpoint. That thread will reset the tx processing count when
		 * the queue is drained.
		 */
		qdf_atomic_dec(&pEndpoint->TxProcessCount);
		UNLOCK_HTC_TX(target);
		return QDF_STATUS_SUCCESS;
	}

	/***** beyond this point only 1 thread may enter ******/

	INIT_HTC_PACKET_QUEUE(&sendQueue);
	if (IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
#if DEBUG_CREDIT
		int cred = pEndpoint->TxCredits;
#endif
		get_htc_send_packets_credit_based(target, pEndpoint,
						 &sendQueue);
#if DEBUG_CREDIT
		if (ep_debug_mask & (1 << pEndpoint->Id)) {
			if (cred - pEndpoint->TxCredits > 0) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						(" <HTC> Decrease EP%d %d - %d = %d credits.\n",
						 pEndpoint->Id, cred,
						 cred - pEndpoint->TxCredits,
						 pEndpoint->TxCredits));
			}
		}
#endif
		UNLOCK_HTC_TX(target);
	}

	else if (HTC_TX_BUNDLE_ENABLED(target)) {
		if (hif_get_bus_type(target->hif_dev) == QDF_BUS_TYPE_USB) {
			if (hif_get_free_queue_number(target->hif_dev,
						      pEndpoint->UL_PipeID))
				/*
				 * Header and payload belongs to the different
				 * fragments and consume 2 resource for one HTC
				 * package but USB combine into one transfer.
				 */
				get_htc_send_packets(target, pEndpoint,
						     &sendQueue,
						     HTC_MAX_MSG_PER_BUNDLE_TX
						     * 2);
		} else {
			/* Dequeue max packets from endpoint tx queue */
			get_htc_send_packets(target, pEndpoint, &sendQueue,
					     HTC_MAX_TX_BUNDLE_SEND_LIMIT);
		}
		UNLOCK_HTC_TX(target);
	} else {
		/*
		 * Now drain the endpoint TX queue for transmission as long as
		 * we have enough transmit resources
		 */
		tx_resources =
			hif_get_free_queue_number(target->hif_dev,
						  pEndpoint->UL_PipeID);
		get_htc_send_packets(target, pEndpoint, &sendQueue,
				     tx_resources);
		UNLOCK_HTC_TX(target);
	}

	/* send what we can */
	while (true) {
		if (HTC_TX_BUNDLE_ENABLED(target) &&
		    (HTC_PACKET_QUEUE_DEPTH(&sendQueue) >=
		     HTC_MIN_MSG_PER_BUNDLE) &&
		    (hif_get_bus_type(target->hif_dev) == QDF_BUS_TYPE_SDIO ||
		     hif_get_bus_type(target->hif_dev) == QDF_BUS_TYPE_USB)) {
			if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
				if (htc_tx_pad_credit_avail(pEndpoint) < 1) {
					status = QDF_STATUS_E_RESOURCES;
					/* put the sendQueue back at the front
					 * of pEndpoint->TxQueue
					 */
					LOCK_HTC_TX(target);
					HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(
							&pEndpoint->TxQueue,
							&sendQueue);
					UNLOCK_HTC_TX(target);
					break;
				}
			}
			htc_issue_packets_bundle(target, pEndpoint, &sendQueue);
		}
		if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
			if (htc_tx_pad_credit_avail(pEndpoint) < 1) {
				status = QDF_STATUS_E_RESOURCES;
				/* put the sendQueue back at the front
				 * of pEndpoint->TxQueue
				 */
				LOCK_HTC_TX(target);
				HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(
							&pEndpoint->TxQueue,
							&sendQueue);
				UNLOCK_HTC_TX(target);
				break;
			}
		}
		pPacket = htc_packet_dequeue(&sendQueue);
		if (!pPacket)
			break;
		netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
		pHtcHdr = (HTC_FRAME_HDR *)qdf_nbuf_get_frag_vaddr(netbuf, 0);

		LOCK_HTC_TX(target);
		/* store in look up queue to match completions */
		HTC_PACKET_ENQUEUE(&pEndpoint->TxLookupQueue, pPacket);
		INC_HTC_EP_STAT(pEndpoint, TxIssued, 1);
		pEndpoint->ul_outstanding_cnt++;
		UNLOCK_HTC_TX(target);

		used_extra_tx_credit =
				htc_handle_extra_tx_credit(pEndpoint, pPacket,
							   (uint8_t *)pHtcHdr,
							   NULL,
							   pPacket->ActualLength
							   + HTC_HDR_LENGTH);

		status = hif_send_head(target->hif_dev,
				       pEndpoint->UL_PipeID,
				       pEndpoint->Id,
				       HTC_HDR_LENGTH + pPacket->ActualLength,
				       netbuf, data_attr);
		if (status != QDF_STATUS_SUCCESS) {
			if (pEndpoint->EpCallBacks.ep_padding_credit_update) {
				if (used_extra_tx_credit) {
					pEndpoint->EpCallBacks.
					ep_padding_credit_update
					(pEndpoint->EpCallBacks.pContext, 1);
				}
			}
		}
#if DEBUG_BUNDLE
		qdf_print(" Send single EP%d buffer size:0x%x, total:0x%x.",
			  pEndpoint->Id,
			  pEndpoint->TxCreditSize,
			  HTC_HDR_LENGTH + pPacket->ActualLength);
#endif

		htc_issue_tx_bundle_stats_inc(target);

		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			LOCK_HTC_TX(target);
			pEndpoint->ul_outstanding_cnt--;
			/* remove this packet from the tx completion queue */
			HTC_PACKET_REMOVE(&pEndpoint->TxLookupQueue, pPacket);

			/*
			 * Don't bother reclaiming credits - HTC flow control
			 * is not applicable to tx data.
			 * In LL systems, there is no download flow control,
			 * since there's virtually no download delay.
			 * In HL systems, the txrx SW explicitly performs the
			 * tx flow control.
			 */
			/* pEndpoint->TxCredits +=
			 * pPacket->PktInfo.AsTx.CreditsUsed;
			 */

			/* put this frame back at the front of the sendQueue */
			HTC_PACKET_ENQUEUE_TO_HEAD(&sendQueue, pPacket);

			/* put the sendQueue back at the front of
			 * pEndpoint->TxQueue
			 */
			HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&pEndpoint->TxQueue,
							  &sendQueue);
			UNLOCK_HTC_TX(target);
			break;  /* still need to reset TxProcessCount */
		}
	}
	/* done with this endpoint, we can clear the count */
	qdf_atomic_init(&pEndpoint->TxProcessCount);

	if (pEndpoint->ul_is_polled) {
		/*
		 * Start a cleanup timer to poll for download completion.
		 * The download completion should be noticed promptly from
		 * other polling calls, but the timer provides a safety net
		 * in case other polling calls don't occur as expected.
		 */
		htc_send_complete_poll_timer_start(pEndpoint);
	}

	return status;
}
#endif /*ATH_11AC_TXCOMPACT */
qdf_export_symbol(htc_send_data_pkt);

/*
 * In the adapted HIF layer, qdf_nbuf_t are passed between HIF and HTC,
 * since upper layers expects HTC_PACKET containers we use the completed netbuf
 * and lookup its corresponding HTC packet buffer from a lookup list.
 * This is extra overhead that can be fixed by re-aligning HIF interfaces
 * with HTC.
 *
 */
static HTC_PACKET *htc_lookup_tx_packet(HTC_TARGET *target,
					HTC_ENDPOINT *pEndpoint,
					qdf_nbuf_t netbuf)
{
	HTC_PACKET *pPacket = NULL;
	HTC_PACKET *pFoundPacket = NULL;
	HTC_PACKET_QUEUE lookupQueue;

	INIT_HTC_PACKET_QUEUE(&lookupQueue);
	LOCK_HTC_EP_TX_LOOKUP(pEndpoint);

	LOCK_HTC_TX(target);
	/* mark that HIF has indicated the send complete for another packet */
	pEndpoint->ul_outstanding_cnt--;

	/* Dequeue first packet directly because of in-order completion */
	pPacket = htc_packet_dequeue(&pEndpoint->TxLookupQueue);
	if (qdf_unlikely(!pPacket)) {
		UNLOCK_HTC_TX(target);
		UNLOCK_HTC_EP_TX_LOOKUP(pEndpoint);
		return NULL;
	}
	if (netbuf == (qdf_nbuf_t) GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket)) {
		UNLOCK_HTC_TX(target);
		UNLOCK_HTC_EP_TX_LOOKUP(pEndpoint);
		return pPacket;
	}
	HTC_PACKET_ENQUEUE(&lookupQueue, pPacket);

	/*
	 * Move TX lookup queue to temp queue because most of packets that are
	 * not index 0 are not top 10 packets.
	 */
	HTC_PACKET_QUEUE_TRANSFER_TO_TAIL(&lookupQueue,
					  &pEndpoint->TxLookupQueue);

	ITERATE_OVER_LIST_ALLOW_REMOVE(&lookupQueue.QueueHead, pPacket,
				       HTC_PACKET, ListLink) {

		if (!pPacket) {
			pFoundPacket = pPacket;
			break;
		}
		/* check for removal */
		if (netbuf ==
		    (qdf_nbuf_t) GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket)) {
			/* found it */
			HTC_PACKET_REMOVE(&lookupQueue, pPacket);
			pFoundPacket = pPacket;
			break;
		}

	}
	ITERATE_END;

	HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(&pEndpoint->TxLookupQueue,
					  &lookupQueue);
	UNLOCK_HTC_TX(target);
	UNLOCK_HTC_EP_TX_LOOKUP(pEndpoint);

	return pFoundPacket;
}

/**
 * htc_tx_completion_handler() - htc tx completion handler
 * @Context: pointer to HTC_TARGET structure
 * @netbuf: pointer to netbuf for which completion handler is being called
 * @EpID: end point Id on which the packet was sent
 * @toeplitz_hash_result: toeplitz hash result
 *
 * Return: QDF_STATUS_SUCCESS for success or an appropriate QDF_STATUS error
 */
QDF_STATUS htc_tx_completion_handler(void *Context,
				   qdf_nbuf_t netbuf, unsigned int EpID,
				   uint32_t toeplitz_hash_result)
{
	HTC_TARGET *target = (HTC_TARGET *) Context;
	HTC_ENDPOINT *pEndpoint;
	HTC_PACKET *pPacket;
#ifdef USB_HIF_SINGLE_PIPE_DATA_SCHED
	HTC_ENDPOINT_ID eid[DATA_EP_SIZE] = { ENDPOINT_5, ENDPOINT_4,
		ENDPOINT_2, ENDPOINT_3 };
	int epidIdx;
	uint16_t resourcesThresh[DATA_EP_SIZE]; /* urb resources */
	uint16_t resources;
	uint16_t resourcesMax;
#endif

	pEndpoint = &target->endpoint[EpID];
	target->TX_comp_cnt++;
	pEndpoint->htc_comp_cnt++;

	do {
		pPacket = htc_lookup_tx_packet(target, pEndpoint, netbuf);
		if (!pPacket) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC TX lookup failed!\n"));
			/* may have already been flushed and freed */
			netbuf = NULL;
			break;
		}
		if (pPacket->PktInfo.AsTx.Tag != HTC_TX_PACKET_TAG_AUTO_PM &&
		    pPacket->PktInfo.AsTx.Tag != HTC_TX_PACKET_TAG_RUNTIME_PUT &&
		    pPacket->PktInfo.AsTx.Tag != HTC_TX_PACKET_TAG_RTPM_PUT_RC)
			hif_pm_runtime_put(target->hif_dev,
					   RTPM_ID_WMI);

		if (pPacket->PktInfo.AsTx.Tag == HTC_TX_PACKET_TAG_BUNDLED) {
			HTC_PACKET *pPacketTemp;
			HTC_PACKET_QUEUE *pQueueSave =
				(HTC_PACKET_QUEUE *) pPacket->pContext;
			HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(pQueueSave,
							      pPacketTemp) {
				pPacket->Status = QDF_STATUS_SUCCESS;
				send_packet_completion(target, pPacketTemp);
			}
			HTC_PACKET_QUEUE_ITERATE_END;
			free_htc_bundle_packet(target, pPacket);

			if (hif_get_bus_type(target->hif_dev) ==
					     QDF_BUS_TYPE_USB) {
				if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint))
					htc_try_send(target, pEndpoint, NULL);
			}

			return QDF_STATUS_SUCCESS;
		}
		/* will be giving this buffer back to upper layers */
		netbuf = NULL;
		pPacket->Status = QDF_STATUS_SUCCESS;
		send_packet_completion(target, pPacket);

	} while (false);

	if (!IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) {
		/* note: when using TX credit flow, the re-checking of queues
		 * happens when credits flow back from the target. In the non-TX
		 * credit case, we recheck after the packet completes
		 */
		if ((qdf_atomic_read(&pEndpoint->TxProcessCount) == 0) ||
				(!pEndpoint->async_update)) {
			htc_try_send(target, pEndpoint, NULL);
		}
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * htc_ctrl_msg_cmpl(): checks for tx completion for the endpoint specified
 * @HTC_HANDLE : pointer to the htc target context
 * @htc_ep_id  : end point id
 *
 * checks HTC tx completion
 *
 * Return: none
 */
void htc_ctrl_msg_cmpl(HTC_HANDLE htc_pdev, HTC_ENDPOINT_ID htc_ep_id)
{
	HTC_TARGET      *target = GET_HTC_TARGET_FROM_HANDLE(htc_pdev);
	HTC_ENDPOINT    *pendpoint = &target->endpoint[htc_ep_id];

	htc_send_complete_check(pendpoint, 1);
}
qdf_export_symbol(htc_ctrl_msg_cmpl);
#endif

/* callback when TX resources become available */
void htc_tx_resource_avail_handler(void *context, uint8_t pipeID)
{
	int i;
	HTC_TARGET *target = (HTC_TARGET *) context;
	HTC_ENDPOINT *pEndpoint = NULL;

	for (i = 0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		if (pEndpoint->service_id != 0) {
			if (pEndpoint->UL_PipeID == pipeID)
				break;
		}
	}

	if (i >= ENDPOINT_MAX) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("Invalid pipe indicated for TX resource avail : %d!\n",
				 pipeID));
		return;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("HIF indicated more resources for pipe:%d\n",
			 pipeID));

	htc_try_send(target, pEndpoint, NULL);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * htc_kick_queues(): resumes tx transactions of suspended endpoints
 * @context: pointer to the htc target context
 *
 * Iterates through the enpoints and provides a context to empty queues
 * int the hif layer when they are stalled due to runtime suspend.
 *
 * Return: none
 */
void htc_kick_queues(void *context)
{
	int i;
	HTC_TARGET *target = (HTC_TARGET *)context;
	HTC_ENDPOINT *endpoint = NULL;

	if (hif_pm_runtime_get_sync(target->hif_dev, RTPM_ID_HTC))
		return;

	hif_pm_runtime_update_stats(
			target->hif_dev, RTPM_ID_HTC,
			HIF_PM_HTC_STATS_GET_HTC_KICK_QUEUES);

	for (i = 0; i < ENDPOINT_MAX; i++) {
		endpoint = &target->endpoint[i];

		if (endpoint->service_id == 0)
			continue;

		if (endpoint->EpCallBacks.ep_resume_tx_queue)
			endpoint->EpCallBacks.ep_resume_tx_queue(
					endpoint->EpCallBacks.pContext);

		htc_try_send(target, endpoint, NULL);
	}

	hif_fastpath_resume(target->hif_dev);

	if (hif_pm_runtime_put(target->hif_dev, RTPM_ID_HTC))
		return;

	hif_pm_runtime_update_stats(
			target->hif_dev, RTPM_ID_HTC,
			HIF_PM_HTC_STATS_PUT_HTC_KICK_QUEUES);
}
#endif

/* flush endpoint TX queue */
void htc_flush_endpoint_tx(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint,
			   HTC_TX_TAG Tag)
{
	HTC_PACKET *pPacket;

	LOCK_HTC_TX(target);
	while (HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue)) {
		pPacket = htc_packet_dequeue(&pEndpoint->TxQueue);

		if (pPacket) {
			/* let the sender know the packet was not delivered */
			pPacket->Status = QDF_STATUS_E_CANCELED;
			send_packet_completion(target, pPacket);
		}
	}
	UNLOCK_HTC_TX(target);
}

/* flush pending entries in endpoint TX Lookup queue */
void htc_flush_endpoint_txlookupQ(HTC_TARGET *target,
				  HTC_ENDPOINT_ID endpoint_id,
				  bool call_ep_callback)
{
	HTC_PACKET *packet;
	HTC_ENDPOINT *endpoint;

	endpoint = &target->endpoint[endpoint_id];

	if (!endpoint && endpoint->service_id == 0)
		return;

	LOCK_HTC_TX(target);
	while (HTC_PACKET_QUEUE_DEPTH(&endpoint->TxLookupQueue)) {
		packet = htc_packet_dequeue(&endpoint->TxLookupQueue);

		if (packet) {
			if (call_ep_callback == true) {
				packet->Status = QDF_STATUS_E_CANCELED;
				send_packet_completion(target, packet);
			} else {
				qdf_mem_free(packet);
			}
		}
	}
	UNLOCK_HTC_TX(target);
}

/* HTC API to flush an endpoint's TX queue*/
void htc_flush_endpoint(HTC_HANDLE HTCHandle, HTC_ENDPOINT_ID Endpoint,
			HTC_TX_TAG Tag)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint = &target->endpoint[Endpoint];

	if (pEndpoint->service_id == 0) {
		AR_DEBUG_ASSERT(false);
		/* not in use.. */
		return;
	}

	htc_flush_endpoint_tx(target, pEndpoint, Tag);
}

/* HTC API to indicate activity to the credit distribution function */
void htc_indicate_activity_change(HTC_HANDLE HTCHandle,
				  HTC_ENDPOINT_ID Endpoint, bool Active)
{
	/*  TODO */
}

bool htc_is_endpoint_active(HTC_HANDLE HTCHandle, HTC_ENDPOINT_ID Endpoint)
{
	return true;
}

void htc_set_pkt_dbg(HTC_HANDLE handle, A_BOOL dbg_flag)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(handle);

	target->htc_pkt_dbg = dbg_flag;
}

void htc_set_nodrop_pkt(HTC_HANDLE HTCHandle, A_BOOL isNodropPkt)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	target->is_nodrop_pkt = isNodropPkt;
}

void htc_enable_hdr_length_check(HTC_HANDLE htc_hdl, bool htc_hdr_length_check)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_hdl);

	target->htc_hdr_length_check = htc_hdr_length_check;
}

/**
 * htc_process_credit_rpt() - process credit report, call distribution function
 * @target: pointer to HTC_TARGET
 * @pRpt: pointer to HTC_CREDIT_REPORT
 * @NumEntries: number of entries in credit report
 * @FromEndpoint: endpoint for which  credit report is received
 *
 * Return: A_OK for success or an appropriate A_STATUS error
 */
void htc_process_credit_rpt(HTC_TARGET *target, HTC_CREDIT_REPORT *pRpt,
			    int NumEntries, HTC_ENDPOINT_ID FromEndpoint)
{
	int i;
	HTC_ENDPOINT *pEndpoint;
	int totalCredits = 0;
	uint8_t rpt_credits, rpt_ep_id;

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("+htc_process_credit_rpt, Credit Report Entries:%d\n",
			 NumEntries));

	/* lock out TX while we update credits */
	LOCK_HTC_TX(target);

	for (i = 0; i < NumEntries; i++, pRpt++) {

		rpt_ep_id = HTC_GET_FIELD(pRpt, HTC_CREDIT_REPORT, ENDPOINTID);

		if (rpt_ep_id >= ENDPOINT_MAX) {
			AR_DEBUG_ASSERT(false);
			break;
		}

		rpt_credits = HTC_GET_FIELD(pRpt, HTC_CREDIT_REPORT, CREDITS);

		pEndpoint = &target->endpoint[rpt_ep_id];
#if DEBUG_CREDIT
		if (ep_debug_mask & (1 << pEndpoint->Id)) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					(" <HTC> Increase EP%d %d + %d = %d credits\n",
					 rpt_ep_id, pEndpoint->TxCredits,
					 rpt_credits,
					 pEndpoint->TxCredits + rpt_credits));
		}
#endif

#ifdef HTC_EP_STAT_PROFILING

		INC_HTC_EP_STAT(pEndpoint, TxCreditRpts, 1);
		INC_HTC_EP_STAT(pEndpoint, TxCreditsReturned, rpt_credits);

		if (FromEndpoint == rpt_ep_id) {
			/* this credit report arrived on the same endpoint
			 * indicating it arrived in an RX packet
			 */
			INC_HTC_EP_STAT(pEndpoint, TxCreditsFromRx,
					rpt_credits);
			INC_HTC_EP_STAT(pEndpoint, TxCreditRptsFromRx, 1);
		} else if (FromEndpoint == ENDPOINT_0) {
			/* this credit arrived on endpoint 0 as a NULL msg */
			INC_HTC_EP_STAT(pEndpoint, TxCreditsFromEp0,
					rpt_credits);
			INC_HTC_EP_STAT(pEndpoint, TxCreditRptsFromEp0, 1);
		} else {
			/* arrived on another endpoint */
			INC_HTC_EP_STAT(pEndpoint, TxCreditsFromOther,
					rpt_credits);
			INC_HTC_EP_STAT(pEndpoint, TxCreditRptsFromOther, 1);
		}

#endif

		if (pEndpoint->service_id == WMI_CONTROL_SVC) {
			htc_credit_record(HTC_PROCESS_CREDIT_REPORT,
					  pEndpoint->TxCredits + rpt_credits,
					  HTC_PACKET_QUEUE_DEPTH(&pEndpoint->
							TxQueue));
			hif_latency_detect_credit_record_time(
					HIF_PROCESS_CREDIT_REPORT,
					target->hif_dev);
		}

		pEndpoint->TxCredits += rpt_credits;

		if (pEndpoint->TxCredits
		    && HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue)) {
			UNLOCK_HTC_TX(target);
#ifdef ATH_11AC_TXCOMPACT
			htc_try_send(target, pEndpoint, NULL);
#else
			if (pEndpoint->service_id == HTT_DATA_MSG_SVC)
				htc_send_data_pkt((HTC_HANDLE)target, NULL, 0);
			else
				htc_try_send(target, pEndpoint, NULL);
#endif
			LOCK_HTC_TX(target);
		}
		totalCredits += rpt_credits;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND,
			("  Report indicated %d credits to distribute\n",
			 totalCredits));

	UNLOCK_HTC_TX(target);

	AR_DEBUG_PRINTF(ATH_DEBUG_SEND, ("-htc_process_credit_rpt\n"));
}

/* function to fetch stats from htc layer*/
struct ol_ath_htc_stats *ieee80211_ioctl_get_htc_stats(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	return &(target->htc_pkt_stats);
}

#ifdef SYSTEM_PM_CHECK
void htc_system_resume(HTC_HANDLE htc)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc);
	HTC_ENDPOINT *endpoint = NULL;
	int i;

	if (!target)
		return;

	for (i = 0; i < ENDPOINT_MAX; i++) {
		endpoint = &target->endpoint[i];

		if (endpoint->service_id == 0)
			continue;

		htc_try_send(target, endpoint, NULL);
	}
}
#endif
