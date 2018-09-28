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
#ifndef __WLAN_POLICY_MGR_UCFG
#define __WLAN_POLICY_MGR_UCFG
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_global_obj.h"
#include "qdf_status.h"


/**
 * ucfg_policy_mgr_psoc_open() - This API sets CFGs to policy manager context
 *
 * This API pulls policy manager's context from PSOC and initialize the CFG
 * structure of policy manager.
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_psoc_open(struct wlan_objmgr_psoc *psoc);
/**
 * ucfg_policy_mgr_psoc_close() - This API resets CFGs for policy manager ctx
 *
 * This API pulls policy manager's context from PSOC and resets the CFG
 * structure of policy manager.
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
void ucfg_policy_mgr_psoc_close(struct wlan_objmgr_psoc *psoc);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * ucfg_policy_mgr_get_mcc_scc_switch() - To mcc to scc switch setting from INI
 * @psoc: pointer to psoc
 * @mcc_scc_switch: value to be filled
 *
 * This API pulls mcc to scc switch setting which is given as part of INI and
 * stored in policy manager's CFGs.
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_mcc_scc_switch(struct wlan_objmgr_psoc *psoc,
					      uint8_t *mcc_scc_switch);
#else
static inline
QDF_STATUS ucfg_policy_mgr_get_mcc_scc_switch(struct wlan_objmgr_psoc *psoc,
					      uint8_t *mcc_scc_switch)
{
	return QDF_STATUS_SUCCESS;
}
#endif //FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * ucfg_policy_mgr_get_sys_pref() - to get system preference
 * @psoc: pointer to psoc
 * @sys_pref: value to be filled
 *
 * This API pulls the system preference for policy manager to provide
 * PCL
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_sys_pref(struct wlan_objmgr_psoc *psoc,
					uint8_t *sys_pref);
/**
 * ucfg_policy_mgr_set_sys_pref() - to set system preference
 * @psoc: pointer to psoc
 * @sys_pref: value to be applied as new INI setting
 *
 * This API is meant to override original INI setting for system pref
 * with new value which is used by policy manager to provide PCL
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_set_sys_pref(struct wlan_objmgr_psoc *psoc,
					uint8_t sys_pref);
/**
 * ucfg_policy_mgr_get_max_conc_cxns() - to get max num of conc connections
 * @psoc: pointer to psoc
 * @max_conc_cxns: value to be filled
 *
 * This API pulls max number of active connections which can be allowed
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_max_conc_cxns(struct wlan_objmgr_psoc *psoc,
						uint8_t *max_conc_cxns);
/**
 * ucfg_policy_mgr_get_conc_rule1() - to find out if conc rule1 is enabled
 * @psoc: pointer to psoc
 * @conc_rule1: value to be filled
 *
 * This API is used to find out if conc rule-1 is enabled by user
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_conc_rule1(struct wlan_objmgr_psoc *psoc,
						uint8_t *conc_rule1);
/**
 * ucfg_policy_mgr_get_conc_rule2() - to find out if conc rule2 is enabled
 * @psoc: pointer to psoc
 * @conc_rule2: value to be filled
 *
 * This API is used to find out if conc rule-2 is enabled by user
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_conc_rule2(struct wlan_objmgr_psoc *psoc,
						uint8_t *conc_rule2);
/**
 * ucfg_policy_mgr_get_dbs_selection_plcy() - DBS HW mode selection setting
 * @psoc: pointer to psoc
 * @dbs_selection_plcy: value to be filled
 *
 * This API is used to find out DBS HW mode preference.
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_dbs_selection_plcy(struct wlan_objmgr_psoc *psoc,
						uint32_t *dbs_selection_plcy);
/**
 * ucfg_policy_mgr_get_vdev_priority_list() - to get vdev priority list
 * @psoc: pointer to psoc
 * @vdev_priority_list: value to be filled
 *
 * This API is used to find out vdev_priority_list setting
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_vdev_priority_list(struct wlan_objmgr_psoc *psoc,
						uint32_t *vdev_priority_list);
/**
 * policy_mgr_get_chnl_select_plcy() - to get channel selection policy
 * @psoc: pointer to psoc
 * @chnl_select_plcy: value to be filled
 *
 * This API is used to find out which channel selection policy has been
 * configured
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_chnl_select_plcy(struct wlan_objmgr_psoc *psoc,
						uint32_t *chnl_select_plcy);
/**
 * policy_mgr_get_mcc_adaptive_sch() - to get mcc adaptive scheduler
 * @psoc: pointer to psoc
 * @enable_mcc_adaptive_sch: value to be filled
 *
 * This API is used to find out if mcc adaptive scheduler enabled or disabled
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS
ucfg_policy_mgr_get_mcc_adaptive_sch(struct wlan_objmgr_psoc *psoc,
				     uint8_t *enable_mcc_adaptive_sch);

/**
 * ucfg_policy_mgr_get_sta_cxn_5g_band() - to get STA's connection in 5G config
 *
 * @psoc: pointer to psoc
 * @enable_sta_cxn_5g_band: value to be filled
 *
 * This API is used to find out if STA connection in 5G band is allowed or
 * disallowed.
 *
 * Return: QDF_STATUS_SUCCESS up on success and any other status for failure.
 */
QDF_STATUS ucfg_policy_mgr_get_sta_cxn_5g_band(struct wlan_objmgr_psoc *psoc,
					       uint8_t *enable_sta_cxn_5g_band);
#endif //__WLAN_POLICY_MGR_UCFG
