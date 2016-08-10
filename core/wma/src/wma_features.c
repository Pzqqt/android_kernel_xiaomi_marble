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
#include "ol_txrx_ctrl_api.h"
#include <cdp_txrx_tx_delay.h>
#include <cdp_txrx_peer_ops.h>

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
#include "radar_filters.h"
#include "wma_internal.h"
#include "ol_txrx.h"
#include "wma_nan_datapath.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a)         (sizeof(a) / sizeof((a)[0]))
#endif

#define WMA_WOW_STA_WAKE_UP_EVENTS ((1 << WOW_CSA_IE_EVENT) |\
				(1 << WOW_CLIENT_KICKOUT_EVENT) |\
				(1 << WOW_PATTERN_MATCH_EVENT) |\
				(1 << WOW_MAGIC_PKT_RECVD_EVENT) |\
				(1 << WOW_DEAUTH_RECVD_EVENT) |\
				(1 << WOW_DISASSOC_RECVD_EVENT) |\
				(1 << WOW_BMISS_EVENT) |\
				(1 << WOW_GTK_ERR_EVENT) |\
				(1 << WOW_BETTER_AP_EVENT) |\
				(1 << WOW_HTT_EVENT) |\
				(1 << WOW_RA_MATCH_EVENT) |\
				(1 << WOW_NLO_DETECTED_EVENT) |\
				(1 << WOW_EXTSCAN_EVENT)) |\
				(1 << WOW_OEM_RESPONSE_EVENT)\

#define WMA_WOW_SAP_WAKE_UP_EVENTS ((1 << WOW_PROBE_REQ_WPS_IE_EVENT) |\
				(1 << WOW_PATTERN_MATCH_EVENT) |\
				(1 << WOW_AUTH_REQ_EVENT) |\
				(1 << WOW_ASSOC_REQ_EVENT) |\
				(1 << WOW_DEAUTH_RECVD_EVENT) |\
				(1 << WOW_DISASSOC_RECVD_EVENT) |\
				(1 << WOW_HTT_EVENT))\

static const uint8_t arp_ptrn[] = {0x08, 0x06};
static const uint8_t arp_mask[] = {0xff, 0xff};
static const uint8_t ns_ptrn[] = {0x86, 0xDD};
static const uint8_t discvr_ptrn[] = {0xe0, 0x00, 0x00, 0xf8};
static const uint8_t discvr_mask[] = {0xf0, 0x00, 0x00, 0xf8};

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/**
 * wma_post_auto_shutdown_msg() - to post auto shutdown event to sme
 *
 * Return: 0 for success or error code
 */
static int wma_post_auto_shutdown_msg(void)
{
	tSirAutoShutdownEvtParams *auto_sh_evt;
	QDF_STATUS qdf_status;
	cds_msg_t sme_msg = { 0 };

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

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Fail to post eWNI_SME_AUTO_SHUTDOWN_IND msg to SME");
		qdf_mem_free(auto_sh_evt);
		return -EINVAL;
	}

	return 0;
}
#endif
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
			WMA_LOGE("Failed to allocate memory for tAniGetRssiReq");
			wma_handle->pGetRssiReq = NULL;
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_set(pRssiBkUp, sizeof(tAniGetRssiReq), 0);
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
	cds_msg_t tsf_msg = {0};
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
		cds_mq_post_message(CDS_MQ_ID_SME, &tsf_msg)) {

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

#ifdef FEATURE_WLAN_LPHB
/**
 * wma_lphb_conf_hbenable() - enable command of LPHB configuration requests
 * @wma_handle: WMA handle
 * @lphb_conf_req: configuration info
 * @by_user: whether this call is from user or cached resent
 *
 * Return: QDF status
 */
QDF_STATUS wma_lphb_conf_hbenable(tp_wma_handle wma_handle,
				  tSirLPHBReq *lphb_conf_req, bool by_user)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int status = 0;
	tSirLPHBEnableStruct *ts_lphb_enable;
	wmi_hb_set_enable_cmd_fixed_param hb_enable_fp;
	int i;

	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ts_lphb_enable = &(lphb_conf_req->params.lphbEnableReq);
	WMA_LOGI("%s: WMA --> WMI_HB_SET_ENABLE enable=%d, item=%d, session=%d",
		 __func__,
		 ts_lphb_enable->enable,
		 ts_lphb_enable->item, ts_lphb_enable->session);

	if ((ts_lphb_enable->item != 1) && (ts_lphb_enable->item != 2)) {
		WMA_LOGE("%s : LPHB configuration wrong item %d",
			 __func__, ts_lphb_enable->item);
		return QDF_STATUS_E_FAILURE;
	}


	/* fill in values */
	hb_enable_fp.vdev_id = ts_lphb_enable->session;
	hb_enable_fp.enable = ts_lphb_enable->enable;
	hb_enable_fp.item = ts_lphb_enable->item;
	hb_enable_fp.session = ts_lphb_enable->session;

	status = wmi_unified_lphb_config_hbenable_cmd(wma_handle->wmi_handle,
				      &hb_enable_fp);
	if (status != EOK) {
		qdf_status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	if (by_user) {
		/* target already configured, now cache command status */
		if (ts_lphb_enable->enable) {
			i = ts_lphb_enable->item - 1;
			wma_handle->wow.lphb_cache[i].cmd
				= LPHB_SET_EN_PARAMS_INDID;
			wma_handle->wow.lphb_cache[i].params.lphbEnableReq.
			enable = ts_lphb_enable->enable;
			wma_handle->wow.lphb_cache[i].params.lphbEnableReq.
			item = ts_lphb_enable->item;
			wma_handle->wow.lphb_cache[i].params.lphbEnableReq.
			session = ts_lphb_enable->session;

			WMA_LOGI("%s: cached LPHB status in WMA context for item %d",
				__func__, i);
		} else {
			qdf_mem_zero((void *)&wma_handle->wow.lphb_cache,
				     sizeof(wma_handle->wow.lphb_cache));
			WMA_LOGI("%s: cleared all cached LPHB status in WMA context",
				__func__);
		}
	}

	return QDF_STATUS_SUCCESS;
error:
	return qdf_status;
}

/**
 * wma_lphb_conf_tcp_params() - set tcp params of LPHB configuration requests
 * @wma_handle: wma handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF status
 */
QDF_STATUS wma_lphb_conf_tcp_params(tp_wma_handle wma_handle,
				    tSirLPHBReq *lphb_conf_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int status = 0;
	tSirLPHBTcpParamStruct *ts_lphb_tcp_param;
	wmi_hb_set_tcp_params_cmd_fixed_param hb_tcp_params_fp = {0};


	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ts_lphb_tcp_param = &(lphb_conf_req->params.lphbTcpParamReq);
	WMA_LOGI("%s: WMA --> WMI_HB_SET_TCP_PARAMS srv_ip=%08x, "
		"dev_ip=%08x, src_port=%d, dst_port=%d, timeout=%d, "
		"session=%d, gateway_mac="MAC_ADDRESS_STR", timePeriodSec=%d, "
		"tcpSn=%d", __func__, ts_lphb_tcp_param->srv_ip,
		ts_lphb_tcp_param->dev_ip, ts_lphb_tcp_param->src_port,
		ts_lphb_tcp_param->dst_port, ts_lphb_tcp_param->timeout,
		ts_lphb_tcp_param->session,
		MAC_ADDR_ARRAY(ts_lphb_tcp_param->gateway_mac.bytes),
		ts_lphb_tcp_param->timePeriodSec, ts_lphb_tcp_param->tcpSn);

	/* fill in values */
	hb_tcp_params_fp.vdev_id = ts_lphb_tcp_param->session;
	hb_tcp_params_fp.srv_ip = ts_lphb_tcp_param->srv_ip;
	hb_tcp_params_fp.dev_ip = ts_lphb_tcp_param->dev_ip;
	hb_tcp_params_fp.seq = ts_lphb_tcp_param->tcpSn;
	hb_tcp_params_fp.src_port = ts_lphb_tcp_param->src_port;
	hb_tcp_params_fp.dst_port = ts_lphb_tcp_param->dst_port;
	hb_tcp_params_fp.interval = ts_lphb_tcp_param->timePeriodSec;
	hb_tcp_params_fp.timeout = ts_lphb_tcp_param->timeout;
	hb_tcp_params_fp.session = ts_lphb_tcp_param->session;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ts_lphb_tcp_param->gateway_mac.bytes,
				   &hb_tcp_params_fp.gateway_mac);

	status = wmi_unified_lphb_config_tcp_params_cmd(wma_handle->wmi_handle,
				      &hb_tcp_params_fp);
	if (status != EOK) {
		qdf_status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	return qdf_status;
}

/**
 * wma_lphb_conf_tcp_pkt_filter() - configure tcp packet filter command of LPHB
 * @wma_handle: wma handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF status
 */
QDF_STATUS wma_lphb_conf_tcp_pkt_filter(tp_wma_handle wma_handle,
					tSirLPHBReq *lphb_conf_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int status = 0;
	tSirLPHBTcpFilterStruct *ts_lphb_tcp_filter;
	wmi_hb_set_tcp_pkt_filter_cmd_fixed_param hb_tcp_filter_fp = {0};

	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ts_lphb_tcp_filter = &(lphb_conf_req->params.lphbTcpFilterReq);
	WMA_LOGI("%s: WMA --> WMI_HB_SET_TCP_PKT_FILTER length=%d, offset=%d, session=%d, "
		"filter=%2x:%2x:%2x:%2x:%2x:%2x ...", __func__,
		ts_lphb_tcp_filter->length, ts_lphb_tcp_filter->offset,
		ts_lphb_tcp_filter->session, ts_lphb_tcp_filter->filter[0],
		ts_lphb_tcp_filter->filter[1], ts_lphb_tcp_filter->filter[2],
		ts_lphb_tcp_filter->filter[3], ts_lphb_tcp_filter->filter[4],
		ts_lphb_tcp_filter->filter[5]);

	/* fill in values */
	hb_tcp_filter_fp.vdev_id = ts_lphb_tcp_filter->session;
	hb_tcp_filter_fp.length = ts_lphb_tcp_filter->length;
	hb_tcp_filter_fp.offset = ts_lphb_tcp_filter->offset;
	hb_tcp_filter_fp.session = ts_lphb_tcp_filter->session;
	memcpy((void *)&hb_tcp_filter_fp.filter,
	       (void *)&ts_lphb_tcp_filter->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	status = wmi_unified_lphb_config_tcp_pkt_filter_cmd(wma_handle->wmi_handle,
				      &hb_tcp_filter_fp);
	if (status != EOK) {
		qdf_status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	return qdf_status;
}

/**
 * wma_lphb_conf_udp_params() - configure udp param command of LPHB
 * @wma_handle: wma handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF status
 */
QDF_STATUS wma_lphb_conf_udp_params(tp_wma_handle wma_handle,
				    tSirLPHBReq *lphb_conf_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int status = 0;
	tSirLPHBUdpParamStruct *ts_lphb_udp_param;
	wmi_hb_set_udp_params_cmd_fixed_param hb_udp_params_fp = {0};


	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ts_lphb_udp_param = &(lphb_conf_req->params.lphbUdpParamReq);
	WMA_LOGI("%s: WMA --> WMI_HB_SET_UDP_PARAMS srv_ip=%d, dev_ip=%d, src_port=%d, "
		"dst_port=%d, interval=%d, timeout=%d, session=%d, "
		"gateway_mac="MAC_ADDRESS_STR, __func__,
		ts_lphb_udp_param->srv_ip, ts_lphb_udp_param->dev_ip,
		ts_lphb_udp_param->src_port, ts_lphb_udp_param->dst_port,
		ts_lphb_udp_param->interval, ts_lphb_udp_param->timeout,
		ts_lphb_udp_param->session,
		MAC_ADDR_ARRAY(ts_lphb_udp_param->gateway_mac.bytes));


	/* fill in values */
	hb_udp_params_fp.vdev_id = ts_lphb_udp_param->session;
	hb_udp_params_fp.srv_ip = ts_lphb_udp_param->srv_ip;
	hb_udp_params_fp.dev_ip = ts_lphb_udp_param->dev_ip;
	hb_udp_params_fp.src_port = ts_lphb_udp_param->src_port;
	hb_udp_params_fp.dst_port = ts_lphb_udp_param->dst_port;
	hb_udp_params_fp.interval = ts_lphb_udp_param->interval;
	hb_udp_params_fp.timeout = ts_lphb_udp_param->timeout;
	hb_udp_params_fp.session = ts_lphb_udp_param->session;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ts_lphb_udp_param->gateway_mac.bytes,
				   &hb_udp_params_fp.gateway_mac);

	status = wmi_unified_lphb_config_udp_params_cmd(wma_handle->wmi_handle,
				      &hb_udp_params_fp);
	if (status != EOK) {
		qdf_status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	return qdf_status;
}

/**
 * wma_lphb_conf_udp_pkt_filter() - configure udp pkt filter command of LPHB
 * @wma_handle: wma handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF status
 */
QDF_STATUS wma_lphb_conf_udp_pkt_filter(tp_wma_handle wma_handle,
					tSirLPHBReq *lphb_conf_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int status = 0;
	tSirLPHBUdpFilterStruct *ts_lphb_udp_filter;
	wmi_hb_set_udp_pkt_filter_cmd_fixed_param hb_udp_filter_fp = {0};

	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ts_lphb_udp_filter = &(lphb_conf_req->params.lphbUdpFilterReq);
	WMA_LOGI("%s: WMA --> WMI_HB_SET_UDP_PKT_FILTER length=%d, offset=%d, session=%d, "
		"filter=%2x:%2x:%2x:%2x:%2x:%2x ...", __func__,
		ts_lphb_udp_filter->length, ts_lphb_udp_filter->offset,
		ts_lphb_udp_filter->session, ts_lphb_udp_filter->filter[0],
		ts_lphb_udp_filter->filter[1], ts_lphb_udp_filter->filter[2],
		ts_lphb_udp_filter->filter[3], ts_lphb_udp_filter->filter[4],
		ts_lphb_udp_filter->filter[5]);


	/* fill in values */
	hb_udp_filter_fp.vdev_id = ts_lphb_udp_filter->session;
	hb_udp_filter_fp.length = ts_lphb_udp_filter->length;
	hb_udp_filter_fp.offset = ts_lphb_udp_filter->offset;
	hb_udp_filter_fp.session = ts_lphb_udp_filter->session;
	memcpy((void *)&hb_udp_filter_fp.filter,
	       (void *)&ts_lphb_udp_filter->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	status = wmi_unified_lphb_config_udp_pkt_filter_cmd(wma_handle->wmi_handle,
				      &hb_udp_filter_fp);
	if (status != EOK) {
		qdf_status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	return qdf_status;
}

/**
 * wma_process_lphb_conf_req() - handle LPHB configuration requests
 * @wma_handle: wma handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_lphb_conf_req(tp_wma_handle wma_handle,
				     tSirLPHBReq *lphb_conf_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (lphb_conf_req == NULL) {
		WMA_LOGE("%s : LPHB configuration is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGI("%s : LPHB configuration cmd id is %d", __func__,
		 lphb_conf_req->cmd);
	switch (lphb_conf_req->cmd) {
	case LPHB_SET_EN_PARAMS_INDID:
		qdf_status = wma_lphb_conf_hbenable(wma_handle,
						    lphb_conf_req, true);
		break;

	case LPHB_SET_TCP_PARAMS_INDID:
		qdf_status = wma_lphb_conf_tcp_params(wma_handle,
						      lphb_conf_req);
		break;

	case LPHB_SET_TCP_PKT_FILTER_INDID:
		qdf_status = wma_lphb_conf_tcp_pkt_filter(wma_handle,
							  lphb_conf_req);
		break;

	case LPHB_SET_UDP_PARAMS_INDID:
		qdf_status = wma_lphb_conf_udp_params(wma_handle,
						      lphb_conf_req);
		break;

	case LPHB_SET_UDP_PKT_FILTER_INDID:
		qdf_status = wma_lphb_conf_udp_pkt_filter(wma_handle,
							  lphb_conf_req);
		break;

	case LPHB_SET_NETWORK_INFO_INDID:
	default:
		break;
	}

	qdf_mem_free(lphb_conf_req);
	return qdf_status;
}
#endif /* FEATURE_WLAN_LPHB */

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

	WMA_LOGI("%s: WMA --> WMI_PEER_SET_PARAM triggered by DHCP, "
		 "msgType=%s,"
		 "device_mode=%d, macAddr=" MAC_ADDRESS_STR,
		 __func__,
		 ta_dhcp_ind->msgType == WMA_DHCP_START_IND ?
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
	if (status != EOK) {
		return QDF_STATUS_E_FAILURE;
	}

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
	uint16_t bw_val = cds_bw_value(chan_width);

	if (CDS_IS_CHANNEL_24GHZ(chan)) {
		if (((CH_WIDTH_5MHZ == chan_width) ||
		     (CH_WIDTH_10MHZ == chan_width)) &&
		    ((WNI_CFG_DOT11_MODE_11B == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11G == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11N == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_ALL == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AC == dot11_mode)))
			phymode = MODE_11G;
		else {
			switch (dot11_mode) {
			case WNI_CFG_DOT11_MODE_11B:
				if ((20 == bw_val) ||
				    (40 == bw_val))
					phymode = MODE_11B;
				break;
			case WNI_CFG_DOT11_MODE_11G:
			  if ((20 == bw_val) ||
			      (40 == bw_val))
					phymode = MODE_11G;
				break;
			case WNI_CFG_DOT11_MODE_11G_ONLY:
				if ((20 == bw_val) ||
				    (40 == bw_val))
					phymode = MODE_11GONLY;
				break;
			case WNI_CFG_DOT11_MODE_11N:
			case WNI_CFG_DOT11_MODE_11N_ONLY:
				if (20 == bw_val)
					phymode = MODE_11NG_HT20;
				else if (40 == bw_val)
					phymode = MODE_11NG_HT40;
				break;
			case WNI_CFG_DOT11_MODE_ALL:
			case WNI_CFG_DOT11_MODE_11AC:
			case WNI_CFG_DOT11_MODE_11AC_ONLY:
				if (20 == bw_val)
					phymode = MODE_11AC_VHT20_2G;
				else if (40 == bw_val)
					phymode = MODE_11AC_VHT40_2G;
				break;
			default:
				break;
			}
		}
	} else if (CDS_IS_CHANNEL_DSRC(chan))
		phymode = MODE_11A;
	else {
		if (((CH_WIDTH_5MHZ == chan_width) ||
		     (CH_WIDTH_10MHZ == chan_width)) &&
		    ((WNI_CFG_DOT11_MODE_11A == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11N == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_ALL == dot11_mode) ||
		     (WNI_CFG_DOT11_MODE_11AC == dot11_mode)))
			phymode = MODE_11A;
		else {
			switch (dot11_mode) {
			case WNI_CFG_DOT11_MODE_11A:
				if (0 < bw_val)
					phymode = MODE_11A;
				break;
			case WNI_CFG_DOT11_MODE_11N:
			case WNI_CFG_DOT11_MODE_11N_ONLY:
				if (20 == bw_val)
					phymode = MODE_11NA_HT20;
				else if (40 <= bw_val)
					phymode = MODE_11NA_HT40;
				break;
			case WNI_CFG_DOT11_MODE_ALL:
			case WNI_CFG_DOT11_MODE_11AC:
			case WNI_CFG_DOT11_MODE_11AC_ONLY:
				if (20 == bw_val)
					phymode = MODE_11AC_VHT20;
				else if (40 == bw_val)
					phymode = MODE_11AC_VHT40;
				else if (80 == bw_val)
					phymode = MODE_11AC_VHT80;
				else if (CH_WIDTH_160MHZ == chan_width)
					phymode = MODE_11AC_VHT160;
				else if (CH_WIDTH_80P80MHZ == chan_width)
					phymode = MODE_11AC_VHT80_80;
				break;
			default:
				break;
			}
		}
	}

	WMA_LOGD("%s: phymode %d channel %d ch_width %d"
		 "dot11_mode %d", __func__, phymode, chan,
		 chan_width, dot11_mode);

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
	if (!WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				    WMI_SERVICE_ESTIMATE_LINKSPEED)) {
		WMA_LOGE("%s: Linkspeed feature bit not enabled"
			 " Sending value 0 as link speed.", __func__);
		wma_send_link_speed(0);
		return QDF_STATUS_E_FAILURE;
	}

	/* Copy the peer macaddress to the wma buffer */
	WMI_CHAR_ARRAY_TO_MAC_ADDR(pLinkSpeed->peer_macaddr.bytes,
				   &peer_macaddr);

	WMA_LOGD("%s: pLinkSpeed->peerMacAddr: %pM, "
		 "peer_macaddr.mac_addr31to0: 0x%x, peer_macaddr.mac_addr47to32: 0x%x",
		 __func__, pLinkSpeed->peer_macaddr.bytes,
		 peer_macaddr.mac_addr31to0,
		 peer_macaddr.mac_addr47to32);

	if (wmi_unified_get_link_speed_cmd(wma_handle->wmi_handle,
					peer_macaddr)) {
		return QDF_STATUS_E_FAILURE;
	}

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
	WMA_LOGD("added beacon filter = %d", ret);

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
	WMA_LOGD("removed beacon filter = %d", ret);

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

#ifdef FEATURE_GREEN_AP

/**
 * wma_egap_info_status_event() - egap info status event
 * @handle:	pointer to wma handler
 * @event:	pointer to event
 * @len:	len of the event
 *
 * Return:	0 for success, otherwise appropriate error code
 */
static int wma_egap_info_status_event(void *handle, u_int8_t *event,
				      uint32_t len)
{
	WMI_TX_PAUSE_EVENTID_param_tlvs *param_buf;
	wmi_ap_ps_egap_info_event_fixed_param  *egap_info_event;
	wmi_ap_ps_egap_info_chainmask_list *chainmask_event;
	u_int8_t *buf_ptr;

	param_buf = (WMI_TX_PAUSE_EVENTID_param_tlvs *)event;
	if (!param_buf) {
		WMA_LOGE("Invalid EGAP Info status event buffer");
		return -EINVAL;
	}

	egap_info_event = (wmi_ap_ps_egap_info_event_fixed_param  *)
				param_buf->fixed_param;
	buf_ptr = (uint8_t *)egap_info_event;
	buf_ptr += sizeof(wmi_ap_ps_egap_info_event_fixed_param);
	chainmask_event = (wmi_ap_ps_egap_info_chainmask_list *)buf_ptr;

	WMA_LOGI("mac_id: %d, status: %d, tx_mask: %x, rx_mask: %d",
		 chainmask_event->mac_id,
		 egap_info_event->status,
		 chainmask_event->tx_chainmask,
		 chainmask_event->rx_chainmask);
	return 0;
}

/**
 * wma_send_egap_conf_params() - send wmi cmd of egap configuration params
 * @wma_handle:	 wma handler
 * @egap_params: pointer to egap_params
 *
 * Return:	 0 for success, otherwise appropriate error code
 */
QDF_STATUS wma_send_egap_conf_params(WMA_HANDLE handle,
				     struct egap_conf_params *egap_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_ap_ps_egap_param_cmd_fixed_param cmd = {0};
	int32_t err;

	cmd.enable = egap_params->enable;
	cmd.inactivity_time = egap_params->inactivity_time;
	cmd.wait_time = egap_params->wait_time;
	cmd.flags = egap_params->flags;
	err = wmi_unified_egap_conf_params_cmd(wma_handle->wmi_handle, &cmd);
	if (err) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_setup_egap_support() - setup the EGAP support flag
 * @tgt_cfg:  pointer to hdd target configuration
 * @egap_support: EGAP support flag
 *
 * Return:	  None
 */
void wma_setup_egap_support(struct wma_tgt_cfg *tgt_cfg, WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (tgt_cfg && wma_handle)
		tgt_cfg->egap_support = wma_handle->egap_support;
}

/**
 * wma_register_egap_event_handle() - register the EGAP event handle
 * @wma_handle:	wma handler
 *
 * Return:	None
 */
void wma_register_egap_event_handle(WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	QDF_STATUS status;

	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_EGAP)) {
		status = wmi_unified_register_event_handler(
						   wma_handle->wmi_handle,
						   WMI_AP_PS_EGAP_INFO_EVENTID,
						   wma_egap_info_status_event,
						   WMA_RX_SERIALIZER_CTX);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("Failed to register Enhance Green AP event");
			wma_handle->egap_support = false;
		} else {
			WMA_LOGI("Set the Enhance Green AP event handler");
			wma_handle->egap_support = true;
		}
	} else
		wma_handle->egap_support = false;
}
#endif /* FEATURE_GREEN_AP */

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

#ifdef FEATURE_WLAN_LPHB
/**
 * wma_lphb_handler() - send LPHB indication to SME
 * @wma: wma handle
 * @event: event handler
 *
 * Return: 0 for success or error code
 */
static int wma_lphb_handler(tp_wma_handle wma, uint8_t *event)
{
	wmi_hb_ind_event_fixed_param *hb_fp;
	tSirLPHBInd *slphb_indication;
	QDF_STATUS qdf_status;
	cds_msg_t sme_msg = { 0 };

	hb_fp = (wmi_hb_ind_event_fixed_param *) event;
	if (!hb_fp) {
		WMA_LOGE("Invalid wmi_hb_ind_event_fixed_param buffer");
		return -EINVAL;
	}

	WMA_LOGD("lphb indication received with vdev_id=%d, session=%d, reason=%d",
		hb_fp->vdev_id, hb_fp->session, hb_fp->reason);

	slphb_indication = (tSirLPHBInd *) qdf_mem_malloc(sizeof(tSirLPHBInd));

	if (!slphb_indication) {
		WMA_LOGE("Invalid LPHB indication buffer");
		return -ENOMEM;
	}

	slphb_indication->sessionIdx = hb_fp->session;
	slphb_indication->protocolType = hb_fp->reason;
	slphb_indication->eventReason = hb_fp->reason;

	sme_msg.type = eWNI_SME_LPHB_IND;
	sme_msg.bodyptr = slphb_indication;
	sme_msg.bodyval = 0;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Fail to post eWNI_SME_LPHB_IND msg to SME");
		qdf_mem_free(slphb_indication);
		return -EINVAL;
	}

	return 0;
}
#endif /* FEATURE_WLAN_LPHB */

#ifdef FEATURE_WLAN_RA_FILTERING
/**
 * wma_wow_sta_ra_filter() - set RA filter pattern in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
static QDF_STATUS wma_wow_sta_ra_filter(tp_wma_handle wma, uint8_t vdev_id)
{

	struct wma_txrx_node *iface;
	int ret;
	uint8_t default_pattern;

	iface = &wma->interfaces[vdev_id];

	default_pattern = iface->num_wow_default_patterns++;

	WMA_LOGD("%s: send RA rate limit [%d] to fw vdev = %d", __func__,
		 wma->RArateLimitInterval, vdev_id);

	ret = wmi_unified_wow_sta_ra_filter_cmd(wma->wmi_handle, vdev_id,
				   default_pattern, wma->RArateLimitInterval);
	if (ret) {
		WMA_LOGE("%s: Failed to send RA rate limit to fw", __func__);
		iface->num_wow_default_patterns--;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}
#endif /* FEATURE_WLAN_RA_FILTERING */

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
	cds_msg_t cds_msg;
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
	nan_rsp_event = (tSirNanEvent *) qdf_mem_malloc(alloc_len);
	if (NULL == nan_rsp_event) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return -ENOMEM;
	}

	nan_rsp_event->event_data_len = nan_rsp_event_hdr->data_len;
	qdf_mem_copy(nan_rsp_event->event_data, buf_ptr +
		     sizeof(wmi_nan_event_hdr) + WMI_TLV_HDR_SIZE,
		     nan_rsp_event->event_data_len);
	cds_msg.type = eWNI_SME_NAN_EVENT;
	cds_msg.bodyptr = (void *)nan_rsp_event;
	cds_msg.bodyval = 0;

	status = cds_mq_post_message(CDS_MQ_ID_SME, &cds_msg);
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
int wma_nan_rsp_event_handler(void *handle, uint8_t *event_buf,
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
	if ((0 == csa_offload_event->channel) ||
	    (cur_chan == csa_offload_event->channel)) {
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

	WMA_LOGI(FL("Sending OEM_DATA_RSP(len: %d) to upper layer"), datalen);

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


/**
 * wma_unified_dfs_radar_rx_event_handler() - dfs radar rx event handler
 * @handle: wma handle
 * @data: data buffer
 * @datalen: data length
 *
 * WMI handler for WMI_DFS_RADAR_EVENTID
 * This handler is registered for handling
 * filtered DFS Phyerror. This handler is
 * will be invoked only when DFS Phyerr
 * filtering offload is enabled.
 *
 * Return: 1 for Success and 0 for error
 */
static int wma_unified_dfs_radar_rx_event_handler(void *handle,
						  uint8_t *data,
						  uint32_t datalen)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct ieee80211com *ic;
	struct ath_dfs *dfs;
	struct dfs_event *event;
	struct dfs_ieee80211_channel *chan;
	int empty;
	int do_check_chirp = 0;
	int is_hw_chirp = 0;
	int is_sw_chirp = 0;
	int is_pri = 0;
	bool is_ch_dfs = false;

	WMI_DFS_RADAR_EVENTID_param_tlvs *param_tlvs;
	wmi_dfs_radar_event_fixed_param *radar_event;

	ic = wma->dfs_ic;
	if (NULL == ic) {
		WMA_LOGE("%s: dfs_ic is  NULL ", __func__);
		return 0;
	}

	dfs = (struct ath_dfs *)ic->ic_dfs;
	param_tlvs = (WMI_DFS_RADAR_EVENTID_param_tlvs *) data;

	if (NULL == dfs) {
		WMA_LOGE("%s: dfs is  NULL ", __func__);
		return 0;
	}
	/*
	 * This parameter holds the number
	 * of phyerror interrupts to the host
	 * after the phyerrors have passed through
	 * false detect filters in the firmware.
	 */
	dfs->dfs_phyerr_count++;

	if (!param_tlvs) {
		WMA_LOGE("%s: Received NULL data from FW", __func__);
		return 0;
	}

	radar_event = param_tlvs->fixed_param;

	qdf_spin_lock_bh(&ic->chan_lock);
	chan = ic->ic_curchan;
	if (ic->disable_phy_err_processing) {
		WMA_LOGD("%s: radar indication done,drop phyerror event",
			__func__);
		qdf_spin_unlock_bh(&ic->chan_lock);
		return 0;
	}

	if (IEEE80211_IS_CHAN_11AC_VHT160(chan)) {
		is_ch_dfs = true;
	} else if (IEEE80211_IS_CHAN_11AC_VHT80P80(chan)) {
		if (cds_get_channel_state(chan->ic_ieee) == CHANNEL_STATE_DFS ||
		    cds_get_channel_state(chan->ic_ieee_ext -
					  WMA_80MHZ_START_CENTER_CH_DIFF) ==
							CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	} else {
		if (cds_get_channel_state(chan->ic_ieee) == CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	}
	if (!is_ch_dfs) {
		WMA_LOGE
			("%s: Invalid DFS Phyerror event. Channel=%d is Non-DFS",
			__func__, chan->ic_ieee);
		qdf_spin_unlock_bh(&ic->chan_lock);
		return 0;
	}

	qdf_spin_unlock_bh(&ic->chan_lock);
	dfs->ath_dfs_stats.total_phy_errors++;

	if (dfs->dfs_caps.ath_chip_is_bb_tlv) {
		do_check_chirp = 1;
		is_pri = 1;
		is_hw_chirp = radar_event->pulse_is_chirp;

		if ((uint32_t) dfs->dfs_phyerr_freq_min >
		    radar_event->pulse_center_freq) {
			dfs->dfs_phyerr_freq_min =
				(int)radar_event->pulse_center_freq;
		}

		if (dfs->dfs_phyerr_freq_max <
		    (int)radar_event->pulse_center_freq) {
			dfs->dfs_phyerr_freq_max =
				(int)radar_event->pulse_center_freq;
		}
	}

	/*
	 * Now, add the parsed, checked and filtered
	 * radar phyerror event radar pulse event list.
	 * This event will then be processed by
	 * dfs_radar_processevent() to see if the pattern
	 * of pulses in radar pulse list match any radar
	 * singnature in the current regulatory domain.
	 */

	ATH_DFSEVENTQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_eventq));
	ATH_DFSEVENTQ_UNLOCK(dfs);
	if (empty) {
		return 0;
	}
	/*
	 * Add the event to the list, if there's space.
	 */
	ATH_DFSEVENTQ_LOCK(dfs);
	event = STAILQ_FIRST(&(dfs->dfs_eventq));
	if (event == NULL) {
		ATH_DFSEVENTQ_UNLOCK(dfs);
		WMA_LOGE("%s: No more space left for queuing DFS Phyerror events",
			__func__);
		return 0;
	}
	STAILQ_REMOVE_HEAD(&(dfs->dfs_eventq), re_list);
	ATH_DFSEVENTQ_UNLOCK(dfs);
	dfs->dfs_phyerr_queued_count++;
	dfs->dfs_phyerr_w53_counter++;
	event->re_dur = (uint8_t) radar_event->pulse_duration;
	event->re_rssi = radar_event->rssi;
	event->re_ts = radar_event->pulse_detect_ts & DFS_TSMASK;
	event->re_full_ts = (((uint64_t) radar_event->upload_fullts_high) << 32)
			    | radar_event->upload_fullts_low;

	/*
	 * Index of peak magnitude
	 */
	event->sidx = radar_event->peak_sidx;

	/*
	 * Handle chirp flags.
	 */
	if (do_check_chirp) {
		event->re_flags |= DFS_EVENT_CHECKCHIRP;
		if (is_hw_chirp) {
			event->re_flags |= DFS_EVENT_HW_CHIRP;
		}
		if (is_sw_chirp) {
			event->re_flags |= DFS_EVENT_SW_CHIRP;
		}
	}
	/*
	 * Correctly set which channel is being reported on
	 */
	if (is_pri) {
		event->re_chanindex = (uint8_t) dfs->dfs_curchan_radindex;
	} else {
		if (dfs->dfs_extchan_radindex == -1) {
			WMA_LOGI("%s phyerr on ext channel", __func__);
		}
		event->re_chanindex = (uint8_t) dfs->dfs_extchan_radindex;
		WMA_LOGI("%s:New extension channel event is added to queue",
			 __func__);
	}

	ATH_DFSQ_LOCK(dfs);

	STAILQ_INSERT_TAIL(&(dfs->dfs_radarq), event, re_list);

	empty = STAILQ_EMPTY(&dfs->dfs_radarq);

	ATH_DFSQ_UNLOCK(dfs);

	if (!empty && !dfs->ath_radar_tasksched) {
		dfs->ath_radar_tasksched = 1;
		OS_SET_TIMER(&dfs->ath_dfs_task_timer, 0);
	}

	return 1;

}

/**
 * wma_unified_phyerr_rx_event_handler() - phyerr event handler
 * @handle: wma handle
 * @data: data buffer
 * @datalen: buffer length
 *
 * WMI Handler for WMI_PHYERR_EVENTID event from firmware.
 * This handler is currently handling only DFS phy errors.
 * This handler will be invoked only when the DFS phyerror
 * filtering offload is disabled.
 *
 * Return:  1:Success, 0:Failure
 */
static int wma_unified_phyerr_rx_event_handler(void *handle,
					       uint8_t *data, uint32_t datalen)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_PHYERR_EVENTID_param_tlvs *param_tlvs;
	wmi_comb_phyerr_rx_hdr *pe_hdr;
	uint8_t *bufp;
	wmi_single_phyerr_rx_event *ev;
	struct ieee80211com *ic = wma->dfs_ic;
	qdf_size_t n;
	A_UINT64 tsf64 = 0;
	int phy_err_code = 0;
	A_UINT32 phy_err_mask = 0;
	int error = 0;
	tpAniSirGlobal mac_ctx =
		(tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);
	bool enable_log = false;
	int max_dfs_buf_length = 0;

	if (NULL == mac_ctx) {
		WMA_LOGE("%s: mac_ctx is NULL", __func__);
		return 0;
	}
	enable_log = mac_ctx->sap.enable_dfs_phy_error_logs;

	param_tlvs = (WMI_PHYERR_EVENTID_param_tlvs *) data;

	if (!param_tlvs) {
		WMA_LOGE("%s: Received NULL data from FW", __func__);
		return 0;
	}

	pe_hdr = param_tlvs->hdr;
	if (pe_hdr == NULL) {
		WMA_LOGE("%s: Received Data PE Header is NULL", __func__);
		return 0;
	}

	/* Ensure it's at least the size of the header */
	if (datalen < sizeof(*pe_hdr)) {
		WMA_LOGE("%s:  Expected minimum size %zu, received %d",
			 __func__, sizeof(*pe_hdr), datalen);
		return 0;
	}
	/*
	 * The max buffer lenght is larger for DFS-3 than DFS-2.
	 * So, accordingly use the correct max buffer size.
	 */
	if (wma->hw_bd_id != WMI_HWBD_QCA6174)
		max_dfs_buf_length = DFS3_MAX_BUF_LENGTH;
	else
		max_dfs_buf_length = DFS_MAX_BUF_LENGTH;

	if (pe_hdr->buf_len > max_dfs_buf_length) {
		WMA_LOGE("%s: Received Invalid Phyerror event buffer length = %d"
			"Maximum allowed buf length = %d", __func__,
			pe_hdr->buf_len, max_dfs_buf_length);

		return 0;
	}

	/*
	 * Reconstruct the 64 bit event TSF. This isn't from the MAC, it's
	 * at the time the event was sent to us, the TSF value will be
	 * in the future.
	 */
	tsf64 = pe_hdr->tsf_l32;
	tsf64 |= (((uint64_t) pe_hdr->tsf_u32) << 32);

	/*
	 * Check the HW board ID to figure out
	 * if DFS-3 is supported. In DFS-3
	 * phyerror mask indicates the type of
	 * phyerror, whereas in DFS-2 phyerrorcode
	 * indicates the type of phyerror. If the
	 * board is NOT WMI_HWBD_QCA6174, for now
	 * assume that it supports DFS-3.
	 */
	if (wma->hw_bd_id != WMI_HWBD_QCA6174) {
		phy_err_mask = pe_hdr->rsPhyErrMask0;
		WMA_LOGD("%s: DFS-3 phyerror mask = 0x%x",
			  __func__, phy_err_mask);
	}

	/*
	 * Loop over the bufp, extracting out phyerrors
	 * wmi_unified_comb_phyerr_rx_event.bufp is a char pointer,
	 * which isn't correct here - what we have received here
	 * is an array of TLV-style PHY errors.
	 */
	n = 0;                  /* Start just after the header */
	bufp = param_tlvs->bufp;
	while (n < pe_hdr->buf_len) {
		/* ensure there's at least space for the header */
		if ((pe_hdr->buf_len - n) < sizeof(ev->hdr)) {
			WMA_LOGE("%s: Not enough space.(datalen=%d, n=%zu, hdr=%zu bytes",
				__func__, pe_hdr->buf_len, n, sizeof(ev->hdr));
			error = 1;
			break;
		}
		/*
		 * Obtain a pointer to the beginning of the current event.
		 * data[0] is the beginning of the WMI payload.
		 */
		ev = (wmi_single_phyerr_rx_event *) &bufp[n];

		/*
		 * Sanity check the buffer length of the event against
		 * what we currently have.
		 * Since buf_len is 32 bits, we check if it overflows
		 * a large 32 bit value. It's not 0x7fffffff because
		 * we increase n by (buf_len + sizeof(hdr)), which would
		 * in itself cause n to overflow.
		 * If "int" is 64 bits then this becomes a moot point.
		 */
		if (ev->hdr.buf_len > 0x7f000000) {
			WMA_LOGE("%s:buf_len is garbage (0x%x)", __func__,
				 ev->hdr.buf_len);
			error = 1;
			break;
		}
		if (n + ev->hdr.buf_len > pe_hdr->buf_len) {
			WMA_LOGE("%s: buf_len exceeds available space n=%zu,"
				 "buf_len=%d, datalen=%d",
				 __func__, n, ev->hdr.buf_len, pe_hdr->buf_len);
			error = 1;
			break;
		}
		/*
		 * If the board id is WMI_HWBD_QCA6174
		 * then it supports only DFS-2. So, fetch
		 * phyerror code in order to know the type
		 * of phyerror.
		 */
		if (wma->hw_bd_id == WMI_HWBD_QCA6174) {
			phy_err_code = WMI_UNIFIED_PHYERRCODE_GET(&ev->hdr);
			WMA_LOGD("%s: DFS-2 phyerror code = 0x%x",
				  __func__, phy_err_code);
		}

		/*
		 * phy_err_code is set for DFS-2 and phy_err_mask
		 * is set for DFS-3. Checking both to support
		 * compatability for older platforms.
		 * If the phyerror or phyerrmask category matches,
		 * pass radar events to the dfs pattern matching code.
		 * Don't pass radar events with no buffer payload.
		 */
		if (((phy_err_mask & WMI_PHY_ERROR_MASK0_RADAR) ||
		     (phy_err_mask & WMI_PHY_ERROR_MASK0_FALSE_RADAR_EXT)) ||
		    (phy_err_code == WMA_DFS2_PHYERROR_CODE ||
		     phy_err_code == WMA_DFS2_FALSE_RADAR_EXT)) {
			if (ev->hdr.buf_len > 0) {
				/* Calling in to the DFS module to process the phyerr */
				dfs_process_phyerr(ic, &ev->bufp[0],
						   ev->hdr.buf_len,
						   WMI_UNIFIED_RSSI_COMB_GET
							   (&ev->hdr) & 0xff,
						   /* Extension RSSI */
						   WMI_UNIFIED_RSSI_COMB_GET
							   (&ev->hdr) & 0xff,
						   ev->hdr.tsf_timestamp,
						   tsf64, enable_log);
			}
		}

		/*
		 * Advance the buffer pointer to the next PHY error.
		 * buflen is the length of this payload, so we need to
		 * advance past the current header _AND_ the payload.
		 */
		n += sizeof(*ev) + ev->hdr.buf_len;

	} /*end while() */
	if (error)
		return 0;
	else
		return 1;
}

/**
 * wma_register_dfs_event_handler() - register dfs event handler
 * @wma_handle: wma handle
 *
 * Register appropriate dfs phyerror event handler
 * based on phyerror filtering offload is enabled
 * or disabled.
 *
 * Return: none
 */
void wma_register_dfs_event_handler(tp_wma_handle wma_handle)
{
	if (NULL == wma_handle) {
		WMA_LOGE("%s:wma_handle is NULL", __func__);
		return;
	}

	if (false == wma_handle->dfs_phyerr_filter_offload) {
		/*
		 * Register the wma_unified_phyerr_rx_event_handler
		 * for filtering offload disabled case to handle
		 * the DFS phyerrors.
		 */
		WMA_LOGD("%s:Phyerror Filtering offload is Disabled in ini",
			 __func__);
		wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_PHYERR_EVENTID,
					wma_unified_phyerr_rx_event_handler,
					WMA_RX_WORK_CTX);
		WMA_LOGD("%s: WMI_PHYERR_EVENTID event handler registered",
			 __func__);
	} else {
		WMA_LOGD("%s:Phyerror Filtering offload is Enabled in ini",
			 __func__);
		wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_DFS_RADAR_EVENTID,
					wma_unified_dfs_radar_rx_event_handler,
					WMA_RX_WORK_CTX);
		WMA_LOGD("%s:WMI_DFS_RADAR_EVENTID event handler registered",
			 __func__);
	}

	return;
}


