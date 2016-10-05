/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 *  DOC:    wma_power.c
 *  This file contains powersave related functions.
 */

/* Header files */

#include "wma.h"
#include "wma_api.h"
#include "cds_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wni_api.h"
#include "ani_global.h"
#include "wmi_unified.h"
#include "wni_cfg.h"
#include "cfg_api.h"

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"
#include "ol_txrx_peer_find.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"
#include "ol_fw.h"

#include "dfs.h"
#include "wma_internal.h"

/**
 * wma_unified_modem_power_state() - set modem power state to fw
 * @wmi_handle: wmi handle
 * @param_value: parameter value
 *
 * Return: 0 for success or error code
 */
static int
wma_unified_modem_power_state(wmi_unified_t wmi_handle, uint32_t param_value)
{
	int ret;
	wmi_modem_power_state_cmd_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_modem_power_state_cmd_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_modem_power_state_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_modem_power_state_cmd_param));
	cmd->modem_power_state = param_value;
	WMA_LOGD("%s: Setting cmd->modem_power_state = %u", __func__,
		 param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_MODEM_POWER_STATE_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send notify cmd ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * wma_unified_set_sta_ps_param() - set sta power save parameter to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @param: param
 * @value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_unified_set_sta_ps_param(wmi_unified_t wmi_handle,
					    uint32_t vdev_id, uint32_t param,
					    uint32_t value)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface;
	struct sta_ps_params sta_ps_param = {0};
	QDF_STATUS status;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (NULL == wma) {
		WMA_LOGE("%s: wma is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	WMA_LOGD("Set Sta Ps param vdevId %d Param %d val %d",
		 vdev_id, param, value);
	iface = &wma->interfaces[vdev_id];

	sta_ps_param.vdev_id = vdev_id;
	sta_ps_param.param = param;
	sta_ps_param.value = value;
	status = wmi_unified_sta_ps_cmd_send(wmi_handle, &sta_ps_param);
	if (QDF_IS_STATUS_ERROR(status))
		return status;
	/* Store the PS Status */
	iface->ps_enabled = value ? true : false;

	return status;
}

#ifdef QCA_IBSS_SUPPORT
/**
 * wma_set_ibss_pwrsave_params() - set ibss power save parameter to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
wma_set_ibss_pwrsave_params(tp_wma_handle wma, uint8_t vdev_id)
{
	QDF_STATUS ret;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_ATIM_WINDOW_LENGTH,
			wma->wma_ibss_power_save_params.atimWindowLength);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed to set WMI_VDEV_PARAM_ATIM_WINDOW_LENGTH ret = %d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_IS_IBSS_POWER_SAVE_ALLOWED,
			wma->wma_ibss_power_save_params.isPowerSaveAllowed);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_IS_IBSS_POWER_SAVE_ALLOWED ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_IS_POWER_COLLAPSE_ALLOWED,
			wma->wma_ibss_power_save_params.isPowerCollapseAllowed);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_IS_POWER_COLLAPSE_ALLOWED ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			 WMI_VDEV_PARAM_IS_AWAKE_ON_TXRX_ENABLED,
			 wma->wma_ibss_power_save_params.isAwakeonTxRxEnabled);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_IS_AWAKE_ON_TXRX_ENABLED ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_INACTIVITY_CNT,
			wma->wma_ibss_power_save_params.inactivityCount);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_INACTIVITY_CNT ret=%d",
			 ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_TXSP_END_INACTIVITY_TIME_MS,
			wma->wma_ibss_power_save_params.txSPEndInactivityTime);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_TXSP_END_INACTIVITY_TIME_MS ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_IBSS_PS_WARMUP_TIME_SECS,
			wma->wma_ibss_power_save_params.ibssPsWarmupTime);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed, set WMI_VDEV_PARAM_IBSS_PS_WARMUP_TIME_SECS ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_ENABLE,
			wma->wma_ibss_power_save_params.ibssPs1RxChainInAtimEnable);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed to set IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_ENABLE ret=%d",
			ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_IBSS_SUPPORT */

/**
 * wma_set_ap_peer_uapsd() - set powersave parameters in ap mode to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @peer_addr: peer mac address
 * @uapsd_value: uapsd value
 * @max_sp: maximum service period
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_set_ap_peer_uapsd(tp_wma_handle wma, uint32_t vdev_id,
			      uint8_t *peer_addr, uint8_t uapsd_value,
			      uint8_t max_sp)
{
	uint32_t uapsd = 0;
	uint32_t max_sp_len = 0;
	QDF_STATUS ret;
	struct ap_ps_params param = {0};

	if (uapsd_value & UAPSD_VO_ENABLED) {
		uapsd |= WMI_AP_PS_UAPSD_AC3_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC3_TRIGGER_EN;
	}

	if (uapsd_value & UAPSD_VI_ENABLED) {
		uapsd |= WMI_AP_PS_UAPSD_AC2_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC2_TRIGGER_EN;
	}

	if (uapsd_value & UAPSD_BK_ENABLED) {
		uapsd |= WMI_AP_PS_UAPSD_AC1_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC1_TRIGGER_EN;
	}

	if (uapsd_value & UAPSD_BE_ENABLED) {
		uapsd |= WMI_AP_PS_UAPSD_AC0_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC0_TRIGGER_EN;
	}

	switch (max_sp) {
	case UAPSD_MAX_SP_LEN_2:
		max_sp_len = WMI_AP_PS_PEER_PARAM_MAX_SP_2;
		break;
	case UAPSD_MAX_SP_LEN_4:
		max_sp_len = WMI_AP_PS_PEER_PARAM_MAX_SP_4;
		break;
	case UAPSD_MAX_SP_LEN_6:
		max_sp_len = WMI_AP_PS_PEER_PARAM_MAX_SP_6;
		break;
	default:
		max_sp_len = WMI_AP_PS_PEER_PARAM_MAX_SP_UNLIMITED;
		break;
	}

	WMA_LOGD("Set WMI_AP_PS_PEER_PARAM_UAPSD 0x%x for %pM",
		 uapsd, peer_addr);
	param.vdev_id = vdev_id;
	param.param = WMI_AP_PS_PEER_PARAM_UAPSD;
	param.value = uapsd;
	ret = wmi_unified_ap_ps_cmd_send(wma->wmi_handle, peer_addr,
						&param);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed to set WMI_AP_PS_PEER_PARAM_UAPSD for %pM",
			 peer_addr);
		return ret;
	}

	WMA_LOGD("Set WMI_AP_PS_PEER_PARAM_MAX_SP 0x%x for %pM",
		 max_sp_len, peer_addr);

	param.vdev_id = vdev_id;
	param.param = WMI_AP_PS_PEER_PARAM_MAX_SP;
	param.value = max_sp_len;
	ret = wmi_unified_ap_ps_cmd_send(wma->wmi_handle, peer_addr,
					  &param);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Failed to set WMI_AP_PS_PEER_PARAM_MAX_SP for %pM",
			 peer_addr);
		return ret;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_update_edca_params_for_ac() - to update per ac EDCA parameters
 * @edca_param: EDCA parameters
 * @wmm_param: wmm parameters
 * @ac: access category
 *
 * Return: none
 */
void wma_update_edca_params_for_ac(tSirMacEdcaParamRecord *edca_param,
				   wmi_wmm_vparams *wmm_param, int ac)
{
#define WMA_WMM_EXPO_TO_VAL(val)        ((1 << (val)) - 1)
	wmm_param->cwmin = WMA_WMM_EXPO_TO_VAL(edca_param->cw.min);
	wmm_param->cwmax = WMA_WMM_EXPO_TO_VAL(edca_param->cw.max);
	wmm_param->aifs = edca_param->aci.aifsn;
	wmm_param->txoplimit = edca_param->txoplimit;
	wmm_param->acm = edca_param->aci.acm;

	/* TODO: No ack is not present in EdcaParamRecord */
	wmm_param->no_ack = 0;

	WMA_LOGI("WMM PARAMS AC[%d]: AIFS %d Min %d Max %d TXOP %d ACM %d NOACK %d",
		ac, wmm_param->aifs, wmm_param->cwmin, wmm_param->cwmax,
		wmm_param->txoplimit, wmm_param->acm, wmm_param->no_ack);
}

/**
 * wma_set_tx_power() - set tx power limit in fw
 * @handle: wma handle
 * @tx_pwr_params: tx power parameters
 *
 * Return: none
 */
void wma_set_tx_power(WMA_HANDLE handle,
		      tMaxTxPowerParams *tx_pwr_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint8_t vdev_id;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;
	void *pdev;

	if (tx_pwr_params->dev_mode == QDF_SAP_MODE ||
	    tx_pwr_params->dev_mode == QDF_P2P_GO_MODE) {
		pdev = wma_find_vdev_by_addr(wma_handle,
					     tx_pwr_params->bssId.bytes,
					     &vdev_id);
	} else {
		pdev = wma_find_vdev_by_bssid(wma_handle,
					      tx_pwr_params->bssId.bytes,
					      &vdev_id);
	}
	if (!pdev) {
		WMA_LOGE("vdev handle is invalid for %pM",
			 tx_pwr_params->bssId.bytes);
		qdf_mem_free(tx_pwr_params);
		return;
	}

	if (!(wma_handle->interfaces[vdev_id].vdev_up)) {
		WMA_LOGE("%s: vdev id %d is not up for %pM", __func__, vdev_id,
			 tx_pwr_params->bssId.bytes);
		qdf_mem_free(tx_pwr_params);
		return;
	}

	if (tx_pwr_params->power == 0) {
		/* set to default. Since the app does not care the tx power
		 * we keep the previous setting */
		wma_handle->interfaces[vdev_id].tx_power = 0;
		ret = 0;
		goto end;
	}
	if (wma_handle->interfaces[vdev_id].max_tx_power != 0) {
		/* make sure tx_power less than max_tx_power */
		if (tx_pwr_params->power >
		    wma_handle->interfaces[vdev_id].max_tx_power) {
			tx_pwr_params->power =
				wma_handle->interfaces[vdev_id].max_tx_power;
		}
	}
	if (wma_handle->interfaces[vdev_id].tx_power != tx_pwr_params->power) {

		/* tx_power changed, Push the tx_power to FW */
		WMA_LOGI("%s: Set TX pwr limit [WMI_VDEV_PARAM_TX_PWRLIMIT] to %d",
			__func__, tx_pwr_params->power);
		ret = wma_vdev_set_param(wma_handle->wmi_handle,
						      vdev_id,
						      WMI_VDEV_PARAM_TX_PWRLIMIT,
						      tx_pwr_params->power);
		if (ret == QDF_STATUS_SUCCESS)
			wma_handle->interfaces[vdev_id].tx_power =
				tx_pwr_params->power;
	} else {
		/* no tx_power change */
		ret = QDF_STATUS_SUCCESS;
	}
end:
	qdf_mem_free(tx_pwr_params);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to set vdev param WMI_VDEV_PARAM_TX_PWRLIMIT");
}

/**
 * wma_set_max_tx_power() - set max tx power limit in fw
 * @handle: wma handle
 * @tx_pwr_params: tx power parameters
 *
 * Return: none
 */
void wma_set_max_tx_power(WMA_HANDLE handle,
			  tMaxTxPowerParams *tx_pwr_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint8_t vdev_id;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;
	void *pdev;
	int8_t prev_max_power;

	pdev = wma_find_vdev_by_addr(wma_handle, tx_pwr_params->bssId.bytes,
				     &vdev_id);
	if (pdev == NULL) {
		/* not in SAP array. Try the station/p2p array */
		pdev = wma_find_vdev_by_bssid(wma_handle,
					      tx_pwr_params->bssId.bytes,
					      &vdev_id);
	}
	if (!pdev) {
		WMA_LOGE("vdev handle is invalid for %pM",
			 tx_pwr_params->bssId.bytes);
		qdf_mem_free(tx_pwr_params);
		return;
	}

	if (!(wma_handle->interfaces[vdev_id].vdev_up)) {
		WMA_LOGE("%s: vdev id %d is not up", __func__, vdev_id);
		qdf_mem_free(tx_pwr_params);
		return;
	}

	if (wma_handle->interfaces[vdev_id].max_tx_power ==
	    tx_pwr_params->power) {
		ret = QDF_STATUS_SUCCESS;
		goto end;
	}
	prev_max_power = wma_handle->interfaces[vdev_id].max_tx_power;
	wma_handle->interfaces[vdev_id].max_tx_power = tx_pwr_params->power;
	if (wma_handle->interfaces[vdev_id].max_tx_power == 0) {
		ret = QDF_STATUS_SUCCESS;
		goto end;
	}
	WMA_LOGI("Set MAX TX pwr limit [WMI_VDEV_PARAM_TX_PWRLIMIT] to %d",
		 wma_handle->interfaces[vdev_id].max_tx_power);
	ret = wma_vdev_set_param(wma_handle->wmi_handle, vdev_id,
				WMI_VDEV_PARAM_TX_PWRLIMIT,
				wma_handle->interfaces[vdev_id].max_tx_power);
	if (ret == QDF_STATUS_SUCCESS)
		wma_handle->interfaces[vdev_id].tx_power =
			wma_handle->interfaces[vdev_id].max_tx_power;
	else
		wma_handle->interfaces[vdev_id].max_tx_power = prev_max_power;
end:
	qdf_mem_free(tx_pwr_params);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("%s: Failed to set vdev param WMI_VDEV_PARAM_TX_PWRLIMIT",
			__func__);
}

/**
 * wmi_unified_set_sta_ps() - set sta powersave params in fw
 * @handle: wma handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_set_sta_ps(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val)
{
	QDF_STATUS ret;

	ret = wmi_unified_set_sta_ps_mode(wmi_handle, vdev_id,
				   val);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set Mimo PS ret = %d", ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_uapsd_mask() - get uapsd mask based on uapsd parameters
 * @uapsd_params: uapsed parameters
 *
 * Return: uapsd mask
 */
static inline uint32_t wma_get_uapsd_mask(tpUapsd_Params uapsd_params)
{
	uint32_t uapsd_val = 0;

	if (uapsd_params->beDeliveryEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC0_DELIVERY_EN;

	if (uapsd_params->beTriggerEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC0_TRIGGER_EN;

	if (uapsd_params->bkDeliveryEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC1_DELIVERY_EN;

	if (uapsd_params->bkTriggerEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC1_TRIGGER_EN;

	if (uapsd_params->viDeliveryEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC2_DELIVERY_EN;

	if (uapsd_params->viTriggerEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC2_TRIGGER_EN;

	if (uapsd_params->voDeliveryEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC3_DELIVERY_EN;

	if (uapsd_params->voTriggerEnabled)
		uapsd_val |= WMI_STA_PS_UAPSD_AC3_TRIGGER_EN;

	return uapsd_val;
}

/**
 * wma_set_force_sleep() - set power save parameters to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @enable: enable/disable
 * @qpower_config: qpower configuration
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS wma_set_force_sleep(tp_wma_handle wma,
				uint32_t vdev_id,
				uint8_t enable,
				enum powersave_qpower_mode qpower_config,
				bool enable_ps)
{
	QDF_STATUS ret;
	uint32_t cfg_data_val = 0;
	/* get mac to acess CFG data base */
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t rx_wake_policy;
	uint32_t tx_wake_threshold;
	uint32_t pspoll_count;
	uint32_t inactivity_time;
	uint32_t psmode;

	WMA_LOGD("Set Force Sleep vdevId %d val %d", vdev_id, enable);

	if (NULL == mac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	/* Set Tx/Rx Data InActivity Timeout   */
	if (wlan_cfg_get_int(mac, WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT,
			     &cfg_data_val) != eSIR_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "Failed to get WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT");
		cfg_data_val = POWERSAVE_DEFAULT_INACTIVITY_TIME;
	}
	inactivity_time = (uint32_t) cfg_data_val;

	if (enable) {
		/* override normal configuration and force station asleep */
		rx_wake_policy = WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD;
		tx_wake_threshold = WMI_STA_PS_TX_WAKE_THRESHOLD_NEVER;

		if (wlan_cfg_get_int(mac, WNI_CFG_MAX_PS_POLL,
				     &cfg_data_val) != eSIR_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
				  "Failed to get value for WNI_CFG_MAX_PS_POLL");
		}
		if (cfg_data_val)
			pspoll_count = (uint32_t) cfg_data_val;
		else
			pspoll_count = WMA_DEFAULT_MAX_PSPOLL_BEFORE_WAKE;

		psmode = WMI_STA_PS_MODE_ENABLED;
	} else {
		/* Ps Poll Wake Policy */
		if (wlan_cfg_get_int(mac, WNI_CFG_MAX_PS_POLL,
				     &cfg_data_val) != eSIR_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
				  "Failed to get value for WNI_CFG_MAX_PS_POLL");
		}
		if (cfg_data_val) {
			/* Ps Poll is enabled */
			rx_wake_policy = WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD;
			pspoll_count = (uint32_t) cfg_data_val;
			tx_wake_threshold = WMI_STA_PS_TX_WAKE_THRESHOLD_NEVER;
		} else {
			rx_wake_policy = WMI_STA_PS_RX_WAKE_POLICY_WAKE;
			pspoll_count = WMI_STA_PS_PSPOLL_COUNT_NO_MAX;
			tx_wake_threshold = WMI_STA_PS_TX_WAKE_THRESHOLD_ALWAYS;
		}
		psmode = WMI_STA_PS_MODE_ENABLED;
	}

	/*
	 * QPower is enabled by default in Firmware
	 * So Disable QPower explicitly
	 */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_ENABLE_QPOWER,
					   qpower_config);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("%s(%d) QPower Failed vdevId %d",
			qpower_config ? "Enable" : "Disable",
			qpower_config, vdev_id);
		return ret;
	}
	WMA_LOGD("QPower %s(%d) vdevId %d",
			qpower_config ? "Enabled" : "Disabled",
			qpower_config, vdev_id);

	/* Set the Wake Policy to WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_RX_WAKE_POLICY,
					   rx_wake_policy);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Setting wake policy Failed vdevId %d", vdev_id);
		return ret;
	}
	WMA_LOGD("Setting wake policy to %d vdevId %d",
		 rx_wake_policy, vdev_id);

	/* Set the Tx Wake Threshold */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD,
					   tx_wake_threshold);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Setting TxWake Threshold vdevId %d", vdev_id);
		return ret;
	}
	WMA_LOGD("Setting TxWake Threshold to %d vdevId %d",
		 tx_wake_threshold, vdev_id);

	/* Set the Ps Poll Count */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_PSPOLL_COUNT,
					   pspoll_count);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Set Ps Poll Count Failed vdevId %d ps poll cnt %d",
			 vdev_id, pspoll_count);
		return ret;
	}
	WMA_LOGD("Set Ps Poll Count vdevId %d ps poll cnt %d",
		 vdev_id, pspoll_count);

	/* Set the Tx/Rx InActivity */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_INACTIVITY_TIME,
					   inactivity_time);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Setting Tx/Rx InActivity Failed vdevId %d InAct %d",
			 vdev_id, inactivity_time);
		return ret;
	}
	WMA_LOGD("Set Tx/Rx InActivity vdevId %d InAct %d",
		 vdev_id, inactivity_time);

	/* Enable Sta Mode Power save */
	if (enable_ps) {
		ret = wmi_unified_set_sta_ps(wma->wmi_handle, vdev_id, true);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Enable Sta Mode Ps Failed vdevId %d",
				vdev_id);
			return ret;
		}
	}

	/* Set Listen Interval */
	if (wlan_cfg_get_int(mac, WNI_CFG_LISTEN_INTERVAL,
			     &cfg_data_val) != eSIR_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "Failed to get value for WNI_CFG_LISTEN_INTERVAL");
		cfg_data_val = POWERSAVE_DEFAULT_LISTEN_INTERVAL;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_LISTEN_INTERVAL,
					      cfg_data_val);
	if (QDF_IS_STATUS_ERROR(ret)) {
		/* Even it fails continue Fw will take default LI */
		WMA_LOGE("Failed to Set Listen Interval vdevId %d", vdev_id);
	}
	WMA_LOGD("Set Listen Interval vdevId %d Listen Intv %d",
		 vdev_id, cfg_data_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_qpower_force_sleep() - set qpower params in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @enable: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_set_qpower_force_sleep(tp_wma_handle wma, uint32_t vdev_id,
				   uint8_t enable)
{
	QDF_STATUS ret;
	uint32_t cfg_data_val = 0;
	/* get mac to acess CFG data base */
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t pspoll_count = WMA_DEFAULT_MAX_PSPOLL_BEFORE_WAKE;

	WMA_LOGE("Set QPower Force(1)/Normal(0) Sleep vdevId %d val %d",
		 vdev_id, enable);

	if (NULL == mac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	/* Get Configured Ps Poll Count */
	if (wlan_cfg_get_int(mac, WNI_CFG_MAX_PS_POLL,
			     &cfg_data_val) != eSIR_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "Failed to get value for WNI_CFG_MAX_PS_POLL");
	}
	if (cfg_data_val) {
		pspoll_count = (uint32_t) cfg_data_val;
	}

	/* Enable QPower */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_ENABLE_QPOWER, 1);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Enable QPower Failed vdevId %d", vdev_id);
		return ret;
	}
	WMA_LOGD("QPower Enabled vdevId %d", vdev_id);

	/* Set the Wake Policy to WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_RX_WAKE_POLICY,
					   WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Setting wake policy to pspoll/uapsd Failed vdevId %d",
			 vdev_id);
		return ret;
	}
	WMA_LOGD("Wake policy set to to pspoll/uapsd vdevId %d", vdev_id);

	if (enable) {
		/* Set the Tx Wake Threshold */
		ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
						   WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD,
						   WMI_STA_PS_TX_WAKE_THRESHOLD_NEVER);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Setting TxWake Threshold vdevId %d", vdev_id);
			return ret;
		}
		WMA_LOGD("TxWake Threshold set to TX_WAKE_THRESHOLD_NEVER %d",
			 vdev_id);
	}

	/* Set the QPower Ps Poll Count */
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
					   pspoll_count);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Set QPower Ps Poll Count Failed vdevId %d ps poll cnt %d",
			vdev_id, pspoll_count);
		return ret;
	}
	WMA_LOGD("Set QPower Ps Poll Count vdevId %d ps poll cnt %d",
		 vdev_id, pspoll_count);

	/* Enable Sta Mode Power save */
	ret = wmi_unified_set_sta_ps(wma->wmi_handle, vdev_id, true);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Enable Sta Mode Ps Failed vdevId %d", vdev_id);
		return ret;
	}

	/* Set Listen Interval */
	if (wlan_cfg_get_int(mac, WNI_CFG_LISTEN_INTERVAL,
			     &cfg_data_val) != eSIR_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "Failed to get value for WNI_CFG_LISTEN_INTERVAL");
		cfg_data_val = POWERSAVE_DEFAULT_LISTEN_INTERVAL;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_LISTEN_INTERVAL,
					      cfg_data_val);
	if (QDF_IS_STATUS_ERROR(ret)) {
		/* Even it fails continue Fw will take default LI */
		WMA_LOGE("Failed to Set Listen Interval vdevId %d", vdev_id);
	}
	WMA_LOGD("Set Listen Interval vdevId %d Listen Intv %d",
		 vdev_id, cfg_data_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_qpower_config() - get qpower configuration
 * @wma: WMA handle
 *
 * Power Save Offload configuration:
 * 0 -> Power save offload is disabled
 * 1 -> Legacy Power save enabled + Deep sleep Disabled
 * 2 -> QPower enabled + Deep sleep Disabled
 * 3 -> Legacy Power save enabled + Deep sleep Enabled
 * 4 -> QPower enabled + Deep sleep Enabled
 * 5 -> Duty cycling QPower enabled
 *
 * Return: enum powersave_qpower_mode with below values
 * QPOWER_DISABLED if QPOWER is disabled
 * QPOWER_ENABLED if QPOWER is enabled
 * QPOWER_DUTY_CYCLING if DUTY CYCLING QPOWER is enabled
 */
static enum powersave_qpower_mode wma_get_qpower_config(tp_wma_handle wma)
{
	switch (wma->powersave_mode) {
	case PS_QPOWER_NODEEPSLEEP:
	case PS_QPOWER_DEEPSLEEP:
		WMA_LOGI("QPOWER is enabled in power save mode %d",
			wma->powersave_mode);
		return QPOWER_ENABLED;
	case PS_DUTY_CYCLING_QPOWER:
		WMA_LOGI("DUTY cycling QPOWER is enabled in power save mode %d",
			wma->powersave_mode);
		return QPOWER_DUTY_CYCLING;

	default:
		WMA_LOGI("QPOWER is disabled in power save mode %d",
			wma->powersave_mode);
		return QPOWER_DISABLED;
	}
}

/**
 * wma_enable_sta_ps_mode() - enable sta powersave params in fw
 * @wma: wma handle
 * @ps_req: power save request
 *
 * Return: none
 */
void wma_enable_sta_ps_mode(tp_wma_handle wma, tpEnablePsParams ps_req)
{
	uint32_t vdev_id = ps_req->sessionid;
	QDF_STATUS ret;
	enum powersave_qpower_mode qpower_config = wma_get_qpower_config(wma);
	struct wma_txrx_node *iface = &wma->interfaces[vdev_id];

	if (!iface->handle) {
		WMA_LOGE("vdev id %d is not active", vdev_id);
		return;
	}
	if (eSIR_ADDON_NOTHING == ps_req->psSetting) {
		WMA_LOGD("Enable Sta Mode Ps vdevId %d", vdev_id);
		ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
				WMI_STA_PS_PARAM_UAPSD, 0);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Set Uapsd param 0 Failed vdevId %d", vdev_id);
			return;
		}

		ret = wma_set_force_sleep(wma, vdev_id, false,
				qpower_config, true);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Enable Sta Ps Failed vdevId %d", vdev_id);
			return;
		}
	} else if (eSIR_ADDON_ENABLE_UAPSD == ps_req->psSetting) {
		uint32_t uapsd_val = 0;
		uapsd_val = wma_get_uapsd_mask(&ps_req->uapsdParams);

		if (uapsd_val != iface->uapsd_cached_val) {
			WMA_LOGD("Enable Uapsd vdevId %d Mask %d",
					vdev_id, uapsd_val);
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
					vdev_id,
					WMI_STA_PS_PARAM_UAPSD,
					uapsd_val);
			if (QDF_IS_STATUS_ERROR(ret)) {
				WMA_LOGE("Enable Uapsd Failed vdevId %d",
						vdev_id);
				return;
			}
			/* Cache the Uapsd Mask */
			iface->uapsd_cached_val = uapsd_val;
		} else {
			WMA_LOGD("Already Uapsd Enabled vdevId %d Mask %d",
					vdev_id, uapsd_val);
		}

		WMA_LOGD("Enable Forced Sleep vdevId %d", vdev_id);
		ret = wma_set_force_sleep(wma, vdev_id, true,
				qpower_config, true);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Enable Forced Sleep Failed vdevId %d",
					vdev_id);
			return;
		}
	}
}

