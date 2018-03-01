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

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

/* Include files */
#include <linux/semaphore.h>
#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <qdf_types.h>
#include <ani_global.h>
#include <qdf_types.h>
#include <net/ieee80211_radiotap.h>
#include <cds_sched.h>
#include <wlan_hdd_napi.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_peer_ops.h>
#include <cds_utils.h>
#include <cdp_txrx_flow_ctrl_v2.h>
#include <cdp_txrx_misc.h>
#include <wlan_hdd_object_manager.h>
#include "wlan_p2p_ucfg_api.h"
#include <wlan_hdd_ipa.h>
#include <wlan_hdd_regulatory.h>

/* Preprocessor definitions and constants */
#undef QCA_HDD_SAP_DUMP_SK_BUFF

/* Type declarations */

/* Function definitions and documenation */
#ifdef QCA_HDD_SAP_DUMP_SK_BUFF
/**
 * hdd_softap_dump_sk_buff() - Dump an skb
 * @skb: skb to dump
 *
 * Return: None
 */
static void hdd_softap_dump_sk_buff(struct sk_buff *skb)
{
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: head = %pK ", __func__, skb->head);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO,
		  "%s: tail = %pK ", __func__, skb->tail);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: end = %pK ", __func__, skb->end);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: len = %d ", __func__, skb->len);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: data_len = %d ", __func__, skb->data_len);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: mac_len = %d", __func__, skb->mac_len);

	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", skb->data[0],
		  skb->data[1], skb->data[2], skb->data[3], skb->data[4],
		  skb->data[5], skb->data[6], skb->data[7]);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", skb->data[8],
		  skb->data[9], skb->data[10], skb->data[11], skb->data[12],
		  skb->data[13], skb->data[14], skb->data[15]);
}
#else
static void hdd_softap_dump_sk_buff(struct sk_buff *skb)
{
}
#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
/**
 * hdd_softap_tx_resume_timer_expired_handler() - TX Q resume timer handler
 * @adapter_context: pointer to vdev adapter
 *
 * TX Q resume timer handler for SAP and P2P GO interface.  If Blocked
 * OS Q is not resumed during timeout period, to prevent permanent
 * stall, resume OS Q forcefully for SAP and P2P GO interface.
 *
 * Return: None
 */
void hdd_softap_tx_resume_timer_expired_handler(void *adapter_context)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) adapter_context;

	if (!adapter) {
		hdd_err("NULL adapter");
		return;
	}

	hdd_debug("Enabling queues");
	wlan_hdd_netif_queue_control(adapter, WLAN_WAKE_ALL_NETIF_QUEUE,
				     WLAN_CONTROL_PATH);
}

#if defined(CONFIG_PER_VDEV_TX_DESC_POOL)

/**
 * hdd_softap_tx_resume_false() - Resume OS TX Q false leads to queue disabling
 * @adapter: pointer to hdd adapter
 * @tx_resume: TX Q resume trigger
 *
 *
 * Return: None
 */
static void
hdd_softap_tx_resume_false(struct hdd_adapter *adapter, bool tx_resume)
{
	if (true == tx_resume)
		return;

	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
				     WLAN_DATA_FLOW_CONTROL);

	if (QDF_TIMER_STATE_STOPPED ==
			qdf_mc_timer_get_current_state(&adapter->
						       tx_flow_control_timer)) {
		QDF_STATUS status;

		status = qdf_mc_timer_start(&adapter->tx_flow_control_timer,
				WLAN_SAP_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("Failed to start tx_flow_control_timer");
		else
			adapter->hdd_stats.tx_rx_stats.txflow_timer_cnt++;
	}
}
#else

static inline void
hdd_softap_tx_resume_false(struct hdd_adapter *adapter, bool tx_resume)
{
}
#endif

/**
 * hdd_softap_tx_resume_cb() - Resume OS TX Q.
 * @adapter_context: pointer to vdev apdapter
 * @tx_resume: TX Q resume trigger
 *
 * Q was stopped due to WLAN TX path low resource condition
 *
 * Return: None
 */
