/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_scan.c
 *
 * WLAN Host Device Driver scan implementation
 */

#include <linux/wireless.h>
#include <net/cfg80211.h>

#include "wlan_hdd_includes.h"
#include "cds_api.h"
#include "cds_api.h"
#include "ani_global.h"
#include "dot11f.h"
#include "cds_sched.h"
#include "wlan_hdd_p2p.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_scan.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_power.h"
#include "wma_api.h"
#include "cds_utils.h"
#include "wlan_p2p_ucfg_api.h"

#ifdef WLAN_UMAC_CONVERGENCE
#include "wlan_cfg80211.h"
#endif
#include <qca_vendor.h>
#include <wlan_cfg80211_scan.h>

#include "wlan_utility.h"

#define MAX_RATES                       12
#define HDD_WAKE_LOCK_SCAN_DURATION (5 * 1000) /* in msec */

#define SCAN_DONE_EVENT_BUF_SIZE 4096
#define RATE_MASK 0x7f

/*
 * Count to ratelimit the HDD logs during Scan and connect
 */
#define HDD_SCAN_REJECT_RATE_LIMIT 5

/**
 * enum essid_bcast_type - SSID broadcast type
 * @eBCAST_UNKNOWN: Broadcast unknown
 * @eBCAST_NORMAL: Broadcast normal
 * @eBCAST_HIDDEN: Broadcast hidden
 */
enum essid_bcast_type {
	eBCAST_UNKNOWN = 0,
	eBCAST_NORMAL = 1,
	eBCAST_HIDDEN = 2,
};

/**
 * hdd_vendor_scan_callback() - Scan completed callback event
 * @hddctx: HDD context
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function sends scan completed callback event to NL.
 *
 * Return: none
 */
