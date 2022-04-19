/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * RMNET_CTL header
 *
 */

#ifndef _RMNET_CTL_H_
#define _RMNET_CTL_H_

#include <linux/skbuff.h>

enum rmnet_ctl_log_lvl {
	RMNET_CTL_LOG_CRIT,
	RMNET_CTL_LOG_ERR,
	RMNET_CTL_LOG_INFO,
	RMNET_CTL_LOG_DEBUG,
};

struct rmnet_ctl_client_hooks {
	void (*ctl_dl_client_hook)(struct sk_buff *skb);
};

struct rmnet_ctl_client_if {
	void *	(*reg)(struct rmnet_ctl_client_hooks *hook);
	int	(*dereg)(void *handle);
	int	(*send)(void *handle, struct sk_buff *skb);
	void	(*log)(enum rmnet_ctl_log_lvl lvl, const char *msg, int rc,
		       const void *data, unsigned int len);
};

#ifdef RMNET_LA_PLATFORM
struct rmnet_ctl_client_if *rmnet_ctl_if(void);
int rmnet_ctl_get_stats(u64 *s, int n);
#else
static inline struct rmnet_ctl_client_if *rmnet_ctl_if(void) {return NULL;};
static inline int rmnet_ctl_get_stats(u64 *s, int n) {return 0;};
#endif /* RMNET_LA_PLATFORM */

#endif /* _RMNET_CTL_H_ */
