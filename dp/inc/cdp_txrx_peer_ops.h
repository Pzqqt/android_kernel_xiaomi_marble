/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_peer.h
 * @brief Define the host data path peer API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_PEER_H_
#define _CDP_TXRX_PEER_H_
#include <cdp_txrx_ops.h>
#include "cdp_txrx_handle.h"

/**
 * cdp_peer_register() - Register peer into physical device
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @sta_desc - peer description
 *
 * Register peer into physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_register(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		struct ol_txrx_desc_type *sta_desc)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->register_peer)
		return soc->ops->peer_ops->register_peer(pdev, sta_desc);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_clear_peer() - remove peer from physical device
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @sta_id - local peer id
 *
 * remove peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_clear_peer(ol_txrx_soc_handle soc, struct cdp_pdev *pdev, uint8_t sta_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->clear_peer)
		return soc->ops->peer_ops->clear_peer(pdev, sta_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_register_ocb_peer() - register ocb peer from physical device
 * @soc - data path soc handle
 * @cds_ctx - cds void context
 * @mac_addr - mac address for ocb self peer
 * @peer_id - local peer id
 *
 * register ocb peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_register_ocb_peer(ol_txrx_soc_handle soc,
		uint8_t *mac_addr, uint8_t *peer_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->register_ocb_peer)
		return soc->ops->peer_ops->register_ocb_peer(mac_addr, peer_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_remove_for_vdev() - remove peer instance from virtual interface
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 * @callback - remove done notification callback function pointer
 * @callback_context - callback caller context
 * @remove_last_peer - removed peer is last peer or not
 *
 * remove peer instance from virtual interface
 *
 * Return: NONE
 */
static inline void
cdp_peer_remove_for_vdev(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, ol_txrx_vdev_peer_remove_cb callback,
		void *callback_context, bool remove_last_peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->remove_peers_for_vdev)
		return soc->ops->peer_ops->remove_peers_for_vdev(
			vdev, callback, callback_context, remove_last_peer);

	return;
}

/**
 * cdp_peer_remove_for_vdev_no_lock() - remove peer instance from vdev
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 * @callback - remove done notification callback function pointer
 * @callback_context - callback caller context
 *
 * remove peer instance from virtual interface without lock
 *
 * Return: NONE
 */
static inline void
cdp_peer_remove_for_vdev_no_lock(ol_txrx_soc_handle soc,
				 struct cdp_vdev *vdev,
				 ol_txrx_vdev_peer_remove_cb callback,
				 void *callback_context)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->remove_peers_for_vdev_no_lock)
		return soc->ops->peer_ops->remove_peers_for_vdev_no_lock(
			vdev, callback, callback_context);
}

/**
 * cdp_peer_get_ref_by_addr() - Find peer by peer mac address and inc peer ref
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 * @peer_id - local peer id with target mac address
 * @debug_id - debug_id to track caller
 *
 * To release the peer ref, cdp_peer_release_ref needs to be called.
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
static inline void
*cdp_peer_get_ref_by_addr(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
			  uint8_t *peer_addr, uint8_t *peer_id,
			  enum peer_debug_id_type debug_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->peer_get_ref_by_addr)
		return soc->ops->peer_ops->peer_get_ref_by_addr(
			pdev, peer_addr, peer_id, debug_id);

	return NULL;
}

/**
 * cdp_peer_release_ref() - Release peer reference
 * @soc - data path soc handle
 * @peer - peer pointer
 * @debug_id - debug_id to track caller
 *
 * Return:void
 */
static inline void
cdp_peer_release_ref(ol_txrx_soc_handle soc, void *peer,
		     enum peer_debug_id_type debug_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->peer_release_ref)
		soc->ops->peer_ops->peer_release_ref(peer, debug_id);
}

