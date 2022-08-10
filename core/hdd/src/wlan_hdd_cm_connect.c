/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_hdd_connectivity_logging.h"
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
#include "sme_qos_internal.h"
#include "wlan_blm_ucfg_api.h"
#include "wlan_hdd_scan.h"
#include "wlan_osif_priv.h"
#include <enet.h>
#include <wlan_mlme_twt_ucfg_api.h>
#include "wlan_roam_debug.h"
#include <wlan_hdd_regulatory.h>
#include <wlan_twt_ucfg_ext_api.h>
#include <osif_twt_internal.h>
#include "wlan_hdd_hostapd.h"

bool hdd_cm_is_vdev_associated(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_active;
	enum QDF_OPMODE opmode;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (adapter->device_mode == QDF_NDI_MODE)
		return (sta_ctx->conn_info.conn_state ==
			eConnectionState_NdiConnected);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_active = ucfg_cm_is_vdev_active(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_active;
}

bool hdd_cm_is_vdev_connected(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_connected;
	enum QDF_OPMODE opmode;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (adapter->device_mode == QDF_NDI_MODE)
		return (sta_ctx->conn_info.conn_state ==
			eConnectionState_NdiConnected);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_connected = ucfg_cm_is_vdev_connected(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_connected;
}

bool hdd_cm_is_connecting(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_connecting;
	enum QDF_OPMODE opmode;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_connecting = ucfg_cm_is_vdev_connecting(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_connecting;
}

bool hdd_cm_is_disconnected(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_disconnected;
	enum QDF_OPMODE opmode;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_disconnected = ucfg_cm_is_vdev_disconnected(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_disconnected;
}

bool hdd_cm_is_disconnecting(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_disconnecting;
	enum QDF_OPMODE opmode;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);

	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_disconnecting = ucfg_cm_is_vdev_disconnecting(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_disconnecting;
}

bool hdd_cm_is_vdev_roaming(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_roaming;
	enum QDF_OPMODE opmode;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return false;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
		return false;
	}
	is_vdev_roaming = ucfg_cm_is_vdev_roaming(vdev);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return is_vdev_roaming;
}

void hdd_cm_set_peer_authenticate(struct hdd_adapter *adapter,
				  struct qdf_mac_addr *bssid,
				  bool is_auth_required)
{
	hdd_debug("sta: " QDF_MAC_ADDR_FMT "Changing TL state to %s",
		  QDF_MAC_ADDR_REF(bssid->bytes),
		  is_auth_required ? "CONNECTED" : "AUTHENTICATED");

	hdd_change_peer_state(adapter, bssid->bytes,
			      is_auth_required ?
			      OL_TXRX_PEER_STATE_CONN :
			      OL_TXRX_PEER_STATE_AUTH);
	hdd_conn_set_authenticated(adapter, !is_auth_required);
	hdd_objmgr_set_peer_mlme_auth_state(adapter->vdev, !is_auth_required);
}

void hdd_cm_update_rssi_snr_by_bssid(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx;
	int8_t snr = 0;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
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
		hdd_err("adapter is NULL for vdev %d", wlan_vdev_get_id(vdev));
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
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

/**
 * hdd_cm_netif_features_update_required() - Check if feature update
 * is required
 * @adapter: pointer to the adapter structure
 * Returns: true if the connection is legacy and TSO and Checksum offload
 * enabled or if the connection is not latency and TSO and Checksum
 * offload are not enabled, false otherwise
 */
static bool hdd_cm_netif_features_update_required(struct hdd_adapter *adapter)
{
	bool is_legacy_connection = hdd_is_legacy_connection(adapter);

	hdd_debug("Legacy Connection: %d, TSO_CSUM Feature Enabled:%d",
		  is_legacy_connection, adapter->tso_csum_feature_enabled);

	if (adapter->tso_csum_feature_enabled  && is_legacy_connection)
		return true;

	if (!adapter->tso_csum_feature_enabled  && !is_legacy_connection)
		return true;

	return false;
}

void hdd_cm_netif_queue_enable(struct hdd_adapter *adapter)
{
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (cdp_cfg_get(soc, cfg_dp_disable_legacy_mode_csum_offload) &&
	    hdd_cm_netif_features_update_required(adapter)) {
		hdd_adapter_ops_record_event(hdd_ctx,
					     WLAN_HDD_ADAPTER_OPS_WORK_POST,
					     adapter->vdev_id);
		qdf_queue_work(0, hdd_ctx->adapter_ops_wq,
			       &adapter->netdev_features_update_work);
	}

	wlan_hdd_netif_queue_control(adapter,
				     WLAN_WAKE_ALL_NETIF_QUEUE,
				     WLAN_CONTROL_PATH);
}

void hdd_cm_clear_pmf_stats(struct hdd_adapter *adapter)
{
	qdf_mem_zero(&adapter->hdd_stats.hdd_pmf_stats,
		     sizeof(adapter->hdd_stats.hdd_pmf_stats));
}

void hdd_cm_save_connect_status(struct hdd_adapter *adapter,
				uint32_t reason_code)
{
	struct hdd_station_ctx *hdd_sta_ctx;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	adapter->connect_req_status = reason_code;
	hdd_sta_ctx->conn_info.assoc_status_code = reason_code;
	hdd_sta_ctx->cache_conn_info.assoc_status_code = reason_code;
}

#ifdef FEATURE_WLAN_WAPI
static bool hdd_cm_is_wapi_sta(enum csr_akm_type auth_type)
{
	if (auth_type == eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE ||
	    auth_type == eCSR_AUTH_TYPE_WAPI_WAI_PSK)
		return true;
	else
		return false;
}
#else
static inline bool hdd_cm_is_wapi_sta(enum csr_akm_type auth_type)
{
	return false;
}
#endif

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

/**
 * hdd_get_sap_adapter_of_dfs - Get sap adapter on dfs channel
 * @hdd_ctx: HDD context
 *
 * This function is used to get the sap adapter on dfs channel.
 *
 * Return: pointer to adapter or null
 */
static struct hdd_adapter
*hdd_get_sap_adapter_of_dfs(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter, *next_adapter = NULL;
	wlan_net_dev_ref_dbgid dbgid = NET_DEV_HOLD_GET_ADAPTER;
	qdf_freq_t chan_freq = 0;

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		if (adapter->device_mode != QDF_SAP_MODE)
			goto loop_next;

		/*
		 * sap is not in started state and also not under doing CAC,
		 * so it is fine to go ahead with sta.
		 */
		if (!test_bit(SOFTAP_BSS_STARTED, &(adapter)->event_flags) &&
		    (hdd_ctx->dev_dfs_cac_status != DFS_CAC_IN_PROGRESS))
			goto loop_next;

		chan_freq = wlan_get_operation_chan_freq_vdev_id(hdd_ctx->pdev,
							adapter->vdev_id);

		if (chan_freq && wlan_reg_is_dfs_for_freq(hdd_ctx->pdev,
							  chan_freq)) {
			hdd_adapter_dev_put_debug(adapter, dbgid);
			if (next_adapter)
				hdd_adapter_dev_put_debug(next_adapter, dbgid);

			return adapter;
		}
loop_next:
		hdd_adapter_dev_put_debug(adapter, dbgid);
	}

	return NULL;
}

/**
 * wlan_hdd_cm_handle_sap_sta_dfs_conc() - to handle SAP STA DFS conc
 * @hdd_ctx: hdd_ctx
 * @req: connect req
 *
 * This routine will move SAP from dfs to non-dfs, if sta is coming up.
 *
 * Return: false if sta-sap conc is not allowed, else return true
 */
static
bool wlan_hdd_cm_handle_sap_sta_dfs_conc(struct hdd_context *hdd_ctx,
					 struct cfg80211_connect_params *req)
{
	struct hdd_adapter *ap_adapter;
	struct hdd_ap_ctx *hdd_ap_ctx;
	struct hdd_hostapd_state *hostapd_state;
	QDF_STATUS status;
	qdf_freq_t ch_freq = 0;
	struct scan_filter *scan_filter;
	qdf_list_t *list = NULL;
	qdf_list_node_t *cur_lst = NULL;
	struct scan_cache_node *cur_node = NULL;

	ap_adapter = hdd_get_sap_adapter_of_dfs(hdd_ctx);
	/* probably no dfs sap running, no handling required */
	if (!ap_adapter)
		return true;

	/* if sap is currently doing CAC then don't allow sta to go further */
	if (hdd_ctx->dev_dfs_cac_status == DFS_CAC_IN_PROGRESS) {
		hdd_err("Concurrent SAP is in CAC state, STA is not allowed");
		return false;
	}

	/*
	 * log and return error, if we allow STA to go through, we don't
	 * know what is going to happen better stop sta connection
	 */
	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	if (!hdd_ap_ctx) {
		hdd_err("AP context not found");
		return false;
	}

	if (req->channel && req->channel->center_freq) {
		ch_freq = req->channel->center_freq;
		goto def_chan;
	}
	/*
	 * find out by looking in to scan cache where sta is going to
	 * connect by passing its ssid amd bssid.
	 */
	scan_filter = qdf_mem_malloc(sizeof(*scan_filter));
	if (!scan_filter)
		goto def_chan;

	if (req->bssid) {
		scan_filter->num_of_bssid = 1;
		qdf_mem_copy(scan_filter->bssid_list[0].bytes, req->bssid,
			     QDF_MAC_ADDR_SIZE);
	}
	scan_filter->num_of_ssid = 1;
	scan_filter->ssid_list[0].length =
				QDF_MIN(req->ssid_len, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(scan_filter->ssid_list[0].ssid, req->ssid,
		     scan_filter->ssid_list[0].length);
	scan_filter->ignore_auth_enc_type = true;
	list = ucfg_scan_get_result(hdd_ctx->pdev, scan_filter);
	qdf_mem_free(scan_filter);

	if (!list || (list && !qdf_list_size(list))) {
		hdd_debug("scan list empty");
		goto purge_list;
	}
	qdf_list_peek_front(list, &cur_lst);
	if (!cur_lst)
		goto purge_list;

	cur_node = qdf_container_of(cur_lst, struct scan_cache_node, node);
	ch_freq = cur_node->entry->channel.chan_freq;
purge_list:
	if (list)
		ucfg_scan_purge_results(list);
def_chan:
	/*
	 * If the STA's channel is 2.4 GHz, then set pcl with only 2.4 GHz
	 * channels for roaming case.
	 */
	if (WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq)) {
		hdd_info("sap is on dfs, new sta conn on 2.4 is allowed");
		return true;
	}

	/*
	 * If channel is 0 or DFS or LTE unsafe then better to call pcl and
	 * find out the best channel. If channel is non-dfs 5 GHz then
	 * better move SAP to STA's channel to make scc, so we have room
	 * for 3port MCC scenario.
	 */
	if (!ch_freq || wlan_reg_is_dfs_for_freq(hdd_ctx->pdev, ch_freq) ||
	    !policy_mgr_is_safe_channel(hdd_ctx->psoc, ch_freq))
		ch_freq = policy_mgr_get_nondfs_preferred_channel(
			hdd_ctx->psoc, PM_SAP_MODE, true);

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(ap_adapter);
	qdf_event_reset(&hostapd_state->qdf_event);
	wlan_hdd_set_sap_csa_reason(hdd_ctx->psoc, ap_adapter->vdev_id,
				    CSA_REASON_STA_CONNECT_DFS_TO_NON_DFS);

	status = wlansap_set_channel_change_with_csa(
			WLAN_HDD_GET_SAP_CTX_PTR(ap_adapter), ch_freq,
			hdd_ap_ctx->sap_config.ch_width_orig, false);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Set channel with CSA IE failed, can't allow STA");
		return false;
	}

	/*
	 * wait here for SAP to finish the channel switch. When channel
	 * switch happens, SAP sends few beacons with CSA_IE. After
	 * successfully Transmission of those beacons, it will move its
	 * state from started to disconnected and move to new channel.
	 * once it moves to new channel, sap again moves its state
	 * machine from disconnected to started and set this event.
	 * wait for 10 secs to finish this.
	 */
	status = qdf_wait_for_event_completion(&hostapd_state->qdf_event,
					       10000);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("wait for qdf_event failed, STA not allowed!!");
		return false;
	}

	return true;
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
	struct hdd_station_ctx *hdd_sta_ctx;

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
	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	hdd_reg_wait_for_country_change(hdd_ctx);

	if (policy_mgr_is_hw_dbs_capable(hdd_ctx->psoc) &&
	    !wlan_hdd_cm_handle_sap_sta_dfs_conc(hdd_ctx, req)) {
		hdd_err("sap-sta conc will fail, can't allow sta");
		return -EINVAL;
	}

	qdf_mem_zero(&params, sizeof(params));
	ucfg_blm_dump_black_list_ap(hdd_ctx->pdev);
	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return -EINVAL;

	ucfg_pmo_flush_gtk_offload_req(vdev);
	qdf_mem_zero(&hdd_sta_ctx->conn_info.conn_flag,
		     sizeof(hdd_sta_ctx->conn_info.conn_flag));

	/*
	 * Reset the ptk, gtk status flags to avoid using old/previous
	 * connection status.
	 */
	hdd_sta_ctx->conn_info.gtk_installed = false;
	hdd_sta_ctx->conn_info.ptk_installed = false;
	adapter->last_disconnect_reason = 0;

	qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.connect);
	hdd_prevent_suspend_timeout(HDD_WAKELOCK_CONNECT_COMPLETE,
				    WIFI_POWER_EVENT_WAKELOCK_CONNECT);

	params.force_rsne_override = hdd_ctx->force_rsne_override;
	params.dot11mode_filter = hdd_get_dot11mode_filter(hdd_ctx);

	hdd_update_scan_ie_for_connect(adapter, &params);

	wlan_hdd_connectivity_event_connecting(hdd_ctx, req, adapter->vdev_id);
	status = osif_cm_connect(ndev, vdev, req, &params);

	if (status || ucfg_cm_is_vdev_roaming(vdev)) {
		/* Release suspend and wake lock for failure or roam invoke */
		if (status)
			hdd_err("Vdev %d connect failed status %d",
				adapter->vdev_id, status);
		else
			hdd_debug("Vdev %d: connect lead to roam invoke",
				  adapter->vdev_id);
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	}

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
	return status;
}

