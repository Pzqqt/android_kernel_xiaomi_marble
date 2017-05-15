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
#include <cdp_txrx_handle.h>
#include <wlan_hdd_object_manager.h>
#include "wlan_p2p_ucfg_api.h"
#ifdef IPA_OFFLOAD
#include <wlan_hdd_ipa.h>
#endif

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
		  "%s: head = %p ", __func__, skb->head);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO,
		  "%s: tail = %p ", __func__, skb->tail);
	QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: end = %p ", __func__, skb->end);
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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) adapter_context;

	if (!pAdapter) {
		hdd_err("NULL adapter");
		return;
	}

	hdd_notice("Enabling queues");
	wlan_hdd_netif_queue_control(pAdapter, WLAN_WAKE_ALL_NETIF_QUEUE,
				     WLAN_CONTROL_PATH);
}

#if defined(CONFIG_PER_VDEV_TX_DESC_POOL)

/**
 * hdd_softap_tx_resume_false() - Resume OS TX Q false leads to queue disabling
 * @pAdapter: pointer to hdd adapter
 * @tx_resume: TX Q resume trigger
 *
 *
 * Return: None
 */
static void
hdd_softap_tx_resume_false(hdd_adapter_t *pAdapter, bool tx_resume)
{
	if (true == tx_resume)
		return;

	hdd_notice("Disabling queues");
	wlan_hdd_netif_queue_control(pAdapter, WLAN_STOP_ALL_NETIF_QUEUE,
				     WLAN_DATA_FLOW_CONTROL);

	if (QDF_TIMER_STATE_STOPPED ==
			qdf_mc_timer_get_current_state(&pAdapter->
						       tx_flow_control_timer)) {
		QDF_STATUS status;

		status = qdf_mc_timer_start(&pAdapter->tx_flow_control_timer,
				WLAN_SAP_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

		if (!QDF_IS_STATUS_SUCCESS(status))
			hdd_err("Failed to start tx_flow_control_timer");
		else
			pAdapter->hdd_stats.hddTxRxStats.txflow_timer_cnt++;
	}
}
#else

static inline void
hdd_softap_tx_resume_false(hdd_adapter_t *pAdapter, bool tx_resume)
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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) adapter_context;

	if (!pAdapter) {
		hdd_err("NULL adapter");
		return;
	}

	/* Resume TX  */
	if (true == tx_resume) {
		if (QDF_TIMER_STATE_STOPPED !=
		    qdf_mc_timer_get_current_state(&pAdapter->
						   tx_flow_control_timer)) {
			qdf_mc_timer_stop(&pAdapter->tx_flow_control_timer);
		}

		hdd_notice("Enabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					WLAN_WAKE_ALL_NETIF_QUEUE,
					WLAN_DATA_FLOW_CONTROL);
	}
	hdd_softap_tx_resume_false(pAdapter, tx_resume);
}

static inline struct sk_buff *hdd_skb_orphan(hdd_adapter_t *pAdapter,
		struct sk_buff *skb)
{
	if (pAdapter->tx_flow_low_watermark > 0)
		skb_orphan(skb);
	else
		skb = skb_unshare(skb, GFP_ATOMIC);

	return skb;
}

#else
/**
 * hdd_skb_orphan() - skb_unshare a cloned packed else skb_orphan
 * @pAdapter: pointer to HDD adapter
 * @skb: pointer to skb data packet
 *
 * Return: pointer to skb structure
 */
