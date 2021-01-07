/*
 * Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_ol_api.c
 *
 * This file provide definitions for following
 * - (de)init cp stat global ctx obj
 * - (de)init common specific ucfg handler
 * - (de)register to WMI events for psoc open
 */
#include <wlan_objmgr_cmn.h>
#include "wlan_cp_stats_defs.h"
#include "wlan_cp_stats_ol_api.h"
#include "wlan_cp_stats_cmn_api_i.h"
#include <wlan_cp_stats_ucfg_api.h>
#include <wlan_cp_stats_utils_api.h>
#include <target_if_cp_stats.h>

QDF_STATUS wlan_cp_stats_psoc_obj_init_ol(struct psoc_cp_stats *psoc_cs)
{
	qdf_spinlock_create(&psoc_cs->psoc_cp_stats_lock);
	wlan_cp_stats_psoc_cs_init(psoc_cs);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_psoc_obj_deinit_ol(struct psoc_cp_stats *psoc_cs)
{
	wlan_cp_stats_psoc_cs_deinit(psoc_cs);
	qdf_spinlock_destroy(&psoc_cs->psoc_cp_stats_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_obj_init_ol(struct pdev_cp_stats *pdev_cs)
{
	qdf_spinlock_create(&pdev_cs->pdev_cp_stats_lock);
	wlan_cp_stats_pdev_cs_init(pdev_cs);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_obj_deinit_ol(struct pdev_cp_stats *pdev_cs)
{
	wlan_cp_stats_pdev_cs_deinit(pdev_cs);
	qdf_spinlock_destroy(&pdev_cs->pdev_cp_stats_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_obj_init_ol(struct vdev_cp_stats *vdev_cs)
{
	qdf_spinlock_create(&vdev_cs->vdev_cp_stats_lock);
	wlan_cp_stats_vdev_cs_init(vdev_cs);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_obj_deinit_ol(struct vdev_cp_stats *vdev_cs)
{
	wlan_cp_stats_vdev_cs_deinit(vdev_cs);
	qdf_spinlock_destroy(&vdev_cs->vdev_cp_stats_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_obj_init_ol(struct peer_cp_stats *peer_cs)
{
	qdf_spinlock_create(&peer_cs->peer_cp_stats_lock);
	wlan_cp_stats_peer_cs_init(peer_cs);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_obj_deinit_ol(struct peer_cp_stats *peer_cs)
{
	wlan_cp_stats_peer_cs_deinit(peer_cs);
	qdf_spinlock_destroy(&peer_cs->peer_cp_stats_lock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_open_ol(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_close_ol(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_enable_ol(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops) {
		cp_stats_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->cp_stats_attach) {
		cp_stats_err("cp_stats_attach function ptr is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	tx_ops->cp_stats_attach(psoc);

	if (tx_ops->cp_stats_legacy_attach)
		tx_ops->cp_stats_legacy_attach(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_disable_ol(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops) {
		cp_stats_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->cp_stats_legacy_detach) {
		cp_stats_err("cp_stats_legacy_detach function ptr is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}
	tx_ops->cp_stats_legacy_detach(psoc);

	if (!tx_ops->cp_stats_detach) {
		cp_stats_err("cp_stats_detach function ptr is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}
	tx_ops->cp_stats_detach(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_ctx_init_ol(struct cp_stats_context *csc)
{
	csc->cp_stats_open = wlan_cp_stats_open_ol;
	csc->cp_stats_close = wlan_cp_stats_close_ol;
	csc->cp_stats_enable = wlan_cp_stats_enable_ol;
	csc->cp_stats_disable = wlan_cp_stats_disable_ol;
	csc->cp_stats_psoc_obj_init = wlan_cp_stats_psoc_obj_init_ol;
	csc->cp_stats_psoc_obj_deinit = wlan_cp_stats_psoc_obj_deinit_ol;
	csc->cp_stats_pdev_obj_init = wlan_cp_stats_pdev_obj_init_ol;
	csc->cp_stats_pdev_obj_deinit = wlan_cp_stats_pdev_obj_deinit_ol;
	csc->cp_stats_vdev_obj_init = wlan_cp_stats_vdev_obj_init_ol;
	csc->cp_stats_vdev_obj_deinit = wlan_cp_stats_vdev_obj_deinit_ol;
	csc->cp_stats_peer_obj_init = wlan_cp_stats_peer_obj_init_ol;
	csc->cp_stats_peer_obj_deinit = wlan_cp_stats_peer_obj_deinit_ol;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_ctx_deinit_ol(struct cp_stats_context *csc)
{
	csc->cp_stats_open = NULL;
	csc->cp_stats_close = NULL;
	csc->cp_stats_enable = NULL;
	csc->cp_stats_disable = NULL;
	csc->cp_stats_psoc_obj_init = NULL;
	csc->cp_stats_psoc_obj_deinit = NULL;
	csc->cp_stats_pdev_obj_init = NULL;
	csc->cp_stats_pdev_obj_deinit = NULL;
	csc->cp_stats_vdev_obj_init = NULL;
	csc->cp_stats_vdev_obj_deinit = NULL;
	csc->cp_stats_peer_obj_init = NULL;
	csc->cp_stats_peer_obj_deinit = NULL;

	return QDF_STATUS_SUCCESS;
}
