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

#ifndef __RMNET_LL_CORE_H__
#define __RMNET_LL_CORE_H__

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>

#define RMNET_LL_DEFAULT_MRU 0x8000

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
	struct net_device *phys_dev;
	void *priv;
	u32 dev_mru;
	u32 page_order;
	u32 buf_len;
};

/* Core operations to hide differences between physical transports.
 *
 * buffer_queue: Queue an allocated buffer to the HW for RX. Optional.
 * query_free_descriptors: Return number of free RX descriptors. Optional.
 * tx: Send an SKB over the channel in the TX direction.
 * init: Initialization callback on module load
 * exit: Exit callback on module unload
 */
struct rmnet_ll_client_ops {
	int (*buffer_queue)(struct rmnet_ll_endpoint *ll_ep,
			    struct rmnet_ll_buffer *ll_buf);
	int (*query_free_descriptors)(struct rmnet_ll_endpoint *ll_ep);
	int (*tx)(struct sk_buff *skb);
	int (*init)(void);
	int (*exit)(void);
};

int rmnet_ll_buffer_pool_alloc(struct rmnet_ll_endpoint *ll_ep);
void rmnet_ll_buffer_pool_free(struct rmnet_ll_endpoint *ll_ep);
void rmnet_ll_buffers_recycle(struct rmnet_ll_endpoint *ll_ep);

#endif
