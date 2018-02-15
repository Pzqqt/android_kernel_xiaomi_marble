/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
 *  DOC:    wma_features.c
 *  This file contains different features related functions like WoW,
 *  Offloads, TDLS etc.
 */

/* Header files */

#include "cds_ieee80211_common.h"	/* ieee80211_frame */
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
#include <cdp_txrx_tx_delay.h>
#include <cdp_txrx_peer_ops.h>

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"
#include "qdf_util.h"

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

#include "wma_internal.h"
#include "wma_nan_datapath.h"
#include <cdp_txrx_handle.h>
#include "wlan_pmo_ucfg_api.h"
#include <target_if_scan.h>
#include "wlan_reg_services_api.h"
#include "wlan_roam_debug.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)         (sizeof(a) / sizeof((a)[0]))
#endif

/**
 * WMA_SET_VDEV_IE_SOURCE_HOST - Flag to identify the source of VDEV SET IE
 * command. The value is 0x0 for the VDEV SET IE WMI commands from mobile
 * MCL platform.
 */
#define WMA_SET_VDEV_IE_SOURCE_HOST 0x0


#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * qdf_wma_wow_wakeup_stats_event()- send wow wakeup stats
 * @tp_wma_handle wma: WOW wakeup packet counter
 *
 * This function sends wow wakeup stats diag event
 *
 * Return: void.
 */
static inline void qdf_wma_wow_wakeup_stats_event(tp_wma_handle wma)
{
	QDF_STATUS status;
	struct sir_wake_lock_stats stats;

	WLAN_HOST_DIAG_EVENT_DEF(WowStats,
	struct host_event_wlan_powersave_wow_stats);

	status = wma_get_wakelock_stats(&stats);
	if (QDF_IS_STATUS_ERROR(status))
		return;
	qdf_mem_zero(&WowStats, sizeof(WowStats));

	WowStats.wow_bcast_wake_up_count =
		stats.wow_bcast_wake_up_count;
	WowStats.wow_ipv4_mcast_wake_up_count =
		stats.wow_ipv4_mcast_wake_up_count;
	WowStats.wow_ipv6_mcast_wake_up_count =
		stats.wow_ipv6_mcast_wake_up_count;
	WowStats.wow_ipv6_mcast_ra_stats =
		stats.wow_ipv6_mcast_ra_stats;
	WowStats.wow_ipv6_mcast_ns_stats =
		stats.wow_ipv6_mcast_ns_stats;
	WowStats.wow_ipv6_mcast_na_stats =
		stats.wow_ipv6_mcast_na_stats;
	WowStats.wow_pno_match_wake_up_count =
		stats.wow_pno_match_wake_up_count;
	WowStats.wow_pno_complete_wake_up_count =
		stats.wow_pno_complete_wake_up_count;
	WowStats.wow_gscan_wake_up_count =
		stats.wow_gscan_wake_up_count;
	WowStats.wow_low_rssi_wake_up_count =
		stats.wow_low_rssi_wake_up_count;
	WowStats.wow_rssi_breach_wake_up_count =
		stats.wow_rssi_breach_wake_up_count;
	WowStats.wow_icmpv4_count =
		stats.wow_icmpv4_count;
	WowStats.wow_icmpv6_count =
		stats.wow_icmpv6_count;
	WowStats.wow_oem_response_wake_up_count =
		stats.wow_oem_response_wake_up_count;

	WLAN_HOST_DIAG_EVENT_REPORT(&WowStats, EVENT_WLAN_POWERSAVE_WOW_STATS);
}
#else
static inline void qdf_wma_wow_wakeup_stats_event(tp_wma_handle wma)
{
	return;
}
#endif

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/**
 * wma_post_auto_shutdown_msg() - to post auto shutdown event to sme
 *
 * Return: 0 for success or error code
 */
static int wma_wake_reason_auto_shutdown(void)
{
	tSirAutoShutdownEvtParams *auto_sh_evt;
	QDF_STATUS qdf_status;
	struct scheduler_msg sme_msg = { 0 };

	auto_sh_evt = (tSirAutoShutdownEvtParams *)
		      qdf_mem_malloc(sizeof(tSirAutoShutdownEvtParams));
	if (!auto_sh_evt) {
		WMA_LOGE(FL("No Mem"));
		return -ENOMEM;
	}

	auto_sh_evt->shutdown_reason =
		WMI_HOST_AUTO_SHUTDOWN_REASON_TIMER_EXPIRY;
	sme_msg.type = eWNI_SME_AUTO_SHUTDOWN_IND;
	sme_msg.bodyptr = auto_sh_evt;
	sme_msg.bodyval = 0;

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Fail to post eWNI_SME_AUTO_SHUTDOWN_IND msg to SME");
		qdf_mem_free(auto_sh_evt);
		return -EINVAL;
	}

	return 0;
}
#else
static inline int wma_wake_reason_auto_shutdown(void)
{
	return 0;
}
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */

#ifdef FEATURE_WLAN_SCAN_PNO
static int wma_wake_reason_nlod(t_wma_handle *wma, uint8_t vdev_id)
{
	wmi_nlo_event nlo_event = { .vdev_id = vdev_id };
	WMI_NLO_MATCH_EVENTID_param_tlvs param = { .fixed_param = &nlo_event };

	return target_if_nlo_match_event_handler(wma, (uint8_t *)&param,
						 sizeof(param));
}
#else
static inline int wma_wake_reason_nlod(uint8_t vdev_id)
{
	return 0;
}
#endif /* FEATURE_WLAN_SCAN_PNO */

/**
 * wma_send_snr_request() - send request to fw to get RSSI stats
 * @wma_handle: wma handle
 * @pGetRssiReq: get RSSI request
 *
 * Return: QDF status
 */
QDF_STATUS wma_send_snr_request(tp_wma_handle wma_handle,
				void *pGetRssiReq)
{
	tAniGetRssiReq *pRssiBkUp = NULL;

	/* command is in progess */
	if (NULL != wma_handle->pGetRssiReq)
		return QDF_STATUS_SUCCESS;

	/* create a copy of csrRssiCallback to send rssi value
	 * after wmi event
	 */
	if (pGetRssiReq) {
		pRssiBkUp = qdf_mem_malloc(sizeof(tAniGetRssiReq));
		if (!pRssiBkUp) {
			WMA_LOGE("Failed to alloc memory for tAniGetRssiReq");
			wma_handle->pGetRssiReq = NULL;
			return QDF_STATUS_E_NOMEM;
		}
		pRssiBkUp->sessionId =
			((tAniGetRssiReq *) pGetRssiReq)->sessionId;
		pRssiBkUp->rssiCallback =
			((tAniGetRssiReq *) pGetRssiReq)->rssiCallback;
		pRssiBkUp->pDevContext =
			((tAniGetRssiReq *) pGetRssiReq)->pDevContext;
		wma_handle->pGetRssiReq = (void *)pRssiBkUp;
	}

	if (wmi_unified_snr_request_cmd(wma_handle->wmi_handle)) {
		WMA_LOGE("Failed to send host stats request to fw");
		qdf_mem_free(pRssiBkUp);
		wma_handle->pGetRssiReq = NULL;
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_snr() - get RSSI from fw
 * @psnr_req: request params
 *
 * Return: QDF status
 */
QDF_STATUS wma_get_snr(tAniGetSnrReq *psnr_req)
{
	tAniGetSnrReq *psnr_req_bkp;
	tp_wma_handle wma_handle = NULL;
	struct wma_txrx_node *intr;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s : Failed to get wma_handle", __func__);
		return QDF_STATUS_E_FAULT;
	}

	intr = &wma_handle->interfaces[psnr_req->sessionId];
	/* command is in progess */
	if (NULL != intr->psnr_req) {
		WMA_LOGE("%s : previous snr request is pending", __func__);
		return QDF_STATUS_SUCCESS;
	}

	psnr_req_bkp = qdf_mem_malloc(sizeof(tAniGetSnrReq));
	if (!psnr_req_bkp) {
		WMA_LOGE("Failed to allocate memory for tAniGetSnrReq");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_set(psnr_req_bkp, sizeof(tAniGetSnrReq), 0);
	psnr_req_bkp->staId = psnr_req->staId;
	psnr_req_bkp->pDevContext = psnr_req->pDevContext;
	psnr_req_bkp->snrCallback = psnr_req->snrCallback;
	intr->psnr_req = (void *)psnr_req_bkp;

	if (wmi_unified_snr_cmd(wma_handle->wmi_handle,
				 psnr_req->sessionId)) {
		WMA_LOGE("Failed to send host stats request to fw");
		qdf_mem_free(psnr_req_bkp);
		intr->psnr_req = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_link_status_req() - process link status request from UMAC
 * @wma: wma handle
 * @pGetLinkStatus: get link params
 *
 * Return: none
 */
void wma_process_link_status_req(tp_wma_handle wma,
				 tAniGetLinkStatus *pGetLinkStatus)
{
	struct link_status_params cmd = {0};
	struct wma_txrx_node *iface =
		&wma->interfaces[pGetLinkStatus->sessionId];

	if (iface->plink_status_req) {
		WMA_LOGE("%s:previous link status request is pending,deleting the new request",
			__func__);
		qdf_mem_free(pGetLinkStatus);
		return;
	}

	iface->plink_status_req = pGetLinkStatus;
	cmd.session_id = pGetLinkStatus->sessionId;
	if (wmi_unified_link_status_req_cmd(wma->wmi_handle, &cmd)) {
		WMA_LOGE("Failed to send WMI link  status request to fw");
		iface->plink_status_req = NULL;
		goto end;
	}

	return;

end:
	wma_post_link_status(pGetLinkStatus, LINK_STATUS_LEGACY);
}

#ifdef WLAN_FEATURE_TSF
/**
 * wma_vdev_tsf_handler() - handle tsf event indicated by FW
 * @handle: wma context
 * @data: event buffer
 * @data len: length of event buffer
 *
 * Return: 0 on success
 */
int wma_vdev_tsf_handler(void *handle, uint8_t *data, uint32_t data_len)
{
	struct scheduler_msg tsf_msg = {0};
	WMI_VDEV_TSF_REPORT_EVENTID_param_tlvs *param_buf;
	wmi_vdev_tsf_report_event_fixed_param *tsf_event;
	struct stsf *ptsf;

	if (data == NULL) {
		WMA_LOGE("%s: invalid pointer", __func__);
		return -EINVAL;
	}
	ptsf = qdf_mem_malloc(sizeof(*ptsf));
	if (NULL == ptsf) {
		WMA_LOGE("%s: failed to allocate tsf data structure", __func__);
		return -ENOMEM;
	}

	param_buf = (WMI_VDEV_TSF_REPORT_EVENTID_param_tlvs *)data;
	tsf_event = param_buf->fixed_param;

	ptsf->vdev_id = tsf_event->vdev_id;
	ptsf->tsf_low = tsf_event->tsf_low;
	ptsf->tsf_high = tsf_event->tsf_high;
	ptsf->soc_timer_low = tsf_event->qtimer_low;
	ptsf->soc_timer_high = tsf_event->qtimer_high;

	WMA_LOGD("%s: receive WMI_VDEV_TSF_REPORT_EVENTID ", __func__);
	WMA_LOGD("%s: vdev_id = %u,tsf_low =%u, tsf_high = %u", __func__,
	ptsf->vdev_id, ptsf->tsf_low, ptsf->tsf_high);

	tsf_msg.type = eWNI_SME_TSF_EVENT;
	tsf_msg.bodyptr = ptsf;
	tsf_msg.bodyval = 0;

	if (QDF_STATUS_SUCCESS !=
		scheduler_post_msg(QDF_MODULE_ID_SME, &tsf_msg)) {

		WMA_LOGP("%s: Failed to post eWNI_SME_TSF_EVENT", __func__);
		qdf_mem_free(ptsf);
		return -EINVAL;
	}
	return 0;
}

#ifdef QCA_WIFI_3_0
#define TSF_FW_ACTION_CMD TSF_TSTAMP_QTIMER_CAPTURE_REQ
#else
#define TSF_FW_ACTION_CMD TSF_TSTAMP_CAPTURE_REQ
#endif
/**
 * wma_capture_tsf() - send wmi to fw to capture tsf
 * @wma_handle: wma handler
 * @vdev_id: vdev id
 *
 * Return: wmi send state
 */
QDF_STATUS wma_capture_tsf(tp_wma_handle wma_handle, uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	wmi_vdev_tsf_tstamp_action_cmd_fixed_param *cmd;
	int ret;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: failed to allocate memory for cap tsf cmd",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_tsf_tstamp_action_cmd_fixed_param *) wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	cmd->tsf_action = TSF_FW_ACTION_CMD;
	WMA_LOGD("%s :vdev_id %u, tsf_cmd: %d", __func__, cmd->vdev_id,
						cmd->tsf_action);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_vdev_tsf_tstamp_action_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
	wmi_vdev_tsf_tstamp_action_cmd_fixed_param));

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				   WMI_VDEV_TSF_TSTAMP_ACTION_CMDID);
	if (ret != EOK) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d", status);
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	return QDF_STATUS_SUCCESS;

error:
	if (buf)
		wmi_buf_free(buf);
	return status;
}

/**
 * wma_reset_tsf_gpio() - send wmi to fw to reset GPIO
 * @wma_handle: wma handler
 * @vdev_id: vdev id
 *
 * Return: wmi send state
 */
QDF_STATUS wma_reset_tsf_gpio(tp_wma_handle wma_handle, uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	wmi_vdev_tsf_tstamp_action_cmd_fixed_param *cmd;
	int ret;
	int len = sizeof(*cmd);
	uint8_t *buf_ptr;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: failed to allocate memory for reset tsf gpio",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_tsf_tstamp_action_cmd_fixed_param *) buf_ptr;
	cmd->vdev_id = vdev_id;
	cmd->tsf_action = TSF_TSTAMP_CAPTURE_RESET;

	WMA_LOGD("%s :vdev_id %u, TSF_TSTAMP_CAPTURE_RESET", __func__,
		 cmd->vdev_id);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_tsf_tstamp_action_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_vdev_tsf_tstamp_action_cmd_fixed_param));

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				   WMI_VDEV_TSF_TSTAMP_ACTION_CMDID);

	if (ret != EOK) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d", status);
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	return QDF_STATUS_SUCCESS;

error:
	if (buf)
		wmi_buf_free(buf);
	return status;
}

/**
 * wma_set_tsf_gpio_pin() - send wmi cmd to configure gpio pin
 * @handle: wma handler
 * @pin: GPIO pin id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_set_tsf_gpio_pin(WMA_HANDLE handle, uint32_t pin)
{
	tp_wma_handle wma = (tp_wma_handle)handle;
	struct pdev_params pdev_param = {0};
	int32_t ret;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not set gpio", __func__);
		return QDF_STATUS_E_INVAL;
	}

	WMA_LOGD("%s: set tsf gpio pin: %d", __func__, pin);

	pdev_param.param_id = WMI_PDEV_PARAM_WNTS_CONFIG;
	pdev_param.param_value = pin;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &pdev_param,
					 WMA_WILDCARD_PDEV_ID);
	if (ret) {
		WMA_LOGE("%s: Failed to set tsf gpio pin (%d)", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * wma_set_wisa_params(): Set WISA features related params in FW
 * @wma_handle: WMA handle
 * @wisa: Pointer to WISA param struct
 *
 * Return: CDF status
 */
QDF_STATUS wma_set_wisa_params(tp_wma_handle wma_handle,
				struct sir_wisa_params *wisa)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	wmi_vdev_wisa_cmd_fixed_param *cmd;
	int ret, len = sizeof(*cmd);

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: failed to allocate memory for WISA params",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_wisa_cmd_fixed_param *) wmi_buf_data(buf);
	cmd->wisa_mode = wisa->mode;
	cmd->vdev_id = wisa->vdev_id;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_wisa_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_vdev_wisa_cmd_fixed_param));

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				   WMI_VDEV_WISA_CMDID);
	if (ret != EOK) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d", status);
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	return QDF_STATUS_SUCCESS;

error:
	wmi_buf_free(buf);
	return status;
}

/**
 * wma_process_dhcp_ind() - process dhcp indication from SME
 * @wma_handle: wma handle
 * @ta_dhcp_ind: DHCP indication
 *
 * Return: QDF Status
 */
