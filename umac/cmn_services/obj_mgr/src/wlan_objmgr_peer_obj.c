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
 * DOC: Public APIs to perform operations on Peer object
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
#include <wlan_objmgr_vdev_obj_i.h>
#include <qdf_mem.h>


/**
 ** APIs to Create/Delete Peer object APIs
 */
static QDF_STATUS wlan_objmgr_peer_object_status(
		struct wlan_objmgr_peer *peer)
{
	uint8_t id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	wlan_peer_obj_lock(peer);
	/* Iterate through all components to derive the object status */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		/* If component disabled, Ignore */
		if (peer->obj_status[id] == QDF_STATUS_COMP_DISABLED)
			continue;
		/* If component operates in Async, status is Partially created,
			break */
		else if (peer->obj_status[id] == QDF_STATUS_COMP_ASYNC) {
			if (peer->peer_comp_priv_obj[id] == NULL) {
				status = QDF_STATUS_COMP_ASYNC;
				break;
			}
		/* If component failed to allocate its object, treat it as
			failure, complete object need to be cleaned up */
		} else if ((peer->obj_status[id] == QDF_STATUS_E_NOMEM) ||
			(peer->obj_status[id] == QDF_STATUS_E_FAILURE)) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}
	}
	wlan_peer_obj_unlock(peer);
	return status;
}

struct wlan_objmgr_peer *wlan_objmgr_peer_obj_create(
			struct wlan_objmgr_vdev *vdev,
			enum wlan_peer_type type,
			uint8_t *macaddr)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_psoc *psoc;
	wlan_objmgr_peer_create_handler handler;
	wlan_objmgr_peer_status_handler stat_handler;
	void *arg;
	QDF_STATUS obj_status;
	uint8_t id;

	if (vdev == NULL) {
		qdf_print("%s: VDEV is NULL\n", __func__);
		return NULL;
	}
	/* Get psoc, if psoc is NULL, return */
	psoc = wlan_vdev_get_psoc(vdev);
	if (psoc == NULL) {
		qdf_print("%s: PSOC is NULL\n", __func__);
		return NULL;
	}
	/* Allocate memory for peer object */
	peer = qdf_mem_malloc(sizeof(*peer));
	if (peer == NULL) {
		qdf_print("%s: Peer allocation failure\n", __func__);
		return NULL;
	}
	/* set vdev to peer */
	wlan_peer_set_vdev(peer, vdev);
	/* set peer type */
	wlan_peer_set_peer_type(peer, type);
	/* set mac address of peer */
	wlan_peer_set_macaddr(peer, macaddr);
	/* Attach peer to psoc, psoc maintains the node table for the device */
	if (wlan_objmgr_psoc_peer_attach(psoc, peer) !=
					QDF_STATUS_SUCCESS) {
		qdf_print("%s: Peer PSOC attach failure\n", __func__);
		qdf_mem_free(peer);
		return NULL;
	}
	/* Attach peer to vdev peer table */
	if (wlan_objmgr_vdev_peer_attach(vdev, peer) !=
					QDF_STATUS_SUCCESS) {
		qdf_print("%s: Peer VDEV attach failure\n", __func__);
		/* if attach fails, detach from psoc table before free */
		wlan_objmgr_psoc_peer_detach(psoc, peer);
		qdf_mem_free(peer);
		return NULL;
	}
	/* init spinlock */
	qdf_spinlock_create(&peer->peer_lock);
	/* Increment ref count for BSS peer, so that BSS peer deletes last*/
	if ((type == WLAN_PEER_STA) || (type == WLAN_PEER_STA_TEMP))
		wlan_objmgr_peer_ref_peer(wlan_vdev_get_bsspeer(vdev));
	/* TODO init other parameters */
	/* Invoke registered create handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->peer_create_handler[id];
		arg = g_umac_glb_obj->peer_create_handler_arg[id];
		if (handler != NULL)
			peer->obj_status[id] = handler(peer, arg);
		else
			peer->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* derive the object status */
	obj_status = wlan_objmgr_peer_object_status(peer);
	/* If SUCCESS, Object is created */
	if (obj_status == QDF_STATUS_SUCCESS) {
		peer->obj_state = WLAN_OBJ_STATE_CREATED;
		for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
			stat_handler = g_umac_glb_obj->peer_status_handler[id];
			arg = g_umac_glb_obj->peer_status_handler_arg[id];
			if (stat_handler != NULL)
				stat_handler(peer, arg,
					     QDF_STATUS_SUCCESS);
		}
	} else if (obj_status == QDF_STATUS_COMP_ASYNC) {
		/* If any component operates in different context, update it
		as partially created */
		peer->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
	} else if (obj_status == QDF_STATUS_E_FAILURE) {
		/* Clean up the peer */
		qdf_print("%s: Peer component object alloc failure\n",
			  __func__);
		wlan_objmgr_peer_obj_delete(peer);
		return NULL;
	}
	return peer;
}

