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
 * DOC: wlan_hdd_tx_rx.c
 *
 * Linux HDD Tx/RX APIs
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_napi.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/inetdevice.h>
#include <cds_sched.h>
#include <cds_utils.h>

#include <wlan_hdd_p2p.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <net/ieee80211_radiotap.h>
#include "sap_api.h"
#include "wlan_hdd_wmm.h"
#include "wlan_hdd_tdls.h"
#include <wlan_hdd_ipa.h>
#include "wlan_hdd_ocb.h"
#include "wlan_hdd_lro.h"
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_peer_ops.h>
#include <cdp_txrx_flow_ctrl_v2.h>
#include "wlan_hdd_nan_datapath.h"
#include "pld_common.h"
#include <cdp_txrx_misc.h>
#include "wlan_hdd_rx_monitor.h"
#include "wlan_hdd_power.h"
#include <wlan_hdd_tsf.h>

/*
 * Count to ratelimit the HDD logs during TX failures
 */
#define HDD_TX_BLOCKED_RATE 256

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/*
 * Mapping Linux AC interpretation to SME AC.
 * Host has 5 tx queues, 4 flow-controlled queues for regular traffic and
 * one non-flow-controlled queue for high priority control traffic(EOPOL, DHCP).
 * The fifth queue is mapped to AC_VO to allow for proper prioritization.
 */
const uint8_t hdd_qdisc_ac_to_tl_ac[] = {
	SME_AC_VO,
	SME_AC_VI,
	SME_AC_BE,
	SME_AC_BK,
	SME_AC_VO,
};

#else
const uint8_t hdd_qdisc_ac_to_tl_ac[] = {
	SME_AC_VO,
	SME_AC_VI,
	SME_AC_BE,
	SME_AC_BK,
};

#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
/**
 * hdd_tx_resume_timer_expired_handler() - TX Q resume timer handler
 * @adapter_context: pointer to vdev adapter
 *
 * If Blocked OS Q is not resumed during timeout period, to prevent
 * permanent stall, resume OS Q forcefully.
 *
 * Return: None
 */
void hdd_tx_resume_timer_expired_handler(void *adapter_context)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) adapter_context;

	if (!adapter) {
		/* INVALID ARG */
		return;
	}

	hdd_debug("Enabling queues");
	wlan_hdd_netif_queue_control(adapter, WLAN_WAKE_ALL_NETIF_QUEUE,
				     WLAN_CONTROL_PATH);
}
#if defined(CONFIG_PER_VDEV_TX_DESC_POOL)

/**
 * hdd_tx_resume_false() - Resume OS TX Q false leads to queue disabling
 * @adapter: pointer to hdd adapter
 * @tx_resume: TX Q resume trigger
 *
 *
 * Return: None
 */
static void
hdd_tx_resume_false(struct hdd_adapter *adapter, bool tx_resume)
{
	if (true == tx_resume)
		return;

	/* Pause TX  */
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
				     WLAN_DATA_FLOW_CONTROL);

	if (QDF_TIMER_STATE_STOPPED ==
			qdf_mc_timer_get_current_state(&adapter->
						       tx_flow_control_timer)) {
		QDF_STATUS status;

		status = qdf_mc_timer_start(&adapter->tx_flow_control_timer,
				WLAN_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("Failed to start tx_flow_control_timer");
		else
			adapter->hdd_stats.tx_rx_stats.txflow_timer_cnt++;
	}

	adapter->hdd_stats.tx_rx_stats.txflow_pause_cnt++;
	adapter->hdd_stats.tx_rx_stats.is_txflow_paused = true;
}
#else

static inline void
hdd_tx_resume_false(struct hdd_adapter *adapter, bool tx_resume)
{
}
#endif

static inline struct sk_buff *hdd_skb_orphan(struct hdd_adapter *adapter,
		struct sk_buff *skb)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int need_orphan = 0;

	if (adapter->tx_flow_low_watermark > 0) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 19, 0))
		/*
		 * The TCP TX throttling logic is changed a little after
		 * 3.19-rc1 kernel, the TCP sending limit will be smaller,
		 * which will throttle the TCP packets to the host driver.
		 * The TCP UP LINK throughput will drop heavily. In order to
		 * fix this issue, need to orphan the socket buffer asap, which
		 * will call skb's destructor to notify the TCP stack that the
		 * SKB buffer is unowned. And then the TCP stack will pump more
		 * packets to host driver.
		 *
		 * The TX packets might be dropped for UDP case in the iperf
		 * testing. So need to be protected by follow control.
		 */
		need_orphan = 1;
#else
		if (hdd_ctx->config->tx_orphan_enable)
			need_orphan = 1;
#endif
	} else if (hdd_ctx->config->tx_orphan_enable) {
		if (qdf_nbuf_is_ipv4_tcp_pkt(skb) ||
		    qdf_nbuf_is_ipv6_tcp_pkt(skb))
			need_orphan = 1;
	}

	if (need_orphan) {
		skb_orphan(skb);
		++adapter->hdd_stats.tx_rx_stats.tx_orphaned;
	} else
		skb = skb_unshare(skb, GFP_ATOMIC);

	return skb;
}

/**
 * hdd_tx_resume_cb() - Resume OS TX Q.
 * @adapter_context: pointer to vdev apdapter
 * @tx_resume: TX Q resume trigger
 *
 * Q was stopped due to WLAN TX path low resource condition
 *
 * Return: None
 */
void hdd_tx_resume_cb(void *adapter_context, bool tx_resume)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) adapter_context;
	struct hdd_station_ctx *hdd_sta_ctx = NULL;

	if (!adapter) {
		/* INVALID ARG */
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* Resume TX  */
	if (true == tx_resume) {
		if (QDF_TIMER_STATE_STOPPED !=
		    qdf_mc_timer_get_current_state(&adapter->
						   tx_flow_control_timer)) {
			qdf_mc_timer_stop(&adapter->tx_flow_control_timer);
		}
		hdd_debug("Enabling queues");
		wlan_hdd_netif_queue_control(adapter,
					     WLAN_WAKE_ALL_NETIF_QUEUE,
					     WLAN_DATA_FLOW_CONTROL);
	}
	hdd_tx_resume_false(adapter, tx_resume);
}

bool hdd_tx_flow_control_is_pause(void *adapter_context)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) adapter_context;

	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		/* INVALID ARG */
		hdd_err("invalid adapter %pK", adapter);
		return false;
	}

	return adapter->pause_map & (1 << WLAN_DATA_FLOW_CONTROL);
}

void hdd_register_tx_flow_control(struct hdd_adapter *adapter,
		qdf_mc_timer_callback_t timer_callback,
		ol_txrx_tx_flow_control_fp flow_control_fp,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause_fp)
{
	if (adapter->tx_flow_timer_initialized == false) {
		qdf_mc_timer_init(&adapter->tx_flow_control_timer,
			  QDF_TIMER_TYPE_SW,
			  timer_callback,
			  adapter);
		adapter->tx_flow_timer_initialized = true;
	}
	cdp_fc_register(cds_get_context(QDF_MODULE_ID_SOC),
		adapter->session_id, flow_control_fp, adapter,
		flow_control_is_pause_fp);
}

/**
 * hdd_deregister_tx_flow_control() - Deregister TX Flow control
 * @adapter: adapter handle
 *
 * Return: none
 */
void hdd_deregister_tx_flow_control(struct hdd_adapter *adapter)
{
	cdp_fc_deregister(cds_get_context(QDF_MODULE_ID_SOC),
			adapter->session_id);
	if (adapter->tx_flow_timer_initialized == true) {
		qdf_mc_timer_stop(&adapter->tx_flow_control_timer);
		qdf_mc_timer_destroy(&adapter->tx_flow_control_timer);
		adapter->tx_flow_timer_initialized = false;
	}
}