/**
 * wma_unified_dfs_phyerr_filter_offload_enable() - enable dfs phyerr filter
 * @wma_handle: wma handle
 *
 * Send WMI_DFS_PHYERR_FILTER_ENA_CMDID or
 * WMI_DFS_PHYERR_FILTER_DIS_CMDID command
 * to firmware based on phyerr filtering
 * offload status.
 *
 * Return: 1 success, 0 failure
 */
int
wma_unified_dfs_phyerr_filter_offload_enable(tp_wma_handle wma_handle)
{
	int ret;

	if (NULL == wma_handle) {
		WMA_LOGE("%s:wma_handle is NULL", __func__);
		return 0;
	}

	ret = wmi_unified_dfs_phyerr_filter_offload_en_cmd(wma_handle->wmi_handle,
					   wma_handle->dfs_phyerr_filter_offload);
	if (ret)
		return QDF_STATUS_E_FAILURE;


	return QDF_STATUS_SUCCESS;
}

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

	/*Check if packet log is enabled in cfg.ini */
	if (!cds_is_packet_log_enabled()) {
		WMA_LOGE("%s:pkt log is not enabled in cfg.ini", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_pktlog_wmi_send_cmd(wma_handle->wmi_handle,
					   params->pktlog_event,
					   params->cmd_id);
	if (ret)
		return QDF_STATUS_E_FAILURE;


	return QDF_STATUS_SUCCESS;
}
#endif /* REMOVE_PKT_LOG */

static void wma_send_status_to_suspend_ind(tp_wma_handle wma, bool suspended)
{
	tSirReadyToSuspendInd *ready_to_suspend;
	QDF_STATUS status;
	cds_msg_t cds_msg;
	uint8_t len;

	WMA_LOGD("Posting ready to suspend indication to umac");

	len = sizeof(tSirReadyToSuspendInd);
	ready_to_suspend = (tSirReadyToSuspendInd *) qdf_mem_malloc(len);

	if (NULL == ready_to_suspend) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return;
	}

	ready_to_suspend->mesgType = eWNI_SME_READY_TO_SUSPEND_IND;
	ready_to_suspend->mesgLen = len;
	ready_to_suspend->suspended = suspended;

	cds_msg.type = eWNI_SME_READY_TO_SUSPEND_IND;
	cds_msg.bodyptr = (void *)ready_to_suspend;
	cds_msg.bodyval = 0;

	status = cds_mq_post_message(CDS_MQ_ID_SME, &cds_msg);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to post ready to suspend");
		qdf_mem_free(ready_to_suspend);
	}
}

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
#ifdef FEATURE_WLAN_LPHB
	case WOW_REASON_WLAN_HB:
		return "WLAN_HB";
#endif /* FEATURE_WLAN_LPHB */

	case WOW_REASON_CSA_EVENT:
		return "CSA_EVENT";
	case WOW_REASON_PROBE_REQ_WPS_IE_RECV:
		return "PROBE_REQ_RECV";
	case WOW_REASON_AUTH_REQ_RECV:
		return "AUTH_REQ_RECV";
	case WOW_REASON_ASSOC_REQ_RECV:
		return "ASSOC_REQ_RECV";
	case WOW_REASON_HTT_EVENT:
		return "WOW_REASON_HTT_EVENT";
