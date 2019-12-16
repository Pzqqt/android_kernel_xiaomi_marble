/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#include <qdf_list.h>
#include <qdf_status.h>
#include <qdf_module.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <wlan_scan_utils_api.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_scan.h>
#include <wlan_osif_priv.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_cfg80211_scan.h>
#include <qdf_mem.h>
#include <wlan_utility.h>
#include "cfg_ucfg_api.h"
#ifdef WLAN_POLICY_MGR_ENABLE
#include <wlan_policy_mgr_api.h>
#endif
#include <wlan_reg_services_api.h>
#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "host_diag_core_event.h"
#endif

static const
struct nla_policy scan_policy[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE] = {.type = NLA_FLAG},
	[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE] = {.type = NLA_U64},
};

#if defined(CFG80211_SCAN_RANDOM_MAC_ADDR) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
/**
 * wlan_fill_scan_rand_attrs() - Populate the scan randomization attrs
 * @vdev: pointer to objmgr vdev
 * @flags: cfg80211 scan flags
 * @mac_addr: random mac addr from cfg80211
 * @mac_addr_mask: mac addr mask from cfg80211
 * @randomize: output variable to check scan randomization status
 * @addr: output variable to hold random addr
 * @mask: output variable to hold mac mask
 *
 * Return: None
 */
static void wlan_fill_scan_rand_attrs(struct wlan_objmgr_vdev *vdev,
				      uint32_t flags,
				      uint8_t *mac_addr,
				      uint8_t *mac_addr_mask,
				      bool *randomize,
				      uint8_t *addr,
				      uint8_t *mask)
{
	*randomize = false;
	if (!(flags & NL80211_SCAN_FLAG_RANDOM_ADDR))
		return;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE)
		return;

	if (wlan_vdev_is_up(vdev) == QDF_STATUS_SUCCESS)
		return;

	*randomize = true;
	memcpy(addr, mac_addr, QDF_MAC_ADDR_SIZE);
	memcpy(mask, mac_addr_mask, QDF_MAC_ADDR_SIZE);
	osif_debug("Random mac addr: %pM and Random mac mask: %pM",
		   addr, mask);
}

/**
 * wlan_scan_rand_attrs() - Wrapper function to fill scan random attrs
 * @vdev: pointer to objmgr vdev
 * @request: pointer to cfg80211 scan request
 * @req: pointer to cmn module scan request
 *
 * This is a wrapper function which invokes wlan_fill_scan_rand_attrs()
 * to fill random attributes of internal scan request with cfg80211_scan_request
 *
 * Return: None
 */
static void wlan_scan_rand_attrs(struct wlan_objmgr_vdev *vdev,
				 struct cfg80211_scan_request *request,
				 struct scan_start_request *req)
{
	bool *randomize = &req->scan_req.scan_random.randomize;
	uint8_t *mac_addr = req->scan_req.scan_random.mac_addr;
	uint8_t *mac_mask = req->scan_req.scan_random.mac_mask;

	wlan_fill_scan_rand_attrs(vdev, request->flags, request->mac_addr,
				  request->mac_addr_mask, randomize, mac_addr,
				  mac_mask);
	if (!*randomize)
		return;

	req->scan_req.scan_f_add_spoofed_mac_in_probe = true;
	req->scan_req.scan_f_add_rand_seq_in_probe = true;
}
#else
/**
 * wlan_scan_rand_attrs() - Wrapper function to fill scan random attrs
 * @vdev: pointer to objmgr vdev
 * @request: pointer to cfg80211 scan request
 * @req: pointer to cmn module scan request
 *
 * This is a wrapper function which invokes wlan_fill_scan_rand_attrs()
 * to fill random attributes of internal scan request with cfg80211_scan_request
 *
 * Return: None
 */
static void wlan_scan_rand_attrs(struct wlan_objmgr_vdev *vdev,
				 struct cfg80211_scan_request *request,
				 struct scan_start_request *req)
{
}
#endif

#ifdef FEATURE_WLAN_SCAN_PNO
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) || \
	defined(CFG80211_MULTI_SCAN_PLAN_BACKPORT))

/**
 * wlan_config_sched_scan_plan() - configures the sched scan plans
 *   from the framework.
 * @pno_req: pointer to PNO scan request
 * @request: pointer to scan request from framework
 *
 * Return: None
 */
static void
wlan_config_sched_scan_plan(struct pno_scan_req_params *pno_req,
			    struct cfg80211_sched_scan_request *request)
{
	/*
	 * As of now max 2 scan plans were supported by firmware
	 * if number of scan plan supported by firmware increased below logic
	 * must change.
	 */
	if (request->n_scan_plans == SCAN_PNO_MAX_PLAN_REQUEST) {
		pno_req->fast_scan_period =
			request->scan_plans[0].interval * MSEC_PER_SEC;
		pno_req->fast_scan_max_cycles =
			request->scan_plans[0].iterations;
		pno_req->slow_scan_period =
			request->scan_plans[1].interval * MSEC_PER_SEC;
	} else if (request->n_scan_plans == 1) {
		pno_req->fast_scan_period =
			request->scan_plans[0].interval * MSEC_PER_SEC;
		/*
		 * if only one scan plan is configured from framework
		 * then both fast and slow scan should be configured with the
		 * same value that is why fast scan cycles are hardcoded to one
		 */
		pno_req->fast_scan_max_cycles = 1;
		pno_req->slow_scan_period =
			request->scan_plans[0].interval * MSEC_PER_SEC;
	} else {
		osif_err("Invalid number of scan plans %d !!",
			 request->n_scan_plans);
	}
}
#else
#define wlan_config_sched_scan_plan(pno_req, request) \
	__wlan_config_sched_scan_plan(pno_req, request, psoc)

static void
__wlan_config_sched_scan_plan(struct pno_scan_req_params *pno_req,
			      struct cfg80211_sched_scan_request *request,
			      struct wlan_objmgr_psoc *psoc)
{
	uint32_t scan_timer_repeat_value, slow_scan_multiplier;

	scan_timer_repeat_value = ucfg_scan_get_scan_timer_repeat_value(psoc);
	slow_scan_multiplier = ucfg_scan_get_slow_scan_multiplier(psoc);

	pno_req->fast_scan_period = request->interval;
	pno_req->fast_scan_max_cycles = scan_timer_repeat_value;
	pno_req->slow_scan_period =
		(slow_scan_multiplier * pno_req->fast_scan_period);
	osif_debug("Base scan interval: %d sec PNO Scan Timer Repeat Value: %d",
		   (request->interval / 1000), scan_timer_repeat_value);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline void
wlan_cfg80211_sched_scan_results(struct wiphy *wiphy, uint64_t reqid)
{
	cfg80211_sched_scan_results(wiphy);
}
#else
static inline void
wlan_cfg80211_sched_scan_results(struct wiphy *wiphy, uint64_t reqid)
{
	cfg80211_sched_scan_results(wiphy, reqid);
}
#endif

/**
 * wlan_cfg80211_pno_callback() - pno callback function to handle
 * pno events.
 * @vdev: vdev ptr
 * @event: scan events
 * @args: argument
 *
 * Return: void
 */
static void wlan_cfg80211_pno_callback(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event,
	void *args)
{
	struct wlan_objmgr_pdev *pdev;
	struct pdev_osif_priv *pdev_ospriv;

	if (event->type != SCAN_EVENT_TYPE_NLO_COMPLETE)
		return;

	osif_debug("vdev id = %d", event->vdev_id);

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("pdev is NULL");
		return;
	}

	pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	if (!pdev_ospriv) {
		osif_err("pdev_ospriv is NULL");
		return;
	}
	wlan_cfg80211_sched_scan_results(pdev_ospriv->wiphy, 0);
}

#ifdef WLAN_POLICY_MGR_ENABLE
static bool wlan_cfg80211_is_ap_go_present(struct wlan_objmgr_psoc *psoc)
{
	return policy_mgr_mode_specific_connection_count(psoc,
							  PM_SAP_MODE,
							  NULL) ||
		policy_mgr_mode_specific_connection_count(psoc,
							  PM_P2P_GO_MODE,
							  NULL);
}

