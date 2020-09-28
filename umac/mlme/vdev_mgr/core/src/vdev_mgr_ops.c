/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: vdev_mgr_ops.c
 *
 * This file provide API definitions for filling data structures
 * and sending vdev mgmt commands to target_if/mlme
 */
#include "vdev_mgr_ops.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_vdev_mlme_api.h>
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mgr_tgt_if_tx_api.h>
#include <target_if.h>
#include <init_deinit_lmac.h>
#include <wlan_lmac_if_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_dfs_tgt_api.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_vdev_mgr_ucfg_api.h>
#include <qdf_module.h>

static QDF_STATUS vdev_mgr_create_param_update(
					struct vdev_mlme_obj *mlme_obj,
					struct vdev_create_params *param)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_mbss_11ax *mbss;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("PDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mbss = &mlme_obj->mgmt.mbss_11ax;
	param->pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	param->vdev_id = wlan_vdev_get_id(vdev);
	param->nss_2g = mlme_obj->proto.generic.nss_2g;
	param->nss_5g = mlme_obj->proto.generic.nss_5g;
	param->type = mlme_obj->mgmt.generic.type;
	param->subtype = mlme_obj->mgmt.generic.subtype;
	param->mbssid_flags = mbss->mbssid_flags;
	param->vdevid_trans = mbss->vdevid_trans;
	param->special_vdev_mode = mlme_obj->mgmt.generic.special_vdev_mode;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_create_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct vdev_create_params param = {0};

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_create_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_create_send(mlme_obj, &param);

	return status;
}

#ifdef QCA_MCL_DFS_SUPPORT
static bool vdev_mgr_is_opmode_sap_or_p2p_go(enum QDF_OPMODE op_mode)
{
	return (op_mode == QDF_SAP_MODE || op_mode == QDF_P2P_GO_MODE);
}

static bool vdev_mgr_is_49G_5G_6G_chan_freq(uint16_t chan_freq)
{
	return WLAN_REG_IS_5GHZ_CH_FREQ(chan_freq) ||
		WLAN_REG_IS_49GHZ_FREQ(chan_freq) ||
		WLAN_REG_IS_6GHZ_CHAN_FREQ(chan_freq);
}
#else
static inline bool vdev_mgr_is_opmode_sap_or_p2p_go(enum QDF_OPMODE op_mode)
{
	return true;
}

static inline bool vdev_mgr_is_49G_5G_6G_chan_freq(uint16_t chan_freq)
{
	return true;
}
#endif

