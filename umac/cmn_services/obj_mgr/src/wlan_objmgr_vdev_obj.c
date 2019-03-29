/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
#include "wlan_objmgr_vdev_obj_i.h"

/**
 ** APIs to Create/Delete Global object APIs
 */

static QDF_STATUS wlan_objmgr_vdev_object_status(
		struct wlan_objmgr_vdev *vdev)
{
	uint8_t id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	wlan_vdev_obj_lock(vdev);

	/* Iterate through all components to derive the object status */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		/* If component disabled, Ignore */
		if (vdev->obj_status[id] == QDF_STATUS_COMP_DISABLED) {
			continue;
		/*
		 * If component operates in Async, status is Partially created,
		 * break
		 */
		} else if (vdev->obj_status[id] == QDF_STATUS_COMP_ASYNC) {
			if (!vdev->vdev_comp_priv_obj[id]) {
				status = QDF_STATUS_COMP_ASYNC;
				break;
			}
		/*
		 * If component failed to allocate its object, treat it as
		 * failure, complete object need to be cleaned up
		 */
		} else if ((vdev->obj_status[id] == QDF_STATUS_E_NOMEM) ||
			(vdev->obj_status[id] == QDF_STATUS_E_FAILURE)) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
	}
	wlan_vdev_obj_unlock(vdev);

	return status;
}

