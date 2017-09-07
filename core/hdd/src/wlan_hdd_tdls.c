/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_tdls.c
 *
 * WLAN Host Device Driver implementation for TDLS
 */

#include <wlan_hdd_includes.h>
#include <ani_global.h>
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_trace.h>
#include <net/cfg80211.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_assoc.h"
#include "sme_api.h"
#include "cds_sched.h"
#include "wma_types.h"
#include "wlan_policy_mgr_api.h"
#include <qca_vendor.h>
#include "wlan_hdd_ipa.h"

/**
 * enum qca_wlan_vendor_tdls_trigger_mode_hdd_map: Maps the user space TDLS
 *	trigger mode in the host driver.
 * @WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT: TDLS Connection and
 *	disconnection handled by user space.
 * @WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT: TDLS connection and
 *	disconnection controlled by host driver based on data traffic.
 * @WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL: TDLS connection and
 *	disconnection jointly controlled by user space and host driver.
 */
enum qca_wlan_vendor_tdls_trigger_mode_hdd_map {
	WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT =
		QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT,
	WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT =
		QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT,
	WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL =
		((QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT |
		  QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT) << 1),
};

/*
 * wlan_hdd_tdls_determine_channel_opclass() - determine channel and opclass
 * @hddctx: pointer to hdd context
 * @adapter: pointer to adapter
 * @curr_peer: pointer to current tdls peer
 * @channel: pointer to channel
 * @opclass: pointer to opclass
 *
 * Function determines the channel and operating class
 *
 * Return: None
 */
static void wlan_hdd_tdls_determine_channel_opclass(struct hdd_context *hddctx,
			struct hdd_adapter *adapter, hddTdlsPeer_t *curr_peer,
			uint32_t *channel, uint32_t *opclass)
{
	struct hdd_station_ctx *hdd_sta_ctx;

	/*
	 * If tdls offchannel is not enabled then we provide base channel
	 * and in that case pass opclass as 0 since opclass is mainly needed
	 * for offchannel cases.
	 */
	if (!(hddctx->config->fEnableTDLSOffChannel) ||
		(hddctx->tdls_fw_off_chan_mode != ENABLE_CHANSWITCH)) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		*channel = hdd_sta_ctx->conn_info.operationChannel;
		*opclass = 0;
	} else {
		*channel = curr_peer->pref_off_chan_num;
		*opclass = curr_peer->op_class_for_pref_off_chan;
	}
	hdd_debug("channel:%d opclass:%d", *channel, *opclass);
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * hdd_send_wlan_tdls_teardown_event()- send TDLS teardown event
 * @reason: reason for tear down.
 * @peer_mac: peer mac
 *
 * This Function sends TDLS teardown diag event
 *
 * Return: void.
 */
void hdd_send_wlan_tdls_teardown_event(uint32_t reason,
					uint8_t *peer_mac)
{
	WLAN_HOST_DIAG_EVENT_DEF(tdls_tear_down,
		struct host_event_tdls_teardown);
	qdf_mem_zero(&tdls_tear_down,
			sizeof(tdls_tear_down));

	tdls_tear_down.reason = reason;
	qdf_mem_copy(tdls_tear_down.peer_mac, peer_mac, MAC_ADDR_LEN);
	WLAN_HOST_DIAG_EVENT_REPORT(&tdls_tear_down,
		EVENT_WLAN_TDLS_TEARDOWN);
}

/**
 * hdd_wlan_tdls_enable_link_event()- send TDLS enable link event
 * @peer_mac: peer mac
 * @is_off_chan_supported: Does peer supports off chan
 * @is_off_chan_configured: If off channel is configured
 * @is_off_chan_established: If off chan is established
 *
 * This Function send TDLS enable link diag event
 *
 * Return: void.
 */

void hdd_wlan_tdls_enable_link_event(const uint8_t *peer_mac,
				uint8_t is_off_chan_supported,
				uint8_t is_off_chan_configured,
				uint8_t is_off_chan_established)
{
	WLAN_HOST_DIAG_EVENT_DEF(tdls_event,
		struct host_event_tdls_enable_link);

	qdf_mem_copy(tdls_event.peer_mac,
			peer_mac, MAC_ADDR_LEN);

	tdls_event.is_off_chan_supported =
			is_off_chan_supported;
	tdls_event.is_off_chan_configured =
			is_off_chan_configured;
	tdls_event.is_off_chan_established =
			is_off_chan_established;

	WLAN_HOST_DIAG_EVENT_REPORT(&tdls_event,
		EVENT_WLAN_TDLS_ENABLE_LINK);
}

/**
 * hdd_wlan_block_scan_by_tdls_event()- send event
 * if scan is blocked by tdls
 *
 * This Function send send diag event if scan is
 * blocked by tdls
 *
 * Return: void.
 */
void hdd_wlan_block_scan_by_tdls_event(void)
{
	WLAN_HOST_DIAG_EVENT_DEF(tdls_scan_block_status,
		struct host_event_tdls_scan_rejected);

	tdls_scan_block_status.status = true;
	WLAN_HOST_DIAG_EVENT_REPORT(&tdls_scan_block_status,
		EVENT_TDLS_SCAN_BLOCK);
}

#endif

/**
 * wlan_hdd_tdls_hash_key() - calculate tdls hash key given mac address
 * @mac: mac address
 *
 * Return: hash key
 */
static u8 wlan_hdd_tdls_hash_key(const u8 *mac)
{
	int i;
	u8 key = 0;

	for (i = 0; i < 6; i++)
		key ^= mac[i];

	return key;
}

/**
 * wlan_hdd_tdls_free_list() - free TDLS peer list
 * @pHddTdlsCtx: TDLS context
 *
 * Return: None
 */
static void wlan_hdd_tdls_free_list(tdlsCtx_t *pHddTdlsCtx)
{
	int i;
	struct list_head *head;
	hddTdlsPeer_t *tmp;
	struct list_head *pos, *q;

	if (NULL == pHddTdlsCtx) {
		hdd_debug("pHddTdlsCtx is NULL");
		return;
	}
	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];
		list_for_each_safe(pos, q, head) {
			tmp = list_entry(pos, hddTdlsPeer_t, node);
			list_del(pos);
			qdf_mem_free(tmp);
			tmp = NULL;
		}
	}
}

/**
 * dump_tdls_state_param_setting() - print tdls state & parameters to send to fw
 * @info: tdls setting to be sent to fw
 *
 * Return: void
 */
static void dump_tdls_state_param_setting(tdlsInfo_t *info)
{
	if (!info)
		return;

	hdd_debug("Setting tdls state and param in fw: vdev_id: %d, tdls_state: %d, notification_interval_ms: %d, tx_discovery_threshold: %d, tx_teardown_threshold: %d, rssi_teardown_threshold: %d, rssi_delta: %d, tdls_options: 0x%x, peer_traffic_ind_window: %d, peer_traffic_response_timeout: %d, puapsd_mask: 0x%x, puapsd_inactivity_time: %d, puapsd_rx_frame_threshold: %d, teardown_notification_ms: %d, tdls_peer_kickout_threshold: %d",
		   info->vdev_id,
		   info->tdls_state,
		   info->notification_interval_ms,
		   info->tx_discovery_threshold,
		   info->tx_teardown_threshold,
		   info->rssi_teardown_threshold,
		   info->rssi_delta,
		   info->tdls_options,
		   info->peer_traffic_ind_window,
		   info->peer_traffic_response_timeout,
		   info->puapsd_mask,
		   info->puapsd_inactivity_time,
		   info->puapsd_rx_frame_threshold,
		   info->teardown_notification_ms,
		   info->tdls_peer_kickout_threshold);

}

/**
 * wlan_hdd_tdls_del_non_forced_peers() - delete non forced tdls peers
 * @hdd_tdls_ctx: TDLS context
 *
 * Return: none
 */
static void wlan_hdd_tdls_del_non_forced_peers(tdlsCtx_t *hdd_tdls_ctx)
{
	struct list_head *head, *pos, *q;
	hddTdlsPeer_t *peer = NULL;
	int i;

	/* remove entries from peer list only if peer is not forced */
	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &hdd_tdls_ctx->peer_list[i];
		list_for_each_safe(pos, q, head) {
			peer = list_entry(pos, hddTdlsPeer_t, node);
			if (false == peer->isForcedPeer) {
				list_del(pos);
				qdf_mem_free(peer);
			} else {
				peer->link_status = eTDLS_LINK_IDLE;
				peer->reason = eTDLS_LINK_UNSPECIFIED;
				peer->staId = OL_TXRX_INVALID_TDLS_PEER_ID;
				peer->discovery_attempt = 0;
			}
		}
	}
}

/**
 * hdd_tdls_context_init() - Init TDLS context
 * @hdd_ctx:	HDD context
 * @ssr:	SSR case
 *
 * Initialize TDLS global context.
 *
 * Return: None
 */
void hdd_tdls_context_init(struct hdd_context *hdd_ctx, bool ssr)
{
	uint8_t sta_idx;

	if (!ssr) {
		mutex_init(&hdd_ctx->tdls_lock);
		qdf_spinlock_create(&hdd_ctx->tdls_ct_spinlock);
	}

	/* initialize TDLS global context */
	hdd_ctx->connected_peer_count = 0;
	hdd_ctx->tdls_nss_switch_in_progress = false;
	hdd_ctx->tdls_teardown_peers_cnt = 0;
	hdd_ctx->set_state_info.set_state_cnt = 0;
	hdd_ctx->set_state_info.vdev_id = 0;
	hdd_ctx->tdls_nss_teardown_complete = false;
	hdd_ctx->tdls_nss_transition_mode = TDLS_NSS_TRANSITION_UNKNOWN;

	if (false == hdd_ctx->config->fEnableTDLSImplicitTrigger) {
		hdd_ctx->tdls_mode = eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY;
		hdd_debug("TDLS Implicit trigger not enabled!");
	} else if (true == hdd_ctx->config->fTDLSExternalControl) {
		hdd_ctx->tdls_mode = eTDLS_SUPPORT_EXTERNAL_CONTROL;
	} else {
		hdd_ctx->tdls_mode = eTDLS_SUPPORT_ENABLED;
	}

	hdd_ctx->tdls_mode_last = hdd_ctx->tdls_mode;

	if (hdd_ctx->config->fEnableTDLSSleepSta ||
	    hdd_ctx->config->fEnableTDLSBufferSta ||
	    hdd_ctx->config->fEnableTDLSOffChannel)
		hdd_ctx->max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA_P_UAPSD_OFFCHAN;
	else
		hdd_ctx->max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA;

	hdd_debug("max_num_tdls_sta: %d", hdd_ctx->max_num_tdls_sta);

	for (sta_idx = 0; sta_idx < hdd_ctx->max_num_tdls_sta; sta_idx++) {
		hdd_ctx->tdlsConnInfo[sta_idx].staId = 0;
		hdd_ctx->tdlsConnInfo[sta_idx].sessionId = 255;
		qdf_mem_zero(&hdd_ctx->tdlsConnInfo[sta_idx].peerMac,
			     QDF_MAC_ADDR_SIZE);
	}

	/* Don't reset TDLS external peer count for SSR case */
	if (!ssr)
		hdd_ctx->tdls_external_peer_count = 0;
}

