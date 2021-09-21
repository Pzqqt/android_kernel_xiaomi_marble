/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains MLO manager public file containing AP functionality
 */
#ifndef _WLAN_MLO_MGR_AP_H_
#define _WLAN_MLO_MGR_AP_H_

#include <wlan_mlo_mgr_cmn.h>
#include <wlan_mlo_mgr_public_structs.h>

/**
 * mlo_ap_vdev_attach() - update vdev obj and vdev count to
 *                         wlan_mlo_dev_context
 * @vdev: vdev pointer
 * @link_id: link id
 * @vdev_count: number of vdev in the mlo
 *
 * Return: true if succeeds
 */
bool mlo_ap_vdev_attach(struct wlan_objmgr_vdev *vdev,
			uint8_t link_id,
			uint16_t vdev_count);

/**
 * mlo_ap_get_vdev_list() - get mlo vdev list
 * @vdev: vdev pointer
 * @vdev_count: vdev count
 * @wlan_vdev_list: vdev list
 *
 * Return: None
 */
void mlo_ap_get_vdev_list(struct wlan_objmgr_vdev *vdev,
			  uint16_t *vdev_count,
			  struct wlan_objmgr_vdev **wlan_vdev_list);

/**
 * mlo_ap_link_sync_wait_notify() - notify the mlo manager, once vdev
 *                                  enters WLAN_VDEV_SS_MLO_SYNC_WAIT
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_link_sync_wait_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_link_start_rsp_notify - Notify that the link start is completed
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_ap_link_start_rsp_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_vdev_detach() - notify the mlo manager to detach given vdev
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_vdev_detach(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_link_down_cmpl_notify() - notify the mlo manager, once vdev
 *                                  is down completely
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlo_ap_link_down_cmpl_notify(struct wlan_objmgr_vdev *vdev);
/**
 * wlan_vdev_aid_mgr_init() - VDEV AID mgr init
 * @max_aid: max AID
 *
 * This function is called as part of vdev/MLO DEV initialization.
 * This will allocate aid mgr structure for a VDEV
 *
 * Return: aid_mgr
 */
struct wlan_vdev_aid_mgr *wlan_vdev_aid_mgr_init(uint16_t max_aid);

/**
 * wlan_vdev_aid_mgr_free() - VDEV AID mgr free
 * @aid_mgr: AID mgr
 *
 * This function frees the aid mgr of the VDEV
 *
 * Return: void
 */
void wlan_vdev_aid_mgr_free(struct wlan_vdev_aid_mgr *aid_mgr);

/**
 * wlan_mlo_vdev_aid_mgr_init() - MLO AID mgr init
 * @ml_dev: MLO DEV context
 *
 * This function allocate AID space for all associated VDEVs of MLD
 *
 * Return: SUCCESS if allocated successfully
 */
QDF_STATUS wlan_mlo_vdev_aid_mgr_init(struct wlan_mlo_dev_context *ml_dev);

/**
 * wlan_mlo_vdev_aid_mgr_deinit() - MLO AID mgr free
 * @ml_dev: MLO DEV context
 *
 * This function frees AID space for all associated VDEVs of MLD
 *
 * Return: void
 */
void wlan_mlo_vdev_aid_mgr_deinit(struct wlan_mlo_dev_context *ml_dev);

/**
 * wlan_mlo_vdev_alloc_aid_mgr() - Allocate AID space for a VDEV
 * @ml_dev: MLO DEV context
 * @vdev: VDEV
 *
 * This function allocates AID space for an associated VDEV of MLD
 *
 * Return: SUCCESS if allocated successfully
 */
QDF_STATUS wlan_mlo_vdev_alloc_aid_mgr(struct wlan_mlo_dev_context *ml_dev,
				       struct wlan_objmgr_vdev *vdev);

/**
 * wlan_mlo_vdev_free_aid_mgr() - Free AID space for a VDEV
 * @ml_dev: MLO DEV context
 * @vdev: VDEV
 *
 * This function frees AID space for an associated VDEV of MLD
 *
 * Return: SUCCESS if freed successfully
 */
QDF_STATUS wlan_mlo_vdev_free_aid_mgr(struct wlan_mlo_dev_context *ml_dev,
				      struct wlan_objmgr_vdev *vdev);

/**
 * mlo_peer_allocate_aid() - Allocate AID for MLO peer
 * @ml_dev: MLO DEV context
 * @ml_peer: MLO peer object
 *
 * This function allocates AID for an MLO peer
 *
 * Return: SUCCESS if allocated successfully
 */
QDF_STATUS mlo_peer_allocate_aid(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer);

/**
 * mlo_get_aid() - Allocate AID for a associated station
 * @vdev: VDEV
 *
 * This function allocates AID for an associated station of MLD
 *
 * Return: AID
 */
