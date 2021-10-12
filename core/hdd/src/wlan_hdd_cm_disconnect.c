/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
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
 * DOC: hdd_cm_disconnect.c
 *
 * WLAN Host Device Driver disconnect APIs implementation
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_hdd_trace.h"
#include <osif_cm_req.h>
#include "wlan_hdd_cm_api.h"
#include "wlan_ipa_ucfg_api.h"
#include "wlan_hdd_periodic_sta_stats.h"
#include "wlan_hdd_stats.h"
#include "wlan_hdd_scan.h"
#include "sme_power_save_api.h"
#include <wlan_logging_sock_svc.h>
#include "wlan_hdd_ftm_time_sync.h"
#include "wlan_hdd_bcn_recv.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_ipa.h"
#include "wlan_hdd_green_ap.h"
#include "wlan_hdd_lpass.h"
#include "wlan_hdd_bootup_marker.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_crypto_global_api.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "hif.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_napi.h"
#include "wlan_hdd_cfr.h"
#include "wlan_roam_debug.h"
#include "wma_api.h"

void hdd_handle_disassociation_event(struct hdd_adapter *adapter,
				     struct qdf_mac_addr *peer_macaddr)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);

	hdd_green_ap_start_state_mc(hdd_ctx, adapter->device_mode, false);

	wlan_hdd_auto_shutdown_enable(hdd_ctx, true);

	if ((adapter->device_mode == QDF_STA_MODE) ||
	    (adapter->device_mode == QDF_P2P_CLIENT_MODE))
		/* send peer status indication to oem app */
		hdd_send_peer_status_ind_to_app(peer_macaddr,
						ePeerDisconnected, 0,
						adapter->vdev_id, NULL,
						adapter->device_mode);

	hdd_lpass_notify_disconnect(adapter);

	hdd_del_latency_critical_client(
		adapter,
		hdd_convert_cfgdot11mode_to_80211mode(
			sta_ctx->conn_info.dot11mode));
	/* stop timer in sta/p2p_cli */
	hdd_bus_bw_compute_reset_prev_txrx_stats(adapter);
	hdd_bus_bw_compute_timer_try_stop(hdd_ctx);
	cdp_display_txrx_hw_info(soc);
}

/**
 * hdd_cm_print_bss_info() - print bss info
 * @hdd_sta_ctx: pointer to hdd station context
 *
 * Return: None
 */
static void hdd_cm_print_bss_info(struct hdd_station_ctx *hdd_sta_ctx)
{
	uint32_t *ht_cap_info;
	uint32_t *vht_cap_info;
	struct hdd_connection_info *conn_info;

	conn_info = &hdd_sta_ctx->conn_info;

	hdd_nofl_debug("*********** WIFI DATA LOGGER **************");
	hdd_nofl_debug("freq: %d dot11mode %d AKM %d ssid: \"%.*s\" ,roam_count %d nss %d legacy %d mcs %d signal %d noise: %d",
		       conn_info->chan_freq, conn_info->dot11mode,
		       conn_info->last_auth_type,
		       conn_info->last_ssid.SSID.length,
		       conn_info->last_ssid.SSID.ssId, conn_info->roam_count,
		       conn_info->txrate.nss, conn_info->txrate.legacy,
		       conn_info->txrate.mcs, conn_info->signal,
		       conn_info->noise);
	ht_cap_info = (uint32_t *)&conn_info->ht_caps;
	vht_cap_info = (uint32_t *)&conn_info->vht_caps;
	hdd_nofl_debug("HT 0x%x VHT 0x%x ht20 info 0x%x",
		       conn_info->conn_flag.ht_present ? *ht_cap_info : 0,
		       conn_info->conn_flag.vht_present ? *vht_cap_info : 0,
		       conn_info->conn_flag.hs20_present ?
		       conn_info->hs20vendor_ie.release_num : 0);
}