/**
 * wma_disable_sta_ps_mode() - disable sta powersave params in fw
 * @wma: wma handle
 * @ps_req: power save request
 *
 * Return: none
 */
void wma_disable_sta_ps_mode(tp_wma_handle wma, tpDisablePsParams ps_req)
{
	QDF_STATUS ret;
	uint32_t vdev_id = ps_req->sessionid;

	WMA_LOGD("Disable Sta Mode Ps vdevId %d", vdev_id);

	/* Disable Sta Mode Power save */
	ret = wmi_unified_set_sta_ps(wma->wmi_handle, vdev_id, false);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Sta Mode Ps Failed vdevId %d", vdev_id);
		return;
	}

	/* Disable UAPSD incase if additional Req came */
	if (eSIR_ADDON_DISABLE_UAPSD == ps_req->psSetting) {
		WMA_LOGD("Disable Uapsd vdevId %d", vdev_id);
		ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
				WMI_STA_PS_PARAM_UAPSD, 0);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Disable Uapsd Failed vdevId %d", vdev_id);
			/*
			 * Even this fails we can proceed as success
			 * since we disabled powersave
			 */
		}
	}
}

/**
 * wma_set_powersave_config() - update power save config in wma
 * @val: new power save value
 *
 * This function update qpower value in wma layer
 *
 * Return: QDF_STATUS_SUCCESS on success, error number otherwise
 */