void hdd_softap_tx_resume_cb(void *adapter_context, bool tx_resume)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) adapter_context;

	if (!adapter) {
		hdd_err("NULL adapter");
		return;
	}

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
	hdd_softap_tx_resume_false(adapter, tx_resume);
}

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
 * __hdd_softap_hard_start_xmit() - Transmit a frame
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
static int __hdd_softap_hard_start_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
	sme_ac_enum_type ac = SME_AC_BE;
	struct hdd_adapter *adapter = (struct hdd_adapter *) netdev_priv(dev);
	struct hdd_ap_ctx *ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
	struct qdf_mac_addr *pDestMacAddress;
	uint8_t STAId;
	uint32_t num_seg;

	++adapter->hdd_stats.tx_rx_stats.tx_called;
	adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;

	/* Prevent this function from being called during SSR since TL
	 * context may not be reinitialized at this time which may
	 * lead to a crash.
	 */
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state() ||
	    cds_is_load_or_unload_in_progress()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Recovery/(Un)load in Progress. Ignore!!!",
			  __func__);
		goto drop_pkt;
	}

	/*
	 * If the device is operating on a DFS Channel
	 * then check if SAP is in CAC WAIT state and
	 * drop the packets. In CAC WAIT state device
	 * is expected not to transmit any frames.
	 * SAP starts Tx only after the BSS START is
	 * done.
	 */
	if (ap_ctx->dfs_cac_block_tx)
		goto drop_pkt;

	/*
	 * If a transmit function is not registered, drop packet
	 */
	if (!adapter->tx_fn) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			 "%s: TX function not registered by the data path",
			 __func__);
		goto drop_pkt;
	}

	wlan_hdd_classify_pkt(skb);

	pDestMacAddress = (struct qdf_mac_addr *) skb->data;

	if (QDF_NBUF_CB_GET_IS_BCAST(skb) ||
	    QDF_NBUF_CB_GET_IS_MCAST(skb)) {
		/* The BC/MC station ID is assigned during BSS
		 * starting phase.  SAP will return the station ID
		 * used for BC/MC traffic.
		 */
		STAId = ap_ctx->broadcast_sta_id;
	} else {
		if (QDF_STATUS_SUCCESS !=
			 hdd_softap_get_sta_id(adapter,
				 pDestMacAddress, &STAId)) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Failed to find right station", __func__);
			goto drop_pkt;
		}

		if (STAId >= WLAN_MAX_STA_COUNT) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Failed to find right station", __func__);
			goto drop_pkt;
		} else if (false == adapter->sta_info[STAId].in_use) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: STA %d is unregistered", __func__,
				  STAId);
			goto drop_pkt;
		} else if (true == adapter->sta_info[STAId].
							is_deauth_in_progress) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: STA %d deauth in progress", __func__,
				  STAId);
			goto drop_pkt;
		}

		if ((OL_TXRX_PEER_STATE_CONN !=
		     adapter->sta_info[STAId].peer_state)
		    && (OL_TXRX_PEER_STATE_AUTH !=
			adapter->sta_info[STAId].peer_state)) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Station not connected yet", __func__);
			goto drop_pkt;
		} else if (OL_TXRX_PEER_STATE_CONN ==
			   adapter->sta_info[STAId].peer_state) {
			if (ntohs(skb->protocol) != HDD_ETHERTYPE_802_1_X) {
				QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
					  QDF_TRACE_LEVEL_INFO_HIGH,
					  "%s: NON-EAPOL packet in non-Authenticated state",
					  __func__);
				goto drop_pkt;
			}
		}
	}

	hdd_get_tx_resource(adapter, STAId,
			WLAN_SAP_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

	/* Get TL AC corresponding to Qdisc queue index/AC. */
	ac = hdd_qdisc_ac_to_tl_ac[skb->queue_mapping];
	++adapter->hdd_stats.tx_rx_stats.tx_classified_ac[ac];

#if defined(IPA_OFFLOAD)
	if (!qdf_nbuf_ipa_owned_get(skb)) {
#endif

		skb = hdd_skb_orphan(adapter, skb);
		if (!skb)
			goto drop_pkt_accounting;

#if defined(IPA_OFFLOAD)
	} else {
		/*
		 * Clear the IPA ownership after check it to avoid ipa_free_skb
		 * is called when Tx completed for intra-BSS Tx packets
		 */
		qdf_nbuf_ipa_owned_clear(skb);
	}
#endif

	/*
	 * Add SKB to internal tracking table before further processing
	 * in WLAN driver.
	 */
	qdf_net_buf_debug_acquire_skb(skb, __FILE__, __LINE__);

	adapter->stats.tx_bytes += skb->len;
	adapter->sta_info[STAId].tx_bytes += skb->len;

	if (qdf_nbuf_is_tso(skb)) {
		num_seg = qdf_nbuf_get_tso_num_seg(skb);
		adapter->stats.tx_packets += num_seg;
		adapter->sta_info[STAId].tx_packets += num_seg;
	} else {
		++adapter->stats.tx_packets;
		adapter->sta_info[STAId].tx_packets++;
	}
	adapter->sta_info[STAId].last_tx_rx_ts = qdf_system_ticks();

	hdd_event_eapol_log(skb, QDF_TX);
	qdf_dp_trace_log_pkt(adapter->session_id, skb, QDF_TX,
			QDF_TRACE_DEFAULT_PDEV_ID);
	QDF_NBUF_CB_TX_PACKET_TRACK(skb) = QDF_NBUF_TX_PKT_DATA_TRACK;
	QDF_NBUF_UPDATE_TX_PKT_COUNT(skb, QDF_NBUF_TX_PKT_HDD);

	qdf_dp_trace_set_track(skb, QDF_TX);
	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, qdf_nbuf_data_addr(skb),
			sizeof(qdf_nbuf_data(skb)),
			QDF_TX));
	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_TX_PACKET_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, (uint8_t *)skb->data,
			qdf_nbuf_len(skb), QDF_TX));
	if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_TX_PACKET_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
			(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE), QDF_TX));

	if (adapter->tx_fn(adapter->txrx_vdev,
		 (qdf_nbuf_t) skb) != NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Failed to send packet to txrx for staid:%d",
			  __func__, STAId);
		++adapter->hdd_stats.tx_rx_stats.tx_dropped_ac[ac];
		goto drop_pkt_and_release_skb;
	}
	netif_trans_update(dev);

	return NETDEV_TX_OK;

