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
 * DOC: target_if_vdev_mgr_tx_ops.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wmi_unified_api.h>
#include <wmi_unified_param.h>
#include <init_deinit_lmac.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include <target_if_vdev_mgr_rx_ops.h>
#include <target_if.h>
#include <target_type.h>
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <wlan_vdev_mgr_utils_api.h>
#include <wlan_cmn.h>

static QDF_STATUS target_if_vdev_mgr_register_event_handler(
					struct wlan_objmgr_psoc *psoc)
{
	return target_if_vdev_mgr_wmi_event_register(psoc);
}

static QDF_STATUS target_if_vdev_mgr_unregister_event_handler(
					struct wlan_objmgr_psoc *psoc)
{
	return target_if_vdev_mgr_wmi_event_unregister(psoc);
}

QDF_STATUS target_if_vdev_mgr_rsp_timer_mgmt(
					struct wlan_objmgr_vdev *vdev,
					qdf_timer_t *rsp_timer,
					bool init)
{
	if (init) {
		qdf_timer_init(NULL, rsp_timer,
			       target_if_vdev_mgr_rsp_timer_mgmt_cb,
			       (void *)vdev, QDF_TIMER_TYPE_WAKE_APPS);
		wlan_objmgr_vdev_get_ref(vdev, WLAN_MLME_SB_ID);
	} else {
		qdf_timer_free(rsp_timer);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	}

	return QDF_STATUS_SUCCESS;
}

struct wmi_unified
*target_if_vdev_mgr_wmi_handle_get(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		QDF_ASSERT(0);
		return NULL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("wmi_handle is null");
		return NULL;
	}

	return wmi_handle;
}

static QDF_STATUS target_if_check_is_pre_lithium(
					struct wlan_objmgr_psoc *psoc)
{
	if (lmac_get_tgt_type(psoc) < TARGET_TYPE_QCA8074)
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_INVAL;
}

static inline uint32_t
target_if_vdev_mlme_id_2_wmi(uint32_t cfg_id)
{
	int wmi_id;

	switch (cfg_id) {
	case WLAN_MLME_CFG_DTIM_PERIOD:
		wmi_id = wmi_vdev_param_dtim_period;
		break;
	case WLAN_MLME_CFG_SLOT_TIME:
		wmi_id = wmi_vdev_param_slot_time;
		break;
	case WLAN_MLME_CFG_PROTECTION_MODE:
		wmi_id = wmi_vdev_param_protection_mode;
		break;
	case WLAN_MLME_CFG_BEACON_INTERVAL:
		wmi_id = wmi_vdev_param_beacon_interval;
		break;
	case WLAN_MLME_CFG_LDPC:
		wmi_id = wmi_vdev_param_ldpc;
		break;
	case WLAN_MLME_CFG_NSS:
		wmi_id = wmi_vdev_param_nss;
		break;
	case WLAN_MLME_CFG_SUBFER:
	case WLAN_MLME_CFG_MUBFER:
	case WLAN_MLME_CFG_SUBFEE:
	case WLAN_MLME_CFG_MUBFEE:
	case WLAN_MLME_CFG_IMLICIT_BF:
	case WLAN_MLME_CFG_SOUNDING_DIM:
		wmi_id = wmi_vdev_param_txbf;
		break;
	case WLAN_MLME_CFG_HE_OPS:
		wmi_id = wmi_vdev_param_set_heop;
		break;
	case WLAN_MLME_CFG_RTS_THRESHOLD:
		wmi_id = wmi_vdev_param_rts_threshold;
		break;
	case WLAN_MLME_CFG_FRAG_THRESHOLD:
		wmi_id = wmi_vdev_param_fragmentation_threshold;
		break;
	case WLAN_MLME_CFG_DROP_UNENCRY:
		wmi_id = wmi_vdev_param_drop_unencry;
		break;
	case WLAN_MLME_CFG_TX_POWER:
		wmi_id = wmi_vdev_param_tx_power;
		break;
	case WLAN_MLME_CFG_AMPDU:
		wmi_id = wmi_vdev_param_amsdu_subframe_size_per_ac;
		break;
	case WLAN_MLME_CFG_AMSDU:
		wmi_id = wmi_vdev_param_amsdu_subframe_size_per_ac;
		break;
	case WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_min_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_max_idle_inactive_time_secs;
		break;
	case WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME:
		wmi_id =
			wmi_vdev_param_ap_keepalive_max_unresponsive_time_secs;
		break;
	case WLAN_MLME_CFG_UAPSD:
		wmi_id = WMI_HOST_STA_PS_PARAM_UAPSD;
		break;
	default:
		wmi_id = cfg_id;
		break;
	}

	return wmi_id;
}

