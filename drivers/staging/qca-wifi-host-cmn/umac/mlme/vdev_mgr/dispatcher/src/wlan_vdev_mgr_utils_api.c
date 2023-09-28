/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_vdev_mgr_utils_api.c
 *
 * This file provide definition for APIs to enable Tx Ops and Rx Ops registered
 * through LMAC
 */
#include <wlan_vdev_mgr_utils_api.h>
#include <wlan_vdev_mgr_tgt_if_tx_api.h>
#include <cdp_txrx_cmn_struct.h>
#include <wlan_mlme_dbg.h>
#include <qdf_module.h>
#include <wlan_vdev_mgr_tgt_if_tx_api.h>
#include <wlan_dfs_mlme_api.h>
#ifndef MOBILE_DFS_SUPPORT
#include <wlan_dfs_utils_api.h>
#endif /* MOBILE_DFS_SUPPORT */
#ifdef WLAN_FEATURE_11BE_MLO
#include <wlan_utility.h>
#include <wlan_mlo_mgr_sta.h>
#endif

static QDF_STATUS vdev_mgr_config_ratemask_update(
				uint8_t vdev_id,
				struct vdev_ratemask_params *rate_params,
				struct config_ratemask_params *param,
				uint8_t index)
{
	param->vdev_id = vdev_id;
	param->type = index;
	param->lower32 = rate_params->lower32;
	param->lower32_2 = rate_params->lower32_2;
	param->higher32 = rate_params->higher32;
	param->higher32_2 = rate_params->higher32_2;

	return QDF_STATUS_SUCCESS;
}

enum wlan_op_subtype
wlan_util_vdev_get_cdp_txrx_subtype(struct wlan_objmgr_vdev *vdev)
{
	enum QDF_OPMODE qdf_opmode;
	enum wlan_op_subtype cdp_txrx_subtype;

	qdf_opmode = wlan_vdev_mlme_get_opmode(vdev);
	switch (qdf_opmode) {
	case QDF_P2P_DEVICE_MODE:
		cdp_txrx_subtype = wlan_op_subtype_p2p_device;
		break;
	case QDF_P2P_CLIENT_MODE:
		cdp_txrx_subtype = wlan_op_subtype_p2p_cli;
		break;
	case QDF_P2P_GO_MODE:
		cdp_txrx_subtype = wlan_op_subtype_p2p_go;
		break;
	default:
		cdp_txrx_subtype = wlan_op_subtype_none;
	};

	return cdp_txrx_subtype;
}

enum wlan_op_mode
wlan_util_vdev_get_cdp_txrx_opmode(struct wlan_objmgr_vdev *vdev)
{
	enum QDF_OPMODE qdf_opmode;
	enum wlan_op_mode cdp_txrx_opmode;

	qdf_opmode = wlan_vdev_mlme_get_opmode(vdev);
	switch (qdf_opmode) {
	case QDF_STA_MODE:
		cdp_txrx_opmode = wlan_op_mode_sta;
		break;
	case QDF_SAP_MODE:
		cdp_txrx_opmode = wlan_op_mode_ap;
		break;
	case QDF_MONITOR_MODE:
		cdp_txrx_opmode = wlan_op_mode_monitor;
		break;
	case QDF_P2P_DEVICE_MODE:
		cdp_txrx_opmode = wlan_op_mode_ap;
		break;
	case QDF_P2P_CLIENT_MODE:
		cdp_txrx_opmode = wlan_op_mode_sta;
		break;
	case QDF_P2P_GO_MODE:
		cdp_txrx_opmode = wlan_op_mode_ap;
		break;
	case QDF_OCB_MODE:
		cdp_txrx_opmode = wlan_op_mode_ocb;
		break;
	case QDF_IBSS_MODE:
		cdp_txrx_opmode = wlan_op_mode_ibss;
		break;
	case QDF_NDI_MODE:
		cdp_txrx_opmode = wlan_op_mode_ndi;
		break;
	default:
		cdp_txrx_opmode = wlan_op_mode_unknown;
	};

	return cdp_txrx_opmode;
}