QDF_STATUS wma_process_dhcp_ind(tp_wma_handle wma_handle,
				tAniDHCPInd *ta_dhcp_ind)
{
	uint8_t vdev_id;
	int status = 0;
	wmi_peer_set_param_cmd_fixed_param peer_set_param_fp = {0};

	if (!ta_dhcp_ind) {
		WMA_LOGE("%s : DHCP indication is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!wma_find_vdev_by_addr(wma_handle,
				   ta_dhcp_ind->adapterMacAddr.bytes,
				   &vdev_id)) {
		WMA_LOGE("%s: Failed to find vdev id for DHCP indication",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("%s: WMA --> WMI_PEER_SET_PARAM triggered by DHCP, msgType=%s, device_mode=%d, macAddr=" MAC_ADDRESS_STR,
		__func__, ta_dhcp_ind->msgType == WMA_DHCP_START_IND ?
		"WMA_DHCP_START_IND" : "WMA_DHCP_STOP_IND",
		ta_dhcp_ind->device_mode,
		MAC_ADDR_ARRAY(ta_dhcp_ind->peerMacAddr.bytes));

	/* fill in values */
	peer_set_param_fp.vdev_id = vdev_id;
	peer_set_param_fp.param_id = WMI_PEER_CRIT_PROTO_HINT_ENABLED;
	if (WMA_DHCP_START_IND == ta_dhcp_ind->msgType)
		peer_set_param_fp.param_value = 1;
	else
		peer_set_param_fp.param_value = 0;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ta_dhcp_ind->peerMacAddr.bytes,
				   &peer_set_param_fp.peer_macaddr);

	status = wmi_unified_process_dhcp_ind(wma_handle->wmi_handle,
						&peer_set_param_fp);
	if (status != EOK)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_chan_phy__mode() - get WLAN_PHY_MODE for channel
 * @chan: channel number
 * @chan_width: maximum channel width possible
 * @dot11_mode: maximum phy_mode possible
 *
 * Return: return WLAN_PHY_MODE
 */
WLAN_PHY_MODE wma_chan_phy_mode(u8 chan, enum phy_ch_width chan_width,
				u8 dot11_mode)
{
	WLAN_PHY_MODE phymode = MODE_UNKNOWN;
	uint16_t bw_val = wlan_reg_get_bw_value(chan_width);

	if (WLAN_REG_IS_24GHZ_CH(chan)) {
		if (((CH_WIDTH_5MHZ == chan_width) ||
		     (CH_WIDTH_10MHZ == chan_width)) &&
		    ((WNI_CFG_DOT11_MODE_11B == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11G == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11N == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_ALL == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AC == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AX == dot11_mode)))
			phymode = MODE_11G;
		else {
			switch (dot11_mode) {
			case WNI_CFG_DOT11_MODE_11B:
				if ((bw_val == 20) || (bw_val == 40))
					phymode = MODE_11B;
				break;
			case WNI_CFG_DOT11_MODE_11G:
				if ((bw_val == 20) || (bw_val == 40))
					phymode = MODE_11G;
				break;
			case WNI_CFG_DOT11_MODE_11G_ONLY:
				if ((bw_val == 20) || (bw_val == 40))
					phymode = MODE_11GONLY;
				break;
			case WNI_CFG_DOT11_MODE_11N:
			case WNI_CFG_DOT11_MODE_11N_ONLY:
				if (bw_val == 20)
					phymode = MODE_11NG_HT20;
				else if (bw_val == 40)
					phymode = MODE_11NG_HT40;
				break;
			case WNI_CFG_DOT11_MODE_ALL:
			case WNI_CFG_DOT11_MODE_11AC:
			case WNI_CFG_DOT11_MODE_11AC_ONLY:
				if (bw_val == 20)
					phymode = MODE_11AC_VHT20_2G;
				else if (bw_val == 40)
					phymode = MODE_11AC_VHT40_2G;
				break;
			case WNI_CFG_DOT11_MODE_11AX:
			case WNI_CFG_DOT11_MODE_11AX_ONLY:
				if (20 == bw_val)
					phymode = MODE_11AX_HE20_2G;
				else if (40 == bw_val)
					phymode = MODE_11AX_HE40_2G;
				break;
			default:
				break;
			}
		}
	} else if (WLAN_REG_IS_11P_CH(chan))
		phymode = MODE_11A;
	else {
		if (((CH_WIDTH_5MHZ == chan_width) ||
		     (CH_WIDTH_10MHZ == chan_width)) &&
		    ((WNI_CFG_DOT11_MODE_11A == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11N == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_ALL == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AC == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AX == dot11_mode)))
			phymode = MODE_11A;
		else {
			switch (dot11_mode) {
			case WNI_CFG_DOT11_MODE_11A:
				if (0 < bw_val)
					phymode = MODE_11A;
				break;
			case WNI_CFG_DOT11_MODE_11N:
			case WNI_CFG_DOT11_MODE_11N_ONLY:
				if (bw_val == 20)
					phymode = MODE_11NA_HT20;
				else if (40 <= bw_val)
					phymode = MODE_11NA_HT40;
				break;
			case WNI_CFG_DOT11_MODE_ALL:
			case WNI_CFG_DOT11_MODE_11AC:
			case WNI_CFG_DOT11_MODE_11AC_ONLY:
				if (bw_val == 20)
					phymode = MODE_11AC_VHT20;
				else if (bw_val == 40)
					phymode = MODE_11AC_VHT40;
				else if (bw_val == 80)
					phymode = MODE_11AC_VHT80;
				else if (chan_width == CH_WIDTH_160MHZ)
					phymode = MODE_11AC_VHT160;
				else if (chan_width == CH_WIDTH_80P80MHZ)
					phymode = MODE_11AC_VHT80_80;
				break;
			case WNI_CFG_DOT11_MODE_11AX:
			case WNI_CFG_DOT11_MODE_11AX_ONLY:
				if (20 == bw_val)
					phymode = MODE_11AX_HE20;
				else if (40 == bw_val)
					phymode = MODE_11AX_HE40;
				else if (80 == bw_val)
					phymode = MODE_11AX_HE80;
				else if (CH_WIDTH_160MHZ == chan_width)
					phymode = MODE_11AX_HE160;
				else if (CH_WIDTH_80P80MHZ == chan_width)
					phymode = MODE_11AX_HE80_80;
				break;
			default:
				break;
			}
		}
	}

	WMA_LOGD("%s: phymode %d channel %d ch_width %d dot11_mode %d",
		 __func__, phymode, chan, chan_width, dot11_mode);

	QDF_ASSERT(MODE_UNKNOWN != phymode);
	return phymode;
}

/**
 * wma_get_link_speed() -send command to get linkspeed
 * @handle: wma handle
 * @pLinkSpeed: link speed info
 *
 * Return: QDF status
 */
QDF_STATUS wma_get_link_speed(WMA_HANDLE handle, tSirLinkSpeedInfo *pLinkSpeed)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_mac_addr peer_macaddr;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue get link speed cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma_handle->wmi_handle,
				    wmi_service_estimate_linkspeed)) {
		WMA_LOGE("%s: Linkspeed feature bit not enabled Sending value 0 as link speed.",
			__func__);
		wma_send_link_speed(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Copy the peer macaddress to the wma buffer */
	WMI_CHAR_ARRAY_TO_MAC_ADDR(pLinkSpeed->peer_macaddr.bytes,
				   &peer_macaddr);
	WMA_LOGD("%s: pLinkSpeed->peerMacAddr: %pM, peer_macaddr.mac_addr31to0: 0x%x, peer_macaddr.mac_addr47to32: 0x%x",
		 __func__, pLinkSpeed->peer_macaddr.bytes,
		 peer_macaddr.mac_addr31to0,
		 peer_macaddr.mac_addr47to32);
	if (wmi_unified_get_link_speed_cmd(wma_handle->wmi_handle,
					peer_macaddr)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wma_get_peer_info(WMA_HANDLE handle,
				struct sir_peer_info_req *peer_info_req)
{
	tp_wma_handle wma_handle = (tp_wma_handle)handle;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t  wmi_buf;
	uint32_t  len;
	uint8_t *buf_ptr;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue get rssi",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	len  = sizeof(wmi_request_stats_cmd_fixed_param);
	wmi_buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(wmi_buf);

	cmd = (wmi_request_stats_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_request_stats_cmd_fixed_param));

	cmd->stats_id = WMI_REQUEST_PEER_STAT;
	cmd->vdev_id = peer_info_req->sessionid;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_info_req->peer_macaddr.bytes,
				   &cmd->peer_macaddr);
	wma_handle->get_sta_peer_info = true;

	if (wmi_unified_cmd_send(wma_handle->wmi_handle, wmi_buf, len,
				WMI_REQUEST_STATS_CMDID)) {
		WMA_LOGE("Failed to send host stats request to fw");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&(wma_handle->peer_macaddr),
					&(peer_info_req->peer_macaddr),
					QDF_MAC_ADDR_SIZE);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wma_get_peer_info_ext(WMA_HANDLE handle,
				struct sir_peer_info_ext_req *peer_info_req)
{
	tp_wma_handle wma_handle = (tp_wma_handle)handle;
	wmi_request_peer_stats_info_cmd_fixed_param *cmd;
	wmi_buf_t  wmi_buf;
	uint32_t  len;
	uint8_t *buf_ptr;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue get rssi",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	WMA_LOGI("%s send WMI_REQUEST_PEER_STATS_INFO_CMDID", __func__);

	len  = sizeof(wmi_request_peer_stats_info_cmd_fixed_param);
	wmi_buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(wmi_buf);

	cmd = (wmi_request_peer_stats_info_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_request_peer_stats_info_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_request_peer_stats_info_cmd_fixed_param));
	cmd->vdev_id = peer_info_req->sessionid;
	cmd->request_type = WMI_REQUEST_ONE_PEER_STATS_INFO;
	wma_handle->get_one_peer_info = true;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_info_req->peer_macaddr.bytes,
			&cmd->peer_macaddr);
	cmd->reset_after_request = peer_info_req->reset_after_request;

	if (wmi_unified_cmd_send(wma_handle->wmi_handle, wmi_buf, len,
				WMI_REQUEST_PEER_STATS_INFO_CMDID)) {
		WMA_LOGE("Failed to send peer stats request to fw");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGI("%s vdev_id %d, mac %pM, req_type %x, reset %x",
			__func__,
			cmd->vdev_id,
			peer_info_req->peer_macaddr.bytes,
			cmd->request_type,
			cmd->reset_after_request);

	qdf_mem_copy(&(wma_handle->peer_macaddr),
					&(peer_info_req->peer_macaddr),
					QDF_MAC_ADDR_SIZE);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_add_beacon_filter() - Issue WMI command to set beacon filter
 * @wma: wma handler
 * @filter_params: beacon_filter_param to set
 *
 * Return: Return QDF_STATUS
 */
QDF_STATUS wma_add_beacon_filter(WMA_HANDLE handle,
				struct beacon_filter_param *filter_params)
{
	int i;
	wmi_buf_t wmi_buf;
	u_int8_t *buf;
	A_UINT32 *ie_map;
	int ret;
	struct wma_txrx_node *iface;
	tp_wma_handle wma = (tp_wma_handle) handle;

	wmi_add_bcn_filter_cmd_fixed_param *cmd;
	int len = sizeof(wmi_add_bcn_filter_cmd_fixed_param);

	len += WMI_TLV_HDR_SIZE;
	len += BCN_FLT_MAX_ELEMS_IE_LIST*sizeof(A_UINT32);

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue set beacon filter",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	iface = &wma->interfaces[filter_params->vdev_id];
	qdf_mem_copy(&iface->beacon_filter, filter_params,
			sizeof(struct beacon_filter_param));
	iface->beacon_filter_enabled = true;

	wmi_buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf = (u_int8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_add_bcn_filter_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	cmd->vdev_id = filter_params->vdev_id;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_add_bcn_filter_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_add_bcn_filter_cmd_fixed_param));

	buf += sizeof(wmi_add_bcn_filter_cmd_fixed_param);

	WMITLV_SET_HDR(buf, WMITLV_TAG_ARRAY_UINT32,
			(BCN_FLT_MAX_ELEMS_IE_LIST * sizeof(u_int32_t)));

	ie_map = (A_UINT32 *)(buf + WMI_TLV_HDR_SIZE);
	for (i = 0; i < BCN_FLT_MAX_ELEMS_IE_LIST; i++) {
		ie_map[i] = filter_params->ie_map[i];
		WMA_LOGD("beacon filter ie map = %u", ie_map[i]);
	}

	ret = wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, len,
			WMI_ADD_BCN_FILTER_CMDID);
	if (ret) {
		WMA_LOGE("Failed to send wmi add beacon filter = %d",
				ret);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
* wma_remove_beacon_filter() - Issue WMI command to remove beacon filter
* @wma: wma handler
* @filter_params: beacon_filter_params
*
* Return: Return QDF_STATUS
*/
QDF_STATUS wma_remove_beacon_filter(WMA_HANDLE handle,
				struct beacon_filter_param *filter_params)
{
	wmi_buf_t buf;
	tp_wma_handle wma = (tp_wma_handle) handle;
	wmi_rmv_bcn_filter_cmd_fixed_param *cmd;
	int len = sizeof(wmi_rmv_bcn_filter_cmd_fixed_param);
	int ret;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, cannot issue remove beacon filter",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_rmv_bcn_filter_cmd_fixed_param *)wmi_buf_data(buf);
	cmd->vdev_id = filter_params->vdev_id;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_rmv_bcn_filter_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_rmv_bcn_filter_cmd_fixed_param));

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
			WMI_RMV_BCN_FILTER_CMDID);
	if (ret) {
		WMA_LOGE("Failed to send wmi remove beacon filter = %d",
				ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_adapt_dwelltime_params() - send adaptive dwelltime configuration
 * params to firmware
 * @wma_handle:	 wma handler
 * @dwelltime_params: pointer to dwelltime_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wma_send_adapt_dwelltime_params(WMA_HANDLE handle,
			struct adaptive_dwelltime_params *dwelltime_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct wmi_adaptive_dwelltime_params wmi_param = {0};
	int32_t err;

	wmi_param.is_enabled = dwelltime_params->is_enabled;
	wmi_param.dwelltime_mode = dwelltime_params->dwelltime_mode;
	wmi_param.lpf_weight = dwelltime_params->lpf_weight;
	wmi_param.passive_mon_intval = dwelltime_params->passive_mon_intval;
	wmi_param.wifi_act_threshold = dwelltime_params->wifi_act_threshold;
	err = wmi_unified_send_adapt_dwelltime_params_cmd(wma_handle->
					wmi_handle, &wmi_param);
	if (err)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wma_send_dbs_scan_selection_params(WMA_HANDLE handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	int32_t err;

	err = wmi_unified_send_dbs_scan_sel_params_cmd(wma_handle->
					wmi_handle, dbs_scan_params);
	if (err)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_unified_fw_profiling_cmd() - send FW profiling cmd to WLAN FW
 * @wma: wma handle
 * @cmd: Profiling command index
 * @value1: parameter1 value
 * @value2: parameter2 value
 *
 * Return: 0 for success else error code
 */
QDF_STATUS wma_unified_fw_profiling_cmd(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2)
{
	int ret;

	ret = wmi_unified_fw_profiling_data_cmd(wmi_handle, cmd,
			value1, value2);
	if (ret) {
		WMA_LOGE("enable cmd Failed for id %d value %d",
				value1, value2);
		return ret;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wow_set_wake_time() - set timer pattern tlv, so that firmware will wake
 * up host after specified time is elapsed
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @cookie: value to identify reason why host set up wake call.
 * @time: time in ms
 *
 * Return: QDF status
 */
static QDF_STATUS wma_wow_set_wake_time(WMA_HANDLE wma_handle, uint8_t vdev_id,
					uint32_t cookie, uint32_t time)
{
	int ret;
	tp_wma_handle wma = (tp_wma_handle)wma_handle;

	WMA_LOGD(FL("send timer patter with time: %d and vdev = %d to fw"),
		    time, vdev_id);
	ret = wmi_unified_wow_timer_pattern_cmd(wma->wmi_handle, vdev_id,
						cookie, time);
	if (ret) {
		WMA_LOGE(FL("Failed to send timer patter to fw"));
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_check_and_set_wake_timer(): checks all interfaces and if any interface
 * has install_key pending, sets timer pattern in fw to wake up host after
 * specified time has elapsed.
 * @wma: wma handle
 * @time: time after which host wants to be awaken.
 *
 * Return: None
 */
void wma_check_and_set_wake_timer(uint32_t time)
{
	int i;
	struct wma_txrx_node *iface;
	bool is_set_key_in_progress = false;
	t_wma_handle *wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		WMA_LOGE("%s: WMA is closed",
			__func__);
		return;
	}

	if (!wmi_service_enabled(wma->wmi_handle,
		wmi_service_wow_wakeup_by_timer_pattern)) {
		WMA_LOGD("TIME_PATTERN is not enabled");
		return;
	}

	for (i = 0; i < wma->max_bssid; i++) {
		iface = &wma->interfaces[i];
		if (iface->is_vdev_valid && iface->is_waiting_for_key) {
			/*
			 * right now cookie is dont care, since FW disregards
			 * that.
			 */
			is_set_key_in_progress = true;
			wma_wow_set_wake_time((WMA_HANDLE)wma, i, 0, time);
			break;
		}
	}

	if (!is_set_key_in_progress)
		WMA_LOGD("set key not in progress for any vdev");
}

/**
 * wmi_unified_nat_keepalive_enable() - enable NAT keepalive filter
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
int wmi_unified_nat_keepalive_enable(tp_wma_handle wma, uint8_t vdev_id)
{

	if (wmi_unified_nat_keepalive_en_cmd(wma->wmi_handle, vdev_id))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_unified_csa_offload_enable() - sen CSA offload enable command
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
int wma_unified_csa_offload_enable(tp_wma_handle wma, uint8_t vdev_id)
{
	if (wmi_unified_csa_offload_enable(wma->wmi_handle,
				 vdev_id)) {
		WMA_LOGP("%s: Failed to send CSA offload enable command",
			 __func__);
		return -EIO;
	}

	return 0;
}

#ifdef WLAN_FEATURE_NAN
/**
 * wma_nan_rsp_event_handler() - Function is used to handle nan response
 * @handle: wma handle
 * @event_buf: event buffer
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
int wma_nan_rsp_event_handler(void *handle, uint8_t *event_buf,
			      uint32_t len)
{
	WMI_NAN_EVENTID_param_tlvs *param_buf;
	tSirNanEvent *nan_rsp_event;
	wmi_nan_event_hdr *nan_rsp_event_hdr;
	QDF_STATUS status;
	struct scheduler_msg message = {0};
	uint8_t *buf_ptr;
	uint32_t alloc_len;

	/*
	 * This is how received event_buf looks like
	 *
	 * <-------------------- event_buf ----------------------------------->
	 *
	 * <--wmi_nan_event_hdr--><---WMI_TLV_HDR_SIZE---><----- data -------->
	 *
	 * +-----------+---------+-----------------------+--------------------+
	 * | tlv_header| data_len| WMITLV_TAG_ARRAY_BYTE | nan_rsp_event_data |
	 * +-----------+---------+-----------------------+--------------------+
	 */

	WMA_LOGD("%s: Posting NaN response event to SME", __func__);
	param_buf = (WMI_NAN_EVENTID_param_tlvs *) event_buf;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid nan response event buf", __func__);
		return -EINVAL;
	}
	nan_rsp_event_hdr = param_buf->fixed_param;
	buf_ptr = (uint8_t *) nan_rsp_event_hdr;
	alloc_len = sizeof(tSirNanEvent);
	alloc_len += nan_rsp_event_hdr->data_len;
	if (nan_rsp_event_hdr->data_len > ((WMI_SVC_MSG_MAX_SIZE -
	    sizeof(*nan_rsp_event_hdr)) / sizeof(uint8_t)) ||
	    nan_rsp_event_hdr->data_len > param_buf->num_data) {
		WMA_LOGE("excess data length:%d, num_data:%d",
			nan_rsp_event_hdr->data_len, param_buf->num_data);
		return -EINVAL;
	}
	nan_rsp_event = (tSirNanEvent *) qdf_mem_malloc(alloc_len);
	if (NULL == nan_rsp_event) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return -ENOMEM;
	}

	nan_rsp_event->event_data_len = nan_rsp_event_hdr->data_len;
	qdf_mem_copy(nan_rsp_event->event_data, buf_ptr +
		     sizeof(wmi_nan_event_hdr) + WMI_TLV_HDR_SIZE,
		     nan_rsp_event->event_data_len);
	message.type = eWNI_SME_NAN_EVENT;
	message.bodyptr = (void *)nan_rsp_event;
	message.bodyval = 0;

	status = scheduler_post_msg(QDF_MODULE_ID_SME, &message);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to post NaN response event to SME",
			 __func__);
		qdf_mem_free(nan_rsp_event);
		return -EFAULT;
	}
	WMA_LOGD("%s: NaN response event Posted to SME", __func__);
	return 0;
}
#else
static int wma_nan_rsp_event_handler(void *handle, uint8_t *event_buf,
				     uint32_t len)
{
	return 0;
}
#endif /* WLAN_FEATURE_NAN */

/**
 * wma_csa_offload_handler() - CSA event handler
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * This event is sent by firmware when it receives CSA IE.
 *
 * Return: 0 for success or error code
 */
int wma_csa_offload_handler(void *handle, uint8_t *event, uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_CSA_HANDLING_EVENTID_param_tlvs *param_buf;
	wmi_csa_event_fixed_param *csa_event;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint8_t vdev_id = 0;
	uint8_t cur_chan = 0;
	struct ieee80211_channelswitch_ie *csa_ie;
	struct csa_offload_params *csa_offload_event;
	struct ieee80211_extendedchannelswitch_ie *xcsa_ie;
	struct ieee80211_ie_wide_bw_switch *wb_ie;
	struct wma_txrx_node *intr = wma->interfaces;

	param_buf = (WMI_CSA_HANDLING_EVENTID_param_tlvs *) event;

	WMA_LOGD("%s: Enter", __func__);
	if (!param_buf) {
		WMA_LOGE("Invalid csa event buffer");
		return -EINVAL;
	}
	csa_event = param_buf->fixed_param;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&csa_event->i_addr2, &bssid[0]);

	if (wma_find_vdev_by_bssid(wma, bssid, &vdev_id) == NULL) {
		WMA_LOGE("Invalid bssid received %s:%d", __func__, __LINE__);
		return -EINVAL;
	}

	csa_offload_event = qdf_mem_malloc(sizeof(*csa_offload_event));
	if (!csa_offload_event) {
		WMA_LOGE("QDF MEM Alloc Failed for csa_offload_event");
		return -EINVAL;
	}

	qdf_mem_zero(csa_offload_event, sizeof(*csa_offload_event));
	qdf_mem_copy(csa_offload_event->bssId, &bssid, IEEE80211_ADDR_LEN);

	if (csa_event->ies_present_flag & WMI_CSA_IE_PRESENT) {
		csa_ie = (struct ieee80211_channelswitch_ie *)
						(&csa_event->csa_ie[0]);
		csa_offload_event->channel = csa_ie->newchannel;
		csa_offload_event->switch_mode = csa_ie->switchmode;
	} else if (csa_event->ies_present_flag & WMI_XCSA_IE_PRESENT) {
		xcsa_ie = (struct ieee80211_extendedchannelswitch_ie *)
						(&csa_event->xcsa_ie[0]);
		csa_offload_event->channel = xcsa_ie->newchannel;
		csa_offload_event->switch_mode = xcsa_ie->switchmode;
		csa_offload_event->new_op_class = xcsa_ie->newClass;
	} else {
		WMA_LOGE("CSA Event error: No CSA IE present");
		qdf_mem_free(csa_offload_event);
		return -EINVAL;
	}

	if (csa_event->ies_present_flag & WMI_WBW_IE_PRESENT) {
		wb_ie = (struct ieee80211_ie_wide_bw_switch *)
						(&csa_event->wb_ie[0]);
		csa_offload_event->new_ch_width = wb_ie->new_ch_width;
		csa_offload_event->new_ch_freq_seg1 = wb_ie->new_ch_freq_seg1;
		csa_offload_event->new_ch_freq_seg2 = wb_ie->new_ch_freq_seg2;
	}

	csa_offload_event->ies_present_flag = csa_event->ies_present_flag;

	WMA_LOGD("CSA: New Channel = %d BSSID:%pM",
		 csa_offload_event->channel, csa_offload_event->bssId);

	cur_chan = cds_freq_to_chan(intr[vdev_id].mhz);
	/*
	 * basic sanity check: requested channel should not be 0
	 * and equal to home channel
	 */
	if (0 == csa_offload_event->channel) {
		WMA_LOGE("CSA Event with channel %d. Ignore !!",
			 csa_offload_event->channel);
		qdf_mem_free(csa_offload_event);
		return -EINVAL;
	}
	wma->interfaces[vdev_id].is_channel_switch = true;
	wma_send_msg(wma, WMA_CSA_OFFLOAD_EVENT, (void *)csa_offload_event, 0);
	return 0;
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * wma_oem_data_response_handler() - OEM data response event handler
 * @handle: wma handle
 * @datap: data ptr
 * @len: data length
 *
 * Return: 0 for success or error code
 */
int wma_oem_data_response_handler(void *handle,
				  uint8_t *datap, uint32_t len)
{
	WMI_OEM_RESPONSE_EVENTID_param_tlvs *param_buf;
	uint8_t *data;
	uint32_t datalen;
	struct oem_data_rsp *oem_rsp;
	tpAniSirGlobal pmac = cds_get_context(QDF_MODULE_ID_PE);

	if (!pmac) {
		WMA_LOGE(FL("Invalid pmac"));
		return -EINVAL;
	}

	if (!pmac->sme.oem_data_rsp_callback) {
		WMA_LOGE(FL("Callback not registered"));
		return -EINVAL;
	}

	param_buf = (WMI_OEM_RESPONSE_EVENTID_param_tlvs *) datap;
	if (!param_buf) {
		WMA_LOGE(FL("Received NULL buf ptr from FW"));
		return -ENOMEM;
	}

	data = param_buf->data;
	datalen = param_buf->num_data;

	if (!data) {
		WMA_LOGE(FL("Received NULL data from FW"));
		return -EINVAL;
	}

	if (datalen > OEM_DATA_RSP_SIZE) {
		WMA_LOGE(FL("Received data len %d exceeds max value %d"),
			 datalen, OEM_DATA_RSP_SIZE);
		return -EINVAL;
	}

	oem_rsp = qdf_mem_malloc(sizeof(*oem_rsp));
	if (!oem_rsp) {
		WMA_LOGE(FL("Failed to alloc oem_data_rsp"));
		return -ENOMEM;
	}
	oem_rsp->rsp_len = datalen;
	if (oem_rsp->rsp_len) {
		oem_rsp->data = qdf_mem_malloc(oem_rsp->rsp_len);
		if (!oem_rsp->data) {
			WMA_LOGE(FL("malloc failed for data"));
			qdf_mem_free(oem_rsp);
			return -ENOMEM;
		}
	} else {
		WMA_LOGE(FL("Invalid rsp length: %d"),
			 oem_rsp->rsp_len);
		qdf_mem_free(oem_rsp);
		return -EINVAL;
	}

	qdf_mem_copy(oem_rsp->data, data, datalen);

	WMA_LOGD("Sending OEM_DATA_RSP(len: %d) to upper layer", datalen);

	pmac->sme.oem_data_rsp_callback(oem_rsp);

	if (oem_rsp->data)
		qdf_mem_free(oem_rsp->data);
	qdf_mem_free(oem_rsp);

	return 0;
}

/**
 * wma_start_oem_data_req() - start OEM data request to target
 * @wma_handle: wma handle
 * @oem_data_req: start request params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_start_oem_data_req(tp_wma_handle wma_handle,
			    struct oem_data_req *oem_data_req)
{
	int ret = 0;

	WMA_LOGD(FL("Send OEM Data Request to target"));

	if (!oem_data_req || !oem_data_req->data) {
		WMA_LOGE(FL("oem_data_req is null"));
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE(FL("WMA - closed, can not send Oem data request cmd"));
		qdf_mem_free(oem_data_req->data);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_start_oem_data_cmd(wma_handle->wmi_handle,
				   oem_data_req->data_len,
				   oem_data_req->data);

	if (!QDF_IS_STATUS_SUCCESS(ret))
		WMA_LOGE(FL("wmi cmd send failed"));

	return ret;
}
#endif /* FEATURE_OEM_DATA_SUPPORT */

#if !defined(REMOVE_PKT_LOG)
/**
 * wma_pktlog_wmi_send_cmd() - send pktlog enable/disable command to target
 * @handle: wma handle
 * @params: pktlog params
 *
 * Return: QDF status
 */
QDF_STATUS wma_pktlog_wmi_send_cmd(WMA_HANDLE handle,
				   struct ath_pktlog_wmi_params *params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	int ret;

	ret = wmi_unified_pktlog_wmi_send_cmd(wma_handle->wmi_handle,
			params->pktlog_event,
			params->cmd_id, params->user_triggered);
	if (ret)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
#endif /* REMOVE_PKT_LOG */

/**
 * wma_wow_wake_reason_str() -  Converts wow wakeup reason code to text format
 * @wake_reason - WOW wake reason
 *
 * Return: reason code in string format
 */
static const u8 *wma_wow_wake_reason_str(A_INT32 wake_reason)
{
	switch (wake_reason) {
	case WOW_REASON_UNSPECIFIED:
		return "UNSPECIFIED";
	case WOW_REASON_NLOD:
		return "NLOD";
	case WOW_REASON_AP_ASSOC_LOST:
		return "AP_ASSOC_LOST";
	case WOW_REASON_LOW_RSSI:
		return "LOW_RSSI";
	case WOW_REASON_DEAUTH_RECVD:
		return "DEAUTH_RECVD";
	case WOW_REASON_DISASSOC_RECVD:
		return "DISASSOC_RECVD";
	case WOW_REASON_GTK_HS_ERR:
		return "GTK_HS_ERR";
	case WOW_REASON_EAP_REQ:
		return "EAP_REQ";
	case WOW_REASON_FOURWAY_HS_RECV:
		return "FOURWAY_HS_RECV";
	case WOW_REASON_TIMER_INTR_RECV:
		return "TIMER_INTR_RECV";
	case WOW_REASON_PATTERN_MATCH_FOUND:
		return "PATTERN_MATCH_FOUND";
	case WOW_REASON_RECV_MAGIC_PATTERN:
		return "RECV_MAGIC_PATTERN";
	case WOW_REASON_P2P_DISC:
		return "P2P_DISC";
	case WOW_REASON_WLAN_HB:
		return "WLAN_HB";
	case WOW_REASON_CSA_EVENT:
		return "CSA_EVENT";
	case WOW_REASON_PROBE_REQ_WPS_IE_RECV:
		return "PROBE_REQ_WPS_IE_RECV";
	case WOW_REASON_AUTH_REQ_RECV:
		return "AUTH_REQ_RECV";
	case WOW_REASON_ASSOC_REQ_RECV:
		return "ASSOC_REQ_RECV";
	case WOW_REASON_HTT_EVENT:
		return "HTT_EVENT";
	case WOW_REASON_RA_MATCH:
		return "RA_MATCH";
	case WOW_REASON_HOST_AUTO_SHUTDOWN:
		return "HOST_AUTO_SHUTDOWN";
	case WOW_REASON_IOAC_MAGIC_EVENT:
		return "IOAC_MAGIC_EVENT";
	case WOW_REASON_IOAC_SHORT_EVENT:
		return "IOAC_SHORT_EVENT";
	case WOW_REASON_IOAC_EXTEND_EVENT:
		return "IOAC_EXTEND_EVENT";
	case WOW_REASON_IOAC_TIMER_EVENT:
		return "IOAC_TIMER_EVENT";
	case WOW_REASON_ROAM_HO:
		return "ROAM_HO";
	case WOW_REASON_DFS_PHYERR_RADADR_EVENT:
		return "DFS_PHYERR_RADADR_EVENT";
	case WOW_REASON_BEACON_RECV:
		return "BEACON_RECV";
	case WOW_REASON_CLIENT_KICKOUT_EVENT:
		return "CLIENT_KICKOUT_EVENT";
	case WOW_REASON_NAN_EVENT:
		return "NAN_EVENT";
	case WOW_REASON_EXTSCAN:
		return "EXTSCAN";
	case WOW_REASON_RSSI_BREACH_EVENT:
		return "RSSI_BREACH_EVENT";
	case WOW_REASON_IOAC_REV_KA_FAIL_EVENT:
		return "IOAC_REV_KA_FAIL_EVENT";
	case WOW_REASON_IOAC_SOCK_EVENT:
		return "IOAC_SOCK_EVENT";
	case WOW_REASON_NLO_SCAN_COMPLETE:
		return "NLO_SCAN_COMPLETE";
	case WOW_REASON_PACKET_FILTER_MATCH:
		return "PACKET_FILTER_MATCH";
	case WOW_REASON_ASSOC_RES_RECV:
		return "ASSOC_RES_RECV";
	case WOW_REASON_REASSOC_REQ_RECV:
		return "REASSOC_REQ_RECV";
	case WOW_REASON_REASSOC_RES_RECV:
		return "REASSOC_RES_RECV";
	case WOW_REASON_ACTION_FRAME_RECV:
		return "ACTION_FRAME_RECV";
	case WOW_REASON_BPF_ALLOW:
		return "BPF_ALLOW";
	case WOW_REASON_NAN_DATA:
		return "NAN_DATA";
	case WOW_REASON_OEM_RESPONSE_EVENT:
		return "OEM_RESPONSE_EVENT";
	case WOW_REASON_TDLS_CONN_TRACKER_EVENT:
		return "TDLS_CONN_TRACKER_EVENT";
	case WOW_REASON_CRITICAL_LOG:
		return "CRITICAL_LOG";
	case WOW_REASON_P2P_LISTEN_OFFLOAD:
		return "P2P_LISTEN_OFFLOAD";
	case WOW_REASON_NAN_EVENT_WAKE_HOST:
		return "NAN_EVENT_WAKE_HOST";
	case WOW_REASON_DEBUG_TEST:
		return "DEBUG_TEST";
	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
		return "CHIP_POWER_FAILURE_DETECT";
	case WOW_REASON_11D_SCAN:
		return "11D_SCAN";
	case WOW_REASON_SAP_OBSS_DETECTION:
		return "SAP_OBSS_DETECTION";
	case WOW_REASON_BSS_COLOR_COLLISION_DETECT:
		return "BSS_COLOR_COLLISION_DETECT";
	default:
		return "unknown";
	}
}

/**
 * wma_wow_stats_display() - display wow wake up stats
 * @stats: per vdev stats counters
 *
 * Return: none
 */
static void wma_wow_stats_display(struct sir_vdev_wow_stats *stats)
{
	WMA_LOGA("uc %d bc %d v4_mc %d v6_mc %d ra %d ns %d na %d pno_match %d pno_complete %d gscan %d low_rssi %d rssi_breach %d icmp %d icmpv6 %d oem %d",
		stats->ucast,
		stats->bcast,
		stats->ipv4_mcast,
		stats->ipv6_mcast,
		stats->ipv6_mcast_ra,
		stats->ipv6_mcast_ns,
		stats->ipv6_mcast_na,
		stats->pno_match,
		stats->pno_complete,
		stats->gscan,
		stats->low_rssi,
		stats->rssi_breach,
		stats->icmpv4,
		stats->icmpv6,
		stats->oem_response);
}

static void wma_print_wow_stats(t_wma_handle *wma,
				WOW_EVENT_INFO_fixed_param *wake_info)
{
	struct sir_vdev_wow_stats *stats;

	switch (wake_info->wake_reason) {
	case WOW_REASON_BPF_ALLOW:
	case WOW_REASON_PATTERN_MATCH_FOUND:
	case WOW_REASON_RA_MATCH:
	case WOW_REASON_NLOD:
	case WOW_REASON_NLO_SCAN_COMPLETE:
	case WOW_REASON_LOW_RSSI:
	case WOW_REASON_EXTSCAN:
	case WOW_REASON_RSSI_BREACH_EVENT:
	case WOW_REASON_OEM_RESPONSE_EVENT:
	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
	case WOW_REASON_11D_SCAN:
		break;
	default:
		return;
	}

	stats = &wma->interfaces[wake_info->vdev_id].wow_stats;
	wma_wow_stats_display(stats);
}

/**
 * wma_inc_wow_stats() - maintain wow pattern match wake up stats
 * @wma: wma handle, containing the stats counters
 * @wake_info: the wake event information
 *
 * Return: none
 */
static void wma_inc_wow_stats(t_wma_handle *wma,
			      WOW_EVENT_INFO_fixed_param *wake_info)
{
	struct sir_vdev_wow_stats *stats;

	if (wake_info->wake_reason == WOW_REASON_UNSPECIFIED) {
		wma->wow_unspecified_wake_count++;
		return;
	}

	stats = &wma->interfaces[wake_info->vdev_id].wow_stats;
	switch (wake_info->wake_reason) {
	case WOW_REASON_RA_MATCH:
		stats->ipv6_mcast++;
		stats->ipv6_mcast_ra++;
		stats->icmpv6++;
		break;
	case WOW_REASON_NLOD:
		stats->pno_match++;
		break;
	case WOW_REASON_NLO_SCAN_COMPLETE:
		stats->pno_complete++;
		break;
	case WOW_REASON_LOW_RSSI:
		stats->low_rssi++;
		break;
	case WOW_REASON_EXTSCAN:
		stats->gscan++;
		break;
	case WOW_REASON_RSSI_BREACH_EVENT:
		stats->rssi_breach++;
		break;
	case WOW_REASON_OEM_RESPONSE_EVENT:
		stats->oem_response++;
	case WOW_REASON_11D_SCAN:
		stats->scan_11d++;
		break;
	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
		stats->pwr_save_fail_detected++;
		break;
	}
}

#ifdef FEATURE_WLAN_EXTSCAN
/**
 * wma_extscan_get_eventid_from_tlvtag() - map tlv tag to corresponding event id
 * @tag: WMI TLV tag
 *
 * Return:
 *	0 if TLV tag is invalid
 *	else return corresponding WMI event id
 */
static int wma_extscan_get_eventid_from_tlvtag(uint32_t tag)
{
	uint32_t event_id;

	switch (tag) {
	case WMITLV_TAG_STRUC_wmi_extscan_start_stop_event_fixed_param:
		event_id = WMI_EXTSCAN_START_STOP_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_operation_event_fixed_param:
		event_id = WMI_EXTSCAN_OPERATION_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_table_usage_event_fixed_param:
		event_id = WMI_EXTSCAN_TABLE_USAGE_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_cached_results_event_fixed_param:
		event_id = WMI_EXTSCAN_CACHED_RESULTS_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_wlan_change_results_event_fixed_param:
		event_id = WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_hotlist_match_event_fixed_param:
		event_id = WMI_EXTSCAN_HOTLIST_MATCH_EVENTID;
		break;

	case WMITLV_TAG_STRUC_wmi_extscan_capabilities_event_fixed_param:
		event_id = WMI_EXTSCAN_CAPABILITIES_EVENTID;
		break;

	default:
		event_id = 0;
		WMA_LOGE("%s: Unknown tag: %d", __func__, tag);
		break;
	}

	WMA_LOGI("%s: For tag %d WMI event 0x%x", __func__, tag, event_id);
	return event_id;
}
#else
static int wma_extscan_get_eventid_from_tlvtag(uint32_t tag)
{
	return 0;
}
#endif

/**
 * wow_get_wmi_eventid() - map reason or tlv tag to corresponding event id
 * @tag: WMI TLV tag
 * @reason: WOW reason
 *
 * WOW reason type is primarily used to find the ID. If there could be
 * multiple events that can be sent as a WOW event with same reason
 * then tlv tag is used to identify the corresponding event.
 *
 * Return:
 *      0 if TLV tag/reason is invalid
 *      else return corresponding WMI event id
 */
static int wow_get_wmi_eventid(int32_t reason, uint32_t tag)
{
	int event_id;

	switch (reason) {
	case WOW_REASON_AP_ASSOC_LOST:
		event_id = WMI_ROAM_EVENTID;
		break;
	case WOW_REASON_NLO_SCAN_COMPLETE:
		event_id = WMI_NLO_SCAN_COMPLETE_EVENTID;
		break;
	case WOW_REASON_CSA_EVENT:
		event_id = WMI_CSA_HANDLING_EVENTID;
		break;
	case WOW_REASON_LOW_RSSI:
		event_id = WMI_ROAM_EVENTID;
		break;
	case WOW_REASON_CLIENT_KICKOUT_EVENT:
		event_id = WMI_PEER_STA_KICKOUT_EVENTID;
		break;
	case WOW_REASON_EXTSCAN:
		event_id = wma_extscan_get_eventid_from_tlvtag(tag);
		break;
	case WOW_REASON_RSSI_BREACH_EVENT:
		event_id = WMI_RSSI_BREACH_EVENTID;
		break;
	case WOW_REASON_NAN_EVENT:
		event_id = WMI_NAN_EVENTID;
		break;
	case WOW_REASON_NAN_DATA:
		event_id = wma_ndp_get_eventid_from_tlvtag(tag);
		break;
	case WOW_REASON_TDLS_CONN_TRACKER_EVENT:
		event_id = WOW_TDLS_CONN_TRACKER_EVENT;
		break;
	case WOW_REASON_ROAM_HO:
		event_id = WMI_ROAM_EVENTID;
		break;
	case WOW_REASON_11D_SCAN:
		event_id = WMI_11D_NEW_COUNTRY_EVENTID;
		break;
	default:
		WMA_LOGD(FL("No Event Id for WOW reason %s(%d)"),
			 wma_wow_wake_reason_str(reason), reason);
		event_id = 0;
		break;
	}
	wlan_roam_debug_log(WMA_INVALID_VDEV_ID, DEBUG_WOW_REASON,
			    DEBUG_INVALID_PEER_ID, NULL, NULL,
			    reason, event_id);

	return event_id;
}

/**
 * is_piggybacked_event() - Returns true if the given wake reason indicates
 *	there will be piggybacked TLV event data
 * @reason: WOW reason
 *
 * There are three types of WoW event payloads: none, piggybacked event, and
 * network packet. This function returns true for wake reasons that fall into
 * the piggybacked event case.
 *
 * Return: true for piggybacked event data
 */
static bool is_piggybacked_event(int32_t reason)
{
	switch (reason) {
	case WOW_REASON_AP_ASSOC_LOST:
	case WOW_REASON_NLO_SCAN_COMPLETE:
	case WOW_REASON_CSA_EVENT:
	case WOW_REASON_LOW_RSSI:
	case WOW_REASON_CLIENT_KICKOUT_EVENT:
	case WOW_REASON_EXTSCAN:
	case WOW_REASON_RSSI_BREACH_EVENT:
	case WOW_REASON_NAN_EVENT:
	case WOW_REASON_NAN_DATA:
	case WOW_REASON_TDLS_CONN_TRACKER_EVENT:
	case WOW_REASON_ROAM_HO:
		return true;
	default:
		return false;
	}
}

/**
 * wma_pkt_proto_subtype_to_string() - to convert proto subtype
 *         of data packet to string.
 * @proto_subtype: proto subtype for data packet
 *
 * This function returns the string for the proto subtype of
 * data packet.
 *
 * Return: string for proto subtype for data packet
 */
static const char *
wma_pkt_proto_subtype_to_string(enum qdf_proto_subtype proto_subtype)
{
	switch (proto_subtype) {
	case QDF_PROTO_EAPOL_M1:
		return "EAPOL M1";
	case QDF_PROTO_EAPOL_M2:
		return "EAPOL M2";
	case QDF_PROTO_EAPOL_M3:
		return "EAPOL M3";
	case QDF_PROTO_EAPOL_M4:
		return "EAPOL M4";
	case QDF_PROTO_DHCP_DISCOVER:
		return "DHCP DISCOVER";
	case QDF_PROTO_DHCP_REQUEST:
		return "DHCP REQUEST";
	case QDF_PROTO_DHCP_OFFER:
		return "DHCP OFFER";
	case QDF_PROTO_DHCP_ACK:
		return "DHCP ACK";
	case QDF_PROTO_DHCP_NACK:
		return "DHCP NACK";
	case QDF_PROTO_DHCP_RELEASE:
		return "DHCP RELEASE";
	case QDF_PROTO_DHCP_INFORM:
		return "DHCP INFORM";
	case QDF_PROTO_DHCP_DECLINE:
		return "DHCP DECLINE";
	case QDF_PROTO_ARP_REQ:
		return "ARP REQUEST";
	case QDF_PROTO_ARP_RES:
		return "ARP RESPONSE";
	case QDF_PROTO_ICMP_REQ:
		return "ICMP REQUEST";
	case QDF_PROTO_ICMP_RES:
		return "ICMP RESPONSE";
	case QDF_PROTO_ICMPV6_REQ:
		return "ICMPV6 REQUEST";
	case QDF_PROTO_ICMPV6_RES:
		return "ICMPV6 RESPONSE";
	case QDF_PROTO_ICMPV6_RS:
		return "ICMPV6 RS";
	case QDF_PROTO_ICMPV6_RA:
		return "ICMPV6 RA";
	case QDF_PROTO_ICMPV6_NS:
		return "ICMPV6 NS";
	case QDF_PROTO_ICMPV6_NA:
		return "ICMPV6 NA";
	case QDF_PROTO_IPV4_UDP:
		return "IPV4 UDP Packet";
	case QDF_PROTO_IPV4_TCP:
		return "IPV4 TCP Packet";
	case QDF_PROTO_IPV6_UDP:
		return "IPV6 UDP Packet";
	case QDF_PROTO_IPV6_TCP:
		return "IPV6 TCP Packet";
	default:
		return NULL;
	}
}

/**
 * wma_wow_get_pkt_proto_subtype() - get the proto subtype of the packet.
 * @data: Pointer to the packet data buffer
 * @len: length of the packet data buffer
 *
 * Return: proto subtype of the packet.
 */
static enum qdf_proto_subtype
wma_wow_get_pkt_proto_subtype(uint8_t *data, uint32_t len)
{
	uint16_t eth_type;
	uint8_t proto_type;

	if (len < QDF_NBUF_TRAC_ETH_TYPE_OFFSET + 2) {
		WMA_LOGE("Malformed ethernet packet: length %u < %d",
			 len, QDF_NBUF_TRAC_ETH_TYPE_OFFSET + 2);
		return QDF_PROTO_INVALID;
	}

	eth_type = *(uint16_t *)(data + QDF_NBUF_TRAC_ETH_TYPE_OFFSET);
	eth_type = qdf_cpu_to_be16(eth_type);

	WMA_LOGD("Ether Type: 0x%04x", eth_type);
	switch (eth_type) {
	case QDF_NBUF_TRAC_EAPOL_ETH_TYPE:
		if (len < WMA_EAPOL_SUBTYPE_GET_MIN_LEN)
			return QDF_PROTO_INVALID;

		WMA_LOGD("EAPOL Packet");
		return qdf_nbuf_data_get_eapol_subtype(data);

	case QDF_NBUF_TRAC_ARP_ETH_TYPE:
		if (len < WMA_ARP_SUBTYPE_GET_MIN_LEN)
			return QDF_PROTO_INVALID;

		WMA_LOGD("ARP Packet");
		return qdf_nbuf_data_get_arp_subtype(data);

	case QDF_NBUF_TRAC_IPV4_ETH_TYPE:
		if (len < WMA_IPV4_PROTO_GET_MIN_LEN)
			return QDF_PROTO_INVALID;

		WMA_LOGD("IPV4 Packet");

		proto_type = qdf_nbuf_data_get_ipv4_proto(data);
		WMA_LOGD("IPV4_proto_type: %u", proto_type);

		switch (proto_type) {
		case QDF_NBUF_TRAC_ICMP_TYPE:
			if (len < WMA_ICMP_SUBTYPE_GET_MIN_LEN)
				return QDF_PROTO_INVALID;

			WMA_LOGD("ICMP Packet");
			return qdf_nbuf_data_get_icmp_subtype(data);

		case QDF_NBUF_TRAC_UDP_TYPE:
			if (len < WMA_IS_DHCP_GET_MIN_LEN)
				return QDF_PROTO_IPV4_UDP;

			if (!qdf_nbuf_data_is_ipv4_dhcp_pkt(data))
				return QDF_PROTO_INVALID;

			if (len < WMA_DHCP_SUBTYPE_GET_MIN_LEN)
				return QDF_PROTO_INVALID;

			WMA_LOGD("DHCP Packet");
			return qdf_nbuf_data_get_dhcp_subtype(data);

		case QDF_NBUF_TRAC_TCP_TYPE:
			return QDF_PROTO_IPV4_TCP;

		default:
			return QDF_PROTO_INVALID;
		}

	case QDF_NBUF_TRAC_IPV6_ETH_TYPE:
		if (len < WMA_IPV6_PROTO_GET_MIN_LEN)
			return QDF_PROTO_INVALID;

		WMA_LOGD("IPV6 Packet");

		proto_type = qdf_nbuf_data_get_ipv6_proto(data);
		WMA_LOGD("IPV6_proto_type: %u", proto_type);

		switch (proto_type) {
		case QDF_NBUF_TRAC_ICMPV6_TYPE:
			if (len < WMA_ICMPV6_SUBTYPE_GET_MIN_LEN)
				return QDF_PROTO_INVALID;

			WMA_LOGD("ICMPV6 Packet");
			return qdf_nbuf_data_get_icmpv6_subtype(data);

		case QDF_NBUF_TRAC_UDP_TYPE:
			return QDF_PROTO_IPV6_UDP;

		case QDF_NBUF_TRAC_TCP_TYPE:
			return QDF_PROTO_IPV6_TCP;

		default:
			return QDF_PROTO_INVALID;
		}

	default:
		return QDF_PROTO_INVALID;
	}
}

static void wma_log_pkt_eapol(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len, key_len;

	if (length < WMA_EAPOL_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + EAPOL_PKT_LEN_OFFSET);
	key_len = *(uint16_t *)(data + EAPOL_KEY_LEN_OFFSET);
	WMA_LOGD("Pkt_len: %u, Key_len: %u",
		 qdf_cpu_to_be16(pkt_len), qdf_cpu_to_be16(key_len));
}

static void wma_log_pkt_dhcp(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len;
	uint32_t trans_id;

	if (length < WMA_DHCP_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + DHCP_PKT_LEN_OFFSET);
	trans_id = *(uint32_t *)(data + DHCP_TRANSACTION_ID_OFFSET);
	WMA_LOGD("Pkt_len: %u, Transaction_id: %u",
		 qdf_cpu_to_be16(pkt_len), qdf_cpu_to_be16(trans_id));
}

static void wma_log_pkt_icmpv4(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len, seq_num;

	if (length < WMA_IPV4_PKT_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + IPV4_PKT_LEN_OFFSET);
	seq_num = *(uint16_t *)(data + ICMP_SEQ_NUM_OFFSET);
	WMA_LOGD("Pkt_len: %u, Seq_num: %u",
		 qdf_cpu_to_be16(pkt_len), qdf_cpu_to_be16(seq_num));
}

static void wma_log_pkt_icmpv6(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len, seq_num;

	if (length < WMA_IPV6_PKT_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + IPV6_PKT_LEN_OFFSET);
	seq_num = *(uint16_t *)(data + ICMPV6_SEQ_NUM_OFFSET);
	WMA_LOGD("Pkt_len: %u, Seq_num: %u",
		 qdf_cpu_to_be16(pkt_len), qdf_cpu_to_be16(seq_num));
}

static void wma_log_pkt_ipv4(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len, src_port, dst_port;
	char *ip_addr;

	if (length < WMA_IPV4_PKT_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + IPV4_PKT_LEN_OFFSET);
	ip_addr = (char *)(data + IPV4_SRC_ADDR_OFFSET);
	WMA_LOGD("src addr %d:%d:%d:%d", ip_addr[0], ip_addr[1],
		 ip_addr[2], ip_addr[3]);
	ip_addr = (char *)(data + IPV4_DST_ADDR_OFFSET);
	WMA_LOGD("dst addr %d:%d:%d:%d", ip_addr[0], ip_addr[1],
		 ip_addr[2], ip_addr[3]);
	src_port = *(uint16_t *)(data + IPV4_SRC_PORT_OFFSET);
	dst_port = *(uint16_t *)(data + IPV4_DST_PORT_OFFSET);
	WMA_LOGD("Pkt_len: %u, src_port: %u, dst_port: %u",
		 qdf_cpu_to_be16(pkt_len),
		 qdf_cpu_to_be16(src_port),
		 qdf_cpu_to_be16(dst_port));
}

static void wma_log_pkt_ipv6(uint8_t *data, uint32_t length)
{
	uint16_t pkt_len, src_port, dst_port;
	char *ip_addr;

	if (length < WMA_IPV6_PKT_INFO_GET_MIN_LEN)
		return;

	pkt_len = *(uint16_t *)(data + IPV6_PKT_LEN_OFFSET);
	ip_addr = (char *)(data + IPV6_SRC_ADDR_OFFSET);
	WMA_LOGD("src addr "IPV6_ADDR_STR, ip_addr[0],
		 ip_addr[1], ip_addr[2], ip_addr[3], ip_addr[4],
		 ip_addr[5], ip_addr[6], ip_addr[7], ip_addr[8],
		 ip_addr[9], ip_addr[10], ip_addr[11],
		 ip_addr[12], ip_addr[13], ip_addr[14],
		 ip_addr[15]);
	ip_addr = (char *)(data + IPV6_DST_ADDR_OFFSET);
	WMA_LOGD("dst addr "IPV6_ADDR_STR, ip_addr[0],
		 ip_addr[1], ip_addr[2], ip_addr[3], ip_addr[4],
		 ip_addr[5], ip_addr[6], ip_addr[7], ip_addr[8],
		 ip_addr[9], ip_addr[10], ip_addr[11],
		 ip_addr[12], ip_addr[13], ip_addr[14],
		 ip_addr[15]);
	src_port = *(uint16_t *)(data + IPV6_SRC_PORT_OFFSET);
	dst_port = *(uint16_t *)(data + IPV6_DST_PORT_OFFSET);
	WMA_LOGD("Pkt_len: %u, src_port: %u, dst_port: %u",
		 qdf_cpu_to_be16(pkt_len),
		 qdf_cpu_to_be16(src_port),
		 qdf_cpu_to_be16(dst_port));
}

static void wma_log_pkt_tcpv4(uint8_t *data, uint32_t length)
{
	uint32_t seq_num;

	if (length < WMA_IPV4_PKT_INFO_GET_MIN_LEN)
		return;

	seq_num = *(uint32_t *)(data + IPV4_TCP_SEQ_NUM_OFFSET);
	WMA_LOGD("TCP_seq_num: %u", qdf_cpu_to_be16(seq_num));
}

static void wma_log_pkt_tcpv6(uint8_t *data, uint32_t length)
{
	uint32_t seq_num;

	if (length < WMA_IPV6_PKT_INFO_GET_MIN_LEN)
		return;

	seq_num = *(uint32_t *)(data + IPV6_TCP_SEQ_NUM_OFFSET);
	WMA_LOGD("TCP_seq_num: %u", qdf_cpu_to_be16(seq_num));
}

/**
 * wma_wow_parse_data_pkt() - API to parse data buffer for data
 *    packet that resulted in WOW wakeup.
 * @stats: per-vdev stats for tracking packet types
 * @data: Pointer to data buffer
 * @length: data buffer length
 *
 * This function parses the data buffer received (first few bytes of
 * skb->data) to get informaton like src mac addr, dst mac addr, packet
 * len, seq_num, etc. It also increments stats for different packet types.
 *
 * Return: void
 */
static void wma_wow_parse_data_pkt(struct sir_vdev_wow_stats *stats,
				   uint8_t *data,
				   uint32_t length)
{
	enum qdf_proto_subtype proto_subtype;
	const char *proto_subtype_name;
	uint8_t *dest_mac;
	uint8_t *src_mac;

	WMA_LOGD("packet length: %u", length);
	if (length < QDF_NBUF_TRAC_IPV4_OFFSET)
		return;

	src_mac = data + QDF_NBUF_SRC_MAC_OFFSET;
	dest_mac = data + QDF_NBUF_DEST_MAC_OFFSET;
	WMA_LOGD("Src_mac: " MAC_ADDRESS_STR ", Dst_mac: " MAC_ADDRESS_STR,
		 MAC_ADDR_ARRAY(src_mac), MAC_ADDR_ARRAY(dest_mac));

	switch (*dest_mac) {
	case WMA_BCAST_MAC_ADDR:
		stats->bcast++;
		break;
	case WMA_MCAST_IPV4_MAC_ADDR:
		stats->ipv4_mcast++;
		break;
	case WMA_MCAST_IPV6_MAC_ADDR:
		stats->ipv6_mcast++;
		break;
	default:
		stats->ucast++;
		break;
	}

	proto_subtype = wma_wow_get_pkt_proto_subtype(data, length);
	proto_subtype_name = wma_pkt_proto_subtype_to_string(proto_subtype);
	if (proto_subtype_name)
		WMA_LOGD("WOW Wakeup: %s rcvd", proto_subtype_name);

	switch (proto_subtype) {
	case QDF_PROTO_EAPOL_M1:
	case QDF_PROTO_EAPOL_M2:
	case QDF_PROTO_EAPOL_M3:
	case QDF_PROTO_EAPOL_M4:
		wma_log_pkt_eapol(data, length);
		break;

	case QDF_PROTO_DHCP_DISCOVER:
	case QDF_PROTO_DHCP_REQUEST:
	case QDF_PROTO_DHCP_OFFER:
	case QDF_PROTO_DHCP_ACK:
	case QDF_PROTO_DHCP_NACK:
	case QDF_PROTO_DHCP_RELEASE:
	case QDF_PROTO_DHCP_INFORM:
	case QDF_PROTO_DHCP_DECLINE:
		wma_log_pkt_dhcp(data, length);
		break;

	case QDF_PROTO_ICMP_REQ:
	case QDF_PROTO_ICMP_RES:
		stats->icmpv4++;
		wma_log_pkt_icmpv4(data, length);
		break;

	case QDF_PROTO_ICMPV6_REQ:
	case QDF_PROTO_ICMPV6_RES:
	case QDF_PROTO_ICMPV6_RS:
		stats->icmpv6++;
		wma_log_pkt_icmpv6(data, length);
		break;
	case QDF_PROTO_ICMPV6_RA:
		stats->icmpv6++;
		stats->ipv6_mcast_ra++;
		wma_log_pkt_icmpv6(data, length);
		break;
	case QDF_PROTO_ICMPV6_NS:
		stats->icmpv6++;
		stats->ipv6_mcast_ns++;
		wma_log_pkt_icmpv6(data, length);
		break;
	case QDF_PROTO_ICMPV6_NA:
		stats->icmpv6++;
		stats->ipv6_mcast_na++;
		wma_log_pkt_icmpv6(data, length);
		break;

	case QDF_PROTO_IPV4_UDP:
		wma_log_pkt_ipv4(data, length);
		break;
	case QDF_PROTO_IPV4_TCP:
		wma_log_pkt_ipv4(data, length);
		wma_log_pkt_tcpv4(data, length);
		break;

	case QDF_PROTO_IPV6_UDP:
		wma_log_pkt_ipv6(data, length);
		break;
	case QDF_PROTO_IPV6_TCP:
		wma_log_pkt_ipv6(data, length);
		wma_log_pkt_tcpv6(data, length);
		break;
	default:
		break;
	}
}

/**
 * wma_wow_dump_mgmt_buffer() - API to parse data buffer for mgmt.
 *    packet that resulted in WOW wakeup.
 * @wow_packet_buffer: Pointer to data buffer
 * @buf_len: length of data buffer
 *
 * This function parses the data buffer received (802.11 header)
 * to get informaton like src mac addr, dst mac addr, seq_num,
 * frag_num, etc.
 *
 * Return: void
 */
static void wma_wow_dump_mgmt_buffer(uint8_t *wow_packet_buffer,
				     uint32_t buf_len)
{
	struct ieee80211_frame_addr4 *wh;

	WMA_LOGD("wow_buf_pkt_len: %u", buf_len);
	wh = (struct ieee80211_frame_addr4 *)
		(wow_packet_buffer);
	if (buf_len >= sizeof(struct ieee80211_frame)) {
		uint8_t to_from_ds, frag_num;
		uint32_t seq_num;

		WMA_LOGE("RA: " MAC_ADDRESS_STR " TA: " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(wh->i_addr1),
			MAC_ADDR_ARRAY(wh->i_addr2));

		WMA_LOGE("TO_DS: %u, FROM_DS: %u",
			wh->i_fc[1] & IEEE80211_FC1_DIR_TODS,
			wh->i_fc[1] & IEEE80211_FC1_DIR_FROMDS);

		to_from_ds = wh->i_fc[1] & IEEE80211_FC1_DIR_DSTODS;

		switch (to_from_ds) {
		case IEEE80211_NO_DS:
			WMA_LOGE("BSSID: " MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(wh->i_addr3));
			break;
		case IEEE80211_TO_DS:
			WMA_LOGE("DA: " MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(wh->i_addr3));
			break;
		case IEEE80211_FROM_DS:
			WMA_LOGE("SA: " MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(wh->i_addr3));
			break;
		case IEEE80211_DS_TO_DS:
			if (buf_len >= sizeof(struct ieee80211_frame_addr4))
				WMA_LOGE("DA: " MAC_ADDRESS_STR " SA: "
					MAC_ADDRESS_STR,
					MAC_ADDR_ARRAY(wh->i_addr3),
					MAC_ADDR_ARRAY(wh->i_addr4));
			break;
		}

		seq_num = (((*(uint16_t *)wh->i_seq) &
				IEEE80211_SEQ_SEQ_MASK) >>
				IEEE80211_SEQ_SEQ_SHIFT);
		frag_num = (((*(uint16_t *)wh->i_seq) &
				IEEE80211_SEQ_FRAG_MASK) >>
				IEEE80211_SEQ_FRAG_SHIFT);

		WMA_LOGE("SEQ_NUM: %u, FRAG_NUM: %u",
				seq_num, frag_num);
	} else {
		WMA_LOGE("Insufficient buffer length for mgmt. packet");
	}
}

/**
 * wma_acquire_wakelock() - conditionally aquires a wakelock base on wake reason
 * @wma: the wma handle with the wakelocks to aquire
 * @wake_reason: wow wakeup reason
 *
 * Return: None
 */
static void wma_acquire_wow_wakelock(t_wma_handle *wma, int wake_reason)
{
	qdf_wake_lock_t *wl;
	uint32_t ms;

	switch (wake_reason) {
	case WOW_REASON_AUTH_REQ_RECV:
		wl = &wma->wow_auth_req_wl;
		ms = WMA_AUTH_REQ_RECV_WAKE_LOCK_TIMEOUT;
		break;
	case WOW_REASON_ASSOC_REQ_RECV:
		wl = &wma->wow_assoc_req_wl;
		ms = WMA_ASSOC_REQ_RECV_WAKE_LOCK_DURATION;
		break;
	case WOW_REASON_DEAUTH_RECVD:
		wl = &wma->wow_deauth_rec_wl;
		ms = WMA_DEAUTH_RECV_WAKE_LOCK_DURATION;
		break;
	case WOW_REASON_DISASSOC_RECVD:
		wl = &wma->wow_disassoc_rec_wl;
		ms = WMA_DISASSOC_RECV_WAKE_LOCK_DURATION;
		break;
	case WOW_REASON_AP_ASSOC_LOST:
		wl = &wma->wow_ap_assoc_lost_wl;
		ms = WMA_BMISS_EVENT_WAKE_LOCK_DURATION;
		break;
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case WOW_REASON_HOST_AUTO_SHUTDOWN:
		wl = &wma->wow_auto_shutdown_wl;
		ms = WMA_AUTO_SHUTDOWN_WAKE_LOCK_DURATION;
		break;
#endif
	case WOW_REASON_ROAM_HO:
		wl = &wma->roam_ho_wl;
		ms = WMA_ROAM_HO_WAKE_LOCK_DURATION;
		break;
	default:
		return;
	}

	WMA_LOGA("Holding %d msec wake_lock", ms);
	cds_host_diag_log_work(wl, ms, WIFI_POWER_EVENT_WAKELOCK_WOW);
	qdf_wake_lock_timeout_acquire(wl, ms);
}

/**
 * wma_wake_reason_ap_assoc_lost() - WOW_REASON_AP_ASSOC_LOST handler
 * @wma: Pointer to wma handle
 * @event: pointer to piggybacked WMI_ROAM_EVENTID_param_tlvs buffer
 * @len: length of the event buffer
 *
 * Return: Errno
 */
static int
wma_wake_reason_ap_assoc_lost(t_wma_handle *wma, void *event, uint32_t len)
{
	WMI_ROAM_EVENTID_param_tlvs *event_param;
	wmi_roam_event_fixed_param *roam_event;

	event_param = event;
	if (!event_param) {
		WMA_LOGE("AP Assoc Lost event data is null");
		return -EINVAL;
	}

	roam_event = event_param->fixed_param;
	WMA_LOGA(FL("Beacon miss indication on vdev %d"), roam_event->vdev_id);

	wma_beacon_miss_handler(wma, roam_event->vdev_id, roam_event->rssi);

	return 0;
}

static const char *wma_vdev_type_str(uint32_t vdev_type)
{
	switch (vdev_type) {
	case WMI_VDEV_TYPE_AP:
		return "AP";
	case WMI_VDEV_TYPE_STA:
		return "STA";
	case WMI_VDEV_TYPE_IBSS:
		return "IBSS";
	case WMI_VDEV_TYPE_MONITOR:
		return "MONITOR";
	case WMI_VDEV_TYPE_NAN:
		return "NAN";
	case WMI_VDEV_TYPE_OCB:
		return "OCB";
	case WMI_VDEV_TYPE_NDI:
		return "NDI";
	default:
		return "unknown";
	}
}

static int wma_wake_event_packet(
	t_wma_handle *wma,
	WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *event_param,
	uint32_t length)
{
	WOW_EVENT_INFO_fixed_param *wake_info;
	struct wma_txrx_node *vdev;
	uint8_t *packet;
	uint32_t packet_len;

	if (event_param->num_wow_packet_buffer <= 4) {
		WMA_LOGE("Invalid wow packet buffer from firmware %u",
			 event_param->num_wow_packet_buffer);
		return -EINVAL;
	}
	/* first 4 bytes are the length, followed by the buffer */
	packet_len = *(uint32_t *)event_param->wow_packet_buffer;
	packet = event_param->wow_packet_buffer + 4;

	if (!packet_len) {
		WMA_LOGE("Wake event packet is empty");
		return 0;
	}

	if (packet_len > (event_param->num_wow_packet_buffer - 4)) {
		WMA_LOGE("Invalid packet_len from firmware, packet_len: %u, num_wow_packet_buffer: %u",
			 packet_len,
			 event_param->num_wow_packet_buffer);
		return -EINVAL;
	}

	wake_info = event_param->fixed_param;

	switch (wake_info->wake_reason) {
	case WOW_REASON_AUTH_REQ_RECV:
	case WOW_REASON_ASSOC_REQ_RECV:
	case WOW_REASON_DEAUTH_RECVD:
	case WOW_REASON_DISASSOC_RECVD:
	case WOW_REASON_ASSOC_RES_RECV:
	case WOW_REASON_REASSOC_REQ_RECV:
	case WOW_REASON_REASSOC_RES_RECV:
	case WOW_REASON_BEACON_RECV:
	case WOW_REASON_ACTION_FRAME_RECV:
		/* management frame case */
		wma_wow_dump_mgmt_buffer(packet, packet_len);
		break;

	case WOW_REASON_BPF_ALLOW:
	case WOW_REASON_PATTERN_MATCH_FOUND:
	case WOW_REASON_RA_MATCH:
	case WOW_REASON_RECV_MAGIC_PATTERN:
		WMA_LOGD("Wake event packet:");
		qdf_trace_hex_dump(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
				   packet, packet_len);

		vdev = &wma->interfaces[wake_info->vdev_id];
		wma_wow_parse_data_pkt(&vdev->wow_stats, packet, packet_len);
		break;

	default:
		WMA_LOGE("Wake reason %s(%u) is not a packet event",
			 wma_wow_wake_reason_str(wake_info->wake_reason),
			 wake_info->wake_reason);
		return -EINVAL;
	}

	return 0;
}

static int wma_wake_event_no_payload(
	t_wma_handle *wma,
	WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *event_param,
	uint32_t length)
{
	WOW_EVENT_INFO_fixed_param *wake_info = event_param->fixed_param;

	switch (wake_info->wake_reason) {
	case WOW_REASON_HOST_AUTO_SHUTDOWN:
		return wma_wake_reason_auto_shutdown();

	case WOW_REASON_NLOD:
		return wma_wake_reason_nlod(wma, wake_info->vdev_id);

	default:
		return 0;
	}
}

static int wma_wake_event_piggybacked(
	t_wma_handle *wma,
	WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *event_param,
	uint32_t length)
{
	int errno = 0;
	void *pb_event;
	uint32_t pb_event_len;
	uint32_t wake_reason;
	uint32_t event_id;

	/*
	 * There are "normal" cases where a wake reason that usually contains a
	 * piggybacked event is empty. In these cases we just want to wake up,
	 * and no action is needed. Bail out now if that is the case.
	 */
	if (!event_param->wow_packet_buffer ||
	    event_param->num_wow_packet_buffer <= 4) {
		WMA_LOGE("Invalid wow packet buffer from firmware %u",
			 event_param->num_wow_packet_buffer);
		return 0;
	}

	wake_reason = event_param->fixed_param->wake_reason;

	/* parse piggybacked event from param buffer */
	{
		int ret_code;
		uint8_t *pb_event_buf;
		uint32_t tag;

		/* first 4 bytes are the length, followed by the buffer */
		pb_event_len = *(uint32_t *)event_param->wow_packet_buffer;
		if (pb_event_len > (event_param->num_wow_packet_buffer - 4)) {
			WMA_LOGE("Invalid pb_event_len from firmware, pb_event_len: %u, num_wow_packet_buffer: %u",
				 pb_event_len,
				 event_param->num_wow_packet_buffer);
			return -EINVAL;
		}
		pb_event_buf = event_param->wow_packet_buffer + 4;

		WMA_LOGD("piggybacked event buffer:");
		qdf_trace_hex_dump(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
				   pb_event_buf, pb_event_len);

		tag = WMITLV_GET_TLVTAG(WMITLV_GET_HDR(pb_event_buf));
		event_id = wow_get_wmi_eventid(wake_reason, tag);
		if (!event_id) {
			WMA_LOGE(FL("Unable to find Event Id"));
			return -EINVAL;
		}

		ret_code = wmitlv_check_and_pad_event_tlvs(wma, pb_event_buf,
							   pb_event_len,
							   event_id, &pb_event);
		if (ret_code) {
			WMA_LOGE(FL("Bad TLVs; len:%d, event_id:%d, status:%d"),
				 pb_event_len, event_id, ret_code);
			return -EINVAL;
		}
	}

	switch (wake_reason) {
	case WOW_REASON_AP_ASSOC_LOST:
		errno = wma_wake_reason_ap_assoc_lost(wma, pb_event,
						      pb_event_len);
		break;

#ifdef FEATURE_WLAN_SCAN_PNO
	case WOW_REASON_NLO_SCAN_COMPLETE:
		errno = target_if_nlo_complete_handler(wma, pb_event,
						       pb_event_len);
		break;
#endif /* FEATURE_WLAN_SCAN_PNO */

	case WOW_REASON_CSA_EVENT:
		errno = wma_csa_offload_handler(wma, pb_event, pb_event_len);
		break;

	/*
	 * WOW_REASON_LOW_RSSI is used for following roaming events -
	 * WMI_ROAM_REASON_BETTER_AP, WMI_ROAM_REASON_BMISS,
	 * WMI_ROAM_REASON_SUITABLE_AP will be handled by
	 * wma_roam_event_callback().
	 * WOW_REASON_ROAM_HO is associated with
	 * WMI_ROAM_REASON_HO_FAILED event and it will be handled by
	 * wma_roam_event_callback().
	 */
	case WOW_REASON_LOW_RSSI:
	case WOW_REASON_ROAM_HO:
		wlan_roam_debug_log(event_param->fixed_param->vdev_id,
				    DEBUG_WOW_ROAM_EVENT,
				    DEBUG_INVALID_PEER_ID,
				    NULL, NULL, wake_reason,
				    pb_event_len);
		if (pb_event_len > 0) {
			errno = wma_roam_event_callback(wma, pb_event,
							pb_event_len);
		} else {
			/*
			 * No wow_packet_buffer means a better AP beacon
			 * will follow in a later event.
			 */
			WMA_LOGD("Host woken up because of better AP beacon");
		}
		break;

	case WOW_REASON_CLIENT_KICKOUT_EVENT:
		errno = wma_peer_sta_kickout_event_handler(wma, pb_event,
							   pb_event_len);
		break;

#ifdef FEATURE_WLAN_EXTSCAN
	case WOW_REASON_EXTSCAN:
		errno = wma_extscan_wow_event_callback(wma, pb_event,
						       pb_event_len);
		break;
#endif

	case WOW_REASON_RSSI_BREACH_EVENT:
		errno = wma_rssi_breached_event_handler(wma, pb_event,
							pb_event_len);
		break;

	case WOW_REASON_NAN_EVENT:
		errno = wma_nan_rsp_event_handler(wma, pb_event, pb_event_len);
		break;

	case WOW_REASON_NAN_DATA:
		errno = wma_ndp_wow_event_callback(wma, pb_event, pb_event_len,
						   event_id);
		break;

#ifdef FEATURE_WLAN_TDLS
	case WOW_REASON_TDLS_CONN_TRACKER_EVENT:
		errno = wma_tdls_event_handler(wma, pb_event, pb_event_len);
		break;
#endif

	case WOW_REASON_TIMER_INTR_RECV:
		/*
		 * Right now firmware is not returning any cookie host has
		 * programmed. So do not check for cookie.
		 */
		WMA_LOGE("WOW_REASON_TIMER_INTR_RECV received, indicating key exchange did not finish. Initiate disconnect");
		errno = wma_peer_sta_kickout_event_handler(wma, pb_event,
							   pb_event_len);
		break;

	default:
		WMA_LOGE("Wake reason %s(%u) is not a piggybacked event",
			 wma_wow_wake_reason_str(wake_reason), wake_reason);
		errno = -EINVAL;
		break;
	}

	wmitlv_free_allocated_event_tlvs(event_id, &pb_event);

	return errno;
}

static void wma_wake_event_log_reason(t_wma_handle *wma,
				      WOW_EVENT_INFO_fixed_param *wake_info)
{
	struct wma_txrx_node *vdev;

	/* "Unspecified" means APPS triggered wake, else firmware triggered */
	if (wake_info->wake_reason != WOW_REASON_UNSPECIFIED) {
		vdev = &wma->interfaces[wake_info->vdev_id];
		WMA_LOGA("WLAN triggered wakeup: %s (%d), vdev: %d (%s)",
			 wma_wow_wake_reason_str(wake_info->wake_reason),
			 wake_info->wake_reason,
			 wake_info->vdev_id,
			 wma_vdev_type_str(vdev->type));
	} else if (!wmi_get_runtime_pm_inprogress(wma->wmi_handle)) {
		WMA_LOGA("Non-WLAN triggered wakeup: %s (%d)",
			 wma_wow_wake_reason_str(wake_info->wake_reason),
			 wake_info->wake_reason);
	}

	qdf_wow_wakeup_host_event(wake_info->wake_reason);
	qdf_wma_wow_wakeup_stats_event(wma);
}

/**
 * wma_wow_wakeup_host_event() - wakeup host event handler
 * @handle: wma handle
 * @event: event data
 * @len: buffer length
 *
 * Handler to catch wow wakeup host event. This event will have
 * reason why the firmware has woken the host.
 *
 * Return: Errno
 */
int wma_wow_wakeup_host_event(void *handle, uint8_t *event, uint32_t len)
{
	int errno;
	t_wma_handle *wma = handle;
	WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *event_param;
	WOW_EVENT_INFO_fixed_param *wake_info;

	event_param = (WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *)event;
	if (!event_param) {
		WMA_LOGE("Wake event data is null");
		return -EINVAL;
	}

	wake_info = event_param->fixed_param;

	if (wake_info->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: received invalid vdev_id %d",
			 __func__, wake_info->vdev_id);
		return -EINVAL;
	}

	wma_wake_event_log_reason(wma, wake_info);

	pmo_ucfg_psoc_wakeup_host_event_received(wma->psoc);

	wma_print_wow_stats(wma, wake_info);

	/* split based on payload type */
	if (is_piggybacked_event(wake_info->wake_reason))
		errno = wma_wake_event_piggybacked(wma, event_param, len);
	else if (event_param->wow_packet_buffer)
		errno = wma_wake_event_packet(wma, event_param, len);
	else
		errno = wma_wake_event_no_payload(wma, event_param, len);

	wma_inc_wow_stats(wma, wake_info);
	wma_print_wow_stats(wma, wake_info);

	wma_acquire_wow_wakelock(wma, wake_info->wake_reason);

	return errno;
}

#ifdef FEATURE_WLAN_D0WOW
/**
 * wma_d0_wow_disable_ack_event() - wakeup host event handler
 * @handle: wma handle
 * @event: event data
 * @len: buffer length
 *
 * Handler to catch D0-WOW disable ACK event.  This event will have
 * reason why the firmware has woken the host.
 * This is for backward compatible with cld2.0.
 *
 * Return: 0 for success or error
 */
int wma_d0_wow_disable_ack_event(void *handle, uint8_t *event, uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle)handle;
	WMI_D0_WOW_DISABLE_ACK_EVENTID_param_tlvs *param_buf;
	wmi_d0_wow_disable_ack_event_fixed_param *resp_data;

	param_buf = (WMI_D0_WOW_DISABLE_ACK_EVENTID_param_tlvs *)event;
	if (!param_buf) {
		WMA_LOGE("Invalid D0-WOW disable ACK event buffer!");
		return -EINVAL;
	}

	resp_data = param_buf->fixed_param;

	pmo_ucfg_psoc_wakeup_host_event_received(wma->psoc);

	WMA_LOGD("Received D0-WOW disable ACK");

	return 0;
}
#else
int wma_d0_wow_disable_ack_event(void *handle, uint8_t *event, uint32_t len)
{
	return 0;
}
#endif

/**
 * wma_pdev_resume_event_handler() - PDEV resume event handler
 * @handle: wma handle
 * @event: event data
 * @len: buffer length
 *
 * Return: 0 for success or error
 */
int wma_pdev_resume_event_handler(void *handle, uint8_t *event, uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;

	WMA_LOGA("Received PDEV resume event");

	pmo_ucfg_psoc_wakeup_host_event_received(wma->psoc);

	return 0;
}

/**
 * wma_wow_enter() - store enable/disable status for pattern
 * @wma: wma handle
 * @info: wow parameters
 *
 * Records pattern enable/disable status locally. This choice will
 * take effect when the driver enter into suspend state.
 *
 * Return: QDF status
 */
QDF_STATUS wma_wow_enter(tp_wma_handle wma, tpSirHalWowlEnterParams info)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wow_exit() - clear all wma states
 * @wma: wma handle
 * @info: wow params
 *
 * Return: QDF status
 */
QDF_STATUS wma_wow_exit(tp_wma_handle wma, tpSirHalWowlExitParams info)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_del_ts_req() - send DELTS request to fw
 * @wma: wma handle
 * @msg: delts params
 *
 * Return: none
 */
void wma_del_ts_req(tp_wma_handle wma, tDelTsParams *msg)
{
	if (wmi_unified_del_ts_cmd(wma->wmi_handle,
				 msg->sessionId,
				 TID_TO_WME_AC(msg->userPrio))) {
		WMA_LOGP("%s: Failed to send vdev DELTS command", __func__);
	}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (msg->setRICparams == true)
		wma_set_ric_req(wma, msg, false);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	qdf_mem_free(msg);
}

/**
 * wma_aggr_qos_req() - send aggr qos request to fw
 * @wma: handle to wma
 * @pAggrQosRspMsg - combined struct for all ADD_TS requests.
 *
 * A function to handle WMA_AGGR_QOS_REQ. This will send out
 * ADD_TS requestes to firmware in loop for all the ACs with
 * active flow.
 *
 * Return: none
 */
void wma_aggr_qos_req(tp_wma_handle wma,
		      tAggrAddTsParams *pAggrQosRspMsg)
{
	wmi_unified_aggr_qos_cmd(wma->wmi_handle,
			   (struct aggr_add_ts_param *)pAggrQosRspMsg);
	/* send reponse to upper layers from here only. */
	wma_send_msg_high_priority(wma, WMA_AGGR_QOS_RSP, pAggrQosRspMsg, 0);
}

#ifdef FEATURE_WLAN_ESE
/**
 * wma_set_tsm_interval() - Set TSM interval
 * @req: pointer to ADDTS request
 *
 * Return: QDF_STATUS_E_FAILURE or QDF_STATUS_SUCCESS
 */
static QDF_STATUS wma_set_tsm_interval(tAddTsParams *req)
{
	/*
	 * msmt_interval is in unit called TU (1 TU = 1024 us)
	 * max value of msmt_interval cannot make resulting
	 * interval_milliseconds overflow 32 bit
	 *
	 */
	uint32_t interval_milliseconds;
	struct cdp_pdev *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	interval_milliseconds = (req->tsm_interval * 1024) / 1000;

	cdp_tx_set_compute_interval(cds_get_context(QDF_MODULE_ID_SOC),
			pdev,
			interval_milliseconds);
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS wma_set_tsm_interval(tAddTsParams *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_ESE */

/**
 * wma_add_ts_req() - send ADDTS request to fw
 * @wma: wma handle
 * @msg: ADDTS params
 *
 * Return: none
 */
void wma_add_ts_req(tp_wma_handle wma, tAddTsParams *msg)
{
	struct add_ts_param cmd = {0};

	msg->status = QDF_STATUS_SUCCESS;
	if (wma_set_tsm_interval(msg) == QDF_STATUS_SUCCESS) {

		cmd.sme_session_id = msg->sme_session_id;
		cmd.tspec.tsinfo.traffic.userPrio =
			TID_TO_WME_AC(msg->tspec.tsinfo.traffic.userPrio);
		cmd.tspec.mediumTime = msg->tspec.mediumTime;
		if (wmi_unified_add_ts_cmd(wma->wmi_handle, &cmd))
			msg->status = QDF_STATUS_E_FAILURE;

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (msg->setRICparams == true)
			wma_set_ric_req(wma, msg, true);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

	}
	wma_send_msg_high_priority(wma, WMA_ADD_TS_RSP, msg, 0);
}

#ifdef FEATURE_WLAN_ESE

#define TSM_DELAY_HISTROGRAM_BINS 4
/**
 * wma_process_tsm_stats_req() - process tsm stats request
 * @wma_handler - handle to wma
 * @pTsmStatsMsg - TSM stats struct that needs to be populated and
 *         passed in message.
 *
 * A parallel function to WMA_ProcessTsmStatsReq for pronto. This
 * function fetches stats from data path APIs and post
 * WMA_TSM_STATS_RSP msg back to LIM.
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_tsm_stats_req(tp_wma_handle wma_handler,
				     void *pTsmStatsMsg)
{
	uint8_t counter;
	uint32_t queue_delay_microsec = 0;
	uint32_t tx_delay_microsec = 0;
	uint16_t packet_count = 0;
	uint16_t packet_loss_count = 0;
	tpAniTrafStrmMetrics pTsmMetric = NULL;
	tpAniGetTsmStatsReq pStats = (tpAniGetTsmStatsReq) pTsmStatsMsg;
	tpAniGetTsmStatsRsp pTsmRspParams = NULL;
	int tid = pStats->tid;
	/*
	 * The number of histrogram bin report by data path api are different
	 * than required by TSM, hence different (6) size array used
	 */
	uint16_t bin_values[QCA_TX_DELAY_HIST_REPORT_BINS] = { 0, };
	struct cdp_pdev *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		qdf_mem_free(pTsmStatsMsg);
		return QDF_STATUS_E_INVAL;
	}

	/* get required values from data path APIs */
	cdp_tx_delay(soc,
		pdev,
		&queue_delay_microsec,
		&tx_delay_microsec, tid);
	cdp_tx_delay_hist(soc,
		pdev,
		bin_values, tid);
	cdp_tx_packet_count(soc,
		pdev,
		&packet_count,
		&packet_loss_count, tid);

	pTsmRspParams = qdf_mem_malloc(sizeof(*pTsmRspParams));
	if (NULL == pTsmRspParams) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "%s: QDF MEM Alloc Failure", __func__);
		QDF_ASSERT(0);
		qdf_mem_free(pTsmStatsMsg);
		return QDF_STATUS_E_NOMEM;
	}
	pTsmRspParams->staId = pStats->staId;
	pTsmRspParams->rc = eSIR_FAILURE;
	pTsmRspParams->tsmStatsReq = pStats;
	pTsmMetric = &pTsmRspParams->tsmMetrics;
	/* populate pTsmMetric */
	pTsmMetric->UplinkPktQueueDly = queue_delay_microsec;
	/* store only required number of bin values */
	for (counter = 0; counter < TSM_DELAY_HISTROGRAM_BINS; counter++) {
		pTsmMetric->UplinkPktQueueDlyHist[counter] =
			bin_values[counter];
	}
	pTsmMetric->UplinkPktTxDly = tx_delay_microsec;
	pTsmMetric->UplinkPktLoss = packet_loss_count;
	pTsmMetric->UplinkPktCount = packet_count;

	/*
	 * No need to populate roaming delay and roaming count as they are
	 * being populated just before sending IAPP frame out
	 */
	/* post this message to LIM/PE */
	wma_send_msg(wma_handler, WMA_TSM_STATS_RSP, (void *)pTsmRspParams, 0);
	return QDF_STATUS_SUCCESS;
}

