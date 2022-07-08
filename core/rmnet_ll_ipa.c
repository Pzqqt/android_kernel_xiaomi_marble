/* Copyright (c) 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * RmNet IPA Low Latency channel handlers
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ipa.h>
#include <linux/if_ether.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include "rmnet_ll.h"
#include "rmnet_ll_core.h"

#define IPA_RMNET_LL_RECEIVE 1
#define IPA_RMNET_LL_FLOW_EVT 2

#define MAX_Q_LEN 1000

static struct rmnet_ll_endpoint *rmnet_ll_ipa_ep;
static struct sk_buff_head tx_pending_list;
extern spinlock_t rmnet_ll_tx_lock;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void rmnet_ll_ipa_tx_pending(unsigned long data);
DECLARE_TASKLET(tx_pending_task, rmnet_ll_ipa_tx_pending, 0);
static void rmnet_ll_ipa_tx_pending(unsigned long data)
#else
static void rmnet_ll_ipa_tx_pending(struct tasklet_struct *t);
DECLARE_TASKLET(tx_pending_task, rmnet_ll_ipa_tx_pending);
static void rmnet_ll_ipa_tx_pending(struct tasklet_struct *t)
#endif
{
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	struct sk_buff *skb;
	int rc;

	spin_lock_bh(&rmnet_ll_tx_lock);

	while ((skb = __skb_dequeue(&tx_pending_list))) {
		rc = ipa_rmnet_ll_xmit(skb);
		if (rc == -EAGAIN) {
			stats->tx_disabled++;
			__skb_queue_head(&tx_pending_list, skb);
			break;
		}
		if (rc >= 0)
			stats->tx_fc_sent++;
		else
			stats->tx_fc_err++;
	}

	spin_unlock_bh(&rmnet_ll_tx_lock);
}

static void rmnet_ll_ipa_rx(void *arg, void *rx_data)
{
	struct rmnet_ll_endpoint *ll_ep = rmnet_ll_ipa_ep;
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	struct sk_buff *skb, *tmp;

	if (arg == (void *)(uintptr_t)(IPA_RMNET_LL_FLOW_EVT)) {
		stats->tx_enabled++;
		tasklet_schedule(&tx_pending_task);
		return;
	}

	if (unlikely(arg != (void *)(uintptr_t)(IPA_RMNET_LL_RECEIVE))) {
		pr_err("%s: invalid arg %lu\n", __func__, (uintptr_t)arg);
		return;
	}

	skb = rx_data;
	/* Odds are IPA does this, but just to be safe */
	skb->dev = ll_ep->phys_dev;
	skb->protocol = htons(ETH_P_MAP);
	skb_record_rx_queue(skb, 1);

	tmp = skb;
	while (tmp) {
		/* Mark the SKB as low latency */
		tmp->priority = 0xda1a;
		tmp = skb_shinfo(tmp)->frag_list;
	}

	stats->rx_pkts++;
	netif_rx(skb);
}

static void rmnet_ll_ipa_probe(void *arg)
{
	struct rmnet_ll_endpoint *ll_ep;

	ll_ep = kzalloc(sizeof(*ll_ep), GFP_KERNEL);
	if (!ll_ep) {
		pr_err("%s(): allocating LL CTX failed\n", __func__);
		return;
	}

	ll_ep->phys_dev = dev_get_by_name(&init_net, "rmnet_ipa0");
	if (!ll_ep->phys_dev) {
		pr_err("%s(): Invalid physical device\n", __func__);
		kfree(ll_ep);
		return;
	}

	*((struct rmnet_ll_endpoint **)arg) = ll_ep;
}

static void rmnet_ll_ipa_remove(void *arg)
{
	struct rmnet_ll_endpoint **ll_ep = arg;
	struct sk_buff *skb;

	dev_put((*ll_ep)->phys_dev);
	kfree(*ll_ep);
	*ll_ep = NULL;

	spin_lock_bh(&rmnet_ll_tx_lock);
	while ((skb = __skb_dequeue(&tx_pending_list)))
		kfree_skb(skb);
	spin_unlock_bh(&rmnet_ll_tx_lock);
}

static void rmnet_ll_ipa_ready(void * __unused)
{
	int rc;

	rc = ipa_register_rmnet_ll_cb(rmnet_ll_ipa_probe,
				      (void *)&rmnet_ll_ipa_ep,
				      rmnet_ll_ipa_remove,
				      (void *)&rmnet_ll_ipa_ep,
				      rmnet_ll_ipa_rx,
				      (void *)&rmnet_ll_ipa_ep);
	if (rc)
		pr_err("%s(): Registering IPA LL callback failed with rc %d\n",
		       __func__, rc);
}

static int rmnet_ll_ipa_tx(struct sk_buff *skb)
{
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	int rc;

	if (!rmnet_ll_ipa_ep)
		return -ENODEV;

	if (!skb_queue_empty(&tx_pending_list))
		goto queue_skb;

	rc = ipa_rmnet_ll_xmit(skb);

	/* rc >=0: success, return number of free descriptors left */
	if (rc >= 0)
		return 0;

	/* IPA handles freeing the SKB on failure */
	if (rc != -EAGAIN)
		return rc;

	stats->tx_disabled++;

queue_skb:
	/* Flow controlled */
	if (skb_queue_len(&tx_pending_list) >= MAX_Q_LEN) {
		kfree_skb(skb);
		return -ENOMEM;
	}

	__skb_queue_tail(&tx_pending_list, skb);
	stats->tx_fc_queued++;

	return 0;
}

static int rmnet_ll_ipa_init(void)
{
	int rc;

	__skb_queue_head_init(&tx_pending_list);
	rc = ipa_register_ipa_ready_cb(rmnet_ll_ipa_ready, NULL);
	if (rc == -EEXIST) {
		/* IPA is already up. Call it ourselves, since they don't */
		rmnet_ll_ipa_ready(NULL);
		rc = 0;
	}

	return rc;
}

static int rmnet_ll_ipa_exit(void)
{
	if (rmnet_ll_ipa_ep) {
		ipa_unregister_rmnet_ll_cb();
		/* Teardown? */
		rmnet_ll_ipa_ep = NULL;
	}

	return 0;
}

/* Export operations struct to the main framework */
struct rmnet_ll_client_ops rmnet_ll_client = {
	.tx = rmnet_ll_ipa_tx,
	.init = rmnet_ll_ipa_init,
	.exit = rmnet_ll_ipa_exit,
};