/**
 * hdd_tdls_context_destroy() - Destroy TDLS context
 * @hdd_ctx:	HDD context
 *
 * Destroy TDLS global context.
 *
 * Return: None
 */
void hdd_tdls_context_destroy(struct hdd_context *hdd_ctx)
{
	hdd_ctx->tdls_external_peer_count = 0;
	hdd_info("hdd_ctx->enable_tdls_connection_tracker: 0");
	mutex_destroy(&hdd_ctx->tdls_lock);
	qdf_spinlock_destroy(&hdd_ctx->tdls_ct_spinlock);
}

/**
 * wlan_hdd_tdls_init() - tdls initializaiton
 * @pAdapter: hdd adapter
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_init(struct hdd_adapter *pAdapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx;
	int i;

	if (NULL == hdd_ctx)
		return -EINVAL;

	ENTER();

	if (hdd_ctx->tdls_umac_comp_active)
		return 0;

	mutex_lock(&hdd_ctx->tdls_lock);

	if (false == hdd_ctx->config->fEnableTDLSSupport) {
		hdd_ctx->tdls_mode = eTDLS_SUPPORT_NOT_ENABLED;
		pAdapter->sessionCtx.station.pHddTdlsCtx = NULL;
		mutex_unlock(&hdd_ctx->tdls_lock);
		hdd_err("TDLS not enabled (%d) or FW doesn't support",
			hdd_ctx->config->fEnableTDLSSupport);
		return 0;
	}
	/* TDLS is supported only in STA / P2P Client modes,
	 * hence the check for TDLS support in a specific Device mode.
	 * Do not return a failure rather do not continue further
	 * with the initialization as tdls_init would be called
	 * during the open adapter for a p2p interface at which point
	 * the device mode would be a P2P_DEVICE. The point here is to
	 * continue initialization for STA / P2P Client modes.
	 * TDLS exit also check for the device mode for clean up hence
	 * there is no issue even if success is returned.
	 */
	if (0 == WLAN_HDD_IS_TDLS_SUPPORTED_ADAPTER(pAdapter)) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		return 0;
	}
	/* Check for the valid pHddTdlsCtx. If valid do not further
	 * allocate the memory, rather continue with the initialization.
	 * If tdls_initialization would get reinvoked  without tdls_exit
	 * getting invoked (SSR) there is no point to further proceed
	 * with the memory allocations.
	 */
	if (NULL == pAdapter->sessionCtx.station.pHddTdlsCtx) {
		pHddTdlsCtx = qdf_mem_malloc(sizeof(tdlsCtx_t));

		if (NULL == pHddTdlsCtx) {
			pAdapter->sessionCtx.station.pHddTdlsCtx = NULL;
			mutex_unlock(&hdd_ctx->tdls_lock);
			hdd_err("malloc failed!");
			return -ENOMEM;
		}

		pAdapter->sessionCtx.station.pHddTdlsCtx = pHddTdlsCtx;
		for (i = 0; i < TDLS_PEER_LIST_SIZE; i++)
			INIT_LIST_HEAD(&pHddTdlsCtx->peer_list[i]);
	} else {
		pHddTdlsCtx = pAdapter->sessionCtx.station.pHddTdlsCtx;

		wlan_hdd_tdls_del_non_forced_peers(pHddTdlsCtx);

		hdd_tdls_context_init(hdd_ctx, true);
	}

	sme_set_tdls_power_save_prohibited(WLAN_HDD_GET_HAL_CTX(pAdapter),
					   pAdapter->sessionId, 0);

	pHddTdlsCtx->pAdapter = pAdapter;

	pHddTdlsCtx->curr_candidate = NULL;
	pHddTdlsCtx->magic = 0;
	hdd_ctx->valid_mac_entries = 0;
	pHddTdlsCtx->last_flush_ts = 0;

	pHddTdlsCtx->threshold_config.tx_period_t =
		hdd_ctx->config->fTDLSTxStatsPeriod;
	pHddTdlsCtx->threshold_config.tx_packet_n =
		hdd_ctx->config->fTDLSTxPacketThreshold;
	pHddTdlsCtx->threshold_config.discovery_tries_n =
		hdd_ctx->config->fTDLSMaxDiscoveryAttempt;
	pHddTdlsCtx->threshold_config.idle_timeout_t =
		hdd_ctx->config->tdls_idle_timeout;
	pHddTdlsCtx->threshold_config.idle_packet_n =
		hdd_ctx->config->fTDLSIdlePacketThreshold;
	pHddTdlsCtx->threshold_config.rssi_trigger_threshold =
		hdd_ctx->config->fTDLSRSSITriggerThreshold;
	pHddTdlsCtx->threshold_config.rssi_teardown_threshold =
		hdd_ctx->config->fTDLSRSSITeardownThreshold;
	pHddTdlsCtx->threshold_config.rssi_delta =
		hdd_ctx->config->fTDLSRSSIDelta;

	mutex_unlock(&hdd_ctx->tdls_lock);

	if (hdd_ctx->config->fEnableTDLSOffChannel)
		hdd_ctx->tdls_fw_off_chan_mode = ENABLE_CHANSWITCH;

	EXIT();
	return 0;
}

/**
 * wlan_hdd_tdls_exit() - TDLS de-initialization
 * @pAdapter: HDD adapter
 *
 * Return: None
 */
void wlan_hdd_tdls_exit(struct hdd_adapter *pAdapter)
{
	tdlsCtx_t *pHddTdlsCtx;
	struct hdd_context *hdd_ctx;

	ENTER();
	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	if (!hdd_ctx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  FL("hdd_ctx is NULL"));
		return;
	}

	if (hdd_ctx->tdls_umac_comp_active)
		return;

	if (!test_bit(TDLS_INIT_DONE, &pAdapter->event_flags)) {
		hdd_info("TDLS init was not done, exit");
		return;
	}

	mutex_lock(&hdd_ctx->tdls_lock);

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		/*
		 * TDLS context can be null and might have been freed up during
		 * cleanup for STA adapter
		 */
		mutex_unlock(&hdd_ctx->tdls_lock);

		hdd_info("pHddTdlsCtx is NULL, adapter device mode: %s(%d)",
			 hdd_device_mode_to_string(pAdapter->device_mode),
			 pAdapter->device_mode);
		goto done;
	}

	/*
	 * must stop timer here before freeing peer list, because
	 * peerIdleTimer is part of peer list structure.
	 */
	wlan_hdd_tdls_free_list(pHddTdlsCtx);

	pHddTdlsCtx->magic = 0;
	pHddTdlsCtx->pAdapter = NULL;
	pAdapter->sessionCtx.station.pHddTdlsCtx = NULL;

	mutex_unlock(&hdd_ctx->tdls_lock);

	qdf_mem_free(pHddTdlsCtx);
done:
	EXIT();
	clear_bit(TDLS_INIT_DONE, &pAdapter->event_flags);
}

/**
 * wlan_hdd_tdls_get_peer() - find or add an peer given mac address
 * @pAdapter: HDD adapter
 * @mac: MAC address used to find or create peer
 *
 * Search peer given an MAC address and create one if not found.
 *
 * Return: Pointer to peer if mac address exist or peer creation
 *         succeeds; NULL if peer creation fails
 */
hddTdlsPeer_t *wlan_hdd_tdls_get_peer(struct hdd_adapter *pAdapter,
				      const u8 *mac)
{
	struct list_head *head;
	hddTdlsPeer_t *peer;
	u8 key;
	tdlsCtx_t *pHddTdlsCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return NULL;

	/* if already there, just update */
	peer = wlan_hdd_tdls_find_peer(pAdapter, mac);
	if (peer != NULL)
		return peer;

	/* not found, allocate and add the list */
	peer = qdf_mem_malloc(sizeof(hddTdlsPeer_t));
	if (NULL == peer) {
		hdd_err("peer malloc failed!");
		return NULL;
	}

	peer->staId = 0xff;
	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);

	if (NULL == pHddTdlsCtx) {
		qdf_mem_free(peer);
		hdd_debug("pHddTdlsCtx is NULL");
		return NULL;
	}

	key = wlan_hdd_tdls_hash_key(mac);
	head = &pHddTdlsCtx->peer_list[key];

	qdf_mem_copy(peer->peerMac, mac, sizeof(peer->peerMac));
	peer->pHddTdlsCtx = pHddTdlsCtx;
	peer->pref_off_chan_num = hdd_ctx->config->fTDLSPrefOffChanNum;
	peer->op_class_for_pref_off_chan =
		wlan_hdd_find_opclass(hdd_ctx->hHal, peer->pref_off_chan_num,
				hdd_ctx->config->fTDLSPrefOffChanBandwidth);

	list_add_tail(&peer->node, head);

	return peer;
}

/**
 * wlan_hdd_tdls_set_peer_link_status() - set TDLS peer link status
 * @curr_peer: peer
 * @status: status
 * @reason: reason
 *
 * Return: Void
 */
void wlan_hdd_tdls_set_peer_link_status(hddTdlsPeer_t *curr_peer,
					enum tdls_link_status status,
					enum tdls_link_reason reason)
{
	uint32_t state = 0;
	int32_t res = 0;
	struct hdd_context *hdd_ctx;

	if (curr_peer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("curr_peer is NULL"));
		return;
	}

	if (curr_peer->pHddTdlsCtx == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("curr_peer->pHddTdlsCtx is NULL"));
		return;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(curr_peer->pHddTdlsCtx->pAdapter);
	if ((wlan_hdd_validate_context(hdd_ctx)))
		return;
	hdd_warn("tdls set peer " MAC_ADDRESS_STR " link status to %u",
		 MAC_ADDR_ARRAY(curr_peer->peerMac), status);

	curr_peer->link_status = status;

	/* If TDLS link status is already passed the discovery state
	 * then clear discovery attempt count
	 */
	if (status >= eTDLS_LINK_DISCOVERED)
		curr_peer->discovery_attempt = 0;

	if (curr_peer->isForcedPeer && curr_peer->state_change_notification) {
		uint32_t opclass;
		uint32_t channel;
		struct hdd_adapter *adapter = curr_peer->pHddTdlsCtx->pAdapter;

		curr_peer->reason = reason;

		hdd_debug("Peer is forced and the reason:%d", reason);
		wlan_hdd_tdls_determine_channel_opclass(hdd_ctx, adapter,
					curr_peer, &channel, &opclass);

		wlan_hdd_tdls_get_wifi_hal_state(curr_peer, &state, &res);
		(*curr_peer->state_change_notification)(curr_peer->peerMac,
							opclass, channel,
							state, res, adapter);
	}
}

/**
 * wlan_hdd_tdls_extract_sa() - Extract source address from socket buffer
 * @skb: socket buffer
 * @mac: output mac address buffer to store the source address
 *
 * Return: Void
 */
void wlan_hdd_tdls_extract_sa(struct sk_buff *skb, uint8_t *mac)
{
	memcpy(mac, skb->data + 6, 6);
}