#endif /* FEATURE_WLAN_ESE */

/**
 * wma_process_mcbc_set_filter_req() - process mcbc set filter request
 * @wma_handle: wma handle
 * @mcbc_param: mcbc params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_mcbc_set_filter_req(tp_wma_handle wma_handle,
					   tSirRcvFltMcAddrList *mcbc_param)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_cesium_enable_ind() - enables cesium functionality in target
 * @wma: wma handle
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_cesium_enable_ind(tp_wma_handle wma)
{
	QDF_STATUS ret;
	int32_t vdev_id;

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not enable cesium",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Send enable cesium command to target */
	WMA_LOGE("Enable cesium in target for vdevId %d ", vdev_id);
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				 WMI_VDEV_PARAM_ENABLE_RMC, 1);
	if (ret) {
		WMA_LOGE("Enable cesium failed for vdevId %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_get_peer_info_req() - sends get peer info cmd to target
 * @wma: wma handle
 * @preq: get peer info request
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_get_peer_info_req
	(tp_wma_handle wma, tSirIbssGetPeerInfoReqParams *pReq)
{
	int32_t ret;
	uint8_t *p;
	uint16_t len;
	wmi_buf_t buf;
	int32_t vdev_id;
	struct cdp_pdev *pdev;
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
	uint8_t *peer_mac_raw;
	wmi_peer_info_req_cmd_fixed_param *p_get_peer_info_cmd;
	uint8_t bcast_mac[IEEE80211_ADDR_LEN] = { 0xff, 0xff, 0xff,
						  0xff, 0xff, 0xff };

	if (NULL == soc) {
		WMA_LOGE("%s: SOC context is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not get peer info",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev context", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (0xFF == pReq->staIdx) {
		/*get info for all peers */
		qdf_mem_copy(peer_mac, bcast_mac, IEEE80211_ADDR_LEN);
	} else {
		/*get info for a single peer */
		peer = cdp_peer_find_by_local_id(soc,
				pdev, pReq->staIdx);
		if (!peer) {
			WMA_LOGE("%s: Failed to get peer handle using peer id %d",
				__func__, pReq->staIdx);
			return QDF_STATUS_E_FAILURE;
		}
		peer_mac_raw = cdp_peer_get_peer_mac_addr(soc, peer);
		if (peer_mac_raw == NULL) {
			WMA_LOGE("peer_mac_raw is NULL");
			return QDF_STATUS_E_FAILURE;
		}

		WMA_LOGE("%s: staIdx %d peer mac: 0x%2x:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x",
			__func__, pReq->staIdx, peer_mac_raw[0],
			peer_mac_raw[1], peer_mac_raw[2],
			peer_mac_raw[3], peer_mac_raw[4],
			peer_mac_raw[5]);
		qdf_mem_copy(peer_mac, peer_mac_raw, IEEE80211_ADDR_LEN);
	}

	len = sizeof(wmi_peer_info_req_cmd_fixed_param);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s %d: No WMI resource!", __func__, __LINE__);
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(p, len);
	p_get_peer_info_cmd = (wmi_peer_info_req_cmd_fixed_param *) p;

	WMITLV_SET_HDR(&p_get_peer_info_cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_info_req_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_info_req_cmd_fixed_param));

	p_get_peer_info_cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_mac,
				   &p_get_peer_info_cmd->peer_mac_address);

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_PEER_INFO_REQ_CMDID);
	if (ret != QDF_STATUS_SUCCESS)
		wmi_buf_free(buf);

	WMA_LOGE("IBSS get peer info cmd sent len: %d, vdev %d command id: %d, status: %d",
		len, vdev_id, WMI_PEER_INFO_REQ_CMDID, ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_tx_fail_monitor_ind() - sends tx fail monitor cmd to target
 * @wma: wma handle
 * @pReq: tx fail monitor command params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_tx_fail_monitor_ind(tp_wma_handle wma,
					tAniTXFailMonitorInd *pReq)
{
	QDF_STATUS ret;
	int32_t vdev_id;

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not send fast tx fail monitor indication message to target",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Send enable cesium command to target */
	WMA_LOGE("send fast tx fail monitor ind cmd target for vdevId %d val %d",
		vdev_id, pReq->tx_fail_count);

	if (pReq->tx_fail_count == 0)
		wma->hddTxFailCb = NULL;
	else
		wma->hddTxFailCb = pReq->txFailIndCallback;
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				 WMI_VDEV_PARAM_SET_IBSS_TX_FAIL_CNT_THR,
				 pReq->tx_fail_count);
	if (ret) {
		WMA_LOGE("tx fail monitor failed for vdevId %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_rmc_enable_ind() - enables RMC functionality in target
 * @wma: wma handle
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_rmc_enable_ind(tp_wma_handle wma)
{
	int ret;
	uint8_t *p;
	uint16_t len;
	wmi_buf_t buf;
	int32_t vdev_id;
	wmi_rmc_set_mode_cmd_fixed_param *p_rmc_enable_cmd;

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not enable RMC",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	len = sizeof(wmi_rmc_set_mode_cmd_fixed_param);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s %d: No WMI resource!", __func__, __LINE__);
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(p, len);
	p_rmc_enable_cmd = (wmi_rmc_set_mode_cmd_fixed_param *) p;

	WMITLV_SET_HDR(&p_rmc_enable_cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_rmc_set_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_rmc_set_mode_cmd_fixed_param));

	p_rmc_enable_cmd->vdev_id = vdev_id;
	p_rmc_enable_cmd->enable_rmc = WMI_RMC_MODE_ENABLED;

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_RMC_SET_MODE_CMDID);
	if (ret != QDF_STATUS_SUCCESS)
		wmi_buf_free(buf);

	WMA_LOGE("Enable RMC cmd sent len: %d, vdev %d command id: %d, status: %d",
		 len, vdev_id, WMI_RMC_SET_MODE_CMDID, ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_rmc_disable_ind() - disables rmc functionality in target
 * @wma: wma handle
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_rmc_disable_ind(tp_wma_handle wma)
{
	int ret;
	uint8_t *p;
	uint16_t len;
	wmi_buf_t buf;
	int32_t vdev_id;
	wmi_rmc_set_mode_cmd_fixed_param *p_rmc_disable_cmd;

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not disable RMC",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	len = sizeof(wmi_rmc_set_mode_cmd_fixed_param);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s %d: No WMI resource!", __func__, __LINE__);
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(p, len);
	p_rmc_disable_cmd = (wmi_rmc_set_mode_cmd_fixed_param *) p;

	WMITLV_SET_HDR(&p_rmc_disable_cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_rmc_set_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_rmc_set_mode_cmd_fixed_param));

	p_rmc_disable_cmd->vdev_id = vdev_id;
	p_rmc_disable_cmd->enable_rmc = WMI_RMC_MODE_DISABLED;

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_RMC_SET_MODE_CMDID);
	if (ret != QDF_STATUS_SUCCESS)
		wmi_buf_free(buf);

	WMA_LOGE("Disable RMC cmd sent len: %d, vdev %d command id: %d, status: %d",
		 len, vdev_id, WMI_RMC_SET_MODE_CMDID, ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_rmc_action_period_ind() - sends RMC action period to target
 * @wma: wma handle
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_rmc_action_period_ind(tp_wma_handle wma)
{
	int ret;
	uint8_t *p;
	uint16_t len;
	uint32_t periodicity_msec;
	wmi_buf_t buf;
	int32_t vdev_id;
	wmi_rmc_set_action_period_cmd_fixed_param *p_rmc_cmd;
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (NULL == mac) {
		WMA_LOGE("%s: MAC mac does not exist", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not send RMC action period to target",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	len = sizeof(wmi_rmc_set_action_period_cmd_fixed_param);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s %d: No WMI resource!", __func__, __LINE__);
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(p, len);
	p_rmc_cmd = (wmi_rmc_set_action_period_cmd_fixed_param *) p;

	WMITLV_SET_HDR(&p_rmc_cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_rmc_set_action_period_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_rmc_set_action_period_cmd_fixed_param));

	if (wlan_cfg_get_int(mac, WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY,
			     &periodicity_msec) != eSIR_SUCCESS) {
		WMA_LOGE("Failed to get value for RMC action period using default");
		periodicity_msec = WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY_STADEF;
	}

	p_rmc_cmd->vdev_id = vdev_id;
	p_rmc_cmd->periodicity_msec = periodicity_msec;

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_RMC_SET_ACTION_PERIOD_CMDID);
	if (ret != QDF_STATUS_SUCCESS)
		wmi_buf_free(buf);

	WMA_LOGE("RMC action period %d cmd sent len: %d, vdev %d command id: %d, status: %d",
		periodicity_msec, len, vdev_id, WMI_RMC_SET_ACTION_PERIOD_CMDID,
		ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_add_periodic_tx_ptrn_ind - add periodic tx ptrn
 * @handle: wma handle
 * @pAddPeriodicTxPtrnParams: tx ptrn params
 *
 * Retrun: QDF status
 */
QDF_STATUS wma_process_add_periodic_tx_ptrn_ind(WMA_HANDLE handle,
						tSirAddPeriodicTxPtrn *
						pAddPeriodicTxPtrnParams)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct periodic_tx_pattern *params_ptr;
	uint8_t vdev_id;
	QDF_STATUS status;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue fw add pattern cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	params_ptr = qdf_mem_malloc(sizeof(*params_ptr));

	if (!params_ptr) {
		WMA_LOGE(
			"%s: unable to allocate memory for periodic_tx_pattern",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (!wma_find_vdev_by_addr(wma_handle,
				   pAddPeriodicTxPtrnParams->mac_address.bytes,
				   &vdev_id)) {
		WMA_LOGE("%s: Failed to find vdev id for %pM", __func__,
			 pAddPeriodicTxPtrnParams->mac_address.bytes);
		return QDF_STATUS_E_INVAL;
	}

	params_ptr->ucPtrnId = pAddPeriodicTxPtrnParams->ucPtrnId;
	params_ptr->ucPtrnSize = pAddPeriodicTxPtrnParams->ucPtrnSize;
	params_ptr->usPtrnIntervalMs =
				pAddPeriodicTxPtrnParams->usPtrnIntervalMs;
	qdf_mem_copy(&params_ptr->mac_address,
			&pAddPeriodicTxPtrnParams->mac_address,
			sizeof(struct qdf_mac_addr));
	qdf_mem_copy(params_ptr->ucPattern,
			pAddPeriodicTxPtrnParams->ucPattern,
			params_ptr->ucPtrnSize);

	status =  wmi_unified_process_add_periodic_tx_ptrn_cmd(
			wma_handle->wmi_handle,	params_ptr, vdev_id);

	qdf_mem_free(params_ptr);
	return status;
}

/**
 * wma_process_del_periodic_tx_ptrn_ind - del periodic tx ptrn
 * @handle: wma handle
 * @pDelPeriodicTxPtrnParams: tx ptrn params
 *
 * Retrun: QDF status
 */
QDF_STATUS wma_process_del_periodic_tx_ptrn_ind(WMA_HANDLE handle,
						tSirDelPeriodicTxPtrn *
						pDelPeriodicTxPtrnParams)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint8_t vdev_id;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue Del Pattern cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_find_vdev_by_addr(wma_handle,
				   pDelPeriodicTxPtrnParams->mac_address.bytes,
				   &vdev_id)) {
		WMA_LOGE("%s: Failed to find vdev id for %pM", __func__,
			 pDelPeriodicTxPtrnParams->mac_address.bytes);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_process_del_periodic_tx_ptrn_cmd(
				wma_handle->wmi_handle, vdev_id,
				pDelPeriodicTxPtrnParams->ucPtrnId);
}

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * wma_stats_ext_req() - request ext stats from fw
 * @wma_ptr: wma handle
 * @preq: stats ext params
 *
 * Return: QDF status
 */
QDF_STATUS wma_stats_ext_req(void *wma_ptr, tpStatsExtRequest preq)
{
	tp_wma_handle wma = (tp_wma_handle) wma_ptr;
	struct stats_ext_params *params;
	size_t params_len;
	QDF_STATUS status;

	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	params_len = sizeof(*params) + preq->request_data_len;
	params = qdf_mem_malloc(params_len);

	if (params == NULL) {
		WMA_LOGE(FL("memory allocation failed"));
		return QDF_STATUS_E_NOMEM;
	}

	params->vdev_id = preq->vdev_id;
	params->request_data_len = preq->request_data_len;
	if (preq->request_data_len > 0)
		qdf_mem_copy(params->request_data, preq->request_data,
			     params->request_data_len);

	status = wmi_unified_stats_ext_req_cmd(wma->wmi_handle, params);
	qdf_mem_free(params);

	return status;
}

#endif /* WLAN_FEATURE_STATS_EXT */

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/**
 * wma_send_status_of_ext_wow() - send ext wow status to SME
 * @wma: wma handle
 * @status: status
 *
 * Return: none
 */
static void wma_send_status_of_ext_wow(tp_wma_handle wma, bool status)
{
	tSirReadyToExtWoWInd *ready_to_extwow;
	QDF_STATUS vstatus;
	struct scheduler_msg message = {0};
	uint8_t len;

	WMA_LOGD("Posting ready to suspend indication to umac");

	len = sizeof(tSirReadyToExtWoWInd);
	ready_to_extwow = (tSirReadyToExtWoWInd *) qdf_mem_malloc(len);

	if (NULL == ready_to_extwow) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return;
	}

	ready_to_extwow->mesgType = eWNI_SME_READY_TO_EXTWOW_IND;
	ready_to_extwow->mesgLen = len;
	ready_to_extwow->status = status;

	message.type = eWNI_SME_READY_TO_EXTWOW_IND;
	message.bodyptr = (void *)ready_to_extwow;
	message.bodyval = 0;

	vstatus = scheduler_post_msg(QDF_MODULE_ID_SME, &message);
	if (vstatus != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to post ready to suspend");
		qdf_mem_free(ready_to_extwow);
	}
}

/**
 * wma_enable_ext_wow() - enable ext wow in fw
 * @wma: wma handle
 * @params: ext wow params
 *
 * Return:0 for success or error code
 */
QDF_STATUS wma_enable_ext_wow(tp_wma_handle wma, tpSirExtWoWParams params)
{
	struct ext_wow_params wow_params = {0};
	QDF_STATUS status;

	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	wow_params.vdev_id = params->vdev_id;
	wow_params.type = (enum wmi_ext_wow_type) params->type;
	wow_params.wakeup_pin_num = params->wakeup_pin_num;

	status = wmi_unified_enable_ext_wow_cmd(wma->wmi_handle,
				&wow_params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma_send_status_of_ext_wow(wma, true);
	return status;

}

/**
 * wma_set_app_type1_params_in_fw() - set app type1 params in fw
 * @wma: wma handle
 * @appType1Params: app type1 params
 *
 * Return: QDF status
 */
int wma_set_app_type1_params_in_fw(tp_wma_handle wma,
				   tpSirAppType1Params appType1Params)
{
	int ret;

	ret = wmi_unified_app_type1_params_in_fw_cmd(wma->wmi_handle,
				   (struct app_type1_params *)appType1Params);
	if (ret) {
		WMA_LOGE("%s: Failed to set APP TYPE1 PARAMS", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_app_type2_params_in_fw() - set app type2 params in fw
 * @wma: wma handle
 * @appType2Params: app type2 params
 *
 * Return: QDF status
 */
QDF_STATUS wma_set_app_type2_params_in_fw(tp_wma_handle wma,
					  tpSirAppType2Params appType2Params)
{
	struct app_type2_params params = {0};

	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	params.vdev_id = appType2Params->vdev_id;
	params.rc4_key_len = appType2Params->rc4_key_len;
	qdf_mem_copy(params.rc4_key, appType2Params->rc4_key, 16);
	params.ip_id = appType2Params->ip_id;
	params.ip_device_ip = appType2Params->ip_device_ip;
	params.ip_server_ip = appType2Params->ip_server_ip;
	params.tcp_src_port = appType2Params->tcp_src_port;
	params.tcp_dst_port = appType2Params->tcp_dst_port;
	params.tcp_seq = appType2Params->tcp_seq;
	params.tcp_ack_seq = appType2Params->tcp_ack_seq;
	params.keepalive_init = appType2Params->keepalive_init;
	params.keepalive_min = appType2Params->keepalive_min;
	params.keepalive_max = appType2Params->keepalive_max;
	params.keepalive_inc = appType2Params->keepalive_inc;
	params.tcp_tx_timeout_val = appType2Params->tcp_tx_timeout_val;
	params.tcp_rx_timeout_val = appType2Params->tcp_rx_timeout_val;
	qdf_mem_copy(&params.gateway_mac, &appType2Params->gateway_mac,
			sizeof(struct qdf_mac_addr));

	return wmi_unified_set_app_type2_params_in_fw_cmd(wma->wmi_handle,
							&params);
}
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/**
 * wma_auto_shutdown_event_handler() - process auto shutdown timer trigger
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_auto_shutdown_event_handler(void *handle, uint8_t *event,
				    uint32_t len)
{
	wmi_host_auto_shutdown_event_fixed_param *wmi_auto_sh_evt;
	WMI_HOST_AUTO_SHUTDOWN_EVENTID_param_tlvs *param_buf =
		(WMI_HOST_AUTO_SHUTDOWN_EVENTID_param_tlvs *)
		event;

	if (!param_buf || !param_buf->fixed_param) {
		WMA_LOGE("%s:%d: Invalid Auto shutdown timer evt", __func__,
			 __LINE__);
		return -EINVAL;
	}

	wmi_auto_sh_evt = param_buf->fixed_param;

	if (wmi_auto_sh_evt->shutdown_reason
	    != WMI_HOST_AUTO_SHUTDOWN_REASON_TIMER_EXPIRY) {
		WMA_LOGE("%s:%d: Invalid Auto shutdown timer evt", __func__,
			 __LINE__);
		return -EINVAL;
	}

	WMA_LOGD("%s:%d: Auto Shutdown Evt: %d", __func__, __LINE__,
		 wmi_auto_sh_evt->shutdown_reason);
	return wma_post_auto_shutdown_msg();
}

/**
 * wma_set_auto_shutdown_timer_req() - sets auto shutdown timer in firmware
 * @wma: wma handle
 * @auto_sh_cmd: auto shutdown timer value
 *
 * Return: QDF status
 */
QDF_STATUS wma_set_auto_shutdown_timer_req(tp_wma_handle wma_handle,
						  tSirAutoShutdownCmdParams *
						  auto_sh_cmd)
{
	if (auto_sh_cmd == NULL) {
		WMA_LOGE("%s : Invalid Autoshutdown cfg cmd", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_set_auto_shutdown_timer_cmd(wma_handle->wmi_handle,
					auto_sh_cmd->timer_val);
}
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */

#ifdef WLAN_FEATURE_NAN
/**
 * wma_nan_req() - to send nan request to target
 * @wma: wma_handle
 * @nan_req: request data which will be non-null
 *
 * Return: QDF status
 */
QDF_STATUS wma_nan_req(void *wma_ptr, tpNanRequest nan_req)
{
	tp_wma_handle wma_handle = (tp_wma_handle) wma_ptr;
	struct nan_req_params *params;
	size_t params_len;
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	params_len = sizeof(*params) + nan_req->request_data_len;
	params = qdf_mem_malloc(params_len);

	if (params == NULL) {
		WMA_LOGE(FL("memory allocation failed"));
		return QDF_STATUS_E_NOMEM;
	}

	params->request_data_len = nan_req->request_data_len;
	if (params->request_data_len > 0)
		qdf_mem_copy(params->request_data, nan_req->request_data,
			     params->request_data_len);

	status = wmi_unified_nan_req_cmd(wma_handle->wmi_handle, params);
	qdf_mem_free(params);

	return status;
}
#endif /* WLAN_FEATURE_NAN */

#ifdef DHCP_SERVER_OFFLOAD
/**
 * wma_process_dhcpserver_offload() - enable DHCP server offload
 * @wma_handle: wma handle
 * @pDhcpSrvOffloadInfo: DHCP server offload info
 *
 * Return: 0 for success or error code
 */
QDF_STATUS wma_process_dhcpserver_offload(tp_wma_handle wma_handle,
					  tSirDhcpSrvOffloadInfo *
					  pDhcpSrvOffloadInfo)
{
	struct dhcp_offload_info_params params = {0};
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return -EIO;
	}

	params.vdev_id = pDhcpSrvOffloadInfo->vdev_id;
	params.dhcp_offload_enabled =
				pDhcpSrvOffloadInfo->dhcpSrvOffloadEnabled;
	params.dhcp_client_num = pDhcpSrvOffloadInfo->dhcpClientNum;
	params.dhcp_srv_addr = pDhcpSrvOffloadInfo->dhcpSrvIP;

	status = wmi_unified_process_dhcpserver_offload_cmd(
				wma_handle->wmi_handle, &params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	WMA_LOGD("Set dhcp server offload to vdevId %d",
		 pDhcpSrvOffloadInfo->vdev_id);
	return status;
}
#endif /* DHCP_SERVER_OFFLOAD */

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
/**
 * wma_set_led_flashing() - set led flashing in fw
 * @wma_handle: wma handle
 * @flashing: flashing request
 *
 * Return: QDF status
 */
QDF_STATUS wma_set_led_flashing(tp_wma_handle wma_handle,
				struct flashing_req_params *flashing)
{
	QDF_STATUS status;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE(FL("WMA is closed, can not issue cmd"));
		return QDF_STATUS_E_INVAL;
	}
	if (!flashing) {
		WMA_LOGE(FL("invalid parameter: flashing"));
		return QDF_STATUS_E_INVAL;
	}
	status = wmi_unified_set_led_flashing_cmd(wma_handle->wmi_handle,
						  flashing);
	return status;
}
#endif /* WLAN_FEATURE_GPIO_LED_FLASHING */

#ifdef FEATURE_WLAN_CH_AVOID
/**
 * wma_process_ch_avoid_update_req() - handles channel avoid update request
 * @wma_handle: wma handle
 * @ch_avoid_update_req: channel avoid update params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_ch_avoid_update_req(tp_wma_handle wma_handle,
					   tSirChAvoidUpdateReq *
					   ch_avoid_update_req)
{
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (ch_avoid_update_req == NULL) {
		WMA_LOGE("%s : ch_avoid_update_req is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("%s: WMA --> WMI_CHAN_AVOID_UPDATE", __func__);

	status = wmi_unified_process_ch_avoid_update_cmd(
					wma_handle->wmi_handle);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	WMA_LOGD("%s: WMA --> WMI_CHAN_AVOID_UPDATE sent through WMI",
		 __func__);
	return status;
}
#endif

/**
 * wma_send_regdomain_info_to_fw() - send regdomain info to fw
 * @reg_dmn: reg domain
 * @regdmn2G: 2G reg domain
 * @regdmn5G: 5G reg domain
 * @ctl2G: 2G test limit
 * @ctl5G: 5G test limit
 *
 * Return: none
 */
void wma_send_regdomain_info_to_fw(uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	int32_t cck_mask_val = 0;
	struct pdev_params pdev_param = {0};
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == wma) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return;
	}

	status = wmi_unified_send_regdomain_info_to_fw_cmd(wma->wmi_handle,
			reg_dmn, regdmn2G, regdmn5G, ctl2G, ctl5G);
	if (status == QDF_STATUS_E_NOMEM)
		return;

	if ((((reg_dmn & ~CTRY_FLAG) == CTRY_JAPAN15) ||
	     ((reg_dmn & ~CTRY_FLAG) == CTRY_KOREA_ROC)) &&
	    (true == wma->tx_chain_mask_cck))
		cck_mask_val = 1;

	cck_mask_val |= (wma->self_gen_frm_pwr << 16);
	pdev_param.param_id = WMI_PDEV_PARAM_TX_CHAIN_MASK_CCK;
	pdev_param.param_value = cck_mask_val;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &pdev_param,
					 WMA_WILDCARD_PDEV_ID);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set PDEV tx_chain_mask_cck %d",
			 ret);
}

#ifdef FEATURE_WLAN_TDLS
/**
 * wma_tdls_event_handler() - handle TDLS event
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_tdls_event_handler(void *handle, uint8_t *event, uint32_t len)
{
	/* TODO update with target rx ops */
	return 0;
}

/**
 * wma_set_tdls_offchan_mode() - set tdls off channel mode
 * @handle: wma handle
 * @chan_switch_params: Pointer to tdls channel switch parameter structure
 *
 * This function sets tdls off channel mode
 *
 * Return: 0 on success; Negative errno otherwise
 */
QDF_STATUS wma_set_tdls_offchan_mode(WMA_HANDLE handle,
			      tdls_chan_switch_params *chan_switch_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct tdls_channel_switch_params params = {0};
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE(FL(
			    "WMA is closed, can not issue tdls off channel cmd"
			 ));
		ret = -EINVAL;
		goto end;
	}

	if (wma_is_roam_synch_in_progress(wma_handle,
					  chan_switch_params->vdev_id)) {
		WMA_LOGE("%s: roaming in progress, reject offchan mode cmd!",
			 __func__);
		ret = -EPERM;
		goto end;
	}

	params.vdev_id = chan_switch_params->vdev_id;
	params.tdls_off_ch_bw_offset =
			chan_switch_params->tdls_off_ch_bw_offset;
	params.tdls_off_ch = chan_switch_params->tdls_off_ch;
	params.tdls_sw_mode = chan_switch_params->tdls_sw_mode;
	params.oper_class = chan_switch_params->oper_class;
	params.is_responder = chan_switch_params->is_responder;
	qdf_mem_copy(params.peer_mac_addr, chan_switch_params->peer_mac_addr,
		     IEEE80211_ADDR_LEN);

	ret = wmi_unified_set_tdls_offchan_mode_cmd(wma_handle->wmi_handle,
							&params);

end:
	if (chan_switch_params)
		qdf_mem_free(chan_switch_params);
	return ret;
}

/**
 * wma_update_tdls_peer_state() - update TDLS peer state
 * @handle: wma handle
 * @peerStateParams: TDLS peer state params
 *
 * Return: 0 for success or error code
 */
int wma_update_tdls_peer_state(WMA_HANDLE handle,
			       tTdlsPeerStateParams *peerStateParams)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint32_t i;
	struct cdp_pdev *pdev;
	uint8_t peer_id;
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t *peer_mac_addr;
	int ret = 0;
	uint32_t *ch_mhz = NULL;
	bool restore_last_peer = false;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue cmd", __func__);
		ret = -EINVAL;
		goto end_tdls_peer_state;
	}

	if (!soc) {
		WMA_LOGE("%s: SOC context is NULL", __func__);
		ret = -EINVAL;
		goto end_tdls_peer_state;
	}

	if (wma_is_roam_synch_in_progress(wma_handle,
					  peerStateParams->vdevId)) {
		WMA_LOGE("%s: roaming in progress, reject peer update cmd!",
			 __func__);
		ret = -EPERM;
		goto end_tdls_peer_state;
	}

	/* peer capability info is valid only when peer state is connected */
	if (WMA_TDLS_PEER_STATE_CONNECTED != peerStateParams->peerState) {
		qdf_mem_zero(&peerStateParams->peerCap,
			     sizeof(tTdlsPeerCapParams));
	}

	if (peerStateParams->peerCap.peerChanLen) {
		ch_mhz = qdf_mem_malloc(sizeof(uint32_t) *
				peerStateParams->peerCap.peerChanLen);
		if (ch_mhz == NULL) {
			WMA_LOGE("%s: memory allocation failed", __func__);
			ret = -ENOMEM;
			goto end_tdls_peer_state;
		}
	}

	for (i = 0; i < peerStateParams->peerCap.peerChanLen; ++i) {
		ch_mhz[i] =
			cds_chan_to_freq(peerStateParams->peerCap.peerChan[i].
					 chanId);
	}

	/* Make sure that peer exists before sending peer state cmd*/
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		ret = -EIO;
		goto end_tdls_peer_state;
	}

	peer = cdp_peer_find_by_addr(soc,
			pdev,
			peerStateParams->peerMacAddr,
			&peer_id);
	if (!peer) {
		WMA_LOGE("%s: Failed to get peer handle using peer mac %pM",
				__func__, peerStateParams->peerMacAddr);
		ret = -EIO;
		goto end_tdls_peer_state;
	}

	if (wmi_unified_update_tdls_peer_state_cmd(wma_handle->wmi_handle,
			 (struct tdls_peer_state_params *)peerStateParams,
			 ch_mhz)) {
		WMA_LOGE("%s: failed to send tdls peer update state command",
			 __func__);
		ret = -EIO;
		goto end_tdls_peer_state;
	}

	/* in case of teardown, remove peer from fw */
	if (WMA_TDLS_PEER_STATE_TEARDOWN == peerStateParams->peerState) {
		peer_mac_addr = cdp_peer_get_peer_mac_addr(soc, peer);
		if (peer_mac_addr == NULL) {
			WMA_LOGE("peer_mac_addr is NULL");
			ret = -EIO;
			goto end_tdls_peer_state;
		}

		restore_last_peer = cdp_peer_is_vdev_restore_last_peer(
						soc, peer);

		WMA_LOGD("%s: calling wma_remove_peer for peer " MAC_ADDRESS_STR
			 " vdevId: %d", __func__,
			 MAC_ADDR_ARRAY(peer_mac_addr),
			 peerStateParams->vdevId);
		wma_remove_peer(wma_handle, peer_mac_addr,
				peerStateParams->vdevId, peer, false);
		cdp_peer_update_last_real_peer(soc,
				pdev, peer, &peer_id,
				restore_last_peer);
	}

