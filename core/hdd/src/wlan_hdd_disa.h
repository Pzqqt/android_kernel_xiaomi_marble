/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_HDD_DISA_H
#define _WLAN_HDD_DISA_H

#include "wlan_hdd_main.h"
#include "sir_api.h"

#ifdef WLAN_FEATURE_DISA
/**
 * hdd_encrypt_decrypt_init () - exposes encrypt/decrypt initialization
 * functionality
 * @hdd_ctx: hdd context
 *
 Return: 0 on success, negative errno on failure
 */
int hdd_encrypt_decrypt_init(hdd_context_t *hdd_ctx);

/**
 * hdd_encrypt_decrypt_deinit () - exposes encrypt/decrypt deinitialization
 * functionality
 * @hdd_ctx: hdd context
 *
 Return: 0 on success, negative errno on failure
 */
int hdd_encrypt_decrypt_deinit(hdd_context_t *hdd_ctx);
#else
static inline int hdd_encrypt_decrypt_init(hdd_context_t *hdd_ctx)
{
	return -ENOTSUPP;
}
static inline int hdd_encrypt_decrypt_deinit(hdd_context_t *hdd_ctx)
{
	return -ENOTSUPP;
}
#endif

#ifdef WLAN_FEATURE_DISA
/**
 * wlan_hdd_cfg80211_encrypt_decrypt_msg () - Encrypt/Decrypt msg
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_encrypt_decrypt_msg(struct wiphy *wiphy,
			struct wireless_dev *wdev,
			const void *data,
			int data_len);
#endif

#endif