static QDF_STATUS wlan_cfg80211_is_chan_ok_for_dnbs(
			struct wlan_objmgr_psoc *psoc,
			u16 chan_freq, bool *ok)
{
	QDF_STATUS status = policy_mgr_is_chan_ok_for_dnbs(
				psoc, chan_freq, ok);

	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("DNBS check failed");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static bool wlan_cfg80211_is_ap_go_present(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static QDF_STATUS wlan_cfg80211_is_chan_ok_for_dnbs(
			struct wlan_objmgr_psoc *psoc,
			u16 chan_freq,
			bool *ok)
{
	if (!ok)
		return QDF_STATUS_E_INVAL;

	*ok = true;
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(CFG80211_SCAN_RANDOM_MAC_ADDR) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
/**
 * wlan_pno_scan_rand_attr() - Wrapper function to fill sched scan random attrs
 * @vdev: pointer to objmgr vdev
 * @request: pointer to cfg80211 sched scan request
 * @req: pointer to cmn module pno scan request
 *
 * This is a wrapper function which invokes wlan_fill_scan_rand_attrs()
 * to fill random attributes of internal pno scan
 * with cfg80211_sched_scan_request
 *
 * Return: None
 */
static void wlan_pno_scan_rand_attr(struct wlan_objmgr_vdev *vdev,
				    struct cfg80211_sched_scan_request *request,
				    struct pno_scan_req_params *req)
{
	bool *randomize = &req->scan_random.randomize;
	uint8_t *mac_addr = req->scan_random.mac_addr;
	uint8_t *mac_mask = req->scan_random.mac_mask;

	wlan_fill_scan_rand_attrs(vdev, request->flags, request->mac_addr,
				  request->mac_addr_mask, randomize, mac_addr,
				  mac_mask);
}
#else
/**
 * wlan_pno_scan_rand_attr() - Wrapper function to fill sched scan random attrs
 * @vdev: pointer to objmgr vdev
 * @request: pointer to cfg80211 sched scan request
 * @req: pointer to cmn module pno scan request
 *
 * This is a wrapper function which invokes wlan_fill_scan_rand_attrs()
 * to fill random attributes of internal pno scan
 * with cfg80211_sched_scan_request
 *
 * Return: None
 */
static void wlan_pno_scan_rand_attr(struct wlan_objmgr_vdev *vdev,
				    struct cfg80211_sched_scan_request *request,
				    struct pno_scan_req_params *req)
{
}
#endif

/**
 * wlan_hdd_sched_scan_update_relative_rssi() - update CPNO params
 * @pno_request: pointer to PNO scan request
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * This function is used to update Connected PNO params sent by kernel
 *
 * Return: None
 */
#if defined(CFG80211_REPORT_BETTER_BSS_IN_SCHED_SCAN) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
static inline void wlan_hdd_sched_scan_update_relative_rssi(
			struct pno_scan_req_params *pno_request,
			struct cfg80211_sched_scan_request *request)
{
	pno_request->relative_rssi_set = request->relative_rssi_set;
	pno_request->relative_rssi = request->relative_rssi;
	if (NL80211_BAND_2GHZ == request->rssi_adjust.band)
		pno_request->band_rssi_pref.band = WLAN_BAND_2_4_GHZ;
	else if (NL80211_BAND_5GHZ == request->rssi_adjust.band)
		pno_request->band_rssi_pref.band = WLAN_BAND_5_GHZ;
	pno_request->band_rssi_pref.rssi = request->rssi_adjust.delta;
}
#else
static inline void wlan_hdd_sched_scan_update_relative_rssi(
			struct pno_scan_req_params *pno_request,
			struct cfg80211_sched_scan_request *request)
{
}
#endif

#ifdef FEATURE_WLAN_SCAN_PNO
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static uint32_t wlan_config_sched_scan_start_delay(
		struct cfg80211_sched_scan_request *request)
{
	return request->delay;
}
#else
static uint32_t wlan_config_sched_scan_start_delay(
		struct cfg80211_sched_scan_request *request)
{
	return 0;
}
#endif /*(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) */
#endif /* FEATURE_WLAN_SCAN_PNO */

int wlan_cfg80211_sched_scan_start(struct wlan_objmgr_vdev *vdev,
				   struct cfg80211_sched_scan_request *request,
				   uint8_t scan_backoff_multiplier)
{
	struct pno_scan_req_params *req;
	int i, j, ret = 0;
	QDF_STATUS status;
	uint8_t num_chan = 0;
	uint16_t chan_freq;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct wlan_objmgr_psoc *psoc;
	uint32_t valid_ch[SCAN_PNO_MAX_NETW_CHANNELS_EX] = {0};
	bool enable_dfs_pno_chnl_scan;

	if (ucfg_scan_get_pno_in_progress(vdev)) {
		osif_debug("pno is already in progress");
		return -EBUSY;
	}

	if (ucfg_scan_get_pdev_status(pdev) !=
	   SCAN_NOT_IN_PROGRESS) {
		status = wlan_abort_scan(pdev,
				wlan_objmgr_pdev_get_pdev_id(pdev),
				INVAL_VDEV_ID, INVAL_SCAN_ID, true);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("aborting the existing scan is unsuccessful");
			return -EBUSY;
		}
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return -ENOMEM;

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);

	req->networks_cnt = request->n_match_sets;
	req->vdev_id = wlan_vdev_get_id(vdev);

	if ((!req->networks_cnt) ||
	    (req->networks_cnt > SCAN_PNO_MAX_SUPP_NETWORKS)) {
		osif_err("Network input is not correct %d",
			 req->networks_cnt);
		ret = -EINVAL;
		goto error;
	}

	if (request->n_channels > SCAN_PNO_MAX_NETW_CHANNELS_EX) {
		osif_err("Incorrect number of channels %d",
			 request->n_channels);
		ret = -EINVAL;
		goto error;
	}

	enable_dfs_pno_chnl_scan = ucfg_scan_is_dfs_chnl_scan_enabled(psoc);
	if (request->n_channels) {
		char *chl = qdf_mem_malloc((request->n_channels * 5) + 1);
		int len = 0;
		bool ap_or_go_present = wlan_cfg80211_is_ap_go_present(psoc);

		if (!chl) {
			ret = -ENOMEM;
			goto error;
		}
		for (i = 0; i < request->n_channels; i++) {
			chan_freq = request->channels[i]->center_freq;
			if ((!enable_dfs_pno_chnl_scan) &&
			    (wlan_reg_is_dfs_for_freq(pdev, chan_freq))) {
				osif_debug("Dropping DFS channel freq :%d",
					   chan_freq);
				continue;
			}
			if (wlan_reg_is_dsrc_freq(chan_freq))
				continue;

			if (ap_or_go_present) {
				bool ok;

				status =
				wlan_cfg80211_is_chan_ok_for_dnbs(psoc,
								  chan_freq,
								  &ok);
				if (QDF_IS_STATUS_ERROR(status)) {
					osif_err("DNBS check failed");
					qdf_mem_free(chl);
					chl = NULL;
					ret = -EINVAL;
					goto error;
				}
				if (!ok)
					continue;
			}
			len += snprintf(chl + len, 5, "%d ", chan_freq);
			valid_ch[num_chan++] = chan_freq;
		}
		osif_notice("No. of Scan Channels: %d", num_chan);
		osif_notice("Channel-List: %s", chl);
		qdf_mem_free(chl);
		chl = NULL;
		/* If all channels are DFS and dropped,
		 * then ignore the PNO request
		 */
		if (!num_chan) {
			osif_notice("Channel list empty due to filtering of DSRC");
			ret = -EINVAL;
			goto error;
		}
	}

	/* Filling per profile  params */
	for (i = 0; i < req->networks_cnt; i++) {
		req->networks_list[i].ssid.length =
			request->match_sets[i].ssid.ssid_len;

		if ((!req->networks_list[i].ssid.length) ||
		    (req->networks_list[i].ssid.length > WLAN_SSID_MAX_LEN)) {
			osif_err(" SSID Len %d is not correct for network %d",
				 req->networks_list[i].ssid.length, i);
			ret = -EINVAL;
			goto error;
		}

		qdf_mem_copy(req->networks_list[i].ssid.ssid,
			request->match_sets[i].ssid.ssid,
			req->networks_list[i].ssid.length);
		req->networks_list[i].authentication = 0;   /*eAUTH_TYPE_ANY */
		req->networks_list[i].encryption = 0;       /*eED_ANY */
		req->networks_list[i].bc_new_type = 0;    /*eBCAST_UNKNOWN */

		osif_notice("Received ssid:%.*s",
			    req->networks_list[i].ssid.length,
			    req->networks_list[i].ssid.ssid);

		/*Copying list of valid channel into request */
		qdf_mem_copy(req->networks_list[i].channels, valid_ch,
			num_chan * sizeof(uint32_t));
		req->networks_list[i].channel_cnt = num_chan;
		req->networks_list[i].rssi_thresh =
			request->match_sets[i].rssi_thold;
	}

	/* set scan to passive if no SSIDs are specified in the request */
	if (0 == request->n_ssids)
		req->do_passive_scan = true;
	else
		req->do_passive_scan = false;

	for (i = 0; i < request->n_ssids; i++) {
		j = 0;
		while (j < req->networks_cnt) {
			if ((req->networks_list[j].ssid.length ==
			     request->ssids[i].ssid_len) &&
			    (!qdf_mem_cmp(req->networks_list[j].ssid.ssid,
					 request->ssids[i].ssid,
					 req->networks_list[j].ssid.length))) {
				req->networks_list[j].bc_new_type =
					SSID_BC_TYPE_HIDDEN;
				break;
			}
			j++;
		}
	}
	osif_notice("Number of hidden networks being Configured = %d",
		    request->n_ssids);

	/*
	 * Before Kernel 4.4
	 *   Driver gets only one time interval which is hard coded in
	 *   supplicant for 10000ms.
	 *
	 * After Kernel 4.4
	 *   User can configure multiple scan_plans, each scan would have
	 *   separate scan cycle and interval. (interval is in unit of second.)
	 *   For our use case, we would only have supplicant set one scan_plan,
	 *   and firmware also support only one as well, so pick up the first
	 *   index.
	 *
	 *   Taking power consumption into account
	 *   firmware after gPNOScanTimerRepeatValue times fast_scan_period
	 *   switches slow_scan_period. This is less frequent scans and firmware
	 *   shall be in slow_scan_period mode until next PNO Start.
	 */
	wlan_config_sched_scan_plan(req, request);
	req->delay_start_time = wlan_config_sched_scan_start_delay(request);
	req->scan_backoff_multiplier = scan_backoff_multiplier;
	osif_notice("Base scan interval: %d sec, scan cycles: %d, slow scan interval %d",
		    req->fast_scan_period, req->fast_scan_max_cycles,
		    req->slow_scan_period);
	wlan_hdd_sched_scan_update_relative_rssi(req, request);

	psoc = wlan_pdev_get_psoc(pdev);
	ucfg_scan_register_pno_cb(psoc,
		wlan_cfg80211_pno_callback, NULL);
	ucfg_scan_get_pno_def_params(vdev, req);

	if (req->scan_random.randomize)
		wlan_pno_scan_rand_attr(vdev, request, req);

	if (ucfg_ie_whitelist_enabled(psoc, vdev))
		ucfg_copy_ie_whitelist_attrs(psoc, &req->ie_whitelist);
	status = ucfg_scan_pno_start(vdev, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to enable PNO");
		ret = -EINVAL;
		goto error;
	}

	osif_info("PNO scan request offloaded");

error:
	qdf_mem_free(req);
	return ret;
}

int wlan_cfg80211_sched_scan_stop(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;

	status = ucfg_scan_pno_stop(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Failed to disabled PNO");
	else
		osif_info("PNO scan disabled");

	return 0;
}
#endif /*FEATURE_WLAN_SCAN_PNO */

/**
 * wlan_copy_bssid_scan_request() - API to copy the bssid to Scan request
 * @scan_req: Pointer to scan_start_request
 * @request: scan request from Supplicant
 *
 * This API copies the BSSID in scan request from Supplicant and copies it to
 * the scan_start_request
 *
 * Return: None
 */
#if defined(CFG80211_SCAN_BSSID) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void
wlan_copy_bssid_scan_request(struct scan_start_request *scan_req,
		struct cfg80211_scan_request *request)
{
	qdf_mem_copy(scan_req->scan_req.bssid_list[0].bytes,
				request->bssid, QDF_MAC_ADDR_SIZE);
}
#else
static inline void
wlan_copy_bssid_scan_request(struct scan_start_request *scan_req,
		struct cfg80211_scan_request *request)
{

}
#endif

/**
 * wlan_scan_request_enqueue() - enqueue Scan Request
 * @pdev: pointer to pdev object
 * @req: Pointer to the scan request
 * @source: source of the scan request
 * @scan_id: scan identifier
 *
 * Enqueue scan request in the global  scan list.This list
 * stores the active scan request information.
 *
 * Return: 0 on success, error number otherwise
 */
static int wlan_scan_request_enqueue(struct wlan_objmgr_pdev *pdev,
			struct cfg80211_scan_request *req,
			uint8_t source, uint32_t scan_id)
{
	struct scan_req *scan_req;
	QDF_STATUS status;
	struct pdev_osif_priv *osif_ctx;
	struct osif_scan_pdev *osif_scan;

	scan_req = qdf_mem_malloc(sizeof(*scan_req));
	if (!scan_req)
		return -ENOMEM;

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	osif_scan = osif_ctx->osif_scan;
	scan_req->scan_request = req;
	scan_req->source = source;
	scan_req->scan_id = scan_id;
	scan_req->dev = req->wdev->netdev;

	qdf_mutex_acquire(&osif_scan->scan_req_q_lock);
	if (qdf_list_size(&osif_scan->scan_req_q) < WLAN_MAX_SCAN_COUNT)
		status = qdf_list_insert_back(&osif_scan->scan_req_q,
					      &scan_req->node);
	else
		status = QDF_STATUS_E_RESOURCES;
	qdf_mutex_release(&osif_scan->scan_req_q_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_rl_debug("Failed to enqueue Scan Req as max scan %d already queued",
			      qdf_list_size(&osif_scan->scan_req_q));
		qdf_mem_free(scan_req);
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_scan_request_dequeue() - dequeue scan request
 * @nl_ctx: Global HDD context
 * @scan_id: scan id
 * @req: scan request
 * @dev: net device
 * @source : returns source of the scan request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_scan_request_dequeue(
	struct wlan_objmgr_pdev *pdev,
	uint32_t scan_id, struct cfg80211_scan_request **req,
	uint8_t *source, struct net_device **dev)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct scan_req *scan_req;
	qdf_list_node_t *node = NULL, *next_node = NULL;
	struct pdev_osif_priv *osif_ctx;
	struct osif_scan_pdev *scan_priv;

	osif_debug("Dequeue Scan id: %d", scan_id);

	if ((!source) || (!req)) {
		osif_err("source or request is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	if (!osif_ctx) {
		osif_err("Failed to retrieve osif context");
		return status;
	}
	scan_priv = osif_ctx->osif_scan;

	if (qdf_list_empty(&scan_priv->scan_req_q)) {
		osif_info("Scan List is empty");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&scan_priv->scan_req_q_lock);
	if (QDF_STATUS_SUCCESS !=
		qdf_list_peek_front(&scan_priv->scan_req_q, &next_node)) {
		qdf_mutex_release(&scan_priv->scan_req_q_lock);
		osif_err("Failed to remove Scan Req from queue");
		return QDF_STATUS_E_FAILURE;
	}

	do {
		node = next_node;
		scan_req = qdf_container_of(node, struct scan_req, node);
		if (scan_req->scan_id == scan_id) {
			status = qdf_list_remove_node(&scan_priv->scan_req_q,
						      node);
			if (status == QDF_STATUS_SUCCESS) {
				*req = scan_req->scan_request;
				*source = scan_req->source;
				*dev = scan_req->dev;
				qdf_mem_free(scan_req);
				qdf_mutex_release(&scan_priv->scan_req_q_lock);
				osif_debug("removed Scan id: %d, req = %pK, pending scans %d",
					   scan_id, req,
					   qdf_list_size(&scan_priv->scan_req_q));
				return QDF_STATUS_SUCCESS;
			} else {
				qdf_mutex_release(&scan_priv->scan_req_q_lock);
				osif_err("Failed to remove node scan id %d, pending scans %d",
					 scan_id,
					 qdf_list_size(&scan_priv->scan_req_q));
				return status;
			}
		}
	} while (QDF_STATUS_SUCCESS ==
		qdf_list_peek_next(&scan_priv->scan_req_q, node, &next_node));
	qdf_mutex_release(&scan_priv->scan_req_q_lock);
	osif_err("Failed to find scan id %d", scan_id);

	return status;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
/**
 * wlan_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @netdev: Net device
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void wlan_cfg80211_scan_done(struct net_device *netdev,
				    struct cfg80211_scan_request *req,
				    bool aborted)
{
	struct cfg80211_scan_info info = {
		.aborted = aborted
	};

	if (netdev->flags & IFF_UP)
		cfg80211_scan_done(req, &info);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
/**
 * wlan_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @netdev: Net device
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void wlan_cfg80211_scan_done(struct net_device *netdev,
				    struct cfg80211_scan_request *req,
				    bool aborted)
{
	if (netdev->flags & IFF_UP)
		cfg80211_scan_done(req, aborted);
}
#endif

/**
 * wlan_vendor_scan_callback() - Scan completed callback event
 *
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function sends scan completed callback event to NL.
 *
 * Return: none
 */
static void wlan_vendor_scan_callback(struct cfg80211_scan_request *req,
					bool aborted)
{
	struct sk_buff *skb;
	struct nlattr *attr;
	int i;
	uint8_t scan_status;
	uint64_t cookie;

	skb = cfg80211_vendor_event_alloc(req->wdev->wiphy, req->wdev,
			SCAN_DONE_EVENT_BUF_SIZE + 4 + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
			GFP_ATOMIC);

	if (!skb) {
		osif_err("skb alloc failed");
		qdf_mem_free(req);
		return;
	}

	cookie = (uintptr_t)req;

	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_ssids; i++) {
		if (nla_put(skb, i, req->ssids[i].ssid_len, req->ssids[i].ssid))
			goto nla_put_failure;
	}
	nla_nest_end(skb, attr);

	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_channels; i++) {
		if (nla_put_u32(skb, i, req->channels[i]->center_freq))
			goto nla_put_failure;
	}
	nla_nest_end(skb, attr);

	if (req->ie &&
		nla_put(skb, QCA_WLAN_VENDOR_ATTR_SCAN_IE, req->ie_len,
			req->ie))
		goto nla_put_failure;

	if (req->flags &&
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS, req->flags))
		goto nla_put_failure;

	if (wlan_cfg80211_nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
					cookie))
		goto nla_put_failure;

	scan_status = (aborted == true) ? VENDOR_SCAN_STATUS_ABORTED :
		VENDOR_SCAN_STATUS_NEW_RESULTS;
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_SCAN_STATUS, scan_status))
		goto nla_put_failure;

	cfg80211_vendor_event(skb, GFP_ATOMIC);
	qdf_mem_free(req);

	return;

nla_put_failure:
	kfree_skb(skb);
	qdf_mem_free(req);
}

