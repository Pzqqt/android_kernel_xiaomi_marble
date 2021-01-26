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
 * cm_fw_roam_start() - Handle roam start event
 * @cm_ctx: connection mgr context
 * @vdev_id: vdev id
 * @pause_serialization: boolean indicating to pause serialization
 *
 * This function handles the roam start event received from FW.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_start(struct cnx_mgr *cm_ctx, uint8_t vdev_id,
			    bool pause_serialization);

/**
 * cm_fw_roam_sync_req() - Post roam sync to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function posts roam sync event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * cm_fw_roam_sync_propagation() - Post roam sync propagation to CM SM
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function posts roam sync propagation event change to connection manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_fw_roam_sync_propagation(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);
#endif /* _WLAN_CM_ROAM_I_H_ */