/**
 * hdd_get_tx_resource() - check tx resources and take action
 * @adapter: adapter handle
 * @STAId: station id
 * @timer_value: timer value
 *
 * Return: none
 */
void hdd_get_tx_resource(struct hdd_adapter *adapter,
			uint8_t STAId, uint16_t timer_value)
{
	if (false ==
	    cdp_fc_get_tx_resource(cds_get_context(QDF_MODULE_ID_SOC), STAId,
				   adapter->tx_flow_low_watermark,
				   adapter->tx_flow_high_watermark_offset)) {
		hdd_debug("Disabling queues lwm %d hwm offset %d",
			 adapter->tx_flow_low_watermark,
			 adapter->tx_flow_high_watermark_offset);
		wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_DATA_FLOW_CONTROL);
		if ((adapter->tx_flow_timer_initialized == true) &&
		    (QDF_TIMER_STATE_STOPPED ==
		    qdf_mc_timer_get_current_state(&adapter->
						    tx_flow_control_timer))) {
			qdf_mc_timer_start(&adapter->tx_flow_control_timer,
					   timer_value);
			adapter->hdd_stats.tx_rx_stats.txflow_timer_cnt++;
			adapter->hdd_stats.tx_rx_stats.txflow_pause_cnt++;
			adapter->hdd_stats.tx_rx_stats.is_txflow_paused = true;
		}
	}
}

#else
/**
 * hdd_skb_orphan() - skb_unshare a cloned packed else skb_orphan
 * @adapter: pointer to HDD adapter
 * @skb: pointer to skb data packet
 *
 * Return: pointer to skb structure
 */
static inline struct sk_buff *hdd_skb_orphan(struct hdd_adapter *adapter,
		struct sk_buff *skb) {

	struct sk_buff *nskb;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 19, 0))
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
#endif

	hdd_skb_fill_gso_size(adapter->dev, skb);

	nskb = skb_unshare(skb, GFP_ATOMIC);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 19, 0))
	if (unlikely(hdd_ctx->config->tx_orphan_enable) && (nskb == skb)) {
		/*
		 * For UDP packets we want to orphan the packet to allow the app
		 * to send more packets. The flow would ultimately be controlled
		 * by the limited number of tx descriptors for the vdev.
		 */
		++adapter->hdd_stats.tx_rx_stats.tx_orphaned;
		skb_orphan(skb);
	}
#endif
	return nskb;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * qdf_event_eapol_log() - send event to wlan diag
 * @skb: skb ptr
 * @dir: direction
 * @eapol_key_info: eapol key info
 *
 * Return: None
 */
void hdd_event_eapol_log(struct sk_buff *skb, enum qdf_proto_dir dir)
{
	int16_t eapol_key_info;

	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event, struct host_event_wlan_eapol);

	if ((dir == QDF_TX &&
		(QDF_NBUF_CB_PACKET_TYPE_EAPOL !=
		 QDF_NBUF_CB_GET_PACKET_TYPE(skb))))
		return;
	else if (!qdf_nbuf_is_ipv4_eapol_pkt(skb))
		return;

	eapol_key_info = (uint16_t)(*(uint16_t *)
				(skb->data + EAPOL_KEY_INFO_OFFSET));

	wlan_diag_event.event_sub_type =
		(dir == QDF_TX ?
		 WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED :
		 WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED);
	wlan_diag_event.eapol_packet_type = (uint8_t)(*(uint8_t *)
				(skb->data + EAPOL_PACKET_TYPE_OFFSET));
	wlan_diag_event.eapol_key_info = eapol_key_info;
	wlan_diag_event.eapol_rate = 0;
	qdf_mem_copy(wlan_diag_event.dest_addr,
			(skb->data + QDF_NBUF_DEST_MAC_OFFSET),
			sizeof(wlan_diag_event.dest_addr));
	qdf_mem_copy(wlan_diag_event.src_addr,
			(skb->data + QDF_NBUF_SRC_MAC_OFFSET),
			sizeof(wlan_diag_event.src_addr));

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event, EVENT_WLAN_EAPOL);
}


/**
 * wlan_hdd_classify_pkt() - classify packet
 * @skb - sk buff
 *
 * Return: none
 */