void __hdd_cm_disconnect_handler_pre_user_update(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	uint32_t time_buffer_size;

	hdd_stop_tsf_sync(adapter);
	time_buffer_size = sizeof(sta_ctx->conn_info.connect_time);
	qdf_mem_zero(sta_ctx->conn_info.connect_time, time_buffer_size);
	if (ucfg_ipa_is_enabled() &&
	    QDF_IS_STATUS_SUCCESS(wlan_hdd_validate_mac_address(
				  &sta_ctx->conn_info.bssid)))
		ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode,
				  adapter->vdev_id,
				  WLAN_IPA_STA_DISCONNECT,
				  sta_ctx->conn_info.bssid.bytes,
				  false);

	hdd_periodic_sta_stats_stop(adapter);
	wlan_hdd_auto_shutdown_enable(hdd_ctx, true);

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
		adapter->vdev_id,
		QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_DISASSOC));

	hdd_wmm_dscp_initial_state(adapter);
	wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);

	hdd_place_marker(adapter, "DISCONNECTED", NULL);
}

void __hdd_cm_disconnect_handler_post_user_update(struct hdd_adapter *adapter,
						  struct wlan_objmgr_vdev *vdev)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	mac_handle_t mac_handle;

	mac_handle = hdd_ctx->mac_handle;

	/* update P2P connection status */
	ucfg_p2p_status_disconnect(vdev);
	hdd_cfr_disconnect(adapter->vdev);

	hdd_wmm_adapter_clear(adapter);
	ucfg_cm_ft_reset(vdev);
	ucfg_cm_reset_key(hdd_ctx->pdev, adapter->vdev_id);
	hdd_clear_roam_profile_ie(adapter);

	if (adapter->device_mode == QDF_STA_MODE)
		wlan_crypto_reset_vdev_params(vdev);

	hdd_remove_beacon_filter(adapter);
	if (sme_is_beacon_report_started(mac_handle, adapter->vdev_id)) {
		hdd_debug("Sending beacon pause indication to userspace");
		hdd_beacon_recv_pause_indication((hdd_handle_t)hdd_ctx,
						 adapter->vdev_id,
						 SCAN_EVENT_TYPE_MAX, true);
	}

	/* Clear saved connection information in HDD */
	hdd_conn_remove_connect_info(sta_ctx);

	/* Setting the RTS profile to original value */
	if (sme_cli_set_command(adapter->vdev_id, WMI_VDEV_PARAM_ENABLE_RTSCTS,
				cfg_get(hdd_ctx->psoc,
					CFG_ENABLE_FW_RTS_PROFILE),
				VDEV_CMD))
		hdd_debug("Failed to set RTS_PROFILE");

	hdd_init_scan_reject_params(hdd_ctx);
	ucfg_pmo_flush_gtk_offload_req(vdev);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
		sme_ps_disable_auto_ps_timer(mac_handle,
					     adapter->vdev_id);
		adapter->send_mode_change = true;
	}
	wlan_hdd_clear_link_layer_stats(adapter);

	adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;

	hdd_nud_reset_tracking(adapter);
	hdd_reset_limit_off_chan(adapter);

	hdd_cm_print_bss_info(sta_ctx);
}

#ifdef WLAN_FEATURE_MSCS
void reset_mscs_params(struct hdd_adapter *adapter)
{
	mlme_set_is_mscs_req_sent(adapter->vdev, false);
	adapter->mscs_counter = 0;
}
#endif

QDF_STATUS wlan_hdd_cm_issue_disconnect(struct hdd_adapter *adapter,
					enum wlan_reason_code reason,
					bool sync)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;
	void *hif_ctx;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_OSIF_CM_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;
	hdd_place_marker(adapter, "TRY TO DISCONNECT", NULL);
	reset_mscs_params(adapter);
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (hif_ctx)
		/*
		 * Trigger runtime sync resume before sending disconneciton
		 */
		hif_pm_runtime_sync_resume(hif_ctx, RTPM_ID_CONN_DISCONNECT);

	wlan_rec_conn_info(adapter->vdev_id, DEBUG_CONN_DISCONNECT,
			   sta_ctx->conn_info.bssid.bytes, 0, reason);

	if (sync)
		status = osif_cm_disconnect_sync(vdev, reason);
	else
		status = osif_cm_disconnect(adapter->dev, vdev, reason);

	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_CM_ID);

	return status;
}

