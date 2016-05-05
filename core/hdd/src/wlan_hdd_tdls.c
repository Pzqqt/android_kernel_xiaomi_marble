/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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
#include "cds_sched.h"
#include "wma_types.h"

static int32_t wlan_hdd_tdls_peer_reset_discovery_processed(tdlsCtx_t *
							    pHddTdlsCtx);
static void wlan_hdd_tdls_timers_destroy(tdlsCtx_t *pHddTdlsCtx);
int wpa_tdls_is_allowed_force_peer(tdlsCtx_t *pHddTdlsCtx, u8 *mac);
static void wlan_hdd_tdls_pre_setup(struct work_struct *work);

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
static void wlan_hdd_tdls_determine_channel_opclass(hdd_context_t *hddctx,
			hdd_adapter_t *adapter, hddTdlsPeer_t *curr_peer,
			uint32_t *channel, uint32_t *opclass)
{
	hdd_station_ctx_t *hdd_sta_ctx;

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
 * wlan_hdd_tdls_disable_offchan_and_teardown_links - Disable offchannel
 * and teardown TDLS links
 * @hddCtx : pointer to hdd context
 *
 * Return: None
 */
void wlan_hdd_tdls_disable_offchan_and_teardown_links(hdd_context_t *hddctx)
{
	u16 connected_tdls_peers = 0;
	u8 staidx;
	hddTdlsPeer_t *curr_peer = NULL;
	hdd_adapter_t *adapter = NULL;

	if (eTDLS_SUPPORT_NOT_ENABLED == hddctx->tdls_mode) {
		hddLog(LOG1, FL("TDLS mode is disabled OR not enabled in FW"));
		return ;
	}

	adapter = hdd_get_adapter(hddctx, QDF_STA_MODE);

	if (adapter == NULL) {
		hddLog(LOGE, FL("Station Adapter Not Found"));
		return;
	}

	connected_tdls_peers = wlan_hdd_tdls_connected_peers(adapter);

	if (!connected_tdls_peers)
		return ;

	/* TDLS is not supported in case of concurrency.
	 * Disable TDLS Offchannel in FW to avoid more
	 * than two concurrent channels and generate TDLS
	 * teardown indication to supplicant.
	 * Below function Finds the first connected peer and
	 * disables TDLS offchannel for that peer.
	 * FW enables TDLS offchannel only when there is
	 * one TDLS peer. When there are more than one TDLS peer,
	 * there will not be TDLS offchannel in FW.
	 * So to avoid sending multiple request to FW, for now,
	 * just invoke offchannel mode functions only once
	 */
	hdd_set_tdls_offchannel(hddctx, hddctx->config->fTDLSPrefOffChanNum);
	hdd_set_tdls_secoffchanneloffset(hddctx,
			TDLS_SEC_OFFCHAN_OFFSET_40PLUS);
	hdd_set_tdls_offchannelmode(adapter, DISABLE_CHANSWITCH);

	for (staidx = 0; staidx < hddctx->max_num_tdls_sta;
							staidx++) {
		if (!hddctx->tdlsConnInfo[staidx].staId)
			continue;

		curr_peer = wlan_hdd_tdls_find_all_peer(hddctx,
				hddctx->tdlsConnInfo[staidx].peerMac.bytes);

		if (!curr_peer)
			continue;

		hddLog(LOG1, FL("indicate TDLS teardown (staId %d)"),
				curr_peer->staId);

		wlan_hdd_tdls_indicate_teardown(
					curr_peer->pHddTdlsCtx->pAdapter,
					curr_peer,
					eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON);
		hdd_send_wlan_tdls_teardown_event(eTDLS_TEARDOWN_CONCURRENCY,
			curr_peer->peerMac);
	}
}

/**
 * hdd_tdls_notify_mode_change - Notify mode change
 * @adapter: pointer to hdd adapter
 * @hddCtx : pointer to hdd context
 *
 * Return: None
 */
void hdd_tdls_notify_mode_change(hdd_adapter_t *adapter, hdd_context_t *hddctx)
{
	if (adapter->device_mode != QDF_STA_MODE)
		wlan_hdd_tdls_disable_offchan_and_teardown_links(hddctx);
}


/**
 * wlan_hdd_tdls_pre_setup_init_work() - schedule work for tdls pre-setup
 * @pHddTdlsCtx: HDD TDLS context
 * @curr_candidate: current candidate peer
 *
 * Return: None
 */
void wlan_hdd_tdls_pre_setup_init_work(tdlsCtx_t *pHddTdlsCtx,
				       hddTdlsPeer_t *curr_candidate)
{
	if (!pHddTdlsCtx || !curr_candidate) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: either pHddTdlsCtx or curr_candidate is null",
			  __func__);
		return;
	}

	if (TDLS_CTX_MAGIC != pHddTdlsCtx->magic) {
		/* When TDLS discovery attempt for a peer reaches to max configured
		 * threshold then tdls support for that peer would be disabled and
		 * in that case, ignore discovery trigger from FW for that peer.
		 */
		if (eTDLS_CAP_NOT_SUPPORTED == curr_candidate->tdls_support) {
			hddLog(LOGW,
				  "%s: tdls_support is marked disabled for peer: "
				  MAC_ADDRESS_STR
				  ", ignore pre_setup_init_work", __func__,
				  MAC_ADDR_ARRAY(curr_candidate->peerMac));
			return;
		}

		pHddTdlsCtx->curr_candidate = curr_candidate;
		pHddTdlsCtx->magic = TDLS_CTX_MAGIC;

		schedule_work(&pHddTdlsCtx->implicit_setup);
	}
}

/**
 * wlan_hdd_tdls_pre_setup_init_work() - get value of discovery counter sent
 * @pHddCtx: HDD context
 *
 * Return: the value of the transmitted TDLS discovery counter
 */
static uint32_t wlan_hdd_tdls_discovery_sent_cnt(hdd_context_t *pHddCtx)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *pAdapter = NULL;
	tdlsCtx_t *pHddTdlsCtx = NULL;
	QDF_STATUS status = 0;
	uint32_t count = 0;

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			count = count + pHddTdlsCtx->discovery_sent_cnt;
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	return count;
}

/**
 * wlan_hdd_tdls_check_power_save_prohibited() - set/clear proper TDLS power
 *                                               save probihited bit
 * @pAdapter: HDD adapter handle
 *
 * Ensure TDLS power save probihited bit is set/cleared properly
 *
 * Return: None
 */
static void wlan_hdd_tdls_check_power_save_prohibited(hdd_adapter_t *pAdapter)
{
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if ((NULL == pHddTdlsCtx) || (NULL == pHddCtx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddCtx or pHddTdlsCtx points to NULL"));
		return;
	}

	if ((0 == pHddCtx->connected_peer_count) &&
	    (0 == wlan_hdd_tdls_discovery_sent_cnt(pHddCtx))) {
		sme_set_tdls_power_save_prohibited(WLAN_HDD_GET_HAL_CTX
							   (pHddTdlsCtx->pAdapter),
						   pAdapter->sessionId, 0);
		return;
	}
	sme_set_tdls_power_save_prohibited(WLAN_HDD_GET_HAL_CTX
						   (pHddTdlsCtx->pAdapter),
					   pAdapter->sessionId, 1);
	return;
}

/**
 * wlan_hdd_tdls_free_scan_request() - free tdls scan request
 * @tdls_scan_ctx: tdls scan context
 *
 * Return: None
 */
static void wlan_hdd_tdls_free_scan_request(tdls_scan_context_t *tdls_scan_ctx)
{
	if (NULL == tdls_scan_ctx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("tdls_scan_ctx is NULL"));
		return;
	}

	tdls_scan_ctx->attempt = 0;
	tdls_scan_ctx->reject = 0;
	tdls_scan_ctx->magic = 0;
	tdls_scan_ctx->scan_request = NULL;
	return;
}

/**
 * wlan_hdd_tdls_discovery_timeout_peer_cb() - tdls discovery timeout callback
 * @userData: tdls context
 *
 * Return: None
 */
static void wlan_hdd_tdls_discovery_timeout_peer_cb(void *userData)
{
	int i;
	struct list_head *head;
	hddTdlsPeer_t *tmp;
	struct list_head *pos, *q;
	tdlsCtx_t *pHddTdlsCtx;
	hdd_context_t *pHddCtx;

	ENTER();

	pHddTdlsCtx = (tdlsCtx_t *) userData;

	if ((NULL == pHddTdlsCtx) || (NULL == pHddTdlsCtx->pAdapter)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddTdlsCtx or pAdapter points to NULL"));
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != pHddTdlsCtx->pAdapter->magic) {
		hddLog(LOGE, FL("pAdapter has invalid magic"));
		return;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pHddTdlsCtx->pAdapter);
	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	mutex_lock(&pHddCtx->tdls_lock);

	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];
		list_for_each_safe(pos, q, head) {
			tmp = list_entry(pos, hddTdlsPeer_t, node);
			if (eTDLS_LINK_DISCOVERING == tmp->link_status) {
				mutex_unlock(&pHddCtx->tdls_lock);
				hddLog(LOG1,
					  "%s: " MAC_ADDRESS_STR
					  " to idle state", __func__,
					  MAC_ADDR_ARRAY(tmp->peerMac));
				wlan_hdd_tdls_set_peer_link_status(tmp,
								   eTDLS_LINK_IDLE,
								   eTDLS_LINK_NOT_SUPPORTED);
				mutex_lock(&pHddCtx->tdls_lock);
			}
		}
	}

	pHddTdlsCtx->discovery_sent_cnt = 0;
	wlan_hdd_tdls_check_power_save_prohibited(pHddTdlsCtx->pAdapter);

	mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
	return;
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
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddTdlsCtx is NULL"));
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
 * wlan_hdd_tdls_schedule_scan() - schedule scan for tdls
 * @work: work_struct used to find tdls scan context
 *
 * Return: None
 */
static void wlan_hdd_tdls_schedule_scan(struct work_struct *work)
{
	tdls_scan_context_t *scan_ctx =
		container_of(work, tdls_scan_context_t, tdls_scan_work.work);

	if (NULL == scan_ctx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("scan_ctx is NULL"));
		return;
	}

	if (unlikely(TDLS_CTX_MAGIC != scan_ctx->magic))
		return;

	scan_ctx->attempt++;

	wlan_hdd_cfg80211_scan(scan_ctx->wiphy,
			       scan_ctx->scan_request);
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

	hddLog(LOG1,
		FL(
		   "Setting tdls state and param in fw: vdev_id: %d, tdls_state: %d, notification_interval_ms: %d, tx_discovery_threshold: %d, tx_teardown_threshold: %d, rssi_teardown_threshold: %d, rssi_delta: %d, tdls_options: 0x%x, peer_traffic_ind_window: %d, peer_traffic_response_timeout: %d, puapsd_mask: 0x%x, puapsd_inactivity_time: %d, puapsd_rx_frame_threshold: %d, teardown_notification_ms: %d, tdls_peer_kickout_threshold: %d"
		),
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
 * wlan_hdd_tdls_monitor_timers_stop() - stop all monitoring timers
 * @hdd_tdls_ctx: TDLS context
 *
 * Return: none
 */
static void wlan_hdd_tdls_monitor_timers_stop(tdlsCtx_t *hdd_tdls_ctx)
{
	qdf_mc_timer_stop(&hdd_tdls_ctx->peerDiscoveryTimeoutTimer);
}

/**
 * wlan_hdd_tdls_timers_stop() - stop all the tdls timers running
 * @hdd_tdls_ctx: TDLS context
 *
 * Return: none
 */
static void wlan_hdd_tdls_timers_stop(tdlsCtx_t *hdd_tdls_ctx)
{
	wlan_hdd_tdls_monitor_timers_stop(hdd_tdls_ctx);
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
				peer->staId = 0;
				peer->discovery_attempt = 0;
			}
		}
	}
}

/**
 * hdd_tdls_context_init() - Init TDLS context
 * @hdd_ctx:	HDD context
 *
 * Initialize TDLS global context.
 *
 * Return: None
 */
void hdd_tdls_context_init(hdd_context_t *hdd_ctx)
{
	mutex_init(&hdd_ctx->tdls_lock);
}

/**
 * hdd_tdls_context_destroy() - Destroy TDLS context
 * @hdd_ctx:	HDD context
 *
 * Destroy TDLS global context.
 *
 * Return: None
 */
void hdd_tdls_context_destroy(hdd_context_t *hdd_ctx)
{
	mutex_destroy(&hdd_ctx->tdls_lock);
}

