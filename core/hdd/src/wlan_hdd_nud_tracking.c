/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains nud event tracking main function definitions
 */

#include "wlan_hdd_main.h"

void hdd_nud_set_gateway_addr(struct hdd_adapter *adapter,
			      struct qdf_mac_addr gw_mac_addr)
{
	qdf_mem_copy(adapter->nud_tracking.gw_mac_addr.bytes,
		     gw_mac_addr.bytes,
		     sizeof(struct qdf_mac_addr));
}

void hdd_nud_cfg_print(struct hdd_context *hdd_ctx)
{
	hdd_debug("Name = [%s] value = [0x%x]",
		  CFG_ENABLE_NUD_TRACKING_NAME,
		  hdd_ctx->config->enable_nud_tracking);
}

void hdd_nud_incr_gw_rx_pkt_cnt(struct hdd_adapter *adapter,
				struct qdf_mac_addr *mac_addr)
{
	if (!adapter->nud_tracking.is_gw_rx_pkt_track_enabled)
		return;

	if (qdf_is_macaddr_equal(&adapter->nud_tracking.gw_mac_addr,
				 mac_addr))
		qdf_atomic_inc(&adapter
			       ->nud_tracking.tx_rx_stats.gw_rx_packets);
}

/**
 * hdd_nud_flush_work() - flush pending nud work
 * @adapter: Pointer to hdd adapter
 *
 * Return: None
 */
static inline void
hdd_nud_flush_work(struct hdd_adapter *adapter)
{
	qdf_disable_work(&adapter->nud_tracking.nud_event_work);
}

void hdd_nud_deinit_tracking(struct hdd_adapter *adapter)
{
	qdf_destroy_work(NULL, &adapter->nud_tracking.nud_event_work);
}

void hdd_nud_ignore_tracking(struct hdd_adapter *adapter, bool ignoring)
{
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (adapter->device_mode == QDF_STA_MODE &&
	    hdd_ctx->config->enable_nud_tracking)
		adapter->nud_tracking.ignore_nud_tracking = ignoring;
}

void hdd_nud_reset_tracking(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (adapter->device_mode == QDF_STA_MODE &&
	    hdd_ctx->config->enable_nud_tracking) {
		hdd_debug("De Initialize the NUD tracking");

		qdf_zero_macaddr(&adapter->nud_tracking.gw_mac_addr);
		qdf_mem_zero(&adapter->nud_tracking.tx_rx_stats,
			     sizeof(struct hdd_nud_tx_rx_stats));

		adapter->nud_tracking.curr_state = NUD_NONE;
		qdf_atomic_set(&adapter
			       ->nud_tracking.tx_rx_stats.gw_rx_packets, 0);

		hdd_nud_flush_work(adapter);
	}
}

/**
 * hdd_nud_stats_info() - display wlan NUD stats info
 * @hdd_adapter: Pointer to hdd adapter
 *
 * Return: None
 */
static void hdd_nud_stats_info(struct hdd_adapter *adapter)
{
	hdd_debug("**** NUD STATS: ****");
	hdd_debug("NUD Probe Tx  : %d",
		  adapter->nud_tracking.tx_rx_stats.pre_tx_packets);
	hdd_debug("NUD Probe Ack : %d",
		  adapter->nud_tracking.tx_rx_stats.pre_tx_acked);
	hdd_debug("NUD Probe Rx  : %d",
		  adapter->nud_tracking.tx_rx_stats.pre_rx_packets);
	hdd_debug("NUD Failure Tx  : %d",
		  adapter->nud_tracking.tx_rx_stats.post_tx_packets);
	hdd_debug("NUD Failure Ack : %d",
		  adapter->nud_tracking.tx_rx_stats.post_tx_acked);
	hdd_debug("NUD Failure Rx  : %d",
		  adapter->nud_tracking.tx_rx_stats.post_rx_packets);
	hdd_debug("NUD Gateway Rx  : %d",
		  qdf_atomic_read(&adapter
				  ->nud_tracking.tx_rx_stats.gw_rx_packets));
}

/**
 * hdd_nud_capture_stats() - capture wlan NUD stats
 * @hdd_adapter: Pointer to hdd adapter
 * @nud_state: NUD state for which stats to capture
 *
 * Return: None
 */
static void hdd_nud_capture_stats(struct hdd_adapter *adapter,
				  uint8_t nud_state)
{
	switch (nud_state) {
	case NUD_INCOMPLETE:
	case NUD_PROBE:
		adapter->nud_tracking.tx_rx_stats.pre_tx_packets =
				adapter->stats.tx_packets;
		adapter->nud_tracking.tx_rx_stats.pre_rx_packets =
				adapter->stats.rx_packets;
		adapter->nud_tracking.tx_rx_stats.pre_tx_acked =
				hdd_txrx_get_tx_ack_count(adapter);
		break;
	case NUD_FAILED:
		adapter->nud_tracking.tx_rx_stats.post_tx_packets =
				adapter->stats.tx_packets;
		adapter->nud_tracking.tx_rx_stats.post_rx_packets =
				adapter->stats.rx_packets;
		adapter->nud_tracking.tx_rx_stats.post_tx_acked =
				hdd_txrx_get_tx_ack_count(adapter);
		break;
	default:
		break;
	}
}