/**
 * wlan_util_vdev_mlme_set_ratemask_config() - common MLME API to fill
 * ratemask parameters of vdev_mlme object
 * @vdev_mlme: pointer to vdev_mlme object
 * @index: array index of ratemask_params
 */
QDF_STATUS
wlan_util_vdev_mlme_set_ratemask_config(struct vdev_mlme_obj *vdev_mlme,
					uint8_t index)
{
	struct config_ratemask_params rm_param = {0};
	uint8_t vdev_id;
	struct vdev_mlme_rate_info *rate_info;
	struct vdev_ratemask_params *rate_params;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_id = wlan_vdev_get_id(vdev_mlme->vdev);
	rate_info = &vdev_mlme->mgmt.rate_info;
	rate_params = &rate_info->ratemask_params[index];
	vdev_mgr_config_ratemask_update(vdev_id,
					rate_params,
					&rm_param, index);

	return tgt_vdev_mgr_config_ratemask_cmd_send(vdev_mlme,
						    &rm_param);
}

qdf_export_symbol(wlan_util_vdev_mlme_set_ratemask_config);

static QDF_STATUS
tgt_vdev_mgr_vdev_set_param_wrapper(struct vdev_mlme_obj *vdev_mlme,
				    enum wlan_mlme_cfg_id param_id,
				    struct wlan_vdev_mgr_cfg mlme_cfg)
{
	uint8_t id, count = 0;
	bool is_mbss_enabled, is_cmn_param = 0;
	unsigned long vdev_bmap = 0;
	struct wlan_objmgr_pdev *pdev;
	struct vdev_mlme_mbss_11ax *mbss;
	struct vdev_set_params param1 = {0};
	struct multiple_vdev_set_param param2 = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	mbss = &vdev_mlme->mgmt.mbss_11ax;
	is_mbss_enabled = (mbss->mbssid_flags
				& WLAN_VDEV_MLME_FLAGS_NON_MBSSID_AP) ? 0 : 1;

	if (is_mbss_enabled) {
		vdev_bmap = mbss->vdev_bmap;
		is_cmn_param = mbss->is_cmn_param;
	}

	/* 1. if non tx vap and cmn param, dont send any WMI
	 * 2. if tx vap and cmn param, send multi vdev set WMI
	 * 3. if non tx vap and non cmn param, send vdev set WMI
	 * 4. if tx vap and non cmn param, send vdev set WMI
	 * 5. if non mbss vap, send vdev set WMI
	 */
	if (!is_mbss_enabled || !is_cmn_param) {
		param1.param_id = param_id;
		param1.vdev_id = wlan_vdev_get_id(vdev_mlme->vdev);
		param1.param_value = mlme_cfg.value;
		return tgt_vdev_mgr_set_param_send(vdev_mlme, &param1);
	}

	if (is_cmn_param && vdev_bmap) {
		pdev = wlan_vdev_get_pdev(vdev_mlme->vdev);
		param2.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		param2.param_id = param_id;
		param2.param_value = mlme_cfg.value;

		for (id = 0; id < WLAN_UMAC_PDEV_MAX_VDEVS; id++) {
			if (qdf_test_bit(id, &vdev_bmap)) {
				param2.vdev_ids[count] = id;
				count++;
			}
		}
		param2.num_vdevs = count;
		status = tgt_vdev_mgr_multiple_vdev_set_param(pdev, &param2);
		mbss->vdev_bmap = 0;
	}

	/* Reset the is_cmn_param for this vap */
	mbss->is_cmn_param = 0;

	return status;
}

