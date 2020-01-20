/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <qdf_mem.h>
#include <qdf_module.h>
#include "wlan_objmgr_global_obj_i.h"
#include "wlan_objmgr_psoc_obj_i.h"
#include "wlan_objmgr_pdev_obj_i.h"


/**
 ** APIs to Create/Delete Global object APIs
 */
static QDF_STATUS wlan_objmgr_pdev_object_status(
		struct wlan_objmgr_pdev *pdev)
{
	uint8_t id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	wlan_pdev_obj_lock(pdev);
	/* Iterate through all components to derive the object status */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		/* If component disabled, Ignore */
		if (pdev->obj_status[id] == QDF_STATUS_COMP_DISABLED) {
			continue;
		/* If component operates in Async, status is Partially created,
			break */
		} else if (pdev->obj_status[id] == QDF_STATUS_COMP_ASYNC) {
			if (!pdev->pdev_comp_priv_obj[id]) {
				status = QDF_STATUS_COMP_ASYNC;
				break;
			}
		/* If component failed to allocate its object, treat it as
			failure, complete object need to be cleaned up */
		} else if ((pdev->obj_status[id] == QDF_STATUS_E_NOMEM) ||
			(pdev->obj_status[id] == QDF_STATUS_E_FAILURE)) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
	}
	wlan_pdev_obj_unlock(pdev);
	return status;
}

