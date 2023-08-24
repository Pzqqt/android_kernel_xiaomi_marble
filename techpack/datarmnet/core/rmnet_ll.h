/* Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __RMNET_LL_H__
#define __RMNET_LL_H__

#include <linux/skbuff.h>

struct rmnet_ll_stats {
		u64 tx_queue;
		u64 tx_queue_err;
		u64 tx_complete;
		u64 tx_complete_err;
		u64 rx_queue;
		u64 rx_queue_err;
		u64 rx_status_err;
		u64 rx_null;
		u64 rx_oom;
		u64 rx_pkts;
		u64 rx_tmp_allocs;
		u64 tx_disabled;
		u64 tx_enabled;
		u64 tx_fc_queued;
		u64 tx_fc_sent;
		u64 tx_fc_err;
};

int rmnet_ll_send_skb(struct sk_buff *skb);
struct rmnet_ll_stats *rmnet_ll_get_stats(void);
int rmnet_ll_init(void);
void rmnet_ll_exit(void);

#endif