QDF_STATUS
wlan_util_vdev_mlme_set_param(struct vdev_mlme_obj *vdev_mlme,
			      enum wlan_mlme_cfg_id param_id,
			      struct wlan_vdev_mgr_cfg mlme_cfg)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_proto *mlme_proto;
	struct vdev_mlme_mgmt *mlme_mgmt;
	struct vdev_mlme_inactivity_params *inactivity_params;
	bool is_wmi_cmd = false;
	int ret = QDF_STATUS_SUCCESS;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	vdev = vdev_mlme->vdev;
	mlme_proto = &vdev_mlme->proto;
	mlme_mgmt = &vdev_mlme->mgmt;
	inactivity_params = &mlme_mgmt->inactivity_params;

	switch (param_id) {
	case WLAN_MLME_CFG_DTIM_PERIOD:
		mlme_proto->generic.dtim_period = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_SLOT_TIME:
		mlme_proto->generic.slot_time = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_PROTECTION_MODE:
		mlme_proto->generic.protection_mode = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_BEACON_INTERVAL:
		mlme_proto->generic.beacon_interval = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_LDPC:
		mlme_proto->generic.ldpc = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_NSS:
		mlme_proto->generic.nss = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_TSF_ADJUST:
		mlme_proto->generic.tsfadjust = mlme_cfg.tsf;
		break;
	case WLAN_MLME_CFG_ASSOC_ID:
		mlme_proto->sta.assoc_id = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_VHT_CAPS:
		mlme_proto->vht_info.caps = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_SUBFER:
		mlme_proto->vht_info.subfer = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MUBFER:
		mlme_proto->vht_info.mubfer = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_SUBFEE:
		mlme_proto->vht_info.subfee = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MUBFEE:
		mlme_proto->vht_info.mubfee = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_IMLICIT_BF:
		mlme_proto->vht_info.implicit_bf = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_SOUNDING_DIM:
		mlme_proto->vht_info.sounding_dimension = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_BFEE_STS_CAP:
		mlme_proto->vht_info.bfee_sts_cap = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TXBF_CAPS:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_HT_CAPS:
		mlme_proto->ht_info.ht_caps = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_HE_OPS:
		mlme_proto->he_ops_info.he_ops = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
#ifdef WLAN_FEATURE_11BE
	case WLAN_MLME_CFG_EHT_OPS:
		mlme_proto->eht_ops_info.eht_ops = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
#endif
	case WLAN_MLME_CFG_RTS_THRESHOLD:
		mlme_mgmt->generic.rts_threshold = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_FRAG_THRESHOLD:
		mlme_mgmt->generic.frag_threshold = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_PROBE_DELAY:
		mlme_mgmt->generic.probe_delay = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_REPEAT_PROBE_TIME:
		mlme_mgmt->generic.repeat_probe_time = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_DROP_UNENCRY:
		mlme_mgmt->generic.drop_unencry = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_TX_PWR_LIMIT:
		mlme_mgmt->generic.tx_pwrlimit = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_POWER:
		mlme_mgmt->generic.tx_power = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_AMPDU:
		mlme_mgmt->generic.ampdu = mlme_cfg.value;
		mlme_cfg.value = (mlme_cfg.value << 8) + 0xFF;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_AMPDU_SIZE:
		mlme_mgmt->generic.ampdu = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_AMSDU:
		mlme_mgmt->generic.amsdu = mlme_cfg.value;
		mlme_cfg.value = (mlme_cfg.value << 8) + 0xFF;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_AMSDU_SIZE:
		mlme_mgmt->generic.amsdu = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_BMISS_FIRST_BCNT:
		inactivity_params->bmiss_first_bcnt = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_BMISS_FINAL_BCNT:
		inactivity_params->bmiss_final_bcnt = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME:
		inactivity_params->keepalive_min_idle_inactive_time_secs =
							mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME:
		inactivity_params->keepalive_max_idle_inactive_time_secs =
							mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME:
		inactivity_params->keepalive_max_unresponsive_time_secs =
							mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_RATE_FLAGS:
		mlme_mgmt->rate_info.rate_flags = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_PER_BAND_TX_MGMT_RATE:
		mlme_mgmt->rate_info.per_band_tx_mgmt_rate = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MAX_RATE:
		mlme_mgmt->rate_info.max_rate = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_MGMT_RATE:
		mlme_mgmt->rate_info.tx_mgmt_rate = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_RTSCTS_RATE:
		mlme_mgmt->rate_info.rtscts_tx_rate = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_CHAINMASK:
		mlme_mgmt->chainmask_info.tx_chainmask = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RX_CHAINMASK:
		mlme_mgmt->chainmask_info.rx_chainmask = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_PKT_POWERSAVE:
		mlme_mgmt->powersave_info.packet_powersave = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MAX_LI_OF_MODDTIM:
		mlme_mgmt->powersave_info.max_li_of_moddtim = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_DYNDTIM_CNT:
		mlme_mgmt->powersave_info.dyndtim_cnt = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_LISTEN_INTERVAL:
		mlme_mgmt->powersave_info.listen_interval = mlme_cfg.value;
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_MODDTIM_CNT:
		mlme_mgmt->powersave_info.moddtim_cnt = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_PROFILE_IDX:
		mlme_mgmt->mbss_11ax.profile_idx = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_PROFILE_NUM:
		mlme_mgmt->mbss_11ax.profile_num = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_MBSSID_FLAGS:
		mlme_mgmt->mbss_11ax.mbssid_flags = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_VDEVID_TRANS:
		mlme_mgmt->mbss_11ax.vdevid_trans = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_SSID:
		if (mlme_cfg.ssid_cfg.length <= WLAN_SSID_MAX_LEN) {
			qdf_mem_copy(mlme_mgmt->generic.ssid,
				     mlme_cfg.ssid_cfg.ssid,
				     mlme_cfg.ssid_cfg.length);
			mlme_mgmt->generic.ssid_len =
						mlme_cfg.ssid_cfg.length;
		} else {
			mlme_mgmt->generic.ssid_len = 0;
		}

		break;
	case WLAN_MLME_CFG_TRANS_BSSID:
		qdf_mem_copy(mlme_mgmt->mbss_11ax.trans_bssid,
			     mlme_cfg.trans_bssid, QDF_MAC_ADDR_SIZE);
		break;
	case WLAN_MLME_CFG_TYPE:
		mlme_mgmt->generic.type = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_SUBTYPE:
		mlme_mgmt->generic.subtype = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_UAPSD:
		mlme_proto->sta.uapsd_cfg = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_ENCAP_TYPE:
		is_wmi_cmd = true;
		mlme_mgmt->generic.tx_encap_type = mlme_cfg.value;
		tgt_vdev_mgr_set_tx_rx_decap_type(vdev_mlme,
						  WLAN_MLME_CFG_TX_ENCAP_TYPE,
						  mlme_cfg.value);
		break;
	case WLAN_MLME_CFG_RX_DECAP_TYPE:
		is_wmi_cmd = true;
		mlme_mgmt->generic.rx_decap_type = mlme_cfg.value;
		tgt_vdev_mgr_set_tx_rx_decap_type(vdev_mlme,
						  WLAN_MLME_CFG_RX_DECAP_TYPE,
						  mlme_cfg.value);
		break;
	case WLAN_MLME_CFG_RATEMASK_LOWER32:
		if (mlme_cfg.ratemask.index < WLAN_VDEV_RATEMASK_TYPE_MAX)
			mlme_mgmt->rate_info.ratemask_params[
					mlme_cfg.ratemask.index].lower32 =
							mlme_cfg.ratemask.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_HIGHER32:
		if (mlme_cfg.ratemask.index < WLAN_VDEV_RATEMASK_TYPE_MAX)
			mlme_mgmt->rate_info.ratemask_params[
					mlme_cfg.ratemask.index].higher32 =
							mlme_cfg.ratemask.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_LOWER32_2:
		if (mlme_cfg.ratemask.index < WLAN_VDEV_RATEMASK_TYPE_MAX)
			mlme_mgmt->rate_info.ratemask_params[
					mlme_cfg.ratemask.index].lower32_2 =
							mlme_cfg.ratemask.value;
		break;
	case WLAN_MLME_CFG_BCN_TX_RATE:
		mlme_mgmt->rate_info.bcn_tx_rate = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_BCN_TX_RATE_CODE:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_TX_MGMT_RATE_CODE:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_ENABLE_MULTI_GROUP_KEY:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_MAX_GROUP_KEYS:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_TX_STREAMS:
		mlme_mgmt->chainmask_info.num_tx_chain = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RX_STREAMS:
		mlme_mgmt->chainmask_info.num_rx_chain = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_ENABLE_DISABLE_RTT_RESPONDER_ROLE:
		is_wmi_cmd = true;
		break;
	case WLAN_MLME_CFG_ENABLE_DISABLE_RTT_INITIATOR_ROLE:
		is_wmi_cmd = true;
		break;
	default:
		break;
	}

	if (is_wmi_cmd)
		ret = tgt_vdev_mgr_vdev_set_param_wrapper(vdev_mlme, param_id,
							  mlme_cfg);

	return ret;
}