static void hdd_vendor_scan_callback(struct hdd_adapter *adapter,
					struct cfg80211_scan_request *req,
					bool aborted)
{
	struct hdd_context *hddctx = WLAN_HDD_GET_CTX(adapter);
	struct sk_buff *skb;
	struct nlattr *attr;
	int i;
	uint8_t scan_status;
	uint64_t cookie;

	ENTER();

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("Invalid adapter magic");
		qdf_mem_free(req);
		return;
	}
	skb = cfg80211_vendor_event_alloc(hddctx->wiphy, &(adapter->wdev),
			SCAN_DONE_EVENT_BUF_SIZE + 4 + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
			GFP_KERNEL);

	if (!skb) {
		hdd_err("skb alloc failed");
		qdf_mem_free(req);
		return;
	}

	cookie = (uintptr_t)req;
	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_ssids; i++) {
		if (nla_put(skb, i, req->ssids[i].ssid_len,
			req->ssids[i].ssid)) {
			hdd_err("Failed to add ssid");
			goto nla_put_failure;
		}
	}
	nla_nest_end(skb, attr);
	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_channels; i++) {
		if (nla_put_u32(skb, i, req->channels[i]->center_freq)) {
			hdd_err("Failed to add channel");
			goto nla_put_failure;
		}
	}
	nla_nest_end(skb, attr);

	if (req->ie &&
		nla_put(skb, QCA_WLAN_VENDOR_ATTR_SCAN_IE, req->ie_len,
			req->ie)) {
		hdd_err("Failed to add scan ie");
		goto nla_put_failure;
	}
	if (req->flags &&
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS, req->flags)) {
		hdd_err("Failed to add scan flags");
		goto nla_put_failure;
	}
	if (hdd_wlan_nla_put_u64(skb,
				  QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				  cookie)) {
		hdd_err("Failed to add scan cookie");
		goto nla_put_failure;
	}
	scan_status = (aborted == true) ? VENDOR_SCAN_STATUS_ABORTED :
		VENDOR_SCAN_STATUS_NEW_RESULTS;
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_SCAN_STATUS, scan_status)) {
		hdd_err("Failed to add scan staus");
		goto nla_put_failure;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	hdd_info("scan complete event sent to NL");
	qdf_mem_free(req);
	return;

nla_put_failure:
	kfree_skb(skb);
	qdf_mem_free(req);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(struct hdd_adapter *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	struct cfg80211_scan_info info = {
		.aborted = aborted
	};

	if (adapter->dev->flags & IFF_UP)
		cfg80211_scan_done(req, &info);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(struct hdd_adapter *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	if (adapter->dev->flags & IFF_UP)
		cfg80211_scan_done(req, aborted);
}
#else
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(struct hdd_adapter *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	cfg80211_scan_done(req, aborted);
}
#endif

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
/**
 * wlan_hdd_sap_skip_scan_check() - The function will check OBSS
 *         scan skip or not for SAP.
 * @hdd_ctx: pointer to hdd context.
 * @request: pointer to scan request.
 *
 * This function will check the scan request's chan list against the
 * previous ACS scan chan list. If all the chan are covered by
 * previous ACS scan, we can skip the scan and return scan complete
 * to save the SAP starting time.
 *
 * Return: true to skip the scan,
 *            false to continue the scan
 */
static bool wlan_hdd_sap_skip_scan_check(struct hdd_context *hdd_ctx,
	struct cfg80211_scan_request *request)
{
	int i, j;
	bool skip;

	hdd_debug("HDD_ACS_SKIP_STATUS = %d",
		hdd_ctx->skip_acs_scan_status);
	if (hdd_ctx->skip_acs_scan_status != eSAP_SKIP_ACS_SCAN)
		return false;
	qdf_spin_lock(&hdd_ctx->acs_skip_lock);
	if (hdd_ctx->last_acs_channel_list == NULL ||
	   hdd_ctx->num_of_channels == 0 ||
	   request->n_channels == 0) {
		qdf_spin_unlock(&hdd_ctx->acs_skip_lock);
		return false;
	}
	skip = true;
	for (i = 0; i < request->n_channels ; i++) {
		bool find = false;

		for (j = 0; j < hdd_ctx->num_of_channels; j++) {
			if (hdd_ctx->last_acs_channel_list[j] ==
			   request->channels[i]->hw_value) {
				find = true;
				break;
			}
		}
		if (!find) {
			skip = false;
			hdd_debug("Chan %d isn't in ACS chan list",
				request->channels[i]->hw_value);
			break;
		}
	}
	qdf_spin_unlock(&hdd_ctx->acs_skip_lock);
	return skip;
}
#else
static bool wlan_hdd_sap_skip_scan_check(struct hdd_context *hdd_ctx,
	struct cfg80211_scan_request *request)
{
	return false;
}
#endif

static void __wlan_hdd_cfg80211_scan_block_cb(struct work_struct *work)
{
	struct hdd_adapter *adapter;
	struct cfg80211_scan_request *request;
	struct hdd_context *hdd_ctx;

	adapter = container_of(work, struct hdd_adapter, scan_block_work);
	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("HDD adapter context is invalid");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	request = adapter->request;
	if (request) {
		request->n_ssids = 0;
		request->n_channels = 0;

		hdd_err("##In DFS Master mode. Scan aborted. Null result sent");
		hdd_cfg80211_scan_done(adapter, request, true);
		adapter->request = NULL;
	}
	request = adapter->vendor_request;
	if (request) {
		request->n_ssids = 0;
		request->n_channels = 0;

		hdd_err("In DFS Master mode. Scan aborted. Null result sent");
		hdd_vendor_scan_callback(adapter, request, true);
		adapter->vendor_request = NULL;
	}
}

void wlan_hdd_cfg80211_scan_block_cb(struct work_struct *work)
{
	cds_ssr_protect(__func__);
	__wlan_hdd_cfg80211_scan_block_cb(work);
	cds_ssr_unprotect(__func__);
}

/*
 * wlan_hdd_update_scan_ies() - API to update the scan IEs of scan request
 * with already stored default scan IEs
 *
 * @adapter: Pointer to HDD adapter
 * @scan_info: Pointer to scan info in HDD adapter
 * @scan_ie: Pointer to scan IE in scan request
 * @scan_ie_len: Pointer to scan IE length in scan request
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_update_scan_ies(struct hdd_adapter *adapter,
			struct hdd_scan_info *scan_info, uint8_t *scan_ie,
			uint16_t *scan_ie_len)
{
	uint16_t rem_len = scan_info->default_scan_ies_len;
	uint8_t *temp_ie = scan_info->default_scan_ies;
	uint8_t *current_ie;
	uint8_t elem_id;
	uint16_t elem_len;
	bool add_ie = false;

	if (!scan_info->default_scan_ies_len || !scan_info->default_scan_ies)
		return 0;

	while (rem_len >= 2) {
		current_ie = temp_ie;
		elem_id = *temp_ie++;
		elem_len = *temp_ie++;
		rem_len -= 2;

		switch (elem_id) {
		case DOT11F_EID_EXTCAP:
			if (!wlan_get_ie_ptr_from_eid(DOT11F_EID_EXTCAP,
						      scan_ie, *scan_ie_len))
				add_ie = true;
			break;
		case IE_EID_VENDOR:
			if ((0 != qdf_mem_cmp(&temp_ie[0], MBO_OUI_TYPE,
							MBO_OUI_TYPE_SIZE)) ||
				(0 == qdf_mem_cmp(&temp_ie[0], QCN_OUI_TYPE,
							QCN_OUI_TYPE_SIZE)))
				add_ie = true;
			break;
		}

		if (add_ie && (((*scan_ie_len) + elem_len) >
					SIR_MAC_MAX_ADD_IE_LENGTH)){
			hdd_err("Not enough buffer to save default scan IE's");
			return 0;
		}

		if (add_ie) {
			qdf_mem_copy(scan_ie + (*scan_ie_len),
						current_ie, elem_len + 2);
			(*scan_ie_len) += (elem_len + 2);
			add_ie = false;
		}

		temp_ie += elem_len;
		rem_len -= elem_len;
	}
	return 0;
}

/* Define short name to use in cds_trigger_recovery */
#define SCAN_FAILURE QDF_SCAN_ATTEMPT_FAILURES

/**
 * __wlan_hdd_cfg80211_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @request: Pointer to scan request
 * @source: scan request source(NL/Vendor scan)
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
				    struct cfg80211_scan_request *request,
				    uint8_t source)
{
	struct net_device *dev = request->wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_config *cfg_param = NULL;
	int status;
	struct hdd_scan_info *scan_info = NULL;
	struct hdd_adapter *con_sap_adapter;
	uint16_t con_dfs_ch;
	struct hdd_wext_state *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	uint8_t curr_session_id;
	enum scan_reject_states curr_reason;
	static uint32_t scan_ebusy_cnt;
	struct scan_params params = {0};

	ENTER();

	if (cds_is_fw_down()) {
		hdd_err("firmware is down, scan cmd cannot be processed");
		return -EINVAL;
	}

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SCAN,
			 adapter->session_id, request->n_channels));

	if (!sme_is_session_id_valid(hdd_ctx->hHal, adapter->session_id))
		return -EINVAL;

	if ((eConnectionState_Associated ==
			WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
						conn_info.connState) &&
	    (!hdd_ctx->config->enable_connected_scan)) {
		hdd_info("enable_connected_scan is false, Aborting scan");
		if (NL_SCAN == source)
			adapter->request = request;
		else
			adapter->vendor_request = request;
		schedule_work(&adapter->scan_block_work);
		return 0;
	}

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	/*
	 * IBSS vdev does not need to scan to establish
	 * IBSS connection. If IBSS vdev need to support scan,
	 * Firmware need to make the change to add self peer
	 * per mac for IBSS vdev.
	 * NDI does not need scan from userspace to establish connection
	 * and it does not support scan request either.
	 */
	if (QDF_IBSS_MODE == adapter->device_mode ||
	    QDF_NDI_MODE == adapter->device_mode) {
		hdd_err("Scan not supported for %s",
			hdd_device_mode_to_string(adapter->device_mode));
		return -EINVAL;
	}

	cfg_param = hdd_ctx->config;
	scan_info = &adapter->scan_info;

	/* Block All Scan during DFS operation and send null scan result */
	con_sap_adapter = hdd_get_con_sap_adapter(adapter, true);
	if (con_sap_adapter) {
		con_dfs_ch = con_sap_adapter->session.ap.sap_config.channel;
		if (con_dfs_ch == AUTO_CHANNEL_SELECT)
			con_dfs_ch =
				con_sap_adapter->session.ap.operating_channel;

		if (!policy_mgr_is_hw_dbs_capable(hdd_ctx->hdd_psoc) &&
			wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, con_dfs_ch)) {
			/* Provide empty scan result during DFS operation since
			 * scanning not supported during DFS. Reason is
			 * following case:
			 * DFS is supported only in SCC for MBSSID Mode.
			 * We shall not return EBUSY or ENOTSUPP as when Primary
			 * AP is operating in DFS channel and secondary AP is
			 * started. Though we force SCC in driver, the hostapd
			 * issues obss scan before starting secAP. This results
			 * in MCC in DFS mode. Thus we return null scan result.
			 * If we return scan failure hostapd fails secondary AP
			 * startup.
			 */
			hdd_err("##In DFS Master mode. Scan aborted");
			if (NL_SCAN == source)
				adapter->request = request;
			else
				adapter->vendor_request = request;

			schedule_work(&adapter->scan_block_work);
			return 0;
		}
	}

	/* Check if scan is allowed at this point of time */
	if (hdd_is_connection_in_progress(&curr_session_id, &curr_reason)) {
		scan_ebusy_cnt++;
		hdd_err_ratelimited(HDD_SCAN_REJECT_RATE_LIMIT,
			"Scan not allowed. scan_ebusy_cnt: %d Session %d Reason %d",
			scan_ebusy_cnt, curr_session_id, curr_reason);
		if (hdd_ctx->last_scan_reject_session_id != curr_session_id ||
		    hdd_ctx->last_scan_reject_reason != curr_reason ||
		    !hdd_ctx->last_scan_reject_timestamp) {
			hdd_ctx->last_scan_reject_session_id = curr_session_id;
			hdd_ctx->last_scan_reject_reason = curr_reason;
			hdd_ctx->last_scan_reject_timestamp =
				jiffies_to_msecs(jiffies) +
				SCAN_REJECT_THRESHOLD_TIME;
			hdd_ctx->scan_reject_cnt = 0;
		} else {
			hdd_ctx->scan_reject_cnt++;
			if ((hdd_ctx->scan_reject_cnt >=
			   SCAN_REJECT_THRESHOLD) &&
			   qdf_system_time_after(jiffies_to_msecs(jiffies),
			   hdd_ctx->last_scan_reject_timestamp)) {
				hdd_err("scan reject threshold reached Session %d Reason %d count %d",
					curr_session_id, curr_reason,
					hdd_ctx->scan_reject_cnt);
				hdd_ctx->last_scan_reject_timestamp = 0;
				hdd_ctx->scan_reject_cnt = 0;
				if (hdd_ctx->config->enable_fatal_event) {
					cds_flush_logs(WLAN_LOG_TYPE_FATAL,
					   WLAN_LOG_INDICATOR_HOST_DRIVER,
					   WLAN_LOG_REASON_SCAN_NOT_ALLOWED,
					   false,
					   hdd_ctx->config->enableSelfRecovery);
				} else {
					hdd_err("Triggering SSR due to scan stuck");
					cds_trigger_recovery(SCAN_FAILURE);
				}
			}
		}
		return -EBUSY;
	}
	hdd_ctx->last_scan_reject_timestamp = 0;
	hdd_ctx->last_scan_reject_session_id = 0xFF;
	hdd_ctx->last_scan_reject_reason = 0;
	hdd_ctx->scan_reject_cnt = 0;

	/* Check whether SAP scan can be skipped or not */
	if (adapter->device_mode == QDF_SAP_MODE &&
	   wlan_hdd_sap_skip_scan_check(hdd_ctx, request)) {
		hdd_debug("sap scan skipped");
		if (NL_SCAN == source)
			adapter->request = request;
		else
			adapter->vendor_request = request;
		schedule_work(&adapter->scan_block_work);
		return 0;
	}

	params.source = source;
	params.default_ie.len = 0;
	/* Store the Scan IE's in Adapter*/
	if (request->ie_len) {
		/* save this for future association (join requires this) */
		memset(&scan_info->scan_add_ie, 0, sizeof(scan_info->scan_add_ie));
		memcpy(scan_info->scan_add_ie.addIEdata, request->ie,
		       request->ie_len);
		scan_info->scan_add_ie.length = request->ie_len;

		wlan_hdd_update_scan_ies(adapter, scan_info,
				scan_info->scan_add_ie.addIEdata,
				&scan_info->scan_add_ie.length);
	} else {
		if (scan_info->default_scan_ies &&
		    scan_info->default_scan_ies_len) {
			qdf_mem_copy(scan_info->scan_add_ie.addIEdata,
				     scan_info->default_scan_ies,
				     scan_info->default_scan_ies_len);
			scan_info->scan_add_ie.length =
				scan_info->default_scan_ies_len;
			params.default_ie.ptr =
				qdf_mem_malloc(scan_info->default_scan_ies_len);
			if (params.default_ie.ptr != NULL) {
				qdf_mem_copy(params.default_ie.ptr,
					     scan_info->default_scan_ies,
					     scan_info->default_scan_ies_len);
				params.default_ie.len =
						scan_info->default_scan_ies_len;
			}
		}
	}

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)) {
		pwextBuf->roamProfile.pAddIEScan =
			scan_info->scan_add_ie.addIEdata;
		pwextBuf->roamProfile.nAddIEScanLength =
			scan_info->scan_add_ie.length;
	}

	if ((request->n_ssids == 1) && (request->ssids != NULL) &&
	    (request->ssids[0].ssid_len > 7) &&
	     !qdf_mem_cmp(&request->ssids[0], "DIRECT-", 7))
		ucfg_p2p_status_scan(adapter->hdd_vdev);

	status = wlan_cfg80211_scan(hdd_ctx->hdd_pdev, request, &params);
	if (params.default_ie.ptr)
		qdf_mem_free(params.default_ie.ptr);
	EXIT();
	return status;
}