/**
 * hdd_nud_honour_failure() - check if nud failure to be honored
 * @hdd_adapter: Pointer to hdd_adapter
 *
 * Return: true if nud failure to be honored, else false.
 */
static bool hdd_nud_honour_failure(struct hdd_adapter *adapter)
{
	uint32_t tx_transmitted;
	uint32_t tx_acked;
	uint32_t gw_rx_pkt;

	tx_transmitted = adapter->nud_tracking.tx_rx_stats.post_tx_packets -
		adapter->nud_tracking.tx_rx_stats.pre_tx_packets;
	tx_acked = adapter->nud_tracking.tx_rx_stats.post_tx_acked -
		adapter->nud_tracking.tx_rx_stats.pre_tx_acked;
	gw_rx_pkt = qdf_atomic_read(&adapter
			->nud_tracking.tx_rx_stats.gw_rx_packets);

	if (!tx_transmitted || !tx_acked || !gw_rx_pkt) {
		hdd_debug("NUD_FAILURE_HONORED");
		hdd_nud_stats_info(adapter);
		return true;
	}
	hdd_debug("NUD_FAILURE_NOT_HONORED");
	hdd_nud_stats_info(adapter);
	return false;
}

/**
 * hdd_nud_set_tracking() - set the NUD tracking info
 * @hdd_adapter: Pointer to hdd_adapter
 * @nud_state: Current NUD state to set
 * @capture_enabled: GW Rx packet to be capture or not
 *
 * Return: None
 */
static void hdd_nud_set_tracking(struct hdd_adapter *adapter,
				 uint8_t nud_state,
				 bool capture_enabled)
{
	hdd_debug("set the NUD tracking");

	adapter->nud_tracking.curr_state = nud_state;
	qdf_atomic_set(&adapter->nud_tracking.tx_rx_stats.gw_rx_packets, 0);
	adapter->nud_tracking.is_gw_rx_pkt_track_enabled = capture_enabled;
}

/**
 * __hdd_nud_failure_work() - work for nud event
 * @data: Pointer to hdd_adapter
 *
 * Return: None
 */
static void __hdd_nud_failure_work(void *data)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	eConnectionState conn_state;
	int status;

	hdd_enter();

	if (!data)
		return;

	adapter = (struct hdd_adapter *)data;

	status = hdd_validate_adapter(adapter);
	if (status)
		return;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	conn_state = (WLAN_HDD_GET_STATION_CTX_PTR(adapter))
		      ->conn_info.connState;

	if (eConnectionState_Associated != conn_state) {
		hdd_debug("Not in Connected State");
		return;
	}

	qdf_mutex_acquire(&adapter->disconnection_status_lock);
	if (adapter->disconnection_in_progress) {
		qdf_mutex_release(&adapter->disconnection_status_lock);
		hdd_debug("Disconnect is already in progress");
		return;
	}
	adapter->disconnection_in_progress = true;
	qdf_mutex_release(&adapter->disconnection_status_lock);

	hdd_debug("Disconnecting STA with session id: %d",
		  adapter->session_id);
	/* Issue Disconnect */
	status = wlan_hdd_disconnect(adapter, eCSR_DISCONNECT_REASON_DEAUTH);
	if (0 != status) {
		hdd_err("wlan_hdd_disconnect failed, status: %d", status);
		hdd_set_disconnect_status(adapter, false);
	}

	hdd_exit();
}

/**
 * hdd_nud_failure_work() - work for nud event
 * @data: Pointer to hdd_adapter
 *
 * Return: None
 */
static void hdd_nud_failure_work(void *data)
{
	cds_ssr_protect(__func__);
	__hdd_nud_failure_work(data);
	cds_ssr_unprotect(__func__);
}

void hdd_nud_init_tracking(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (adapter->device_mode == QDF_STA_MODE &&
	    hdd_ctx->config->enable_nud_tracking) {
		hdd_debug("Initialize the NUD tracking");

		qdf_zero_macaddr(&adapter->nud_tracking.gw_mac_addr);
		qdf_mem_zero(&adapter->nud_tracking.tx_rx_stats,
			     sizeof(struct hdd_nud_tx_rx_stats));

		adapter->nud_tracking.curr_state = NUD_NONE;
		adapter->nud_tracking.ignore_nud_tracking = false;

		qdf_atomic_init(&adapter
				->nud_tracking.tx_rx_stats.gw_rx_packets);
		qdf_create_work(0, &adapter->nud_tracking.nud_event_work,
				hdd_nud_failure_work,
				(void *)adapter);
	}
}

