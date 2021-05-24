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
 * RmNet Low Latency channel handlers
 */

#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/skbuff.h>
#include <linux/mhi.h>
#include <linux/if_ether.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/version.h>
#include "rmnet_ll.h"

#define RMNET_LL_DEFAULT_MRU 0x8000
#define RMNET_LL_MAX_RECYCLE_ITER 16

struct rmnet_ll_buffer {
	struct list_head list;
	struct page *page;
	bool temp_alloc;
	bool submitted;
};

struct rmnet_ll_buffer_pool {
	struct list_head buf_list;
	/* Protect access to the recycle buffer pool */
	spinlock_t pool_lock;
	struct list_head *last;
	u32 pool_size;
};

struct rmnet_ll_endpoint {
	struct rmnet_ll_buffer_pool buf_pool;
	struct mhi_device *mhi_dev;
	struct net_device *mhi_netdev;
	u32 dev_mru;
	u32 page_order;
	u32 buf_len;
};

static struct rmnet_ll_endpoint *rmnet_ll_ep;
static struct rmnet_ll_stats rmnet_ll_stats;
/* For TX synch with MHI via mhi_queue_transfer() */
static DEFINE_SPINLOCK(rmnet_ll_tx_lock);

static void rmnet_ll_buffers_submit(struct rmnet_ll_endpoint *ll_ep,
				    struct list_head *buf_list)
{
	struct rmnet_ll_buffer *ll_buf;
	int rc;

	list_for_each_entry(ll_buf, buf_list, list) {
		if (ll_buf->submitted)
			continue;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		rc = mhi_queue_transfer(ll_ep->mhi_dev, DMA_FROM_DEVICE,
					page_address(ll_buf->page),
					ll_ep->buf_len, MHI_EOT);
#else
		rc = mhi_queue_buf(ll_ep->mhi_dev, DMA_FROM_DEVICE,
				   page_address(ll_buf->page),
				   ll_ep->buf_len, MHI_EOT);
#endif
		if (rc) {
			rmnet_ll_stats.rx_queue_err++;
			/* Don't leak the page if we're not storing it */
			if (ll_buf->temp_alloc)
				put_page(ll_buf->page);
		} else {
			ll_buf->submitted = true;
			rmnet_ll_stats.rx_queue++;
		}
	}
}

static struct rmnet_ll_buffer *
rmnet_ll_buffer_alloc(struct rmnet_ll_endpoint *ll_ep, gfp_t gfp)
{
	struct rmnet_ll_buffer *ll_buf;
	struct page *page;
	void *addr;

	page = __dev_alloc_pages(gfp, ll_ep->page_order);
	if (!page)
		return NULL;

	/* Store the buffer information at the end */
	addr = page_address(page);
	ll_buf = addr + ll_ep->buf_len;
	ll_buf->page = page;
	ll_buf->submitted = false;
	INIT_LIST_HEAD(&ll_buf->list);
	return ll_buf;
}

static int rmnet_ll_buffer_pool_alloc(struct rmnet_ll_endpoint *ll_ep)
{
	spin_lock_init(&ll_ep->buf_pool.pool_lock);
	INIT_LIST_HEAD(&ll_ep->buf_pool.buf_list);
	ll_ep->buf_pool.last = ll_ep->buf_pool.buf_list.next;
	ll_ep->buf_pool.pool_size = 0;
	return 0;
}

static void rmnet_ll_buffer_pool_free(struct rmnet_ll_endpoint *ll_ep)
{
	struct rmnet_ll_buffer *ll_buf, *tmp;
	list_for_each_entry_safe(ll_buf, tmp, &ll_ep->buf_pool.buf_list, list) {
		list_del(&ll_buf->list);
		put_page(ll_buf->page);
	}

	ll_ep->buf_pool.last = NULL;
}

static void rmnet_ll_buffers_recycle(struct rmnet_ll_endpoint *ll_ep)
{
	struct rmnet_ll_buffer *ll_buf, *tmp;
	LIST_HEAD(buf_list);
	int num_tre, count = 0, iter = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	num_tre = mhi_get_no_free_descriptors(ll_ep->mhi_dev, DMA_FROM_DEVICE);
#else
	num_tre = mhi_get_free_desc_count(ll_ep->mhi_dev, DMA_FROM_DEVICE);
#endif
	if (!num_tre)
		goto out;

	list_for_each_entry_safe(ll_buf, tmp, ll_ep->buf_pool.last, list) {
		if (++iter > RMNET_LL_MAX_RECYCLE_ITER || count == num_tre)
			break;

		if (ll_buf->submitted)
			continue;

		count++;
		list_move_tail(&ll_buf->list, &buf_list);
	}

	/* Mark where we left off */
	ll_ep->buf_pool.last = &ll_buf->list;
	/* Submit any pool buffers to the HW if we found some */
	if (count) {
		rmnet_ll_buffers_submit(ll_ep, &buf_list);
		/* requeue immediately BEFORE the last checked element */
		list_splice_tail_init(&buf_list, ll_ep->buf_pool.last);
	}

	/* Do any temporary allocations needed to fill the rest */
	for (; count < num_tre; count++) {
		ll_buf = rmnet_ll_buffer_alloc(ll_ep, GFP_ATOMIC);
		if (!ll_buf)
			break;

		list_add_tail(&ll_buf->list, &buf_list);
		ll_buf->temp_alloc = true;
		rmnet_ll_stats.rx_tmp_allocs++;
	}

	if (!list_empty(&buf_list))
		rmnet_ll_buffers_submit(ll_ep, &buf_list);

out:
	return;
}

