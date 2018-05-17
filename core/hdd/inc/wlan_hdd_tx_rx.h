/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_TX_RX_H)
#define WLAN_HDD_TX_RX_H

/**
 *
 * DOC: wlan_hdd_tx_rx.h
 *
 * Linux HDD Tx/RX APIs
 */

#include <wlan_hdd_includes.h>
#include <cds_api.h>
#include <linux/skbuff.h>
#include "cdp_txrx_flow_ctrl_legacy.h"

struct hdd_context;

#define HDD_ETHERTYPE_802_1_X              0x888E
#define HDD_ETHERTYPE_802_1_X_FRAME_OFFSET 12
#ifdef FEATURE_WLAN_WAPI
#define HDD_ETHERTYPE_WAI                  0x88b4
#define IS_HDD_ETHERTYPE_WAI(_skb) (ntohs(_skb->protocol) == \
					HDD_ETHERTYPE_WAI)
#else
#define IS_HDD_ETHERTYPE_WAI(_skb) (false)
#endif

#define HDD_PSB_CFG_INVALID                   0xFF
#define HDD_PSB_CHANGED                       0xFF
#define SME_QOS_UAPSD_CFG_BK_CHANGED_MASK     0xF1
#define SME_QOS_UAPSD_CFG_BE_CHANGED_MASK     0xF2
#define SME_QOS_UAPSD_CFG_VI_CHANGED_MASK     0xF4
#define SME_QOS_UAPSD_CFG_VO_CHANGED_MASK     0xF8

netdev_tx_t hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
void hdd_tx_timeout(struct net_device *dev);

QDF_STATUS hdd_init_tx_rx(struct hdd_adapter *adapter);
QDF_STATUS hdd_deinit_tx_rx(struct hdd_adapter *adapter);
QDF_STATUS hdd_rx_packet_cbk(void *context, qdf_nbuf_t rxBuf);

/**
 * hdd_rx_ol_init() - Initialize Rx mode(LRO or GRO) method
 * @hdd_ctx: pointer to HDD Station Context
 *
 * Return: 0 on success and non zero on failure.
 */
int hdd_rx_ol_init(struct hdd_context *hdd_ctx);

/**
 * hdd_disable_rx_ol_in_concurrency() - Disable Rx offload due to concurrency
 * @disable: true/false to disable/enable the Rx offload
 *
 * Return: none
 */
void hdd_disable_rx_ol_in_concurrency(bool disable);

/**
 * hdd_disable_rx_ol_for_low_tput() - Disable Rx offload in low TPUT scenario
 * @hdd_ctx: hdd context
 * @disable: true/false to disable/enable the Rx offload
 *
 * Return: none
 */
void hdd_disable_rx_ol_for_low_tput(struct hdd_context *hdd_ctx, bool disable);

QDF_STATUS hdd_get_peer_sta_id(struct hdd_station_ctx *sta_ctx,
				struct qdf_mac_addr *peer_mac_addr,
				uint8_t *sta_id);
/**
 * hdd_reset_all_adapters_connectivity_stats() - reset connectivity stats
 * @hdd_ctx: pointer to HDD Station Context
 *
 * Return: None
 */
void hdd_reset_all_adapters_connectivity_stats(struct hdd_context *hdd_ctx);

/**
 * hdd_tx_rx_collect_connectivity_stats_info() - collect connectivity stats
 * @skb: pointer to skb data
 * @adapter: pointer to vdev apdapter
 * @action: action done on pkt.
 * @pkt_type: data pkt type
 *
 * Return: None
 */
void hdd_tx_rx_collect_connectivity_stats_info(struct sk_buff *skb,
		void *adapter, enum connectivity_stats_pkt_status action,
		uint8_t *pkt_type);

/**
 * hdd_tx_queue_cb() - Disable/Enable the Transmit Queues
 * @context: HDD context
 * @vdev_id: vdev id
 * @action: Action to be taken on the Tx Queues
 * @reason: Reason for the netif action
 *
 * Return: None
 */
void hdd_tx_queue_cb(void *context, uint32_t vdev_id,
		     enum netif_action_type action,
		     enum netif_reason_type reason);

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
void hdd_tx_resume_cb(void *adapter_context, bool tx_resume);

/**
 * hdd_tx_flow_control_is_pause() - Is TX Q paused by flow control
 * @adapter_context: pointer to vdev apdapter
 *
 * Return: true if TX Q is paused by flow control
 */
bool hdd_tx_flow_control_is_pause(void *adapter_context);
void hdd_tx_resume_timer_expired_handler(void *adapter_context);

/**
 * hdd_register_tx_flow_control() - Register TX Flow control
 * @adapter: adapter handle
 * @timer_callback: timer callback
 * @flow_control_fp: txrx flow control
 * @flow_control_is_pause_fp: is txrx paused by flow control
 *
 * Return: none
 */