end_tdls_peer_state:
	if (ch_mhz)
		qdf_mem_free(ch_mhz);
	if (peerStateParams)
		qdf_mem_free(peerStateParams);
	return ret;
}
#endif /* FEATURE_WLAN_TDLS */


/*
 * wma_process_set_ie_info() - Function to send IE info to firmware
 * @wma:                Pointer to WMA handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
QDF_STATUS wma_process_set_ie_info(tp_wma_handle wma,
				   struct vdev_ie_info *ie_info)
{
	struct wma_txrx_node *interface;
	struct vdev_ie_info_param cmd = {0};
	int ret;

	if (!ie_info || !wma) {
		WMA_LOGE(FL("input pointer is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	/* Validate the input */
	if (ie_info->length  <= 0) {
		WMA_LOGE(FL("Invalid IE length"));
		return QDF_STATUS_E_INVAL;
	}

	if (ie_info->vdev_id >= wma->max_bssid) {
		WMA_LOGE(FL("Invalid vdev_id: %d"), ie_info->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	interface = &wma->interfaces[ie_info->vdev_id];
	if (!interface->is_vdev_valid) {
		WMA_LOGE(FL("vdev_id: %d is not active"), ie_info->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	cmd.vdev_id = ie_info->vdev_id;
	cmd.ie_id = ie_info->ie_id;
	cmd.length = ie_info->length;
	cmd.band = ie_info->band;
	cmd.data = ie_info->data;
	cmd.ie_source = WMA_SET_VDEV_IE_SOURCE_HOST;

	WMA_LOGD(FL("vdev id: %d, ie_id: %d, band: %d, len: %d"),
		 ie_info->vdev_id, ie_info->ie_id, ie_info->band,
		 ie_info->length);

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
		ie_info->data, ie_info->length);

	ret = wmi_unified_process_set_ie_info_cmd(wma->wmi_handle,
				   &cmd);
	return ret;
}

/**
 *  wma_get_bpf_caps_event_handler() - Event handler for get bpf capability
 *  @handle: WMA global handle
 *  @cmd_param_info: command event data
 *  @len: Length of @cmd_param_info
 *
 *  Return: 0 on Success or Errno on failure
 */
int wma_get_bpf_caps_event_handler(void *handle,
			u_int8_t *cmd_param_info,
			u_int32_t len)
{
	WMI_BPF_CAPABILIY_INFO_EVENTID_param_tlvs  *param_buf;
	wmi_bpf_capability_info_evt_fixed_param *event;
	struct sir_bpf_get_offload *bpf_get_offload;
	tpAniSirGlobal pmac = (tpAniSirGlobal)cds_get_context(
				QDF_MODULE_ID_PE);

	if (!pmac) {
		WMA_LOGE("%s: Invalid pmac", __func__);
		return -EINVAL;
	}
	if (!pmac->sme.bpf_get_offload_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_BPF_CAPABILIY_INFO_EVENTID_param_tlvs *)cmd_param_info;
	event = param_buf->fixed_param;
	bpf_get_offload = qdf_mem_malloc(sizeof(*bpf_get_offload));

	if (!bpf_get_offload) {
		WMA_LOGP("%s: Memory allocation failed.", __func__);
		return -ENOMEM;
	}

	bpf_get_offload->bpf_version = event->bpf_version;
	bpf_get_offload->max_bpf_filters = event->max_bpf_filters;
	bpf_get_offload->max_bytes_for_bpf_inst =
			event->max_bytes_for_bpf_inst;
	WMA_LOGD("%s: BPF capabilities version: %d max bpf filter size: %d",
			__func__, bpf_get_offload->bpf_version,
	bpf_get_offload->max_bytes_for_bpf_inst);

	WMA_LOGD("%s: sending bpf capabilities event to hdd", __func__);
	pmac->sme.bpf_get_offload_cb(pmac->sme.bpf_get_offload_context,
				     bpf_get_offload);
	qdf_mem_free(bpf_get_offload);
	return 0;
}

/**
 * wma_get_bpf_capabilities - Send get bpf capability to firmware
 * @wma_handle: wma handle
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS wma_get_bpf_capabilities(tp_wma_handle wma)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	wmi_bpf_get_capability_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t   len;
	u_int8_t *buf_ptr;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE(FL("WMA is closed, can not issue get BPF capab"));
		return QDF_STATUS_E_INVAL;
	}

	if (!wmi_service_enabled(wma->wmi_handle,
		wmi_service_bpf_offload)) {
		WMA_LOGE(FL("BPF cababilities feature bit not enabled"));
		return QDF_STATUS_E_FAILURE;
	}

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (u_int8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_bpf_get_capability_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_bpf_get_capability_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_bpf_get_capability_cmd_fixed_param));

	if (wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, len,
		WMI_BPF_GET_CAPABILITY_CMDID)) {
		WMA_LOGE(FL("Failed to send BPF capability command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return status;
}

/**
 *  wma_set_bpf_instructions - Set bpf instructions to firmware
 *  @wma: wma handle
 *  @bpf_set_offload: Bpf offload information to set to firmware
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_set_bpf_instructions(tp_wma_handle wma,
				struct sir_bpf_set_offload *bpf_set_offload)
{
	wmi_bpf_set_vdev_instructions_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t   len = 0, len_aligned = 0;
	u_int8_t *buf_ptr;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue set BPF capability",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wmi_service_enabled(wma->wmi_handle,
		wmi_service_bpf_offload)) {
		WMA_LOGE(FL("BPF offload feature Disabled"));
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (!bpf_set_offload) {
		WMA_LOGE("%s: Invalid BPF instruction request", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (bpf_set_offload->session_id >= wma->max_bssid) {
		WMA_LOGE(FL("Invalid vdev_id: %d"),
			bpf_set_offload->session_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_is_vdev_up(bpf_set_offload->session_id)) {
		WMA_LOGE("vdev %d is not up skipping BPF offload",
			bpf_set_offload->session_id);
		return QDF_STATUS_E_INVAL;
	}

	if (bpf_set_offload->total_length) {
		len_aligned = roundup(bpf_set_offload->current_length,
					sizeof(A_UINT32));
		len = len_aligned + WMI_TLV_HDR_SIZE;
	}

	len += sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (u_int8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_bpf_set_vdev_instructions_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bpf_set_vdev_instructions_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_bpf_set_vdev_instructions_cmd_fixed_param));
	cmd->vdev_id = bpf_set_offload->session_id;
	cmd->filter_id = bpf_set_offload->filter_id;
	cmd->total_length = bpf_set_offload->total_length;
	cmd->current_offset = bpf_set_offload->current_offset;
	cmd->current_length = bpf_set_offload->current_length;

	if (bpf_set_offload->total_length) {
		buf_ptr +=
			sizeof(wmi_bpf_set_vdev_instructions_cmd_fixed_param);
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, len_aligned);
		buf_ptr += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(buf_ptr, bpf_set_offload->program,
					bpf_set_offload->current_length);
	}

	if (wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, len,
		WMI_BPF_SET_VDEV_INSTRUCTIONS_CMDID)) {
		WMA_LOGE(FL("Failed to send config bpf instructions command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMA_LOGD(FL("BPF offload enabled in fw"));

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_tx_rx_aggregation_size() - sets tx rx aggregation sizes
 * @tx_rx_aggregation_size: aggregation size parameters
 *
 * This function sets tx rx aggregation sizes
 *
 * Return: VOS_STATUS_SUCCESS on success, error number otherwise
 */
QDF_STATUS wma_set_tx_rx_aggregation_size(
	struct sir_set_tx_rx_aggregation_size *tx_rx_aggregation_size)
{
	tp_wma_handle wma_handle;
	wmi_vdev_set_custom_aggr_size_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	u_int8_t *buf_ptr;
	int ret;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!tx_rx_aggregation_size) {
		WMA_LOGE("%s: invalid pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_handle) {
		WMA_LOGE("%s: WMA context is invald!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);

	if (!buf) {
		WMA_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (u_int8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_set_custom_aggr_size_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_set_custom_aggr_size_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_vdev_set_custom_aggr_size_cmd_fixed_param));

	cmd->vdev_id = tx_rx_aggregation_size->vdev_id;
	cmd->tx_aggr_size = tx_rx_aggregation_size->tx_aggregation_size;
	cmd->rx_aggr_size = tx_rx_aggregation_size->rx_aggregation_size;

	WMA_LOGD("tx aggr: %d rx aggr: %d vdev: %d",
		cmd->tx_aggr_size, cmd->rx_aggr_size, cmd->vdev_id);

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				WMI_VDEV_SET_CUSTOM_AGGR_SIZE_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send aggregation size command",
				__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  wma_p2p_lo_start() - P2P listen offload start
 *  @params: p2p listen offload parameters
 *
 *  This function sends WMI command to start P2P listen offload.
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_p2p_lo_start(struct sir_p2p_lo_start *params)
{
	wmi_buf_t buf;
	wmi_p2p_lo_start_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	uint8_t *buf_ptr;
	int ret;
	int device_types_len_aligned, probe_resp_len_aligned;

	if (NULL == wma) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	device_types_len_aligned = qdf_roundup(params->dev_types_len,
						sizeof(A_UINT32));
	probe_resp_len_aligned = qdf_roundup(params->probe_resp_len,
						sizeof(A_UINT32));

	len += 2 * WMI_TLV_HDR_SIZE + device_types_len_aligned +
			probe_resp_len_aligned;

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: failed to allocate memory for p2p lo start",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_p2p_lo_start_cmd_fixed_param *)wmi_buf_data(buf);
	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_p2p_lo_start_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(
			wmi_p2p_lo_start_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	cmd->ctl_flags = params->ctl_flags;
	cmd->channel = params->freq;
	cmd->period = params->period;
	cmd->interval = params->interval;
	cmd->count = params->count;
	cmd->device_types_len = params->dev_types_len;
	cmd->prob_resp_len = params->probe_resp_len;

	buf_ptr += sizeof(wmi_p2p_lo_start_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				device_types_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, params->device_types, params->dev_types_len);

	buf_ptr += device_types_len_aligned;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, probe_resp_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, params->probe_resp_tmplt, params->probe_resp_len);

	WMA_LOGI("%s: Sending WMI_P2P_LO_START command, channel=%d, period=%d, interval=%d, count=%d",
			__func__, cmd->channel, cmd->period,
			cmd->interval, cmd->count);

	ret = wmi_unified_cmd_send(wma->wmi_handle,
				   buf, len,
				   WMI_P2P_LISTEN_OFFLOAD_START_CMDID);
	if (ret) {
		WMA_LOGE("Failed to send p2p lo start: %d", ret);
		wmi_buf_free(buf);
	}

	WMA_LOGI("%s: Successfully sent WMI_P2P_LO_START", __func__);
	wma->interfaces[params->vdev_id].p2p_lo_in_progress = true;

	return ret;
}

/**
 *  wma_p2p_lo_stop() - P2P listen offload stop
 *  @vdev_id: vdev identifier
 *
 *  This function sends WMI command to stop P2P listen offload.
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_p2p_lo_stop(u_int32_t vdev_id)
{
	wmi_buf_t buf;
	wmi_p2p_lo_stop_cmd_fixed_param *cmd;
	int32_t len;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	int ret;

	if (NULL == wma) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: failed to allocate memory for p2p lo stop",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_p2p_lo_stop_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_p2p_lo_stop_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_p2p_lo_stop_cmd_fixed_param));

	cmd->vdev_id = vdev_id;

	WMA_LOGI("%s: Sending WMI_P2P_LO_STOP command", __func__);

	ret = wmi_unified_cmd_send(wma->wmi_handle,
				   buf, len,
				   WMI_P2P_LISTEN_OFFLOAD_STOP_CMDID);
	if (ret) {
		WMA_LOGE("Failed to send p2p lo stop: %d", ret);
		wmi_buf_free(buf);
	}

	WMA_LOGI("%s: Successfully sent WMI_P2P_LO_STOP", __func__);
	wma->interfaces[vdev_id].p2p_lo_in_progress = false;

	return ret;
}

/**
 * wma_p2p_lo_event_handler() - p2p lo event
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * This function receives P2P listen offload stop event from FW and
 * pass the event information to upper layer.
 *
 * Return: 0 on success
 */
int wma_p2p_lo_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct sir_p2p_lo_event *event;
	WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID_param_tlvs *param_tlvs;
	wmi_p2p_lo_stopped_event_fixed_param *fix_param;
	tpAniSirGlobal p_mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!wma) {
		WMA_LOGE("%s: Invalid WMA Context", __func__);
		return -EINVAL;
	}

	if (!p_mac) {
		WMA_LOGE("%s: Invalid p_mac", __func__);
		return -EINVAL;
	}

	if (!p_mac->sme.p2p_lo_event_callback) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID_param_tlvs *)
								event_buf;
	fix_param = param_tlvs->fixed_param;
	if (fix_param->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: received invalid vdev_id %d",
			 __func__, fix_param->vdev_id);
		return -EINVAL;
	}
	event = qdf_mem_malloc(sizeof(*event));
	if (event == NULL) {
		WMA_LOGE("Event allocation failed");
		return -ENOMEM;
	}
	event->vdev_id = fix_param->vdev_id;
	event->reason_code = fix_param->reason;

	p_mac->sme.p2p_lo_event_callback(p_mac->hHdd, event);

	wma->interfaces[event->vdev_id].p2p_lo_in_progress = false;

	return 0;
}

/**
 * wma_get_wakelock_stats() - Populates wake lock stats
 * @stats: non-null wakelock structure to populate
 *
 * This function collects wake lock stats
 *
 * Return: QDF_STATUS_SUCCESS on success, error value otherwise
 */
QDF_STATUS wma_get_wakelock_stats(struct sir_wake_lock_stats *stats)
{
	t_wma_handle *wma;
	struct sir_vdev_wow_stats *vstats;
	int i;

	if (!stats) {
		WMA_LOGE("%s: invalid stats pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: invalid WMA context", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* ensure counters are zeroed */
	qdf_mem_zero(stats, sizeof(*stats));

	/* populate global level stats */
	stats->wow_unspecified_wake_up_count = wma->wow_unspecified_wake_count;

	/* populate vdev level stats */
	for (i = 0; i < wma->max_bssid; ++i) {
		if (!wma->interfaces[i].handle)
			continue;

		vstats = &wma->interfaces[i].wow_stats;

		stats->wow_ucast_wake_up_count += vstats->ucast;
		stats->wow_bcast_wake_up_count += vstats->bcast;
		stats->wow_ipv4_mcast_wake_up_count += vstats->ipv4_mcast;
		stats->wow_ipv6_mcast_wake_up_count += vstats->ipv6_mcast;
		stats->wow_ipv6_mcast_ra_stats += vstats->ipv6_mcast_ra;
		stats->wow_ipv6_mcast_ns_stats += vstats->ipv6_mcast_ns;
		stats->wow_ipv6_mcast_na_stats += vstats->ipv6_mcast_na;
		stats->wow_icmpv4_count += vstats->icmpv4;
		stats->wow_icmpv6_count += vstats->icmpv6;
		stats->wow_rssi_breach_wake_up_count += vstats->rssi_breach;
		stats->wow_low_rssi_wake_up_count += vstats->low_rssi;
		stats->wow_gscan_wake_up_count += vstats->gscan;
		stats->wow_pno_complete_wake_up_count += vstats->pno_complete;
		stats->wow_pno_match_wake_up_count += vstats->pno_match;
		stats->wow_oem_response_wake_up_count += vstats->oem_response;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_fw_test_cmd() - send unit test command to fw.
 * @handle: wma handle
 * @wma_fwtest: fw test command
 *
 * This function send fw test command to fw.
 *
 * Return: none
 */
void wma_process_fw_test_cmd(WMA_HANDLE handle,
			     struct set_fwtest_params *wma_fwtest)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue fw test cmd",
			 __func__);
		return;
	}

	if (wmi_unified_fw_test_cmd(wma_handle->wmi_handle,
				    (struct set_fwtest_params *)wma_fwtest)) {
		WMA_LOGE("%s: Failed to issue fw test cmd",
			 __func__);
		return;
	}
}

/**
 * wma_enable_disable_caevent_ind() - Issue WMI command to enable or
 * disable ca event indication
 * @wma: wma handler
 * @val: boolean value true or false
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_enable_disable_caevent_ind(tp_wma_handle wma, uint8_t val)
{
	WMI_CHAN_AVOID_RPT_ALLOW_CMD_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t len;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE(FL("WMA is closed, can not issue set/clear CA"));
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (WMI_CHAN_AVOID_RPT_ALLOW_CMD_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_CHAN_AVOID_RPT_ALLOW_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				WMI_CHAN_AVOID_RPT_ALLOW_CMD_fixed_param));
	cmd->rpt_allow = val;
	if (wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, len,
				WMI_CHAN_AVOID_RPT_ALLOW_CMDID)) {
		WMA_LOGE(FL("Failed to send enable/disable CA event command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static wma_sar_cb sar_callback;
static void *sar_context;

static int wma_sar_event_handler(void *handle, uint8_t *evt_buf, uint32_t len)
{
	tp_wma_handle wma_handle;
	wmi_unified_t wmi_handle;
	struct sar_limit_event *event;
	wma_sar_cb callback;
	QDF_STATUS status;

	WMA_LOGI(FL("handle:%pK event:%pK len:%u"), handle, evt_buf, len);

	wma_handle = handle;
	if (!wma_handle) {
		WMA_LOGE(FL("NULL wma_handle"));
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE(FL("NULL wmi_handle"));
		return QDF_STATUS_E_INVAL;
	}

	event = qdf_mem_malloc(sizeof(*event));
	if (!event) {
		WMA_LOGE(FL("failed to malloc sar_limit_event"));
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_unified_extract_sar_limit_event(wmi_handle,
						     evt_buf, event);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE(FL("Event extract failure: %d"), status);
		qdf_mem_free(event);
		return QDF_STATUS_E_INVAL;
	}

	callback = sar_callback;
	sar_callback = NULL;
	if (callback)
		callback(sar_context, event);

	qdf_mem_free(event);

	return 0;
}

QDF_STATUS wma_sar_register_event_handlers(WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = handle;
	wmi_unified_t wmi_handle;

	if (!wma_handle) {
		WMA_LOGE(FL("NULL wma_handle"));
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE(FL("NULL wmi_handle"));
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_register_event_handler(wmi_handle,
						  wmi_sar_get_limits_event_id,
						  wma_sar_event_handler,
						  WMA_RX_WORK_CTX);
}

QDF_STATUS wma_get_sar_limit(WMA_HANDLE handle,
			     wma_sar_cb callback, void *context)
{
	tp_wma_handle wma_handle = handle;
	wmi_unified_t wmi_handle;
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE(FL("NULL wma_handle"));
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE(FL("NULL wmi_handle"));
		return QDF_STATUS_E_INVAL;
	}

	sar_callback = callback;
	sar_context = context;
	status = wmi_unified_get_sar_limit_cmd(wmi_handle);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE(FL("wmi_unified_get_sar_limit_cmd() error: %u"),
			 status);
		sar_callback = NULL;
	}

	return status;
}

QDF_STATUS wma_set_sar_limit(WMA_HANDLE handle,
		struct sar_limit_cmd_params *sar_limit_params)
{
	int ret;
	tp_wma_handle wma = (tp_wma_handle) handle;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue set sar limit msg",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	if (sar_limit_params == NULL) {
		WMA_LOGE("%s: set sar limit ptr NULL",
			__func__);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_send_sar_limit_cmd(wma->wmi_handle,
				sar_limit_params);

	return ret;
}

/**
 * wma_get_arp_stats_handler() - handle arp stats data
 * indicated by FW
 * @handle: wma context
 * @data: event buffer
 * @data len: length of event buffer
 *
 * Return: 0 on success
 */
int wma_get_arp_stats_handler(void *handle, uint8_t *data,
			uint32_t data_len)
{
	WMI_VDEV_GET_ARP_STAT_EVENTID_param_tlvs *param_buf;
	wmi_vdev_get_arp_stats_event_fixed_param *data_event;
	struct rsp_stats rsp;
	tpAniSirGlobal mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac context", __func__);
		return -EINVAL;
	}

	if (!mac->sme.get_arp_stats_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}

	if (data == NULL) {
		WMA_LOGE("%s: invalid pointer", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_VDEV_GET_ARP_STAT_EVENTID_param_tlvs *)data;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid get arp stats event", __func__);
		return -EINVAL;
	}
	data_event = param_buf->fixed_param;
	if (!data_event) {
		WMA_LOGE("%s: Invalid get arp stats data event", __func__);
		return -EINVAL;
	}
	rsp.arp_req_enqueue = data_event->arp_req_enqueue;
	rsp.vdev_id = data_event->vdev_id;
	rsp.arp_req_tx_success = data_event->arp_req_tx_success;
	rsp.arp_req_tx_failure = data_event->arp_req_tx_failure;
	rsp.arp_rsp_recvd = data_event->arp_rsp_recvd;
	rsp.out_of_order_arp_rsp_drop_cnt =
		data_event->out_of_order_arp_rsp_drop_cnt;
	rsp.dad_detected = data_event->dad_detected;
	rsp.connect_status = data_event->connect_status;
	rsp.ba_session_establishment_status =
		data_event->ba_session_establishment_status;

	mac->sme.get_arp_stats_cb(mac->hHdd, &rsp);

	return 0;
}

/**
 * wma_unified_power_debug_stats_event_handler() - WMA handler function to
 * handle Power stats event from firmware
 * @handle: Pointer to wma handle
 * @cmd_param_info: Pointer to Power stats event TLV
 * @len: Length of the cmd_param_info
 *
 * Return: 0 on success, error number otherwise
 */
#ifdef WLAN_POWER_DEBUGFS
int wma_unified_power_debug_stats_event_handler(void *handle,
			uint8_t *cmd_param_info, uint32_t len)
{
	WMI_PDEV_CHIP_POWER_STATS_EVENTID_param_tlvs *param_tlvs;
	struct power_stats_response *power_stats_results;
	wmi_pdev_chip_power_stats_event_fixed_param *param_buf;
	uint32_t power_stats_len, stats_registers_len, *debug_registers;

	tpAniSirGlobal mac = (tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);

	param_tlvs =
		(WMI_PDEV_CHIP_POWER_STATS_EVENTID_param_tlvs *) cmd_param_info;

	param_buf = (wmi_pdev_chip_power_stats_event_fixed_param *)
		param_tlvs->fixed_param;
	if (!mac || !mac->sme.power_stats_resp_callback) {
		WMA_LOGD("%s: NULL mac ptr or HDD callback is null", __func__);
		return -EINVAL;
	}

	if (!param_buf) {
		WMA_LOGD("%s: NULL power stats event fixed param", __func__);
		return -EINVAL;
	}

	if (param_buf->num_debug_register > ((WMI_SVC_MSG_MAX_SIZE -
		sizeof(wmi_pdev_chip_power_stats_event_fixed_param)) /
		sizeof(uint32_t)) ||
	    param_buf->num_debug_register > param_tlvs->num_debug_registers) {
		WMA_LOGE("excess payload: LEN num_debug_register:%u",
				param_buf->num_debug_register);
		return -EINVAL;
	}
	debug_registers = param_tlvs->debug_registers;
	stats_registers_len =
		(sizeof(uint32_t) * param_buf->num_debug_register);
	power_stats_len = stats_registers_len + sizeof(*power_stats_results);
	power_stats_results = qdf_mem_malloc(power_stats_len);
	if (!power_stats_results) {
		WMA_LOGD("%s: could not allocate mem for power stats results",
				__func__);
		return -ENOMEM;
	}
	WMA_LOGD("Cumulative sleep time %d cumulative total on time %d deep sleep enter counter %d last deep sleep enter tstamp ts %d debug registers fmt %d num debug register %d",
			param_buf->cumulative_sleep_time_ms,
			param_buf->cumulative_total_on_time_ms,
			param_buf->deep_sleep_enter_counter,
			param_buf->last_deep_sleep_enter_tstamp_ms,
			param_buf->debug_register_fmt,
			param_buf->num_debug_register);

	power_stats_results->cumulative_sleep_time_ms
		= param_buf->cumulative_sleep_time_ms;
	power_stats_results->cumulative_total_on_time_ms
		= param_buf->cumulative_total_on_time_ms;
	power_stats_results->deep_sleep_enter_counter
		= param_buf->deep_sleep_enter_counter;
	power_stats_results->last_deep_sleep_enter_tstamp_ms
		= param_buf->last_deep_sleep_enter_tstamp_ms;
	power_stats_results->debug_register_fmt
		= param_buf->debug_register_fmt;
	power_stats_results->num_debug_register
		= param_buf->num_debug_register;

	power_stats_results->debug_registers
		= (uint32_t *)(power_stats_results + 1);

	qdf_mem_copy(power_stats_results->debug_registers,
			debug_registers, stats_registers_len);

	mac->sme.power_stats_resp_callback(power_stats_results,
			mac->sme.power_debug_stats_context);
	qdf_mem_free(power_stats_results);
	return 0;
}
#else
int wma_unified_power_debug_stats_event_handler(void *handle,
		uint8_t *cmd_param_info, uint32_t len)
{
	return 0;
}
#endif

int wma_chan_info_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle)handle;
	WMI_CHAN_INFO_EVENTID_param_tlvs *param_buf;
	wmi_chan_info_event_fixed_param *event;
	struct scan_chan_info buf;
	tpAniSirGlobal mac = NULL;
	struct lim_channel_status *channel_status;

	WMA_LOGD("%s: Enter", __func__);

	if (wma != NULL && wma->cds_context != NULL)
		mac = (tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac context", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: monitor:%d", __func__, mac->snr_monitor_enabled);
	if (mac->snr_monitor_enabled && mac->chan_info_cb) {
		param_buf =
			(WMI_CHAN_INFO_EVENTID_param_tlvs *)event_buf;
		if (!param_buf) {
			WMA_LOGA("%s: Invalid chan info event", __func__);
			return -EINVAL;
		}

		event = param_buf->fixed_param;
		if (!event) {
			WMA_LOGA("%s: Invalid fixed param", __func__);
			return -EINVAL;
		}
		buf.tx_frame_count = event->tx_frame_cnt;
		buf.clock_freq = event->mac_clk_mhz;
		buf.cmd_flag = event->cmd_flags;
		buf.freq = event->freq;
		buf.noise_floor = event->noise_floor;
		buf.cycle_count = event->cycle_count;
		buf.rx_clear_count = event->rx_clear_count;
		mac->chan_info_cb(&buf);
	}

	if (mac->sap.acs_with_more_param &&
	    mac->sme.currDeviceMode == QDF_SAP_MODE) {
		param_buf = (WMI_CHAN_INFO_EVENTID_param_tlvs *) event_buf;
		if (!param_buf)  {
			WMA_LOGE("Invalid chan info event buffer");
			return -EINVAL;
		}
		event = param_buf->fixed_param;
		channel_status =
			qdf_mem_malloc(sizeof(*channel_status));
		if (!channel_status) {
			WMA_LOGE(FL("Mem alloc fail"));
			return -ENOMEM;
		}
		WMA_LOGD(FL("freq=%d nf=%d rxcnt=%u cyccnt=%u tx_r=%d tx_t=%d"),
			 event->freq,
			 event->noise_floor,
			 event->rx_clear_count,
			 event->cycle_count,
			 event->chan_tx_pwr_range,
			 event->chan_tx_pwr_tp);

		channel_status->channelfreq = event->freq;
		channel_status->noise_floor = event->noise_floor;
		channel_status->rx_clear_count =
			 event->rx_clear_count;
		channel_status->cycle_count = event->cycle_count;
		channel_status->chan_tx_pwr_range =
			 event->chan_tx_pwr_range;
		channel_status->chan_tx_pwr_throughput =
			 event->chan_tx_pwr_tp;
		channel_status->rx_frame_count =
			 event->rx_frame_count;
		channel_status->bss_rx_cycle_count =
			event->my_bss_rx_cycle_count;
		channel_status->rx_11b_mode_data_duration =
			event->rx_11b_mode_data_duration;
		channel_status->tx_frame_count = event->tx_frame_cnt;
		channel_status->mac_clk_mhz = event->mac_clk_mhz;
		channel_status->channel_id =
			cds_freq_to_chan(event->freq);
		channel_status->cmd_flags =
			event->cmd_flags;

		wma_send_msg(handle, WMA_RX_CHN_STATUS_EVENT,
			     (void *)channel_status, 0);
	}

	return 0;
}

