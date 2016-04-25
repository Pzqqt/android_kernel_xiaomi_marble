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
 * DOC: wlan_hdd_tx_rx.c
 *
 * Linux HDD Tx/RX APIs
 */

#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_napi.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <cds_sched.h>

#include <wlan_hdd_p2p.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <net/ieee80211_radiotap.h>
#include "sap_api.h"
#include "wlan_hdd_wmm.h"

#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif
#include <wlan_hdd_ipa.h>

#include "wlan_hdd_ocb.h"
#include "wlan_hdd_lro.h"

#include "cdp_txrx_peer_ops.h"
#include "ol_txrx.h"

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#define HDD_EAPOL_ETHER_TYPE             (0x888E)
#define HDD_EAPOL_ETHER_TYPE_OFFSET      (12)
#define HDD_EAPOL_PACKET_TYPE_OFFSET     (15)
#define HDD_EAPOL_KEY_INFO_OFFSET        (19)
#define HDD_EAPOL_DEST_MAC_OFFSET        (0)
#define HDD_EAPOL_SRC_MAC_OFFSET         (6)
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

const uint8_t hdd_wmm_ac_to_highest_up[] = {
	SME_QOS_WMM_UP_RESV,
	SME_QOS_WMM_UP_EE,
	SME_QOS_WMM_UP_VI,
	SME_QOS_WMM_UP_NC
};

/* Mapping Linux AC interpretation to SME AC. */
const uint8_t hdd_qdisc_ac_to_tl_ac[] = {
	SME_AC_VO,
	SME_AC_VI,
	SME_AC_BE,
	SME_AC_BK,
};

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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) adapter_context;

	if (!pAdapter) {
		/* INVALID ARG */
		return;
	}

	hddLog(LOG1, FL("Enabling queues"));
	wlan_hdd_netif_queue_control(pAdapter, WLAN_WAKE_ALL_NETIF_QUEUE,
				   WLAN_CONTROL_PATH);
	return;
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
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) adapter_context;
	hdd_station_ctx_t *hdd_sta_ctx = NULL;

	if (!pAdapter) {
		/* INVALID ARG */
		return;
	}

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	/* Resume TX  */
	if (true == tx_resume) {
		if (QDF_TIMER_STATE_STOPPED !=
		    qdf_mc_timer_get_current_state(&pAdapter->
						   tx_flow_control_timer)) {
			qdf_mc_timer_stop(&pAdapter->tx_flow_control_timer);
		}
		if (qdf_unlikely(hdd_sta_ctx->hdd_ReassocScenario)) {
			hddLog(LOGW,
			       FL("flow control, tx queues un-pause avoided as we are in REASSOCIATING state"));
			       return;
		}
		hddLog(LOG1, FL("Enabling queues"));
		wlan_hdd_netif_queue_control(pAdapter,
					 WLAN_WAKE_ALL_NETIF_QUEUE,
					 WLAN_DATA_FLOW_CONTROL);
	}
	return;
}

/**
 * hdd_register_tx_flow_control() - Register TX Flow control
 * @adapter: adapter handle
 * @timer_callback: timer callback
 * @flow_control_fp: txrx flow control
 *
 * Return: none
 */
void hdd_register_tx_flow_control(hdd_adapter_t *adapter,
		qdf_mc_timer_callback_t timer_callback,
		ol_txrx_tx_flow_control_fp flow_control_fp)
{
	if (adapter->tx_flow_timer_initialized == false) {
		qdf_mc_timer_init(&adapter->tx_flow_control_timer,
			  QDF_TIMER_TYPE_SW,
			  timer_callback,
			  adapter);
		adapter->tx_flow_timer_initialized = true;
	}
	ol_txrx_register_tx_flow_control(adapter->sessionId,
					flow_control_fp,
					adapter);

}

/**
 * hdd_deregister_tx_flow_control() - Deregister TX Flow control
 * @adapter: adapter handle
 *
 * Return: none
 */
