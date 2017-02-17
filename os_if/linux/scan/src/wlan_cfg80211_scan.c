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
 * DOC: defines driver functions interfacing with linux kernel
 */

#include <wlan_cfg80211_scan.h>
#include <wlan_osif_priv.h>
#include <wlan_cfg80211.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <wlan_scan_utils_api.h>

int wlan_cfg80211_scan(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
		struct net_device *dev,
#endif
		struct cfg80211_scan_request *request)
{
	return 0;
}

static inline struct ieee80211_channel *
wlan_get_ieee80211_channel(struct wiphy *wiphy, int chan_no)
{
	unsigned int freq;
	struct ieee80211_channel *chan;

	if (WLAN_CHAN_IS_2GHZ(chan_no) &&
	   (wiphy->bands[NL80211_BAND_2GHZ] != NULL)) {
		freq =
			ieee80211_channel_to_frequency(chan_no,
			NL80211_BAND_2GHZ);
	} else if (WLAN_CHAN_IS_5GHZ(chan_no) &&
	   (wiphy->bands[NL80211_BAND_5GHZ] != NULL)) {
		freq =
			ieee80211_channel_to_frequency(chan_no,
			NL80211_BAND_5GHZ);
	} else {
		cfg80211_err("Invalid chan_no %d", chan_no);
		return NULL;
	}

	chan = __ieee80211_get_channel(wiphy, freq);

	if (!chan)
		cfg80211_err("chan is NULL, chan_no: %d freq: %d",
			chan_no, freq);

	return chan;
}

#ifdef WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS
static inline int wlan_get_frame_len(struct scan_cache_entry *scan_params)
{
	return util_scan_entry_frame_len(scan_params) + sizeof(qcom_ie_age);
}

static inline void wlan_add_age_ie(uint8_t *mgmt_frame,
	struct scan_cache_entry *scan_params)
{
	qcom_ie_age *qie_age = NULL;

	/* GPS Requirement: need age ie per entry. Using vendor specific. */
	/* Assuming this is the last IE, copy at the end */
	qie_age = (qcom_ie_age *) (mgmt_frame +
		   util_scan_entry_frame_len(scan_params));
	qie_age->element_id = QCOM_VENDOR_IE_ID;
	qie_age->len = QCOM_VENDOR_IE_AGE_LEN;
	qie_age->oui_1 = QCOM_OUI1;
	qie_age->oui_2 = QCOM_OUI2;
	qie_age->oui_3 = QCOM_OUI3;
	qie_age->type = QCOM_VENDOR_IE_AGE_TYPE;
	/*
	 * Lowi expects the timestamp of bss in units of 1/10 ms. In driver
	 * all bss related timestamp is in units of ms. Due to this when scan
	 * results are sent to lowi the scan age is high.To address this,
	 * send age in units of 1/10 ms.
	 */
	qie_age->age =
		(uint32_t)(qdf_mc_timer_get_system_time() -
		  scan_params->scan_entry_time)/10;
	qie_age->tsf_delta = scan_params->tsf_delta;
	memcpy(&qie_age->beacon_tsf, scan_params->tsf_info.data,
		  sizeof(qie_age->beacon_tsf));
	memcpy(&qie_age->seq_ctrl, &scan_params->seq_num,
	       sizeof(qie_age->seq_ctrl));
}
#else
static inline int wlan_get_frame_len(struct scan_cache_entry *scan_params)
{
	return util_scan_entry_frame_len(scan_params);
}

static inline void wlan_add_age_ie(uint8_t *mgmt_frame,
	struct scan_cache_entry *scan_params)
{
}
#endif /* WLAN_ENABLE_AGEIE_ON_SCAN_RESULTS */

void wlan_cfg80211_inform_bss_frame(struct wlan_objmgr_pdev *pdev,
		struct scan_cache_entry *scan_params)
{
	struct pdev_osif_priv *pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	struct wiphy *wiphy;
	int frame_len;
	struct ieee80211_mgmt *mgmt = NULL;
	struct ieee80211_channel *chan;
	int rssi = 0;
	struct cfg80211_bss *bss_status = NULL;

	if (!pdev_ospriv) {
		cfg80211_err("os_priv is NULL");
		return;
	}

	wiphy = pdev_ospriv->wiphy;

	frame_len = wlan_get_frame_len(scan_params);
	mgmt = qdf_mem_malloc(frame_len);
	if (!mgmt) {
		cfg80211_err("mem alloc failed");
		return;
	}
	qdf_mem_copy(mgmt,
		 util_scan_entry_frame_ptr(scan_params),
		 util_scan_entry_frame_len(scan_params));
	/*
	 * Android does not want the timestamp from the frame.
	 * Instead it wants a monotonic increasing value
	 */
	mgmt->u.probe_resp.timestamp = qdf_get_monotonic_boottime();
	wlan_add_age_ie((uint8_t *)mgmt, scan_params);
	/*
	 * Based on .ini configuration, raw rssi can be reported for bss.
	 * Raw rssi is typically used for estimating power.
	 */
	rssi = scan_params->rssi_raw;

	chan = wlan_get_ieee80211_channel(wiphy,
		scan_params->channel.chan_idx);
	if (!chan) {
		qdf_mem_free(mgmt);
		return;
	}

	/*
	 * Supplicant takes the signal strength in terms of
	 * mBm (1 dBm = 100 mBm).
	 */
	rssi = QDF_MIN(rssi, 0) * 100;

	cfg80211_info("BSSID: %pM Channel:%d RSSI:%d",
		mgmt->bssid, chan->center_freq, (int)(rssi / 100));

	bss_status =
		cfg80211_inform_bss_frame(wiphy, chan, mgmt,
		frame_len, rssi, GFP_KERNEL);
	qdf_mem_free(mgmt);
}

