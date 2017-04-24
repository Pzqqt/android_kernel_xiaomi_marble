/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#if !defined __HDD_REGULATORY_H
#define __HDD_REGULATORY_H

/**
 * DOC: wlan_hdd_regulatory.h
 *
 * HDD Regulatory prototype implementation
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) || defined(WITH_BACKPORTS)
#define IEEE80211_CHAN_PASSIVE_SCAN IEEE80211_CHAN_NO_IR
#define IEEE80211_CHAN_NO_IBSS IEEE80211_CHAN_NO_IR
#endif

int hdd_regulatory_init(hdd_context_t *hdd_ctx, struct wiphy *wiphy);
void hdd_program_country_code(hdd_context_t *hdd_ctx);
void hdd_reset_global_reg_params(void);

/**
 * hdd_reg_set_country() - helper function for setting the regulatory country
 * @hdd_ctx: the HDD context to set the country for
 * @country_code: the two character country code to configure
 *
 * Return: zero for success, non-zero error code for failure
 */
int hdd_reg_set_country(hdd_context_t *hdd_ctx, char *country_code);

/**
 * hdd_reg_set_band() - helper function for setting the regulatory band
 * @hdd_ctx: the HDD context to set the band for
 * @ui_band: the UI band to configure
 *
 * Return: zero for success, non-zero error code for failure
 */
int hdd_reg_set_band(struct net_device *dev, u8 ui_band);

#endif
