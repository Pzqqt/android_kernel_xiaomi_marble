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

/*
 * CFG80211_TX_CONTROL_PORT_LINK_SUPPORT
 * Used to indicate Linux kernel contains support for TX control port from
 * specific link ID
 *
 * This feature was introduced in Linux Kernel 6.0 via:
 * 9b6bf4d6120a wifi: nl80211: set BSS to NULL if IS_ERR()
 * 45aaf17c0c34 wifi: nl80211: check MLO support in authenticate
 * d2bc52498b6b wifi: nl80211: Support MLD parameters in nl80211_set_station()
 * 67207bab9341 wifi: cfg80211/mac80211: Support control port TX from specific link
 * 69c3f2d30c35 wifi: nl80211: allow link ID in set_wiphy with frequency
 * fa2ca639c4e6 wifi: nl80211: advertise MLO support
 * e3d331c9b620 wifi: cfg80211: set country_elem to NULL
 * 34d76a14f8f7 wifi: nl80211: reject link specific elements on assoc link
 * df35f3164ec1 wifi: nl80211: reject fragmented and non-inheritance elements
 * ff5c4dc4cd78 wifi: nl80211: fix some attribute policy entries
 * 7464f665158e wifi: cfg80211: add cfg80211_get_iftype_ext_capa()
 * 8876c67e6296 wifi: nl80211: require MLD address on link STA add/modify
 * 9dd1953846c7 wifi: nl80211/mac80211: clarify link ID in control port TX
 * 00b3d8401019 wifi: cfg80211/nl80211: move rx management data into a struct
 * 6074c9e57471 wifi: cfg80211: report link ID in NL80211_CMD_FRAME
 * 95f498bb49f7 wifi: nl80211: add MLO link ID to the NL80211_CMD_FRAME TX API
 * 1e0b3b0b6cb5 wifi: mac80211: Align with Draft P802.11be_D1.5
 * 062e8e02dfd4 wifi: mac80211: Align with Draft P802.11be_D2.0
 * d776763f4808 wifi: cfg80211: debugfs: fix return type in ht40allow_map_read()
 * 64e966d1e84b wifi: cfg80211: fix MCS divisor value
 * 4e9c3af39820 wifi: nl80211: add EML/MLD capabilities to per-iftype capabilities
 * 80b0ed70a271 wifi: nl80211: add RX and TX timestamp attributes
 * ea7d50c925ce wifi: cfg80211: add a function for reporting TX status with hardware timestamps
 * 1ff715ffa0ec wifi: cfg80211: add hardware timestamps to frame RX info
 *
 * This feature was backported to Android Common Kernel 5.15.74 via:
 * https://android-review.googlesource.com/c/kernel/common/+/2253173
 * https://android-review.googlesource.com/c/kernel/common/+/2253174
 * https://android-review.googlesource.com/c/kernel/common/+/2253175
 * https://android-review.googlesource.com/c/kernel/common/+/2253176
 * https://android-review.googlesource.com/c/kernel/common/+/2253177
 * https://android-review.googlesource.com/c/kernel/common/+/2253178
 * https://android-review.googlesource.com/c/kernel/common/+/2253179
 * https://android-review.googlesource.com/c/kernel/common/+/2253180
 * https://android-review.googlesource.com/c/kernel/common/+/2253181
 * https://android-review.googlesource.com/c/kernel/common/+/2253182
 * https://android-review.googlesource.com/c/kernel/common/+/2253183
 * https://android-review.googlesource.com/c/kernel/common/+/2253184
 * https://android-review.googlesource.com/c/kernel/common/+/2253185
 * https://android-review.googlesource.com/c/kernel/common/+/2253186
 * https://android-review.googlesource.com/c/kernel/common/+/2253187
 * https://android-review.googlesource.com/c/kernel/common/+/2253188
 * https://android-review.googlesource.com/c/kernel/common/+/2253189
 * https://android-review.googlesource.com/c/kernel/common/+/2253190
 * https://android-review.googlesource.com/c/kernel/common/+/2253191
 * https://android-review.googlesource.com/c/kernel/common/+/2253192
 * https://android-review.googlesource.com/c/kernel/common/+/2253193
 * https://android-review.googlesource.com/c/kernel/common/+/2253194
 * https://android-review.googlesource.com/c/kernel/common/+/2267469
 * https://android-review.googlesource.com/c/kernel/common/+/2267204
 * https://android-review.googlesource.com/c/kernel/common/+/2267210
 */

#if (defined(__ANDROID_COMMON_KERNEL__) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 74)) || \
	(defined IEEE80211_EHT_OPER_INFO_PRESENT))
#define CFG80211_TX_CONTROL_PORT_LINK_SUPPORT 1
#endif
#endif