static inline struct sk_buff *hdd_skb_orphan(hdd_adapter_t *pAdapter,
		struct sk_buff *skb) {

	struct sk_buff *nskb;
	nskb = skb_unshare(skb, GFP_ATOMIC);

	if (nskb == skb) {
		/*
		 * For UDP packets we want to orphan the packet to allow the app
		 * to send more packets. The flow would ultimately be controlled
		 * by the limited number of tx descriptors for the vdev.
		 */
		++pAdapter->hdd_stats.hddTxRxStats.txXmitOrphaned;
		skb_orphan(skb);
	}
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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) netdev_priv(dev);
	hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
	struct qdf_mac_addr *pDestMacAddress;
	uint8_t STAId;

	++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;
	/* Prevent this function from being called during SSR since TL
	 * context may not be reinitialized at this time which may
	 * lead to a crash.
	 */
	if (cds_is_driver_recovering()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Recovery in Progress. Ignore!!!", __func__);
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
	if (pHddApCtx->dfs_cac_block_tx)
		goto drop_pkt;

	/*
	 * If a transmit function is not registered, drop packet
	 */
	if (!pAdapter->tx_fn) {
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
		STAId = pHddApCtx->uBCStaId;
	} else {
		if (QDF_STATUS_SUCCESS !=
			 hdd_softap_get_sta_id(pAdapter,
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
		} else if (false == pAdapter->aStaInfo[STAId].isUsed) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: STA %d is unregistered", __func__,
				  STAId);
			goto drop_pkt;
		} else if (true == pAdapter->aStaInfo[STAId].
							isDeauthInProgress) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: STA %d deauth in progress", __func__,
				  STAId);
			goto drop_pkt;
		}

		if ((OL_TXRX_PEER_STATE_CONN !=
		     pAdapter->aStaInfo[STAId].tlSTAState)
		    && (OL_TXRX_PEER_STATE_AUTH !=
			pAdapter->aStaInfo[STAId].tlSTAState)) {
			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
				  QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Station not connected yet", __func__);
			goto drop_pkt;
		} else if (OL_TXRX_PEER_STATE_CONN ==
			   pAdapter->aStaInfo[STAId].tlSTAState) {
			if (ntohs(skb->protocol) != HDD_ETHERTYPE_802_1_X) {
				QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA,
					  QDF_TRACE_LEVEL_INFO_HIGH,
					  "%s: NON-EAPOL packet in non-Authenticated state",
					  __func__);
				goto drop_pkt;
			}
		}
	}

	hdd_get_tx_resource(pAdapter, STAId,
			WLAN_SAP_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

	/* Get TL AC corresponding to Qdisc queue index/AC. */
	ac = hdd_qdisc_ac_to_tl_ac[skb->queue_mapping];
	++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];

#if defined(IPA_OFFLOAD)
	if (!qdf_nbuf_ipa_owned_get(skb)) {
#endif

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
		skb = hdd_skb_orphan(pAdapter, skb);
#else
		/* Check if the buffer has enough header room */
		skb = skb_unshare(skb, GFP_ATOMIC);
#endif

		if (!skb)
			goto drop_pkt_accounting;

#if defined(IPA_OFFLOAD)
	}
#endif

	/*
	 * Add SKB to internal tracking table before further processing
	 * in WLAN driver.
	 */
	qdf_net_buf_debug_acquire_skb(skb, __FILE__, __LINE__);

	pAdapter->stats.tx_bytes += skb->len;

	if (qdf_nbuf_is_tso(skb))
		pAdapter->stats.tx_packets += qdf_nbuf_get_tso_num_seg(skb);
	else
		++pAdapter->stats.tx_packets;

	hdd_event_eapol_log(skb, QDF_TX);
	qdf_dp_trace_log_pkt(pAdapter->sessionId, skb, QDF_TX,
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

	if (pAdapter->tx_fn(pAdapter->txrx_vdev,
		 (qdf_nbuf_t) skb) != NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Failed to send packet to txrx for staid:%d",
			  __func__, STAId);
		++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
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
	++pAdapter->stats.tx_dropped;
	++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;

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

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * hdd_wlan_datastall_sap_event()- Send SAP datastall information
 *
 * This Function send send SAP datastall diag event
 *
 * Return: void.
 */
static void hdd_wlan_datastall_sap_event(void)
{
	WLAN_HOST_DIAG_EVENT_DEF(sap_data_stall,
					struct host_event_wlan_datastall);
	qdf_mem_zero(&sap_data_stall, sizeof(sap_data_stall));
	sap_data_stall.reason = SOFTAP_TX_TIMEOUT;
	WLAN_HOST_DIAG_EVENT_REPORT(&sap_data_stall,
						EVENT_WLAN_SOFTAP_DATASTALL);
}
#else
static inline void hdd_wlan_datastall_sap_event(void)
{

}
#endif

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
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	struct netdev_queue *txq;
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
	if (cds_is_driver_recovering()) {
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

	wlan_hdd_display_netif_queue_history(hdd_ctx);
	cdp_dump_flow_pool_info(cds_get_context(QDF_MODULE_ID_SOC));
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_DEBUG,
			"carrier state: %d", netif_carrier_ok(dev));
	hdd_wlan_datastall_sap_event();
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
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_init_tx_rx(hdd_adapter_t *pAdapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	uint8_t STAId = 0;

	qdf_mem_zero(&pAdapter->stats, sizeof(struct net_device_stats));

	spin_lock_init(&pAdapter->staInfo_lock);

	for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++) {
		qdf_mem_zero(&pAdapter->aStaInfo[STAId],
			     sizeof(hdd_station_info_t));
	}

	return status;
}