#ifdef FEATURE_WLAN_RA_FILTERING
	case WOW_REASON_RA_MATCH:
		return "WOW_REASON_RA_MATCH";
#endif /* FEATURE_WLAN_RA_FILTERING */
	case WOW_REASON_BEACON_RECV:
		return "WOW_REASON_IBSS_BEACON_RECV";
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case WOW_REASON_HOST_AUTO_SHUTDOWN:
		return "WOW_REASON_HOST_AUTO_SHUTDOWN";
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case WOW_REASON_ROAM_HO:
		return "WOW_REASON_ROAM_HO";
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#ifdef FEATURE_WLAN_EXTSCAN
	case WOW_REASON_EXTSCAN:
		return "WOW_REASON_EXTSCAN";
#endif
	case WOW_REASON_RSSI_BREACH_EVENT:
		return "WOW_REASON_RSSI_BREACH_EVENT";
	case WOW_REASON_NLO_SCAN_COMPLETE:
		return "WOW_REASON_NLO_SCAN_COMPLETE";
	case WOW_REASON_NAN_EVENT:
		return "WOW_REASON_NAN_EVENT";
	case WOW_REASON_OEM_RESPONSE_EVENT:
		return "WOW_OEM_RESPONSE_EVENT";
	case WOW_REASON_ASSOC_RES_RECV:
		return "ASSOC_RES_RECV";
	case WOW_REASON_REASSOC_REQ_RECV:
		return "REASSOC_REQ_RECV";
	case WOW_REASON_REASSOC_RES_RECV:
		return "REASSOC_RES_RECV";
	case WOW_REASON_ACTION_FRAME_RECV:
		return "ACTION_FRAME_RECV";
	case WOW_REASON_BPF_ALLOW:
		return "WOW_REASON_BPF_ALLOW";
	}
	return "unknown";
}

/**
 * wma_wow_wake_up_stats_display() - display wow wake up stats
 * @wma: Pointer to wma handle
 *
 * Return: none
 */
static void wma_wow_wake_up_stats_display(tp_wma_handle wma)
{
	WMA_LOGA("uc %d bc %d v4_mc %d v6_mc %d ra %d ns %d na %d pno_match %d pno_complete %d gscan %d low_rssi %d rssi_breach %d icmp %d icmpv6 %d oem %d",
		wma->wow_ucast_wake_up_count,
		wma->wow_bcast_wake_up_count,
		wma->wow_ipv4_mcast_wake_up_count,
		wma->wow_ipv6_mcast_wake_up_count,
		wma->wow_ipv6_mcast_ra_stats,
		wma->wow_ipv6_mcast_ns_stats,
		wma->wow_ipv6_mcast_na_stats,
		wma->wow_pno_match_wake_up_count,
		wma->wow_pno_complete_wake_up_count,
		wma->wow_gscan_wake_up_count,
		wma->wow_low_rssi_wake_up_count,
		wma->wow_rssi_breach_wake_up_count,
		wma->wow_icmpv4_count,
		wma->wow_icmpv6_uc_bc_count,
		wma->wow_oem_response_wake_up_count);

	return;
}

/**
 * wma_wow_ipv6_mcast_stats() - ipv6 mcast wake up stats
 * @wma: Pointer to wma handle
 * @data: Pointer to pattern match data
 *
 * Return: none
 */
static void wma_wow_ipv6_mcast_stats(tp_wma_handle wma, uint8_t *data)
{
	static const uint8_t ipv6_mcast[] = {0x86, 0xDD};

	if (!memcmp(ipv6_mcast, (data + WMA_ETHER_TYPE_OFFSET),
						sizeof(ipv6_mcast))) {
		if (WMA_ICMP_V6_HEADER_TYPE ==
			*(data + WMA_ICMP_V6_HEADER_OFFSET)) {
			if (WMA_ICMP_V6_RA_TYPE ==
				*(data + WMA_ICMP_V6_TYPE_OFFSET))
				wma->wow_ipv6_mcast_ra_stats++;
			else if (WMA_ICMP_V6_NS_TYPE ==
				*(data + WMA_ICMP_V6_TYPE_OFFSET))
				wma->wow_ipv6_mcast_ns_stats++;
			else if (WMA_ICMP_V6_NA_TYPE ==
				*(data + WMA_ICMP_V6_TYPE_OFFSET))
				wma->wow_ipv6_mcast_na_stats++;
			else
				WMA_LOGA("ICMP V6 type : 0x%x",
					*(data + WMA_ICMP_V6_TYPE_OFFSET));
		} else {
			WMA_LOGA("ICMP_V6 header 0x%x",
				*(data + WMA_ICMP_V6_HEADER_OFFSET));
		}
	} else {
		WMA_LOGA("Ethertype x%x:0x%x",
			*(data + WMA_ETHER_TYPE_OFFSET),
			*(data + WMA_ETHER_TYPE_OFFSET + 1));
	}

	return;
}

/**
 * wma_wow_wake_up_stats() - maintain wow pattern match wake up stats
 * @wma: Pointer to wma handle
 * @data: Pointer to pattern match data
 * @len: Pattern match data length
 * @event: Wake up event
 *
 * Return: none
 */
static void wma_wow_wake_up_stats(tp_wma_handle wma, uint8_t *data,
	int32_t len, WOW_WAKE_REASON_TYPE event)
{
	switch (event) {

	case WOW_REASON_PATTERN_MATCH_FOUND:
		if (WMA_BCAST_MAC_ADDR == *data) {
			wma->wow_bcast_wake_up_count++;
			if (qdf_nbuf_data_is_ipv4_pkt(data) &&
			    (WMA_ICMP_PROTOCOL == *(data + WMA_IPV4_PROTOCOL)))
				wma->wow_icmpv4_count++;
			else if (qdf_nbuf_data_is_ipv6_pkt(data) &&
			    (len > WMA_ICMP_V6_TYPE_OFFSET) &&
			    (WMA_ICMP_V6_HEADER_TYPE ==
			     *(data + WMA_ICMP_V6_HEADER_OFFSET)))
				wma->wow_icmpv6_uc_bc_count++;
		} else if (WMA_MCAST_IPV4_MAC_ADDR == *data) {
			wma->wow_ipv4_mcast_wake_up_count++;
			if (WMA_ICMP_PROTOCOL == *(data + WMA_IPV4_PROTOCOL))
				wma->wow_icmpv4_count++;
		} else if (WMA_MCAST_IPV6_MAC_ADDR == *data) {
			wma->wow_ipv6_mcast_wake_up_count++;
			if (len > WMA_ICMP_V6_TYPE_OFFSET)
				wma_wow_ipv6_mcast_stats(wma, data);
			else
				WMA_LOGA("ICMP_V6 data len %d", len);
		} else {
			wma->wow_ucast_wake_up_count++;
			if (qdf_nbuf_data_is_ipv4_pkt(data) &&
			    (WMA_ICMP_PROTOCOL == *(data + WMA_IPV4_PROTOCOL)))
				wma->wow_icmpv4_count++;
			else if (qdf_nbuf_data_is_ipv6_pkt(data) &&
			    (len > WMA_ICMP_V6_TYPE_OFFSET) &&
			    (WMA_ICMP_V6_HEADER_TYPE ==
			     *(data + WMA_ICMP_V6_HEADER_OFFSET)))
				wma->wow_icmpv6_uc_bc_count++;
		}
		break;

	case WOW_REASON_RA_MATCH:
		wma->wow_ipv6_mcast_ra_stats++;
		break;

	case WOW_REASON_NLOD:
		wma->wow_pno_match_wake_up_count++;
		break;

	case WOW_REASON_NLO_SCAN_COMPLETE:
		wma->wow_pno_complete_wake_up_count++;
		break;

	case WOW_REASON_LOW_RSSI:
		wma->wow_low_rssi_wake_up_count++;
		break;

	case WOW_REASON_EXTSCAN:
		wma->wow_gscan_wake_up_count++;
		break;

	case WOW_REASON_RSSI_BREACH_EVENT:
		wma->wow_rssi_breach_wake_up_count++;
		break;
	case WOW_REASON_OEM_RESPONSE_EVENT:
		wma->wow_oem_response_wake_up_count++;
		break;

	default:
		WMA_LOGE("Unknown wake up reason");
		break;
	}

	wma_wow_wake_up_stats_display(wma);
	return;
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

	case WMITLV_TAG_STRUC_wmi_extscan_hotlist_ssid_match_event_fixed_param:
		event_id = WMI_EXTSCAN_HOTLIST_SSID_MATCH_EVENTID;
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
	uint32_t event_id;

	switch (reason) {
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
	default:
		WMA_LOGD(FL("Unexpected WOW reason : %s(%d)"),
			 wma_wow_wake_reason_str(reason), reason);
		event_id = 0;
		break;
	}

	return event_id;
}

/**
 * tlv_check_required() - tells whether to check the wow packet buffer
 *                        for proper TLV structure.
 * @reason: WOW reason
 *
 * In most cases, wow wake up event carries the actual event buffer in
 * wow_packet_buffer with some exceptions. This function is used to
 * determine when to check for the TLVs in wow_packet_buffer.
 *
 * Return: true if check is required and false otherwise.
 */
static bool tlv_check_required(int32_t reason)
{
	switch (reason) {
	case WOW_REASON_PATTERN_MATCH_FOUND:
		return false;
	default:
		return true;
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
const char *
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
	case QDF_PROTO_IPV4_UDP:
		return "IPV4 UDP Packet";
	case QDF_PROTO_IPV4_TCP:
		return "IPV4 TCP Packet";
	case QDF_PROTO_IPV6_UDP:
		return "IPV6 UDP Packet";
	case QDF_PROTO_IPV6_TCP:
		return "IPV6 TCP Packet";
	default:
		return "Invalid Packet";
	}
}

/**
 * wma_wow_get_pkt_proto_subtype() - get the proto subtype
 *            of the packet.
 * @data: Pointer to data buffer
 * @len: length of the data buffer
 *
 * This function gives the proto subtype of the packet.
 *
 * Return: proto subtype of the packet.
 */
static enum qdf_proto_subtype
wma_wow_get_pkt_proto_subtype(uint8_t *data,
			uint32_t len)
{
	uint16_t ether_type = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	WMA_LOGE("Ether Type: 0x%04x",
		ani_cpu_to_be16(ether_type));

	if (QDF_NBUF_TRAC_EAPOL_ETH_TYPE ==
		   ani_cpu_to_be16(ether_type)) {
		if (len >= WMA_EAPOL_SUBTYPE_GET_MIN_LEN)
			return qdf_nbuf_data_get_eapol_subtype(data);
		QDF_TRACE(QDF_MODULE_ID_WMA,
			QDF_TRACE_LEVEL_ERROR, "EAPOL Packet");
		return QDF_PROTO_INVALID;
	} else if (QDF_NBUF_TRAC_ARP_ETH_TYPE ==
		   ani_cpu_to_be16(ether_type)) {
		if (len >= WMA_ARP_SUBTYPE_GET_MIN_LEN)
			return qdf_nbuf_data_get_arp_subtype(data);
		QDF_TRACE(QDF_MODULE_ID_WMA,
			QDF_TRACE_LEVEL_ERROR, "ARP Packet");
		return QDF_PROTO_INVALID;
	} else if (QDF_NBUF_TRAC_IPV4_ETH_TYPE ==
		   ani_cpu_to_be16(ether_type)) {
		if (len >= WMA_IPV4_PROTO_GET_MIN_LEN) {
			uint8_t proto_type;

			proto_type = qdf_nbuf_data_get_ipv4_proto(data);
			WMA_LOGE("IPV4_proto_type: %u", proto_type);
			if (proto_type == QDF_NBUF_TRAC_ICMP_TYPE) {
				if (len >= WMA_ICMP_SUBTYPE_GET_MIN_LEN)
					return qdf_nbuf_data_get_icmp_subtype(
							data);
				QDF_TRACE(QDF_MODULE_ID_WMA,
					QDF_TRACE_LEVEL_ERROR, "ICMP Packet");
				return QDF_PROTO_INVALID;
			} else if (proto_type == QDF_NBUF_TRAC_UDP_TYPE) {
				if (len >= WMA_IS_DHCP_GET_MIN_LEN) {
					if (qdf_nbuf_data_is_ipv4_dhcp_pkt(data)) {
						if (len >=
						   WMA_DHCP_SUBTYPE_GET_MIN_LEN)
						  return qdf_nbuf_data_get_dhcp_subtype(data);
						QDF_TRACE(QDF_MODULE_ID_WMA,
						    QDF_TRACE_LEVEL_ERROR,
						    "DHCP Packet");
						return QDF_PROTO_INVALID;
					}
				}
				return QDF_PROTO_IPV4_UDP;
			} else if (proto_type == QDF_NBUF_TRAC_TCP_TYPE) {
				return QDF_PROTO_IPV4_TCP;
			}
		}
		QDF_TRACE(QDF_MODULE_ID_WMA,
			QDF_TRACE_LEVEL_ERROR, "IPV4 Packet");
		return QDF_PROTO_INVALID;
	} else if (QDF_NBUF_TRAC_IPV6_ETH_TYPE ==
		   ani_cpu_to_be16(ether_type)) {
		if (len >= WMA_IPV6_PROTO_GET_MIN_LEN) {
			uint8_t proto_type;

			proto_type = qdf_nbuf_data_get_ipv6_proto(data);
			WMA_LOGE("IPV6_proto_type: %u", proto_type);
			if (proto_type == QDF_NBUF_TRAC_ICMPV6_TYPE) {
				if (len >= WMA_ICMPV6_SUBTYPE_GET_MIN_LEN)
					return qdf_nbuf_data_get_icmpv6_subtype(
							data);
				QDF_TRACE(QDF_MODULE_ID_WMA,
					QDF_TRACE_LEVEL_ERROR, "ICMPV6 Packet");
				return QDF_PROTO_INVALID;
			} else if (proto_type == QDF_NBUF_TRAC_UDP_TYPE) {
				return QDF_PROTO_IPV6_UDP;
			} else if (proto_type == QDF_NBUF_TRAC_TCP_TYPE) {
				return QDF_PROTO_IPV6_TCP;
			}
		}
		QDF_TRACE(QDF_MODULE_ID_WMA,
			QDF_TRACE_LEVEL_ERROR, "IPV6 Packet");
		return QDF_PROTO_INVALID;
	}

	return QDF_PROTO_INVALID;
}

/**
 * wma_wow_parse_data_pkt_buffer() - API to parse data buffer for data
 *    packet that resulted in WOW wakeup.
 * @data: Pointer to data buffer
 * @buf_len: data buffer length
 *
 * This function parses the data buffer received (first few bytes of
 * skb->data) to get informaton like src mac addr, dst mac addr, packet
 * len, seq_num, etc.
 *
 * Return: void
 */
static void wma_wow_parse_data_pkt_buffer(uint8_t *data,
			uint32_t buf_len)
{
	enum qdf_proto_subtype proto_subtype;
	uint16_t pkt_len, key_len, seq_num;
	uint16_t src_port, dst_port;
	uint32_t transaction_id, tcp_seq_num;

	WMA_LOGD("wow_buf_pkt_len: %u", buf_len);
	if (buf_len >= QDF_NBUF_TRAC_IPV4_OFFSET)
		WMA_LOGE("Src_mac: " MAC_ADDRESS_STR " Dst_mac: " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(data),
			MAC_ADDR_ARRAY(data + QDF_NBUF_SRC_MAC_OFFSET));
	else
		goto end;

	proto_subtype = wma_wow_get_pkt_proto_subtype(data, buf_len);
	switch (proto_subtype) {
	case QDF_PROTO_EAPOL_M1:
	case QDF_PROTO_EAPOL_M2:
	case QDF_PROTO_EAPOL_M3:
	case QDF_PROTO_EAPOL_M4:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_EAPOL_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				EAPOL_PKT_LEN_OFFSET));
			key_len = (uint16_t)(*(uint16_t *)(data +
				EAPOL_KEY_LEN_OFFSET));
			WMA_LOGE("Pkt_len: %u, Key_len: %u",
				ani_cpu_to_be16(pkt_len),
				ani_cpu_to_be16(key_len));
		}
		break;

	case QDF_PROTO_DHCP_DISCOVER:
	case QDF_PROTO_DHCP_REQUEST:
	case QDF_PROTO_DHCP_OFFER:
	case QDF_PROTO_DHCP_ACK:
	case QDF_PROTO_DHCP_NACK:
	case QDF_PROTO_DHCP_RELEASE:
	case QDF_PROTO_DHCP_INFORM:
	case QDF_PROTO_DHCP_DECLINE:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_DHCP_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				DHCP_PKT_LEN_OFFSET));
			transaction_id = (uint32_t)(*(uint32_t *)(data +
				DHCP_TRANSACTION_ID_OFFSET));
			WMA_LOGE("Pkt_len: %u, Transaction_id: %u",
				ani_cpu_to_be16(pkt_len),
				ani_cpu_to_be16(transaction_id));
		}
		break;

	case QDF_PROTO_ARP_REQ:
	case QDF_PROTO_ARP_RES:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		break;

	case QDF_PROTO_ICMP_REQ:
	case QDF_PROTO_ICMP_RES:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_IPV4_PKT_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				IPV4_PKT_LEN_OFFSET));
			seq_num = (uint16_t)(*(uint16_t *)(data +
				ICMP_SEQ_NUM_OFFSET));
			WMA_LOGE("Pkt_len: %u, Seq_num: %u",
				ani_cpu_to_be16(pkt_len),
				ani_cpu_to_be16(seq_num));
		}
		break;

	case QDF_PROTO_ICMPV6_REQ:
	case QDF_PROTO_ICMPV6_RES:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_IPV6_PKT_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				IPV6_PKT_LEN_OFFSET));
			seq_num = (uint16_t)(*(uint16_t *)(data +
				ICMPV6_SEQ_NUM_OFFSET));
			WMA_LOGE("Pkt_len: %u, Seq_num: %u",
				ani_cpu_to_be16(pkt_len),
				ani_cpu_to_be16(seq_num));
		}
		break;

	case QDF_PROTO_IPV4_UDP:
	case QDF_PROTO_IPV4_TCP:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_IPV4_PKT_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				IPV4_PKT_LEN_OFFSET));
			src_port = (uint16_t)(*(uint16_t *)(data +
				IPV4_SRC_PORT_OFFSET));
			dst_port = (uint16_t)(*(uint16_t *)(data +
				IPV4_DST_PORT_OFFSET));
			WMA_LOGE("Pkt_len: %u",
				ani_cpu_to_be16(pkt_len));
			WMA_LOGE("src_port: %u, dst_port: %u",
				ani_cpu_to_be16(src_port),
				ani_cpu_to_be16(dst_port));
			if (proto_subtype == QDF_PROTO_IPV4_TCP) {
				tcp_seq_num = (uint32_t)(*(uint32_t *)(data +
					IPV4_TCP_SEQ_NUM_OFFSET));
				WMA_LOGE("TCP_seq_num: %u",
					ani_cpu_to_be16(tcp_seq_num));
			}
		}
		break;

	case QDF_PROTO_IPV6_UDP:
	case QDF_PROTO_IPV6_TCP:
		WMA_LOGE("WOW Wakeup: %s rcvd",
			wma_pkt_proto_subtype_to_string(proto_subtype));
		if (buf_len >= WMA_IPV6_PKT_INFO_GET_MIN_LEN) {
			pkt_len = (uint16_t)(*(uint16_t *)(data +
				IPV6_PKT_LEN_OFFSET));
			src_port = (uint16_t)(*(uint16_t *)(data +
				IPV6_SRC_PORT_OFFSET));
			dst_port = (uint16_t)(*(uint16_t *)(data +
				IPV6_DST_PORT_OFFSET));
			WMA_LOGE("Pkt_len: %u",
				ani_cpu_to_be16(pkt_len));
			WMA_LOGE("src_port: %u, dst_port: %u",
				ani_cpu_to_be16(src_port),
				ani_cpu_to_be16(dst_port));
			if (proto_subtype == QDF_PROTO_IPV6_TCP) {
				tcp_seq_num = (uint32_t)(*(uint32_t *)(data +
					IPV6_TCP_SEQ_NUM_OFFSET));
				WMA_LOGE("TCP_seq_num: %u",
					ani_cpu_to_be16(tcp_seq_num));
			}
		}
		break;

	default:
end:
		WMA_LOGE("wow_buf_pkt_len: %u", buf_len);
		WMA_LOGE("Invalid Packet Type or Smaller WOW packet buffer than expected");
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
		(wow_packet_buffer + 4);
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
 * wma_wow_get_wakelock_duration() - return the wakelock duration
 *        for some mgmt packets received.
 * @wake_reason: wow wakeup reason
 *
 * This function returns the wakelock duration for some mgmt packets
 * received while in wow suspend.
 *
 * Return: wakelock duration
 */
