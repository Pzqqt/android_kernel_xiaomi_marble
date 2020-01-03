/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */


#ifndef _WLAN_CFG80211_SCAN_H_
#define _WLAN_CFG80211_SCAN_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <wlan_scan_public_structs.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_mgmt_txrx_utils_api.h>

/* Max number of scans allowed from userspace */
#define WLAN_MAX_SCAN_COUNT 8

/* GPS application requirement */
#define QCOM_VENDOR_IE_ID 221
#define QCOM_OUI1         0x00
#define QCOM_OUI2         0xA0
#define QCOM_OUI3         0xC6
#define QCOM_VENDOR_IE_AGE_TYPE  0x100
#define QCOM_VENDOR_IE_AGE_LEN   (sizeof(qcom_ie_age) - 2)
#define SCAN_DONE_EVENT_BUF_SIZE 4096
#define SCAN_WAKE_LOCK_CONNECT_DURATION (1 * 1000) /* in msec */
#define SCAN_WAKE_LOCK_SCAN_DURATION (5 * 1000) /* in msec */

/**
 * typedef struct qcom_ie_age - age ie
 *
 * @element_id: Element id
 * @len: Length
 * @oui_1: OUI 1
 * @oui_2: OUI 2
 * @oui_3: OUI 3
 * @type: Type
 * @age: Age
 * @tsf_delta: tsf delta from FW
 * @beacon_tsf: original beacon TSF
 * @seq_ctrl: sequence control field
 */
typedef struct {
	u8 element_id;
	u8 len;
	u8 oui_1;
	u8 oui_2;
	u8 oui_3;
	u32 type;
	u32 age;
	u32 tsf_delta;
	u64 beacon_tsf;
	u16 seq_ctrl;
} __attribute__ ((packed)) qcom_ie_age;

/**
 * struct osif_scan_pdev - OS scan private strcutre
 * scan_req_q: Scan request queue
 * scan_req_q_lock: Protect scan request queue
 * req_id: Scan request Id
 * runtime_pm_lock: Runtime suspend lock
 * scan_wake_lock: Scan wake lock
 */
struct osif_scan_pdev{
	qdf_list_t scan_req_q;
	qdf_mutex_t scan_req_q_lock;
	wlan_scan_requester req_id;
	qdf_runtime_lock_t runtime_pm_lock;
	qdf_wake_lock_t scan_wake_lock;
};

/*
 * enum scan_source - scan request source
 * @NL_SCAN: Scan initiated from NL
 * @VENDOR_SCAN: Scan intiated from vendor command
 */
enum scan_source {
	NL_SCAN,
	VENDOR_SCAN,
};

/**
 * struct scan_req - Scan Request entry
 * @node : List entry element
 * @scan_request: scan request holder
 * @scan_id: scan identifier used across host layers which is generated at WMI
 * @source: scan request originator (NL/Vendor scan)
 * @dev: net device (same as what is in scan_request)
 *
 * Scan request linked list element
 */
struct scan_req {
	qdf_list_node_t node;
	struct cfg80211_scan_request *scan_request;
	uint32_t scan_id;
	uint8_t source;
	struct net_device *dev;
};

/**
 * struct scan_params - Scan params
 * @source: scan request source
 * @default_ie: default scan ie
 * @vendor_ie: vendor ie
 * @priority: scan priority
 * @half_rate: Half rate flag
 * @quarter_rate: Quarter rate flag
 * @strict_pscan: strict passive scan flag
 * @dwell_time_active: Active dwell time. Ignored if zero or inapplicable.
 * @dwell_time_active_2g: 2.4 GHz specific active dwell time. Ignored if zero or
 * inapplicable.
 * @dwell_time_passive: Passive dwell time. Ignored if zero or inapplicable.
 * @dwell_time_active_6g: 6 GHz specific active dwell time. Ignored if zero or
 * inapplicable.
 * @dwell_time_passive_6g: 6 GHz specific passive dwell time. Ignored if zero or
 * inapplicable.
 */