qdf_export_symbol(wlan_util_vdev_mlme_set_param);

void wlan_util_vdev_mlme_get_param(struct vdev_mlme_obj *vdev_mlme,
				   enum wlan_mlme_cfg_id param_id,
				   uint32_t *value)
{
	struct vdev_mlme_proto *mlme_proto;
	struct vdev_mlme_mgmt *mlme_mgmt;
	struct vdev_mlme_inactivity_params *inactivity_params;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME is NULL");
		return;
	}
	mlme_proto = &vdev_mlme->proto;
	mlme_mgmt = &vdev_mlme->mgmt;
	inactivity_params = &mlme_mgmt->inactivity_params;

	switch (param_id) {
	case WLAN_MLME_CFG_DTIM_PERIOD:
		*value = mlme_proto->generic.dtim_period;
		break;
	case WLAN_MLME_CFG_SLOT_TIME:
		*value = mlme_proto->generic.slot_time;
		break;
	case WLAN_MLME_CFG_PROTECTION_MODE:
		*value = mlme_proto->generic.protection_mode;
		break;
	case WLAN_MLME_CFG_BEACON_INTERVAL:
		*value = mlme_proto->generic.beacon_interval;
		break;
	case WLAN_MLME_CFG_LDPC:
		*value = mlme_proto->generic.ldpc;
		break;
	case WLAN_MLME_CFG_NSS:
		*value = mlme_proto->generic.nss;
		break;
	case WLAN_MLME_CFG_ASSOC_ID:
		*value = mlme_proto->sta.assoc_id;
		break;
	case WLAN_MLME_CFG_VHT_CAPS:
		*value = mlme_proto->vht_info.caps;
		break;
	case WLAN_MLME_CFG_SUBFER:
		*value = mlme_proto->vht_info.subfer;
		break;
	case WLAN_MLME_CFG_MUBFER:
		*value = mlme_proto->vht_info.mubfer;
		break;
	case WLAN_MLME_CFG_SUBFEE:
		*value = mlme_proto->vht_info.subfee;
		break;
	case WLAN_MLME_CFG_MUBFEE:
		*value = mlme_proto->vht_info.mubfee;
		break;
	case WLAN_MLME_CFG_IMLICIT_BF:
		*value = mlme_proto->vht_info.implicit_bf;
		break;
	case WLAN_MLME_CFG_SOUNDING_DIM:
		*value = mlme_proto->vht_info.sounding_dimension;
		break;
	case WLAN_MLME_CFG_BFEE_STS_CAP:
		*value = mlme_proto->vht_info.bfee_sts_cap;
		break;
	case WLAN_MLME_CFG_HT_CAPS:
		*value = mlme_proto->ht_info.ht_caps;
		break;
	case WLAN_MLME_CFG_HE_OPS:
		*value = mlme_proto->he_ops_info.he_ops;
		break;
#ifdef WLAN_FEATURE_11BE
	case WLAN_MLME_CFG_EHT_OPS:
		*value = mlme_proto->eht_ops_info.eht_ops;
		break;
#endif
	case WLAN_MLME_CFG_RTS_THRESHOLD:
		*value = mlme_mgmt->generic.rts_threshold;
		break;
	case WLAN_MLME_CFG_FRAG_THRESHOLD:
		*value = mlme_mgmt->generic.frag_threshold;
		break;
	case WLAN_MLME_CFG_PROBE_DELAY:
		*value = mlme_mgmt->generic.probe_delay;
		break;
	case WLAN_MLME_CFG_REPEAT_PROBE_TIME:
		*value = mlme_mgmt->generic.repeat_probe_time;
		break;
	case WLAN_MLME_CFG_DROP_UNENCRY:
		*value = mlme_mgmt->generic.drop_unencry;
		break;
	case WLAN_MLME_CFG_TX_PWR_LIMIT:
		*value = mlme_mgmt->generic.tx_pwrlimit;
		break;
	case WLAN_MLME_CFG_TX_POWER:
		*value = mlme_mgmt->generic.tx_power;
		break;
	case WLAN_MLME_CFG_AMPDU:
		*value = mlme_mgmt->generic.ampdu;
		break;
	case WLAN_MLME_CFG_AMSDU:
		*value = mlme_mgmt->generic.amsdu;
		break;
	case WLAN_MLME_CFG_SSID_LEN:
		*value = mlme_mgmt->generic.ssid_len;
		break;
	case WLAN_MLME_CFG_BMISS_FIRST_BCNT:
		*value = inactivity_params->bmiss_first_bcnt;
		break;
	case WLAN_MLME_CFG_BMISS_FINAL_BCNT:
		*value = inactivity_params->bmiss_final_bcnt;
		break;
	case WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME:
		*value =
		      inactivity_params->keepalive_min_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME:
		*value =
		      inactivity_params->keepalive_max_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME:
		*value =
		      inactivity_params->keepalive_max_unresponsive_time_secs;
		break;
	case WLAN_MLME_CFG_RATE_FLAGS:
		*value = mlme_mgmt->rate_info.rate_flags;
		break;
	case WLAN_MLME_CFG_PER_BAND_TX_MGMT_RATE:
		*value = mlme_mgmt->rate_info.per_band_tx_mgmt_rate;
		break;
	case WLAN_MLME_CFG_MAX_RATE:
		*value = mlme_mgmt->rate_info.max_rate;
		break;
	case WLAN_MLME_CFG_TX_MGMT_RATE:
		*value = mlme_mgmt->rate_info.tx_mgmt_rate;
		break;
	case WLAN_MLME_CFG_TX_RTSCTS_RATE:
		*value = mlme_mgmt->rate_info.rtscts_tx_rate;
		break;
	case WLAN_MLME_CFG_TX_CHAINMASK:
		*value = mlme_mgmt->chainmask_info.tx_chainmask;
		break;
	case WLAN_MLME_CFG_RX_CHAINMASK:
		*value = mlme_mgmt->chainmask_info.rx_chainmask;
		break;
	case WLAN_MLME_CFG_PKT_POWERSAVE:
		*value = mlme_mgmt->powersave_info.packet_powersave;
		break;
	case WLAN_MLME_CFG_MAX_LI_OF_MODDTIM:
		*value = mlme_mgmt->powersave_info.max_li_of_moddtim;
		break;
	case WLAN_MLME_CFG_DYNDTIM_CNT:
		*value = mlme_mgmt->powersave_info.dyndtim_cnt;
		break;
	case WLAN_MLME_CFG_LISTEN_INTERVAL:
		*value = mlme_mgmt->powersave_info.listen_interval;
		break;
	case WLAN_MLME_CFG_MODDTIM_CNT:
		*value = mlme_mgmt->powersave_info.moddtim_cnt;
		break;
	case WLAN_MLME_CFG_PROFILE_IDX:
		*value = mlme_mgmt->mbss_11ax.profile_idx;
		break;
	case WLAN_MLME_CFG_PROFILE_NUM:
		*value = mlme_mgmt->mbss_11ax.profile_num;
		break;
	case WLAN_MLME_CFG_MBSSID_FLAGS:
		*value = mlme_mgmt->mbss_11ax.mbssid_flags;
		break;
	case WLAN_MLME_CFG_VDEVID_TRANS:
		*value = mlme_mgmt->mbss_11ax.vdevid_trans;
		break;
	case WLAN_MLME_CFG_BCN_TX_RATE:
		*value = mlme_mgmt->rate_info.bcn_tx_rate;
		break;
	case WLAN_MLME_CFG_TX_STREAMS:
		*value = mlme_mgmt->chainmask_info.num_tx_chain;
		break;
	case WLAN_MLME_CFG_RX_STREAMS:
		*value = mlme_mgmt->chainmask_info.num_rx_chain;
		break;
	default:
		break;
	}
}