/**
 * @hdd_softap_deinit_tx_rx() - Deinitialize Tx/RX module
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx(hdd_adapter_t *pAdapter)
{
	if (pAdapter == NULL) {
		hdd_err("Called with pAdapter = NULL.");
		return QDF_STATUS_E_FAILURE;
	}

	pAdapter->txrx_vdev = NULL;
	pAdapter->tx_fn = NULL;
	hdd_info("Deregistering TX function hook !");
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_init_tx_rx_sta() - Initialize tx/rx for a softap station
 * @pAdapter: pointer to adapter context
 * @STAId: Station ID to initialize
 * @pmacAddrSTA: pointer to the MAC address of the station
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_init_tx_rx_sta(hdd_adapter_t *pAdapter, uint8_t STAId,
				     struct qdf_mac_addr *pmacAddrSTA)
{
	spin_lock_bh(&pAdapter->staInfo_lock);
	if (pAdapter->aStaInfo[STAId].isUsed) {
		spin_unlock_bh(&pAdapter->staInfo_lock);
		hdd_err("Reinit of in use station %d", STAId);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&pAdapter->aStaInfo[STAId], sizeof(hdd_station_info_t));

	pAdapter->aStaInfo[STAId].isUsed = true;
	pAdapter->aStaInfo[STAId].isDeauthInProgress = false;
	qdf_copy_macaddr(&pAdapter->aStaInfo[STAId].macAddrSTA, pmacAddrSTA);

	spin_unlock_bh(&pAdapter->staInfo_lock);
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_deinit_tx_rx_sta() - Deinitialize tx/rx for a softap station
 * @pAdapter: pointer to adapter context
 * @STAId: Station ID to deinitialize
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx_sta(hdd_adapter_t *pAdapter, uint8_t STAId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_hostapd_state_t *pHostapdState;

	pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

	spin_lock_bh(&pAdapter->staInfo_lock);

	if (false == pAdapter->aStaInfo[STAId].isUsed) {
		spin_unlock_bh(&pAdapter->staInfo_lock);
		hdd_err("Deinit station not inited %d", STAId);
		return QDF_STATUS_E_FAILURE;
	}

	pAdapter->aStaInfo[STAId].isUsed = false;
	pAdapter->aStaInfo[STAId].isDeauthInProgress = false;

	spin_unlock_bh(&pAdapter->staInfo_lock);
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
	hdd_adapter_t *pAdapter = NULL;
	int rxstat;
	unsigned int cpu_index;
	struct sk_buff *skb = NULL;
	struct sk_buff *next = NULL;
	hdd_context_t *pHddCtx = NULL;

	/* Sanity check on inputs */
	if (unlikely((NULL == context) || (NULL == rxBuf))) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pAdapter = (hdd_adapter_t *)context;
	if (unlikely(WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Magic cookie(%x) for adapter sanity verification is invalid",
			  pAdapter->magic);
		return QDF_STATUS_E_FAILURE;
	}

	pHddCtx = pAdapter->pHddCtx;
	if (unlikely(NULL == pHddCtx)) {
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

#ifdef QCA_WIFI_NAPIER_EMULATION /* Debug code, remove later */
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			 "%s: skb %p skb->len %d\n", __func__, skb, skb->len);
#endif

		hdd_softap_dump_sk_buff(skb);

		skb->dev = pAdapter->dev;

		if (unlikely(skb->dev == NULL)) {

			QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_ERROR,
				  "%s: ERROR!!Invalid netdevice", __func__);
			continue;
		}
		cpu_index = wlan_hdd_get_cpu();
		++pAdapter->hdd_stats.hddTxRxStats.rxPackets[cpu_index];
		++pAdapter->stats.rx_packets;
		pAdapter->stats.rx_bytes += skb->len;

		hdd_event_eapol_log(skb, QDF_RX);
		DPTRACE(qdf_dp_trace(skb,
			QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD,
			QDF_TRACE_DEFAULT_PDEV_ID,
			qdf_nbuf_data_addr(skb),
			sizeof(qdf_nbuf_data(skb)), QDF_RX));
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_RX_PACKET_RECORD,
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

		skb->protocol = eth_type_trans(skb, skb->dev);

		/* hold configurable wakelock for unicast traffic */
		if (pHddCtx->config->rx_wakelock_timeout &&
			skb->pkt_type != PACKET_BROADCAST &&
			skb->pkt_type != PACKET_MULTICAST) {
			cds_host_diag_log_work(&pHddCtx->rx_wake_lock,
						   pHddCtx->config->rx_wakelock_timeout,
						   WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);
			qdf_wake_lock_timeout_acquire(&pHddCtx->rx_wake_lock,
							  pHddCtx->config->
								  rx_wakelock_timeout);
		}

		/* Remove SKB from internal tracking table before submitting
		 * it to stack
		 */
		qdf_net_buf_debug_release_skb(skb);
		if (hdd_napi_enabled(HDD_NAPI_ANY) &&
			!pHddCtx->enableRxThread)
			rxstat = netif_receive_skb(skb);
		else
			rxstat = netif_rx_ni(skb);
		if (NET_RX_SUCCESS == rxstat)
			++pAdapter->hdd_stats.hddTxRxStats.rxDelivered[cpu_index];
		else
			++pAdapter->hdd_stats.hddTxRxStats.rxRefused[cpu_index];

		pAdapter->dev->last_rx = jiffies;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_deregister_sta(hdd_adapter_t *pAdapter, uint8_t staId)
 * @pAdapter: pointer to adapter context
 * @staId: Station ID to deregister
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_deregister_sta(hdd_adapter_t *pAdapter, uint8_t staId)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	hdd_context_t *pHddCtx;
	int ret;

	if (NULL == pAdapter) {
		hdd_err("NULL adapter");
		return QDF_STATUS_E_INVAL;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) {
		hdd_err("Invalid adapter magic");
		return QDF_STATUS_E_INVAL;
	}

	pHddCtx = (hdd_context_t *) (pAdapter->pHddCtx);
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

	ret = hdd_objmgr_remove_peer_object(pAdapter->hdd_vdev,
					    pAdapter->aStaInfo[staId].
						macAddrSTA.bytes);
	if (ret)
		hdd_err("Peer obj %pM delete fails",
			pAdapter->aStaInfo[staId].macAddrSTA.bytes);

	if (pAdapter->aStaInfo[staId].isUsed) {
		spin_lock_bh(&pAdapter->staInfo_lock);
		qdf_mem_zero(&pAdapter->aStaInfo[staId],
			     sizeof(hdd_station_info_t));
		spin_unlock_bh(&pAdapter->staInfo_lock);
	}

	pHddCtx->sta_to_adapter[staId] = NULL;

	return qdf_status;
}

