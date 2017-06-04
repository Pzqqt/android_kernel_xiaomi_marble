/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_objmgr_global_obj_i.h"
#include "wlan_objmgr_psoc_obj_i.h"
#include "wlan_objmgr_pdev_obj_i.h"

/**
 ** APIs to Create/Delete Global object APIs
 */
static QDF_STATUS wlan_objmgr_psoc_object_status(
			struct wlan_objmgr_psoc *psoc)
{
	uint8_t id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	wlan_psoc_obj_lock(psoc);
	/* Iterate through all components to derive the object status */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		/* If component disabled, Ignore */
		if (psoc->obj_status[id] == QDF_STATUS_COMP_DISABLED)
			continue;
		/* If component operates in Async, status is Partially created,
		 * break
		 */
		else if (psoc->obj_status[id] == QDF_STATUS_COMP_ASYNC) {
			if (psoc->soc_comp_priv_obj[id] == NULL) {
				status = QDF_STATUS_COMP_ASYNC;
				break;
			}
		/*
		 * If component failed to allocate its object, treat it as
		 * failure, complete object need to be cleaned up
		 */
		} else if ((psoc->obj_status[id] == QDF_STATUS_E_NOMEM) ||
			(psoc->obj_status[id] == QDF_STATUS_E_FAILURE)) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
	}
	wlan_psoc_obj_unlock(psoc);

	return status;
}

static void wlan_objmgr_psoc_peer_list_init(struct wlan_peer_list *peer_list)
{
	uint8_t i;

	qdf_spinlock_create(&peer_list->peer_list_lock);
	for (i = 0; i < WLAN_PEER_HASHSIZE; i++)
		qdf_list_create(&peer_list->peer_hash[i],
				WLAN_UMAC_PSOC_MAX_PEERS);
}

static void wlan_objmgr_psoc_peer_list_deinit(struct wlan_peer_list *peer_list)
{
	uint8_t i;

	/* deinit the lock */
	qdf_spinlock_destroy(&peer_list->peer_list_lock);
	for (i = 0; i < WLAN_PEER_HASHSIZE; i++)
		qdf_list_destroy(&peer_list->peer_hash[i]);
}

