/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_comp_handler.c
 *
 * This file maintain definitions to APIs which handle attach/detach of other
 * UMAC component specific cp stat object to cp stats
 *
 * Components calling configure API should alloc data structure while attaching
 * dealloc while detaching, where as address for which to be deallocated will
 * be passed back to component for data
 */
#include "wlan_cp_stats_comp_handler.h"
#include "wlan_cp_stats_defs.h"
#include <wlan_cp_stats_ucfg_api.h>
#include <wlan_cp_stats_utils_api.h>

static QDF_STATUS
wlan_cp_stats_psoc_comp_obj_config
(struct wlan_objmgr_psoc *psoc, enum wlan_cp_stats_comp_id comp_id,
	enum wlan_cp_stats_cfg_state cfg_state, void *data)
{
	struct psoc_cp_stats *psoc_cs;

	psoc_cs = wlan_cp_stats_get_psoc_stats_obj(psoc);
	if (!psoc_cs) {
		cp_stats_err("psoc cp stats object is null");
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_psoc_obj_lock(psoc_cs);
	if (cfg_state == WLAN_CP_STATS_OBJ_ATTACH) {
		if (psoc_cs->psoc_comp_priv_obj[comp_id]) {
			wlan_cp_stats_psoc_obj_unlock(psoc_cs);
			return QDF_STATUS_E_EXISTS;
		}
		psoc_cs->psoc_comp_priv_obj[comp_id] = data;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_DETACH) {
		if (psoc_cs->psoc_comp_priv_obj[comp_id] != data) {
			wlan_cp_stats_psoc_obj_unlock(psoc_cs);
			return QDF_STATUS_E_INVAL;
		}
		data = psoc_cs->psoc_comp_priv_obj[comp_id];
		psoc_cs->psoc_comp_priv_obj[comp_id] = NULL;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_INVALID) {
		cp_stats_err("Invalid cp stats cfg_state");
		wlan_cp_stats_psoc_obj_unlock(psoc_cs);
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_psoc_obj_unlock(psoc_cs);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
wlan_cp_stats_pdev_comp_obj_config
(struct wlan_objmgr_pdev *pdev, enum wlan_cp_stats_comp_id comp_id,
	enum wlan_cp_stats_cfg_state cfg_state, void *data)
{
	struct pdev_cp_stats *pdev_cs;

	pdev_cs = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (!pdev_cs) {
		cp_stats_err("pdev cp stats object is null");
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_pdev_obj_lock(pdev_cs);
	if (cfg_state == WLAN_CP_STATS_OBJ_ATTACH) {
		if (pdev_cs->pdev_comp_priv_obj[comp_id]) {
			wlan_cp_stats_pdev_obj_unlock(pdev_cs);
			return QDF_STATUS_E_EXISTS;
		}
		pdev_cs->pdev_comp_priv_obj[comp_id] = data;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_DETACH) {
		if (pdev_cs->pdev_comp_priv_obj[comp_id] != data) {
			wlan_cp_stats_pdev_obj_unlock(pdev_cs);
			return QDF_STATUS_E_INVAL;
		}
		data = pdev_cs->pdev_comp_priv_obj[comp_id];
		pdev_cs->pdev_comp_priv_obj[comp_id] = NULL;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_INVALID) {
		cp_stats_err("Invalid cp stats cfg_state");
		wlan_cp_stats_pdev_obj_unlock(pdev_cs);
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_pdev_obj_unlock(pdev_cs);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
wlan_cp_stats_vdev_comp_obj_config
(struct wlan_objmgr_vdev *vdev, enum wlan_cp_stats_comp_id comp_id,
	enum wlan_cp_stats_cfg_state cfg_state, void *data)
{
	struct vdev_cp_stats *vdev_cs;

	vdev_cs = wlan_cp_stats_get_vdev_stats_obj(vdev);
	if (!vdev_cs) {
		cp_stats_err("vdev cp stats object is null");
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_vdev_obj_lock(vdev_cs);
	if (cfg_state == WLAN_CP_STATS_OBJ_ATTACH) {
		if (vdev_cs->vdev_comp_priv_obj[comp_id]) {
			wlan_cp_stats_vdev_obj_unlock(vdev_cs);
			return QDF_STATUS_E_EXISTS;
		}
		vdev_cs->vdev_comp_priv_obj[comp_id] = data;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_DETACH) {
		if (vdev_cs->vdev_comp_priv_obj[comp_id] != data) {
			wlan_cp_stats_vdev_obj_unlock(vdev_cs);
			return QDF_STATUS_E_INVAL;
		}
		data = vdev_cs->vdev_comp_priv_obj[comp_id];
		vdev_cs->vdev_comp_priv_obj[comp_id] = NULL;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_INVALID) {
		cp_stats_err("Invalid cp stats cfg_state");
		wlan_cp_stats_vdev_obj_unlock(vdev_cs);
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_vdev_obj_unlock(vdev_cs);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
wlan_cp_stats_peer_comp_obj_config
(struct wlan_objmgr_peer *peer, enum wlan_cp_stats_comp_id comp_id,
	enum wlan_cp_stats_cfg_state cfg_state, void *data)
{
	struct peer_cp_stats *peer_cs;

	peer_cs = wlan_cp_stats_get_peer_stats_obj(peer);
	if (!peer_cs) {
		cp_stats_err("peer cp stats object is null");
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_peer_obj_lock(peer_cs);
	if (cfg_state == WLAN_CP_STATS_OBJ_ATTACH) {
		if (peer_cs->peer_comp_priv_obj[comp_id]) {
			wlan_cp_stats_peer_obj_unlock(peer_cs);
			return QDF_STATUS_E_EXISTS;
		}
		peer_cs->peer_comp_priv_obj[comp_id] = data;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_DETACH) {
		if (peer_cs->peer_comp_priv_obj[comp_id] != data) {
			wlan_cp_stats_peer_obj_unlock(peer_cs);
			return QDF_STATUS_E_INVAL;
		}
		data = peer_cs->peer_comp_priv_obj[comp_id];
		peer_cs->peer_comp_priv_obj[comp_id] = NULL;
	} else if (cfg_state == WLAN_CP_STATS_OBJ_INVALID) {
		cp_stats_err("Invalid cp stats cfg_state");
		wlan_cp_stats_peer_obj_unlock(peer_cs);
		return QDF_STATUS_E_INVAL;
	}

	wlan_cp_stats_peer_obj_unlock(peer_cs);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_comp_obj_config(enum wlan_objmgr_obj_type obj_type,
			      enum wlan_cp_stats_cfg_state cfg_state,
			      enum wlan_cp_stats_comp_id comp_id,
			      void *cmn_obj, void *data)
{
	QDF_STATUS status;

	if (!cmn_obj) {
		cp_stats_err("Common object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* component id is invalid */
	if (comp_id >= WLAN_CP_STATS_MAX_COMPONENTS) {
		cp_stats_err("Invalid component Id");
		return QDF_STATUS_MAXCOMP_FAIL;
	}

	switch (obj_type) {
	case WLAN_PSOC_OP:
		status =
			wlan_cp_stats_psoc_comp_obj_config(
					(struct wlan_objmgr_psoc *)cmn_obj,
					comp_id, cfg_state, data);
		break;
	case WLAN_PDEV_OP:
		status =
			wlan_cp_stats_pdev_comp_obj_config(
					(struct wlan_objmgr_pdev *)cmn_obj,
					comp_id, cfg_state, data);
		break;
	case WLAN_VDEV_OP:
		status =
			wlan_cp_stats_vdev_comp_obj_config(
					(struct wlan_objmgr_vdev *)cmn_obj,
					comp_id, cfg_state, data);
		break;
	case WLAN_PEER_OP:
		status =
			wlan_cp_stats_peer_comp_obj_config(
					(struct wlan_objmgr_peer *)cmn_obj,
					comp_id, cfg_state, data);
		break;
	default:
		cp_stats_err("Invalid common object");
		return QDF_STATUS_E_INVAL;
	}

	return status;
}