static void hdd_cm_rec_connect_info(struct hdd_adapter *adapter,
				    struct wlan_cm_connect_resp *rsp)
{
	if (rsp->is_reassoc)
		wlan_rec_conn_info(rsp->vdev_id, DEBUG_CONN_ROAMED_IND,
				   rsp->bssid.bytes, rsp->cm_id, 0);
	else
		wlan_rec_conn_info(rsp->vdev_id, DEBUG_CONN_CONNECT_RESULT,
				   rsp->bssid.bytes, rsp->cm_id << 16 |
				   rsp->reason,
				   rsp->status_code);
}

static void
hdd_cm_connect_failure_pre_user_update(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *hdd_sta_ctx;
	uint32_t time_buffer_size;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rsp->vdev_id);
	if (!adapter) {
		hdd_err("adapter is NULL vdev %d", rsp->vdev_id);
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	time_buffer_size = sizeof(hdd_sta_ctx->conn_info.connect_time);
	qdf_mem_zero(hdd_sta_ctx->conn_info.connect_time, time_buffer_size);
	hdd_init_scan_reject_params(hdd_ctx);
	hdd_cm_save_connect_status(adapter, rsp->status_code);
	hdd_conn_remove_connect_info(hdd_sta_ctx);
	hdd_cm_update_rssi_snr_by_bssid(adapter);
	hdd_cm_rec_connect_info(adapter, rsp);
	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);
}