QDF_STATUS wma_set_powersave_config(uint8_t val)
{
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("%s: WMA context is invald!", __func__);
		return QDF_STATUS_E_INVAL;
	}
	wma_handle->powersave_mode = val;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_enable_uapsd_mode() - enable uapsd mode in fw
 * @wma: wma handle
 * @ps_req: power save request
 *
 * Return: none
 */
void wma_enable_uapsd_mode(tp_wma_handle wma, tpEnableUapsdParams ps_req)
{
	QDF_STATUS ret;
	uint32_t vdev_id = ps_req->sessionid;
	uint32_t uapsd_val = 0;
	enum powersave_qpower_mode qpower_config = wma_get_qpower_config(wma);

	/* Disable Sta Mode Power save */
	ret = wmi_unified_set_sta_ps(wma->wmi_handle, vdev_id, false);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Sta Mode Ps Failed vdevId %d", vdev_id);
		return;
	}

	uapsd_val = wma_get_uapsd_mask(&ps_req->uapsdParams);

	WMA_LOGD("Enable Uapsd vdevId %d Mask %d", vdev_id, uapsd_val);
	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
			WMI_STA_PS_PARAM_UAPSD, uapsd_val);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Enable Uapsd Failed vdevId %d", vdev_id);
		return;
	}

	WMA_LOGD("Enable Forced Sleep vdevId %d", vdev_id);
	ret = wma_set_force_sleep(wma, vdev_id, true,
			qpower_config, ps_req->uapsdParams.enable_ps);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Enable Forced Sleep Failed vdevId %d", vdev_id);
		return;
	}

}

