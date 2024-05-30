/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_objmgr.c
 * This file defines the APIs of TWT component.
 */
#include "wlan_twt_common.h"
#include "wlan_twt_priv.h"
#include "wlan_twt_objmgr_handler.h"
#include "wlan_objmgr_peer_obj.h"
#include "include/wlan_mlme_cmn.h"

QDF_STATUS
wlan_twt_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = qdf_mem_malloc(sizeof(*twt_psoc_obj));
	if (!twt_psoc_obj)
		return QDF_STATUS_E_NOMEM;

	twt_psoc_obj->enable_context.context = NULL;
	twt_psoc_obj->disable_context.context = NULL;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_TWT,
						       (void *)twt_psoc_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(twt_psoc_obj);
		twt_err("Failed to attach twt psoc priv object");
		return status;
	}

	twt_debug("twt psoc priv obj attach successful");
	return status;
}

QDF_STATUS
wlan_twt_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						WLAN_UMAC_COMP_TWT);
	if (!twt_psoc_obj) {
		twt_err("Failed to get twt obj in psoc");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_TWT,
						       twt_psoc_obj);

	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to detach twt psoc priv object");

	qdf_mem_free(twt_psoc_obj);

	return status;
}

QDF_STATUS
wlan_twt_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	QDF_STATUS status;
	struct twt_vdev_priv_obj *twt_vdev_obj;

	twt_vdev_obj = qdf_mem_malloc(sizeof(*twt_vdev_obj));
	if (!twt_vdev_obj)
		return QDF_STATUS_E_NOMEM;

	twt_vdev_obj->twt_wait_for_notify = false;

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_TWT,
						       twt_vdev_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(twt_vdev_obj);
		twt_err("Failed to attach twt vdev priv object");
		return status;
	}

	twt_debug("twt vdev priv obj attach successful");

	status = mlme_twt_vdev_create_notification(vdev);

	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("vdev create notification failed");
		return status;
	}

	return status;
}

QDF_STATUS
wlan_twt_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	QDF_STATUS status;
	struct twt_vdev_priv_obj *twt_vdev_obj;

	status = mlme_twt_vdev_destroy_notification(vdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("vdev destroy notification failed");
		return status;
	}

	twt_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							    WLAN_UMAC_COMP_TWT);
	if (!twt_vdev_obj) {
		twt_err("Failed to get twt obj in vdev");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
						       WLAN_UMAC_COMP_TWT,
						       twt_vdev_obj);

	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to detach twt vdev priv object");

	qdf_mem_free(twt_vdev_obj);

	return status;
}

QDF_STATUS
wlan_twt_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct twt_peer_priv_obj *twt_peer_obj = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!peer) {
		twt_err("peer is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	twt_peer_obj = qdf_mem_malloc(sizeof(*twt_peer_obj));
	if (!twt_peer_obj)
		return QDF_STATUS_E_NOMEM;

	qdf_mutex_create(&twt_peer_obj->twt_peer_lock);

	status = wlan_objmgr_peer_component_obj_attach(peer,
						       WLAN_UMAC_COMP_TWT,
						       twt_peer_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mutex_destroy(&twt_peer_obj->twt_peer_lock);
		qdf_mem_free(twt_peer_obj);
		twt_err("peer twt object attach failed");
		return QDF_STATUS_E_FAILURE;
	}

	twt_debug("twt peer priv obj attach successful");
	return status;
}

QDF_STATUS
wlan_twt_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct twt_peer_priv_obj *twt_peer_obj;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!peer) {
		twt_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	twt_peer_obj = wlan_objmgr_peer_get_comp_private_obj(peer,
						WLAN_UMAC_COMP_TWT);
	if (!twt_peer_obj) {
		twt_err("twt_peer_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mutex_destroy(&twt_peer_obj->twt_peer_lock);

	status = wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_TWT,
						       twt_peer_obj);
	if (QDF_IS_STATUS_ERROR(status))
		twt_warn("Failed to detach twt peer priv object");

	qdf_mem_free(twt_peer_obj);
	twt_debug("peer twt object detached");
	return QDF_STATUS_SUCCESS;
}