struct scan_params {
	uint8_t source;
	struct element_info default_ie;
	struct element_info vendor_ie;
	enum scan_priority priority;
	bool half_rate;
	bool quarter_rate;
	bool strict_pscan;
	uint32_t dwell_time_active;
	uint32_t dwell_time_active_2g;
	uint32_t dwell_time_passive;
	uint32_t dwell_time_active_6g;
	uint32_t dwell_time_passive_6g;
};

/**
 * struct wlan_cfg80211_inform_bss - BSS inform data
 * @chan: channel the frame was received on
 * @mgmt: beacon/probe resp frame
 * @frame_len: frame length
 * @rssi: signal strength in mBm (100*dBm)
 * @boottime_ns: timestamp (CLOCK_BOOTTIME) when the information was received.
 * @per_chain_rssi: per chain rssi received
 */
struct wlan_cfg80211_inform_bss {
	struct ieee80211_channel *chan;
	struct ieee80211_mgmt *mgmt;
	size_t frame_len;
	int rssi;
	uint64_t boottime_ns;
	uint8_t per_chain_rssi[WLAN_MGMT_TXRX_HOST_MAX_ANTENNA];
};


#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * wlan_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @vdev: vdev pointer
 * @request: Pointer to cfg80211 scheduled scan start request
 * @scan_backoff_multiplier: multiply scan period by this after max cycles
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_sched_scan_start(struct wlan_objmgr_vdev *vdev,
				   struct cfg80211_sched_scan_request *request,
				   uint8_t scan_backoff_multiplier);

/**
 * wlan_cfg80211_sched_scan_stop() - cfg80211 scheduled scan(pno) stop
 * @vdev: vdev pointer
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_sched_scan_stop(struct wlan_objmgr_vdev *vdev);
#endif

/**
 * wlan_scan_runtime_pm_init() - API to initialize runtime pm context for scan
 * @pdev: Pointer to pdev
 *
 * This will help to initialize scan runtime pm context separately.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_scan_runtime_pm_init(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_scan_runtime_pm_deinit() - API to deinitialize runtime pm
 * for scan.
 * @pdev: Pointer to pdev
 *
 * This will help to deinitialize scan runtime pm before deinitialize
 * HIF
 *
 * Return: void
 */
