/*
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains MLO manager public file containing roaming functionality
 */
#ifndef _WLAN_MLO_MGR_ROAM_H_
#define _WLAN_MLO_MGR_ROAM_H_

#include <wlan_mlo_mgr_cmn.h>
#include <wlan_mlo_mgr_public_structs.h>
#include <wlan_cm_roam_public_struct.h>

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * mlo_fw_roam_sync_req - Handler for roam sync event handling
 *
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @event: event ptr
 * @event_data_len: event data len
 *
 * This api will be called from target_if layer to mlo mgr,
 * handles mlo roaming and posts roam sync propagation to
 * connection manager state machine.
 *
 * Return: qdf status
 */
QDF_STATUS mlo_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id, void *event,
				uint32_t event_data_len);

/**
 * mlo_fw_ho_fail_req - Handler for HO fail event handling
 *
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @bssid: bssid mac addr
 *
 * This api will be called from target_if layer to mlo mgr,
 * handles mlo ho fail req and posts to connection manager
 * state machine.
 *
 * Return: void
 */
void
mlo_fw_ho_fail_req(struct wlan_objmgr_psoc *psoc,
		   uint8_t vdev_id, struct qdf_mac_addr bssid);

/**
 * mlo_get_sta_link_mac_addr - get sta link mac addr
 *
 * @vdev_id: vdev id
 * @sync_ind: roam sync ind pointer
 * @link_mac_addr: link mac addr pointer
 *
 * This api will be called to get the link specific mac address.
 *
 * Return: qdf status
 */
QDF_STATUS
mlo_get_sta_link_mac_addr(uint8_t vdev_id,
			  struct roam_offload_synch_ind *sync_ind,
			  struct qdf_mac_addr *link_mac_addr);

/**
 * is_multi_link_roam - check if MLO roaming
 *
 * @sync_ind: roam sync ind pointer
 *
 * This api will be called to check if MLO roaming.
 *
 * Return: true/false
 */
bool
is_multi_link_roam(struct roam_offload_synch_ind *sync_ind);

/**
 * mlo_enable_rso - Enable rso on assoc vdev
 *
 * @pdev: pdev pointer
 * @vdev: assoc vdev pointer
 *
 * This api will be called to enable RSO for MLO connection.
 *
 * Return: qdf_status success or fail
 */
QDF_STATUS mlo_enable_rso(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_vdev *vdev);

#else
static inline QDF_STATUS
mlo_get_sta_link_mac_addr(uint8_t vdev_id,
			  struct roam_offload_synch_ind *sync_ind,
			  struct qdf_mac_addr *link_mac_addr)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool
is_multi_link_roam(struct roam_offload_synch_ind *sync_ind)
{
	return false;
}

static inline
QDF_STATUS mlo_enable_rso(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_11BE_MLO */
#endif
