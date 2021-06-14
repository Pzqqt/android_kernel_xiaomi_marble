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
  * DOC: Public APIs to perform operations on Global objects
  */

#include "wlan_objmgr_global_obj_i.h"
#include <wlan_objmgr_global_obj.h>
#include "wlan_objmgr_debug.h"
#include "wlan_objmgr_psoc_obj.h"
#include "qdf_mem.h"
#include <qdf_module.h>

/* Global object, it is declared globally */
struct wlan_objmgr_global *g_umac_glb_obj;

qdf_export_symbol(g_umac_glb_obj);

/*
** APIs to Create/Delete Global object APIs
*/
QDF_STATUS wlan_objmgr_global_obj_init(void)
{
	struct wlan_objmgr_global *umac_global_obj;

	/* If it is already created, ignore */
	if (g_umac_glb_obj) {
		obj_mgr_err("Global object is already created");
		return QDF_STATUS_E_FAILURE;
	}

	/* Allocation of memory for Global object */
	umac_global_obj = (struct wlan_objmgr_global *)qdf_mem_malloc(
				sizeof(*umac_global_obj));
	if (!umac_global_obj)
		return QDF_STATUS_E_NOMEM;

	/* Store Global object pointer in Global variable */
	g_umac_glb_obj = umac_global_obj;
	/* Initialize spinlock */
	qdf_spinlock_create(&g_umac_glb_obj->global_lock);
	wlan_objmgr_debug_info_init();

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_global_obj_init);

QDF_STATUS wlan_objmgr_global_obj_deinit(void)
{
	/* If it is already destroyed */
	if (!g_umac_glb_obj) {
		obj_mgr_err("Global object is not allocated");
		return QDF_STATUS_E_FAILURE;
	}

	wlan_objmgr_debug_info_deinit();

	if (QDF_STATUS_SUCCESS == wlan_objmgr_global_obj_can_destroyed()) {
		qdf_spinlock_destroy(&g_umac_glb_obj->global_lock);
		qdf_mem_free(g_umac_glb_obj);
		g_umac_glb_obj = NULL;
	} else {
		obj_mgr_err("PSOCs are leaked can't free global objmgr ctx");
		WLAN_OBJMGR_BUG(0);
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_global_obj_deinit);

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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_create_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->psoc_create_handler[id] = handler;
	g_umac_glb_obj->psoc_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_psoc_create_handler);

QDF_STATUS wlan_objmgr_unregister_psoc_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->psoc_create_handler[id] = NULL;
	g_umac_glb_obj->psoc_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_psoc_create_handler);

QDF_STATUS wlan_objmgr_register_psoc_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_destroy_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->psoc_destroy_handler[id] = handler;
	g_umac_glb_obj->psoc_destroy_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_psoc_destroy_handler);

QDF_STATUS wlan_objmgr_unregister_psoc_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_destroy_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->psoc_destroy_handler[id] = NULL;
	g_umac_glb_obj->psoc_destroy_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_psoc_destroy_handler);

QDF_STATUS wlan_objmgr_register_psoc_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->psoc_status_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->psoc_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_create_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->pdev_create_handler[id] = handler;
	g_umac_glb_obj->pdev_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_pdev_create_handler);

QDF_STATUS wlan_objmgr_unregister_pdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->pdev_create_handler[id] = NULL;
	g_umac_glb_obj->pdev_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_pdev_create_handler);

QDF_STATUS wlan_objmgr_register_pdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_destroy_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->pdev_destroy_handler[id] = handler;
	g_umac_glb_obj->pdev_destroy_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_pdev_destroy_handler);

QDF_STATUS wlan_objmgr_unregister_pdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_destroy_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for Component %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->pdev_destroy_handler[id] = NULL;
	g_umac_glb_obj->pdev_destroy_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_pdev_destroy_handler);

QDF_STATUS wlan_objmgr_register_pdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->pdev_status_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->pdev_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for Component %d is not registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_create_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->vdev_create_handler[id] = handler;
	g_umac_glb_obj->vdev_create_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_vdev_create_handler);

QDF_STATUS wlan_objmgr_unregister_vdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_create_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_create_handler[id] = NULL;
	g_umac_glb_obj->vdev_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_vdev_create_handler);

QDF_STATUS wlan_objmgr_register_vdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_destroy_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->vdev_destroy_handler[id] = handler;
	g_umac_glb_obj->vdev_destroy_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_register_vdev_destroy_handler);