/**
 * wlan_hdd_tdls_check_config() - validate tdls configuration parameters
 * @config: tdls configuration parameter structure
 *
 * Return: 0 if all parameters are valid; -EINVAL otherwise
 */
static int wlan_hdd_tdls_check_config(tdls_config_params_t *config)
{
	if (config->tdls > 2) {
		hdd_err("Invalid 1st argument %d. <0...2>",
			config->tdls);
		return -EINVAL;
	}
	if (config->tx_period_t < CFG_TDLS_TX_STATS_PERIOD_MIN ||
	    config->tx_period_t > CFG_TDLS_TX_STATS_PERIOD_MAX) {
		hdd_err("Invalid 2nd argument %d. <%d...%ld>",
			config->tx_period_t, CFG_TDLS_TX_STATS_PERIOD_MIN,
			CFG_TDLS_TX_STATS_PERIOD_MAX);
		return -EINVAL;
	}
	if (config->tx_packet_n < CFG_TDLS_TX_PACKET_THRESHOLD_MIN ||
	    config->tx_packet_n > CFG_TDLS_TX_PACKET_THRESHOLD_MAX) {
		hdd_err("Invalid 3rd argument %d. <%d...%ld>",
			config->tx_packet_n, CFG_TDLS_TX_PACKET_THRESHOLD_MIN,
			CFG_TDLS_TX_PACKET_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->discovery_tries_n < CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN ||
	    config->discovery_tries_n > CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX) {
		hdd_err("Invalid 5th argument %d. <%d...%d>",
			config->discovery_tries_n,
			CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN,
			CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX);
		return -EINVAL;
	}
	if (config->idle_packet_n < CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN ||
	    config->idle_packet_n > CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX) {
		hdd_err("Invalid 7th argument %d. <%d...%d>",
			config->idle_packet_n,
			CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN,
			CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->rssi_trigger_threshold < CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN
	    || config->rssi_trigger_threshold >
	    CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX) {
		hdd_err("Invalid 9th argument %d. <%d...%d>",
			config->rssi_trigger_threshold,
			CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN,
			CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->rssi_teardown_threshold <
	    CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN
	    || config->rssi_teardown_threshold >
	    CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX) {
		hdd_err("Invalid 10th argument %d. <%d...%d>",
			config->rssi_teardown_threshold,
			CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN,
			CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->rssi_delta < CFG_TDLS_RSSI_DELTA_MIN
	    || config->rssi_delta > CFG_TDLS_RSSI_DELTA_MAX) {
		hdd_err("Invalid 11th argument %d. <%d...%d>",
			config->rssi_delta,
			CFG_TDLS_RSSI_DELTA_MIN,
			CFG_TDLS_RSSI_DELTA_MAX);
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_hdd_tdls_set_mode() - set TDLS mode
 * @hdd_ctx: HDD context
 * @tdls_mode: TDLS mode
 * @bUpdateLast: Switch on if to set hdd_ctx->tdls_mode_last to tdls_mode.
 *               If 1, set hdd_ctx->tdls_mode_last to tdls_mode, otherwise
 *               set hdd_ctx->tdls_mode_last to hdd_ctx->tdls_mode
 * @source: TDLS disable source enum values
 *
 * Return: Void
 */
static void wlan_hdd_tdls_set_mode(struct hdd_context *hdd_ctx,
				   enum tdls_support_mode tdls_mode,
				   bool bUpdateLast,
				   enum tdls_disable_source source)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	struct hdd_adapter *pAdapter;
	tdlsCtx_t *pHddTdlsCtx;

	ENTER();

	hdd_debug("mode %d", (int)tdls_mode);

	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return;

	mutex_lock(&hdd_ctx->tdls_lock);

	if (bUpdateLast)
		hdd_ctx->tdls_mode_last = tdls_mode;

	if (hdd_ctx->tdls_mode == tdls_mode) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		hdd_debug("already in mode %d", (int)tdls_mode);

		switch (tdls_mode) {
		/* TDLS is already enabled hence clear source mask, return */
		case eTDLS_SUPPORT_ENABLED:
		case eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY:
		case eTDLS_SUPPORT_EXTERNAL_CONTROL:
			clear_bit((unsigned long)source,
				  &hdd_ctx->tdls_source_bitmap);
			hdd_debug("clear source mask:%d", source);
			return;
		/* TDLS is already disabled hence set source mask, return */
		case eTDLS_SUPPORT_DISABLED:
			set_bit((unsigned long)source,
				&hdd_ctx->tdls_source_bitmap);
			hdd_debug("set source mask:%d", source);
			return;
		default:
			return;
		}
	}

	status = hdd_get_front_adapter(hdd_ctx, &pAdapterNode);

	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			if (eTDLS_SUPPORT_ENABLED == tdls_mode ||
			    eTDLS_SUPPORT_EXTERNAL_CONTROL == tdls_mode) {
				clear_bit((unsigned long)source,
					  &hdd_ctx->tdls_source_bitmap);

				/*
				 * Check if any TDLS source bit is set and if
				 * bitmap is not zero then we should not
				 * enable TDLS
				 */
				if (hdd_ctx->tdls_source_bitmap) {
					mutex_unlock(&hdd_ctx->tdls_lock);
					hdd_debug("Don't enable TDLS, source bitmap: %lu",
						hdd_ctx->tdls_source_bitmap);
					return;
				}
			} else if (eTDLS_SUPPORT_DISABLED == tdls_mode) {
				set_bit((unsigned long)source,
					&hdd_ctx->tdls_source_bitmap);
			} else if (eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY ==
				   tdls_mode) {
				clear_bit((unsigned long)source,
					  &hdd_ctx->tdls_source_bitmap);

				/*
				 * Check if any TDLS source bit is set and if
				 * bitmap is not zero then we should not
				 * enable TDLS
				 */
				if (hdd_ctx->tdls_source_bitmap) {
					mutex_unlock(&hdd_ctx->tdls_lock);
					return;
				}
			}
		}
		status = hdd_get_next_adapter(hdd_ctx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	if (!bUpdateLast)
		hdd_ctx->tdls_mode_last = hdd_ctx->tdls_mode;

	hdd_ctx->tdls_mode = tdls_mode;

	mutex_unlock(&hdd_ctx->tdls_lock);
	EXIT();
}

/**
 * wlan_hdd_tdls_set_params() - set TDLS parameters
 * @dev: net device
 * @config: TDLS configuration parameters
 *
 * Return: 0 if success or negative errno otherwise
 */
int wlan_hdd_tdls_set_params(struct net_device *dev,
			     tdls_config_params_t *config)
{
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx;
	enum tdls_support_mode req_tdls_mode;
	tdlsInfo_t *tdlsParams;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (wlan_hdd_tdls_check_config(config) != 0)
		return -EINVAL;

	/* config->tdls is mapped to 0->1, 1->2, 2->3 */
	req_tdls_mode = config->tdls + 1;
	if (hdd_ctx->tdls_mode == req_tdls_mode) {
		hdd_err("Already in mode %d", config->tdls);
		return -EINVAL;
	}

	mutex_lock(&hdd_ctx->tdls_lock);

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		hdd_err("pHddTdlsCtx is NULL");
			return -EINVAL;
	}

	/* Copy the configuration only when given tdls mode
	 * is implicit trigger enable
	 */
	if (eTDLS_SUPPORT_ENABLED == req_tdls_mode ||
	    eTDLS_SUPPORT_EXTERNAL_CONTROL == req_tdls_mode) {
		memcpy(&pHddTdlsCtx->threshold_config, config,
		       sizeof(tdls_config_params_t));
	}

	mutex_unlock(&hdd_ctx->tdls_lock);

	hdd_debug("iw set tdls params: %d %d %d %d %d %d %d",
		config->tdls,
		config->tx_period_t,
		config->tx_packet_n,
		config->discovery_tries_n,
		config->idle_packet_n,
		config->rssi_trigger_threshold,
		config->rssi_teardown_threshold);

	wlan_hdd_tdls_set_mode(hdd_ctx, req_tdls_mode, true,
			       HDD_SET_TDLS_MODE_SOURCE_USER);

	tdlsParams = qdf_mem_malloc(sizeof(tdlsInfo_t));
	if (NULL == tdlsParams) {
		hdd_err("qdf_mem_malloc failed for tdlsParams");
		return -ENOMEM;
	}

	tdlsParams->vdev_id = pAdapter->sessionId;
	tdlsParams->tdls_state = config->tdls;
	tdlsParams->notification_interval_ms = config->tx_period_t;
	tdlsParams->tx_discovery_threshold = config->tx_packet_n;
	tdlsParams->tx_teardown_threshold = config->idle_packet_n;
	tdlsParams->rssi_teardown_threshold = config->rssi_teardown_threshold;
	tdlsParams->rssi_delta = config->rssi_delta;
	tdlsParams->tdls_options = 0;
	if (hdd_ctx->config->fEnableTDLSOffChannel)
		tdlsParams->tdls_options |= ENA_TDLS_OFFCHAN;
	if (hdd_ctx->config->fEnableTDLSBufferSta)
		tdlsParams->tdls_options |= ENA_TDLS_BUFFER_STA;
	if (hdd_ctx->config->fEnableTDLSSleepSta)
		tdlsParams->tdls_options |= ENA_TDLS_SLEEP_STA;
	tdlsParams->peer_traffic_ind_window =
		hdd_ctx->config->fTDLSPuapsdPTIWindow;
	tdlsParams->peer_traffic_response_timeout =
		hdd_ctx->config->fTDLSPuapsdPTRTimeout;
	tdlsParams->puapsd_mask = hdd_ctx->config->fTDLSUapsdMask;
	tdlsParams->puapsd_inactivity_time =
		hdd_ctx->config->fTDLSPuapsdInactivityTimer;
	tdlsParams->puapsd_rx_frame_threshold =
		hdd_ctx->config->fTDLSRxFrameThreshold;
	tdlsParams->teardown_notification_ms =
		hdd_ctx->config->tdls_idle_timeout;
	tdlsParams->tdls_peer_kickout_threshold =
		hdd_ctx->config->tdls_peer_kickout_threshold;

	dump_tdls_state_param_setting(tdlsParams);

	qdf_ret_status = sme_update_fw_tdls_state(hdd_ctx->hHal,
						  tdlsParams, true);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		qdf_mem_free(tdlsParams);
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_tdls_set_extctrl_param() - set external control parameter on a peer
 * @pAdapter: HDD adapter
 * @mac: MAC address of the peer
 * @chan: Channel
 * @max_latency: Maximum latency
 * @op_class: Operation class
 * @min_bandwidth: Minimal bandwidth
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_hdd_tdls_set_extctrl_param(struct hdd_adapter *pAdapter,
				    const uint8_t *mac,
				    uint32_t chan, uint32_t max_latency,
				    uint32_t op_class, uint32_t min_bandwidth)
{
	hddTdlsPeer_t *curr_peer;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (!hdd_ctx)
		return -EINVAL;

	mutex_lock(&hdd_ctx->tdls_lock);
	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		return -EINVAL;
	}
	curr_peer->op_class_for_pref_off_chan = (uint8_t) op_class;
	curr_peer->pref_off_chan_num = (uint8_t) chan;

	mutex_unlock(&hdd_ctx->tdls_lock);
	return 0;
}

/**
 * wlan_hdd_tdls_update_peer_mac() - Update the peer mac information to firmware
 * @adapter: hdd adapter to interface
 * @mac: Mac address of the peer to be added
 * @peerState: Current state of the peer
 *
 * This function updates TDLS peer state to firmware. Firmware will update
 * connection table based on new peer state.
 *
 * Return:success (0) or failure (errno value)
 */
int wlan_hdd_tdls_update_peer_mac(struct hdd_adapter *adapter,
				  const uint8_t *mac,
				  uint32_t peer_state)
{
	tSmeTdlsPeerStateParams sme_tdls_peer_state_params = {0};
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	sme_tdls_peer_state_params.vdevId = adapter->sessionId;
	qdf_mem_copy(&sme_tdls_peer_state_params.peerMacAddr, mac,
		     sizeof(sme_tdls_peer_state_params.peerMacAddr));
	sme_tdls_peer_state_params.peerState = peer_state;
	status = sme_update_tdls_peer_state(hdd_ctx->hHal,
					    &sme_tdls_peer_state_params);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("sme_UpdateTdlsPeerState failed for "MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(mac));
		return -EPERM;
	}
	return 0;
}

/**
 * wlan_hdd_tdls_set_force_peer() - set/clear isForcedPeer flag on a peer
 * @pAdapter: HDD adapter
 * @mac: MAC address of the tdls peer
 * @forcePeer: value used to set isForcedPeer flag on the peer
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_hdd_tdls_set_force_peer(struct hdd_adapter *pAdapter,
				 const uint8_t *mac,
				 bool forcePeer)
{
	hddTdlsPeer_t *curr_peer;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (!hdd_ctx)
		return -EINVAL;

	mutex_lock(&hdd_ctx->tdls_lock);
	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		return -EINVAL;
	}

	curr_peer->isForcedPeer = forcePeer;
	mutex_unlock(&hdd_ctx->tdls_lock);
	return 0;
}

/**
 * wlan_hdd_tdls_find_peer() - find TDLS peer given its MAC address
 * @pAdapter: HDD adapter
 * @mac: MAC address of peer
 *
 * Return: If peerMac is found, then it returns pointer to hddTdlsPeer_t;
 *         otherwise, it returns NULL
 */
hddTdlsPeer_t *wlan_hdd_tdls_find_peer(struct hdd_adapter *pAdapter,
				       const uint8_t *mac)
{
	uint8_t key;
	struct list_head *pos;
	struct list_head *head;
	hddTdlsPeer_t *curr_peer;
	tdlsCtx_t *pHddTdlsCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);


	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return NULL;

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx)
		return NULL;

	key = wlan_hdd_tdls_hash_key(mac);

	head = &pHddTdlsCtx->peer_list[key];

	list_for_each(pos, head) {
		curr_peer = list_entry(pos, hddTdlsPeer_t, node);
		if (!memcmp(mac, curr_peer->peerMac, 6)) {
			hdd_debug("findTdlsPeer: found staId %d",
				  curr_peer->staId);
			return curr_peer;
		}
	}

	return NULL;
}