static enum wlan_phymode
target_if_wmi_phymode_from_wlan_phymode(struct wlan_objmgr_vdev *vdev,
					uint32_t phy_mode,
					bool is_2gvht_en,
					bool is_11ax_stub_enabled)
{
	static uint32_t phy_2_wmi[WLAN_PHYMODE_11AXA_HE80_80 + 1] = {
		[WLAN_PHYMODE_AUTO] = WMI_HOST_MODE_UNKNOWN,
		[WLAN_PHYMODE_11A] = WMI_HOST_MODE_11A,
		[WLAN_PHYMODE_11B] = WMI_HOST_MODE_11B,
		[WLAN_PHYMODE_11G] = WMI_HOST_MODE_11G,
		[WLAN_PHYMODE_11NA_HT20] = WMI_HOST_MODE_11NA_HT20,
		[WLAN_PHYMODE_11NG_HT20] = WMI_HOST_MODE_11NG_HT20,
		[WLAN_PHYMODE_11NA_HT40PLUS] = WMI_HOST_MODE_11NA_HT40,
		[WLAN_PHYMODE_11NA_HT40MINUS] = WMI_HOST_MODE_11NA_HT40,
		[WLAN_PHYMODE_11NG_HT40PLUS] = WMI_HOST_MODE_11NG_HT40,
		[WLAN_PHYMODE_11NG_HT40MINUS] = WMI_HOST_MODE_11NG_HT40,
		[WLAN_PHYMODE_11NG_HT40] = WMI_HOST_MODE_11NG_HT40,
		[WLAN_PHYMODE_11NA_HT40] = WMI_HOST_MODE_11NA_HT40,
		[WLAN_PHYMODE_11AC_VHT20] = WMI_HOST_MODE_11AC_VHT20,
		[WLAN_PHYMODE_11AC_VHT40PLUS] = WMI_HOST_MODE_11AC_VHT40,
		[WLAN_PHYMODE_11AC_VHT40MINUS] = WMI_HOST_MODE_11AC_VHT40,
		[WLAN_PHYMODE_11AC_VHT40] = WMI_HOST_MODE_11AC_VHT40,
		[WLAN_PHYMODE_11AC_VHT80] = WMI_HOST_MODE_11AC_VHT80,
		[WLAN_PHYMODE_11AC_VHT160] = WMI_HOST_MODE_11AC_VHT160,
		[WLAN_PHYMODE_11AC_VHT80_80] = WMI_HOST_MODE_11AC_VHT80_80,
		[WLAN_PHYMODE_11AXA_HE20] = WMI_HOST_MODE_11AX_HE20,
		[WLAN_PHYMODE_11AXG_HE20] = WMI_HOST_MODE_11AX_HE20_2G,
		[WLAN_PHYMODE_11AXA_HE40PLUS] = WMI_HOST_MODE_11AX_HE40,
		[WLAN_PHYMODE_11AXA_HE40MINUS] = WMI_HOST_MODE_11AX_HE40,
		[WLAN_PHYMODE_11AXG_HE40PLUS] = WMI_HOST_MODE_11AX_HE40_2G,
		[WLAN_PHYMODE_11AXG_HE40MINUS] = WMI_HOST_MODE_11AX_HE40_2G,
		[WLAN_PHYMODE_11AXA_HE40] = WMI_HOST_MODE_11AX_HE40,
		[WLAN_PHYMODE_11AXG_HE40] = WMI_HOST_MODE_11AX_HE40_2G,
		[WLAN_PHYMODE_11AXA_HE80] = WMI_HOST_MODE_11AX_HE80,
		[WLAN_PHYMODE_11AXA_HE160] = WMI_HOST_MODE_11AX_HE160,
		[WLAN_PHYMODE_11AXA_HE80_80] = WMI_HOST_MODE_11AX_HE80_80,
	};
	uint32_t temp_phymode;
	enum wmi_target_type target;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	uint32_t target_type;

	if (phy_mode < WLAN_PHYMODE_11A ||
	    phy_mode > WLAN_PHYMODE_11AXA_HE80_80) {
		temp_phymode = WMI_HOST_MODE_UNKNOWN;
		return temp_phymode;
	}

	target_type = lmac_get_tgt_type(psoc);

	/*
	 * 802.11ax stubbing is enabled only if the enable_11ax_stub module
	 * parameter is set to 1, and only for QCA9984.
	 */
	if (is_11ax_stub_enabled && (target_type == TARGET_TYPE_QCA9984)) {
		/* re-map 802.11ax modes to equivalent 802.11n/ac modes. */
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE20] =
						WMI_HOST_MODE_11AC_VHT20;
		phy_2_wmi[WLAN_PHYMODE_11AXG_HE20] =
						WMI_HOST_MODE_11NG_HT20;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE40PLUS] =
						WMI_HOST_MODE_11AC_VHT40;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE40MINUS] =
						WMI_HOST_MODE_11AC_VHT40;
		phy_2_wmi[WLAN_PHYMODE_11AXG_HE40PLUS] =
						WMI_HOST_MODE_11NG_HT40;
		phy_2_wmi[WLAN_PHYMODE_11AXG_HE40MINUS] =
						WMI_HOST_MODE_11NG_HT40;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE40] =
						WMI_HOST_MODE_11AC_VHT40;
		phy_2_wmi[WLAN_PHYMODE_11AXG_HE40] =
						WMI_HOST_MODE_11NG_HT40;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE80] =
						WMI_HOST_MODE_11AC_VHT80;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE160] =
						WMI_HOST_MODE_11AC_VHT160;
		phy_2_wmi[WLAN_PHYMODE_11AXA_HE80_80] =
						WMI_HOST_MODE_11AC_VHT80_80;
	}

	switch (target_type) {
	case TARGET_TYPE_AR6002:
	case TARGET_TYPE_AR6003:
	case TARGET_TYPE_AR6004:
	case TARGET_TYPE_AR6006:
	case TARGET_TYPE_AR9888:
	case TARGET_TYPE_AR6320:
	case TARGET_TYPE_AR900B:
	case TARGET_TYPE_QCA9984:
	case TARGET_TYPE_QCA9888:
	case TARGET_TYPE_IPQ4019:
		target = WMI_NON_TLV_TARGET;
		break;
	case TARGET_TYPE_QCA8074:
	case TARGET_TYPE_QCA8074V2:
		target = WMI_TLV_TARGET;
		break;
	case TARGET_TYPE_QCA6290:
		target = WMI_TLV_TARGET;
		break;
	case TARGET_TYPE_QCA6018:
		target = WMI_TLV_TARGET;
		break;
	default:
		temp_phymode = WMI_HOST_MODE_UNKNOWN;
		return temp_phymode;
	}

	temp_phymode = phy_2_wmi[phy_mode];

	if ((target == WMI_TLV_TARGET) && (is_2gvht_en == true)) {
		switch (phy_mode) {
		case WLAN_PHYMODE_11NG_HT20:
			temp_phymode = WMI_HOST_MODE_11AC_VHT20_2G;
			break;

		case WLAN_PHYMODE_11NG_HT40PLUS:
		case WLAN_PHYMODE_11NG_HT40MINUS:
		case WLAN_PHYMODE_11NG_HT40:
			temp_phymode = WMI_HOST_MODE_11AC_VHT40_2G;
			break;
		}
	}

	return temp_phymode;
}