/**
 * wlan_hdd_tdls_init() - tdls initializaiton
 * @pAdapter: hdd adapter
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_init(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx;
	int i;
	uint8_t staIdx;
	tdlsInfo_t *tInfo;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (NULL == pHddCtx)
		return -EINVAL;

	mutex_lock(&pHddCtx->tdls_lock);

	if (false == pHddCtx->config->fEnableTDLSSupport) {
		pHddCtx->tdls_mode = eTDLS_SUPPORT_NOT_ENABLED;
		pAdapter->sessionCtx.station.pHddTdlsCtx = NULL;
		mutex_unlock(&pHddCtx->tdls_lock);
		hddLog(LOGE,
		       FL("TDLS not enabled (%d) or FW doesn't support"),
		       pHddCtx->config->fEnableTDLSSupport);
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
		mutex_unlock(&pHddCtx->tdls_lock);
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
			mutex_unlock(&pHddCtx->tdls_lock);
			hddLog(LOGE, FL("malloc failed!"));
			return -ENOMEM;
		}
		/* initialize TDLS pAdater context */
		qdf_mem_zero(pHddTdlsCtx, sizeof(tdlsCtx_t));

		qdf_mc_timer_init(&pHddTdlsCtx->peerDiscoveryTimeoutTimer,
				  QDF_TIMER_TYPE_SW,
				  wlan_hdd_tdls_discovery_timeout_peer_cb,
				  pHddTdlsCtx);

		pAdapter->sessionCtx.station.pHddTdlsCtx = pHddTdlsCtx;
		for (i = 0; i < TDLS_PEER_LIST_SIZE; i++)
			INIT_LIST_HEAD(&pHddTdlsCtx->peer_list[i]);
	} else {
		pHddTdlsCtx = pAdapter->sessionCtx.station.pHddTdlsCtx;

		wlan_hdd_tdls_timers_stop(pHddTdlsCtx);

		wlan_hdd_tdls_del_non_forced_peers(pHddTdlsCtx);

		pHddCtx->connected_peer_count = 0;
	}

	/* initialize TDLS global context */
	pHddCtx->connected_peer_count = 0;
	sme_set_tdls_power_save_prohibited(WLAN_HDD_GET_HAL_CTX(pAdapter),
					   pAdapter->sessionId, 0);

	pHddCtx->tdls_scan_ctxt.magic = 0;
	pHddCtx->tdls_scan_ctxt.attempt = 0;
	pHddCtx->tdls_scan_ctxt.reject = 0;
	pHddCtx->tdls_scan_ctxt.scan_request = NULL;

	if (pHddCtx->config->fEnableTDLSSleepSta ||
	    pHddCtx->config->fEnableTDLSBufferSta ||
	    pHddCtx->config->fEnableTDLSOffChannel)
		pHddCtx->max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA_P_UAPSD_OFFCHAN;
	else
		pHddCtx->max_num_tdls_sta = HDD_MAX_NUM_TDLS_STA;

	hddLog(LOG1, FL("max_num_tdls_sta: %d"), pHddCtx->max_num_tdls_sta);

	for (staIdx = 0; staIdx < pHddCtx->max_num_tdls_sta; staIdx++) {
		pHddCtx->tdlsConnInfo[staIdx].staId = 0;
		pHddCtx->tdlsConnInfo[staIdx].sessionId = 255;
		qdf_mem_zero(&pHddCtx->tdlsConnInfo[staIdx].peerMac,
			     QDF_MAC_ADDR_SIZE);
	}

	pHddTdlsCtx->pAdapter = pAdapter;

	pHddTdlsCtx->curr_candidate = NULL;
	pHddTdlsCtx->magic = 0;

	/* remember configuration even if it is not used right now. it could be used later */
	pHddTdlsCtx->threshold_config.tx_period_t =
		pHddCtx->config->fTDLSTxStatsPeriod;
	pHddTdlsCtx->threshold_config.tx_packet_n =
		pHddCtx->config->fTDLSTxPacketThreshold;
	pHddTdlsCtx->threshold_config.discovery_tries_n =
		pHddCtx->config->fTDLSMaxDiscoveryAttempt;
	pHddTdlsCtx->threshold_config.idle_packet_n =
		pHddCtx->config->fTDLSIdlePacketThreshold;
	pHddTdlsCtx->threshold_config.rssi_trigger_threshold =
		pHddCtx->config->fTDLSRSSITriggerThreshold;
	pHddTdlsCtx->threshold_config.rssi_teardown_threshold =
		pHddCtx->config->fTDLSRSSITeardownThreshold;
	pHddTdlsCtx->threshold_config.rssi_delta =
		pHddCtx->config->fTDLSRSSIDelta;

	if (false == pHddCtx->config->fEnableTDLSImplicitTrigger) {
		pHddCtx->tdls_mode = eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY;
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s TDLS Implicit trigger not enabled!", __func__);
	} else if (true == pHddCtx->config->fTDLSExternalControl) {
		pHddCtx->tdls_mode = eTDLS_SUPPORT_EXTERNAL_CONTROL;
	} else {
		pHddCtx->tdls_mode = eTDLS_SUPPORT_ENABLED;
	}

	INIT_WORK(&pHddTdlsCtx->implicit_setup, wlan_hdd_tdls_pre_setup);

	INIT_DELAYED_WORK(&pHddCtx->tdls_scan_ctxt.tdls_scan_work,
			  wlan_hdd_tdls_schedule_scan);

	/*
	 * Release tdls lock before calling in SME api
	 * which would try to acquire sme lock.
	 */
	mutex_unlock(&pHddCtx->tdls_lock);
	tInfo = qdf_mem_malloc(sizeof(tdlsInfo_t));
	if (NULL == tInfo) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("qdf_mem_malloc failed for tInfo"));
		qdf_mc_timer_destroy(&pHddTdlsCtx->peerDiscoveryTimeoutTimer);
		qdf_mem_free(pHddTdlsCtx);
		return -ENOMEM;
	}

	tInfo->vdev_id = pAdapter->sessionId;
	tInfo->tdls_state = pHddCtx->tdls_mode;
	tInfo->notification_interval_ms =
		pHddTdlsCtx->threshold_config.tx_period_t;
	tInfo->tx_discovery_threshold =
		pHddTdlsCtx->threshold_config.tx_packet_n;
	tInfo->tx_teardown_threshold =
		pHddTdlsCtx->threshold_config.idle_packet_n;
	tInfo->rssi_teardown_threshold =
		pHddTdlsCtx->threshold_config.rssi_teardown_threshold;
	tInfo->rssi_delta = pHddTdlsCtx->threshold_config.rssi_delta;
	tInfo->tdls_options = 0;

	if (pHddCtx->config->fEnableTDLSOffChannel) {
		tInfo->tdls_options |= ENA_TDLS_OFFCHAN;
		pHddCtx->tdls_fw_off_chan_mode = ENABLE_CHANSWITCH;
	}

	if (pHddCtx->config->fEnableTDLSBufferSta)
		tInfo->tdls_options |= ENA_TDLS_BUFFER_STA;
	if (pHddCtx->config->fEnableTDLSSleepSta)
		tInfo->tdls_options |= ENA_TDLS_SLEEP_STA;
	tInfo->peer_traffic_ind_window = pHddCtx->config->fTDLSPuapsdPTIWindow;
	tInfo->peer_traffic_response_timeout =
		pHddCtx->config->fTDLSPuapsdPTRTimeout;
	tInfo->puapsd_mask = pHddCtx->config->fTDLSUapsdMask;
	tInfo->puapsd_inactivity_time =
		pHddCtx->config->fTDLSPuapsdInactivityTimer;
	tInfo->puapsd_rx_frame_threshold =
		pHddCtx->config->fTDLSRxFrameThreshold;
	tInfo->teardown_notification_ms =
		pHddCtx->config->tdls_idle_timeout;
	tInfo->tdls_peer_kickout_threshold =
		pHddCtx->config->tdls_peer_kickout_threshold;
	dump_tdls_state_param_setting(tInfo);

	qdf_ret_status = sme_update_fw_tdls_state(pHddCtx->hHal, tInfo, true);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		qdf_mem_free(tInfo);
		qdf_mc_timer_destroy(&pHddTdlsCtx->peerDiscoveryTimeoutTimer);
		qdf_mem_free(pHddTdlsCtx);
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_tdls_exit() - TDLS de-initialization
 * @pAdapter: HDD adapter
 *
 * Return: None
 */
void wlan_hdd_tdls_exit(hdd_adapter_t *pAdapter)
{
	tdlsCtx_t *pHddTdlsCtx;
	hdd_context_t *pHddCtx;
	tdlsInfo_t *tInfo;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	if (!pHddCtx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  FL("pHddCtx is NULL"));
		return;
	}

	if (!test_bit(TDLS_INIT_DONE, &pAdapter->event_flags)) {
		hddLog(LOGE, FL("TDLS init was not done, exit"));
		return;
	}

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		/* TDLS context can be null and might have been freed up during
		 * cleanup for STA adapter
		 */
		hddLog(LOG2,
			FL("pHddTdlsCtx is NULL, adapter device mode: %s(%d)"),
			hdd_device_mode_to_string(pAdapter->device_mode),
			pAdapter->device_mode);
		goto done;
	}

	cds_flush_work(&pHddTdlsCtx->implicit_setup);
	cds_flush_delayed_work(&pHddCtx->tdls_scan_ctxt.tdls_scan_work);

	mutex_lock(&pHddCtx->tdls_lock);

	/* must stop timer here before freeing peer list, because peerIdleTimer is
	   part of peer list structure. */
	wlan_hdd_tdls_timers_destroy(pHddTdlsCtx);
	wlan_hdd_tdls_free_list(pHddTdlsCtx);

	mutex_unlock(&pHddCtx->tdls_lock);

	wlan_hdd_tdls_free_scan_request(&pHddCtx->tdls_scan_ctxt);

	/* No need to post message during driver unlaod because MC thread is
	   already shutdown */
	if (!cds_is_driver_unloading()) {
		tInfo = qdf_mem_malloc(sizeof(tdlsInfo_t));
		if (NULL != tInfo) {
			tInfo->vdev_id = pAdapter->sessionId;
			tInfo->tdls_state = eTDLS_SUPPORT_DISABLED;
			tInfo->notification_interval_ms =
				pHddTdlsCtx->threshold_config.tx_period_t;
			tInfo->tx_discovery_threshold =
				pHddTdlsCtx->threshold_config.tx_packet_n;
			tInfo->tx_teardown_threshold =
				pHddTdlsCtx->threshold_config.idle_packet_n;
			tInfo->rssi_teardown_threshold =
				pHddTdlsCtx->threshold_config.
				rssi_teardown_threshold;
			tInfo->rssi_delta =
				pHddTdlsCtx->threshold_config.rssi_delta;
			tInfo->tdls_options = 0;
			if (pHddCtx->config->fEnableTDLSOffChannel)
				tInfo->tdls_options |= ENA_TDLS_OFFCHAN;
			if (pHddCtx->config->fEnableTDLSBufferSta)
				tInfo->tdls_options |= ENA_TDLS_BUFFER_STA;
			if (pHddCtx->config->fEnableTDLSSleepSta)
				tInfo->tdls_options |= ENA_TDLS_SLEEP_STA;
			tInfo->peer_traffic_ind_window =
				pHddCtx->config->fTDLSPuapsdPTIWindow;
			tInfo->peer_traffic_response_timeout =
				pHddCtx->config->fTDLSPuapsdPTRTimeout;
			tInfo->puapsd_mask = pHddCtx->config->fTDLSUapsdMask;
			tInfo->puapsd_inactivity_time =
				pHddCtx->config->fTDLSPuapsdInactivityTimer;
			tInfo->puapsd_rx_frame_threshold =
				pHddCtx->config->fTDLSRxFrameThreshold;
			tInfo->teardown_notification_ms =
				pHddCtx->config->tdls_idle_timeout;
			tInfo->tdls_peer_kickout_threshold =
				pHddCtx->config->tdls_peer_kickout_threshold;
			dump_tdls_state_param_setting(tInfo);

			qdf_ret_status =
				sme_update_fw_tdls_state(pHddCtx->hHal, tInfo, false);
			if (QDF_STATUS_SUCCESS != qdf_ret_status) {
				qdf_mem_free(tInfo);
			}
		} else {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: qdf_mem_malloc failed for tInfo", __func__);
		}
	}

	pHddTdlsCtx->magic = 0;
	pHddTdlsCtx->pAdapter = NULL;

	qdf_mem_free(pHddTdlsCtx);
	pAdapter->sessionCtx.station.pHddTdlsCtx = NULL;
	pHddTdlsCtx = NULL;

done:
	clear_bit(TDLS_INIT_DONE, &pAdapter->event_flags);
}

/**
 * wlan_hdd_tdls_monitor_timers_destroy() - destroy all tdls monitoring timers
 * @pHddTdlsCtx: TDLS context
 *
 * Return: Void
 */
static void wlan_hdd_tdls_monitor_timers_destroy(tdlsCtx_t *pHddTdlsCtx)
{
	qdf_mc_timer_stop(&pHddTdlsCtx->peerDiscoveryTimeoutTimer);
	qdf_mc_timer_destroy(&pHddTdlsCtx->peerDiscoveryTimeoutTimer);
}

/**
 * wlan_hdd_tdls_timers_destroy() - Destroy all the tdls timers running
 * @pHddTdlsCtx: TDLS Context
 *
 * Return: Void
 */
static void wlan_hdd_tdls_timers_destroy(tdlsCtx_t *pHddTdlsCtx)
{
	wlan_hdd_tdls_monitor_timers_destroy(pHddTdlsCtx);
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
hddTdlsPeer_t *wlan_hdd_tdls_get_peer(hdd_adapter_t *pAdapter, const u8 *mac)
{
	struct list_head *head;
	hddTdlsPeer_t *peer;
	u8 key;
	tdlsCtx_t *pHddTdlsCtx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return NULL;

	/* if already there, just update */
	peer = wlan_hdd_tdls_find_peer(pAdapter, mac, true);
	if (peer != NULL) {
		return peer;
	}

	/* not found, allocate and add the list */
	peer = qdf_mem_malloc(sizeof(hddTdlsPeer_t));
	if (NULL == peer) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s peer malloc failed!",
		       __func__);
		return NULL;
	}

	mutex_lock(&pHddCtx->tdls_lock);

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);

	if (NULL == pHddTdlsCtx) {
		qdf_mem_free(peer);
		mutex_unlock(&pHddCtx->tdls_lock);
		hddLog(LOG1, FL("pHddTdlsCtx is NULL"));
		return NULL;
	}

	key = wlan_hdd_tdls_hash_key(mac);
	head = &pHddTdlsCtx->peer_list[key];

	qdf_mem_zero(peer, sizeof(hddTdlsPeer_t));
	qdf_mem_copy(peer->peerMac, mac, sizeof(peer->peerMac));
	peer->pHddTdlsCtx = pHddTdlsCtx;
	peer->pref_off_chan_num = pHddCtx->config->fTDLSPrefOffChanNum;
	peer->op_class_for_pref_off_chan =
		wlan_hdd_find_opclass(pHddCtx->hHal, peer->pref_off_chan_num,
				pHddCtx->config->fTDLSPrefOffChanBandwidth);

	list_add_tail(&peer->node, head);
	mutex_unlock(&pHddCtx->tdls_lock);

	return peer;
}

/**
 * wlan_hdd_tdls_set_cap() - set TDLS capability type
 * @pAdapter: HDD adapter
 * @mac: peer mac address
 * @cap: TDLS capability type
 *
 * Return: 0 if successful or negative errno otherwise
 */
int wlan_hdd_tdls_set_cap(hdd_adapter_t *pAdapter, const uint8_t *mac,
			  tTDLSCapType cap)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->tdls_support = cap;

	return 0;
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
					tTDLSLinkStatus status,
					tTDLSLinkReason reason)
{
	uint32_t state = 0;
	int32_t res = 0;
	hdd_context_t *pHddCtx;
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
	pHddCtx = WLAN_HDD_GET_CTX(curr_peer->pHddTdlsCtx->pAdapter);
	if ((wlan_hdd_validate_context(pHddCtx)))
		return;
	hddLog(QDF_TRACE_LEVEL_WARN,
	       "tdls set peer " MAC_ADDRESS_STR " link status to %u",
	       MAC_ADDR_ARRAY(curr_peer->peerMac), status);

	mutex_lock(&pHddCtx->tdls_lock);
	curr_peer->link_status = status;

	/* If TDLS link status is already passed the discovery state
	 * then clear discovery attempt count
	 */
	if (status >= eTDLS_LINK_DISCOVERED) {
		curr_peer->discovery_attempt = 0;
	}

	mutex_unlock(&pHddCtx->tdls_lock);
	if (curr_peer->isForcedPeer && curr_peer->state_change_notification) {
		uint32_t opclass;
		uint32_t channel;

		hdd_adapter_t *adapter = curr_peer->pHddTdlsCtx->pAdapter;
		curr_peer->reason = reason;

		wlan_hdd_tdls_determine_channel_opclass(pHddCtx, adapter,
					curr_peer, &channel, &opclass);

		wlan_hdd_tdls_get_wifi_hal_state(curr_peer, &state, &res);
		(*curr_peer->state_change_notification)(curr_peer->peerMac,
							opclass, channel,
							state, res, adapter);
	}
	return;
}

/**
 * wlan_hdd_tdls_set_link_status() - set TDLS peer link status
 * @pAdapter: HDD adapter
 * @mac: mac address of TDLS peer
 * @linkStatus: status
 * @reason: reason
 *
 * Return: Void
 */
void wlan_hdd_tdls_set_link_status(hdd_adapter_t *pAdapter,
				   const uint8_t *mac,
				   tTDLSLinkStatus linkStatus,
				   tTDLSLinkReason reason)
{
	uint32_t state = 0;
	int32_t res = 0;
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (wlan_hdd_validate_context(pHddCtx))
		return;

	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac, true);
	if (curr_peer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("curr_peer is NULL"));
		return;
	}

	mutex_lock(&pHddCtx->tdls_lock);
	curr_peer->link_status = linkStatus;

	/* If TDLS link status is already passed the discovery state
	 * then clear discovery attempt count
	 */
	if (linkStatus >= eTDLS_LINK_DISCOVERED) {
		curr_peer->discovery_attempt = 0;
	}
	mutex_unlock(&pHddCtx->tdls_lock);
	if (curr_peer->isForcedPeer && curr_peer->state_change_notification) {
		uint32_t opclass;
		uint32_t channel;
		hdd_adapter_t *adapter = curr_peer->pHddTdlsCtx->pAdapter;

		curr_peer->reason = reason;

		wlan_hdd_tdls_determine_channel_opclass(pHddCtx, adapter,
					curr_peer, &channel, &opclass);

		wlan_hdd_tdls_get_wifi_hal_state(curr_peer, &state, &res);
		(curr_peer->state_change_notification)(mac, opclass, channel,
						       state, res, adapter);
	}

	return;
}

/**
 * wlan_hdd_tdls_recv_discovery_resp() - handling of tdls discovery response
 * @pAdapter: HDD adapter
 * @mac: mac address of peer from which the response was received
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_recv_discovery_resp(hdd_adapter_t *pAdapter,
				      const uint8_t *mac)
{
	hddTdlsPeer_t *curr_peer;
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx;

	ENTER();

	if (NULL == pHddTdlsCtx) {
		hddLog(LOGE, FL("pHddTdlsCtx is NULL"));
		return -EINVAL;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pHddTdlsCtx->pAdapter);

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return -EINVAL;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (NULL == curr_peer) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	if (pHddTdlsCtx->discovery_sent_cnt)
		pHddTdlsCtx->discovery_sent_cnt--;

	mutex_lock(&pHddCtx->tdls_lock);
	wlan_hdd_tdls_check_power_save_prohibited(pAdapter);

	mutex_unlock(&pHddCtx->tdls_lock);
	if (0 == pHddTdlsCtx->discovery_sent_cnt) {
		qdf_mc_timer_stop(&pHddTdlsCtx->peerDiscoveryTimeoutTimer);
	}

	hddLog(LOG1,
		  "Discovery(%u) Response from " MAC_ADDRESS_STR
		  " link_status %d", pHddTdlsCtx->discovery_sent_cnt,
		  MAC_ADDR_ARRAY(curr_peer->peerMac), curr_peer->link_status);

	if (eTDLS_LINK_DISCOVERING == curr_peer->link_status) {
		/* Since we are here, it means Throughput threshold is alredy met. Make sure RSSI
		   threshold is also met before setting up TDLS link */
		if ((int32_t) curr_peer->rssi >
		    (int32_t) pHddTdlsCtx->threshold_config.
		    rssi_trigger_threshold) {
			wlan_hdd_tdls_set_peer_link_status(curr_peer,
							   eTDLS_LINK_DISCOVERED,
							   eTDLS_LINK_SUCCESS);
			hddLog(LOG1,
				  "Rssi Threshold met: " MAC_ADDRESS_STR
				  " rssi = %d threshold= %d",
				  MAC_ADDR_ARRAY(curr_peer->peerMac),
				  curr_peer->rssi,
				  pHddTdlsCtx->threshold_config.
				  rssi_trigger_threshold);
			cfg80211_tdls_oper_request(pAdapter->dev,
						   curr_peer->peerMac,
						   NL80211_TDLS_SETUP, false,
						   GFP_KERNEL);
		} else {
			hddLog(LOG1,
				  "Rssi Threshold not met: " MAC_ADDRESS_STR
				  " rssi = %d threshold = %d ",
				  MAC_ADDR_ARRAY(curr_peer->peerMac),
				  curr_peer->rssi,
				  pHddTdlsCtx->threshold_config.
				  rssi_trigger_threshold);
			wlan_hdd_tdls_set_peer_link_status(curr_peer,
							   eTDLS_LINK_IDLE,
							   eTDLS_LINK_UNSPECIFIED);

			/* if RSSI threshold is not met then allow further discovery
			 * attempts by decrementing count for the last attempt
			 */
			if (curr_peer->discovery_attempt)
				curr_peer->discovery_attempt--;
		}
	}

	curr_peer->tdls_support = eTDLS_CAP_SUPPORTED;
	EXIT();
	return 0;
}