static QDF_STATUS vdev_mgr_start_param_update(
					struct vdev_mlme_obj *mlme_obj,
					struct vdev_start_params *param)
{
	struct wlan_channel *des_chan;
	uint32_t dfs_reg;
	bool set_agile = false, dfs_set_cfreq2 = false;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	enum QDF_OPMODE op_mode;
	bool is_dfs_chan_updated = false;
	struct vdev_mlme_mbss_11ax *mbss;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("PDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_MLME_SB_ID) !=
							QDF_STATUS_SUCCESS) {
		mlme_err("Failed to get pdev reference");
		return QDF_STATUS_E_FAILURE;
	}

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);
	param->vdev_id = wlan_vdev_get_id(vdev);

	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (vdev_mgr_is_opmode_sap_or_p2p_go(op_mode) &&
	    vdev_mgr_is_49G_5G_6G_chan_freq(des_chan->ch_freq)) {
		tgt_dfs_set_current_channel_for_freq(pdev, des_chan->ch_freq,
						     des_chan->ch_flags,
						     des_chan->ch_flagext,
						     des_chan->ch_ieee,
						     des_chan->ch_freq_seg1,
						     des_chan->ch_freq_seg2,
						     des_chan->ch_cfreq1,
						     des_chan->ch_cfreq2,
						     &is_dfs_chan_updated);
		if (des_chan->ch_cfreq2)
			param->channel.dfs_set_cfreq2 =
				utils_is_dfs_cfreq2_ch(pdev);
	}

	/* The Agile state machine should be stopped only once for the channel
	 * change. If  the same channel is being sent to the FW then do
	 * not send unnecessary STOP to the state machine.
	 */
	if (is_dfs_chan_updated)
		utils_dfs_agile_sm_deliver_evt(pdev,
					       DFS_AGILE_SM_EV_AGILE_STOP);

	param->beacon_interval = mlme_obj->proto.generic.beacon_interval;
	param->dtim_period = mlme_obj->proto.generic.dtim_period;
	param->disable_hw_ack = mlme_obj->mgmt.generic.disable_hw_ack;
	param->preferred_rx_streams =
		mlme_obj->mgmt.chainmask_info.num_rx_chain;
	param->preferred_tx_streams =
		mlme_obj->mgmt.chainmask_info.num_tx_chain;

	wlan_reg_get_dfs_region(pdev, &dfs_reg);
	param->regdomain = dfs_reg;
	param->he_ops = mlme_obj->proto.he_ops_info.he_ops;

	param->channel.chan_id = des_chan->ch_ieee;
	param->channel.pwr = mlme_obj->mgmt.generic.tx_power;
	param->channel.mhz = des_chan->ch_freq;
	param->channel.half_rate = mlme_obj->mgmt.rate_info.half_rate;
	param->channel.quarter_rate = mlme_obj->mgmt.rate_info.quarter_rate;
	param->channel.dfs_set = wlan_reg_is_dfs_for_freq(pdev,
							  des_chan->ch_freq);
	param->channel.is_chan_passive =
		utils_is_dfs_chan_for_freq(pdev, param->channel.mhz);
	param->channel.allow_ht = mlme_obj->proto.ht_info.allow_ht;
	param->channel.allow_vht = mlme_obj->proto.vht_info.allow_vht;
	param->channel.phy_mode = mlme_obj->mgmt.generic.phy_mode;
	param->channel.cfreq1 = des_chan->ch_cfreq1;
	param->channel.cfreq2 = des_chan->ch_cfreq2;
	param->channel.maxpower = mlme_obj->mgmt.generic.maxpower;
	param->channel.minpower = mlme_obj->mgmt.generic.minpower;
	param->channel.maxregpower = mlme_obj->mgmt.generic.maxregpower;
	param->channel.antennamax = mlme_obj->mgmt.generic.antennamax;
	param->channel.reg_class_id = mlme_obj->mgmt.generic.reg_class_id;
	param->bcn_tx_rate_code = vdev_mgr_fetch_ratecode(mlme_obj);
	param->ldpc_rx_enabled = mlme_obj->proto.generic.ldpc;

	mbss = &mlme_obj->mgmt.mbss_11ax;
	param->mbssid_flags = mbss->mbssid_flags;
	param->vdevid_trans = mbss->vdevid_trans;

	if (mlme_obj->mgmt.generic.type == WLAN_VDEV_MLME_TYPE_AP) {
		param->hidden_ssid = mlme_obj->mgmt.ap.hidden_ssid;
		param->cac_duration_ms = mlme_obj->mgmt.ap.cac_duration_ms;
	}
	wlan_vdev_mlme_get_ssid(vdev, param->ssid.ssid, &param->ssid.length);

	if (des_chan->ch_phymode == WLAN_PHYMODE_11AC_VHT80 ||
	    des_chan->ch_phymode == WLAN_PHYMODE_11AXA_HE80) {
		tgt_dfs_find_vht80_precac_chan_freq(pdev,
						    des_chan->ch_phymode,
						    des_chan->ch_freq_seg1,
						    &param->channel.cfreq1,
						    &param->channel.cfreq2,
						    &param->channel.phy_mode,
						    &dfs_set_cfreq2,
						    &set_agile);
		param->channel.dfs_set_cfreq2 = dfs_set_cfreq2;
		param->channel.set_agile = set_agile;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_SB_ID);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_start_send(
			struct vdev_mlme_obj *mlme_obj,
			bool restart)
{
	QDF_STATUS status;
	struct vdev_start_params param = {0};

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_start_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	param.is_restart = restart;
	status = tgt_vdev_mgr_start_send(mlme_obj, &param);

	return status;
}

