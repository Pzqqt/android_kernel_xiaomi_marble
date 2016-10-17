/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
  * DOC: Define the global data structure of UMAC
  */
#ifndef _WLAN_OBJMGR_GLOBAL_OBJ_I_H_
#define _WLAN_OBJMGR_GLOBAL_OBJ_I_H_

#include "wlan_objmgr_cmn.h"
/**
 *  struct wlan_objmgr_global - Global object definition
 *  @psoc[]:                    Array of PSOCs to maintain PSOC's list,
 *                              its optional
 *  @psoc_create_handler[]:     PSOC create handler array
 *  @psoc_create_handler_arg[]: PSOC create handler args array
 *  @psoc_delete_handler[]:     PSOC delete handler array
 *  @psoc_delete_handler_arg[]: PSOC delete handler args array
 *  @psoc_status_handler[]:     PSOC status handler array
 *  @psoc_status_handler_arg[]: PSOC status handler args array
 *  @pdev_create_handler[]:     PDEV create handler array
 *  @pdev_create_handler_arg[]: PDEV create handler args array
 *  @pdev_delete_handler[]:     PDEV delete handler array
 *  @pdev_delete_handler_arg[]: PDEV delete handler args array
 *  @pdev_status_handler[]:     PDEV status handler array
 *  @pdev_status_handler_arg[]: PDEV status handler args array
 *  @vdev_create_handler[]:     VDEV create handler array
 *  @vdev_create_handler_arg[]: VDEV create handler args array
 *  @vdev_delete_handler[]:     VDEV delete handler array
 *  @vdev_delete_handler_arg[]: VDEV delete handler args array
 *  @vdev_status_handler[]:     VDEV status handler array
 *  @vdev_status_handler_arg[]: VDEV status handler args array
 *  @peer_create_handler[]:     PEER create handler array
 *  @peer_create_handler_arg[]: PEER create handler args array
 *  @peer_delete_handler[]:     PEER delete handler array
 *  @peer_delete_handler_arg[]: PEER delete handler args array
 *  @peer_status_handler[]:     PEER status handler array
 *  @peer_status_handler_arg[]: PEER status handler args array
 *  @global_lock:               Global lock
 */
struct wlan_objmgr_global {
	struct wlan_objmgr_psoc *psoc[WLAN_OBJMGR_MAX_DEVICES];
	wlan_objmgr_psoc_create_handler
		psoc_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_psoc_delete_handler
		psoc_delete_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_delete_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_psoc_status_handler
		psoc_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_create_handler
		pdev_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_delete_handler
		pdev_delete_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_delete_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_status_handler
		pdev_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_create_handler
		vdev_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_delete_handler
		vdev_delete_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_delete_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_status_handler
		vdev_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_create_handler
		peer_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_delete_handler
		peer_delete_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_delete_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_status_handler
		peer_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	qdf_spinlock_t	global_lock;
};

#define MAX_SLEEP_ITERATION 5

extern struct wlan_objmgr_global *g_umac_glb_obj;

#endif /* _WLAN_OBJMGR_GLOBAL_OBJ_I_H_ */
