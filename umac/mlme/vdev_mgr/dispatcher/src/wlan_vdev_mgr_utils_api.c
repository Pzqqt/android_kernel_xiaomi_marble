/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

static QDF_STATUS vdev_mgr_config_ratemask_update(
				struct vdev_mlme_obj *mlme_obj,
				struct config_ratemask_params *param)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = mlme_obj->vdev;
	param->vdev_id = wlan_vdev_get_id(vdev);
	param->type = mlme_obj->mgmt.rate_info.type;
	param->lower32 = mlme_obj->mgmt.rate_info.lower32;
	param->higher32 = mlme_obj->mgmt.rate_info.higher32;
	param->lower32_2 = mlme_obj->mgmt.rate_info.lower32_2;

	return QDF_STATUS_SUCCESS;
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
	default:
		cdp_txrx_opmode = wlan_op_mode_unknown;
	};

	return cdp_txrx_opmode;
}

QDF_STATUS
wlan_util_vdev_mlme_set_ratemask_config(struct vdev_mlme_obj *vdev_mlme)
{
	struct config_ratemask_params rm_param = {0};

	if (!vdev_mlme) {
		mlme_err("VDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mgr_config_ratemask_update(vdev_mlme, &rm_param);

	return tgt_vdev_mgr_config_ratemask_cmd_send(vdev_mlme, &rm_param);
}

qdf_export_symbol(wlan_util_vdev_mlme_set_ratemask_config);

QDF_STATUS
wlan_util_vdev_mlme_set_param(struct vdev_mlme_obj *vdev_mlme,
			      enum wlan_mlme_cfg_id param_id,
			      struct wlan_vdev_mgr_cfg mlme_cfg)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_proto *mlme_proto;
	struct vdev_mlme_mgmt *mlme_mgmt;
	struct vdev_mlme_inactivity_params *inactivity_params;
	int is_wmi_cmd = 0;
	int ret = QDF_STATUS_SUCCESS;
	struct vdev_set_params param = {0};

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
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_SLOT_TIME:
		mlme_proto->generic.slot_time = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_PROTECTION_MODE:
		mlme_proto->generic.protection_mode = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_BEACON_INTERVAL:
		mlme_proto->generic.beacon_interval = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_LDPC:
		mlme_proto->generic.ldpc = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_NSS:
		mlme_proto->generic.nss = mlme_cfg.value;
		is_wmi_cmd = 1;
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
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_HT_CAPS:
		mlme_proto->ht_info.ht_caps = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_HE_OPS:
		mlme_proto->he_ops_info.he_ops = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_RTS_THRESHOLD:
		mlme_mgmt->generic.rts_threshold = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_FRAG_THRESHOLD:
		mlme_mgmt->generic.frag_threshold = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_PROBE_DELAY:
		mlme_mgmt->generic.probe_delay = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_REPEAT_PROBE_TIME:
		mlme_mgmt->generic.repeat_probe_time = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_DROP_UNENCRY:
		mlme_mgmt->generic.drop_unencry = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_TX_PWR_LIMIT:
		mlme_mgmt->generic.tx_pwrlimit = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_TX_POWER:
		mlme_mgmt->generic.tx_power = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_AMPDU:
		mlme_mgmt->generic.ampdu = mlme_cfg.value;
		mlme_cfg.value = (mlme_cfg.value << 8) + 0xFF;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_AMSDU:
		mlme_mgmt->generic.amsdu = mlme_cfg.value;
		mlme_cfg.value = (mlme_cfg.value << 8) + 0xFF;
		is_wmi_cmd = 1;
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
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME:
		inactivity_params->keepalive_max_idle_inactive_time_secs =
							mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	case WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME:
		inactivity_params->keepalive_max_unresponsive_time_secs =
							mlme_cfg.value;
		is_wmi_cmd = 1;
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
		is_wmi_cmd = 1;
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
				     mlme_cfg.ssid_cfg.mac_ssid,
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
	case WLAN_MLME_CFG_TX_DECAP_TYPE:
		mlme_mgmt->generic.tx_decap_type = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RX_DECAP_TYPE:
		mlme_mgmt->generic.rx_decap_type = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_TYPE:
		mlme_mgmt->rate_info.type = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_LOWER32:
		mlme_mgmt->rate_info.lower32 = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_HIGHER32:
		mlme_mgmt->rate_info.higher32 = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_RATEMASK_LOWER32_2:
		mlme_mgmt->rate_info.lower32_2 = mlme_cfg.value;
		break;
	case WLAN_MLME_CFG_BCN_TX_RATE:
		mlme_mgmt->rate_info.bcn_tx_rate = mlme_cfg.value;
		is_wmi_cmd = 1;
		break;
	default:
		break;
	}

	if (is_wmi_cmd) {
		param.param_id = param_id;
		param.vdev_id = wlan_vdev_get_id(vdev);
		param.param_value = mlme_cfg.value;
		ret = tgt_vdev_mgr_set_param_send(vdev_mlme, &param);
	}

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
	case WLAN_MLME_CFG_2G_VHT:
		*value = mlme_proto->vht_info.en_2gvht;
		break;
	case WLAN_MLME_CFG_11AX_STUB:
		*value = mlme_mgmt->generic.is_11ax_stub_enabled;
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