void wlan_hdd_classify_pkt(struct sk_buff *skb)
{
	struct ethhdr *eh = (struct ethhdr *)skb->data;

	qdf_mem_set(skb->cb, sizeof(skb->cb), 0);

	/* check destination mac address is broadcast/multicast */
	if (is_broadcast_ether_addr((uint8_t *)eh))
		QDF_NBUF_CB_GET_IS_BCAST(skb) = true;
	else if (is_multicast_ether_addr((uint8_t *)eh))
		QDF_NBUF_CB_GET_IS_MCAST(skb) = true;

	if (qdf_nbuf_is_ipv4_arp_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_ARP;
	else if (qdf_nbuf_is_ipv4_dhcp_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_DHCP;
	else if (qdf_nbuf_is_ipv4_eapol_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_EAPOL;
	else if (qdf_nbuf_is_ipv4_wapi_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_WAPI;
	else if (qdf_nbuf_is_icmp_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_ICMP;
	else if (qdf_nbuf_is_icmpv6_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_ICMPv6;
}

/**
 * wlan_hdd_latency_opt()- latency option
 * @adapter:  pointer to the adapter structure
 * @skb:      pointer to sk buff
 *
 * Function to disable power save for icmp packets.
 *
 * Return: None
 */
#ifdef WLAN_ICMP_DISABLE_PS
static inline void
wlan_hdd_latency_opt(struct hdd_adapter *adapter, struct sk_buff *skb)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (hdd_ctx->config->icmp_disable_ps_val <= 0)
		return;

	if (QDF_NBUF_CB_GET_PACKET_TYPE(skb) ==
				QDF_NBUF_CB_PACKET_TYPE_ICMP) {
		wlan_hdd_set_powersave(adapter, false,
				hdd_ctx->config->icmp_disable_ps_val);
		sme_ps_enable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX(adapter),
					  adapter->session_id,
					  hdd_ctx->config->icmp_disable_ps_val);
	}
}
#else
static inline void
wlan_hdd_latency_opt(struct hdd_adapter *adapter, struct sk_buff *skb)
{
}
#endif

/**
 * hdd_get_transmit_sta_id() - function to retrieve station id to be used for
 * sending traffic towards a particular destination address. The destination
 * address can be unicast, multicast or broadcast
 *
 * @adapter: Handle to adapter context
 * @dst_addr: Destination address
 * @station_id: station id
 *
 * Returns: None
 */
static void hdd_get_transmit_sta_id(struct hdd_adapter *adapter,
			struct sk_buff *skb, uint8_t *station_id)
{
	bool mcbc_addr = false;
	QDF_STATUS status;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct qdf_mac_addr *dst_addr = NULL;

	dst_addr = (struct qdf_mac_addr *)skb->data;
	status = hdd_get_peer_sta_id(sta_ctx, dst_addr, station_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		if (QDF_NBUF_CB_GET_IS_BCAST(skb) ||
				QDF_NBUF_CB_GET_IS_MCAST(skb)) {
			hdd_debug("Received MC/BC packet for transmission");
			mcbc_addr = true;
		}
	}

	if (adapter->device_mode == QDF_IBSS_MODE ||
		adapter->device_mode == QDF_NDI_MODE) {
		/*
		 * This check is necessary to make sure station id is not
		 * overwritten for UC traffic in IBSS or NDI mode
		 */
		if (mcbc_addr)
			*station_id = sta_ctx->broadcast_staid;
	} else {
		/* For the rest, traffic is directed to AP/P2P GO */
		if (eConnectionState_Associated == sta_ctx->conn_info.connState)
			*station_id = sta_ctx->conn_info.staId[0];
	}
}

/**
 * hdd_is_tx_allowed() - check if Tx is allowed based on current peer state
 * @skb: pointer to OS packet (sk_buff)
 * @peer_id: Peer STA ID in peer table
 *
 * This function gets the peer state from DP and check if it is either
 * in OL_TXRX_PEER_STATE_CONN or OL_TXRX_PEER_STATE_AUTH. Only EAP packets
 * are allowed when peer_state is OL_TXRX_PEER_STATE_CONN. All packets
 * allowed when peer_state is OL_TXRX_PEER_STATE_AUTH.
 *
 * Return: true if Tx is allowed and false otherwise.
 */
static inline bool hdd_is_tx_allowed(struct sk_buff *skb, uint8_t peer_id)
{
	enum ol_txrx_peer_state peer_state;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	void *peer;

	QDF_BUG(soc);
	QDF_BUG(pdev);

	peer = cdp_peer_find_by_local_id(soc, pdev, peer_id);

	if (peer == NULL) {
		hdd_err_ratelimited(HDD_TX_BLOCKED_RATE,
				    "Unable to find peer entry for staid: %d",
				    peer_id);
		return false;
	}

	peer_state = cdp_peer_state_get(soc, peer);
	if (likely(OL_TXRX_PEER_STATE_AUTH == peer_state))
		return true;
	if (OL_TXRX_PEER_STATE_CONN == peer_state &&
	    ntohs(skb->protocol) == HDD_ETHERTYPE_802_1_X)
		return true;
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Invalid peer state for Tx: %d"), peer_state);
	return false;
}

/**
 * __hdd_hard_start_xmit() - Transmit a frame
 * @skb: pointer to OS packet (sk_buff)
 * @dev: pointer to network device
 *
 * Function registered with the Linux OS for transmitting
 * packets. This version of the function directly passes
 * the packet to Transport Layer.
 * In case of any packet drop or error, log the error with
 * INFO HIGH/LOW/MEDIUM to avoid excessive logging in kmsg.
 *
 * Return: Always returns NETDEV_TX_OK
 */
static int __hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	QDF_STATUS status;
	sme_ac_enum_type ac;
	enum sme_qos_wmmuptype up;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	bool granted;
	uint8_t STAId;
	struct hdd_station_ctx *sta_ctx = &adapter->session.station;
	struct qdf_mac_addr *mac_addr;
	bool pkt_proto_logged = false;
#ifdef QCA_PKT_PROTO_TRACE
	uint8_t proto_type = 0;
#endif
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	bool is_arp;

#ifdef QCA_WIFI_FTM
	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}
#endif

	++adapter->hdd_stats.tx_rx_stats.tx_called;
	adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;

	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state() ||
	    cds_is_load_or_unload_in_progress()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Recovery/(Un)load in progress, dropping the packet");
		goto drop_pkt;
	}

	wlan_hdd_classify_pkt(skb);
	if (QDF_NBUF_CB_GET_PACKET_TYPE(skb) == QDF_NBUF_CB_PACKET_TYPE_ARP) {
		is_arp = true;
		if (qdf_nbuf_data_is_arp_req(skb) &&
		    (hdd_ctx->track_arp_ip == qdf_nbuf_get_arp_tgt_ip(skb))) {
			++adapter->hdd_stats.hdd_arp_stats.tx_arp_req_count;
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
					"%s : ARP packet", __func__);
		}
	}

	if (cds_is_driver_recovering()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_WARN,
			"Recovery in progress, dropping the packet");
		goto drop_pkt;
	}

	STAId = HDD_WLAN_INVALID_STA_ID;

	hdd_get_transmit_sta_id(adapter, skb, &STAId);
	if (STAId >= WLAN_MAX_STA_COUNT) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Invalid station id, transmit operation suspended");
		goto drop_pkt;
	}

	hdd_get_tx_resource(adapter, STAId,
				WLAN_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

	/* Get TL AC corresponding to Qdisc queue index/AC. */
	ac = hdd_qdisc_ac_to_tl_ac[skb->queue_mapping];

	if (!qdf_nbuf_ipa_owned_get(skb)) {
		skb = hdd_skb_orphan(adapter, skb);
		if (!skb)
			goto drop_pkt_accounting;
	}

	/*
	 * Add SKB to internal tracking table before further processing
	 * in WLAN driver.
	 */
	qdf_net_buf_debug_acquire_skb(skb, __FILE__, __LINE__);

	/*
	 * user priority from IP header, which is already extracted and set from
	 * select_queue call back function
	 */
	up = skb->priority;

	++adapter->hdd_stats.tx_rx_stats.tx_classified_ac[ac];
#ifdef HDD_WMM_DEBUG
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Classified as ac %d up %d", __func__, ac, up);
#endif /* HDD_WMM_DEBUG */

	if (HDD_PSB_CHANGED == adapter->psb_changed) {
		/*
		 * Function which will determine acquire admittance for a
		 * WMM AC is required or not based on psb configuration done
		 * in the framework
		 */
		hdd_wmm_acquire_access_required(adapter, ac);
	}
	/*
	 * Make sure we already have access to this access category
	 * or it is EAPOL or WAPI frame during initial authentication which
	 * can have artifically boosted higher qos priority.
	 */

	if (((adapter->psb_changed & (1 << ac)) &&
		likely(adapter->hdd_wmm_status.wmmAcStatus[ac].
			wmmAcAccessAllowed)) ||
		((sta_ctx->conn_info.uIsAuthenticated == false) &&
		 (QDF_NBUF_CB_PACKET_TYPE_EAPOL ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb) ||
		  QDF_NBUF_CB_PACKET_TYPE_WAPI ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb)))) {
		granted = true;
	} else {
		status = hdd_wmm_acquire_access(adapter, ac, &granted);
		adapter->psb_changed |= (1 << ac);
	}

	if (!granted) {
		bool isDefaultAc = false;
		/*
		 * ADDTS request for this AC is sent, for now
		 * send this packet through next avaiable lower
		 * Access category until ADDTS negotiation completes.
		 */
		while (!likely
			       (adapter->hdd_wmm_status.wmmAcStatus[ac].
			       wmmAcAccessAllowed)) {
			switch (ac) {
			case SME_AC_VO:
				ac = SME_AC_VI;
				up = SME_QOS_WMM_UP_VI;
				break;
			case SME_AC_VI:
				ac = SME_AC_BE;
				up = SME_QOS_WMM_UP_BE;
				break;
			case SME_AC_BE:
				ac = SME_AC_BK;
				up = SME_QOS_WMM_UP_BK;
				break;
			default:
				ac = SME_AC_BK;
				up = SME_QOS_WMM_UP_BK;
				isDefaultAc = true;
				break;
			}
			if (isDefaultAc)
				break;
		}
		skb->priority = up;
		skb->queue_mapping = hdd_linux_up_to_ac_map[up];
	}

#ifdef QCA_PKT_PROTO_TRACE
	if ((hdd_ctx->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_EAPOL) ||
	    (hdd_ctx->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_DHCP)) {
		proto_type = cds_pkt_get_proto_type(skb,
						    hdd_ctx->config->gEnableDebugLog,
						    0);
		if (CDS_PKT_TRAC_TYPE_EAPOL & proto_type)
			cds_pkt_trace_buf_update("ST:T:EPL");
		else if (CDS_PKT_TRAC_TYPE_DHCP & proto_type)
			cds_pkt_trace_buf_update("ST:T:DHC");
	}
