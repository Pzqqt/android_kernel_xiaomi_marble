/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 *
 * RMNET_CTL client handlers
 *
 */

#ifndef _RMNET_CTL_CLIENT_H_
#define _RMNET_CTL_CLIENT_H_

#include <linux/skbuff.h>
#include "rmnet_ctl.h"

void rmnet_ctl_log(enum rmnet_ctl_log_lvl lvl, const char *msg,
		   int rc, const void *data, unsigned int len);

#define rmnet_ctl_log_err(msg, rc, data, len) \
		rmnet_ctl_log(RMNET_CTL_LOG_ERR, msg, rc, data, len)

#define rmnet_ctl_log_info(msg, data, len) \
		rmnet_ctl_log(RMNET_CTL_LOG_INFO, msg, 0, data, len)

#define rmnet_ctl_log_debug(msg, data, len) \
		rmnet_ctl_log(RMNET_CTL_LOG_DEBUG, msg, 0, data, len)

struct rmnet_ctl_stats {
	u64 rx_pkts;
	u64 rx_err;
	u64 tx_pkts;
	u64 tx_err;
	u64 tx_complete;
};

struct rmnet_ctl_dev {
	int (*xmit)(struct rmnet_ctl_dev *dev, struct sk_buff *skb);
	struct rmnet_ctl_stats stats;
};

void rmnet_ctl_endpoint_post(const void *data, size_t len);
void rmnet_ctl_endpoint_setdev(const struct rmnet_ctl_dev *dev);
void rmnet_ctl_set_dbgfs(bool enable);

#endif /* _RMNET_CTL_CLIENT_H_ */