int wlan_hdd_cm_disconnect(struct wiphy *wiphy,
			   struct net_device *dev, u16 reason)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;
	int ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	if (hdd_ctx->is_wiphy_suspended) {
		hdd_info_rl("wiphy is suspended retry disconnect");
		return -EAGAIN;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_DISCONNECT,
		   adapter->vdev_id, reason);

	hdd_print_netdev_txq_status(dev);

	if (reason == WLAN_REASON_DEAUTH_LEAVING)
		qdf_dp_trace_dump_all(
				WLAN_DEAUTH_DPTRACE_DUMP_COUNT,
				QDF_TRACE_DEFAULT_PDEV_ID);
	/*
	 * for Supplicant initiated disconnect always wait for complete,
	 * as for WPS connection or back to back connect, supplicant initiate a
	 * disconnect which is followed by connect and if kernel is not yet
	 * disconnected, this new connect will be rejected by kernel with status
	 * EALREADY. In case connect is rejected with EALREADY, supplicant will
	 * queue one more disconnect followed by connect immediately, Now if
	 * driver is not disconnected by this time, the kernel will again reject
	 * connect and thus the failing the connect req in supplicant.
	 * Thus we need to wait for disconnect to complete in this case,
	 * and thus use sync API here.
	 */
	status = wlan_hdd_cm_issue_disconnect(adapter, reason, true);

	return qdf_status_to_os_return(status);
}

static QDF_STATUS
hdd_cm_disconnect_complete_pre_user_update(struct wlan_objmgr_vdev *vdev,
					   struct wlan_cm_discon_rsp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_INVAL;
	}
	hdd_napi_serialize(0);
	hdd_disable_and_flush_mc_addr_list(adapter, pmo_peer_disconnect);
	__hdd_cm_disconnect_handler_pre_user_update(adapter);

	hdd_handle_disassociation_event(adapter, &rsp->req.req.bssid);

	wlan_rec_conn_info(adapter->vdev_id, DEBUG_CONN_DISCONNECT_HANDLER,
			   rsp->req.req.bssid.bytes,
			   rsp->req.cm_id,
			   rsp->req.req.reason_code << 16 |
			   rsp->req.req.source);
	hdd_ipa_set_tx_flow_info();
	/*
	 * Convert and cache internal reason code in adapter. This can be
	 * sent to userspace with a vendor event.
	 */
	adapter->last_disconnect_reason =
			osif_cm_mac_to_qca_reason(rsp->req.req.reason_code);

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_cm_set_default_wlm_mode - reset the default wlm mode if
 *				 wlm_latency_reset_on_disconnect is set.
 *@adapter: adapter pointer
 *
 * return: None.
 */
static void hdd_cm_set_default_wlm_mode(struct hdd_adapter *adapter)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	bool reset;
	uint8_t def_level;
	mac_handle_t mac_handle;
	uint16_t vdev_id;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	status = ucfg_mlme_cfg_get_wlm_reset(hdd_ctx->psoc, &reset);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not get wlm reset flag");
		return;
	}
	if (!reset)
		return;

	status = ucfg_mlme_cfg_get_wlm_level(hdd_ctx->psoc, &def_level);
	if (QDF_IS_STATUS_ERROR(status))
		def_level = QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL;

	mac_handle = hdd_ctx->mac_handle;
	vdev_id = adapter->vdev_id;

	status = sme_set_wlm_latency_level(mac_handle, vdev_id, def_level);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		hdd_debug("reset wlm mode %x on disconnection", def_level);
		adapter->latency_level = def_level;
	} else {
		hdd_err("reset wlm mode failed: %d", status);
	}
}

/**
 * hdd_cm_reset_udp_qos_upgrade_config() - Reset the threshold for UDP packet
 * QoS upgrade.
 * @adapter: adapter for which this configuration is to be applied
 *
 * Return: None
 */