/**
 * hdd_softap_register_sta() - Register a SoftAP STA
 * @pAdapter: pointer to adapter context
 * @fAuthRequired: is additional authentication required?
 * @fPrivacyBit: should 802.11 privacy bit be set?
 * @staId: station ID assigned to this station
 * @ucastSig: unicast security signature
 * @bcastSig: broadcast security signature
 * @pPeerMacAddress: station MAC address
 * @fWmmEnabled: is WMM enabled for this STA?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_sta(hdd_adapter_t *pAdapter,
				   bool fAuthRequired,
				   bool fPrivacyBit,
				   uint8_t staId,
				   uint8_t ucastSig,
				   uint8_t bcastSig,
				   struct qdf_mac_addr *pPeerMacAddress,
				   bool fWmmEnabled)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };
	hdd_context_t *pHddCtx = pAdapter->pHddCtx;
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	hdd_info("STA:%u, Auth:%u, Priv:%u, WMM:%u",
		 staId, fAuthRequired, fPrivacyBit, fWmmEnabled);

	/*
	 * Clean up old entry if it is not cleaned up properly
	 */
	if (pAdapter->aStaInfo[staId].isUsed) {
		hdd_info("clean up old entry for STA %d", staId);
		hdd_softap_deregister_sta(pAdapter, staId);
	}

	/* Get the Station ID from the one saved during the assocation. */
	staDesc.sta_id = staId;

	/* Save the pAdapter Pointer for this staId */
	pHddCtx->sta_to_adapter[staId] = pAdapter;

	qdf_status =
		hdd_softap_init_tx_rx_sta(pAdapter, staId,
					  pPeerMacAddress);

	staDesc.is_qos_enabled = fWmmEnabled;

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_softap_rx_packet_cbk;
	cdp_vdev_register(soc,
		(struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
		(struct cdp_pdev *)pdev, pAdapter->sessionId),
		pAdapter, &txrx_ops);
	pAdapter->txrx_vdev = (void *)cdp_get_vdev_from_vdev_id(soc,
					(struct cdp_pdev *)pdev,
					pAdapter->sessionId);
	pAdapter->tx_fn = txrx_ops.tx.tx;

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

	pAdapter->aStaInfo[staId].ucSTAId = staId;
	pAdapter->aStaInfo[staId].isQosEnabled = fWmmEnabled;

	if (!fAuthRequired) {
		hdd_info("open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time",
			 pAdapter->aStaInfo[staId].ucSTAId);

		/* Connections that do not need Upper layer auth,
		 * transition TL directly to 'Authenticated' state.
		 */
		qdf_status = hdd_change_peer_state(pAdapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_AUTH, false);

		pAdapter->aStaInfo[staId].tlSTAState = OL_TXRX_PEER_STATE_AUTH;
		pAdapter->sessionCtx.ap.uIsAuthenticated = true;
	} else {

		hdd_info("ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time",
			 pAdapter->aStaInfo[staId].ucSTAId);

		qdf_status = hdd_change_peer_state(pAdapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_CONN, false);
		pAdapter->aStaInfo[staId].tlSTAState = OL_TXRX_PEER_STATE_CONN;

		pAdapter->sessionCtx.ap.uIsAuthenticated = false;

	}

	/* Enable Tx queue */
	hdd_notice("Enabling queues");
	wlan_hdd_netif_queue_control(pAdapter,
				   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
				   WLAN_CONTROL_PATH);

	return qdf_status;
}