static uint32_t wma_wow_get_wakelock_duration(int wake_reason)
{
	uint32_t wake_lock_duration = 0;

	switch (wake_reason) {
	case WOW_REASON_AUTH_REQ_RECV:
		wake_lock_duration = WMA_AUTH_REQ_RECV_WAKE_LOCK_TIMEOUT;
		break;
	case WOW_REASON_ASSOC_REQ_RECV:
		wake_lock_duration = WMA_ASSOC_REQ_RECV_WAKE_LOCK_DURATION;
		break;
	case WOW_REASON_DEAUTH_RECVD:
		wake_lock_duration = WMA_DEAUTH_RECV_WAKE_LOCK_DURATION;
		break;
	case WOW_REASON_DISASSOC_RECVD:
		wake_lock_duration = WMA_DISASSOC_RECV_WAKE_LOCK_DURATION;
		break;
	default:
		break;
	}

	return wake_lock_duration;
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
 * Return: 0 for success or error
 */
int wma_wow_wakeup_host_event(void *handle, uint8_t *event,
			      uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *param_buf;
	WOW_EVENT_INFO_fixed_param *wake_info;
#ifdef FEATURE_WLAN_SCAN_PNO
	struct wma_txrx_node *node;
#endif /* FEATURE_WLAN_SCAN_PNO */
	uint32_t wake_lock_duration = 0;
	void *wmi_cmd_struct_ptr = NULL;
	uint32_t tlv_hdr, tag, wow_buf_pkt_len = 0, event_id = 0;
	int tlv_ok_status;

	param_buf = (WMI_WOW_WAKEUP_HOST_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid wow wakeup host event buf");
		return -EINVAL;
	}

	wake_info = param_buf->fixed_param;

	WMA_LOGA("WOW wakeup host event received (reason: %s(%d)) for vdev %d",
		 wma_wow_wake_reason_str(wake_info->wake_reason),
		 wake_info->wake_reason, wake_info->vdev_id);

	qdf_event_set(&wma->wma_resume_event);

	if (param_buf->wow_packet_buffer &&
	    tlv_check_required(wake_info->wake_reason)) {
		/*
		 * In case of wow_packet_buffer, first 4 bytes is the length.
		 * Following the length is the actual buffer.
		 */
		wow_buf_pkt_len = *(uint32_t *)param_buf->wow_packet_buffer;
		tlv_hdr = WMITLV_GET_HDR(
				(uint8_t *)param_buf->wow_packet_buffer + 4);

		tag = WMITLV_GET_TLVTAG(tlv_hdr);
		event_id = wow_get_wmi_eventid(wake_info->wake_reason, tag);
		if (!event_id) {
			WMA_LOGE(FL("Unable to find matching ID"));
			return -EINVAL;
		}

		tlv_ok_status = wmitlv_check_and_pad_event_tlvs(
				    handle, param_buf->wow_packet_buffer + 4,
				    wow_buf_pkt_len, event_id,
				    &wmi_cmd_struct_ptr);

		if (tlv_ok_status != 0) {
			WMA_LOGE(FL("Invalid TLVs, Length:%d event_id:%d status: %d"),
				 wow_buf_pkt_len, event_id, tlv_ok_status);
			return -EINVAL;
		}
	}

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
		wake_lock_duration =
			wma_wow_get_wakelock_duration(wake_info->wake_reason);
		if (param_buf->wow_packet_buffer) {
			/* First 4-bytes of wow_packet_buffer is the length */
			qdf_mem_copy((uint8_t *) &wow_buf_pkt_len,
				param_buf->wow_packet_buffer, 4);
			if (wow_buf_pkt_len)
				wma_wow_dump_mgmt_buffer(
					param_buf->wow_packet_buffer,
					wow_buf_pkt_len);
			else
				WMA_LOGE("wow packet buffer is empty");
		} else {
			WMA_LOGE("No wow packet buffer present");
		}
		break;

	case WOW_REASON_AP_ASSOC_LOST:
		wake_lock_duration = WMA_BMISS_EVENT_WAKE_LOCK_DURATION;
		WMA_LOGA("Beacon miss indication on vdev %x",
			 wake_info->vdev_id);
		wma_beacon_miss_handler(wma, wake_info->vdev_id);
		break;
#ifdef FEATURE_WLAN_RA_FILTERING
	case WOW_REASON_RA_MATCH:
		wma_wow_wake_up_stats(wma, NULL, 0, WOW_REASON_RA_MATCH);
		break;
#endif /* FEATURE_WLAN_RA_FILTERING */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case WOW_REASON_HOST_AUTO_SHUTDOWN:
		wake_lock_duration = WMA_AUTO_SHUTDOWN_WAKE_LOCK_DURATION;
		WMA_LOGA("Received WOW Auto Shutdown trigger in suspend");
		if (wma_post_auto_shutdown_msg())
			return -EINVAL;
		break;
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */
#ifdef FEATURE_WLAN_SCAN_PNO
	case WOW_REASON_NLOD:
		wma_wow_wake_up_stats(wma, NULL, 0, WOW_REASON_NLOD);
		node = &wma->interfaces[wake_info->vdev_id];
		if (node) {
			WMA_LOGD("NLO match happened");
			node->nlo_match_evt_received = true;
			qdf_wake_lock_timeout_acquire(&wma->pno_wake_lock,
					WMA_PNO_MATCH_WAKE_LOCK_TIMEOUT,
					WIFI_POWER_EVENT_WAKELOCK_PNO);
		}
		break;

	case WOW_REASON_NLO_SCAN_COMPLETE:
		WMA_LOGD("Host woken up due to pno scan complete reason");
		if (param_buf->wow_packet_buffer)
			wma_nlo_scan_cmp_evt_handler(handle,
					wmi_cmd_struct_ptr, wow_buf_pkt_len);
		else
			WMA_LOGD("No wow_packet_buffer present");
		break;
#endif /* FEATURE_WLAN_SCAN_PNO */

	case WOW_REASON_CSA_EVENT:
		WMA_LOGD("Host woken up because of CSA IE");
		wma_csa_offload_handler(handle, wmi_cmd_struct_ptr,
					wow_buf_pkt_len);
	break;

#ifdef FEATURE_WLAN_LPHB
	case WOW_REASON_WLAN_HB:
		wma_lphb_handler(wma, (uint8_t *) param_buf->hb_indevt);
		break;
#endif /* FEATURE_WLAN_LPHB */

	case WOW_REASON_HTT_EVENT:
		break;
	case WOW_REASON_PATTERN_MATCH_FOUND:
		wma_wow_wake_up_stats_display(wma);
		WMA_LOGD("Wake up for Rx packet, dump starting from ethernet hdr");
		if (param_buf->wow_packet_buffer) {
			/* First 4-bytes of wow_packet_buffer is the length */
			qdf_mem_copy((uint8_t *) &wow_buf_pkt_len,
				     param_buf->wow_packet_buffer, 4);
			if (wow_buf_pkt_len) {
				uint8_t *data;

				wma_wow_wake_up_stats(wma,
					param_buf->wow_packet_buffer + 4,
					wow_buf_pkt_len,
					WOW_REASON_PATTERN_MATCH_FOUND);
				qdf_trace_hex_dump(QDF_MODULE_ID_WMA,
					QDF_TRACE_LEVEL_DEBUG,
					param_buf->wow_packet_buffer + 4,
					wow_buf_pkt_len);

				data = (uint8_t *)
					(param_buf->wow_packet_buffer + 4);
				wma_wow_parse_data_pkt_buffer(data,
					wow_buf_pkt_len);
			} else {
				WMA_LOGE("wow packet buffer is empty");
			}
		} else {
			WMA_LOGE("No wow packet buffer present");
		}
		break;

	case WOW_REASON_LOW_RSSI:
		/* WOW_REASON_LOW_RSSI is used for all roaming events.
		 * WMI_ROAM_REASON_BETTER_AP, WMI_ROAM_REASON_BMISS,
		 * WMI_ROAM_REASON_SUITABLE_AP will be handled by
		 * wma_roam_event_callback().
		 */
		wma_wow_wake_up_stats(wma, NULL, 0, WOW_REASON_LOW_RSSI);
		WMA_LOGD("Host woken up because of roam event");
		if (param_buf->wow_packet_buffer) {
			/* Roam event is embedded in wow_packet_buffer */
			WMA_LOGD("wow_packet_buffer dump");
			qdf_trace_hex_dump(QDF_MODULE_ID_WMA,
					   QDF_TRACE_LEVEL_DEBUG,
					   param_buf->wow_packet_buffer,
					   wow_buf_pkt_len);
			wma_roam_event_callback(handle, wmi_cmd_struct_ptr,
						wow_buf_pkt_len);
		} else {
			/*
			 * No wow_packet_buffer means a better AP beacon
			 * will follow in a later event.
			 */
			WMA_LOGD("Host woken up because of better AP beacon");
		}
		break;
	case WOW_REASON_CLIENT_KICKOUT_EVENT:
		WMA_LOGD("Host woken up because of sta_kickout event");
		if (param_buf->wow_packet_buffer) {
			WMA_LOGD("wow_packet_buffer dump");
			qdf_trace_hex_dump(QDF_MODULE_ID_WMA,
				QDF_TRACE_LEVEL_DEBUG,
				param_buf->wow_packet_buffer, wow_buf_pkt_len);
			wma_peer_sta_kickout_event_handler(handle,
				wmi_cmd_struct_ptr, wow_buf_pkt_len);
		} else {
		    WMA_LOGD("No wow_packet_buffer present");
		}
		break;
#ifdef FEATURE_WLAN_EXTSCAN
	case WOW_REASON_EXTSCAN:
		WMA_LOGD("Host woken up because of extscan reason");
		wma_wow_wake_up_stats(wma, NULL, 0, WOW_REASON_EXTSCAN);
		if (param_buf->wow_packet_buffer)
			wma_extscan_wow_event_callback(handle,
				wmi_cmd_struct_ptr, wow_buf_pkt_len);
		else
			WMA_LOGE("wow_packet_buffer is empty");
		break;
#endif
	case WOW_REASON_RSSI_BREACH_EVENT:
		wma_wow_wake_up_stats(wma, NULL, 0,
				WOW_REASON_RSSI_BREACH_EVENT);
		WMA_LOGD("Host woken up because of rssi breach reason");
		/* rssi breach event is embedded in wow_packet_buffer */
		if (param_buf->wow_packet_buffer)
			wma_rssi_breached_event_handler(handle,
				wmi_cmd_struct_ptr, wow_buf_pkt_len);
		else
		    WMA_LOGD("No wow_packet_buffer present");
		break;
	case WOW_REASON_NAN_EVENT:
		WMA_LOGA("Host woken up due to NAN event reason");
		wma_nan_rsp_event_handler(handle,
				wmi_cmd_struct_ptr, wow_buf_pkt_len);
		break;
	case WOW_REASON_NAN_DATA:
		WMA_LOGD(FL("Host woken up for NAN data path event from FW"));
		if (param_buf->wow_packet_buffer) {
			wow_buf_pkt_len =
				*(uint32_t *)param_buf->wow_packet_buffer;
			WMA_LOGD(FL("wow_packet_buffer dump"));
			qdf_trace_hex_dump(QDF_MODULE_ID_WMA,
				QDF_TRACE_LEVEL_DEBUG,
				param_buf->wow_packet_buffer,
				wow_buf_pkt_len);
			wma_ndp_wow_event_callback(handle,
				(param_buf->wow_packet_buffer + 4),
				wow_buf_pkt_len);
		} else {
			WMA_LOGE(FL("wow_packet_buffer is empty"));
		}
		break;
	case WOW_REASON_OEM_RESPONSE_EVENT:
		wma_wow_wake_up_stats(wma, NULL, 0,
				WOW_REASON_OEM_RESPONSE_EVENT);
		/*
		 * Actual OEM Response event will follow after this
		 * WOW Wakeup event
		 */
		WMA_LOGD(FL("Host woken up for OEM Response event"));
		break;
	default:
		WMA_LOGE(FL("WOW reason %s(%d)- not handled"),
			wma_wow_wake_reason_str(wake_info->wake_reason),
			wake_info->wake_reason);
		break;
	}

	if (wake_lock_duration) {
		qdf_wake_lock_timeout_acquire(&wma->wow_wake_lock,
					      wake_lock_duration,
					      WIFI_POWER_EVENT_WAKELOCK_WOW);
		WMA_LOGA("Holding %d msec wake_lock", wake_lock_duration);
	}

	wmitlv_free_allocated_event_tlvs(event_id, &wmi_cmd_struct_ptr);
	return 0;
}

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

	qdf_event_set(&wma->wma_resume_event);

	return 0;
}
/**
 * wma_set_wow_bus_suspend() - set suspend flag
 * @wma: wma handle
 * @val: value
 *
 * Return: none
 */
static inline void wma_set_wow_bus_suspend(tp_wma_handle wma, int val)
{

	qdf_atomic_set(&wma->is_wow_bus_suspended, val);
}



/**
 * wma_add_wow_wakeup_event() -  Configures wow wakeup events.
 * @wma: wma handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: QDF status
 */
QDF_STATUS wma_add_wow_wakeup_event(tp_wma_handle wma,
					uint32_t vdev_id,
					uint32_t bitmap,
					bool enable)
{
	int ret;

	ret = wmi_unified_add_wow_wakeup_event_cmd(wma->wmi_handle, vdev_id,
			bitmap, enable);
	if (ret) {
		WMA_LOGE("Failed to config wow wakeup event");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_wow_patterns_to_fw() - Sends WOW patterns to FW.
 * @wma: wma handle
 * @vdev_id: vdev id
 * @ptrn_id: pattern id
 * @ptrn: pattern
 * @ptrn_len: pattern length
 * @ptrn_offset: pattern offset
 * @mask: mask
 * @mask_len: mask length
 * @user: true for user configured pattern and false for default pattern
 *
 * Return: QDF status
 */
static QDF_STATUS wma_send_wow_patterns_to_fw(tp_wma_handle wma,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user)
{
	struct wma_txrx_node *iface;
	int ret;
	uint8_t default_patterns;

	iface = &wma->interfaces[vdev_id];
	default_patterns = iface->num_wow_default_patterns++;
	ret = wmi_unified_wow_patterns_to_fw_cmd(wma->wmi_handle,
			    vdev_id, ptrn_id, ptrn,
				ptrn_len, ptrn_offset, mask,
				mask_len, user, default_patterns);
	if (ret) {
		if (!user)
			iface->num_wow_default_patterns--;
		return QDF_STATUS_E_FAILURE;
	}

	if (user)
		iface->num_wow_user_patterns++;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wow_ap() - set WOW patterns in ap mode
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Configures default WOW pattern for the given vdev_id which is in AP mode.
 *
 * Return: QDF status
 */
static QDF_STATUS wma_wow_ap(tp_wma_handle wma, uint8_t vdev_id)
{
	QDF_STATUS ret;
	uint8_t arp_offset = 20;
	uint8_t mac_mask[IEEE80211_ADDR_LEN];

	/* Setup unicast pkt pattern */
	qdf_mem_set(&mac_mask, IEEE80211_ADDR_LEN, 0xFF);
	ret = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
				wma->interfaces[vdev_id].addr,
				IEEE80211_ADDR_LEN, 0, mac_mask,
				IEEE80211_ADDR_LEN, false);
	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to add WOW unicast pattern ret %d", ret);
		return ret;
	}

	/*
	 * Setup all ARP pkt pattern. This is dummy pattern hence the length
	 * is zero
	 */
	ret = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
			arp_ptrn, 0, arp_offset, arp_mask, 0, false);
	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to add WOW ARP pattern ret %d", ret);
		return ret;
	}

	return ret;
}

/**
 * wma_configure_wow_ssdp() - API to configure WoW SSDP
 * @wma: WMA Handle
 * @vdev_id: Vdev Id
 *
 * API to configure SSDP pattern as WoW pattern
 *
 * Return: Success/Failure
 */
static QDF_STATUS wma_configure_wow_ssdp(tp_wma_handle wma, uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t discvr_offset = 30;

	 status = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
				discvr_ptrn, sizeof(discvr_ptrn), discvr_offset,
				discvr_mask, sizeof(discvr_ptrn), false);

	if (status != QDF_STATUS_SUCCESS)
		WMA_LOGE("Failed to add WOW mDNS/SSDP/LLMNR pattern");

	return status;
}

/**
  * wma_configure_mc_ssdp() - API to configure SSDP address as MC list
  * @wma: WMA Handle
  * @vdev_id: Vdev Id
  *
  * SSDP address 239.255.255.250 is converted to Multicast Mac address
  * and configure it to FW. Firmware will apply this pattern on the incoming
  * packets to filter them out during chatter/wow mode.
  *
  * Return: Success/Failure
  */
static QDF_STATUS wma_configure_mc_ssdp(tp_wma_handle wma, uint8_t vdev_id)
{
	WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	const tSirMacAddr ssdp_addr = {0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa};
	int ret;
	WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param fixed_param;
	uint32_t tag =
		WMITLV_TAG_STRUC_WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param;

	buf = wmi_buf_alloc(wma->wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGE("%s No Memory for MC address", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header, tag,
		       WMITLV_GET_STRUCT_TLVLEN(fixed_param));

	cmd->action = WMI_MCAST_FILTER_SET;
	cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ssdp_addr, &cmd->mcastbdcastaddr);
	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, sizeof(*cmd),
				   WMI_SET_MCASTBCAST_FILTER_CMDID);
	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s Failed to configure FW with SSDP MC address",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_configure_ssdp() - API to Configure SSDP pattern to FW
 * @wma: WMA Handle
 * @vdev_id: VDEV ID
 *
 * Setup multicast pattern for mDNS 224.0.0.251, SSDP 239.255.255.250 and LLMNR
 * 224.0.0.252
 *
 * Return: Success/Failure.
 */
static QDF_STATUS wma_configure_ssdp(tp_wma_handle wma, uint8_t vdev_id)
{
	if (!wma->ssdp) {
		WMA_LOGD("mDNS, SSDP, LLMNR patterns are disabled from ini");
		return QDF_STATUS_SUCCESS;
	}

	WMA_LOGD("%s, enable_mc_list:%d", __func__, wma->enable_mc_list);

	if (wma->enable_mc_list)
		return wma_configure_mc_ssdp(wma, vdev_id);

	return wma_configure_wow_ssdp(wma, vdev_id);
}

/**
 * wma_wow_sta() - set WOW patterns in sta mode
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Configures default WOW pattern for the given vdev_id which is in sta mode.
 *
 * Return: QDF status
 */
static QDF_STATUS wma_wow_sta(tp_wma_handle wma, uint8_t vdev_id)
{
	uint8_t arp_offset = 12;
	uint8_t mac_mask[IEEE80211_ADDR_LEN];
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	/* Setup unicast pkt pattern */
	qdf_mem_set(&mac_mask, IEEE80211_ADDR_LEN, 0xFF);
	ret = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
				wma->interfaces[vdev_id].addr,
				IEEE80211_ADDR_LEN, 0, mac_mask,
				IEEE80211_ADDR_LEN, false);
	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to add WOW unicast pattern ret %d", ret);
		return ret;
	}

	ret = wma_configure_ssdp(wma, vdev_id);
	if (ret != QDF_STATUS_SUCCESS)
		WMA_LOGE("Failed to configure SSDP patterns to FW");

	/* when arp offload or ns offloaded is disabled
	 * from ini file, configure broad cast arp pattern
	 * to fw, so that host can wake up
	 */
	if (!(wma->ol_ini_info & 0x1)) {
		/* Setup all ARP pkt pattern */
		ret = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
				arp_ptrn, sizeof(arp_ptrn), arp_offset,
				arp_mask, sizeof(arp_mask), false);
		if (ret != QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to add WOW ARP pattern");
			return ret;
		}
	}

	/* for NS or NDP offload packets */
	if (!(wma->ol_ini_info & 0x2)) {
		/* Setup all NS pkt pattern */
		ret = wma_send_wow_patterns_to_fw(wma, vdev_id, 0,
				ns_ptrn, sizeof(arp_ptrn), arp_offset,
				arp_mask, sizeof(arp_mask), false);
		if (ret != QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to add WOW NS pattern");
			return ret;
		}
	}

	return ret;
}

/**
 * wma_register_wow_default_patterns() - register default wow patterns with fw
 * @handle: Pointer to wma handle
 * @vdev_id: vdev id
 *
 * WoW default wake up pattern rule is:
 *  - For STA & P2P CLI mode register for same STA specific wow patterns
 *  - For SAP/P2P GO & IBSS mode register for same SAP specific wow patterns
 *
 * Return: none
 */
void wma_register_wow_default_patterns(WMA_HANDLE handle, uint8_t vdev_id)
{
	tp_wma_handle wma = handle;
	struct wma_txrx_node *iface;

	if (vdev_id > wma->max_bssid) {
		WMA_LOGE("Invalid vdev id %d", vdev_id);
		return;
	}
	iface = &wma->interfaces[vdev_id];

	if (iface->ptrn_match_enable) {
		if (wma_is_vdev_in_beaconning_mode(wma, vdev_id)) {
			/* Configure SAP/GO/IBSS mode default wow patterns */
			WMA_LOGI("Config SAP specific default wow patterns vdev_id %d",
				 vdev_id);
			wma_wow_ap(wma, vdev_id);
		} else {
			/* Configure STA/P2P CLI mode default wow patterns */
			WMA_LOGI("Config STA specific default wow patterns vdev_id %d",
				vdev_id);
			wma_wow_sta(wma, vdev_id);
			if (wma->IsRArateLimitEnabled) {
				WMA_LOGI("Config STA RA limit wow patterns vdev_id %d",
					vdev_id);
				wma_wow_sta_ra_filter(wma, vdev_id);
			}
		}
	}

	return;
}

/**
 * wma_register_wow_wakeup_events() - register vdev specific wake events with fw
 * @handle: Pointer to wma handle
 * @vdev_id: vdev Id
 * @vdev_type: vdev type
 * @vdev_subtype: vdev sub type
 *
 * WoW wake up event rule is following:
 * 1) STA mode and P2P CLI mode wake up events are same
 * 2) SAP mode and P2P GO mode wake up events are same
 * 3) IBSS mode wake events are same as STA mode plus WOW_BEACON_EVENT
 *
 * Return: none
 */
void wma_register_wow_wakeup_events(WMA_HANDLE handle,
				uint8_t vdev_id,
				uint8_t vdev_type,
				uint8_t vdev_subtype)
{
	tp_wma_handle wma = handle;
	uint32_t event_bitmap;

	WMA_LOGI("vdev_type %d vdev_subtype %d vdev_id %d", vdev_type,
			vdev_subtype, vdev_id);

	if ((WMI_VDEV_TYPE_STA == vdev_type) ||
		((WMI_VDEV_TYPE_AP == vdev_type) &&
		 (WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE == vdev_subtype))) {
		/* Configure STA/P2P CLI mode specific default wake up events */
		event_bitmap = WMA_WOW_STA_WAKE_UP_EVENTS;
		WMA_LOGI("STA specific default wake up event 0x%x vdev id %d",
			event_bitmap, vdev_id);
	} else if (WMI_VDEV_TYPE_IBSS == vdev_type) {
		/* Configure IBSS mode specific default wake up events */
		event_bitmap = (WMA_WOW_STA_WAKE_UP_EVENTS |
				(1 << WOW_BEACON_EVENT));
		WMA_LOGI("IBSS specific default wake up event 0x%x vdev id %d",
			event_bitmap, vdev_id);
	} else if (WMI_VDEV_TYPE_AP == vdev_type) {
		/* Configure SAP/GO mode specific default wake up events */
		event_bitmap = WMA_WOW_SAP_WAKE_UP_EVENTS;
		WMA_LOGI("SAP specific default wake up event 0x%x vdev id %d",
			event_bitmap, vdev_id);
	} else if (WMI_VDEV_TYPE_NDI == vdev_type) {
		/*
		 * Configure NAN data path specific default wake up events.
		 * Following routine sends the command to firmware.
		 */
		wma_ndp_add_wow_wakeup_event(wma, vdev_id);
		return;
	} else {
		WMA_LOGE("unknown type %d subtype %d", vdev_type, vdev_subtype);
		return;
	}

	wma_add_wow_wakeup_event(wma, vdev_id, event_bitmap, true);

	return;
}

/**
 * wma_enable_disable_wakeup_event() -  Configures wow wakeup events
 * @wma: wma handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: none
 */
void wma_enable_disable_wakeup_event(WMA_HANDLE handle,
				uint32_t vdev_id,
				uint32_t bitmap,
				bool enable)
{
	tp_wma_handle wma = handle;

	WMA_LOGI("vdev_id %d wake up event 0x%x enable %d",
		vdev_id, bitmap, enable);
	wma_add_wow_wakeup_event(wma, vdev_id, bitmap, enable);
}

/**
 * wma_enable_wow_in_fw() - wnable wow in fw
 * @wma: wma handle
 *
 * Return: QDF status
 */
QDF_STATUS wma_enable_wow_in_fw(WMA_HANDLE handle)
{
	tp_wma_handle wma = handle;
	int ret;
	struct hif_opaque_softc *scn;
	int host_credits;
	int wmi_pending_cmds;
	struct wow_cmd_params param = {0};

#ifdef CONFIG_CNSS
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);

	if (NULL == pMac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_FAILURE;
	}
