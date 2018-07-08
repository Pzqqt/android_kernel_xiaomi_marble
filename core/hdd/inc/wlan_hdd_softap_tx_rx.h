/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_SOFTAP_TX_RX_H)
#define WLAN_HDD_SOFTAP_TX_RX_H

/**
 * DOC: wlan_hdd_softap_tx_rx.h
 *
 * Linux HDD SOFTAP Tx/Rx APIs
 */

#include <wlan_hdd_hostapd.h>
#include <cdp_txrx_peer_ops.h>

/**
 * hdd_softap_hard_start_xmit() - Transmit a frame
 * @skb: pointer to OS packet
 * @dev: pointer to net_device structure
 *
 * Function registered as a net_device .ndo_start_xmit() method for
 * master mode interfaces (SoftAP/P2P GO), called by the OS if any
 * packet needs to be transmitted.
 *
 * Return: Status of the transmission
 */
netdev_tx_t hdd_softap_hard_start_xmit(struct sk_buff *skb,
				       struct net_device *dev);

/**
 * hdd_softap_tx_timeout() - TX timeout handler
 * @dev: pointer to network device
 *
 * Function registered as a net_device .ndo_tx_timeout() method for
 * master mode interfaces (SoftAP/P2P GO), called by the OS if the
 * driver takes too long to transmit a frame.
 *
 * Return: None
 */
void hdd_softap_tx_timeout(struct net_device *dev);

/**
 * hdd_softap_init_tx_rx() - Initialize Tx/Rx module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_init_tx_rx(struct hdd_adapter *adapter);

/**
 * hdd_softap_deinit_tx_rx() - Deinitialize Tx/Rx module
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx(struct hdd_adapter *adapter);

/**
 * hdd_softap_init_tx_rx_sta() - Initialize Tx/Rx for a softap station
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
 * hdd_softap_deinit_tx_rx_sta() - Deinitialize Tx/Rx for a softap station
 * @adapter: pointer to adapter context
 * @sta_id: Station ID to deinitialize
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_deinit_tx_rx_sta(struct hdd_adapter *adapter,
				       uint8_t sta_id);

/**
 * hdd_softap_rx_packet_cbk() - Receive packet handler
 * @context: pointer to HDD context
 * @rx_buf: pointer to rx qdf_nbuf chain
 *
 * Receive callback registered with the Data Path.  The Data Path will
 * call this to notify the HDD when one or more packets were received
 * for a registered STA.
 *
 * Return: QDF_STATUS_E_FAILURE if any errors encountered,
 *	   QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS hdd_softap_rx_packet_cbk(void *context, qdf_nbuf_t rx_buf);

/**
 * hdd_softap_deregister_sta() - Deregister a STA with the Data Path
 * @adapter: pointer to adapter context
 * @sta_id: Station ID to deregister
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_deregister_sta(struct hdd_adapter *adapter,
				     uint8_t sta_id);

/**
 * hdd_softap_register_sta() - Register a SoftAP STA
 * @adapter: pointer to adapter context
 * @auth_required: is additional authentication required?
 * @privacy_required: should 802.11 privacy bit be set?
 * @sta_id: station ID assigned to this station
 * @sta_mac: station MAC address
 * @wmm_enabled: is WMM enabled for this STA?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_sta(struct hdd_adapter *adapter,
				   bool auth_required,
				   bool privacy_required,
				   uint8_t sta_id,
				   struct qdf_mac_addr *sta_mac,
				   bool wmm_enabled);

/**
 * hdd_softap_register_bc_sta() - Register the SoftAP broadcast STA
 * @adapter: pointer to adapter context
 * @privacy_required: should 802.11 privacy bit be set?
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_register_bc_sta(struct hdd_adapter *adapter,
				      bool privacy_required);

/**
 * hdd_softap_stop_bss() - Stop the BSS
 * @adapter: pointer to adapter context
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_stop_bss(struct hdd_adapter *adapter);

/**
 * hdd_softap_change_sta_state() - Change the state of a SoftAP station
 * @adapter: pointer to adapter context
 * @sta_mac: MAC address of the station
 * @state: new state of the station
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS hdd_softap_change_sta_state(struct hdd_adapter *adapter,
				       struct qdf_mac_addr *sta_mac,
				       enum ol_txrx_peer_state state);

/**
 * hdd_softap_get_sta_id() - Find station ID from MAC address
 * @adapter: pointer to adapter context
 * @sta_mac: MAC address of the destination
 * @sta_id: Station ID associated with the MAC address
 *
 * Return: QDF_STATUS_SUCCESS if a match was found, in which case
 *	   @sta_id is populated, QDF_STATUS_E_FAILURE if a match is
 *	   not found
 */
QDF_STATUS hdd_softap_get_sta_id(struct hdd_adapter *adapter,
				 struct qdf_mac_addr *sta_mac,
				 uint8_t *sta_id);

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
void hdd_softap_tx_resume_timer_expired_handler(void *adapter_context);

/**
 * hdd_softap_tx_resume_cb() - Resume OS TX Q.
 * @adapter_context: pointer to vdev apdapter
 * @tx_resume: TX Q resume trigger
 *
 * Q was stopped due to WLAN TX path low resource condition
 *
 * Return: None
 */
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

/**
 * hdd_post_dhcp_ind() - Send DHCP START/STOP indication to FW
 * @adapter: pointer to hdd adapter
 * @sta_id: peer station ID
 * @type: WMA message type
 *
 * Return: error number
 */
int hdd_post_dhcp_ind(struct hdd_adapter *adapter,
		      uint8_t sta_id, uint16_t type);

/**
 * hdd_inspect_dhcp_packet() -  Inspect DHCP packet
 * @adapter: pointer to hdd adapter
 * @sta_id: peer station ID
 * @skb: pointer to OS packet (sk_buff)
 * @dir: direction
 *
 * Return: error number
 */
int hdd_inspect_dhcp_packet(struct hdd_adapter *adapter,
			    uint8_t sta_id,
			    struct sk_buff *skb,
			    enum qdf_proto_dir dir);

#endif /* end #if !defined(WLAN_HDD_SOFTAP_TX_RX_H) */
