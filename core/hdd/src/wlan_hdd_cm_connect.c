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
#include "wlan_crypto_global_api.h"
#include "wlan_vdev_mgr_ucfg_api.h"
#include "wlan_hdd_bootup_marker.h"

void hdd_cm_update_rssi_snr_by_bssid(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx;
	int8_t snr = 0;

	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}
	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		return;
	}

	hdd_get_rssi_snr_by_bssid(adapter, sta_ctx->conn_info.bssid.bytes,
				  &adapter->rssi, &snr);

	/* If RSSi is reported as positive then it is invalid */
	if (adapter->rssi > 0) {
		hdd_debug_rl("RSSI invalid %d", adapter->rssi);
		adapter->rssi = 0;
	}

	hdd_debug("snr: %d, rssi: %d", snr, adapter->rssi);

	sta_ctx->conn_info.signal = adapter->rssi;
	sta_ctx->conn_info.noise =
		sta_ctx->conn_info.signal - snr;
	sta_ctx->cache_conn_info.signal = sta_ctx->conn_info.signal;
	sta_ctx->cache_conn_info.noise = sta_ctx->conn_info.noise;
}

void hdd_cm_handle_assoc_event(struct wlan_objmgr_vdev *vdev, uint8_t *peer_mac)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	int ret;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		return;
	}

	ucfg_p2p_status_connect(adapter->vdev);

	ret = hdd_objmgr_set_peer_mlme_state(adapter->vdev,
					     WLAN_ASSOC_STATE);
	if (ret)
		hdd_err("Peer object " QDF_MAC_ADDR_FMT " fail to set associated state",
			QDF_MAC_ADDR_REF(peer_mac));
	hdd_add_latency_critical_client(
			adapter,
			hdd_convert_cfgdot11mode_to_80211mode(
				sta_ctx->conn_info.dot11mode));

	hdd_bus_bw_compute_prev_txrx_stats(adapter);
	hdd_bus_bw_compute_timer_start(hdd_ctx);

	if (ucfg_pkt_capture_get_pktcap_mode(hdd_ctx->psoc))
		ucfg_pkt_capture_record_channel(adapter->vdev);
}

#ifdef FEATURE_CM_ENABLE
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
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	hdd_cm_update_rssi_snr_by_bssid(adapter);

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

static void hdd_cm_save_bss_info(struct hdd_adapter *adapter,
				 struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *hdd_sta_ctx;
	QDF_STATUS status;
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	struct sDot11fAssocResponse *assoc_resp;

	if (!adapter) {
		hdd_err("Adapter is NULL");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!hdd_sta_ctx) {
		hdd_err("hdd_sta_ctx is NULL");
		return;
	}

	assoc_resp = qdf_mem_malloc(sizeof(struct sDot11fAssocResponse));
	if (!assoc_resp)
		return;

	qdf_mem_zero(&hdd_sta_ctx->conn_info.hs20vendor_ie,
		     sizeof(hdd_sta_ctx->conn_info.hs20vendor_ie));
	sme_get_hs20vendor_ie(mac_handle, rsp->connect_ies.bcn_probe_rsp.ptr,
			      rsp->connect_ies.bcn_probe_rsp.len,
			      &hdd_sta_ctx->conn_info.hs20vendor_ie);

	status = sme_unpack_assoc_rsp(mac_handle,
				      rsp->connect_ies.assoc_req.ptr,
				      rsp->connect_ies.assoc_req.len,
				      assoc_resp);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not parse assoc response");
		qdf_mem_free(assoc_resp);
		return;
	}

	if (assoc_resp->VHTCaps.present) {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = true;
		hdd_copy_vht_caps(&hdd_sta_ctx->conn_info.vht_caps,
				  &assoc_resp->VHTCaps);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = false;
	}
	if (assoc_resp->HTCaps.present) {
		hdd_sta_ctx->conn_info.conn_flag.ht_present = true;
		hdd_copy_ht_caps(&hdd_sta_ctx->conn_info.ht_caps,
				 &assoc_resp->HTCaps);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.ht_present = false;
	}
	if (hdd_is_roam_sync_in_progress(hdd_ctx, adapter->vdev_id))
		hdd_sta_ctx->conn_info.roam_count++;

	if (assoc_resp->HTInfo.present) {
		hdd_sta_ctx->conn_info.conn_flag.ht_op_present = true;
		hdd_copy_ht_operation(hdd_sta_ctx, &assoc_resp->HTInfo);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.ht_op_present = false;
	}
	if (assoc_resp->VHTOperation.present) {
		hdd_sta_ctx->conn_info.conn_flag.vht_op_present = true;
		hdd_copy_vht_operation(hdd_sta_ctx, &assoc_resp->VHTOperation);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.vht_op_present = false;
	}

	/* Cleanup already existing he info */
	hdd_cleanup_conn_info(adapter);

	/* Cache last connection info */
	qdf_mem_copy(&hdd_sta_ctx->cache_conn_info, &hdd_sta_ctx->conn_info,
		     sizeof(hdd_sta_ctx->cache_conn_info));

	hdd_copy_he_operation(hdd_sta_ctx, &assoc_resp->he_op);
	qdf_mem_free(assoc_resp);
}

