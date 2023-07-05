/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
 * @pdev_id - data path device instance id
 * @sta_desc - peer description
 *
 * Register peer into physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_register(ol_txrx_soc_handle soc, uint8_t pdev_id,
		  struct ol_txrx_desc_type *sta_desc)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->register_peer)
		return soc->ops->peer_ops->register_peer(soc, pdev_id,
							 sta_desc);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_clear_peer() - remove peer from physical device
 * @soc - data path soc handle
 * @pdev_id - data path device instance id
 * @peer_addr - peer mac address
 *
 * remove peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_clear_peer(ol_txrx_soc_handle soc, uint8_t pdev_id,
	       struct qdf_mac_addr peer_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->clear_peer)
		return soc->ops->peer_ops->clear_peer(soc, pdev_id, peer_addr);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_register_ocb_peer() - register ocb peer from physical device
 * @soc - data path soc handle
 * @cds_ctx - cds void context
 * @mac_addr - mac address for ocb self peer
 *
 * register ocb peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_register_ocb_peer(ol_txrx_soc_handle soc,
		uint8_t *mac_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->register_ocb_peer)
		return soc->ops->peer_ops->register_ocb_peer(mac_addr);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_find_peer_exist - Find if peer already exists
 * @soc - data path soc handle
 * @pdev_id - data path device instance id
 * @peer_addr - peer mac address
 *
 * Return: true or false
 */
static inline bool
cdp_find_peer_exist(ol_txrx_soc_handle soc, uint8_t pdev_id,
		    uint8_t *peer_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->find_peer_exist)
		return soc->ops->peer_ops->find_peer_exist(soc, pdev_id,
							   peer_addr);

	return false;
}

/**
 * cdp_find_peer_exist_on_vdev - Find if duplicate peer exists
 * on the given vdev
 * @soc - data path soc handle
 * @vdev_id - data path virtual interface id
 * @peer_addr - peer mac address
 *
 * Return: true or false
 */
static inline bool
cdp_find_peer_exist_on_vdev(ol_txrx_soc_handle soc, uint8_t vdev_id,
			    uint8_t *peer_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->find_peer_exist_on_vdev)
		return soc->ops->peer_ops->find_peer_exist_on_vdev(soc, vdev_id,
								   peer_addr);

	return false;
}

/**
 * cdp_find_peer_exist_on_other_vdev - Find if duplicate peer exists
 * on other than the given vdev
 * @soc - data path soc handle
 * @vdev_id - data path virtual interface id
 * @peer_addr - peer mac address
 * @max_bssid - max number of bssids
 *
 * Return: true or false
 */
static inline bool
cdp_find_peer_exist_on_other_vdev(ol_txrx_soc_handle soc, uint8_t vdev_id,
				  uint8_t *peer_addr, uint16_t max_bssid)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->find_peer_exist_on_other_vdev)
		return soc->ops->peer_ops->find_peer_exist_on_other_vdev(
								soc, vdev_id,
								peer_addr,
								max_bssid);

	return false;
}

/**
 * cdp_peer_state_update() - update peer local state
 * @soc - data path soc handle
 * @peer_addr - peer mac address
 * @state - new peer local state
 *
 * update peer local state
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_state_update(ol_txrx_soc_handle soc, uint8_t *peer_addr,
		      enum ol_txrx_peer_state state)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->peer_state_update)
		return soc->ops->peer_ops->peer_state_update(soc, peer_addr,
							     state);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_state_get() - Get local peer state
 * @soc - data path soc handle
 * @vdev_id - virtual interface id
 * @peer_mac - peer mac addr
 *
 * Get local peer state
 *
 * Return: peer status
 */
static inline int
cdp_peer_state_get(ol_txrx_soc_handle soc, uint8_t vdev_id, uint8_t *peer_mac)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->peer_ops->get_peer_state)
		return soc->ops->peer_ops->get_peer_state(soc, vdev_id,
							  peer_mac);

	return 0;
}

/**
 * cdp_peer_get_vdevid() - Get virtual interface id which peer registered
 * @soc - data path soc handle
 * @peer_mac - peer mac address
 * @vdev_id - virtual interface id which peer registered
 *
 * Get virtual interface id which peer registered
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_NOSUPPORT not support this feature
 */
static inline QDF_STATUS
cdp_peer_get_vdevid(ol_txrx_soc_handle soc,
		    uint8_t  *peer_mac, uint8_t *vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->peer_ops->get_vdevid)
		return soc->ops->peer_ops->get_vdevid(soc, peer_mac, vdev_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_peer_get_vdev_by_sta_id() - Get vdev instance by local peer id
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 *
 * Get virtual interface id by local peer id
 *
 * Return: Virtual interface instance
 *         NULL in case cannot find
 */
static inline struct cdp_vdev
*cdp_peer_get_vdev_by_peer_addr(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
				struct qdf_mac_addr peer_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->peer_ops->get_vdev_by_peer_addr)
		return soc->ops->peer_ops->get_vdev_by_peer_addr(pdev,
								 peer_addr);

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
 * cdp_peer_update_ibss_add_peer_num_of_vdev() - update number of peer
 * @soc - data path soc handle
 * @vdev_id - virtual interface instance id
 * @peer_num_delta - number of peer should be updated
 *
 * update number of peer
 *
 * Return: updated number of peer
 *         0 fail
 */
static inline int16_t
cdp_peer_update_ibss_add_peer_num_of_vdev(ol_txrx_soc_handle soc,
					  uint8_t vdev_id,
					  int16_t peer_num_delta)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->peer_ops->update_ibss_add_peer_num_of_vdev)
		return soc->ops->peer_ops->update_ibss_add_peer_num_of_vdev(
								soc, vdev_id,
								peer_num_delta);

	return 0;
}