static QDF_STATUS wlan_objmgr_pdev_obj_free(struct wlan_objmgr_pdev *pdev)
{

	uint8_t pdev_id;

	if (!pdev) {
		obj_mgr_err("pdev obj is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	/* Detach PDEV from PSOC PDEV's list */
	if (wlan_objmgr_psoc_pdev_detach(pdev->pdev_objmgr.wlan_psoc, pdev) ==
						QDF_STATUS_E_FAILURE) {
		obj_mgr_err("PSOC PDEV detach failed: pdev-id: %d", pdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_spinlock_destroy(&pdev->pdev_lock);
	qdf_mem_free(pdev);

	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_pdev *wlan_objmgr_pdev_obj_create(
			struct wlan_objmgr_psoc *psoc,
			struct pdev_osif_priv *osdev_priv)
{
	struct wlan_objmgr_pdev *pdev;
	uint8_t id;
	wlan_objmgr_pdev_create_handler handler;
	wlan_objmgr_pdev_status_handler s_handler;
	void *arg;
	QDF_STATUS obj_status;

	if (!psoc) {
		obj_mgr_err("psoc is NULL");
		return NULL;
	}
	/* Allocate PDEV object's memory */
	pdev = qdf_mem_malloc(sizeof(*pdev));
	if (!pdev)
		return NULL;

	pdev->obj_state = WLAN_OBJ_STATE_ALLOCATED;
	/* Initialize PDEV spinlock */
	qdf_spinlock_create(&pdev->pdev_lock);
	/* Attach PDEV with PSOC */
	if (wlan_objmgr_psoc_pdev_attach(psoc, pdev)
				!= QDF_STATUS_SUCCESS) {
		obj_mgr_err("pdev psoc attach failed");
		qdf_spinlock_destroy(&pdev->pdev_lock);
		qdf_mem_free(pdev);
		return NULL;
	}
	/* Save PSOC object pointer in PDEV */
	wlan_pdev_set_psoc(pdev, psoc);
	/* Initialize PDEV's VDEV list, assign default values */
	qdf_list_create(&pdev->pdev_objmgr.wlan_vdev_list,
			WLAN_UMAC_PDEV_MAX_VDEVS);
	pdev->pdev_objmgr.wlan_vdev_count = 0;
	pdev->pdev_objmgr.max_vdev_count = WLAN_UMAC_PDEV_MAX_VDEVS;
	pdev->pdev_objmgr.wlan_peer_count = 0;
	pdev->pdev_objmgr.temp_peer_count = 0;
	pdev->pdev_objmgr.max_peer_count = wlan_psoc_get_max_peer_count(psoc);
	/* Save HDD/OSIF pointer */
	pdev->pdev_nif.pdev_ospriv = osdev_priv;
	qdf_atomic_init(&pdev->pdev_objmgr.ref_cnt);
	pdev->pdev_objmgr.print_cnt = 0;
	wlan_objmgr_pdev_get_ref(pdev, WLAN_OBJMGR_ID);
	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->pdev_create_handler[id];
		arg = g_umac_glb_obj->pdev_create_handler_arg[id];
		if (handler)
			pdev->obj_status[id] = handler(pdev, arg);
		else
			pdev->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_pdev_object_status(pdev);

	if (obj_status == QDF_STATUS_SUCCESS) {
		/* Object status is SUCCESS, Object is created */
		pdev->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Invoke component registered status handlers */
		for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
			s_handler = g_umac_glb_obj->pdev_status_handler[id];
			arg = g_umac_glb_obj->pdev_status_handler_arg[id];
			if (s_handler) {
				s_handler(pdev, arg,
					  QDF_STATUS_SUCCESS);
			}
		}
	/* Few components operates in Asynchrous communction, Object state
	partially created */
	} else if (obj_status == QDF_STATUS_COMP_ASYNC) {
		pdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
	/* Component object failed to be created, clean up the object */
	} else if (obj_status == QDF_STATUS_E_FAILURE) {
		/* Clean up the psoc */
		obj_mgr_err("PDEV component objects allocation failed");
		wlan_objmgr_pdev_obj_delete(pdev);
		return NULL;
	}

	obj_mgr_debug("Created pdev %d", pdev->pdev_objmgr.wlan_pdev_id);

	return pdev;
}
qdf_export_symbol(wlan_objmgr_pdev_obj_create);

static QDF_STATUS wlan_objmgr_pdev_obj_destroy(struct wlan_objmgr_pdev *pdev)
{
	uint8_t id;
	wlan_objmgr_pdev_destroy_handler handler;
	QDF_STATUS obj_status;
	void *arg;
	uint8_t pdev_id;

	if (!pdev) {
		obj_mgr_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_notify_destroy(pdev, WLAN_PDEV_OP);

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	wlan_print_pdev_info(pdev);
	obj_mgr_debug("Physically deleting pdev %d", pdev_id);

	if (pdev->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED) {
		obj_mgr_err("PDEV object delete is not invoked pdevid:%d objstate:%d",
			    pdev_id, pdev->obj_state);
		WLAN_OBJMGR_BUG(0);
	}

	/* Invoke registered destroy handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->pdev_destroy_handler[id];
		arg = g_umac_glb_obj->pdev_destroy_handler_arg[id];
		if (handler &&
		    (pdev->obj_status[id] == QDF_STATUS_SUCCESS ||
		     pdev->obj_status[id] == QDF_STATUS_COMP_ASYNC))
			pdev->obj_status[id] = handler(pdev, arg);
		else
			pdev->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_pdev_object_status(pdev);

	if (obj_status == QDF_STATUS_E_FAILURE) {
		obj_mgr_err("PDEV component objects destroy failed: pdev-id:%d",
				pdev_id);
		/* Ideally should not happen */
		/* This leads to memleak ??? how to handle */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		pdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	}

	/* Free PDEV object */
	return wlan_objmgr_pdev_obj_free(pdev);
}

QDF_STATUS wlan_objmgr_pdev_obj_delete(struct wlan_objmgr_pdev *pdev)
{
	uint8_t print_idx;

	if (!pdev) {
		obj_mgr_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	obj_mgr_debug("Logically deleting pdev %d",
		      pdev->pdev_objmgr.wlan_pdev_id);

	print_idx = qdf_get_pidx();
	wlan_objmgr_print_ref_ids(pdev->pdev_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_DEBUG);
	/*
	 * Update PDEV object state to LOGICALLY DELETED
	 * It prevents further access of this object
	 */
	wlan_pdev_obj_lock(pdev);
	pdev->obj_state = WLAN_OBJ_STATE_LOGICALLY_DELETED;
	wlan_pdev_obj_unlock(pdev);
	wlan_objmgr_notify_log_delete(pdev, WLAN_PDEV_OP);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_pdev_obj_delete);

/**
 ** APIs to attach/detach component objects
 */
QDF_STATUS wlan_objmgr_pdev_component_obj_attach(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status)
{
	uint8_t i;
	wlan_objmgr_pdev_status_handler s_hlr;
	void *a;
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		obj_mgr_err("component-id %d is not supported", id);
		return QDF_STATUS_MAXCOMP_FAIL;
	}
	wlan_pdev_obj_lock(pdev);
	/* If there is a valid entry, return failure */
	if (pdev->pdev_comp_priv_obj[id]) {
		obj_mgr_err("component-%d already have valid pointer", id);
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Save component's pointer and status */
	pdev->pdev_comp_priv_obj[id] = comp_priv_obj;
	pdev->obj_status[id] = status;

	wlan_pdev_obj_unlock(pdev);

	if (pdev->obj_state != WLAN_OBJ_STATE_PARTIALLY_CREATED)
		return QDF_STATUS_SUCCESS;
	/**
	 * If PDEV object status is partially created means, this API is
	 * invoked with differnt context, this block should be executed for
	 * async components only
	 */
	/* Derive status */
	obj_status = wlan_objmgr_pdev_object_status(pdev);
	/* STATUS_SUCCESS means, object is CREATED */
	if (obj_status == QDF_STATUS_SUCCESS)
		pdev->obj_state = WLAN_OBJ_STATE_CREATED;
	/* update state as CREATION failed, caller has to delete the
	PDEV object */
	else if (obj_status == QDF_STATUS_E_FAILURE)
		pdev->obj_state = WLAN_OBJ_STATE_CREATION_FAILED;
	/* Notify components about the CREATION success/failure */
	if ((obj_status == QDF_STATUS_SUCCESS) ||
	    (obj_status == QDF_STATUS_E_FAILURE)) {
		/* nofity object status */
		for (i = 0; i < WLAN_UMAC_MAX_COMPONENTS; i++) {
			s_hlr = g_umac_glb_obj->pdev_status_handler[i];
			a = g_umac_glb_obj->pdev_status_handler_arg[i];
			if (s_hlr)
				s_hlr(pdev, a, obj_status);
		}
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_pdev_component_obj_attach);

QDF_STATUS wlan_objmgr_pdev_component_obj_detach(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj)
{
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_pdev_obj_lock(pdev);
	/* If there is a invalid entry, return failure */
	if (pdev->pdev_comp_priv_obj[id] != comp_priv_obj) {
		pdev->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset pointers to NULL, update the status*/
	pdev->pdev_comp_priv_obj[id] = NULL;
	pdev->obj_status[id] = QDF_STATUS_SUCCESS;
	wlan_pdev_obj_unlock(pdev);

	/* If PDEV object status is partially destroyed means, this API is
	invoked with differnt context, this block should be executed for async
	components only */
	if ((pdev->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED) ||
	    (pdev->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)) {
		/* Derive object status */
		obj_status = wlan_objmgr_pdev_object_status(pdev);
		if (obj_status == QDF_STATUS_SUCCESS) {
			/*Update the status as Deleted, if full object
				deletion is in progress */
			if (pdev->obj_state ==
				WLAN_OBJ_STATE_PARTIALLY_DELETED)
				pdev->obj_state = WLAN_OBJ_STATE_DELETED;
			/* Move to creation state, since this component
			deletion alone requested */
			if (pdev->obj_state ==
				WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				pdev->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Object status is failure */
		} else if (obj_status == QDF_STATUS_E_FAILURE) {
			/*Update the status as Deletion failed, if full object
				deletion is in progress */
			if (pdev->obj_state ==
					WLAN_OBJ_STATE_PARTIALLY_DELETED)
				pdev->obj_state =
					WLAN_OBJ_STATE_DELETION_FAILED;
			/* Move to creation state, since this component
			deletion alone requested (do not block other
			components)*/
			if (pdev->obj_state ==
					WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				pdev->obj_state = WLAN_OBJ_STATE_CREATED;
		}

		/* Delete pdev object */
		if ((obj_status == QDF_STATUS_SUCCESS) &&
		    (pdev->obj_state == WLAN_OBJ_STATE_DELETED)) {
			/* Free PDEV object */
			return wlan_objmgr_pdev_obj_free(pdev);
		}
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_pdev_component_obj_detach);

/**
 ** APIs to operations on pdev objects
 */
static void wlan_objmgr_pdev_vdev_iterate_peers(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_vdev *vdev,
				wlan_objmgr_pdev_op_handler handler,
				void *arg, uint8_t lock_free_op,
				wlan_objmgr_ref_dbgid dbg_id)
{
	qdf_list_t *peer_list = NULL;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_objmgr_peer *peer_next = NULL;

	/* Iterating through vdev's peer list, so lock is
		needed */
	/* Get peer list of the vdev */
	peer_list = &vdev->vdev_objmgr.wlan_peer_list;
	if (peer_list) {
		peer = wlan_vdev_peer_list_peek_active_head(vdev, peer_list,
								dbg_id);
		while (peer) {
			/* Invoke the handler */
			handler(pdev, (void *)peer, arg);
			/* Get next peer pointer, increments the ref count */
			peer_next = wlan_peer_get_next_active_peer_of_vdev(vdev,
						peer_list, peer, dbg_id);
			wlan_objmgr_peer_release_ref(peer, dbg_id);
			peer = peer_next;
		}
	}
}

QDF_STATUS wlan_objmgr_pdev_iterate_obj_list(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_pdev_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_pdev_objmgr *objmgr = &pdev->pdev_objmgr;
	qdf_list_t *vdev_list = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_objmgr_vdev *vdev_next = NULL;

	/* VDEV list */
	vdev_list = &objmgr->wlan_vdev_list;

	switch (obj_type) {
	case WLAN_VDEV_OP:
		/* Iterate through all VDEV object, and invoke handler for each
			VDEV object */
		vdev = wlan_pdev_vdev_list_peek_active_head(pdev, vdev_list,
								dbg_id);
		while (vdev) {
			handler(pdev, (void *)vdev, arg);
			 /* Get next vdev, it increments ref of next vdev */
			vdev_next = wlan_vdev_get_next_active_vdev_of_pdev(
					pdev, vdev_list, vdev, dbg_id);
			wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			vdev = vdev_next;
		}
		break;
	case WLAN_PEER_OP:
		vdev = wlan_pdev_vdev_list_peek_active_head(pdev, vdev_list,
								dbg_id);
		while (vdev) {
			wlan_objmgr_pdev_vdev_iterate_peers(pdev, vdev,	handler,
						arg, lock_free_op, dbg_id);
			 /* Get next vdev, it increments ref of next vdev */
			vdev_next = wlan_vdev_get_next_active_vdev_of_pdev(
					pdev, vdev_list, vdev, dbg_id);
			wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			vdev = vdev_next;
		}
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_pdev_iterate_obj_list);

QDF_STATUS wlan_objmgr_trigger_pdev_comp_priv_object_creation(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_pdev_create_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* Component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_pdev_obj_lock(pdev);
	/* If component object is already created, delete old
		component object, then invoke creation */
	if (pdev->pdev_comp_priv_obj[id]) {
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_pdev_obj_unlock(pdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->pdev_create_handler[id];
	arg = g_umac_glb_obj->pdev_create_handler_arg[id];
	if (handler)
		pdev->obj_status[id] = handler(pdev, arg);
	else
		return QDF_STATUS_E_FAILURE;
	/* If object status is created, then only handle this object status */
	if (pdev->obj_state == WLAN_OBJ_STATE_CREATED) {
		/* Derive object status */
		obj_status = wlan_objmgr_pdev_object_status(pdev);
		/* Move PDEV object state to Partially created state */
		if (obj_status == QDF_STATUS_COMP_ASYNC) {
			/*TODO atomic */
			pdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
		}
	}
	return obj_status;
}

QDF_STATUS wlan_objmgr_trigger_pdev_comp_priv_object_deletion(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_pdev_destroy_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_pdev_obj_lock(pdev);
	/* Component object was never created, invalid operation */
	if (!pdev->pdev_comp_priv_obj[id]) {
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_pdev_obj_unlock(pdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->pdev_destroy_handler[id];
	arg = g_umac_glb_obj->pdev_destroy_handler_arg[id];
	if (handler)
		pdev->obj_status[id] = handler(pdev, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (pdev->obj_state == WLAN_OBJ_STATE_CREATED) {
		obj_status = wlan_objmgr_pdev_object_status(pdev);
		/* move object state to DEL progress */
		if (obj_status == QDF_STATUS_COMP_ASYNC)
			pdev->obj_state = WLAN_OBJ_STATE_COMP_DEL_PROGRESS;
	}
	return obj_status;
}

static void wlan_obj_pdev_vdevlist_add_tail(qdf_list_t *obj_list,
				struct wlan_objmgr_vdev *obj)
{
	qdf_list_insert_back(obj_list, &obj->vdev_node);
}

static QDF_STATUS wlan_obj_pdev_vdevlist_remove_vdev(
				qdf_list_t *obj_list,
				struct wlan_objmgr_vdev *vdev)
{
	qdf_list_node_t *vdev_node = NULL;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;
	/* get vdev list node element */
	vdev_node = &vdev->vdev_node;
	/* list is empty, return failure */
	if (qdf_list_remove_node(obj_list, vdev_node) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_pdev_vdev_attach(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev_objmgr *objmgr = &pdev->pdev_objmgr;

	wlan_pdev_obj_lock(pdev);
	/* If Max vdev count exceeds, return failure */
	if (objmgr->wlan_vdev_count >= objmgr->max_vdev_count) {
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Add vdev to pdev's vdev list */
	wlan_obj_pdev_vdevlist_add_tail(&objmgr->wlan_vdev_list, vdev);
	/* Increment pdev ref count to make sure it won't be destroyed before */
	wlan_objmgr_pdev_get_ref(pdev, WLAN_OBJMGR_ID);
	/* Increment vdev count of pdev */
	objmgr->wlan_vdev_count++;
	wlan_pdev_obj_unlock(pdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_pdev_vdev_detach(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev_objmgr *objmgr = &pdev->pdev_objmgr;

	wlan_pdev_obj_lock(pdev);
	/* if vdev count is 0, return failure */
	if (objmgr->wlan_vdev_count == 0) {
		wlan_pdev_obj_unlock(pdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* remove vdev from pdev's vdev list */
	wlan_obj_pdev_vdevlist_remove_vdev(&objmgr->wlan_vdev_list, vdev);
	/* decrement vdev count */
	objmgr->wlan_vdev_count--;
	wlan_pdev_obj_unlock(pdev);
	/* Decrement pdev ref count since vdev is releasing reference */
	wlan_objmgr_pdev_release_ref(pdev, WLAN_OBJMGR_ID);
	return QDF_STATUS_SUCCESS;
}

void *wlan_objmgr_pdev_get_comp_private_obj(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id)
{
	void *comp_priv_obj;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		QDF_BUG(0);
		return NULL;
	}

	if (!pdev) {
		QDF_BUG(0);
		return NULL;
	}

	comp_priv_obj = pdev->pdev_comp_priv_obj[id];

	return comp_priv_obj;
}

qdf_export_symbol(wlan_objmgr_pdev_get_comp_private_obj);

void wlan_objmgr_pdev_get_ref(struct wlan_objmgr_pdev *pdev,
			      wlan_objmgr_ref_dbgid id)
{
	if (!pdev) {
		obj_mgr_err("pdev obj is NULL");
		QDF_ASSERT(0);
		return;
	}
	qdf_atomic_inc(&pdev->pdev_objmgr.ref_cnt);
	qdf_atomic_inc(&pdev->pdev_objmgr.ref_id_dbg[id]);
}

qdf_export_symbol(wlan_objmgr_pdev_get_ref);

QDF_STATUS wlan_objmgr_pdev_try_get_ref(struct wlan_objmgr_pdev *pdev,
					wlan_objmgr_ref_dbgid id)
{
	uint8_t pdev_id;

	if (!pdev) {
		obj_mgr_err("pdev obj is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_pdev_obj_lock(pdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (pdev->obj_state != WLAN_OBJ_STATE_CREATED) {
		wlan_pdev_obj_unlock(pdev);
		if (pdev->pdev_objmgr.print_cnt++ <=
				WLAN_OBJMGR_RATELIMIT_THRESH)
			obj_mgr_err(
			"[Ref id: %d] pdev [%d] is not in Created(st:%d)",
					id, pdev_id, pdev->obj_state);
		return QDF_STATUS_E_RESOURCES;
	}

	wlan_objmgr_pdev_get_ref(pdev, id);
	wlan_pdev_obj_unlock(pdev);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_objmgr_pdev_try_get_ref);

void wlan_objmgr_pdev_release_ref(struct wlan_objmgr_pdev *pdev,
				  wlan_objmgr_ref_dbgid id)
{
	uint8_t pdev_id;

	if (!pdev) {
		obj_mgr_err("pdev obj is NULL");
		QDF_ASSERT(0);
		return;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (!qdf_atomic_read(&pdev->pdev_objmgr.ref_id_dbg[id])) {
		obj_mgr_err("pdev (id:%d)ref cnt was not taken by %d",
			    pdev_id, id);
		wlan_objmgr_print_ref_ids(pdev->pdev_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_FATAL);
		WLAN_OBJMGR_BUG(0);
		return;
	}

	if (!qdf_atomic_read(&pdev->pdev_objmgr.ref_cnt)) {
		obj_mgr_err("pdev ref cnt is 0: pdev-id:%d", pdev_id);
		WLAN_OBJMGR_BUG(0);
		return;
	}

	qdf_atomic_dec(&pdev->pdev_objmgr.ref_id_dbg[id]);
	/* Decrement ref count, free pdev, if ref count == 0 */
	if (qdf_atomic_dec_and_test(&pdev->pdev_objmgr.ref_cnt))
		wlan_objmgr_pdev_obj_destroy(pdev);
}

qdf_export_symbol(wlan_objmgr_pdev_release_ref);

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_pdev_get_first_vdev_debug(
		struct wlan_objmgr_pdev *pdev,
		wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_pdev_objmgr *objmgr = &pdev->pdev_objmgr;
	qdf_list_t *vdev_list = NULL;
	struct wlan_objmgr_vdev *vdev;
	qdf_list_node_t *node = NULL;
	qdf_list_node_t *prev_node = NULL;

	wlan_pdev_obj_lock(pdev);

	/* VDEV list */
	vdev_list = &objmgr->wlan_vdev_list;
	if (qdf_list_peek_front(vdev_list, &node) != QDF_STATUS_SUCCESS) {
		wlan_pdev_obj_unlock(pdev);
		return NULL;
	}

	do {
		vdev = qdf_container_of(node, struct wlan_objmgr_vdev,
					vdev_node);
		if (wlan_objmgr_vdev_try_get_ref_debug(vdev,
						       dbg_id, func, line) ==
						QDF_STATUS_SUCCESS) {
			wlan_pdev_obj_unlock(pdev);
			return vdev;
		}

		prev_node = node;
	} while (qdf_list_peek_next(vdev_list, prev_node, &node) ==
						QDF_STATUS_SUCCESS);

	wlan_pdev_obj_unlock(pdev);

	return NULL;
}

qdf_export_symbol(wlan_objmgr_pdev_get_first_vdev_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_pdev_get_first_vdev(
		struct wlan_objmgr_pdev *pdev,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_pdev_objmgr *objmgr = &pdev->pdev_objmgr;
	qdf_list_t *vdev_list = NULL;
	struct wlan_objmgr_vdev *vdev;
	qdf_list_node_t *node = NULL;
	qdf_list_node_t *prev_node = NULL;

	wlan_pdev_obj_lock(pdev);

	/* VDEV list */
	vdev_list = &objmgr->wlan_vdev_list;
	if (qdf_list_peek_front(vdev_list, &node) != QDF_STATUS_SUCCESS) {
		wlan_pdev_obj_unlock(pdev);
		return NULL;
	}

	do {
		vdev = qdf_container_of(node, struct wlan_objmgr_vdev,
					vdev_node);
		if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) ==
						QDF_STATUS_SUCCESS) {
			wlan_pdev_obj_unlock(pdev);
			return vdev;
		}

		prev_node = node;
	} while (qdf_list_peek_next(vdev_list, prev_node, &node) ==
						QDF_STATUS_SUCCESS);

	wlan_pdev_obj_unlock(pdev);

	return NULL;
}

qdf_export_symbol(wlan_objmgr_pdev_get_first_vdev);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev_debug(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);

	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev id matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (wlan_vdev_get_id(vdev) == vdev_id) {
			if (wlan_objmgr_vdev_try_get_ref_debug(vdev, dbg_id,
							       func, line) !=
				QDF_STATUS_SUCCESS)
				vdev = NULL;

			wlan_pdev_obj_unlock(pdev);
			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);
	return NULL;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_pdev_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);

	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev id matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (wlan_vdev_get_id(vdev) == vdev_id) {
			if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
				vdev = NULL;

			wlan_pdev_obj_unlock(pdev);
			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);
	return NULL;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_pdev);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev_no_state_debug(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);

	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev id matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (wlan_vdev_get_id(vdev) == vdev_id) {
			wlan_objmgr_vdev_get_ref_debug(vdev, dbg_id,
						       func, line);
			wlan_pdev_obj_unlock(pdev);

			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_pdev_no_state_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev_no_state(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);

	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev id matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (wlan_vdev_get_id(vdev) == vdev_id) {
			wlan_objmgr_vdev_get_ref(vdev, dbg_id);
			wlan_pdev_obj_unlock(pdev);

			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_pdev_no_state);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_pdev_debug(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id,
		const char *fnc, int ln)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);
	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev macaddr matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (QDF_IS_STATUS_SUCCESS(
		    WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr))) {
			if (QDF_IS_STATUS_SUCCESS(
				wlan_objmgr_vdev_try_get_ref_debug(vdev, dbg_id,
								   fnc, ln))) {
				wlan_pdev_obj_unlock(pdev);
				return vdev;
			}
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_pdev(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);
	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev macaddr matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (QDF_IS_STATUS_SUCCESS(
		    WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr))) {
			if (QDF_IS_STATUS_SUCCESS(
				wlan_objmgr_vdev_try_get_ref(vdev, dbg_id))) {
				wlan_pdev_obj_unlock(pdev);
				return vdev;
			}
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev
	*wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state_debug(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);
	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev macaddr matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr)
					== QDF_STATUS_SUCCESS) {
			wlan_objmgr_vdev_get_ref_debug(vdev, dbg_id,
						       func, line);
			wlan_pdev_obj_unlock(pdev);

			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	struct wlan_objmgr_pdev_objmgr *objmgr;
	qdf_list_t *vdev_list;

	wlan_pdev_obj_lock(pdev);
	objmgr = &pdev->pdev_objmgr;
	vdev_list = &objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);
	/**
	 * Iterate through pdev's vdev list, till vdev macaddr matches with
	 * entry of vdev list
	 */
	while (vdev) {
		if (WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr)
					== QDF_STATUS_SUCCESS) {
			wlan_objmgr_vdev_get_ref(vdev, dbg_id);
			wlan_pdev_obj_unlock(pdev);

			return vdev;
		}
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}
#endif

#ifdef WLAN_OBJMGR_DEBUG
void wlan_print_pdev_info(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_pdev_objmgr *pdev_objmgr;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *vdev_next;
	qdf_list_t *vdev_list;
	uint16_t index = 0;

	pdev_objmgr = &pdev->pdev_objmgr;

	obj_mgr_debug("pdev: %pK", pdev);
	obj_mgr_debug("wlan_pdev_id: %d", pdev_objmgr->wlan_pdev_id);
	obj_mgr_debug("wlan_vdev_count: %d", pdev_objmgr->wlan_vdev_count);
	obj_mgr_debug("max_vdev_count: %d", pdev_objmgr->max_vdev_count);
	obj_mgr_debug("wlan_peer_count: %d", pdev_objmgr->wlan_peer_count);
	obj_mgr_debug("max_peer_count: %d", pdev_objmgr->max_peer_count);
	obj_mgr_debug("temp_peer_count: %d", pdev_objmgr->temp_peer_count);
	obj_mgr_debug("wlan_psoc: %pK", pdev_objmgr->wlan_psoc);
	obj_mgr_debug("ref_cnt: %d", qdf_atomic_read(&pdev_objmgr->ref_cnt));

	wlan_pdev_obj_lock(pdev);
	vdev_list = &pdev_objmgr->wlan_vdev_list;
	/* Get first vdev */
	vdev = wlan_pdev_vdev_list_peek_head(vdev_list);

	while (vdev) {
		obj_mgr_debug("wlan_vdev_list[%d]: %pK", index, vdev);
		wlan_print_vdev_info(vdev);
		index++;
		/* get next vdev */
		vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
		vdev = vdev_next;
	}
	wlan_pdev_obj_unlock(pdev);
}

qdf_export_symbol(wlan_print_pdev_info);
#endif