drop_pkt_and_release_skb:
	qdf_net_buf_debug_release_skb(skb);
drop_pkt:

	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_DROP_PACKET_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID, (uint8_t *)skb->data,
			qdf_nbuf_len(skb), QDF_TX));
	if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_DROP_PACKET_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
			(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE), QDF_TX));
	kfree_skb(skb);

drop_pkt_accounting:
	++adapter->stats.tx_dropped;
	++adapter->hdd_stats.tx_rx_stats.tx_dropped;

	return NETDEV_TX_OK;
}

/**
 * hdd_softap_hard_start_xmit() - Wrapper function to protect
 * __hdd_softap_hard_start_xmit from SSR
 * @skb: pointer to OS packet
 * @dev: pointer to net_device structure
 *
 * Function called by OS if any packet needs to transmit.
 *
 * Return: Always returns NETDEV_TX_OK
 */
int hdd_softap_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_softap_hard_start_xmit(skb, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_softap_tx_timeout() - TX timeout handler
 * @dev: pointer to network device
 *
 * This function is registered as a netdev ndo_tx_timeout method, and
 * is invoked by the kernel if the driver takes too long to transmit a
 * frame.
 *
 * Return: None
 */
static void __hdd_softap_tx_timeout(struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct netdev_queue *txq;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	int i;

	DPTRACE(qdf_dp_trace(NULL, QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT,
			QDF_TRACE_DEFAULT_PDEV_ID,
			NULL, 0, QDF_TX));
	/* Getting here implies we disabled the TX queues for too
	 * long. Queues are disabled either because of disassociation
	 * or low resource scenarios. In case of disassociation it is
	 * ok to ignore this. But if associated, we have do possible
	 * recovery here
	 */
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
			 "%s: Recovery in Progress. Ignore!!!", __func__);
		return;
	}

	TX_TIMEOUT_TRACE(dev, QDF_MODULE_ID_HDD_SAP_DATA);

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		txq = netdev_get_tx_queue(dev, i);
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
			  QDF_TRACE_LEVEL_DEBUG,
			  "Queue: %d status: %d txq->trans_start: %lu",
			  i, netif_tx_queue_stopped(txq), txq->trans_start);
	}

	wlan_hdd_display_netif_queue_history(hdd_ctx,
					     QDF_STATS_VERBOSITY_LEVEL_HIGH);
	cdp_dump_flow_pool_info(cds_get_context(QDF_MODULE_ID_SOC));
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			"carrier state: %d", netif_carrier_ok(dev));

	++adapter->hdd_stats.tx_rx_stats.tx_timeout_cnt;
	++adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt;

	if (adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt >
	    HDD_TX_STALL_THRESHOLD) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Detected data stall due to continuous TX timeouts");
		adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;
		if (hdd_ctx->config->enable_data_stall_det)
			cdp_post_data_stall_event(soc,
					  DATA_STALL_LOG_INDICATOR_HOST_DRIVER,
					  DATA_STALL_LOG_HOST_SOFTAP_TX_TIMEOUT,
					  0xFF, 0xFF,
					  DATA_STALL_LOG_RECOVERY_TRIGGER_PDR);
	}
}