static QDF_STATUS target_if_vdev_mgr_set_param_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_set_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	int param_id;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}
	param_id = target_if_vdev_mlme_id_2_wmi(param->param_id);
	param->param_id = param_id;

	status = wmi_unified_vdev_set_param_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_create_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_create_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	uint8_t vap_addr[QDF_MAC_ADDR_SIZE] = {0};

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	WLAN_ADDR_COPY(vap_addr, wlan_vdev_mlme_get_macaddr(vdev));
	status = wmi_unified_vdev_create_send(wmi_handle, vap_addr,
					      param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_start_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_start_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	enum wlan_phymode phy_mode;
	uint32_t is_2gvht;
	uint32_t is_11ax_stub_enabled;
	struct vdev_response_timer *vdev_rsp;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->vdev_mgr_get_response_timer_info) {
		mlme_err("No Rx Ops");
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->vdev_mgr_get_response_timer_info(vdev);
	if (vdev_rsp)
		qdf_timer_start(&vdev_rsp->rsp_timer, START_RESPONSE_TIMER);

	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_2G_VHT, &is_2gvht);
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_11AX_STUB,
				 &is_11ax_stub_enabled);

	phy_mode = target_if_wmi_phymode_from_wlan_phymode(
							vdev,
							param->channel.phy_mode,
							is_2gvht,
							is_11ax_stub_enabled);
	param->channel.phy_mode = phy_mode;
	status = wmi_unified_vdev_start_send(wmi_handle, param);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (qdf_atomic_test_and_set_bit(START_RESPONSE_BIT,
						&vdev_rsp->rsp_status))
			mlme_debug("Response bit already set");
	} else {
		qdf_timer_stop(&vdev_rsp->rsp_timer);
	}

	return status;
}