static void
hdd_cm_connect_failure_post_user_update(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	bool is_roam = rsp->is_reassoc;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rsp->vdev_id);
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", rsp->vdev_id);
		return;
	}
	if (!is_roam) {
		/* call only for connect */
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	}

	wlan_hdd_connectivity_fail_event(vdev, rsp);
	hdd_clear_roam_profile_ie(adapter);
	ucfg_cm_reset_key(hdd_ctx->pdev, adapter->vdev_id);
	hdd_wmm_dscp_initial_state(adapter);
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);
	hdd_periodic_sta_stats_start(adapter);
	wlan_twt_concurrency_update(hdd_ctx);
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

/**
 * hdd_cm_update_prev_ap_ie() - Update the connected AP IEs
 * @hdd_sta_ctx: Station context.
 * @rsp: Connect response
 *
 * This API updates the connected ap beacon IEs to station context connection
 * info.
 *
 * Return: None
 */
static void hdd_cm_update_prev_ap_ie(struct hdd_station_ctx *hdd_sta_ctx,
				     struct wlan_cm_connect_resp *rsp)
{
	struct element_info *bcn_probe_rsp = &rsp->connect_ies.bcn_probe_rsp;
	struct element_info *bcn_ie;
	uint32_t len;

	bcn_ie = &hdd_sta_ctx->conn_info.prev_ap_bcn_ie;
	if (bcn_ie->ptr) {
		qdf_mem_free(bcn_ie->ptr);
		bcn_ie->ptr = NULL;
		bcn_ie->len = 0;
	}