int wma_rx_aggr_failure_event_handler(void *handle, u_int8_t *event_buf,
							u_int32_t len)
{
	WMI_REPORT_RX_AGGR_FAILURE_EVENTID_param_tlvs *param_buf;
	struct sir_sme_rx_aggr_hole_ind *rx_aggr_hole_event;
	wmi_rx_aggr_failure_event_fixed_param *rx_aggr_failure_info;
	wmi_rx_aggr_failure_info *hole_info;
	uint32_t i, alloc_len;
	tpAniSirGlobal mac;

	mac = (tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);
	if (!mac || !mac->sme.stats_ext2_cb) {
		WMA_LOGD("%s: NULL mac ptr or HDD callback is null", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_REPORT_RX_AGGR_FAILURE_EVENTID_param_tlvs *)event_buf;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid stats ext event buf", __func__);
		return -EINVAL;
	}

	rx_aggr_failure_info = param_buf->fixed_param;
	hole_info = param_buf->failure_info;

	if (rx_aggr_failure_info->num_failure_info > ((WMI_SVC_MSG_MAX_SIZE -
	    sizeof(*rx_aggr_hole_event)) /
	    sizeof(rx_aggr_hole_event->hole_info_array[0]))) {
		WMA_LOGE("%s: Excess data from WMI num_failure_info %d",
			 __func__, rx_aggr_failure_info->num_failure_info);
		return -EINVAL;
	}

	alloc_len = sizeof(*rx_aggr_hole_event) +
		(rx_aggr_failure_info->num_failure_info)*
		sizeof(rx_aggr_hole_event->hole_info_array[0]);
	rx_aggr_hole_event = qdf_mem_malloc(alloc_len);
	if (NULL == rx_aggr_hole_event) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return -ENOMEM;
	}

	rx_aggr_hole_event->hole_cnt = rx_aggr_failure_info->num_failure_info;
	if (rx_aggr_hole_event->hole_cnt > param_buf->num_failure_info) {
		WMA_LOGE("Invalid no of hole count: %d",
				rx_aggr_hole_event->hole_cnt);
		qdf_mem_free(rx_aggr_hole_event);
		return -EINVAL;
	}
	WMA_LOGD("aggr holes_sum: %d\n",
		rx_aggr_failure_info->num_failure_info);
	for (i = 0; i < rx_aggr_hole_event->hole_cnt; i++) {
		rx_aggr_hole_event->hole_info_array[i] =
			hole_info->end_seq - hole_info->start_seq + 1;
		WMA_LOGD("aggr_index: %d\tstart_seq: %d\tend_seq: %d\t"
			"hole_info: %d mpdu lost",
			i, hole_info->start_seq, hole_info->end_seq,
			rx_aggr_hole_event->hole_info_array[i]);
		hole_info++;
	}

	mac->sme.stats_ext2_cb(mac->hHdd, rx_aggr_hole_event);
	qdf_mem_free(rx_aggr_hole_event);

	return 0;
}

