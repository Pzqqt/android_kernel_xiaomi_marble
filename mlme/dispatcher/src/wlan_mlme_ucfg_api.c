/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
#include <include/wlan_pdev_mlme.h>
#include "wlan_pdev_mlme_api.h"
#endif

#ifdef CONFIG_VDEV_SM
static QDF_STATUS ucfg_mlme_vdev_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ucfg_mlme_vdev_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_mlme_global_init(void)
{
	mlme_register_mlme_ext_ops();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_mlme_global_deinit(void)
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
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = wlan_objmgr_register_peer_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_peer_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("peer create register notification failed");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_register_peer_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_peer_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("peer destroy register notification failed");
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

QDF_STATUS ucfg_mlme_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_peer_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_peer_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister peer destroy handle");

	status = wlan_objmgr_unregister_peer_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_peer_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("unable to unregister peer create handle");

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
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	pdev_mlme->mlme_register_ops = mlme_register_vdev_mgr_ops;

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
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.dfs_cfg.dfs_master_capable;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_fine_time_meas_cap(struct wlan_objmgr_psoc *psoc,
				 uint32_t *fine_time_meas_cap)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*fine_time_meas_cap =
			cfg_default(CFG_FINE_TIME_MEAS_CAPABILITY);
		return QDF_STATUS_E_INVAL;
	}

	*fine_time_meas_cap = mlme_obj->cfg.wifi_pos_cfg.fine_time_meas_cap;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_fine_time_meas_cap(struct wlan_objmgr_psoc *psoc,
				 uint32_t fine_time_meas_cap)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.wifi_pos_cfg.fine_time_meas_cap = fine_time_meas_cap;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	return wlan_mlme_get_cfg_str(buf, &mlme_obj->cfg.rates.opr_rate_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_ext_opr_rate_set(struct wlan_objmgr_psoc *psoc,
			       uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	return wlan_mlme_get_cfg_str(buf, &mlme_obj->cfg.rates.ext_opr_rate_set,
				     len);
}

QDF_STATUS
ucfg_mlme_get_supported_mcs_set(struct wlan_objmgr_psoc *psoc,
				uint8_t *buf, qdf_size_t *len)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

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
ucfg_mlme_stats_get_periodic_display_time(struct wlan_objmgr_psoc *psoc,
					  uint32_t *periodic_display_time)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*periodic_display_time =
			cfg_default(CFG_PERIODIC_STATS_DISPLAY_TIME);
		return QDF_STATUS_E_INVAL;
	}

	*periodic_display_time =
		mlme_obj->cfg.stats.stats_periodic_display_time;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_stats_get_cfg_values(struct wlan_objmgr_psoc *psoc,
			       int *link_speed_rssi_high,
			       int *link_speed_rssi_mid,
			       int *link_speed_rssi_low,
			       uint32_t *link_speed_rssi_report)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*link_speed_rssi_high =
			cfg_default(CFG_LINK_SPEED_RSSI_HIGH);
		*link_speed_rssi_mid =
			cfg_default(CFG_LINK_SPEED_RSSI_MID);
		*link_speed_rssi_low =
			cfg_default(CFG_LINK_SPEED_RSSI_LOW);
		*link_speed_rssi_report =
			cfg_default(CFG_REPORT_MAX_LINK_SPEED);
		return QDF_STATUS_E_INVAL;
	}

	*link_speed_rssi_high =
		mlme_obj->cfg.stats.stats_link_speed_rssi_high;
	*link_speed_rssi_mid =
		mlme_obj->cfg.stats.stats_link_speed_rssi_med;
	*link_speed_rssi_low =
		mlme_obj->cfg.stats.stats_link_speed_rssi_low;
	*link_speed_rssi_report =
		mlme_obj->cfg.stats.stats_report_max_link_speed_rssi;

	return QDF_STATUS_SUCCESS;
}

bool ucfg_mlme_stats_is_link_speed_report_actual(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	int report_link_speed = 0;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		report_link_speed = cfg_default(CFG_REPORT_MAX_LINK_SPEED);
	else
		report_link_speed =
			mlme_obj->cfg.stats.stats_report_max_link_speed_rssi;

	return (report_link_speed == CFG_STATS_LINK_SPEED_REPORT_ACTUAL);
}

bool ucfg_mlme_stats_is_link_speed_report_max(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	int report_link_speed = 0;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		report_link_speed = cfg_default(CFG_REPORT_MAX_LINK_SPEED);
	else
		report_link_speed =
			mlme_obj->cfg.stats.stats_report_max_link_speed_rssi;

	return (report_link_speed == CFG_STATS_LINK_SPEED_REPORT_MAX);
}