#undef SCAN_FAILURE

/**
 * wlan_hdd_cfg80211_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @request: Pointer to scan request
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
			   struct cfg80211_scan_request *request)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_scan(wiphy,
				request, NL_SCAN);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * wlan_hdd_cfg80211_tdls_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @request: Pointer to scan request
 * @source: scan request source(NL/Vendor scan)
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results. This
 * function gets called when tdls module queues the scan request.
 *
 * Return: 0 for success, non zero for failure.
 */
int wlan_hdd_cfg80211_tdls_scan(struct wiphy *wiphy,
				struct cfg80211_scan_request *request,
				uint8_t source)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_scan(wiphy,
				request, source);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * wlan_hdd_get_rates() -API to get the rates from scan request
 * @wiphy: Pointer to wiphy
 * @band: Band
 * @rates: array of rates
 * @rate_count: number of rates
 *
 * Return: o for failure, rate bitmap for success
 */
static uint32_t wlan_hdd_get_rates(struct wiphy *wiphy,
	enum nl80211_band band,
	const u8 *rates, unsigned int rate_count)
{
	uint32_t j, count, rate_bitmap = 0;
	uint32_t rate;
	bool found;

	for (count = 0; count < rate_count; count++) {
		rate = ((rates[count]) & RATE_MASK) * 5;
		found = false;
		for (j = 0; j < wiphy->bands[band]->n_bitrates; j++) {
			if (wiphy->bands[band]->bitrates[j].bitrate == rate) {
				found = true;
				rate_bitmap |= (1 << j);
				break;
			}
		}
		if (!found)
			return 0;
	}
	return rate_bitmap;
}

