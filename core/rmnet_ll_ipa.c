/* Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include "rmnet_ll.h"
#include "rmnet_ll_core.h"

static struct rmnet_ll_endpoint *rmnet_ll_ipa_ep;

static void rmnet_ll_ipa_rx(void *arg, void *rx_data)
{
	struct rmnet_ll_endpoint *ll_ep = *((struct rmnet_ll_endpoint **)arg);
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	struct sk_buff *skb, *tmp;

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

	dev_put((*ll_ep)->phys_dev);
	kfree(*ll_ep);
	*ll_ep = NULL;
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
	if (!rmnet_ll_ipa_ep)
		return -ENODEV;

	/* IPA handles freeing the SKB on failure */
	return ipa_rmnet_ll_xmit(skb);
}

static int rmnet_ll_ipa_init(void)
{
	int rc;

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