/**
 * cdp_peer_find_by_addr() - Find peer by peer mac address
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 * @peer_id - local peer id with target mac address
 *
 * Find peer and local peer id by peer mac address
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
static inline void
*cdp_peer_find_by_addr(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		uint8_t *peer_addr, uint8_t *peer_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->find_peer_by_addr)
		return soc->ops->peer_ops->find_peer_by_addr(
			pdev, peer_addr, peer_id);

	return NULL;
}

/**
 * cdp_peer_find_by_addr_and_vdev() - Find peer by peer mac address within vdev
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @vdev - virtual interface instance
 * @peer_addr - peer mac address
 * @peer_id - local peer id with target mac address
 *
 * Find peer by peer mac address within vdev
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
static inline void
*cdp_peer_find_by_addr_and_vdev(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		struct cdp_vdev *vdev, uint8_t *peer_addr, uint8_t *peer_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->find_peer_by_addr_and_vdev)
		return soc->ops->peer_ops->find_peer_by_addr_and_vdev(
			pdev, vdev, peer_addr, peer_id);

	return NULL;
}

/**
 * cdp_peer_find_by_local_id() - Find peer by local peer id
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @local_peer_id - local peer id want to find
 *
 * Find peer by local peer id within physical device
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
static inline void
*cdp_peer_find_by_local_id(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		uint8_t local_peer_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->peer_find_by_local_id)
		return soc->ops->peer_ops->peer_find_by_local_id(
			pdev, local_peer_id);

	return NULL;
}

/**
 * cdp_peer_state_update() - update peer local state
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 * @state - new peer local state
 *
 * update peer local state
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_state_update(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		uint8_t *peer_addr, enum ol_txrx_peer_state state)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->peer_state_update)
		return soc->ops->peer_ops->peer_state_update(
			pdev, peer_addr, state);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_state_get() - Get local peer state
 * @soc - data path soc handle
 * @peer - peer instance
 *
 * Get local peer state
 *
 * Return: peer status
 */
static inline int
cdp_peer_state_get(ol_txrx_soc_handle soc, void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->peer_ops->get_peer_state)
		return soc->ops->peer_ops->get_peer_state(peer);

	return 0;
}

/**
 * cdp_peer_get_local_peer_id() - Find local peer id within peer instance
 * @soc - data path soc handle
 * @peer - peer instance
 *
 * Find local peer id within peer instance
 *
 * Return: local peer id
 *         HTT_INVALID_PEER Invalid peer
 */
static inline uint16_t
cdp_peer_get_local_peer_id(ol_txrx_soc_handle soc, void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return HTT_INVALID_PEER;
	}

	if (soc->ops->peer_ops->local_peer_id)
		return soc->ops->peer_ops->local_peer_id(peer);

	return HTT_INVALID_PEER;
}

/**
 * cdp_peer_get_vdevid() - Get virtual interface id which peer registered
 * @soc - data path soc handle
 * @peer - peer instance
 * @vdev_id - virtual interface id which peer registered
 *
 * Get virtual interface id which peer registered
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_get_vdevid(ol_txrx_soc_handle soc, void *peer, uint8_t *vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->get_vdevid)
		return soc->ops->peer_ops->get_vdevid(peer, vdev_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_get_vdev_by_sta_id() - Get vdev instance by local peer id
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @sta_id - local peer id
 *
 * Get virtual interface id by local peer id
 *
 * Return: Virtual interface instance
 *         NULL in case cannot find
 */
static inline struct cdp_vdev
*cdp_peer_get_vdev_by_sta_id(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
			     uint8_t sta_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->get_vdev_by_sta_id)
		return soc->ops->peer_ops->get_vdev_by_sta_id(pdev, sta_id);

	return NULL;
}

/**
 * cdp_peer_get_peer_mac_addr() - Get peer mac address
 * @soc - data path soc handle
 * @peer - peer instance
 *
 * Get peer mac address
 *
 * Return: peer mac address pointer
 *         NULL in case cannot find
 */
static inline uint8_t
*cdp_peer_get_peer_mac_addr(ol_txrx_soc_handle soc, void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->peer_get_peer_mac_addr)
		return soc->ops->peer_ops->peer_get_peer_mac_addr(peer);

	return NULL;
}