/**
 * hdd_softap_register_bc_sta() - Register the SoftAP broadcast STA
 * @pAdapter: pointer to adapter context
 * @fPrivacyBit: should 802.11 privacy bit be set?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_bc_sta(hdd_adapter_t *pAdapter,
				      bool fPrivacyBit)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	struct qdf_mac_addr broadcastMacAddr =
					QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	hdd_ap_ctx_t *pHddApCtx;

	pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);

	pHddCtx->sta_to_adapter[WLAN_RX_BCMC_STA_ID] = pAdapter;
	pHddCtx->sta_to_adapter[pHddApCtx->uBCStaId] = pAdapter;
	qdf_status =
		hdd_softap_register_sta(pAdapter, false, fPrivacyBit,
					(WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->
					uBCStaId, 0, 1, &broadcastMacAddr, 0);

	return qdf_status;
}

/**
 * hdd_softap_deregister_bc_sta() - Deregister the SoftAP broadcast STA
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_deregister_bc_sta(hdd_adapter_t *pAdapter)
{
	return hdd_softap_deregister_sta(pAdapter,
					 (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->
					 uBCStaId);
}

/**
 * hdd_softap_stop_bss() - Stop the BSS
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_stop_bss(hdd_adapter_t *pAdapter)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t staId = 0;
	hdd_context_t *pHddCtx;

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	/* bss deregister is not allowed during wlan driver loading or
	 * unloading
	 */
	if (cds_is_load_or_unload_in_progress()) {
		hdd_err("Loading_unloading in Progress, state: 0x%x. Ignore!!!",
			cds_get_driver_state());
		return QDF_STATUS_E_PERM;
	}

	qdf_status = hdd_softap_deregister_bc_sta(pAdapter);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Failed to deregister BC sta Id %d",
			(WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->uBCStaId);
	}

	for (staId = 0; staId < WLAN_MAX_STA_COUNT; staId++) {
		/* This excludes BC sta as it is already deregistered */
		if (pAdapter->aStaInfo[staId].isUsed) {
			qdf_status = hdd_softap_deregister_sta(pAdapter, staId);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_err("Failed to deregister sta Id %d",
					staId);
			}
		}
	}
	return qdf_status;
}