	if (bcn_probe_rsp->ptr &&
	    bcn_probe_rsp->len > sizeof(struct wlan_frame_hdr)) {
		len = bcn_probe_rsp->len - sizeof(struct wlan_frame_hdr);
		bcn_ie->ptr = qdf_mem_malloc(len);
		if (!bcn_ie->ptr) {
			bcn_ie->len = 0;
			return;
		}
		qdf_mem_copy(bcn_ie->ptr, bcn_probe_rsp->ptr +
			     sizeof(struct wlan_frame_hdr), len);
		bcn_ie->len = len;
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

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	assoc_resp = qdf_mem_malloc(sizeof(struct sDot11fAssocResponse));
	if (!assoc_resp)
		return;

	qdf_mem_zero(&hdd_sta_ctx->conn_info.hs20vendor_ie,
		     sizeof(hdd_sta_ctx->conn_info.hs20vendor_ie));
	if (rsp->connect_ies.bcn_probe_rsp.ptr)
		sme_get_hs20vendor_ie(mac_handle,
				      rsp->connect_ies.bcn_probe_rsp.ptr,
				      rsp->connect_ies.bcn_probe_rsp.len,
				      &hdd_sta_ctx->conn_info.hs20vendor_ie);

	status = sme_unpack_assoc_rsp(mac_handle,
				      rsp->connect_ies.assoc_rsp.ptr,
				      rsp->connect_ies.assoc_rsp.len,
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

	/*
	 * Cache connection info only in case of station
	 */
	if (adapter->device_mode == QDF_STA_MODE) {
		/* Cleanup already existing he info */
		hdd_cleanup_conn_info(adapter);

		/* Cache last connection info */
		qdf_mem_copy(&hdd_sta_ctx->cache_conn_info,
			     &hdd_sta_ctx->conn_info,
			     sizeof(hdd_sta_ctx->cache_conn_info));

		hdd_copy_he_operation(hdd_sta_ctx, &assoc_resp->he_op);
		hdd_cm_update_prev_ap_ie(hdd_sta_ctx, rsp);
	}

	qdf_mem_free(assoc_resp);
}

#ifdef FEATURE_WLAN_ESE
static bool hdd_is_ese_assoc(enum csr_akm_type auth_type,
			     tDot11fBeaconIEs *bcn_ie,
			     struct mac_context *mac_ctx)
{
	if ((csr_is_auth_type_ese(auth_type) ||
	     (bcn_ie->ESEVersion.present &&
	     auth_type == eCSR_AUTH_TYPE_OPEN_SYSTEM)) &&
	    mac_ctx->mlme_cfg->lfr.ese_enabled) {
		return true;
	}

	return false;
}
#else
static bool hdd_is_ese_assoc(enum csr_akm_type auth_type,
			     tDot11fBeaconIEs *bcn_ie,
			     struct mac_context *mac_ctx)
{
	return false;
}
#endif

static const uint8_t acm_mask_bit[WLAN_MAX_AC] = {
	0x4,                    /* SME_AC_BK */
	0x8,                    /* SME_AC_BE */
	0x2,                    /* SME_AC_VI */
	0x1                     /* SME_AC_VO */
};

static void hdd_wmm_cm_connect(struct wlan_objmgr_vdev *vdev,
			       struct hdd_adapter *adapter,
			       tDot11fBeaconIEs *bcn_ie,
			       enum csr_akm_type auth_type)
{
	int ac;
	bool qap;
	bool qos_connection;
	uint8_t acm_mask = 0;
	struct vdev_mlme_obj *vdev_mlme;
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		hdd_err("vdev component object is NULL");
		return;
	}
	if (CSR_IS_QOS_BSS(bcn_ie) || bcn_ie->HTCaps.present)
		/* Some HT AP's dont send WMM IE so in that case we
		 * assume all HT Ap's are Qos Enabled AP's
		 */
		qap = true;
	else
		qap = false;

	qos_connection = vdev_mlme->ext_vdev_ptr->connect_info.qos_enabled;

	acm_mask = sme_qos_get_acm_mask(mac_ctx, NULL, bcn_ie);

	hdd_debug("qap is %d, qos_connection is %d, acm_mask is 0x%x",
		  qap, qos_connection, acm_mask);

	adapter->hdd_wmm_status.qap = qap;
	adapter->hdd_wmm_status.qos_connection = qos_connection;

	for (ac = 0; ac < WLAN_MAX_AC; ac++) {
		if (qap && qos_connection && (acm_mask & acm_mask_bit[ac])) {
			hdd_debug("ac %d on", ac);

			/* admission is required */
			adapter->hdd_wmm_status.ac_status[ac].
			is_access_required = true;
			adapter->hdd_wmm_status.ac_status[ac].
			is_access_allowed = false;
			adapter->hdd_wmm_status.ac_status[ac].
			was_access_granted = false;
			/* after reassoc if we have valid tspec, allow access */
			if (adapter->hdd_wmm_status.ac_status[ac].
			    is_tspec_valid &&
			    (adapter->hdd_wmm_status.ac_status[ac].
				tspec.ts_info.direction !=
				SME_QOS_WMM_TS_DIR_DOWNLINK)) {
				adapter->hdd_wmm_status.ac_status[ac].
				is_access_allowed = true;
			}
			if (!sme_neighbor_roam_is11r_assoc(
							mac_handle,
							adapter->vdev_id) &&
			    !hdd_is_ese_assoc(auth_type, bcn_ie, mac_ctx)) {
				adapter->hdd_wmm_status.ac_status[ac].
					is_tspec_valid = false;
				adapter->hdd_wmm_status.ac_status[ac].
					is_access_allowed = false;
			}
		} else {
			hdd_debug("ac %d off", ac);
			/* admission is not required so access is allowed */
			adapter->hdd_wmm_status.ac_status[ac].
			is_access_required = false;
			adapter->hdd_wmm_status.ac_status[ac].
			is_access_allowed = true;
		}
	}
}

static void hdd_cm_save_connect_info(struct hdd_adapter *adapter,
				     struct wlan_cm_connect_resp *rsp)
{
	struct hdd_station_ctx *sta_ctx;
	struct wlan_crypto_params *crypto_params;
	struct wlan_channel *des_chan;
	struct wlan_objmgr_vdev *vdev;
	uint8_t *ie_field;
	uint32_t ie_len, status;
	tDot11fBeaconIEs *bcn_ie;
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	bcn_ie = qdf_mem_malloc(sizeof(*bcn_ie));
	if (!bcn_ie)
		return;

