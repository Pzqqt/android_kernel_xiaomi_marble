/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <wlan_objmgr_debug.h>
#include <qdf_mem.h>
#include <qdf_types.h>
#include <qdf_module.h>
#include "wlan_objmgr_global_obj_i.h"
#include "wlan_objmgr_psoc_obj_i.h"
#include "wlan_objmgr_pdev_obj_i.h"
#include "wlan_objmgr_vdev_obj_i.h"
#include <wlan_utility.h>

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
			if (!psoc->soc_comp_priv_obj[id]) {
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
			WLAN_UMAC_PSOC_MAX_PEERS +
			WLAN_MAX_PSOC_TEMP_PEERS);
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
	/* Detach PSOC from global object's psoc list  */
	if (wlan_objmgr_psoc_object_detach(psoc) == QDF_STATUS_E_FAILURE) {
		obj_mgr_err("PSOC object detach failed");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_psoc_peer_list_deinit(&psoc->soc_objmgr.peer_list);

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
	if (!psoc)
		return NULL;

	psoc->obj_state = WLAN_OBJ_STATE_ALLOCATED;
	qdf_spinlock_create(&psoc->psoc_lock);
	/* Initialize with default values */
	objmgr = &psoc->soc_objmgr;
	objmgr->wlan_pdev_count = 0;
	objmgr->wlan_vdev_count = 0;
	objmgr->max_vdev_count = WLAN_UMAC_PSOC_MAX_VDEVS;
	objmgr->wlan_peer_count = 0;
	objmgr->temp_peer_count = 0;
	objmgr->max_peer_count = WLAN_UMAC_PSOC_MAX_PEERS;
	qdf_atomic_init(&objmgr->ref_cnt);
	objmgr->print_cnt = 0;
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
		if (handler)
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
			if (stat_handler)
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
	wlan_minidump_log(psoc, sizeof(*psoc), psoc,
			  WLAN_MD_OBJMGR_PSOC, "wlan_objmgr_psoc");
	obj_mgr_info("Created psoc %d", psoc->soc_objmgr.psoc_id);

	return psoc;
}
qdf_export_symbol(wlan_objmgr_psoc_obj_create);

static QDF_STATUS wlan_objmgr_psoc_obj_destroy(struct wlan_objmgr_psoc *psoc)
{
	uint8_t id;
	wlan_objmgr_psoc_destroy_handler handler;
	QDF_STATUS obj_status;
	void *arg;

	if (!psoc) {
		obj_mgr_err("psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_notify_destroy(psoc, WLAN_PSOC_OP);

	wlan_print_psoc_info(psoc);
	obj_mgr_info("Physically deleting psoc %d", psoc->soc_objmgr.psoc_id);

	if (psoc->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED) {
		obj_mgr_err("PSOC object delete is not invoked obj_state:%d",
			    psoc->obj_state);
		WLAN_OBJMGR_BUG(0);
	}

	wlan_minidump_remove(psoc, sizeof(*psoc), psoc,
			     WLAN_MD_OBJMGR_PSOC, "wlan_objmgr_psoc");

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->psoc_destroy_handler[id];
		arg = g_umac_glb_obj->psoc_destroy_handler_arg[id];
		if (handler &&
		    (psoc->obj_status[id] == QDF_STATUS_SUCCESS ||
		     psoc->obj_status[id] == QDF_STATUS_COMP_ASYNC))
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
	uint8_t print_idx;

	if (!psoc) {
		obj_mgr_err("psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	obj_mgr_info("Logically deleting psoc %d", psoc->soc_objmgr.psoc_id);

	print_idx = qdf_get_pidx();
	wlan_objmgr_print_ref_ids(psoc->soc_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_DEBUG);
	/*
	 * Update PSOC object state to LOGICALLY DELETED
	 * It prevents further access of this object
	 */
	wlan_psoc_obj_lock(psoc);
	psoc->obj_state = WLAN_OBJ_STATE_LOGICALLY_DELETED;
	wlan_psoc_obj_unlock(psoc);
	wlan_objmgr_notify_log_delete(psoc, WLAN_PSOC_OP);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_psoc_obj_delete);

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
	if (psoc->soc_comp_priv_obj[id]) {
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
			if (stat_handler)
				stat_handler(psoc, arg, obj_status);
		}
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_psoc_component_obj_attach);

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
qdf_export_symbol(wlan_objmgr_psoc_component_obj_detach);

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
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_next;
	uint16_t max_vdev_cnt;

	switch (obj_type) {
	case WLAN_PDEV_OP:
		/* Iterate through PDEV list, invoke handler for each pdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_MAX_PDEVS; obj_id++) {
			pdev = wlan_objmgr_get_pdev_by_id(psoc, obj_id, dbg_id);
			if (pdev) {
				handler(psoc, (void *)pdev, arg);
				wlan_objmgr_pdev_release_ref(pdev, dbg_id);
			}
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		max_vdev_cnt = wlan_psoc_get_max_vdev_count(psoc);
		for (obj_id = 0; obj_id < max_vdev_cnt; obj_id++) {
			vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						obj_id, dbg_id);
			if (vdev) {
				handler(psoc, vdev, arg);
				wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			}
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			peer = wlan_psoc_peer_list_peek_active_head(peer_list,
					i, dbg_id);
			while (peer) {
				handler(psoc, (void *)peer, arg);
				/* Get next peer */
				peer_next =
					wlan_peer_get_next_active_peer_of_psoc(
						peer_list, i, peer, dbg_id);
				wlan_objmgr_peer_release_ref(peer,  dbg_id);
				peer = peer_next;
			}
		}
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_iterate_obj_list);

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
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_next;
	uint16_t max_vdev_cnt;

	/* If caller requests for lock free opeation, do not acquire,
	 * handler will handle the synchronization
	 */

	switch (obj_type) {
	case WLAN_PDEV_OP:
		/* Iterate through PDEV list, invoke handler for each pdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_MAX_PDEVS; obj_id++) {
			pdev = wlan_objmgr_get_pdev_by_id_no_state(psoc,
							obj_id, dbg_id);
			if (pdev) {
				handler(psoc, (void *)pdev, arg);
				wlan_objmgr_pdev_release_ref(pdev, dbg_id);
			}
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		max_vdev_cnt = wlan_psoc_get_max_vdev_count(psoc);
		for (obj_id = 0; obj_id < max_vdev_cnt; obj_id++) {
			vdev = wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
					psoc, obj_id, dbg_id);
			if (vdev) {
				handler(psoc, vdev, arg);
				wlan_objmgr_vdev_release_ref(vdev, dbg_id);
			}
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			peer = wlan_psoc_peer_list_peek_head_ref(peer_list, i,
									dbg_id);

			while (peer) {
				handler(psoc, (void *)peer, arg);
				/* Get next peer */
				peer_next = wlan_peer_get_next_peer_of_psoc_ref(
								peer_list, i,
								peer, dbg_id);
				wlan_objmgr_peer_release_ref(peer, dbg_id);
				peer = peer_next;
			}
		}
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_iterate_obj_list_all);