#endif /* CONFIG_CNSS */

	qdf_event_reset(&wma->target_suspend);
	wma->wow_nack = false;

	host_credits = wmi_get_host_credits(wma->wmi_handle);
	wmi_pending_cmds = wmi_get_pending_cmds(wma->wmi_handle);

	WMA_LOGD("Credits:%d; Pending_Cmds: %d",
		 host_credits, wmi_pending_cmds);

	if (host_credits < WMI_WOW_REQUIRED_CREDITS) {
		WMA_LOGE("%s: Host Doesn't have enough credits to Post WMI_WOW_ENABLE_CMDID! "
			"Credits:%d, pending_cmds:%d\n", __func__, host_credits,
			wmi_pending_cmds);
#ifndef QCA_WIFI_3_0_EMU
		goto error;
#endif
	}

	param.enable = true;
	param.can_suspend_link = htc_can_suspend_link(wma->htc_handle);
	ret = wmi_unified_wow_enable_send(wma->wmi_handle, &param,
				   WMA_WILDCARD_PDEV_ID);
	if (ret) {
		WMA_LOGE("Failed to enable wow in fw");
		goto error;
	}

	wmi_set_target_suspend(wma->wmi_handle, true);

	if (qdf_wait_single_event(&wma->target_suspend,
				  WMA_TGT_SUSPEND_COMPLETE_TIMEOUT)
	    != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to receive WoW Enable Ack from FW");
		WMA_LOGE("Credits:%d; Pending_Cmds: %d",
			 wmi_get_host_credits(wma->wmi_handle),
			 wmi_get_pending_cmds(wma->wmi_handle));
		if (!cds_is_driver_recovering()) {
#ifdef CONFIG_CNSS
			if (pMac->sme.enableSelfRecovery) {
				cds_trigger_recovery();
			} else {
				QDF_BUG(0);
			}
#else
			QDF_BUG(0);
#endif /* CONFIG_CNSS */
		} else {
			WMA_LOGE("%s: LOGP is in progress, ignore!", __func__);
		}

		wmi_set_target_suspend(wma->wmi_handle, false);
		return QDF_STATUS_E_FAILURE;
	}

	if (wma->wow_nack) {
		WMA_LOGE("FW not ready to WOW");
		wmi_set_target_suspend(wma->wmi_handle, false);
		return QDF_STATUS_E_AGAIN;
	}

	host_credits = wmi_get_host_credits(wma->wmi_handle);
	wmi_pending_cmds = wmi_get_pending_cmds(wma->wmi_handle);

	if (host_credits < WMI_WOW_REQUIRED_CREDITS) {
		WMA_LOGE("%s: No Credits after HTC ACK:%d, pending_cmds:%d, "
			 "cannot resume back", __func__, host_credits,
			 wmi_pending_cmds);
		htc_dump_counter_info(wma->htc_handle);
		if (!cds_is_driver_recovering())
			QDF_BUG(0);
		else
			WMA_LOGE("%s: SSR in progress, ignore no credit issue",
				 __func__);
	}

	WMA_LOGD("WOW enabled successfully in fw: credits:%d"
		 "pending_cmds: %d", host_credits, wmi_pending_cmds);

	scn = cds_get_context(QDF_MODULE_ID_HIF);

	if (scn == NULL) {
		WMA_LOGE("%s: Failed to get HIF context", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	wma->wow.wow_enable_cmd_sent = true;

	return QDF_STATUS_SUCCESS;

error:
	return QDF_STATUS_E_FAILURE;
}

/**
 * wma_resume_req() - clear configured wow patterns in fw
 * @wma: wma handle
 * @type: type of suspend
 *
 * Return: QDF status
 */
QDF_STATUS wma_resume_req(tp_wma_handle wma, enum qdf_suspend_type type)
{
	if (type == QDF_SYSTEM_SUSPEND) {
		wma->no_of_resume_ind++;

		if (wma->no_of_resume_ind < wma_get_vdev_count(wma))
			return QDF_STATUS_SUCCESS;

		wma->no_of_resume_ind = 0;
	}

	/* Reset the DTIM Parameters */
	wma_set_resume_dtim(wma);
	/* need to reset if hif_pci_suspend_fails */
	wma_set_wow_bus_suspend(wma, 0);
	/* unpause the vdev if left paused and hif_pci_suspend fails */
	wma_unpause_vdev(wma);

	wmi_set_runtime_pm_inprogress(wma->wmi_handle, false);

	if (type == QDF_RUNTIME_SUSPEND)
		qdf_runtime_pm_allow_suspend(wma->wma_runtime_resume_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wow_delete_pattern() - delete wow pattern in target
 * @wma: wma handle
 * @ptrn_id: pattern id
 * @vdev_id: vdev id
 * @user: true for user pattern and false for default pattern
 *
 * Return: QDF status
 */
static QDF_STATUS wma_wow_delete_pattern(tp_wma_handle wma, uint8_t ptrn_id,
					uint8_t vdev_id, bool user)
{

	struct wma_txrx_node *iface;
	int ret;

	iface = &wma->interfaces[vdev_id];
	ret = wmi_unified_wow_delete_pattern_cmd(wma->wmi_handle, ptrn_id,
				   vdev_id);
	if (ret) {
		return QDF_STATUS_E_FAILURE;
	}

	if (user)
		iface->num_wow_user_patterns--;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wow_add_pattern() - add wow pattern in target
 * @wma: wma handle
 * @ptrn: wow pattern
 *
 * This function does following:
 * 1) Delete all default patterns of the vdev
 * 2) Add received wow patterns for given vdev in target.
 *
 * Target is responsible for caching wow patterns accross multiple
 * suspend/resumes until the pattern is deleted by user
 *
 * Return: QDF status
 */
QDF_STATUS wma_wow_add_pattern(tp_wma_handle wma, struct wow_add_pattern *ptrn)
{
	uint8_t id;
	uint8_t bit_to_check, pos;
	struct wma_txrx_node *iface;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t new_mask[SIR_WOWL_BCAST_PATTERN_MAX_SIZE];

	if (ptrn->session_id > wma->max_bssid) {
		WMA_LOGE("Invalid vdev id (%d)", ptrn->session_id);
		return QDF_STATUS_E_INVAL;
	}

	iface = &wma->interfaces[ptrn->session_id];

	/* clear all default patterns cofigured by wma */
	for (id = 0; id < iface->num_wow_default_patterns; id++)
		wma_wow_delete_pattern(wma, id, ptrn->session_id, false);

	iface->num_wow_default_patterns = 0;

	WMA_LOGI("Add user passed wow pattern id %d vdev id %d",
		ptrn->pattern_id, ptrn->session_id);
	/*
	 * Convert received pattern mask value from bit representation
	 * to byte representation.
	 *
	 * For example, received value from umac,
	 *
	 *      Mask value    : A1 (equivalent binary is "1010 0001")
	 *      Pattern value : 12:00:13:00:00:00:00:44
	 *
	 * The value which goes to FW after the conversion from this
	 * function (1 in mask value will become FF and 0 will
	 * become 00),
	 *
	 *      Mask value    : FF:00:FF:00:0:00:00:FF
	 *      Pattern value : 12:00:13:00:00:00:00:44
	 */
	qdf_mem_zero(new_mask, sizeof(new_mask));
	for (pos = 0; pos < ptrn->pattern_size; pos++) {
		bit_to_check = (WMA_NUM_BITS_IN_BYTE - 1) -
			       (pos % WMA_NUM_BITS_IN_BYTE);
		bit_to_check = 0x1 << bit_to_check;
		if (ptrn->pattern_mask[pos / WMA_NUM_BITS_IN_BYTE] &
							bit_to_check)
			new_mask[pos] = WMA_WOW_PTRN_MASK_VALID;
	}

	ret = wma_send_wow_patterns_to_fw(wma, ptrn->session_id,
			ptrn->pattern_id,
			ptrn->pattern, ptrn->pattern_size,
			ptrn->pattern_byte_offset, new_mask,
			ptrn->pattern_size, true);
	if (ret != QDF_STATUS_SUCCESS)
		WMA_LOGE("Failed to add wow pattern %d", ptrn->pattern_id);

	return ret;
}

/**
 * wma_wow_delete_user_pattern() - delete user configured wow pattern in target
 * @wma: wma handle
 * @ptrn: wow pattern
 *
 * This function does following:
 * 1) Deletes a particular user configured wow pattern in target
 * 2) After deleting all user wow patterns add default wow patterns
 *    specific to that vdev.
 *
 * Return: QDF status
 */
QDF_STATUS wma_wow_delete_user_pattern(tp_wma_handle wma,
					struct wow_delete_pattern *pattern)
{
	struct wma_txrx_node *iface;

	if (pattern->session_id > wma->max_bssid) {
		WMA_LOGE("Invalid vdev id %d", pattern->session_id);
		return QDF_STATUS_E_INVAL;
	}

	iface = &wma->interfaces[pattern->session_id];
	if (iface->num_wow_user_patterns <= 0) {
		WMA_LOGE("No valid user pattern. Num user pattern %u vdev %d",
			iface->num_wow_user_patterns, pattern->session_id);
		return QDF_STATUS_E_INVAL;
	}

	WMA_LOGI("Delete user passed wow pattern id %d total user pattern %d",
		pattern->pattern_id, iface->num_wow_user_patterns);

	wma_wow_delete_pattern(wma, pattern->pattern_id,
				pattern->session_id, true);

	/* configure default patterns once all user patterns are deleted */
	if (!iface->num_wow_user_patterns)
		wma_register_wow_default_patterns(wma, pattern->session_id);

	return QDF_STATUS_SUCCESS;
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
	struct wma_txrx_node *iface;

	WMA_LOGD("wow enable req received for vdev id: %d", info->sessionId);

	if (info->sessionId > wma->max_bssid) {
		WMA_LOGE("Invalid vdev id (%d)", info->sessionId);
		qdf_mem_free(info);
		return QDF_STATUS_E_INVAL;
	}

	iface = &wma->interfaces[info->sessionId];
	iface->ptrn_match_enable = info->ucPatternFilteringEnable ?
				   true : false;
	wma->wow.magic_ptrn_enable = info->ucMagicPktEnable ? true : false;
	wma->wow.deauth_enable = info->ucWowDeauthRcv ? true : false;
	wma->wow.disassoc_enable = info->ucWowDeauthRcv ? true : false;
	wma->wow.bmiss_enable = info->ucWowMaxMissedBeacons ? true : false;

	qdf_mem_free(info);

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
	struct wma_txrx_node *iface;

	WMA_LOGD("wow disable req received for vdev id: %d", info->sessionId);

	if (info->sessionId > wma->max_bssid) {
		WMA_LOGE("Invalid vdev id (%d)", info->sessionId);
		qdf_mem_free(info);
		return QDF_STATUS_E_INVAL;
	}

	iface = &wma->interfaces[info->sessionId];
	iface->ptrn_match_enable = false;
	wma->wow.magic_ptrn_enable = false;
	qdf_mem_free(info);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_calculate_and_update_conn_state(): calculate each interfaces conn state
 * @wma: validated wma handle
 *
 * Identifies any vdev that is up and not in ap mode as connected.
 * stores this in the interfaces conn_state varible.
 */
void wma_calculate_and_update_conn_state(tp_wma_handle wma)
{
	int i;
	for (i = 0; i < wma->max_bssid; i++) {
		wma->interfaces[i].conn_state =
			!!(wma->interfaces[i].vdev_up &&
					!wma_is_vdev_in_ap_mode(wma, i));
	}
}

/**
 * wma_update_conn_state(): synchronize wma & hdd
 * @wma: wma handle
 * @conn_state: boolean array to populate
 * @len: validation parameter
 *
 * populate interfaces conn_state with true if the interface
 * is a connected client and wow will configure a pattern.
 */
void wma_update_conn_state(tp_wma_handle wma, uint32_t conn_mask)
{
	int i;
	for (i = 0; i < wma->max_bssid; i++) {
		if (conn_mask & (1 << i))
			wma->interfaces[i].conn_state = true;
		else
			wma->interfaces[i].conn_state = false;
	}

	if (wma->wow.magic_ptrn_enable)
		return;

	for (i = 0; i < wma->max_bssid; i++) {
		if (!wma->interfaces[i].ptrn_match_enable)
			wma->interfaces[i].conn_state = false;
	}
}

/**
 * wma_is_beaconning_vdev_up(): check if a beaconning vdev is up
 * @wma: wma handle
 *
 * Return TRUE if beaconning vdev is up
 */
static inline
bool wma_is_beaconning_vdev_up(tp_wma_handle wma)
{
	int i;
	for (i = 0; i < wma->max_bssid; i++) {
		if (wma_is_vdev_in_beaconning_mode(wma, i)
				&& wma->interfaces[i].vdev_up)
			return true;
	}
	return false;
}

/**
 * wma_support_wow_for_beaconing: wow query for beaconning
 * @wma: wma handle
 *
 * Need to configure wow to enable beaconning offload when
 * a beaconing vdev is up and beaonning offload is configured.
 *
 * Return: true if we need to enable wow for beaconning offload
 */
static inline
bool wma_support_wow_for_beaconing(tp_wma_handle wma)
{
	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				WMI_SERVICE_BEACON_OFFLOAD)) {
		if (wma_is_beaconning_vdev_up(wma))
			return true;
	}
	return false;
}

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * wma_is_pnoscan_in_progress(): check if a pnoscan is in progress
 * @wma: wma handle
 * @vdev_id: vdev_id
 *
 * Return: TRUE/FALSE
 */
static inline
bool wma_is_pnoscan_in_progress(tp_wma_handle wma, int vdev_id)
{
	return wma->interfaces[vdev_id].pno_in_progress;
}

/**
 * wma_is_pnoscan_match_found(): check if a scan match was found
 * @wma: wma handle
 * @vdev_id: vdev_id
 *
 * Return: TRUE/FALSE
 */
static inline
bool wma_is_pnoscan_match_found(tp_wma_handle wma, int vdev_id)
{
	return wma->interfaces[vdev_id].nlo_match_evt_received;
}
#else
/**
 * wma_is_pnoscan_in_progress(): dummy
 *
 * Return: False since no pnoscan cannot be in progress
 * when feature flag is not defined.
 */
bool wma_is_pnoscan_in_progress(tp_wma_handle wma, int vdev_id)
{
	return FALSE;
}

/**
 * wma_is_pnoscan_match_found(): dummy
 * @wma: wma handle
 * @vdev_id: vdev_id
 *
 * Return: False since no pnoscan cannot occur
 * when feature flag is not defined.
 */
static inline
bool wma_is_pnoscan_match_found(tp_wma_handle wma, int vdev_id)
{
	return FALSE;
}
#endif

#ifdef FEATURE_WLAN_EXTSCAN
static inline
/**
 * wma_is_extscan_in_progress(): check if an extscan is in progress
 * @wma: wma handle
 * @vdev_id: vdev_id
 *
 * Return: TRUE/FALSvE
 */
bool wma_is_extscan_in_progress(tp_wma_handle wma, int vdev_id)
{
	return wma->interfaces[vdev_id].extscan_in_progress;
}
#else
/**
 * wma_is_extscan_in_progress(): dummy
 *
 * Return: False since no extscan can be in progress
 * when feature flag is not defined.
 */
bool wma_is_extscan_in_progress(tp_wma_handle wma, int vdev_id)
{
	return false;
}
#endif

/**
 * wma_is_p2plo_in_progress(): check if P2P listen offload is in progress
 * @wma: wma handle
 * @vdev_id: vdev_id
 *
 * This function is to check if p2p listen offload is in progress,
 *  true: p2p listen offload in progress
 *  false: otherwise
 *
 * Return: TRUE/FALSE
 */
static inline
bool wma_is_p2plo_in_progress(tp_wma_handle wma, int vdev_id)
{
	return wma->interfaces[vdev_id].p2p_lo_in_progress;
}

/**
 * wma_is_wow_applicable(): should enable wow
 * @wma: wma handle
 *
 *  Enable WOW if any one of the condition meets,
 *  1) Is any one of vdev in beaconning mode (in AP mode) ?
 *  2) Is any one of vdev in connected state (in STA mode) ?
 *  3) Is PNO in progress in any one of vdev ?
 *  4) Is Extscan in progress in any one of vdev ?
 *  5) Is P2P listen offload in any one of vdev?
 *  6) Is any vdev in NAN data mode? BSS is already started at the
 *     the time of device creation. It is ready to accept data
 *     requests.
 *  If none of above conditions is true then return false
 *
 * Return: true if wma needs to configure wow false otherwise.
 */
bool wma_is_wow_applicable(tp_wma_handle wma)
{
	int vdev_id;
	if (wma_support_wow_for_beaconing(wma)) {
		WMA_LOGD("vdev is in beaconning mode, enabling wow");
		return true;
	}

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		if (wma->interfaces[vdev_id].conn_state) {
			WMA_LOGD("STA is connected, enabling wow");
			return true;
		} else if (wma_is_pnoscan_in_progress(wma, vdev_id)) {
			WMA_LOGD("PNO is in progress, enabling wow");
			return true;
		} else if (wma_is_extscan_in_progress(wma, vdev_id)) {
			WMA_LOGD("EXT is in progress, enabling wow");
			return true;
		} else if (wma_is_p2plo_in_progress(wma, vdev_id)) {
			WMA_LOGD("P2P LO is in progress, enabling wow");
			return true;
		}
		if (WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces, vdev_id)) {
			WMA_LOGD("vdev %d is in NAN data mode, enabling wow",
				vdev_id);
			return true;
		}
	}

	WMA_LOGD("All vdev are in disconnected state and pno/extscan is not in progress, skipping wow");
	return false;
}

/**
 * wma_configure_dynamic_wake_events(): configure dyanmic wake events
 * @wma: wma handle
 *
 * Some wake events need to be enabled dynamically.  Controll those here.
 *
 * Return: none
 */
void wma_configure_dynamic_wake_events(tp_wma_handle wma)
{
	int vdev_id;
	int enable_mask;
	int disable_mask;

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		enable_mask = 0;
		disable_mask = 0;

		if (wma_is_pnoscan_in_progress(wma, vdev_id)) {
			if (wma_is_pnoscan_match_found(wma, vdev_id))
				enable_mask |=
					(1 << WOW_NLO_SCAN_COMPLETE_EVENT);
			else
				disable_mask |=
					(1 << WOW_NLO_SCAN_COMPLETE_EVENT);
		}

		if (enable_mask != 0)
			wma_enable_disable_wakeup_event(wma, vdev_id,
					enable_mask, true);
		if (disable_mask != 0)
			wma_enable_disable_wakeup_event(wma, vdev_id,
					disable_mask, false);
	}
}

#ifdef FEATURE_WLAN_LPHB
/**
 * wma_apply_lphb(): apply cached LPHB settings
 * @wma: wma handle
 *
 * LPHB cache, if any item was enabled, should be
 * applied.
 */
static inline
void wma_apply_lphb(tp_wma_handle wma)
{
	int i;
	WMA_LOGD("%s: checking LPHB cache", __func__);
	for (i = 0; i < 2; i++) {
		if (wma->wow.lphb_cache[i].params.lphbEnableReq.enable) {
			WMA_LOGD("%s: LPHB cache for item %d is marked as enable",
				__func__, i + 1);
			wma_lphb_conf_hbenable(wma, &(wma->wow.lphb_cache[i]),
					       false);
		}
	}
}
#else
void wma_apply_lphb(tp_wma_handle wma) {}
#endif /* FEATURE_WLAN_LPHB */

static void wma_notify_suspend_req_procesed(tp_wma_handle wma,
		enum qdf_suspend_type type)
{
	if (type == QDF_SYSTEM_SUSPEND)
		wma_send_status_to_suspend_ind(wma, true);
	else if (type == QDF_RUNTIME_SUSPEND)
		qdf_event_set(&wma->runtime_suspend);
}

/**
 * wma_suspend_req() -  Handles suspend indication request received from umac.
 * @wma: wma handle
 * @type: type of suspend
 *
 * The type controlls how we notify the indicator that the indication has
 * been processed
 *
 * Return: QDF status
 */
QDF_STATUS wma_suspend_req(tp_wma_handle wma, enum qdf_suspend_type type)
{
	if (type == QDF_RUNTIME_SUSPEND)
		wmi_set_runtime_pm_inprogress(wma->wmi_handle, true);

	if (wma_is_wow_applicable(wma)) {
		WMA_LOGE("WOW Suspend");
		wma_apply_lphb(wma);

		wma_configure_dynamic_wake_events(wma);

		wma->wow.wow_enable = true;
		wma->wow.wow_enable_cmd_sent = false;
	}

	/* Set the Suspend DTIM Parameters */
	wma_set_suspend_dtim(wma);

	wma_notify_suspend_req_procesed(wma, type);

	/* to handle race between hif_pci_suspend and
	 * unpause/pause tx handler
	 */
	wma_set_wow_bus_suspend(wma, 1);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_host_wakeup_ind_to_fw() - send wakeup ind to fw
 * @wma: wma handle
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: QDF status
 */
static QDF_STATUS wma_send_host_wakeup_ind_to_fw(tp_wma_handle wma)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int ret;
#ifdef CONFIG_CNSS
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);
	if (NULL == pMac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_FAILURE;
	}
#endif /* CONFIG_CNSS */

	qdf_event_reset(&wma->wma_resume_event);

	ret = wmi_unified_host_wakeup_ind_to_fw_cmd(wma->wmi_handle);
	if (ret) {
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Host wakeup indication sent to fw");

	qdf_status = qdf_wait_single_event(&(wma->wma_resume_event),
					   WMA_RESUME_TIMEOUT);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		WMA_LOGP("%s: Timeout waiting for resume event from FW",
			 __func__);
		WMA_LOGP("%s: Pending commands %d credits %d", __func__,
			 wmi_get_pending_cmds(wma->wmi_handle),
			 wmi_get_host_credits(wma->wmi_handle));
		if (!cds_is_driver_recovering()) {
#ifdef CONFIG_CNSS
			if (pMac->sme.enableSelfRecovery) {
				cds_trigger_recovery();
			} else {
				QDF_BUG(0);
			}
#else
			QDF_BUG(0);
#endif /* CONFIG_CNSS */
		} else {
			WMA_LOGE("%s: SSR in progress, ignore resume timeout",
				 __func__);
		}
	} else {
		WMA_LOGD("Host wakeup received");
	}

	if (QDF_STATUS_SUCCESS == qdf_status)
		wmi_set_target_suspend(wma->wmi_handle, false);

	return qdf_status;
}

/**
 * wma_disable_wow_in_fw() -  Disable wow in PCIe resume context.
 * @handle: wma handle
 *
 * Return: 0 for success or error code
 */
QDF_STATUS wma_disable_wow_in_fw(WMA_HANDLE handle)
{
	tp_wma_handle wma = handle;
	QDF_STATUS ret;

	if (!wma->wow.wow_enable || !wma->wow.wow_enable_cmd_sent)
		return QDF_STATUS_SUCCESS;

	ret = wma_send_host_wakeup_ind_to_fw(wma);

	if (ret != QDF_STATUS_SUCCESS)
		return ret;

	wma->wow.wow_enable = false;
	wma->wow.wow_enable_cmd_sent = false;

	/* To allow the tx pause/unpause events */
	wma_set_wow_bus_suspend(wma, 0);
	/* Unpause the vdev as we are resuming */
	wma_unpause_vdev(wma);

	return ret;
}

#ifdef WLAN_FEATURE_LPSS
/**
 * wma_is_lpass_enabled() - check if lpass is enabled
 * @handle: Pointer to wma handle
 *
 * WoW is needed if LPASS or NaN feature is enabled in INI because
 * target can't wake up itself if its put in PDEV suspend when LPASS
 * or NaN features are supported
 *
 * Return: true if lpass is enabled else false
 */
bool static wma_is_lpass_enabled(tp_wma_handle wma)
{
	if (wma->is_lpass_enabled)
		return true;
	else
		return false;
}
#else
bool static wma_is_lpass_enabled(tp_wma_handle wma)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_NAN
/**
 * wma_is_nan_enabled() - check if NaN is enabled
 * @handle: Pointer to wma handle
 *
 * WoW is needed if LPASS or NaN feature is enabled in INI because
 * target can't wake up itself if its put in PDEV suspend when LPASS
 * or NaN features are supported
 *
 * Return: true if NaN is enabled else false
 */
bool static wma_is_nan_enabled(tp_wma_handle wma)
{
	if (wma->is_nan_enabled)
		return true;
	else
		return false;
}
#else
bool static wma_is_nan_enabled(tp_wma_handle wma)
{
	return false;
}
#endif