static QDF_STATUS wlan_objmgr_psoc_obj_free(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_psoc_host_service_ext_param *ext_param =
		&(psoc->ext_service_param.service_ext_param);

	/* Detach PSOC from global object's psoc list  */
	if (wlan_objmgr_psoc_object_detach(psoc) == QDF_STATUS_E_FAILURE) {
		obj_mgr_err("PSOC object detach failed");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_psoc_peer_list_deinit(&psoc->soc_objmgr.peer_list);
	wlan_objmgr_ext_service_ready_chainmask_table_free(ext_param);

	qdf_spinlock_destroy(&psoc->psoc_lock);
	qdf_mem_free(psoc);

	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_psoc *wlan_objmgr_psoc_obj_create(uint32_t phy_version,
						WLAN_DEV_TYPE dev_type)
{
	uint8_t id;
	struct wlan_objmgr_psoc *psoc = NULL;
	wlan_objmgr_psoc_create_handler handler;
	wlan_objmgr_psoc_status_handler stat_handler;
	struct wlan_objmgr_psoc_objmgr *objmgr;
	QDF_STATUS obj_status;
	void *arg;

	psoc = qdf_mem_malloc(sizeof(*psoc));
	if (psoc == NULL) {
		obj_mgr_err("PSOC allocation failed");
		return NULL;
	}
	qdf_spinlock_create(&psoc->psoc_lock);
	/* Initialize with default values */
	objmgr = &psoc->soc_objmgr;
	objmgr->wlan_pdev_count = 0;
	objmgr->wlan_vdev_count = 0;
	objmgr->max_vdev_count = WLAN_UMAC_PSOC_MAX_VDEVS;
	objmgr->wlan_peer_count = 0;
	qdf_atomic_init(&objmgr->ref_cnt);
	/* set phy version, dev_type in psoc */
	wlan_psoc_set_nif_phy_version(psoc, phy_version);
	wlan_psoc_set_dev_type(psoc, dev_type);
	/* Initialize peer list */
	wlan_objmgr_psoc_peer_list_init(&objmgr->peer_list);
	wlan_objmgr_psoc_get_ref(psoc, WLAN_OBJMGR_ID);
	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->psoc_create_handler[id];
		arg = g_umac_glb_obj->psoc_create_handler_arg[id];
		if (handler != NULL)
			psoc->obj_status[id] = handler(psoc, arg);
		else
			psoc->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_psoc_object_status(psoc);

	if (obj_status == QDF_STATUS_SUCCESS) {
		/* Object status is SUCCESS, Object is created */
		psoc->obj_state = WLAN_OBJ_STATE_CREATED;
		for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
			stat_handler = g_umac_glb_obj->psoc_status_handler[id];
			arg = g_umac_glb_obj->psoc_status_handler_arg[id];
			if (stat_handler != NULL)
				stat_handler(psoc, arg,
					     QDF_STATUS_SUCCESS);
		}
	} else if (obj_status == QDF_STATUS_COMP_ASYNC) {
		/*
		 * Few components operates in Asynchrous communction
		 * Object state partially created
		 */
		psoc->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
	} else if (obj_status == QDF_STATUS_E_FAILURE) {
		/* Component object failed to be created, clean up the object */
		obj_mgr_err("PSOC component objects allocation failed");
		/* Clean up the psoc */
		wlan_objmgr_psoc_obj_delete(psoc);
		return NULL;
	}

	if (wlan_objmgr_psoc_object_attach(psoc) !=
				QDF_STATUS_SUCCESS) {
		obj_mgr_err("PSOC object attach failed");
		wlan_objmgr_psoc_obj_delete(psoc);
		return NULL;
	}

	return psoc;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_obj_create);

static QDF_STATUS wlan_objmgr_psoc_obj_destroy(struct wlan_objmgr_psoc *psoc)
{
	uint8_t id;
	wlan_objmgr_psoc_destroy_handler handler;
	QDF_STATUS obj_status;
	void *arg;

	if (psoc == NULL) {
		obj_mgr_err("psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED) {
		obj_mgr_err("psoc object delete is not invoked");
		WLAN_OBJMGR_BUG(0);
	}

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->psoc_destroy_handler[id];
		arg = g_umac_glb_obj->psoc_destroy_handler_arg[id];
		if (handler != NULL)
			psoc->obj_status[id] = handler(psoc, arg);
		else
			psoc->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_psoc_object_status(psoc);

	if (obj_status == QDF_STATUS_E_FAILURE) {
		obj_mgr_err("PSOC component object free failed");
		/* Ideally should not happen
		 * This leads to memleak, BUG_ON to find which component
		 * delete notification failed and fix it.
		 */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		psoc->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	}

	/* Free psoc object */
	return wlan_objmgr_psoc_obj_free(psoc);
}


QDF_STATUS wlan_objmgr_psoc_obj_delete(struct wlan_objmgr_psoc *psoc)
{
	if (psoc == NULL) {
		obj_mgr_err("psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * Update PSOC object state to LOGICALLY DELETED
	 * It prevents further access of this object
	 */
	wlan_psoc_obj_lock(psoc);
	psoc->obj_state = WLAN_OBJ_STATE_LOGICALLY_DELETED;
	wlan_psoc_obj_unlock(psoc);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_obj_delete);

QDF_STATUS wlan_objmgr_psoc_component_obj_attach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status)
{
	wlan_objmgr_psoc_status_handler stat_handler;
	void *arg = NULL;
	QDF_STATUS obj_status;
	uint8_t i;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* If there is a valid entry, return failure */
	if (psoc->soc_comp_priv_obj[id] != NULL) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Save component's pointer and status */
	psoc->soc_comp_priv_obj[id] = comp_priv_obj;
	psoc->obj_status[id] = status;

	wlan_psoc_obj_unlock(psoc);

	if (psoc->obj_state != WLAN_OBJ_STATE_PARTIALLY_CREATED)
		return QDF_STATUS_SUCCESS;
	/* If PSOC object status is partially created means, this API is
	 * invoked with differnt context, this block should be executed for
	 * async components only
	 */
	/* Derive status */
	obj_status = wlan_objmgr_psoc_object_status(psoc);
	/* STATUS_SUCCESS means, object is CREATED */
	if (obj_status == QDF_STATUS_SUCCESS)
		psoc->obj_state = WLAN_OBJ_STATE_CREATED;
	/* update state as CREATION failed, caller has to delete the
	 * PSOC object
	 */
	else if (obj_status == QDF_STATUS_E_FAILURE)
		psoc->obj_state = WLAN_OBJ_STATE_CREATION_FAILED;

	/* Notify components about the CREATION success/failure */
	if ((obj_status == QDF_STATUS_SUCCESS) ||
	    (obj_status == QDF_STATUS_E_FAILURE)) {
		/* nofity object status */
		for (i = 0; i < WLAN_UMAC_MAX_COMPONENTS; i++) {
			stat_handler = g_umac_glb_obj->psoc_status_handler[i];
			arg = g_umac_glb_obj->psoc_status_handler_arg[i];
			if (stat_handler != NULL)
				stat_handler(psoc, arg, obj_status);
		}
	}

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_component_obj_attach);

QDF_STATUS wlan_objmgr_psoc_component_obj_detach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj)
{
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* If there is a valid entry, return failure */
	if (psoc->soc_comp_priv_obj[id] != comp_priv_obj) {
		psoc->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset pointers to NULL, update the status*/
	psoc->soc_comp_priv_obj[id] = NULL;
	psoc->obj_status[id] = QDF_STATUS_SUCCESS;
	wlan_psoc_obj_unlock(psoc);

	/* If PSOC object status is partially created means, this API is
	 * invoked with differnt context, this block should be executed for
	 * async components only
	 */
	if ((psoc->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED) ||
	    (psoc->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)) {
		/* Derive object status */
		obj_status = wlan_objmgr_psoc_object_status(psoc);
		if (obj_status == QDF_STATUS_SUCCESS) {
			/* Update the status as Deleted, if full object
			 * deletion is in progress
			 */
			if (psoc->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				psoc->obj_state = WLAN_OBJ_STATE_DELETED;

			/* Move to creation state, since this component
			 * deletion alone requested
			 */
			if (psoc->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				psoc->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Object status is failure */
		} else if (obj_status == QDF_STATUS_E_FAILURE) {
			/* Update the status as Deletion failed, if full object
			 * deletion is in progress
			 */
			if (psoc->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				psoc->obj_state =
					WLAN_OBJ_STATE_DELETION_FAILED;

			/* Move to creation state, since this component
			 * deletion alone requested (do not block other
			 * components)
			 */
			if (psoc->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				psoc->obj_state = WLAN_OBJ_STATE_CREATED;
		}

		/* Delete psoc object */
		if ((obj_status == QDF_STATUS_SUCCESS)  &&
		    (psoc->obj_state == WLAN_OBJ_STATE_DELETED)) {
			/* Free psoc object */
			return wlan_objmgr_psoc_obj_free(psoc);
		}
	}

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_component_obj_detach);

QDF_STATUS wlan_objmgr_iterate_obj_list(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id)
{
	uint16_t obj_id;
	uint8_t i;
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	struct wlan_peer_list *peer_list;
	qdf_list_t *obj_list;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_next;

	/* If caller requests for lock free opeation, do not acquire,
	 * handler will handle the synchronization
	 */
	if (!lock_free_op)
		wlan_psoc_obj_lock(psoc);

	switch (obj_type) {
	case WLAN_PDEV_OP:
		/* Iterate through PDEV list, invoke handler for each pdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_MAX_PDEVS; obj_id++) {
			pdev = objmgr->wlan_pdev_list[obj_id];
			if ((pdev != NULL) &&
			    (pdev->obj_state !=
				WLAN_OBJ_STATE_LOGICALLY_DELETED)) {
				wlan_objmgr_pdev_get_ref(pdev, dbg_id);
				handler(psoc, (void *)pdev, arg);
				wlan_objmgr_pdev_release_ref(pdev, dbg_id);
			}
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_PSOC_MAX_VDEVS; obj_id++) {
			vdev = objmgr->wlan_vdev_list[obj_id];
			if ((vdev != NULL) &&
			    (vdev->obj_state !=
				WLAN_OBJ_STATE_LOGICALLY_DELETED)) {
				wlan_objmgr_vdev_get_ref(vdev, dbg_id);
				handler(psoc, vdev, arg);
				wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			}
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		/* psoc lock should be taken before list lock */
		if (!lock_free_op)
			qdf_spin_lock_bh(&peer_list->peer_list_lock);
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			obj_list = &peer_list->peer_hash[i];
			peer = wlan_psoc_peer_list_peek_head(obj_list);
			while (peer) {
				/* Get next peer */
				peer_next = wlan_peer_get_next_peer_of_psoc(
								obj_list, peer);
				if (wlan_objmgr_peer_try_get_ref(peer, dbg_id)
						== QDF_STATUS_SUCCESS) {
					handler(psoc, (void *)peer, arg);
					wlan_objmgr_peer_release_ref(peer,
								     dbg_id);
				}
				peer = peer_next;
			}
		}
		if (!lock_free_op)
			qdf_spin_unlock_bh(&peer_list->peer_list_lock);
		break;
	default:
		break;
	}
	if (!lock_free_op)
		wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_iterate_obj_list);

QDF_STATUS wlan_objmgr_iterate_obj_list_all(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id)
{
	uint16_t obj_id;
	uint8_t i;
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	struct wlan_peer_list *peer_list;
	qdf_list_t *obj_list;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_next;

	/* If caller requests for lock free opeation, do not acquire,
	 * handler will handle the synchronization
	 */
	if (!lock_free_op)
		wlan_psoc_obj_lock(psoc);

	switch (obj_type) {
	case WLAN_PDEV_OP:
		/* Iterate through PDEV list, invoke handler for each pdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_MAX_PDEVS; obj_id++) {
			pdev = objmgr->wlan_pdev_list[obj_id];
			if (pdev != NULL) {
				wlan_objmgr_pdev_get_ref(pdev, dbg_id);
				handler(psoc, (void *)pdev, arg);
				wlan_objmgr_pdev_release_ref(pdev, dbg_id);
			}
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_PSOC_MAX_VDEVS; obj_id++) {
			vdev = objmgr->wlan_vdev_list[obj_id];
			if (vdev != NULL) {
				wlan_objmgr_vdev_get_ref(vdev, dbg_id);
				handler(psoc, vdev, arg);
				wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			}
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		/* psoc lock should be taken before list lock */
		if (!lock_free_op)
			qdf_spin_lock_bh(&peer_list->peer_list_lock);
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			obj_list = &peer_list->peer_hash[i];
			peer = wlan_psoc_peer_list_peek_head(obj_list);
			while (peer) {
				/* Get next peer */
				peer_next = wlan_peer_get_next_peer_of_psoc(
								obj_list, peer);
				wlan_objmgr_peer_get_ref(peer, dbg_id);
				handler(psoc, (void *)peer, arg);
				wlan_objmgr_peer_release_ref(peer, dbg_id);
				peer = peer_next;
			}
		}
		if (!lock_free_op)
			qdf_spin_unlock_bh(&peer_list->peer_list_lock);
		break;
	default:
		break;
	}
	if (!lock_free_op)
		wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_iterate_obj_list_all);

static void wlan_objmgr_psoc_peer_delete(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_peer *peer = (struct wlan_objmgr_peer *)obj;

	wlan_objmgr_peer_obj_delete(peer);
}

static void wlan_objmgr_psoc_vdev_delete(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;

	wlan_objmgr_vdev_obj_delete(vdev);
}

static void wlan_objmgr_psoc_pdev_delete(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)obj;

	wlan_objmgr_pdev_obj_delete(pdev);
}

QDF_STATUS wlan_objmgr_free_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc)
{
	/* Free all peers */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PEER_OP,
				     wlan_objmgr_psoc_peer_delete, NULL, 1,
				     WLAN_OBJMGR_ID);
	/* Free all vdevs */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wlan_objmgr_psoc_vdev_delete, NULL, 1,
				     WLAN_OBJMGR_ID);
	/* Free all PDEVs */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wlan_objmgr_psoc_pdev_delete, NULL, 1,
				     WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_trigger_psoc_comp_priv_object_creation(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_psoc_create_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* Component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* If component object is already created, delete old
	 * component object, then invoke creation
	 */
	if (psoc->soc_comp_priv_obj[id] != NULL) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);
	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->psoc_create_handler[id];
	arg = g_umac_glb_obj->psoc_create_handler_arg[id];
	if (handler != NULL)
		psoc->obj_status[id] = handler(psoc, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (psoc->obj_state == WLAN_OBJ_STATE_CREATED) {
		/* Derive object status */
		obj_status = wlan_objmgr_psoc_object_status(psoc);
		/* Move PSOC object state to Partially created state */
		if (obj_status == QDF_STATUS_COMP_ASYNC) {
			/*TODO atomic */
			psoc->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
		}
	}

	return obj_status;
}

QDF_STATUS wlan_objmgr_trigger_psoc_comp_priv_object_deletion(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_psoc_destroy_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* Component object was never created, invalid operation */
	if (psoc->soc_comp_priv_obj[id] == NULL) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);
	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->psoc_destroy_handler[id];
	arg = g_umac_glb_obj->psoc_destroy_handler_arg[id];
	if (handler != NULL)
		psoc->obj_status[id] = handler(psoc, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (psoc->obj_state == WLAN_OBJ_STATE_CREATED) {
		obj_status = wlan_objmgr_psoc_object_status(psoc);
			/* move object state to DEL progress */
		if (obj_status == QDF_STATUS_COMP_ASYNC)
			psoc->obj_state = WLAN_OBJ_STATE_COMP_DEL_PROGRESS;
	}

	return obj_status;
}

/* Util APIs */

QDF_STATUS wlan_objmgr_psoc_pdev_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id = 0;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/*
	 * Derive pdev id from pdev map
	 * First free pdev id is assigned
	 */
	while ((id < WLAN_UMAC_MAX_PDEVS) &&
			(objmgr->wlan_pdev_id_map & (1<<id)))
		id++;

	if (id == WLAN_UMAC_MAX_PDEVS) {
		status = QDF_STATUS_E_FAILURE;
	} else {
		/* Update the map for reserving the id */
		objmgr->wlan_pdev_id_map |= (1<<id);
		/* store pdev in pdev list */
		objmgr->wlan_pdev_list[id] = pdev;
		/* Increment pdev count */
		objmgr->wlan_pdev_count++;
		/* save pdev id */
		pdev->pdev_objmgr.wlan_pdev_id = id;
		status = QDF_STATUS_SUCCESS;
		/* Inrement psoc ref count to block its free before pdev */
		wlan_objmgr_psoc_get_ref(psoc, WLAN_OBJMGR_ID);
	}
	wlan_psoc_obj_unlock(psoc);

	return status;
}

QDF_STATUS wlan_objmgr_psoc_pdev_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id;

	id = pdev->pdev_objmgr.wlan_pdev_id;
	/* If id is invalid, return */
	if (id >= WLAN_UMAC_MAX_PDEVS)
		return QDF_STATUS_E_FAILURE;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* Free pdev id slot */
	objmgr->wlan_pdev_id_map &= ~(1<<id);
	objmgr->wlan_pdev_list[id] = NULL;
	objmgr->wlan_pdev_count--;
	pdev->pdev_objmgr.wlan_pdev_id = 0xff;
	wlan_psoc_obj_unlock(psoc);
	/* Release ref count of psoc */
	wlan_objmgr_psoc_release_ref(psoc, WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_id(
		struct wlan_objmgr_psoc *psoc, uint8_t id,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	struct wlan_objmgr_pdev *pdev = NULL;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* get pdev from pdev list */
	pdev = objmgr->wlan_pdev_list[id];
	/* Do not return object, if it is not CREATED state */
	if (pdev != NULL) {
		if (wlan_objmgr_pdev_try_get_ref(pdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
			pdev = NULL;
	}

	wlan_psoc_obj_unlock(psoc);

	return pdev;
}
EXPORT_SYMBOL(wlan_objmgr_get_pdev_by_id);

struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_id_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t id,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	struct wlan_objmgr_pdev *pdev = NULL;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* get pdev from pdev list */
	pdev = objmgr->wlan_pdev_list[id];
	/* Do not return object, if it is not CREATED state */
	if (pdev != NULL)
		wlan_objmgr_pdev_get_ref(pdev, dbg_id);

	wlan_psoc_obj_unlock(psoc);

	return pdev;
}
QDF_STATUS wlan_objmgr_psoc_vdev_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id = 0;
	uint8_t map_index = 0;
	uint8_t map_entry_size = 32;
	uint8_t adjust_ix = 0;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* Find first free vdev id */
	while ((id < objmgr->max_vdev_count) &&
		(objmgr->wlan_vdev_id_map[map_index] & (1<<(id - adjust_ix)))) {
		id++;
		/*
		 * The map is two DWORDS(32 bits), so, map_index
		 * adjust_ix derived based on the id value
		 */
		if (id == ((map_index+1)*map_entry_size)) {
			adjust_ix = map_index*map_entry_size;
			map_index++;
		}
	}
	/* If no free slot, return failure */
	if (id == objmgr->max_vdev_count) {
		status = QDF_STATUS_E_FAILURE;
	} else {
		/* set free vdev id index */
		objmgr->wlan_vdev_id_map[map_index] |= (1<<(id-adjust_ix));
		/* store vdev pointer in vdev list */
		objmgr->wlan_vdev_list[id] = vdev;
		/* increment vdev counter */
		objmgr->wlan_vdev_count++;
		/* save vdev id */
		vdev->vdev_objmgr.vdev_id = id;
		status = QDF_STATUS_SUCCESS;
	}
	wlan_psoc_obj_unlock(psoc);

	return status;
}

QDF_STATUS wlan_objmgr_psoc_vdev_detach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id = 0;
	uint8_t map_index = 0;
	uint8_t map_entry_size = 32;
	uint8_t adjust_ix = 0;

	id = vdev->vdev_objmgr.vdev_id;
	/* Invalid vdev id */
	if (id >= WLAN_UMAC_PSOC_MAX_VDEVS)
		return QDF_STATUS_E_FAILURE;
	/*
	 * Derive map_index and adjust_ix to find actual DWORD
	 * the id map is present
	 */
	while ((id - adjust_ix) >= map_entry_size) {
		map_index++;
		adjust_ix = map_index * map_entry_size;
	}
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* unset bit, to free the slot */
	objmgr->wlan_vdev_id_map[map_index] &= ~(1<<(id-adjust_ix));
	/* reset VDEV pointer to NULL in VDEV list array */
	objmgr->wlan_vdev_list[id] = NULL;
	/* decrement vdev count */
	objmgr->wlan_vdev_count--;
	vdev->vdev_objmgr.vdev_id = 0xff;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_opmode_from_psoc(
			struct wlan_objmgr_psoc *psoc,
			enum tQDF_ADAPTER_MODE opmode,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	int vdev_cnt = 0;

	/* if PSOC is NULL, return */
	if (psoc == NULL)
		return NULL;

	wlan_psoc_obj_lock(psoc);

	/* retrieve vdev pointer from vdev list */
	while (vdev_cnt < WLAN_UMAC_PSOC_MAX_VDEVS) {
		vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_cnt];
		vdev_cnt++;
		if (vdev == NULL)
			continue;
		wlan_vdev_obj_lock(vdev);
		if (vdev->vdev_mlme.vdev_opmode == opmode) {
			wlan_vdev_obj_unlock(vdev);
			if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
				vdev = NULL;
			break;
		}
		wlan_vdev_obj_unlock(vdev);
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (psoc == NULL)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev != NULL) {
		if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
			vdev = NULL;
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}
EXPORT_SYMBOL(wlan_objmgr_get_vdev_by_id_from_psoc);

struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (psoc == NULL)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev != NULL)
		wlan_objmgr_vdev_get_ref(vdev, dbg_id);

	wlan_psoc_obj_unlock(psoc);

	return vdev;
}
EXPORT_SYMBOL(wlan_objmgr_get_vdev_by_id_from_psoc_no_state);

struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t id;

	/* if PSOC is NULL, return */
	if (psoc == NULL)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* Iterate through PSOC's vdev list */
	for (id = 0; id < WLAN_UMAC_PSOC_MAX_VDEVS; id++) {
		vdev = psoc->soc_objmgr.wlan_vdev_list[id];
		if (vdev == NULL)
			continue;
		/* MAC address matches, break */
		if (WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr)
			== QDF_STATUS_SUCCESS) {
			if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
				vdev = NULL;

			wlan_psoc_obj_unlock(psoc);

			return vdev;
		}
	}
	wlan_psoc_obj_unlock(psoc);

	return NULL;
}
EXPORT_SYMBOL(wlan_objmgr_get_vdev_by_macaddr_from_psoc);

struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t id;

	/* if PSOC is NULL, return */
	if (psoc == NULL)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* Iterate through PSOC's vdev list */
	for (id = 0; id < WLAN_UMAC_PSOC_MAX_VDEVS; id++) {
		vdev = psoc->soc_objmgr.wlan_vdev_list[id];
		if (vdev == NULL)
			continue;
		/* MAC address matches, break */
		if (WLAN_ADDR_EQ(wlan_vdev_mlme_get_macaddr(vdev), macaddr)
			== QDF_STATUS_SUCCESS) {
			wlan_objmgr_vdev_get_ref(vdev, dbg_id);
			wlan_psoc_obj_unlock(psoc);

			return vdev;
		}
	}
	wlan_psoc_obj_unlock(psoc);

	return NULL;
}
EXPORT_SYMBOL(wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state);