#endif /* QCA_PKT_PROTO_TRACE */

	adapter->stats.tx_bytes += skb->len;

	mac_addr = (struct qdf_mac_addr *)skb->data;

	ucfg_tdls_update_tx_pkt_cnt(adapter->hdd_vdev, mac_addr);

	if (qdf_nbuf_is_tso(skb))
		adapter->stats.tx_packets += qdf_nbuf_get_tso_num_seg(skb);
	else
		++adapter->stats.tx_packets;

	hdd_event_eapol_log(skb, QDF_TX);
	pkt_proto_logged = qdf_dp_trace_log_pkt(adapter->session_id,
						skb, QDF_TX,
						QDF_TRACE_DEFAULT_PDEV_ID);
	QDF_NBUF_CB_TX_PACKET_TRACK(skb) = QDF_NBUF_TX_PKT_DATA_TRACK;
	QDF_NBUF_UPDATE_TX_PKT_COUNT(skb, QDF_NBUF_TX_PKT_HDD);

	qdf_dp_trace_set_track(skb, QDF_TX);

	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, qdf_nbuf_data_addr(skb),
			sizeof(qdf_nbuf_data(skb)),
			QDF_TX));
	if (!pkt_proto_logged) {
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_TX_PACKET_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID, (uint8_t *)skb->data,
				qdf_nbuf_len(skb), QDF_TX));
		if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE) {
			DPTRACE(qdf_dp_trace(skb,
				QDF_DP_TRACE_HDD_TX_PACKET_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID,
				(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
				(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE),
				QDF_TX));
		}
	}

	if (!hdd_is_tx_allowed(skb, STAId)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Tx not allowed for sta_id: %d"), STAId);
		++adapter->hdd_stats.tx_rx_stats.tx_dropped_ac[ac];
		goto drop_pkt_and_release_skb;
	}

	/*
	 * If a transmit function is not registered, drop packet
	 */
	if (!adapter->tx_fn) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			 "%s: TX function not registered by the data path",
			 __func__);
		++adapter->hdd_stats.tx_rx_stats.tx_dropped_ac[ac];
		goto drop_pkt_and_release_skb;
	}

	if (adapter->tx_fn(adapter->txrx_vdev,
		 (qdf_nbuf_t) skb) != NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Failed to send packet to txrx for staid: %d",
			  __func__, STAId);
		++adapter->hdd_stats.tx_rx_stats.tx_dropped_ac[ac];
		goto drop_pkt_and_release_skb;
	}

	netif_trans_update(dev);

	return NETDEV_TX_OK;

drop_pkt_and_release_skb:
	qdf_net_buf_debug_release_skb(skb);
drop_pkt:

	if (skb) {
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_DROP_PACKET_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, (uint8_t *)skb->data,
			qdf_nbuf_len(skb), QDF_TX));
		if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
			DPTRACE(qdf_dp_trace(skb,
				 QDF_DP_TRACE_DROP_PACKET_RECORD,
				 QDF_TRACE_DEFAULT_PDEV_ID,
				(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
				(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE),
				 QDF_TX));

		kfree_skb(skb);
	}

drop_pkt_accounting:

	++adapter->stats.tx_dropped;
	++adapter->hdd_stats.tx_rx_stats.tx_dropped;
	if (is_arp) {
		++adapter->hdd_stats.hdd_arp_stats.tx_dropped;
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s : ARP packet dropped", __func__);
	}

	return NETDEV_TX_OK;
}

/**
 * hdd_hard_start_xmit() - Wrapper function to protect
 * __hdd_hard_start_xmit from SSR
 * @skb: pointer to OS packet
 * @dev: pointer to net_device structure
 *
 * Function called by OS if any packet needs to transmit.
 *
 * Return: Always returns NETDEV_TX_OK
 */
int hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_hard_start_xmit(skb, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_get_peer_sta_id() - Get the StationID using the Peer Mac address
 * @sta_ctx: pointer to HDD Station Context
 * @pMacAddress: pointer to Peer Mac address
 * @staID: pointer to returned Station Index
 *
 * Return: QDF_STATUS_SUCCESS/QDF_STATUS_E_FAILURE
 */

QDF_STATUS hdd_get_peer_sta_id(struct hdd_station_ctx *sta_ctx,
			       struct qdf_mac_addr *pMacAddress, uint8_t *staId)
{
	uint8_t idx;

	for (idx = 0; idx < MAX_PEERS; idx++) {
		if (!qdf_mem_cmp(&sta_ctx->conn_info.peerMacAddress[idx],
				    pMacAddress, QDF_MAC_ADDR_SIZE)) {
			*staId = sta_ctx->conn_info.staId[idx];
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

/**
 * __hdd_tx_timeout() - TX timeout handler
 * @dev: pointer to network device
 *
 * This function is registered as a netdev ndo_tx_timeout method, and
 * is invoked by the kernel if the driver takes too long to transmit a
 * frame.
 *
 * Return: None
 */
static void __hdd_tx_timeout(struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct netdev_queue *txq;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	u64 diff_jiffies;
	int i = 0;

	TX_TIMEOUT_TRACE(dev, QDF_MODULE_ID_HDD_DATA);
	DPTRACE(qdf_dp_trace(NULL, QDF_DP_TRACE_HDD_TX_TIMEOUT,
				QDF_TRACE_DEFAULT_PDEV_ID,
				NULL, 0, QDF_TX));

	/* Getting here implies we disabled the TX queues for too
	 * long. Queues are disabled either because of disassociation
	 * or low resource scenarios. In case of disassociation it is
	 * ok to ignore this. But if associated, we have do possible
	 * recovery here
	 */

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		txq = netdev_get_tx_queue(dev, i);
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			  "Queue: %d status: %d txq->trans_start: %lu",
			   i, netif_tx_queue_stopped(txq), txq->trans_start);
	}

	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
		  "carrier state: %d", netif_carrier_ok(dev));
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	wlan_hdd_display_netif_queue_history(hdd_ctx,
					     QDF_STATS_VERBOSITY_LEVEL_HIGH);
	cdp_dump_flow_pool_info(cds_get_context(QDF_MODULE_ID_SOC));

	++adapter->hdd_stats.tx_rx_stats.tx_timeout_cnt;
	++adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt;

	diff_jiffies = jiffies -
		       adapter->hdd_stats.tx_rx_stats.jiffies_last_txtimeout;

	if ((adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt > 1) &&
	    (diff_jiffies > (HDD_TX_TIMEOUT * 2))) {
		/*
		 * In case when there is no traffic is running, it may
		 * possible tx time-out may once happen and later system
		 * recovered then continuous tx timeout count has to be
		 * reset as it is gets modified only when traffic is running.
		 * If over a period of time if this count reaches to threshold
		 * then host triggers a false subsystem restart. In genuine
		 * time out case kernel will call the tx time-out back to back
		 * at interval of HDD_TX_TIMEOUT. Here now check if previous
		 * TX TIME out has occurred more than twice of HDD_TX_TIMEOUT
		 * back then host may recovered here from data stall.
		 */
		adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			  "Reset continous tx timeout stat");
	}

	adapter->hdd_stats.tx_rx_stats.jiffies_last_txtimeout = jiffies;

	if (adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt >
	    HDD_TX_STALL_THRESHOLD) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Data stall due to continuous TX timeouts");
		adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;
		if (hdd_ctx->config->enable_data_stall_det)
			cdp_post_data_stall_event(soc,
					  DATA_STALL_LOG_INDICATOR_HOST_DRIVER,
					  DATA_STALL_LOG_HOST_STA_TX_TIMEOUT,
					  0xFF, 0xFF,
					  DATA_STALL_LOG_RECOVERY_TRIGGER_PDR);
	}
}

/**
 * hdd_tx_timeout() - Wrapper function to protect __hdd_tx_timeout from SSR
 * @dev: pointer to net_device structure
 *
 * Function called by OS if there is any timeout during transmission.
 * Since HDD simply enqueues packet and returns control to OS right away,
 * this would never be invoked
 *
 * Return: none
 */
void hdd_tx_timeout(struct net_device *dev)
{
	cds_ssr_protect(__func__);
	__hdd_tx_timeout(dev);
	cds_ssr_unprotect(__func__);
}