static QDF_STATUS vdev_mgr_delete_param_update(
					struct vdev_mlme_obj *mlme_obj,
					struct vdev_delete_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = wlan_vdev_get_id(vdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_delete_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct vdev_delete_params param;

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_delete_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_delete_send(mlme_obj, &param);

	return status;
}

static QDF_STATUS vdev_mgr_stop_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_stop_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = wlan_vdev_get_id(vdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_stop_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct vdev_stop_params param = {0};

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_stop_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_stop_send(mlme_obj, &param);

	return status;
}

static QDF_STATUS vdev_mgr_bcn_tmpl_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct beacon_tmpl_params *param)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS vdev_mgr_sta_ps_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct sta_ps_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	param->vdev_id = wlan_vdev_get_id(vdev);
	param->param_id = WLAN_MLME_CFG_UAPSD;
	param->value = mlme_obj->proto.sta.uapsd_cfg;
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS vdev_mgr_up_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_up_params *param)
{
	struct vdev_mlme_mbss_11ax *mbss;
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	param->vdev_id = wlan_vdev_get_id(vdev);
	param->assoc_id = mlme_obj->proto.sta.assoc_id;
	mbss = &mlme_obj->mgmt.mbss_11ax;
	if (mbss->profile_idx) {
		param->profile_idx = mbss->profile_idx;
		param->profile_num = mbss->profile_num;
		qdf_mem_copy(param->trans_bssid, mbss->trans_bssid,
			     QDF_MAC_ADDR_SIZE);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_up_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct vdev_up_params param = {0};
	struct sta_ps_params ps_param = {0};
	struct beacon_tmpl_params bcn_tmpl_param = {0};
	enum QDF_OPMODE opmode;
	struct wlan_objmgr_vdev *vdev;
	struct config_fils_params fils_param = {0};
	uint8_t is_6g_sap_fd_enabled;
	bool is_non_tx_vdev;

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev_mgr_up_param_update(mlme_obj, &param);
	vdev_mgr_bcn_tmpl_param_update(mlme_obj, &bcn_tmpl_param);

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode == QDF_STA_MODE) {
		vdev_mgr_sta_ps_param_update(mlme_obj, &ps_param);
		status = tgt_vdev_mgr_sta_ps_param_send(mlme_obj, &ps_param);

	}

	status = tgt_vdev_mgr_beacon_tmpl_send(mlme_obj, &bcn_tmpl_param);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = tgt_vdev_mgr_up_send(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	is_6g_sap_fd_enabled = wlan_vdev_mlme_feat_ext_cap_get(vdev,
					WLAN_VDEV_FEXT_FILS_DISC_6G_SAP);
	mlme_debug("SAP FD enabled %d", is_6g_sap_fd_enabled);

	/*
	 * In case of a non-tx vdev, 'profile_num' must be greater
	 * than 0 indicating one or more non-tx vdev and 'profile_idx'
	 * must be in the range [1, 2^n] where n is the max bssid
	 * indicator
	 */
	is_non_tx_vdev = param.profile_num && param.profile_idx;

	if (opmode == QDF_SAP_MODE && mlme_obj->vdev->vdev_mlme.des_chan &&
	    WLAN_REG_IS_6GHZ_CHAN_FREQ(
			mlme_obj->vdev->vdev_mlme.des_chan->ch_freq) &&
		!is_non_tx_vdev) {
		fils_param.vdev_id = wlan_vdev_get_id(mlme_obj->vdev);
		if (is_6g_sap_fd_enabled) {
			fils_param.fd_period = DEFAULT_FILS_DISCOVERY_PERIOD;
		} else {
			fils_param.send_prb_rsp_frame = true;
			fils_param.fd_period = DEFAULT_PROBE_RESP_PERIOD;
		}
		status = tgt_vdev_mgr_fils_enable_send(mlme_obj,
						       &fils_param);
	}

	return status;
}

static QDF_STATUS vdev_mgr_down_param_update(
					struct vdev_mlme_obj *mlme_obj,
					struct vdev_down_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = wlan_vdev_get_id(vdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_down_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct vdev_down_params param = {0};

	if (!mlme_obj) {
		mlme_err("VDEV_MLME is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_down_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_down_send(mlme_obj, &param);

	return status;
}

static QDF_STATUS vdev_mgr_peer_flush_tids_param_update(
					struct vdev_mlme_obj *mlme_obj,
					struct peer_flush_params *param,
					uint8_t *mac,
					uint32_t peer_tid_bitmap)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = wlan_vdev_get_id(vdev);
	param->peer_tid_bitmap = peer_tid_bitmap;
	qdf_mem_copy(param->peer_mac, mac, QDF_MAC_ADDR_SIZE);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_peer_flush_tids_send(struct vdev_mlme_obj *mlme_obj,
					 uint8_t *mac,
					 uint32_t peer_tid_bitmap)
{
	QDF_STATUS status;
	struct peer_flush_params param = {0};

	if (!mlme_obj || !mac) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_peer_flush_tids_param_update(mlme_obj, &param,
						       mac, peer_tid_bitmap);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_peer_flush_tids_send(mlme_obj, &param);

	return status;
}

static QDF_STATUS vdev_mgr_multiple_restart_param_update(
				struct wlan_objmgr_pdev *pdev,
				struct mlme_channel_param *chan,
				uint32_t disable_hw_ack,
				uint32_t *vdev_ids,
				uint32_t num_vdevs,
				struct vdev_mlme_mvr_param *mvr_param,
				struct multiple_vdev_restart_params *param)
{
	param->pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	param->requestor_id = MULTIPLE_VDEV_RESTART_REQ_ID;
	param->disable_hw_ack = disable_hw_ack;
	param->cac_duration_ms = WLAN_DFS_WAIT_MS;
	param->num_vdevs = num_vdevs;

	qdf_mem_copy(param->vdev_ids, vdev_ids,
		     sizeof(uint32_t) * (param->num_vdevs));
	qdf_mem_copy(&param->ch_param, chan,
		     sizeof(struct mlme_channel_param));
	qdf_mem_copy(param->mvr_param, mvr_param,
		     sizeof(*mvr_param) * (param->num_vdevs));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_multiple_restart_send(struct wlan_objmgr_pdev *pdev,
					  struct mlme_channel_param *chan,
					  uint32_t disable_hw_ack,
					  uint32_t *vdev_ids,
					  uint32_t num_vdevs,
					  struct vdev_mlme_mvr_param *mvr_param)
{
	struct multiple_vdev_restart_params param = {0};

	vdev_mgr_multiple_restart_param_update(pdev, chan,
					       disable_hw_ack,
					       vdev_ids, num_vdevs,
					       mvr_param, &param);

	return tgt_vdev_mgr_multiple_vdev_restart_send(pdev, &param);
}

qdf_export_symbol(vdev_mgr_multiple_restart_send);

static QDF_STATUS vdev_mgr_set_custom_aggr_size_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct set_custom_aggr_size_params *param,
				bool is_amsdu)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->aggr_type = is_amsdu ? WLAN_MLME_CUSTOM_AGGR_TYPE_AMSDU
				    : WLAN_MLME_CUSTOM_AGGR_TYPE_AMPDU;
	/*
	 * We are only setting TX params, therefore
	 * we are disabling rx_aggr_size
	 */
	param->rx_aggr_size_disable = true;
	param->tx_aggr_size = is_amsdu ? mlme_obj->mgmt.generic.amsdu
				       : mlme_obj->mgmt.generic.ampdu;
	param->vdev_id = wlan_vdev_get_id(vdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_set_custom_aggr_size_send(
				struct vdev_mlme_obj *vdev_mlme,
				bool is_amsdu)
{
	QDF_STATUS status;
	struct set_custom_aggr_size_params param = {0};

	status = vdev_mgr_set_custom_aggr_size_param_update(vdev_mlme,
							    &param, is_amsdu);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	return tgt_vdev_mgr_set_custom_aggr_size_send(vdev_mlme, &param);
}

static QDF_STATUS vdev_mgr_peer_delete_all_param_update(
				struct vdev_mlme_obj *mlme_obj,
				struct peer_delete_all_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = wlan_vdev_get_id(vdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS vdev_mgr_peer_delete_all_send(struct vdev_mlme_obj *mlme_obj)
{
	QDF_STATUS status;
	struct peer_delete_all_params param = {0};

	if (!mlme_obj) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	status = vdev_mgr_peer_delete_all_param_update(mlme_obj, &param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Param Update Error: %d", status);
		return status;
	}

	status = tgt_vdev_mgr_peer_delete_all_send(mlme_obj, &param);

	return status;
}