/**
 * wlan_hdd_tdls_find_all_peer() - find all peers matching the input MAC
 * @hdd_ctx: HDD context
 * @mac: MAC address
 *
 * Return: TDLS peer if a matching is detected; NULL otherwise
 */
hddTdlsPeer_t *wlan_hdd_tdls_find_all_peer(struct hdd_context *hdd_ctx,
					   const u8 *mac)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	struct hdd_adapter *pAdapter = NULL;
	tdlsCtx_t *pHddTdlsCtx = NULL;
	hddTdlsPeer_t *curr_peer = NULL;
	QDF_STATUS status = 0;

	status = hdd_get_front_adapter(hdd_ctx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			curr_peer =
				wlan_hdd_tdls_find_peer(pAdapter, mac);
			if (curr_peer)
				return curr_peer;
		}
		status = hdd_get_next_adapter(hdd_ctx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	return curr_peer;
}

/**
 * wlan_hdd_tdls_connected_peers() - Find the number of connected TDLS peers
 * @pAdapter: HDD adapter
 *
 * Return: The number of connected TDLS peers or 0 if error is detected
 */
uint16_t wlan_hdd_tdls_connected_peers(struct hdd_adapter *pAdapter)
{
	struct hdd_context *hdd_ctx;

	if ((NULL == pAdapter) ||
	    (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		hdd_err("invalid pAdapter: %pK", pAdapter);
		return 0;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	return hdd_ctx->connected_peer_count;
}

/**
 * wlan_hdd_tdls_get_all_peers() - dump all TDLS peer info into output string
 * @pAdapter: HDD adapter
 * @buf: output string buffer to hold the peer info
 * @buflen: the size of output string buffer
 *
 * Return: The size (in bytes) of the valid peer info in the output buffer
 */
int wlan_hdd_tdls_get_all_peers(struct hdd_adapter *pAdapter,
				char *buf, int buflen)
{
	int i;
	int len, init_len;
	struct list_head *head;
	struct list_head *pos;
	hddTdlsPeer_t *curr_peer;
	tdlsCtx_t *pHddTdlsCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	struct hdd_station_ctx *hdd_sta_ctx;

	ENTER();

	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return 0;

	if ((QDF_STA_MODE != pAdapter->device_mode)
	    && (QDF_P2P_CLIENT_MODE != pAdapter->device_mode)) {
		len = scnprintf(buf, buflen,
				"\nNo TDLS support for this adapter\n");
		return len;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	if (eConnectionState_Associated != hdd_sta_ctx->conn_info.connState) {
		len = scnprintf(buf, buflen, "\nSTA is not associated\n");
		return len;
	}

	init_len = buflen;
	len = scnprintf(buf, buflen, "\n%-18s%-3s%-4s%-3s%-5s\n",
			"MAC", "Id", "cap", "up", "RSSI");
	buf += len;
	buflen -= len;
	/*                           1234567890123456789012345678901234567 */
	len = scnprintf(buf, buflen, "---------------------------------\n");
	buf += len;
	buflen -= len;

	mutex_lock(&hdd_ctx->tdls_lock);

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		len = scnprintf(buf, buflen, "TDLS not enabled\n");
		return len;
	}
	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];

		list_for_each(pos, head) {
			curr_peer = list_entry(pos, hddTdlsPeer_t, node);

			if (buflen < 32 + 1)
				break;
			len = scnprintf(buf, buflen,
					MAC_ADDRESS_STR "%3d%4s%3s%5d\n",
					MAC_ADDR_ARRAY(curr_peer->peerMac),
					curr_peer->staId,
					(curr_peer->tdls_support ==
					 eTDLS_CAP_SUPPORTED) ? "Y" : "N",
					TDLS_IS_CONNECTED(curr_peer) ? "Y" :
					"N", curr_peer->rssi);
			buf += len;
			buflen -= len;
		}
	}
	mutex_unlock(&hdd_ctx->tdls_lock);
	EXIT();
	return init_len - buflen;
}

/**
 * wlan_hdd_tdls_indicate_teardown() - indicate tdls teardown
 * @pAdapter: HDD adapter
 * @curr_peer: peer tdls teardown happened
 * @reason: teardown reason
 *
 * Return: Void
 */
void wlan_hdd_tdls_indicate_teardown(struct hdd_adapter *pAdapter,
				     hddTdlsPeer_t *curr_peer, uint16_t reason)
{
	if ((NULL == pAdapter || WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) ||
	    (NULL == curr_peer)) {
		hdd_err("parameters passed are invalid");
		if (!curr_peer)
			hdd_err("curr_peer is NULL");
		return;
	}

	if (eTDLS_LINK_CONNECTED != curr_peer->link_status)
		return;

	wlan_hdd_tdls_set_peer_link_status(curr_peer,
					   eTDLS_LINK_TEARING,
					   eTDLS_LINK_UNSPECIFIED);
	hdd_debug("Teardown reason %d", reason);
	cfg80211_tdls_oper_request(pAdapter->dev,
				   curr_peer->peerMac,
				   NL80211_TDLS_TEARDOWN, reason, GFP_KERNEL);
}

/**
 * wlan_hdd_set_callback() - set state change callback on current TDLS peer
 * @curr_peer: current TDLS peer
 * @callback: state change callback
 *
 * Return: 0 if success; negative errno otherwise
 */
int wlan_hdd_set_callback(hddTdlsPeer_t *curr_peer,
			  cfg80211_exttdls_callback callback)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *pAdapter;

	if (!curr_peer)
		return -EINVAL;

	pAdapter = curr_peer->pHddTdlsCtx->pAdapter;
	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	if (NULL == hdd_ctx)
		return -EINVAL;

	curr_peer->state_change_notification = callback;
	return 0;
}

/**
 * wlan_hdd_tdls_get_wifi_hal_state() - get tdls wifi hal state on current peer
 * @curr_peer: current TDLS peer
 * @state: output parameter to store the tdls wifi hal state
 * @reason: output parameter to store the reason of the current peer
 *
 * Return: Void
 */
void wlan_hdd_tdls_get_wifi_hal_state(hddTdlsPeer_t *curr_peer,
				      uint32_t *state, int32_t *reason)
{
	struct hdd_context *hddctx;
	struct hdd_adapter *adapter;

	if (!curr_peer) {
		hdd_err("curr_peer is NULL");
		return;
	}

	adapter = curr_peer->pHddTdlsCtx->pAdapter;
	hddctx = WLAN_HDD_GET_CTX(adapter);

	if (0 != (wlan_hdd_validate_context(hddctx)))
		return;

	*reason = curr_peer->reason;

	switch (curr_peer->link_status) {
	case eTDLS_LINK_IDLE:
	case eTDLS_LINK_DISCOVERED:
		*state = QCA_WIFI_HAL_TDLS_ENABLED;
		break;
	case eTDLS_LINK_DISCOVERING:
	case eTDLS_LINK_CONNECTING:
		*state = QCA_WIFI_HAL_TDLS_ENABLED;
		break;
	case eTDLS_LINK_CONNECTED:
		if ((hddctx->config->fEnableTDLSOffChannel) &&
			(hddctx->tdls_fw_off_chan_mode == ENABLE_CHANSWITCH))
			*state = QCA_WIFI_HAL_TDLS_ESTABLISHED_OFF_CHANNEL;
		else
			*state = QCA_WIFI_HAL_TDLS_ESTABLISHED;
		break;
	case eTDLS_LINK_TEARING:
		*state = QCA_WIFI_HAL_TDLS_DROPPED;
		break;
	}
}

/**
 * wlan_hdd_tdls_get_status() - get tdls status on current tdls peer
 * @pAdapter: HDD adapter
 * @mac: MAC address of current TDLS peer
 * @state: output parameter to store the tdls wifi hal state
 * @reason: output parameter to store the reason of the current peer
 *
 * Return: 0 if success; negative errno otherwise
 */
