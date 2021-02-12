/* Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * RmNet MHI Low Latency channel handlers
 */

#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/skbuff.h>
#include <linux/mhi.h>
#include <linux/if_ether.h>
#include <linux/mm.h>
#include "rmnet_ll.h"
#include "rmnet_ll_core.h"

static struct rmnet_ll_endpoint *rmnet_ll_mhi_ep;

static void rmnet_ll_mhi_rx(struct mhi_device *mhi_dev, struct mhi_result *res)
{
	struct rmnet_ll_endpoint *ll_ep = dev_get_drvdata(&mhi_dev->dev);
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	struct rmnet_ll_buffer *ll_buf;
	struct sk_buff *skb;

	/* Get the buffer struct back for our page information */
	ll_buf = res->buf_addr + ll_ep->buf_len;
	ll_buf->submitted = false;
	if (res->transaction_status) {
		stats->rx_status_err++;
		goto err;
	} else if (!res->bytes_xferd) {
		stats->rx_null++;
		goto err;
	}

	/* Store this away so we don't have to look it up every time */
	if (!ll_ep->phys_dev) {
		ll_ep->phys_dev = dev_get_by_name(&init_net, "rmnet_mhi0");
		if (!ll_ep->phys_dev)
			goto err;
	}

	skb = alloc_skb(0, GFP_ATOMIC);
	if (!skb) {
		stats->rx_oom++;
		goto err;
	}

	/* Build the SKB and pass it off to the stack */
	skb_add_rx_frag(skb, 0, ll_buf->page, 0, res->bytes_xferd,
			ll_ep->buf_len);
	if (!ll_buf->temp_alloc)
		get_page(ll_buf->page);

	skb->dev = ll_ep->phys_dev;
	skb->protocol = htons(ETH_P_MAP);
	/* Mark this as arriving on the LL channel. Allows rmnet to skip
	 * module handling as needed.
	 */
	skb->priority = 0xda1a;
	stats->rx_pkts++;
	netif_rx(skb);
	rmnet_ll_buffers_recycle(ll_ep);
	return;

err:
	/* Go, and never darken my towels again! */
	if (ll_buf->temp_alloc)
		put_page(ll_buf->page);
}

static void rmnet_ll_mhi_tx_complete(struct mhi_device *mhi_dev,
				     struct mhi_result *res)
{
	struct rmnet_ll_stats *stats = rmnet_ll_get_stats();
	struct sk_buff *skb = res->buf_addr;

	/* Check the result and free the SKB */
	if (res->transaction_status)
		stats->tx_complete_err++;
	else
		stats->tx_complete++;

	dev_kfree_skb_any(skb);
}

static int rmnet_ll_mhi_probe(struct mhi_device *mhi_dev,
			      const struct mhi_device_id *id)
{
	struct rmnet_ll_endpoint *ll_ep;
	int rc;

	/* Allocate space for our state from the managed pool tied to the life
	 * of the mhi device.
	 */
	ll_ep = devm_kzalloc(&mhi_dev->dev, sizeof(*ll_ep), GFP_KERNEL);
	if (!ll_ep)
		return -ENOMEM;

	/* Hold on to the mhi_dev so we can send data to it later */
	ll_ep->priv = (void *)mhi_dev;

	/* Grab the MRU of the device so we know the size of the pages we need
	 * to allocate for the pool.
	 */
	rc = of_property_read_u32(mhi_dev->dev.of_node, "mhi,mru",
				  &ll_ep->dev_mru);
	if (rc || !ll_ep->dev_mru)
		/* Use our default mru */
		ll_ep->dev_mru = RMNET_LL_DEFAULT_MRU;

	ll_ep->page_order = get_order(ll_ep->dev_mru);
	/* We store some stuff at the end of the page, so don't let the HW
	 * use that part of it.
	 */
	ll_ep->buf_len = ll_ep->dev_mru - sizeof(struct rmnet_ll_buffer);

	/* Tell MHI to initialize the UL/DL channels for transfer */
	rc = mhi_prepare_for_transfer(mhi_dev);
	if (rc) {
		pr_err("%s(): Failed to prepare device for transfer: 0x%x\n",
		       __func__, rc);
		return rc;
	}

	rc = rmnet_ll_buffer_pool_alloc(ll_ep);
	if (rc) {
		pr_err("%s(): Failed to allocate buffer pool: %d\n", __func__,
		       rc);
		mhi_unprepare_from_transfer(mhi_dev);
		return rc;
	}

	rmnet_ll_buffers_recycle(ll_ep);

	/* Not a fan of storing this pointer in two locations, but I've yet to
	 * come up with any other good way of accessing it on the TX path from
	 * rmnet otherwise, since we won't have any references to the mhi_dev.
	 */
	dev_set_drvdata(&mhi_dev->dev, ll_ep);
	rmnet_ll_mhi_ep = ll_ep;
	return 0;
}