/**
 * wlan_objmgr_iterate_obj_list_all_noref() - iterate through all psoc objects
 *                                            without taking ref
 * @psoc: PSOC object
 * @obj_type: PDEV_OP/VDEV_OP/PEER_OP
 * @handler: the handler will be called for each object of requested type
 *            the handler should be implemented to perform required operation
 * @arg:     agruments passed by caller
 *
 * API to be used for performing the operations on all PDEV/VDEV/PEER objects
 * of psoc with lock protected
 *
 * Return: SUCCESS/FAILURE
 */
static QDF_STATUS wlan_objmgr_iterate_obj_list_all_noref(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg)
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
	uint16_t max_vdev_cnt;

	/* If caller requests for lock free opeation, do not acquire,
	 * handler will handle the synchronization
	 */
	wlan_psoc_obj_lock(psoc);

	switch (obj_type) {
	case WLAN_PDEV_OP:
		/* Iterate through PDEV list, invoke handler for each pdev */
		for (obj_id = 0; obj_id < WLAN_UMAC_MAX_PDEVS; obj_id++) {
			pdev = objmgr->wlan_pdev_list[obj_id];
			if (pdev)
				handler(psoc, (void *)pdev, arg);
		}
		break;
	case WLAN_VDEV_OP:
		/* Iterate through VDEV list, invoke handler for each vdev */
		max_vdev_cnt = wlan_psoc_get_max_vdev_count(psoc);
		for (obj_id = 0; obj_id < max_vdev_cnt; obj_id++) {
			vdev = objmgr->wlan_vdev_list[obj_id];
			if (vdev)
				handler(psoc, vdev, arg);
		}
		break;
	case WLAN_PEER_OP:
		/* Iterate through PEER list, invoke handler for each peer */
		peer_list = &objmgr->peer_list;
		/* psoc lock should be taken before list lock */
		qdf_spin_lock_bh(&peer_list->peer_list_lock);
		/* Since peer list has sublist, iterate through sublists */
		for (i = 0; i < WLAN_PEER_HASHSIZE; i++) {
			obj_list = &peer_list->peer_hash[i];
			peer = wlan_psoc_peer_list_peek_head(obj_list);
			while (peer) {
				/* Get next peer */
				peer_next = wlan_peer_get_next_peer_of_psoc(
								obj_list, peer);
				handler(psoc, (void *)peer, arg);
				peer = peer_next;
			}
		}
		qdf_spin_unlock_bh(&peer_list->peer_list_lock);
		break;
	default:
		break;
	}
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

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
	if (psoc->soc_comp_priv_obj[id]) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);
	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->psoc_create_handler[id];
	arg = g_umac_glb_obj->psoc_create_handler_arg[id];
	if (handler)
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
	if (!psoc->soc_comp_priv_obj[id]) {
		wlan_psoc_obj_unlock(psoc);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);
	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->psoc_destroy_handler[id];
	arg = g_umac_glb_obj->psoc_destroy_handler_arg[id];
	if (handler)
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

	/* If id is invalid, return */
	if (id >= WLAN_UMAC_MAX_PDEVS)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* get pdev from pdev list */
	pdev = objmgr->wlan_pdev_list[id];
	/* Do not return object, if it is not CREATED state */
	if (pdev) {
		if (wlan_objmgr_pdev_try_get_ref(pdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
			pdev = NULL;
	}

	wlan_psoc_obj_unlock(psoc);

	return pdev;
}
qdf_export_symbol(wlan_objmgr_get_pdev_by_id);

struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_id_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t id,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	struct wlan_objmgr_pdev *pdev = NULL;

	/* If id is invalid, return */
	if (id >= WLAN_UMAC_MAX_PDEVS)
		return NULL;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* get pdev from pdev list */
	pdev = objmgr->wlan_pdev_list[id];
	/* Do not return object, if it is not CREATED state */
	if (pdev)
		wlan_objmgr_pdev_get_ref(pdev, dbg_id);

	wlan_psoc_obj_unlock(psoc);

	return pdev;
}
QDF_STATUS wlan_objmgr_psoc_vdev_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id = 0;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* Find first free vdev id */
	while ((id < objmgr->max_vdev_count)) {
		if (qdf_test_bit(id, objmgr->wlan_vdev_id_map)) {
			id++;
			continue;
		}
		break;
	}

	/* If no free slot, return failure */
	if (id < objmgr->max_vdev_count) {
		/* set free vdev id index */
		qdf_set_bit(id, objmgr->wlan_vdev_id_map);
		/* store vdev pointer in vdev list */
		objmgr->wlan_vdev_list[id] = vdev;
		/* increment vdev counter */
		objmgr->wlan_vdev_count++;
		/* save vdev id */
		vdev->vdev_objmgr.vdev_id = id;
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}
	wlan_psoc_obj_unlock(psoc);

	return status;
}

