/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_flow_ctrl_legacy.h
 * @brief Define the host data path legacy flow control API
 * functions
 */
#ifndef _CDP_TXRX_FC_LEG_H_
#define _CDP_TXRX_FC_LEG_H_

/**
 * enum netif_action_type - Type of actions on netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE: stop all netif queues
 * @WLAN_START_ALL_NETIF_QUEUE: start all netif queues
 * @WLAN_WAKE_ALL_NETIF_QUEUE: wake all netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER: stop all queues and off carrier
 * @WLAN_START_ALL_NETIF_QUEUE_N_CARRIER: start all queues and on carrier
 * @WLAN_NETIF_TX_DISABLE: disable tx
 * @WLAN_NETIF_TX_DISABLE_N_CARRIER: disable tx and off carrier
 * @WLAN_NETIF_CARRIER_ON: on carrier
 * @WLAN_NETIF_CARRIER_OFF: off carrier
 */
enum netif_action_type {
	WLAN_STOP_ALL_NETIF_QUEUE = 1,
	WLAN_START_ALL_NETIF_QUEUE,
	WLAN_WAKE_ALL_NETIF_QUEUE,
	WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
	WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
	WLAN_NETIF_TX_DISABLE,
	WLAN_NETIF_TX_DISABLE_N_CARRIER,
	WLAN_NETIF_CARRIER_ON,
	WLAN_NETIF_CARRIER_OFF,
	WLAN_NETIF_ACTION_TYPE_MAX,
};

/**
 * enum netif_reason_type - reason for netif queue action
 * @WLAN_CONTROL_PATH: action from control path
 * @WLAN_DATA_FLOW_CONTROL: because of flow control
 * @WLAN_FW_PAUSE: because of firmware pause
 * @WLAN_TX_ABORT: because of tx abort
 * @WLAN_VDEV_STOP: because of vdev stop
 * @WLAN_PEER_UNAUTHORISED: because of peer is unauthorised
 * @WLAN_THERMAL_MITIGATION: because of thermal mitigation
 */
enum netif_reason_type {
	WLAN_CONTROL_PATH = 1,
	WLAN_DATA_FLOW_CONTROL,
	WLAN_FW_PAUSE,
	WLAN_TX_ABORT,
	WLAN_VDEV_STOP,
	WLAN_PEER_UNAUTHORISED,
	WLAN_THERMAL_MITIGATION,
	WLAN_REASON_TYPE_MAX,
};

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
/**
 * ol_txrx_tx_flow_control_fp - tx flow control notification
 * function from txrx to OS shim
 * @osif_dev - the virtual device's OS shim object
 * @tx_resume - tx os q should be resumed or not
 */
typedef void (*ol_txrx_tx_flow_control_fp)(void *osif_dev,
			 bool tx_resume);

int ol_txrx_register_tx_flow_control(uint8_t vdev_id,
		 ol_txrx_tx_flow_control_fp flowControl,
		 void *osif_fc_ctx);

int ol_txrx_deregister_tx_flow_control_cb(uint8_t vdev_id);

void ol_txrx_flow_control_cb(ol_txrx_vdev_handle vdev,
			 bool tx_resume);
bool
ol_txrx_get_tx_resource(uint8_t sta_id,
			 unsigned int low_watermark,
			 unsigned int high_watermark_offset);

int
ol_txrx_ll_set_tx_pause_q_depth(uint8_t vdev_id, int pause_q_depth);

#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

void ol_txrx_vdev_flush(ol_txrx_vdev_handle data_vdev);

#ifdef CONFIG_ICNSS
static inline void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	return;
}
#else
void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason);
#endif

#ifdef CONFIG_ICNSS
static inline void ol_txrx_vdev_unpause(ol_txrx_vdev_handle data_vdev,
					uint32_t reason)
{
	return;
}
#else
void ol_txrx_vdev_unpause(ol_txrx_vdev_handle data_vdev, uint32_t reason);
#endif

#endif /* _CDP_TXRX_FC_LEG_H_ */