/**
 * wlan_scan_acquire_wake_lock_timeout() - acquire scan wake lock
 * @psoc: psoc ptr
 * @scan_wake_lock: Scan wake lock
 * @timeout: timeout in ms
 *
 * Return: void
 */
static inline
void wlan_scan_acquire_wake_lock_timeout(struct wlan_objmgr_psoc *psoc,
					 qdf_wake_lock_t *scan_wake_lock,
					 uint32_t timeout)
{
	if (!psoc || !scan_wake_lock)
		return;

	if (ucfg_scan_wake_lock_in_user_scan(psoc))
		qdf_wake_lock_timeout_acquire(scan_wake_lock, timeout);
}


/**
 * wlan_scan_release_wake_lock() - release scan wake lock
 * @psoc: psoc ptr
 * @scan_wake_lock: Scan wake lock
 *
 * Return: void
 */
#ifdef FEATURE_WLAN_DIAG_SUPPORT
static inline
void wlan_scan_release_wake_lock(struct wlan_objmgr_psoc *psoc,
				 qdf_wake_lock_t *scan_wake_lock)
{
	if (!psoc || !scan_wake_lock)
		return;

	if (ucfg_scan_wake_lock_in_user_scan(psoc))
		qdf_wake_lock_release(scan_wake_lock,
				      WIFI_POWER_EVENT_WAKELOCK_SCAN);
}
#else
static inline
void wlan_scan_release_wake_lock(struct wlan_objmgr_psoc *psoc,
				 qdf_wake_lock_t *scan_wake_lock)
{
	if (!psoc || !scan_wake_lock)
		return;

	if (ucfg_scan_wake_lock_in_user_scan(psoc))
		qdf_wake_lock_release(scan_wake_lock, 0);
}
#endif