/**
 * hdd_softap_tx_timeout() - SSR wrapper for __hdd_softap_tx_timeout
 * @dev: pointer to net_device
 *
 * Return: none
 */
void hdd_softap_tx_timeout(struct net_device *dev)
{
	cds_ssr_protect(__func__);
	__hdd_softap_tx_timeout(dev);
	cds_ssr_unprotect(__func__);
}

/**
 * @hdd_softap_init_tx_rx() - Initialize Tx/RX module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_init_tx_rx(struct hdd_adapter *adapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	uint8_t STAId = 0;

	qdf_mem_zero(&adapter->stats, sizeof(struct net_device_stats));

	spin_lock_init(&adapter->sta_info_lock);

	for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++) {
		qdf_mem_zero(&adapter->sta_info[STAId],
			     sizeof(struct hdd_station_info));
	}

	return status;
}

/**
 * @hdd_softap_deinit_tx_rx() - Deinitialize Tx/RX module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx(struct hdd_adapter *adapter)
{
	if (adapter == NULL) {
		hdd_err("Called with adapter = NULL.");
		return QDF_STATUS_E_FAILURE;
	}

	adapter->txrx_vdev = NULL;
	adapter->tx_fn = NULL;
	hdd_info("Deregistering TX function hook !");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_softap_init_tx_rx_sta(struct hdd_adapter *adapter,
				     uint8_t sta_id,
				     struct qdf_mac_addr *sta_mac)
{
	spin_lock_bh(&adapter->sta_info_lock);
	if (adapter->sta_info[sta_id].in_use) {
		spin_unlock_bh(&adapter->sta_info_lock);
		hdd_err("Reinit of in use station %d", sta_id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&adapter->sta_info[sta_id],
		     sizeof(struct hdd_station_info));

	adapter->sta_info[sta_id].in_use = true;
	adapter->sta_info[sta_id].is_deauth_in_progress = false;
	qdf_copy_macaddr(&adapter->sta_info[sta_id].sta_mac, sta_mac);

	spin_unlock_bh(&adapter->sta_info_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_softap_deinit_tx_rx_sta(struct hdd_adapter *adapter,
				       uint8_t sta_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_hostapd_state *hostapd_state;

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	spin_lock_bh(&adapter->sta_info_lock);

	if (false == adapter->sta_info[sta_id].in_use) {
		spin_unlock_bh(&adapter->sta_info_lock);
		hdd_err("Deinit station not inited %d", sta_id);
		return QDF_STATUS_E_FAILURE;
	}

	adapter->sta_info[sta_id].in_use = false;
	adapter->sta_info[sta_id].is_deauth_in_progress = false;

	spin_unlock_bh(&adapter->sta_info_lock);
	return status;
}

/**
 * hdd_softap_rx_packet_cbk() - Receive packet handler
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
QDF_STATUS hdd_softap_rx_packet_cbk(void *context, qdf_nbuf_t rxBuf)
{
	struct hdd_adapter *adapter = NULL;
	int rxstat;
	unsigned int cpu_index;
	struct sk_buff *skb = NULL;
	struct sk_buff *next = NULL;
	struct hdd_context *hdd_ctx = NULL;
	struct qdf_mac_addr src_mac;
	uint8_t staid;
	bool proto_pkt_logged = false;

	/* Sanity check on inputs */
	if (unlikely((NULL == context) || (NULL == rxBuf))) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
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
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD context is Null", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* walk the chain until all are processed */
	next = (struct sk_buff *)rxBuf;

	while (next) {
		skb = next;
		next = skb->next;
		skb->next = NULL;

#ifdef QCA_WIFI_QCA6290 /* Debug code, remove later */
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			 "%s: skb %pK skb->len %d\n", __func__, skb, skb->len);
#endif

		hdd_softap_dump_sk_buff(skb);

		skb->dev = adapter->dev;

		if (unlikely(skb->dev == NULL)) {

			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
				  "%s: ERROR!!Invalid netdevice", __func__);
			continue;
		}
		cpu_index = wlan_hdd_get_cpu();
		++adapter->hdd_stats.tx_rx_stats.rx_packets[cpu_index];
		++adapter->stats.rx_packets;
		adapter->stats.rx_bytes += skb->len;

	qdf_mem_copy(&src_mac, skb->data + QDF_NBUF_SRC_MAC_OFFSET,
		     sizeof(src_mac));
	if (QDF_STATUS_SUCCESS ==
		hdd_softap_get_sta_id(adapter, &src_mac, &staid)) {
		if (staid < WLAN_MAX_STA_COUNT) {
			adapter->sta_info[staid].rx_packets++;
			adapter->sta_info[staid].rx_bytes += skb->len;
			adapter->sta_info[staid].last_tx_rx_ts =
				qdf_system_ticks();
		}
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
				(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
				(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE),
				QDF_RX));
			}

		skb->protocol = eth_type_trans(skb, skb->dev);

		/* hold configurable wakelock for unicast traffic */
		if (hdd_ctx->config->rx_wakelock_timeout &&
			skb->pkt_type != PACKET_BROADCAST &&
			skb->pkt_type != PACKET_MULTICAST) {
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
		if (hdd_napi_enabled(HDD_NAPI_ANY) &&
			!hdd_ctx->enable_rxthread)
			rxstat = netif_receive_skb(skb);
		else
			rxstat = netif_rx_ni(skb);
		if (NET_RX_SUCCESS == rxstat)
			++adapter->hdd_stats.tx_rx_stats.rx_delivered[cpu_index];
		else
			++adapter->hdd_stats.tx_rx_stats.rx_refused[cpu_index];
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_deregister_sta(struct hdd_adapter *adapter, uint8_t staId)
 * @adapter: pointer to adapter context
 * @staId: Station ID to deregister
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_deregister_sta(struct hdd_adapter *adapter,
				     uint8_t staId)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct hdd_context *hdd_ctx;
	int ret;

	if (NULL == adapter) {
		hdd_err("NULL adapter");
		return QDF_STATUS_E_INVAL;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("Invalid adapter magic");
		return QDF_STATUS_E_INVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	/* Clear station in TL and then update HDD data
	 * structures. This helps to block RX frames from other
	 * station to this station.
	 */
	qdf_status = cdp_clear_peer(cds_get_context(QDF_MODULE_ID_SOC),
			(struct cdp_pdev *)cds_get_context(QDF_MODULE_ID_TXRX),
			staId);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_clear_peer failed for staID %d, Status=%d [0x%08X]",
			staId, qdf_status, qdf_status);
	}

	ret = hdd_objmgr_remove_peer_object(adapter->hdd_vdev,
					    adapter->sta_info[staId].
						sta_mac.bytes);
	if (ret)
		hdd_err("Peer obj %pM delete fails",
			adapter->sta_info[staId].sta_mac.bytes);

	if (adapter->sta_info[staId].in_use) {
		if (hdd_ipa_uc_is_enabled(hdd_ctx)) {
			hdd_ipa_wlan_evt(adapter,
					 adapter->sta_info[staId].sta_id,
					 HDD_IPA_CLIENT_DISCONNECT,
					 adapter->sta_info[staId].sta_mac.
					 bytes);
		}
		spin_lock_bh(&adapter->sta_info_lock);
		qdf_mem_zero(&adapter->sta_info[staId],
			     sizeof(struct hdd_station_info));
		spin_unlock_bh(&adapter->sta_info_lock);
	}

	hdd_ctx->sta_to_adapter[staId] = NULL;

	return qdf_status;
}