static QDF_STATUS wlan_objmgr_vdev_obj_free(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	if (!vdev) {
		obj_mgr_err("vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/* if PDEV is NULL, return */
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		obj_mgr_err("pdev is NULL for vdev-id: %d",
			vdev->vdev_objmgr.vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		obj_mgr_err("psoc is NULL in pdev");
		return QDF_STATUS_E_FAILURE;
	}

	/* Detach VDEV from PDEV VDEV's list */
	if (wlan_objmgr_pdev_vdev_detach(pdev, vdev) ==
					QDF_STATUS_E_FAILURE)
		return QDF_STATUS_E_FAILURE;

	/* Detach VDEV from PSOC VDEV's list */
	if (wlan_objmgr_psoc_vdev_detach(psoc, vdev) ==
					 QDF_STATUS_E_FAILURE)
		return QDF_STATUS_E_FAILURE;

	qdf_spinlock_destroy(&vdev->vdev_lock);

	qdf_mem_free(vdev->vdev_mlme.bss_chan);
	qdf_mem_free(vdev->vdev_mlme.des_chan);
	qdf_mem_free(vdev);

	return QDF_STATUS_SUCCESS;

}

struct wlan_objmgr_vdev *wlan_objmgr_vdev_obj_create(
			struct wlan_objmgr_pdev *pdev,
			struct wlan_vdev_create_params *params)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	uint8_t id;
	wlan_objmgr_vdev_create_handler handler;
	wlan_objmgr_vdev_status_handler stat_handler;
	void *arg;
	QDF_STATUS obj_status;

	if (!pdev) {
		obj_mgr_err("pdev is NULL");
		return NULL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	/* PSOC is NULL */
	if (!psoc) {
		obj_mgr_err("psoc is NULL for pdev-id:%d",
			pdev->pdev_objmgr.wlan_pdev_id);
		return NULL;
	}
	/* Allocate vdev object memory */
	vdev = qdf_mem_malloc(sizeof(*vdev));
	if (!vdev)
		return NULL;

	vdev->obj_state = WLAN_OBJ_STATE_ALLOCATED;

	vdev->vdev_mlme.bss_chan = qdf_mem_malloc(sizeof(struct wlan_channel));
	if (!vdev->vdev_mlme.bss_chan) {
		qdf_mem_free(vdev);
		return NULL;
	}

	vdev->vdev_mlme.des_chan = qdf_mem_malloc(sizeof(struct wlan_channel));
	if (!vdev->vdev_mlme.des_chan) {
		qdf_mem_free(vdev->vdev_mlme.bss_chan);
		qdf_mem_free(vdev);
		return NULL;
	}

	/* Initialize spinlock */
	qdf_spinlock_create(&vdev->vdev_lock);
	/* Attach VDEV to PSOC VDEV's list */
	if (wlan_objmgr_psoc_vdev_attach(psoc, vdev) !=
				QDF_STATUS_SUCCESS) {
		obj_mgr_err("psoc vdev attach failed for vdev-id:%d",
					vdev->vdev_objmgr.vdev_id);
		qdf_mem_free(vdev->vdev_mlme.bss_chan);
		qdf_mem_free(vdev->vdev_mlme.des_chan);
		qdf_spinlock_destroy(&vdev->vdev_lock);
		qdf_mem_free(vdev);
		return NULL;
	}
	/* Store pdev in vdev */
	wlan_vdev_set_pdev(vdev, pdev);
	/* Attach vdev to PDEV */
	if (wlan_objmgr_pdev_vdev_attach(pdev, vdev) !=
				QDF_STATUS_SUCCESS) {
		obj_mgr_err("pdev vdev attach failed for vdev-id:%d",
				vdev->vdev_objmgr.vdev_id);
		wlan_objmgr_psoc_vdev_detach(psoc, vdev);
		qdf_mem_free(vdev->vdev_mlme.bss_chan);
		qdf_mem_free(vdev->vdev_mlme.des_chan);
		qdf_spinlock_destroy(&vdev->vdev_lock);
		qdf_mem_free(vdev);
		return NULL;
	}
	/* set opmode */
	wlan_vdev_mlme_set_opmode(vdev, params->opmode);
	/* set MAC address */
	wlan_vdev_mlme_set_macaddr(vdev, params->macaddr);
	/* set MAT address */
	wlan_vdev_mlme_set_mataddr(vdev, params->mataddr);
	/* Set create flags */
	vdev->vdev_objmgr.c_flags = params->flags;
	/* store os-specific pointer */
	vdev->vdev_nif.osdev = params->osifp;
	/* peer count to 0 */
	vdev->vdev_objmgr.wlan_peer_count = 0;
	qdf_atomic_init(&vdev->vdev_objmgr.ref_cnt);
	vdev->vdev_objmgr.print_cnt = 0;
	wlan_objmgr_vdev_get_ref(vdev, WLAN_OBJMGR_ID);
	/* Initialize max peer count based on opmode type */
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		vdev->vdev_objmgr.max_peer_count = WLAN_UMAC_MAX_STA_PEERS;
	else
		vdev->vdev_objmgr.max_peer_count =
				wlan_pdev_get_max_peer_count(pdev);

	/* Initialize peer list */
	qdf_list_create(&vdev->vdev_objmgr.wlan_peer_list,
			vdev->vdev_objmgr.max_peer_count +
			WLAN_MAX_PDEV_TEMP_PEERS);
	/* TODO init other parameters */

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->vdev_create_handler[id];
		arg = g_umac_glb_obj->vdev_create_handler_arg[id];
		if (handler)
			vdev->obj_status[id] = handler(vdev, arg);
		else
			vdev->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}

	/* Derive object status */
	obj_status = wlan_objmgr_vdev_object_status(vdev);

	if (obj_status == QDF_STATUS_SUCCESS) {
		/* Object status is SUCCESS, Object is created */
		vdev->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Invoke component registered status handlers */
		for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
			stat_handler = g_umac_glb_obj->vdev_status_handler[id];
			arg = g_umac_glb_obj->vdev_status_handler_arg[id];
			if (stat_handler) {
				stat_handler(vdev, arg,
					     QDF_STATUS_SUCCESS);
			}
		}
	/*
	 * Few components operates in Asynchrous communction, Object state
	 * partially created
	 */
	} else if (obj_status == QDF_STATUS_COMP_ASYNC) {
		vdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
	/* Component object failed to be created, clean up the object */
	} else if (obj_status == QDF_STATUS_E_FAILURE) {
		/* Clean up the psoc */
		wlan_objmgr_vdev_obj_delete(vdev);
		obj_mgr_err("VDEV comp objects creation failed for vdev-id:%d",
			vdev->vdev_objmgr.vdev_id);
		return NULL;
	}

	obj_mgr_info("Created vdev %d", vdev->vdev_objmgr.vdev_id);

	return vdev;
}
qdf_export_symbol(wlan_objmgr_vdev_obj_create);