/**
 * wlan_cfg80211_scan_done_callback() - scan done callback function called after
 * scan is finished
 * @vdev: vdev ptr
 * @event: Scan event
 * @args: Scan cb arg
 *
 * Return: void
 */
static void wlan_cfg80211_scan_done_callback(
					struct wlan_objmgr_vdev *vdev,
					struct scan_event *event,
					void *args)
{
	struct cfg80211_scan_request *req = NULL;
	bool success = false;
	uint32_t scan_id = event->scan_id;
	uint8_t source = NL_SCAN;
	struct wlan_objmgr_pdev *pdev;
	struct pdev_osif_priv *osif_priv;
	struct net_device *netdev = NULL;
	QDF_STATUS status;

	qdf_mtrace(QDF_MODULE_ID_SCAN, QDF_MODULE_ID_OS_IF, event->type,
		   event->vdev_id, event->scan_id);

	if (!util_is_scan_completed(event, &success))
		return;

	osif_debug("scan ID = %d vdev id = %d, event type %s(%d) reason = %s(%d)",
		   scan_id, event->vdev_id,
		   util_scan_get_ev_type_name(event->type), event->type,
		   util_scan_get_ev_reason_name(event->reason),
		   event->reason);

	pdev = wlan_vdev_get_pdev(vdev);
	status = wlan_scan_request_dequeue(
			pdev, scan_id, &req, &source, &netdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Dequeue of scan request failed ID: %d", scan_id);
		goto allow_suspend;
	}

	if (!netdev) {
		osif_err("net dev is NULL,Drop scan event Id: %d", scan_id);
		goto allow_suspend;
	}

	/* Make sure vdev is active */
	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_OSIF_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to get vdev reference: scan Id: %d", scan_id);
		goto allow_suspend;
	}

	/*
	 * Scan can be triggred from NL or vendor scan
	 * - If scan is triggered from NL then cfg80211 scan done should be
	 * called to updated scan completion to NL.
	 * - If scan is triggred through vendor command then
	 * scan done event will be posted
	 */
	if (NL_SCAN == source)
		wlan_cfg80211_scan_done(netdev, req, !success);
	else
		wlan_vendor_scan_callback(req, !success);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
allow_suspend:
	osif_priv = wlan_pdev_get_ospriv(pdev);
	if (qdf_list_empty(&osif_priv->osif_scan->scan_req_q)) {
		struct wlan_objmgr_psoc *psoc;

		qdf_runtime_pm_allow_suspend(
			&osif_priv->osif_scan->runtime_pm_lock);

		psoc = wlan_pdev_get_psoc(pdev);
		wlan_scan_release_wake_lock(psoc,
					&osif_priv->osif_scan->scan_wake_lock);
		/*
		 * Acquire wakelock to handle the case where APP's tries
		 * to suspend immediately after the driver gets connect
		 * request(i.e after scan) from supplicant, this result in
		 * app's is suspending and not able to process the connect
		 * request to AP
		 */
		wlan_scan_acquire_wake_lock_timeout(psoc,
					&osif_priv->osif_scan->scan_wake_lock,
					SCAN_WAKE_LOCK_CONNECT_DURATION);
	}
}

QDF_STATUS wlan_scan_runtime_pm_init(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;

	wlan_pdev_obj_lock(pdev);
	osif_priv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_obj_unlock(pdev);

	scan_priv = osif_priv->osif_scan;

	return qdf_runtime_lock_init(&scan_priv->runtime_pm_lock);
}

void wlan_scan_runtime_pm_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;

	wlan_pdev_obj_lock(pdev);
	osif_priv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_obj_unlock(pdev);

	scan_priv = osif_priv->osif_scan;
	qdf_runtime_lock_deinit(&scan_priv->runtime_pm_lock);
}

