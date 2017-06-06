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

#include <qdf_list.h>
#include <qdf_status.h>
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
#ifdef WLAN_POLICY_MGR_ENABLE
#include <wlan_policy_mgr_api.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static uint32_t hdd_config_sched_scan_start_delay(
		struct cfg80211_sched_scan_request *request)
{
	return request->delay;
}
#else
static uint32_t hdd_config_sched_scan_start_delay(
		struct cfg80211_sched_scan_request *request)
{
	return 0;
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
static void wlan_config_sched_scan_plan(struct pno_scan_req_params *pno_req,
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
		cfg80211_err("Invalid number of scan plans %d !!",
			request->n_scan_plans);
	}
}
#else
static void wlan_config_sched_scan_plan(struct pno_scan_req_params *pno_req,
	struct cfg80211_sched_scan_request *request)
{
	pno_req->fast_scan_period = request->interval;
	pno_req->fast_scan_max_cycles = SCAN_PNO_DEF_SCAN_TIMER_REPEAT;
	pno_req->slow_scan_period =
		SCAN_PNO_DEF_SLOW_SCAN_MULTIPLIER *
		pno_req->fast_scan_period;
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

	cfg80211_info("vdev id = %d", event->vdev_id);

	wlan_vdev_obj_lock(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	wlan_vdev_obj_unlock(vdev);
	if (!pdev) {
		cfg80211_err("pdev is NULL");
		return;
	}

	wlan_pdev_obj_lock(pdev);
	pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_obj_unlock(pdev);
	if (!pdev_ospriv) {
		cfg80211_err("pdev_osprivis NULL");
		return;
	}
	cfg80211_sched_scan_results(pdev_ospriv->wiphy);
}

/**
 * wlan_cfg80211_is_pno_allowed() -  Check if PNO is allowed
 * @vdev: vdev ptr
 *
 * The PNO Start request is coming from upper layers.
 * It is to be allowed only for Infra STA device type
 * and the link should be in a disconnected state.
 *
 * Return: Success if PNO is allowed, Failure otherwise.
 */
static QDF_STATUS wlan_cfg80211_is_pno_allowed(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum tQDF_ADAPTER_MODE vdev_opmode;
	uint8_t vdev_id;

	wlan_vdev_obj_lock(vdev);
	vdev_opmode = wlan_vdev_mlme_get_opmode(vdev);
	state = wlan_vdev_mlme_get_state(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);

	cfg80211_notice("dev_mode=%d, state=%d vdev id %d",
		vdev_opmode, state, vdev_id);

	if ((vdev_opmode == QDF_STA_MODE) &&
	   ((state == WLAN_VDEV_S_INIT) ||
	   (state == WLAN_VDEV_S_STOP)))
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_FAILURE;
}

int wlan_cfg80211_sched_scan_start(struct wlan_objmgr_pdev *pdev,
	struct net_device *dev,
	struct cfg80211_sched_scan_request *request)
{
	struct pno_scan_req_params *req;
	int i, j, ret = 0;
	QDF_STATUS status;
	uint8_t num_chan = 0, channel;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	uint32_t valid_ch[SCAN_PNO_MAX_NETW_CHANNELS_EX] = {0};

	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, dev->dev_addr,
		WLAN_OSIF_ID);
	if (!vdev) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	status = wlan_cfg80211_is_pno_allowed(vdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("pno is not allowed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		return -ENOTSUPP;
	}

	if (ucfg_scan_get_pno_in_progress(vdev)) {
		cfg80211_debug("pno is already in progress");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		return -EBUSY;
	}

	if (ucfg_scan_get_pdev_status(pdev) !=
	   SCAN_NOT_IN_PROGRESS) {
		status = wlan_abort_scan(pdev,
				wlan_objmgr_pdev_get_pdev_id(pdev),
				INVAL_VDEV_ID, INVAL_SCAN_ID, true);
		if (QDF_IS_STATUS_ERROR(status)) {
			cfg80211_err("aborting the existing scan is unsuccessful");
			wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
			return -EBUSY;
		}
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		cfg80211_err("req malloc failed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		return -ENOMEM;
	}

	req->networks_cnt = request->n_match_sets;
	wlan_vdev_obj_lock(vdev);
	req->vdev_id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);

	if ((!req->networks_cnt) ||
	    (req->networks_cnt > SCAN_PNO_MAX_SUPP_NETWORKS)) {
		cfg80211_err("Network input is not correct %d",
			req->networks_cnt);
		ret = -EINVAL;
		goto error;
	}

	if (request->n_channels > SCAN_PNO_MAX_NETW_CHANNELS_EX) {
		cfg80211_err("Incorrect number of channels %d",
			request->n_channels);
		ret = -EINVAL;
		goto error;
	}

	if (request->n_channels) {
		char chl[(request->n_channels * 5) + 1];
		int len = 0;

		for (i = 0; i < request->n_channels; i++) {
			channel = request->channels[i]->hw_value;
			if (wlan_is_dsrc_channel(wlan_chan_to_freq(channel)))
				continue;
			len += snprintf(chl + len, 5, "%d ", channel);
			valid_ch[num_chan++] = wlan_chan_to_freq(channel);
		}
		cfg80211_notice("No. of Scan Channels: %d", num_chan);
		cfg80211_notice("Channel-List: %s", chl);
		/* If all channels are DFS and dropped,
		 * then ignore the PNO request
		 */
		if (!num_chan) {
			cfg80211_notice("Channel list empty due to filtering of DSRC");
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
			cfg80211_err(" SSID Len %d is not correct for network %d",
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

		cfg80211_notice("Received ssid:%.*s",
			req->networks_list[i].ssid.length,
			req->networks_list[i].ssid.ssid);

		/*Copying list of valid channel into request */
		qdf_mem_copy(req->networks_list[i].channels, valid_ch,
			num_chan * sizeof(uint32_t));
		req->networks_list[i].channel_cnt = num_chan;
		req->networks_list[i].rssi_thresh =
			request->match_sets[i].rssi_thold;
	}

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
	cfg80211_notice("Number of hidden networks being Configured = %d",
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
	req->delay_start_time = hdd_config_sched_scan_start_delay(request);
	cfg80211_notice("Base scan interval: %d sec, scan cycles: %d, slow scan interval %d",
		req->fast_scan_period, req->fast_scan_max_cycles,
		req->slow_scan_period);

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);
	ucfg_scan_register_pno_cb(psoc,
		wlan_cfg80211_pno_callback, NULL);
	ucfg_scan_get_pno_def_params(vdev, req);
	status = ucfg_scan_pno_start(vdev, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Failed to enable PNO");
		ret = -EINVAL;
		goto error;
	}

	cfg80211_info("PNO scan request offloaded");

error:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	qdf_mem_free(req);
	return ret;
}

int wlan_cfg80211_sched_scan_stop(struct wlan_objmgr_pdev *pdev,
	struct net_device *dev)
{
	int ret = 0;
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, dev->dev_addr,
		WLAN_OSIF_ID);
	if (!vdev) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	status = ucfg_scan_pno_stop(vdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Failed to disabled PNO");
		ret = -EINVAL;
	} else {
		cfg80211_info("PNO scan disabled");
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	return ret;
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
	if (NULL == scan_req) {
		cfg80211_alert("malloc failed for Scan req");
		return -ENOMEM;
	}

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	osif_scan = osif_ctx->osif_scan;
	scan_req->scan_request = req;
	scan_req->source = source;
	scan_req->scan_id = scan_id;

	status = qdf_list_insert_back(&osif_scan->scan_req_q,
					&scan_req->node);

	if (QDF_STATUS_SUCCESS != status) {
		cfg80211_err("Failed to enqueue Scan Req");
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
 * @source : returns source of the scan request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_scan_request_dequeue(
	struct wlan_objmgr_pdev *pdev,
	uint32_t scan_id, struct cfg80211_scan_request **req, uint8_t *source)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct scan_req *scan_req;
	qdf_list_node_t *node = NULL, *next_node = NULL;
	struct pdev_osif_priv *osif_ctx;
	struct osif_scan_pdev *scan_priv;

	cfg80211_info("Dequeue Scan id: %d", scan_id);

	if ((source == NULL) || (req == NULL)) {
		cfg80211_err("source or request is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	if (!osif_ctx) {
		cfg80211_err("Failed to retrieve osif context");
		return status;
	}
	scan_priv = osif_ctx->osif_scan;

	if (qdf_list_empty(&scan_priv->scan_req_q)) {
		cfg80211_info("Scan List is empty");
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_STATUS_SUCCESS !=
		qdf_list_peek_front(&scan_priv->scan_req_q, &next_node)) {
		cfg80211_err("Failed to remove Scan Req from queue");
		return QDF_STATUS_E_FAILURE;
	}

	do {
		node = next_node;
		scan_req = qdf_container_of(node, struct scan_req,
					node);
		if (scan_req->scan_id == scan_id) {
			status = qdf_list_remove_node(&scan_priv->scan_req_q,
					node);
			if (status == QDF_STATUS_SUCCESS) {
				*req = scan_req->scan_request;
				*source = scan_req->source;
				qdf_mem_free(scan_req);
				cfg80211_info("removed Scan id: %d, req = %p, pending scans %d",
				      scan_id, req,
				      qdf_list_size(&scan_priv->scan_req_q));
				return QDF_STATUS_SUCCESS;
			} else {
				cfg80211_err("Failed to remove node scan id %d, pending scans %d",
				      scan_id,
				      qdf_list_size(&scan_priv->scan_req_q));
				return status;
			}
		}
	} while (QDF_STATUS_SUCCESS ==
		qdf_list_peek_next(&scan_priv->scan_req_q, node, &next_node));

	cfg80211_err("Failed to find scan id %d", scan_id);

	return status;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
/**
 * wlan_cfg80211_scan_done() - Scan completed callback to cfg80211
 *
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void wlan_cfg80211_scan_done(struct cfg80211_scan_request *req,
				   bool aborted)
{
	struct cfg80211_scan_info info = {
		.aborted = aborted
	};

	if (req->wdev->netdev->flags & IFF_UP)
		cfg80211_scan_done(req, &info);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
/**
 * wlan_cfg80211_scan_done() - Scan completed callback to cfg80211
 *
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void wlan_cfg80211_scan_done(struct cfg80211_scan_request *req,
				   bool aborted)
{
	if (req->wdev->netdev->flags & IFF_UP)
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
			GFP_KERNEL);

	if (!skb) {
		cfg80211_err("skb alloc failed");
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

	cfg80211_vendor_event(skb, GFP_KERNEL);
	qdf_mem_free(req);

	return;

nla_put_failure:
	kfree_skb(skb);
	qdf_mem_free(req);
}


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
	bool aborted = false;
	uint32_t scan_id = event->scan_id;
	uint8_t source = NL_SCAN;
	struct wlan_objmgr_pdev *pdev;
	struct pdev_osif_priv *osif_priv;
	QDF_STATUS status;

	if ((event->type != SCAN_EVENT_TYPE_COMPLETED) &&
	    (event->type != SCAN_EVENT_TYPE_DEQUEUED) &&
	    (event->type != SCAN_EVENT_TYPE_START_FAILED))
		return;

	cfg80211_info("scan ID = %d vdev id = %d, event type %s(%d) reason = %s(%d)",
		scan_id, event->vdev_id,
		util_scan_get_ev_type_name(event->type),
		event->type,
		util_scan_get_ev_reason_name(event->reason),
		event->reason);

	/*
	 * cfg80211_scan_done informing NL80211 about completion
	 * of scanning
	 */
	if ((event->type == SCAN_EVENT_TYPE_COMPLETED) &&
	    ((event->reason == SCAN_REASON_CANCELLED) ||
	     (event->reason == SCAN_REASON_TIMEDOUT) ||
	     (event->reason == SCAN_REASON_INTERNAL_FAILURE))) {
		aborted = true;
	} else if ((event->type == SCAN_EVENT_TYPE_COMPLETED) &&
		   (event->reason == SCAN_REASON_COMPLETED))
		aborted = false;
	else if ((event->type == SCAN_EVENT_TYPE_DEQUEUED) &&
		 (event->reason == SCAN_REASON_CANCELLED))
		aborted = true;
	else if ((event->type == SCAN_EVENT_TYPE_START_FAILED) &&
		 (event->reason == SCAN_REASON_COMPLETED))
		aborted = true;
	else
		/* cfg80211 is not interested on all other scan events */
		return;

	pdev = wlan_vdev_get_pdev(vdev);
	status = wlan_scan_request_dequeue(pdev, scan_id, &req, &source);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Dequeue of scan request failed ID: %d", scan_id);
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
		wlan_cfg80211_scan_done(req, aborted);
	else
		wlan_vendor_scan_callback(req, aborted);

allow_suspend:
	osif_priv = wlan_pdev_get_ospriv(pdev);
	if (qdf_list_empty(&osif_priv->osif_scan->scan_req_q))
		qdf_runtime_pm_allow_suspend(
			osif_priv->osif_scan->runtime_pm_lock);

}

QDF_STATUS wlan_cfg80211_scan_priv_init(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;
	struct wlan_objmgr_psoc *psoc;
	wlan_scan_requester req_id;

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);

	req_id = ucfg_scan_register_requester(psoc, "CFG",
		wlan_cfg80211_scan_done_callback, NULL);

	osif_priv = wlan_pdev_get_ospriv(pdev);
	scan_priv = qdf_mem_malloc(sizeof(*scan_priv));
	if (!scan_priv) {
		cfg80211_err("failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}
	/* Initialize the scan request queue */
	osif_priv->osif_scan = scan_priv;
	qdf_list_create(&scan_priv->scan_req_q, WLAN_MAX_SCAN_COUNT);
	scan_priv->req_id = req_id;
	scan_priv->runtime_pm_lock = qdf_runtime_lock_init("scan");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cfg80211_scan_priv_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;
	struct wlan_objmgr_psoc *psoc;

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	osif_priv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_obj_unlock(pdev);

	scan_priv = osif_priv->osif_scan;
	osif_priv->osif_scan = NULL;
	ucfg_scan_unregister_requester(psoc, scan_priv->req_id);
	qdf_list_destroy(&scan_priv->scan_req_q);
	qdf_runtime_lock_deinit(scan_priv->runtime_pm_lock);
	scan_priv->runtime_pm_lock = NULL;
	qdf_mem_free(scan_priv);

	return QDF_STATUS_SUCCESS;
}

void wlan_cfg80211_cleanup_scan_queue(struct wlan_objmgr_pdev *pdev)
{
	struct scan_req *scan_req;
	qdf_list_node_t *node = NULL;
	struct cfg80211_scan_request *req;
	uint8_t source;
	bool aborted = true;
	struct pdev_osif_priv *osif_priv;
	struct osif_scan_pdev *scan_priv;

	if (!pdev) {
		cfg80211_err("pdev is Null");
		return;
	}

	wlan_pdev_obj_lock(pdev);
	osif_priv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_obj_unlock(pdev);

	scan_priv = osif_priv->osif_scan;

	while (!qdf_list_empty(&scan_priv->scan_req_q)) {
		if (QDF_STATUS_SUCCESS !=
			qdf_list_remove_front(&scan_priv->scan_req_q,
						&node)) {
			cfg80211_err("Failed to remove scan request");
			return;
		}

		scan_req = container_of(node, struct scan_req, node);
		req = scan_req->scan_request;
		source = scan_req->source;
		if (NL_SCAN == source)
			wlan_cfg80211_scan_done(req, aborted);
		else
			wlan_vendor_scan_callback(req, aborted);
		qdf_mem_free(scan_req);
	}

	return;
}

/**
 * wlan_cfg80211_scan() - Process scan request
 * @pdev: pdev object pointer
 * @request: scan request
 * @source : returns source of the scan request
 *
 * Return: 0 on success, error number otherwise
 */
int wlan_cfg80211_scan(struct wlan_objmgr_pdev *pdev,
		struct cfg80211_scan_request *request,
		uint8_t source)
{
	struct net_device *dev = request->wdev->netdev;
	struct scan_start_request *req;
	struct wlan_ssid *pssid;
	uint8_t i;
	int status;
	uint8_t num_chan = 0, channel;
	struct wlan_objmgr_vdev *vdev;
	wlan_scan_requester req_id;
	struct pdev_osif_priv *osif_priv;
	struct wlan_objmgr_psoc *psoc;
	wlan_scan_id scan_id;
	bool is_p2p_scan = false;

	/* Get the vdev object */
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, dev->dev_addr,
		WLAN_OSIF_ID);
	if (vdev == NULL) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		cfg80211_err("Invalid psoc object");
		return -EINVAL;
	}
	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		cfg80211_err("Failed to allocate scan request memory");
		return -EINVAL;
	}
	/* Initialize the scan global params */
	ucfg_scan_init_default_params(vdev, req);

	/* Get NL global context from objmgr*/
	osif_priv = wlan_pdev_get_ospriv(pdev);
	req_id = osif_priv->osif_scan->req_id;
	scan_id = ucfg_scan_get_scan_id(psoc);
	if (!scan_id) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		cfg80211_err("Invalid scan id");
		qdf_mem_free(req);
		return -EINVAL;
	}
	/* fill the scan request structure */
	req->vdev = vdev;
	req->scan_req.vdev_id = wlan_vdev_get_id(vdev);
	req->scan_req.scan_id = scan_id;
	req->scan_req.scan_req_id = req_id;
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

		/* copy all the ssid's and their length */
		for (j = 0; j < request->n_ssids; j++)  {
			pssid = &req->scan_req.ssid[j];
			/* get the ssid length */
			pssid->length = request->ssids[j].ssid_len;
			qdf_mem_copy(pssid->ssid,
				     &request->ssids[j].ssid[0],
				     pssid->length);
			pssid->ssid[pssid->length] = '\0';
			cfg80211_notice("SSID number %d: %s", j,
				    pssid->ssid);
		}
	}
	if (request->ssids ||
	   (wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_GO_MODE))
		req->scan_req.scan_f_passive = false;

	if ((request->n_ssids == 1) && request->ssids &&
	   !qdf_mem_cmp(&request->ssids[0], "DIRECT-", 7))
		is_p2p_scan = true;

	if (is_p2p_scan && request->no_cck) {
		req->scan_req.adaptive_dwell_time_mode =
			SCAN_DWELL_MODE_STATIC;
		req->scan_req.dwell_time_active +=
			P2P_SEARCH_DWELL_TIME_INC;
		req->scan_req.repeat_probe_time =
			req->scan_req.dwell_time_active / 5;
		req->scan_req.burst_duration =
			BURST_SCAN_MAX_NUM_OFFCHANNELS *
			req->scan_req.dwell_time_active;
		if (req->scan_req.burst_duration >
		    P2P_SCAN_MAX_BURST_DURATION) {
			uint8_t channels =
				P2P_SCAN_MAX_BURST_DURATION /
				req->scan_req.dwell_time_active;
			if (channels)
				req->scan_req.burst_duration =
					channels *
					req->scan_req.dwell_time_active;
			else
				req->scan_req.burst_duration =
					P2P_SCAN_MAX_BURST_DURATION;
		}
		req->scan_req.scan_ev_bss_chan = false;
	} else {
		req->scan_req.scan_f_cck_rates = true;
		if (!req->scan_req.num_ssids)
			req->scan_req.scan_f_bcast_probe = true;
		req->scan_req.scan_f_add_tpc_ie_in_probe = true;
	}
	req->scan_req.scan_f_add_ds_ie_in_probe = true;
	req->scan_req.scan_f_filter_prb_req = true;

	req->scan_req.n_probes = (req->scan_req.repeat_probe_time > 0) ?
		(req->scan_req.dwell_time_active /
		req->scan_req.repeat_probe_time) : 0;

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
		char chl[(request->n_channels * 5) + 1];
		int len = 0;
#ifdef WLAN_POLICY_MGR_ENABLE
		bool ap_or_go_present =
			policy_mgr_mode_specific_connection_count(
			     psoc, QDF_SAP_MODE, NULL) ||
			     policy_mgr_mode_specific_connection_count(
			     psoc, QDF_P2P_GO_MODE, NULL);
#endif

		for (i = 0; i < request->n_channels; i++) {
			channel = request->channels[i]->hw_value;
			if (wlan_is_dsrc_channel(wlan_chan_to_freq(channel)))
				continue;
#ifdef WLAN_POLICY_MGR_ENABLE
			if (ap_or_go_present) {
				bool ok;
				int ret;

				ret = policy_mgr_is_chan_ok_for_dnbs(psoc,
								channel,
								&ok);

				if (QDF_IS_STATUS_ERROR(ret)) {
					cfg80211_err("DNBS check failed");
					qdf_mem_free(req);
					status = -EINVAL;
					goto end;
				}
				if (!ok)
					continue;
			}
#endif

			len += snprintf(chl + len, 5, "%d ", channel);
			req->scan_req.chan_list[num_chan] =
				wlan_chan_to_freq(channel);
			num_chan++;
		}
		cfg80211_notice("Channel-List: %s", chl);
		cfg80211_notice("No. of Scan Channels: %d", num_chan);
	}
	if (!num_chan) {
		cfg80211_err("Received zero non-dsrc channels");
		qdf_mem_free(req);
		status = -EINVAL;
		goto end;
	}
	req->scan_req.num_chan = num_chan;

