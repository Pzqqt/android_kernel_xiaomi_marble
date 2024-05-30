/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

QDF_STATUS ucfg_dfs_get_legacy_precac_enable(struct wlan_objmgr_pdev *pdev,
					     bool *buff)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev))
		return QDF_STATUS_SUCCESS;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return  QDF_STATUS_E_FAILURE;
	}

	*buff = dfs_is_legacy_precac_enabled(dfs);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_legacy_precac_enable);

QDF_STATUS ucfg_dfs_get_agile_precac_enable(struct wlan_objmgr_pdev *pdev,
					    bool *buff)
{
	struct wlan_dfs *dfs;

	if (!pdev || !buff)
		return QDF_STATUS_E_FAILURE;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev)) {
		*buff = false;
		return QDF_STATUS_SUCCESS;
	}

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		*buff = false;
		return QDF_STATUS_SUCCESS;
	}

	*buff = dfs_is_agile_precac_enabled(dfs);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_agile_precac_enable);

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

#ifdef CONFIG_CHAN_FREQ_API
enum precac_chan_state
ucfg_dfs_get_precac_chan_state_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t precac_chan_freq)
{
	struct wlan_dfs *dfs;
	enum precac_chan_state retval = PRECAC_ERR;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return PRECAC_ERR;
	}

	retval = dfs_get_precac_chan_state_for_freq(dfs, precac_chan_freq);
	if (retval == PRECAC_ERR) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Could not find precac channel state");
	}

	return retval;
}
#endif
#endif

#ifdef MOBILE_DFS_SUPPORT
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

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(WLAN_DFS_SYNTHETIC_RADAR)
void ucfg_dfs_allow_hw_pulses(struct wlan_objmgr_pdev *pdev,
			      bool allow_hw_pulses)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev))
		return;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_allow_hw_pulses(dfs, allow_hw_pulses);
}

qdf_export_symbol(ucfg_dfs_allow_hw_pulses);

bool ucfg_dfs_is_hw_pulses_allowed(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev))
		return false;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return false;
	}

	return dfs_is_hw_pulses_allowed(dfs);
}

qdf_export_symbol(ucfg_dfs_is_hw_pulses_allowed);
#endif

#ifdef QCA_SUPPORT_AGILE_DFS
QDF_STATUS ucfg_dfs_reset_agile_config(struct wlan_objmgr_psoc *psoc)
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

	dfs_reset_agile_config(soc_obj);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_reset_agile_config);
#endif

QDF_STATUS ucfg_dfs_reinit_timers(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev))
		return QDF_STATUS_SUCCESS;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_reinit_timers(dfs);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_reinit_timers);

#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS ucfg_dfs_set_rcac_enable(struct wlan_objmgr_pdev *pdev,
				    bool rcac_en)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_set_rcac_enable(dfs, rcac_en);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_rcac_enable);

QDF_STATUS ucfg_dfs_get_rcac_enable(struct wlan_objmgr_pdev *pdev,
				    bool *rcac_en)
{
	struct wlan_dfs *dfs;

	if (!tgt_dfs_is_5ghz_supported_in_pdev(pdev)) {
		*rcac_en = false;
		return QDF_STATUS_SUCCESS;
	}

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_rcac_enable(dfs, rcac_en);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_rcac_enable);

QDF_STATUS ucfg_dfs_set_rcac_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t rcac_freq)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_set_rcac_freq(dfs, rcac_freq);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_rcac_freq);

QDF_STATUS ucfg_dfs_get_rcac_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t *rcac_freq)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_rcac_freq(dfs, rcac_freq);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_rcac_freq);

bool ucfg_dfs_is_agile_rcac_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return false;
	}

	return dfs_is_agile_rcac_enabled(dfs);
}

qdf_export_symbol(ucfg_dfs_is_agile_rcac_enabled);
#endif