QDF_STATUS wlan_cfg80211_scan_priv_init(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;
	struct wlan_objmgr_psoc *psoc;
	wlan_scan_requester req_id;

	psoc = wlan_pdev_get_psoc(pdev);

	req_id = ucfg_scan_register_requester(psoc, "CFG",
		wlan_cfg80211_scan_done_callback, NULL);

	osif_priv = wlan_pdev_get_ospriv(pdev);
	scan_priv = qdf_mem_malloc(sizeof(*scan_priv));
	if (!scan_priv)
		return QDF_STATUS_E_NOMEM;

	/* Initialize the scan request queue */
	osif_priv->osif_scan = scan_priv;
	scan_priv->req_id = req_id;
	qdf_list_create(&scan_priv->scan_req_q, WLAN_MAX_SCAN_COUNT);
	qdf_mutex_create(&scan_priv->scan_req_q_lock);
	qdf_wake_lock_create(&scan_priv->scan_wake_lock, "scan_wake_lock");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cfg80211_scan_priv_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	osif_priv = wlan_pdev_get_ospriv(pdev);

	wlan_cfg80211_cleanup_scan_queue(pdev, NULL);
	scan_priv = osif_priv->osif_scan;
	qdf_wake_lock_destroy(&scan_priv->scan_wake_lock);
	qdf_mutex_destroy(&scan_priv->scan_req_q_lock);
	qdf_list_destroy(&scan_priv->scan_req_q);
	ucfg_scan_unregister_requester(psoc, scan_priv->req_id);
	osif_priv->osif_scan = NULL;
	qdf_mem_free(scan_priv);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_cfg80211_enqueue_for_cleanup() - Function to populate scan cleanup queue
 * @scan_cleanup_q: Scan cleanup queue to be populated
 * @scan_priv: Pointer to scan related data used by cfg80211 scan
 * @dev: Netdevice pointer
 *
 * The function synchrounously iterates through the global scan queue to
 * identify entries that have to be cleaned up, copies identified entries
 * to another queue(to send scan complete event to NL later) and removes the
 * entry from the global scan queue.
 *
 * Return: None
 */
static void
wlan_cfg80211_enqueue_for_cleanup(qdf_list_t *scan_cleanup_q,
				  struct osif_scan_pdev *scan_priv,
				  struct net_device *dev)
{
	struct scan_req *scan_req, *scan_cleanup;
	qdf_list_node_t *node = NULL, *next_node = NULL;

	qdf_mutex_acquire(&scan_priv->scan_req_q_lock);
	if (QDF_STATUS_SUCCESS !=
		qdf_list_peek_front(&scan_priv->scan_req_q,
				    &node)) {
		qdf_mutex_release(&scan_priv->scan_req_q_lock);
		return;
	}

	while (node) {
		/*
		 * Keep track of the next node, to traverse through the list
		 * in the event of the current node being deleted.
		 */
		qdf_list_peek_next(&scan_priv->scan_req_q,
				   node, &next_node);
		scan_req = qdf_container_of(node, struct scan_req, node);
		if (!dev || (dev == scan_req->dev)) {
			scan_cleanup = qdf_mem_malloc(sizeof(struct scan_req));
			if (!scan_cleanup) {
				qdf_mutex_release(&scan_priv->scan_req_q_lock);
				return;
			}
			scan_cleanup->scan_request = scan_req->scan_request;
			scan_cleanup->scan_id = scan_req->scan_id;
			scan_cleanup->source = scan_req->source;
			scan_cleanup->dev = scan_req->dev;
			qdf_list_insert_back(scan_cleanup_q,
					     &scan_cleanup->node);
			if (QDF_STATUS_SUCCESS !=
				qdf_list_remove_node(&scan_priv->scan_req_q,
						     node)) {
				qdf_mutex_release(&scan_priv->scan_req_q_lock);
				osif_err("Failed to remove scan request");
				return;
			}
			qdf_mem_free(scan_req);
		}
		node = next_node;
		next_node = NULL;
	}
	qdf_mutex_release(&scan_priv->scan_req_q_lock);
}

void wlan_cfg80211_cleanup_scan_queue(struct wlan_objmgr_pdev *pdev,
				      struct net_device *dev)
{
	struct scan_req *scan_req;
	struct cfg80211_scan_request *req;
	uint8_t source;
	bool aborted = true;
	struct pdev_osif_priv *osif_priv;
	qdf_list_t scan_cleanup_q;
	qdf_list_node_t *node = NULL;

	if (!pdev) {
		osif_err("pdev is Null");
		return;
	}

	osif_priv = wlan_pdev_get_ospriv(pdev);

	/*
	 * To avoid any race conditions, create a local list to copy all the
	 * scan entries to be removed and then send scan complete for each of
	 * the identified entries to NL.
	 */
	qdf_list_create(&scan_cleanup_q, WLAN_MAX_SCAN_COUNT);
	wlan_cfg80211_enqueue_for_cleanup(&scan_cleanup_q,
					  osif_priv->osif_scan, dev);

	while (!qdf_list_empty(&scan_cleanup_q)) {
		if (QDF_STATUS_SUCCESS != qdf_list_remove_front(&scan_cleanup_q,
								&node)) {
			osif_err("Failed to remove scan request");
			return;
		}
		scan_req = container_of(node, struct scan_req, node);
		req = scan_req->scan_request;
		source = scan_req->source;
		if (NL_SCAN == source)
			wlan_cfg80211_scan_done(scan_req->dev, req,
						aborted);
		else
			wlan_vendor_scan_callback(req, aborted);

		qdf_mem_free(scan_req);
	}
	qdf_list_destroy(&scan_cleanup_q);

	return;
}

/**
 * wlan_cfg80211_update_scan_policy_type_flags() - Set scan flags according to
 * scan request
 * @scan_req: Pointer to csr scan req
 *
 * Return: None
 */
#if defined(CFG80211_SCAN_DBS_CONTROL_SUPPORT) || \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
static void wlan_cfg80211_update_scan_policy_type_flags(
	struct cfg80211_scan_request *req,
	struct scan_req_params *scan_req)
{
	if (req->flags & NL80211_SCAN_FLAG_HIGH_ACCURACY)
		scan_req->scan_policy_high_accuracy = true;
	if (req->flags & NL80211_SCAN_FLAG_LOW_SPAN)
		scan_req->scan_policy_low_span = true;
	if (req->flags & NL80211_SCAN_FLAG_LOW_POWER)
		scan_req->scan_policy_low_power = true;
}
#else
static inline void wlan_cfg80211_update_scan_policy_type_flags(
		struct cfg80211_scan_request *req,
		struct scan_req_params *scan_req)
{
}
#endif

#ifdef WLAN_POLICY_MGR_ENABLE
static bool
wlan_cfg80211_allow_simultaneous_scan(struct wlan_objmgr_psoc *psoc)
{
	return policy_mgr_is_scan_simultaneous_capable(psoc);
}
#else
static bool
wlan_cfg80211_allow_simultaneous_scan(struct wlan_objmgr_psoc *psoc)
{
	return true;
}
#endif

int wlan_cfg80211_scan(struct wlan_objmgr_vdev *vdev,
		       struct cfg80211_scan_request *request,
		       struct scan_params *params)
{
	struct scan_start_request *req;
	struct wlan_ssid *pssid;
	uint8_t i;
	int ret = 0;
	uint8_t num_chan = 0;
	uint32_t c_freq;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	wlan_scan_requester req_id;
	struct pdev_osif_priv *osif_priv;
	struct wlan_objmgr_psoc *psoc;
	wlan_scan_id scan_id;
	bool is_p2p_scan = false;
	enum wlan_band band;
	struct net_device *netdev = NULL;
	QDF_STATUS qdf_status;
	uint32_t extra_ie_len = 0;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		osif_err("Invalid psoc object");
		return -EINVAL;
	}

	/* Get NL global context from objmgr*/
	osif_priv = wlan_pdev_get_ospriv(pdev);
	if (!osif_priv) {
		osif_err("Invalid osif priv object");
		return -EINVAL;
	}

	/*
	 * For a non-SAP vdevs, if a scan is already going on i.e the scan queue
	 * is not empty, and the simultaneous scan is disabled, dont allow 2nd
	 * scan.
	 */
	if (!wlan_cfg80211_allow_simultaneous_scan(psoc) &&
	    !qdf_list_empty(&osif_priv->osif_scan->scan_req_q) &&
	    wlan_vdev_mlme_get_opmode(vdev) != QDF_SAP_MODE) {
		osif_err("Simultaneous scan disabled, reject scan");
		return -EBUSY;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return -EINVAL;

	/* Initialize the scan global params */
	ucfg_scan_init_default_params(vdev, req);

	req_id = osif_priv->osif_scan->req_id;
	scan_id = ucfg_scan_get_scan_id(psoc);
	if (!scan_id) {
		osif_err("Invalid scan id");
		qdf_mem_free(req);
		return -EINVAL;
	}

	/* fill the scan request structure */
	req->vdev = vdev;
	req->scan_req.vdev_id = wlan_vdev_get_id(vdev);
	req->scan_req.scan_id = scan_id;
	req->scan_req.scan_req_id = req_id;

	/* Enqueue the scan request */
	ret = wlan_scan_request_enqueue(pdev, request, params->source,
					req->scan_req.scan_id);
	if (ret) {
		qdf_mem_free(req);
		return ret;
	}

	/* Update scan policy type flags according to cfg scan request */
	wlan_cfg80211_update_scan_policy_type_flags(request,
					     &req->scan_req);
	/*
	 * Even though supplicant doesn't provide any SSIDs, n_ssids is
	 * set to 1.  Because of this, driver is assuming that this is not
	 * wildcard scan and so is not aging out the scan results.
	 */
	if ((request->ssids) && (request->n_ssids == 1) &&
	    ('\0' == request->ssids->ssid[0])) {
		request->n_ssids = 0;
	}

	if ((request->ssids) && (0 < request->n_ssids)) {
		int j;
		req->scan_req.num_ssids = request->n_ssids;

		if (req->scan_req.num_ssids > WLAN_SCAN_MAX_NUM_SSID) {
			osif_info("number of ssid received %d is greater than MAX %d so copy only MAX nuber of SSIDs",
				  req->scan_req.num_ssids,
				  WLAN_SCAN_MAX_NUM_SSID);
			req->scan_req.num_ssids = WLAN_SCAN_MAX_NUM_SSID;
		}
		/* copy all the ssid's and their length */
		for (j = 0; j < req->scan_req.num_ssids; j++)  {
			pssid = &req->scan_req.ssid[j];
			/* get the ssid length */
			pssid->length = request->ssids[j].ssid_len;
			if (pssid->length > WLAN_SSID_MAX_LEN)
				pssid->length = WLAN_SSID_MAX_LEN;
			qdf_mem_copy(pssid->ssid,
				     &request->ssids[j].ssid[0],
				     pssid->length);
			osif_info("SSID number %d: %.*s", j, pssid->length,
				  pssid->ssid);
		}
	}
	if (request->ssids ||
	   (wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_GO_MODE))
		req->scan_req.scan_f_passive = false;

	if (params->half_rate)
		req->scan_req.scan_f_half_rate = true;
	else if (params->quarter_rate)
		req->scan_req.scan_f_quarter_rate = true;

	if (params->strict_pscan)
		req->scan_req.scan_f_strict_passive_pch = true;

	if ((request->n_ssids == 1) && request->ssids &&
	   !qdf_mem_cmp(&request->ssids[0], "DIRECT-", 7))
		is_p2p_scan = true;

	if (is_p2p_scan && request->no_cck)
		req->scan_req.scan_type = SCAN_TYPE_P2P_SEARCH;

	/* Set dwell time mode according to scan policy type flags */
	if (ucfg_scan_cfg_honour_nl_scan_policy_flags(psoc)) {
		if (req->scan_req.scan_policy_high_accuracy)
			req->scan_req.adaptive_dwell_time_mode =
						SCAN_DWELL_MODE_STATIC;
		if (req->scan_req.scan_policy_low_power ||
		    req->scan_req.scan_policy_low_span)
			req->scan_req.adaptive_dwell_time_mode =
						SCAN_DWELL_MODE_AGGRESSIVE;
	}

	/*
	 * FW require at least 1 MAC to send probe request.
	 * If MAC is all 0 set it to BC addr as this is the address on
	 * which fw will send probe req.
	 */
	req->scan_req.num_bssid = 1;
	wlan_copy_bssid_scan_request(req, request);
	if (qdf_is_macaddr_zero(&req->scan_req.bssid_list[0]))
		qdf_set_macaddr_broadcast(&req->scan_req.bssid_list[0]);

	if (request->n_channels) {
		uint32_t buff_len = (request->n_channels * 5) + 1;
		char *chl = qdf_mem_malloc(buff_len);
		int len = 0;
#ifdef WLAN_POLICY_MGR_ENABLE
		bool ap_or_go_present =
			policy_mgr_mode_specific_connection_count(
			     psoc, PM_SAP_MODE, NULL) ||
			     policy_mgr_mode_specific_connection_count(
			     psoc, PM_P2P_GO_MODE, NULL);
#endif
		if (!chl) {
			ret = -ENOMEM;
			goto err;
		}
		for (i = 0; i < request->n_channels; i++) {
			c_freq = request->channels[i]->center_freq;
			if (wlan_reg_is_dsrc_freq(c_freq))
				continue;
#ifdef WLAN_POLICY_MGR_ENABLE
			if (ap_or_go_present) {
				bool ok;

				qdf_status = policy_mgr_is_chan_ok_for_dnbs(
							psoc, c_freq, &ok);

				if (QDF_IS_STATUS_ERROR(qdf_status)) {
					osif_err("DNBS check failed");
					qdf_mem_free(chl);
					chl = NULL;
					ret = -EINVAL;
					goto err;
				}
				if (!ok)
					continue;
			}
#endif
			len += snprintf(chl + len, buff_len - len, "%d ",
					c_freq);
			req->scan_req.chan_list.chan[num_chan].freq = c_freq;
			band = util_scan_scm_freq_to_band(c_freq);
			if (band == WLAN_BAND_2_4_GHZ)
				req->scan_req.chan_list.chan[num_chan].phymode =
					SCAN_PHY_MODE_11G;
			else
				req->scan_req.chan_list.chan[num_chan].phymode =
					SCAN_PHY_MODE_11A;
			num_chan++;
			if (num_chan >= NUM_CHANNELS)
				break;
		}
		osif_info("Channel-List: %s", chl);
		qdf_mem_free(chl);
		chl = NULL;
		osif_info("No. of Scan Channels: %d", num_chan);
	}
	if (!num_chan) {
		osif_err("Received zero non-dsrc channels");
		ret = -EINVAL;
		goto err;
	}
	req->scan_req.chan_list.num_chan = num_chan;

	/* P2P increase the scan priority */
	if (is_p2p_scan)
		req->scan_req.scan_priority = SCAN_PRIORITY_HIGH;

	if (params->priority != SCAN_PRIORITY_COUNT)
		req->scan_req.scan_priority = params->priority;

	if (request->ie_len)
		extra_ie_len = request->ie_len;
	else if (params->default_ie.ptr && params->default_ie.len)
		extra_ie_len = params->default_ie.len;

	if (params->vendor_ie.ptr && params->vendor_ie.len)
		extra_ie_len += params->vendor_ie.len;

	if (extra_ie_len) {
		req->scan_req.extraie.ptr = qdf_mem_malloc(extra_ie_len);
		if (!req->scan_req.extraie.ptr) {
			ret = -ENOMEM;
			goto err;
		}
	}

	if (request->ie_len) {
		req->scan_req.extraie.len = request->ie_len;
		qdf_mem_copy(req->scan_req.extraie.ptr, request->ie,
			     request->ie_len);
	} else if (params->default_ie.ptr && params->default_ie.len) {
		req->scan_req.extraie.len = params->default_ie.len;
		qdf_mem_copy(req->scan_req.extraie.ptr, params->default_ie.ptr,
			     params->default_ie.len);
	}

	if (params->vendor_ie.ptr && params->vendor_ie.len) {
		qdf_mem_copy((req->scan_req.extraie.ptr +
			      req->scan_req.extraie.len),
			     params->vendor_ie.ptr, params->vendor_ie.len);

		req->scan_req.extraie.len += params->vendor_ie.len;
	}

	if (!is_p2p_scan) {
		if (req->scan_req.scan_random.randomize)
			wlan_scan_rand_attrs(vdev, request, req);
		if (ucfg_ie_whitelist_enabled(psoc, vdev) &&
		    ucfg_copy_ie_whitelist_attrs(psoc,
					&req->scan_req.ie_whitelist))
			req->scan_req.scan_f_en_ie_whitelist_in_probe = true;
	}

	if (request->flags & NL80211_SCAN_FLAG_FLUSH)
		ucfg_scan_flush_results(pdev, NULL);

	/*
	 * Acquire wakelock to handle the case where APP's send scan to connect.
	 * If suspend is received during scan scan will be aborted and APP will
	 * not get scan result and not connect. eg if PNO is implemented in
	 * framework.
	 */
	wlan_scan_acquire_wake_lock_timeout(psoc,
					&osif_priv->osif_scan->scan_wake_lock,
					SCAN_WAKE_LOCK_SCAN_DURATION);

	qdf_runtime_pm_prevent_suspend(
		&osif_priv->osif_scan->runtime_pm_lock);

	qdf_status = ucfg_scan_start(req);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		osif_err("ucfg_scan_start returned error %d", qdf_status);
		if (qdf_status == QDF_STATUS_E_RESOURCES)
			osif_err("HO is in progress.So defer the scan by informing busy");
		wlan_scan_request_dequeue(pdev, scan_id, &request,
					  &params->source, &netdev);
		if (qdf_list_empty(&osif_priv->osif_scan->scan_req_q)) {
			qdf_runtime_pm_allow_suspend(
				&osif_priv->osif_scan->runtime_pm_lock);
			wlan_scan_release_wake_lock(psoc,
				&osif_priv->osif_scan->scan_wake_lock);
		}
	}

	return qdf_status_to_os_return(qdf_status);

err:
	qdf_mem_free(req);
	wlan_scan_request_dequeue(pdev, scan_id, &request,
				  &params->source, &netdev);
	return ret;
}

