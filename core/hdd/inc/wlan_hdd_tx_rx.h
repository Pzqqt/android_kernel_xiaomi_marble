/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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
#endif

#define HDD_PSB_CFG_INVALID                   0xFF
#define HDD_PSB_CHANGED                       0xFF
#define SME_QOS_UAPSD_CFG_BK_CHANGED_MASK     0xF1
#define SME_QOS_UAPSD_CFG_BE_CHANGED_MASK     0xF2
#define SME_QOS_UAPSD_CFG_VI_CHANGED_MASK     0xF4
#define SME_QOS_UAPSD_CFG_VO_CHANGED_MASK     0xF8

int hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
void hdd_tx_timeout(struct net_device *dev);

QDF_STATUS hdd_init_tx_rx(struct hdd_adapter *pAdapter);
QDF_STATUS hdd_deinit_tx_rx(struct hdd_adapter *pAdapter);
QDF_STATUS hdd_rx_packet_cbk(void *context, qdf_nbuf_t rxBuf);

QDF_STATUS hdd_get_peer_sta_id(struct hdd_station_ctx *sta_ctx,
				struct qdf_mac_addr *peer_mac_addr,
				uint8_t *sta_id);

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

int hdd_get_peer_idx(struct hdd_station_ctx *sta_ctx, struct qdf_mac_addr *addr);

const char *hdd_reason_type_to_string(enum netif_reason_type reason);
const char *hdd_action_type_to_string(enum netif_action_type action);
void wlan_hdd_netif_queue_control(struct hdd_adapter *adapter,
		enum netif_action_type action, enum netif_reason_type reason);
int hdd_set_mon_rx_cb(struct net_device *dev);
void hdd_send_rps_ind(struct hdd_adapter *adapter);
void wlan_hdd_classify_pkt(struct sk_buff *skb);

#ifdef MSM_PLATFORM
void hdd_reset_tcp_delack(struct hdd_context *hdd_ctx);
#else
static inline void hdd_reset_tcp_delack(struct hdd_context *hdd_ctx) {}
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

#endif /* end #if !defined(WLAN_HDD_TX_RX_H) */