int wlan_hdd_tdls_get_status(struct hdd_adapter *pAdapter,
			     const uint8_t *mac, uint32_t *opclass,
			     uint32_t *channel, uint32_t *state,
			     int32_t *reason)
{
	hddTdlsPeer_t *curr_peer;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return -EINVAL;

	mutex_lock(&hdd_ctx->tdls_lock);
	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("curr_peer is NULL"));
		*state = QCA_WIFI_HAL_TDLS_DISABLED;
		*reason = eTDLS_LINK_UNSPECIFIED;
		return -EINVAL;
	}
	if (hdd_ctx->config->fTDLSExternalControl &&
		(false == curr_peer->isForcedPeer)) {
		hdd_debug("curr_peer is not Forced");
		*state = QCA_WIFI_HAL_TDLS_DISABLED;
		*reason = eTDLS_LINK_UNSPECIFIED;
	} else {
		wlan_hdd_tdls_determine_channel_opclass(hdd_ctx, pAdapter,
					curr_peer, channel, opclass);
		wlan_hdd_tdls_get_wifi_hal_state(curr_peer, state, reason);
	}
	mutex_unlock(&hdd_ctx->tdls_lock);

	return 0;
}

#ifdef FEATURE_WLAN_TDLS
static const struct nla_policy
	wlan_hdd_tdls_config_enable_policy[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX +
					   1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR] = {
		.type = NLA_UNSPEC,
		.len = QDF_MAC_ADDR_SIZE},
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS] = {.type =
								NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS] = {.type =
								NLA_U32},
};
static const struct nla_policy
	wlan_hdd_tdls_config_disable_policy[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX +
					    1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR] = {
		.type = NLA_UNSPEC,
		.len = QDF_MAC_ADDR_SIZE},
};
static const struct nla_policy
	wlan_hdd_tdls_config_state_change_policy[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX
						 + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR] = {
		.type = NLA_UNSPEC,
		.len = QDF_MAC_ADDR_SIZE},
	[QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON] = {.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS] = {.type =
								NLA_U32},
};
static const struct nla_policy
	wlan_hdd_tdls_config_get_status_policy
[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR] = {
		.type = NLA_UNSPEC,
		.len = QDF_MAC_ADDR_SIZE},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON] = {.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS] = {
							.type = NLA_U32},
};

static const struct nla_policy
	wlan_hdd_tdls_mode_configuration_policy
	[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_DISCOVERY_PERIOD] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD] = {
						.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD] = {
						.type = NLA_S32},
		[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD] = {
						.type = NLA_S32},
};

/**
 * __wlan_hdd_cfg80211_exttdls_get_status() - handle get status cfg80211 command
 * @wiphy: wiphy
 * @wdev: wireless dev
 * @data: netlink buffer with the mac address of the peer to get the status for
 * @data_len: length of data in bytes
 */
static int
__wlan_hdd_cfg80211_exttdls_get_status(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	uint8_t peer[ETH_ALEN] = { 0 };
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX + 1];
	QDF_STATUS ret;
	uint32_t state;
	int32_t reason;
	uint32_t global_operating_class = 0;
	uint32_t channel = 0;
	struct sk_buff *skb = NULL;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	if (hdd_ctx->config->fTDLSExternalControl == false)
		return -ENOTSUPP;

	if (hdd_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX, data,
			  data_len, wlan_hdd_tdls_config_get_status_policy)) {
		hdd_err("Invalid attribute");
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR]) {
		hdd_err("attr mac addr failed");
		return -EINVAL;
	}
	memcpy(peer,
	       nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR]),
	       sizeof(peer));
	hdd_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(peer));
	ret = wlan_hdd_tdls_get_status(pAdapter, peer, &global_operating_class,
				&channel, &state, &reason);
	if (0 != ret) {
		hdd_err("get status Failed");
		return -EINVAL;
	}
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
						  4 * sizeof(int32_t) +
						  NLMSG_HDRLEN);
	if (!skb) {
		hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -EINVAL;
	}
	hdd_debug("Reason %d Status %d class %d channel %d peer " MAC_ADDRESS_STR,
		   reason, state, global_operating_class, channel,
		   MAC_ADDR_ARRAY(peer));
	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE,
			state) ||
		nla_put_s32(skb,
			QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON,
			reason) ||
		nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS,
			global_operating_class) ||
		nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL,
			channel)) {
		hdd_err("nla put fail");
		goto nla_put_failure;
	}
	ret = cfg80211_vendor_cmd_reply(skb);
	EXIT();
	return ret;
nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * __wlan_hdd_cfg80211_configure_tdls_mode() - configure the tdls mode
 * @wiphy: wiphy
 * @wdev: wireless dev
 * @data: netlink buffer
 * @data_len: length of data in bytes
 *
 * Return 0 for success and error code for failure
 */
static int
__wlan_hdd_cfg80211_configure_tdls_mode(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX + 1];
	int ret;
	enum tdls_support_mode tdls_mode;
	uint32_t trigger_mode;
	tdlsCtx_t *hdd_tdls_ctx;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	if (NULL == adapter)
		return -EINVAL;

	if (hdd_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX, data,
			  data_len, wlan_hdd_tdls_mode_configuration_policy)) {
		hdd_err("Invalid attribute");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE]) {
		hdd_err("attr tdls trigger mode failed");
		return -EINVAL;
	}
	trigger_mode = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE]);
	hdd_debug("TDLS trigger mode %d", trigger_mode);

	if (hdd_ctx->tdls_umac_comp_active) {
		ret = wlan_cfg80211_tdls_configure_mode(adapter->hdd_vdev,
						trigger_mode);
		return ret;
	}

	switch (trigger_mode) {
	case WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT:
		tdls_mode = eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY;
		break;
	case WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL:
		tdls_mode = eTDLS_SUPPORT_EXTERNAL_CONTROL;
		break;
	case WLAN_HDD_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT:
		tdls_mode = eTDLS_SUPPORT_ENABLED;
		break;
	default:
		hdd_err("Invalid TDLS trigger mode");
		return -EINVAL;
	}
	wlan_hdd_tdls_set_mode(hdd_ctx, tdls_mode, false,
			HDD_SET_TDLS_MODE_SOURCE_USER);

	mutex_lock(&hdd_ctx->tdls_lock);

	hdd_tdls_ctx = adapter->sessionCtx.station.pHddTdlsCtx;
	if (NULL == hdd_tdls_ctx) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD]) {
		hdd_tdls_ctx->threshold_config.tx_period_t = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD]);
		hdd_debug("attr tdls tx stats period %d",
			hdd_tdls_ctx->threshold_config.tx_period_t);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD]) {
		hdd_tdls_ctx->threshold_config.tx_packet_n = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD]);
		hdd_debug("attr tdls tx packet period %d",
			hdd_tdls_ctx->threshold_config.tx_packet_n);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT]) {
		hdd_tdls_ctx->threshold_config.discovery_tries_n = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT]);
		hdd_debug("attr tdls max discovery attempt %d",
			hdd_tdls_ctx->threshold_config.discovery_tries_n);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT]) {
		hdd_tdls_ctx->threshold_config.idle_timeout_t = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT]);
		hdd_debug("attr tdls idle time out period %d",
			hdd_tdls_ctx->threshold_config.idle_timeout_t);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD]) {
		hdd_tdls_ctx->threshold_config.idle_packet_n = nla_get_u32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD]);
		hdd_debug("attr tdls idle pkt threshold %d",
			hdd_tdls_ctx->threshold_config.idle_packet_n);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD]) {
		hdd_tdls_ctx->threshold_config.rssi_trigger_threshold = nla_get_s32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD]);
		hdd_debug("attr tdls rssi trigger threshold %d",
			hdd_tdls_ctx->threshold_config.rssi_trigger_threshold);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD]) {
		hdd_tdls_ctx->threshold_config.rssi_teardown_threshold = nla_get_s32(
			tb[QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD]);
		hdd_debug("attr tdls tx stats period %d",
			hdd_tdls_ctx->threshold_config.rssi_teardown_threshold);
	}

	mutex_unlock(&hdd_ctx->tdls_lock);

	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_configure_tdls_mode() - configure tdls mode
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_configure_tdls_mode(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_configure_tdls_mode(wiphy, wdev, data,
							data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_exttdls_get_status() - get ext tdls status
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_exttdls_get_status(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_exttdls_get_status(wiphy, wdev, data,
							data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_exttdls_callback() - notify cfg80211 state change
 * @mac: MAC address of the peer with state change
 * @state: New state
 * @reason: Reason to enter new state
 * @ctx: HDD adapter
 *
 * Return: 0 for success; negative errno otherwise
 */
static int wlan_hdd_cfg80211_exttdls_callback(const uint8_t *mac,
					      uint32_t global_operating_class,
					      uint32_t channel,
					      uint32_t state,
					      int32_t reason, void *ctx)
{
	struct hdd_adapter *pAdapter = (struct hdd_adapter *) ctx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	struct sk_buff *skb = NULL;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;
	if (hdd_ctx->config->fTDLSExternalControl == false)
		return -ENOTSUPP;

	skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
					  NULL,
					  EXTTDLS_EVENT_BUF_SIZE + NLMSG_HDRLEN,
					  QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX,
					  GFP_KERNEL);
	if (!skb) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return -EINVAL;
	}
	hdd_debug("Reason: %d Status: %d Class: %d Channel: %d tdls peer " MAC_ADDRESS_STR,
		   reason, state, global_operating_class, channel,
		   MAC_ADDR_ARRAY(mac));
	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR,
		QDF_MAC_ADDR_SIZE, mac) ||
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE,
			state) ||
		nla_put_s32(skb, QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON,
			reason) ||
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL,
			channel) ||
		nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS,
			global_operating_class)) {
		hdd_err("nla put fail");
		goto nla_put_failure;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	EXIT();
	return 0;
nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * __wlan_hdd_cfg80211_exttdls_enable() - enable an externally controllable
 *                                      TDLS peer and set parameters
 * wiphy: wiphy
 * @wdev: wireless dev pointer
 * @data: netlink buffer with peer MAC address and configuration parameters
 * @data_len: size of data in bytes
 *
 * This function sets channel, operation class, maximum latency and minimal
 * bandwidth parameters on a TDLS peer that's externally controllable.
 *
 * Return: 0 for success; negative errno otherwise
 */
static int
__wlan_hdd_cfg80211_exttdls_enable(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len)
{
	uint8_t peer[ETH_ALEN] = { 0 };
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX + 1];
	QDF_STATUS status;
	tdls_req_params_t pReqMsg = { 0 };
	int ret;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;
	if (hdd_ctx->config->fTDLSExternalControl == false) {
		hdd_err("TDLS External Control is not enabled");
		return -ENOTSUPP;
	}
	if (hdd_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX, data,
			  data_len, wlan_hdd_tdls_config_enable_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR]) {
		hdd_err("attr mac addr failed");
		return -EINVAL;
	}
	memcpy(peer, nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR]),
	       sizeof(peer));
	hdd_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(peer));
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL]) {
		hdd_err("attr channel failed");
		return -EINVAL;
	}
	pReqMsg.channel =
		nla_get_s32(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL]);
	hdd_debug("Channel Num (%d)", pReqMsg.channel);
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS]) {
		hdd_err("attr operating class failed");
		return -EINVAL;
	}
	pReqMsg.global_operating_class =
		nla_get_s32(tb
			    [QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS]);
	hdd_debug("Operating class (%d)",
		   pReqMsg.global_operating_class);
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS]) {
		hdd_err("attr latency failed");
		return -EINVAL;
	}
	pReqMsg.max_latency_ms =
		nla_get_s32(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS]);

	hdd_debug("Latency (%d)", pReqMsg.max_latency_ms);

	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS]) {
		hdd_err("attr bandwidth failed");
		return -EINVAL;
	}
	pReqMsg.min_bandwidth_kbps =
		nla_get_s32(tb
			    [QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS]);

	hdd_debug("Bandwidth (%d)", pReqMsg.min_bandwidth_kbps);

	ret = wlan_hdd_tdls_extctrl_config_peer(
					pAdapter,
					peer,
					wlan_hdd_cfg80211_exttdls_callback,
					pReqMsg.channel,
					pReqMsg.max_latency_ms,
					pReqMsg.
					global_operating_class,
					pReqMsg.min_bandwidth_kbps);
	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_exttdls_enable() - enable ext tdls
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_exttdls_enable(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_exttdls_enable(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_exttdls_disable() - disable an externally controllable
 *                                       TDLS peer
 * wiphy: wiphy
 * @wdev: wireless dev pointer
 * @data: netlink buffer with peer MAC address
 * @data_len: size of data in bytes
 *
 * This function disables an externally controllable TDLS peer
 *
 * Return: 0 for success; negative errno otherwise
 */
static int __wlan_hdd_cfg80211_exttdls_disable(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len)
{
	u8 peer[ETH_ALEN] = {0};
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX + 1];
	QDF_STATUS status;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;
	if (hdd_ctx->config->fTDLSExternalControl == false)
		return -ENOTSUPP;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX,
		      data, data_len, wlan_hdd_tdls_config_disable_policy)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR]) {
		hdd_err("attr mac addr failed");
		return -EINVAL;
	}
	memcpy(peer, nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR]),
	       sizeof(peer));
	hdd_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(peer));
	status = wlan_hdd_tdls_extctrl_deconfig_peer(pAdapter, peer);
	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_exttdls_disable() - disable ext tdls
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * Return:   Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_exttdls_disable(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret = 0;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_exttdls_disable(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

#if TDLS_MGMT_VERSION2
/**
 * __wlan_hdd_cfg80211_tdls_mgmt() - handle management actions on a given peer
 * @wiphy: wiphy
 * @dev: net device
 * @peer: MAC address of the TDLS peer
 * @action_code: action code
 * @dialog_token: dialog token
 * @status_code: status code
 * @peer_capability: peer capability
 * @buf: additional IE to include
 * @len: length of buf in bytes
 *
 * Return: 0 if success; negative errno otherwise
 */
static int __wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, u8 *peer,
				u8 action_code, u8 dialog_token,
				u16 status_code, u32 peer_capability,
				const u8 *buf, size_t len)
#else
/**
 * __wlan_hdd_cfg80211_tdls_mgmt() - handle management actions on a given peer
 * @wiphy: wiphy
 * @dev: net device
 * @peer: MAC address of the TDLS peer
 * @action_code: action code
 * @dialog_token: dialog token
 * @status_code: status code
 * @buf: additional IE to include
 * @len: length of buf in bytes
 *
 * Return: 0 if success; negative errno otherwise
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
static int __wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				bool initiator, const uint8_t *buf,
				size_t len)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static int __wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
static int __wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len)
#else
static int __wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, const uint8_t *buf,
				size_t len)
#endif
#endif
{
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
#if !(TDLS_MGMT_VERSION2)
	u32 peer_capability;

	peer_capability = 0;
#endif
#endif

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
		hdd_err("invalid session id: %d", pAdapter->sessionId);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_TDLS_MGMT,
			 pAdapter->sessionId, action_code));

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (hdd_ctx->tdls_umac_comp_active)
		return wlan_cfg80211_tdls_mgmt(hdd_ctx->hdd_pdev, dev,
					       peer,
					       action_code, dialog_token,
					       status_code, peer_capability,
					       buf, len);

	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_tdls_mgmt() - cfg80211 tdls mgmt handler function
 * @wiphy: Pointer to wiphy structure.
 * @dev: Pointer to net_device structure.
 * @peer: peer address
 * @action_code: action code
 * @dialog_token: dialog token
 * @status_code: status code
 * @peer_capability: peer capability
 * @buf: buffer
 * @len: Length of @buf
 *
 * This is the cfg80211 tdls mgmt handler function which invokes
 * the internal function @__wlan_hdd_cfg80211_tdls_mgmt with
 * SSR protection.
 *
 * Return: 0 for success, error number on failure.
 */
#if TDLS_MGMT_VERSION2
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
					struct net_device *dev,
					u8 *peer, u8 action_code,
					u8 dialog_token,
					u16 status_code, u32 peer_capability,
					const u8 *buf, size_t len)