/**
 * wlan_get_scanid() - API to get the scan id
 * from the scan cookie attribute.
 * @pdev: Pointer to pdev object
 * @scan_id: Pointer to scan id
 * @cookie : Scan cookie attribute
 *
 * API to get the scan id from the scan cookie attribute
 * sent from supplicant by matching scan request.
 *
 * Return: 0 for success, non zero for failure
 */
static int wlan_get_scanid(struct wlan_objmgr_pdev *pdev,
			       uint32_t *scan_id, uint64_t cookie)
{
	struct scan_req *scan_req;
	qdf_list_node_t *node = NULL;
	qdf_list_node_t *ptr_node = NULL;
	int ret = -EINVAL;
	struct pdev_osif_priv *osif_ctx;
	struct osif_scan_pdev *scan_priv;

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	if (!osif_ctx) {
		osif_err("Failed to retrieve osif context");
		return ret;
	}
	scan_priv = osif_ctx->osif_scan;
	qdf_mutex_acquire(&scan_priv->scan_req_q_lock);
	if (qdf_list_empty(&scan_priv->scan_req_q)) {
		qdf_mutex_release(&scan_priv->scan_req_q_lock);
		osif_err("Failed to retrieve scan id");
		return ret;
	}

	if (QDF_STATUS_SUCCESS !=
			    qdf_list_peek_front(&scan_priv->scan_req_q,
			    &ptr_node)) {
		qdf_mutex_release(&scan_priv->scan_req_q_lock);
		return ret;
	}

	do {
		node = ptr_node;
		scan_req = qdf_container_of(node, struct scan_req, node);
		if (cookie ==
		    (uintptr_t)(scan_req->scan_request)) {
			*scan_id = scan_req->scan_id;
			ret = 0;
			break;
		}
	} while (QDF_STATUS_SUCCESS ==
		 qdf_list_peek_next(&scan_priv->scan_req_q,
		 node, &ptr_node));

	qdf_mutex_release(&scan_priv->scan_req_q_lock);

	return ret;
}