QDF_STATUS hdd_softap_register_sta(struct hdd_adapter *adapter,
				   bool fAuthRequired,
				   bool fPrivacyBit,
				   uint8_t staId,
				   struct qdf_mac_addr *pPeerMacAddress,
				   bool fWmmEnabled)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	hdd_info("STA:%u, Auth:%u, Priv:%u, WMM:%u",
		 staId, fAuthRequired, fPrivacyBit, fWmmEnabled);

	/*
	 * Clean up old entry if it is not cleaned up properly
	 */
	if (adapter->sta_info[staId].in_use) {
		hdd_info("clean up old entry for STA %d", staId);
		hdd_softap_deregister_sta(adapter, staId);
	}

	/* Get the Station ID from the one saved during the assocation. */
	staDesc.sta_id = staId;

	/* Save the adapter Pointer for this staId */
	hdd_ctx->sta_to_adapter[staId] = adapter;

	qdf_status =
		hdd_softap_init_tx_rx_sta(adapter, staId,
					  pPeerMacAddress);

	staDesc.is_qos_enabled = fWmmEnabled;

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_softap_rx_packet_cbk;
	cdp_vdev_register(soc,
		(struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
		(struct cdp_pdev *)pdev, adapter->session_id),
		adapter, &txrx_ops);
	adapter->txrx_vdev = (void *)cdp_get_vdev_from_vdev_id(soc,
					(struct cdp_pdev *)pdev,
					adapter->session_id);
	adapter->tx_fn = txrx_ops.tx.tx;

	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &staDesc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed to register.  Status = %d [0x%08X]",
			qdf_status, qdf_status);
		return qdf_status;
	}

	/* if ( WPA ), tell TL to go to 'connected' and after keys come to the
	 * driver then go to 'authenticated'.  For all other authentication
	 * types (those that do not require upper layer authentication) we can
	 * put TL directly into 'authenticated' state
	 */

	adapter->sta_info[staId].sta_id = staId;
	adapter->sta_info[staId].is_qos_enabled = fWmmEnabled;

	if (!fAuthRequired) {
		hdd_info("open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time",
			 adapter->sta_info[staId].sta_id);

		/* Connections that do not need Upper layer auth,
		 * transition TL directly to 'Authenticated' state.
		 */
		qdf_status = hdd_change_peer_state(adapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_AUTH, false);

		adapter->sta_info[staId].peer_state = OL_TXRX_PEER_STATE_AUTH;
	} else {

		hdd_info("ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time",
			 adapter->sta_info[staId].sta_id);

		qdf_status = hdd_change_peer_state(adapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_CONN, false);
		adapter->sta_info[staId].peer_state = OL_TXRX_PEER_STATE_CONN;
	}

	hdd_debug("Enabling queues");
	wlan_hdd_netif_queue_control(adapter,
				   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
				   WLAN_CONTROL_PATH);

	return qdf_status;
}

