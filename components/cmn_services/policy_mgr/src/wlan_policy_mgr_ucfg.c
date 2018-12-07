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
#include "wlan_policy_mgr_ucfg.h"
#include "wlan_policy_mgr_i.h"
#include "cfg_ucfg_api.h"
#include "wlan_policy_mgr_api.h"

static QDF_STATUS policy_mgr_init_cfg(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	struct policy_mgr_cfg *cfg;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("pm_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	cfg = &pm_ctx->cfg;

	cfg->mcc_to_scc_switch = cfg_get(psoc, CFG_MCC_TO_SCC_SWITCH);
	cfg->sys_pref = cfg_get(psoc, CFG_CONC_SYS_PREF);
	cfg->max_conc_cxns = cfg_get(psoc, CFG_MAX_CONC_CXNS);
	cfg->conc_rule1 = cfg_get(psoc, CFG_ENABLE_CONC_RULE1);
	cfg->conc_rule2 = cfg_get(psoc, CFG_ENABLE_CONC_RULE2);
	cfg->dbs_selection_plcy = cfg_get(psoc, CFG_DBS_SELECTION_PLCY);
	cfg->vdev_priority_list = cfg_get(psoc, CFG_VDEV_CUSTOM_PRIORITY_LIST);
	cfg->chnl_select_plcy = cfg_get(psoc, CFG_CHNL_SELECT_LOGIC_CONC);
	cfg->enable_mcc_adaptive_sch =
		cfg_get(psoc, CFG_ENABLE_MCC_ADATIVE_SCH_ENABLED_NAME);
	cfg->enable_sta_cxn_5g_band =
		cfg_get(psoc, CFG_ENABLE_STA_CONNECTION_IN_5GHZ);

	return QDF_STATUS_SUCCESS;
}

static void policy_mgr_deinit_cfg(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("pm_ctx is NULL");
		return;
	}

	qdf_mem_zero(&pm_ctx->cfg, sizeof(pm_ctx->cfg));
}

QDF_STATUS ucfg_policy_mgr_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = policy_mgr_init_cfg(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("pm_ctx is NULL");
		return status;
	}
	return QDF_STATUS_SUCCESS;
}

void ucfg_policy_mgr_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	policy_mgr_deinit_cfg(psoc);
}

QDF_STATUS ucfg_policy_mgr_get_mcc_scc_switch(struct wlan_objmgr_psoc *psoc,
					      uint8_t *mcc_scc_switch)
{
	return policy_mgr_get_mcc_scc_switch(psoc, mcc_scc_switch);
}

QDF_STATUS ucfg_policy_mgr_get_sys_pref(struct wlan_objmgr_psoc *psoc,
					uint8_t *sys_pref)
{
	return policy_mgr_get_sys_pref(psoc, sys_pref);
}

QDF_STATUS ucfg_policy_mgr_set_sys_pref(struct wlan_objmgr_psoc *psoc,
					uint8_t sys_pref)
{
	return policy_mgr_set_sys_pref(psoc, sys_pref);
}

QDF_STATUS ucfg_policy_mgr_get_max_conc_cxns(struct wlan_objmgr_psoc *psoc,
						uint8_t *max_conc_cxns)
{
	return policy_mgr_get_max_conc_cxns(psoc, max_conc_cxns);;
}

QDF_STATUS ucfg_policy_mgr_get_conc_rule1(struct wlan_objmgr_psoc *psoc,
						uint8_t *conc_rule1)
{
	return policy_mgr_get_conc_rule1(psoc, conc_rule1);
}

QDF_STATUS ucfg_policy_mgr_get_conc_rule2(struct wlan_objmgr_psoc *psoc,
						uint8_t *conc_rule2)
{
	return policy_mgr_get_conc_rule2(psoc, conc_rule2);
}

QDF_STATUS ucfg_policy_mgr_get_dbs_selection_plcy(struct wlan_objmgr_psoc *psoc,
						uint32_t *dbs_selection_plcy)
{
	return policy_mgr_get_dbs_selection_plcy(psoc, dbs_selection_plcy);
}

QDF_STATUS ucfg_policy_mgr_get_vdev_priority_list(struct wlan_objmgr_psoc *psoc,
						uint32_t *vdev_priority_list)
{
	return policy_mgr_get_vdev_priority_list(psoc, vdev_priority_list);
}

QDF_STATUS ucfg_policy_mgr_get_chnl_select_plcy(struct wlan_objmgr_psoc *psoc,
						uint32_t *chnl_select_plcy)
{
	return policy_mgr_get_chnl_select_plcy(psoc, chnl_select_plcy);
}


QDF_STATUS ucfg_policy_mgr_get_mcc_adaptive_sch(struct wlan_objmgr_psoc *psoc,
						uint8_t *mcc_adaptive_sch)
{
	return policy_mgr_get_mcc_adaptive_sch(psoc, mcc_adaptive_sch);
}

QDF_STATUS ucfg_policy_mgr_get_sta_cxn_5g_band(struct wlan_objmgr_psoc *psoc,
					       uint8_t *enable_sta_cxn_5g_band)
{
	return policy_mgr_get_sta_cxn_5g_band(psoc, enable_sta_cxn_5g_band);
}
