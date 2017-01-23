/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_LMAC_IF_API_H_
#define _WLAN_LMAC_IF_API_H_

#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_psoc_obj.h"

/**
 * wlan_lmac_if_open() - lmac_if open
 * @psoc: psoc context
 *
 * Opens up lmac_if southbound layer. This function calls OL,DA and UMAC
 * modules to register respective tx and rx callbacks.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_lmac_if_open(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_lmac_if_register_rx_handlers() - UMAC rx handler register
 * @rx_ops: Pointer to rx_ops structure to be populated
 *
 * Register umac RX callabacks which will be called by DA/OL/WMA/WMI
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_register_rx_handlers
		(struct wlan_lmac_if_rx_ops *rx_ops);

/**
 * wlan_lmac_if_close() - Close lmac_if
 * @psoc: psoc context
 *
 * Deregister lmac_if TX and RX handlers
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_lmac_if_assign_tx_registration_cb() -tx registration callback assignment
 * @dev_type: Dev type can be either Direct attach or Offload
 * @handler: handler to be called for LMAC tx ops registration
 *
 * API to assign appropriate tx registration callback handler based on the
 * device type(Offload or Direct attach)
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_assign_tx_registration_cb(WLAN_DEV_TYPE dev_type,
		QDF_STATUS (*handler)(struct wlan_lmac_if_tx_ops *));


/**
 * wlan_lmac_if_get_mgmt_txrx_rx_ops() - retrieve the mgmt rx_ops
 * @psoc: psoc context
 *
 * API to retrieve the mgmt rx_ops from the psoc context
 *
 * Return: mgmt_rx_ops pointer
 */
static inline struct wlan_lmac_if_mgmt_txrx_rx_ops *
wlan_lmac_if_get_mgmt_txrx_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return &psoc->soc_cb.rx_ops.mgmt_txrx_rx_ops;
}

/**
 * mgmt_txrx_get_nbuf() - retrieve nbuf from mgmt desc_id
 * @psoc: psoc context
 * @desc_id: mgmt desc_id
 *
 * API to retrieve the nbuf from mgmt desc_id
 *
 * Return: nbuf
 */
static inline qdf_nbuf_t
mgmt_txrx_get_nbuf(struct wlan_objmgr_psoc *psoc, uint32_t desc_id)
{
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops =
				wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (mgmt_rx_ops && mgmt_rx_ops->mgmt_txrx_get_nbuf_from_desc_id)
		return mgmt_rx_ops->mgmt_txrx_get_nbuf_from_desc_id(psoc,
								     desc_id);

	return NULL;
}

/**
 * mgmt_txrx_tx_completion_handler() - mgmt tx completion handler
 * @psoc: psoc context
 * @desc_id: mgmt desc_id
 * @status: tx status
 * @params: tx params
 *
 * API to handle the tx completion for mgmt frames
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static inline QDF_STATUS
mgmt_txrx_tx_completion_handler(struct wlan_objmgr_psoc *psoc,
				uint32_t desc_id, uint32_t status,
				void *params)
{
	qdf_nbuf_t nbuf;
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops =
				wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (mgmt_rx_ops && mgmt_rx_ops->mgmt_tx_completion_handler)
		return mgmt_rx_ops->mgmt_tx_completion_handler(psoc, desc_id,
							status, params);

	nbuf = mgmt_txrx_get_nbuf(psoc, desc_id);
	if (nbuf)
		qdf_nbuf_free(nbuf);

	return QDF_STATUS_E_NULL_VALUE;
}

/**
 * mgmt_txrx_rx_handler() - mgmt rx frame handler
 * @psoc: psoc context
 * @nbuf: nbuf
 * @params: rx params
 *
 * API to receive mgmt frames
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static inline QDF_STATUS
mgmt_txrx_rx_handler(struct wlan_objmgr_psoc *psoc, qdf_nbuf_t nbuf,
			void *params)
{
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops =
				wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (mgmt_rx_ops && mgmt_rx_ops->mgmt_rx_frame_handler)
		return mgmt_rx_ops->mgmt_rx_frame_handler(psoc, nbuf, params);

	if (nbuf)
		qdf_nbuf_free(nbuf);

	return QDF_STATUS_E_NULL_VALUE;
}

/**
 * mgmt_txrx_get_peer() - retrieve peer from mgmt desc_id
 * @psoc: psoc context
 * @desc_id: mgmt desc_id
 *
 * API to retrieve the peer from mgmt desc_id
 *
 * Return: objmgr peer pointer
 */
static inline struct wlan_objmgr_peer *
mgmt_txrx_get_peer(struct wlan_objmgr_psoc *psoc, uint32_t desc_id)
{
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops =
				wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (mgmt_rx_ops && mgmt_rx_ops->mgmt_txrx_get_peer_from_desc_id)
		return mgmt_rx_ops->mgmt_txrx_get_peer_from_desc_id(psoc,
								     desc_id);

	return NULL;
}

/**
 * mgmt_txrx_get_vdev_id() - retrieve vdev_id from mgmt desc_id
 * @psoc: psoc context
 * @desc_id: mgmt desc_id
 *
 * API to retrieve the vdev_id from mgmt desc_id
 *
 * Return: vdev_id
 */
static inline uint8_t
mgmt_txrx_get_vdev_id(struct wlan_objmgr_psoc *psoc, uint32_t desc_id)
{
	struct wlan_lmac_if_mgmt_txrx_rx_ops *mgmt_rx_ops =
				wlan_lmac_if_get_mgmt_txrx_rx_ops(psoc);
	if (mgmt_rx_ops && mgmt_rx_ops->mgmt_txrx_get_vdev_id_from_desc_id)
		return mgmt_rx_ops->mgmt_txrx_get_vdev_id_from_desc_id(psoc,
								   desc_id);

	return WLAN_UMAC_VDEV_ID_MAX;
}
#endif /* _WLAN_LMAC_IF_API_H */