qdf_export_symbol(wlan_util_vdev_mlme_get_param);

void wlan_util_vdev_get_param(struct wlan_objmgr_vdev *vdev,
			      enum wlan_mlme_cfg_id param_id,
			      uint32_t *value)
{
	ucfg_wlan_vdev_mgr_get_param(vdev, param_id, value);
}

qdf_export_symbol(wlan_util_vdev_get_param);

#ifndef MOBILE_DFS_SUPPORT
int wlan_util_vdev_mgr_get_cac_timeout_for_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_channel *des_chan = NULL;
	struct wlan_channel *bss_chan = NULL;
	bool continue_current_cac = 0;
	int dfs_cac_timeout = 0;

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);
	if (!des_chan)
		return 0;

	bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
	if (!bss_chan)
		return 0;

	if (!utils_dfs_is_cac_required(wlan_vdev_get_pdev(vdev), des_chan,
				       bss_chan, &continue_current_cac))
		return 0;

	dfs_cac_timeout = dfs_mlme_get_cac_timeout_for_freq(
				wlan_vdev_get_pdev(vdev), des_chan->ch_freq,
				des_chan->ch_cfreq2, des_chan->ch_flags);
	/* Seconds to milliseconds */
	return SECONDS_TO_MS(dfs_cac_timeout);
}
#else
int wlan_util_vdev_mgr_get_cac_timeout_for_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev_mlme is null");
		return 0;
	}

	return vdev_mlme->mgmt.ap.cac_duration_ms;
}

