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

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/version.h>
#include "rmnet_ll.h"
#include "rmnet_ll_core.h"

#define RMNET_LL_MAX_RECYCLE_ITER 16

static struct rmnet_ll_stats rmnet_ll_stats;
/* For TX sync with DMA operations */
static DEFINE_SPINLOCK(rmnet_ll_tx_lock);

/* Client operations for respective underlying HW */
extern struct rmnet_ll_client_ops rmnet_ll_client;

static void rmnet_ll_buffers_submit(struct rmnet_ll_endpoint *ll_ep,
				    struct list_head *buf_list)
{
	struct rmnet_ll_buffer *ll_buf;

	list_for_each_entry(ll_buf, buf_list, list) {
		if (ll_buf->submitted)
			continue;

		if (!rmnet_ll_client.buffer_queue ||
		    rmnet_ll_client.buffer_queue(ll_ep, ll_buf)) {
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

int rmnet_ll_buffer_pool_alloc(struct rmnet_ll_endpoint *ll_ep)
{
	spin_lock_init(&ll_ep->buf_pool.pool_lock);
	INIT_LIST_HEAD(&ll_ep->buf_pool.buf_list);
	ll_ep->buf_pool.last = ll_ep->buf_pool.buf_list.next;
	ll_ep->buf_pool.pool_size = 0;
	return 0;
}

void rmnet_ll_buffer_pool_free(struct rmnet_ll_endpoint *ll_ep)
{
	struct rmnet_ll_buffer *ll_buf, *tmp;
	list_for_each_entry_safe(ll_buf, tmp, &ll_ep->buf_pool.buf_list, list) {
		list_del(&ll_buf->list);
		put_page(ll_buf->page);
	}

	ll_ep->buf_pool.last = NULL;
}

void rmnet_ll_buffers_recycle(struct rmnet_ll_endpoint *ll_ep)
{
	struct rmnet_ll_buffer *ll_buf, *tmp;
	LIST_HEAD(buf_list);
	int num_tre, count = 0, iter = 0;

	if (!rmnet_ll_client.query_free_descriptors)
		goto out;

	num_tre = rmnet_ll_client.query_free_descriptors(ll_ep);
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

int rmnet_ll_send_skb(struct sk_buff *skb)
{
	int rc;

	spin_lock_bh(&rmnet_ll_tx_lock);
	rc = rmnet_ll_client.tx(skb);
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
	return rmnet_ll_client.init();
}

void rmnet_ll_exit(void)
{
	rmnet_ll_client.exit();
}