/**
 * wlan_hdd_tdls_set_peer_caps() - set TDLS peer capability
 * @pAdapter: HDD adapter
 * @mac: MAC address of the TDLS peer
 * @StaParams: CSR Station Parameter
 * @isBufSta: is peer buffer station
 * @isOffChannelSupported: Is off channel supported
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_set_peer_caps(hdd_adapter_t *pAdapter,
				const uint8_t *mac,
				tCsrStaParams *StaParams,
				bool isBufSta, bool isOffChannelSupported)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->uapsdQueues = StaParams->uapsd_queues;
	curr_peer->maxSp = StaParams->max_sp;
	curr_peer->isBufSta = isBufSta;
	curr_peer->isOffChannelSupported = isOffChannelSupported;

	qdf_mem_copy(curr_peer->supported_channels,
		     StaParams->supported_channels,
		     StaParams->supported_channels_len);

	curr_peer->supported_channels_len = StaParams->supported_channels_len;

	qdf_mem_copy(curr_peer->supported_oper_classes,
		     StaParams->supported_oper_classes,
		     StaParams->supported_oper_classes_len);

	curr_peer->supported_oper_classes_len =
		StaParams->supported_oper_classes_len;
	return 0;
}

/**
 * wlan_hdd_tdls_get_link_establish_params() - get TDLS link establish
 *                                             parameter
 * @pAdapter: HDD adapter
 * @mac: mac address
 * @tdlsLinkEstablishParams: output parameter to store the result
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_get_link_establish_params(hdd_adapter_t *pAdapter,
					    const u8 *mac,
					    tCsrTdlsLinkEstablishParams *
					    tdlsLinkEstablishParams)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	tdlsLinkEstablishParams->isResponder = curr_peer->is_responder;
	tdlsLinkEstablishParams->uapsdQueues = curr_peer->uapsdQueues;
	tdlsLinkEstablishParams->maxSp = curr_peer->maxSp;
	tdlsLinkEstablishParams->isBufSta = curr_peer->isBufSta;
	tdlsLinkEstablishParams->isOffChannelSupported =
		curr_peer->isOffChannelSupported;

	qdf_mem_copy(tdlsLinkEstablishParams->supportedChannels,
		     curr_peer->supported_channels,
		     curr_peer->supported_channels_len);

	tdlsLinkEstablishParams->supportedChannelsLen =
		curr_peer->supported_channels_len;

	qdf_mem_copy(tdlsLinkEstablishParams->supportedOperClasses,
		     curr_peer->supported_oper_classes,
		     curr_peer->supported_oper_classes_len);

	tdlsLinkEstablishParams->supportedOperClassesLen =
		curr_peer->supported_oper_classes_len;
	return 0;
}

/**
 * wlan_hdd_tdls_set_rssi() - Set TDLS RSSI on peer given by mac
 * @pAdapter: HDD adapter
 * @mac: MAC address of Peer
 * @rxRssi: rssi value
 *
 * Set RSSI on TDSL peer
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int wlan_hdd_tdls_set_rssi(hdd_adapter_t *pAdapter, const uint8_t *mac,
			   int8_t rxRssi)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac, true);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->rssi = rxRssi;

	return 0;
}

/**
 * wlan_hdd_tdls_set_responder() - Set/clear TDLS peer's responder role
 * @pAdapter: HDD adapter
 * @mac: MAC address of Peer
 * @responder: flag that indicates if the TDLS peer should be responder or not
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int wlan_hdd_tdls_set_responder(hdd_adapter_t *pAdapter, const uint8_t *mac,
				uint8_t responder)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->is_responder = responder;

	return 0;
}

/**
 * wlan_hdd_tdls_set_signature() - Set TDLS peer's signature
 * @pAdapter: HDD adapter
 * @mac: MAC address of TDLS Peer
 * @uSignature: signature value
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int wlan_hdd_tdls_set_signature(hdd_adapter_t *pAdapter, const uint8_t *mac,
				uint8_t uSignature)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->signature = uSignature;

	return 0;
}

/**
 * wlan_hdd_tdls_extract_da() - Extract destination address from socket buffer
 * @skb: socket buffer
 * @mac: output mac address buffer to store the destination address
 *
 * Return: Void
 */
void wlan_hdd_tdls_extract_da(struct sk_buff *skb, uint8_t *mac)
{
	memcpy(mac, skb->data, 6);
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
 * wlan_hdd_tdls_increment_pkt_count() - update statistics counter on tdls peer
 * @pAdapter: HDD adapter
 * @mac: MAC address of the TDLS peer
 * @tx: If 1, increment tx packet counter, if 0, increment rx packet counter
 *
 * Return: 0 for success or negative errno otherwise
 */
int wlan_hdd_tdls_increment_pkt_count(hdd_adapter_t *pAdapter,
				      const uint8_t *mac, uint8_t tx)
{
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (eTDLS_SUPPORT_ENABLED != pHddCtx->tdls_mode &&
	    eTDLS_SUPPORT_EXTERNAL_CONTROL != pHddCtx->tdls_mode)
		return -EINVAL;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOG1, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	if (tx)
		curr_peer->tx_pkt++;
	else
		curr_peer->rx_pkt++;

	return 0;
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
		hddLog(LOGE,
		       FL("Invalid 1st argument %d. <0...2>"),
		       config->tdls);
		return -EINVAL;
	}
	if (config->tx_period_t < CFG_TDLS_TX_STATS_PERIOD_MIN ||
	    config->tx_period_t > CFG_TDLS_TX_STATS_PERIOD_MAX) {
		hddLog(LOGE,
		       FL("Invalid 2nd argument %d. <%d...%ld>"),
		       config->tx_period_t, CFG_TDLS_TX_STATS_PERIOD_MIN,
		       CFG_TDLS_TX_STATS_PERIOD_MAX);
		return -EINVAL;
	}
	if (config->tx_packet_n < CFG_TDLS_TX_PACKET_THRESHOLD_MIN ||
	    config->tx_packet_n > CFG_TDLS_TX_PACKET_THRESHOLD_MAX) {
		hddLog(LOGE,
		       FL("Invalid 3rd argument %d. <%d...%ld>"),
		       config->tx_packet_n, CFG_TDLS_TX_PACKET_THRESHOLD_MIN,
		       CFG_TDLS_TX_PACKET_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->discovery_tries_n < CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN ||
	    config->discovery_tries_n > CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX) {
		hddLog(LOGE,
		       FL("Invalid 5th argument %d. <%d...%d>"),
		       config->discovery_tries_n,
		       CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MIN,
		       CFG_TDLS_MAX_DISCOVERY_ATTEMPT_MAX);
		return -EINVAL;
	}
	if (config->idle_packet_n < CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN ||
	    config->idle_packet_n > CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX) {
		hddLog(LOGE,
		       FL("Invalid 7th argument %d. <%d...%d>"),
		       config->idle_packet_n,
		       CFG_TDLS_IDLE_PACKET_THRESHOLD_MIN,
		       CFG_TDLS_IDLE_PACKET_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->rssi_trigger_threshold < CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN
	    || config->rssi_trigger_threshold >
	    CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX) {
		hddLog(LOGE,
		       FL("Invalid 9th argument %d. <%d...%d>"),
		       config->rssi_trigger_threshold,
		       CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MIN,
		       CFG_TDLS_RSSI_TRIGGER_THRESHOLD_MAX);
		return -EINVAL;
	}
	if (config->rssi_teardown_threshold <
	    CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN
	    || config->rssi_teardown_threshold >
	    CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX) {
		hddLog(LOGE,
		       FL("Invalid 10th argument %d. <%d...%d>"),
		       config->rssi_teardown_threshold,
		       CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MIN,
		       CFG_TDLS_RSSI_TEARDOWN_THRESHOLD_MAX);
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_tdd_tdls_reset_tx_rx() - reset tx/rx counters for all tdls peers
 * @pHddTdlsCtx: TDLS context
 *
 * Caller has to take the TDLS lock before calling this function
 *
 * Return: Void
 */
static void wlan_tdd_tdls_reset_tx_rx(tdlsCtx_t *pHddTdlsCtx)
{
	int i;
	struct list_head *head;
	hddTdlsPeer_t *tmp;
	struct list_head *pos, *q;

	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];
		list_for_each_safe(pos, q, head) {
			tmp = list_entry(pos, hddTdlsPeer_t, node);
			tmp->tx_pkt = 0;
			tmp->rx_pkt = 0;
		}
	}

	return;
}

/**
 * wlan_hdd_tdls_implicit_disable() - disable implicit tdls triggering
 * @pHddTdlsCtx: TDLS context
 *
 * Return: Void
 */
static void wlan_hdd_tdls_implicit_disable(tdlsCtx_t *pHddTdlsCtx)
{
	wlan_hdd_tdls_timers_stop(pHddTdlsCtx);
}

/**
 * wlan_hdd_tdls_implicit_enable() - enable implicit tdls triggering
 * @pHddTdlsCtx: TDLS context
 *
 * Return: Void
 */
static void wlan_hdd_tdls_implicit_enable(tdlsCtx_t *pHddTdlsCtx)
{
	wlan_hdd_tdls_peer_reset_discovery_processed(pHddTdlsCtx);
	pHddTdlsCtx->discovery_sent_cnt = 0;
	wlan_tdd_tdls_reset_tx_rx(pHddTdlsCtx);
	wlan_hdd_tdls_check_power_save_prohibited(pHddTdlsCtx->pAdapter);

}

/**
 * wlan_hdd_tdls_set_mode() - set TDLS mode
 * @pHddCtx: HDD context
 * @tdls_mode: TDLS mode
 * @bUpdateLast: Switch on if to set pHddCtx->tdls_mode_last to tdls_mode.
 *               If 1, set pHddCtx->tdls_mode_last to tdls_mode, otherwise
 *               set pHddCtx->tdls_mode_last to pHddCtx->tdls_mode
 *
 * Return: Void
 */
void wlan_hdd_tdls_set_mode(hdd_context_t *pHddCtx,
			    eTDLSSupportMode tdls_mode, bool bUpdateLast)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *pAdapter;
	tdlsCtx_t *pHddTdlsCtx;

	ENTER();

	hddLog(LOG1, "%s mode %d", __func__, (int)tdls_mode);

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	mutex_lock(&pHddCtx->tdls_lock);

	if (pHddCtx->tdls_mode == tdls_mode) {
		mutex_unlock(&pHddCtx->tdls_lock);
		hddLog(LOG1, FL("already in mode %d"), (int)tdls_mode);
		return;
	}

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);

	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			if (eTDLS_SUPPORT_ENABLED == tdls_mode ||
			    eTDLS_SUPPORT_EXTERNAL_CONTROL == tdls_mode)
				wlan_hdd_tdls_implicit_enable(pHddTdlsCtx);
			else if ((eTDLS_SUPPORT_DISABLED == tdls_mode) ||
				 (eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == tdls_mode))
				wlan_hdd_tdls_implicit_disable(pHddTdlsCtx);
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	if (bUpdateLast) {
		pHddCtx->tdls_mode_last = tdls_mode;
	} else {
		pHddCtx->tdls_mode_last = pHddCtx->tdls_mode;
	}
	pHddCtx->tdls_mode = tdls_mode;

	mutex_unlock(&pHddCtx->tdls_lock);
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	eTDLSSupportMode req_tdls_mode;
	tdlsInfo_t *tdlsParams;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (NULL == pHddTdlsCtx) {
		hddLog(LOGE, FL("TDLS not enabled!"));
		return -EINVAL;
	}

	if (wlan_hdd_tdls_check_config(config) != 0) {
		return -EINVAL;
	}

	/* config->tdls is mapped to 0->1, 1->2, 2->3 */
	req_tdls_mode = config->tdls + 1;
	if (pHddCtx->tdls_mode == req_tdls_mode) {
		hddLog(LOGE, FL("Already in mode %d"), config->tdls);
		return -EINVAL;
	}

	/* copy the configuration only when given tdls mode is implicit trigger enable */
	if (eTDLS_SUPPORT_ENABLED == req_tdls_mode ||
	    eTDLS_SUPPORT_EXTERNAL_CONTROL == req_tdls_mode) {
		memcpy(&pHddTdlsCtx->threshold_config, config,
		       sizeof(tdls_config_params_t));
	}

	hddLog(LOGE,
		FL("iw set tdls params: %d %d %d %d %d %d %d"),
		config->tdls,
		config->tx_period_t,
		config->tx_packet_n,
		config->discovery_tries_n,
		config->idle_packet_n,
		config->rssi_trigger_threshold,
		config->rssi_teardown_threshold);

	wlan_hdd_tdls_set_mode(pHddCtx, req_tdls_mode, true);

	tdlsParams = qdf_mem_malloc(sizeof(tdlsInfo_t));
	if (NULL == tdlsParams) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: qdf_mem_malloc failed for tdlsParams", __func__);
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
	if (pHddCtx->config->fEnableTDLSOffChannel)
		tdlsParams->tdls_options |= ENA_TDLS_OFFCHAN;
	if (pHddCtx->config->fEnableTDLSBufferSta)
		tdlsParams->tdls_options |= ENA_TDLS_BUFFER_STA;
	if (pHddCtx->config->fEnableTDLSSleepSta)
		tdlsParams->tdls_options |= ENA_TDLS_SLEEP_STA;
	tdlsParams->peer_traffic_ind_window =
		pHddCtx->config->fTDLSPuapsdPTIWindow;
	tdlsParams->peer_traffic_response_timeout =
		pHddCtx->config->fTDLSPuapsdPTRTimeout;
	tdlsParams->puapsd_mask = pHddCtx->config->fTDLSUapsdMask;
	tdlsParams->puapsd_inactivity_time =
		pHddCtx->config->fTDLSPuapsdInactivityTimer;
	tdlsParams->puapsd_rx_frame_threshold =
		pHddCtx->config->fTDLSRxFrameThreshold;
	tdlsParams->teardown_notification_ms =
		pHddCtx->config->tdls_idle_timeout;
	tdlsParams->tdls_peer_kickout_threshold =
		pHddCtx->config->tdls_peer_kickout_threshold;

	dump_tdls_state_param_setting(tdlsParams);

	qdf_ret_status = sme_update_fw_tdls_state(pHddCtx->hHal, tdlsParams, true);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		qdf_mem_free(tdlsParams);
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_update_tdls_info - update tdls status info
 * @adapter: ptr to device adapter.
 * @tdls_prohibited: indicates whether tdls is prohibited.
 * @tdls_chan_swit_prohibited: indicates whether tdls channel switch
 *                             is prohibited.
 *
 * Normally an AP does not influence TDLS connection between STAs
 * associated to it. But AP may set bits for TDLS Prohibited or
 * TDLS Channel Switch Prohibited in Extended Capability IE in
 * Assoc/Re-assoc response to STA. So after STA is connected to
 * an AP, call this function to update TDLS status as per those
 * bits set in Ext Cap IE in received Assoc/Re-assoc response
 * from AP.
 *
 * Return: None.
 */
void wlan_hdd_update_tdls_info(hdd_adapter_t *adapter, bool tdls_prohibited,
				bool tdls_chan_swit_prohibited)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tdlsCtx_t *hdd_tdls_ctx = WLAN_HDD_GET_TDLS_CTX_PTR(adapter);
	tdlsInfo_t *tdls_param;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (!hdd_tdls_ctx) {
		/* may be TDLS is not applicable for this adapter */
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD TDLS context is null"));
		return;
	}

	/* If TDLS support is disabled then no need to update target */
	if (false == hdd_ctx->config->fEnableTDLSSupport) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("TDLS not enabled"));
		return;
	}

	/* If AP indicated TDLS Prohibited then disable tdls mode */
	mutex_lock(&hdd_ctx->tdls_lock);
	if (tdls_prohibited) {
		hdd_ctx->tdls_mode = eTDLS_SUPPORT_NOT_ENABLED;
	} else {
		if (false == hdd_ctx->config->fEnableTDLSImplicitTrigger)
			hdd_ctx->tdls_mode = eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY;
		else if (true == hdd_ctx->config->fTDLSExternalControl)
			hdd_ctx->tdls_mode = eTDLS_SUPPORT_EXTERNAL_CONTROL;
		else
			hdd_ctx->tdls_mode = eTDLS_SUPPORT_ENABLED;
	}
	mutex_unlock(&hdd_ctx->tdls_lock);
	tdls_param = qdf_mem_malloc(sizeof(*tdls_param));
	if (!tdls_param) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
			FL("memory allocation failed for tdlsParams"));
		return;
	}

	tdls_param->vdev_id = adapter->sessionId;
	tdls_param->tdls_state = hdd_ctx->tdls_mode;
	tdls_param->notification_interval_ms =
	hdd_tdls_ctx->threshold_config.tx_period_t;
	tdls_param->tx_discovery_threshold =
	hdd_tdls_ctx->threshold_config.tx_packet_n;
	tdls_param->tx_teardown_threshold =
	hdd_tdls_ctx->threshold_config.idle_packet_n;
	tdls_param->rssi_teardown_threshold =
	hdd_tdls_ctx->threshold_config.rssi_teardown_threshold;
	tdls_param->rssi_delta = hdd_tdls_ctx->threshold_config.rssi_delta;

	tdls_param->tdls_options = 0;

	/* Do not enable TDLS offchannel, if AP prohibited TDLS channel switch */
	if ((hdd_ctx->config->fEnableTDLSOffChannel) &&
	    (!tdls_chan_swit_prohibited)) {
		tdls_param->tdls_options |= ENA_TDLS_OFFCHAN;
	}

	if (hdd_ctx->config->fEnableTDLSBufferSta)
		tdls_param->tdls_options |= ENA_TDLS_BUFFER_STA;

	if (hdd_ctx->config->fEnableTDLSSleepSta)
		tdls_param->tdls_options |= ENA_TDLS_SLEEP_STA;

	tdls_param->peer_traffic_ind_window =
		hdd_ctx->config->fTDLSPuapsdPTIWindow;
	tdls_param->peer_traffic_response_timeout =
		hdd_ctx->config->fTDLSPuapsdPTRTimeout;
	tdls_param->puapsd_mask =
		hdd_ctx->config->fTDLSUapsdMask;
	tdls_param->puapsd_inactivity_time =
		hdd_ctx->config->fTDLSPuapsdInactivityTimer;
	tdls_param->puapsd_rx_frame_threshold =
		hdd_ctx->config->fTDLSRxFrameThreshold;
	tdls_param->teardown_notification_ms =
		hdd_ctx->config->tdls_idle_timeout;
	tdls_param->tdls_peer_kickout_threshold =
		hdd_ctx->config->tdls_peer_kickout_threshold;

	dump_tdls_state_param_setting(tdls_param);

	qdf_ret_status = sme_update_fw_tdls_state(hdd_ctx->hHal,
					       tdls_param,
					       true);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		qdf_mem_free(tdls_param);
		return;
	}
	return;
}