QDF_STATUS wlan_objmgr_psoc_vdev_detach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t id = 0;

	id = vdev->vdev_objmgr.vdev_id;
	/* Invalid vdev id */
	if (id >= wlan_psoc_get_max_vdev_count(psoc))
		return QDF_STATUS_E_FAILURE;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* unset bit, to free the slot */
	qdf_clear_bit(id, objmgr->wlan_vdev_id_map);
	/* reset VDEV pointer to NULL in VDEV list array */
	objmgr->wlan_vdev_list[id] = NULL;
	/* decrement vdev count */
	objmgr->wlan_vdev_count--;
	vdev->vdev_objmgr.vdev_id = 0xff;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev) {
		if (wlan_objmgr_vdev_try_get_ref_debug(vdev, dbg_id,
						       func, line) !=
			QDF_STATUS_SUCCESS)
			vdev = NULL;
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev) {
		if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS)
			vdev = NULL;
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev)
		wlan_objmgr_vdev_get_ref_debug(vdev, dbg_id, func, line);

	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc_no_state_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev)
		wlan_objmgr_vdev_get_ref(vdev, dbg_id);

	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc_no_state);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];
	if (vdev && vdev->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED)
		wlan_objmgr_vdev_get_ref_debug(vdev, dbg_id, func, line);
	else
		vdev = NULL;
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;
	/* vdev id is invalid */
	if (vdev_id >= wlan_psoc_get_max_vdev_count(psoc))
		return NULL;

	wlan_psoc_obj_lock(psoc);
	/* retrieve vdev pointer from vdev list */
	vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_id];

	if (vdev && vdev->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED)
		wlan_objmgr_vdev_get_ref(vdev, dbg_id);
	else
		vdev = NULL;
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_opmode_from_psoc_debug(
			struct wlan_objmgr_psoc *psoc,
			enum QDF_OPMODE opmode,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	int vdev_cnt = 0;
	uint16_t max_vdev_cnt;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	wlan_psoc_obj_lock(psoc);

	max_vdev_cnt = wlan_psoc_get_max_vdev_count(psoc);
	/* retrieve vdev pointer from vdev list */
	while (vdev_cnt < max_vdev_cnt) {
		vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_cnt];
		vdev_cnt++;
		if (!vdev)
			continue;
		wlan_vdev_obj_lock(vdev);
		if (vdev->vdev_mlme.vdev_opmode == opmode) {
			wlan_vdev_obj_unlock(vdev);
			if (wlan_objmgr_vdev_try_get_ref_debug(vdev, dbg_id,
							       func, line) !=
				QDF_STATUS_SUCCESS) {
				vdev = NULL;
				continue;
			}
			break;
		}
		wlan_vdev_obj_unlock(vdev);
		vdev = NULL;
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_opmode_from_psoc(
			struct wlan_objmgr_psoc *psoc,
			enum QDF_OPMODE opmode,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	int vdev_cnt = 0;
	uint16_t max_vdev_cnt;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	wlan_psoc_obj_lock(psoc);

	max_vdev_cnt = wlan_psoc_get_max_vdev_count(psoc);
	/* retrieve vdev pointer from vdev list */
	while (vdev_cnt < max_vdev_cnt) {
		vdev = psoc->soc_objmgr.wlan_vdev_list[vdev_cnt];
		vdev_cnt++;
		if (!vdev)
			continue;
		wlan_vdev_obj_lock(vdev);
		if (vdev->vdev_mlme.vdev_opmode == opmode) {
			wlan_vdev_obj_unlock(vdev);
			if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) !=
							QDF_STATUS_SUCCESS) {
				vdev = NULL;
				continue;
			}
			break;
		}
		wlan_vdev_obj_unlock(vdev);
		vdev = NULL;
	}
	wlan_psoc_obj_unlock(psoc);

	return vdev;
}
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		const uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	if (!macaddr)
		return NULL;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbg_id);
	if (!pdev) {
		obj_mgr_err("pdev is null");
		return NULL;
	}
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev_debug(pdev, macaddr,
							       dbg_id,
							       func, line);
	wlan_objmgr_pdev_release_ref(pdev, dbg_id);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_macaddr_from_psoc_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		const uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	if (!macaddr)
		return NULL;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbg_id);
	if (!pdev) {
		obj_mgr_err("pdev is null");
		return NULL;
	}
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, macaddr, dbg_id);
	wlan_objmgr_pdev_release_ref(pdev, dbg_id);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_macaddr_from_psoc);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev
	*wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		const uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	if (!macaddr)
		return NULL;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbg_id);
	if (!pdev) {
		obj_mgr_err("pdev is null");
		return NULL;
	}
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state_debug(pdev,
									macaddr,
									dbg_id,
									func,
									line);
	wlan_objmgr_pdev_release_ref(pdev, dbg_id);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state_debug);
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		const uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	/* if PSOC is NULL, return */
	if (!psoc)
		return NULL;

	if (!macaddr)
		return NULL;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, dbg_id);
	if (!pdev) {
		obj_mgr_err("pdev is null");
		return NULL;
	}
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state(pdev, macaddr, dbg_id);
	wlan_objmgr_pdev_release_ref(pdev, dbg_id);

	return vdev;
}

