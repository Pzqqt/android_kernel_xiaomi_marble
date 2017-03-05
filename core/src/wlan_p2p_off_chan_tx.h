/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Defines off channel tx API & structures
 */

#ifndef _WLAN_P2P_OFF_CHAN_TX_H_
#define _WLAN_P2P_OFF_CHAN_TX_H_

#include <qdf_types.h>
#include <qdf_mc_timer.h>

struct p2p_soc_priv_obj;
struct cancel_roc_context;
struct p2p_tx_conf_event;
struct p2p_rx_mgmt_event;

/**
 * struct tx_action_context - tx action frame context
 * @node:           Node for next element in the list
 * @p2p_soc_obj:    Pointer to SoC global p2p private object
 * @vdev_id:        Vdev id on which this request has come
 * @scan_id:        Scan id given by scan component for this roc req
 * @action_cookie:  Action cookie
 * @chan:           Chan for which this tx has been requested
 * @buf:            tx buffer
 * @buf_len:        Length of tx buffer
 * @off_chan:       Is this off channel tx
 * @no_cck:         Required cck or not
 * @no_ack:         Required ack or not
 * @duration:       Duration for the RoC
 * @tx_timer:       RoC timer
 */
struct tx_action_context {
	qdf_list_node_t node;
	struct p2p_soc_priv_obj *p2p_soc_obj;
	int vdev_id;
	int scan_id;
	uint64_t action_cookie;
	uint8_t chan;
	uint8_t *buf;
	int buf_len;
	bool off_chan;
	bool no_cck;
	bool no_ack;
	uint32_t duration;
	qdf_mc_timer_t tx_timer;
};

/**
 * p2p_process_mgmt_tx() - Process mgmt frame tx request
 * @tx_ctx: tx context
 *
 * This function handles mgmt frame tx request. It will call API from
 * mgmt txrx component.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_mgmt_tx(struct tx_action_context *tx_ctx);

/**
 * p2p_process_mgmt_tx_cancel() - Process cancel mgmt frame tx request
 * @cancel_tx: cancel tx context
 *
 * This function cancel mgmt frame tx request by cookie.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_mgmt_tx_cancel(
	struct cancel_roc_context *cancel_tx);

/**
 * p2p_process_mgmt_tx_ack_cnf() - Process tx ack event
 * @tx_cnf_event: tx confirmation event information
 *
 * This function mgmt frame tx confirmation. It will deliver this
 * event to HDD
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_mgmt_tx_ack_cnf(
	struct p2p_tx_conf_event *tx_cnf_event);

/**
 * p2p_process_rx_mgmt() - Process rx mgmt frame event
 * @rx_mgmt_event: rx mgmt frame event information
 *
 * This function mgmt frame rx mgmt frame event. It will deliver this
 * event to HDD
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_rx_mgmt(
	struct p2p_rx_mgmt_event *rx_mgmt_event);

#endif /* _WLAN_P2P_OFF_CHAN_TX_H_ */
