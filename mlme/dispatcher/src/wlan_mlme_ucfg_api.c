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
 * DOC: define UCFG APIs exposed by the mlme component
 */

#include "cfg_ucfg_api.h"
#include "cfg_mlme_sta.h"
#include "wlan_mlme_main.h"
#include "wlan_mlme_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_mlme_vdev_mgr_interface.h"

#ifdef CONFIG_VDEV_SM
static QDF_STATUS ucfg_mlme_vdev_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ucfg_mlme_vdev_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

#else
static QDF_STATUS ucfg_mlme_vdev_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_vdev_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_vdev_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("unable to register vdev create handle");
		return status;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_vdev_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to register vdev create handle");

	return status;

}

static QDF_STATUS ucfg_mlme_vdev_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_vdev_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_vdev_object_destroyed_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister vdev destroy handle");

	status = wlan_objmgr_unregister_vdev_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_vdev_object_created_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister vdev create handle");

	return status;
}

#endif

QDF_STATUS ucfg_mlme_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_created_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("unable to register psoc create handle");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_destroyed_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("unable to register psoc create handle");
		return status;
	}
	status = ucfg_mlme_vdev_init();

	return status;
}

QDF_STATUS ucfg_mlme_deinit(void)
{
	QDF_STATUS status;

	status = ucfg_mlme_vdev_deinit();
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister vdev destroy handle");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister psoc destroy handle");

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_created_notification,
			NULL);

	if (status != QDF_STATUS_SUCCESS)
		mlme_err("unable to unregister psoc create handle");

	return status;
}

QDF_STATUS ucfg_mlme_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = mlme_cfg_on_psoc_enable(psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		mlme_err("Failed to initialize MLME CFG");

	return status;
}

void ucfg_mlme_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	/* Clear the MLME CFG Structure */
}

