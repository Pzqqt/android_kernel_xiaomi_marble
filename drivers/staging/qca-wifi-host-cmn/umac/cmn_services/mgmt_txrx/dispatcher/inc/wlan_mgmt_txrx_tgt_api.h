/*
 * Copyright (c) 2016-2018, 2021 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_MGMT_TXRX_TGT_API_H_
#define _WLAN_MGMT_TXRX_TGT_API_H_

/**
 * DOC:  wlan_mgmt_txrx_tgt_api.h
 *
 * management tx/rx layer public API and structures for
 * umac southbound interface.
 *
 */

#include "wlan_objmgr_cmn.h"
#include <wlan_objmgr_pdev_obj.h>
#include "wlan_mgmt_txrx_utils_api.h"
#include "qdf_nbuf.h"
#include <wlan_lmac_if_def.h>

/**
 * tgt_mgmt_txrx_rx_frame_handler() - handles rx mgmt. frames
 * @psoc: psoc context
 * @buf: buffer
 * @mgmt_rx_params: rx event params
 *
 * This function handles mgmt. rx frames and is registered to southbound
 * interface through rx ops.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_mgmt_txrx_rx_frame_handler(
			struct wlan_objmgr_psoc *psoc,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);

/**
 * tgt_mgmt_txrx_tx_completion_handler() - handles mgmt. tx completions
 * @pdev: pdev context
 * @desc_id: mgmt desc. id
 * @status: status of download of tx packet
 * @tx_compl_params: tx completion params
 *
 * This function handles tx completions of mgmt. frames and is registered to
 * LMAC_if layer through lmac_if cbs.The cb needs to free the nbuf. In case no
 * callback is registered, this function will free the nbuf.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_mgmt_txrx_tx_completion_handler(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id, uint32_t status,
			void *tx_compl_params);

/**
 * tgt_mgmt_txrx_get_nbuf_from_desc_id() - extracts nbuf from mgmt desc
 * @pdev: pdev context
 * @desc_id: desc_id
 *
 * This function extracts nbuf from mgmt desc extracted from desc id.
 *
 * Return: nbuf - in case of success
 *         NULL - in case of failure
 */
qdf_nbuf_t tgt_mgmt_txrx_get_nbuf_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id);

/**
 * tgt_mgmt_txrx_get_peer_from_desc_id() - extracts peer from mgmt desc
 * @pdev: pdev context
 * @desc_id: desc_id
 *
 * This function extracts peer from mgmt desc extracted from desc id.
 *
 * Return: peer - in case of success
 *         NULL - in case of failure
 */
struct wlan_objmgr_peer *
tgt_mgmt_txrx_get_peer_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id);

/**
 * tgt_mgmt_txrx_get_vdev_id_from_desc_id() - extracts vdev id from mgmt desc
 * @pdev: pdev context
 * @desc_id: desc_id
 *
 * This function extracts vdev id from mgmt desc extracted from desc id.
 *
 * Return: vdev_id - in case of success
 *         WLAN_UMAC_VDEV_ID_MAX - in case of failure
 */
uint8_t tgt_mgmt_txrx_get_vdev_id_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id);

/**
 * tgt_mgmt_txrx_get_free_desc_pool_count() - get free mgmt desc count
 * @pdev: pdev context
 *
 * This function returns the count of free mgmt descriptors.
 *
 * Return:  free descpriptor count
 */
uint32_t tgt_mgmt_txrx_get_free_desc_pool_count(
			struct wlan_objmgr_pdev *pdev);

/**
 * tgt_mgmt_txrx_register_ev_handler() - Register to mgmt txrx WMI events
 * @psoc: Pointer to psoc object
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
tgt_mgmt_txrx_register_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_mgmt_txrx_unregister_ev_handler() - Unregister to mgmt txrx WMI events
 * @psoc: Pointer to psoc object
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
tgt_mgmt_txrx_unregister_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_psoc_get_mgmt_txrx_txops() - Get txops of MGMT TxRx module using psoc
 * @psoc: Pointer to psoc object
 *
 * Return: txops of MGMT TxRx module on success, otherwise NULL
 */
static inline struct wlan_lmac_if_mgmt_txrx_tx_ops *
wlan_psoc_get_mgmt_txrx_txops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		mgmt_txrx_err("psoc is null");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		mgmt_txrx_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->mgmt_txrx_tx_ops;
}

/**
 * wlan_pdev_get_mgmt_txrx_txops() - Get txops of MGMT TxRx module using pdev
 * @pdev: Pointer to pdev object
 *
 * Return: txops of MGMT TxRx module on success, otherwise NULL
 */
static inline struct wlan_lmac_if_mgmt_txrx_tx_ops *
wlan_pdev_get_mgmt_txrx_txops(struct wlan_objmgr_pdev *pdev)
{
	if (!pdev) {
		mgmt_txrx_err("pdev is null");
		return NULL;
	}

	return wlan_psoc_get_mgmt_txrx_txops(wlan_pdev_get_psoc(pdev));
}

/**
 * tgt_mgmt_txrx_process_rx_frame() - Process management rx frames
 * @pdev: pdev for which this management frame is intended
 * @buf: buffer
 * @mgmt_rx_params: rx event params
 *
 * This API processes MGMT Rx frames and delivers them to the upper layers.
 *
 * Return: QDF_STATUS of operation.
 */
QDF_STATUS tgt_mgmt_txrx_process_rx_frame(
			struct wlan_objmgr_pdev *pdev,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);

/**
 * tgt_mgmt_txrx_rx_frame_entry() - Entry point to the MGMT TxRx module for
 * management Rx frames.
 * @pdev: pdev for which this management frame is intended
 * @buf: buffer
 * @mgmt_rx_params: rx event params
 *
 * Return: QDF_STATUS of operation.
 */
QDF_STATUS tgt_mgmt_txrx_rx_frame_entry(
			struct wlan_objmgr_pdev *pdev,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);
#endif