static void rmnet_ll_rx(struct mhi_device *mhi_dev, struct mhi_result *res)
{
	struct rmnet_ll_endpoint *ll_ep = dev_get_drvdata(&mhi_dev->dev);
	struct rmnet_ll_buffer *ll_buf;
	struct sk_buff *skb;

	/* Get the buffer struct back for our page information */
	ll_buf = res->buf_addr + ll_ep->buf_len;
	ll_buf->submitted = false;
	if (res->transaction_status) {
		rmnet_ll_stats.rx_status_err++;
		goto err;
	} else if (!res->bytes_xferd) {
		rmnet_ll_stats.rx_null++;
		goto err;
	}

	/* Store this away so we don't have to look it up every time */
	if (!ll_ep->mhi_netdev) {
		ll_ep->mhi_netdev = dev_get_by_name(&init_net, "rmnet_mhi0");
		if (!ll_ep->mhi_netdev)
			goto err;
	}

	skb = alloc_skb(0, GFP_ATOMIC);
	if (!skb) {
		rmnet_ll_stats.rx_oom++;
		goto err;
	}

	/* Build the SKB and pass it off to the stack */
	skb_add_rx_frag(skb, 0, ll_buf->page, 0, res->bytes_xferd,
			ll_ep->buf_len);
	if (!ll_buf->temp_alloc)
		get_page(ll_buf->page);

	skb->dev = ll_ep->mhi_netdev;
	skb->protocol = htons(ETH_P_MAP);
	/* Mark this as arriving on the LL channel. Allows rmnet to skip
	 * module handling as needed.
	 */
	skb->priority = 0xda1a;
	rmnet_ll_stats.rx_pkts++;
	netif_rx(skb);
	rmnet_ll_buffers_recycle(ll_ep);
	return;

err:
	/* Go, and never darken my towels again! */
	if (ll_buf->temp_alloc)
		put_page(ll_buf->page);
}

static void rmnet_ll_tx_complete(struct mhi_device *mhi_dev,
				 struct mhi_result *res)
{
	struct sk_buff *skb = res->buf_addr;

	/* Check the result and free the SKB */
	if (res->transaction_status)
		rmnet_ll_stats.tx_complete_err++;
	else
		rmnet_ll_stats.tx_complete++;

	dev_kfree_skb_any(skb);
}

static int rmnet_ll_probe(struct mhi_device *mhi_dev,
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
	ll_ep->mhi_dev = mhi_dev;

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
	rmnet_ll_ep = ll_ep;
	return 0;
}

static void rmnet_ll_remove(struct mhi_device *mhi_dev)
{
	struct rmnet_ll_endpoint *ll_ep;

	ll_ep = dev_get_drvdata(&mhi_dev->dev);
	/* Remove our private data form the device. No need to free it though.
	 * It will be freed once the mhi_dev is released since it was alloced
	 * from a managed pool.
	 */
	dev_set_drvdata(&mhi_dev->dev, NULL);
	rmnet_ll_ep = NULL;
	rmnet_ll_buffer_pool_free(ll_ep);
}

static const struct mhi_device_id rmnet_ll_channel_table[] = {
	{
		.chan = "RMNET_DATA_LL",
	},
	{},
};

static struct mhi_driver rmnet_ll_driver = {
	.probe = rmnet_ll_probe,
	.remove = rmnet_ll_remove,
	.dl_xfer_cb = rmnet_ll_rx,
	.ul_xfer_cb = rmnet_ll_tx_complete,
	.id_table = rmnet_ll_channel_table,
	.driver = {
		.name = "rmnet_ll",
		.owner = THIS_MODULE,
	},
};

int rmnet_ll_send_skb(struct sk_buff *skb)
{
	struct rmnet_ll_endpoint *ll_ep = rmnet_ll_ep;
	int rc = -ENODEV;

	/* Lock to prevent multiple sends at the same time. mhi_queue_transfer()
	 * cannot be called in parallel for the same DMA direction.
	 */
	spin_lock_bh(&rmnet_ll_tx_lock);
	if (ll_ep)
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		rc = mhi_queue_transfer(ll_ep->mhi_dev, DMA_TO_DEVICE, skb,
					skb->len, MHI_EOT);
#else
		rc = mhi_queue_skb(ll_ep->mhi_dev, DMA_TO_DEVICE, skb,
				   skb->len, MHI_EOT);
#endif

	spin_unlock_bh(&rmnet_ll_tx_lock);
	if (rc)
		rmnet_ll_stats.tx_queue_err++;
	else
		rmnet_ll_stats.tx_queue++;

	return rc;
}

struct rmnet_ll_stats *rmnet_ll_get_stats(void)
{
	return &rmnet_ll_stats;
}

int rmnet_ll_init(void)
{
	return mhi_driver_register(&rmnet_ll_driver);
}

void rmnet_ll_exit(void)
{
	mhi_driver_unregister(&rmnet_ll_driver);
}