uint16_t mlo_get_aid(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_free_aid() - Frees AID for a station
 * @vdev: VDEV
 * @assoc_id: Assoc ID
 *
 * This function frees AID for an associated station of MLD
 *
 * Return: SUCCESS if freed
 */
QDF_STATUS mlo_free_aid(struct wlan_objmgr_vdev *vdev, uint16_t assoc_id);

/**
 * mlme_get_aid() - Allocate AID for a non-MLD station
 * @vdev: VDEV
 *
 * This function allocates AID for an associated NON-MLD station of MLD
 *
 * Return: AID
 */
uint16_t mlme_get_aid(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_mlo_peer_free_aid() - Free assoc id
 * @ml_aid_mgr: MLO AID mgr
 * @link_ix: Link index
 * @assoc_id: Assoc id to be freed
 *
 * This function frees assoc id, resets bit in all bitmaps
 *
 * Return: SUCCESS,if it freed
 */
QDF_STATUS wlan_mlo_peer_free_aid(
		struct wlan_ml_vdev_aid_mgr *ml_aid_mgr,
		uint8_t link_ix,
		uint16_t assoc_id);

/**
 * wlan_mlme_peer_free_aid() - Free link assoc id
 * @vdev_aid_mgr: VDEV AID mgr
 * @no_lock: lock needed for the operation
 * @assoc_id: Assoc id to be freed
 *
 * This function frees assoc id of a specific VDEV
 *
 * Return: void
 */
void wlan_mlme_peer_free_aid(
		struct wlan_vdev_aid_mgr *vdev_aid_mgr,
		bool no_lock, uint16_t assoc_id);

/**
 * mlo_peer_free_aid() - public API to free AID
 * @ml_dev: MLO DEV context
 * @ml_peer: MLO peer object
 *
 * This function invokes low level API to free assoc id
 *
 * Return: SUCCESS, if it freed
 */
QDF_STATUS mlo_peer_free_aid(struct wlan_mlo_dev_context *ml_dev,
			     struct wlan_mlo_peer_context *ml_peer);

/**
 * mlme_free_aid() - public API to free link assoc id
 * @vdev: VDEV object
 * @assoc_id: Assoc id to be freed
 *
 * This function invokes low level API to free assoc id of a specific VDEV
 *
 * Return: void
 */
void mlme_free_aid(struct wlan_objmgr_vdev *vdev, uint16_t assoc_id);

/**
 * mlo_ap_ml_peerid_alloc() - public API to allocate MLO peer id
 *
 * This function allocates MLO peer ID
 *
 * Return: mlo_peer_id on success,
	 MLO_INVALID_PEER_ID on failure
 */
uint16_t mlo_ap_ml_peerid_alloc(void);

/**
 * mlo_ap_ml_peerid_free() - public API to free MLO peer id
 * @ml_peer_id: ML peer id
 *
 * This function frees MLO peer ID
 *
 * Return: void
 */
void mlo_ap_ml_peerid_free(uint16_t mlo_peer_id);

#define ML_PRIMARY_UMAC_ID_INVAL 0xff
/**
 * mlo_peer_assign_primary_umac() - Assign Primary UMAC
 * @ml_peer: MLO peer object
 * @peer_entry: Link peer entry
 *
 * This function assigns primary UMAC flag in peer entry
 *
 * Return: SUCCESS,if it allocated
 */
void mlo_peer_assign_primary_umac(
		struct wlan_mlo_peer_context *ml_peer,
		struct wlan_mlo_link_peer_entry *peer_entry);

/**
 * mlo_peer_allocate_primary_umac() - Allocate Primary UMAC
 * @ml_dev: MLO DEV context
 * @ml_peer: MLO peer object
 * @link_vdev: link vdev array
 *
 * This function allocates primary UMAC for a MLO peer
 *
 * Return: SUCCESS,if it allocated
 */
QDF_STATUS mlo_peer_allocate_primary_umac(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer,
		struct wlan_objmgr_vdev *link_vdevs[]);

/**
 * mlo_peer_free_primary_umac() - Free Primary UMAC
 * @ml_dev: MLO DEV context
 * @ml_peer: MLO peer object
 *
 * This function frees primary UMAC for a MLO peer
 *
 * Return: SUCCESS,if it is freed
 */
QDF_STATUS mlo_peer_free_primary_umac(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer);

/**
 * mlo_ap_vdev_quiet_set() - Set quiet bitmap for requested vdev
 * @vdev: Pointer to object manager vdev
 *
 * Return: void
 */
void mlo_ap_vdev_quiet_set(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_vdev_quiet_clear() - Clear quiet bitmap for requested vdev
 * @vdev: Pointer to object manager vdev
 *
 * Return: void
 */
void mlo_ap_vdev_quiet_clear(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_ap_vdev_quiet_is_any_idx_set() - Check if any index is set in
 * quiet bitmap
 * @vdev: Pointer to object manager vdev
 *
 * Return: true, if any index is set, else false
 */
bool mlo_ap_vdev_quiet_is_any_idx_set(struct wlan_objmgr_vdev *vdev);
#endif