static void wlan_obj_psoc_peerlist_add_tail(qdf_list_t *obj_list,
				struct wlan_objmgr_peer *obj)
{
	qdf_list_insert_back(obj_list, &obj->psoc_peer);
}

static QDF_STATUS wlan_obj_psoc_peerlist_remove_peer(
				qdf_list_t *obj_list,
				struct wlan_objmgr_peer *peer)
{
	qdf_list_node_t *psoc_node = NULL;

	if (peer == NULL)
		return QDF_STATUS_E_FAILURE;
	/* get vdev list node element */
	psoc_node = &peer->psoc_peer;
	/* list is empty, return failure */
	if (qdf_list_remove_node(obj_list, psoc_node) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wlan_peer_bssid_match(struct wlan_objmgr_peer *peer,
				     uint8_t *bssid)
{
	struct wlan_objmgr_vdev *vdev = wlan_peer_get_vdev(peer);
	uint8_t *peer_bssid = wlan_vdev_mlme_get_macaddr(vdev);

	if (WLAN_ADDR_EQ(peer_bssid, bssid) == QDF_STATUS_SUCCESS)
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_FAILURE;
}

/**
 * wlan_obj_psoc_peerlist_get_peer_logically_deleted() - get peer
 * from psoc peer list
 * @psoc: PSOC object
 * @macaddr: MAC address
 *
 * API to finds peer object pointer of logically deleted peer
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
static struct wlan_objmgr_peer *
			wlan_obj_psoc_peerlist_get_peer_logically_deleted(
				qdf_list_t *obj_list, uint8_t *macaddr,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/* Return peer in logically deleted state */
			if (peer->obj_state ==
					WLAN_OBJ_STATE_LOGICALLY_DELETED) {
				wlan_objmgr_peer_get_ref(peer, dbg_id);

				return peer;
			}

		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}

/**
 * wlan_obj_psoc_peerlist_get_peer() - get peer from psoc peer list
 * @psoc: PSOC object
 * @macaddr: MAC address
 *
 * API to finds peer object pointer by MAC addr from hash list
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer(
				qdf_list_t *obj_list, uint8_t *macaddr,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			if (wlan_objmgr_peer_try_get_ref(peer, dbg_id) ==
							QDF_STATUS_SUCCESS) {
				return peer;
			}
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}

static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_no_state(
				qdf_list_t *obj_list, uint8_t *macaddr,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			wlan_objmgr_peer_get_ref(peer, dbg_id);

			return peer;
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}

/**
 * wlan_obj_psoc_peerlist_get_peer_logically_deleted_by_mac_n_bssid() - get peer
 *                                           from psoc peer list using
 *                                           mac and vdev self mac
 * @obj_list: peer object list
 * @macaddr: MAC address
 * @bssid: BSSID address
 * @dbg_id: id of the caller
 *
 * API to finds peer object pointer by MAC addr and BSSID from
 * peer hash list for a node which is in logically deleted state,
 *  bssid check is done on matching peer
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
static struct wlan_objmgr_peer
	*wlan_obj_psoc_peerlist_get_peer_logically_deleted_by_mac_n_bssid(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t *bssid,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if (wlan_peer_bssid_match(peer, bssid) ==
							QDF_STATUS_SUCCESS) {
				/* Return peer in logically deleted state */
				if (peer->obj_state ==
					WLAN_OBJ_STATE_LOGICALLY_DELETED) {
					wlan_objmgr_peer_get_ref(peer, dbg_id);

					return peer;
				}
			}
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}