/**
 * hdd_softap_register_bc_sta() - Register the SoftAP broadcast STA
 * @adapter: pointer to adapter context
 * @fPrivacyBit: should 802.11 privacy bit be set?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_bc_sta(struct hdd_adapter *adapter,
				      bool fPrivacyBit)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct qdf_mac_addr broadcastMacAddr = QDF_MAC_ADDR_BCAST_INIT;
	struct hdd_ap_ctx *ap_ctx;

	ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);

	hdd_ctx->sta_to_adapter[WLAN_RX_BCMC_STA_ID] = adapter;
	hdd_ctx->sta_to_adapter[ap_ctx->broadcast_sta_id] = adapter;
	qdf_status =
		hdd_softap_register_sta(adapter, false, fPrivacyBit,
					ap_ctx->broadcast_sta_id,
					&broadcastMacAddr, 0);

	return qdf_status;
}

/**
 * hdd_softap_deregister_bc_sta() - Deregister the SoftAP broadcast STA
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_deregister_bc_sta(struct hdd_adapter *adapter)
{
	struct hdd_ap_ctx *ap_ctx;

	ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
	return hdd_softap_deregister_sta(adapter, ap_ctx->broadcast_sta_id);
}

/**
 * hdd_softap_stop_bss() - Stop the BSS
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_stop_bss(struct hdd_adapter *adapter)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t staId = 0;
	struct hdd_context *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	/* This is stop bss callback running in scheduler thread so do not
	 * driver unload in progress check otherwise it can lead to peer
	 * object leak
	 */
	qdf_status = hdd_softap_deregister_bc_sta(adapter);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		struct hdd_ap_ctx *ap_ctx;

		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
		hdd_err("Failed to deregister BC sta Id %d",
			ap_ctx->broadcast_sta_id);
	}

	for (staId = 0; staId < WLAN_MAX_STA_COUNT; staId++) {
		/* This excludes BC sta as it is already deregistered */
		if (adapter->sta_info[staId].in_use) {
			qdf_status = hdd_softap_deregister_sta(adapter, staId);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_err("Failed to deregister sta Id %d",
					staId);
			}
		}
	}

	/*  Mark the indoor channel (passive) to enable  */
	if (hdd_ctx->config->force_ssc_disable_indoor_channel) {
		hdd_update_indoor_channel(hdd_ctx, false);
		sme_update_channel_list(hdd_ctx->hHal);
	}

	return qdf_status;
}