/**
 * wma_disable_uapsd_mode() - disable uapsd mode in fw
 * @wma: wma handle
 * @ps_req: power save request
 *
 * Return: none
 */
void wma_disable_uapsd_mode(tp_wma_handle wma,
			    tpDisableUapsdParams ps_req)
{
	QDF_STATUS ret;
	uint32_t vdev_id = ps_req->sessionid;
	enum powersave_qpower_mode qpower_config = wma_get_qpower_config(wma);

	WMA_LOGD("Disable Uapsd vdevId %d", vdev_id);

	/* Disable Sta Mode Power save */
	ret = wmi_unified_set_sta_ps(wma->wmi_handle, vdev_id, false);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Sta Mode Ps Failed vdevId %d", vdev_id);
		return;
	}

	ret = wma_unified_set_sta_ps_param(wma->wmi_handle, vdev_id,
			WMI_STA_PS_PARAM_UAPSD, 0);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Uapsd Failed vdevId %d", vdev_id);
		return;
	}

	/* Re enable Sta Mode Powersave with proper configuration */
	ret = wma_set_force_sleep(wma, vdev_id, false,
			qpower_config, true);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Forced Sleep Failed vdevId %d", vdev_id);
		return;
	}
}

/**
 * wma_set_sta_uapsd_auto_trig_cmd() - set uapsd auto trigger command
 * @wmi_handle: wma handle
 * @vdevid: vdev id
 * @peer_addr: peer mac address
 * @autoTriggerparam: auto trigger parameters
 * @num_ac: number of access category
 *
 * This function sets the trigger
 * uapsd params such as service interval, delay interval
 * and suspend interval which will be used by the firmware
 * to send trigger frames periodically when there is no
 * traffic on the transmit side.
 *
 * Return: 0 for success or error code.
 */
static QDF_STATUS wma_set_sta_uapsd_auto_trig_cmd(wmi_unified_t wmi_handle,
					uint32_t vdevid,
					uint8_t peer_addr[IEEE80211_ADDR_LEN],
					uint8_t *autoTriggerparam,
					uint32_t num_ac)
{
	QDF_STATUS ret;
	struct sta_uapsd_trig_params cmd = {0};

	cmd.vdevid = vdevid;
	cmd.auto_triggerparam = autoTriggerparam;
	cmd.num_ac = num_ac;

	qdf_mem_copy((uint8_t *) cmd.peer_addr, (uint8_t *) peer_addr,
		     sizeof(peer_addr));
	ret = wmi_unified_set_sta_uapsd_auto_trig_cmd(wmi_handle,
				   &cmd);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set uapsd param ret = %d", ret);

	return ret;
}