bool
ucfg_mlme_stats_is_link_speed_report_max_scaled(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	int report_link_speed = 0;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		report_link_speed = cfg_default(CFG_REPORT_MAX_LINK_SPEED);
	else
		report_link_speed =
			mlme_obj->cfg.stats.stats_report_max_link_speed_rssi;

	return (report_link_speed == CFG_STATS_LINK_SPEED_REPORT_MAX_SCALED);
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
		return QDF_STATUS_E_FAILURE;
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
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	if (!cfg_in_range(CFG_PS_DATA_INACTIVITY_TIMEOUT, inactivity_timeout)) {
		mlme_err("inactivity timeout set value is invalid %d",
			 inactivity_timeout);
		return QDF_STATUS_E_INVAL;
	}
	mlme_obj->cfg.timeouts.ps_data_inactivity_timeout = inactivity_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_sta_keepalive_method(struct wlan_objmgr_psoc *psoc,
				   enum station_keepalive_method *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	*val = mlme_obj->cfg.sta.sta_keepalive_method;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_enable_deauth_to_disassoc_map(struct wlan_objmgr_psoc *psoc,
					    bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	*value = mlme_obj->cfg.gen.enable_deauth_to_disassoc_map;
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS
ucfg_mlme_get_ap_random_bssid_enable(struct wlan_objmgr_psoc *psoc,
				     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	*value = mlme_obj->cfg.sap_cfg.ap_random_bssid_enable;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_latency_enable(struct wlan_objmgr_psoc *psoc, bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.wlm_config.latency_enable;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_mlme_get_ibss_cfg(struct wlan_objmgr_psoc *psoc,
				  struct wlan_mlme_ibss_cfg *ibss_cfg)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	if (!ibss_cfg)
		return QDF_STATUS_E_FAILURE;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("MLME Obj null on get IBSS config");
		mlme_init_ibss_cfg(psoc, ibss_cfg);
		return QDF_STATUS_E_INVAL;
	}
	*ibss_cfg = mlme_obj->cfg.ibss;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_mlme_set_ibss_auto_bssid(struct wlan_objmgr_psoc *psoc,
					 uint32_t auto_bssid)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("MLME Obj null on get IBSS config");
		return QDF_STATUS_E_INVAL;
	}
	mlme_obj->cfg.ibss.auto_bssid = auto_bssid;
	return QDF_STATUS_SUCCESS;
}

#ifdef MWS_COEX
QDF_STATUS
ucfg_mlme_get_mws_coex_4g_quick_tdm(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_MWS_COEX_4G_QUICK_FTDM);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.mwc.mws_coex_4g_quick_tdm;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_mws_coex_5g_nr_pwr_limit(struct wlan_objmgr_psoc *psoc,
				       uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_MWS_COEX_5G_NR_PWR_LIMIT);
		mlme_err("mlme obj null");
		return QDF_STATUS_E_INVAL;
	}

	*val = mlme_obj->cfg.mwc.mws_coex_5g_nr_pwr_limit;

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
ucfg_mlme_get_etsi13_srd_chan_in_master_mode(struct wlan_objmgr_psoc *psoc,
					     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_ETSI13_SRD_CHAN_IN_MASTER_MODE);
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.reg.etsi13_srd_chan_in_master_mode;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_11d_in_world_mode(struct wlan_objmgr_psoc *psoc,
				bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_ENABLE_11D_IN_WORLD_MODE);
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.reg.enable_11d_in_world_mode;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_restart_beaconing_on_ch_avoid(struct wlan_objmgr_psoc *psoc,
					    uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_RESTART_BEACONING_ON_CH_AVOID);
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.reg.restart_beaconing_on_ch_avoid;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_indoor_channel_support(struct wlan_objmgr_psoc *psoc,
				     bool *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_INDOOR_CHANNEL_SUPPORT);
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.reg.indoor_channel_support;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_scan_11d_interval(struct wlan_objmgr_psoc *psoc,
				uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_SCAN_11D_INTERVAL);
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.reg.scan_11d_interval;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_valid_channel_list(struct wlan_objmgr_psoc *psoc,
				 uint8_t *channel_list,
				 uint32_t *channel_list_num)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	qdf_size_t valid_channel_list_num = 0;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		qdf_uint8_array_parse(cfg_default(CFG_VALID_CHANNEL_LIST),
				      channel_list,
				      CFG_VALID_CHANNEL_LIST_LEN,
				      &valid_channel_list_num);
		*channel_list_num = (uint8_t)valid_channel_list_num;
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_INVAL;
	}

	*channel_list_num = (uint32_t)mlme_obj->cfg.reg.valid_channel_list_num;
	qdf_mem_copy(channel_list, mlme_obj->cfg.reg.valid_channel_list,
		     *channel_list_num);

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS
ucfg_mlme_is_subnet_detection_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_LFR3_ENABLE_SUBNET_DETECTION);
		return QDF_STATUS_E_INVAL;
	}
	*val = mlme_obj->cfg.lfr.enable_lfr_subnet_detection;

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
ucfg_mlme_set_current_tx_power_level(struct wlan_objmgr_psoc *psoc,
				     uint8_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.power.current_tx_power_level = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_current_tx_power_level(struct wlan_objmgr_psoc *psoc,
				     uint8_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_CURRENT_TX_POWER_LEVEL);
		return QDF_STATUS_E_INVAL;
	}

	*value = mlme_obj->cfg.power.current_tx_power_level;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_obss_detection_offload_enabled(struct wlan_objmgr_psoc *psoc,
					     uint8_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.obss_ht40.obss_detection_offload_enabled = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_obss_color_collision_offload_enabled(
		struct wlan_objmgr_psoc *psoc, uint8_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.obss_ht40.obss_color_collision_offload_enabled = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_channel_bonding_24ghz(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*val = cfg_default(CFG_CHANNEL_BONDING_MODE_24GHZ);
		return QDF_STATUS_E_INVAL;
	}
	*val = mlme_obj->cfg.feature_flags.channel_bonding_mode_24ghz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_channel_bonding_24ghz(struct wlan_objmgr_psoc *psoc,
				    uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.feature_flags.channel_bonding_mode_24ghz = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_get_channel_bonding_5ghz(struct wlan_objmgr_psoc *psoc,
				   uint32_t *value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		*value = cfg_default(CFG_CHANNEL_BONDING_MODE_5GHZ);
		return QDF_STATUS_E_INVAL;
	}
	*value = mlme_obj->cfg.feature_flags.channel_bonding_mode_5ghz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_mlme_set_channel_bonding_5ghz(struct wlan_objmgr_psoc *psoc,
				   uint32_t value)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	mlme_obj->cfg.feature_flags.channel_bonding_mode_5ghz = value;

	return QDF_STATUS_SUCCESS;
}