void hdd_deregister_tx_flow_control(hdd_adapter_t *adapter)
{
	ol_txrx_deregister_tx_flow_control_cb(adapter->sessionId);
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
void hdd_get_tx_resource(hdd_adapter_t *adapter,
			uint8_t STAId, uint16_t timer_value)
{
	if (false ==
	    ol_txrx_get_tx_resource(STAId,
				   adapter->tx_flow_low_watermark,
				   adapter->tx_flow_high_watermark_offset)) {
		hdd_info("Disabling queues lwm %d hwm offset %d",
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
			adapter->hdd_stats.hddTxRxStats.txflow_timer_cnt++;
			adapter->hdd_stats.hddTxRxStats.txflow_pause_cnt++;
			adapter->hdd_stats.hddTxRxStats.is_txflow_paused = true;
		}
	}
}

#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * wlan_hdd_is_eapol() - Function to check if frame is EAPOL or not
 * @skb:    skb data
 *
 * This function checks if the frame is an EAPOL frame or not
 *
 * Return: true (1) if packet is EAPOL
 *
 */
static bool wlan_hdd_is_eapol(struct sk_buff *skb)
{
	uint16_t ether_type;

	if (!skb) {
		hdd_err(FL("skb is NULL"));
		return false;
	}

	ether_type = (uint16_t)(*(uint16_t *)
			(skb->data + HDD_ETHERTYPE_802_1_X_FRAME_OFFSET));

	if (ether_type == QDF_SWAP_U16(HDD_ETHERTYPE_802_1_X))
		return true;

	return false;
}

/**
 * wlan_hdd_is_eapol_or_wai() - Check if frame is EAPOL or WAPI
 * @skb:    skb data
 *
 * This function checks if the frame is EAPOL or WAPI.
 * single routine call will check for both types, thus avoiding
 * data path performance penalty.
 *
 * Return: true (1) if packet is EAPOL or WAPI
 *
 */
static bool wlan_hdd_is_eapol_or_wai(struct sk_buff *skb)
{
	uint16_t ether_type;

	if (!skb) {
		hdd_err(FL("skb is NULL"));
		return false;
	}

	ether_type = (uint16_t)(*(uint16_t *)
			(skb->data + HDD_ETHERTYPE_802_1_X_FRAME_OFFSET));

	if (ether_type == QDF_SWAP_U16(HDD_ETHERTYPE_802_1_X) ||
	    ether_type == QDF_SWAP_U16(HDD_ETHERTYPE_WAI))
		return true;

	/* No error msg handled since this will happen often */
	return false;
}

/**
 * hdd_hard_start_xmit() - Transmit a frame
 * @skb: pointer to OS packet (sk_buff)
 * @dev: pointer to network device
 *
 * Function registered with the Linux OS for transmitting
 * packets. This version of the function directly passes
 * the packet to Transport Layer.
 *
 * Return: Always returns NETDEV_TX_OK
 */
int hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	QDF_STATUS status;
	sme_ac_enum_type ac;
	sme_QosWmmUpType up;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	bool granted;
	uint8_t STAId = WLAN_MAX_STA_COUNT;
	hdd_station_ctx_t *pHddStaCtx = &pAdapter->sessionCtx.station;
#ifdef QCA_PKT_PROTO_TRACE
	uint8_t proto_type = 0;
	hdd_context_t *hddCtxt = WLAN_HDD_GET_CTX(pAdapter);
#endif /* QCA_PKT_PROTO_TRACE */

#ifdef QCA_WIFI_FTM
	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}
