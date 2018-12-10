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

#include "wlan_dfs_ucfg_api.h"
#include "wlan_dfs_init_deinit_api.h"
#include "../../core/src/dfs.h"
#include "../../core/src/dfs_zero_cac.h"
#include "../../core/src/dfs_partial_offload_radar.h"
#include "../../core/src/dfs_process_radar_found_ind.h"
#include <qdf_module.h>

QDF_STATUS ucfg_dfs_is_ap_cac_timer_running(struct wlan_objmgr_pdev *pdev,
		int *is_ap_cac_timer_running)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	*is_ap_cac_timer_running = dfs_is_ap_cac_timer_running(dfs);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_is_ap_cac_timer_running);

QDF_STATUS ucfg_dfs_getnol(struct wlan_objmgr_pdev *pdev,
		void *dfs_nolinfo)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	dfs_getnol(dfs, dfs_nolinfo);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_getnol);

QDF_STATUS ucfg_dfs_override_cac_timeout(struct wlan_objmgr_pdev *pdev,
		int cac_timeout,
		int *status)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	*status = dfs_override_cac_timeout(dfs, cac_timeout);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_override_cac_timeout);

QDF_STATUS ucfg_dfs_get_override_cac_timeout(struct wlan_objmgr_pdev *pdev,
		int *cac_timeout,
		int *status)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	*status = dfs_get_override_cac_timeout(dfs, cac_timeout);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_get_override_cac_timeout);

QDF_STATUS ucfg_dfs_get_override_precac_timeout(struct wlan_objmgr_pdev *pdev,
		int *precac_timeout)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	dfs_get_override_precac_timeout(dfs, precac_timeout);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_get_override_precac_timeout);

QDF_STATUS ucfg_dfs_override_precac_timeout(struct wlan_objmgr_pdev *pdev,
		int precac_timeout)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	dfs_override_precac_timeout(dfs, precac_timeout);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_override_precac_timeout);

QDF_STATUS ucfg_dfs_set_precac_enable(struct wlan_objmgr_pdev *pdev,
		uint32_t value)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_set_precac_enable(dfs, value);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_set_precac_enable);

QDF_STATUS ucfg_dfs_get_precac_enable(struct wlan_objmgr_pdev *pdev,
		int *buff)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_pdev_5ghz(pdev))
		return QDF_STATUS_SUCCESS;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	*buff = dfs_get_precac_enable(dfs);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_get_precac_enable);

QDF_STATUS
ucfg_dfs_set_nol_subchannel_marking(struct wlan_objmgr_pdev *pdev,
				    bool nol_subchannel_marking)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	dfs_set_nol_subchannel_marking(dfs, nol_subchannel_marking);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_set_nol_subchannel_marking);

QDF_STATUS ucfg_dfs_get_nol_subchannel_marking(struct wlan_objmgr_pdev *pdev,
					       bool *nol_subchannel_marking)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return  QDF_STATUS_E_FAILURE;

	dfs_get_nol_subchannel_marking(dfs, nol_subchannel_marking);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_get_nol_subchannel_marking);
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
QDF_STATUS ucfg_dfs_set_precac_intermediate_chan(struct wlan_objmgr_pdev *pdev,
						 uint32_t value)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_set_precac_intermediate_chan(dfs, value);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_dfs_get_precac_intermediate_chan(struct wlan_objmgr_pdev *pdev,
						 int *buff)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	*buff = dfs_get_precac_intermediate_chan(dfs);

	return QDF_STATUS_SUCCESS;
}

enum precac_chan_state
ucfg_dfs_get_precac_chan_state(struct wlan_objmgr_pdev *pdev,
			       uint8_t precac_chan)
{
	struct wlan_dfs *dfs;
	enum precac_chan_state retval = PRECAC_ERR;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return PRECAC_ERR;
	}

	retval = dfs_get_precac_chan_state(dfs, precac_chan);
	if (PRECAC_ERR == retval) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Could not find precac channel state");
	}

	return retval;
}
#endif

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS ucfg_dfs_update_config(struct wlan_objmgr_psoc *psoc,
		struct dfs_user_config *req)
{
	struct dfs_soc_priv_obj *soc_obj;

	if (!psoc || !req) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"psoc: 0x%pK, req: 0x%pK", psoc, req);
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_DFS);
	if (!soc_obj) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to get dfs psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj->dfs_is_phyerr_filter_offload =
			req->dfs_is_phyerr_filter_offload;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(ucfg_dfs_update_config);
#endif

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS ucfg_dfs_set_override_status_timeout(struct wlan_objmgr_pdev *pdev,
					    int status_timeout)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_set_override_status_timeout(dfs, status_timeout);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_override_status_timeout);

QDF_STATUS ucfg_dfs_get_override_status_timeout(struct wlan_objmgr_pdev *pdev,
						int *status_timeout)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	dfs_get_override_status_timeout(dfs, status_timeout);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_override_status_timeout);
#endif