void hdd_register_tx_flow_control(struct hdd_adapter *adapter,
		qdf_mc_timer_callback_t timer_callback,
		ol_txrx_tx_flow_control_fp flowControl,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause);
void hdd_deregister_tx_flow_control(struct hdd_adapter *adapter);
void hdd_get_tx_resource(struct hdd_adapter *adapter,
			uint8_t STAId, uint16_t timer_value);

#else
static inline void hdd_tx_resume_cb(void *adapter_context, bool tx_resume)
{
}
static inline bool hdd_tx_flow_control_is_pause(void *adapter_context)
{
	return false;
}
static inline void hdd_tx_resume_timer_expired_handler(void *adapter_context)
{
}
static inline void hdd_register_tx_flow_control(struct hdd_adapter *adapter,
		qdf_mc_timer_callback_t timer_callback,
		ol_txrx_tx_flow_control_fp flowControl,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause)
{
}
static inline void hdd_deregister_tx_flow_control(struct hdd_adapter *adapter)
{
}
static inline void hdd_get_tx_resource(struct hdd_adapter *adapter,
			uint8_t STAId, uint16_t timer_value)
{
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

int hdd_get_peer_idx(struct hdd_station_ctx *sta_ctx,
		     struct qdf_mac_addr *addr);

const char *hdd_reason_type_to_string(enum netif_reason_type reason);
const char *hdd_action_type_to_string(enum netif_action_type action);
void wlan_hdd_netif_queue_control(struct hdd_adapter *adapter,
		enum netif_action_type action, enum netif_reason_type reason);

#ifdef FEATURE_MONITOR_MODE_SUPPORT
int hdd_set_mon_rx_cb(struct net_device *dev);
#else
static inline
int hdd_set_mon_rx_cb(struct net_device *dev)
{
	return 0;
}
#endif

void hdd_send_rps_ind(struct hdd_adapter *adapter);
void hdd_send_rps_disable_ind(struct hdd_adapter *adapter);
void wlan_hdd_classify_pkt(struct sk_buff *skb);

#ifdef MSM_PLATFORM
void hdd_reset_tcp_delack(struct hdd_context *hdd_ctx);
#define HDD_MSM_CFG(msm_cfg)	msm_cfg
#else
static inline void hdd_reset_tcp_delack(struct hdd_context *hdd_ctx) {}
#define HDD_MSM_CFG(msm_cfg)	0
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void hdd_event_eapol_log(struct sk_buff *skb, enum qdf_proto_dir dir);
#else
static inline
void hdd_event_eapol_log(struct sk_buff *skb, enum qdf_proto_dir dir)
{}
#endif

/*
 * As of the 4.7 kernel, net_device->trans_start is removed. Create shims to
 * support compiling against older versions of the kernel.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
static inline void netif_trans_update(struct net_device *dev)
{
	dev->trans_start = jiffies;
}

#define TX_TIMEOUT_TRACE(dev, module_id) QDF_TRACE( \
	module_id, QDF_TRACE_LEVEL_ERROR, \
	"%s: Transmission timeout occurred jiffies %lu trans_start %lu", \
	__func__, jiffies, dev->trans_start)
#else
#define TX_TIMEOUT_TRACE(dev, module_id) QDF_TRACE( \
	module_id, QDF_TRACE_LEVEL_ERROR, \
	"%s: Transmission timeout occurred jiffies %lu", \
	__func__, jiffies)
#endif

static inline void
hdd_skb_fill_gso_size(struct net_device *dev, struct sk_buff *skb)
{
	if (skb_cloned(skb) && skb_is_nonlinear(skb) &&
	    skb_shinfo(skb)->gso_size == 0 &&
	    ip_hdr(skb)->protocol == IPPROTO_TCP) {
		skb_shinfo(skb)->gso_size = dev->mtu -
			((skb_transport_header(skb) - skb_network_header(skb))
				+ tcp_hdrlen(skb));
	}
}

/**
 * hdd_txrx_get_tx_ack_count() - get tx acked count
 * @adapter: Pointer to adapter
 *
 * Return: tx acked count
 */
uint32_t hdd_txrx_get_tx_ack_count(struct hdd_adapter *adapter);

#ifdef CONFIG_HL_SUPPORT
static inline QDF_STATUS
hdd_skb_nontso_linearize(struct sk_buff *skb)
{
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
hdd_skb_nontso_linearize(struct sk_buff *skb)
{
	if (qdf_nbuf_is_nonlinear(skb) && qdf_nbuf_is_tso(skb) == false) {
		if (qdf_unlikely(skb_linearize(skb)))
			return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}
#endif

#endif /* end #if !defined(WLAN_HDD_TX_RX_H) */