/**
 * @hdd_init_tx_rx() - Initialize Tx/RX module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_init_tx_rx(struct hdd_adapter *adapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == adapter) {
		hdd_err("adapter is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * @hdd_deinit_tx_rx() - Deinitialize Tx/RX module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_deinit_tx_rx(struct hdd_adapter *adapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == adapter) {
		hdd_err("adapter is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * hdd_mon_rx_packet_cbk() - Receive callback registered with OL layer.
 * @context: [in] pointer to qdf context
 * @rxBuf:      [in] pointer to rx qdf_nbuf
 *
 * TL will call this to notify the HDD when one or more packets were
 * received for a registered STA.
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered, QDF_STATUS_SUCCESS
 * otherwise
 */
static QDF_STATUS hdd_mon_rx_packet_cbk(void *context, qdf_nbuf_t rxbuf)
{
	struct hdd_adapter *adapter;
	int rxstat;
	struct sk_buff *skb;
	struct sk_buff *skb_next;
	unsigned int cpu_index;

	/* Sanity check on inputs */
	if ((NULL == context) || (NULL == rxbuf)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	adapter = (struct hdd_adapter *)context;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "invalid adapter %pK", adapter);
		return QDF_STATUS_E_FAILURE;
	}

	cpu_index = wlan_hdd_get_cpu();

	/* walk the chain until all are processed */
	skb = (struct sk_buff *) rxbuf;
	while (NULL != skb) {
		skb_next = skb->next;
		skb->dev = adapter->dev;

		++adapter->hdd_stats.tx_rx_stats.rx_packets[cpu_index];
		++adapter->stats.rx_packets;
		adapter->stats.rx_bytes += skb->len;

		/* Remove SKB from internal tracking table before submitting
		 * it to stack
		 */
		qdf_net_buf_debug_release_skb(skb);

		/*
		 * If this is not a last packet on the chain
		 * Just put packet into backlog queue, not scheduling RX sirq
		 */
		if (skb->next) {
			rxstat = netif_rx(skb);
		} else {
			/*
			 * This is the last packet on the chain
			 * Scheduling rx sirq
			 */
			rxstat = netif_rx_ni(skb);
		}

		if (NET_RX_SUCCESS == rxstat)
			++adapter->
				hdd_stats.tx_rx_stats.rx_delivered[cpu_index];
		else
			++adapter->hdd_stats.tx_rx_stats.rx_refused[cpu_index];

		skb = skb_next;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_get_peer_idx() - Get the idx for given address in peer table
 * @sta_ctx: pointer to HDD Station Context
 * @addr: pointer to Peer Mac address
 *
 * Return: index when success else INVALID_PEER_IDX
 */
int hdd_get_peer_idx(struct hdd_station_ctx *sta_ctx,
		     struct qdf_mac_addr *addr)
{
	uint8_t idx;

	for (idx = 0; idx < MAX_PEERS; idx++) {
		if (sta_ctx->conn_info.staId[idx] == HDD_WLAN_INVALID_STA_ID)
			continue;
		if (qdf_mem_cmp(&sta_ctx->conn_info.peerMacAddress[idx],
				addr, sizeof(struct qdf_mac_addr)))
			continue;
		return idx;
	}

	return INVALID_PEER_IDX;
}

/*
 * hdd_is_mcast_replay() - checks if pkt is multicast replay
 * @skb: packet skb
 *
 * Return: true if replayed multicast pkt, false otherwise
 */
static bool hdd_is_mcast_replay(struct sk_buff *skb)
{
	struct ethhdr *eth;

	eth = eth_hdr(skb);
	if (unlikely(skb->pkt_type == PACKET_MULTICAST)) {
		if (unlikely(ether_addr_equal(eth->h_source,
				skb->dev->dev_addr)))
			return true;
	}
	return false;
}

/**
 * hdd_is_arp_local() - check if local or non local arp
 * @skb: pointer to sk_buff
 *
 * Return: true if local arp or false otherwise.
 */
static bool hdd_is_arp_local(struct sk_buff *skb)
{
	struct arphdr *arp;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct in_device *in_dev;
	unsigned char *arp_ptr;
	__be32 tip;

	arp = (struct arphdr *)skb->data;
	if (arp->ar_op == htons(ARPOP_REQUEST)) {
		in_dev = __in_dev_get_rtnl(skb->dev);
		if (in_dev) {
			for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
				ifap = &ifa->ifa_next) {
				if (!strcmp(skb->dev->name, ifa->ifa_label))
					break;
			}
		}

		if (ifa && ifa->ifa_local) {
			arp_ptr = (unsigned char *)(arp + 1);
			arp_ptr += (skb->dev->addr_len + 4 +
					skb->dev->addr_len);
			memcpy(&tip, arp_ptr, 4);
			hdd_debug("ARP packet: local IP: %x dest IP: %x",
				ifa->ifa_local, tip);
			if (ifa->ifa_local == tip)
				return true;
		}
	}

	return false;
}

/**
 * hdd_is_rx_wake_lock_needed() - check if wake lock is needed
 * @skb: pointer to sk_buff
 *
 * RX wake lock is needed for:
 * 1) Unicast data packet OR
 * 2) Local ARP data packet
 *
 * Return: true if wake lock is needed or false otherwise.
 */
static bool hdd_is_rx_wake_lock_needed(struct sk_buff *skb)
{
	if ((skb->pkt_type != PACKET_BROADCAST &&
	     skb->pkt_type != PACKET_MULTICAST) || hdd_is_arp_local(skb))
		return true;

	return false;
}

#ifdef WLAN_FEATURE_TSF_PLUS
static inline void hdd_tsf_timestamp_rx(struct hdd_context *hdd_ctx,
					qdf_nbuf_t netbuf,
					uint64_t target_time)
{
	if (!HDD_TSF_IS_RX_SET(hdd_ctx))
		return;

	hdd_rx_timestamp(netbuf, target_time);
}
#else
static inline void hdd_tsf_timestamp_rx(struct hdd_context *hdd_ctx,
					qdf_nbuf_t netbuf,
					uint64_t target_time)
{
}
#endif