qdf_export_symbol(wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state);
#endif

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

	if (!peer)
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
 * wlan_obj_psoc_peerlist_get_peer_by_pdev_id() - get peer from
 *                                                      psoc peer list
 * @psoc: PSOC object
 * @macaddr: MAC address
 * #pdev_id: Pdev id
 *
 * API to finds peer object pointer by MAC addr and pdev id from hash list
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer
	*wlan_obj_psoc_peerlist_get_peer_by_pdev_id_debug(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t pdev_id, wlan_objmgr_ref_dbgid dbg_id,
				const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
			if (wlan_objmgr_peer_try_get_ref_debug(peer, dbg_id,
							       func, line) ==
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
#else
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_by_pdev_id(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t pdev_id, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
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
#endif

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
#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_debug(
		qdf_list_t *obj_list, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
				== QDF_STATUS_SUCCESS) {
			if (wlan_objmgr_peer_try_get_ref_debug(peer, dbg_id,
							       func, line) ==
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
#else
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer(
		qdf_list_t *obj_list, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
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
#endif

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
#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer *
		wlan_obj_psoc_peerlist_get_peer_logically_deleted_debug(
				qdf_list_t *obj_list, uint8_t *macaddr,
				wlan_objmgr_ref_dbgid dbg_id,
				const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/* Return peer in logically deleted state */
			if (peer->obj_state ==
					WLAN_OBJ_STATE_LOGICALLY_DELETED) {
				wlan_objmgr_peer_get_ref_debug(peer, dbg_id,
							       func, line);

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
#else
static struct wlan_objmgr_peer *
			wlan_obj_psoc_peerlist_get_peer_logically_deleted(
				qdf_list_t *obj_list, uint8_t *macaddr,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
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
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer
		*wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state_debug(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid,
					uint8_t pdev_id,
					wlan_objmgr_ref_dbgid dbg_id,
					const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((wlan_peer_bssid_match(peer, bssid) ==
				QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
				wlan_objmgr_peer_get_ref_debug(peer, dbg_id,
							       func, line);

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
#else
static struct wlan_objmgr_peer
		*wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid,
					uint8_t pdev_id,
					wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((wlan_peer_bssid_match(peer, bssid) ==
				QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
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
#endif

/**
 * wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid() - get peer
 *                                                    from psoc peer
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
#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer
		*wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_debug(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid, uint8_t pdev_id,
					wlan_objmgr_ref_dbgid dbg_id,
					const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((wlan_peer_bssid_match(peer, bssid) ==
				QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
				if (wlan_objmgr_peer_try_get_ref_debug(peer,
								       dbg_id,
								       func,
								       line)
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
#else
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid(
					qdf_list_t *obj_list, uint8_t *macaddr,
					uint8_t *bssid, uint8_t pdev_id,
					wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr is key */
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) {
			/*
			 *  BSSID match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((wlan_peer_bssid_match(peer, bssid) ==
				QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
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
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_no_state_debug(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t pdev_id, wlan_objmgr_ref_dbgid dbg_id,
				const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr and pdev_id is key */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
			wlan_objmgr_peer_get_ref_debug(peer, dbg_id, func,
						       line);

			return peer;
		}
		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
	}

	/* Not found, return NULL */
	return NULL;
}
#else
static struct wlan_objmgr_peer *wlan_obj_psoc_peerlist_get_peer_no_state(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t pdev_id, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		/* For peer, macaddr and pdev_id is key */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
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
#endif

/**
 * wlan_obj_psoc_populate_logically_del_peerlist_by_mac_n_bssid() -
 *                                           get peer
 *                                           from psoc peer list using
 *                                           mac and vdev self mac
 * @obj_list: peer object list
 * @macaddr: MAC address
 * @bssid: BSSID address
 * @dbg_id: id of the caller
 * @func: function name
 * @line: line number
 *
 * API to finds peer object pointer by MAC addr and BSSID from
 * peer hash list for a node which is in logically deleted state,
 * bssid check is done on matching peer
 *
 * Caller to free the list allocated in this function
 *
 * Return: list of peer pointers
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
static qdf_list_t
	*wlan_obj_psoc_populate_logically_del_peerlist_by_mac_n_bssid_debug(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t *bssid, uint8_t pdev_id,
				wlan_objmgr_ref_dbgid dbg_id,
				const char *func, int line)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;
	struct wlan_logically_del_peer *peer_list = NULL;
	qdf_list_t *logical_del_peer_list = NULL;
	bool lock_released = false;

	logical_del_peer_list = qdf_mem_malloc(sizeof(*logical_del_peer_list));
	if (!logical_del_peer_list)
		return NULL;

	qdf_list_create(logical_del_peer_list, WLAN_UMAC_PSOC_MAX_PEERS);

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		wlan_peer_obj_lock(peer);
		/* For peer, macaddr and pdev id are keys */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
			/*
			 *  if BSSID not NULL,
			 *  then match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((!bssid) ||
			    (wlan_peer_bssid_match(peer, bssid) ==
				 QDF_STATUS_SUCCESS)) {
				/* Return peer in logically deleted state */
				if ((peer->obj_state ==
					WLAN_OBJ_STATE_LOGICALLY_DELETED) &&
				     qdf_atomic_read(
						&peer->peer_objmgr.ref_cnt)) {
					wlan_objmgr_peer_get_ref_debug(peer,
								       dbg_id,
								       func,
								       line);
					wlan_peer_obj_unlock(peer);
					lock_released = true;

					peer_list =
					qdf_mem_malloc(
					sizeof(struct wlan_logically_del_peer));
					if (!peer_list) {
						wlan_objmgr_peer_release_ref(peer, dbg_id);
						/* Lock is already released */
						WLAN_OBJMGR_BUG(0);
						break;
					}

					peer_list->peer = peer;

					qdf_list_insert_front(
						logical_del_peer_list,
							&peer_list->list);
				}
			}
		}

		if (!lock_released)
			wlan_peer_obj_unlock(peer);

		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
		lock_released = false;
	}

	/* Not found, return NULL */
	if (qdf_list_empty(logical_del_peer_list)) {
		qdf_mem_free(logical_del_peer_list);
		return NULL;
	} else {
		return logical_del_peer_list;
	}
}
#else
static qdf_list_t
	*wlan_obj_psoc_populate_logically_del_peerlist_by_mac_n_bssid(
				qdf_list_t *obj_list, uint8_t *macaddr,
				uint8_t *bssid, uint8_t pdev_id,
				wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_peer *peer_temp;
	struct wlan_logically_del_peer *peer_list = NULL;
	qdf_list_t *logical_del_peer_list = NULL;
	bool lock_released = false;

	logical_del_peer_list = qdf_mem_malloc(sizeof(*logical_del_peer_list));
	if (!logical_del_peer_list)
		return NULL;

	qdf_list_create(logical_del_peer_list, WLAN_UMAC_PSOC_MAX_PEERS);

	/* Iterate through hash list to get the peer */
	peer = wlan_psoc_peer_list_peek_head(obj_list);
	while (peer) {
		wlan_peer_obj_lock(peer);
		/* For peer, macaddr and pdev id are keys */
		if ((WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer), macaddr)
			== QDF_STATUS_SUCCESS) &&
				(wlan_peer_get_pdev_id(peer) == pdev_id)) {
			/*
			 *  if BSSID not NULL,
			 *  then match is requested by caller, check BSSID
			 *  (vdev mac == bssid) --  return peer
			 *  (vdev mac != bssid) --  perform next iteration
			 */
			if ((!bssid) ||
			    (wlan_peer_bssid_match(peer, bssid) ==
				 QDF_STATUS_SUCCESS)) {
				/* Return peer in logically deleted state */
				if ((peer->obj_state ==
					WLAN_OBJ_STATE_LOGICALLY_DELETED) &&
				     qdf_atomic_read(
						&peer->peer_objmgr.ref_cnt)) {
					wlan_objmgr_peer_get_ref(peer, dbg_id);
					wlan_peer_obj_unlock(peer);
					lock_released = true;

					peer_list =
					qdf_mem_malloc(
					sizeof(struct wlan_logically_del_peer));
					if (!peer_list) {
						wlan_objmgr_peer_release_ref(peer, dbg_id);
						/* Lock is already released */
						WLAN_OBJMGR_BUG(0);
						break;
					}

					peer_list->peer = peer;

					qdf_list_insert_front(
						logical_del_peer_list,
							&peer_list->list);
				}
			}
		}

		if (!lock_released)
			wlan_peer_obj_unlock(peer);

		/* Move to next peer */
		peer_temp = peer;
		peer = wlan_peer_get_next_peer_of_psoc(obj_list, peer_temp);
		lock_released = false;
	}

	/* Not found, return NULL */
	if (qdf_list_empty(logical_del_peer_list)) {
		qdf_mem_free(logical_del_peer_list);
		return NULL;
	} else {
		return logical_del_peer_list;
	}
}
#endif

QDF_STATUS wlan_objmgr_psoc_peer_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_peer_list *peer_list;

	wlan_psoc_obj_lock(psoc);
	objmgr = &psoc->soc_objmgr;
	/* Max temporary peer limit is reached, return failure */
	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP ||
	    peer->peer_mlme.peer_type == WLAN_PEER_MLO_TEMP) {
		if (objmgr->temp_peer_count >= WLAN_MAX_PSOC_TEMP_PEERS) {
			wlan_psoc_obj_unlock(psoc);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		/* Max peer limit is reached, return failure */
		if (objmgr->wlan_peer_count
			>= wlan_psoc_get_max_peer_count(psoc)) {
			wlan_psoc_obj_unlock(psoc);
			return QDF_STATUS_E_FAILURE;
		}
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
	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP ||
	    peer->peer_mlme.peer_type == WLAN_PEER_MLO_TEMP)
		objmgr->temp_peer_count++;
	else
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
	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP ||
	    peer->peer_mlme.peer_type == WLAN_PEER_MLO_TEMP)
		objmgr->temp_peer_count--;
	else
		objmgr->wlan_peer_count--;
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	if (pdev_id >= WLAN_UMAC_MAX_PDEVS)
		QDF_ASSERT(0);

	if (!macaddr)
		return NULL;

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
	peer = wlan_obj_psoc_peerlist_get_peer_by_pdev_id_debug(
		&peer_list->peer_hash[hash_index], macaddr,
		pdev_id, dbg_id, func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	if (pdev_id >= WLAN_UMAC_MAX_PDEVS)
		QDF_ASSERT(0);

	if (!macaddr)
		return NULL;

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
	peer = wlan_obj_psoc_peerlist_get_peer_by_pdev_id(
		&peer_list->peer_hash[hash_index], macaddr, pdev_id, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	if (!macaddr)
		return NULL;

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
	peer = wlan_obj_psoc_peerlist_get_peer_debug(
			&peer_list->peer_hash[hash_index],
			macaddr, dbg_id, func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_by_mac_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_peer_list *peer_list;

	if (!macaddr)
		return NULL;

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

qdf_export_symbol(wlan_objmgr_get_peer_by_mac);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_logically_deleted_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
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
	peer = wlan_obj_psoc_peerlist_get_peer_logically_deleted_debug(
		&peer_list->peer_hash[hash_index], macaddr, dbg_id,
		func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}
#else
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
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state_debug(
		&peer_list->peer_hash[hash_index], macaddr, bssid,
		pdev_id, dbg_id, func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_by_mac_n_vdev_no_state_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_no_state(
		&peer_list->peer_hash[hash_index], macaddr, bssid,
		pdev_id, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_by_mac_n_vdev_no_state);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid_debug(
		&peer_list->peer_hash[hash_index], macaddr, bssid,
		pdev_id, dbg_id, func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_by_mac_n_vdev_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_mac_n_bssid(
		&peer_list->peer_hash[hash_index], macaddr, bssid,
		pdev_id, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_by_mac_n_vdev);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_nolock_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_pdev_id_debug(
		&peer_list->peer_hash[hash_index], macaddr,
		pdev_id, dbg_id, func, line);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_nolock_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_nolock(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id)
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
	peer = wlan_obj_psoc_peerlist_get_peer_by_pdev_id(
		&peer_list->peer_hash[hash_index], macaddr, pdev_id, dbg_id);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_nolock);
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
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
	peer = wlan_obj_psoc_peerlist_get_peer_no_state_debug(
		&peer_list->peer_hash[hash_index], macaddr,
		pdev_id, dbg_id, func, line);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_no_state_debug);
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id)
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
		&peer_list->peer_hash[hash_index], macaddr, pdev_id, dbg_id);
	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return peer;
}

qdf_export_symbol(wlan_objmgr_get_peer_no_state);
#endif

/**
 * wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev() -
 *                                                           get peer from psoc
 *                                                           peer list using
 *                                                           mac and vdev
 *                                                           self mac
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: MAC address
 * @bssid: BSSID address. NULL mac means search all.
 * @dbg_id: id of the caller
 * @func: function name
 * @line: line number
 *
 * API to finds peer object pointer by MAC addr and BSSID from
 * peer hash list, bssid check is done on matching peer
 *
 * Return: list of peer pointer pointers
 *         NULL on FAILURE
 */

#ifdef WLAN_OBJMGR_REF_ID_TRACE
qdf_list_t *wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_peer_list *peer_list = NULL;
	qdf_list_t *logical_del_peer_list = NULL;

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
	logical_del_peer_list =
	wlan_obj_psoc_populate_logically_del_peerlist_by_mac_n_bssid_debug(
			&peer_list->peer_hash[hash_index], macaddr,
			bssid, pdev_id, dbg_id, func, line);

	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return logical_del_peer_list;
}

qdf_export_symbol(wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev_debug);
#else
qdf_list_t *wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_psoc_objmgr *objmgr;
	uint8_t hash_index;
	struct wlan_peer_list *peer_list = NULL;
	qdf_list_t *logical_del_peer_list = NULL;

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
	logical_del_peer_list =
		wlan_obj_psoc_populate_logically_del_peerlist_by_mac_n_bssid(
			&peer_list->peer_hash[hash_index], macaddr,
			bssid, pdev_id, dbg_id);

	qdf_spin_unlock_bh(&peer_list->peer_list_lock);
	wlan_psoc_obj_unlock(psoc);

	return logical_del_peer_list;
}

qdf_export_symbol(wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev);
#endif

void *wlan_objmgr_psoc_get_comp_private_obj(struct wlan_objmgr_psoc *psoc,
					enum wlan_umac_comp_id id)
{
	void *comp_private_obj;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		QDF_BUG(0);
		return NULL;
	}

	if (!psoc) {
		QDF_BUG(0);
		return NULL;
	}

	comp_private_obj = psoc->soc_comp_priv_obj[id];

	return comp_private_obj;
}
qdf_export_symbol(wlan_objmgr_psoc_get_comp_private_obj);

void wlan_objmgr_psoc_get_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (!psoc) {
		obj_mgr_err("psoc obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return;
	}
	/* Increment ref count */
	qdf_atomic_inc(&psoc->soc_objmgr.ref_cnt);
	qdf_atomic_inc(&psoc->soc_objmgr.ref_id_dbg[id]);
	return;
}
qdf_export_symbol(wlan_objmgr_psoc_get_ref);

QDF_STATUS wlan_objmgr_psoc_try_get_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (!psoc) {
		obj_mgr_err("psoc obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_psoc_obj_lock(psoc);
	if (psoc->obj_state != WLAN_OBJ_STATE_CREATED) {
		wlan_psoc_obj_unlock(psoc);
		if (psoc->soc_objmgr.print_cnt++ <=
				WLAN_OBJMGR_RATELIMIT_THRESH)
			obj_mgr_err(
			"[Ref id: %d] psoc is not in Created state(%d)",
					id, psoc->obj_state);

		return QDF_STATUS_E_RESOURCES;
	}

	/* Increment ref count */
	wlan_objmgr_psoc_get_ref(psoc, id);
	wlan_psoc_obj_unlock(psoc);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_psoc_try_get_ref);

void wlan_objmgr_psoc_release_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id)
{
	if (!psoc) {
		obj_mgr_err("psoc obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return;
	}

	if (!qdf_atomic_read(&psoc->soc_objmgr.ref_id_dbg[id])) {
		obj_mgr_err("psoc ref cnt was not taken by %d", id);
		wlan_objmgr_print_ref_ids(psoc->soc_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_FATAL);
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
qdf_export_symbol(wlan_objmgr_psoc_release_ref);

static void wlan_objmgr_psoc_peer_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_peer *peer = (struct wlan_objmgr_peer *)obj;
	WLAN_OBJ_STATE obj_state;
	uint8_t vdev_id;
	uint8_t *macaddr;

	wlan_peer_obj_lock(peer);
	macaddr = wlan_peer_get_macaddr(peer);
	obj_state = peer->obj_state;
	vdev_id = wlan_vdev_get_id(wlan_peer_get_vdev(peer));
	wlan_peer_obj_unlock(peer);

	obj_mgr_alert("Peer MAC:%02x:%02x:%02x:%02x:%02x:%02x state:%d vdev_id:%d",
		  macaddr[0], macaddr[1], macaddr[2], macaddr[3],
		  macaddr[4], macaddr[5], obj_state, vdev_id);
	wlan_objmgr_print_peer_ref_ids(peer, QDF_TRACE_LEVEL_FATAL);
}

static void wlan_objmgr_psoc_vdev_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	WLAN_OBJ_STATE obj_state;
	uint8_t id;

	wlan_vdev_obj_lock(vdev);
	id = wlan_vdev_get_id(vdev);
	obj_state =  vdev->obj_state;
	wlan_vdev_obj_unlock(vdev);
	obj_mgr_alert("Vdev ID is %d, state %d", id, obj_state);

	wlan_objmgr_print_ref_ids(vdev->vdev_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_FATAL);
}

static void wlan_objmgr_psoc_pdev_ref_print(struct wlan_objmgr_psoc *psoc,
					 void *obj, void *args)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)obj;
	uint8_t id;

	wlan_pdev_obj_lock(pdev);
	id = wlan_objmgr_pdev_get_pdev_id(pdev);
	wlan_pdev_obj_unlock(pdev);
	obj_mgr_alert("pdev ID is %d", id);

	wlan_objmgr_print_ref_ids(pdev->pdev_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_FATAL);
}

