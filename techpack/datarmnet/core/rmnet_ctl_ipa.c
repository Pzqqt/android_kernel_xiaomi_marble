// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * RMNET_CTL mhi handler
 *
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ipa.h>
#include "rmnet_ctl.h"
#include "rmnet_ctl_client.h"

struct rmnet_ctl_ipa_dev {
	struct rmnet_ctl_dev dev;
	spinlock_t rx_lock; /* rx lock */
	spinlock_t tx_lock; /* tx lock */
};

static struct rmnet_ctl_ipa_dev ctl_ipa_dev;
static bool rmnet_ctl_ipa_registered;

static int rmnet_ctl_send_ipa(struct rmnet_ctl_dev *dev, struct sk_buff *skb)
{
	struct rmnet_ctl_ipa_dev *ctl_dev = container_of(
				dev, struct rmnet_ctl_ipa_dev, dev);
	int rc;

	spin_lock_bh(&ctl_dev->tx_lock);

	rc = ipa_rmnet_ctl_xmit(skb);
	if (rc)
		dev->stats.tx_err++;
	else
		dev->stats.tx_pkts++;

	spin_unlock_bh(&ctl_dev->tx_lock);

	return rc;
}

static void rmnet_ctl_dl_callback(void *user_data, void *rx_data)
{
	struct rmnet_ctl_ipa_dev *ctl_dev = user_data;

	ctl_dev->dev.stats.rx_pkts++;
	rmnet_ctl_endpoint_post(rx_data, 0xFFFFFFFF);
}

static void rmnet_ctl_probe(void *user_data)
{
	memset(&ctl_ipa_dev, 0, sizeof(ctl_ipa_dev));

	spin_lock_init(&ctl_ipa_dev.rx_lock);
	spin_lock_init(&ctl_ipa_dev.tx_lock);

	ctl_ipa_dev.dev.xmit = rmnet_ctl_send_ipa;
	rmnet_ctl_endpoint_setdev(&ctl_ipa_dev.dev);

	pr_info("rmnet_ctl driver probed\n");
}

static void rmnet_ctl_remove(void *user_data)
{
	rmnet_ctl_endpoint_setdev(NULL);

	pr_info("rmnet_ctl driver removed\n");
}

static void rmnet_ctl_ipa_ready(void *user_data)
{
	int rc;

	rc = ipa_register_rmnet_ctl_cb(
			rmnet_ctl_probe,
			&ctl_ipa_dev,
			rmnet_ctl_remove,
			&ctl_ipa_dev,
			rmnet_ctl_dl_callback,
			&ctl_ipa_dev);

	if (rc)
		pr_err("%s: %d\n", __func__, rc);
	else
		rmnet_ctl_ipa_registered = true;
}

static int __init rmnet_ctl_init(void)
{
	int rc;

	rc = ipa_register_ipa_ready_cb(rmnet_ctl_ipa_ready, NULL);
	if (rc == -EEXIST)
		rmnet_ctl_ipa_ready(NULL);
	else if (rc)
		pr_err("%s: %d\n", __func__, rc);

	return 0;
}

static void __exit rmnet_ctl_exit(void)
{
	if (rmnet_ctl_ipa_registered) {
		ipa_unregister_rmnet_ctl_cb();
		rmnet_ctl_ipa_registered = false;
	}
}

module_init(rmnet_ctl_init)
module_exit(rmnet_ctl_exit)

MODULE_DESCRIPTION("RmNet control IPA Driver");
MODULE_LICENSE("GPL v2");