QDF_STATUS wlan_abort_scan(struct wlan_objmgr_pdev *pdev,
				   uint32_t pdev_id, uint32_t vdev_id,
				   wlan_scan_id scan_id, bool sync)
{
	struct scan_cancel_request *req;
	struct pdev_osif_priv *osif_ctx;
	struct osif_scan_pdev *scan_priv;
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	if (!osif_ctx) {
		osif_err("Failed to retrieve osif context");
		qdf_mem_free(req);
		return QDF_STATUS_E_FAILURE;
	}
	if (vdev_id == INVAL_VDEV_ID)
		vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_OSIF_ID);
	else
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev,
				vdev_id, WLAN_OSIF_ID);

	if (!vdev) {
		osif_err("Failed get vdev");
		qdf_mem_free(req);
		return QDF_STATUS_E_INVAL;
	}
	scan_priv = osif_ctx->osif_scan;
	req->cancel_req.requester = scan_priv->req_id;
	req->vdev = vdev;
	req->cancel_req.scan_id = scan_id;
	req->cancel_req.pdev_id = pdev_id;
	req->cancel_req.vdev_id = vdev_id;
	if (scan_id != INVAL_SCAN_ID)
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_SINGLE;
	else if (vdev_id == INVAL_VDEV_ID)
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_PDEV_ALL;
	else
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	if (sync)
		status = ucfg_scan_cancel_sync(req);
	else
		status = ucfg_scan_cancel(req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Cancel scan request failed");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);

	return status;
}

qdf_export_symbol(wlan_abort_scan);

int wlan_cfg80211_abort_scan(struct wlan_objmgr_pdev *pdev)
{
	uint8_t pdev_id;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (ucfg_scan_get_pdev_status(pdev) !=
	   SCAN_NOT_IN_PROGRESS)
		wlan_abort_scan(pdev, pdev_id,
			INVAL_VDEV_ID, INVAL_SCAN_ID, true);

	return 0;
}

int wlan_vendor_abort_scan(struct wlan_objmgr_pdev *pdev,
			const void *data, int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1];
	int ret = -EINVAL;
	wlan_scan_id scan_id;
	uint64_t cookie;
	uint8_t pdev_id;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SCAN_MAX, data,
				    data_len, scan_policy)) {
		osif_err("Invalid ATTR");
		return ret;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE]) {
		cookie = nla_get_u64(
			    tb[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE]);
		ret = wlan_get_scanid(pdev, &scan_id, cookie);
		if (ret != 0)
			return ret;
		if (ucfg_scan_get_pdev_status(pdev) !=
		   SCAN_NOT_IN_PROGRESS)
			wlan_abort_scan(pdev, INVAL_PDEV_ID,
					INVAL_VDEV_ID, scan_id, true);
	}
	return 0;
}

static inline struct ieee80211_channel *
wlan_get_ieee80211_channel(struct wiphy *wiphy,
		struct wlan_objmgr_pdev *pdev,
		int chan_freq)
{
	struct ieee80211_channel *chan;

	chan = ieee80211_get_channel(wiphy, chan_freq);
	if (!chan)
		osif_err("chan is NULL, freq: %d", chan_freq);

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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) || \
	defined(CFG80211_INFORM_BSS_FRAME_DATA)
/**
 * wlan_fill_per_chain_rssi() - fill per chain RSSI in inform bss
 * @data: bss data
 * @per_chain_snr: per chain RSSI
 *
 * Return: void
 */