/**
 * hdd_rx_packet_cbk() - Receive packet handler
 * @context: pointer to HDD context
 * @rxBuf: pointer to rx qdf_nbuf
 *
 * Receive callback registered with TL.  TL will call this to notify
 * the HDD when one or more packets were received for a registered
 * STA.
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_rx_packet_cbk(void *context, qdf_nbuf_t rxBuf)
{
	struct hdd_adapter *adapter = NULL;
	struct hdd_context *hdd_ctx = NULL;
	int rxstat;
	struct sk_buff *skb = NULL;
	struct sk_buff *next = NULL;
	struct hdd_station_ctx *sta_ctx = NULL;
	unsigned int cpu_index;
	struct qdf_mac_addr *mac_addr;
	bool wake_lock = false;
	bool proto_pkt_logged = false;
	bool track_arp = false;

	/* Sanity check on inputs */
	if (unlikely((NULL == context) || (NULL == rxBuf))) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	adapter = (struct hdd_adapter *)context;
	if (unlikely(WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Magic cookie(%x) for adapter sanity verification is invalid",
			  adapter->magic);
		return QDF_STATUS_E_FAILURE;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (unlikely(NULL == hdd_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD context is Null", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cpu_index = wlan_hdd_get_cpu();

	next = (struct sk_buff *)rxBuf;

	while (next) {
		skb = next;
		next = skb->next;
		skb->next = NULL;

#ifdef QCA_WIFI_QCA6290 /* Debug code, remove later */
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			 "%s: skb %pK skb->len %d\n", __func__, skb, skb->len);
#endif
		if (QDF_NBUF_CB_PACKET_TYPE_ARP ==
		    QDF_NBUF_CB_GET_PACKET_TYPE(skb)) {
			if (qdf_nbuf_data_is_arp_rsp(skb) &&
				(hdd_ctx->track_arp_ip ==
			     qdf_nbuf_get_arp_src_ip(skb))) {
				++adapter->hdd_stats.hdd_arp_stats.
							rx_arp_rsp_count;
				QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
						QDF_TRACE_LEVEL_INFO,
						"%s: ARP packet received",
						__func__);
				track_arp = true;
			}
		}

		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if ((sta_ctx->conn_info.proxyARPService) &&
			cfg80211_is_gratuitous_arp_unsolicited_na(skb)) {
			uint32_t rx_dropped;

			rx_dropped = ++adapter->hdd_stats.tx_rx_stats.
							rx_dropped[cpu_index];
			/* rate limit error messages to 1/8th */
			if ((rx_dropped & 0x07) == 0)
				QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: Dropping HS 2.0 Gratuitous ARP or Unsolicited NA count=%u",
					  __func__, rx_dropped);
			/* Remove SKB from internal tracking table before submitting
			 * it to stack
			 */
			qdf_nbuf_free(skb);
			continue;
		}

		hdd_event_eapol_log(skb, QDF_RX);
		proto_pkt_logged = qdf_dp_trace_log_pkt(adapter->session_id,
						skb, QDF_RX,
						QDF_TRACE_DEFAULT_PDEV_ID);

		DPTRACE(qdf_dp_trace(skb,
			QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			qdf_nbuf_data_addr(skb),
			sizeof(qdf_nbuf_data(skb)), QDF_RX));

		if (!proto_pkt_logged) {
			DPTRACE(qdf_dp_trace(skb,
				QDF_DP_TRACE_HDD_RX_PACKET_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID,
				(uint8_t *)skb->data, qdf_nbuf_len(skb),
				QDF_RX));
			if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
				DPTRACE(qdf_dp_trace(skb,
					QDF_DP_TRACE_HDD_RX_PACKET_RECORD,
					QDF_TRACE_DEFAULT_PDEV_ID,
					(uint8_t *)
					&skb->data[QDF_DP_TRACE_RECORD_SIZE],
					(qdf_nbuf_len(skb) -
						QDF_DP_TRACE_RECORD_SIZE),
					QDF_RX));
		}
		mac_addr = (struct qdf_mac_addr *)(skb->data+QDF_MAC_ADDR_SIZE);

		ucfg_tdls_update_rx_pkt_cnt(adapter->hdd_vdev, mac_addr);

		skb->dev = adapter->dev;
		skb->protocol = eth_type_trans(skb, skb->dev);
		++adapter->hdd_stats.tx_rx_stats.rx_packets[cpu_index];
		++adapter->stats.rx_packets;
		adapter->stats.rx_bytes += skb->len;

		/* Check & drop replayed mcast packets (for IPV6) */
		if (hdd_ctx->config->multicast_replay_filter &&
				hdd_is_mcast_replay(skb)) {
			++adapter->hdd_stats.tx_rx_stats.rx_dropped[cpu_index];
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
				"%s: Dropping multicast replay pkt", __func__);
			qdf_nbuf_free(skb);
			continue;
		}

		/* hold configurable wakelock for unicast traffic */
		if (hdd_ctx->config->rx_wakelock_timeout &&
				sta_ctx->conn_info.uIsAuthenticated)
			wake_lock = hdd_is_rx_wake_lock_needed(skb);

		if (wake_lock) {
			cds_host_diag_log_work(&hdd_ctx->rx_wake_lock,
						   hdd_ctx->config->rx_wakelock_timeout,
						   WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);
			qdf_wake_lock_timeout_acquire(&hdd_ctx->rx_wake_lock,
							  hdd_ctx->config->
								  rx_wakelock_timeout);
		}

		/* Remove SKB from internal tracking table before submitting
		 * it to stack
		 */
		qdf_net_buf_debug_release_skb(skb);

		hdd_tsf_timestamp_rx(hdd_ctx, skb, ktime_to_us(skb->tstamp));

		if (HDD_LRO_NO_RX ==
			 hdd_lro_rx(hdd_ctx, adapter, skb)) {
			if (hdd_napi_enabled(HDD_NAPI_ANY) &&
				!hdd_ctx->enable_rxthread &&
				!QDF_NBUF_CB_RX_PEER_CACHED_FRM(skb))
				rxstat = netif_receive_skb(skb);
			else
				rxstat = netif_rx_ni(skb);

			if (NET_RX_SUCCESS == rxstat) {
				++adapter->hdd_stats.tx_rx_stats.
					 rx_delivered[cpu_index];
				if (track_arp)
					++adapter->hdd_stats.hdd_arp_stats.
						rx_delivered;
			} else {
				++adapter->hdd_stats.tx_rx_stats.
					 rx_refused[cpu_index];
				if (track_arp)
					++adapter->hdd_stats.hdd_arp_stats.
						rx_refused;
			}
		} else {
			++adapter->hdd_stats.tx_rx_stats.
						rx_delivered[cpu_index];
			if (track_arp)
				++adapter->hdd_stats.hdd_arp_stats.
						rx_delivered;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_reason_type_to_string() - return string conversion of reason type
 * @reason: reason type
 *
 * This utility function helps log string conversion of reason type.
 *
 * Return: string conversion of device mode, if match found;
 *        "Unknown" otherwise.
 */
const char *hdd_reason_type_to_string(enum netif_reason_type reason)
{
	switch (reason) {
	CASE_RETURN_STRING(WLAN_CONTROL_PATH);
	CASE_RETURN_STRING(WLAN_DATA_FLOW_CONTROL);
	CASE_RETURN_STRING(WLAN_FW_PAUSE);
	CASE_RETURN_STRING(WLAN_TX_ABORT);
	CASE_RETURN_STRING(WLAN_VDEV_STOP);
	CASE_RETURN_STRING(WLAN_PEER_UNAUTHORISED);
	CASE_RETURN_STRING(WLAN_THERMAL_MITIGATION);
	CASE_RETURN_STRING(WLAN_DATA_FLOW_CONTROL_PRIORITY);
	default:
		return "Invalid";
	}
}

/**
 * hdd_action_type_to_string() - return string conversion of action type
 * @action: action type
 *
 * This utility function helps log string conversion of action_type.
 *
 * Return: string conversion of device mode, if match found;
 *        "Unknown" otherwise.
 */
const char *hdd_action_type_to_string(enum netif_action_type action)
{

	switch (action) {
	CASE_RETURN_STRING(WLAN_STOP_ALL_NETIF_QUEUE);
	CASE_RETURN_STRING(WLAN_START_ALL_NETIF_QUEUE);
	CASE_RETURN_STRING(WLAN_WAKE_ALL_NETIF_QUEUE);
	CASE_RETURN_STRING(WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER);
	CASE_RETURN_STRING(WLAN_START_ALL_NETIF_QUEUE_N_CARRIER);
	CASE_RETURN_STRING(WLAN_NETIF_TX_DISABLE);
	CASE_RETURN_STRING(WLAN_NETIF_TX_DISABLE_N_CARRIER);
	CASE_RETURN_STRING(WLAN_NETIF_CARRIER_ON);
	CASE_RETURN_STRING(WLAN_NETIF_CARRIER_OFF);
	CASE_RETURN_STRING(WLAN_NETIF_PRIORITY_QUEUE_ON);
	CASE_RETURN_STRING(WLAN_NETIF_PRIORITY_QUEUE_OFF);
	CASE_RETURN_STRING(WLAN_WAKE_NON_PRIORITY_QUEUE);
	CASE_RETURN_STRING(WLAN_STOP_NON_PRIORITY_QUEUE);
	default:
		return "Invalid";
	}
}

/**
 * wlan_hdd_update_queue_oper_stats - update queue operation statistics
 * @adapter: adapter handle
 * @action: action type
 * @reason: reason type
 */
static void wlan_hdd_update_queue_oper_stats(struct hdd_adapter *adapter,
	enum netif_action_type action, enum netif_reason_type reason)
{
	switch (action) {
	case WLAN_STOP_ALL_NETIF_QUEUE:
	case WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER:
	case WLAN_NETIF_PRIORITY_QUEUE_OFF:
	case WLAN_STOP_NON_PRIORITY_QUEUE:
		adapter->queue_oper_stats[reason].pause_count++;
		break;
	case WLAN_START_ALL_NETIF_QUEUE:
	case WLAN_WAKE_ALL_NETIF_QUEUE:
	case WLAN_START_ALL_NETIF_QUEUE_N_CARRIER:
	case WLAN_NETIF_PRIORITY_QUEUE_ON:
	case WLAN_WAKE_NON_PRIORITY_QUEUE:
		adapter->queue_oper_stats[reason].unpause_count++;
		break;
	default:
		break;
	}
}

/**
 * hdd_netdev_queue_is_locked()
 * @txq: net device tx queue
 *
 * For SMP system, always return false and we could safely rely on
 * __netif_tx_trylock().
 *
 * Return: true locked; false not locked
 */
#ifdef QCA_CONFIG_SMP
static inline bool hdd_netdev_queue_is_locked(struct netdev_queue *txq)
{
	return false;
}
#else
static inline bool hdd_netdev_queue_is_locked(struct netdev_queue *txq)
{
	return txq->xmit_lock_owner != -1;
}
#endif

/**
 * wlan_hdd_update_txq_timestamp() - update txq timestamp
 * @dev: net device
 *
 * Return: none
 */
static void wlan_hdd_update_txq_timestamp(struct net_device *dev)
{
	struct netdev_queue *txq;
	int i;

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		txq = netdev_get_tx_queue(dev, i);

		/*
		 * On UP system, kernel will trigger watchdog bite if spinlock
		 * recursion is detected. Unfortunately recursion is possible
		 * when it is called in dev_queue_xmit() context, where stack
		 * grabs the lock before calling driver's ndo_start_xmit
		 * callback.
		 */
		if (!hdd_netdev_queue_is_locked(txq)) {
			if (__netif_tx_trylock(txq)) {
				txq_trans_update(txq);
				__netif_tx_unlock(txq);
			}
		}
	}
}