void wlan_scan_runtime_pm_deinit(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfg80211_scan_priv_init() - API to initialize cfg80211 scan
 * @pdev: Pointer to net device
 *
 * API to initialize cfg80211 scan module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cfg80211_scan_priv_init(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfg80211_scan_priv_deinit() - API to deinitialize cfg80211 scan
 * @pdev: Pointer to net device
 *
 * API to deinitialize cfg80211 scan module.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cfg80211_scan_priv_deinit(
		struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfg80211_scan() - API to process cfg80211 scan request
 * @vdev: Pointer to vdev
 * @request: Pointer to scan request
 * @params: scan params
 *
 * API to trigger scan and update cfg80211 scan database.
 * scan dump command can be used to fetch scan results
 * on receipt of scan complete event.
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_scan(struct wlan_objmgr_vdev *vdev,
		       struct cfg80211_scan_request *request,
		       struct scan_params *params);

/**
 * wlan_cfg80211_inform_bss_frame_data() - API to inform beacon to cfg80211
 * @wiphy: wiphy
 * @bss_data: bss data
 *
 * API to inform beacon to cfg80211
 *
 * Return: pointer to bss entry
 */
struct cfg80211_bss *
wlan_cfg80211_inform_bss_frame_data(struct wiphy *wiphy,
		struct wlan_cfg80211_inform_bss *bss);

/**
 * wlan_cfg80211_inform_bss_frame() - API to inform beacon to cfg80211
 * @pdev: Pointer to pdev
 * @scan_params: scan entry
 * @request: Pointer to scan request
 *
 * API to inform beacon to cfg80211
 *
 * Return: void
 */
void wlan_cfg80211_inform_bss_frame(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_params);

/**
 * __wlan_cfg80211_unlink_bss_list() - flush bss from the kernel cache
 * @wiphy: wiphy
 * @bssid: bssid of the BSS to find
 * @ssid: ssid of the BSS to find
 * @ssid_len: ssid len of of the BSS to find
 *
 * Return: None
 */
void __wlan_cfg80211_unlink_bss_list(struct wiphy *wiphy, uint8_t *bssid,
				     uint8_t *ssid, uint8_t ssid_len);

/**
 * wlan_cfg80211_get_bss() - Get the bss entry matching the chan, bssid and ssid
 * @wiphy: wiphy
 * @channel: channel of the BSS to find
 * @bssid: bssid of the BSS to find
 * @ssid: ssid of the BSS to find
 * @ssid_len: ssid len of of the BSS to find
 *
 * The API is a wrapper to get bss from kernel matching the chan,
 * bssid and ssid
 *
 * Return: bss structure if found else NULL
 */
struct cfg80211_bss *wlan_cfg80211_get_bss(struct wiphy *wiphy,
					   struct ieee80211_channel *channel,
					   const u8 *bssid,
					   const u8 *ssid, size_t ssid_len);

/*
 * wlan_cfg80211_unlink_bss_list : flush bss from the kernel cache
 * @pdev: Pointer to pdev
 * @scan_entry: scan entry
 *
 * Return: bss which is unlinked from kernel cache
 */
void wlan_cfg80211_unlink_bss_list(struct wlan_objmgr_pdev *pdev,
				   struct scan_cache_entry *scan_entry);

/**
 * wlan_vendor_abort_scan() - API to vendor abort scan
 * @pdev: Pointer to pdev
 * @data: pointer to data
 * @data_len: Data length
 *
 * API to abort scan through vendor command
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_vendor_abort_scan(struct wlan_objmgr_pdev *pdev,
				const void *data, int data_len);

/**
 * wlan_cfg80211_abort_scan() - API to abort scan through cfg80211
 * @pdev: Pointer to pdev
 *
 * API to abort scan through cfg80211 request
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_cfg80211_abort_scan(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_abort_scan() - Generic API to abort scan request
 * @pdev: Pointer to pdev
 * @pdev_id: pdev id
 * @vdev_id: vdev id
 * @scan_id: scan id
 * @sync: if wait for scan complete is required
 *
 * Generic API to abort scans
 *
 * Return: 0 for success, non zero for failure
 */
QDF_STATUS wlan_abort_scan(struct wlan_objmgr_pdev *pdev,
				   uint32_t pdev_id,
				   uint32_t vdev_id,
				   wlan_scan_id scan_id,
				   bool sync);

/**
 * wlan_cfg80211_cleanup_scan_queue() - remove entries in scan queue
 * @pdev: pdev pointer
 * @dev: net device pointer
 *
 * Removes entries in scan queue depending on dev provided and sends scan
 * complete event to NL.
 * Removes all entries in scan queue, if dev provided is NULL
 *
 * Return: None
 */
void wlan_cfg80211_cleanup_scan_queue(struct wlan_objmgr_pdev *pdev,
				      struct net_device *dev);

/**
 * wlan_hdd_cfg80211_add_connected_pno_support() - Set connected PNO support
 * @wiphy: Pointer to wireless phy
 *
 * This function is used to set connected PNO support to kernel
 *
 * Return: None
 */
#if defined(CFG80211_REPORT_BETTER_BSS_IN_SCHED_SCAN) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
void wlan_scan_cfg80211_add_connected_pno_support(struct wiphy *wiphy);

#else
static inline
void wlan_scan_cfg80211_add_connected_pno_support(struct wiphy *wiphy)
{
}
#endif

#if ((LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)) || \
		defined(CFG80211_MULTI_SCAN_PLAN_BACKPORT)) && \
		defined(FEATURE_WLAN_SCAN_PNO)
/**
 * hdd_config_sched_scan_plans_to_wiphy() - configure sched scan plans to wiphy
 * @wiphy: pointer to wiphy
 * @config: pointer to config
 *
 * Return: None
 */
void wlan_config_sched_scan_plans_to_wiphy(struct wiphy *wiphy,
					   struct wlan_objmgr_psoc *psoc);
#else
static inline
void wlan_config_sched_scan_plans_to_wiphy(struct wiphy *wiphy,
					   struct wlan_objmgr_psoc *psoc)
{
}
#endif /* FEATURE_WLAN_SCAN_PNO */

#endif