/**
 * wlan_hdd_tdls_set_sta_id() - set station ID on a tdls peer
 * @pAdapter: HDD adapter
 * @mac: MAC address of a tdls peer
 * @staId: station ID
 *
 * Return: 0 if success; negative errno otherwise
 */
int wlan_hdd_tdls_set_sta_id(hdd_adapter_t *pAdapter, const uint8_t *mac,
			     uint8_t staId)
{
	hddTdlsPeer_t *curr_peer;

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	curr_peer->staId = staId;

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
int wlan_hdd_tdls_set_extctrl_param(hdd_adapter_t *pAdapter, const uint8_t *mac,
				    uint32_t chan, uint32_t max_latency,
				    uint32_t op_class, uint32_t min_bandwidth)
{
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	if (!pHddCtx)
		return -EINVAL;
	mutex_lock(&pHddCtx->tdls_lock);
	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac, false);
	if (curr_peer == NULL) {
		mutex_unlock(&pHddCtx->tdls_lock);
		return -EINVAL;
	}
	curr_peer->op_class_for_pref_off_chan = (uint8_t) op_class;
	curr_peer->pref_off_chan_num = (uint8_t) chan;

	mutex_unlock(&pHddCtx->tdls_lock);
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
int wlan_hdd_tdls_update_peer_mac(hdd_adapter_t *adapter, const uint8_t *mac,
				  uint32_t peer_state)
{
	tSmeTdlsPeerStateParams sme_tdls_peer_state_params = {0};
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	sme_tdls_peer_state_params.vdevId = adapter->sessionId;
	qdf_mem_copy(&sme_tdls_peer_state_params.peerMacAddr, mac,
		     sizeof(sme_tdls_peer_state_params.peerMacAddr));
	sme_tdls_peer_state_params.peerState = peer_state;
	status = sme_update_tdls_peer_state(hdd_ctx->hHal,
					    &sme_tdls_peer_state_params);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGE, FL("sme_UpdateTdlsPeerState failed for "MAC_ADDRESS_STR),
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
int wlan_hdd_tdls_set_force_peer(hdd_adapter_t *pAdapter, const uint8_t *mac,
				 bool forcePeer)
{
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (!pHddCtx)
		return -EINVAL;

	mutex_lock(&pHddCtx->tdls_lock);

	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac, false);
	if (curr_peer == NULL) {
		mutex_unlock(&pHddCtx->tdls_lock);
		return -EINVAL;
	}

	curr_peer->isForcedPeer = forcePeer;
	mutex_unlock(&pHddCtx->tdls_lock);
	return 0;
}

/**
 * wlan_hdd_tdls_find_peer() - find TDLS peer given its MAC address
 * @pAdapter: HDD adapter
 * @mac: MAC address of peer
 * @mutexLock: Option to indicate if mutex locking is required for searching
 *
 * Return: If peerMac is found, then it returns pointer to hddTdlsPeer_t;
 *         otherwise, it returns NULL
 */
hddTdlsPeer_t *wlan_hdd_tdls_find_peer(hdd_adapter_t *pAdapter,
				       const uint8_t *mac, bool mutexLock)
{
	uint8_t key;
	struct list_head *pos;
	struct list_head *head;
	hddTdlsPeer_t *curr_peer;
	tdlsCtx_t *pHddTdlsCtx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return NULL;

	if (mutexLock) {
		mutex_lock(&pHddCtx->tdls_lock);
	}
	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		if (mutexLock)
			mutex_unlock(&pHddCtx->tdls_lock);
		return NULL;
	}

	key = wlan_hdd_tdls_hash_key(mac);

	head = &pHddTdlsCtx->peer_list[key];

	list_for_each(pos, head) {
		curr_peer = list_entry(pos, hddTdlsPeer_t, node);
		if (!memcmp(mac, curr_peer->peerMac, 6)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
				  "findTdlsPeer: found staId %d",
				  curr_peer->staId);
			if (mutexLock)
				mutex_unlock(&pHddCtx->tdls_lock);
			return curr_peer;
		}
	}
	if (mutexLock)
		mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
	return NULL;
}

/**
 * wlan_hdd_tdls_find_all_peer() - find all peers matching the input MAC
 * @pHddCtx: HDD context
 * @mac: MAC address
 *
 * Return: TDLS peer if a matching is detected; NULL otherwise
 */
hddTdlsPeer_t *wlan_hdd_tdls_find_all_peer(hdd_context_t *pHddCtx,
					   const u8 *mac)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *pAdapter = NULL;
	tdlsCtx_t *pHddTdlsCtx = NULL;
	hddTdlsPeer_t *curr_peer = NULL;
	QDF_STATUS status = 0;

	mutex_lock(&pHddCtx->tdls_lock);

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			curr_peer =
				wlan_hdd_tdls_find_peer(pAdapter, mac, false);
			if (curr_peer) {
				mutex_unlock(&pHddCtx->tdls_lock);
				return curr_peer;
			}
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	mutex_unlock(&pHddCtx->tdls_lock);
	return curr_peer;
}

/**
 * wlan_hdd_tdls_reset_peer() - reset TDLS peer identified by MAC address
 * @pAdapter: HDD adapter
 * @mac: MAC address of the peer
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_hdd_tdls_reset_peer(hdd_adapter_t *pAdapter, const uint8_t *mac)
{
	hdd_context_t *pHddCtx;
	hddTdlsPeer_t *curr_peer;

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	curr_peer = wlan_hdd_tdls_get_peer(pAdapter, mac);
	if (curr_peer == NULL) {
		hddLog(LOGE, FL("curr_peer is NULL"));
		return -EINVAL;
	}

	/*
	 * Reset preferred offchannel and opclass for offchannel as
	 * per INI configuration only if peer is not forced one. For
	 * forced peer, offchannel and opclass is set in HAL API at the
	 * time of enabling TDLS for that specific peer and so do not overwrite
	 * those set by user space.
	 */
	if (false == curr_peer->isForcedPeer) {
		curr_peer->pref_off_chan_num =
			pHddCtx->config->fTDLSPrefOffChanNum;
		curr_peer->op_class_for_pref_off_chan =
		    wlan_hdd_find_opclass(WLAN_HDD_GET_HAL_CTX(pAdapter),
				curr_peer->pref_off_chan_num,
				pHddCtx->config->fTDLSPrefOffChanBandwidth);
	}

	wlan_hdd_tdls_set_peer_link_status(curr_peer,
					   eTDLS_LINK_IDLE,
					   eTDLS_LINK_UNSPECIFIED);
	curr_peer->staId = 0;

	return 0;
}

/**
 * wlan_hdd_tdls_peer_reset_discovery_processed() - reset discovery status
 * @pHddTdlsCtx: TDLS context
 *
 * This function resets discovery processing bit for all TDLS peers
 *
 * Caller has to take the lock before calling this function
 *
 * Return: 0
 */
static int32_t wlan_hdd_tdls_peer_reset_discovery_processed(tdlsCtx_t *
							    pHddTdlsCtx)
{
	int i;
	struct list_head *head;
	hddTdlsPeer_t *tmp;
	struct list_head *pos, *q;

	pHddTdlsCtx->discovery_peer_cnt = 0;

	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];
		list_for_each_safe(pos, q, head) {
			tmp = list_entry(pos, hddTdlsPeer_t, node);
			tmp->discovery_processed = 0;
		}
	}

	return 0;
}

/**
 * wlan_hdd_tdls_connected_peers() - Find the number of connected TDLS peers
 * @pAdapter: HDD adapter
 *
 * Return: The number of connected TDLS peers or 0 if error is detected
 */
uint16_t wlan_hdd_tdls_connected_peers(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (wlan_hdd_validate_context(pHddCtx))
		return 0;

	return pHddCtx->connected_peer_count;
}

/**
 * wlan_hdd_tdls_get_all_peers() - dump all TDLS peer info into output string
 * @pAdapter: HDD adapter
 * @buf: output string buffer to hold the peer info
 * @buflen: the size of output string buffer
 *
 * Return: The size (in bytes) of the valid peer info in the output buffer
 */