/**
 * wma_is_wow_mode_selected() - check if wow needs to be enabled in fw
 * @handle: Pointer to wma handle
 *
 * If lpass is enabled then always do wow else check wow_enable config
 *
 * Return: true is wow mode is needed else false
 */
bool wma_is_wow_mode_selected(WMA_HANDLE handle)
{
	tp_wma_handle wma = (tp_wma_handle) handle;

	if (wma_is_lpass_enabled(wma)) {
		WMA_LOGD("LPASS is enabled select WoW");
		return true;
	} else if (wma_is_nan_enabled(wma)) {
		WMA_LOGD("NAN is enabled select WoW");
		return true;
	} else {
		WMA_LOGD("WoW enable %d", wma->wow.wow_enable);
		return wma->wow.wow_enable;
	}
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
	wma_send_msg(wma, WMA_AGGR_QOS_RSP, pAggrQosRspMsg, 0);
}

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

#ifdef FEATURE_WLAN_ESE
	/*
	 * msmt_interval is in unit called TU (1 TU = 1024 us)
	 * max value of msmt_interval cannot make resulting
	 * interval_miliseconds overflow 32 bit
	 */
	uint32_t intervalMiliseconds;
	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto err;
	}

	intervalMiliseconds = (msg->tsm_interval * 1024) / 1000;

	ol_tx_set_compute_interval(pdev, intervalMiliseconds);
#endif /* FEATURE_WLAN_ESE */
	msg->status = QDF_STATUS_SUCCESS;


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

err:
	wma_send_msg(wma, WMA_ADD_TS_RSP, msg, 0);
}

/**
 * wma_enable_disable_packet_filter() - enable/disable packet filter in target
 * @wma: Pointer to wma handle
 * @vdev_id: vdev id
 * @enable: Flag to enable/disable packet filter
 *
 * Return: 0 for success or error code
 */
static int wma_enable_disable_packet_filter(tp_wma_handle wma,
					uint8_t vdev_id, bool enable)
{
	int ret;

	ret = wmi_unified_enable_disable_packet_filter_cmd(wma->wmi_handle,
			 vdev_id, enable);
	if (ret)
		WMA_LOGE("Failed to send packet filter wmi cmd to fw");

	return ret;
}

/**
 * wma_config_packet_filter() - configure packet filter in target
 * @wma: Pointer to wma handle
 * @vdev_id: vdev id
 * @rcv_filter_param: Packet filter parameters
 * @filter_id: Filter id
 * @enable: Flag to add/delete packet filter configuration
 *
 * Return: 0 for success or error code
 */
static int wma_config_packet_filter(tp_wma_handle wma,
		uint8_t vdev_id, tSirRcvPktFilterCfgType *rcv_filter_param,
		uint8_t filter_id, bool enable)
{
	int err;

	/* send the command along with data */
	err = wmi_unified_config_packet_filter_cmd(wma->wmi_handle,
				vdev_id, (struct rcv_pkt_filter_config *)rcv_filter_param,
				filter_id, enable);
	if (err) {
		WMA_LOGE("Failed to send pkt_filter cmd");
		return -EIO;
	}

	/* Enable packet filter */
	if (enable)
		wma_enable_disable_packet_filter(wma, vdev_id, true);

	return 0;
}

/**
 * wma_process_receive_filter_set_filter_req() - enable packet filter
 * @wma_handle: wma handle
 * @rcv_filter_param: filter params
 *
 * Return: 0 for success or error code
 */
int wma_process_receive_filter_set_filter_req(tp_wma_handle wma,
				tSirRcvPktFilterCfgType *rcv_filter_param)
{
	int ret = 0;
	uint8_t vdev_id;

	/* Get the vdev id */
	if (!wma_find_vdev_by_bssid(wma,
		rcv_filter_param->bssid.bytes, &vdev_id)) {
		WMA_LOGE("vdev handle is invalid for %pM",
			rcv_filter_param->bssid.bytes);
		return -EINVAL;
	}

	ret = wma_config_packet_filter(wma, vdev_id, rcv_filter_param,
				rcv_filter_param->filterId, true);

	return ret;
}

/**
 * wma_process_receive_filter_clear_filter_req() - disable packet filter
 * @wma_handle: wma handle
 * @rcv_clear_param: filter params
 *
 * Return: 0 for success or error code
 */
int wma_process_receive_filter_clear_filter_req(tp_wma_handle wma,
				tSirRcvFltPktClearParam *rcv_clear_param)
{
	int ret = 0;
	uint8_t vdev_id;

	/* Get the vdev id */
	if (!wma_find_vdev_by_bssid(wma,
				rcv_clear_param->bssid.bytes, &vdev_id)) {
		WMA_LOGE("vdev handle is invalid for %pM",
			 rcv_clear_param->bssid.bytes);
		return -EINVAL;
	}

	ret = wma_config_packet_filter(wma, vdev_id, NULL,
			rcv_clear_param->filterId, false);

	return ret;
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

	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		qdf_mem_free(pTsmStatsMsg);
		return QDF_STATUS_E_INVAL;
	}

	/* get required values from data path APIs */
	ol_tx_delay(pdev, &queue_delay_microsec, &tx_delay_microsec, tid);
	ol_tx_delay_hist(pdev, bin_values, tid);
	ol_tx_packet_count(pdev, &packet_count, &packet_loss_count, tid);

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
 * wma_add_clear_mcbc_filter() - set mcast filter command to fw
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @multicastAddr: mcast address
 * @clearList: clear list flag
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS wma_add_clear_mcbc_filter(tp_wma_handle wma_handle,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList)
{
	return wmi_unified_add_clear_mcbc_filter_cmd(wma_handle->wmi_handle,
				vdev_id, multicast_addr, clearList);
}

/**
 * wma_config_enhance_multicast_offload() - config enhance multicast offload
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @action: enable or disable enhance multicast offload
 *
 * Return: none
 */
static void wma_config_enhance_multicast_offload(tp_wma_handle wma_handle,
						uint8_t vdev_id,
						uint8_t action)
{
	int status;
	wmi_buf_t buf;
	wmi_config_enhanced_mcast_filter_cmd_fixed_param *cmd;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set key cmd");
		return;
	}

	cmd = (wmi_config_enhanced_mcast_filter_cmd_fixed_param *)
							wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_config_enhanced_mcast_filter_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(wmi_config_enhanced_mcast_filter_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->enable = ((0 == action) ? ENHANCED_MCAST_FILTER_DISABLED :
			ENHANCED_MCAST_FILTER_ENABLED);

	WMA_LOGD("%s: config enhance multicast offload action %d for vdev %d",
		__func__, action, vdev_id);

	status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
			sizeof(*cmd), WMI_CONFIG_ENHANCED_MCAST_FILTER_CMDID);
	if (status) {
		qdf_nbuf_free(buf);
		WMA_LOGE("%s:Failed to send WMI_CONFIG_ENHANCED_MCAST_FILTER_CMDID",
			__func__);
	}

	return;
}

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
	uint8_t vdev_id = 0;
	int i;

	if (mcbc_param->ulMulticastAddrCnt <= 0) {
		WMA_LOGW("Number of multicast addresses is 0");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wma_find_vdev_by_addr(wma_handle,
			mcbc_param->self_macaddr.bytes, &vdev_id)) {
		WMA_LOGE("%s: Failed to find vdev id for %pM", __func__,
			 mcbc_param->bssid.bytes);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Configure enhance multicast offload feature for filtering out
	 * multicast IP data packets transmitted using unicast MAC address
	 */
	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
		WMI_SERVICE_ENHANCED_MCAST_FILTER)) {
		WMA_LOGD("%s: FW supports enhance multicast offload", __func__);
		wma_config_enhance_multicast_offload(wma_handle, vdev_id,
			mcbc_param->action);
	} else {
		WMA_LOGD("%s: FW does not support enhance multicast offload",
		__func__);
	}

	/* set mcbc_param->action to clear MCList and reset
	 * to configure the MCList in FW
	 */

	for (i = 0; i < mcbc_param->ulMulticastAddrCnt; i++) {
		wma_add_clear_mcbc_filter(wma_handle, vdev_id,
					  mcbc_param->multicastAddr[i],
					  (mcbc_param->action == 0));
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
#define GTK_OFFLOAD_ENABLE      0
#define GTK_OFFLOAD_DISABLE     1

/**
 * wma_gtk_offload_status_event() - GTK offload status event handler
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_gtk_offload_status_event(void *handle, uint8_t *event,
				 uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *status;
	WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *param_buf;
	tpSirGtkOffloadGetInfoRspParams resp;
	cds_msg_t cds_msg;
	uint8_t *bssid;

	WMA_LOGD("%s Enter", __func__);

	param_buf = (WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("param_buf is NULL");
		return -EINVAL;
	}

	status = (WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *) param_buf->fixed_param;

	if (len < sizeof(WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param)) {
		WMA_LOGE("Invalid length for GTK status");
		return -EINVAL;
	}
	bssid = wma_find_bssid_by_vdev_id(wma, status->vdev_id);
	if (!bssid) {
		WMA_LOGE("invalid bssid for vdev id %d", status->vdev_id);
		return -ENOENT;
	}

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp) {
		WMA_LOGE("%s: Failed to alloc response", __func__);
		return -ENOMEM;
	}
	qdf_mem_zero(resp, sizeof(*resp));
	resp->mesgType = eWNI_PMC_GTK_OFFLOAD_GETINFO_RSP;
	resp->mesgLen = sizeof(*resp);
	resp->ulStatus = QDF_STATUS_SUCCESS;
	resp->ulTotalRekeyCount = status->refresh_cnt;
	/* TODO: Is the total rekey count and GTK rekey count same? */
	resp->ulGTKRekeyCount = status->refresh_cnt;

	qdf_mem_copy(&resp->ullKeyReplayCounter, &status->replay_counter,
		     GTK_REPLAY_COUNTER_BYTES);

	qdf_mem_copy(resp->bssid.bytes, bssid, IEEE80211_ADDR_LEN);

#ifdef IGTK_OFFLOAD
	/* TODO: Is the refresh count same for GTK and IGTK? */
	resp->ulIGTKRekeyCount = status->refresh_cnt;
#endif /* IGTK_OFFLOAD */

	cds_msg.type = eWNI_PMC_GTK_OFFLOAD_GETINFO_RSP;
	cds_msg.bodyptr = (void *)resp;
	cds_msg.bodyval = 0;

	if (cds_mq_post_message(CDS_MQ_ID_SME, (cds_msg_t *) &cds_msg)
	    != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to post GTK response to SME");
		qdf_mem_free(resp);
		return -EINVAL;
	}

	WMA_LOGD("GTK: got target status with replay counter "
		 "%02x%02x%02x%02x%02x%02x%02x%02x. vdev %d "
		 "Refresh GTK %d times exchanges since last set operation",
		 status->replay_counter[0],
		 status->replay_counter[1],
		 status->replay_counter[2],
		 status->replay_counter[3],
		 status->replay_counter[4],
		 status->replay_counter[5],
		 status->replay_counter[6],
		 status->replay_counter[7],
		 status->vdev_id, status->refresh_cnt);

	WMA_LOGD("%s Exit", __func__);

	return 0;
}

/**
 * wma_send_gtk_offload_req() - send GTK offload command to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @params: GTK offload parameters
 *
 * Return: QDF status
 */
static QDF_STATUS wma_send_gtk_offload_req(tp_wma_handle wma, uint8_t vdev_id,
					   tpSirGtkOffloadParams params)
{
	struct gtk_offload_params offload_params = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool enable_offload;
	uint32_t gtk_offload_opcode;

	WMA_LOGD("%s Enter", __func__);

	/* Request target to enable GTK offload */
	if (params->ulFlags == GTK_OFFLOAD_ENABLE) {
		gtk_offload_opcode = GTK_OFFLOAD_ENABLE_OPCODE;
		wma->wow.gtk_err_enable[vdev_id] = true;

		/* Copy the keys and replay counter */
		qdf_mem_copy(offload_params.aKCK, params->aKCK,
			GTK_OFFLOAD_KCK_BYTES);
		qdf_mem_copy(offload_params.aKEK, params->aKEK,
			GTK_OFFLOAD_KEK_BYTES);
		qdf_mem_copy(&offload_params.ullKeyReplayCounter,
			&params->ullKeyReplayCounter, GTK_REPLAY_COUNTER_BYTES);
	} else {
		wma->wow.gtk_err_enable[vdev_id] = false;
		gtk_offload_opcode = GTK_OFFLOAD_DISABLE_OPCODE;
	}

	enable_offload = params->ulFlags;

	/* send the wmi command */
	status = wmi_unified_send_gtk_offload_cmd(wma->wmi_handle,
					vdev_id, &offload_params,
					enable_offload,
					gtk_offload_opcode);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	WMA_LOGD("VDEVID: %d, GTK_FLAGS: x%x", vdev_id, gtk_offload_opcode);
out:
	WMA_LOGD("%s Exit", __func__);
	return status;
}

/**
 * wma_process_gtk_offload_req() - process GTK offload req from umac
 * @handle: wma handle
 * @params: GTK offload params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_gtk_offload_req(tp_wma_handle wma,
				       tpSirGtkOffloadParams params)
{
	uint8_t vdev_id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	WMA_LOGD("%s Enter", __func__);

	/* Get the vdev id */
	if (!wma_find_vdev_by_bssid(wma, params->bssid.bytes, &vdev_id)) {
		WMA_LOGE("vdev handle is invalid for %pM", params->bssid.bytes);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	/* Validate vdev id */
	if (vdev_id >= WMA_MAX_SUPPORTED_BSS) {
		WMA_LOGE("invalid vdev_id %d for %pM", vdev_id,
			 params->bssid.bytes);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	if ((params->ulFlags == GTK_OFFLOAD_ENABLE) &&
	    (wma->wow.gtk_err_enable[vdev_id] == true)) {
		WMA_LOGE("%s GTK Offload already enabled. Disable it first "
			 "vdev_id %d", __func__, vdev_id);
		params->ulFlags = GTK_OFFLOAD_DISABLE;
		status = wma_send_gtk_offload_req(wma, vdev_id, params);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("%s Failed to disable GTK Offload", __func__);
			goto out;
		}
		WMA_LOGD("%s Enable GTK Offload again with updated inputs",
			 __func__);
		params->ulFlags = GTK_OFFLOAD_ENABLE;
	}
	status = wma_send_gtk_offload_req(wma, vdev_id, params);
out:
	qdf_mem_free(params);
	WMA_LOGD("%s Exit", __func__);
	return status;
}

/**
 * wma_process_gtk_offload_getinfo_req() - send GTK offload cmd to fw
 * @wma: wma handle
 * @params: GTK offload params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_gtk_offload_getinfo_req(tp_wma_handle wma,
				tpSirGtkOffloadGetInfoRspParams params)
{
	uint8_t vdev_id;
	uint64_t offload_req_opcode;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	WMA_LOGD("%s Enter", __func__);

	/* Get the vdev id */
	if (!wma_find_vdev_by_bssid(wma, params->bssid.bytes, &vdev_id)) {
		WMA_LOGE("vdev handle is invalid for %pM", params->bssid.bytes);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	/* Request for GTK offload status */
	offload_req_opcode = GTK_OFFLOAD_REQUEST_STATUS_OPCODE;

	/* send the wmi command */
	status = wmi_unified_process_gtk_offload_getinfo_cmd(wma->wmi_handle,
				 vdev_id, offload_req_opcode);

out:
	qdf_mem_free(params);
	WMA_LOGD("%s Exit", __func__);
	return status;
}
#endif /* WLAN_FEATURE_GTK_OFFLOAD */

/**
 * wma_enable_arp_ns_offload() - enable ARP NS offload
 * @wma: wma handle
 * @tpSirHostOffloadReq: offload request
 * @arp_only: flag
 *
 * To configure ARP NS off load data to firmware
 * when target goes to wow mode.
 *
 * Return: QDF Status
 */
QDF_STATUS wma_enable_arp_ns_offload(tp_wma_handle wma,
					    tpSirHostOffloadReq
					    pHostOffloadParams, bool arp_only)
{
	int32_t res;
	uint8_t vdev_id;

	/* Get the vdev id */
	if (!wma_find_vdev_by_bssid(wma, pHostOffloadParams->bssid.bytes,
					&vdev_id)) {
		WMA_LOGE("vdev handle is invalid for %pM",
			 pHostOffloadParams->bssid.bytes);
		qdf_mem_free(pHostOffloadParams);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma->interfaces[vdev_id].vdev_up) {
		WMA_LOGE("vdev %d is not up skipping arp/ns offload", vdev_id);
		qdf_mem_free(pHostOffloadParams);
		return QDF_STATUS_E_FAILURE;
	}


	res = wmi_unified_enable_arp_ns_offload_cmd(wma->wmi_handle,
				     (struct host_offload_req_param *)pHostOffloadParams,
				     arp_only,
					 vdev_id);
	if (res) {
		WMA_LOGE("Failed to enable ARP NDP/NSffload");
		qdf_mem_free(pHostOffloadParams);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_free(pHostOffloadParams);
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
	ol_txrx_pdev_handle pdev;
	struct ol_txrx_peer_t *peer;
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
	wmi_peer_info_req_cmd_fixed_param *p_get_peer_info_cmd;
	uint8_t bcast_mac[IEEE80211_ADDR_LEN] = { 0xff, 0xff, 0xff,
						  0xff, 0xff, 0xff };

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
		peer = ol_txrx_peer_find_by_local_id(pdev, pReq->staIdx);
		if (!peer) {
			WMA_LOGE("%s: Failed to get peer handle using peer id %d",
				__func__, pReq->staIdx);
			return QDF_STATUS_E_FAILURE;
		}
		WMA_LOGE("%s: staIdx %d peer mac: 0x%2x:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x",
			__func__, pReq->staIdx, peer->mac_addr.raw[0],
			peer->mac_addr.raw[1], peer->mac_addr.raw[2],
			peer->mac_addr.raw[3], peer->mac_addr.raw[4],
			peer->mac_addr.raw[5]);
		qdf_mem_copy(peer_mac, peer->mac_addr.raw, IEEE80211_ADDR_LEN);
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

	WMA_LOGE("IBSS get peer info cmd sent len: %d, vdev %d"
		 " command id: %d, status: %d", len,
		 p_get_peer_info_cmd->vdev_id, WMI_PEER_INFO_REQ_CMDID, ret);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_tx_fail_monitor_ind() - sends tx fail monitor cmd to target
 * @wma: wma handle
 * @pReq: tx fail monitor command params
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_tx_fail_monitor_ind
	(tp_wma_handle wma, tAniTXFailMonitorInd *pReq)
{
	QDF_STATUS ret;
	int32_t vdev_id;

	vdev_id = wma_find_vdev_by_type(wma, WMI_VDEV_TYPE_IBSS);
	if (vdev_id < 0) {
		WMA_LOGE("%s: IBSS vdev does not exist could not send fast tx fail"
			" monitor indication message to target", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Send enable cesium command to target */
	WMA_LOGE("send fast tx fail monitor ind cmd target for vdevId %d val %d",
		vdev_id, pReq->tx_fail_count);

	if (0 == pReq->tx_fail_count) {
		wma->hddTxFailCb = NULL;
	} else {
		wma->hddTxFailCb = pReq->txFailIndCallback;
	}
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

	WMA_LOGE("Enable RMC cmd sent len: %d, vdev %d" " command id: %d,"
		 " status: %d", len, p_rmc_enable_cmd->vdev_id,
		 WMI_RMC_SET_MODE_CMDID, ret);

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

	WMA_LOGE("Disable RMC cmd sent len: %d, vdev %d" " command id: %d,"
		 " status: %d", len, p_rmc_disable_cmd->vdev_id,
		 WMI_RMC_SET_MODE_CMDID, ret);

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
	uint32_t val;
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
		WMA_LOGE("%s: IBSS vdev does not exist could not send"
			 " RMC action period to target", __func__);
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

	if (wlan_cfg_get_int(mac, WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY, &val)
	    != eSIR_SUCCESS) {
		WMA_LOGE("Failed to get value for RMC action period using default");
		val = WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY_STADEF;
	}

	p_rmc_cmd->vdev_id = vdev_id;
	p_rmc_cmd->periodicity_msec = val;

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_RMC_SET_ACTION_PERIOD_CMDID);

	WMA_LOGE("RMC action period %d cmd sent len: %d, vdev %d"
		 " command id: %d, status: %d", p_rmc_cmd->periodicity_msec,
		 len, p_rmc_cmd->vdev_id, WMI_RMC_SET_ACTION_PERIOD_CMDID, ret);

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
	struct periodic_tx_pattern params;
	uint8_t vdev_id;

	qdf_mem_set(&params, sizeof(struct periodic_tx_pattern), 0);

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue fw add pattern cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_find_vdev_by_addr(wma_handle,
				   pAddPeriodicTxPtrnParams->mac_address.bytes,
				   &vdev_id)) {
		WMA_LOGE("%s: Failed to find vdev id for %pM", __func__,
			 pAddPeriodicTxPtrnParams->mac_address.bytes);
		return QDF_STATUS_E_INVAL;
	}

	params.ucPtrnId = pAddPeriodicTxPtrnParams->ucPtrnId;
	params.ucPtrnSize = pAddPeriodicTxPtrnParams->ucPtrnSize;
	params.usPtrnIntervalMs = pAddPeriodicTxPtrnParams->usPtrnIntervalMs;
	qdf_mem_copy(&params.mac_address,
			&pAddPeriodicTxPtrnParams->mac_address,
			sizeof(struct qdf_mac_addr));
	qdf_mem_copy(params.ucPattern, pAddPeriodicTxPtrnParams->ucPattern,
					params.ucPtrnSize);

	return wmi_unified_process_add_periodic_tx_ptrn_cmd(
			wma_handle->wmi_handle,	&params, vdev_id);
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
	cds_msg_t cds_msg;
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

	cds_msg.type = eWNI_SME_READY_TO_EXTWOW_IND;
	cds_msg.bodyptr = (void *)ready_to_extwow;
	cds_msg.bodyval = 0;

	vstatus = cds_mq_post_message(CDS_MQ_ID_SME, &cds_msg);
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
	params.dhcpSrvOffloadEnabled =
				pDhcpSrvOffloadInfo->dhcpSrvOffloadEnabled;
	params.dhcpClientNum = pDhcpSrvOffloadInfo->dhcpClientNum;
	params.dhcpSrvIP = pDhcpSrvOffloadInfo->;

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
				tSirLedFlashingReq *flashing)
{
	struct flashing_req_params cmd = {0};

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE(FL("WMA is closed, can not issue cmd"));
		return QDF_STATUS_E_INVAL;
	}
	if (!flashing) {
		WMA_LOGE(FL("invalid parameter: flashing"));
		return QDF_STATUS_E_INVAL;
	}
	cmd.req_id = flashing->reqId;
	cmd.pattern_id = flashing->pattern_id;
	cmd.led_x0 = flashing->led_x0;
	cmd.led_x1 = flashing->led_x1;
	status = wmi_unified_set_led_flashing_cmd(wma_handle->wmi_handle,
				      &cmd);
	if (status != EOK) {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_GPIO_LED_FLASHING */

#ifdef FEATURE_WLAN_CH_AVOID
/**
 * wma_channel_avoid_evt_handler() -  process channel to avoid event from FW.
 * @handle: wma handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_channel_avoid_evt_handler(void *handle, uint8_t *event,
				  uint32_t len)
{
	wmi_avoid_freq_ranges_event_fixed_param *afr_fixed_param;
	wmi_avoid_freq_range_desc *afr_desc;
	uint32_t num_freq_ranges, freq_range_idx;
	tSirChAvoidIndType *sca_indication;
	QDF_STATUS qdf_status;
	cds_msg_t sme_msg = { 0 };
	WMI_WLAN_FREQ_AVOID_EVENTID_param_tlvs *param_buf =
		(WMI_WLAN_FREQ_AVOID_EVENTID_param_tlvs *) event;

	if (!param_buf) {
		WMA_LOGE("Invalid channel avoid event buffer");
		return -EINVAL;
	}

	afr_fixed_param = param_buf->fixed_param;
	if (!afr_fixed_param) {
		WMA_LOGE("Invalid channel avoid event fixed param buffer");
		return -EINVAL;
	}

	num_freq_ranges =
		(afr_fixed_param->num_freq_ranges >
		 SIR_CH_AVOID_MAX_RANGE) ? SIR_CH_AVOID_MAX_RANGE :
		afr_fixed_param->num_freq_ranges;

	WMA_LOGD("Channel avoid event received with %d ranges",
		 num_freq_ranges);
	for (freq_range_idx = 0; freq_range_idx < num_freq_ranges;
	     freq_range_idx++) {
		afr_desc = (wmi_avoid_freq_range_desc *)
				((void *)param_buf->avd_freq_range +
				freq_range_idx * sizeof(wmi_avoid_freq_range_desc));

		WMA_LOGD("range %d: tlv id = %u, start freq = %u,  end freq = %u",
			freq_range_idx, afr_desc->tlv_header, afr_desc->start_freq,
			afr_desc->end_freq);
	}

	sca_indication = (tSirChAvoidIndType *)
			 qdf_mem_malloc(sizeof(tSirChAvoidIndType));
	if (!sca_indication) {
		WMA_LOGE("Invalid channel avoid indication buffer");
		return -EINVAL;
	}

	sca_indication->avoid_range_count = num_freq_ranges;
	for (freq_range_idx = 0; freq_range_idx < num_freq_ranges;
	     freq_range_idx++) {
		afr_desc = (wmi_avoid_freq_range_desc *)
				((void *)param_buf->avd_freq_range +
				freq_range_idx * sizeof(wmi_avoid_freq_range_desc));
		sca_indication->avoid_freq_range[freq_range_idx].start_freq =
			afr_desc->start_freq;
		sca_indication->avoid_freq_range[freq_range_idx].end_freq =
			afr_desc->end_freq;
	}

	sme_msg.type = eWNI_SME_CH_AVOID_IND;
	sme_msg.bodyptr = sca_indication;
	sme_msg.bodyval = 0;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Fail to post eWNI_SME_CH_AVOID_IND msg to SME");
		qdf_mem_free(sca_indication);
		return -EINVAL;
	}

	return 0;
}

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

	WMA_LOGI("%s: WMA --> WMI_CHAN_AVOID_UPDATE", __func__);

	status = wmi_unified_process_ch_avoid_update_cmd(
					wma_handle->wmi_handle);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	WMA_LOGI("%s: WMA --> WMI_CHAN_AVOID_UPDATE sent through WMI",
		 __func__);
	return status;
}
#endif /* FEATURE_WLAN_CH_AVOID */

/**
 * wma_set_reg_domain() - set reg domain
 * @clientCtxt: client context
 * @regId: reg id
 *
 * Return: QDF status
 */
QDF_STATUS wma_set_reg_domain(void *clientCtxt, v_REGDOMAIN_t regId)
{
	if (QDF_STATUS_SUCCESS !=
	    cds_set_reg_domain(clientCtxt, regId))
		return QDF_STATUS_E_INVAL;

	return QDF_STATUS_SUCCESS;
}

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
				   uint16_t regdmn5G, int8_t ctl2G,
				   int8_t ctl5G)
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

	if ((((reg_dmn & ~COUNTRY_ERD_FLAG) == CTRY_JAPAN14) ||
	     ((reg_dmn & ~COUNTRY_ERD_FLAG) == CTRY_KOREA_ROC)) &&
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

	return;
}

/**
 * wma_post_runtime_resume_msg() - post the resume request
 * @handle: validated wma handle
 *
 * request the MC thread unpaus the vdev and set resume dtim
 *
 * Return: qdf status of the mq post
 */
static QDF_STATUS wma_post_runtime_resume_msg(WMA_HANDLE handle)
{
	cds_msg_t resume_msg;
	QDF_STATUS status;
	tp_wma_handle wma = (tp_wma_handle) handle;

	qdf_runtime_pm_prevent_suspend(wma->wma_runtime_resume_lock);

	resume_msg.bodyptr = NULL;
	resume_msg.type    = WMA_RUNTIME_PM_RESUME_IND;

	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &resume_msg);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		WMA_LOGE("Failed to post Runtime PM Resume IND to VOS");
		qdf_runtime_pm_allow_suspend(wma->wma_runtime_resume_lock);
	}

	return status;
}