/**
 * wlan_hdd_send_scan_start_event() -API to send the scan start event
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @cookie: scan identifier
 *
 * Return: return 0 on success and negative error code on failure
 */
static int wlan_hdd_send_scan_start_event(struct wiphy *wiphy,
		struct wireless_dev *wdev, uint64_t cookie)
{
	struct sk_buff *skb;
	int ret;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u64) +
			NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		hdd_err(" reply skb alloc failed");
		return -ENOMEM;
	}

	if (hdd_wlan_nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				 cookie)) {
		hdd_err("nla put fail");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = cfg80211_vendor_cmd_reply(skb);

	/* Send a scan started event to supplicant */
	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
		sizeof(u64) + 4 + NLMSG_HDRLEN,
		QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX, GFP_KERNEL);
	if (!skb) {
		hdd_err("skb alloc failed");
		return -ENOMEM;
	}

	if (hdd_wlan_nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				 cookie)) {
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);

	return ret;
}

/**
 * wlan_hdd_copy_bssid() - API to copy the bssid to vendor Scan request
 * @request: Pointer to vendor scan request
 * @bssid: Pointer to BSSID
 *
 * This API copies the specific BSSID received from Supplicant and copies it to
 * the vendor Scan request
 *
 * Return: None
 */
#if defined(CFG80211_SCAN_BSSID) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void wlan_hdd_copy_bssid(struct cfg80211_scan_request *request,
					uint8_t *bssid)
{
	qdf_mem_copy(request->bssid, bssid, QDF_MAC_ADDR_SIZE);
}
#else
static inline void wlan_hdd_copy_bssid(struct cfg80211_scan_request *request,
					uint8_t *bssid)
{
}
#endif