	qdf_copy_macaddr(&sta_ctx->conn_info.bssid, &rsp->bssid);
	sta_ctx->conn_info.assoc_status_code = rsp->status_code;

	crypto_params = wlan_crypto_vdev_get_crypto_params(adapter->vdev);

	if (crypto_params) {
		sme_fill_enc_type(&sta_ctx->conn_info.uc_encrypt_type,
				  crypto_params->ucastcipherset);

		sme_fill_auth_type(&sta_ctx->conn_info.auth_type,
				   crypto_params->authmodeset,
				   crypto_params->key_mgmt,
				   crypto_params->ucastcipherset);
		sta_ctx->conn_info.last_auth_type =
					sta_ctx->conn_info.auth_type;
	}
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
	if (!rsp->connect_ies.bcn_probe_rsp.ptr ||
	    (rsp->connect_ies.bcn_probe_rsp.len <
	     (sizeof(struct wlan_frame_hdr) +
	      offsetof(struct wlan_bcn_frame, ie)))) {
		hdd_err("beacon len is invalid %d",
			rsp->connect_ies.bcn_probe_rsp.len);
		qdf_mem_free(bcn_ie);
		return;
	}

	ie_len = (rsp->connect_ies.bcn_probe_rsp.len -
			sizeof(struct wlan_frame_hdr) -
			offsetof(struct wlan_bcn_frame, ie));
	ie_field = (uint8_t *)(rsp->connect_ies.bcn_probe_rsp.ptr +
				sizeof(struct wlan_frame_hdr) +
				offsetof(struct wlan_bcn_frame, ie));

	status = dot11f_unpack_beacon_i_es(MAC_CONTEXT(mac_handle), ie_field,
					   ie_len, bcn_ie, false);

	if (DOT11F_FAILED(status)) {
		hdd_err("Failed to parse beacon ie");
		qdf_mem_free(bcn_ie);
		return;
	}
	if (bcn_ie->ExtCap.present) {
		struct s_ext_cap *p_ext_cap = (struct s_ext_cap *)
						bcn_ie->ExtCap.bytes;
		sta_ctx->conn_info.proxy_arp_service =
						p_ext_cap->proxy_arp_service;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (vdev) {
		sta_ctx->conn_info.nss = wlan_vdev_mlme_get_nss(vdev);
		ucfg_wlan_vdev_mgr_get_param(vdev, WLAN_MLME_CFG_RATE_FLAGS,
					     &sta_ctx->conn_info.rate_flags);
		hdd_wmm_cm_connect(vdev, adapter, bcn_ie,
				   sta_ctx->conn_info.auth_type);
		hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);
	}
	qdf_mem_free(bcn_ie);

	hdd_cm_save_bss_info(adapter, rsp);
}

#ifdef WLAN_FEATURE_FILS_SK
static bool hdd_cm_is_fils_connection(struct wlan_cm_connect_resp *rsp)
{
	return rsp->is_fils_connection;
}
#else
static inline
bool hdd_cm_is_fils_connection(struct wlan_cm_connect_resp *rsp)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static bool hdd_cm_is_roam_auth_required(struct hdd_station_ctx *sta_ctx,
					 struct wlan_cm_connect_resp *rsp)
{
	if (!rsp->roaming_info)
		return false;

	if (rsp->roaming_info->auth_status == ROAM_AUTH_STATUS_AUTHENTICATED)
		return false;

	return true;
}
#else
static bool hdd_cm_is_roam_auth_required(struct hdd_station_ctx *sta_ctx,
					 struct wlan_cm_connect_resp *rsp)
{
	return true;
}
#endif

#if defined(WLAN_FEATURE_11BE_MLO) && defined(CFG80211_11BE_BASIC)
static
struct hdd_adapter *hdd_get_assoc_link_adapter(struct hdd_adapter *ml_adapter)
{
	int i;

