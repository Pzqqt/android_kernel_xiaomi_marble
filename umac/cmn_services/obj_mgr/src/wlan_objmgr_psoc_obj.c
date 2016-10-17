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

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_global_obj_i.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_psoc_obj_i.h>
#include <wlan_objmgr_pdev_obj_i.h>
#include <qdf_mem.h>

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
			break */
		else if (psoc->obj_status[id] == QDF_STATUS_COMP_ASYNC) {
			if (psoc->soc_comp_obj[id] == NULL) {
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
/*
 * wlan_objmgr_psco_create_handler would return following status values
 */
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

	/* Allocate PSOC object's memory */
	psoc = qdf_mem_malloc(sizeof(*psoc));
	if (psoc == NULL) {
		qdf_print("%s: PSOC allocation failed\n", __func__);
		return NULL;
	}
	/* Init spinlock */
	qdf_spinlock_create(&psoc->psoc_lock);
	/* Initialize with default values */
	objmgr = &psoc->soc_objmgr;
	objmgr->wlan_pdev_count = 0;
	objmgr->wlan_vdev_count = 0;
	objmgr->max_vdev_count = WLAN_UMAC_PSOC_MAX_VDEVS;
	objmgr->wlan_peer_count = 0;
	/* set phy version, dev_type in psoc */
	wlan_psoc_set_nif_phy_version(psoc, phy_version);
	wlan_psoc_set_dev_type(psoc, dev_type);
	/* Initialize peer list */
	wlan_objmgr_psoc_peer_list_init(&objmgr->peer_list);
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
	/*
	 * Few components operates in Asynchrous communction, Object state
	 * partially created
	 */
	} else if (obj_status == QDF_STATUS_COMP_ASYNC) {
		psoc->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
	/* Component object failed to be created, clean up the object */
	} else if (obj_status == QDF_STATUS_E_FAILURE) {
		qdf_print("%s: PSOC component objects allocation failed\n",
			  __func__);
		/* Clean up the psoc */
		wlan_objmgr_psoc_obj_delete(psoc);
		return NULL;
	}

	if (wlan_objmgr_psoc_object_attach(psoc) !=
				QDF_STATUS_SUCCESS) {
		qdf_print("%s: PSOC object attach failed\n", __func__);
		wlan_objmgr_psoc_obj_delete(psoc);
		return NULL;
	}
	return psoc;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_obj_create);