int wlan_hdd_tdls_get_all_peers(hdd_adapter_t *pAdapter, char *buf, int buflen)
{
	int i;
	int len, init_len;
	struct list_head *head;
	struct list_head *pos;
	hddTdlsPeer_t *curr_peer;
	tdlsCtx_t *pHddTdlsCtx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return 0;

	init_len = buflen;
	len = scnprintf(buf, buflen, "\n%-18s%-3s%-4s%-3s%-5s\n",
			"MAC", "Id", "cap", "up", "RSSI");
	buf += len;
	buflen -= len;
	/*                           1234567890123456789012345678901234567 */
	len = scnprintf(buf, buflen, "---------------------------------\n");
	buf += len;
	buflen -= len;

	mutex_lock(&pHddCtx->tdls_lock);

	pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	if (NULL == pHddTdlsCtx) {
		mutex_unlock(&pHddCtx->tdls_lock);
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
	mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
	return init_len - buflen;
}

/**
 * wlan_hdd_tdls_connection_callback() - callback after tdls connection
 * @pAdapter: HDD adapter
 *
 * Return: Void
 */
void wlan_hdd_tdls_connection_callback(hdd_adapter_t *pAdapter)
{
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if ((NULL == pHddCtx) || (NULL == pHddTdlsCtx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddCtx or pHddTdlsCtx points to NULL"));
		return;
	}

	mutex_lock(&pHddCtx->tdls_lock);

	hddLog(LOG1,
		FL("update %d"),
		pHddTdlsCtx->threshold_config.tx_period_t);

	if (eTDLS_SUPPORT_ENABLED == pHddCtx->tdls_mode ||
	    eTDLS_SUPPORT_EXTERNAL_CONTROL == pHddCtx->tdls_mode) {
		wlan_hdd_tdls_peer_reset_discovery_processed(pHddTdlsCtx);
		pHddTdlsCtx->discovery_sent_cnt = 0;
		wlan_hdd_tdls_check_power_save_prohibited(pHddTdlsCtx->pAdapter);
	}

	mutex_unlock(&pHddCtx->tdls_lock);

}

/**
 * wlan_hdd_tdls_disconnection_callback() - callback after tdls disconnection
 * @pAdapter: HDD adapter
 *
 * Return: Void
 */
void wlan_hdd_tdls_disconnection_callback(hdd_adapter_t *pAdapter)
{
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if ((NULL == pHddCtx) || (NULL == pHddTdlsCtx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddCtx or pHddTdlsCtx points to NULL"));
		return;
	}

	hddLog(LOG1, "%s", __func__);

	mutex_lock(&pHddCtx->tdls_lock);

	if (NULL == pHddTdlsCtx) {
		mutex_unlock(&pHddCtx->tdls_lock);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddTdlsCtx is NULL"));
		return;
	}
	pHddTdlsCtx->discovery_sent_cnt = 0;
	wlan_hdd_tdls_check_power_save_prohibited(pHddTdlsCtx->pAdapter);

	wlan_hdd_tdls_monitor_timers_stop(pHddTdlsCtx);
	wlan_hdd_tdls_free_list(pHddTdlsCtx);

	pHddTdlsCtx->curr_candidate = NULL;

	mutex_unlock(&pHddCtx->tdls_lock);
}

/**
 * wlan_hdd_tdls_mgmt_completion_callback() - callback for TDLS management
 *                                            TX completion
 * @pAdapter: HDD adapter
 * @statusCode: management TX completion status
 *
 * Return: Void
 */
void wlan_hdd_tdls_mgmt_completion_callback(hdd_adapter_t *pAdapter,
					    uint32_t statusCode)
{
	pAdapter->mgmtTxCompletionStatus = statusCode;
	hddLog(LOG1,
		  "%s: Mgmt TX Completion %d", __func__, statusCode);
	complete(&pAdapter->tdls_mgmt_comp);
}

/**
 * wlan_hdd_tdls_increment_peer_count() - increment connected TDLS peer counter
 * @pAdapter: HDD adapter
 *
 * Return: Void
 */
void wlan_hdd_tdls_increment_peer_count(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	mutex_lock(&pHddCtx->tdls_lock);

	pHddCtx->connected_peer_count++;
	wlan_hdd_tdls_check_power_save_prohibited(pAdapter);

	hddLog(LOG1, "%s: %d",
		  __func__, pHddCtx->connected_peer_count);

	mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
}

/**
 * wlan_hdd_tdls_decrement_peer_count() - decrement connected TDLS peer counter
 * @pAdapter: HDD adapter
 *
 * Return: Void
 */
void wlan_hdd_tdls_decrement_peer_count(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	mutex_lock(&pHddCtx->tdls_lock);

	if (pHddCtx->connected_peer_count)
		pHddCtx->connected_peer_count--;
	wlan_hdd_tdls_check_power_save_prohibited(pAdapter);

	hddLog(LOG1, "%s: %d",
		  __func__, pHddCtx->connected_peer_count);

	mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
}


/**
 * wlan_hdd_tdls_find_progress_peer() - find peer if TDLS is ongoing
 * @pAdapter: HDD adapter
 * @mac: If NULL check for all the peer list, otherwise, skip this mac when
 *       skip_self is true
 * @skip_self: If true, skip this mac. otherwise, check all the peer list. if
 *             mac is NULL, this argument is ignored, and check for all the peer
 *             list.
 *
 * Return: Pointer to hddTdlsPeer_t if TDLS is ongoing. Otherwise return NULL.
 */
static hddTdlsPeer_t *wlan_hdd_tdls_find_progress_peer(hdd_adapter_t *pAdapter,
						       const u8 *mac,
						       u8 skip_self)
{
	int i;
	struct list_head *head;
	hddTdlsPeer_t *curr_peer;
	struct list_head *pos;
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);;

	if (NULL == pHddTdlsCtx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddTdlsCtx is NULL"));
		return NULL;
	}

	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &pHddTdlsCtx->peer_list[i];
		list_for_each(pos, head) {
			curr_peer = list_entry(pos, hddTdlsPeer_t, node);
			if (skip_self && mac
			    && !memcmp(mac, curr_peer->peerMac, 6)) {
				continue;
			} else {
				if (eTDLS_LINK_CONNECTING ==
				    curr_peer->link_status) {
					hddLog(LOG1,
						  "%s:" MAC_ADDRESS_STR
						  " eTDLS_LINK_CONNECTING",
						  __func__,
						  MAC_ADDR_ARRAY(curr_peer->
								 peerMac));
					return curr_peer;
				}
			}
		}
	}
	return NULL;
}

/**
 * wlan_hdd_tdls_is_progress() - find the peer with ongoing TDLS progress
 * @pHddCtx: HDD context
 * @mac: mac address of the peer
 * @skip_self: if 1, skip checking self. If 0, search includes self
 *
 * Return: TDLS peer if found; NULL otherwise
 */
hddTdlsPeer_t *wlan_hdd_tdls_is_progress(hdd_context_t *pHddCtx,
					 const uint8_t *mac, uint8_t skip_self)
{
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *pAdapter = NULL;
	tdlsCtx_t *pHddTdlsCtx = NULL;
	hddTdlsPeer_t *curr_peer = NULL;
	QDF_STATUS status = 0;

	mutex_lock(&pHddCtx->tdls_lock);

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (NULL != pHddTdlsCtx) {
			curr_peer =
				wlan_hdd_tdls_find_progress_peer(pAdapter, mac,
								 skip_self);
			if (curr_peer) {
				mutex_unlock(&pHddCtx->tdls_lock);
				return curr_peer;
			}
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	mutex_unlock(&pHddCtx->tdls_lock);
	return NULL;
}

/**
 * __wlan_hdd_tdls_pre_setup() - TDLS Pre implicit setup
 * @work: work_struct used to find the TDLS context
 *
 * Return: Void
 */
static void __wlan_hdd_tdls_pre_setup(struct work_struct *work)
{
	tdlsCtx_t *pHddTdlsCtx = container_of(work, tdlsCtx_t, implicit_setup);
	hdd_context_t *pHddCtx;
	hddTdlsPeer_t *curr_peer;
	hddTdlsPeer_t *temp_peer;
	int status;
	tSirMacAddr peer_mac;

	ENTER();

	if (NULL == pHddTdlsCtx) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddTdlsCtx is NULL"));
		return;
	}

	if (unlikely(TDLS_CTX_MAGIC != pHddTdlsCtx->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: tdls magic number mis-match %u",
			  __func__, pHddTdlsCtx->magic);
		return;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pHddTdlsCtx->pAdapter);

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	mutex_lock(&pHddCtx->tdls_lock);

	curr_peer = pHddTdlsCtx->curr_candidate;

	if (NULL == curr_peer) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("pHddCtx is not valid"));
		goto done;
	}

	qdf_mem_copy(&peer_mac, curr_peer->peerMac, sizeof(peer_mac));

	mutex_unlock(&pHddCtx->tdls_lock);

	temp_peer = wlan_hdd_tdls_is_progress(pHddCtx, NULL, 0);

	if (NULL != temp_peer) {
		hddLog(LOG1,
			  "%s: " MAC_ADDRESS_STR " ongoing. pre_setup ignored",
			  __func__, MAC_ADDR_ARRAY(temp_peer->peerMac));
		goto done;
	}

	if (eTDLS_CAP_UNKNOWN != curr_peer->tdls_support)
		wlan_hdd_tdls_set_peer_link_status(curr_peer,
						   eTDLS_LINK_DISCOVERING,
						   eTDLS_LINK_SUCCESS);

	mutex_lock(&pHddCtx->tdls_lock);

	/* Ignore discovery attempt if External Control is enabled, that
	 * is, peer is forced. In that case, continue discovery attempt
	 * regardless attempt count
	 */
	if (false == curr_peer->isForcedPeer) {
		if (curr_peer->discovery_attempt >=
		    pHddTdlsCtx->threshold_config.discovery_tries_n) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: discovery attempt (%d) reached max (%d) for peer "
				  MAC_ADDRESS_STR
				  ", ignore discovery trigger from fw",
				  __func__, curr_peer->discovery_attempt,
				  pHddTdlsCtx->threshold_config.
				  discovery_tries_n,
				  MAC_ADDR_ARRAY(curr_peer->peerMac));
			curr_peer->tdls_support = eTDLS_CAP_NOT_SUPPORTED;
			/* Since TDLS discovery attempt reached the
			 * maximum threshold, so we remove the peer
			 * from the FW connection table.
			 */
			if (0 != wlan_hdd_tdls_update_peer_mac(pHddTdlsCtx->pAdapter,
				curr_peer->peerMac, eSME_TDLS_PEER_REMOVE_MAC_ADDR))
				hddLog(LOGE, FL("TDLS Peer mac update Failed "
					   MAC_ADDRESS_STR),
					   MAC_ADDR_ARRAY(curr_peer->peerMac));
			goto done;
		}
	}

	mutex_unlock(&pHddCtx->tdls_lock);
	wlan_hdd_tdls_set_peer_link_status(curr_peer,
					   eTDLS_LINK_DISCOVERING,
					   eTDLS_LINK_SUCCESS);

	status =
		wlan_hdd_cfg80211_send_tdls_discover_req(pHddTdlsCtx->pAdapter->
							 wdev.wiphy,
							 pHddTdlsCtx->pAdapter->dev,
							 peer_mac);

	mutex_lock(&pHddCtx->tdls_lock);

	if (NULL == pHddTdlsCtx->curr_candidate) {
		hddLog(LOGE,
			  "%s: current candidate Not valid any more", __func__);
		goto done;
	}

	curr_peer = pHddTdlsCtx->curr_candidate;

	if (0 != status) {
		hddLog(LOGE,
			  "%s: " MAC_ADDRESS_STR " discovery could not sent",
			  __func__, MAC_ADDR_ARRAY(curr_peer->peerMac));
		if (eTDLS_CAP_UNKNOWN != curr_peer->tdls_support) {
			mutex_unlock(&pHddCtx->tdls_lock);
			wlan_hdd_tdls_set_peer_link_status(curr_peer,
							   eTDLS_LINK_IDLE,
							   eTDLS_LINK_UNSPECIFIED);
			mutex_lock(&pHddCtx->tdls_lock);
		}
		goto done;
	}

	pHddTdlsCtx->discovery_sent_cnt++;

	curr_peer->discovery_attempt++;

	wlan_hdd_tdls_check_power_save_prohibited(pHddTdlsCtx->pAdapter);

	hddLog(LOG1,
		  "%s: discovery count %u timeout %u msec", __func__,
		  pHddTdlsCtx->discovery_sent_cnt,
		  pHddTdlsCtx->threshold_config.tx_period_t -
		  TDLS_DISCOVERY_TIMEOUT_BEFORE_UPDATE);

	wlan_hdd_tdls_timer_restart(pHddTdlsCtx->pAdapter,
				    &pHddTdlsCtx->peerDiscoveryTimeoutTimer,
				    pHddTdlsCtx->threshold_config.tx_period_t -
				    TDLS_DISCOVERY_TIMEOUT_BEFORE_UPDATE);

done:
	pHddTdlsCtx->curr_candidate = NULL;
	pHddTdlsCtx->magic = 0;
	mutex_unlock(&pHddCtx->tdls_lock);
	EXIT();
	return;
}

/**
 * wlan_hdd_tdls_pre_setup() - TDLS Pre implicit setup
 * @work: work_struct used to find the TDLS context
 *
 * Return: Void
 */
void wlan_hdd_tdls_pre_setup(struct work_struct *work)
{
	cds_ssr_protect(__func__);
	__wlan_hdd_tdls_pre_setup(work);
	cds_ssr_unprotect(__func__);
}

/**
 * wlan_hdd_tdls_copy_scan_context() - Copy TDLS scan context
 * @pHddCtx: HDD context
 * @wiphy: wiphy pointer
 * @dev: net device
 * request: source scan context
 *
 * Copy the source scan context into the HDD context's TDLS scan context
 *
 * Return: 0 for success; negative errno otherwise
 */
int wlan_hdd_tdls_copy_scan_context(hdd_context_t *pHddCtx,
				    struct wiphy *wiphy,
				    struct cfg80211_scan_request *request)
{
	tdls_scan_context_t *scan_ctx;

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return -EINVAL;

	scan_ctx = &pHddCtx->tdls_scan_ctxt;

	scan_ctx->wiphy = wiphy;

	scan_ctx->scan_request = request;
	EXIT();
	return 0;
}

/**
 * wlan_hdd_tdls_scan_init_work() - schedule tdls scan work
 * @pHddCtx: HDD context
 * @wiphy: wiphy pointer
 * @dev: net device
 * @request: scan request
 * @delay: delay value to pass to the work scheduling
 *
 * Return: Void
 */
static void wlan_hdd_tdls_scan_init_work(hdd_context_t *pHddCtx,
					 struct wiphy *wiphy,
					 struct cfg80211_scan_request *request,
					 unsigned long delay)
{
	if (TDLS_CTX_MAGIC != pHddCtx->tdls_scan_ctxt.magic) {
		wlan_hdd_tdls_copy_scan_context(pHddCtx, wiphy, request);
		pHddCtx->tdls_scan_ctxt.attempt = 0;
		pHddCtx->tdls_scan_ctxt.magic = TDLS_CTX_MAGIC;
	}
	schedule_delayed_work(&pHddCtx->tdls_scan_ctxt.tdls_scan_work, delay);
}

/**
 * wlan_hdd_tdls_scan_callback() - callback for TDLS scan operation
 * @pAdapter: HDD adapter
 * @wiphy: wiphy
 * @dev: net device
 * @request: scan request
 *
 * Return: negative = caller should stop and return error code immediately
 *         0 = caller should stop and return success immediately
 *         1 = caller can continue to scan
 */
int wlan_hdd_tdls_scan_callback(hdd_adapter_t *pAdapter, struct wiphy *wiphy,
				struct cfg80211_scan_request *request)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	u16 connectedTdlsPeers;
	hddTdlsPeer_t *curr_peer;
	unsigned long delay;
	int ret;

	ENTER();

	ret = wlan_hdd_validate_context(pHddCtx);
	if (ret)
		return ret;

	/* if tdls is not enabled, then continue scan */
	if (eTDLS_SUPPORT_NOT_ENABLED == pHddCtx->tdls_mode)
		return 1;

	curr_peer = wlan_hdd_tdls_is_progress(pHddCtx, NULL, 0);
	if (NULL != curr_peer) {
		if (pHddCtx->tdls_scan_ctxt.reject++ >= TDLS_MAX_SCAN_REJECT) {
			pHddCtx->tdls_scan_ctxt.reject = 0;
			hddLog(LOG1,
				  "%s: " MAC_ADDRESS_STR
				  ". scan rejected %d. force it to idle",
				  __func__, MAC_ADDR_ARRAY(curr_peer->peerMac),
				  pHddCtx->tdls_scan_ctxt.reject);

			wlan_hdd_tdls_set_peer_link_status(curr_peer,
							   eTDLS_LINK_IDLE,
							   eTDLS_LINK_UNSPECIFIED);
			return 1;
		}
		hddLog(LOGW,
			  "%s: tdls in progress. scan rejected %d",
			  __func__, pHddCtx->tdls_scan_ctxt.reject);
		return -EBUSY;
	}

	/* tdls teardown is ongoing */
	if (eTDLS_SUPPORT_DISABLED == pHddCtx->tdls_mode) {
		connectedTdlsPeers = wlan_hdd_tdls_connected_peers(pAdapter);
		if (connectedTdlsPeers
		    && (pHddCtx->tdls_scan_ctxt.attempt <
			TDLS_MAX_SCAN_SCHEDULE)) {
			delay =
				(unsigned long)(TDLS_DELAY_SCAN_PER_CONNECTION *
						connectedTdlsPeers);
			hddLog(LOG1,
				  "%s: tdls disabled, but still connected_peers %d attempt %d. schedule scan %lu msec",
				  __func__, connectedTdlsPeers,
				  pHddCtx->tdls_scan_ctxt.attempt, delay);

			wlan_hdd_tdls_scan_init_work(pHddCtx, wiphy,
						     request,
						     msecs_to_jiffies(delay));
			/* scan should not continue */
			return 0;
		}
		/* no connected peer or max retry reached, scan continue */
		hddLog(LOG1,
			  "%s: tdls disabled. connected_peers %d attempt %d. scan allowed",
			  __func__, connectedTdlsPeers,
			  pHddCtx->tdls_scan_ctxt.attempt);
		return 1;
	}
	/* while tdls is up, first time scan */
	else if (eTDLS_SUPPORT_ENABLED == pHddCtx->tdls_mode ||
		 eTDLS_SUPPORT_EXTERNAL_CONTROL == pHddCtx->tdls_mode ||
		 eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == pHddCtx->tdls_mode) {
		/* disable implicit trigger logic & tdls operatoin */
		wlan_hdd_tdls_set_mode(pHddCtx, eTDLS_SUPPORT_DISABLED, false);
		/* indicate the teardown all connected to peer */
		connectedTdlsPeers = wlan_hdd_tdls_connected_peers(pAdapter);
		if (connectedTdlsPeers) {
			uint8_t staIdx;
			uint8_t num = 0;
			uint8_t i;
			bool allPeersBufStas = 1;
			hddTdlsPeer_t *curr_peer;
			hddTdlsPeer_t *connectedPeerList[HDD_MAX_NUM_TDLS_STA];

			/* If TDLSScan is enabled then allow scan and
			 * maintain tdls link regardless if peer is buffer
			 * sta capable or not and if device is sleep sta
			 * capable or not. If peer is not buffer sta
			 * capable, then Tx would stop when device
			 * initiates scan and there will be loss of Rx
			 * packets since peer would not know when device
			 * moves away from the tdls channel.
			 */
			if (1 == pHddCtx->config->enable_tdls_scan) {
				hddLog(LOG1, FL(
				 "TDLSScan enabled, keep tdls link and allow scan, connectedTdlsPeers: %d"),
				 connectedTdlsPeers);
				return 1;
			}

			for (staIdx = 0; staIdx < pHddCtx->max_num_tdls_sta;
			     staIdx++) {
				if (pHddCtx->tdlsConnInfo[staIdx].staId) {
					curr_peer =
						wlan_hdd_tdls_find_all_peer(pHddCtx,
									    pHddCtx->
									    tdlsConnInfo
									    [staIdx].
									    peerMac.
									    bytes);
					if (curr_peer) {
						connectedPeerList[num++] =
							curr_peer;
						if (!(curr_peer->isBufSta))
							allPeersBufStas = 0;
					}
				}
			}

			if ((TDLS_MAX_CONNECTED_PEERS_TO_ALLOW_SCAN ==
			     connectedTdlsPeers) &&
			    (pHddCtx->config->fEnableTDLSSleepSta) &&
			    (allPeersBufStas)) {
				/* All connected peers bufStas and we can be sleepSta
				 * so allow scan
				 */
				hddLog(LOG1,
					  "%s: All peers (num %d) bufSTAs, we can be sleep sta, so allow scan, tdls mode changed to %d",
					  __func__, connectedTdlsPeers,
					  pHddCtx->tdls_mode);
				return 1;
			} else {
				for (i = 0; i < num; i++) {
					hddLog(LOG1,
						  "%s: indicate TDLS teadown (staId %d)",
						  __func__,
						  connectedPeerList[i]->staId);

					wlan_hdd_tdls_indicate_teardown
						(connectedPeerList[i]->pHddTdlsCtx->
						pAdapter, connectedPeerList[i],
						eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON);
					hdd_send_wlan_tdls_teardown_event(
						eTDLS_TEARDOWN_SCAN,
						connectedPeerList[i]->peerMac);
				}
			}
			/* schedule scan */
			delay =
				(unsigned long)(TDLS_DELAY_SCAN_PER_CONNECTION *
						connectedTdlsPeers);

			hddLog(LOG1,
				  "%s: tdls enabled (mode %d), connected_peers %d. schedule scan %lu msec",
				  __func__, pHddCtx->tdls_mode,
				  wlan_hdd_tdls_connected_peers(pAdapter), delay);

			wlan_hdd_tdls_scan_init_work(pHddCtx, wiphy,
						     request,
						     msecs_to_jiffies(delay));
			/* scan should not continue */
			return 0;
		}
		/* no connected peer, scan continue */
		hddLog(LOG1,
			  "%s: tdls_mode %d, and no tdls connection. scan allowed",
			  __func__, pHddCtx->tdls_mode);
	}
	EXIT();
	return 1;
}