void wlan_util_vdev_mgr_set_cac_timeout_for_vdev(struct wlan_objmgr_vdev *vdev,
						 uint32_t new_chan_cac_ms)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev_mlme is null");
		return;
	}

	vdev_mlme->mgmt.ap.cac_duration_ms = new_chan_cac_ms;
}
#endif /* MOBILE_DFS_SUPPORT */

QDF_STATUS wlan_util_vdev_mgr_get_csa_channel_switch_time(
		struct wlan_objmgr_vdev *vdev,
		uint32_t *chan_switch_time)
{
	struct vdev_mlme_obj *vdev_mlme = NULL;

	*chan_switch_time = 0;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev_mlme is null");
		return QDF_STATUS_E_FAILURE;
	}

	/* Time between CSA count 1 and CSA count 0 is one beacon interval. */
	*chan_switch_time = vdev_mlme->proto.generic.beacon_interval;

	/* Vdev restart time */
	*chan_switch_time += SECONDS_TO_MS(VDEV_RESTART_TIME);

	/* Add one beacon interval time required to send beacon on the
	 * new channel after switching to the new channel.
	 */
	*chan_switch_time += vdev_mlme->proto.generic.beacon_interval;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_util_vdev_mgr_compute_max_channel_switch_time(
		struct wlan_objmgr_vdev *vdev, uint32_t *max_chan_switch_time)
{
	QDF_STATUS status;

