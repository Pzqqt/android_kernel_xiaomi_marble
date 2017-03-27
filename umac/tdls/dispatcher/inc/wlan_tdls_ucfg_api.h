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
 * DOC: wlan_tdls_ucfg_api.h
 *
 * TDLS north bound interface declaration
 */

#if !defined(_WLAN_TDLS_UCFG_API_H_)
#define _WLAN_TDLS_UCFG_API_H_

#include <scheduler_api.h>
#include <wlan_tdls_public_structs.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

/**
 * ucfg_tdls_init() - TDLS module initialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_init(void);

/**
 * ucfg_tdls_deinit() - TDLS module deinitilization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_deinit(void);

/**
 * ucfg_tdls_psoc_open() - TDLS module psoc open API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_tdls_psoc_close() - TDLS module psoc close API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_tdls_psoc_start() - TDLS module start
 * @psoc: psoc object
 * @req: tdls start paramets
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_update_config(struct wlan_objmgr_psoc *psoc,
				   struct tdls_start_params *req);

/**
 * ucfg_tdls_psoc_enable() - TDLS module enable API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_tdls_psoc_disable() - TDLS moudle disable API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_psoc_disable(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_tdls_add_peer() - handle TDLS add peer
 * @vdev: vdev object
 * @add_peer_req: add peer request parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_add_peer(struct wlan_objmgr_vdev *vdev,
			      struct tdls_add_peer_params *add_peer_req);

/**
 * ucfg_tdls_update_peer() - handle TDLS update peer
 * @vdev: vdev object
 * @update_peer: update TDLS request parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_update_peer(struct wlan_objmgr_vdev *vdev,
				 struct tdls_update_peer_params *update_peer);

/**
 * ucfg_tdls_oper() - handle TDLS oper functions
 * @vdev: vdev object
 * @macaddr: MAC address of TDLS peer
 * @cmd: oper cmd
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_oper(struct wlan_objmgr_vdev *vdev,
			  const uint8_t *macaddr, enum tdls_command_type cmd);

/**
 * ucfg_tdls_send_mgmt_frame() - send TDLS mgmt frame
 * @mgmt_req: pointer to TDLS action frame request struct
 *
 * This will TDLS action frames to peer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_tdls_send_mgmt_frame(
				struct tdls_action_frame_request *mgmt_req);

#endif