QDF_STATUS wlan_objmgr_print_ref_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc)
{
	obj_mgr_alert("Ref counts of PEER");
	wlan_objmgr_iterate_obj_list_all_noref(psoc, WLAN_PEER_OP,
				wlan_objmgr_psoc_peer_ref_print, NULL);
	obj_mgr_alert("Ref counts of VDEV");
	wlan_objmgr_iterate_obj_list_all_noref(psoc, WLAN_VDEV_OP,
				wlan_objmgr_psoc_vdev_ref_print, NULL);
	obj_mgr_alert("Ref counts of PDEV");
	wlan_objmgr_iterate_obj_list_all_noref(psoc, WLAN_PDEV_OP,
				wlan_objmgr_psoc_pdev_ref_print, NULL);

	obj_mgr_alert(" Ref counts of PSOC");
	wlan_objmgr_print_ref_ids(psoc->soc_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_FATAL);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_print_ref_all_objects_per_psoc);

QDF_STATUS wlan_objmgr_psoc_set_user_config(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_psoc_user_config *user_config_data)
{
	if (!user_config_data) {
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

uint32_t wlan_objmgr_psoc_check_for_pdev_leaks(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc_objmgr *_psoc;
	struct wlan_objmgr_pdev *pdev;
	int pdev_id;
	uint32_t leaks = 0;

	QDF_BUG(psoc);
	if (!psoc)
		return leaks;

	wlan_psoc_obj_lock(psoc);
	_psoc = &psoc->soc_objmgr;
	if (!_psoc->wlan_pdev_count) {
		wlan_psoc_obj_unlock(psoc);
		return leaks;
	}

	obj_mgr_alert("objmgr pdev leaks detected for psoc %u!",
		      _psoc->psoc_id);
	obj_mgr_alert("----------------------------------------------------");
	obj_mgr_alert("Pdev Id   Refs   Module");
	obj_mgr_alert("----------------------------------------------------");

	wlan_objmgr_for_each_psoc_pdev(psoc, pdev_id, pdev) {
		qdf_atomic_t *ref_id_dbg;
		int ref_id;
		int32_t refs;

		wlan_pdev_obj_lock(pdev);
		ref_id_dbg = pdev->pdev_objmgr.ref_id_dbg;
		wlan_objmgr_for_each_refs(ref_id_dbg, ref_id, refs) {
			leaks++;
			obj_mgr_alert("%7u   %4u   %s(%d)",
				      pdev_id, refs,
				      string_from_dbgid(ref_id), ref_id);
		}
		wlan_pdev_obj_unlock(pdev);
	}

	wlan_psoc_obj_unlock(psoc);
	return leaks;
}
qdf_export_symbol(wlan_objmgr_psoc_check_for_pdev_leaks);

uint32_t wlan_objmgr_psoc_check_for_vdev_leaks(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc_objmgr *_psoc;
	struct wlan_objmgr_vdev *vdev;
	int vdev_id;
	uint32_t leaks = 0;

	QDF_BUG(psoc);
	if (!psoc)
		return leaks;

	wlan_psoc_obj_lock(psoc);
	_psoc = &psoc->soc_objmgr;
	if (!_psoc->wlan_vdev_count) {
		wlan_psoc_obj_unlock(psoc);
		return leaks;
	}

	obj_mgr_alert("objmgr vdev leaks detected for psoc %u!",
		      _psoc->psoc_id);
	obj_mgr_alert("----------------------------------------------------");
	obj_mgr_alert("Vdev Id   Refs   Module");
	obj_mgr_alert("----------------------------------------------------");

	wlan_objmgr_for_each_psoc_vdev(psoc, vdev_id, vdev) {
		qdf_atomic_t *ref_id_dbg;
		int ref_id;
		int32_t refs;

		wlan_vdev_obj_lock(vdev);
		ref_id_dbg = vdev->vdev_objmgr.ref_id_dbg;
		wlan_objmgr_for_each_refs(ref_id_dbg, ref_id, refs) {
			leaks++;
			obj_mgr_alert("%7u   %4u   %s(%d)",
				      vdev_id, refs, string_from_dbgid(ref_id),
				      ref_id);
		}
		wlan_vdev_obj_unlock(vdev);
	}

	wlan_psoc_obj_unlock(psoc);
	return leaks;
}
qdf_export_symbol(wlan_objmgr_psoc_check_for_vdev_leaks);

#ifdef WLAN_OBJMGR_REF_ID_DEBUG
static void
wlan_objmgr_print_peer_ref_leaks(struct wlan_objmgr_peer *peer, int vdev_id)
{
	qdf_atomic_t *ref_id_dbg;
	int32_t refs;
	int ref_id;

	ref_id_dbg = peer->peer_objmgr.ref_id_dbg;
	wlan_objmgr_for_each_refs(ref_id_dbg, ref_id, refs) {
		obj_mgr_alert(QDF_MAC_ADDR_FMT " %7u   %4u   %s(%d)",
			      QDF_MAC_ADDR_REF(peer->macaddr),
			      vdev_id,
			      refs,
			      string_from_dbgid(ref_id), ref_id);
	}
}
#else
static inline void
wlan_objmgr_print_peer_ref_leaks(struct wlan_objmgr_peer *peer, int vdev_id)
{
	obj_mgr_alert(QDF_MAC_ADDR_FMT " %7u   %4u   %s",
		      QDF_MAC_ADDR_REF(peer->macaddr),
		      vdev_id,
		      qdf_atomic_read(&peer->peer_objmgr.ref_cnt),
		      "TOTAL_REF_COUNT");
}
#endif

uint32_t wlan_objmgr_psoc_check_for_peer_leaks(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc_objmgr *_psoc;
	struct wlan_objmgr_vdev *vdev;
	int vdev_id;
	uint32_t leaks = 0;

	QDF_BUG(psoc);
	if (!psoc)
		return leaks;

	wlan_psoc_obj_lock(psoc);
	_psoc = &psoc->soc_objmgr;
	if (!_psoc->temp_peer_count && !_psoc->wlan_peer_count) {
		wlan_psoc_obj_unlock(psoc);
		return leaks;
	}

	obj_mgr_alert("objmgr peer leaks detected for psoc %u!",
		      _psoc->psoc_id);
	obj_mgr_alert("----------------------------------------------------");
	obj_mgr_alert("Peer MAC          Vdev Id   Refs   Module");
	obj_mgr_alert("----------------------------------------------------");

	wlan_objmgr_for_each_psoc_vdev(psoc, vdev_id, vdev) {
		struct wlan_objmgr_peer *peer;

		wlan_vdev_obj_lock(vdev);
		wlan_objmgr_for_each_vdev_peer(vdev, peer) {
			wlan_peer_obj_lock(peer);
			leaks += qdf_atomic_read(&peer->peer_objmgr.ref_cnt);
			wlan_objmgr_print_peer_ref_leaks(peer, vdev_id);
			wlan_peer_obj_unlock(peer);
		}
		wlan_vdev_obj_unlock(vdev);
	}

	wlan_psoc_obj_unlock(psoc);
	return leaks;
}
qdf_export_symbol(wlan_objmgr_psoc_check_for_peer_leaks);

void wlan_objmgr_psoc_check_for_leaks(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc_objmgr *_psoc;
	uint32_t peer_leaks = 0;
	uint32_t vdev_leaks = 0;
	uint32_t pdev_leaks = 0;

	_psoc = &psoc->soc_objmgr;

	peer_leaks = wlan_objmgr_psoc_check_for_peer_leaks(psoc);
	vdev_leaks = wlan_objmgr_psoc_check_for_vdev_leaks(psoc);
	pdev_leaks = wlan_objmgr_psoc_check_for_pdev_leaks(psoc);

	if (peer_leaks || vdev_leaks || pdev_leaks) {
		QDF_DEBUG_PANIC("%u objmgr peer leaks %u objmgr vdev leaks"
				"%u objmgr pdev leaks detected for psoc %u!",
				peer_leaks, vdev_leaks, pdev_leaks,
				_psoc->psoc_id);
	}
}

qdf_export_symbol(wlan_objmgr_psoc_check_for_leaks);

#ifdef WLAN_OBJMGR_DEBUG
void wlan_print_psoc_info(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc_objmgr *psoc_objmgr;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	uint16_t index = 0;

	psoc_objmgr = &psoc->soc_objmgr;

	obj_mgr_debug("psoc: %pK", psoc);
	obj_mgr_debug("psoc_id: %d", psoc_objmgr->psoc_id);
	obj_mgr_debug("wlan_pdev_count: %d", psoc_objmgr->wlan_pdev_count);
	obj_mgr_debug("wlan_pdev_id_map: 0x%x", psoc_objmgr->wlan_pdev_id_map);
	obj_mgr_debug("wlan_vdev_count: %d", psoc_objmgr->wlan_vdev_count);
	obj_mgr_debug("max_vdev_count: %d", psoc_objmgr->max_vdev_count);
	obj_mgr_debug("wlan_peer_count: %d", psoc_objmgr->wlan_peer_count);
	obj_mgr_debug("max_peer_count: %d", psoc_objmgr->max_peer_count);
	obj_mgr_debug("temp_peer_count: %d", psoc_objmgr->temp_peer_count);
	obj_mgr_debug("ref_cnt: %d", qdf_atomic_read(&psoc_objmgr->ref_cnt));
	obj_mgr_debug("qdf_dev: %pK", psoc_objmgr->qdf_dev);

	obj_mgr_debug("wlan_vdev_id_map:");
	obj_mgr_debug_hex(psoc_objmgr->wlan_vdev_id_map,
			  sizeof(psoc_objmgr->wlan_vdev_id_map));

	wlan_objmgr_for_each_psoc_pdev(psoc, index, pdev) {
		obj_mgr_debug("wlan_pdev_list[%d]: %pK", index, pdev);
		wlan_print_pdev_info(pdev);
	}

	wlan_objmgr_for_each_psoc_vdev(psoc, index, vdev) {
		obj_mgr_debug("wlan_vdev_list[%d]: %pK", index, vdev);
		wlan_print_vdev_info(vdev);
	}
}

qdf_export_symbol(wlan_print_psoc_info);
#endif