/**
 * wlan_hdd_tdls_scan_done_callback() - callback for tdls scan done event
 * @pAdapter: HDD adapter
 *
 * Return: Void
 */
void wlan_hdd_tdls_scan_done_callback(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER();

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return;

	if (eTDLS_SUPPORT_NOT_ENABLED == pHddCtx->tdls_mode) {
		hdd_info("TDLS mode is disabled OR not enabled");
		return;
	}

	/* free allocated memory at scan time */
	wlan_hdd_tdls_free_scan_request(&pHddCtx->tdls_scan_ctxt);

	/* if tdls was enabled before scan, re-enable tdls mode */
	if (eTDLS_SUPPORT_ENABLED == pHddCtx->tdls_mode_last ||
	    eTDLS_SUPPORT_EXTERNAL_CONTROL == pHddCtx->tdls_mode_last ||
	    eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == pHddCtx->tdls_mode_last) {
		hddLog(LOG1,
			  ("%s: revert tdls mode %d"), __func__,
			  pHddCtx->tdls_mode_last);

		wlan_hdd_tdls_set_mode(pHddCtx, pHddCtx->tdls_mode_last, false);
	}
	EXIT();
}

/**
 * wlan_hdd_tdls_timer_restart() - restart TDLS timer
 * @pAdapter: HDD adapter
 * @timer: timer to restart
 * @expirationTime: new expiration time to set for the timer
 *
 * Return: Void
 */
void wlan_hdd_tdls_timer_restart(hdd_adapter_t *pAdapter,
				 qdf_mc_timer_t *timer,
				 uint32_t expirationTime)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	/* Check whether driver load unload is in progress */
	if (cds_is_load_or_unload_in_progress()) {
		hddLog(LOGE, FL("Driver load/unload is in progress."));
		return;
	}

	if (hdd_conn_is_connected(pHddStaCtx)) {
		qdf_mc_timer_stop(timer);
		qdf_mc_timer_start(timer, expirationTime);
	}
}

/**
 * wlan_hdd_tdls_indicate_teardown() - indicate tdls teardown
 * @pAdapter: HDD adapter
 * @curr_peer: peer tdls teardown happened
 * @reason: teardown reason
 *
 * Return: Void
 */
void wlan_hdd_tdls_indicate_teardown(hdd_adapter_t *pAdapter,
				     hddTdlsPeer_t *curr_peer, uint16_t reason)
{
	if (NULL == pAdapter || NULL == curr_peer) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("parameters passed are invalid"));
		return;
	}

	if (eTDLS_LINK_CONNECTED != curr_peer->link_status)
		return;

	wlan_hdd_tdls_set_peer_link_status(curr_peer,
					   eTDLS_LINK_TEARING,
					   eTDLS_LINK_UNSPECIFIED);
	hdd_info("Teardown reason %d", reason);
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
	hdd_context_t *pHddCtx;
	hdd_adapter_t *pAdapter;
	if (!curr_peer)
		return -EINVAL;
	pAdapter = curr_peer->pHddTdlsCtx->pAdapter;
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	if ((NULL == pHddCtx))
		return -EINVAL;
	mutex_lock(&pHddCtx->tdls_lock);
	curr_peer->state_change_notification = callback;
	mutex_unlock(&pHddCtx->tdls_lock);
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
	hdd_context_t *hddctx;
	hdd_adapter_t *adapter;

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
int wlan_hdd_tdls_get_status(hdd_adapter_t *pAdapter,
			     const uint8_t *mac, uint32_t *opclass,
			     uint32_t *channel, uint32_t *state,
			     int32_t *reason)
{
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (0 != (wlan_hdd_validate_context(pHddCtx)))
		return -EINVAL;

	mutex_lock(&pHddCtx->tdls_lock);
	curr_peer = wlan_hdd_tdls_find_peer(pAdapter, mac, false);
	if (curr_peer == NULL) {
		mutex_unlock(&pHddCtx->tdls_lock);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("curr_peer is NULL"));
		*state = QCA_WIFI_HAL_TDLS_DISABLED;
		*reason = eTDLS_LINK_UNSPECIFIED;
		return -EINVAL;
	}
	if (pHddCtx->config->fTDLSExternalControl &&
		(false == curr_peer->isForcedPeer)) {
		hdd_err("curr_peer is not Forced");
		*state = QCA_WIFI_HAL_TDLS_DISABLED;
		*reason = eTDLS_LINK_UNSPECIFIED;
	} else {
		wlan_hdd_tdls_determine_channel_opclass(pHddCtx, pAdapter,
					curr_peer, channel, opclass);
		wlan_hdd_tdls_get_wifi_hal_state(curr_peer, state, reason);
	}
	mutex_unlock(&pHddCtx->tdls_lock);

	return 0;
}

#ifdef FEATURE_WLAN_TDLS
static const struct nla_policy
	wlan_hdd_tdls_config_enable_policy[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX +
					   1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR] = {.type = NLA_UNSPEC},
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
	[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR] = {.type = NLA_UNSPEC},
};
static const struct nla_policy
	wlan_hdd_tdls_config_state_change_policy[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX
						 + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR] = {.type = NLA_UNSPEC},
	[QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON] = {.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS] = {.type =
								NLA_U32},
};
static const struct nla_policy
	wlan_hdd_tdls_config_get_status_policy
[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR] = {.type = NLA_UNSPEC},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON] = {.type = NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS] = {
							.type = NLA_U32},
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
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

	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return -EINVAL;
	if (pHddCtx->config->fTDLSExternalControl == false) {
		return -ENOTSUPP;
	}
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX,
		      data, data_len, wlan_hdd_tdls_config_get_status_policy)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("Invalid attribute"));
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr mac addr failed"));
		return -EINVAL;
	}
	memcpy(peer,
	       nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR]),
	       sizeof(peer));
	hddLog(QDF_TRACE_LEVEL_INFO, FL(MAC_ADDRESS_STR), MAC_ADDR_ARRAY(peer));
	ret = wlan_hdd_tdls_get_status(pAdapter, peer, &global_operating_class,
				&channel, &state, &reason);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("get status Failed"));
		return -EINVAL;
	}
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
						  4 * sizeof(int32_t) +
						  NLMSG_HDRLEN);
	if (!skb) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
		return -EINVAL;
	}
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Reason %d Status %d class %d channel %d peer " MAC_ADDRESS_STR),
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
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("nla put fail"));
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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) ctx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	struct sk_buff *skb = NULL;

	ENTER();

	if (wlan_hdd_validate_context(pHddCtx))
		return -EINVAL;
	if (pHddCtx->config->fTDLSExternalControl == false) {
		return -ENOTSUPP;
	}
	skb = cfg80211_vendor_event_alloc(pHddCtx->wiphy,
					  NULL,
					  EXTTDLS_EVENT_BUF_SIZE + NLMSG_HDRLEN,
					  QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX,
					  GFP_KERNEL);
	if (!skb) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("cfg80211_vendor_event_alloc failed"));
		return -EINVAL;
	}
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Reason: %d Status: %d Class: %d Channel: %d tdls peer " MAC_ADDRESS_STR),
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
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("nla put fail"));
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX + 1];
	QDF_STATUS status;
	tdls_req_params_t pReqMsg = { 0 };
	int ret;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status)
		return -EINVAL;
	if (pHddCtx->config->fTDLSExternalControl == false) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("TDLS External Control is not enabled"));
		return -ENOTSUPP;
	}
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX,
		      data, data_len, wlan_hdd_tdls_config_enable_policy)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("Invalid ATTR"));
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr mac addr failed"));
		return -EINVAL;
	}
	memcpy(peer, nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR]),
	       sizeof(peer));
	hddLog(QDF_TRACE_LEVEL_INFO, FL(MAC_ADDRESS_STR), MAC_ADDR_ARRAY(peer));
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr channel failed"));
		return -EINVAL;
	}
	pReqMsg.channel =
		nla_get_s32(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL]);
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Channel Num (%d)"), pReqMsg.channel);
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS]) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("attr operating class failed"));
		return -EINVAL;
	}
	pReqMsg.global_operating_class =
		nla_get_s32(tb
			    [QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS]);
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Operating class (%d)"),
	       pReqMsg.global_operating_class);
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr latency failed"));
		return -EINVAL;
	}
	pReqMsg.max_latency_ms =
		nla_get_s32(tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS]);
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Latency (%d)"),
	       pReqMsg.max_latency_ms);
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr bandwidth failed"));
		return -EINVAL;
	}
	pReqMsg.min_bandwidth_kbps =
		nla_get_s32(tb
			    [QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS]);
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Bandwidth (%d)"),
	       pReqMsg.min_bandwidth_kbps);
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX + 1];
	QDF_STATUS status;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status)
		return -EINVAL;
	if (pHddCtx->config->fTDLSExternalControl == false) {
		return -ENOTSUPP;
	}
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX,
		      data, data_len, wlan_hdd_tdls_config_disable_policy)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("Invalid ATTR"));
		return -EINVAL;
	}
	if (!tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR]) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("attr mac addr failed"));
		return -EINVAL;
	}
	memcpy(peer, nla_data(tb[QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR]),
	       sizeof(peer));
	hddLog(QDF_TRACE_LEVEL_INFO, FL(MAC_ADDRESS_STR), MAC_ADDR_ARRAY(peer));
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

/**
 * wlan_hdd_tdls_add_station() - add or change a TDLS peer station
 * @wiphy: wiphy
 * @dev: net device
 * @mac: MAC address of the TDLS peer
 * @update: if non-0, modify the peer with StaParams; if 0, add new peer
 * @StaParams: station parameters for the TDLS to change
 *
 * Return: 0 if success; negative errno otherwise
 */
int wlan_hdd_tdls_add_station(struct wiphy *wiphy,
			      struct net_device *dev, const uint8_t *mac,
			      bool update, tCsrStaParams *StaParams)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	QDF_STATUS status;
	hddTdlsPeer_t *pTdlsPeer;
	uint16_t numCurrTdlsPeers;
	unsigned long rc;
	long ret;

	ENTER();

	ret = wlan_hdd_validate_context(pHddCtx);
	if (ret)
		return ret;

	if ((eTDLS_SUPPORT_NOT_ENABLED == pHddCtx->tdls_mode) ||
	    (eTDLS_SUPPORT_DISABLED == pHddCtx->tdls_mode)) {
		hddLog(LOG1,
			  "%s: TDLS mode is disabled OR not enabled in FW."
			  MAC_ADDRESS_STR " Request declined.",
			  __func__, MAC_ADDR_ARRAY(mac));
		return -ENOTSUPP;
	}

	pTdlsPeer = wlan_hdd_tdls_get_peer(pAdapter, mac);

	if (NULL == pTdlsPeer) {
		hddLog(LOGE,
			  "%s: " MAC_ADDRESS_STR
			  " (update %d) not exist. return invalid", __func__,
			  MAC_ADDR_ARRAY(mac), update);
		return -EINVAL;
	}

	/* in add station, we accept existing valid staId if there is */
	if ((0 == update) &&
	    ((pTdlsPeer->link_status >= eTDLS_LINK_CONNECTING) ||
	     (TDLS_STA_INDEX_VALID(pTdlsPeer->staId)))) {
		hddLog(LOG1,
			  "%s: " MAC_ADDRESS_STR
			  " link_status %d. staId %d. add station ignored.",
			  __func__, MAC_ADDR_ARRAY(mac), pTdlsPeer->link_status,
			  pTdlsPeer->staId);
		return 0;
	}
	/* in change station, we accept only when staId is valid */
	if ((1 == update) &&
	    ((pTdlsPeer->link_status > eTDLS_LINK_CONNECTING) ||
	     (!TDLS_STA_INDEX_VALID(pTdlsPeer->staId)))) {
		hddLog(LOGE,
			  "%s: " MAC_ADDRESS_STR
			  " link status %d. staId %d. change station %s.",
			  __func__, MAC_ADDR_ARRAY(mac), pTdlsPeer->link_status,
			  pTdlsPeer->staId,
			  (TDLS_STA_INDEX_VALID(pTdlsPeer->staId)) ? "ignored" :
			  "declined");
		return (TDLS_STA_INDEX_VALID(pTdlsPeer->staId)) ? 0 : -EPERM;
	}

	/* when others are on-going, we want to change link_status to idle */
	if (NULL != wlan_hdd_tdls_is_progress(pHddCtx, mac, true)) {
		hddLog(LOG1,
			  "%s: " MAC_ADDRESS_STR
			  " TDLS setup is ongoing. Request declined.",
			  __func__, MAC_ADDR_ARRAY(mac));
		goto error;
	}

	/* first to check if we reached to maximum supported TDLS peer.
	   TODO: for now, return -EPERM looks working fine,
	   but need to check if any other errno fit into this category. */
	numCurrTdlsPeers = wlan_hdd_tdls_connected_peers(pAdapter);
	if (pHddCtx->max_num_tdls_sta <= numCurrTdlsPeers) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: " MAC_ADDRESS_STR
			  " TDLS Max peer already connected. Request declined."
			  " Num of peers (%d), Max allowed (%d).",
			  __func__, MAC_ADDR_ARRAY(mac), numCurrTdlsPeers,
			  pHddCtx->max_num_tdls_sta);
		goto error;
	} else {
		hddTdlsPeer_t *pTdlsPeer;
		pTdlsPeer = wlan_hdd_tdls_find_peer(pAdapter, mac, true);
		if (pTdlsPeer && TDLS_IS_CONNECTED(pTdlsPeer)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: " MAC_ADDRESS_STR
				  " already connected. Request declined.",
				  __func__, MAC_ADDR_ARRAY(mac));
			return -EPERM;
		}
	}
	if (0 == update)
		wlan_hdd_tdls_set_link_status(pAdapter,
					      mac,
					      eTDLS_LINK_CONNECTING,
					      eTDLS_LINK_SUCCESS);

	/* debug code */
	if (NULL != StaParams) {
		hddLog(LOG1,
			  "%s: TDLS Peer Parameters.", __func__);
		if (StaParams->htcap_present) {
			hddLog(LOG1,
				  "ht_capa->cap_info: %0x",
				  StaParams->HTCap.capInfo);
			hddLog(LOG1,
				  "ht_capa->extended_capabilities: %0x",
				  StaParams->HTCap.extendedHtCapInfo);
		}
		hddLog(LOG1,
			  "params->capability: %0x", StaParams->capability);
		hddLog(LOG1,
			  "params->ext_capab_len: %0x",
			  StaParams->extn_capability[0]);
		if (StaParams->vhtcap_present) {
			hddLog(LOG1,
				  "rxMcsMap %x rxHighest %x txMcsMap %x txHighest %x",
				  StaParams->VHTCap.suppMcs.rxMcsMap,
				  StaParams->VHTCap.suppMcs.rxHighest,
				  StaParams->VHTCap.suppMcs.txMcsMap,
				  StaParams->VHTCap.suppMcs.txHighest);
		}
		{
			int i = 0;
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
				  "Supported rates:");
			for (i = 0; i < sizeof(StaParams->supported_rates); i++)
				hddLog(LOG1, "[%d]: %x ", i,
					StaParams->supported_rates[i]);
		}
	} /* end debug code */
	else if ((1 == update) && (NULL == StaParams)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s : update is true, but staParams is NULL. Error!",
			  __func__);
		return -EPERM;
	}

	INIT_COMPLETION(pAdapter->tdls_add_station_comp);

	if (!update) {
		status = sme_add_tdls_peer_sta(WLAN_HDD_GET_HAL_CTX(pAdapter),
					       pAdapter->sessionId, mac);
	} else {
		status = sme_change_tdls_peer_sta(WLAN_HDD_GET_HAL_CTX(pAdapter),
						  pAdapter->sessionId, mac,
						  StaParams);
	}

	rc = wait_for_completion_timeout(&pAdapter->tdls_add_station_comp,
					 msecs_to_jiffies
						 (WAIT_TIME_TDLS_ADD_STA));

	if (!rc) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: timeout waiting for tdls add station indication",
			  __func__);
		return -EPERM;
	}

	if (QDF_STATUS_SUCCESS != pAdapter->tdlsAddStaStatus) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Add Station is unsuccessful", __func__);
		return -EPERM;
	}

	return 0;