	for (i = 0; i < WLAN_MAX_MLD; i++) {
		if (hdd_adapter_is_associated_with_ml_adapter(
		    ml_adapter->mlo_adapter_info.link_adapter[i])) {
			return ml_adapter->mlo_adapter_info.link_adapter[i];
		}
	}
	return NULL;
}

#else
static
struct hdd_adapter *hdd_get_assoc_link_adapter(struct hdd_adapter *ml_adapter)
{
	return NULL;
}
#endif

static void
hdd_cm_connect_success_pre_user_update(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	struct vdev_mlme_obj *vdev_mlme;
	unsigned long rc;
	uint32_t ie_len;
	uint8_t *ie_field;
	mac_handle_t mac_handle;
	bool is_auth_required = true;
	bool is_roam_offload = false;
	bool is_roam = rsp->is_reassoc;
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t phymode;
	uint32_t time_buffer_size;
	struct hdd_adapter *assoc_link_adapter;

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

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	mac_handle = hdd_adapter_get_mac_handle(adapter);

	wlan_hdd_ft_set_key_delay(vdev);
	hdd_cm_update_rssi_snr_by_bssid(adapter);
	hdd_cm_save_connect_status(adapter, rsp->status_code);

	hdd_init_scan_reject_params(hdd_ctx);
	time_buffer_size = sizeof(sta_ctx->conn_info.connect_time);
	qdf_mem_zero(sta_ctx->conn_info.connect_time, time_buffer_size);
	qdf_get_time_of_the_day_in_hr_min_sec_usec(sta_ctx->conn_info.connect_time,
						   time_buffer_size);
	hdd_start_tsf_sync(adapter);
	hdd_cm_rec_connect_info(adapter, rsp);

	hdd_cm_save_connect_info(adapter, rsp);
	if (adapter->device_mode == QDF_STA_MODE &&
	    hdd_adapter_is_ml_adapter(adapter)) {
		/* Save connection info in assoc link adapter as well */
		assoc_link_adapter = hdd_get_assoc_link_adapter(adapter);
		if (assoc_link_adapter)
			hdd_cm_save_connect_info(assoc_link_adapter, rsp);
	}
	phymode = wlan_reg_get_max_phymode(hdd_ctx->pdev, REG_PHYMODE_MAX,
					   rsp->freq);
	sta_ctx->reg_phymode = csr_convert_from_reg_phy_mode(phymode);

	if (hdd_add_beacon_filter(adapter) != 0)
		hdd_err("add beacon fileter failed");

	adapter->wapi_info.is_wapi_sta = hdd_cm_is_wapi_sta(
						sta_ctx->conn_info.auth_type);
	if (adapter->device_mode == QDF_STA_MODE &&
	    rsp->connect_ies.bcn_probe_rsp.ptr &&
	    (rsp->connect_ies.bcn_probe_rsp.len >
	     (sizeof(struct wlan_frame_hdr) +
	      offsetof(struct wlan_bcn_frame, ie)))) {
		ie_len = (rsp->connect_ies.bcn_probe_rsp.len -
				sizeof(struct wlan_frame_hdr) -
				offsetof(struct wlan_bcn_frame, ie));

		ie_field  = (uint8_t *)(rsp->connect_ies.bcn_probe_rsp.ptr +
				sizeof(struct wlan_frame_hdr) +
				offsetof(struct wlan_bcn_frame, ie));
		sta_ctx->ap_supports_immediate_power_save =
				wlan_hdd_is_ap_supports_immediate_power_save(
				     ie_field, ie_len);
		hdd_debug("ap_supports_immediate_power_save flag [%d]",
			  sta_ctx->ap_supports_immediate_power_save);
	}
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

	if (!is_roam) {
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
	}

	if (!rsp->is_wps_connection &&
	    (sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_NONE ||
	     sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_OPEN_SYSTEM ||
	     sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_SHARED_KEY ||
	     hdd_cm_is_fils_connection(rsp)))
		is_auth_required = false;

	if (is_roam)
		/* If roaming is set check if FW roaming/LFR3  */
		ucfg_mlme_get_roaming_offload(hdd_ctx->psoc, &is_roam_offload);

	if (is_roam_offload)
		/* for LFR 3 get authenticated info from resp */
		is_auth_required = hdd_cm_is_roam_auth_required(sta_ctx, rsp);

	if (ucfg_ipa_is_enabled() && !is_auth_required)
		ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode,
				  adapter->vdev_id,
				  WLAN_IPA_STA_CONNECT,
				  rsp->bssid.bytes,
				  WLAN_REG_IS_24GHZ_CH_FREQ(
					sta_ctx->conn_info.chan_freq));

	if (adapter->device_mode == QDF_STA_MODE)
		cdp_reset_rx_hw_ext_stats(soc);

	wlan_hdd_auto_shutdown_enable(hdd_ctx, false);

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
		adapter->vdev_id,
		QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_ASSOC));

	if (is_roam)
		hdd_nud_indicate_roam(adapter);
	 /* hdd_objmgr_set_peer_mlme_auth_state */
}

static void
hdd_cm_connect_success_post_user_update(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct vdev_mlme_obj *mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	uint8_t uapsd_mask = 0;
	bool is_auth_required = true;
	bool is_roam_offload = false;
	bool is_roam = rsp->is_reassoc;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rsp->vdev_id);
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", rsp->vdev_id);
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (mlme_obj)
		uapsd_mask =
			mlme_obj->ext_vdev_ptr->connect_info.uapsd_per_ac_bitmask;
	if (is_roam) {
		/* If roaming is set check if FW roaming/LFR3  */
		ucfg_mlme_get_roaming_offload(hdd_ctx->psoc, &is_roam_offload);
	} else {
		/* call only for connect */
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_CONNECT);
	}

	cdp_hl_fc_set_td_limit(soc, adapter->vdev_id,
			       sta_ctx->conn_info.chan_freq);
	hdd_wmm_assoc(adapter, false, uapsd_mask);

	if (!rsp->is_wps_connection &&
	    (sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_NONE ||
	     sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_OPEN_SYSTEM ||
	     sta_ctx->conn_info.auth_type == eCSR_AUTH_TYPE_SHARED_KEY ||
	     hdd_cm_is_fils_connection(rsp)))
		is_auth_required = false;

	if (is_roam_offload || !is_roam) {
		/* For FW_ROAM/LFR3 OR connect */
		/* for LFR 3 get authenticated info from resp */
		if (is_roam)
			is_auth_required =
				hdd_cm_is_roam_auth_required(sta_ctx, rsp);
		hdd_roam_register_sta(adapter, &rsp->bssid, is_auth_required);
	} else {
		/* for host roam/LFR2 */
		hdd_cm_set_peer_authenticate(adapter, &rsp->bssid,
					     is_auth_required);
	}

	hdd_debug("Enabling queues");
	hdd_cm_netif_queue_enable(adapter);

	hdd_cm_clear_pmf_stats(adapter);

	if (adapter->device_mode == QDF_STA_MODE) {
		/* Inform FTM TIME SYNC about the connection with AP */
		hdd_ftm_time_sync_sta_state_notify(adapter,
						   FTM_TIME_SYNC_STA_CONNECTED);
		ucfg_mlme_init_twt_context(hdd_ctx->psoc,
					   &rsp->bssid,
					   TWT_ALL_SESSIONS_DIALOG_ID);
		ucfg_twt_init_context(hdd_ctx->psoc,
				      &rsp->bssid,
				      TWT_ALL_SESSIONS_DIALOG_ID);
	}
	hdd_periodic_sta_stats_start(adapter);
	wlan_twt_concurrency_update(hdd_ctx);
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

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS hdd_cm_save_gtk(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_resp *rsp)
{
	uint8_t *kek;
	uint32_t kek_len;
	uint8_t *kck = NULL;
	uint8_t kck_len = 0;
	uint8_t replay_ctr_def[REPLAY_CTR_LEN] = {0};
	uint8_t *replay_ctr;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rsp->vdev_id);
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", rsp->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	if (rsp->is_reassoc && rsp->roaming_info) {
		kek = rsp->roaming_info->kek;
		kek_len = rsp->roaming_info->kek_len;
		kck = rsp->roaming_info->kck;
		kck_len = rsp->roaming_info->kck_len;
		replay_ctr = rsp->roaming_info->replay_ctr;
	} else if (rsp->connect_ies.fils_ie) {
		kek = rsp->connect_ies.fils_ie->kek;
		kek_len = rsp->connect_ies.fils_ie->kek_len;
		replay_ctr = replay_ctr_def;
	} else {
		return QDF_STATUS_SUCCESS;
	}
	wlan_hdd_save_gtk_offload_params(adapter, kck, kck_len, kek, kek_len,
					 replay_ctr, true);

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS hdd_cm_save_gtk(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_resp *rsp)
{
	uint8_t *kek;
	uint32_t kek_len;
	uint8_t *kck = NULL;
	uint8_t kck_len = 0;
	uint8_t *replay_ctr;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, rsp->vdev_id);
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", rsp->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	if (rsp->is_reassoc && rsp->roaming_info) {
		kek = rsp->roaming_info.kek;
		kek_len = rsp->roaming_info.kek_len;
		kck = rsp->roaming_info.kck;
		kck_len = rsp->roaming_info.kck_len;
		replay_ctr = rsp->roaming_info.replay_ctr;
	} else {
		return QDF_STATUS_SUCCESS;
	}
	wlan_hdd_save_gtk_offload_params(adapter, kck, kck_len, kek, kek_len,
					 replay_ctr, true);

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_FILS_SK*/
#else
QDF_STATUS hdd_cm_save_gtk(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_resp *rsp)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
static void hdd_update_hlp_info(struct net_device *dev,
				struct wlan_cm_connect_resp *rsp)
{
	struct sk_buff *skb;
	uint16_t skb_len;
	struct llc_snap_hdr_t *llc_hdr;
	QDF_STATUS status;
	uint8_t *hlp_data;
	uint16_t hlp_data_len;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	if (!rsp || !rsp->connect_ies.fils_ie) {
		return;
	}