#else /* TDLS_MGMT_VERSION2 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) || defined(WITH_BACKPORTS)
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
					struct net_device *dev,
					const u8 *peer, u8 action_code,
					u8 dialog_token, u16 status_code,
					u32 peer_capability, bool initiator,
					const u8 *buf, size_t len)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
					struct net_device *dev,
					const u8 *peer, u8 action_code,
					u8 dialog_token, u16 status_code,
					u32 peer_capability, const u8 *buf,
					size_t len)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
					struct net_device *dev,
					u8 *peer, u8 action_code,
					u8 dialog_token,
					u16 status_code, u32 peer_capability,
					const u8 *buf, size_t len)
#else
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
					struct net_device *dev,
					u8 *peer, u8 action_code,
					u8 dialog_token,
					u16 status_code, const u8 *buf,
					size_t len)
#endif
#endif
{
	int ret;

	cds_ssr_protect(__func__);
#if TDLS_MGMT_VERSION2
	ret = __wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer, action_code,
						dialog_token, status_code,
						peer_capability, buf, len);
#else /* TDLS_MGMT_VERSION2 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) || defined(WITH_BACKPORTS)
	ret = __wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer, action_code,
					dialog_token, status_code,
					peer_capability, initiator,
					buf, len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	ret = __wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer, action_code,
					dialog_token, status_code,
					peer_capability, buf, len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
	ret = __wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer, action_code,
					dialog_token, status_code,
					peer_capability, buf, len);
#else
	ret = __wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer, action_code,
					dialog_token, status_code, buf, len);
#endif
#endif

	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_tdls_extctrl_config_peer() - configure an externally controllable
 *                                       TDLS peer
 * @pAdapter: HDD adapter
 * @peer: MAC address of the TDLS peer
 * @callback: Callback to set on the peer
 * @chan: Channel
 * @max_latency: Maximum latency
 * @op_class: Operation class
 * @min_bandwidth: Minimal bandwidth
 *
 * Return: 0 on success; negative otherwise
 */
int wlan_hdd_tdls_extctrl_config_peer(struct hdd_adapter *pAdapter,
				      const uint8_t *peer,
				      cfg80211_exttdls_callback callback,
				      u32 chan,
				      u32 max_latency,
				      u32 op_class, u32 min_bandwidth)
{
	hddTdlsPeer_t *pTdlsPeer;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	int status = 0;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s : NL80211_TDLS_SETUP for " MAC_ADDRESS_STR,
		  __func__, MAC_ADDR_ARRAY(peer));
	if ((false == hdd_ctx->config->fTDLSExternalControl) ||
	    (false == hdd_ctx->config->fEnableTDLSImplicitTrigger)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s TDLS External control or Implicit Trigger not enabled ",
			  __func__);
		status = -ENOTSUPP;
		goto ret_status;
	}

	mutex_lock(&hdd_ctx->tdls_lock);
	pTdlsPeer = wlan_hdd_tdls_get_peer(pAdapter, peer);
	if (pTdlsPeer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: peer " MAC_ADDRESS_STR " does not exist",
			  __func__, MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto rel_lock;
	}
	mutex_unlock(&hdd_ctx->tdls_lock);

	if (0 != wlan_hdd_tdls_set_force_peer(pAdapter, peer, true)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS Add Force Peer Failed", __func__);
		status = -EINVAL;
		goto ret_status;
	}
	/* Update the peer mac to firmware, so firmware
	 * could update the connection table
	 */
	if (0 != wlan_hdd_tdls_update_peer_mac(pAdapter, peer,
	    eSME_TDLS_PEER_ADD_MAC_ADDR)) {
		hdd_err("TDLS Peer mac update Failed " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto ret_status;
	}

	hdd_ctx->tdls_external_peer_count++;

	/* validate if off channel is DFS channel */
	if (wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, chan)) {
		hdd_err("Resetting TDLS off-channel from %d to %d",
		       chan, CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT);
		chan = CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT;
	}

	if (0 != wlan_hdd_tdls_set_extctrl_param(pAdapter, peer,
						 chan, max_latency,
						 op_class, min_bandwidth)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS Set Peer's External Ctrl Parameter Failed",
			  __func__);
		status = -EINVAL;
		goto ret_status;
	}

	mutex_lock(&hdd_ctx->tdls_lock);
	pTdlsPeer = wlan_hdd_tdls_get_peer(pAdapter, peer);
	if (pTdlsPeer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: peer " MAC_ADDRESS_STR " does not exist",
			  __func__, MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto rel_lock;
	}
	if (0 != wlan_hdd_set_callback(pTdlsPeer, callback)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS set callback Failed", __func__);
		status = -EINVAL;
		goto rel_lock;
	}

	mutex_unlock(&hdd_ctx->tdls_lock);

	return status;
rel_lock:
	mutex_unlock(&hdd_ctx->tdls_lock);
ret_status:
	return status;
}

/**
 * wlan_hdd_tdls_extctrl_deconfig_peer() - de-configure an externally
 *                                         controllable TDLS peer
 * @pAdapter: HDD adapter
 * @peer: MAC address of the tdls peer
 *
 * Return: 0 if success; negative errno otherwisw
 */
