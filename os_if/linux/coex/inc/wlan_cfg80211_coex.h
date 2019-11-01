/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_COEX_H_
#define _WLAN_CFG80211_COEX_H_
#include <wlan_cfg80211.h>
#include <wlan_objmgr_cmn.h>

#ifdef FEATURE_COEX
int wlan_cfg80211_coex_set_btc_chain_mode(struct wlan_objmgr_vdev *vdev,
					  const void *data, int data_len);
#else
static inline int
wlan_cfg80211_coex_set_btc_chain_mode(struct wlan_objmgr_vdev *vdev,
				      const void *data, int data_len)
{
	return -ENOTSUPP;
}
#endif
#endif
