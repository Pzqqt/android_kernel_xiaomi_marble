/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
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
 * DOC: This file has the DFS dispatcher API implementation which is exposed
 * to outside of DFS component.
 */
#include "wlan_dfs_tgt_api.h"
#include "wlan_lmac_if_def.h"
#include "wlan_lmac_if_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../../core/src/dfs.h"
#include "../../core/src/dfs_zero_cac.h"
#include "../../core/src/dfs_process_radar_found_ind.h"

static inline struct wlan_lmac_if_dfs_tx_ops *
wlan_psoc_get_dfs_txops(struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.dfs_tx_ops));
}

QDF_STATUS tgt_dfs_set_current_channel(struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint64_t dfs_ch_flags,
		uint16_t dfs_ch_flagext,
		uint8_t dfs_ch_ieee,
		uint8_t dfs_ch_vhtop_ch_freq_seg1,
		uint8_t dfs_ch_vhtop_ch_freq_seg2)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_set_current_channel(dfs,
			dfs_ch_freq,
			dfs_ch_flags,
			dfs_ch_flagext,
			dfs_ch_ieee,
			dfs_ch_vhtop_ch_freq_seg1,
			dfs_ch_vhtop_ch_freq_seg2);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_set_current_channel);

QDF_STATUS tgt_dfs_radar_enable(struct wlan_objmgr_pdev *pdev,
		int no_cac, uint32_t opmode)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_is_offload_enabled)
		dfs_radar_enable(dfs, no_cac, opmode);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_radar_enable);

QDF_STATUS tgt_dfs_process_phyerr(struct wlan_objmgr_pdev *pdev,
		void *buf,
		uint16_t datalen,
		uint8_t r_rssi,
		uint8_t r_ext_rssi,
		uint32_t r_rs_tstamp,
		uint64_t r_fulltsf)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_is_offload_enabled)
		dfs_process_phyerr(dfs, buf, datalen, r_rssi,
				r_ext_rssi, r_rs_tstamp, r_fulltsf);
	else
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Received a pulse from firmware even though the DFS is offloaded"
				);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_process_phyerr);

QDF_STATUS tgt_dfs_is_precac_timer_running(struct wlan_objmgr_pdev *pdev,
		bool *is_precac_timer_running)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	*is_precac_timer_running = dfs_is_precac_timer_running(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_is_precac_timer_running);

QDF_STATUS tgt_dfs_get_radars(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_is_offload_enabled)
		dfs_get_radars(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_get_radars);

QDF_STATUS tgt_dfs_destroy_object(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_destroy_object(dfs);
	dfs = NULL;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_destroy_object);

QDF_STATUS tgt_dfs_control(struct wlan_objmgr_pdev *pdev,
		u_int id,
		void *indata,
		uint32_t insize,
		void *outdata,
		uint32_t *outsize,
		int *error)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	*error = dfs_control(dfs, id, indata, insize, outdata, outsize);

	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_control);

QDF_STATUS tgt_dfs_find_vht80_chan_for_precac(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_find_vht80_chan_for_precac(dfs,
			chan_mode,
			ch_freq_seg1,
			cfreq1,
			cfreq2,
			phy_mode,
			dfs_set_cfreq2,
			set_agile);

	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_find_vht80_chan_for_precac);

QDF_STATUS tgt_dfs_process_radar_ind(struct wlan_objmgr_pdev *pdev,
		struct radar_found_info *radar_found)
{
	struct wlan_dfs *dfs;

	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_curchan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs->dfs_curchan is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_process_radar_found_indication(dfs, radar_found);
	dfs_mlme_mark_dfs(pdev, dfs->dfs_curchan->dfs_ch_ieee,
		dfs->dfs_curchan->dfs_ch_freq,
		dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
		dfs->dfs_curchan->dfs_ch_flags);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_process_radar_ind);

QDF_STATUS tgt_dfs_cac_complete(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	dfs_mlme_proc_cac(pdev, vdev_id);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_cac_complete);

QDF_STATUS tgt_dfs_reg_ev_handler(struct wlan_objmgr_psoc *psoc,
		bool dfs_offload)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_objmgr_pdev *pdev = NULL;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0, WLAN_DFS_ID);
	if (!pdev) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (dfs_tx_ops && dfs_tx_ops->dfs_reg_ev_handler)
		status = dfs_tx_ops->dfs_reg_ev_handler(pdev, dfs_offload);
	else
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"dfs_tx_ops=%p", dfs_tx_ops);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);

	return status;
}
EXPORT_SYMBOL(tgt_dfs_reg_ev_handler);

QDF_STATUS tgt_dfs_stop(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_stop(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_stop);