/**
 * wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid() - get peer from psoc peer
 *                                                    list using mac and vdev
 *                                                    self mac
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @bssid: BSSID address
 *
 * API to finds peer object pointer by MAC addr and BSSID from
 * peer hash list, bssid check is done on matching peer
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid,
					wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if (wlan_peer_bssid_match(peer, bssid) ==
							QDF_STATUS_SUCCESS) {
				if (wlan_objmgr_peer_try_get_ref(peer, dbg_id)
					== QDF_STATUS_SUCCESS) {
					return peer;
				}
			}
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}
	/* Not found, return NULL */
	return NULL;
}

static struct wlan_objmgr_peer
		*wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid,
					wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if (wlan_peer_bssid_match(peer, bssid) ==
							QDF_STATUS_SUCCESS) {
				wlan_objmgr_peer_get_ref(peer, dbg_id);

				return peer;
			}
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}

QDF_STATUS wlan_objmgr_psoc_peer_attach(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_peer_list *peer_list;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* Max peer limit is reached, return failure */
	if (objmgr->wlan_peer_count > WLAN_UMAC_PSOC_MAX_PEERS) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Derive hash index from mac address */
	hash_index = WLAN_PEER_HASH(peer->macaddr);
	peer_list = &objmgr->peer_list;
	/* psoc lock should be taken before list lock */
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* add peer to hash peer list */
	wlan_obj_psoc_peerlist_add_tail(
			&peer_list->peer_hash[hash_index],
							peer);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	/* Increment peer count */
	objmgr->wlan_peer_count++;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_psoc_peer_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_peer_list *peer_list;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* if list is empty, return */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Get hash index, to locate the actual peer list */
	hash_index = WLAN_PEER_HASH(peer->macaddr);
	peer_list = &objmgr->peer_list;
	/* psoc lock should be taken before list lock */
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* removes the peer from peer_list */
	if (wlan_obj_psoc_peerlist_remove_peer(
				&peer_list->peer_hash[hash_index],
						peer) ==
				QDF_STATUS_E_FAILURE) {
		qdf_spin_unlock_bh(&peer_list->peer_list_lock);
		wlan_psoc_obj_unlock(psoc);
		obj_mgr_err("Failed to detach peer");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	/* Decrement peer count */
	objmgr->wlan_peer_count--;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_peer *wlan_objmgr_get_peer_logically_deleted(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer_logically_deleted(
		&peer_list->peer_hash[hash_index], macaddr, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

struct wlan_objmgr_peer *wlan_objmgr_get_peer(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer(
		&peer_list->peer_hash[hash_index], macaddr, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer);

struct wlan_objmgr_peer *wlan_objmgr_get_peer_nolock(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0)
		return NULL;

	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer(
		&peer_list->peer_hash[hash_index], macaddr, dbg_id);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer_nolock);


struct wlan_objmgr_peer *wlan_objmgr_get_peer_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer_no_state(
		&peer_list->peer_hash[hash_index], macaddr, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer_no_state);

struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			uint8_t *bssid, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid(
		&peer_list->peer_hash[hash_index], macaddr, bssid, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer_by_mac_n_vdev);


/**
 * wlan_objmgr_get_peer_logically_deleted_by_mac_n_vdev() - get peer from psoc
 *                                                           peer list using
 *                                                           mac and vdev
 *                                                           self mac
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @bssid: BSSID address
 * @dbg_id: id of the caller
 *
 * API to finds peer object pointer by MAC addr and BSSID from
 * peer hash list, bssid check is done on matching peer
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */

struct wlan_objmgr_peer *wlan_objmgr_get_peer_logically_deleted_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			uint8_t *bssid, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer_logically_deleted_by_mac_n_bssid(
		&peer_list->peer_hash[hash_index], macaddr, bssid, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer_logically_deleted_by_mac_n_vdev);

struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			uint8_t *bssid, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	/* psoc lock should be taken before peer list lock */
	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* List is empty, return NULL */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return NULL;
	}
	/* reduce the search window, with hash key */
	hash_index = WLAN_PEER_HASH(macaddr);
	peer_list = &objmgr->peer_list;
	qdf_spin_lock_bh(&peer_list->peer_list_lock);
	/* Iterate through peer list, get peer */
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state(
		&peer_list->peer_hash[hash_index], macaddr, bssid, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
EXPORT_SYMBOL(wlan_objmgr_get_peer_by_mac_n_vdev_no_state);

void *wlan_objmgr_psoc_get_comp_private_obj(struct wlan_objmgr_psoc *psoc,
					enum wlan_umac_comp_id id)
{
	void *comp_private_obj;

	/* This API is invoked with lock acquired, don't add any debug prints */

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		QDF_BUG(0);
		return NULL;
	}

	if (psoc == NULL) {
		QDF_BUG(0);
		return NULL;
	}

	comp_private_obj = psoc->soc_comp_priv_obj[id];

	return comp_private_obj;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_get_comp_private_obj);

void wlan_objmgr_psoc_get_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (psoc == NULL) {
		obj_mgr_err("psoc obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return;
	}
	/* Increment ref count */
	qdf_atomic_inc(&psoc->soc_objmgr.ref_cnt);
	qdf_atomic_inc(&psoc->soc_objmgr.ref_id_dbg[id]);
	return;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_get_ref);

QDF_STATUS wlan_objmgr_psoc_try_get_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (psoc == NULL) {
		obj_mgr_err("psoc obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_psoc_obj_lock(psoc);
	if (psoc->obj_state == WLAN_OBJ_STATE_LOGICALLY_DELETED) {
		wlan_psoc_obj_unlock(psoc);
		obj_mgr_err("Called by %d, psoc obj is in Deletion Progress state", id);
		return QDF_STATUS_E_RESOURCES;
	}

	/* Increment ref count */
	wlan_objmgr_psoc_get_ref(psoc, id);
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_try_get_ref);

void wlan_objmgr_psoc_release_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (psoc == NULL) {
		obj_mgr_err("psoc obj is NULLi for id:%d", id);
		QDF_ASSERT(0);
		return;
	}

	if (!qdf_atomic_read(&psoc->soc_objmgr.ref_id_dbg[id])) {
		obj_mgr_err("psoc ref cnt was not taken by %d", id);
		wlan_objmgr_print_ref_ids(psoc->soc_objmgr.ref_id_dbg);
		WLAN_OBJMGR_BUG(0);
	}

	if (!qdf_atomic_read(&psoc->soc_objmgr.ref_cnt)) {
		obj_mgr_err("psoc ref cnt is 0");
		WLAN_OBJMGR_BUG(0);
		return;
	}

	qdf_atomic_dec(&psoc->soc_objmgr.ref_id_dbg[id]);
	/* Decrement ref count, free psoc, if ref count == 0 */
	if (qdf_atomic_dec_and_test(&psoc->soc_objmgr.ref_cnt))
		wlan_objmgr_psoc_obj_destroy(psoc);

	return;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_release_ref);

static void wlan_objmgr_psoc_peer_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_peer *peer = (struct wlan_objmgr_peer *)obj;
	uint8_t *macaddr;

	wlan_peer_obj_lock(peer);
	macaddr = wlan_peer_get_macaddr(peer);
	wlan_peer_obj_unlock(peer);

	obj_mgr_info("Peer MAC is %02x:%02x:%02x:%02x:%02x:%02x",
		  macaddr[0], macaddr[1], macaddr[2], macaddr[3],
		  macaddr[4], macaddr[5]);
	wlan_objmgr_print_ref_ids(peer->peer_objmgr.ref_id_dbg);
}

static void wlan_objmgr_psoc_vdev_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	uint8_t id;

	wlan_vdev_obj_lock(vdev);
	id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);
	obj_mgr_info("Vdev ID is %d", id);

	wlan_objmgr_print_ref_ids(vdev->vdev_objmgr.ref_id_dbg);
}