#endif

	++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;
	if (cds_is_driver_recovering()) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_WARN,
			"Recovery in progress, dropping the packet");
		++pAdapter->stats.tx_dropped;
		++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	if (QDF_IBSS_MODE == pAdapter->device_mode) {
		struct qdf_mac_addr *pDestMacAddress =
					(struct qdf_mac_addr *) skb->data;

		if (QDF_STATUS_SUCCESS !=
				hdd_ibss_get_sta_id(&pAdapter->sessionCtx.station,
					pDestMacAddress, &STAId))
			STAId = HDD_WLAN_INVALID_STA_ID;

		if ((STAId == HDD_WLAN_INVALID_STA_ID) &&
		    (qdf_is_macaddr_broadcast(pDestMacAddress) ||
		     qdf_is_macaddr_group(pDestMacAddress))) {
			STAId = pHddStaCtx->broadcast_ibss_staid;
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
				  QDF_TRACE_LEVEL_INFO_LOW, "%s: BC/MC packet",
				  __func__);
		} else if (STAId == HDD_WLAN_INVALID_STA_ID) {
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_WARN,
				  "%s: Received Unicast frame with invalid staID",
				  __func__);
			++pAdapter->stats.tx_dropped;
			++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
			kfree_skb(skb);
			return NETDEV_TX_OK;
		}
	} else {
		if (QDF_OCB_MODE != pAdapter->device_mode &&
			eConnectionState_Associated !=
				pHddStaCtx->conn_info.connState) {
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO,
				FL("Tx frame in not associated state in %d context"),
				pAdapter->device_mode);
			++pAdapter->stats.tx_dropped;
			++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
			kfree_skb(skb);
			return NETDEV_TX_OK;
		}
		STAId = pHddStaCtx->conn_info.staId[0];
	}


	hdd_get_tx_resource(pAdapter, STAId,
				WLAN_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME);

	/* Get TL AC corresponding to Qdisc queue index/AC. */
	ac = hdd_qdisc_ac_to_tl_ac[skb->queue_mapping];

	if (!qdf_nbuf_ipa_owned_get(skb)) {
		/* Check if the buffer has enough header room */
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb)
			goto drop_pkt;

		if (skb_headroom(skb) < dev->hard_header_len) {
			struct sk_buff *tmp;
			tmp = skb;
			skb = skb_realloc_headroom(tmp, dev->hard_header_len);
			dev_kfree_skb(tmp);
			if (!skb)
				goto drop_pkt;
		}
	}

	/* user priority from IP header, which is already extracted and set from
	 * select_queue call back function
	 */
	up = skb->priority;

	++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];
#ifdef HDD_WMM_DEBUG
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_FATAL,
		  "%s: Classified as ac %d up %d", __func__, ac, up);
#endif /* HDD_WMM_DEBUG */

	if (HDD_PSB_CHANGED == pAdapter->psbChanged) {
		/* Function which will determine acquire admittance for a
		 * WMM AC is required or not based on psb configuration done
		 * in the framework
		 */
		hdd_wmm_acquire_access_required(pAdapter, ac);
	}
	/*
	 * Make sure we already have access to this access category
	 * or it is EAPOL or WAPI frame during initial authentication which
	 * can have artifically boosted higher qos priority.
	 */

	if (((pAdapter->psbChanged & (1 << ac)) &&
		likely(pAdapter->hddWmmStatus.wmmAcStatus[ac].
			wmmAcAccessAllowed)) ||
		((pHddStaCtx->conn_info.uIsAuthenticated == false) &&
		 wlan_hdd_is_eapol_or_wai(skb))) {
		granted = true;
	} else {
		status = hdd_wmm_acquire_access(pAdapter, ac, &granted);
		pAdapter->psbChanged |= (1 << ac);
	}

	if (!granted) {
		bool isDefaultAc = false;
		/* ADDTS request for this AC is sent, for now
		 * send this packet through next avaiable lower
		 * Access category until ADDTS negotiation completes.
		 */
		while (!likely
			       (pAdapter->hddWmmStatus.wmmAcStatus[ac].
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

	wlan_hdd_log_eapol(skb,
			   WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED);

#ifdef QCA_PKT_PROTO_TRACE
	if ((hddCtxt->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_EAPOL) ||
	    (hddCtxt->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_DHCP)) {
		proto_type = cds_pkt_get_proto_type(skb,
						    hddCtxt->config->
						    gEnableDebugLog, 0);
		if (CDS_PKT_TRAC_TYPE_EAPOL & proto_type) {
			cds_pkt_trace_buf_update("ST:T:EPL");
		} else if (CDS_PKT_TRAC_TYPE_DHCP & proto_type) {
			cds_pkt_trace_buf_update("ST:T:DHC");
		}
	}
#endif /* QCA_PKT_PROTO_TRACE */

	pAdapter->stats.tx_bytes += skb->len;
	++pAdapter->stats.tx_packets;

	/* Zero out skb's context buffer for the driver to use */
	qdf_mem_set(skb->cb, sizeof(skb->cb), 0);
	QDF_NBUF_CB_TX_PACKET_TRACK(skb) = QDF_NBUF_TX_PKT_DATA_TRACK;
	QDF_NBUF_UPDATE_TX_PKT_COUNT(skb, QDF_NBUF_TX_PKT_HDD);

	qdf_dp_trace_set_track(skb);
	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_PACKET_PTR_RECORD,
				(uint8_t *)skb->data, sizeof(skb->data)));
	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_PACKET_RECORD,
				(uint8_t *)skb->data, qdf_nbuf_len(skb)));
	if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_HDD_PACKET_RECORD,
				(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
				(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE)));

	/* Check if station is connected */
	if (OL_TXRX_PEER_STATE_CONN ==
		 pAdapter->aStaInfo[STAId].tlSTAState) {
			QDF_TRACE(QDF_MODULE_ID_HDD_DATA,
				 QDF_TRACE_LEVEL_WARN,
				 "%s: station is not connected..dropping pkt",
				 __func__);
				goto drop_pkt;
	}