static void rmnet_ll_mhi_remove(struct mhi_device *mhi_dev)
{
	struct rmnet_ll_endpoint *ll_ep;

	ll_ep = dev_get_drvdata(&mhi_dev->dev);
	/* Remove our private data form the device. No need to free it though.
	 * It will be freed once the mhi_dev is released since it was alloced
	 * from a managed pool.
	 */
	dev_set_drvdata(&mhi_dev->dev, NULL);
	rmnet_ll_mhi_ep = NULL;
	rmnet_ll_buffer_pool_free(ll_ep);
}

static const struct mhi_device_id rmnet_ll_mhi_channel_table[] = {
	{
		.chan = "RMNET_DATA_LL",
	},
	{},
};

static struct mhi_driver rmnet_ll_driver = {
	.probe = rmnet_ll_mhi_probe,
	.remove = rmnet_ll_mhi_remove,
	.dl_xfer_cb = rmnet_ll_mhi_rx,
	.ul_xfer_cb = rmnet_ll_mhi_tx_complete,
	.id_table = rmnet_ll_mhi_channel_table,
	.driver = {
		.name = "rmnet_ll",
		.owner = THIS_MODULE,
	},
};

static int rmnet_ll_mhi_queue(struct rmnet_ll_endpoint *ll_ep,
			      struct rmnet_ll_buffer *ll_buf)
{
	struct mhi_device *mhi_dev = ll_ep->priv;

	return mhi_queue_buf(mhi_dev, DMA_FROM_DEVICE,
			     page_address(ll_buf->page),
			     ll_ep->buf_len, MHI_EOT);
}

static int rmnet_ll_mhi_query_free_descriptors(struct rmnet_ll_endpoint *ll_ep)
{
	struct mhi_device *mhi_dev = ll_ep->priv;

	return mhi_get_free_desc_count(mhi_dev, DMA_FROM_DEVICE);
}

static int rmnet_ll_mhi_tx(struct sk_buff *skb)
{
	struct mhi_device *mhi_dev;
	int rc;

	if (!rmnet_ll_mhi_ep)
		return -ENODEV;

	mhi_dev = rmnet_ll_mhi_ep->priv;
	rc = mhi_queue_skb(mhi_dev, DMA_TO_DEVICE, skb, skb->len, MHI_EOT);
	if (rc)
		kfree_skb(skb);

	return rc;
}

static int rmnet_ll_mhi_init(void)
{
	return mhi_driver_register(&rmnet_ll_driver);
}

static int rmnet_ll_mhi_exit(void)
{
	mhi_driver_unregister(&rmnet_ll_driver);
	return 0;
}

/* Export operations struct to the main framework */
struct rmnet_ll_client_ops rmnet_ll_client = {
	.buffer_queue = rmnet_ll_mhi_queue,
	.query_free_descriptors = rmnet_ll_mhi_query_free_descriptors,
	.tx = rmnet_ll_mhi_tx,
	.init = rmnet_ll_mhi_init,
	.exit = rmnet_ll_mhi_exit,
};
