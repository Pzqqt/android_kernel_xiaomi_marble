/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __WLAN_HDD_NAN_H
#define __WLAN_HDD_NAN_H

/**
 * DOC: wlan_hdd_nan.h
 *
 * WLAN Host Device Driver NAN API specification
 */

struct hdd_context_s;

#ifdef WLAN_FEATURE_NAN
struct wiphy;
struct wireless_dev;

int wlan_hdd_cfg80211_nan_request(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  const void *data,
				  int data_len);

bool wlan_hdd_nan_is_supported(void);
/**
 * hdd_nan_populate_cds_config() - Populate NAN cds configuration
 * @cds_cfg: CDS Configuration
 * @hdd_ctx: Pointer to hdd context
 *
 * Return: none
 */
static inline void hdd_nan_populate_cds_config(struct cds_config_info *cds_cfg,
			hdd_context_t *hdd_ctx)
{
	cds_cfg->is_nan_enabled = hdd_ctx->config->enable_nan_support;
}
void wlan_hdd_cfg80211_nan_callback(void *ctx, tSirNanEvent *msg);
#else
static inline bool wlan_hdd_nan_is_supported(void)
{
	return false;
}
static inline void hdd_nan_populate_cds_config(struct cds_config_info *cds_cfg,
			hdd_context_t *hdd_ctx)
{
}
static inline void wlan_hdd_cfg80211_nan_callback(void *ctx,
						  tSirNanEvent *msg)
{
}
#endif /* WLAN_FEATURE_NAN */
#endif /* __WLAN_HDD_NAN_H */