/**
 * cdp_peer_copy_mac_addr_raw() - copy peer mac address
 * @soc - data path soc handle
 * @vdev_id - virtual interface instance id
 * @bss_addr - mac address should be copied
 *
 * copy peer mac address
 *
 * Return: none
 */
static inline void
cdp_peer_copy_mac_addr_raw(ol_txrx_soc_handle soc,
			   uint8_t vdev_id, uint8_t *bss_addr)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->copy_mac_addr_raw)
		return soc->ops->peer_ops->copy_mac_addr_raw(soc, vdev_id,
							     bss_addr);

	return;
}

/**
 * cdp_peer_add_last_real_peer() - Add peer with last peer marking
 * @soc - data path soc handle
 * @pdev_id - data path device instance id
 * @vdev_id - virtual interface instance id
 *
 * copy peer mac address
 *
 * Return: none
 */
static inline void
cdp_peer_add_last_real_peer(ol_txrx_soc_handle soc, uint8_t pdev_id,
			    uint8_t vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->add_last_real_peer)
		return soc->ops->peer_ops->add_last_real_peer(soc, pdev_id,
							      vdev_id);
	return;
}

/**
 * cdp_peer_is_vdev_restore_last_peer() - restore last peer
 * @soc - data path soc handle
 * @vdev_id - virtual interface id
 * @peer_mac - peer mac address
 *
 * restore last peer
 *
 * Return: true, restore success
 *         fasle, restore fail
 */
static inline bool
cdp_peer_is_vdev_restore_last_peer(ol_txrx_soc_handle soc, uint8_t vdev_id,
				   uint8_t *peer_mac)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return false;
	}

	if (soc->ops->peer_ops->is_vdev_restore_last_peer)
		return soc->ops->peer_ops->is_vdev_restore_last_peer(soc,
								     vdev_id,
								     peer_mac);

	return false;
}

/**
 * cdp_peer_update_last_real_peer() - update last real peer
 * @soc - data path soc handle
 * @pdev_id - data path device instance id
 * @vdev_id - virtual interface id
 * @restore_last_peer - restore last peer or not
 *
 * update last real peer
 *
 * Return: none
 */
static inline void
cdp_peer_update_last_real_peer(ol_txrx_soc_handle soc, uint8_t pdev_id,
			       uint8_t vdev_id, bool restore_last_peer)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->update_last_real_peer)
		return soc->ops->peer_ops->update_last_real_peer(
						soc, pdev_id, vdev_id,
						restore_last_peer);

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
						uint8_t vdev_id,
						uint8_t *peer_mac)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->peer_detach_force_delete)
		return soc->ops->peer_ops->peer_detach_force_delete(soc,
								    vdev_id,
								    peer_mac);

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

/*
 * cdp_peer_set_peer_as_tdls() - To set peer as tdls peer
 * @soc: pointer to SOC handle
 * @vdev_id: virtual interface id
 * @peer_mac: peer mac address
 * @var: true or false
 *
 * Return: void
 */
static inline void
cdp_peer_set_peer_as_tdls(ol_txrx_soc_handle soc, uint8_t vdev_id,
			  uint8_t *peer_mac, bool val)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->set_peer_as_tdls_peer)
		soc->ops->peer_ops->set_peer_as_tdls_peer(soc, vdev_id,
							  peer_mac, val);
}

/**
 * cdp_peer_set_tdls_offchan_enabled() - Set tdls offchan operation as enabled
 * @soc: pointer to SOC handle
 * @vdev_id: virtual interface id
 * @peer_mac: peer mac address
 * @val: true or false
 *
 * update tdls_offchan_enabled
 *
 * Return: none
 */
static inline void
cdp_peer_set_tdls_offchan_enabled(ol_txrx_soc_handle soc, uint8_t vdev_id,
				  uint8_t *peer_mac, bool val)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->set_tdls_offchan_enabled)
		soc->ops->peer_ops->set_tdls_offchan_enabled(soc, vdev_id,
							     peer_mac, val);
}

/**
 * cdp_peer_flush_frags() - Flush frags on peer
 * @soc - data path soc handle
 * @vdev_id - virtual interface id
 * @peer_mac - peer mac addr
 *
 * Return: None
 */
static inline void
cdp_peer_flush_frags(ol_txrx_soc_handle soc, uint8_t vdev_id, uint8_t *peer_mac)
{
	if (!soc || !soc->ops || !soc->ops->peer_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->peer_ops->peer_flush_frags)
		soc->ops->peer_ops->peer_flush_frags(soc, vdev_id, peer_mac);
}
#endif /* _CDP_TXRX_PEER_H_ */
