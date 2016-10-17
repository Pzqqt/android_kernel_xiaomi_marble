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
			if (vdev->vdev_comp_obj[id] == NULL) {
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

	if (pdev == NULL) {
		qdf_print("%s: pdev is NULL\n", __func__);
		return NULL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	/* PSOC is NULL */
	if (psoc == NULL) {
		qdf_print("%s: psoc is NULL\n", __func__);
		return NULL;
	}
	/* Allocate vdev object memory */
	vdev = qdf_mem_malloc(sizeof(*vdev));
	if (vdev == NULL) {
		qdf_print("%s: Memory allocation failure\n", __func__);
		return NULL;
	}
	/* Attach VDEV to PSOC VDEV's list */
	if (wlan_objmgr_psoc_vdev_attach(psoc, vdev) !=
				QDF_STATUS_SUCCESS) {
		qdf_mem_free(vdev);
		qdf_print("%s: psoc vdev attach failed\n", __func__);
		return NULL;
	}
	/* Store pdev in vdev */
	wlan_vdev_set_pdev(vdev, pdev);
	/* Attach vdev to PDEV */
	if (wlan_objmgr_pdev_vdev_attach(pdev, vdev) !=
				QDF_STATUS_SUCCESS) {
		qdf_print("%s: pdev vdev attach failed\n", __func__);
		wlan_objmgr_psoc_vdev_detach(psoc, vdev);
		qdf_mem_free(vdev);
		return NULL;
	}
	/* Initialize spinlock */
	qdf_spinlock_create(&vdev->vdev_lock);
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
	/* Initialize max peer count based on opmode type */
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		vdev->vdev_objmgr.max_peer_count = WLAN_UMAC_MAX_STA_PEERS;
	else
		vdev->vdev_objmgr.max_peer_count = WLAN_UMAC_MAX_AP_PEERS;

	/* Initialize peer list */
	qdf_list_create(&vdev->vdev_objmgr.wlan_peer_list,
			vdev->vdev_objmgr.max_peer_count);
	/* TODO init other parameters */

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->vdev_create_handler[id];
		arg = g_umac_glb_obj->vdev_create_handler_arg[id];
		if (handler != NULL)
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
			if (stat_handler != NULL) {
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
		qdf_print("%s:VDEV component objects creation failed\n",
			  __func__);
		return NULL;
	}
	return vdev;
}

QDF_STATUS wlan_objmgr_vdev_obj_delete(struct wlan_objmgr_vdev *vdev)
{
	uint8_t id;
	wlan_objmgr_vdev_delete_handler handler;
	QDF_STATUS obj_status;
	void *arg;
	struct wlan_objmgr_pdev *pdev;

	if (vdev == NULL) {
		qdf_print("%s:vdev is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* if PDEV is NULL, return */
	pdev = wlan_vdev_get_pdev(vdev);
	if (pdev == NULL) {
		qdf_print("%s:pdev is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->vdev_delete_handler[id];
		arg = g_umac_glb_obj->vdev_delete_handler_arg[id];
		if (handler != NULL)
			vdev->obj_status[id] = handler(vdev, arg);
		else
			vdev->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive object status */
	obj_status = wlan_objmgr_vdev_object_status(vdev);

	if (obj_status == QDF_STATUS_E_FAILURE) {
		qdf_print("%s: VDEV object deletion failed\n", __func__);
		/* Ideally should not happen */
		/* This leads to memleak ??? how to handle */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	/* Deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		vdev->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	} else {
		/* Detach VDEV from PDEV VDEV's list */
		if (wlan_objmgr_pdev_vdev_detach(pdev, vdev) ==
						QDF_STATUS_E_FAILURE)
			return QDF_STATUS_E_FAILURE;

		/* Detach VDEV from PSOC VDEV's list */
		if (wlan_objmgr_psoc_vdev_detach(
				pdev->pdev_objmgr.wlan_psoc, vdev) ==
						QDF_STATUS_E_FAILURE)
			return QDF_STATUS_E_FAILURE;

		/* de-init lock */
		qdf_spinlock_destroy(&vdev->vdev_lock);
		/* Free the memory */
		qdf_mem_free(vdev);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 ** APIs to attach/detach component objects
 */
QDF_STATUS wlan_objmgr_vdev_component_obj_attach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_objptr,
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
	if (vdev->vdev_comp_obj[id] != NULL) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Save component's pointer and status */
	vdev->vdev_comp_obj[id] = comp_objptr;
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
			if (stat_handler != NULL)
				stat_handler(vdev, arg, obj_status);
		}
	}
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_vdev_component_obj_attach);

QDF_STATUS wlan_objmgr_vdev_component_obj_detach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_objptr)
{
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;
	wlan_vdev_obj_lock(vdev);
	/* If there is a valid entry, return failure */
	if (vdev->vdev_comp_obj[id] != comp_objptr) {
		vdev->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset pointers to NULL, update the status*/
	vdev->vdev_comp_obj[id] = NULL;
	vdev->obj_status[id] = QDF_STATUS_SUCCESS;
	wlan_vdev_obj_unlock(vdev);

	/**
	 *If VDEV object status is partially deleted means, this API is
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
			/* Detach vdev object from psoc */
			if (wlan_objmgr_pdev_vdev_detach(
					wlan_vdev_get_pdev(vdev), vdev)
						== QDF_STATUS_E_FAILURE)
				return QDF_STATUS_E_FAILURE;
			/* Detach vdev object from psoc */
			if (wlan_objmgr_psoc_vdev_detach(
				wlan_vdev_get_psoc(vdev), vdev) ==
						QDF_STATUS_E_FAILURE)
				return QDF_STATUS_E_FAILURE;

			/* Destroy spinlock */
			qdf_spinlock_destroy(&vdev->vdev_lock);
			/* Free VDEV memory */
			qdf_mem_free(vdev);
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 ** APIs to operations on vdev objects
 */
QDF_STATUS wlan_objmgr_iterate_peerobj_list(
		struct wlan_objmgr_vdev *vdev,
		wlan_objmgr_vdev_op_handler handler,
		void *arg)
{
	qdf_list_t *peer_list = NULL;
	struct wlan_objmgr_peer *peer = NULL;
	struct wlan_objmgr_peer *prev_peer = NULL;

	if (vdev == NULL) {
		qdf_print("%s: VDEV NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
		/* TODO increment ref count */
	wlan_vdev_obj_lock(vdev);
	peer_list = &vdev->vdev_objmgr.wlan_peer_list;
	if (peer_list != NULL) {
		/* Iterate through VDEV's peer list */
		peer = wlan_vdev_peer_list_peek_head(peer_list);
		while (peer != NULL) {
			wlan_objmgr_peer_ref_peer(peer);
			/* Invoke handler for operation */
			handler(vdev, (void *)peer, arg);
			prev_peer = peer;
			peer = wlan_peer_get_next_peer_of_vdev(peer_list,
							       prev_peer);
			wlan_objmgr_peer_unref_peer(prev_peer);
		}
	}
	wlan_vdev_obj_unlock(vdev);
		/* TODO decrement ref count */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_trigger_vdev_comp_object_creation(
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
	if (vdev->vdev_comp_obj[id] != NULL) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_vdev_obj_unlock(vdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->vdev_create_handler[id];
	arg = g_umac_glb_obj->vdev_create_handler_arg[id];
	if (handler != NULL)
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

QDF_STATUS wlan_objmgr_trigger_vdev_comp_object_deletion(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_vdev_delete_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_vdev_obj_lock(vdev);
	/* Component object was never created, invalid operation */
	if (vdev->vdev_comp_obj[id] == NULL) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_vdev_obj_unlock(vdev);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->vdev_delete_handler[id];
	arg = g_umac_glb_obj->vdev_delete_handler_arg[id];
	if (handler != NULL)
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

	if (peer == NULL)
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

	wlan_vdev_obj_lock(vdev);
	/* If Max peer count exceeds, return failure */
	if (objmgr->wlan_peer_count > objmgr->max_peer_count) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* Add peer to vdev's peer list */
	wlan_obj_vdev_peerlist_add_tail(&objmgr->wlan_peer_list, peer);
	objmgr->wlan_peer_count++;

	if (wlan_peer_get_peer_type(peer) == WLAN_PEER_AP) {
		if (WLAN_ADDR_EQ(wlan_peer_get_macaddr(peer),
				 wlan_vdev_mlme_get_macaddr(vdev)) ==
					QDF_STATUS_SUCCESS) {
			/*
			 * if peer mac address and vdev mac address match, set
			 * this peer as self peer
			 */
			wlan_vdev_set_selfpeer(vdev, peer);
			/* For AP mode, self peer and BSS peer are same */
			if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE)
				wlan_vdev_set_bsspeer(vdev, peer);
		}
		/* set BSS peer for sta */
		if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
			wlan_vdev_set_bsspeer(vdev, peer);
	}
	wlan_vdev_obj_unlock(vdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_vdev_peer_detach(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_vdev_objmgr *objmgr = &vdev->vdev_objmgr;

	wlan_vdev_obj_lock(vdev);
	/* if peer count is 0, return failure */
	if (objmgr->wlan_peer_count == 0) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* remove peer from vdev's peer list */
	if (wlan_obj_vdev_peerlist_remove_peer(&objmgr->wlan_peer_list, peer)
				== QDF_STATUS_E_FAILURE) {
		wlan_vdev_obj_unlock(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	/* decrement peer count */
	objmgr->wlan_peer_count--;
	wlan_vdev_obj_unlock(vdev);
	return QDF_STATUS_SUCCESS;
}

