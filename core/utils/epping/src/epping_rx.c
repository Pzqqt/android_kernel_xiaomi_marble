/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*========================================================================

   \file  epping_main.c

   \brief WLAN End Point Ping test tool implementation

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cds_api.h>
#include <cds_sched.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <wni_api.h>
#include <wlan_ptt_sock_svc.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <linux/rtnetlink.h>
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include "epping_main.h"
#include "epping_internal.h"
#include "epping_test.h"
#include <wlan_hdd_napi.h>

#define AR6000_MAX_RX_BUFFERS 16
#define AR6000_BUFFER_SIZE 1664
#define AR6000_MIN_HEAD_ROOM 64

static bool enb_rx_dump;

#ifdef HIF_SDIO
void epping_refill(void *ctx, HTC_ENDPOINT_ID Endpoint)
{
	epping_context_t *pEpping_ctx = (epping_context_t *) ctx;
	void *osBuf;
	int RxBuffers;
	int buffersToRefill;
	HTC_PACKET *pPacket;
	HTC_PACKET_QUEUE queue;

	buffersToRefill = (int)AR6000_MAX_RX_BUFFERS -
			  htc_get_num_recv_buffers(pEpping_ctx->HTCHandle, Endpoint);

	if (buffersToRefill <= 0) {
		/* fast return, nothing to fill */
		return;
	}

	INIT_HTC_PACKET_QUEUE(&queue);

	EPPING_LOG(QDF_TRACE_LEVEL_INFO,
		   "%s: providing htc with %d buffers at eid=%d\n",
		   __func__, buffersToRefill, Endpoint);

	for (RxBuffers = 0; RxBuffers < buffersToRefill; RxBuffers++) {
		osBuf = qdf_nbuf_alloc(NULL, AR6000_BUFFER_SIZE,
				       AR6000_MIN_HEAD_ROOM, 4, false);
		if (NULL == osBuf) {
			break;
		}
		/* the HTC packet wrapper is at the head of the reserved area
		 * in the skb */
		pPacket = (HTC_PACKET *) (A_NETBUF_HEAD(osBuf));
		/* set re-fill info */
		SET_HTC_PACKET_INFO_RX_REFILL(pPacket, osBuf,
					      qdf_nbuf_data(osBuf),
					      AR6000_BUFFER_SIZE, Endpoint);
		SET_HTC_PACKET_NET_BUF_CONTEXT(pPacket, osBuf);
		/* add to queue */
		HTC_PACKET_ENQUEUE(&queue, pPacket);
	}

	if (!HTC_QUEUE_EMPTY(&queue)) {
		/* add packets */
		htc_add_receive_pkt_multiple(pEpping_ctx->HTCHandle, &queue);
	}
}
#endif /* HIF_SDIO */

void epping_rx(void *ctx, HTC_PACKET *pPacket)
{
	epping_context_t *pEpping_ctx = (epping_context_t *) ctx;
	epping_adapter_t *pAdapter = pEpping_ctx->epping_adapter;
	struct net_device *dev = pAdapter->dev;
	A_STATUS status = pPacket->Status;
	HTC_ENDPOINT_ID eid = pPacket->Endpoint;
	struct sk_buff *pktSkb = (struct sk_buff *)pPacket->pPktContext;

	EPPING_LOG(QDF_TRACE_LEVEL_INFO,
		   "%s: pAdapter = 0x%p eid=%d, skb=0x%p, data=0x%p, len=0x%x status:%d",
		   __func__, pAdapter, eid, pktSkb, pPacket->pBuffer,
		   pPacket->ActualLength, status);

	if (status != A_OK) {
		if (status != A_ECANCELED) {
			printk("%s: RX ERR (%d)\n", __func__, status);
		}
		qdf_nbuf_free(pktSkb);
		return;
	}

	/* deliver to up layer */
	if (pktSkb) {
		if (EPPING_ALIGNMENT_PAD > 0) {
			A_NETBUF_PULL(pktSkb, EPPING_ALIGNMENT_PAD);
		}
		if (enb_rx_dump)
			epping_hex_dump((void *)qdf_nbuf_data(pktSkb),
					pktSkb->len, __func__);
		pktSkb->dev = dev;
		if ((pktSkb->dev->flags & IFF_UP) == IFF_UP) {
			pktSkb->protocol = eth_type_trans(pktSkb, pktSkb->dev);
			++pAdapter->stats.rx_packets;
			pAdapter->stats.rx_bytes += pktSkb->len;
			qdf_net_buf_debug_release_skb(pktSkb);
			if (hdd_napi_enabled(HDD_NAPI_ANY))
				netif_receive_skb(pktSkb);
			else
				netif_rx_ni(pktSkb);
			if ((pAdapter->stats.rx_packets %
				 EPPING_STATS_LOG_COUNT) == 0) {
				EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
					   "%s: total_rx_pkts = %lu",
					   __func__,
					   pAdapter->stats.rx_packets);
			}
		} else {
			++pAdapter->stats.rx_dropped;
			qdf_nbuf_free(pktSkb);
		}
	}
}
