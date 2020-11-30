/*
 * Copyright (c) 2012-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: hdd_cm_connect.c
 *
 * WLAN Host Device Driver connect APIs implementation
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_cm_api.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_hdd_power.h"
#include <osif_cm_req.h>
#include <wlan_logging_sock_svc.h>
#include <wlan_hdd_periodic_sta_stats.h>
#include <wlan_hdd_green_ap.h>
#include <wlan_hdd_p2p.h>
#include <wlan_p2p_ucfg_api.h>
#include <wlan_pkt_capture_ucfg_api.h>
#include <wlan_hdd_ipa.h>
#include <wlan_ipa_ucfg_api.h>
#include <wlan_hdd_ftm_time_sync.h>

static void hdd_update_scan_ie_for_connect(struct hdd_adapter *adapter,
					   struct osif_connect_params *params)
{
	if (adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		params->scan_ie.ptr =
			&adapter->scan_info.scan_add_ie.addIEdata[0];
		params->scan_ie.len = adapter->scan_info.scan_add_ie.length;
	} else if (adapter->scan_info.default_scan_ies) {
		params->scan_ie.ptr = adapter->scan_info.default_scan_ies;
		params->scan_ie.len = adapter->scan_info.default_scan_ies_len;
	} else if (adapter->scan_info.scan_add_ie.length) {
		params->scan_ie.ptr = adapter->scan_info.scan_add_ie.addIEdata;
		params->scan_ie.len = adapter->scan_info.scan_add_ie.length;
	}
}

/**
 * hdd_get_dot11mode_filter() - Get dot11 mode filter
 * @hdd_ctx: HDD context
 *
 * This function is used to get the dot11 mode filter
 *
 * Context: Any Context.
 * Return: dot11_mode_filter
 */
static enum dot11_mode_filter
hdd_get_dot11mode_filter(struct hdd_context *hdd_ctx)
{
	struct hdd_config *config = hdd_ctx->config;

	if (config->dot11Mode == eHDD_DOT11_MODE_11n_ONLY)
		return ALLOW_11N_ONLY;
	else if (config->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY)
		return ALLOW_11AC_ONLY;
	else if (config->dot11Mode == eHDD_DOT11_MODE_11ax_ONLY)
		return ALLOW_11AX_ONLY;
	else
		return ALLOW_ALL;
}

int wlan_hdd_cm_connect(struct wiphy *wiphy,
			struct net_device *ndev,
			struct cfg80211_connect_params *req)
{
	int status;
	struct wlan_objmgr_vdev *vdev;
	struct osif_connect_params params;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	struct hdd_context *hdd_ctx;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_CONNECT,
		   adapter->vdev_id, adapter->device_mode);

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_err("Device_mode %s(%d) is not supported",
			qdf_opmode_str(adapter->device_mode),
			adapter->device_mode);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	vdev = hdd_objmgr_get_vdev(adapter);

	ucfg_pmo_flush_gtk_offload_req(vdev);

	qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.connect);
	hdd_prevent_suspend_timeout(HDD_WAKELOCK_CONNECT_COMPLETE,
				    WIFI_POWER_EVENT_WAKELOCK_CONNECT);

	params.force_rsne_override = hdd_ctx->force_rsne_override;
	params.dot11mode_filter = hdd_get_dot11mode_filter(hdd_ctx);

	hdd_update_scan_ie_for_connect(adapter, &params);

	status = osif_cm_connect(ndev, vdev, req, &params);

	if (status) {
		hdd_err("Vdev %d connect failed status %d",
			adapter->vdev_id, status);
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	}

	hdd_objmgr_put_vdev(vdev);
	return status;
}

static void
hdd_cm_connect_failure_pre_user_update(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_resp *rsp)
{
	/*
	 * Check if failure of scan for sssid is handled in osif or not
	 */
	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);
}

static void
hdd_cm_connect_failure_post_user_update(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx,
						wlan_vdev_get_id(vdev));

	qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	hdd_wmm_dscp_initial_state(adapter);
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);
	hdd_periodic_sta_stats_start(adapter);
}

static void hdd_cm_connect_failure(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_resp *rsp,
				   enum osif_cb_type type)
{
	switch (type) {
	case OSIF_PRE_USERSPACE_UPDATE:
		hdd_cm_connect_failure_pre_user_update(vdev, rsp);
		break;
	case OSIF_POST_USERSPACE_UPDATE:
		hdd_cm_connect_failure_post_user_update(vdev, rsp);
		break;
	default:
		hdd_cm_connect_failure_pre_user_update(vdev, rsp);
		hdd_cm_connect_failure_post_user_update(vdev, rsp);
	}
}