error:
	wlan_hdd_tdls_set_link_status(pAdapter,
				      mac,
				      eTDLS_LINK_IDLE, eTDLS_LINK_UNSPECIFIED);
	return -EPERM;

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

	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	hdd_station_ctx_t *hdd_sta_ctx;
	u8 peerMac[QDF_MAC_ADDR_SIZE];
	QDF_STATUS status;
	int max_sta_failed = 0;
	int responder;
	unsigned long rc;
	uint16_t numCurrTdlsPeers;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
#if !(TDLS_MGMT_VERSION2)
	u32 peer_capability;
	peer_capability = 0;
#endif
#endif

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_TDLS_MGMT,
			 pAdapter->sessionId, action_code));

	if (wlan_hdd_validate_context(pHddCtx))
		return -EINVAL;

	if (eTDLS_SUPPORT_NOT_ENABLED == pHddCtx->tdls_mode) {
		hddLog(LOG1,
			  "%s: TDLS mode is disabled OR not enabled in FW."
			  MAC_ADDRESS_STR " action %d declined.",
			  __func__, MAC_ADDR_ARRAY(peer), action_code);
		return -ENOTSUPP;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	/*
	 * STA should be connected and authenticated before sending
	 * any TDLS frames
	 */
	if ((eConnectionState_Associated !=
	     hdd_sta_ctx->conn_info.connState) ||
	     (false == hdd_sta_ctx->conn_info.uIsAuthenticated)) {
		hdd_err("STA is not connected or not authenticated. connState %u, uIsAuthenticated %u",
			hdd_sta_ctx->conn_info.connState,
			hdd_sta_ctx->conn_info.uIsAuthenticated);
		return -EAGAIN;
	}

	/* If any concurrency is detected */
	if (((1 << QDF_STA_MODE) != pHddCtx->concurrency_mode) ||
	    (pHddCtx->no_of_active_sessions[QDF_STA_MODE] > 1)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Multiple STA OR Concurrency detected. Ignore TDLS MGMT frame. action_code=%d, concurrency_mode: 0x%x, active_sessions: %d",
			  __func__,
			  action_code,
			  pHddCtx->concurrency_mode,
			  pHddCtx->no_of_active_sessions[QDF_STA_MODE]);
		return -EPERM;
	}
	/* other than teardown frame, mgmt frames are not sent if disabled */
	if (SIR_MAC_TDLS_TEARDOWN != action_code) {
		/* if tdls_mode is disabled to respond to peer's request */
		if (eTDLS_SUPPORT_DISABLED == pHddCtx->tdls_mode) {
			hddLog(LOG1,
				  "%s: " MAC_ADDRESS_STR
				  " TDLS mode is disabled. action %d declined.",
				  __func__, MAC_ADDR_ARRAY(peer), action_code);
			return -ENOTSUPP;
		}
	}

	if (WLAN_IS_TDLS_SETUP_ACTION(action_code)) {
		if (NULL != wlan_hdd_tdls_is_progress(pHddCtx, peer, true)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: " MAC_ADDRESS_STR
				  " TDLS setup is ongoing. action %d declined.",
				  __func__, MAC_ADDR_ARRAY(peer), action_code);
			return -EPERM;
		}
	}

	if (SIR_MAC_TDLS_SETUP_REQ == action_code ||
	    SIR_MAC_TDLS_SETUP_RSP == action_code) {
		numCurrTdlsPeers = wlan_hdd_tdls_connected_peers(pAdapter);
		if (pHddCtx->max_num_tdls_sta <= numCurrTdlsPeers) {
			/* supplicant still sends tdls_mgmt(SETUP_REQ) even after
			   we return error code at 'add_station()'. Hence we have this
			   check again in addtion to add_station().
			   Anyway, there is no hard to double-check. */
			if (SIR_MAC_TDLS_SETUP_REQ == action_code) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: " MAC_ADDRESS_STR
					  " TDLS Max peer already connected. action (%d) declined. Num of peers (%d), Max allowed (%d).",
					  __func__, MAC_ADDR_ARRAY(peer),
					  action_code, numCurrTdlsPeers,
					  pHddCtx->max_num_tdls_sta);
				return -EINVAL;
			} else {
				/* maximum reached. tweak to send error code to peer and return
				   error code to supplicant */
				status_code = eSIR_MAC_UNSPEC_FAILURE_STATUS;
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: " MAC_ADDRESS_STR
					  " TDLS Max peer already connected, send response status (%d). Num of peers (%d), Max allowed (%d).",
					  __func__, MAC_ADDR_ARRAY(peer),
					  status_code, numCurrTdlsPeers,
					  pHddCtx->max_num_tdls_sta);
				max_sta_failed = -EPERM;
				/* fall through to send setup resp with failure status
				   code */
			}
		} else {
			hddTdlsPeer_t *pTdlsPeer;
			pTdlsPeer =
				wlan_hdd_tdls_find_peer(pAdapter, peer, true);
			if (pTdlsPeer && TDLS_IS_CONNECTED(pTdlsPeer)) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s:" MAC_ADDRESS_STR
					  " already connected. action %d declined.",
					  __func__, MAC_ADDR_ARRAY(peer),
					  action_code);
				return -EPERM;
			}
		}
	}
	qdf_mem_copy(peerMac, peer, 6);

	hddLog(LOG1,
		  "%s: " MAC_ADDRESS_STR
		  " action %d, dialog_token %d status %d, len = %zu",
		  "tdls_mgmt", MAC_ADDR_ARRAY(peer), action_code, dialog_token,
		  status_code, len);

	/*Except teardown responder will not be used so just make 0 */
	responder = 0;
	if (SIR_MAC_TDLS_TEARDOWN == action_code) {

		hddTdlsPeer_t *pTdlsPeer;
		pTdlsPeer = wlan_hdd_tdls_find_peer(pAdapter, peerMac, true);

		if (pTdlsPeer && TDLS_IS_CONNECTED(pTdlsPeer))
			responder = pTdlsPeer->is_responder;
		else {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: " MAC_ADDRESS_STR
				  " peer doesn't exist or not connected %d dialog_token %d status %d, len = %zu",
				  __func__, MAC_ADDR_ARRAY(peer),
				  (NULL ==
				   pTdlsPeer) ? -1 : pTdlsPeer->link_status,
				  dialog_token, status_code, len);
			return -EPERM;
		}
	}

	/* For explicit trigger of DIS_REQ come out of BMPS for
	   successfully receiving DIS_RSP from peer. */
	if ((SIR_MAC_TDLS_SETUP_RSP == action_code) ||
	    (SIR_MAC_TDLS_DIS_RSP == action_code) ||
	    (SIR_MAC_TDLS_DIS_REQ == action_code)) {
		/* Fw will take care if PS offload is enabled. */
		if (SIR_MAC_TDLS_DIS_REQ != action_code)
			wlan_hdd_tdls_set_cap(pAdapter, peerMac,
					      eTDLS_CAP_SUPPORTED);
	}

	/* make sure doesn't call send_mgmt() while it is pending */
	if (TDLS_CTX_MAGIC == pAdapter->mgmtTxCompletionStatus) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: " MAC_ADDRESS_STR
			  " action %d couldn't sent, as one is pending. return EBUSY",
			  __func__, MAC_ADDR_ARRAY(peer), action_code);
		return -EBUSY;
	}

	pAdapter->mgmtTxCompletionStatus = TDLS_CTX_MAGIC;
	INIT_COMPLETION(pAdapter->tdls_mgmt_comp);

	status = sme_send_tdls_mgmt_frame(WLAN_HDD_GET_HAL_CTX(pAdapter),
					  pAdapter->sessionId, peerMac,
					  action_code, dialog_token, status_code,
					  peer_capability, (uint8_t *) buf, len,
					  !responder);

	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_send_tdls_mgmt_frame failed!", __func__);
		pAdapter->mgmtTxCompletionStatus = false;
		return -EINVAL;
	}

	if ((SIR_MAC_TDLS_DIS_REQ == action_code) ||
	    (SIR_MAC_TDLS_DIS_RSP == action_code)) {
		/* for DIS_REQ/DIS_RSP, supplicant does not consider the return
		 * status. So no need to wait for tdls_mgmt_comp to
		 * send ack status.
		 */
		hdd_info("tx done for frm %u", action_code);
		return 0;
	}
	hdd_info("Wait for tdls_mgmt_comp. Timeout %u ms",
		WAIT_TIME_TDLS_MGMT);

	rc = wait_for_completion_timeout(&pAdapter->tdls_mgmt_comp,
					 msecs_to_jiffies(WAIT_TIME_TDLS_MGMT));

	if ((0 == rc) || (true != pAdapter->mgmtTxCompletionStatus)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Mgmt Tx Completion timed out TxCompletion %u",
			  __func__, pAdapter->mgmtTxCompletionStatus);

		if (cds_is_driver_recovering()) {
			hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
				cds_get_driver_state());
			return -EAGAIN;
		}

		if (cds_is_driver_unloading()) {
			hdd_err("Unload in progress. State: 0x%x Ignore!!!",
				cds_get_driver_state());
			return -EAGAIN;
		}
		if (rc <= 0)
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_HDD_TIME_OUT,
				true, false);
		pAdapter->mgmtTxCompletionStatus = false;
		return -EINVAL;
	}
	hdd_info("Mgmt Tx Completion status %ld TxCompletion %u",
		rc, pAdapter->mgmtTxCompletionStatus);

	if (max_sta_failed) {
		return max_sta_failed;
	}

	if (SIR_MAC_TDLS_SETUP_RSP == action_code) {
		return wlan_hdd_tdls_set_responder(pAdapter, peerMac, false);
	} else if (SIR_MAC_TDLS_SETUP_CNF == action_code) {
		return wlan_hdd_tdls_set_responder(pAdapter, peerMac, true);
	}

	return 0;
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
int wlan_hdd_tdls_extctrl_config_peer(hdd_adapter_t *pAdapter,
				      const uint8_t *peer,
				      cfg80211_exttdls_callback callback,
				      u32 chan,
				      u32 max_latency,
				      u32 op_class, u32 min_bandwidth)
{
	hddTdlsPeer_t *pTdlsPeer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s : NL80211_TDLS_SETUP for " MAC_ADDRESS_STR,
		  __func__, MAC_ADDR_ARRAY(peer));
	if ((false == pHddCtx->config->fTDLSExternalControl) ||
	    (false == pHddCtx->config->fEnableTDLSImplicitTrigger)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s TDLS External control or Implicit Trigger not enabled ",
			  __func__);
		return -ENOTSUPP;
	}
	pTdlsPeer = wlan_hdd_tdls_get_peer(pAdapter, peer);
	if (pTdlsPeer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: peer " MAC_ADDRESS_STR " does not exist",
			  __func__, MAC_ADDR_ARRAY(peer));
		return -EINVAL;
	}
	if (0 != wlan_hdd_tdls_set_force_peer(pAdapter, peer, true)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS Add Force Peer Failed", __func__);
		return -EINVAL;
	}
	/* Update the peer mac to firmware, so firmware
	 * could update the connection table
	 */
	if (0 != wlan_hdd_tdls_update_peer_mac(pAdapter, peer,
	    eSME_TDLS_PEER_ADD_MAC_ADDR)) {
		hddLog(LOGE, FL("TDLS Peer mac update Failed " MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(peer));
		return -EINVAL;
	}

	/* validate if off channel is DFS channel */
	if (CDS_IS_DFS_CH(chan)) {
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
		return -EINVAL;
	}
	if (0 != wlan_hdd_set_callback(pTdlsPeer, callback)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS set callback Failed", __func__);
		return -EINVAL;
	}
	return 0;
}

/**
 * wlan_hdd_tdls_extctrl_deconfig_peer() - de-configure an externally
 *                                         controllable TDLS peer
 * @pAdapter: HDD adapter
 * @peer: MAC address of the tdls peer
 *
 * Return: 0 if success; negative errno otherwisw
 */