static void hdd_process_vendor_acs_response(struct hdd_adapter *adapter)
{
	if (test_bit(VENDOR_ACS_RESPONSE_PENDING, &adapter->event_flags)) {
		if (QDF_TIMER_STATE_RUNNING ==
		    qdf_mc_timer_get_current_state(&adapter->session.
					ap.vendor_acs_timer)) {
			qdf_mc_timer_stop(&adapter->session.
					ap.vendor_acs_timer);
		}
	}
}

#if defined(CFG80211_SCAN_RANDOM_MAC_ADDR) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
/**
 * wlan_hdd_vendor_scan_random_attr() - check and fill scan randomization attrs
 * @wiphy: Pointer to wiphy
 * @request: Pointer to scan request
 * @wdev: Pointer to wireless device
 * @tb: Pointer to nl attributes
 *
 * This function is invoked to check whether vendor scan needs
 * probe req source addr, if so populates mac_addr and mac_addr_mask
 * in scan request with nl attrs.
 *
 * Return: 0 - on success, negative value on failure
 */
static int wlan_hdd_vendor_scan_random_attr(struct wiphy *wiphy,
					struct cfg80211_scan_request *request,
					struct wireless_dev *wdev,
					struct nlattr **tb)
{
	uint32_t i;
	int32_t len = QDF_MAC_ADDR_SIZE;

