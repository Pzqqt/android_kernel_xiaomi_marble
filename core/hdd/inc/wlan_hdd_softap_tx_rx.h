/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_SOFTAP_TX_RX_H)
#define WLAN_HDD_SOFTAP_TX_RX_H

/**
 * DOC: wlan_hdd_softap_tx_rx.h
 *
 *Linux HDD SOFTAP Tx/RX APIs
 */

#include <wlan_hdd_hostapd.h>
#include <cdp_txrx_peer_ops.h>

int hdd_softap_hard_start_xmit(struct sk_buff *skb,
			       struct net_device *dev);
void hdd_softap_tx_timeout(struct net_device *dev);
QDF_STATUS hdd_softap_init_tx_rx(struct hdd_adapter *adapter);
QDF_STATUS hdd_softap_deinit_tx_rx(struct hdd_adapter *adapter);

/**
 * hdd_softap_init_tx_rx_sta() - Initialize tx/rx for a softap station
 * @adapter: pointer to adapter context
 * @sta_id: Station ID to initialize
 * @sta_mac: pointer to the MAC address of the station
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_init_tx_rx_sta(struct hdd_adapter *adapter,
				     uint8_t sta_id,
				     struct qdf_mac_addr *sta_mac);

/**
 * hdd_softap_deinit_tx_rx_sta() - Deinitialize tx/rx for a softap station
 * @adapter: pointer to adapter context
 * @sta_id: Station ID to deinitialize
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx_sta(struct hdd_adapter *adapter,
				       uint8_t sta_id);

QDF_STATUS hdd_softap_rx_packet_cbk(void *context, qdf_nbuf_t rxBufChain);
#ifdef IPA_OFFLOAD
QDF_STATUS hdd_softap_rx_mul_packet_cbk(void *cds_context,
					qdf_nbuf_t rx_buf_list, uint8_t staId);
#endif /* IPA_OFFLOAD */

QDF_STATUS hdd_softap_deregister_sta(struct hdd_adapter *adapter,
				     uint8_t staId);

/**
 * hdd_softap_register_sta() - Register a SoftAP STA
 * @adapter: pointer to adapter context
 * @fAuthRequired: is additional authentication required?
 * @fPrivacyBit: should 802.11 privacy bit be set?
 * @staId: station ID assigned to this station
 * @pPeerMacAddress: station MAC address
 * @fWmmEnabled: is WMM enabled for this STA?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_sta(struct hdd_adapter *adapter,
				   bool fAuthRequired,
				   bool fPrivacyBit,
				   uint8_t staId,
				   struct qdf_mac_addr *pPeerMacAddress,
				   bool fWmmEnabled);

QDF_STATUS hdd_softap_register_bc_sta(struct hdd_adapter *adapter,
				      bool fPrivacyBit);
QDF_STATUS hdd_softap_deregister_bc_sta(struct hdd_adapter *adapter);
QDF_STATUS hdd_softap_stop_bss(struct hdd_adapter *adapter);
QDF_STATUS hdd_softap_change_sta_state(struct hdd_adapter *adapter,
				       struct qdf_mac_addr *pDestMacAddress,
				       enum ol_txrx_peer_state state);
QDF_STATUS hdd_softap_get_sta_id(struct hdd_adapter *adapter,
				 struct qdf_mac_addr *pMacAddress,
				 uint8_t *staId);

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
void hdd_softap_tx_resume_timer_expired_handler(void *adapter_context);
void hdd_softap_tx_resume_cb(void *adapter_context, bool tx_resume);
#else
static inline
void hdd_softap_tx_resume_timer_expired_handler(void *adapter_context)
{
}
static inline
void hdd_softap_tx_resume_cb(void *adapter_context, bool tx_resume)
{
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

#endif /* end #if !defined(WLAN_HDD_SOFTAP_TX_RX_H) */