#if defined(CFG80211_SCAN_PER_CHAIN_RSSI_SUPPORT) || \
	   (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
static void wlan_fill_per_chain_rssi(struct cfg80211_inform_bss *data,
	struct wlan_cfg80211_inform_bss *bss)
{

	uint32_t i;

	if (!bss || !data) {
		osif_err("Received bss is NULL");
		return;
	}
	for (i = 0; i < WLAN_MGMT_TXRX_HOST_MAX_ANTENNA; i++) {
		if (!bss->per_chain_rssi[i] ||
		    (bss->per_chain_rssi[i] == WLAN_INVALID_PER_CHAIN_RSSI))
			continue;
		data->chain_signal[i] = bss->per_chain_rssi[i];
		data->chains |= BIT(i);
	}
}
#else
static inline void
wlan_fill_per_chain_rssi(struct cfg80211_inform_bss *data,
	struct wlan_cfg80211_inform_bss *bss)
{
}
#endif

struct cfg80211_bss *
wlan_cfg80211_inform_bss_frame_data(struct wiphy *wiphy,
		struct wlan_cfg80211_inform_bss *bss)
{
	struct cfg80211_inform_bss data  = {0};

	if (!bss) {
		osif_err("bss is null");
		return NULL;
	}
	wlan_fill_per_chain_rssi(&data, bss);

	data.chan = bss->chan;
	data.boottime_ns = bss->boottime_ns;
	data.signal = bss->rssi;
	return cfg80211_inform_bss_frame_data(wiphy, &data, bss->mgmt,
					      bss->frame_len, GFP_ATOMIC);
}
#else
struct cfg80211_bss *
wlan_cfg80211_inform_bss_frame_data(struct wiphy *wiphy,
		struct wlan_cfg80211_inform_bss *bss)

{
	return cfg80211_inform_bss_frame(wiphy, bss->chan, bss->mgmt,
					 bss->frame_len,
					 bss->rssi, GFP_ATOMIC);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
static inline void wlan_cfg80211_put_bss(struct wiphy *wiphy,
		struct cfg80211_bss *bss)
{
	cfg80211_put_bss(wiphy, bss);
}
#else
static inline void wlan_cfg80211_put_bss(struct wiphy *wiphy,
		struct cfg80211_bss *bss)
{
	cfg80211_put_bss(bss);
}
#endif

void wlan_cfg80211_inform_bss_frame(struct wlan_objmgr_pdev *pdev,
		struct scan_cache_entry *scan_params)
{
	struct pdev_osif_priv *pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	struct wiphy *wiphy;
	struct cfg80211_bss *bss = NULL;
	struct wlan_cfg80211_inform_bss bss_data = {0};

	if (!pdev_ospriv) {
		osif_err("os_priv is NULL");
		return;
	}

	wiphy = pdev_ospriv->wiphy;

	bss_data.frame_len = wlan_get_frame_len(scan_params);
	bss_data.mgmt = qdf_mem_malloc_atomic(bss_data.frame_len);
	if (!bss_data.mgmt) {
		osif_err("mem alloc failed for bss %pM seq %d",
			 bss_data.mgmt->bssid, scan_params->seq_num);
		return;
	}
	qdf_mem_copy(bss_data.mgmt,
		 util_scan_entry_frame_ptr(scan_params),
		 util_scan_entry_frame_len(scan_params));
	/*
	 * Android does not want the timestamp from the frame.
	 * Instead it wants a monotonic increasing value
	 */
	bss_data.mgmt->u.probe_resp.timestamp = qdf_get_monotonic_boottime();
	wlan_add_age_ie((uint8_t *)bss_data.mgmt, scan_params);
	/*
	 * Based on .ini configuration, raw rssi can be reported for bss.
	 * Raw rssi is typically used for estimating power.
	 */
	bss_data.rssi = scan_params->rssi_raw;

	bss_data.chan = wlan_get_ieee80211_channel(wiphy, pdev,
		scan_params->channel.chan_freq);
	if (!bss_data.chan) {
		osif_err("Channel not found for bss %pM seq %d chan_freq %d",
			 bss_data.mgmt->bssid, scan_params->seq_num,
			 scan_params->channel.chan_freq);
		qdf_mem_free(bss_data.mgmt);
		return;
	}

	/*
	 * Supplicant takes the signal strength in terms of
	 * mBm (1 dBm = 100 mBm).
	 */
	bss_data.rssi = QDF_MIN(bss_data.rssi, 0) * 100;

	bss_data.boottime_ns = scan_params->boottime_ns;

	qdf_mem_copy(bss_data.per_chain_rssi, scan_params->per_chain_rssi,
		     WLAN_MGMT_TXRX_HOST_MAX_ANTENNA);

	bss = wlan_cfg80211_inform_bss_frame_data(wiphy, &bss_data);
	if (!bss)
		osif_err("failed to inform bss %pM seq %d",
			 bss_data.mgmt->bssid, scan_params->seq_num);
	else
		wlan_cfg80211_put_bss(wiphy, bss);

	qdf_mem_free(bss_data.mgmt);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)) && \
	!defined(WITH_BACKPORTS) && !defined(IEEE80211_PRIVACY)
struct cfg80211_bss *wlan_cfg80211_get_bss(struct wiphy *wiphy,
					   struct ieee80211_channel *channel,
					   const u8 *bssid, const u8 *ssid,
					   size_t ssid_len)
{
	return cfg80211_get_bss(wiphy, channel, bssid,
				ssid, ssid_len,
				WLAN_CAPABILITY_ESS,
				WLAN_CAPABILITY_ESS);
}
#else
struct cfg80211_bss *wlan_cfg80211_get_bss(struct wiphy *wiphy,
					   struct ieee80211_channel *channel,
					   const u8 *bssid, const u8 *ssid,
					   size_t ssid_len)
{
	return cfg80211_get_bss(wiphy, channel, bssid,
				ssid, ssid_len,
				IEEE80211_BSS_TYPE_ESS,
				IEEE80211_PRIVACY_ANY);
}
#endif

void __wlan_cfg80211_unlink_bss_list(struct wiphy *wiphy, uint8_t *bssid,
				     uint8_t *ssid, uint8_t ssid_len)
{
	struct cfg80211_bss *bss = NULL;

	bss = wlan_cfg80211_get_bss(wiphy, NULL, bssid,
				    ssid, ssid_len);
	if (!bss) {
		osif_info("BSS %pM not found", bssid);
	} else {
		osif_debug("unlink entry for ssid:%.*s and BSSID %pM",
			   ssid_len, ssid, bssid);
		cfg80211_unlink_bss(wiphy, bss);
		wlan_cfg80211_put_bss(wiphy, bss);
	}

	/*
	 * Kernel creates separate entries into it's bss list for probe resp
	 * and beacon for hidden AP. Both have separate ref count and thus
	 * deleting one will not delete other entry.
	 * If beacon entry of the hidden AP is not deleted and AP switch to
	 * broadcasting SSID from Hiding SSID, kernel will reject the beacon
	 * entry. So unlink the hidden beacon entry (if present) as well from
	 * kernel, to avoid such issue.
	 */
	bss = wlan_cfg80211_get_bss(wiphy, NULL, bssid, NULL, 0);
	if (!bss) {
		osif_debug("Hidden bss not found for Ssid:%.*s BSSID: %pM sid_len %d",
			   ssid_len, ssid, bssid, ssid_len);
	} else {
		osif_debug("unlink entry for Hidden ssid:%.*s and BSSID %pM",
			   ssid_len, ssid, bssid);

		cfg80211_unlink_bss(wiphy, bss);
		/* cfg80211_get_bss get bss with ref count so release it */
		wlan_cfg80211_put_bss(wiphy, bss);
	}
}
void wlan_cfg80211_unlink_bss_list(struct wlan_objmgr_pdev *pdev,
				   struct scan_cache_entry *scan_entry)
{
	struct pdev_osif_priv *pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	struct wiphy *wiphy;

	if (!pdev_ospriv) {
		osif_err("os_priv is NULL");
		return;
	}

	wiphy = pdev_ospriv->wiphy;

	__wlan_cfg80211_unlink_bss_list(wiphy, scan_entry->bssid.bytes,
					scan_entry->ssid.ssid,
					scan_entry->ssid.length);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
/*
 * wlan_scan_wiphy_set_max_sched_scans() - set maximum number of scheduled scans
 * to wiphy.
 * @wiphy: pointer to wiphy
 * @max_scans: max num scans to be configured
 *
 */
static inline void
wlan_scan_wiphy_set_max_sched_scans(struct wiphy *wiphy, uint8_t max_scans)
{
	if (max_scans == 0)
		wiphy->flags &= ~WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
	else
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
}
#else
static inline void
wlan_scan_wiphy_set_max_sched_scans(struct wiphy *wiphy, uint8_t max_scans)
{
	wiphy->max_sched_scan_reqs = max_scans;
}
#endif /* KERNEL_VERSION(4, 12, 0) */

#if defined(CFG80211_REPORT_BETTER_BSS_IN_SCHED_SCAN) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
void wlan_scan_cfg80211_add_connected_pno_support(struct wiphy *wiphy)
{
	wiphy_ext_feature_set(wiphy,
			      NL80211_EXT_FEATURE_SCHED_SCAN_RELATIVE_RSSI);
}
#endif

#if ((LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)) || \
		defined(CFG80211_MULTI_SCAN_PLAN_BACKPORT)) && \
		defined(FEATURE_WLAN_SCAN_PNO)
void wlan_config_sched_scan_plans_to_wiphy(struct wiphy *wiphy,
					   struct wlan_objmgr_psoc *psoc)
{
	if (ucfg_scan_get_pno_scan_support(psoc)) {
		wlan_scan_wiphy_set_max_sched_scans(wiphy, 1);
		wiphy->max_sched_scan_ssids = SCAN_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_match_sets = SCAN_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_sched_scan_ie_len = SCAN_MAX_IE_LENGTH;
		wiphy->max_sched_scan_plans = SCAN_PNO_MAX_PLAN_REQUEST;

		wiphy->max_sched_scan_plan_interval =
			ucfg_scan_get_max_sched_scan_plan_interval(psoc);

		wiphy->max_sched_scan_plan_iterations =
			ucfg_scan_get_max_sched_scan_plan_iterations(psoc);
	}
}
#endif