#ifdef QCA_PKT_PROTO_TRACE
	qdf_nbuf_trace_set_proto_type(skb, proto_type);
#endif

	/*
	* If a transmit function is not registered, drop packet
	*/
	if (!pAdapter->tx_fn) {
		QDF_TRACE(QDF_MODULE_ID_HDD_SAP_DATA, QDF_TRACE_LEVEL_INFO_HIGH,
			 "%s: TX function not registered by the data path",
			 __func__);
		goto drop_pkt;
	}

	if (pAdapter->tx_fn(ol_txrx_get_vdev_by_sta_id(STAId),
		 (qdf_nbuf_t) skb) != NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_WARN,
			  "%s: Failed to send packet to txrx for staid:%d",
			  __func__, STAId);
		goto drop_pkt;
	}
	dev->trans_start = jiffies;

	return NETDEV_TX_OK;

drop_pkt:

	DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_DROP_PACKET_RECORD,
				(uint8_t *)skb->data, qdf_nbuf_len(skb)));
	if (qdf_nbuf_len(skb) > QDF_DP_TRACE_RECORD_SIZE)
		DPTRACE(qdf_dp_trace(skb, QDF_DP_TRACE_DROP_PACKET_RECORD,
				(uint8_t *)&skb->data[QDF_DP_TRACE_RECORD_SIZE],
				(qdf_nbuf_len(skb)-QDF_DP_TRACE_RECORD_SIZE)));

	++pAdapter->stats.tx_dropped;
	++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
	++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

/**
 * hdd_ibss_get_sta_id() - Get the StationID using the Peer Mac address
 * @pHddStaCtx: pointer to HDD Station Context
 * @pMacAddress: pointer to Peer Mac address
 * @staID: pointer to returned Station Index
 *
 * Return: QDF_STATUS_SUCCESS/QDF_STATUS_E_FAILURE
 */

QDF_STATUS hdd_ibss_get_sta_id(hdd_station_ctx_t *pHddStaCtx,
			       struct qdf_mac_addr *pMacAddress, uint8_t *staId)
{
	uint8_t idx;

	for (idx = 0; idx < MAX_IBSS_PEERS; idx++) {
		if (!qdf_mem_cmp(&pHddStaCtx->conn_info.peerMacAddress[idx],
				    pMacAddress, QDF_MAC_ADDR_SIZE)) {
			*staId = pHddStaCtx->conn_info.staId[idx];
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
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	struct netdev_queue *txq;
	int i = 0;

	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
		  "%s: Transmission timeout occurred jiffies %lu trans_start %lu",
		  __func__, jiffies, dev->trans_start);
	DPTRACE(qdf_dp_trace(NULL, QDF_DP_TRACE_HDD_TX_TIMEOUT,
				NULL, 0));

	/* Getting here implies we disabled the TX queues for too
	 * long. Queues are disabled either because of disassociation
	 * or low resource scenarios. In case of disassociation it is
	 * ok to ignore this. But if associated, we have do possible
	 * recovery here
	 */

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		txq = netdev_get_tx_queue(dev, i);
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "Queue%d status: %d txq->trans_start %lu",
			   i, netif_tx_queue_stopped(txq), txq->trans_start);
	}

	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO,
		  "carrier state: %d", netif_carrier_ok(dev));
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	wlan_hdd_display_netif_queue_history(hdd_ctx);
	ol_tx_dump_flow_pool_info();
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
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_init_tx_rx(hdd_adapter_t *pAdapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pAdapter) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  FL("pAdapter is NULL"));
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * @hdd_deinit_tx_rx() - Deinitialize Tx/RX module
 * @pAdapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_deinit_tx_rx(hdd_adapter_t *pAdapter)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pAdapter) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  FL("pAdapter is NULL"));
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
	hdd_adapter_t *adapter;
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

	adapter = (hdd_adapter_t *)context;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "invalid adapter %p", adapter);
		return QDF_STATUS_E_FAILURE;
	}

	cpu_index = wlan_hdd_get_cpu();

	/* walk the chain until all are processed */
	skb = (struct sk_buff *) rxbuf;
	while (NULL != skb) {
		skb_next = skb->next;
		skb->dev = adapter->dev;

		++adapter->hdd_stats.hddTxRxStats.rxPackets[cpu_index];
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
				hdd_stats.hddTxRxStats.rxDelivered[cpu_index];
		else
			++adapter->hdd_stats.hddTxRxStats.rxRefused[cpu_index];

		skb = skb_next;
	}

	adapter->dev->last_rx = jiffies;

	return QDF_STATUS_SUCCESS;
}

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
	hdd_adapter_t *pAdapter = NULL;
	hdd_context_t *pHddCtx = NULL;
	int rxstat;
	struct sk_buff *skb = NULL;
