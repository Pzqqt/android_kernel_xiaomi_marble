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
 * DOC:wlan_cp_stats_mc_tgt_api.c
 *
 * This file provide API definitions to update control plane statistics received
 * from southbound interface
 */

#include "wlan_cp_stats_mc_defs.h"
#include "target_if_cp_stats.h"
#include "wlan_cp_stats_tgt_api.h"
#include "wlan_cp_stats_mc_tgt_api.h"
#include <wlan_cp_stats_mc_ucfg_api.h>
#include <wlan_cp_stats_utils_api.h>
#include "../../core/src/wlan_cp_stats_defs.h"

static void tgt_mc_cp_stats_extract_tx_power(struct wlan_objmgr_psoc *psoc,
					struct stats_event *ev,
					bool is_station_stats)
{
	int32_t max_pwr;
	uint8_t pdev_id;
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev;
	struct request_info last_req = {0};
	struct wlan_objmgr_vdev *vdev = NULL;
	struct pdev_mc_cp_stats *pdev_mc_stats;
	struct pdev_cp_stats *pdev_cp_stats_priv;

	if (!ev->pdev_stats) {
		cp_stats_err("no pdev stats");
		return;
	}

	if (is_station_stats)
		status = ucfg_mc_cp_stats_get_pending_req(psoc,
					TYPE_STATION_STATS, &last_req);
	else
		status = ucfg_mc_cp_stats_get_pending_req(psoc,
					TYPE_CONNECTION_TX_POWER, &last_req);

	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("ucfg_mc_cp_stats_get_pending_req failed");
		goto end;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, last_req.vdev_id,
						    WLAN_CP_STATS_ID);
	if (!vdev) {
		cp_stats_err("vdev is null");
		goto end;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		cp_stats_err("pdev is null");
		goto end;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (pdev_id >= ev->num_pdev_stats) {
		cp_stats_err("pdev_id: %d invalid", pdev_id);
		goto end;
	}

	pdev_cp_stats_priv = wlan_cp_stats_get_pdev_stats_obj(pdev);
	if (!pdev_cp_stats_priv) {
		cp_stats_err("pdev_cp_stats_priv is null");
		goto end;
	}

	wlan_cp_stats_pdev_obj_lock(pdev_cp_stats_priv);
	pdev_mc_stats = pdev_cp_stats_priv->pdev_stats;
	max_pwr = pdev_mc_stats->max_pwr = ev->pdev_stats[pdev_id].max_pwr;
	wlan_cp_stats_pdev_obj_unlock(pdev_cp_stats_priv);

	if (is_station_stats)
		goto end;

	if (last_req.u.get_tx_power_cb)
		last_req.u.get_tx_power_cb(max_pwr, last_req.cookie);
	ucfg_mc_cp_stats_reset_pending_req(psoc, TYPE_CONNECTION_TX_POWER);

end:
	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_CP_STATS_ID);
	qdf_mem_free(ev->pdev_stats);
	ev->pdev_stats = NULL;
}

QDF_STATUS tgt_mc_cp_stats_process_stats_event(struct wlan_objmgr_psoc *psoc,
					       struct stats_event *ev)
{
	if (ucfg_mc_cp_stats_is_req_pending(psoc, TYPE_CONNECTION_TX_POWER))
		tgt_mc_cp_stats_extract_tx_power(psoc, ev, false);


	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_mc_cp_stats_inc_wake_lock_stats(struct wlan_objmgr_psoc *psoc,
					       uint32_t reason,
					       struct wake_lock_stats *stats,
					       uint32_t *unspecified_wake_count)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops)
		return QDF_STATUS_E_NULL_VALUE;

	tx_ops->inc_wake_lock_stats(reason, stats, unspecified_wake_count);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_send_mc_cp_stats_req(struct wlan_objmgr_psoc *psoc,
				    enum stats_req_type type,
				    struct request_info *req)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops) {
		cp_stats_err("could not get tx_ops");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return tx_ops->send_req_stats(psoc, type, req);
}