static QDF_STATUS wlan_objmgr_vdev_obj_destroy(struct wlan_objmgr_vdev *vdev)
{
	uint8_t id;
	wlan_objmgr_vdev_destroy_handler handler;
	QDF_STATUS obj_status;
	void *arg;
	uint8_t vdev_id;

	if (!vdev) {
		obj_mgr_err("vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_notify_destroy(vdev, WLAN_VDEV_OP);

	vdev_id = wlan_vdev_get_id(vdev);

	obj_mgr_debug("Physically deleting vdev %d", vdev_id);

	if (vdev->obj_state != WLAN_OBJ_STATE_LOGICALLY_DELETED) {
		obj_mgr_err("VDEV object delete is not invoked vdevid:%d objstate:%d",
			    wlan_vdev_get_id(vdev), vdev->obj_state);
		WLAN_OBJMGR_BUG(0);
	}

	/* Invoke registered destroy handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->vdev_destroy_handler[id];
		arg = g_umac_glb_obj->vdev_destroy_handler_arg[id];
		if (handler &&
		    (vdev->obj_status[id] == QDF_STATUS_SUCCESS ||
		     vdev->obj_status[id] == QDF_STATUS_COMP_ASYNC))
			vdev->obj_status[id] = handler(vdev, arg);
		else
			vdev->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_vdev_object_status(vdev);

	if (obj_status == QDF_STATUS_E_FAILURE) {
		obj_mgr_err("VDEV object deletion failed: vdev-id: %d",
				vdev_id);
		/* Ideally should not happen */
		/* This leads to memleak ??? how to handle */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	/* Deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		vdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	}

	/* Free VDEV object */
	return wlan_objmgr_vdev_obj_free(vdev);
}

QDF_STATUS wlan_objmgr_vdev_obj_delete(struct wlan_objmgr_vdev *vdev)
{
	uint8_t print_idx;

	if (!vdev) {
		obj_mgr_err("vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	obj_mgr_info("Logically deleting vdev %d", vdev->vdev_objmgr.vdev_id);

	print_idx = qdf_get_pidx();
	wlan_objmgr_print_ref_ids(vdev->vdev_objmgr.ref_id_dbg,
				  QDF_TRACE_LEVEL_DEBUG);
	/*
	 * Update VDEV object state to LOGICALLY DELETED
	 * It prevents further access of this object
	 */
	wlan_vdev_obj_lock(vdev);
	vdev->obj_state = WLAN_OBJ_STATE_LOGICALLY_DELETED;
	wlan_vdev_obj_unlock(vdev);
	wlan_objmgr_notify_log_delete(vdev, WLAN_VDEV_OP);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_vdev_obj_delete);

/**
 ** APIs to attach/detach component objects
 */
QDF_STATUS wlan_objmgr_vdev_component_obj_attach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status)
{
	wlan_objmgr_vdev_status_handler stat_handler;
	void *arg;
	uint8_t i;
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_vdev_obj_lock(vdev);
	/* If there is a valid entry, return failure */
	if (vdev->vdev_comp_priv_obj[id]) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Save component's pointer and status */
	vdev->vdev_comp_priv_obj[id] = comp_priv_obj;
	vdev->obj_status[id] = status;
	wlan_vdev_obj_unlock(vdev);
	if (vdev->obj_state != WLAN_OBJ_STATE_PARTIALLY_CREATED)
		return QDF_STATUS_SUCCESS;
	/*
	 * If VDEV object status is partially created means, this API is
	 * invoked with differnt context, this block should be executed for
	 * async components only
	 */
	/* Derive status */
	obj_status = wlan_objmgr_vdev_object_status(vdev);
	/* STATUS_SUCCESS means, object is CREATED */
	if (obj_status == QDF_STATUS_SUCCESS)
		vdev->obj_state = WLAN_OBJ_STATE_CREATED;
	/*
	 * update state as CREATION failed, caller has to delete the
	 * VDEV object
	 */
	else if (obj_status == QDF_STATUS_E_FAILURE)
		vdev->obj_state = WLAN_OBJ_STATE_CREATION_FAILED;
	/* Notify components about the CREATION success/failure */
	if ((obj_status == QDF_STATUS_SUCCESS) ||
	    (obj_status == QDF_STATUS_E_FAILURE)) {
		for (i = 0; i < WLAN_UMAC_MAX_COMPONENTS; i++) {
			stat_handler = g_umac_glb_obj->vdev_status_handler[i];
			arg = g_umac_glb_obj->vdev_status_handler_arg[i];
			if (stat_handler)
				stat_handler(vdev, arg, obj_status);
		}
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_vdev_component_obj_attach);

QDF_STATUS wlan_objmgr_vdev_component_obj_detach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj)
{
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_vdev_obj_lock(vdev);
	/* If there is a valid entry, return failure */
	if (vdev->vdev_comp_priv_obj[id] != comp_priv_obj) {
		vdev->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset pointers to NULL, update the status*/
	vdev->vdev_comp_priv_obj[id] = NULL;
	vdev->obj_status[id] = QDF_STATUS_SUCCESS;
	wlan_vdev_obj_unlock(vdev);

	/**
	 *If VDEV object status is partially destroyed means, this API is
	 * invoked with differnt context, this block should be executed for
	 * async components only
	 */
	if ((vdev->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED) ||
	    (vdev->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)) {
		/* Derive object status */
		obj_status = wlan_objmgr_vdev_object_status(vdev);
		if (obj_status == QDF_STATUS_SUCCESS) {
			/*
			 * Update the status as Deleted, if full object
			 * deletion is in progress
			 */
			if (vdev->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				vdev->obj_state = WLAN_OBJ_STATE_DELETED;
			/*
			 * Move to creation state, since this component
			 * deletion alone requested
			 */
			else if (vdev->obj_state ==
					WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				vdev->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Object status is failure */
		} else if (obj_status == QDF_STATUS_E_FAILURE) {
			/*
			 * Update the status as Deletion failed, if full object
			 * deletion is in progress
			 */
			if (vdev->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				vdev->obj_state =
					WLAN_OBJ_STATE_DELETION_FAILED;
			/* Move to creation state, since this component
			deletion alone requested (do not block other
			components) */
			else if (vdev->obj_state ==
					WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				vdev->obj_state = WLAN_OBJ_STATE_CREATED;
		}
		/* Delete vdev object */
		if ((obj_status == QDF_STATUS_SUCCESS)  &&
		    (vdev->obj_state == WLAN_OBJ_STATE_DELETED)) {
			/* Free VDEV object */
			return wlan_objmgr_vdev_obj_free(vdev);
		}
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_vdev_component_obj_detach);

/**
 ** APIs to operations on vdev objects
 */
QDF_STATUS wlan_objmgr_iterate_peerobj_list(
		struct wlan_objmgr_vdev *vdev,
		wlan_objmgr_vdev_op_handler handler,
		void *arg, wlan_objmgr_ref_dbgid dbg_id)
{
	qdf_list_t *peer_list = NULL;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_objmgr_peer *peer_next = NULL;
	uint8_t vdev_id;

	if (!vdev) {
		obj_mgr_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_vdev_obj_lock(vdev);
	vdev_id = wlan_vdev_get_id(vdev);

	if (vdev->obj_state != WLAN_OBJ_STATE_CREATED) {
		wlan_vdev_obj_unlock(vdev);
		obj_mgr_err("VDEV is not in create state(:%d): vdev-id:%d",
				vdev_id, vdev->obj_state);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_vdev_get_ref(vdev, dbg_id);
	peer_list = &vdev->vdev_objmgr.wlan_peer_list;
	if (peer_list) {
		/* Iterate through VDEV's peer list */
		peer = wlan_vdev_peer_list_peek_head(peer_list);
		while (peer) {
			peer_next = wlan_peer_get_next_peer_of_vdev(peer_list,
							       peer);
			if (wlan_objmgr_peer_try_get_ref(peer, dbg_id) ==
					QDF_STATUS_SUCCESS) {
				/* Invoke handler for operation */
				handler(vdev, (void *)peer, arg);
				wlan_objmgr_peer_release_ref(peer, dbg_id);
			}
			peer = peer_next;
		}
	}
	wlan_objmgr_vdev_release_ref(vdev, dbg_id);
	wlan_vdev_obj_unlock(vdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_trigger_vdev_comp_priv_object_creation(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_vdev_create_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* Component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_vdev_obj_lock(vdev);
	/*
	 * If component object is already created, delete old
	 * component object, then invoke creation
	 */
	if (vdev->vdev_comp_priv_obj[id]) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_vdev_obj_unlock(vdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->vdev_create_handler[id];
	arg = g_umac_glb_obj->vdev_create_handler_arg[id];
	if (handler)
		vdev->obj_status[id] = handler(vdev, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (vdev->obj_state == WLAN_OBJ_STATE_CREATED) {
		/* Derive object status */
		obj_status = wlan_objmgr_vdev_object_status(vdev);
		/* Move PDEV object state to Partially created state */
		if (obj_status == QDF_STATUS_COMP_ASYNC) {
			/*TODO atomic */
			vdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
		}
	}
	return obj_status;
}

QDF_STATUS wlan_objmgr_trigger_vdev_comp_priv_object_deletion(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_vdev_destroy_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_vdev_obj_lock(vdev);
	/* Component object was never created, invalid operation */
	if (!vdev->vdev_comp_priv_obj[id]) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_vdev_obj_unlock(vdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->vdev_destroy_handler[id];
	arg = g_umac_glb_obj->vdev_destroy_handler_arg[id];
	if (handler)
		vdev->obj_status[id] = handler(vdev, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (vdev->obj_state == WLAN_OBJ_STATE_CREATED) {
		obj_status = wlan_objmgr_vdev_object_status(vdev);
		/* move object state to DEL progress */
		if (obj_status == QDF_STATUS_COMP_ASYNC)
			vdev->obj_state = WLAN_OBJ_STATE_COMP_DEL_PROGRESS;
	}
	return obj_status;
}



static void wlan_obj_vdev_peerlist_add_tail(qdf_list_t *obj_list,
	struct wlan_objmgr_peer *obj)
{
	qdf_list_insert_back(obj_list, &obj->vdev_peer);
}

static QDF_STATUS wlan_obj_vdev_peerlist_remove_peer(qdf_list_t *obj_list,
					struct wlan_objmgr_peer *peer)
{
	qdf_list_node_t *vdev_node = NULL;

	if (!peer)
		return QDF_STATUS_E_FAILURE;
	/* get vdev list node element */
	vdev_node = &peer->vdev_peer;
	/* list is empty, return failure */
	if (qdf_list_remove_node(obj_list, vdev_node) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_vdev_peer_attach(struct wlan_objmgr_vdev *vdev,
						struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_vdev_objmgr *objmgr = &vdev->vdev_objmgr;
	struct wlan_objmgr_pdev *pdev;
	enum QDF_OPMODE opmode;

	wlan_vdev_obj_lock(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	/* If Max VDEV peer count exceeds, return failure */
	if (peer->peer_mlme.peer_type != WLAN_PEER_STA_TEMP) {
		if (objmgr->wlan_peer_count >= objmgr->max_peer_count) {
			wlan_vdev_obj_unlock(vdev);
			return QDF_STATUS_E_FAILURE;
		}
	}
	wlan_vdev_obj_unlock(vdev);

	/* If Max PDEV peer count exceeds, return failure */
	wlan_pdev_obj_lock(pdev);
	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP) {
		if (wlan_pdev_get_temp_peer_count(pdev) >=
			WLAN_MAX_PDEV_TEMP_PEERS) {
			wlan_pdev_obj_unlock(pdev);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (wlan_pdev_get_peer_count(pdev) >=
			wlan_pdev_get_max_peer_count(pdev)) {
			wlan_pdev_obj_unlock(pdev);
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP)
		wlan_pdev_incr_temp_peer_count(wlan_vdev_get_pdev(vdev));
	else
		wlan_pdev_incr_peer_count(wlan_vdev_get_pdev(vdev));
	wlan_pdev_obj_unlock(pdev);

	wlan_vdev_obj_lock(vdev);
	/* Add peer to vdev's peer list */
	wlan_obj_vdev_peerlist_add_tail(&objmgr->wlan_peer_list, peer);
	objmgr->wlan_peer_count++;

	if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer),
			 wlan_vdev_mlme_get_macaddr(vdev)) ==
				QDF_STATUS_SUCCESS) {
		/*
		 * if peer mac address and vdev mac address match, set
		 * this peer as self peer
		 */
		wlan_vdev_set_selfpeer(vdev, peer);
		opmode = wlan_vdev_mlme_get_opmode(vdev);
		/* For AP mode, self peer and BSS peer are same */
		if ((opmode == QDF_SAP_MODE) || (opmode == QDF_P2P_GO_MODE))
			wlan_vdev_set_bsspeer(vdev, peer);
	}
	/* set BSS peer for sta */
	if ((wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE ||
	     wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_CLIENT_MODE) &&
	    (wlan_peer_get_peer_type(peer) == WLAN_PEER_AP ||
	     wlan_peer_get_peer_type(peer) == WLAN_PEER_P2P_GO))
		wlan_vdev_set_bsspeer(vdev, peer);

	/* Increment vdev ref count to make sure it won't be destroyed before */
	wlan_objmgr_vdev_get_ref(vdev, WLAN_OBJMGR_ID);
	wlan_vdev_obj_unlock(vdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_vdev_peer_detach(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_vdev_objmgr *objmgr = &vdev->vdev_objmgr;
	struct wlan_objmgr_pdev *pdev;

	wlan_vdev_obj_lock(vdev);
	/* if peer count is 0, return failure */
	if (objmgr->wlan_peer_count == 0) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}

	if ((wlan_peer_get_peer_type(peer) == WLAN_PEER_AP) ||
	    (wlan_peer_get_peer_type(peer) == WLAN_PEER_P2P_GO)) {
		if (wlan_vdev_get_selfpeer(vdev) == peer) {
			/*
			 * There might be instances where new node is created
			 * before deleting existing node, in which case selfpeer
			 * will be pointing to the new node. So set selfpeer to
			 * NULL only if vdev->vdev_objmgr.self_peer is pointing
			 * to the peer processed for deletion
			 */
			wlan_vdev_set_selfpeer(vdev, NULL);
		}

		if (wlan_vdev_get_bsspeer(vdev) == peer) {
			/*
			 * There might be instances where new node is created
			 * before deleting existing node, in which case bsspeer
			 * in vdev will be pointing to the new node. So set
			 * bsspeer to NULL only if vdev->vdev_objmgr.bss_peer is
			 * pointing to the peer processed for deletion
			 */
			wlan_vdev_set_bsspeer(vdev, NULL);
		}
	}

	/* remove peer from vdev's peer list */
	if (wlan_obj_vdev_peerlist_remove_peer(&objmgr->wlan_peer_list, peer)
				== QDF_STATUS_E_FAILURE) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* decrement peer count */
	objmgr->wlan_peer_count--;
	/* decrement pdev peer count */
	pdev = wlan_vdev_get_pdev(vdev);
	wlan_vdev_obj_unlock(vdev);

	wlan_pdev_obj_lock(pdev);
	if (peer->peer_mlme.peer_type == WLAN_PEER_STA_TEMP)
		wlan_pdev_decr_temp_peer_count(pdev);
	else
		wlan_pdev_decr_peer_count(pdev);
	wlan_pdev_obj_unlock(pdev);

	/* decrement vdev ref count after peer released its reference */
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OBJMGR_ID);
	return QDF_STATUS_SUCCESS;
}

void *wlan_objmgr_vdev_get_comp_private_obj(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id)
{
	void *comp_priv_obj;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS) {
		QDF_BUG(0);
		return NULL;
	}

	if (!vdev) {
		QDF_BUG(0);
		return NULL;
	}

	comp_priv_obj = vdev->vdev_comp_priv_obj[id];

	return comp_priv_obj;
}
qdf_export_symbol(wlan_objmgr_vdev_get_comp_private_obj);

void wlan_objmgr_vdev_get_ref(struct wlan_objmgr_vdev *vdev,
						wlan_objmgr_ref_dbgid id)
{
	if (!vdev) {
		obj_mgr_err("vdev obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return;
	}
	/* Increment ref count */
	qdf_atomic_inc(&vdev->vdev_objmgr.ref_cnt);
	qdf_atomic_inc(&vdev->vdev_objmgr.ref_id_dbg[id]);

	return;
}
qdf_export_symbol(wlan_objmgr_vdev_get_ref);

QDF_STATUS wlan_objmgr_vdev_try_get_ref(struct wlan_objmgr_vdev *vdev,
						wlan_objmgr_ref_dbgid id)
{
	uint8_t vdev_id;

	if (!vdev) {
		obj_mgr_err("vdev obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_vdev_obj_lock(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	if (vdev->obj_state != WLAN_OBJ_STATE_CREATED) {
		wlan_vdev_obj_unlock(vdev);
		if (vdev->vdev_objmgr.print_cnt++ <=
				WLAN_OBJMGR_RATELIMIT_THRESH)
			obj_mgr_err(
			"[Ref id: %d] vdev(%d) is not in Created state(%d)",
				id, vdev_id, vdev->obj_state);

		return QDF_STATUS_E_RESOURCES;
	}

	/* Increment ref count */
	wlan_objmgr_vdev_get_ref(vdev, id);
	wlan_vdev_obj_unlock(vdev);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_objmgr_vdev_try_get_ref);

void wlan_objmgr_vdev_release_ref(struct wlan_objmgr_vdev *vdev,
						wlan_objmgr_ref_dbgid id)
{
	uint8_t vdev_id;

	if (!vdev) {
		obj_mgr_err("vdev obj is NULL for id:%d", id);
		QDF_ASSERT(0);
		return;
	}

	vdev_id = wlan_vdev_get_id(vdev);

	if (!qdf_atomic_read(&vdev->vdev_objmgr.ref_id_dbg[id])) {
		obj_mgr_err("vdev (id:%d)ref cnt was not taken by %d",
				vdev_id, id);
		wlan_objmgr_print_ref_ids(vdev->vdev_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_FATAL);
		WLAN_OBJMGR_BUG(0);
	}

	if (!qdf_atomic_read(&vdev->vdev_objmgr.ref_cnt)) {
		obj_mgr_err("vdev ref cnt is 0");
		WLAN_OBJMGR_BUG(0);
		return;
	}
	qdf_atomic_dec(&vdev->vdev_objmgr.ref_id_dbg[id]);

	/* Decrement ref count, free vdev, if ref count == 0 */
	if (qdf_atomic_dec_and_test(&vdev->vdev_objmgr.ref_cnt))
		wlan_objmgr_vdev_obj_destroy(vdev);

	return;
}
qdf_export_symbol(wlan_objmgr_vdev_release_ref);

struct wlan_objmgr_vdev *wlan_pdev_vdev_list_peek_active_head(
			struct wlan_objmgr_pdev *pdev,
			qdf_list_t *vdev_list, wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	qdf_list_node_t *node = NULL;
	qdf_list_node_t *prev_node = NULL;

	wlan_pdev_obj_lock(pdev);

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

struct wlan_objmgr_vdev *wlan_vdev_get_next_active_vdev_of_pdev(
			struct wlan_objmgr_pdev *pdev,
			qdf_list_t *vdev_list,
			struct wlan_objmgr_vdev *vdev,
			wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev_next;
	qdf_list_node_t *node = &vdev->vdev_node;
	qdf_list_node_t *prev_node = NULL;

	if (!node)
		return NULL;

	wlan_pdev_obj_lock(pdev);
	prev_node = node;
	while (qdf_list_peek_next(vdev_list, prev_node, &node) ==
							QDF_STATUS_SUCCESS) {
		vdev_next = qdf_container_of(node, struct wlan_objmgr_vdev,
								vdev_node);
		if (wlan_objmgr_vdev_try_get_ref(vdev_next, dbg_id) ==
						QDF_STATUS_SUCCESS) {
			wlan_pdev_obj_unlock(pdev);
			return vdev_next;
		}

		prev_node = node;
	}
	wlan_pdev_obj_unlock(pdev);

	return NULL;
}