#ifdef QCA_PKT_PROTO_TRACE
	uint8_t proto_type;
#endif /* QCA_PKT_PROTO_TRACE */
	hdd_station_ctx_t *pHddStaCtx = NULL;
	unsigned int cpu_index;

	/* Sanity check on inputs */
	if (unlikely((NULL == context) || (NULL == rxBuf))) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pAdapter = (hdd_adapter_t *)context;
	if (unlikely(WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_FATAL,
			  "Magic cookie(%x) for adapter sanity verification is invalid",
			  pAdapter->magic);
		return QDF_STATUS_E_FAILURE;
	}

	pHddCtx = pAdapter->pHddCtx;
	if (unlikely(NULL == pHddCtx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD context is Null", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cpu_index = wlan_hdd_get_cpu();

	skb = (struct sk_buff *)rxBuf;

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	if ((pHddStaCtx->conn_info.proxyARPService) &&
	    cfg80211_is_gratuitous_arp_unsolicited_na(skb)) {
		++pAdapter->hdd_stats.hddTxRxStats.rxDropped[cpu_index];
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, QDF_TRACE_LEVEL_INFO,
			  "%s: Dropping HS 2.0 Gratuitous ARP or Unsolicited NA",
			  __func__);
		/* Remove SKB from internal tracking table before submitting
		 * it to stack
		 */
		qdf_nbuf_free(skb);
		return QDF_STATUS_SUCCESS;
	}

	wlan_hdd_log_eapol(skb, WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED);

#ifdef QCA_PKT_PROTO_TRACE
	if ((pHddCtx->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_EAPOL) ||
	    (pHddCtx->config->gEnableDebugLog & CDS_PKT_TRAC_TYPE_DHCP)) {
		proto_type = cds_pkt_get_proto_type(skb,
						    pHddCtx->config->
						    gEnableDebugLog, 0);
		if (CDS_PKT_TRAC_TYPE_EAPOL & proto_type) {
			cds_pkt_trace_buf_update("ST:R:EPL");
		} else if (CDS_PKT_TRAC_TYPE_DHCP & proto_type) {
			cds_pkt_trace_buf_update("ST:R:DHC");
		}
	}
#endif /* QCA_PKT_PROTO_TRACE */

	skb->dev = pAdapter->dev;
	skb->protocol = eth_type_trans(skb, skb->dev);
	++pAdapter->hdd_stats.hddTxRxStats.rxPackets[cpu_index];
	++pAdapter->stats.rx_packets;
	pAdapter->stats.rx_bytes += skb->len;
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
	qdf_wake_lock_timeout_acquire(&pHddCtx->rx_wake_lock,
				      HDD_WAKE_LOCK_DURATION,
				      WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);
#endif

	/* Remove SKB from internal tracking table before submitting
	 * it to stack
	 */
	qdf_net_buf_debug_release_skb(rxBuf);

	if (HDD_LRO_NO_RX ==
		 hdd_lro_rx(pHddCtx, pAdapter, skb)) {
		if (hdd_napi_enabled(HDD_NAPI_ANY) &&
		    !pHddCtx->config->enableRxThread)
			rxstat = netif_receive_skb(skb);
		else
			rxstat = netif_rx_ni(skb);

		if (NET_RX_SUCCESS == rxstat)
			++pAdapter->hdd_stats.hddTxRxStats.
				 rxDelivered[cpu_index];
		else
			++pAdapter->hdd_stats.hddTxRxStats.
				 rxRefused[cpu_index];

	} else {
		++pAdapter->hdd_stats.hddTxRxStats.
			 rxDelivered[cpu_index];
	}

	pAdapter->dev->last_rx = jiffies;

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT

/**
 * wlan_hdd_get_eapol_params() - Function to extract EAPOL params
 * @skb:                sbb data
 * @eapol_params:       Pointer to hold the parsed EAPOL params
 * @event_type:         Event type to indicate Tx/Rx
 *
 * This function parses the input skb data and return the EAPOL parameters if
 * the packet is an eapol packet.
 *
 * Return: -EINVAL if the packet is not an EAPOL packet and 0 on success
 *
 */
static int wlan_hdd_get_eapol_params(struct sk_buff *skb,
		struct host_event_wlan_eapol *eapol_params,
		uint8_t event_type)
{
	bool ret;
	uint8_t packet_type;

	ret = wlan_hdd_is_eapol(skb);

	if (!ret)
		return -EINVAL;

	packet_type = (uint8_t)(*(uint8_t *)
			(skb->data + HDD_EAPOL_PACKET_TYPE_OFFSET));

	eapol_params->eapol_packet_type = packet_type;
	eapol_params->eapol_key_info = (uint16_t)(*(uint16_t *)
			(skb->data + HDD_EAPOL_KEY_INFO_OFFSET));
	eapol_params->event_sub_type = event_type;
	eapol_params->eapol_rate = 0;/* As of now, zero */

	qdf_mem_copy(eapol_params->dest_addr,
			(skb->data + HDD_EAPOL_DEST_MAC_OFFSET),
			sizeof(eapol_params->dest_addr));
	qdf_mem_copy(eapol_params->src_addr,
			(skb->data + HDD_EAPOL_SRC_MAC_OFFSET),
			sizeof(eapol_params->src_addr));
	return 0;
}

/**
 * wlan_hdd_event_eapol_log() - Function to log EAPOL events
 * @eapol_params:    Structure containing EAPOL params
 *
 * This function logs the parsed EAPOL params
 *
 * Return: None
 *
 */
static void wlan_hdd_event_eapol_log(struct host_event_wlan_eapol eapol_params)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event, struct host_event_wlan_eapol);

	wlan_diag_event.event_sub_type = eapol_params.event_sub_type;
	wlan_diag_event.eapol_packet_type = eapol_params.eapol_packet_type;
	wlan_diag_event.eapol_key_info = eapol_params.eapol_key_info;
	wlan_diag_event.eapol_rate = eapol_params.eapol_rate;
	qdf_mem_copy(wlan_diag_event.dest_addr,
			eapol_params.dest_addr,
			sizeof(wlan_diag_event.dest_addr));
	qdf_mem_copy(wlan_diag_event.src_addr,
			eapol_params.src_addr,
			sizeof(wlan_diag_event.src_addr));

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event, EVENT_WLAN_EAPOL);
}