int wlan_hdd_tdls_extctrl_deconfig_peer(struct hdd_adapter *pAdapter,
					const uint8_t *peer)
{
	hddTdlsPeer_t *pTdlsPeer;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	int status = 0;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s : NL80211_TDLS_TEARDOWN for " MAC_ADDRESS_STR,
		  __func__, MAC_ADDR_ARRAY(peer));
	if ((false == hdd_ctx->config->fTDLSExternalControl) ||
	    (false == hdd_ctx->config->fEnableTDLSImplicitTrigger)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s TDLS External control or Implicit Trigger not enabled ",
			  __func__);
		status = -ENOTSUPP;
		goto ret_status;
	}

	mutex_lock(&hdd_ctx->tdls_lock);
	pTdlsPeer = wlan_hdd_tdls_find_peer(pAdapter, peer);
	if (NULL == pTdlsPeer) {
		hdd_debug("peer matching" MAC_ADDRESS_STR "not found",
			   MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto rel_lock;
	}

	wlan_hdd_tdls_indicate_teardown(pAdapter, pTdlsPeer,
					eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON);
	hdd_send_wlan_tdls_teardown_event(eTDLS_TEARDOWN_EXT_CTRL,
					  pTdlsPeer->peerMac);
	mutex_unlock(&hdd_ctx->tdls_lock);

	if (0 != wlan_hdd_tdls_set_force_peer(pAdapter, peer, false)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s Failed", __func__);
		status = -EINVAL;
		goto ret_status;
	}

	if (hdd_ctx->tdls_external_peer_count)
		hdd_ctx->tdls_external_peer_count--;

	mutex_lock(&hdd_ctx->tdls_lock);
	pTdlsPeer = wlan_hdd_tdls_find_peer(pAdapter, peer);
	if (NULL == pTdlsPeer) {
		hdd_debug("peer matching" MAC_ADDRESS_STR "not found",
			   MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto rel_lock;
	}
	if (0 != wlan_hdd_set_callback(pTdlsPeer, NULL)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS set callback Failed", __func__);
		status = -EINVAL;
		goto rel_lock;
	}

	mutex_unlock(&hdd_ctx->tdls_lock);

	/* Update the peer mac to firmware, so firmware
	 * could update the connection table
	 */
	if (0 != wlan_hdd_tdls_update_peer_mac(pAdapter, peer,
	    eSME_TDLS_PEER_REMOVE_MAC_ADDR)) {
		hdd_err("TDLS Peer mac update Failed " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(peer));
		status = -EINVAL;
		goto ret_status;
	}

	goto ret_status;

rel_lock:
	mutex_unlock(&hdd_ctx->tdls_lock);
ret_status:
	return status;
}

/**
 * __wlan_hdd_cfg80211_tdls_oper() - helper function to handle cfg80211 operation
 *                                   on an TDLS peer
 * @wiphy: wiphy
 * @dev: net device
 * @peer: MAC address of the TDLS peer
 * @oper: cfg80211 TDLS operation
 *
 * Return: 0 on success; negative errno otherwise
 */
static int __wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
					 struct net_device *dev,
					 const uint8_t *peer,
					 enum nl80211_tdls_operation oper)
{
	struct hdd_adapter *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
		hdd_err("invalid session id: %d", pAdapter->sessionId);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_TDLS_OPER,
			 pAdapter->sessionId, oper));
	if (NULL == peer) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid arguments", __func__);
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (hdd_ctx->tdls_umac_comp_active) {
		status = wlan_cfg80211_tdls_oper(hdd_ctx->hdd_pdev,
						 dev, peer, oper);
		EXIT();
		return status;
	}

	EXIT();
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_tdls_oper() - handle cfg80211 operation on an TDLS peer
 * @wiphy: wiphy
 * @dev: net device
 * @peer: MAC address of the TDLS peer
 * @oper: cfg80211 TDLS operation
 *
 * Return: 0 on success; negative errno otherwise
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				const uint8_t *peer,
				enum nl80211_tdls_operation oper)
#else
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				uint8_t *peer,
				enum nl80211_tdls_operation oper)
#endif
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_tdls_oper(wiphy, dev, peer, oper);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_send_tdls_discover_req() - send out TDLS discovery for
 *                                              a TDLS peer
 * @wiphy: wiphy
 * @dev: net device
 * @peer: MAC address of the peer
 *
 * Return: 0 if success; negative errno otherwise
 */
int wlan_hdd_cfg80211_send_tdls_discover_req(struct wiphy *wiphy,
					     struct net_device *dev, u8 *peer)
{
	hdd_debug("tdls send discover req: " MAC_ADDRESS_STR,
		   MAC_ADDR_ARRAY(peer));
#if TDLS_MGMT_VERSION2
	return wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer,
					   WLAN_TDLS_DISCOVERY_REQUEST, 1, 0, 0,
					   NULL, 0);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
	return wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer,
					   WLAN_TDLS_DISCOVERY_REQUEST, 1, 0,
					   0, 0, NULL, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	return wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer,
					   WLAN_TDLS_DISCOVERY_REQUEST, 1, 0,
					   0, NULL, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
	return wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer,
					   WLAN_TDLS_DISCOVERY_REQUEST, 1, 0,
					   0, NULL, 0);
#else
	return wlan_hdd_cfg80211_tdls_mgmt(wiphy, dev, peer,
					   WLAN_TDLS_DISCOVERY_REQUEST, 1, 0,
					   NULL, 0);
#endif
#endif
}

#endif /* End of FEATURE_WLAN_TDLS */

/**
 * wlan_hdd_tdls_find_first_connected_peer() - find the 1st connected tdls peer
 * @adapter: Pointer to the HDD adapter
 *
 * This function searchs for the 1st connected TDLS peer
 *
 * Return: The first connected TDLS peer if found; NULL otherwise
 */
hddTdlsPeer_t *
wlan_hdd_tdls_find_first_connected_peer(struct hdd_adapter *adapter)
{
	int i;
	struct list_head *head;
	struct list_head *pos;
	hddTdlsPeer_t *curr_peer = NULL;
	tdlsCtx_t *hdd_tdls_ctx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (wlan_hdd_validate_context(hdd_ctx))
		return NULL;

	hdd_tdls_ctx = WLAN_HDD_GET_TDLS_CTX_PTR(adapter);
	if (NULL == hdd_tdls_ctx)
		return NULL;

	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &hdd_tdls_ctx->peer_list[i];
		list_for_each(pos, head) {
			curr_peer = list_entry(pos, hddTdlsPeer_t, node);
			if (curr_peer && (curr_peer->link_status ==
					eTDLS_LINK_CONNECTED)) {
				hdd_notice(MAC_ADDRESS_STR" eTDLS_LINK_CONNECTED",
					   MAC_ADDR_ARRAY(curr_peer->peerMac));
				return curr_peer;
			}
		}
	}
	return NULL;
}

/**
 * hdd_set_tdls_offchannel() - set tdls off-channel number
 * @adapter: Pointer to the HDD adapter
 * @offchanmode: tdls off-channel number
 *
 * This function sets tdls off-channel number
 *
 * Return: 0 on success; negative errno otherwise
 */
int hdd_set_tdls_offchannel(struct hdd_context *hdd_ctx, int offchannel)
{
	if ((true == hdd_ctx->config->fEnableTDLSOffChannel) &&
	    (eTDLS_SUPPORT_ENABLED == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXTERNAL_CONTROL == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == hdd_ctx->tdls_mode)) {
		if (offchannel < CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN ||
			offchannel > CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX) {
			hdd_err("Invalid tdls off channel %u", offchannel);
			return -EINVAL;
		}
	} else {
		hdd_err("Either TDLS or TDLS Off-channel is not enabled");
		return  -ENOTSUPP;
	}
	hdd_debug("change tdls off channel from %d to %d",
		   hdd_ctx->tdls_off_channel, offchannel);
	hdd_ctx->tdls_off_channel = offchannel;
	return 0;
}

/**
 * hdd_set_tdls_secoffchanneloffset() - set secondary tdls off-channel offset
 * @adapter: Pointer to the HDD adapter
 * @offchanmode: tdls off-channel offset
 *
 * This function sets 2nd tdls off-channel offset
 *
 * Return: 0 on success; negative errno otherwise
 */
int hdd_set_tdls_secoffchanneloffset(struct hdd_context *hdd_ctx, int offchanoffset)
{
	if ((true == hdd_ctx->config->fEnableTDLSOffChannel) &&
	    (eTDLS_SUPPORT_ENABLED == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXTERNAL_CONTROL == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == hdd_ctx->tdls_mode)) {
		hdd_ctx->tdls_channel_offset = 0;

		switch (offchanoffset) {
		case TDLS_SEC_OFFCHAN_OFFSET_0:
			hdd_ctx->tdls_channel_offset = (1 << BW_20_OFFSET_BIT);
			break;
		case TDLS_SEC_OFFCHAN_OFFSET_40PLUS:
		case TDLS_SEC_OFFCHAN_OFFSET_40MINUS:
			hdd_ctx->tdls_channel_offset = (1 << BW_40_OFFSET_BIT);
			break;
		case TDLS_SEC_OFFCHAN_OFFSET_80:
			hdd_ctx->tdls_channel_offset = (1 << BW_80_OFFSET_BIT);
			break;
		case TDLS_SEC_OFFCHAN_OFFSET_160:
			hdd_ctx->tdls_channel_offset = (1 << BW_160_OFFSET_BIT);
			break;
		default:
			hdd_err("Invalid tdls secondary off channel offset %d",
				offchanoffset);
			return -EINVAL;
		} /* end switch */
	} else {
		hdd_err("Either TDLS or TDLS Off-channel is not enabled");
		return  -ENOTSUPP;
	}
	hdd_debug("change tdls secondary off channel offset to 0x%x",
		   hdd_ctx->tdls_channel_offset);
	return 0;
}

/**
 * hdd_set_tdls_offchannelmode() - set tdls off-channel mode
 * @adapter: Pointer to the HDD adapter
 * @offchanmode: tdls off-channel mode
 *
 * This function sets tdls off-channel mode
 *
 * Return: 0 on success; negative errno otherwise
 */