int wma_wlan_bt_activity_evt_handler(void *handle, uint8_t *event, uint32_t len)
{
	wmi_coex_bt_activity_event_fixed_param *fixed_param;
	WMI_WLAN_COEX_BT_ACTIVITY_EVENTID_param_tlvs *param_buf =
		(WMI_WLAN_COEX_BT_ACTIVITY_EVENTID_param_tlvs *)event;
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS qdf_status;

	if (!param_buf) {
		WMA_LOGE(FL("Invalid BT activity event buffer"));
		return -EINVAL;
	}

	fixed_param = param_buf->fixed_param;
	if (!fixed_param) {
		WMA_LOGE(FL("Invalid BT activity event fixed param buffer"));
		return -EINVAL;
	}

	WMA_LOGI(FL("Received BT activity event %u"),
		    fixed_param->coex_profile_evt);

	sme_msg.type = eWNI_SME_BT_ACTIVITY_INFO_IND;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = fixed_param->coex_profile_evt;

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMA_LOGE(FL("Failed to post msg to SME"));
		return -EINVAL;
	}

	return 0;
}

int wma_peer_ant_info_evt_handler(void *handle, u_int8_t *event,
	u_int32_t len)
{
	wmi_peer_antdiv_info *peer_ant_info;
	WMI_PEER_ANTDIV_INFO_EVENTID_param_tlvs *param_buf;
	wmi_peer_antdiv_info_event_fixed_param *fix_param;
	struct chain_rssi_result *chain_rssi_result;
	u_int32_t chain_index;

	tpAniSirGlobal pmac = (tpAniSirGlobal)cds_get_context(
					QDF_MODULE_ID_PE);
	if (!pmac) {
		WMA_LOGE("%s: Invalid pmac", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_PEER_ANTDIV_INFO_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid peer_ant_info event buffer");
		return -EINVAL;
	}
	fix_param = param_buf->fixed_param;
	peer_ant_info = param_buf->peer_info;

	WMA_LOGD("num_peers=%d\tvdev_id=%d",
		fix_param->num_peers, fix_param->vdev_id);
	WMA_LOGD("peer_ant_info: %pK", peer_ant_info);

	if (!peer_ant_info) {
		WMA_LOGE("Invalid peer_ant_info ptr");
		return -EINVAL;
	}

	chain_rssi_result = qdf_mem_malloc(sizeof(*chain_rssi_result));
	if (!chain_rssi_result) {
		WMA_LOGE("%s: Failed to malloc", __func__);
		return -ENOMEM;
	}

	for (chain_index = 0; chain_index < CHAIN_RSSI_NUM; chain_index++)
		WMA_LOGD("chain%d rssi: %x", chain_index,
				peer_ant_info->chain_rssi[chain_index]);

	qdf_mem_copy(chain_rssi_result->chain_rssi,
				peer_ant_info->chain_rssi,
				sizeof(peer_ant_info->chain_rssi));

	pmac->sme.get_chain_rssi_cb(pmac->sme.get_chain_rssi_context,
				chain_rssi_result);

	qdf_mem_free(chain_rssi_result);

	return 0;
}

int wma_vdev_obss_detection_info_handler(void *handle, uint8_t *event,
					 uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct wmi_obss_detect_info *obss_detection;
	QDF_STATUS status;

	if (!event) {
		WMA_LOGE("Invalid obss_detection_info event buffer");
		return -EINVAL;
	}

	obss_detection = qdf_mem_malloc(sizeof(*obss_detection));
	if (!obss_detection) {
		WMA_LOGE("%s: Failed to malloc", __func__);
		return -ENOMEM;
	}

	status = wmi_unified_extract_obss_detection_info(wma->wmi_handle,
							 event, obss_detection);

	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to extract obss info", __func__);
		qdf_mem_free(obss_detection);
		return -EINVAL;
	}

	if (!wma_is_vdev_valid(obss_detection->vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id %d", __func__,
			 obss_detection->vdev_id);
		qdf_mem_free(obss_detection);
		return -EINVAL;
	}

	wma_send_msg(wma, WMA_OBSS_DETECTION_INFO, obss_detection, 0);

	return 0;
}

int wma_vdev_bss_color_collision_info_handler(void *handle,
					      uint8_t *event,
					      uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct wmi_obss_color_collision_info *obss_color_info;
	QDF_STATUS status;

	if (!event) {
		WMA_LOGE("Invalid obss_color_collision event buffer");
		return -EINVAL;
	}

	obss_color_info = qdf_mem_malloc(sizeof(*obss_color_info));
	if (!obss_color_info) {
		WMA_LOGE("%s: Failed to malloc", __func__);
		return -ENOMEM;
	}

	status = wmi_unified_extract_obss_color_collision_info(wma->wmi_handle,
							       event,
							       obss_color_info);

	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to extract obss color info", __func__);
		qdf_mem_free(obss_color_info);
		return -EINVAL;
	}

	if (!wma_is_vdev_valid(obss_color_info->vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id %d", __func__,
			 obss_color_info->vdev_id);
		qdf_mem_free(obss_color_info);
		return -EINVAL;
	}

	wma_send_msg(wma, WMA_OBSS_COLOR_COLLISION_INFO, obss_color_info, 0);

	return 0;
}