/**
 * wlan_hdd_log_eapol() - Logs the EAPOL parameters of a packet
 * @skb:               skb data
 * @event_type:        One of enum wifi_connectivity_events to indicate Tx/Rx
 *
 * This function parses the input skb data to get the EAPOL params and log
 * them to user space, if the packet is EAPOL
 *
 * Return: None
 *
 */
void wlan_hdd_log_eapol(struct sk_buff *skb,
		uint8_t event_type)
{
	int ret;
	struct host_event_wlan_eapol eapol_params;

	ret = wlan_hdd_get_eapol_params(skb, &eapol_params, event_type);
	if (!ret)
		wlan_hdd_event_eapol_log(eapol_params);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

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
static void wlan_hdd_update_queue_oper_stats(hdd_adapter_t *adapter,
	enum netif_action_type action, enum netif_reason_type reason)
{
	switch (action) {
	case WLAN_STOP_ALL_NETIF_QUEUE:
	case WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER:
	case WLAN_NETIF_TX_DISABLE:
	case WLAN_NETIF_TX_DISABLE_N_CARRIER:
		adapter->queue_oper_stats[reason].pause_count++;
		break;
	case WLAN_START_ALL_NETIF_QUEUE:
	case WLAN_WAKE_ALL_NETIF_QUEUE:
	case WLAN_START_ALL_NETIF_QUEUE_N_CARRIER:
		adapter->queue_oper_stats[reason].unpause_count++;
		break;
	default:
		break;
	}

	return;
}

/**
 * wlan_hdd_update_txq_timestamp() - update txq timestamp
 * @dev: net device
 *
 * Return: none
 */
void wlan_hdd_update_txq_timestamp(struct net_device *dev)
{
	struct netdev_queue *txq;
	int i;
	bool unlock;

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		txq = netdev_get_tx_queue(dev, i);
		unlock = __netif_tx_trylock(txq);
		txq_trans_update(txq);
		if (unlock == true)
			__netif_tx_unlock(txq);
	}
}