	if (!(request->flags & NL80211_SCAN_FLAG_RANDOM_ADDR))
		return 0;

	if (!(wiphy->features & NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR) ||
	    (wdev->current_bss)) {
		hdd_err("SCAN RANDOMIZATION not supported");
		return -EOPNOTSUPP;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC] ||
	    !tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK])
		return -EINVAL;

	if ((nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC]) != len) ||
	    (nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK]) != len))
		return -EINVAL;

	qdf_mem_copy(request->mac_addr,
		     nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC]), len);

	qdf_mem_copy(request->mac_addr_mask,
		     nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK]), len);

	/* avoid configure on multicast address */
	if (!cds_is_group_addr(request->mac_addr_mask) ||
	    cds_is_group_addr(request->mac_addr))
		return -EINVAL;

	for (i = 0; i < ETH_ALEN; i++)
		request->mac_addr[i] &= request->mac_addr_mask[i];

	return 0;
}
#else
static int wlan_hdd_vendor_scan_random_attr(struct wiphy *wiphy,
					struct cfg80211_scan_request *request,
					struct wireless_dev *wdev,
					struct nlattr **tb)
{
	return 0;
}
#endif

static const
struct nla_policy scan_policy[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE] = {.type = NLA_FLAG},
	[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE] = {.type = NLA_U64},
};

/**
 * __wlan_hdd_cfg80211_vendor_scan() - API to process venor scan request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * API to process venor scan request.
 *
 * Return: return 0 on success and negative error code on failure
 */