/**
 * cdp_peer_get_vdev() - Get virtual interface instance which peer belongs
 * @soc - data path soc handle
 * @peer - peer instance
 *
 * Get virtual interface instance which peer belongs
 *
 * Return: virtual interface instance pointer
 *         NULL in case cannot find
 */
static inline struct cdp_vdev
*cdp_peer_get_vdev(ol_txrx_soc_handle soc, void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->get_vdev_for_peer)
		return soc->ops->peer_ops->get_vdev_for_peer(peer);

	return NULL;
}

/**
 * cdp_peer_update_ibss_add_peer_num_of_vdev() - update number of peer
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 * @peer_num_delta - number of peer should be updated
 *
 * update number of peer
 *
 * Return: updated number of peer
 *         0 fail
 */
static inline int16_t
cdp_peer_update_ibss_add_peer_num_of_vdev(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, int16_t peer_num_delta)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->peer_ops->update_ibss_add_peer_num_of_vdev)
		return soc->ops->peer_ops->update_ibss_add_peer_num_of_vdev(
			vdev, peer_num_delta);

	return 0;
}

/**
 * cdp_peer_copy_mac_addr_raw() - copy peer mac address
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 * @bss_addr - mac address should be copied
 *
 * copy peer mac address
 *
 * Return: none
 */
static inline void
cdp_peer_copy_mac_addr_raw(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, uint8_t *bss_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->copy_mac_addr_raw)
		return soc->ops->peer_ops->copy_mac_addr_raw(vdev, bss_addr);

	return;
}

/**
 * cdp_peer_add_last_real_peer() - Add peer with last peer marking
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @vdev - virtual interface instance
 * @peer_id - local peer id
 *
 * copy peer mac address
 *
 * Return: none
 */
static inline void
cdp_peer_add_last_real_peer(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, struct cdp_vdev *vdev, uint8_t *peer_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->add_last_real_peer)
		return soc->ops->peer_ops->add_last_real_peer(
			pdev, vdev, peer_id);
	return;
}

/**
 * cdp_peer_is_vdev_restore_last_peer() - restore last peer
 * @soc - data path soc handle
 * @peer - peer instance pointer
 *
 * restore last peer
 *
 * Return: true, restore success
 *         fasle, restore fail
 */
static inline bool
cdp_peer_is_vdev_restore_last_peer(ol_txrx_soc_handle soc, void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->is_vdev_restore_last_peer)
		return soc->ops->peer_ops->is_vdev_restore_last_peer(peer);

	return false;
}

/**
 * cdp_peer_update_last_real_peer() - update last real peer
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @peer - peer instance pointer
 * @peer_id - local peer id
 * @restore_last_peer - restore last peer or not
 *
 * update last real peer
 *
 * Return: none
 */
static inline void
cdp_peer_update_last_real_peer(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		void *vdev, uint8_t *peer_id, bool restore_last_peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->update_last_real_peer)
		return soc->ops->peer_ops->update_last_real_peer(pdev, vdev,
			peer_id, restore_last_peer);

	return;
}

/**
 * ol_txrx_peer_detach_force_delete() - Detach and delete a peer's data object
 * @peer - the object to detach
 *
 * Detach a peer and force the peer object to be removed. It is called during
 * roaming scenario when the firmware has already deleted a peer.
 * Peer object is freed immediately to avoid duplicate peers during roam sync
 * indication processing.
 *
 * Return: None
 */
static inline void cdp_peer_detach_force_delete(ol_txrx_soc_handle soc,
		void *peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->peer_detach_force_delete)
		return soc->ops->peer_ops->peer_detach_force_delete(peer);

	return;
}

/**
 * is_cdp_peer_detach_force_delete_supported() - To check if force delete
 *						 operation is supported
 * @soc: pointer to SOC handle
 *
 * Some of the platforms support force delete operation and some of them
 * don't. This API returns true if API which handles force delete operation
 * is registered and false otherwise.
 *
 * Return: true if API which handles force delete operation is registered
 *	   false in all other cases
 */
static inline bool
is_cdp_peer_detach_force_delete_supported(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->peer_detach_force_delete)
		return true;

	return false;
}
#endif /* _CDP_TXRX_PEER_H_ */