/**
 * wlan_hdd_update_unpause_time() - update unpause time
 * @adapter: adapter handle
 *
 * Return: none
 */
static void wlan_hdd_update_unpause_time(hdd_adapter_t *adapter)
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
static void wlan_hdd_update_pause_time(hdd_adapter_t *adapter)
{
	qdf_time_t curr_time = qdf_system_ticks();

	adapter->total_pause_time += curr_time - adapter->last_time;
	adapter->last_time = curr_time;
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
void wlan_hdd_netif_queue_control(hdd_adapter_t *adapter,
	enum netif_action_type action, enum netif_reason_type reason)
{

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

	case WLAN_START_ALL_NETIF_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_start_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_WAKE_ALL_NETIF_QUEUE:
		spin_lock_bh(&adapter->pause_map_lock);
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_wake_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter);
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
		adapter->pause_map &= ~(1 << reason);
		if (!adapter->pause_map) {
			netif_tx_start_all_queues(adapter->dev);
			wlan_hdd_update_pause_time(adapter);
		}
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_NETIF_TX_DISABLE:
		spin_lock_bh(&adapter->pause_map_lock);
		if (!adapter->pause_map) {
			netif_tx_disable(adapter->dev);
			wlan_hdd_update_txq_timestamp(adapter->dev);
			wlan_hdd_update_unpause_time(adapter);
		}
		adapter->pause_map |= (1 << reason);
		spin_unlock_bh(&adapter->pause_map_lock);
		break;

	case WLAN_NETIF_TX_DISABLE_N_CARRIER:
		spin_lock_bh(&adapter->pause_map_lock);
		if (!adapter->pause_map) {
			netif_tx_disable(adapter->dev);
			wlan_hdd_update_txq_timestamp(adapter->dev);
			wlan_hdd_update_unpause_time(adapter);
		}
		adapter->pause_map |= (1 << reason);
		netif_carrier_off(adapter->dev);
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

	return;
}

/**
 * hdd_set_mon_rx_cb() - Set Monitor mode Rx callback
 * @dev:        Pointer to net_device structure
 *
 * Return: 0 for success; non-zero for failure
 */
int hdd_set_mon_rx_cb(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx =  WLAN_HDD_GET_CTX(adapter);
	int ret;
	QDF_STATUS qdf_status;
	struct ol_txrx_desc_type sta_desc = {0};
	struct ol_txrx_ops txrx_ops;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_mon_rx_packet_cbk;
	ol_txrx_vdev_register(
		 ol_txrx_get_vdev_from_vdev_id(adapter->sessionId),
		 adapter, &txrx_ops);
	/* peer is created wma_vdev_attach->wma_create_peer */
	qdf_status = ol_txrx_register_peer(&sta_desc);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("WLANTL_RegisterSTAClient() failed to register. Status= %d [0x%08X]",
			qdf_status, qdf_status);
		goto exit;
	}

	qdf_status = sme_create_mon_session(hdd_ctx->hHal,
				     adapter->macAddressCurrent.bytes);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_err("sme_create_mon_session() failed to register. Status= %d [0x%08X]",
			qdf_status, qdf_status);
	}
exit:
	ret = qdf_status_to_os_return(qdf_status);
	return ret;
}