/**
 * wma_post_runtime_suspend_msg() - post the suspend request
 * @handle: validated wma handle
 *
 * Requests for offloads to be configured for runtime suspend
 * on the MC thread
 *
 * Return QDF_STATUS_E_AGAIN in case of timeout or QDF_STATUS_SUCCESS
 */
static QDF_STATUS wma_post_runtime_suspend_msg(WMA_HANDLE handle)
{
	cds_msg_t cds_msg;
	QDF_STATUS qdf_status;
	tp_wma_handle wma = (tp_wma_handle) handle;

	qdf_event_reset(&wma->runtime_suspend);

	cds_msg.bodyptr = NULL;
	cds_msg.type    = WMA_RUNTIME_PM_SUSPEND_IND;
	qdf_status = cds_mq_post_message(QDF_MODULE_ID_WMA, &cds_msg);

	if (qdf_status != QDF_STATUS_SUCCESS)
		goto failure;

	if (qdf_wait_single_event(&wma->runtime_suspend,
			WMA_TGT_SUSPEND_COMPLETE_TIMEOUT) !=
			QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to get runtime suspend event");
		goto msg_timed_out;
	}

	return QDF_STATUS_SUCCESS;

msg_timed_out:
	wma_post_runtime_resume_msg(wma);
failure:
	return QDF_STATUS_E_AGAIN;
}

/**
 * __wma_bus_suspend(): handles bus suspend for wma
 * @type: is this suspend part of runtime suspend or system suspend?
 *
 * Bails if a scan is in progress.
 * Calls the appropriate handlers based on configuration and event.
 *
 * Return: 0 for success or error code
 */
static int __wma_bus_suspend(enum qdf_suspend_type type)
{
	WMA_HANDLE handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (NULL == handle) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return -EFAULT;
	}

	if (wma_check_scan_in_progress(handle)) {
		WMA_LOGE("%s: Scan in progress. Aborting suspend", __func__);
		return -EBUSY;
	}

	if (type == QDF_RUNTIME_SUSPEND) {
		QDF_STATUS status = wma_post_runtime_suspend_msg(handle);
		if (status)
			return qdf_status_to_os_return(status);
	}

	if (type == QDF_SYSTEM_SUSPEND)
		WMA_LOGE("%s: wow mode selected %d", __func__,
				wma_is_wow_mode_selected(handle));

	if (wma_is_wow_mode_selected(handle)) {
		QDF_STATUS status = wma_enable_wow_in_fw(handle);
		return qdf_status_to_os_return(status);
	}

	return wma_suspend_target(handle, 0);
}

/**
 * wma_runtime_suspend() - handles runtime suspend request from hdd
 *
 * Calls the appropriate handler based on configuration and event.
 * Last busy marking should prevent race conditions between processing
 * of asyncronous fw events and the running of runtime suspend.
 * (eg. last busy marking should guarantee that any auth requests have
 * been processed)
 * Events comming from the host are not protected, but aren't expected
 * to be an issue.
 *
 * Return: 0 for success or error code
 */
int wma_runtime_suspend(void)
{
	return __wma_bus_suspend(QDF_RUNTIME_SUSPEND);
}

/**
 * wma_bus_suspend() - handles bus suspend request from hdd
 *
 * Calls the appropriate handler based on configuration and event
 *
 * Return: 0 for success or error code
 */
int wma_bus_suspend(void)
{

	return __wma_bus_suspend(QDF_SYSTEM_SUSPEND);
}

/**
 * __wma_bus_resume() - bus resume for wma
 *
 * does the part of the bus resume common to bus and system suspend
 *
 * Return: os error code.
 */
int __wma_bus_resume(WMA_HANDLE handle)
{
	bool wow_mode = wma_is_wow_mode_selected(handle);
	QDF_STATUS status;

	WMA_LOGE("%s: wow mode %d", __func__, wow_mode);

	if (!wow_mode)
		return qdf_status_to_os_return(wma_resume_target(handle));

	status = wma_disable_wow_in_fw(handle);
	return qdf_status_to_os_return(status);
}

/**
 * wma_runtime_resume() - do the runtime resume operation for wma
 *
 * Return: os error code.
 */
int wma_runtime_resume(void)
{
	int ret;
	QDF_STATUS status;
	WMA_HANDLE handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (NULL == handle) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return -EFAULT;
	}

	ret = __wma_bus_resume(handle);
	if (ret)
		return ret;

	status = wma_post_runtime_resume_msg(handle);
	return qdf_status_to_os_return(status);
}

/**
 * wma_bus_resume() - handles bus resume request from hdd
 * @handle: valid wma handle
 *
 * Calls the appropriate handler based on configuration
 *
 * Return: 0 for success or error code
 */
int wma_bus_resume(void)
{
	WMA_HANDLE handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (NULL == handle) {
		WMA_LOGE("%s: wma context is NULL", __func__);
		return -EFAULT;
	}

	return __wma_bus_resume(handle);
}

/**
 * wma_suspend_target_timeout() - Handles the target suspend timeout
 * @is_self_recovery_enabled: Is self recovery enabled or not
 *
 * Return: NONE
 */
#ifdef QCA_WIFI_3_0_ADRASTEA
static inline void wma_suspend_target_timeout(bool is_self_recovery_enabled)
{
	if (cds_is_driver_recovering()) {
		WMA_LOGE("%s: recovery is in progress, ignore!", __func__);
	} else {
		if (is_self_recovery_enabled) {
			cds_trigger_recovery();
		} else {
			QDF_BUG(0);
		}
	}
}
#else /* ROME chipset */
static inline void wma_suspend_target_timeout(bool is_self_recovery_enabled)
{
	if (cds_is_load_or_unload_in_progress() || cds_is_driver_recovering()) {
		WMA_LOGE("%s: Unloading/Loading/recovery is in progress, Ignore!",
			 __func__);
	} else {
		if (is_self_recovery_enabled) {
			cds_trigger_recovery();
		} else {
			QDF_BUG(0);
		}
	}
}
#endif

/**
 * wma_suspend_target() - suspend target
 * @handle: wma handle
 * @disable_target_intr: disable target interrupt
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_suspend_target(WMA_HANDLE handle, int disable_target_intr)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct hif_opaque_softc *scn;
	QDF_STATUS status;
	struct suspend_params param = {0};

	tpAniSirGlobal pmac = cds_get_context(QDF_MODULE_ID_PE);

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("WMA is closed. can not issue suspend cmd");
		return QDF_STATUS_E_INVAL;
	}

	if (NULL == pmac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_INVAL;
	}

	qdf_event_reset(&wma_handle->target_suspend);
	param.disable_target_intr = disable_target_intr;
	status = wmi_unified_suspend_send(wma_handle->wmi_handle,
				&param,
				WMA_WILDCARD_PDEV_ID);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wmi_set_target_suspend(wma_handle->wmi_handle, true);

	if (qdf_wait_single_event(&wma_handle->target_suspend,
				  WMA_TGT_SUSPEND_COMPLETE_TIMEOUT)
	    != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to get ACK from firmware for pdev suspend");
		wmi_set_target_suspend(wma_handle->wmi_handle, false);
		wma_suspend_target_timeout(pmac->sme.enableSelfRecovery);
		return QDF_STATUS_E_FAULT;
	}

	scn = cds_get_context(QDF_MODULE_ID_HIF);

	if (scn == NULL) {
		WMA_LOGE("%s: Failed to get HIF context", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_target_suspend_acknowledge() - update target susspend status
 * @context: HTC_INIT_INFO->context
 * @wow_nack: true when wow is rejected
 *
 * Return: none
 */
void wma_target_suspend_acknowledge(void *context, bool wow_nack)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma) {
		WMA_LOGE("%s: wma is NULL", __func__);
		return;
	}

	wma->wow_nack = wow_nack;
	qdf_event_set(&wma->target_suspend);
	if (wow_nack)
		qdf_wake_lock_timeout_acquire(&wma->wow_wake_lock,
					      WMA_WAKE_LOCK_TIMEOUT,
					      WIFI_POWER_EVENT_WAKELOCK_WOW);
}

/**
 * wma_resume_target() - resume target
 * @handle: wma handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_resume_target(WMA_HANDLE handle)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
#ifdef CONFIG_CNSS
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);
	if (NULL == pMac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		return QDF_STATUS_E_INVAL;
	}
#endif /* CONFIG_CNSS */

	qdf_event_reset(&wma->wma_resume_event);
	qdf_status = wmi_unified_resume_send(wma->wmi_handle,
					WMA_WILDCARD_PDEV_ID);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		WMA_LOGE("Failed to send WMI_PDEV_RESUME_CMDID command");

	qdf_status = qdf_wait_single_event(&(wma->wma_resume_event),
			WMA_RESUME_TIMEOUT);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		WMA_LOGP("%s: Timeout waiting for resume event from FW",
			__func__);
		WMA_LOGP("%s: Pending commands %d credits %d", __func__,
			wmi_get_pending_cmds(wma->wmi_handle),
			wmi_get_host_credits(wma->wmi_handle));
		if (!cds_is_driver_recovering()) {
#ifdef CONFIG_CNSS
			if (pMac->sme.enableSelfRecovery) {
				cds_trigger_recovery();
			} else {
				QDF_BUG(0);
			}
#else
			QDF_BUG(0);
#endif /* CONFIG_CNSS */
		} else {
			WMA_LOGE("%s: SSR in progress, ignore resume timeout",
				__func__);
		}
	} else {
		WMA_LOGD("Host wakeup received");
	}

	if (QDF_STATUS_SUCCESS == qdf_status)
		wmi_set_target_suspend(wma->wmi_handle, false);

	return qdf_status;
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
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_TDLS_PEER_EVENTID_param_tlvs *param_buf = NULL;
	wmi_tdls_peer_event_fixed_param *peer_event = NULL;
	tSirTdlsEventnotify *tdls_event;

	if (!event) {
		WMA_LOGE("%s: event param null", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_TDLS_PEER_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("%s: received null buf from target", __func__);
		return -EINVAL;
	}

	peer_event = param_buf->fixed_param;
	if (!peer_event) {
		WMA_LOGE("%s: received null event data from target", __func__);
		return -EINVAL;
	}

	tdls_event = (tSirTdlsEventnotify *)
		     qdf_mem_malloc(sizeof(*tdls_event));
	if (!tdls_event) {
		WMA_LOGE("%s: failed to allocate memory for tdls_event",
			 __func__);
		return -ENOMEM;
	}

	tdls_event->sessionId = peer_event->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&peer_event->peer_macaddr,
				   tdls_event->peermac.bytes);

	switch (peer_event->peer_status) {
	case WMI_TDLS_SHOULD_DISCOVER:
		tdls_event->messageType = WMA_TDLS_SHOULD_DISCOVER_CMD;
		break;
	case WMI_TDLS_SHOULD_TEARDOWN:
		tdls_event->messageType = WMA_TDLS_SHOULD_TEARDOWN_CMD;
		break;
	case WMI_TDLS_PEER_DISCONNECTED:
		tdls_event->messageType = WMA_TDLS_PEER_DISCONNECTED_CMD;
		break;
	case WMI_TDLS_CONNECTION_TRACKER_NOTIFICATION:
		tdls_event->messageType =
			WMA_TDLS_CONNECTION_TRACKER_NOTIFICATION_CMD;
		break;
	default:
		WMA_LOGE("%s: Discarding unknown tdls event(%d) from target",
			 __func__, peer_event->peer_status);
		return -EINVAL;
	}

	switch (peer_event->peer_reason) {
	case WMI_TDLS_TEARDOWN_REASON_TX:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_TX;
		break;
	case WMI_TDLS_TEARDOWN_REASON_RSSI:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_RSSI;
		break;
	case WMI_TDLS_TEARDOWN_REASON_SCAN:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_SCAN;
		break;
	case WMI_TDLS_DISCONNECTED_REASON_PEER_DELETE:
		tdls_event->peer_reason =
			eWNI_TDLS_DISCONNECTED_REASON_PEER_DELETE;
		break;
	case WMI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT;
		break;
	case WMI_TDLS_TEARDOWN_REASON_BAD_PTR:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_BAD_PTR;
		break;
	case WMI_TDLS_TEARDOWN_REASON_NO_RESPONSE:
		tdls_event->peer_reason = eWNI_TDLS_TEARDOWN_REASON_NO_RESPONSE;
		break;
	case WMI_TDLS_ENTER_BUF_STA:
		tdls_event->peer_reason = eWNI_TDLS_PEER_ENTER_BUF_STA;
		break;
	case WMI_TDLS_EXIT_BUF_STA:
		tdls_event->peer_reason = eWNI_TDLS_PEER_EXIT_BUF_STA;
		break;
	case WMI_TDLS_ENTER_BT_BUSY_MODE:
		tdls_event->peer_reason = eWNI_TDLS_ENTER_BT_BUSY_MODE;
		break;
	case WMI_TDLS_EXIT_BT_BUSY_MODE:
		tdls_event->peer_reason = eWNI_TDLS_EXIT_BT_BUSY_MODE;
		break;
	default:
		WMA_LOGE("%s: unknown reason(%d) in tdls event(%d) from target",
			 __func__, peer_event->peer_reason,
			 peer_event->peer_status);
		return -EINVAL;
	}

	WMA_LOGD("%s: sending msg to umac, messageType: 0x%x, "
		 "for peer: %pM, reason: %d, smesessionId: %d",
		 __func__, tdls_event->messageType, tdls_event->peermac.bytes,
		 tdls_event->peer_reason, tdls_event->sessionId);

	wma_send_msg(wma, tdls_event->messageType, (void *)tdls_event, 0);
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
 * wma_update_fw_tdls_state() - send enable/disable tdls for a vdev
 * @wma: wma handle
 * @pwmaTdlsparams: TDLS params
 *
 * Return: 0 for sucess or error code
 */
QDF_STATUS wma_update_fw_tdls_state(WMA_HANDLE handle, void *pwmaTdlsparams)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	t_wma_tdls_mode tdls_mode;
	t_wma_tdls_params *wma_tdls = (t_wma_tdls_params *) pwmaTdlsparams;
	struct wmi_tdls_params params = {0};
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t tdls_state;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue fw tdls state cmd",
			 __func__);
		ret = -EINVAL;
		goto end_fw_tdls_state;
	}

	params.tdls_state = wma_tdls->tdls_state;
	tdls_mode = wma_tdls->tdls_state;

	if (WMA_TDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY == tdls_mode) {
		tdls_state = WMI_TDLS_ENABLE_PASSIVE;
	} else if (WMA_TDLS_SUPPORT_ENABLED == tdls_mode) {
		tdls_state = WMI_TDLS_ENABLE_CONNECTION_TRACKER_IN_HOST;
	} else if (WMA_TDLS_SUPPORT_ACTIVE_EXTERNAL_CONTROL == tdls_mode) {
		tdls_state = WMI_TDLS_ENABLE_CONNECTION_TRACKER_IN_HOST;
	} else {
		tdls_state = WMI_TDLS_DISABLE;
	}

	params.vdev_id = wma_tdls->vdev_id;
	params.notification_interval_ms = wma_tdls->notification_interval_ms;
	params.tx_discovery_threshold = wma_tdls->tx_discovery_threshold;
	params.tx_teardown_threshold = wma_tdls->tx_teardown_threshold;
	params.rssi_teardown_threshold = wma_tdls->rssi_teardown_threshold;
	params.rssi_delta = wma_tdls->rssi_delta;
	params.tdls_options = wma_tdls->tdls_options;
	params.peer_traffic_ind_window = wma_tdls->peer_traffic_ind_window;
	params.peer_traffic_response_timeout =
		wma_tdls->peer_traffic_response_timeout;
	params.puapsd_mask = wma_tdls->puapsd_mask;
	params.puapsd_inactivity_time = wma_tdls->puapsd_inactivity_time;
	params.puapsd_rx_frame_threshold =
		wma_tdls->puapsd_rx_frame_threshold;
	params.teardown_notification_ms =
		wma_tdls->teardown_notification_ms;
	params.tdls_peer_kickout_threshold =
		wma_tdls->tdls_peer_kickout_threshold;

	ret = wmi_unified_update_fw_tdls_state_cmd(wma_handle->wmi_handle,
					&params, tdls_state);
	if (QDF_IS_STATUS_ERROR(ret))
		goto end_fw_tdls_state;

	WMA_LOGD("%s: vdev_id %d", __func__, wma_tdls->vdev_id);