/**
 * hdd_softap_change_sta_state() - Change the state of a SoftAP station
 * @pAdapter: pointer to adapter context
 * @pDestMacAddress: MAC address of the station
 * @state: new state of the station
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_change_sta_state(hdd_adapter_t *pAdapter,
				       struct qdf_mac_addr *pDestMacAddress,
				       enum ol_txrx_peer_state state)
{
	uint8_t ucSTAId = WLAN_MAX_STA_COUNT;
	QDF_STATUS qdf_status;

	ENTER_DEV(pAdapter->dev);

	qdf_status = hdd_softap_get_sta_id(pAdapter, pDestMacAddress, &ucSTAId);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("Failed to find right station");
		return qdf_status;
	}

	if (false ==
	    qdf_is_macaddr_equal(&pAdapter->aStaInfo[ucSTAId].macAddrSTA,
				 pDestMacAddress)) {
		hdd_err("Station %u MAC address not matching", ucSTAId);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_status =
		hdd_change_peer_state(pAdapter, ucSTAId, state, false);
	hdd_info("Station %u changed to state %d", ucSTAId, state);

	if (QDF_STATUS_SUCCESS == qdf_status) {
		pAdapter->aStaInfo[ucSTAId].tlSTAState =
			OL_TXRX_PEER_STATE_AUTH;
		p2p_peer_authorized(pAdapter->hdd_vdev, pDestMacAddress->bytes);
	}

	EXIT();
	return qdf_status;
}

/*
 * hdd_softap_get_sta_id() - Find station ID from MAC address
 * @pAdapter: pointer to adapter context
 * @pDestMacAddress: MAC address of the destination
 * @staId: Station ID associated with the MAC address
 *
 * Return: QDF_STATUS_SUCCESS if a match was found, in which case
 *	   staId is populated, QDF_STATUS_E_FAILURE if a match is
 *	   not found
 */
QDF_STATUS hdd_softap_get_sta_id(hdd_adapter_t *pAdapter,
				 struct qdf_mac_addr *pMacAddress,
				 uint8_t *staId)
{
	uint8_t i;

	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		if (!qdf_mem_cmp
			(&pAdapter->aStaInfo[i].macAddrSTA, pMacAddress,
			QDF_MAC_ADDR_SIZE) && pAdapter->aStaInfo[i].isUsed) {
			*staId = i;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}
