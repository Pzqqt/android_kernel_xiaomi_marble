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
 * DOC : wlan_hdd_spectralscan.h
 *
 * WLAN Host Device Driver spectral scan implementation
 *
 */

#if !defined(WLAN_HDD_SPECTRALSCAN_H)
#define WLAN_HDD_SPECTRALSCAN_H

#define FEATURE_SPECTRAL_SCAN_VENDOR_COMMANDS \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
			WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scan_start \
}, \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
		WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scan_stop \
}, \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
			WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scam_get_config \
}, \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
		WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scan_get_diag_stats \
}, \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
			WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scan_get_cap_info \
}, \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
		WIPHY_VENDOR_CMD_NEED_NETDEV, \
	.doit = wlan_hdd_cfg80211_spectral_scan_get_status \
},

/**
 * wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scan_start(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

/**
 * wlan_hdd_cfg80211_spectral_scan_stop() - stop spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function stops spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scan_stop(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

/**
 * wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scam_get_config(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

/**
 * wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scan_get_diag_stats(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

/**
 * wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scan_get_cap_info(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);

/**
 * wlan_hdd_cfg80211_spectral_scan_start() - start spectral scan
 * @wiphy:    WIPHY structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of the data received
 *
 * This function starts spectral scan
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_cfg80211_spectral_scan_get_status(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len);


#endif