int wlan_hdd_tdls_extctrl_deconfig_peer(hdd_adapter_t *pAdapter,
					const uint8_t *peer)
{
	hddTdlsPeer_t *pTdlsPeer;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s : NL80211_TDLS_TEARDOWN for " MAC_ADDRESS_STR,
		  __func__, MAC_ADDR_ARRAY(peer));
	if ((false == pHddCtx->config->fTDLSExternalControl) ||
	    (false == pHddCtx->config->fEnableTDLSImplicitTrigger)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s TDLS External control or Implicit Trigger not enabled ",
			  __func__);
		return -ENOTSUPP;
	}
	pTdlsPeer = wlan_hdd_tdls_find_peer(pAdapter, peer, true);
	if (NULL == pTdlsPeer) {
		hddLog(QDF_TRACE_LEVEL_INFO, "%s: " MAC_ADDRESS_STR
		       "peer matching MAC_ADDRESS_STR not found",
		       __func__, MAC_ADDR_ARRAY(peer));
		return -EINVAL;
	} else {
		wlan_hdd_tdls_indicate_teardown(pAdapter, pTdlsPeer,
						eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON);
		hdd_send_wlan_tdls_teardown_event(
			eTDLS_TEARDOWN_EXT_CTRL,
			pTdlsPeer->peerMac);
	}
	if (0 != wlan_hdd_tdls_set_force_peer(pAdapter, peer, false)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s Failed", __func__);
		return -EINVAL;
	}
	/* Update the peer mac to firmware, so firmware
	 * could update the connection table
	 */
	if (0 != wlan_hdd_tdls_update_peer_mac(pAdapter, peer,
	    eSME_TDLS_PEER_REMOVE_MAC_ADDR)) {
		hddLog(LOGE, FL("TDLS Peer mac update Failed " MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(peer));
		return -EINVAL;
	}
	if (0 != wlan_hdd_set_callback(pTdlsPeer, NULL)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s TDLS set callback Failed", __func__);
		return -EINVAL;
	}
	return 0;
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	int status;
	tSmeTdlsPeerStateParams smeTdlsPeerStateParams;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	hddTdlsPeer_t *pTdlsPeer;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
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

	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	/* QCA 2.0 Discrete ANDs feature capability in HDD config with that
	 * received from target, so HDD config gives combined intersected result
	 */
	if (false == pHddCtx->config->fEnableTDLSSupport) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "TDLS Disabled in INI OR not enabled in FW. "
			  "Cannot process TDLS commands");
		return -ENOTSUPP;
	}

	switch (oper) {
	case NL80211_TDLS_ENABLE_LINK:
	{
		QDF_STATUS status;
		unsigned long rc;
		tCsrTdlsLinkEstablishParams tdlsLinkEstablishParams;

		pTdlsPeer =
			wlan_hdd_tdls_find_peer(pAdapter, peer, true);

		if (NULL == pTdlsPeer) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: peer matching " MAC_ADDRESS_STR
				  " not found, ignore NL80211_TDLS_ENABLE_LINK",
				  __func__, MAC_ADDR_ARRAY(peer));
			return -EINVAL;
		}

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: NL80211_TDLS_ENABLE_LINK for peer "
			  MAC_ADDRESS_STR " link_status: %d",
			  __func__, MAC_ADDR_ARRAY(peer),
			  pTdlsPeer->link_status);

		if (!TDLS_STA_INDEX_VALID(pTdlsPeer->staId)) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: invalid sta index %u for "
				  MAC_ADDRESS_STR
				  " TDLS_ENABLE_LINK failed", __func__,
				  pTdlsPeer->staId,
				  MAC_ADDR_ARRAY(peer));
			return -EINVAL;
		}

		if (eTDLS_LINK_CONNECTED != pTdlsPeer->link_status) {
			if (IS_ADVANCE_TDLS_ENABLE) {

				if (0 !=
				    wlan_hdd_tdls_get_link_establish_params
					    (pAdapter, peer,
					    &tdlsLinkEstablishParams)) {
					return -EINVAL;
				}
				INIT_COMPLETION(pAdapter->
						tdls_link_establish_req_comp);

				sme_send_tdls_link_establish_params
					(WLAN_HDD_GET_HAL_CTX(pAdapter),
					pAdapter->sessionId, peer,
					&tdlsLinkEstablishParams);
				/* Send TDLS peer UAPSD capabilities to the firmware and
				 * register with the TL on after the response for this operation
				 * is received .
				 */
				rc = wait_for_completion_timeout
					     (&pAdapter->
					     tdls_link_establish_req_comp,
					     msecs_to_jiffies
						     (WAIT_TIME_TDLS_LINK_ESTABLISH_REQ));
				if (!rc) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: Link Establish Request timed out",
						  __func__);
					return -EINVAL;
				}
			}
			wlan_hdd_tdls_set_peer_link_status(pTdlsPeer,
							   eTDLS_LINK_CONNECTED,
							   eTDLS_LINK_SUCCESS);
			/* start TDLS client registration with TL */
			status =
				hdd_roam_register_tdlssta(pAdapter, peer,
							  pTdlsPeer->staId,
							  pTdlsPeer->
							  signature);
			if (QDF_STATUS_SUCCESS == status) {
				uint8_t i;

				qdf_mem_zero(&smeTdlsPeerStateParams,
					     sizeof
					     (tSmeTdlsPeerStateParams));

				smeTdlsPeerStateParams.vdevId =
					pAdapter->sessionId;
				qdf_mem_copy(&smeTdlsPeerStateParams.
					     peerMacAddr,
					     &pTdlsPeer->peerMac,
					     sizeof(tSirMacAddr));
				smeTdlsPeerStateParams.peerState =
					eSME_TDLS_PEER_STATE_CONNECTED;
				smeTdlsPeerStateParams.peerCap.
				isPeerResponder =
					pTdlsPeer->is_responder;
				smeTdlsPeerStateParams.peerCap.
				peerUapsdQueue =
					pTdlsPeer->uapsdQueues;
				smeTdlsPeerStateParams.peerCap.
				peerMaxSp = pTdlsPeer->maxSp;
				smeTdlsPeerStateParams.peerCap.
				peerBuffStaSupport =
					pTdlsPeer->isBufSta;
				smeTdlsPeerStateParams.peerCap.
				peerOffChanSupport =
					pTdlsPeer->isOffChannelSupported;
				smeTdlsPeerStateParams.peerCap.
				peerCurrOperClass = 0;
				smeTdlsPeerStateParams.peerCap.
				selfCurrOperClass = 0;
				smeTdlsPeerStateParams.peerCap.
				peerChanLen =
					pTdlsPeer->supported_channels_len;
				smeTdlsPeerStateParams.peerCap.
				prefOffChanNum =
					pTdlsPeer->pref_off_chan_num;
				smeTdlsPeerStateParams.peerCap.
				prefOffChanBandwidth =
					pHddCtx->config->
					fTDLSPrefOffChanBandwidth;
				smeTdlsPeerStateParams.peerCap.
					opClassForPrefOffChan =
						pTdlsPeer->
						op_class_for_pref_off_chan;

				if (CDS_IS_DFS_CH(smeTdlsPeerStateParams.
					peerCap.prefOffChanNum)) {
					hdd_err("Resetting TDLS off-channel from %d to %d",
					       smeTdlsPeerStateParams.peerCap.
						prefOffChanNum,
					       CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT);
					smeTdlsPeerStateParams.peerCap.prefOffChanNum =
						CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEFAULT;
				}

				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: Peer " MAC_ADDRESS_STR
					  "vdevId: %d, peerState: %d, isPeerResponder: %d, uapsdQueues: 0x%x, maxSp: 0x%x, peerBuffStaSupport: %d, peerOffChanSupport: %d, peerCurrOperClass: %d, selfCurrOperClass: %d, peerChanLen: %d, peerOperClassLen: %d, prefOffChanNum: %d, prefOffChanBandwidth: %d, op_class_for_pref_off_chan: %d",
					  __func__,
					  MAC_ADDR_ARRAY(peer),
					  smeTdlsPeerStateParams.vdevId,
					  smeTdlsPeerStateParams.
					  peerState,
					  smeTdlsPeerStateParams.
					  peerCap.isPeerResponder,
					  smeTdlsPeerStateParams.
					  peerCap.peerUapsdQueue,
					  smeTdlsPeerStateParams.
					  peerCap.peerMaxSp,
					  smeTdlsPeerStateParams.
					  peerCap.peerBuffStaSupport,
					  smeTdlsPeerStateParams.
					  peerCap.peerOffChanSupport,
					  smeTdlsPeerStateParams.
					  peerCap.peerCurrOperClass,
					  smeTdlsPeerStateParams.
					  peerCap.selfCurrOperClass,
					  smeTdlsPeerStateParams.
					  peerCap.peerChanLen,
					  smeTdlsPeerStateParams.
					  peerCap.peerOperClassLen,
					  smeTdlsPeerStateParams.
					  peerCap.prefOffChanNum,
					  smeTdlsPeerStateParams.
					  peerCap.prefOffChanBandwidth,
					  pTdlsPeer->
					  op_class_for_pref_off_chan);

				for (i = 0;
				     i <
				     pTdlsPeer->supported_channels_len;
				     i++) {
					smeTdlsPeerStateParams.peerCap.
					peerChan[i] =
						pTdlsPeer->
						supported_channels[i];
				}
				smeTdlsPeerStateParams.peerCap.
				peerOperClassLen =
					pTdlsPeer->
					supported_oper_classes_len;
				for (i = 0;
				     i <
				     pTdlsPeer->
				     supported_oper_classes_len; i++) {
					smeTdlsPeerStateParams.peerCap.
					peerOperClass[i] =
						pTdlsPeer->
						supported_oper_classes[i];
				}

				qdf_ret_status =
					sme_update_tdls_peer_state(pHddCtx->
								   hHal,
								   &smeTdlsPeerStateParams);
				if (QDF_STATUS_SUCCESS !=
				    qdf_ret_status) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: sme_update_tdls_peer_state failed for "
						  MAC_ADDRESS_STR,
						  __func__,
						  MAC_ADDR_ARRAY(peer));
					return -EPERM;
				}
				wlan_hdd_tdls_increment_peer_count
					(pAdapter);
			}

			/* Update TL about the UAPSD masks , to route the packets to firmware */
			if ((true ==
			     pHddCtx->config->fEnableTDLSBufferSta)
			    || pHddCtx->config->fTDLSUapsdMask) {
				int ac;
				uint8_t ucAc[4] = { SME_AC_VO,
						    SME_AC_VI,
						    SME_AC_BK,
						    SME_AC_BE};
				uint8_t tlTid[4] = { 7, 5, 2, 3 };
				for (ac = 0; ac < 4; ac++) {
					status = sme_enable_uapsd_for_ac(
						 (WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
						 pTdlsPeer->staId, ucAc[ac],
						 tlTid[ac], tlTid[ac], 0, 0,
						 SME_BI_DIR, 1,
						 pAdapter->sessionId,
						 pHddCtx->config->DelayedTriggerFrmInt);
				}
			}
		}
		hdd_wlan_tdls_enable_link_event(peer,
			pTdlsPeer->isOffChannelSupported,
			0, 0);
	}
	break;
	case NL80211_TDLS_DISABLE_LINK:
	{
		pTdlsPeer =
			wlan_hdd_tdls_find_peer(pAdapter, peer, true);

		if (NULL == pTdlsPeer) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: peer matching " MAC_ADDRESS_STR
				  " not found, ignore NL80211_TDLS_DISABLE_LINK",
				  __func__, MAC_ADDR_ARRAY(peer));
			return -EINVAL;
		}

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: NL80211_TDLS_DISABLE_LINK for peer "
			  MAC_ADDRESS_STR " link_status: %d",
			  __func__, MAC_ADDR_ARRAY(peer),
			  pTdlsPeer->link_status);

		if (TDLS_STA_INDEX_VALID(pTdlsPeer->staId)) {
			unsigned long rc;

			INIT_COMPLETION(pAdapter->
					tdls_del_station_comp);

			sme_delete_tdls_peer_sta(WLAN_HDD_GET_HAL_CTX
							 (pAdapter),
						 pAdapter->sessionId,
						 peer);

			rc = wait_for_completion_timeout(&pAdapter->
							 tdls_del_station_comp,
							 msecs_to_jiffies
								 (WAIT_TIME_TDLS_DEL_STA));
			if (!rc) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: Del station timed out",
					  __func__);
				return -EPERM;
			}
			wlan_hdd_tdls_set_peer_link_status(pTdlsPeer,
						eTDLS_LINK_IDLE,
						(pTdlsPeer->link_status ==
							eTDLS_LINK_TEARING) ?
						eTDLS_LINK_UNSPECIFIED :
						eTDLS_LINK_DROPPED_BY_REMOTE);
		} else {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: TDLS Peer Station doesn't exist.",
				  __func__);
		}
	}
	break;
	case NL80211_TDLS_TEARDOWN:
	{
		status =
			wlan_hdd_tdls_extctrl_deconfig_peer(pAdapter, peer);

		if (0 != status) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: Error in TDLS Teardown",
				  __func__);
			return status;
		}
	}
	break;
	case NL80211_TDLS_SETUP:
	{
		status = wlan_hdd_tdls_extctrl_config_peer(pAdapter,
				peer, NULL,
				pHddCtx->config->fTDLSPrefOffChanNum, 0, 0, 0);
		if (0 != status) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: Error in TDLS Setup", __func__);
			return status;
		}
	}
	break;
	case NL80211_TDLS_DISCOVERY_REQ:
		/* We don't support in-driver setup/teardown/discovery */
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "%s: We don't support in-driver setup/teardown/discovery",
			  __func__);
		return -ENOTSUPP;
	default:
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: unsupported event %d", __func__, oper);
		return -ENOTSUPP;
	}
	EXIT();
	return 0;
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
	hddLog(QDF_TRACE_LEVEL_INFO,
	       "tdls send discover req: " MAC_ADDRESS_STR,
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
hddTdlsPeer_t *wlan_hdd_tdls_find_first_connected_peer(hdd_adapter_t *adapter)
{
	int i;
	struct list_head *head;
	struct list_head *pos;
	hddTdlsPeer_t *curr_peer = NULL;
	tdlsCtx_t *hdd_tdls_ctx;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (wlan_hdd_validate_context(hdd_ctx))
		return NULL;

	mutex_lock(&hdd_ctx->tdls_lock);
	hdd_tdls_ctx = WLAN_HDD_GET_TDLS_CTX_PTR(adapter);
	if (NULL == hdd_tdls_ctx) {
		mutex_unlock(&hdd_ctx->tdls_lock);
		return NULL;
	}
	for (i = 0; i < TDLS_PEER_LIST_SIZE; i++) {
		head = &hdd_tdls_ctx->peer_list[i];
		list_for_each(pos, head) {
			curr_peer = list_entry(pos, hddTdlsPeer_t, node);
			if (curr_peer && (curr_peer->link_status ==
					eTDLS_LINK_CONNECTED)) {
				mutex_unlock(&hdd_ctx->tdls_lock);
				hddLog(LOG1,
					FL(MAC_ADDRESS_STR
					   " eTDLS_LINK_CONNECTED"
					),
					MAC_ADDR_ARRAY(curr_peer->peerMac));
				return curr_peer;
			}
		}
	}
	mutex_unlock(&hdd_ctx->tdls_lock);
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
int hdd_set_tdls_offchannel(hdd_context_t *hdd_ctx, int offchannel)
{
	if ((true == hdd_ctx->config->fEnableTDLSOffChannel) &&
	    (eTDLS_SUPPORT_ENABLED == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXTERNAL_CONTROL == hdd_ctx->tdls_mode ||
	     eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == hdd_ctx->tdls_mode)) {
		if (offchannel < CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN ||
			offchannel > CFG_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX) {
			hddLog(LOGE, FL("Invalid tdls off channel %u"),
				offchannel);
			return -EINVAL;
		}
	} else {
		hddLog(LOGE,
			FL("Either TDLS or TDLS Off-channel is not enabled"));
		return  -ENOTSUPP;
	}
	hddLog(LOG1,
		FL("change tdls off channel from %d to %d"),
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
int hdd_set_tdls_secoffchanneloffset(hdd_context_t *hdd_ctx, int offchanoffset)
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
			hddLog(LOGE,
				FL(
				   "Invalid tdls secondary off channel offset %d"
				  ),
				offchanoffset);
			return -EINVAL;
		} /* end switch */
	} else {
		hddLog(LOGE,
			FL("Either TDLS or TDLS Off-channel is not enabled"));
		return  -ENOTSUPP;
	}
	hddLog(LOG1,
		FL("change tdls secondary off channel offset to 0x%x"),
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
int hdd_set_tdls_offchannelmode(hdd_adapter_t *adapter, int offchanmode)
{
	hddTdlsPeer_t *conn_peer = NULL;
	hdd_station_ctx_t *hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	sme_tdls_chan_switch_params chan_switch_params;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (offchanmode < ENABLE_CHANSWITCH ||
			offchanmode > DISABLE_CHANSWITCH) {
		hddLog(LOGE,
			FL("Invalid tdls off channel mode %d"),
			offchanmode);
		return -EINVAL;
	}
	if (eConnectionState_Associated != hdd_sta_ctx->conn_info.connState) {
		hddLog(LOGE,
			FL(
			   "tdls off channel mode req in not associated state %d"
			  ),
			offchanmode);
		return -EPERM;
	}
	if ((true == hdd_ctx->config->fEnableTDLSOffChannel) &&
		(eTDLS_SUPPORT_ENABLED == hdd_ctx->tdls_mode ||
		 eTDLS_SUPPORT_EXTERNAL_CONTROL == hdd_ctx->tdls_mode ||
		 eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == hdd_ctx->tdls_mode)) {
		 conn_peer = wlan_hdd_tdls_find_first_connected_peer(adapter);
		 if (NULL == conn_peer) {
			hddLog(LOGP,
				FL("No TDLS Connected Peer"));
		return -EPERM;
	}
	} else {
		hddLog(LOGP,
			FL("TDLS Connection not supported"));
		return -ENOTSUPP;
	}

	hddLog(LOG1,
		FL("TDLS Channel Switch in swmode=%d tdls_off_channel %d offchanoffset %d"),
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
			hddLog(LOGE,
				FL(
				   "TDLS off-channel parameters are not set yet!!!"
				));
			return -EINVAL;
		}
		break;
	case DISABLE_CHANSWITCH:
		chan_switch_params.tdls_off_channel = 0;
		chan_switch_params.tdls_off_ch_bw_offset = 0;
		chan_switch_params.opclass = 0;
		break;
	default:
		hddLog(LOGE,
			FL(
			   "Incorrect Parameters mode: %d tdls_off_channel: %d offchanoffset: %d"
			  ),
			offchanmode, hdd_ctx->tdls_off_channel,
			hdd_ctx->tdls_channel_offset);
		return -EINVAL;
	} /* end switch */

	chan_switch_params.vdev_id = adapter->sessionId;
	chan_switch_params.tdls_off_ch_mode = offchanmode;
	chan_switch_params.is_responder =
		conn_peer->is_responder;
	qdf_mem_copy(&chan_switch_params.peer_mac_addr,
		     &conn_peer->peerMac,
		     sizeof(tSirMacAddr));
	hdd_log(LOG1,
		FL("Peer " MAC_ADDRESS_STR
		   " vdevId: %d, off channel: %d, offset: %d, mode: %d, is_responder: %d"),
		MAC_ADDR_ARRAY(chan_switch_params.peer_mac_addr),
		chan_switch_params.vdev_id,
		chan_switch_params.tdls_off_channel,
		chan_switch_params.tdls_off_ch_bw_offset,
		chan_switch_params.tdls_off_ch_mode,
		chan_switch_params.is_responder);

	status = sme_send_tdls_chan_switch_req(WLAN_HDD_GET_HAL_CTX(adapter),
			&chan_switch_params);

	if (status != QDF_STATUS_SUCCESS) {
		hdd_log(LOG1,
			FL("Failed to send channel switch request to sme"));
		return -EINVAL;
	}

	hdd_ctx->tdls_fw_off_chan_mode = offchanmode;

	if (ENABLE_CHANSWITCH == offchanmode) {
		conn_peer->pref_off_chan_num =
			chan_switch_params.tdls_off_channel;
		conn_peer->op_class_for_pref_off_chan =
			chan_switch_params.opclass;
	}

	return 0;
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
int hdd_set_tdls_scan_type(hdd_context_t *hdd_ctx, int val)
{
	if ((val != 0) && (val != 1)) {
		hddLog(LOGE, FL("Incorrect value of tdls scan type: %d"),
		       val);
		return -EINVAL;
	} else {
		hdd_ctx->config->enable_tdls_scan = val;
		return 0;
	}
}