	/* P2P increase the scan priority */
	if (is_p2p_scan)
		req->scan_req.scan_priority = SCAN_PRIORITY_HIGH;
	if (request->ie_len) {
		req->scan_req.extraie.ptr = qdf_mem_malloc(request->ie_len);
		if (!req->scan_req.extraie.ptr) {
			cfg80211_err("Failed to allocate memory");
			status = -ENOMEM;
			qdf_mem_free(req);
			goto end;
		}
		req->scan_req.extraie.len = request->ie_len;
		qdf_mem_copy(req->scan_req.extraie.ptr, request->ie,
				request->ie_len);
	}

	if (request->flags & NL80211_SCAN_FLAG_FLUSH)
		ucfg_scan_flush_results(pdev, NULL);

	/* Enqueue the scan request */
	wlan_scan_request_enqueue(pdev, request, source, req->scan_req.scan_id);

	qdf_runtime_pm_prevent_suspend(
		osif_priv->osif_scan->runtime_pm_lock);

	status = ucfg_scan_start(req);
	if (QDF_STATUS_SUCCESS != status) {
		cfg80211_err("ucfg_scan_start returned error %d", status);
		if (QDF_STATUS_E_RESOURCES == status) {
			cfg80211_err("HO is in progress.So defer the scan by informing busy");
			status = -EBUSY;
		} else {
			status = -EIO;
		}
		wlan_scan_request_dequeue(pdev, scan_id, &request, &source);
		if (qdf_list_empty(&osif_priv->osif_scan->scan_req_q))
			qdf_runtime_pm_allow_suspend(
				osif_priv->osif_scan->runtime_pm_lock);
	}

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	return status;
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
		cfg80211_err("Failed to retrieve osif context");
		return ret;
	}
	scan_priv = osif_ctx->osif_scan;
	if (qdf_list_empty(&scan_priv->scan_req_q)) {
		cfg80211_err("Failed to retrieve scan id");
		return ret;
	}

	if (QDF_STATUS_SUCCESS !=
			    qdf_list_peek_front(&scan_priv->scan_req_q,
			    &ptr_node)) {
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
	if (!req) {
		cfg80211_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	/* Get NL global context from objmgr*/
	osif_ctx = wlan_pdev_get_ospriv(pdev);
	if (!osif_ctx) {
		cfg80211_err("Failed to retrieve osif context");
		qdf_mem_free(req);
		return QDF_STATUS_E_FAILURE;
	}
	if (vdev_id == INVAL_VDEV_ID)
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev,
				0, WLAN_OSIF_ID);
	else
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev,
				vdev_id, WLAN_OSIF_ID);

	if (!vdev) {
		cfg80211_err("Failed get vdev");
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
	if (vdev_id == INVAL_VDEV_ID)
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_PDEV_ALL;
	else
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	if (sync)
		status = ucfg_scan_cancel_sync(req);
	else
		status = ucfg_scan_cancel(req);
	if (QDF_IS_STATUS_ERROR(status))
		cfg80211_err("Cancel scan request failed");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);

	return status;
}

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
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SCAN_MAX, data,
	    data_len, NULL)) {
		cfg80211_err("Invalid ATTR");
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
			wlan_abort_scan(pdev, pdev_id,
					INVAL_VDEV_ID, scan_id, true);
	}
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
