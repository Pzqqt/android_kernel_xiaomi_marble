/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_pdev_obj.h>
#include "wlan_dfs_tgt_api.h"
#include "wlan_lmac_if_def.h"
#include "wlan_lmac_if_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../../core/src/dfs.h"
#include "../../core/src/dfs_zero_cac.h"
#include "../../core/src/dfs_process_radar_found_ind.h"

struct wlan_lmac_if_dfs_tx_ops *
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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
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
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_is_offload_enabled) {
		dfs_radar_enable(dfs, no_cac, opmode);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "psoc is null");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (!dfs_tx_ops) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_tx_ops is null");
		return  QDF_STATUS_E_FAILURE;
	}

	status = dfs_tx_ops->dfs_send_offload_enable_cmd(pdev, true);
	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to enable dfs offload, pdev_id: %d",
			wlan_objmgr_pdev_get_pdev_id(pdev));

	return status;
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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!dfs->dfs_is_offload_enabled)
		dfs_process_phyerr(dfs, buf, datalen, r_rssi,
				   r_ext_rssi, r_rs_tstamp, r_fulltsf);
	else
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Unexpect phyerror as DFS is offloaded, pdev_id: %d",
			 wlan_objmgr_pdev_get_pdev_id(pdev));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_process_phyerr);

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS tgt_dfs_process_phyerr_filter_offload(struct wlan_objmgr_pdev *pdev,
						 struct radar_event_info
						 *wlan_radar_event)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}
	if (!dfs->dfs_is_offload_enabled)
		dfs_process_phyerr_filter_offload(dfs, wlan_radar_event);
	else
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Unexpect phyerror as DFS is offloaded, pdev_id: %d",
			 wlan_objmgr_pdev_get_pdev_id(pdev));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_process_phyerr_filter_offload);

QDF_STATUS tgt_dfs_is_phyerr_filter_offload(struct wlan_objmgr_psoc *psoc,
					    bool *is_phyerr_filter_offload)
{
	struct dfs_soc_priv_obj *soc_obj;

	if (!psoc) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_DFS);
	if (!soc_obj) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to get dfs psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	*is_phyerr_filter_offload = soc_obj->dfs_is_phyerr_filter_offload;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_is_phyerr_filter_offload);
#else
QDF_STATUS tgt_dfs_process_phyerr_filter_offload(struct wlan_objmgr_pdev *pdev,
						 struct radar_event_info
						 *wlan_radar_event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_dfs_is_phyerr_filter_offload(struct wlan_objmgr_psoc *psoc,
					    bool *is_phyerr_filter_offload)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS tgt_dfs_is_precac_timer_running(struct wlan_objmgr_pdev *pdev,
					   bool *is_precac_timer_running)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_destroy_object(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_destroy_object);

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS tgt_dfs_set_tx_leakage_threshold(struct wlan_objmgr_pdev *pdev,
		uint16_t tx_leakage_threshold)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs->tx_leakage_threshold = tx_leakage_threshold;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_set_tx_leakage_threshold);
#endif

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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
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
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is null");
		return QDF_STATUS_E_FAILURE;
	}

	return dfs_process_radar_ind(dfs, radar_found);
}
EXPORT_SYMBOL(tgt_dfs_process_radar_ind);

#ifndef QCA_MCL_DFS_SUPPORT
QDF_STATUS tgt_dfs_cac_complete(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS tgt_dfs_cac_complete(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	dfs_mlme_proc_cac(pdev, vdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(tgt_dfs_cac_complete);

QDF_STATUS tgt_dfs_reg_ev_handler(struct wlan_objmgr_psoc *psoc,
				  bool dfs_offload)
{
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	if (!psoc) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (!dfs_tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "null dfs_tx_ops");
		return QDF_STATUS_E_FAILURE;
	}

	if (dfs_tx_ops->dfs_reg_ev_handler)
		return dfs_tx_ops->dfs_reg_ev_handler(psoc, dfs_offload);

	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(tgt_dfs_reg_ev_handler);

QDF_STATUS tgt_dfs_stop(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_stop(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_stop);

QDF_STATUS tgt_dfs_process_emulate_bang_radar_cmd(struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (dfs_tx_ops && dfs_tx_ops->dfs_process_emulate_bang_radar_cmd)
		return dfs_tx_ops->dfs_process_emulate_bang_radar_cmd(pdev,
				dfs_unit_test);
	else
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_tx_ops=%pK", dfs_tx_ops);

	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(tgt_dfs_process_emulate_bang_radar_cmd);

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS tgt_dfs_set_phyerr_filter_offload(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct dfs_soc_priv_obj *soc_obj;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_DFS);
	if (!soc_obj) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to get dfs psoc component");
		return QDF_STATUS_E_FAILURE;
	}
	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (dfs_tx_ops && dfs_tx_ops->dfs_set_phyerr_filter_offload)
		return dfs_tx_ops->dfs_set_phyerr_filter_offload(pdev,
				soc_obj->dfs_is_phyerr_filter_offload);
	else
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_tx_ops=%pK", dfs_tx_ops);

	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(tgt_dfs_set_phyerr_filter_offload);
#endif