/**
 * hdd_nud_process_failure_event() - processing NUD_FAILED event
 * @hdd_adapter: Pointer to hdd_adapter
 *
 * Return: None
 */
static void hdd_nud_process_failure_event(struct hdd_adapter *adapter)
{
	uint8_t curr_state;

	curr_state = adapter->nud_tracking.curr_state;
	if (curr_state == NUD_PROBE || curr_state == NUD_INCOMPLETE) {
		hdd_nud_capture_stats(adapter, NUD_FAILED);
		if (hdd_nud_honour_failure(adapter))
			qdf_sched_work(0, &adapter
					->nud_tracking.nud_event_work);
		else
			hdd_nud_set_tracking(adapter, NUD_NONE, false);
	} else {
		hdd_debug("NUD FAILED -> Current State [0x%x]", curr_state);
	}
}

/**
 * hdd_nud_filter_netevent() - filter netevents for STA interface
 * @neighbour: Pointer to neighbour
 *
 * Return: None
 */
static void hdd_nud_filter_netevent(struct neighbour *neigh)
{
	int status;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	eConnectionState conn_state;
	const struct net_device *netdev = neigh->dev;

	hdd_enter();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	adapter = hdd_get_adapter_by_macaddr(hdd_ctx, netdev->dev_addr);
	status = hdd_validate_adapter(adapter);
	if (status)
		return;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Device_mode %s(%d) is not supported for NUD handling",
			hdd_device_mode_to_string(adapter->device_mode),
			adapter->device_mode);
		return;
	}
	conn_state = (WLAN_HDD_GET_STATION_CTX_PTR(adapter))
		->conn_info.connState;

	if (eConnectionState_Associated != conn_state) {
		hdd_debug("Not in Connected State");
		return;
	}

	if (adapter->nud_tracking.ignore_nud_tracking) {
		hdd_debug("NUD Tracking is Disabled");
		return;
	}

	if (!qdf_is_macaddr_equal(&adapter->nud_tracking.gw_mac_addr,
				  (struct qdf_mac_addr *)&neigh->ha[0])) {
		hdd_debug("NUD event not for registered GW");
		return;
	}
	switch (neigh->nud_state) {
	case NUD_PROBE:
	case NUD_INCOMPLETE:
		hdd_debug("NUD_START [0x%x]", neigh->nud_state);
		hdd_nud_capture_stats(adapter, neigh->nud_state);
		hdd_nud_set_tracking(adapter,
				     neigh->nud_state,
				     true);
		break;

	case NUD_REACHABLE:
		hdd_debug("NUD_REACHABLE [0x%x]", neigh->nud_state);
		hdd_nud_set_tracking(adapter, NUD_NONE, false);
		break;

	case NUD_FAILED:
		hdd_debug("NUD_FAILED [0x%x]", neigh->nud_state);
		hdd_nud_process_failure_event(adapter);
		break;
	default:
		hdd_debug("NUD Event For Other State [0x%x]",
			  neigh->nud_state);
		break;
	}
	hdd_exit();
}

/**
 * __hdd_nud_netevent_cb() - netevent callback
 * @nb: Pointer to notifier block
 * @event: Net Event triggered
 * @data: Pointer to neighbour struct
 *
 * Callback for netevent
 *
 * Return: None
 */
static void __hdd_nud_netevent_cb(struct notifier_block *nb,
				  unsigned long event,
				  void *data)
{
	hdd_enter();
	hdd_nud_filter_netevent(data);
	hdd_exit();
}

/**
 * hdd_nud_netevent_cb() - netevent callback
 * @nb: Pointer to notifier block
 * @event: Net Event triggered
 * @data: Pointer to neighbour struct
 *
 * Callback for netevent
 *
 * Return: 0 on success
 */
static int hdd_nud_netevent_cb(struct notifier_block *nb, unsigned long event,
			       void *data)
{
	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
		cds_ssr_protect(__func__);
		__hdd_nud_netevent_cb(nb, event, data);
		cds_ssr_unprotect(__func__);
		break;
	case NETEVENT_REDIRECT:
	default:
		break;
	}
	return 0;
}

static struct notifier_block wlan_netevent_nb = {
	.notifier_call = hdd_nud_netevent_cb
};

int hdd_nud_register_netevent_notifier(struct hdd_context *hdd_ctx)
{
	int ret = 0;

	if (hdd_ctx->config->enable_nud_tracking) {
		ret = register_netevent_notifier(&wlan_netevent_nb);
		if (!ret)
			hdd_debug("Registered netevent notifier");
	}
	return ret;
}

void hdd_nud_unregister_netevent_notifier(struct hdd_context *hdd_ctx)
{
	int ret;

	if (hdd_ctx->config->enable_nud_tracking) {
		ret = unregister_netevent_notifier(&wlan_netevent_nb);
		if (!ret)
			hdd_debug("Unregistered netevent notifier");
	}
}