/**
 * wma_trigger_uapsd_params() - set trigger uapsd parameter
 * @wmi_handle: wma handle
 * @vdev_id: vdev id
 * @trigger_uapsd_params: trigger uapsd parameters
 *
 * This function sets the trigger uapsd
 * params such as service interval, delay
 * interval and suspend interval which
 * will be used by the firmware to send
 * trigger frames periodically when there
 * is no traffic on the transmit side.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_trigger_uapsd_params(tp_wma_handle wma_handle, uint32_t vdev_id,
				    tp_wma_trigger_uapsd_params
				    trigger_uapsd_params)
{
	QDF_STATUS ret;
	wmi_sta_uapsd_auto_trig_param uapsd_trigger_param;

	WMA_LOGD("Trigger uapsd params vdev id %d", vdev_id);

	WMA_LOGD("WMM AC %d User Priority %d SvcIntv %d DelIntv %d SusIntv %d",
		 trigger_uapsd_params->wmm_ac,
		 trigger_uapsd_params->user_priority,
		 trigger_uapsd_params->service_interval,
		 trigger_uapsd_params->delay_interval,
		 trigger_uapsd_params->suspend_interval);

	if (!WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				    WMI_STA_UAPSD_BASIC_AUTO_TRIG) ||
	    !WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				    WMI_STA_UAPSD_VAR_AUTO_TRIG)) {
		WMA_LOGD("Trigger uapsd is not supported vdev id %d", vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	uapsd_trigger_param.wmm_ac = trigger_uapsd_params->wmm_ac;
	uapsd_trigger_param.user_priority = trigger_uapsd_params->user_priority;
	uapsd_trigger_param.service_interval =
		trigger_uapsd_params->service_interval;
	uapsd_trigger_param.suspend_interval =
		trigger_uapsd_params->suspend_interval;
	uapsd_trigger_param.delay_interval =
		trigger_uapsd_params->delay_interval;

	ret = wma_set_sta_uapsd_auto_trig_cmd(wma_handle->wmi_handle,
			vdev_id, wma_handle->interfaces[vdev_id].bssid,
			(uint8_t *) (&uapsd_trigger_param), 1);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Fail to send uapsd param cmd for vdevid %d ret = %d",
			 ret, vdev_id);
		return ret;
	}

	return ret;
}

/**
 * wma_disable_uapsd_per_ac() - disable uapsd per ac
 * @wmi_handle: wma handle
 * @vdev_id: vdev id
 * @ac: access category
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_disable_uapsd_per_ac(tp_wma_handle wma_handle,
				    uint32_t vdev_id, enum uapsd_ac ac)
{
	QDF_STATUS ret;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	wmi_sta_uapsd_auto_trig_param uapsd_trigger_param;
	enum uapsd_up user_priority;

	WMA_LOGD("Disable Uapsd per ac vdevId %d ac %d", vdev_id, ac);

	switch (ac) {
	case UAPSD_VO:
		iface->uapsd_cached_val &=
			~(WMI_STA_PS_UAPSD_AC3_DELIVERY_EN |
			  WMI_STA_PS_UAPSD_AC3_TRIGGER_EN);
		user_priority = UAPSD_UP_VO;
		break;
	case UAPSD_VI:
		iface->uapsd_cached_val &=
			~(WMI_STA_PS_UAPSD_AC2_DELIVERY_EN |
			  WMI_STA_PS_UAPSD_AC2_TRIGGER_EN);
		user_priority = UAPSD_UP_VI;
		break;
	case UAPSD_BK:
		iface->uapsd_cached_val &=
			~(WMI_STA_PS_UAPSD_AC1_DELIVERY_EN |
			  WMI_STA_PS_UAPSD_AC1_TRIGGER_EN);
		user_priority = UAPSD_UP_BK;
		break;
	case UAPSD_BE:
		iface->uapsd_cached_val &=
			~(WMI_STA_PS_UAPSD_AC0_DELIVERY_EN |
			  WMI_STA_PS_UAPSD_AC0_TRIGGER_EN);
		user_priority = UAPSD_UP_BE;
		break;
	default:
		WMA_LOGE("Invalid AC vdevId %d ac %d", vdev_id, ac);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Disable Auto Trigger Functionality before
	 * disabling uapsd for a particular AC
	 */
	uapsd_trigger_param.wmm_ac = ac;
	uapsd_trigger_param.user_priority = user_priority;
	uapsd_trigger_param.service_interval = 0;
	uapsd_trigger_param.suspend_interval = 0;
	uapsd_trigger_param.delay_interval = 0;

	ret = wma_set_sta_uapsd_auto_trig_cmd(wma_handle->wmi_handle,
		vdev_id, wma_handle->interfaces[vdev_id].bssid,
		(uint8_t *)(&uapsd_trigger_param), 1);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Fail to send auto trig cmd for vdevid %d ret = %d",
			 ret, vdev_id);
		return ret;
	}

	ret = wma_unified_set_sta_ps_param(wma_handle->wmi_handle, vdev_id,
					   WMI_STA_PS_PARAM_UAPSD,
					   iface->uapsd_cached_val);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("Disable Uapsd per ac Failed vdevId %d ac %d", vdev_id,
			 ac);
		return ret;
	}
	WMA_LOGD("Disable Uapsd per ac vdevId %d val %d", vdev_id,
		 iface->uapsd_cached_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_temperature() - get pdev temperature req
 * @wmi_handle: wma handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_get_temperature(tp_wma_handle wma_handle)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	ret = wmi_unified_get_temperature(wma_handle->wmi_handle);
	if (ret)
		WMA_LOGE("Failed to send set Mimo PS ret = %d", ret);

	return ret;
}

/**
 * wma_pdev_temperature_evt_handler() - pdev temperature event handler
 * @handle: wma handle
 * @event: event buffer
 * @len : length
 *
 * Return: 0 for success or error code.
 */
int wma_pdev_temperature_evt_handler(void *handle, uint8_t *event,
				     uint32_t len)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	cds_msg_t sme_msg = { 0 };
	WMI_PDEV_TEMPERATURE_EVENTID_param_tlvs *param_buf;
	wmi_pdev_temperature_event_fixed_param *wmi_event;

	param_buf = (WMI_PDEV_TEMPERATURE_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid pdev_temperature event buffer");
		return -EINVAL;
	}

	wmi_event = param_buf->fixed_param;
	WMA_LOGI(FL("temperature: %d"), wmi_event->value);

	sme_msg.type = eWNI_SME_MSG_GET_TEMPERATURE_IND;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = wmi_event->value;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE(FL("Fail to post get temperature ind msg"));
	}

	return 0;
}

