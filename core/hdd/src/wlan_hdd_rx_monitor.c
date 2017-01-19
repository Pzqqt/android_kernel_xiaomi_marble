/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
#include "wlan_hdd_includes.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <cds_sched.h>
#include <cds_utils.h>
#include "wlan_hdd_rx_monitor.h"

/**
 * hdd_rx_monitor_callback(): Callback function for receive monitor mode
 * @vdev: Handle to vdev object
 * @mpdu: pointer to mpdu to be delivered to os
 * @rx_status: receive status
 *
 * Returns: None
 */
void hdd_rx_monitor_callback(ol_osif_vdev_handle context,
				qdf_nbuf_t rxbuf,
				void *rx_status)
{
	hdd_adapter_t *adapter;
	int rxstat;
	struct sk_buff *skb;
	struct sk_buff *skb_next;
	unsigned int cpu_index;

	qdf_assert(context);
	qdf_assert(rxbuf);

	adapter = (hdd_adapter_t *)context;
	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			"invalid adapter %p", adapter);
		return;
	}

	cpu_index = wlan_hdd_get_cpu();

	/* walk the chain until all are processed */
	skb = (struct sk_buff *)rxbuf;
	while (NULL != skb) {
		skb_next = skb->next;
		skb->dev = adapter->dev;

		++adapter->hdd_stats.hddTxRxStats.rxPackets[cpu_index];
		++adapter->stats.rx_packets;
		adapter->stats.rx_bytes += skb->len;

		/* Remove SKB from internal tracking table before submitting
		 * it to stack
		 */
		qdf_net_buf_debug_release_skb(skb);

		/*
		 * If this is not a last packet on the chain
		 * Just put packet into backlog queue, not scheduling RX sirq
		 */
		if (skb->next) {
			rxstat = netif_rx(skb);
		} else {
			/*
			 * This is the last packet on the chain
			 * Scheduling rx sirq
			 */
			rxstat = netif_rx_ni(skb);
		}

		if (NET_RX_SUCCESS == rxstat)
			++adapter->
				hdd_stats.hddTxRxStats.rxDelivered[cpu_index];
		else
			++adapter->hdd_stats.hddTxRxStats.rxRefused[cpu_index];

		skb = skb_next;
	}

	adapter->dev->last_rx = jiffies;

	return;
}

/**
 * hdd_monitor_set_rx_monitor_cb(): Set rx monitor mode callback function
 * @txrx: pointer to txrx ops
 * @rx_monitor_cb: pointer to callback function
 *
 * Returns: None
 */
void hdd_monitor_set_rx_monitor_cb(struct ol_txrx_ops *txrx,
				ol_txrx_rx_mon_fp rx_monitor_cb)
{
	txrx->rx.mon = rx_monitor_cb;
}