/**
 * wlan_hdd_update_unpause_time() - update unpause time
 * @adapter: adapter handle
 *
 * Return: none
 */
static void wlan_hdd_update_unpause_time(struct hdd_adapter *adapter)
{
	qdf_time_t curr_time = qdf_system_ticks();

	adapter->total_unpause_time += curr_time - adapter->last_time;
	adapter->last_time = curr_time;
}

/**
 * wlan_hdd_update_pause_time() - update pause time
 * @adapter: adapter handle
 *
 * Return: none
 */
static void wlan_hdd_update_pause_time(struct hdd_adapter *adapter,
	 uint32_t temp_map)
{
	qdf_time_t curr_time = qdf_system_ticks();
	uint8_t i;
	qdf_time_t pause_time;

	pause_time = curr_time - adapter->last_time;
	adapter->total_pause_time += pause_time;
	adapter->last_time = curr_time;

	for (i = 0; i < WLAN_REASON_TYPE_MAX; i++) {
		if (temp_map & (1 << i)) {
			adapter->queue_oper_stats[i].total_pause_time +=
								 pause_time;
			break;
		}
	}

}

/**
 * wlan_hdd_stop_non_priority_queue() - stop non prority queues
 * @adapter: adapter handle
 *
 * Return: None
 */
static inline void wlan_hdd_stop_non_priority_queue(struct hdd_adapter *adapter)
{
	netif_stop_subqueue(adapter->dev, HDD_LINUX_AC_VO);
	netif_stop_subqueue(adapter->dev, HDD_LINUX_AC_VI);
	netif_stop_subqueue(adapter->dev, HDD_LINUX_AC_BE);
	netif_stop_subqueue(adapter->dev, HDD_LINUX_AC_BK);
}

/**
 * wlan_hdd_wake_non_priority_queue() - wake non prority queues
 * @adapter: adapter handle
 *
 * Return: None
 */
static inline void wlan_hdd_wake_non_priority_queue(struct hdd_adapter *adapter)
{
	netif_wake_subqueue(adapter->dev, HDD_LINUX_AC_VO);
	netif_wake_subqueue(adapter->dev, HDD_LINUX_AC_VI);
	netif_wake_subqueue(adapter->dev, HDD_LINUX_AC_BE);
	netif_wake_subqueue(adapter->dev, HDD_LINUX_AC_BK);
}

/**
 * wlan_hdd_netif_queue_control() - Use for netif_queue related actions
 * @adapter: adapter handle
 * @action: action type
 * @reason: reason type
 *
 * This is single function which is used for netif_queue related
 * actions like start/stop of network queues and on/off carrier
 * option.
 *
 * Return: None
 */
void wlan_hdd_netif_queue_control(struct hdd_adapter *adapter,
	enum netif_action_type action, enum netif_reason_type reason)
{
	uint32_t temp_map;

	if ((!adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) ||
		 (!adapter->dev)) {
		hdd_err("adapter is invalid");
		return;
	}

	switch (action) {

	case WLAN_NETIF_CARRIER_ON:
		netif_carrier_on(adapter->dev);
		break;

	case WLAN_NETIF_CARRIER_OFF:
		netif_carrier_off(adapter->dev);
		break;

	case WLAN_STOP_ALL_NETIF_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		if (!adapter->pause_map) {
			netif_tx_stop_all_queues(adapter->dev);
			wlan_hdd_update_txq_timestamp(adapter->dev);
			wlan_hdd_update_unpause_time(adapter);
		}
		adapter->pause_map |= (1 << reason);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_STOP_NON_PRIORITY_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		if (!adapter->pause_map) {
			wlan_hdd_stop_non_priority_queue(adapter);
			wlan_hdd_update_txq_timestamp(adapter->dev);
			wlan_hdd_update_unpause_time(adapter);
		}
		adapter->pause_map |= (1 << reason);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_NETIF_PRIORITY_QUEUE_ON:
		spin_lock_bh(&adapter->pause_map_lock);
		temp_map = adapter->pause_map;
		adapter->pause_map &= ~(1 << reason);
		netif_wake_subqueue(adapter->dev, HDD_LINUX_AC_HI_PRIO);
		wlan_hdd_update_pause_time(adapter, temp_map);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_NETIF_PRIORITY_QUEUE_OFF:
		spin_lock_bh(&adapter->pause_map_lock);
		netif_stop_subqueue(adapter->dev, HDD_LINUX_AC_HI_PRIO);
		wlan_hdd_update_txq_timestamp(adapter->dev);
		wlan_hdd_update_unpause_time(adapter);
		adapter->pause_map |= (1 << reason);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_START_ALL_NETIF_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		temp_map = adapter->pause_map;
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_start_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter, temp_map);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_WAKE_ALL_NETIF_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		temp_map = adapter->pause_map;
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_wake_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter, temp_map);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_WAKE_NON_PRIORITY_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		temp_map = adapter->pause_map;
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			wlan_hdd_wake_non_priority_queue(adapter);
			wlan_hdd_update_pause_time(adapter, temp_map);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER:
		spin_lock_bh(&adapter->pause_map_lock);
		if (!adapter->pause_map) {
			netif_tx_stop_all_queues(adapter->dev);
			wlan_hdd_update_txq_timestamp(adapter->dev);
			wlan_hdd_update_unpause_time(adapter);
		}
		adapter->pause_map |= (1 << reason);
		netif_carrier_off(adapter->dev);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_START_ALL_NETIF_QUEUE_N_CARRIER:
		spin_lock_bh(&adapter->pause_map_lock);
		netif_carrier_on(adapter->dev);
		temp_map = adapter->pause_map;
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_start_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter, temp_map);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	default:
		hdd_err("unsupported action %d", action);
	}

	spin_lock_bh(&adapter->pause_map_lock);
	if (adapter->pause_map & (1 << WLAN_PEER_UNAUTHORISED))
		wlan_hdd_process_peer_unauthorised_pause(adapter);
	spin_unlock_bh(&adapter->pause_map_lock);

	wlan_hdd_update_queue_oper_stats(adapter, action, reason);

	adapter->queue_oper_history[adapter->history_index].time =
							qdf_system_ticks();
	adapter->queue_oper_history[adapter->history_index].netif_action =
									action;
	adapter->queue_oper_history[adapter->history_index].netif_reason =
									reason;
	adapter->queue_oper_history[adapter->history_index].pause_map =
							adapter->pause_map;
	if (++adapter->history_index == WLAN_HDD_MAX_HISTORY_ENTRY)
		adapter->history_index = 0;
}