#ifdef CONFIG_VDEV_SM
QDF_STATUS ucfg_mlme_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_mlme.mlme_register_ops = mlme_register_vdev_mgr_ops;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_mlme_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
ucfg_mlme_get_sta_keep_alive_period(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_INFRA_STA_KEEP_ALIVE_PERIOD);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.sta_keep_alive_period;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_dfs_master_capability(struct wlan_objmgr_psoc *psoc,
				    bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_ENABLE_DFS_MASTER_CAPABILITY);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.dfs_cfg.dfs_master_capable;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_dfs_disable_channel_switch(struct wlan_objmgr_psoc *psoc,
					 bool *dfs_disable_channel_switch)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*dfs_disable_channel_switch =
			cfg_default(CFG_DISABLE_DFS_CH_SWITCH);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*dfs_disable_channel_switch =
		mlme_obj->cfg.dfs_cfg.dfs_disable_channel_switch;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_dfs_disable_channel_switch(struct wlan_objmgr_psoc *psoc,
					 bool dfs_disable_channel_switch)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.dfs_cfg.dfs_disable_channel_switch =
		dfs_disable_channel_switch;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_dfs_ignore_cac(struct wlan_objmgr_psoc *psoc,
			     bool *dfs_ignore_cac)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*dfs_ignore_cac = cfg_default(CFG_IGNORE_CAC);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*dfs_ignore_cac = mlme_obj->cfg.dfs_cfg.dfs_ignore_cac;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_dfs_ignore_cac(struct wlan_objmgr_psoc *psoc,
			     bool dfs_ignore_cac)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.dfs_cfg.dfs_ignore_cac = dfs_ignore_cac;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_sap_tx_leakage_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t *sap_tx_leakage_threshold)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*sap_tx_leakage_threshold =
			cfg_default(CFG_SAP_TX_LEAKAGE_THRESHOLD);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*sap_tx_leakage_threshold =
		mlme_obj->cfg.dfs_cfg.sap_tx_leakage_threshold;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_sap_tx_leakage_threshold(struct wlan_objmgr_psoc *psoc,
				       uint32_t sap_tx_leakage_threshold)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.dfs_cfg.sap_tx_leakage_threshold =
		sap_tx_leakage_threshold;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_dfs_filter_offload(struct wlan_objmgr_psoc *psoc,
				 bool *dfs_filter_offload)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*dfs_filter_offload =
			cfg_default(CFG_ENABLE_DFS_PHYERR_FILTEROFFLOAD);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*dfs_filter_offload = mlme_obj->cfg.dfs_cfg.dfs_filter_offload;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_dfs_filter_offload(struct wlan_objmgr_psoc *psoc,
				 bool dfs_filter_offload)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.dfs_cfg.dfs_filter_offload = dfs_filter_offload;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_pmkid_modes(struct wlan_objmgr_psoc *psoc,
			  uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_PMKID_MODES);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.pmkid_modes;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_pmkid_modes(struct wlan_objmgr_psoc *psoc,
			  uint32_t val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.sta.pmkid_modes = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_TWT_REQUESTOR);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.twt_cfg.is_twt_requestor_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.twt_cfg.is_twt_requestor_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_TWT_RESPONDER);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.twt_cfg.is_twt_responder_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.twt_cfg.is_twt_responder_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_BCAST_TWT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.twt_cfg.is_twt_bcast_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.twt_cfg.is_twt_bcast_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_TWT_CONGESTION_TIMEOUT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.twt_cfg.twt_congestion_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.twt_cfg.twt_congestion_timeout = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_enable_twt(struct wlan_objmgr_psoc *psoc,
			 bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_ENABLE_TWT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.twt_cfg.is_twt_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_enable_twt(struct wlan_objmgr_psoc *psoc,
			 bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.twt_cfg.is_twt_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_dot11p_mode(struct wlan_objmgr_psoc *psoc,
			  enum dot11p_mode *out_mode)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*out_mode = cfg_default(CFG_DOT11P_MODE);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*out_mode = mlme_obj->cfg.sta.dot11p_mode;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_go_cts2self_for_sta(struct wlan_objmgr_psoc *psoc,
				  bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_ENABLE_GO_CTS2SELF_FOR_STA);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.enable_go_cts2self_for_sta;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_force_rsne_override(struct wlan_objmgr_psoc *psoc,
				  bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_FORCE_RSNE_OVERRIDE);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.force_rsne_override;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_qcn_ie_support(struct wlan_objmgr_psoc *psoc,
			     bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_QCN_IE_SUPPORT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.qcn_ie_support;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_tgt_gtx_usr_cfg(struct wlan_objmgr_psoc *psoc,
			      uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_TGT_GTX_USR_CFG);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.sta.tgt_gtx_usr_cfg;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_override_ht20_40_24g(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_OBSS_HT40_OVERRIDE_HT40_20_24GHZ);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}
	*val = mlme_obj->cfg.obss_ht40.is_override_ht20_40_24g;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
ucfg_mlme_get_roaming_offload(struct wlan_objmgr_psoc *psoc,
			      bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR3_ROAMING_OFFLOAD);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.lfr3_roaming_offload;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_roaming_offload(struct wlan_objmgr_psoc *psoc,
			      bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.lfr3_roaming_offload = val;

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
ucfg_mlme_get_first_scan_bucket_threshold(struct wlan_objmgr_psoc *psoc,
					  uint8_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_FIRST_SCAN_BUCKET_THRESHOLD);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.first_scan_bucket_threshold;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_mawc_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_MAWC_FEATURE_ENABLED);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}
	*val = mlme_obj->cfg.lfr.mawc_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_mawc_enabled(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.mawc_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_fast_transition_enabled(struct wlan_objmgr_psoc *psoc,
				     bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_FAST_TRANSITION_ENABLED);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.fast_transition_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_fast_transition_enabled(struct wlan_objmgr_psoc *psoc,
				      bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.fast_transition_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_roam_scan_offload_enabled(struct wlan_objmgr_psoc *psoc,
				       bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ROAM_SCAN_OFFLOAD_ENABLED);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.roam_scan_offload_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_roam_scan_offload_enabled(struct wlan_objmgr_psoc *psoc,
					bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.roam_scan_offload_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_neighbor_scan_max_chan_time(struct wlan_objmgr_psoc *psoc,
					  uint16_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_NEIGHBOR_SCAN_MAX_CHAN_TIME);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.neighbor_scan_max_chan_time;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_neighbor_scan_min_chan_time(struct wlan_objmgr_psoc *psoc,
					  uint16_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_NEIGHBOR_SCAN_MIN_CHAN_TIME);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.neighbor_scan_min_chan_time;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_delay_before_vdev_stop(struct wlan_objmgr_psoc *psoc,
				     uint8_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_DELAY_BEFORE_VDEV_STOP);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.delay_before_vdev_stop;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_roam_bmiss_final_bcnt(struct wlan_objmgr_psoc *psoc,
				    uint8_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ROAM_BMISS_FINAL_BCNT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.roam_bmiss_final_bcnt;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_roam_bmiss_first_bcnt(struct wlan_objmgr_psoc *psoc,
				    uint8_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ROAM_BMISS_FIRST_BCNT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.roam_bmiss_first_bcnt;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_lfr_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_FEATURE_ENABLED);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.lfr_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_lfr_enabled(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.lfr_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_is_roam_prefer_5ghz(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ROAM_PREFER_5GHZ);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.roam_prefer_5ghz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_roam_intra_band(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.roam_intra_band = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_home_away_time(struct wlan_objmgr_psoc *psoc, uint16_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ROAM_SCAN_HOME_AWAY_TIME);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.roam_scan_home_away_time;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_fast_roam_in_concurrency_enabled(struct wlan_objmgr_psoc *psoc,
					       bool val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.lfr.enable_fast_roam_in_concurrency = val;

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_ESE
QDF_STATUS
ucfg_mlme_is_ese_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR_ESE_FEATURE_ENABLED);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.lfr.ese_enabled;

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_ESE */

QDF_STATUS
ucfg_mlme_get_opr_rate_set(struct wlan_objmgr_psoc *psoc,
			   uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_mlme_get_cfg_str(buf, &mlme_obj->cfg.rates.opr_rate_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_ext_opr_rate_set(struct wlan_objmgr_psoc *psoc,
			       uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_mlme_get_cfg_str(buf, &mlme_obj->cfg.rates.ext_opr_rate_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_supported_mcs_set(struct wlan_objmgr_psoc *psoc,
				uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_mlme_get_cfg_str(buf,
				     &mlme_obj->cfg.rates.supported_mcs_set,
				     len);
}

QDF_STATUS
ucfg_mlme_set_supported_mcs_set(struct wlan_objmgr_psoc *psoc,
				uint8_t *buf, qdf_size_t len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_mlme_set_cfg_str(buf,
				     &mlme_obj->cfg.rates.supported_mcs_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_current_mcs_set(struct wlan_objmgr_psoc *psoc,
			      uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_mlme_get_cfg_str(buf,
				     &mlme_obj->cfg.rates.current_mcs_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_wmi_wq_watchdog_timeout(struct wlan_objmgr_psoc *psoc,
				      uint32_t *wmi_wq_watchdog_timeout)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*wmi_wq_watchdog_timeout = cfg_default(CFG_WMI_WQ_WATCHDOG);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*wmi_wq_watchdog_timeout =
		mlme_obj->cfg.timeouts.wmi_wq_watchdog_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_wmi_wq_watchdog_timeout(struct wlan_objmgr_psoc *psoc,
				      uint32_t wmi_wq_watchdog_timeout)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	if (!cfg_in_range(CFG_WMI_WQ_WATCHDOG, wmi_wq_watchdog_timeout)) {
		mlme_err("wmi watchdog bite timeout is invalid %d",
			 wmi_wq_watchdog_timeout);
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj->cfg.timeouts.wmi_wq_watchdog_timeout =
		wmi_wq_watchdog_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_ps_data_inactivity_timeout(struct wlan_objmgr_psoc *psoc,
					 uint32_t *inactivity_timeout)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*inactivity_timeout =
			cfg_default(CFG_PS_DATA_INACTIVITY_TIMEOUT);
	}
	*inactivity_timeout = mlme_obj->cfg.timeouts.ps_data_inactivity_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_ps_data_inactivity_timeout(struct wlan_objmgr_psoc *psoc,
					 uint32_t inactivity_timeout)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	if (!cfg_in_range(CFG_PS_DATA_INACTIVITY_TIMEOUT, inactivity_timeout)) {
		mlme_err("inactivity timeout set value is invalid %d",
			 inactivity_timeout);
		return QDF_STATUS_E_INVAL;
	}
	mlme_obj->cfg.timeouts.ps_data_inactivity_timeout = inactivity_timeout;

	return QDF_STATUS_SUCCESS;
}