static void hdd_cm_reset_udp_qos_upgrade_config(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	bool reset;
	QDF_STATUS status;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	status = ucfg_mlme_cfg_get_wlm_reset(hdd_ctx->psoc, &reset);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not get the wlm reset flag");
		return;
	}

	if (reset) {
		adapter->upgrade_udp_qos_threshold = QCA_WLAN_AC_BK;
		hdd_debug("UDP packets qos upgrade to: %d",
			  adapter->upgrade_udp_qos_threshold);
	}
}

static QDF_STATUS
hdd_cm_disconnect_complete_post_user_update(struct wlan_objmgr_vdev *vdev,
					    struct wlan_cm_discon_rsp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_INVAL;
	}
	if (adapter->device_mode == QDF_STA_MODE) {
	/* Inform FTM TIME SYNC about the disconnection with the AP */
		hdd_ftm_time_sync_sta_state_notify(
				adapter, FTM_TIME_SYNC_STA_DISCONNECTED);
	}

	hdd_cm_set_default_wlm_mode(adapter);
	__hdd_cm_disconnect_handler_post_user_update(adapter, vdev);
	wlan_twt_concurrency_update(hdd_ctx);
	hdd_cm_reset_udp_qos_upgrade_config(adapter);

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_RUNTIME_PM
static void
wlan_hdd_runtime_pm_wow_disconnect_handler(struct hdd_context *hdd_ctx)
{
	struct hif_opaque_softc *hif_ctx;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("hif_ctx is NULL");
		return;
	}

	if (hdd_is_any_sta_connected(hdd_ctx)) {
		hdd_debug("active connections: runtime pm prevented: %d",
			  hdd_ctx->runtime_pm_prevented);
		return;
	}

	hdd_debug("Runtime allowed : %d", hdd_ctx->runtime_pm_prevented);
	qdf_spin_lock_irqsave(&hdd_ctx->pm_qos_lock);
	if (hdd_ctx->runtime_pm_prevented) {
		hif_pm_runtime_put(hif_ctx, RTPM_ID_QOS_NOTIFY);
		hdd_ctx->runtime_pm_prevented = false;
	}
	qdf_spin_unlock_irqrestore(&hdd_ctx->pm_qos_lock);
}
#else
static void
wlan_hdd_runtime_pm_wow_disconnect_handler(struct hdd_context *hdd_ctx)
{
}
#endif

QDF_STATUS hdd_cm_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_cm_discon_rsp *rsp,
				      enum osif_cb_type type)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	switch (type) {
	case OSIF_PRE_USERSPACE_UPDATE:
		return hdd_cm_disconnect_complete_pre_user_update(vdev, rsp);
	case OSIF_POST_USERSPACE_UPDATE:
		hdd_debug("Wifi disconnected: vdev id %d",
			  vdev->vdev_objmgr.vdev_id);
		wlan_hdd_runtime_pm_wow_disconnect_handler(hdd_ctx);

		return hdd_cm_disconnect_complete_post_user_update(vdev, rsp);
	default:
		hdd_cm_disconnect_complete_pre_user_update(vdev, rsp);
		hdd_cm_disconnect_complete_post_user_update(vdev, rsp);
		return QDF_STATUS_SUCCESS;
	}
}

QDF_STATUS hdd_cm_netif_queue_control(struct wlan_objmgr_vdev *vdev,
				      enum netif_action_type action,
				      enum netif_reason_type reason)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, wlan_vdev_get_id(vdev));
	if (!adapter) {
		hdd_err("adapter is NULL for vdev %d", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_INVAL;
	}

	wlan_hdd_netif_queue_control(adapter, action, reason);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_cm_napi_serialize_control(bool action)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	hdd_napi_serialize(action);

	/* reinit scan reject parms for napi off (roam abort/ho fail) */
	if (!action)
		hdd_init_scan_reject_params(hdd_ctx);

	return QDF_STATUS_SUCCESS;
}