QDF_STATUS wlan_objmgr_peer_obj_delete(struct wlan_objmgr_peer *peer)
{
	uint8_t id;
	wlan_objmgr_peer_delete_handler handler;
	QDF_STATUS obj_status;
	void *arg;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;

	if (peer == NULL) {
		qdf_print("%s: PEER is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* get VDEV from peer, if it is NULL, return */
	vdev = wlan_peer_get_vdev(peer);
	if (vdev == NULL) {
		qdf_print("%s: VDEV is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* get PSOC from VDEV, if it is NULL, return */
	psoc = wlan_vdev_get_psoc(vdev);
	if (psoc == NULL) {
		qdf_print("%s: PSOC is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Decrement ref count for BSS peer, so that BSS peer deletes last*/
	if ((wlan_peer_get_peer_type(peer) == WLAN_PEER_STA) ||
	    (wlan_peer_get_peer_type(peer) == WLAN_PEER_STA_TEMP))
		wlan_objmgr_peer_unref_peer(wlan_vdev_get_bsspeer(vdev));

	/* Invoke registered delete handlers */
	for (id = 0; id < WLAN_UMAC_MAX_COMPONENTS; id++) {
		handler = g_umac_glb_obj->peer_delete_handler[id];
		arg = g_umac_glb_obj->peer_delete_handler_arg[id];
		if (handler != NULL)
			peer->obj_status[id] = handler(peer, arg);
		else
			peer->obj_status[id] = QDF_STATUS_COMP_DISABLED;
	}
	/* Derive the object status */
	obj_status = wlan_objmgr_peer_object_status(peer);
	if (obj_status == QDF_STATUS_E_FAILURE) {
		/* If it status is failure, memory will not be freed */
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* few components deletion is in progress */
	if (obj_status == QDF_STATUS_COMP_ASYNC) {
		peer->obj_state = WLAN_OBJ_STATE_PARTIALLY_DELETED;
		return QDF_STATUS_COMP_ASYNC;
	} else {
		/* Detach peer from VDEV's peer list */
		if (wlan_objmgr_vdev_peer_detach(vdev, peer)
				== QDF_STATUS_E_FAILURE) {
			qdf_print("%s: Peer VDEV detach failure\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		/* Detach peer from PSOC's peer list */
		if (wlan_objmgr_psoc_peer_detach(psoc, peer)
					== QDF_STATUS_E_FAILURE) {
			qdf_print("%s: Peer PSOC detach failure\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		/* destroy spinlock */
		qdf_spinlock_destroy(&peer->peer_lock);
		/* Free memory */
		qdf_mem_free(peer);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 ** APIs to attach/detach component objects
 */
QDF_STATUS wlan_objmgr_peer_component_obj_attach(
		struct wlan_objmgr_peer *peer,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status)
{
	wlan_objmgr_peer_status_handler s_hler;
	void *arg;
	uint8_t i;
	QDF_STATUS obj_status;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_peer_obj_lock(peer);
	/* If there is a valid entry, return failure,
	valid object needs to be freed first */
	if (peer->peer_comp_priv_obj[id] != NULL) {
		wlan_peer_obj_unlock(peer);
		return QDF_STATUS_E_FAILURE;
	}
	/* Assign component object private pointer(can be NULL also), status */
	peer->peer_comp_priv_obj[id] = comp_priv_obj;
	peer->obj_status[id] = status;
	wlan_peer_obj_unlock(peer);

	if (peer->obj_state != WLAN_OBJ_STATE_PARTIALLY_CREATED)
		return QDF_STATUS_SUCCESS;

	/* If PEER object status is partially created means, this API is
	invoked with differnt context. this block should be executed for async
	components only */
	/* Derive status */
	obj_status = wlan_objmgr_peer_object_status(peer);
	/* STATUS_SUCCESS means, object is CREATED */
	if (obj_status == QDF_STATUS_SUCCESS)
		peer->obj_state = WLAN_OBJ_STATE_CREATED;
	/* update state as CREATION failed, caller has to delete the
	PEER object */
	else if (obj_status == QDF_STATUS_E_FAILURE)
		peer->obj_state = WLAN_OBJ_STATE_CREATION_FAILED;
	/* Notify components about the CREATION success/failure */
	if ((obj_status == QDF_STATUS_SUCCESS) ||
	    (obj_status == QDF_STATUS_E_FAILURE)) {
		/* nofity object status */
		for (i = 0; i < WLAN_UMAC_MAX_COMPONENTS; i++) {
			s_hler = g_umac_glb_obj->peer_status_handler[i];
			arg = g_umac_glb_obj->peer_status_handler_arg[i];
			if (s_hler != NULL)
				s_hler(peer, arg, obj_status);
		}
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_objmgr_peer_component_obj_detach(
		struct wlan_objmgr_peer *peer,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj)
{
	QDF_STATUS obj_status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	/* get VDEV from peer, if it is NULL, return */
	vdev = wlan_peer_get_vdev(peer);
	if (vdev == NULL) {
		qdf_print("%s: VDEV is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* get PSOC from VDEV, if it is NULL, return */
	psoc = wlan_vdev_get_psoc(vdev);
	if (psoc == NULL) {
		qdf_print("%s: PSOC is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_peer_obj_lock(peer);
	/* If there is a invalid entry, return failure */
	if (peer->peer_comp_priv_obj[id] != comp_priv_obj) {
		peer->obj_status[id] = QDF_STATUS_E_FAILURE;
		wlan_peer_obj_unlock(peer);
		return QDF_STATUS_E_FAILURE;
	}
	/* Reset the pointer to NULL */
	peer->peer_comp_priv_obj[id] = NULL;
	peer->obj_status[id] = QDF_STATUS_SUCCESS;
	wlan_peer_obj_unlock(peer);

	/* If PEER object status is partially deleted means, this API is
	invoked with differnt context, this block should be executed for async
	components only */
	if ((peer->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED) ||
	    (peer->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)) {
		/* Derive object status */
		obj_status = wlan_objmgr_peer_object_status(peer);
		if (obj_status == QDF_STATUS_SUCCESS) {
			/*Update the status as Deleted, if full object
				deletion is in progress */
			if (peer->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				peer->obj_state = WLAN_OBJ_STATE_DELETED;
			/* Move to creation state, since this component
			deletion alone requested */
			if (peer->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				peer->obj_state = WLAN_OBJ_STATE_CREATED;
		/* Object status is failure */
		} else if (obj_status == QDF_STATUS_E_FAILURE) {
			/*Update the status as Deletion failed, if full object
				deletion is in progress */
			if (peer->obj_state == WLAN_OBJ_STATE_PARTIALLY_DELETED)
				peer->obj_state =
					WLAN_OBJ_STATE_DELETION_FAILED;
			/* Move to creation state, since this component
			deletion alone requested (do not block other
			components) */
			if (peer->obj_state == WLAN_OBJ_STATE_COMP_DEL_PROGRESS)
				peer->obj_state = WLAN_OBJ_STATE_CREATED;
		}

			/* Delete peer object */
		if ((obj_status == QDF_STATUS_SUCCESS)  &&
		    (peer->obj_state == WLAN_OBJ_STATE_DELETED)) {
			/* delete peer from vdev's peer list */
			if (wlan_objmgr_vdev_peer_detach(vdev, peer) ==
						QDF_STATUS_E_FAILURE)
				return QDF_STATUS_E_FAILURE;

			/* delete peer from psoc's peer list */
			if (wlan_objmgr_psoc_peer_detach(psoc, peer) ==
						QDF_STATUS_E_FAILURE)
				return QDF_STATUS_E_FAILURE;
			/* Destroy spinlock */
			qdf_spinlock_destroy(&peer->peer_lock);
			/* free memory */
			qdf_mem_free(peer);
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_objmgr_trigger_peer_comp_priv_object_creation(
		struct wlan_objmgr_peer *peer,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_peer_create_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* Component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_peer_obj_lock(peer);
	/* If component object is already created, delete old
		component object, then invoke creation */
	if (peer->peer_comp_priv_obj[id] != NULL) {
		wlan_peer_obj_unlock(peer);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_peer_obj_unlock(peer);

	/* Invoke registered create handlers */
	handler = g_umac_glb_obj->peer_create_handler[id];
	arg = g_umac_glb_obj->peer_create_handler_arg[id];
	if (handler != NULL)
		peer->obj_status[id] = handler(peer, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (peer->obj_state == WLAN_OBJ_STATE_CREATED) {
		/* Derive object status */
		obj_status = wlan_objmgr_peer_object_status(peer);
		/* Move PDEV object state to Partially created state */
		if (obj_status == QDF_STATUS_COMP_ASYNC) {
			/*TODO atomic */
			peer->obj_state = WLAN_OBJ_STATE_PARTIALLY_CREATED;
		}
	}

	return obj_status;
}


QDF_STATUS wlan_objmgr_trigger_peer_comp_priv_object_deletion(
		struct wlan_objmgr_peer *peer,
		enum wlan_umac_comp_id id)
{
	wlan_objmgr_peer_delete_handler handler;
	void *arg;
	QDF_STATUS obj_status = QDF_STATUS_SUCCESS;

	/* component id is invalid */
	if (id >= WLAN_UMAC_MAX_COMPONENTS)
		return QDF_STATUS_MAXCOMP_FAIL;

	wlan_peer_obj_lock(peer);
	/* Component object was never created, invalid operation */
	if (peer->peer_comp_priv_obj[id] == NULL) {
		wlan_peer_obj_unlock(peer);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_peer_obj_unlock(peer);

	/* Invoke registered delete handlers */
	handler = g_umac_glb_obj->peer_delete_handler[id];
	arg = g_umac_glb_obj->peer_delete_handler_arg[id];
	if (handler != NULL)
		peer->obj_status[id] = handler(peer, arg);
	else
		return QDF_STATUS_E_FAILURE;

	/* If object status is created, then only handle this object status */
	if (peer->obj_state == WLAN_OBJ_STATE_CREATED) {
		obj_status = wlan_objmgr_peer_object_status(peer);
			/* move object state to DEL progress */
		if (obj_status == QDF_STATUS_COMP_ASYNC)
			peer->obj_state = WLAN_OBJ_STATE_COMP_DEL_PROGRESS;
	}
	return obj_status;
}