/**
 * wma_process_tx_power_limits() - sends the power limits for 2g/5g to firmware
 * @handle: wma handle
 * @ptxlim: power limit value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_process_tx_power_limits(WMA_HANDLE handle,
				       tSirTxPowerLimit *ptxlim)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	int32_t ret = 0;
	uint32_t txpower_params2g = 0;
	uint32_t txpower_params5g = 0;
	struct pdev_params pdevparam;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue tx power limit",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}
	/* Set value and reason code for 2g and 5g power limit */

	SET_PDEV_PARAM_TXPOWER_REASON(txpower_params2g,
				      WMI_PDEV_PARAM_TXPOWER_REASON_SAR);
	SET_PDEV_PARAM_TXPOWER_VALUE(txpower_params2g, ptxlim->txPower2g);

	SET_PDEV_PARAM_TXPOWER_REASON(txpower_params5g,
				      WMI_PDEV_PARAM_TXPOWER_REASON_SAR);
	SET_PDEV_PARAM_TXPOWER_VALUE(txpower_params5g, ptxlim->txPower5g);

	WMA_LOGD("%s: txpower2g: %x txpower5g: %x",
		 __func__, txpower_params2g, txpower_params5g);

	pdevparam.param_id = WMI_PDEV_PARAM_TXPOWER_LIMIT2G;
	pdevparam.param_value = txpower_params2g;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &pdevparam,
					 WMA_WILDCARD_PDEV_ID);
	if (ret) {
		WMA_LOGE("%s: Failed to set txpower 2g (%d)", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}
	pdevparam.param_id = WMI_PDEV_PARAM_TXPOWER_LIMIT5G;
	pdevparam.param_value = txpower_params5g;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &pdevparam,
					 WMA_WILDCARD_PDEV_ID);
	if (ret) {
		WMA_LOGE("%s: Failed to set txpower 5g (%d)", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_add_p2p_ie() - add p2p IE
 * @frm: ptr where p2p ie needs to add
 *
 * Return: ptr after p2p ie
 */
uint8_t *wma_add_p2p_ie(uint8_t *frm)
{
	uint8_t wfa_oui[3] = WMA_P2P_WFA_OUI;
	struct p2p_ie *p2p_ie = (struct p2p_ie *)frm;

	p2p_ie->p2p_id = WMA_P2P_IE_ID;
	p2p_ie->p2p_oui[0] = wfa_oui[0];
	p2p_ie->p2p_oui[1] = wfa_oui[1];
	p2p_ie->p2p_oui[2] = wfa_oui[2];
	p2p_ie->p2p_oui_type = WMA_P2P_WFA_VER;
	p2p_ie->p2p_len = 4;
	return frm + sizeof(struct p2p_ie);
}

/**
 * wma_update_beacon_noa_ie() - update beacon ie
 * @bcn: beacon info
 * @new_noa_sub_ie_len: ie length
 *
 * Return: none
 */
static void wma_update_beacon_noa_ie(struct beacon_info *bcn,
				     uint16_t new_noa_sub_ie_len)
{
	struct p2p_ie *p2p_ie;
	uint8_t *buf;

	/* if there is nothing to add, just return */
	if (new_noa_sub_ie_len == 0) {
		if (bcn->noa_sub_ie_len && bcn->noa_ie) {
			WMA_LOGD("%s: NoA is present in previous beacon, "
				 "but not present in swba event, "
				 "So Reset the NoA", __func__);
			/* TODO: Assuming p2p noa ie is last ie in the beacon */
			qdf_mem_zero(bcn->noa_ie, (bcn->noa_sub_ie_len +
						   sizeof(struct p2p_ie)));
			bcn->len -= (bcn->noa_sub_ie_len +
				     sizeof(struct p2p_ie));
			bcn->noa_ie = NULL;
			bcn->noa_sub_ie_len = 0;
		}
		WMA_LOGD("%s: No need to update NoA", __func__);
		return;
	}

	if (bcn->noa_sub_ie_len && bcn->noa_ie) {
		/* NoA present in previous beacon, update it */
		WMA_LOGD("%s: NoA present in previous beacon, "
			 "update the NoA IE, bcn->len %u"
			 "bcn->noa_sub_ie_len %u",
			 __func__, bcn->len, bcn->noa_sub_ie_len);
		bcn->len -= (bcn->noa_sub_ie_len + sizeof(struct p2p_ie));
		qdf_mem_zero(bcn->noa_ie,
			     (bcn->noa_sub_ie_len + sizeof(struct p2p_ie)));
	} else {                /* NoA is not present in previous beacon */
		WMA_LOGD("%s: NoA not present in previous beacon, add it"
			 "bcn->len %u", __func__, bcn->len);
		buf = qdf_nbuf_data(bcn->buf);
		bcn->noa_ie = buf + bcn->len;
	}

	bcn->noa_sub_ie_len = new_noa_sub_ie_len;
	wma_add_p2p_ie(bcn->noa_ie);
	p2p_ie = (struct p2p_ie *)bcn->noa_ie;
	p2p_ie->p2p_len += new_noa_sub_ie_len;
	qdf_mem_copy((bcn->noa_ie + sizeof(struct p2p_ie)), bcn->noa_sub_ie,
		     new_noa_sub_ie_len);

	bcn->len += (new_noa_sub_ie_len + sizeof(struct p2p_ie));
	WMA_LOGI("%s: Updated beacon length with NoA Ie is %u",
		 __func__, bcn->len);
}

/**
 * wma_p2p_create_sub_ie_noa() - put p2p noa ie
 * @buf: buffer
 * @noa: noa element ie
 * @new_noa_sub_ie_len: ie length
 *
 * Return: none
 */
static void wma_p2p_create_sub_ie_noa(uint8_t *buf,
				      struct p2p_sub_element_noa *noa,
				      uint16_t *new_noa_sub_ie_len)
{
	uint8_t tmp_octet = 0;
	int i;
	uint8_t *buf_start = buf;

	*buf++ = WMA_P2P_SUB_ELEMENT_NOA;       /* sub-element id */
	ASSERT(noa->num_descriptors <= WMA_MAX_NOA_DESCRIPTORS);

	/*
	 * Length = (2 octets for Index and CTWin/Opp PS) and
	 * (13 octets for each NOA Descriptors)
	 */
	P2PIE_PUT_LE16(buf, WMA_NOA_IE_SIZE(noa->num_descriptors));
	buf += 2;

	*buf++ = noa->index;    /* Instance Index */

	tmp_octet = noa->ctwindow & WMA_P2P_NOA_IE_CTWIN_MASK;
	if (noa->oppPS) {
		tmp_octet |= WMA_P2P_NOA_IE_OPP_PS_SET;
	}
	*buf++ = tmp_octet;     /* Opp Ps and CTWin capabilities */

	for (i = 0; i < noa->num_descriptors; i++) {
		ASSERT(noa->noa_descriptors[i].type_count != 0);

		*buf++ = noa->noa_descriptors[i].type_count;

		P2PIE_PUT_LE32(buf, noa->noa_descriptors[i].duration);
		buf += 4;
		P2PIE_PUT_LE32(buf, noa->noa_descriptors[i].interval);
		buf += 4;
		P2PIE_PUT_LE32(buf, noa->noa_descriptors[i].start_time);
		buf += 4;
	}
	*new_noa_sub_ie_len = (buf - buf_start);
}

/**
 * wma_update_noa() - update noa params
 * @beacon: beacon info
 * @noa_ie: noa ie
 *
 * Return: none
 */
void wma_update_noa(struct beacon_info *beacon,
		    struct p2p_sub_element_noa *noa_ie)
{
	uint16_t new_noa_sub_ie_len;

	/* Call this function by holding the spinlock on beacon->lock */

	if (noa_ie) {
		if ((noa_ie->ctwindow == 0) && (noa_ie->oppPS == 0) &&
		    (noa_ie->num_descriptors == 0)) {
			/* NoA is not present */
			WMA_LOGD("%s: NoA is not present", __func__);
			new_noa_sub_ie_len = 0;
		} else {
			/* Create the binary blob containing NOA sub-IE */
			WMA_LOGD("%s: Create NOA sub ie", __func__);
			wma_p2p_create_sub_ie_noa(&beacon->noa_sub_ie[0],
						  noa_ie, &new_noa_sub_ie_len);
		}
	} else {
		WMA_LOGD("%s: No need to add NOA", __func__);
		new_noa_sub_ie_len = 0; /* no NOA IE sub-attributes */
	}

	wma_update_beacon_noa_ie(beacon, new_noa_sub_ie_len);
}

/**
 * wma_update_probe_resp_noa() - update noa IE in probe response
 * @wma_handle: wma handle
 * @noa_ie: noa ie
 *
 * Return: none
 */
void wma_update_probe_resp_noa(tp_wma_handle wma_handle,
			       struct p2p_sub_element_noa *noa_ie)
{
	tSirP2PNoaAttr *noa_attr =
		(tSirP2PNoaAttr *) qdf_mem_malloc(sizeof(tSirP2PNoaAttr));
	WMA_LOGD("Received update NoA event");
	if (!noa_attr) {
		WMA_LOGE("Failed to allocate memory for tSirP2PNoaAttr");
		return;
	}

	qdf_mem_zero(noa_attr, sizeof(tSirP2PNoaAttr));

	noa_attr->index = noa_ie->index;
	noa_attr->oppPsFlag = noa_ie->oppPS;
	noa_attr->ctWin = noa_ie->ctwindow;
	if (!noa_ie->num_descriptors) {
		WMA_LOGD("Zero NoA descriptors");
	} else {
		WMA_LOGD("%d NoA descriptors", noa_ie->num_descriptors);
		noa_attr->uNoa1IntervalCnt =
			noa_ie->noa_descriptors[0].type_count;
		noa_attr->uNoa1Duration = noa_ie->noa_descriptors[0].duration;
		noa_attr->uNoa1Interval = noa_ie->noa_descriptors[0].interval;
		noa_attr->uNoa1StartTime =
			noa_ie->noa_descriptors[0].start_time;
		if (noa_ie->num_descriptors > 1) {
			noa_attr->uNoa2IntervalCnt =
				noa_ie->noa_descriptors[1].type_count;
			noa_attr->uNoa2Duration =
				noa_ie->noa_descriptors[1].duration;
			noa_attr->uNoa2Interval =
				noa_ie->noa_descriptors[1].interval;
			noa_attr->uNoa2StartTime =
				noa_ie->noa_descriptors[1].start_time;
		}
	}
	WMA_LOGI("Sending SIR_HAL_P2P_NOA_ATTR_IND to LIM");
	wma_send_msg(wma_handle, SIR_HAL_P2P_NOA_ATTR_IND, (void *)noa_attr, 0);
}

/**
 * wma_p2p_noa_event_handler() - p2p noa event handler
 * @handle: wma handle
 * @event: event data
 * @len: length
 *
 * Return: 0 for success or error code.
 */
int wma_p2p_noa_event_handler(void *handle, uint8_t *event,
			      uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_P2P_NOA_EVENTID_param_tlvs *param_buf;
	wmi_p2p_noa_event_fixed_param *p2p_noa_event;
	uint8_t vdev_id, i;
	wmi_p2p_noa_info *p2p_noa_info;
	struct p2p_sub_element_noa noa_ie;
	uint8_t *buf_ptr;
	uint32_t descriptors;

	param_buf = (WMI_P2P_NOA_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid P2P NoA event buffer");
		return -EINVAL;
	}

	p2p_noa_event = param_buf->fixed_param;
	buf_ptr = (uint8_t *) p2p_noa_event;
	buf_ptr += sizeof(wmi_p2p_noa_event_fixed_param);
	p2p_noa_info = (wmi_p2p_noa_info *) (buf_ptr);
	vdev_id = p2p_noa_event->vdev_id;

	if (WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(p2p_noa_info)) {

		qdf_mem_zero(&noa_ie, sizeof(noa_ie));
		noa_ie.index =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_INDEX_GET(p2p_noa_info);
		noa_ie.oppPS =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(p2p_noa_info);
		noa_ie.ctwindow =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_CTWIN_GET(p2p_noa_info);
		descriptors = WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info);
		noa_ie.num_descriptors = (uint8_t) descriptors;

		WMA_LOGI("%s: index %u, oppPs %u, ctwindow %u, "
			 "num_descriptors = %u", __func__, noa_ie.index,
			 noa_ie.oppPS, noa_ie.ctwindow, noa_ie.num_descriptors);
		for (i = 0; i < noa_ie.num_descriptors; i++) {
			noa_ie.noa_descriptors[i].type_count =
				(uint8_t) p2p_noa_info->noa_descriptors[i].
				type_count;
			noa_ie.noa_descriptors[i].duration =
				p2p_noa_info->noa_descriptors[i].duration;
			noa_ie.noa_descriptors[i].interval =
				p2p_noa_info->noa_descriptors[i].interval;
			noa_ie.noa_descriptors[i].start_time =
				p2p_noa_info->noa_descriptors[i].start_time;
			WMA_LOGI("%s: NoA descriptor[%d] type_count %u, "
				 "duration %u, interval %u, start_time = %u",
				 __func__, i,
				 noa_ie.noa_descriptors[i].type_count,
				 noa_ie.noa_descriptors[i].duration,
				 noa_ie.noa_descriptors[i].interval,
				 noa_ie.noa_descriptors[i].start_time);
		}

		/* Send a msg to LIM to update the NoA IE in probe response
		 * frames transmitted by the host */
		wma_update_probe_resp_noa(wma, &noa_ie);
	}

	return 0;
}

/**
 * wma_set_p2pgo_noa_req() - send p2p go noa request to fw
 * @wma: wma handle
 * @noa: p2p power save parameters
 *
 * Return: none
 */
static void wma_set_p2pgo_noa_req(tp_wma_handle wma, tP2pPsParams *noa)
{
	QDF_STATUS ret;
	struct p2p_ps_params cmd = {0};

	cmd.opp_ps = noa->opp_ps;
	cmd.ctwindow = noa->ctWindow;
	cmd.count = noa->count;
	cmd.duration = noa->duration;
	cmd.interval = noa->interval;
	cmd.single_noa_duration = noa->single_noa_duration;
	cmd.ps_selection = noa->psSelection;
	cmd.session_id =  noa->sessionId;

	ret = wmi_unified_set_p2pgo_noa_req_cmd(wma->wmi_handle,
				   &cmd);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set uapsd param ret = %d", ret);

	return;
}

/**
 * wma_set_p2pgo_oppps_req() - send p2p go opp power save request to fw
 * @wma: wma handle
 * @noa: p2p opp power save parameters
 *
 * Return: none
 */
static void wma_set_p2pgo_oppps_req(tp_wma_handle wma, tP2pPsParams *oppps)
{
	QDF_STATUS ret;
	struct p2p_ps_params cmd = {0};

	cmd.opp_ps = oppps->opp_ps;
	cmd.ctwindow = oppps->ctWindow;
	cmd.count = oppps->count;
	cmd.duration = oppps->duration;
	cmd.interval = oppps->interval;
	cmd.single_noa_duration = oppps->single_noa_duration;
	cmd.ps_selection = oppps->psSelection;
	cmd.session_id =  oppps->sessionId;

	ret = wmi_unified_set_p2pgo_oppps_req(wma->wmi_handle,
				   &cmd);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set uapsd param ret = %d", ret);

	return;
}

/**
 * wma_process_set_p2pgo_noa_req() - process p2pgo noa request
 * @handle: wma handle
 * @ps_params: powersave params
 *
 * Return: none
 */
void wma_process_set_p2pgo_noa_req(tp_wma_handle wma,
				   tP2pPsParams *ps_params)
{
	WMA_LOGD("%s: Enter", __func__);
	if (ps_params->opp_ps) {
		wma_set_p2pgo_oppps_req(wma, ps_params);
	} else {
		wma_set_p2pgo_noa_req(wma, ps_params);
	}

	WMA_LOGD("%s: Exit", __func__);
}

/**
 * wma_process_set_mimops_req() - Set the received MiMo PS state to firmware
 * @handle: wma handle
 * @mimops: MIMO powersave params
 *
 * Return: none
 */
void wma_process_set_mimops_req(tp_wma_handle wma_handle,
				tSetMIMOPS *mimops)
{
	/* Translate to what firmware understands */
	if (mimops->htMIMOPSState == eSIR_HT_MIMO_PS_DYNAMIC)
		mimops->htMIMOPSState = WMI_PEER_MIMO_PS_DYNAMIC;
	else if (mimops->htMIMOPSState == eSIR_HT_MIMO_PS_STATIC)
		mimops->htMIMOPSState = WMI_PEER_MIMO_PS_STATIC;
	else if (mimops->htMIMOPSState == eSIR_HT_MIMO_PS_NO_LIMIT)
		mimops->htMIMOPSState = WMI_PEER_MIMO_PS_NONE;

	WMA_LOGD("%s: htMIMOPSState = %d, sessionId = %d \
		 peerMac <%02x:%02x:%02x:%02x:%02x:%02x>", __func__,
		 mimops->htMIMOPSState, mimops->sessionId, mimops->peerMac[0],
		 mimops->peerMac[1], mimops->peerMac[2], mimops->peerMac[3],
		 mimops->peerMac[4], mimops->peerMac[5]);

	wma_set_peer_param(wma_handle, mimops->peerMac,
			   WMI_PEER_MIMO_PS_STATE, mimops->htMIMOPSState,
			   mimops->sessionId);
}

/**
 * wma_set_mimops() - set MIMO powersave
 * @handle: wma handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_set_mimops(tp_wma_handle wma, uint8_t vdev_id, int value)
{
	QDF_STATUS ret;

	ret = wmi_unified_set_mimops(wma->wmi_handle, vdev_id,
				   value);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set Mimo PS ret = %d", ret);

	return ret;
}

/**
 * wma_notify_modem_power_state() - notify modem power state
 * @wma_ptr: wma handle
 * @pReq: modem power state
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_notify_modem_power_state(void *wma_ptr,
					tSirModemPowerStateInd *pReq)
{
	int32_t ret;
	tp_wma_handle wma = (tp_wma_handle) wma_ptr;

	WMA_LOGD("%s: WMA notify Modem Power State %d", __func__, pReq->param);

	ret = wma_unified_modem_power_state(wma->wmi_handle, pReq->param);
	if (ret) {
		WMA_LOGE("%s: Fail to notify Modem Power State %d",
			 __func__, pReq->param);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Successfully notify Modem Power State %d", pReq->param);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_idle_ps_config() - enable/disble Low Power Support(Pdev Specific)
 * @wma_ptr: wma handle
 * @idle_ps: idle powersave
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_set_idle_ps_config(void *wma_ptr, uint32_t idle_ps)
{
	int32_t ret;
	tp_wma_handle wma = (tp_wma_handle) wma_ptr;
	struct pdev_params pdevparam;

	WMA_LOGD("WMA Set Idle Ps Config [1:set 0:clear] val %d", idle_ps);

	/* Set Idle Mode Power Save Config */
	pdevparam.param_id = WMI_PDEV_PARAM_IDLE_PS_CONFIG;
	pdevparam.param_value = idle_ps;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &pdevparam,
					 WMA_WILDCARD_PDEV_ID);

	if (ret) {
		WMA_LOGE("Fail to Set Idle Ps Config %d", idle_ps);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Successfully Set Idle Ps Config %d", idle_ps);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_smps_params() - set smps params
 * @wma: wma handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_set_smps_params(tp_wma_handle wma, uint8_t vdev_id,
			       int value)
{
	QDF_STATUS ret;

	ret = wmi_unified_set_smps_params(wma->wmi_handle, vdev_id,
				   value);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send set Mimo PS ret = %d", ret);

	return ret;
}

/**
 * wma_set_vdev_suspend_dtim() - set suspend dtim parameters in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: none
 */
static void wma_set_vdev_suspend_dtim(tp_wma_handle wma, uint8_t vdev_id)
{
	struct wma_txrx_node *iface = &wma->interfaces[vdev_id];
	enum powersave_qpower_mode qpower_config = wma_get_qpower_config(wma);

	if ((iface->type == WMI_VDEV_TYPE_STA) &&
	    (iface->ps_enabled == true) &&
	    (iface->dtimPeriod != 0)) {
		QDF_STATUS ret;
		uint32_t listen_interval;
		uint32_t max_mod_dtim;

		if (wma->staDynamicDtim) {
			listen_interval = wma->staDynamicDtim;
		} else if ((wma->staModDtim) &&
			   (wma->staMaxLIModDtim)) {
			/*
			 * When the system is in suspend
			 * (maximum beacon will be at 1s == 10)
			 * If maxModulatedDTIM ((MAX_LI_VAL = 10) / AP_DTIM)
			 * equal or larger than MDTIM
			 * (configured in WCNSS_qcom_cfg.ini)
			 * Set LI to MDTIM * AP_DTIM
			 * If Dtim = 2 and Mdtim = 2 then LI is 4
			 * Else
			 * Set LI to maxModulatedDTIM * AP_DTIM
			 */
			max_mod_dtim = wma->staMaxLIModDtim / iface->dtimPeriod;
			if (max_mod_dtim >= wma->staModDtim) {
				listen_interval =
					(wma->staModDtim * iface->dtimPeriod);
			} else {
				listen_interval =
					(max_mod_dtim * iface->dtimPeriod);
			}
		} else {
			return;
		}

		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_LISTEN_INTERVAL,
						      listen_interval);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* Even it fails continue Fw will take default LI */
			WMA_LOGE("Failed to Set Listen Interval vdevId %d",
				 vdev_id);
		}

		WMA_LOGD("Set Listen Interval vdevId %d Listen Intv %d",
			 vdev_id, listen_interval);

		if (qpower_config) {
			WMA_LOGD("disable Qpower in suspend mode!");
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
						vdev_id,
						WMI_STA_PS_ENABLE_QPOWER,
						0);
			if (QDF_IS_STATUS_ERROR(ret))
				WMA_LOGE("Failed to disable Qpower in suspend mode!");

			iface->ps_enabled = true;
		}

		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_DTIM_POLICY,
						      NORMAL_DTIM);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to Set to Normal DTIM vdevId %d",
				 vdev_id);

		/* Set it to Normal DTIM */
		iface->dtim_policy = NORMAL_DTIM;
		WMA_LOGD("Set DTIM Policy to Normal Dtim vdevId %d", vdev_id);
	}
}