static void wlan_objmgr_psoc_pdev_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)obj;
	uint8_t id;

	wlan_pdev_obj_lock(pdev);
	id = wlan_objmgr_pdev_get_pdev_id(pdev);
	wlan_pdev_obj_unlock(pdev);
	obj_mgr_info("pdev ID is %d", id);

	wlan_objmgr_print_ref_ids(pdev->pdev_objmgr.ref_id_dbg);
}

QDF_STATUS wlan_objmgr_print_ref_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc)
{
	obj_mgr_info("Ref counts of PEER");
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PEER_OP,
				wlan_objmgr_psoc_peer_ref_print, NULL, 1,
				WLAN_OBJMGR_ID);
	obj_mgr_info("Ref counts of VDEV");
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				wlan_objmgr_psoc_vdev_ref_print, NULL, 1,
				WLAN_OBJMGR_ID);
	obj_mgr_info("Ref counts of PDEV");
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				wlan_objmgr_psoc_pdev_ref_print, NULL, 1,
				WLAN_OBJMGR_ID);

	obj_mgr_info(" Ref counts of PSOC");
	wlan_objmgr_print_ref_ids(psoc->soc_objmgr.ref_id_dbg);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_print_ref_all_objects_per_psoc);

QDF_STATUS wlan_objmgr_psoc_set_user_config(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_psoc_user_config *user_config_data)
{
	if (user_config_data == NULL) {
		obj_mgr_err("user_config_data is NULL");
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_lock(psoc);
	qdf_mem_copy(&psoc->soc_nif.user_config, user_config_data,
				sizeof(psoc->soc_nif.user_config));
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}