QDF_STATUS wlan_objmgr_unregister_vdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_destroy_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_destroy_handler[id] = NULL;
	g_umac_glb_obj->vdev_destroy_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_unregister_vdev_destroy_handler);

QDF_STATUS wlan_objmgr_register_vdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_status_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_status_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for Component %d is not registered", id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->vdev_status_handler[id] = NULL;
	g_umac_glb_obj->vdev_status_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_vdev_peer_free_notify_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_peer_free_notify_handler handler)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		WLAN_OBJMGR_BUG(0);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->vdev_peer_free_notify_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler in Global object table */
	g_umac_glb_obj->vdev_peer_free_notify_handler[id] = handler;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_vdev_peer_free_notify_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_peer_free_notify_handler handler)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		WLAN_OBJMGR_BUG(0);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->vdev_peer_free_notify_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for Component %d is not registered", id);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers to NULL */
	g_umac_glb_obj->vdev_peer_free_notify_handler[id] = NULL;

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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_create_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_create_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->peer_create_handler[id] = NULL;
	g_umac_glb_obj->peer_create_handler_arg[id] = NULL;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_register_peer_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_destroy_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Store handler and args in Global object table */
	g_umac_glb_obj->peer_destroy_handler[id] = handler;
	g_umac_glb_obj->peer_destroy_handler_arg[id] = arg;

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_unregister_peer_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_destroy_handler handler,
		void *arg)
{
	/* If id is not within valid range, return */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_destroy_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset handlers, and args to NULL */
	g_umac_glb_obj->peer_destroy_handler[id] = NULL;
	g_umac_glb_obj->peer_destroy_handler_arg[id] = NULL;

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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is a valid entry, return failure */
	if (g_umac_glb_obj->peer_status_handler[id]) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is already registered", id);
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
		obj_mgr_err("Component %d is out of range", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* If there is an invalid entry, return failure */
	if (g_umac_glb_obj->peer_status_handler[id] != handler) {
		qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
		obj_mgr_err("Callback for comp %d is not registered", id);
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
		if (!g_umac_glb_obj->psoc[index]) {
			/* Found free slot, store psoc */
			g_umac_glb_obj->psoc[index] = psoc;
			psoc->soc_objmgr.psoc_id = index;
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
	uint8_t psoc_id;

	psoc_id = psoc->soc_objmgr.psoc_id;
	QDF_BUG(psoc_id < WLAN_OBJMGR_MAX_DEVICES);
	if (psoc_id >= WLAN_OBJMGR_MAX_DEVICES)
		return QDF_STATUS_E_INVAL;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	g_umac_glb_obj->psoc[psoc_id] = NULL;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_global_obj_can_destroyed(void)
{
	uint8_t index = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	/* Check whether all PSOCs are freed */
	while (index < WLAN_OBJMGR_MAX_DEVICES) {
		if (g_umac_glb_obj->psoc[index]) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
		index++;
	}
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	return status;
}
qdf_export_symbol(wlan_objmgr_global_obj_can_destroyed);

void wlan_objmgr_print_ref_ids(qdf_atomic_t *id,
				QDF_TRACE_LEVEL log_level)
{
	uint32_t i;
	uint32_t pending_ref;

	obj_mgr_log_level(log_level, "Pending references of object");
	for (i = 0; i < WLAN_REF_ID_MAX; i++) {
		pending_ref = qdf_atomic_read(&id[i]);
		if (pending_ref)
			obj_mgr_log_level(log_level, "%s -- %d",
				string_from_dbgid(i), pending_ref);
	}

	return;
}

QDF_STATUS wlan_objmgr_iterate_psoc_list(
		wlan_objmgr_psoc_handler handler,
		void *arg, wlan_objmgr_ref_dbgid dbg_id)
{
	uint8_t index = 0;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);

	while (index < WLAN_OBJMGR_MAX_DEVICES) {
		if (g_umac_glb_obj->psoc[index]) {
			handler((void *)g_umac_glb_obj->psoc[index],
				arg, index);
		}
		index++;
	}

	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_objmgr_iterate_psoc_list);

#ifdef WLAN_FEATURE_11BE_MLO
struct mlo_mgr_context *wlan_objmgr_get_mlo_ctx(void)
{
	return g_umac_glb_obj->mlo_ctx;
}

void wlan_objmgr_set_mlo_ctx(struct mlo_mgr_context *ctx)
{
	g_umac_glb_obj->mlo_ctx = ctx;
}
#endif