static QDF_STATUS target_if_vdev_mgr_delete_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_delete_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_delete_send(wmi_handle, param->vdev_id);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_stop_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_stop_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	struct vdev_response_timer *vdev_rsp;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);

	if (!rx_ops && !rx_ops->vdev_mgr_get_response_timer_info) {
		mlme_err("No Rx Ops");
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->vdev_mgr_get_response_timer_info(vdev);
	if (vdev_rsp)
		qdf_timer_start(&vdev_rsp->rsp_timer, STOP_RESPONSE_TIMER);

	status = wmi_unified_vdev_stop_send(wmi_handle, param->vdev_id);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (qdf_atomic_test_and_set_bit(STOP_RESPONSE_BIT,
						&vdev_rsp->rsp_status))
			mlme_debug("Response bit already set");
	} else {
		qdf_timer_stop(&vdev_rsp->rsp_timer);
	}

	return status;
}

static QDF_STATUS target_if_vdev_mgr_down_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_down_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_down_send(wmi_handle, param->vdev_id);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_up_send(
					struct wlan_objmgr_vdev *vdev,
					struct vdev_up_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct vdev_set_params sparam = {0};
	uint8_t bssid_null[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	uint8_t *bssid = wlan_vdev_mlme_get_macaddr(vdev);
	uint8_t *bssid_send;
	enum QDF_OPMODE opmode;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	sparam.vdev_id = wlan_vdev_get_id(vdev);

	sparam.param_id = WLAN_MLME_CFG_BEACON_INTERVAL;
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_BEACON_INTERVAL,
				 &sparam.param_value);
	status = target_if_vdev_mgr_set_param_send(vdev, &sparam);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("Failed to set beacon interval!");

	sparam.param_id = WLAN_MLME_CFG_SUBFEE;
	wlan_util_vdev_get_param(vdev, WLAN_MLME_CFG_SUBFEE,
				 &sparam.param_value);
	status = target_if_vdev_mgr_set_param_send(vdev, &sparam);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("Failed to set SU beam formee!");

	bssid_send = (opmode == QDF_MONITOR_MODE) ? bssid_null : bssid;
	status = wmi_unified_vdev_up_send(wmi_handle, bssid_send, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_beacon_tmpl_send(
					struct wlan_objmgr_vdev *vdev,
					struct beacon_tmpl_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_beacon_tmpl_send_cmd(wmi_handle, param);
	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_nac_rssi_send(
				struct wlan_objmgr_vdev *vdev,
				struct vdev_scan_nac_rssi_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_nac_rssi_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_neighbour_rx_cmd_send(
					struct wlan_objmgr_vdev *vdev,
					struct set_neighbour_rx_params *param,
					uint8_t *mac)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_neighbour_rx_cmd_send(wmi_handle,
							    mac, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_sifs_trigger_send(
					struct wlan_objmgr_vdev *vdev,
					struct sifs_trigger_param *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_sifs_trigger_send(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_set_custom_aggr_size_cmd_send(
				struct wlan_objmgr_vdev *vdev,
				struct set_custom_aggr_size_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_set_custom_aggr_size_cmd_send(wmi_handle,
								param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_config_ratemask_cmd_send(
					struct wlan_objmgr_vdev *vdev,
					struct config_ratemask_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_vdev_config_ratemask_cmd_send(wmi_handle,
							   param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_peer_flush_tids_send(
					struct wlan_objmgr_vdev *vdev,
					struct peer_flush_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_peer_flush_tids_send(wmi_handle, param->peer_mac,
						  param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_multiple_vdev_restart_req_cmd(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_restart_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mlme_err("PDEV WMI Handle is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_send_multiple_vdev_restart_req_cmd(wmi_handle,
								param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_beacon_send(
					struct wlan_objmgr_vdev *vdev,
					struct beacon_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_beacon_send_cmd(wmi_handle, param);

	return status;
}

static QDF_STATUS target_if_vdev_mgr_sta_ps_param_send(
					struct wlan_objmgr_vdev *vdev,
					struct sta_ps_params *param)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	int param_id;

	if (!vdev || !param) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_if_vdev_mgr_wmi_handle_get(vdev);
	if (!wmi_handle) {
		mlme_err("Failed to get WMI handle!");
		return QDF_STATUS_E_INVAL;
	}

	param_id = target_if_vdev_mlme_id_2_wmi(param->param);
	param->param = param_id;

	status = wmi_unified_sta_ps_cmd_send(wmi_handle, param);

	return status;
}

QDF_STATUS
target_if_vdev_mgr_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_mlme_tx_ops *mlme_tx_ops;

	if (!tx_ops) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	mlme_tx_ops = &tx_ops->mops;
	if (!mlme_tx_ops) {
		mlme_err("No Tx Ops");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_tx_ops->vdev_mlme_attach =
			target_if_vdev_mgr_register_event_handler;
	mlme_tx_ops->vdev_mlme_detach =
			target_if_vdev_mgr_unregister_event_handler;
	mlme_tx_ops->vdev_create_send = target_if_vdev_mgr_create_send;
	mlme_tx_ops->vdev_start_send = target_if_vdev_mgr_start_send;
	mlme_tx_ops->vdev_up_send = target_if_vdev_mgr_up_send;
	mlme_tx_ops->vdev_delete_send = target_if_vdev_mgr_delete_send;
	mlme_tx_ops->vdev_stop_send = target_if_vdev_mgr_stop_send;
	mlme_tx_ops->vdev_down_send = target_if_vdev_mgr_down_send;
	mlme_tx_ops->vdev_set_nac_rssi_send =
			target_if_vdev_mgr_set_nac_rssi_send;
	mlme_tx_ops->vdev_set_neighbour_rx_cmd_send =
			target_if_vdev_mgr_set_neighbour_rx_cmd_send;
	mlme_tx_ops->vdev_sifs_trigger_send =
			target_if_vdev_mgr_sifs_trigger_send;
	mlme_tx_ops->vdev_set_custom_aggr_size_cmd_send =
			target_if_vdev_mgr_set_custom_aggr_size_cmd_send;
	mlme_tx_ops->vdev_config_ratemask_cmd_send =
			target_if_vdev_mgr_config_ratemask_cmd_send;
	mlme_tx_ops->peer_flush_tids_send =
			target_if_vdev_mgr_peer_flush_tids_send;
	mlme_tx_ops->multiple_vdev_restart_req_cmd =
			target_if_vdev_mgr_multiple_vdev_restart_req_cmd;
	mlme_tx_ops->beacon_cmd_send = target_if_vdev_mgr_beacon_send;
	mlme_tx_ops->beacon_tmpl_send = target_if_vdev_mgr_beacon_tmpl_send;
	mlme_tx_ops->vdev_set_param_send =
			target_if_vdev_mgr_set_param_send;
	mlme_tx_ops->vdev_sta_ps_param_send =
			target_if_vdev_mgr_sta_ps_param_send;
	mlme_tx_ops->target_is_pre_lithium =
			target_if_check_is_pre_lithium;
	mlme_tx_ops->vdev_mgr_resp_timer_mgmt =
			target_if_vdev_mgr_rsp_timer_mgmt;

	return QDF_STATUS_SUCCESS;
}
