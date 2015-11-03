/*
 * Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_EXT_SCAN_H)
#define WLAN_HDD_EXT_SCAN_H

/**
 * DOC: wlan_hdd_ext_scan.h
 *
 * WLAN Host Device Driver EXT SCAN feature implementation
 *
 */

#ifdef FEATURE_WLAN_EXTSCAN

#include "wlan_hdd_main.h"

/*
 * Used to allocate the size of 4096 for the EXTScan NL data.
 * The size of 4096 is considered assuming that all data per
 * respective event fit with in the limit.Please take a call
 * on the limit based on the data requirements.
 */

#define EXTSCAN_EVENT_BUF_SIZE 4096

int wlan_hdd_cfg80211_extscan_start(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len);

int wlan_hdd_cfg80211_extscan_stop(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data, int data_len);

int wlan_hdd_cfg80211_extscan_get_valid_channels(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len);

int wlan_hdd_cfg80211_extscan_get_capabilities(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       const void *data, int data_len);

int wlan_hdd_cfg80211_extscan_get_cached_results(struct wiphy *wiphy,
						 struct wireless_dev
						 *wdev, const void *data,
						 int data_len);

int wlan_hdd_cfg80211_extscan_set_bssid_hotlist(struct wiphy *wiphy,
						struct wireless_dev
						*wdev, const void *data,
						int data_len);

int wlan_hdd_cfg80211_extscan_reset_bssid_hotlist(struct wiphy *wiphy,
						  struct wireless_dev
						  *wdev, const void *data,
						  int data_len);

int wlan_hdd_cfg80211_extscan_set_significant_change(struct wiphy *wiphy,
						     struct wireless_dev
						     *wdev, const void *data,
						     int data_len);

int wlan_hdd_cfg80211_extscan_reset_significant_change(struct wiphy
						       *wiphy,
						       struct
						       wireless_dev
						       *wdev, const void *data,
						       int data_len);

int wlan_hdd_cfg80211_set_epno_list(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len);

int wlan_hdd_cfg80211_set_passpoint_list(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

int wlan_hdd_cfg80211_reset_passpoint_list(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

int
wlan_hdd_cfg80211_extscan_set_ssid_hotlist(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len);

int
wlan_hdd_cfg80211_extscan_reset_ssid_hotlist(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len);

void wlan_hdd_cfg80211_extscan_init(hdd_context_t *hdd_ctx);

#else /* FEATURE_WLAN_EXTSCAN */

static void wlan_hdd_cfg80211_extscan_init(hdd_context_t *hdd_ctx)
{
}

#endif /* End of FEATURE_WLAN_EXTSCAN */

#endif /* end #if !defined(WLAN_HDD_EXT_SCAN_H) */

