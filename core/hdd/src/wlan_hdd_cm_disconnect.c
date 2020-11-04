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

int wlan_hdd_cm_disconnect(struct wiphy *wiphy,
			   struct net_device *dev, u16 reason)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;
	struct wlan_objmgr_vdev *vdev;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (!vdev)
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_DISCONNECT,
		   adapter->vdev_id, reason);

	hdd_print_netdev_txq_status(dev);

	if (reason == WLAN_REASON_DEAUTH_LEAVING)
		qdf_dp_trace_dump_all(
				WLAN_DEAUTH_DPTRACE_DUMP_COUNT,
				QDF_TRACE_DEFAULT_PDEV_ID);

	/* To-Do: This is static api move this api to this file
	 * once this is removed from wlan_hdd_cfg80211.c file
	 * reset_mscs_params(adapter);
	 */
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);
	status = osif_cm_disconnect(dev, vdev, reason);
	hdd_objmgr_put_vdev(vdev);

	return status;
}

static QDF_STATUS
hdd_cm_disconnect_complete_pre_user_update(struct wlan_objmgr_vdev *vdev,
					   struct wlan_cm_discon_rsp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx,
					wlan_vdev_get_id(vdev));
	mac_handle_t mac_handle = hdd_ctx->mac_handle;

	if (ucfg_ipa_is_enabled() &&
	    QDF_IS_STATUS_SUCCESS(wlan_hdd_validate_mac_address(
				  &rsp->req.req.bssid)))
		ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode,
				  adapter->vdev_id,
				  WLAN_IPA_STA_DISCONNECT,
				  rsp->req.req.bssid.bytes);

	hdd_periodic_sta_stats_stop(adapter);

	wlan_hdd_auto_shutdown_enable(hdd_ctx, true);

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
				      adapter->vdev_id,
				      QDF_TRACE_DEFAULT_PDEV_ID,
				      QDF_PROTO_TYPE_MGMT,
				      QDF_PROTO_MGMT_DISASSOC));

	hdd_wmm_dscp_initial_state(adapter);
	wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);

	/*
	 * indicate 'disconnect' status to wpa_supplicant.
	 * hdd_send_association_event check this api and get the required code
	 * hdd_bus_bw_compute_reset_prev_txrx_stats,
	 * hdd_lpass_notify_disconnect, hdd_send_peer_status_ind_to_app,
	 * hdd_bus_bw_compute_timer_try_stop, hdd_ipa_set_tx_flow_info,
	 * update P2P connection status
	 */

	if (adapter->device_mode == QDF_STA_MODE) {
	/* Inform FTM TIME SYNC about the disconnection with the AP */
		hdd_ftm_time_sync_sta_state_notify(
				adapter, FTM_TIME_SYNC_STA_DISCONNECTED);
	}
	hdd_wmm_adapter_clear(adapter);

	if (sme_is_beacon_report_started(mac_handle, adapter->vdev_id)) {
		hdd_debug("Sending beacon pause indication to userspace");
		hdd_beacon_recv_pause_indication((hdd_handle_t)hdd_ctx,
						 adapter->vdev_id,
						 SCAN_EVENT_TYPE_MAX, true);
	}
	/*
	 * Clear saved connection information in HDD
	 * hdd_conn_remove_connect_info(sta_ctx);, this is an static api,
	 * Move this api here once this path is enabled.
	 */

	hdd_init_scan_reject_params(hdd_ctx);
	ucfg_pmo_flush_gtk_offload_req(adapter->vdev);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
		sme_ps_disable_auto_ps_timer(mac_handle,
					     adapter->vdev_id);
		adapter->send_mode_change = true;
	}
	wlan_hdd_clear_link_layer_stats(adapter);

	adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
hdd_cm_disconnect_complete_post_user_update(struct wlan_objmgr_vdev *vdev,
					    struct wlan_cm_discon_rsp *rsp)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx,
					wlan_vdev_get_id(vdev));

	hdd_nud_reset_tracking(adapter);
	hdd_reset_limit_off_chan(adapter);

	/*
	 * Print bss sta info
	 * hdd_print_bss_info(sta_ctx); this is static function, enable this
	 * once this path is enabled.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_cm_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct wlan_cm_discon_rsp *rsp,
				      enum osif_cb_type type)
{
	switch (type) {
	case OSIF_PRE_USERSPACE_UPDATE:
		return hdd_cm_disconnect_complete_pre_user_update(vdev, rsp);
	case OSIF_POST_USERSPACE_UPDATE:
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
	return QDF_STATUS_SUCCESS;
}