QDF_STATUS wlan_objmgr_psoc_obj_delete(struct wlan_objmgr_psoc *psoc)
{
	uint8_t id;
	wlan_objmgr_psoc_delete_handler handler;
	QDF_STATUS obj_status;
	void *arg;

	/* if PSOC is NULL, return */
	if (psoc == NULL) {
		qdf_print("%s:psoc is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->psoc_delete_handler[id];
		arg = g_umac_glb_obj->psoc_delete_handler_arg[id];
		if (handler != NULL)
			psoc->obj_status[id] = handler(psoc, arg);
		else
			psoc->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_psoc_object_status(psoc);

	if (obj_status == QDF_STATUS_E_FAILURE) {
		qdf_print("%s: PSOC component object free failed\n", __func__);
		/* Ideally should not happen */
		/*This leads to memleak ??? how to handle */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		psoc->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	} else {
		/* Detach PSOC from global object's psoc list  */
		if (wlan_objmgr_psoc_object_detach(psoc) ==
					QDF_STATUS_E_FAILURE) {
			qdf_print("%s: PSOC object detach failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		wlan_objmgr_psoc_peer_list_deinit(&psoc->soc_objmgr.peer_list);
		/* Destroy spinlock */
		qdf_spinlock_destroy(&psoc->psoc_lock);
		/* Free the memory */
		qdf_mem_free(psoc);
	}
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_psoc_obj_delete);

/**
 ** APIs to attach/detach component objects
 */
QDF_STATUS wlan_objmgr_psoc_component_obj_attach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_objptr,
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
	if (psoc->soc_comp_obj[id] != NULL) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Save component's pointer and status */
	psoc->soc_comp_obj[id] = comp_objptr;
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

QDF_STATUS wlan_objmgr_psoc_component_obj_detach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_objptr)
{
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* If there is a valid entry, return failure */
	if (psoc->soc_comp_obj[id] != comp_objptr) {
		psoc->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset pointers to NULL, update the status*/
	psoc->soc_comp_obj[id] = NULL;
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
			/* Detach PSOC from global object's psoc list */
			if (wlan_objmgr_psoc_object_detach(psoc) ==
						QDF_STATUS_E_FAILURE)
				return QDF_STATUS_E_FAILURE;

			wlan_objmgr_psoc_peer_list_deinit(
				&psoc->soc_objmgr.peer_list);
			/* Destroy spinlock */
			qdf_spinlock_destroy(&psoc->psoc_lock);
			/* Free memory */
			qdf_mem_free(psoc);
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 ** APIs to operations on psoc objects
 */

QDF_STATUS wlan_objmgr_iterate_obj_list(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg, uint8_t lock_free_op)
{
	uint16_t obj_id;
	uint8_t i;
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	struct wlan_peer_list *peer_list;
	qdf_list_t *obj_list;
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
			if (objmgr->wlan_pdev_list[obj_id] != NULL) {
				/* TODO increment ref count */
				handler(psoc,
					(void *)objmgr->wlan_pdev_list[obj_id],
					arg);
				/* TODO decrement ref count */
			}
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_PSOC_MAX_VDEVS; obj_id++) {
			if (objmgr->wlan_vdev_list[obj_id] != NULL) {
				/* TODO increment ref count */
				handler(psoc,
					(void *)objmgr->wlan_vdev_list[obj_id],
					arg);
				/* TODO decrement ref count */
			}
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		if (!lock_free_op)
			qdf_spin_lock_bh(&peer_list->peer_list_lock);
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			obj_list = &peer_list->peer_hash[i];
			peer = wlan_psoc_peer_list_peek_head(obj_list);
			while (peer) {
				/* Increment ref count, to hold the
					peer pointer */
				wlan_objmgr_peer_ref_peer(peer);
				handler(psoc, (void *)peer, arg);
				/* Get next peer */
				peer_next = wlan_peer_get_next_peer_of_psoc(
								obj_list, peer);
				/* Decrement ref count, this can lead
					to peer deletion also */
				wlan_objmgr_peer_unref_peer(peer);
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

void wlan_objmgr_psoc_peer_delete(struct wlan_objmgr_psoc *psoc, void *obj,
						 void *args)
{
	struct wlan_objmgr_peer *peer = (struct wlan_objmgr_peer *)obj;

	wlan_objmgr_peer_obj_delete(peer);
}

void wlan_objmgr_psoc_vdev_delete(struct wlan_objmgr_psoc *psoc, void *obj,
							void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;

	wlan_objmgr_vdev_obj_delete(vdev);
}

void wlan_objmgr_psoc_pdev_delete(struct wlan_objmgr_psoc *psoc, void *obj,
							void *args)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)obj;

	wlan_objmgr_pdev_obj_delete(pdev);
}

QDF_STATUS wlan_objmgr_free_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc)
{
	/* Free all peers */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PEER_OP,
				     wlan_objmgr_psoc_peer_delete, NULL, 1);
	/* Free all vdevs */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wlan_objmgr_psoc_vdev_delete, NULL, 1);
	/* Free all PDEVs */
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wlan_objmgr_psoc_pdev_delete, NULL, 1);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_trigger_psoc_comp_object_creation(
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
	if (psoc->soc_comp_obj[id] != NULL) {
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

QDF_STATUS wlan_objmgr_trigger_psoc_comp_object_deletion(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_psoc_delete_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_psoc_obj_lock(psoc);
	/* Component object was never created, invalid operation */
	if (psoc->soc_comp_obj[id] == NULL) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);
	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->psoc_delete_handler[id];
	arg = g_umac_glb_obj->psoc_delete_handler_arg[id];
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
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t id = 0;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
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
	}
	wlan_psoc_obj_unlock(psoc);
	return status;
}

QDF_STATUS wlan_objmgr_psoc_pdev_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t id;

	id = pdev->pdev_objmgr.wlan_pdev_id;
	/* If id is invalid, return */
	if (id >= WLAN_UMAC_MAX_PDEVS)
		return QDF_STATUS_E_FAILURE;

	wlan_psoc_obj_lock(psoc);
	/* Free pdev id slot */
	objmgr->wlan_pdev_id_map &= ~(1<<id);
	objmgr->wlan_pdev_list[id] = NULL;
	objmgr->wlan_pdev_count--;
	pdev->pdev_objmgr.wlan_pdev_id = 0xff;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_E_FAILURE;
}

struct wlan_objmgr_pdev *wlan_objmgr_find_pdev_by_id(
		struct wlan_objmgr_psoc *psoc, uint8_t id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	struct wlan_objmgr_pdev *pdev = NULL;

	wlan_psoc_obj_lock(psoc);
	/* get pdev from pdev list */
	pdev = objmgr->wlan_pdev_list[id];
	wlan_psoc_obj_unlock(psoc);
	return pdev;
}

QDF_STATUS wlan_objmgr_psoc_vdev_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t id = 0;
	uint8_t map_index = 0;
	uint8_t map_entry_size = 32;
	uint8_t adjust_ix = 0;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
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
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
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


struct wlan_objmgr_vdev *wlan_objmgr_find_vdev_by_id_from_psoc(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
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
	wlan_psoc_obj_unlock(psoc);
	return vdev;
}

struct wlan_objmgr_vdev *wlan_objmgr_find_vdev_by_macaddr_from_psoc(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr)
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
			wlan_psoc_obj_unlock(psoc);
			return vdev;
		}
	}
	wlan_psoc_obj_unlock(psoc);
	return NULL;
}

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

struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer(
				qdf_list_t *obj_list,
				uint8_t *macaddr)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer != NULL) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/* Increment ref count for access */
			wlan_objmgr_peer_ref_peer(peer);
			return peer;
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
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t hash_index;

	wlan_psoc_obj_lock(psoc);
	/* Max peer limit is reached, return failure */
	if (objmgr->wlan_peer_count > WLAN_UMAC_PSOC_MAX_PEERS) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Derive hash index from mac address */
	hash_index = WLAN_PEER_HASH(peer->macaddr);
	/* add peer to hash peer list */
	wlan_obj_psoc_peerlist_add_tail(
			&objmgr->peer_list.peer_hash[hash_index],
							peer);
	/* Increment peer count */
	objmgr->wlan_peer_count++;

	wlan_psoc_obj_unlock(psoc);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_psoc_peer_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t hash_index;

	wlan_psoc_obj_lock(psoc);
	/* if list is empty, return */
	if (objmgr->wlan_peer_count == 0) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	/* Get hash index, to locate the actual peer list */
	hash_index = WLAN_PEER_HASH(peer->macaddr);
	/* removes the peer from peer_list */
	if (wlan_obj_psoc_peerlist_remove_peer(
				&objmgr->peer_list.peer_hash[hash_index],
						peer) ==
				QDF_STATUS_E_FAILURE) {
		wlan_psoc_obj_unlock(psoc);
		qdf_print("%s: Failed to detach peer\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Decrement peer count */
	objmgr->wlan_peer_count--;

	wlan_psoc_obj_unlock(psoc);
	return QDF_STATUS_SUCCESS;
}

struct wlan_objmgr_peer *wlan_objmgr_find_peer(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr)
{
	struct wlan_objmgr_psoc_objmgr *objmgr = &psoc->soc_objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	wlan_psoc_obj_lock(psoc);
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
		&peer_list->peer_hash[hash_index], macaddr);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);
	return peer;
}
