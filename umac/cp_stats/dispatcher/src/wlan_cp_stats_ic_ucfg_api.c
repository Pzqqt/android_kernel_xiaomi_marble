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
#include <wlan_cp_stats_ic_ucfg_api.h>
#include <wlan_cfg80211_ic_cp_stats.h>
#ifdef WLAN_ATF_ENABLE
#include <wlan_cp_stats_ic_atf_defs.h>
#endif
#include <wlan_cp_stats_ic_defs.h>
#include <wlan_cp_stats_ic_dcs_defs.h>
#include "../../core/src/wlan_cp_stats_cmn_api_i.h"
#include <qdf_module.h>

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

QDF_STATUS wlan_ucfg_get_vdev_cp_stats(struct wlan_objmgr_vdev *vdev,
				       struct vdev_ic_cp_stats *vdev_cps)
{
	struct vdev_cp_stats *vdev_cs;

	if (!vdev) {
		cp_stats_err("Invalid input, vdev obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!vdev_cps) {
		cp_stats_err("Invalid input, vdev cp obj is null");
		return QDF_STATUS_E_INVAL;
	}

	vdev_cs = wlan_cp_stats_get_vdev_stats_obj(vdev);
	if (vdev_cs && vdev_cs->vdev_stats) {
		wlan_cp_stats_vdev_obj_lock(vdev_cs);
		qdf_mem_copy(vdev_cps, vdev_cs->vdev_stats,
			     sizeof(*vdev_cps));
		wlan_cp_stats_vdev_obj_unlock(vdev_cs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_ucfg_get_pdev_cp_stats(struct wlan_objmgr_pdev *pdev,
				       struct pdev_ic_cp_stats *pdev_cps)
{
	struct pdev_cp_stats *pdev_cs;

	if (!pdev) {
		cp_stats_err("Invalid input, pdev obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!pdev_cps) {
		cp_stats_err("Invalid input, pdev cp obj is null");
		return QDF_STATUS_E_INVAL;
	}

	pdev_cs = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (pdev_cs && pdev_cs->pdev_stats) {
		wlan_cp_stats_pdev_obj_lock(pdev_cs);
		qdf_mem_copy(pdev_cps, pdev_cs->pdev_stats,
			     sizeof(*pdev_cps));
		wlan_cp_stats_pdev_obj_unlock(pdev_cs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_ucfg_get_pdev_hw_cp_stats(struct wlan_objmgr_pdev *pdev,
					  struct pdev_hw_stats *hw_stats)
{
	struct pdev_cp_stats *pdev_cs;
	struct pdev_ic_cp_stats *pdev_cps;

	if (!pdev) {
		cp_stats_err("Invalid input, pdev obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!hw_stats) {
		cp_stats_err("Invalid input, pdev hw_stats is null");
		return QDF_STATUS_E_INVAL;
	}

	pdev_cs = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (pdev_cs && pdev_cs->pdev_stats) {
		pdev_cps = pdev_cs->pdev_stats;
		wlan_cp_stats_pdev_obj_lock(pdev_cs);
		qdf_mem_copy(hw_stats, &pdev_cps->stats.hw_stats,
			     sizeof(*hw_stats));
		wlan_cp_stats_pdev_obj_unlock(pdev_cs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_ucfg_set_pdev_hw_cp_stats(struct wlan_objmgr_pdev *pdev,
					  struct pdev_hw_stats *hw_stats)
{
	struct pdev_cp_stats *pdev_cs;
	struct pdev_ic_cp_stats *pdev_cps;

	if (!pdev) {
		cp_stats_err("Invalid input, pdev obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!hw_stats) {
		cp_stats_err("Invalid input, pdev hw_stats is null");
		return QDF_STATUS_E_INVAL;
	}

	pdev_cs = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (pdev_cs && pdev_cs->pdev_stats) {
		pdev_cps = pdev_cs->pdev_stats;
		wlan_cp_stats_pdev_obj_lock(pdev_cs);
		qdf_mem_copy(&pdev_cps->stats.hw_stats, hw_stats,
			     sizeof(*hw_stats));
		wlan_cp_stats_pdev_obj_unlock(pdev_cs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

struct pdev_ic_cp_stats
*wlan_ucfg_get_pdev_cp_stats_ref(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cp_stats *pdev_cs = NULL;

	if (!pdev) {
		cp_stats_err("pdev is null");
		return NULL;
	}

	pdev_cs = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (pdev_cs && pdev_cs->pdev_stats)
		return pdev_cs->pdev_stats;

	return NULL;
}

struct pdev_ic_cp_stats
*wlan_get_pdev_cp_stats_ref(struct  wlan_objmgr_pdev *pdev)
{
	return wlan_ucfg_get_pdev_cp_stats_ref(pdev);
}

qdf_export_symbol(wlan_get_pdev_cp_stats_ref);

#ifdef WLAN_ATF_ENABLE
QDF_STATUS
wlan_ucfg_get_atf_peer_cp_stats(struct wlan_objmgr_peer *peer,
				struct atf_peer_cp_stats *atf_cps)
{
	struct peer_cp_stats *peer_cs;

	if (!peer) {
		cp_stats_err("Invalid input, peer obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!atf_cps) {
		cp_stats_err("Invalid input, ATF cp stats obj is null");
		return QDF_STATUS_E_INVAL;
	}

	peer_cs = wlan_cp_stats_get_peer_stats_obj(peer);
	if (peer_cs) {
		if (peer_cs->peer_comp_priv_obj[WLAN_CP_STATS_ATF]) {
			wlan_cp_stats_peer_obj_lock(peer_cs);
			qdf_mem_copy(atf_cps,
				peer_cs->peer_comp_priv_obj[WLAN_CP_STATS_ATF],
				sizeof(*atf_cps));
			wlan_cp_stats_peer_obj_unlock(peer_cs);
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wlan_ucfg_get_atf_peer_cp_stats_from_mac(struct wlan_objmgr_vdev *vdev,
					 uint8_t *mac,
					 struct atf_peer_cp_stats *astats)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	if (!vdev) {
		cp_stats_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!mac) {
		cp_stats_err("peer mac address is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!astats) {
		cp_stats_err("atf peer stats obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		cp_stats_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		cp_stats_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	peer = wlan_objmgr_get_peer(psoc, wlan_objmgr_pdev_get_pdev_id(pdev),
				    mac, WLAN_CP_STATS_ID);
	if (!peer) {
		cp_stats_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = wlan_ucfg_get_atf_peer_cp_stats(peer, astats);
	wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);

	return status;
}
#endif

QDF_STATUS
wlan_ucfg_get_dcs_chan_stats(struct wlan_objmgr_pdev *pdev,
			     struct pdev_dcs_chan_stats *dcs_chan_stats)
{
	return QDF_STATUS_E_INVAL;
}
