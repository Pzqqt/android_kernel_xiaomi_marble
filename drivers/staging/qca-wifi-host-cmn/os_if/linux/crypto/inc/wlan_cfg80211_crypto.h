/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: declares crypto functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_CRYPTO_H_
#define _WLAN_CFG80211_CRYPTO_H_
#include <net/cfg80211.h>
#include "wlan_crypto_global_def.h"
/**
 * wlan_cfg80211_set_default_key() - to set the default key to be used
 * @vdev: VDEV Object pointer
 * @key_index: Index to be set as the default
 * @bssid: BSSID for which the key is to be set
 *
 * Return: Zero for success and negative for failure.
 */
int wlan_cfg80211_set_default_key(struct wlan_objmgr_vdev *vdev,
				  uint8_t key_index,
				  struct qdf_mac_addr *bssid);

/**
 * wlan_cfg80211_store_key() - Store the key
 * @vdev: VDEV Object pointer
 * @key_index: Index to be set as the default
 * @key_type: denotes if the key is pairwise or group key
 * @mac_addr: BSSID for which the key is to be set
 * @key_params: Params received from the kernel
 *
 * Return: Zero for success and negative for failure.
 */
int wlan_cfg80211_store_key(struct wlan_objmgr_vdev *vdev,
			    uint8_t key_index,
			    enum wlan_crypto_key_type key_type,
			    const u8 *mac_addr, struct key_params *params);

/**
 * wlan_cfg80211_crypto_add_key() - Add key for the specified vdev
 * @vdev: vdev object
 * @key_type: denotes if the add key request is for pairwise or group key
 * @key_index: Index of the key that needs to be added
 * @sync: flag to indicate whether or not to add key synchronously.
 *  DO NOT set to true if it's in scheduler context.
 *
 * Return: Zero on Success, negative value on failure
 */
int wlan_cfg80211_crypto_add_key(struct wlan_objmgr_vdev *vdev,
				 enum wlan_crypto_key_type key_type,
				 uint8_t key_index, bool sync);
#endif