static int __wlan_hdd_cfg80211_vendor_scan(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data,
		int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1];
	struct cfg80211_scan_request *request = NULL;
	struct nlattr *attr;
	enum nl80211_band band;
	uint32_t n_channels = 0, n_ssid = 0;
	uint32_t tmp, count, j;
	size_t len, ie_len;
	struct ieee80211_channel *chan;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	int ret;

	ENTER_DEV(wdev->netdev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SCAN_MAX,
				    data, data_len, scan_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES]) {
		nla_for_each_nested(attr,
			tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES], tmp)
			n_channels++;
	} else {
		for (band = 0; band < HDD_NUM_NL80211_BANDS; band++)
			if (wiphy->bands[band])
				n_channels += wiphy->bands[band]->n_channels;
	}

	if (MAX_CHANNEL < n_channels) {
		hdd_err("Exceed max number of channels: %d", n_channels);
		return -EINVAL;
	}
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS])
		nla_for_each_nested(attr,
			tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS], tmp)
			n_ssid++;

	if (MAX_SCAN_SSID < n_ssid) {
		hdd_err("Exceed max number of SSID: %d", n_ssid);
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE])
		ie_len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]);
	else
		ie_len = 0;

	len = sizeof(*request) + (sizeof(*request->ssids) * n_ssid) +
			(sizeof(*request->channels) * n_channels) + ie_len;

	request = qdf_mem_malloc(len);
	if (!request)
		goto error;
	if (n_ssid)
		request->ssids = (void *)&request->channels[n_channels];
	request->n_ssids = n_ssid;
	if (ie_len) {
		if (request->ssids)
			request->ie = (void *)(request->ssids + n_ssid);
		else
			request->ie = (void *)(request->channels + n_channels);
	}

	count = 0;
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES]) {
		nla_for_each_nested(attr,
				    tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES],
				    tmp) {
			if (nla_len(attr) != sizeof(uint32_t)) {
				hdd_err("len is not correct for frequency %d",
					count);
				goto error;
			}
			chan = ieee80211_get_channel(wiphy, nla_get_u32(attr));
			if (!chan)
				goto error;
			if (chan->flags & IEEE80211_CHAN_DISABLED)
				continue;
			request->channels[count] = chan;
			count++;
		}
	} else {
		for (band = 0; band < HDD_NUM_NL80211_BANDS; band++) {
			if (!wiphy->bands[band])
				continue;
			for (j = 0; j < wiphy->bands[band]->n_channels;
				j++) {
				chan = &wiphy->bands[band]->channels[j];
				if (chan->flags & IEEE80211_CHAN_DISABLED)
					continue;
				request->channels[count] = chan;
				count++;
			}
		}
	}

	if (!count)
		goto error;

	request->n_channels = count;
	count = 0;
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS]) {
		int ssid_length;

		nla_for_each_nested(attr, tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS],
				tmp) {
			ssid_length = nla_len(attr);
			if ((ssid_length > SIR_MAC_MAX_SSID_LENGTH) ||
			    (ssid_length < 0)) {
				hdd_err("SSID Len %d is not correct for network %d",
					 ssid_length, count);
				goto error;
			}

			request->ssids[count].ssid_len = ssid_length;
			memcpy(request->ssids[count].ssid, nla_data(attr),
					ssid_length);
			count++;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]) {
		request->ie_len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]);
		memcpy((void *)request->ie,
				nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]),
				request->ie_len);
	}

	for (count = 0; count < HDD_NUM_NL80211_BANDS; count++)
		if (wiphy->bands[count])
			request->rates[count] =
				(1 << wiphy->bands[count]->n_bitrates) - 1;

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES]) {
		nla_for_each_nested(attr,
				    tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES],
				    tmp) {
			band = nla_type(attr);
			if (band >= HDD_NUM_NL80211_BANDS)
				continue;
			if (!wiphy->bands[band])
				continue;
			request->rates[band] =
				wlan_hdd_get_rates(wiphy,
						   band, nla_data(attr),
						   nla_len(attr));
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]) {
		request->flags =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]);
		if ((request->flags & NL80211_SCAN_FLAG_LOW_PRIORITY) &&
		    !(wiphy->features & NL80211_FEATURE_LOW_PRIORITY_SCAN)) {
			hdd_err("LOW PRIORITY SCAN not supported");
			goto error;
		}

		if (wlan_hdd_vendor_scan_random_attr(wiphy, request, wdev, tb))
			goto error;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]) {
		if (nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]) <
		    QDF_MAC_ADDR_SIZE) {
			hdd_err("invalid bssid length");
			goto error;
		}
		wlan_hdd_copy_bssid(request,
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]));
	}

	/* Check if external acs was requested on this adapter */
	hdd_process_vendor_acs_response(adapter);

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE])
		request->no_cck =
		   nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE]);
	request->wdev = wdev;
	request->wiphy = wiphy;
	request->scan_start = jiffies;

	ret = __wlan_hdd_cfg80211_scan(wiphy, request, VENDOR_SCAN);
	if (0 != ret) {
		hdd_err("Scan Failed. Ret = %d", ret);
		qdf_mem_free(request);
		return ret;
	}
	ret = wlan_hdd_send_scan_start_event(wiphy, wdev, (uintptr_t)request);

	return ret;