/**
 * wma_set_suspend_dtim() - set suspend dtim
 * @wma: wma handle
 *
 * Return: none
 */
void wma_set_suspend_dtim(tp_wma_handle wma)
{
	uint8_t i;

	if (NULL == wma) {
		WMA_LOGE("%s: wma is NULL", __func__);
		return;
	}

	for (i = 0; i < wma->max_bssid; i++) {
	  if ((wma->interfaces[i].handle) &&
	      (false == wma->interfaces[i].alt_modulated_dtim_enabled)) {
			wma_set_vdev_suspend_dtim(wma, i);
		}
	}
}

/**
 * wma_set_vdev_resume_dtim() - set resume dtim parameters in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: none
 */
static void wma_set_vdev_resume_dtim(tp_wma_handle wma, uint8_t vdev_id)
{
	struct wma_txrx_node *iface = &wma->interfaces[vdev_id];
	enum powersave_qpower_mode qpower_config = wma_get_qpower_config(wma);

	if ((iface->type == WMI_VDEV_TYPE_STA) &&
	    (iface->ps_enabled == true) &&
	    (iface->dtim_policy == NORMAL_DTIM)) {
		QDF_STATUS ret;
		uint32_t cfg_data_val = 0;
		/* get mac to acess CFG data base */
		struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);
		if (!mac) {
			WMA_LOGE(FL("Failed to get mac context"));
			return;
		}
		/* Set Listen Interval */
		if ((wlan_cfg_get_int(mac, WNI_CFG_LISTEN_INTERVAL,
				      &cfg_data_val) != eSIR_SUCCESS)) {
			QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
				  "Failed to get value for listen interval");
			cfg_data_val = POWERSAVE_DEFAULT_LISTEN_INTERVAL;
		}

		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_LISTEN_INTERVAL,
						      cfg_data_val);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* Even it fails continue Fw will take default LI */
			WMA_LOGE("Failed to Set Listen Interval vdevId %d",
				 vdev_id);
		}

		WMA_LOGD("Set Listen Interval vdevId %d Listen Intv %d",
			 vdev_id, cfg_data_val);

		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_DTIM_POLICY,
						      STICK_DTIM);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* Set it back to Stick DTIM */
			WMA_LOGE("Failed to Set to Stick DTIM vdevId %d",
				 vdev_id);
		}
		iface->dtim_policy = STICK_DTIM;
		WMA_LOGD("Set DTIM Policy to Stick Dtim vdevId %d", vdev_id);

		if (qpower_config) {
			WMA_LOGD("enable Qpower in resume mode!");
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
						vdev_id,
						WMI_STA_PS_ENABLE_QPOWER,
						1);
			if (QDF_IS_STATUS_ERROR(ret))
				WMA_LOGE("Failed to enable Qpower in resume mode!");
		}
	}
}