static void
hdd_cm_connect_success_pre_user_update(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx,
						wlan_vdev_get_id(vdev));
	unsigned long rc;
	int ret;

	/*
	 * check API hdd_conn_save_connect_info, hdd_add_beacon_filter
	 * FEATURE_WLAN_WAPI, hdd_send_association_event,
	 */

	policy_mgr_incr_active_session(hdd_ctx->psoc,
				adapter->device_mode, adapter->vdev_id);
	hdd_green_ap_start_state_mc(hdd_ctx, adapter->device_mode, true);
	/* move ucfg_p2p_status_connect to cm */

	ret = hdd_objmgr_set_peer_mlme_state(adapter->vdev,
					     WLAN_ASSOC_STATE);
	if (ret)
		hdd_err("Peer object "QDF_MAC_ADDR_FMT" fail to set associated state",
			QDF_MAC_ADDR_REF(rsp->bssid.bytes));

	/*
	 * check update hdd_send_update_beacon_ies_event,
	 * hdd_send_ft_assoc_response, hdd_send_peer_status_ind_to_app
	 * Update tdls module about connection event,
	 * hdd_add_latency_critical_client
	 */
	hdd_bus_bw_compute_prev_txrx_stats(adapter);
	hdd_bus_bw_compute_timer_start(hdd_ctx);

	if (ucfg_pkt_capture_get_pktcap_mode(hdd_ctx->psoc))
		ucfg_pkt_capture_record_channel(adapter->vdev);

	hdd_ipa_set_tx_flow_info();

	if (policy_mgr_is_mcc_in_24G(hdd_ctx->psoc)) {
		if (hdd_ctx->miracast_value)
			wlan_hdd_set_mas(adapter, hdd_ctx->miracast_value);
	}

	/* Initialize the Linkup event completion variable */
	INIT_COMPLETION(adapter->linkup_event_var);

	/*
	 * Enable Linkup Event Servicing which allows the net
	 * device notifier to set the linkup event variable.
	 */
	adapter->is_link_up_service_needed = true;

	/* Switch on the Carrier to activate the device */
	wlan_hdd_netif_queue_control(adapter, WLAN_NETIF_CARRIER_ON,
				     WLAN_CONTROL_PATH);

	/*
	 * Wait for the Link to up to ensure all the queues
	 * are set properly by the kernel.
	 */
	rc = wait_for_completion_timeout(
				&adapter->linkup_event_var,
				 msecs_to_jiffies(ASSOC_LINKUP_TIMEOUT));
	if (!rc)
		hdd_warn("Warning:ASSOC_LINKUP_TIMEOUT");

	/*
	 * Disable Linkup Event Servicing - no more service
	 * required from the net device notifier call.
	 */
	adapter->is_link_up_service_needed = false;

	/*check cdp_hl_fc_set_td_limit */

	if (ucfg_ipa_is_enabled())
		ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode,
				  adapter->vdev_id,
				  WLAN_IPA_STA_CONNECT,
				  rsp->bssid.bytes);

	wlan_hdd_auto_shutdown_enable(hdd_ctx, false);

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
		adapter->vdev_id,
		QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_ASSOC));

	/*
	 * check for update bss db, hdd_roam_register_sta,
	 * hdd_objmgr_set_peer_mlme_auth_state
	 */
}

static void
hdd_cm_connect_success_post_user_update(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx,
						wlan_vdev_get_id(vdev));
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	/*
	 * check for hdd_wmm_assoc, wlan_hdd_send_roam_auth_event,
	 * hdd_roam_register_sta
	 */

	qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);

	cdp_hl_fc_set_td_limit(soc, adapter->vdev_id,
			       sta_ctx->conn_info.chan_freq);

	hdd_debug("Enabling queues");

	/*
	 * hdd_netif_queue_enable(adapter); this is static function
	 * will enable this once this code is enabled
	 */

	/* Inform FTM TIME SYNC about the connection with AP */
	hdd_ftm_time_sync_sta_state_notify(adapter,
					   FTM_TIME_SYNC_STA_CONNECTED);
	hdd_periodic_sta_stats_start(adapter);
}

static void hdd_cm_connect_success(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_resp *rsp,
				   enum osif_cb_type type)
{
	switch (type) {
	case OSIF_PRE_USERSPACE_UPDATE:
		hdd_cm_connect_success_pre_user_update(vdev, rsp);
		break;
	case OSIF_POST_USERSPACE_UPDATE:
		hdd_cm_connect_success_post_user_update(vdev, rsp);
		break;
	default:
		hdd_cm_connect_success_pre_user_update(vdev, rsp);
		hdd_cm_connect_success_post_user_update(vdev, rsp);
	}
}

QDF_STATUS hdd_cm_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_connect_resp *rsp,
				   enum osif_cb_type type)
{
	if (QDF_IS_STATUS_ERROR(rsp->connect_status))
		hdd_cm_connect_failure(vdev, rsp, type);
	else
		hdd_cm_connect_success(vdev, rsp, type);

	return QDF_STATUS_SUCCESS;
}