error:
	hdd_err("Scan Request Failed");
	qdf_mem_free(request);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_vendor_scan() -API to process venor scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * This is called from userspace to request scan.
 *
 * Return: Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_vendor_scan(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data,
		int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_vendor_scan(wiphy, wdev,
					      data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_vendor_abort_scan() - API to process vendor command for
 * abort scan
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * API to process vendor abort scan
 *
 * Return: zero for success and non zero for failure
 */
static int __wlan_hdd_vendor_abort_scan(
		struct wiphy *wiphy, const void *data,
		int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	wlan_vendor_abort_scan(hdd_ctx->hdd_pdev, data, data_len);

	return ret;
}

/**
 * wlan_hdd_vendor_abort_scan() - API to process vendor command for
 * abort scan
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * This is called from supplicant to abort scan
 *
 * Return: zero for success and non zero for failure
 */
int wlan_hdd_vendor_abort_scan(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_vendor_abort_scan(wiphy,
					   data,
					   data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_scan_abort() - abort ongoing scan
 * @adapter: Pointer to interface adapter
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_scan_abort(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
			adapter->session_id, INVALID_SCAN_ID, true);

	return 0;
}

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * __wlan_hdd_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_sched_scan_start(struct wiphy *wiphy,
						struct net_device *dev,
						struct
						cfg80211_sched_scan_request
						*request)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	tHalHandle hHal;
	int ret = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	if (QDF_NDI_MODE == adapter->device_mode) {
		hdd_err("Command not allowed for NDI interface");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);

	if (0 != ret)
		return ret;

	if (!hdd_ctx->config->PnoOffload) {
		hdd_debug("PnoOffloadis not enabled!!!");
		return -EINVAL;
	}

	if ((eConnectionState_Associated ==
				WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
							conn_info.connState) &&
	    (!hdd_ctx->config->enable_connected_scan)) {
		hdd_info("enable_connected_scan is false, Aborting scan");
		return -EBUSY;
	}

	if (!sme_is_session_id_valid(hdd_ctx->hHal, adapter->session_id))
		return -EINVAL;

	hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (NULL == hHal) {
		hdd_err("HAL context  is Null!!!");
		return -EINVAL;
	}

	return wlan_cfg80211_sched_scan_start(hdd_ctx->hdd_pdev, dev, request,
				      hdd_ctx->config->scan_backoff_multiplier);
}

/**
 * wlan_hdd_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_cfg80211_sched_scan_start(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_sched_scan_request
				       *request)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_start(wiphy, dev, request);
	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_sched_scan_stop(struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	tHalHandle hHal;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (NULL == hdd_ctx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}
	if (!hdd_ctx->config->PnoOffload) {
		hdd_debug("PnoOffloadis not enabled!!!");
		return -EINVAL;
	}

	hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (NULL == hHal) {
		hdd_err(" HAL context  is Null!!!");
		return -EINVAL;
	}

	return wlan_cfg80211_sched_scan_stop(hdd_ctx->hdd_pdev, dev);
}

/**
 * __wlan_hdd_cfg80211_sched_scan_stop() - stop cfg80211 scheduled scan(pno)
 * @dev: Pointer network device
 *
 * This is a wrapper around wlan_hdd_sched_scan_stop() that returns success
 * in the event that the driver is currently recovering or unloading. This
 * prevents a race condition where we get a scan stop from kernel during
 * a driver unload from PLD.
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_sched_scan_stop(struct net_device *dev)
{
	int err;

	ENTER_DEV(dev);

	/* The return 0 is intentional when Recovery and Load/Unload in
	 * progress. We did observe a crash due to a return of
	 * failure in sched_scan_stop , especially for a case where the unload
	 * of the happens at the same time. The function
	 * __cfg80211_stop_sched_scan was clearing rdev->sched_scan_req only
	 * when the sched_scan_stop returns success. If it returns a failure ,
	 * then its next invocation due to the clean up of the second interface
	 * will have the dev pointer corresponding to the first one leading to
	 * a crash.
	 */
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return 0;
	}

	if (cds_is_load_or_unload_in_progress()) {
		hdd_info("Unload/Load in Progress, state: 0x%x.  Ignore!!!",
			cds_get_driver_state());
		return 0;
	}

	err = wlan_hdd_sched_scan_stop(dev);

	EXIT();
	return err;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
int wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
				      struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_stop(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#else
int wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
				      struct net_device *dev,
				      uint64_t reqid)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_stop(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /* KERNEL_VERSION(4, 12, 0) */
#endif /*FEATURE_WLAN_SCAN_PNO */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	defined(CFG80211_ABORT_SCAN)
/**
 * __wlan_hdd_cfg80211_abort_scan() - cfg80211 abort scan api
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wireless device structure
 *
 * This function is used to abort an ongoing scan
 *
 * Return: None
 */
static void __wlan_hdd_cfg80211_abort_scan(struct wiphy *wiphy,
					   struct wireless_dev *wdev)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return;

	wlan_cfg80211_abort_scan(hdd_ctx->hdd_pdev);

	EXIT();
}

/**
 * wlan_hdd_cfg80211_abort_scan - cfg80211 abort scan api
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wireless device structure
 *
 * Wrapper to __wlan_hdd_cfg80211_abort_scan() -
 * function is used to abort an ongoing scan
 *
 * Return: None
 */
void wlan_hdd_cfg80211_abort_scan(struct wiphy *wiphy,
				  struct wireless_dev *wdev)
{
	cds_ssr_protect(__func__);
	__wlan_hdd_cfg80211_abort_scan(wiphy, wdev);
	cds_ssr_unprotect(__func__);
}
#endif

/**
 * hdd_scan_context_destroy() - Destroy scan context
 * @hdd_ctx:	HDD context.
 *
 * Destroy scan context.
 *
 * Return: None.
 */
void hdd_scan_context_destroy(struct hdd_context *hdd_ctx)
{
}

/**
 * hdd_scan_context_init() - Initialize scan context
 * @hdd_ctx:	HDD context.
 *
 * Initialize scan related resources like spin lock and lists.
 *
 * Return: 0 on success and errno on failure.
 */
int hdd_scan_context_init(struct hdd_context *hdd_ctx)
{
	return 0;
}