/**
 * hdd_set_mon_rx_cb() - Set Monitor mode Rx callback
 * @dev:        Pointer to net_device structure
 *
 * Return: 0 for success; non-zero for failure
 */
int hdd_set_mon_rx_cb(struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx =  WLAN_HDD_GET_CTX(adapter);
	int ret;
	QDF_STATUS qdf_status;
	struct ol_txrx_desc_type sta_desc = {0};
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_mon_rx_packet_cbk;
	hdd_monitor_set_rx_monitor_cb(&txrx_ops, hdd_rx_monitor_callback);
	cdp_vdev_register(soc,
		(struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
		(struct cdp_pdev *)pdev, adapter->session_id),
		adapter, &txrx_ops);
	/* peer is created wma_vdev_attach->wma_create_peer */
	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &sta_desc);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("cdp_peer_register() failed to register. Status= %d [0x%08X]",
			qdf_status, qdf_status);
		goto exit;
	}

	qdf_status = sme_create_mon_session(hdd_ctx->hHal,
				     adapter->mac_addr.bytes);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("sme_create_mon_session() failed to register. Status= %d [0x%08X]",
			qdf_status, qdf_status);
	}
exit:
	ret = qdf_status_to_os_return(qdf_status);
	return ret;
}

/**
 * hdd_send_rps_ind() - send rps indication to daemon
 * @adapter: adapter context
 *
 * If RPS feature enabled by INI, send RPS enable indication to daemon
 * Indication contents is the name of interface to find correct sysfs node
 * Should send all available interfaces
 *
 * Return: none
 */
void hdd_send_rps_ind(struct hdd_adapter *adapter)
{
	int i;
	uint8_t cpu_map_list_len = 0;
	struct hdd_context *hdd_ctxt = NULL;
	struct wlan_rps_data rps_data;
	struct cds_config_info *cds_cfg;

	cds_cfg = cds_get_ini_config();

	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	if (!cds_cfg) {
		hdd_err("cds_cfg is NULL");
		return;
	}

	hdd_ctxt = WLAN_HDD_GET_CTX(adapter);
	rps_data.num_queues = NUM_TX_QUEUES;

	hdd_info("cpu_map_list '%s'", hdd_ctxt->config->cpu_map_list);

	/* in case no cpu map list is provided, simply return */
	if (!strlen(hdd_ctxt->config->cpu_map_list)) {
		hdd_err("no cpu map list found");
		goto err;
	}

	if (QDF_STATUS_SUCCESS !=
		hdd_hex_string_to_u16_array(hdd_ctxt->config->cpu_map_list,
				rps_data.cpu_map_list,
				&cpu_map_list_len,
				WLAN_SVC_IFACE_NUM_QUEUES)) {
		hdd_err("invalid cpu map list");
		goto err;
	}

	rps_data.num_queues =
		(cpu_map_list_len < rps_data.num_queues) ?
				cpu_map_list_len : rps_data.num_queues;

	for (i = 0; i < rps_data.num_queues; i++) {
		hdd_info("cpu_map_list[%d] = 0x%x",
			i, rps_data.cpu_map_list[i]);
	}

	strlcpy(rps_data.ifname, adapter->dev->name,
			sizeof(rps_data.ifname));
	wlan_hdd_send_svc_nlink_msg(hdd_ctxt->radio_index,
				WLAN_SVC_RPS_ENABLE_IND,
				&rps_data, sizeof(rps_data));

	cds_cfg->rps_enabled = true;

	return;

err:
	hdd_err("Wrong RPS configuration. enabling rx_thread");
	cds_cfg->rps_enabled = false;
}

/**
 * hdd_send_rps_disable_ind() - send rps disable indication to daemon
 * @adapter: adapter context
 *
 * Return: none
 */
void hdd_send_rps_disable_ind(struct hdd_adapter *adapter)
{
	uint8_t cpu_map_list_len = 0;
	struct hdd_context *hdd_ctxt = NULL;
	struct wlan_rps_data rps_data;
	struct cds_config_info *cds_cfg;

	cds_cfg = cds_get_ini_config();

	if (!adapter) {
		hdd_err("adapter is NULL");
		return;
	}

	if (!cds_cfg) {
		hdd_err("cds_cfg is NULL");
		return;
	}

	hdd_ctxt = WLAN_HDD_GET_CTX(adapter);
	rps_data.num_queues = NUM_TX_QUEUES;

	hdd_info("Set cpu_map_list 0");

	qdf_mem_zero(&rps_data.cpu_map_list, sizeof(rps_data.cpu_map_list));
	cpu_map_list_len = 0;
	rps_data.num_queues =
		(cpu_map_list_len < rps_data.num_queues) ?
				cpu_map_list_len : rps_data.num_queues;

	strlcpy(rps_data.ifname, adapter->dev->name, sizeof(rps_data.ifname));
	wlan_hdd_send_svc_nlink_msg(hdd_ctxt->radio_index,
				    WLAN_SVC_RPS_ENABLE_IND,
				    &rps_data, sizeof(rps_data));

	cds_cfg->rps_enabled = false;
}

void hdd_tx_queue_cb(void *context, uint32_t vdev_id,
		     enum netif_action_type action,
		     enum netif_reason_type reason)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *)context;
	struct hdd_adapter *adapter = NULL;

	/*
	 * Validating the context is not required here.
	 * if there is a driver unload/SSR in progress happening in a
	 * different context and it has been scheduled to run and
	 * driver got a firmware event of sta kick out, then it is
	 * good to disable the Tx Queue to stop the influx of traffic.
	 */
	if (hdd_ctx == NULL) {
		hdd_err("Invalid context passed");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (adapter == NULL) {
		hdd_err("vdev_id %d does not exist with host", vdev_id);
		return;
	}
	hdd_debug("Tx Queue action %d on vdev %d", action, vdev_id);

	wlan_hdd_netif_queue_control(adapter, action, reason);
}

#ifdef MSM_PLATFORM
/**
 * hdd_reset_tcp_delack() - Reset tcp delack value to default
 * @hdd_ctx: Handle to hdd context
 *
 * Function used to reset TCP delack value to its default value
 *
 * Return: None
 */
void hdd_reset_tcp_delack(struct hdd_context *hdd_ctx)
{
	enum pld_bus_width_type next_level = WLAN_SVC_TP_LOW;
	struct wlan_rx_tp_data rx_tp_data = {0};

	rx_tp_data.rx_tp_flags |= TCP_DEL_ACK_IND;
	rx_tp_data.level = next_level;
	hdd_ctx->rx_high_ind_cnt = 0;
	wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index, WLAN_SVC_WLAN_TP_IND,
				    &rx_tp_data, sizeof(rx_tp_data));
}
#endif /* MSM_PLATFORM */
