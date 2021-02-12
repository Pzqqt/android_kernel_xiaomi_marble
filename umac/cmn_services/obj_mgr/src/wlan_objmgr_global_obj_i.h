/*
 * Copyright (c) 2016-2019, 2021 The Linux Foundation. All rights reserved.
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
#ifdef WLAN_FEATURE_11BE_MLO
#include "wlan_mlo_mgr_public_structs.h"
#endif

struct wlan_objmgr_debug_info;
/**
 *  struct wlan_objmgr_global - Global object definition
 *  @psoc[]:                    Array of PSOCs to maintain PSOC's list,
 *                              its optional
 *  @psoc_create_handler[]:     PSOC create handler array
 *  @psoc_create_handler_arg[]: PSOC create handler args array
 *  @psoc_destroy_handler[]:     PSOC destroy handler array
 *  @psoc_destroy_handler_arg[]: PSOC destroy handler args array
 *  @psoc_status_handler[]:     PSOC status handler array
 *  @psoc_status_handler_arg[]: PSOC status handler args array
 *  @pdev_create_handler[]:     PDEV create handler array
 *  @pdev_create_handler_arg[]: PDEV create handler args array
 *  @pdev_destroy_handler[]:     PDEV destroy handler array
 *  @pdev_destroy_handler_arg[]: PDEV destroy handler args array
 *  @pdev_status_handler[]:     PDEV status handler array
 *  @pdev_status_handler_arg[]: PDEV status handler args array
 *  @vdev_create_handler[]:     VDEV create handler array
 *  @vdev_create_handler_arg[]: VDEV create handler args array
 *  @vdev_destroy_handler[]:     VDEV destroy handler array
 *  @vdev_destroy_handler_arg[]: VDEV destroy handler args array
 *  @vdev_peer_free_notify_handler[]: VDEV peer free notify handler array
 *  @vdev_status_handler[]:     VDEV status handler array
 *  @vdev_status_handler_arg[]: VDEV status handler args array
 *  @peer_create_handler[]:     PEER create handler array
 *  @peer_create_handler_arg[]: PEER create handler args array
 *  @peer_destroy_handler[]:     PEER destroy handler array
 *  @peer_destroy_handler_arg[]: PEER destroy handler args array
 *  @peer_status_handler[]:     PEER status handler array
 *  @peer_status_handler_arg[]: PEER status handler args array
 *  @debug_info:                Objmgr debug information
 *  @global_lock:               Global lock
 */
struct wlan_objmgr_global {
	struct wlan_objmgr_psoc *psoc[WLAN_OBJMGR_MAX_DEVICES];
#ifdef WLAN_FEATURE_11BE_MLO
	struct mlo_mgr_context *mlo_ctx;
#endif
	wlan_objmgr_psoc_create_handler
		psoc_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_psoc_destroy_handler
		psoc_destroy_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_destroy_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_psoc_status_handler
		psoc_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *psoc_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_create_handler
		pdev_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_destroy_handler
		pdev_destroy_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_destroy_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_pdev_status_handler
		pdev_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pdev_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_create_handler
		vdev_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_destroy_handler
		vdev_destroy_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_destroy_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_peer_free_notify_handler
		vdev_peer_free_notify_handler[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_vdev_status_handler
		vdev_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *vdev_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_create_handler
		peer_create_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_create_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_destroy_handler
		peer_destroy_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_destroy_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	wlan_objmgr_peer_status_handler
		peer_status_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *peer_status_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	struct wlan_objmgr_debug_info *debug_info;
	qdf_spinlock_t	global_lock;
};

#define MAX_SLEEP_ITERATION 5

extern struct wlan_objmgr_global *g_umac_glb_obj;

/**
 * wlan_objmgr_psoc_object_attach() - attach psoc to global object
 * @psoc - PSOC object
 *
 * attaches PSOC to global psoc list
 *
 * Return: SUCCESS
 *         Failure (Max supported PSOCs exceeded)
 */
QDF_STATUS wlan_objmgr_psoc_object_attach(
			struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_psoc_object_detach() - detach psoc from global object
 * @psoc - PSOC object
 *
 * detaches PSOC from global psoc list
 *
 * Return: SUCCESS
 *         Failure (if list is empty and PSOC is not present)
 */
QDF_STATUS wlan_objmgr_psoc_object_detach(
			struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_print_ref_ids() - Print ref counts of modules
 * @id - array of ref debug
 * @log_level - log level
 *
 * Itertes through array, and prints the ref count debug
 *
 * Return: nothing
 */
void wlan_objmgr_print_ref_ids(qdf_atomic_t *id,
				QDF_TRACE_LEVEL log_level);
#endif /* _WLAN_OBJMGR_GLOBAL_OBJ_I_H_ */