end_fw_tdls_state:
	if (pwmaTdlsparams)
		qdf_mem_free(pwmaTdlsparams);
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
	ol_txrx_pdev_handle pdev;
	uint8_t peer_id;
	ol_txrx_peer_handle peer;
	uint8_t *peer_mac_addr;
	int ret = 0;
	uint32_t *ch_mhz = NULL;
	bool restore_last_peer = false;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue cmd", __func__);
		ret = -EINVAL;
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
		pdev = cds_get_context(QDF_MODULE_ID_TXRX);
		if (!pdev) {
			WMA_LOGE("%s: Failed to find pdev", __func__);
			ret = -EIO;
			goto end_tdls_peer_state;
		}

		peer = ol_txrx_find_peer_by_addr(pdev,
						 peerStateParams->peerMacAddr,
						 &peer_id);
		if (!peer) {
			WMA_LOGE("%s: Failed to get peer handle using peer mac %pM",
				__func__, peerStateParams->peerMacAddr);
			ret = -EIO;
			goto end_tdls_peer_state;
		}
		peer_mac_addr = ol_txrx_peer_get_peer_mac_addr(peer);
		restore_last_peer = is_vdev_restore_last_peer(peer);

		WMA_LOGD("%s: calling wma_remove_peer for peer " MAC_ADDRESS_STR
			 " vdevId: %d", __func__,
			 MAC_ADDR_ARRAY(peer_mac_addr),
			 peerStateParams->vdevId);
		wma_remove_peer(wma_handle, peer_mac_addr,
				peerStateParams->vdevId, peer, false);
		ol_txrx_update_last_real_peer(pdev, peer, &peer_id,
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


/**
 * wma_dfs_attach() - Attach DFS methods to the umac state.
 * @dfs_ic: ieee80211com ptr
 *
 * Return: Return ieee80211com ptr with updated info
 */
struct ieee80211com *wma_dfs_attach(struct ieee80211com *dfs_ic)
{
	/*Allocate memory for dfs_ic before passing it up to dfs_attach() */
	dfs_ic = (struct ieee80211com *)
		 os_malloc(NULL, sizeof(struct ieee80211com), GFP_ATOMIC);
	if (dfs_ic == NULL) {
		WMA_LOGE("%s:Allocation of dfs_ic failed %zu",
			 __func__, sizeof(struct ieee80211com));
		return NULL;
	}
	OS_MEMZERO(dfs_ic, sizeof(struct ieee80211com));
	/* DFS pattern matching hooks */
	dfs_ic->ic_dfs_attach = ol_if_dfs_attach;
	dfs_ic->ic_dfs_disable = ol_if_dfs_disable;
	dfs_ic->ic_find_channel = ieee80211_find_channel;
	dfs_ic->ic_dfs_enable = ol_if_dfs_enable;
	dfs_ic->ic_ieee2mhz = ieee80211_ieee2mhz;

	/* Hardware facing hooks */
	dfs_ic->ic_get_ext_busy = ol_if_dfs_get_ext_busy;
	dfs_ic->ic_get_mib_cycle_counts_pct =
		ol_if_dfs_get_mib_cycle_counts_pct;
	dfs_ic->ic_get_TSF64 = ol_if_get_tsf64;

	/* NOL related hooks */
	dfs_ic->ic_dfs_usenol = ol_if_dfs_usenol;
	/*
	 * Hooks from wma/dfs/ back
	 * into the PE/SME
	 * and shared DFS code
	 */
	dfs_ic->ic_dfs_notify_radar = ieee80211_mark_dfs;
	qdf_spinlock_create(&dfs_ic->chan_lock);
	/* Initializes DFS Data Structures and queues */
	dfs_attach(dfs_ic);

	return dfs_ic;
}

/**
 * wma_dfs_detach() - Detach DFS methods
 * @dfs_ic: ieee80211com ptr
 *
 * Return: none
 */
void wma_dfs_detach(struct ieee80211com *dfs_ic)
{
	dfs_detach(dfs_ic);

	qdf_spinlock_destroy(&dfs_ic->chan_lock);
	if (NULL != dfs_ic->ic_curchan) {
		OS_FREE(dfs_ic->ic_curchan);
		dfs_ic->ic_curchan = NULL;
	}

	OS_FREE(dfs_ic);
}

/**
 * wma_dfs_configure() - configure dfs
 * @ic: ieee80211com ptr
 *
 * Configures Radar Filters during
 * vdev start/channel change/regulatory domain
 * change.This Configuration enables to program
 * the DFS pattern matching module.
 *
 * Return: none
 */
void wma_dfs_configure(struct ieee80211com *ic)
{
	struct ath_dfs_radar_tab_info rinfo;
	int dfsdomain;
	int radar_enabled_status = 0;
	if (ic == NULL) {
		WMA_LOGE("%s: DFS ic is Invalid", __func__);
		return;
	}

	dfsdomain = ic->current_dfs_regdomain;

	/* Fetch current radar patterns from the lmac */
	OS_MEMZERO(&rinfo, sizeof(rinfo));

	/*
	 * Look up the current DFS
	 * regulatory domain and decide
	 * which radar pulses to use.
	 */
	switch (dfsdomain) {
	case DFS_FCC_REGION:
		WMA_LOGI("%s: DFS-FCC domain", __func__);
		rinfo.dfsdomain = DFS_FCC_REGION;
		rinfo.dfs_radars = dfs_fcc_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_fcc_radars);
		rinfo.b5pulses = dfs_fcc_bin5pulses;
		rinfo.numb5radars = QDF_ARRAY_SIZE(dfs_fcc_bin5pulses);
		break;
	case DFS_ETSI_REGION:
		WMA_LOGI("%s: DFS-ETSI domain", __func__);
		rinfo.dfsdomain = DFS_ETSI_REGION;
		rinfo.dfs_radars = dfs_etsi_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_etsi_radars);
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	case DFS_MKK_REGION:
		WMA_LOGI("%s: DFS-MKK domain", __func__);
		rinfo.dfsdomain = DFS_MKK_REGION;
		rinfo.dfs_radars = dfs_mkk4_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_mkk4_radars);
		rinfo.b5pulses = dfs_jpn_bin5pulses;
		rinfo.numb5radars = QDF_ARRAY_SIZE(dfs_jpn_bin5pulses);
		break;
	case DFS_CN_REGION:
		WMA_LOGI("%s: DFS-CN domain", __func__);
		rinfo.dfsdomain = DFS_CN_REGION;
		rinfo.dfs_radars = dfs_china_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_china_radars);
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	case DFS_KR_REGION:
		WMA_LOGI("%s: DFS-KR domain", __func__);
		rinfo.dfsdomain = DFS_KR_REGION;
		rinfo.dfs_radars = dfs_korea_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_korea_radars);
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	default:
		WMA_LOGI("%s: DFS-UNINT domain", __func__);
		rinfo.dfsdomain = DFS_UNINIT_REGION;
		rinfo.dfs_radars = NULL;
		rinfo.numradars = 0;
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	}

	rinfo.dfs_pri_multiplier = ic->dfs_pri_multiplier;

	/*
	 * Set the regulatory domain,
	 * radar pulse table and enable
	 * radar events if required.
	 * dfs_radar_enable() returns
	 * 0 on success and non-zero
	 * failure.
	 */
	radar_enabled_status = dfs_radar_enable(ic, &rinfo);
	if (radar_enabled_status != DFS_STATUS_SUCCESS) {
		WMA_LOGE("%s[%d]: DFS- Radar Detection Enabling Failed",
			 __func__, __LINE__);
	}
}

/**
 * wma_dfs_configure_channel() - configure DFS channel
 * @dfs_ic: ieee80211com ptr
 * @band_center_freq1: center frequency 1
 * @band_center_freq2: center frequency 2
 *       (valid only for 11ac vht 80plus80 mode)
 * @ req: vdev start request
 *
 * Set the Channel parameters in to DFS module
 * Also,configure the DFS radar filters for
 * matching the DFS phyerrors.
 *
 * Return: dfs_ieee80211_channel / NULL for error
 */
struct dfs_ieee80211_channel *wma_dfs_configure_channel(
						struct ieee80211com *dfs_ic,
						uint32_t band_center_freq1,
						uint32_t band_center_freq2,
						struct wma_vdev_start_req
						*req)
{
	uint8_t ext_channel;

	if (dfs_ic == NULL) {
		WMA_LOGE("%s: DFS ic is Invalid", __func__);
		return NULL;
	}

	if (!dfs_ic->ic_curchan) {
		dfs_ic->ic_curchan = (struct dfs_ieee80211_channel *)os_malloc(
					NULL,
					sizeof(struct dfs_ieee80211_channel),
					GFP_ATOMIC);
		if (dfs_ic->ic_curchan == NULL) {
			WMA_LOGE(
			    "%s: allocation of dfs_ic->ic_curchan failed %zu",
			    __func__, sizeof(struct dfs_ieee80211_channel));
			return NULL;
		}
	}

	OS_MEMZERO(dfs_ic->ic_curchan, sizeof(struct dfs_ieee80211_channel));

	dfs_ic->ic_curchan->ic_ieee = req->chan;
	dfs_ic->ic_curchan->ic_freq = cds_chan_to_freq(req->chan);
	dfs_ic->ic_curchan->ic_vhtop_ch_freq_seg1 = band_center_freq1;
	dfs_ic->ic_curchan->ic_vhtop_ch_freq_seg2 = band_center_freq2;
	dfs_ic->ic_curchan->ic_pri_freq_center_freq_mhz_separation =
					dfs_ic->ic_curchan->ic_freq -
				dfs_ic->ic_curchan->ic_vhtop_ch_freq_seg1;

	if ((dfs_ic->ic_curchan->ic_ieee >= WMA_11A_CHANNEL_BEGIN) &&
	    (dfs_ic->ic_curchan->ic_ieee <= WMA_11A_CHANNEL_END)) {
		dfs_ic->ic_curchan->ic_flags |= IEEE80211_CHAN_5GHZ;
	}

	switch (req->chan_width) {
	case CH_WIDTH_20MHZ:
		dfs_ic->ic_curchan->ic_flags |=
				(req->vht_capable ? IEEE80211_CHAN_VHT20 :
							IEEE80211_CHAN_HT20);
		break;
	case CH_WIDTH_40MHZ:
		if (req->chan < req->ch_center_freq_seg0)
			dfs_ic->ic_curchan->ic_flags |=
					(req->vht_capable ?
					IEEE80211_CHAN_VHT40PLUS :
					IEEE80211_CHAN_HT40PLUS);
		else
			dfs_ic->ic_curchan->ic_flags |=
					(req->vht_capable ?
					IEEE80211_CHAN_VHT40MINUS :
					IEEE80211_CHAN_HT40MINUS);
		break;
	case CH_WIDTH_80MHZ:
		dfs_ic->ic_curchan->ic_flags |= IEEE80211_CHAN_VHT80;
		break;
	case CH_WIDTH_80P80MHZ:
		ext_channel = cds_freq_to_chan(band_center_freq2);
		dfs_ic->ic_curchan->ic_flags |=
					IEEE80211_CHAN_VHT80P80;
		dfs_ic->ic_curchan->ic_freq_ext =
						band_center_freq2;
		dfs_ic->ic_curchan->ic_ieee_ext = ext_channel;

		/* verify both the 80MHz are DFS bands or not */
		if ((CHANNEL_STATE_DFS ==
		     cds_get_5g_bonded_channel_state(req->chan ,
						     CH_WIDTH_80MHZ)) &&
		    (CHANNEL_STATE_DFS == cds_get_5g_bonded_channel_state(
			    ext_channel - WMA_80MHZ_START_CENTER_CH_DIFF,
			    CH_WIDTH_80MHZ)))
			dfs_ic->ic_curchan->ic_80p80_both_dfs = true;
		break;
	case CH_WIDTH_160MHZ:
		dfs_ic->ic_curchan->ic_flags |=
					IEEE80211_CHAN_VHT160;
		break;
	default:
		WMA_LOGE(
		    "%s: Recieved a wrong channel width %d",
		    __func__, req->chan_width);
		break;
	}

	dfs_ic->ic_curchan->ic_flagext |= IEEE80211_CHAN_DFS;

	if (req->oper_mode == BSS_OPERATIONAL_MODE_AP) {
		dfs_ic->ic_opmode = IEEE80211_M_HOSTAP;
		dfs_ic->vdev_id = req->vdev_id;
	}

	dfs_ic->dfs_pri_multiplier = req->dfs_pri_multiplier;

	/*
	 * Configuring the DFS with current channel and the radar filters
	 */
	wma_dfs_configure(dfs_ic);
	WMA_LOGI("%s: DFS- CHANNEL CONFIGURED", __func__);
	return dfs_ic->ic_curchan;
}


/**
 * wma_set_dfs_region() - set DFS region
 * @wma: wma handle
 *
 * Configure the DFS region for DFS radar filter initialization
 *
 * Return: none
 */
void wma_set_dfs_region(tp_wma_handle wma, enum dfs_region dfs_region)
{
	if (dfs_region >= DFS_UNDEF_REGION ||
	    dfs_region == DFS_UNINIT_REGION)

		/* assign DFS_FCC_REGION as default region*/
		wma->dfs_ic->current_dfs_regdomain = DFS_FCC_REGION;
	else
		wma->dfs_ic->current_dfs_regdomain = dfs_region;

	WMA_LOGI("%s: DFS Region Domain: %d", __func__,
			 wma->dfs_ic->current_dfs_regdomain);
}

/**
 * wma_get_channels() - prepare dfs radar channel list
 * @ichan: channel
 * @chan_list: return channel list
 *
 * Return: return number of channels
 */
int wma_get_channels(struct dfs_ieee80211_channel *ichan,
		     struct wma_dfs_radar_channel_list *chan_list)
{
	uint8_t center_chan = cds_freq_to_chan(ichan->ic_vhtop_ch_freq_seg1);
	int count = 0;
	int start_channel = 0;
	int loop;

	chan_list->nchannels = 0;

	if (IEEE80211_IS_CHAN_11AC_VHT160(ichan)) {

		/*
		 * as per the latest draft for BSS bandwidth 160MHz,
		 * channel frequency segment 2 represents the center
		 * channel frequency.
		 */
		if (ichan->ic_vhtop_ch_freq_seg2)
			center_chan =
				cds_freq_to_chan(ichan->ic_vhtop_ch_freq_seg2);
		/*
		 * In 160MHz channel width, need to
		 * check if each of the 8 20MHz channel
		 * is DFS before adding to the NOL list.
		 * As it is possible that part of the
		 * 160MHz can be Non-DFS channels.
		 */
		start_channel = center_chan - WMA_160MHZ_START_CENTER_CH_DIFF;
		for (loop = 0; loop < WMA_DFS_MAX_20M_SUB_CH; loop++) {
			if (cds_get_channel_state(start_channel +
				    (loop * WMA_NEXT_20MHZ_START_CH_DIFF)) ==
							CHANNEL_STATE_DFS) {
				chan_list->channels[count] = start_channel +
					(loop * WMA_NEXT_20MHZ_START_CH_DIFF);
				count++;
			}
		}
		chan_list->nchannels = count;
	} else if (IEEE80211_IS_CHAN_11AC_VHT80P80(ichan)) {
		chan_list->nchannels = 4;
		/*
		 * If SAP is operating in 80p80 mode, either
		 * one of the two 80 segments or both the 80
		 * segments can be DFS channels, so need to
		 * identify on which 80 segment radar has
		 * been detected and only add those channels
		 * to the NOL list. center frequency should be
		 * based on the segment id passed as part of
		 * channel information in radar indication.
		 */
		if (ichan->ic_radar_found_segid == DFS_80P80_SEG1)
			center_chan =
				cds_freq_to_chan(ichan->ic_vhtop_ch_freq_seg2);
		chan_list->channels[0] = center_chan - 6;
		chan_list->channels[1] = center_chan - 2;
		chan_list->channels[2] = center_chan + 2;
		chan_list->channels[3] = center_chan + 6;
	} else if (IEEE80211_IS_CHAN_11AC_VHT80(ichan)) {
		chan_list->nchannels = 4;
		chan_list->channels[0] = center_chan - 6;
		chan_list->channels[1] = center_chan - 2;
		chan_list->channels[2] = center_chan + 2;
		chan_list->channels[3] = center_chan + 6;
	} else if (IEEE80211_IS_CHAN_11N_HT40(ichan) ||
		   IEEE80211_IS_CHAN_11AC_VHT40(ichan)) {
		chan_list->nchannels = 2;
		chan_list->channels[0] = center_chan - 2;
		chan_list->channels[1] = center_chan + 2;
	} else {
		chan_list->nchannels = 1;
		chan_list->channels[0] = center_chan;
	}

	return chan_list->nchannels;
}


/**
 * wma_dfs_indicate_radar() - Indicate Radar to SAP/HDD
 * @ic: ieee80211com ptr
 * @ichan: ieee 80211 channel
 *
 * Return: 0 for success or error code
 */
int wma_dfs_indicate_radar(struct ieee80211com *ic,
			   struct dfs_ieee80211_channel *ichan)
{
	tp_wma_handle wma;
	void *hdd_ctx;
	struct wma_dfs_radar_indication *radar_event;
	struct wma_dfs_radar_ind wma_radar_event;
	tpAniSirGlobal pmac = NULL;
	bool indication_status;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (wma == NULL) {
		WMA_LOGE("%s: DFS- Invalid wma", __func__);
		return -ENOENT;
	}

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	pmac = (tpAniSirGlobal)
		cds_get_context(QDF_MODULE_ID_PE);

	if (!pmac) {
		WMA_LOGE("%s: Invalid MAC handle", __func__);
		return -ENOENT;
	}

	if (wma->dfs_ic != ic) {
		WMA_LOGE("%s:DFS- Invalid WMA handle", __func__);
		return -ENOENT;
	}
	radar_event = (struct wma_dfs_radar_indication *)
		      qdf_mem_malloc(sizeof(struct wma_dfs_radar_indication));
	if (radar_event == NULL) {
		WMA_LOGE("%s:DFS- Invalid radar_event", __func__);
		return -ENOMEM;
	}

	/*
	 * Do not post multiple Radar events on the same channel.
	 * But, when DFS test mode is enabled, allow multiple dfs
	 * radar events to be posted on the same channel.
	 */
	qdf_spin_lock_bh(&ic->chan_lock);
	if (!pmac->sap.SapDfsInfo.disable_dfs_ch_switch)
		wma->dfs_ic->disable_phy_err_processing = true;

	if ((ichan->ic_ieee != (wma->dfs_ic->last_radar_found_chan)) ||
	    (pmac->sap.SapDfsInfo.disable_dfs_ch_switch == true)) {
		wma->dfs_ic->last_radar_found_chan = ichan->ic_ieee;
		/* Indicate the radar event to HDD to stop the netif Tx queues */
		wma_radar_event.chan_freq = ichan->ic_freq;
		wma_radar_event.dfs_radar_status = WMA_DFS_RADAR_FOUND;
		indication_status =
			wma->dfs_radar_indication_cb(hdd_ctx, &wma_radar_event);
		if (indication_status == false) {
			WMA_LOGE("%s:Application triggered channel switch in progress!.. drop radar event indiaction to SAP",
				__func__);
			qdf_mem_free(radar_event);
			qdf_spin_unlock_bh(&ic->chan_lock);
			return 0;
		}

		WMA_LOGE("%s:DFS- RADAR INDICATED TO HDD", __func__);

		wma_radar_event.ieee_chan_number = ichan->ic_ieee;
		/*
		 * Indicate to the radar event to SAP to
		 * select a new channel and set CSA IE
		 */
		radar_event->vdev_id = ic->vdev_id;
		wma_get_channels(ichan, &radar_event->chan_list);
		radar_event->dfs_radar_status = WMA_DFS_RADAR_FOUND;
		radar_event->use_nol = ic->ic_dfs_usenol(ic);
		wma_send_msg(wma, WMA_DFS_RADAR_IND, (void *)radar_event, 0);
		WMA_LOGE("%s:DFS- WMA_DFS_RADAR_IND Message Posted", __func__);
	}
	qdf_spin_unlock_bh(&ic->chan_lock);

	return 0;
}

#ifdef WLAN_FEATURE_MEMDUMP
/*
 * wma_process_fw_mem_dump_req() - Function to request fw memory dump from
 *                                 firmware
 * @wma:                Pointer to WMA handle
 * @mem_dump_req:       Pointer for mem_dump_req
 *
 * This function sends memory dump request to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
QDF_STATUS wma_process_fw_mem_dump_req(tp_wma_handle wma,
					struct fw_dump_req *mem_dump_req)
{
	int ret;

	if (!mem_dump_req || !wma) {
		WMA_LOGE(FL("input pointer is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_process_fw_mem_dump_cmd(wma->wmi_handle,
			 (struct fw_dump_req_param *) mem_dump_req);
	if (ret)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_fw_mem_dump_rsp() - send fw mem dump response to SME
 *
 * @req_id - request id.
 * @status - copy status from the firmware.
 *
 * This function is called by the memory dump response handler to
 * indicate SME that firmware dump copy is complete
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wma_fw_mem_dump_rsp(uint32_t req_id, uint32_t status)
{
	struct fw_dump_rsp *dump_rsp;
	cds_msg_t sme_msg = {0};
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	dump_rsp = qdf_mem_malloc(sizeof(*dump_rsp));

	if (!dump_rsp) {
		WMA_LOGE(FL("Memory allocation failed."));
		qdf_status = QDF_STATUS_E_NOMEM;
		return qdf_status;
	}

	WMA_LOGI(FL("FW memory dump copy complete status: %d for request: %d"),
		 status, req_id);

	dump_rsp->request_id = req_id;
	dump_rsp->dump_complete = status;

	sme_msg.type = eWNI_SME_FW_DUMP_IND;
	sme_msg.bodyptr = dump_rsp;
	sme_msg.bodyval = 0;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE(FL("Fail to post fw mem dump ind msg"));
		qdf_mem_free(dump_rsp);
	}

	return qdf_status;
}

/**
 * wma_fw_mem_dump_event_handler() - handles fw memory dump event
 *
 * @handle: pointer to wma handle.
 * @cmd_param_info: pointer to TLV info received in the event.
 * @len: length of data in @cmd_param_info
 *
 * This function is a handler for firmware memory dump event.
 *
 * Return: integer (0 for success and error code otherwise)
 */
int wma_fw_mem_dump_event_handler(void *handle, u_int8_t *cmd_param_info,
					 u_int32_t len)
{
	WMI_UPDATE_FW_MEM_DUMP_EVENTID_param_tlvs *param_buf;
	wmi_update_fw_mem_dump_fixed_param *event;
	QDF_STATUS status;

	param_buf =
	    (WMI_UPDATE_FW_MEM_DUMP_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}

	event = param_buf->fixed_param;

	status = wma_fw_mem_dump_rsp(event->request_id,
					 event->fw_mem_dump_complete);
	if (QDF_STATUS_SUCCESS != status) {
		WMA_LOGE("Error posting FW MEM DUMP RSP.");
		return -EINVAL;
	}

	WMA_LOGI("FW MEM DUMP RSP posted successfully");
	return 0;
}
#endif /* WLAN_FEATURE_MEMDUMP */

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

	cmd.vdev_id = ie_info->vdev_id;
	cmd.ie_id = ie_info->ie_id;
	cmd.length = ie_info->length;
	cmd.data = ie_info->data;

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
	if (!pmac->sme.pbpf_get_offload_cb) {
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
	pmac->sme.pbpf_get_offload_cb(pmac->hHdd, bpf_get_offload);
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

	if (!WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
		WMI_SERVICE_BPF_OFFLOAD)) {
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

	if (!WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
		WMI_SERVICE_BPF_OFFLOAD)) {
		WMA_LOGE(FL("BPF offload feature Disabled"));
		return QDF_STATUS_E_NOSUPPORT;
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
		qdf_mem_free(bpf_set_offload->program);
	}

	if (wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, len,
		WMI_BPF_SET_VDEV_INSTRUCTIONS_CMDID)) {
		WMA_LOGE(FL("Failed to send config bpf instructions command"));
		wmi_buf_free(wmi_buf);
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
 * wma_get_wakelock_stats() - Collects wake lock stats
 * @wake_lock_stats: wakelock structure to be filled
 *
 * This function collects wake lock stats
 *
 * Return: VOS_STATUS_SUCCESS on success, error number otherwise
 */
QDF_STATUS wma_get_wakelock_stats(struct sir_wake_lock_stats *wake_lock_stats)
{
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wake_lock_stats) {
		WMA_LOGE("%s: invalid pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_handle) {
		WMA_LOGE("%s: WMA context is invalid!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	wake_lock_stats->wow_ucast_wake_up_count =
			wma_handle->wow_ucast_wake_up_count;
	wake_lock_stats->wow_bcast_wake_up_count =
			wma_handle->wow_bcast_wake_up_count;
	wake_lock_stats->wow_ipv4_mcast_wake_up_count =
			wma_handle->wow_ipv4_mcast_wake_up_count;
	wake_lock_stats->wow_ipv6_mcast_wake_up_count =
			wma_handle->wow_ipv6_mcast_wake_up_count;
	wake_lock_stats->wow_ipv6_mcast_ra_stats =
			wma_handle->wow_ipv6_mcast_ra_stats;
	wake_lock_stats->wow_ipv6_mcast_ns_stats =
			wma_handle->wow_ipv6_mcast_ns_stats;
	wake_lock_stats->wow_ipv6_mcast_na_stats =
			wma_handle->wow_ipv6_mcast_na_stats;
	wake_lock_stats->wow_icmpv4_count = wma_handle->wow_icmpv4_count;
	wake_lock_stats->wow_icmpv6_uc_bc_count =
			wma_handle->wow_icmpv6_uc_bc_count;

	return QDF_STATUS_SUCCESS;
}
