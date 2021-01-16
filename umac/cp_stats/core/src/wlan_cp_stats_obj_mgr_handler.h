/*
 * Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_obj_mgr_handler.h
 *
 * This header file provide declarations for APIs to handle events from object
 * manager for registered events from wlan_cp_stats_init()
 */

#ifndef __WLAN_CP_STATS_OBJ_MGR_HANDLER_H__
#define __WLAN_CP_STATS_OBJ_MGR_HANDLER_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#include "wlan_cp_stats_defs.h"

/**
 * wlan_cp_stats_psoc_obj_create_handler() - psoc create notification handler
 * callback function
 * @psoc:		pointer to psoc object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_psoc_obj_create_handler(
		struct wlan_objmgr_psoc *psoc, void *data);

/**
 * wlan_cp_stats_psoc_obj_destroy_handler() - psoc destroy notification handler
 * callback function
 * @psoc:		pointer to psoc object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_psoc_obj_destroy_handler(
		struct wlan_objmgr_psoc *psoc, void *data);

/**
 * wlan_cp_stats_pdev_obj_create_handler() - Pdev create notification handler
 * callback function
 * @pdev:		pointer to pdev object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_pdev_obj_create_handler(
		struct wlan_objmgr_pdev *pdev, void *data);

/**
 * wlan_cp_stats_pdev_obj_destroy_handler() - Pdev destroy notification handler
 * callback function
 * @pdev:		pointer to pdev object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_pdev_obj_destroy_handler(
		struct wlan_objmgr_pdev *pdev, void *data);

/**
 * wlan_cp_stats_vdev_obj_create_handler() - vdev create notification handler
 * callback function
 * @vdev:		pointer to vdev object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_vdev_obj_create_handler(
		struct wlan_objmgr_vdev *vdev, void *data);

/**
 * wlan_cp_stats_vdev_obj_destroy_handler() - vdev destroy notification handler
 * callback function
 * @vdev:		pointer to vdev object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_vdev_obj_destroy_handler(
		struct wlan_objmgr_vdev *vdev, void *data);

/**
 * wlan_cp_stats_peer_obj_create_handler() - peer create notification handler
 * callback function
 * @peer:		pointer to peer object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_peer_obj_create_handler(
		struct wlan_objmgr_peer *peer, void *data);

/**
 * wlan_cp_stats_peer_obj_destroy_handler() - peer destroy notification handler
 * callback function
 * @peer:		pointer to peer object
 * @data:		pointer to arg data
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_peer_obj_destroy_handler(
		struct wlan_objmgr_peer *peer, void *data);

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
/**
 * wlan_cp_stats_infra_cp_register_resp_cb() - Register the response callback
 * and cookie in the psoc mc_stats object
 * @psoc: pointer to psoc object
 * @req: pointer to request parameter structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
QDF_STATUS
wlan_cp_stats_infra_cp_register_resp_cb(struct wlan_objmgr_psoc *psoc,
					struct infra_cp_stats_cmd_info *req);

/**
 * wlan_cp_stats_infra_cp_get_context() - get the context and callback
 * for sending response
 * @psoc: pointer to psoc object
 * @resp_cb: pointer to store the response callback
 * @context: pointer to store context
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
QDF_STATUS
wlan_cp_stats_infra_cp_get_context(struct wlan_objmgr_psoc *psoc,
				   get_infra_cp_stats_cb *resp_cb,
				   void **context);
/**
 * wlan_cp_stats_send_infra_cp_req() - API to send infra cp stats request to
 * lmac
 * @psoc: pointer to psoc object
 * @req: pointer to infra cp stats request
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
QDF_STATUS
wlan_cp_stats_send_infra_cp_req(struct wlan_objmgr_psoc *psoc,
				struct infra_cp_stats_cmd_info *req);

#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */
#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_OBJ_MGR_HANDLER_H__ */
