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
 * DOC:wlan_cp_stats_ic_ucfg_api.c
 *
 * This file provide APIs definition for registering cp stats cfg80211 command
 * handlers
 */
#include "wlan_cp_stats_ic_ucfg_api.h"
#include <wlan_cfg80211_ic_cp_stats.h>
#include <wlan_cp_stats_ic_atf_defs.h>
#include <wlan_cp_stats_ic_defs.h>
#include "../../core/src/wlan_cp_stats_cmn_api_i.h"

QDF_STATUS wlan_cp_stats_psoc_cs_init(struct psoc_cp_stats *psoc_cs)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_psoc_cs_deinit(struct psoc_cp_stats *psoc_cs)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_cs_init(struct pdev_cp_stats *pdev_cs)
{
	pdev_cs->pdev_stats = qdf_mem_malloc(sizeof(struct pdev_ic_cp_stats));
	if (!pdev_cs->pdev_stats) {
		cp_stats_err("malloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_cs_deinit(struct pdev_cp_stats *pdev_cs)
{
	qdf_mem_free(pdev_cs->pdev_stats);
	pdev_cs->pdev_stats = NULL;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_cs_init(struct vdev_cp_stats *vdev_cs)
{
	vdev_cs->vdev_stats = qdf_mem_malloc(sizeof(struct vdev_ic_cp_stats));
	if (!vdev_cs->vdev_stats) {
		cp_stats_err("malloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_cs_deinit(struct vdev_cp_stats *vdev_cs)
{
	qdf_mem_free(vdev_cs->vdev_stats);
	vdev_cs->vdev_stats = NULL;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_cs_init(struct peer_cp_stats *peer_cs)
{
	peer_cs->peer_stats = qdf_mem_malloc(sizeof(struct peer_ic_cp_stats));
	if (!peer_cs->peer_stats) {
		cp_stats_err("malloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_cs_deinit(struct peer_cp_stats *peer_cs)
{
	qdf_mem_free(peer_cs->peer_stats);
	peer_cs->peer_stats = NULL;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_ucfg_get_peer_cp_stats(struct wlan_objmgr_peer *peer,
				       struct peer_ic_cp_stats *peer_cps)
{
	struct peer_cp_stats *peer_cs;

	if (!peer) {
		cp_stats_err("Invalid input fields, peer obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!peer_cps) {
		cp_stats_err("Invalid input fields, peer cp obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	peer_cs = wlan_cp_stats_get_peer_stats_obj(peer);
	if (peer_cs && peer_cs->peer_stats) {
		wlan_cp_stats_peer_obj_lock(peer_cs);
		qdf_mem_copy(peer_cps, peer_cs->peer_stats,
			     sizeof(struct peer_ic_cp_stats));
		wlan_cp_stats_peer_obj_unlock(peer_cs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
