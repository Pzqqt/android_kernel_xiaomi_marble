/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * Doc: wlan_cp_stats_om_handler.c
 *
 * This file provide definitions to APIs invoked on receiving common object
 * repective create/destroy event notifications, which further
 * (de)allocate cp specific objects and (de)attach to specific
 * common object
 */
#include "wlan_cp_stats_obj_mgr_handler.h"
#include "wlan_cp_stats_defs.h"
#include "wlan_cp_stats_ol_api.h"
#include <wlan_cp_stats_ucfg_api.h>
#include "wlan_cp_stats_utils_api.h"

QDF_STATUS
wlan_cp_stats_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	WLAN_DEV_TYPE dev_type;
	struct cp_stats_context *csc = NULL;
	struct psoc_cp_stats *psoc_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!psoc) {
		cp_stats_err("PSOC is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	csc = qdf_mem_malloc(sizeof(*csc));
	if (!csc) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	csc->psoc_obj = psoc;
	dev_type = wlan_objmgr_psoc_get_dev_type(csc->psoc_obj);
	if (dev_type == WLAN_DEV_INVALID) {
		cp_stats_err("Failed to init cp stats ctx, bad device type");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	} else if (WLAN_DEV_OL == dev_type) {
		csc->cp_stats_ctx_init = wlan_cp_stats_ctx_init_ol;
		csc->cp_stats_ctx_deinit = wlan_cp_stats_ctx_deinit_ol;
	}

	if (QDF_STATUS_SUCCESS != csc->cp_stats_ctx_init(csc)) {
		cp_stats_err("Failed to init global ctx call back handlers");
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	psoc_cs = qdf_mem_malloc(sizeof(*psoc_cs));
	if (!psoc_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	psoc_cs->psoc_obj = psoc;
	csc->psoc_cs = psoc_cs;
	if (csc->cp_stats_psoc_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_psoc_obj_init(psoc_cs)) {
			cp_stats_err("Failed to initialize psoc handlers");
			goto wlan_cp_stats_psoc_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_CP_STATS,
						       csc,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_psoc_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_psoc_obj_deinit && psoc_cs)
				csc->cp_stats_psoc_obj_deinit(psoc_cs);

			if (csc->psoc_cs) {
				qdf_mem_free(csc->psoc_cs);
				csc->psoc_cs = NULL;
			}

			if (csc->cp_stats_ctx_deinit)
				csc->cp_stats_ctx_deinit(csc);

			qdf_mem_free(csc);
			csc = NULL;
		}
		return status;
	}

	cp_stats_debug("cp stats context attach at psoc");
	return status;
}

QDF_STATUS
wlan_cp_stats_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is NULL");
		return QDF_STATUS_E_NOMEM;
	}
	csc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_CP_STATS);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_psoc_component_obj_detach(psoc,
					      WLAN_UMAC_COMP_CP_STATS, csc);
	if (csc->cp_stats_psoc_obj_deinit)
		csc->cp_stats_psoc_obj_deinit(csc->psoc_cs);
	qdf_mem_free(csc->psoc_cs);
	if (csc->cp_stats_ctx_deinit)
		csc->cp_stats_ctx_deinit(csc);
	qdf_mem_free(csc);

	cp_stats_debug("cp stats context dettached at psoc");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct pdev_cp_stats *pdev_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!pdev) {
		cp_stats_err("PDEV is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}

	pdev_cs = qdf_mem_malloc(sizeof(*pdev_cs));
	if (!pdev_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_pdev(pdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}
	pdev_cs->pdev_obj = pdev;
	if (csc->cp_stats_pdev_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_pdev_obj_init(pdev_cs)) {
			cp_stats_err("Failed to initialize pdev handlers");
			goto wlan_cp_stats_pdev_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
						       WLAN_UMAC_COMP_CP_STATS,
						       pdev_cs,
						       QDF_STATUS_SUCCESS);

	cp_stats_debug("pdev cp stats object attached");
wlan_cp_stats_pdev_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_pdev_obj_deinit)
				csc->cp_stats_pdev_obj_deinit(pdev_cs);
		}

		if (pdev_cs)
			qdf_mem_free(pdev_cs);
	}

	return status;
}

QDF_STATUS
wlan_cp_stats_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cp_stats *pdev_cs;
	struct cp_stats_context *csc;

	if (!pdev) {
		cp_stats_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev_cs = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						WLAN_UMAC_COMP_CP_STATS);
	if (!pdev_cs) {
		cp_stats_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_pdev(pdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_pdev_obj_deinit)
		csc->cp_stats_pdev_obj_deinit(pdev_cs);

	wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_CP_STATS,
					      pdev_cs);

	qdf_mem_free(pdev_cs);
	cp_stats_debug("pdev cp stats object dettached");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct vdev_cp_stats *vdev_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!vdev) {
		cp_stats_err("vdev is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}

	vdev_cs = qdf_mem_malloc(sizeof(*vdev_cs));
	if (!vdev_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_vdev(vdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}
	vdev_cs->vdev_obj = vdev;
	if (csc->cp_stats_vdev_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_vdev_obj_init(vdev_cs)) {
			cp_stats_err("Failed to initialize vdev handlers");
			goto wlan_cp_stats_vdev_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_CP_STATS,
						       vdev_cs,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_vdev_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_vdev_obj_deinit)
				csc->cp_stats_vdev_obj_deinit(vdev_cs);
		}

		if (vdev_cs)
			qdf_mem_free(vdev_cs);
	}

	cp_stats_debug("vdev cp stats object attach");
	return status;
}

QDF_STATUS
wlan_cp_stats_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct vdev_cp_stats *vdev_cs;
	struct cp_stats_context *csc;

	if (!vdev) {
		cp_stats_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev_cs = wlan_objmgr_vdev_get_comp_private_obj(vdev,
						WLAN_UMAC_COMP_CP_STATS);
	if (!vdev_cs) {
		cp_stats_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_vdev(vdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_vdev_obj_deinit)
		csc->cp_stats_vdev_obj_deinit(vdev_cs);

	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_CP_STATS,
					      vdev_cs);

	qdf_mem_free(vdev_cs);
	cp_stats_debug("vdev cp stats object dettach");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct peer_cp_stats *peer_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!peer) {
		cp_stats_err("peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}

	peer_cs = qdf_mem_malloc(sizeof(*peer_cs));
	if (!peer_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_peer(peer);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}
	peer_cs->peer_obj = peer;
	if (csc->cp_stats_peer_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_peer_obj_init(peer_cs)) {
			cp_stats_err("Failed to initialize peer handlers");
			goto wlan_cp_stats_peer_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_peer_component_obj_attach(peer,
						       WLAN_UMAC_COMP_CP_STATS,
						       peer_cs,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_peer_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_peer_obj_deinit)
				csc->cp_stats_peer_obj_deinit(peer_cs);
		}

		if (peer_cs)
			qdf_mem_free(peer_cs);
	}

	cp_stats_debug("peer cp stats object attach");
	return status;
}

QDF_STATUS
wlan_cp_stats_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cp_stats *peer_cs;
	struct cp_stats_context *csc;

	if (!peer) {
		cp_stats_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	peer_cs = wlan_objmgr_peer_get_comp_private_obj(peer,
						WLAN_UMAC_COMP_CP_STATS);
	if (!peer_cs) {
		cp_stats_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_peer(peer);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_peer_obj_deinit)
		csc->cp_stats_peer_obj_deinit(peer_cs);

	wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_CP_STATS,
					      peer_cs);

	qdf_mem_free(peer_cs);
	cp_stats_debug("peer cp stats object dettached");
	return QDF_STATUS_SUCCESS;
}
