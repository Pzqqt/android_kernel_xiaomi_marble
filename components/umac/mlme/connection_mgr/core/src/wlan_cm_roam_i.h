/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_roam_i.h
 *
 * Implementation for the common roaming api interfaces.
 */

#ifndef _WLAN_CM_ROAM_I_H_
#define _WLAN_CM_ROAM_I_H_

#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "connection_mgr/core/src/wlan_cm_main.h"
#include "wlan_cm_roam_public_struct.h"

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_add_fw_roam_dummy_ser_cb() - Add dummy blocking command
 * @pdev: pdev pointer
 * @cm_ctx: connection mgr context
 * @cm_req: connect req
 *
 * This function adds dummy blocking command with high priority to avoid
 * any other vdev command till roam is completed.Any NB operations will be
 * blocked in serailization until roam logic completes execution.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_add_fw_roam_dummy_ser_cb(struct wlan_objmgr_pdev *pdev,
			    struct cnx_mgr *cm_ctx,
			    struct cm_req *cm_req);
/**
 * cm_fw_roam_start_req() - Post roam start req to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function posts roam start event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_start_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);
/**
 * cm_fw_roam_abort_req() - Post roam abort req to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function posts roam abort event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * cm_update_scan_mlme_on_roam() - update the scan mlme info
 * on roam sync ind
 * @vdev: Object manager vdev
 * @connected_bssid: Bssid addr
 * @state: scan entry state
 *
 * Return: void
 */
void
cm_update_scan_mlme_on_roam(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *connected_bssid,
			    enum scan_entry_connection_state state);

/**
 * cm_abort_fw_roam() - abort fw roaming
 *
 * @cm_ctx: Connection mgr context
 * @cm_id: CM command id
 *
 * Return: qdf status
 */
QDF_STATUS cm_abort_fw_roam(struct cnx_mgr *cm_ctx,
			    wlan_cm_id cm_id);

/**
 * cm_fw_roam_sync_req() - Post roam sync to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @event: Vdev mgr event
 * @event_data_len: data size
 *
 * This function posts roam sync event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			       void *event, uint32_t event_data_len);

/**
 * cm_fw_roam_sync_start_ind() - Handle roam sync req
 * @vdev: Vdev objmgr
 * @roam_reason: Roam reason
 *
 * This function handles roam sync event to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_fw_roam_sync_start_ind(struct wlan_objmgr_vdev *vdev,
			  uint8_t roam_reason);

/**
 * cm_fw_roam_sync_propagation() - Post roam sync propagation to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @roam_synch_data: Roam sync data ptr
 *
 * This function posts roam sync propagation event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_fw_roam_sync_propagation(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			    struct roam_offload_synch_ind *roam_synch_data);

/**
 * cm_fw_ho_fail_req() - Post roam ho fail req to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @bssid: bssid mac addr
 *
 * This function posts roam ho fail event change to
 * connection manager state machine
 *
 * Return: void
 */
void cm_fw_ho_fail_req(struct wlan_objmgr_psoc *psoc,
		       uint8_t vdev_id, struct qdf_mac_addr bssid);

/**
 * cm_fw_roam_invoke_fail() - Post roam invoke fail to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function posts roam invoke fail event change to
 * connection manager state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_invoke_fail(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id);
#else /*WLAN_FEATURE_ROAM_OFFLOAD */
static inline
QDF_STATUS cm_fw_roam_invoke_fail(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#endif /* _WLAN_CM_ROAM_I_H_ */