int hdd_set_tdls_offchannelmode(struct hdd_adapter *adapter, int offchanmode)
{
	hddTdlsPeer_t *conn_peer = NULL;
	struct hdd_station_ctx *hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	sme_tdls_chan_switch_params chan_switch_params;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int ret_value = 0;

	if (offchanmode < ENABLE_CHANSWITCH ||
			offchanmode > DISABLE_CHANSWITCH) {
		hdd_err("Invalid tdls off channel mode %d", offchanmode);
		ret_value = -EINVAL;
		goto ret_status;
	}
	if (eConnectionState_Associated != hdd_sta_ctx->conn_info.connState) {
		hdd_err("tdls off channel mode req in not associated state %d",
			offchanmode);
		ret_value = -EPERM;
		goto ret_status;
	}
	if ((true == hdd_ctx->config->fEnableTDLSOffChannel) &&
		(eTDLS_SUPPORT_ENABLED == hdd_ctx->tdls_mode ||
		 eTDLS_SUPPORT_EXTERNAL_CONTROL == hdd_ctx->tdls_mode ||
		 eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == hdd_ctx->tdls_mode)) {
		mutex_lock(&hdd_ctx->tdls_lock);
		conn_peer = wlan_hdd_tdls_find_first_connected_peer(adapter);
		if (NULL == conn_peer) {
			hdd_err("No TDLS Connected Peer");
			ret_value = -EPERM;
			goto rel_lock;
		}
	} else {
		hdd_err("TDLS Connection not supported");
		ret_value = -ENOTSUPP;
		goto ret_status;
	}

	hdd_debug("TDLS swmode=%d tdls_off_channel %d offchanoffset %d",
		   offchanmode, hdd_ctx->tdls_off_channel,
		   hdd_ctx->tdls_channel_offset);

	switch (offchanmode) {
	case ENABLE_CHANSWITCH:
		if (hdd_ctx->tdls_off_channel &&
			hdd_ctx->tdls_channel_offset) {
			chan_switch_params.tdls_off_channel =
				hdd_ctx->tdls_off_channel;
			chan_switch_params.tdls_off_ch_bw_offset =
				hdd_ctx->tdls_channel_offset;
			chan_switch_params.opclass =
			   wlan_hdd_find_opclass(WLAN_HDD_GET_HAL_CTX(adapter),
					chan_switch_params.tdls_off_channel,
					chan_switch_params.tdls_off_ch_bw_offset);
		} else {
			hdd_err("TDLS off-channel parameters are not set yet!!!");
			ret_value = -EINVAL;
			goto rel_lock;
		}
		break;
	case DISABLE_CHANSWITCH:
		chan_switch_params.tdls_off_channel = 0;
		chan_switch_params.tdls_off_ch_bw_offset = 0;
		chan_switch_params.opclass = 0;
		break;
	default:
		hdd_err("Incorrect Parameters mode: %d tdls_off_channel: %d offchanoffset: %d",
			offchanmode, hdd_ctx->tdls_off_channel,
			hdd_ctx->tdls_channel_offset);
		ret_value = -EINVAL;
		goto rel_lock;
	} /* end switch */

	chan_switch_params.vdev_id = adapter->sessionId;
	chan_switch_params.tdls_off_ch_mode = offchanmode;
	chan_switch_params.is_responder =
		conn_peer->is_responder;
	qdf_mem_copy(&chan_switch_params.peer_mac_addr,
		     &conn_peer->peerMac,
		     sizeof(tSirMacAddr));
	hdd_debug("Peer " MAC_ADDRESS_STR
		 " vdevId: %d, off channel: %d, offset: %d, mode: %d, is_responder: %d",
		 MAC_ADDR_ARRAY(chan_switch_params.peer_mac_addr),
		 chan_switch_params.vdev_id,
		 chan_switch_params.tdls_off_channel,
		 chan_switch_params.tdls_off_ch_bw_offset,
		 chan_switch_params.tdls_off_ch_mode,
		 chan_switch_params.is_responder);
	mutex_unlock(&hdd_ctx->tdls_lock);

	status = sme_send_tdls_chan_switch_req(WLAN_HDD_GET_HAL_CTX(adapter),
			&chan_switch_params);

	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to send channel switch request to sme");
		ret_value = -EINVAL;
		goto ret_status;
	}

	hdd_ctx->tdls_fw_off_chan_mode = offchanmode;

	if (ENABLE_CHANSWITCH == offchanmode) {
		mutex_lock(&hdd_ctx->tdls_lock);
		conn_peer = wlan_hdd_tdls_find_first_connected_peer(adapter);
		if (NULL == conn_peer) {
			hdd_err("No TDLS Connected Peer");
			ret_value = -EPERM;
			goto rel_lock;
		}
		conn_peer->pref_off_chan_num =
			chan_switch_params.tdls_off_channel;
		conn_peer->op_class_for_pref_off_chan =
			chan_switch_params.opclass;
		goto rel_lock;
	}
	goto ret_status;

rel_lock:
	mutex_unlock(&hdd_ctx->tdls_lock);
ret_status:
	return ret_value;
}

/**
 * hdd_set_tdls_scan_type - set scan during active tdls session
 * @hdd_ctx: ptr to hdd context.
 * @val: scan type value: 0 or 1.
 *
 * Set scan type during tdls session. If set to 1, that means driver
 * shall maintain tdls link and allow scan regardless if tdls peer is
 * buffer sta capable or not and/or if device is sleep sta capable or
 * not. If tdls peer is not buffer sta capable then during scan there
 * will be loss of Rx packets and Tx would stop when device moves away
 * from tdls channel. If set to 0, then driver shall teardown tdls link
 * before initiating scan if peer is not buffer sta capable and device
 * is not sleep sta capable. By default, scan type is set to 0.
 *
 * Return: success (0) or failure (errno value)
 */
int hdd_set_tdls_scan_type(struct hdd_context *hdd_ctx, int val)
{
	if ((val != 0) && (val != 1)) {
		hdd_err("Incorrect value of tdls scan type: %d", val);
		return -EINVAL;
	}

	hdd_ctx->config->enable_tdls_scan = val;
	return 0;
}

/**
 * wlan_hdd_tdls_teardown_links() - teardown tdls links
 * @hddCtx : pointer to hdd context
 *
 * Return: 0 if success else non zero
 */
static int wlan_hdd_tdls_teardown_links(struct hdd_context *hddctx,
					uint32_t mode)
{
	uint16_t connected_tdls_peers = 0;
	uint8_t staidx;
	hddTdlsPeer_t *curr_peer;
	struct hdd_adapter *adapter;
	int ret = 0;

	if (eTDLS_SUPPORT_NOT_ENABLED == hddctx->tdls_mode) {
		hdd_info("TDLS mode is disabled OR not enabled in FW");
		return 0;
	}

	adapter = hdd_get_adapter(hddctx, QDF_STA_MODE);

	if (adapter == NULL) {
		hdd_info("Station Adapter Not Found");
		return 0;
	}

	connected_tdls_peers = wlan_hdd_tdls_connected_peers(adapter);

	if (!connected_tdls_peers)
		return 0;

	for (staidx = 0; staidx < hddctx->max_num_tdls_sta;
							staidx++) {
		if (!hddctx->tdlsConnInfo[staidx].staId)
			continue;

		mutex_lock(&hddctx->tdls_lock);
		curr_peer = wlan_hdd_tdls_find_all_peer(hddctx,
			hddctx->tdlsConnInfo[staidx].peerMac.bytes);

		if (!curr_peer) {
			mutex_unlock(&hddctx->tdls_lock);
			continue;
		}

		/* Check if connected peer supports more than one stream */
		if (curr_peer->spatial_streams == TDLS_NSS_1x1_MODE) {
			mutex_unlock(&hddctx->tdls_lock);
			continue;
		}

		hdd_debug("Indicate TDLS teardown (staId %d)",
			 curr_peer->staId);

		wlan_hdd_tdls_indicate_teardown(
					curr_peer->pHddTdlsCtx->pAdapter,
					curr_peer,
					eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON);
		hddctx->tdls_teardown_peers_cnt++;
		mutex_unlock(&hddctx->tdls_lock);
	}
	mutex_lock(&hddctx->tdls_lock);
	if (hddctx->tdls_teardown_peers_cnt >= 1) {
		hddctx->tdls_nss_switch_in_progress = true;
		hdd_debug("TDLS peers to be torn down = %d",
			 hddctx->tdls_teardown_peers_cnt);
		/*  Antenna switch 2x2 to 1x1 */
		if (mode == HDD_ANTENNA_MODE_1X1) {
			hddctx->tdls_nss_transition_mode =
				TDLS_NSS_TRANSITION_2x2_to_1x1;
			ret = -EAGAIN;
		} else {
		/*  Antenna switch 1x1 to 2x2 */
			hddctx->tdls_nss_transition_mode =
				TDLS_NSS_TRANSITION_1x1_to_2x2;
			ret = 0;
		}
		hdd_debug("TDLS teardown for antenna switch operation starts");
	}
	mutex_unlock(&hddctx->tdls_lock);
	return ret;
}

/**
 * wlan_hdd_tdls_antenna_switch() - Dynamic TDLS antenna  switch 1x1 <-> 2x2
 * antenna mode in standalone station
 * @hdd_ctx: Pointer to hdd contex
 * @adapter: Pointer to hdd adapter
 *
 * Return: 0 if success else non zero
 */
int wlan_hdd_tdls_antenna_switch(struct hdd_context *hdd_ctx,
				 struct hdd_adapter *adapter,
				 uint32_t mode)
{
	uint8_t tdls_peer_cnt;
	uint32_t vdev_nss;
	struct hdd_station_ctx *sta_ctx;

	if (hdd_ctx->connected_peer_count == 0)
		return 0;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	/* Check whether TDLS antenna switch is in progress */
	if (hdd_ctx->tdls_nss_switch_in_progress) {
		if (hdd_ctx->tdls_nss_teardown_complete == false) {
			hdd_err("TDLS antenna switch is in progress");
			return -EAGAIN;
		}
		goto tdls_ant_sw_done;
	}

	/* Check whether TDLS is connected or not */
	mutex_lock(&hdd_ctx->tdls_lock);
	tdls_peer_cnt = hdd_ctx->connected_peer_count;
	mutex_unlock(&hdd_ctx->tdls_lock);
	if (tdls_peer_cnt <= 0) {
		hdd_info("No TDLS connection established");
		goto tdls_ant_sw_done;
	}

	/* Check the supported nss for TDLS */
	if (IS_5G_CH(sta_ctx->conn_info.operationChannel))
		vdev_nss = CFG_TDLS_NSS(
			hdd_ctx->config->vdev_type_nss_5g);
	else
		vdev_nss = CFG_TDLS_NSS(
			hdd_ctx->config->vdev_type_nss_2g);

	if (vdev_nss == HDD_ANTENNA_MODE_1X1) {
		hdd_info("Supported NSS is 1X1, no need to teardown TDLS links");
		goto tdls_ant_sw_done;
	}

	/* teardown all the tdls connections */
	return wlan_hdd_tdls_teardown_links(hdd_ctx, mode);

tdls_ant_sw_done:
	return 0;
}

bool cds_check_is_tdls_allowed(enum tQDF_ADAPTER_MODE device_mode)
{
	bool state = false;
	uint32_t count;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	count = policy_mgr_get_connection_count(hdd_ctx->hdd_psoc);

	if (count > 1)
		state = false;
	else if (device_mode == QDF_STA_MODE ||
		device_mode == QDF_P2P_CLIENT_MODE)
		state = true;

	/* If any concurrency is detected */
	if (!state)
		hdd_ipa_set_tx_flow_info();

	return state;
}

QDF_STATUS hdd_tdls_register_tdls_peer(void *userdata, uint32_t vdev_id,
				       const uint8_t *mac, uint16_t sta_id,
				       uint8_t ucastsig, uint8_t qos)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hddctx;

	hddctx = userdata;
	if (!hddctx) {
		hdd_err("Invalid hddctx");
		return QDF_STATUS_E_INVAL;
	}
	adapter = hdd_get_adapter_by_vdev(hddctx, vdev_id);
	if (!adapter) {
		hdd_err("Invalid adapter");
		return QDF_STATUS_E_FAILURE;
	}

	return hdd_roam_register_tdlssta(adapter, mac, sta_id, ucastsig, qos);
}

QDF_STATUS hdd_tdls_deregister_tdl_peer(void *userdata,
					uint32_t vdev_id, uint8_t sta_id)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hddctx;

	hddctx = userdata;
	if (!hddctx) {
		hdd_err("Invalid hddctx");
		return QDF_STATUS_E_INVAL;
	}
	adapter = hdd_get_adapter_by_vdev(hddctx, vdev_id);
	if (!adapter) {
		hdd_err("Invalid adapter");
		return QDF_STATUS_E_FAILURE;
	}

	return hdd_roam_deregister_tdlssta(adapter, sta_id);
}