/**
 * hdd_softap_change_sta_state() - Change the state of a SoftAP station
 * @adapter: pointer to adapter context
 * @pDestMacAddress: MAC address of the station
 * @state: new state of the station
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_change_sta_state(struct hdd_adapter *adapter,
				       struct qdf_mac_addr *pDestMacAddress,
				       enum ol_txrx_peer_state state)
{
	uint8_t sta_id = WLAN_MAX_STA_COUNT;
	QDF_STATUS qdf_status;

	ENTER_DEV(adapter->dev);

	qdf_status = hdd_softap_get_sta_id(adapter, pDestMacAddress, &sta_id);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("Failed to find right station");
		return qdf_status;
	}

	if (false ==
	    qdf_is_macaddr_equal(&adapter->sta_info[sta_id].sta_mac,
				 pDestMacAddress)) {
		hdd_err("Station %u MAC address not matching", sta_id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_status =
		hdd_change_peer_state(adapter, sta_id, state, false);
	hdd_info("Station %u changed to state %d", sta_id, state);

	if (QDF_STATUS_SUCCESS == qdf_status) {
		adapter->sta_info[sta_id].peer_state =
			OL_TXRX_PEER_STATE_AUTH;
		p2p_peer_authorized(adapter->hdd_vdev, pDestMacAddress->bytes);
	}

	EXIT();
	return qdf_status;
}

/*
 * hdd_softap_get_sta_id() - Find station ID from MAC address
 * @adapter: pointer to adapter context
 * @pDestMacAddress: MAC address of the destination
 * @staId: Station ID associated with the MAC address
 *
 * Return: QDF_STATUS_SUCCESS if a match was found, in which case
 *	   staId is populated, QDF_STATUS_E_FAILURE if a match is
 *	   not found
 */
QDF_STATUS hdd_softap_get_sta_id(struct hdd_adapter *adapter,
				 struct qdf_mac_addr *pMacAddress,
				 uint8_t *staId)
{
	uint8_t i;

	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		if (!qdf_mem_cmp
			(&adapter->sta_info[i].sta_mac, pMacAddress,
			QDF_MAC_ADDR_SIZE) && adapter->sta_info[i].in_use) {
			*staId = i;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}
