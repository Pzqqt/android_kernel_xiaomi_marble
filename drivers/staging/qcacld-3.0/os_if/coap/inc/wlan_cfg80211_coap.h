/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _WLAN_CFG80211_COAP_H_
#define _WLAN_CFG80211_COAP_H_
#include <wlan_objmgr_vdev_obj.h>

#ifdef WLAN_FEATURE_COAP
/**
 * wlan_cfg80211_coap_offload() - configure CoAP offloading
 * @wiphy: pointer to wireless wiphy structure.
 * @vdev: VDEV Object pointer
 * @data: pointer to netlink TLV buffer
 * @data_len: the length of @data in bytes
 *
 * Return: An error code or 0 on success.
 */
int
wlan_cfg80211_coap_offload(struct wiphy *wiphy, struct wlan_objmgr_vdev *vdev,
			   const void *data, int data_len);
#endif
#endif