/**
 * wma_set_resume_dtim() - set resume dtim
 * @wma: wma handle
 *
 * Return: none
 */
void wma_set_resume_dtim(tp_wma_handle wma)
{
	uint8_t i;

	if (NULL == wma) {
		WMA_LOGE("%s: wma is NULL", __func__);
		return;
	}

	for (i = 0; i < wma->max_bssid; i++) {
	  if ((wma->interfaces[i].handle) &&
	      (false == wma->interfaces[i].alt_modulated_dtim_enabled)) {
			wma_set_vdev_resume_dtim(wma, i);
		}
	}
}

/**
 * wma_set_tx_power_scale() - set tx power scale
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_set_tx_power_scale(uint8_t vdev_id, int value)
{
	QDF_STATUS ret;
	tp_wma_handle wma_handle =
			(tp_wma_handle)cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: wma_handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!(wma_handle->interfaces[vdev_id].vdev_up)) {
		WMA_LOGE("%s: vdev id %d is not up", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma_handle->wmi_handle, vdev_id,
				WMI_VDEV_PARAM_TXPOWER_SCALE, value);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Set tx power scale failed");

	return ret;
}

/**
 * wma_set_tx_power_scale_decr_db() - decrease power by DB value
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS wma_set_tx_power_scale_decr_db(uint8_t vdev_id, int value)
{
	QDF_STATUS ret;
	tp_wma_handle wma_handle =
			(tp_wma_handle)cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: wma_handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!(wma_handle->interfaces[vdev_id].vdev_up)) {
		WMA_LOGE("%s: vdev id %d is not up", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma_handle->wmi_handle, vdev_id,
				WMI_VDEV_PARAM_TXPOWER_SCALE_DECR_DB, value);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Decrease tx power value failed");

	return ret;
}