static void hdd_cm_save_connect_info(struct hdd_adapter *adapter,
				     struct wlan_cm_connect_resp *rsp)
{
	struct hdd_station_ctx *sta_ctx;
	struct wlan_crypto_params *crypto_params;
	struct wlan_channel *des_chan;
	struct wlan_objmgr_vdev *vdev;

	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		return;
	}

	qdf_copy_macaddr(&sta_ctx->conn_info.bssid, &rsp->bssid);

	crypto_params = wlan_crypto_vdev_get_crypto_params(adapter->vdev);

	sme_fill_enc_type(&sta_ctx->conn_info.uc_encrypt_type,
			  crypto_params->ucastcipherset);

	sme_fill_auth_type(&sta_ctx->conn_info.auth_type,
			   crypto_params->authmodeset,
			   crypto_params->key_mgmt,
			   crypto_params->ucastcipherset);

	des_chan = wlan_vdev_mlme_get_des_chan(adapter->vdev);

	sta_ctx->conn_info.chan_freq = rsp->freq;

	/* Save the ssid for the connection */
	qdf_mem_copy(&sta_ctx->conn_info.ssid.SSID.ssId,
		     &rsp->ssid.ssid,
		     rsp->ssid.length);
	qdf_mem_copy(&sta_ctx->conn_info.last_ssid.SSID.ssId,
		     &rsp->ssid.ssid,
		     rsp->ssid.length);
	sta_ctx->conn_info.ssid.SSID.length = rsp->ssid.length;
	sta_ctx->conn_info.last_ssid.SSID.length = rsp->ssid.length;

	sta_ctx->conn_info.dot11mode =
				sme_phy_mode_to_dot11mode(des_chan->ch_phymode);

	sta_ctx->conn_info.ch_width = des_chan->ch_width;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (vdev) {
		sta_ctx->conn_info.nss = wlan_vdev_mlme_get_nss(vdev);
		ucfg_wlan_vdev_mgr_get_param(vdev, WLAN_MLME_CFG_RATE_FLAGS,
					     &sta_ctx->conn_info.rate_flags);
		hdd_objmgr_put_vdev(vdev);
	}

	hdd_cm_save_bss_info(adapter, rsp);
	/*
	 *  proxy arp service, notify WMM
	 * hdd_wmm_connect(adapter, roam_info, bss_type);
	 */
}

static void
hdd_cm_connect_success_pre_user_update(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct vdev_mlme_obj *vdev_mlme;
	unsigned long rc;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	hdd_cm_update_rssi_snr_by_bssid(adapter);

	hdd_cm_save_connect_info(adapter, rsp);

	if (hdd_add_beacon_filter(adapter) != 0)
		hdd_err("add beacon fileter failed");

	/*
	 * FEATURE_WLAN_WAPI, hdd_send_association_event,
	 */

	policy_mgr_incr_active_session(hdd_ctx->psoc,
				adapter->device_mode, adapter->vdev_id);
	hdd_green_ap_start_state_mc(hdd_ctx, adapter->device_mode, true);

	hdd_cm_handle_assoc_event(vdev, rsp->bssid.bytes);
	/*
	 * check update hdd_send_update_beacon_ies_event,
	 * hdd_send_ft_assoc_response,
	 */

	/* send peer status indication to oem app */
	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							  WLAN_UMAC_COMP_MLME);
	if (vdev_mlme) {
		hdd_send_peer_status_ind_to_app(
			&rsp->bssid,
			ePeerConnected,
			vdev_mlme->ext_vdev_ptr->connect_info.timing_meas_cap,
			adapter->vdev_id,
			&vdev_mlme->ext_vdev_ptr->connect_info.chan_info,
			adapter->device_mode);
	}

	hdd_ipa_set_tx_flow_info();
	hdd_place_marker(adapter, "ASSOCIATION COMPLETE", NULL);

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
	/*
	 * Disable Linkup Event Servicing - no more service
	 * required from the net device notifier call.
	 */
	adapter->is_link_up_service_needed = false;

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

	 /* hdd_objmgr_set_peer_mlme_auth_state */
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
#endif