	if (!rsp->connect_ies.fils_ie->hlp_data_len) {
		return;
	}

	hlp_data = rsp->connect_ies.fils_ie->hlp_data;
	hlp_data_len = rsp->connect_ies.fils_ie->hlp_data_len;

	/* Calculate skb length */
	skb_len = (2 * ETH_ALEN) + hlp_data_len;
	skb = qdf_nbuf_alloc(NULL, skb_len, 0, 4, false);
	if (!skb) {
		hdd_err("HLP packet nbuf alloc fails");
		return;
	}

	qdf_mem_copy(skb_put(skb, ETH_ALEN),
		     rsp->connect_ies.fils_ie->dst_mac.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(skb_put(skb, ETH_ALEN),
		     rsp->connect_ies.fils_ie->src_mac.bytes,
		     QDF_MAC_ADDR_SIZE);

	llc_hdr = (struct llc_snap_hdr_t *)hlp_data;
	if (IS_SNAP(llc_hdr)) {
		hlp_data += LLC_SNAP_HDR_OFFSET_ETHERTYPE;
		hlp_data_len += LLC_SNAP_HDR_OFFSET_ETHERTYPE;
	}

	qdf_mem_copy(skb_put(skb, hlp_data_len), hlp_data, hlp_data_len);

	/*
	 * This HLP packet is formed from HLP info encapsulated
	 * in assoc response frame which is AEAD encrypted.
	 * Hence, this checksum validation can be set unnecessary.
	 * i.e. network layer need not worry about checksum.
	 */
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	status = hdd_rx_packet_cbk(adapter, skb);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Sending HLP packet fails");
		return;
	}
	hdd_debug("send HLP packet to netif successfully");
}

