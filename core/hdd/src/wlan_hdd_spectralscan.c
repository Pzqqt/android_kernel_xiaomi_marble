/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_spectral_scan.c
 *
 * WLAN Host Device Driver Spectral Scan Implementation
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <net/cfg80211.h>
#include "wlan_hdd_includes.h"
#include "cds_api.h"
#include "ani_global.h"
#include "wlan_cfg80211_spectral.h"
#include "wlan_hdd_spectralscan.h"

/**
 * __wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_start(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_config_and_start(wiphy,
					hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

/**
 * __wlan_hdd_cfg80211_spectral_scan_stop() - stop spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function stops spectral scan
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_stop(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_stop(wiphy, hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

/**
 * __wlan_hdd_cfg80211_spectral_scan_get_config() - spectral scan get config
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function to get the spectral scan configuration
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_get_config(
						struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_get_config(wiphy, hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

/**
 * __wlan_hdd_cfg80211_spectral_scan_get_diag_stats() - get diag stats
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function gets the spectral scan diag stats
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_get_diag_stats(
						struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_get_diag_stats(wiphy,
					hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

/**
 * __wlan_hdd_cfg80211_spectral_scan_get_cap_info() - get spectral caps
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function gets spectral scan configured capabilities
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_get_cap_info(
						struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_get_cap(wiphy, hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

/*
 * __wlan_hdd_cfg80211_spectral_scan_get_status() - get spectral scan
 * status
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function gets current status of spectral scan
 *
 * Return: 0 on success and errno on failure
 */
static int __wlan_hdd_cfg80211_spectral_scan_get_status(
						struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_cfg80211_spectral_scan_get_status(wiphy, hdd_ctx->hdd_pdev,
					data, data_len);
	EXIT();

	return ret;
}

int wlan_hdd_cfg80211_spectral_scan_start(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_start(
				wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_cfg80211_spectral_scan_stop(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_stop(
				wiphy, wdev, data, data_len);

	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_cfg80211_spectral_scam_get_config(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_get_config(
				wiphy, wdev, data, data_len);

	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_cfg80211_spectral_scan_get_diag_stats(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_get_diag_stats(
				wiphy, wdev, data, data_len);

	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_cfg80211_spectral_scan_get_cap_info(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_get_cap_info(
				wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_cfg80211_spectral_scan_get_status(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_spectral_scan_get_status(
				wiphy, wdev, data, data_len);

	cds_ssr_unprotect(__func__);

	return ret;
}