	status = wlan_util_vdev_mgr_get_csa_channel_switch_time(
			vdev, max_chan_switch_time);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to get the CSA channel switch time");
		return status;
	}

	/* Plus the CAC time */
	*max_chan_switch_time +=
			wlan_util_vdev_mgr_get_cac_timeout_for_vdev(vdev);

	return QDF_STATUS_SUCCESS;
}

uint32_t
wlan_utils_get_vdev_remaining_channel_switch_time(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme = NULL;
	int32_t remaining_chan_switch_time;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return 0;

	if (!vdev_mlme->mgmt.ap.last_bcn_ts_ms)
		return 0;

	/* Remaining channel switch time is equal to the time when last beacon
	 * sent on the CSA triggered vap plus max channel switch time minus
	 * current time.
	 */
	remaining_chan_switch_time =
	    ((vdev_mlme->mgmt.ap.last_bcn_ts_ms +
	      vdev_mlme->mgmt.ap.max_chan_switch_time) -
	     qdf_mc_timer_get_system_time());

	return (remaining_chan_switch_time > 0) ?
		remaining_chan_switch_time : 0;
}

#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS wlan_util_vdev_mgr_quiet_offload(
				struct wlan_objmgr_psoc *psoc,
				struct vdev_sta_quiet_event *quiet_event)
{
	uint8_t vdev_id;
	bool connected;
	struct wlan_objmgr_vdev *vdev;