QDF_STATUS hdd_cm_set_hlp_data(struct net_device *dev,
			       struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *rsp)
{
	hdd_update_hlp_info(dev, rsp);
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
/**
 * hdd_cm_get_ft_preauth_response() - get ft preauth response
 * related information
 * @vdev: vdev pointer
 * @rsp: preauth response
 * @ft_ie: ft ie
 * @ft_ie_ip_len: ft ie ip length
 * @ft_ie_length: ft ies length
 *
 * This function is used to get ft ie related information
 *
 * Return: none
 */
static void
hdd_cm_get_ft_preauth_response(struct wlan_objmgr_vdev *vdev,
			       struct wlan_preauth_rsp *rsp, uint8_t *ft_ie,
			       uint32_t ft_ie_ip_len, uint16_t *ft_ie_length)
{
	struct mlme_legacy_priv *mlme_priv;

	*ft_ie_length = 0;

	if (!vdev)
		return;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		return;

	/* All or nothing - proceed only if both BSSID and FT IE fit */
	if ((QDF_MAC_ADDR_SIZE + rsp->ft_ie_length) > ft_ie_ip_len)
		return;
	/*
	 * hdd needs to pack the bssid also along with the
	 * auth response to supplicant
	 */
	qdf_mem_copy(ft_ie, rsp->pre_auth_bssid.bytes, QDF_MAC_ADDR_SIZE);

	/* Copy the auth resp FTIEs */
	qdf_mem_copy(&ft_ie[QDF_MAC_ADDR_SIZE],
		     rsp->ft_ie, rsp->ft_ie_length);

	*ft_ie_length = QDF_MAC_ADDR_SIZE + rsp->ft_ie_length;

	hdd_debug("Filled auth resp: %d", *ft_ie_length);
}

#if defined(KERNEL_SUPPORT_11R_CFG80211)
QDF_STATUS hdd_cm_ft_preauth_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_preauth_rsp *rsp)
{
	mac_handle_t mac_handle;
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	struct wireless_dev *wdev;
	uint16_t auth_resp_len = 0;
	uint32_t ric_ies_length = 0;
	struct cfg80211_ft_event_params ft_event;
	uint8_t ft_ie[DOT11F_IE_FTINFO_MAX_LEN];
	uint8_t ric_ies[DOT11F_IE_RICDESCRIPTOR_MAX_LEN];

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (!mac_handle) {
		hdd_err("mac_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!osif_priv) {
		hdd_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		hdd_err("wdev is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_zero(ft_ie, DOT11F_IE_FTINFO_MAX_LEN);
	qdf_mem_zero(ric_ies, DOT11F_IE_RICDESCRIPTOR_MAX_LEN);

	if (rsp->ric_ies_length &&
	    rsp->ric_ies_length <= DOT11F_IE_RICDESCRIPTOR_MAX_LEN) {
		qdf_mem_copy(ric_ies, rsp->ric_ies, rsp->ric_ies_length);
		ric_ies_length = rsp->ric_ies_length;
	} else {
		hdd_warn("Do not send RIC IEs as length is 0");
	}

	ft_event.ric_ies = ric_ies;
	ft_event.ric_ies_len = ric_ies_length;
	hdd_debug("RIC IEs is of length %d", ric_ies_length);

	hdd_cm_get_ft_preauth_response(vdev, rsp, ft_ie,
				       DOT11F_IE_FTINFO_MAX_LEN,
				       &auth_resp_len);
	if (!auth_resp_len) {
		hdd_debug("AuthRsp FTIES is of length 0");
		return QDF_STATUS_E_FAILURE;
	}

	ucfg_cm_set_ft_pre_auth_state(vdev, true);

	ft_event.target_ap = ft_ie;
	ft_event.ies = (u8 *)(ft_ie + QDF_MAC_ADDR_SIZE);
	ft_event.ies_len = auth_resp_len - QDF_MAC_ADDR_SIZE;

	hdd_debug("ftEvent.ies_len %zu", ft_event.ies_len);
	hdd_debug("ftEvent.ric_ies_len %zu", ft_event.ric_ies_len);
	hdd_debug("ftEvent.target_ap %2x-%2x-%2x-%2x-%2x-%2x",
		  ft_event.target_ap[0], ft_event.target_ap[1],
		  ft_event.target_ap[2], ft_event.target_ap[3],
		  ft_event.target_ap[4], ft_event.target_ap[5]);

	(void)cfg80211_ft_event(wdev->netdev, &ft_event);

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS hdd_cm_ft_preauth_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_preauth_rsp *rsp)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	struct wireless_dev *wdev;
	uint16_t auth_resp_len = 0;
	uint32_t ric_ies_length = 0;
	char *buff;
	union iwreq_data wrqu;
	uint16_t str_len;

	if (!osif_priv) {
		hdd_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		hdd_err("wdev is null");
		return QDF_STATUS_E_INVAL;
	}

	/* need to send the IEs to the supplicant */
	buff = qdf_mem_malloc(IW_CUSTOM_MAX);
	if (!buff)
		return QDF_STATUS_E_NOMEM;

	/* need to send the RIC IEs first */
	str_len = strlcpy(buff, "RIC=", IW_CUSTOM_MAX);
	if (rsp->ric_ies_length &&
	    (rsp->ric_ies_length <= (IW_CUSTOM_MAX - str_len))) {
		qdf_mem_copy(&buff[str_len], rsp->ric_ies,
			     rsp->ric_ies_length);
		ric_ies_length = rsp->ric_ies_length;
		wrqu.data.length = str_len + ric_ies_length;
		hdd_wext_send_event(wdev->netdev, IWEVCUSTOM, &wrqu, buff);
	} else {
		hdd_warn("Do not send RIC IEs as length is 0");
	}

	/* need to provide the Auth Resp */
	qdf_mem_zero(buff, IW_CUSTOM_MAX);
	str_len = strlcpy(buff, "AUTH=", IW_CUSTOM_MAX);
	hdd_cm_get_ft_preauth_response(vdev, rsp, &buff[str_len],
				       (IW_CUSTOM_MAX - str_len),
				       &auth_resp_len);
	if (!auth_resp_len) {
		qdf_mem_free(buff);
		hdd_debug("AuthRsp FTIES is of length 0");
		return QDF_STATUS_E_FAILURE;
	}

	wrqu.data.length = str_len + auth_resp_len;
	hdd_wext_send_event(wdev->netdev, IWEVCUSTOM, &wrqu, buff);

	qdf_mem_free(buff);
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef FEATURE_WLAN_ESE
QDF_STATUS hdd_cm_cckm_preauth_complete(struct wlan_objmgr_vdev *vdev,
					struct wlan_preauth_rsp *rsp)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	struct wireless_dev *wdev;
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	char *pos = buf;
	int nbytes = 0, freebytes = IW_CUSTOM_MAX;

	if (!osif_priv) {
		hdd_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		hdd_err("wdev is null");
		return QDF_STATUS_E_INVAL;
	}

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	/* timestamp0 is lower 32 bits and timestamp1 is upper 32 bits */
	hdd_debug("CCXPREAUTHNOTIFY=" QDF_MAC_ADDR_FMT " %d:%d",
		  QDF_MAC_ADDR_REF(rsp->pre_auth_bssid.bytes),
		  rsp->timestamp[0], rsp->timestamp[1]);

	nbytes = snprintf(pos, freebytes, "CCXPREAUTHNOTIFY=");
	pos += nbytes;
	freebytes -= nbytes;

	qdf_mem_copy(pos, rsp->pre_auth_bssid.bytes, QDF_MAC_ADDR_SIZE);
	pos += QDF_MAC_ADDR_SIZE;
	freebytes -= QDF_MAC_ADDR_SIZE;

	nbytes = snprintf(pos, freebytes, " %u:%u",
			  rsp->timestamp[0], rsp->timestamp[1]);
	freebytes -= nbytes;

	wrqu.data.pointer = buf;
	wrqu.data.length = (IW_CUSTOM_MAX - freebytes);

	/* send the event */
	hdd_wext_send_event(wdev->netdev, IWEVCUSTOM, &wrqu, buf);

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_ESE */
#endif /* WLAN_FEATURE_PREAUTH_ENABLE */
