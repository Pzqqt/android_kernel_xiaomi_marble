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
  * DOC: Public APIs to perform operations on Global objects
  */

#include <wlan_objmgr_global_obj_i.h>
#include <wlan_objmgr_global_obj.h>
#include "qdf_mem.h"

/* Global object, it is declared globally */
struct wlan_objmgr_global *g_umac_glb_obj;
/* Component Name table */
const char *wlan_umac_component_name[] = {
	"MLME",
	"SCAN_MGR",
	"SCAN_CACHE",
	"MGMT_TXRX",
	"",
};
/*
** APIs to Create/Delete Global object APIs
*/
QDF_STATUS wlan_objmgr_global_obj_create(void)
{
	struct wlan_objmgr_global *umac_global_obj;

	/* If it is already created, ignore */
	if (g_umac_glb_obj != NULL) {
		qdf_print("%s: Global object is already created\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Allocation of memory for Global object */
	umac_global_obj = (struct wlan_objmgr_global *)qdf_mem_malloc(
				sizeof(*umac_global_obj));
	if (umac_global_obj == NULL) {
		qdf_print("%s: Global object alloc failed due to malloc\n",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}
	/* Store Global object pointer in Global variable */
	g_umac_glb_obj = umac_global_obj;
	/* Initialize spinlock */
	qdf_spinlock_create(&g_umac_glb_obj->global_lock);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_global_obj_create);

QDF_STATUS wlan_objmgr_global_obj_delete(void)
{
	/* If it is already deleted */
	if (g_umac_glb_obj == NULL) {
		qdf_print("%s: Global object is not allocated\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* TODO: Do we need to check, if any object
	*  is not freed before this is called ??
	*  ideally, init/deinit module should take care of freeing */
	/* Initialize spinlock */
	qdf_spinlock_destroy(&g_umac_glb_obj->global_lock);
	/* Free Global object memory */
	qdf_mem_free(g_umac_glb_obj);
	/* Reset Global variable to NULL */
	g_umac_glb_obj = NULL;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_global_obj_delete);

/**
 ** APIs to register/unregister handlers
 */
QDF_STATUS wlan_objmgr_register_psoc_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_create_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->psoc_create_handler[id] = handler;
	g_umac_glb_obj->psoc_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_psoc_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->psoc_create_handler[id] = NULL;
	g_umac_glb_obj->psoc_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_psoc_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_delete_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->psoc_delete_handler[id] = handler;
	g_umac_glb_obj->psoc_delete_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_psoc_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_delete_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->psoc_delete_handler[id] = NULL;
	g_umac_glb_obj->psoc_delete_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_psoc_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_status_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->psoc_status_handler[id] = handler;
	g_umac_glb_obj->psoc_status_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_psoc_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->psoc_status_handler[id] = NULL;
	g_umac_glb_obj->psoc_status_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_register_pdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_create_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->pdev_create_handler[id] = handler;
	g_umac_glb_obj->pdev_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_pdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->pdev_create_handler[id] = NULL;
	g_umac_glb_obj->pdev_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_pdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_delete_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->pdev_delete_handler[id] = handler;
	g_umac_glb_obj->pdev_delete_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_pdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_delete_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for component %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->pdev_delete_handler[id] = NULL;
	g_umac_glb_obj->pdev_delete_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_pdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_status_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->pdev_status_handler[id] = handler;
	g_umac_glb_obj->pdev_status_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_pdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for component %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->pdev_status_handler[id] = NULL;
	g_umac_glb_obj->pdev_status_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_register_vdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_create_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->vdev_create_handler[id] = handler;
	g_umac_glb_obj->vdev_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_vdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_create_handler[id] = NULL;
	g_umac_glb_obj->vdev_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_vdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_delete_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->vdev_delete_handler[id] = handler;
	g_umac_glb_obj->vdev_delete_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_vdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_delete_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_delete_handler[id] = NULL;
	g_umac_glb_obj->vdev_delete_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_vdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_status_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->vdev_status_handler[id] = handler;
	g_umac_glb_obj->vdev_status_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_vdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for component %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_status_handler[id] = NULL;
	g_umac_glb_obj->vdev_status_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_register_peer_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_create_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->peer_create_handler[id] = handler;
	g_umac_glb_obj->peer_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_unregister_peer_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->peer_create_handler[id] = NULL;
	g_umac_glb_obj->peer_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_peer_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_delete_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->peer_delete_handler[id] = handler;
	g_umac_glb_obj->peer_delete_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_peer_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_delete_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_delete_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->peer_delete_handler[id] = NULL;
	g_umac_glb_obj->peer_delete_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_peer_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_status_handler[id] != NULL) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is already registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->peer_status_handler[id] = handler;
	g_umac_glb_obj->peer_status_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_peer_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		qdf_print("%s: component %d is out of range\n", __func__, id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		qdf_print("%s:callback for comp %d is not registered\n",
			  __func__, id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->peer_status_handler[id] = NULL;
	g_umac_glb_obj->peer_status_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_psoc_object_attach(struct wlan_objmgr_psoc *psoc)
{
	uint8_t index = 0;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* Find free slot in PSOC table, store the PSOC */
	while (index < WLAN_OBJMGR_MAX_DEVICES) {
		if (g_umac_glb_obj->psoc[index] == NULL) {
			/* Found free slot, store psoc */
			g_umac_glb_obj->psoc[index] = psoc;
			status = QDF_STATUS_SUCCESS;
			break;
		}
		index++;
	}
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return status;
}

QDF_STATUS wlan_objmgr_psoc_object_detach(struct wlan_objmgr_psoc *psoc)
{
	uint8_t index = 0;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	while (index < WLAN_OBJMGR_MAX_DEVICES) {
		if (g_umac_glb_obj->psoc[index] == psoc) {
			/* found psoc, store NULL */
			g_umac_glb_obj->psoc[index] = NULL;
			status = QDF_STATUS_SUCCESS;
			break;
		}
		index++;
	}
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return status;
}

QDF_STATUS wlan_objmgr_global_obj_can_deleted(void)
{
	uint8_t index = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* Check whether all PSOCs are freed */
	while (index < WLAN_OBJMGR_MAX_DEVICES) {
		if (g_umac_glb_obj->psoc[index] != NULL) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
		index++;
	}
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return status;
}
EXPORT_SYMBOL(wlan_objmgr_global_obj_can_deleted);