	if (qdf_is_macaddr_zero(&quiet_event->mld_mac) &&
	    qdf_is_macaddr_zero(&quiet_event->link_mac)) {
		mlme_err("mld_mac and link mac are invalid");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_is_macaddr_zero(&quiet_event->mld_mac)) {
		connected = wlan_get_connected_vdev_by_mld_addr(
				psoc, quiet_event->mld_mac.bytes, &vdev_id);
		if (!connected) {
			mlme_err("Can't find vdev with mld " QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(quiet_event->mld_mac.bytes));
			return QDF_STATUS_E_INVAL;
		}
		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
				psoc, vdev_id, WLAN_MLME_OBJMGR_ID);
		if (!vdev) {
			mlme_err("Null vdev");
			return QDF_STATUS_E_INVAL;
		}
		if (wlan_vdev_mlme_is_mlo_vdev(vdev))
			mlo_sta_save_quiet_status(vdev->mlo_dev_ctx,
						  quiet_event->link_id,
						  quiet_event->quiet_status);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
	} else if (!qdf_is_macaddr_zero(&quiet_event->link_mac)) {
		connected = wlan_get_connected_vdev_from_psoc_by_bssid(
				psoc, quiet_event->link_mac.bytes, &vdev_id);
		if (!connected) {
			mlme_err("Can't find vdev with BSSID" QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(quiet_event->link_mac.bytes));
			return QDF_STATUS_E_INVAL;
		}
		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
				psoc, vdev_id, WLAN_MLME_OBJMGR_ID);
		if (!vdev) {
			mlme_err("Null vdev");
			return QDF_STATUS_E_INVAL;
		}
		if (wlan_vdev_mlme_is_mlo_vdev(vdev))
			mlo_sta_save_quiet_status(vdev->mlo_dev_ctx,
						  wlan_vdev_get_link_id(vdev),
						  quiet_event->quiet_status);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_11BE_MLO */
