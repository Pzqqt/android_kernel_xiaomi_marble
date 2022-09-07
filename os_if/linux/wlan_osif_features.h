/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_osif_features.h
 *
 * Define feature flags to cleanly describe when features
 * are present in a given version of the kernel
 */

#ifndef _WLAN_OSIF_FEATURES_H_
#define _WLAN_OSIF_FEATURES_H_

#include <linux/version.h>

/*
 * CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT
 * Used to indicate the Linux Kernel contains support for single netdevice multi
 * link support.
 */
#if (defined(__ANDROID_COMMON_KERNEL__) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && \
	(defined  IEEE80211_MLD_MAX_NUM_LINKS))
#define CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT 1
#endif
#endif
