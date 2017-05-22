/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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

/*
 * This file lim ProcessMessageQueue.cc contains the code
 * for processing LIM message Queue.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "wni_api.h"
#include "wma_types.h"

#include "wni_cfg.h"
#include "cfg_api.h"
#include "sir_common.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"

#include "lim_admit_control.h"
#include "lim_ibss_peer_mgmt.h"
#include "sch_api.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "lim_send_messages.h"

#include "rrm_api.h"

#include "lim_ft.h"

#include "qdf_types.h"
#include "cds_packet.h"
#include "qdf_mem.h"
#include "wlan_policy_mgr_api.h"
#include "nan_datapath.h"
#include "wlan_reg_services_api.h"

void lim_log_session_states(tpAniSirGlobal pMac);
static void lim_process_normal_hdd_msg(tpAniSirGlobal mac_ctx,
	struct scheduler_msg *msg, uint8_t rsp_reqd);

/**
 * lim_process_dual_mac_cfg_resp() - Process set dual mac config response
 * @mac: Global MAC pointer
 * @body: Set dual mac config response in sir_dual_mac_config_resp format
 *
 * Process the set dual mac config response and post the message
 * to SME to process this further and release the active
 * command list
 *
 * Return: None
 */
static void lim_process_dual_mac_cfg_resp(tpAniSirGlobal mac, void *body)
{
	struct sir_dual_mac_config_resp *resp, *param;
	uint32_t len, fail_resp = 0;
	struct scheduler_msg msg = {0};

	resp = (struct sir_dual_mac_config_resp *)body;
	if (!resp) {
		pe_err("Set dual mac cfg param is NULL");
		fail_resp = 1;
		/* Not returning here. If possible, let us proceed
		 * and send fail response to SME
		 */
	}

	len = sizeof(*param);

	param = qdf_mem_malloc(len);
	if (!param) {
		pe_err("Fail to allocate memory");
		/* Memory allocation for param failed.
		 * Cannot send fail status back to SME
		 */
		return;
	}

	if (fail_resp) {
		pe_err("Send fail status to SME");
		param->status = SET_HW_MODE_STATUS_ECANCELED;
	} else {
		param->status = resp->status;
		/*
		 * TODO: Update this HW mode info in any UMAC params, if needed
		 */
	}

	msg.type = eWNI_SME_SET_DUAL_MAC_CFG_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	pe_debug("Send eWNI_SME_SET_DUAL_MAC_CFG_RESP to SME");
	lim_sys_process_mmh_msg_api(mac, &msg, ePROT);
	return;
}

/**
 * lim_process_set_hw_mode_resp() - Process set HW mode response
 * @mac: Global MAC pointer
 * @body: Set HW mode response in sir_set_hw_mode_resp format
 *
 * Process the set HW mode response and post the message
 * to SME to process this further and release the active
 * command list
 *
 * Return: None
 */
static void lim_process_set_hw_mode_resp(tpAniSirGlobal mac, void *body)
{
	struct sir_set_hw_mode_resp *resp, *param;
	uint32_t len, i, fail_resp = 0;
	struct scheduler_msg msg = {0};

	resp = (struct sir_set_hw_mode_resp *)body;
	if (!resp) {
		pe_err("Set HW mode param is NULL");
		fail_resp = 1;
		/* Not returning here. If possible, let us proceed
		 * and send fail response to SME */
	}

	len = sizeof(*param);

	param = qdf_mem_malloc(len);
	if (!param) {
		pe_err("Fail to allocate memory");
		/* Memory allocation for param failed.
		 * Cannot send fail status back to SME
		 */
		return;
	}

	if (fail_resp) {
		pe_err("Send fail status to SME");
		param->status = SET_HW_MODE_STATUS_ECANCELED;
		param->cfgd_hw_mode_index = 0;
		param->num_vdev_mac_entries = 0;
	} else {
		param->status = resp->status;
		param->cfgd_hw_mode_index = resp->cfgd_hw_mode_index;
		param->num_vdev_mac_entries = resp->num_vdev_mac_entries;

		for (i = 0; i < resp->num_vdev_mac_entries; i++) {
			param->vdev_mac_map[i].vdev_id =
				resp->vdev_mac_map[i].vdev_id;
			param->vdev_mac_map[i].mac_id =
				resp->vdev_mac_map[i].mac_id;
		}
		/*
		 * TODO: Update this HW mode info in any UMAC params, if needed
		 */
	}

	msg.type = eWNI_SME_SET_HW_MODE_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	pe_err("Send eWNI_SME_SET_HW_MODE_RESP to SME");
	lim_sys_process_mmh_msg_api(mac, &msg, ePROT);
	return;
}

/**
 * lim_process_antenna_mode_resp() - Process set antenna mode
 * response
 * @mac: Global MAC pointer
 * @body: Set antenna mode response in sir_antenna_mode_resp
 * format
 *
 * Process the set antenna mode response and post the message
 * to SME to process this further and release the active
 * command list
 *
 * Return: None
 */
static void lim_process_set_antenna_resp(tpAniSirGlobal mac, void *body)
{
	struct sir_antenna_mode_resp *resp, *param;
	bool fail_resp = false;
	struct scheduler_msg msg = {0};

	resp = (struct sir_antenna_mode_resp *)body;
	if (!resp) {
		pe_err("Set antenna mode resp is NULL");
		fail_resp = true;
		/* Not returning here. If possible, let us proceed
		 * and send fail response to SME
		 */
	}

	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		pe_err("Fail to allocate memory");
		/* Memory allocation for param failed.
		 * Cannot send fail status back to SME
		 */
		return;
	}

	if (fail_resp) {
		pe_err("Send fail status to SME");
		param->status = SET_ANTENNA_MODE_STATUS_ECANCELED;
	} else {
		param->status = resp->status;
	}

	msg.type = eWNI_SME_SET_ANTENNA_MODE_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	pe_debug("Send eWNI_SME_SET_ANTENNA_MODE_RESP to SME");
	lim_sys_process_mmh_msg_api(mac, &msg, ePROT);
	return;
}

/**
 * lim_process_set_default_scan_ie_request() - Process the Set default
 * Scan IE request from HDD.
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: Pointer to incoming data
 *
 * This function receives the default scan IEs and updates the ext cap IE
 * (if present) with FTM capabilities and pass the Scan IEs to WMA.
 *
 * Return: None
 */
static void lim_process_set_default_scan_ie_request(tpAniSirGlobal mac_ctx,
							uint32_t *msg_buf)
{
	struct hdd_default_scan_ie *set_ie_params;
	struct vdev_ie_info *wma_ie_params;
	uint8_t *local_ie_buf;
	uint16_t local_ie_len;
	struct scheduler_msg msg_q = {0};
	tSirRetStatus ret_code;
	QDF_STATUS qdf_status;

	if (!msg_buf) {
		pe_err("msg_buf is NULL");
		return;
	}

	set_ie_params = (struct hdd_default_scan_ie *) msg_buf;
	local_ie_len = set_ie_params->ie_len;

	local_ie_buf = qdf_mem_malloc(MAX_DEFAULT_SCAN_IE_LEN);
	if (!local_ie_buf) {
		pe_err("Scan IE Update fails due to malloc failure");
		return;
	}

	if (lim_update_ext_cap_ie(mac_ctx,
			(uint8_t *)set_ie_params->ie_data,
			local_ie_buf, &local_ie_len)) {
		pe_err("Update ext cap IEs fails");
		goto scan_ie_send_fail;
	}

	if (mac_ctx->roam.configParam.qcn_ie_support) {
		qdf_status = lim_add_qcn_ie(mac_ctx, local_ie_buf,
							&local_ie_len);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			goto scan_ie_send_fail;
	}

	wma_ie_params = qdf_mem_malloc(sizeof(*wma_ie_params) + local_ie_len);
	if (!wma_ie_params) {
		pe_err("fail to alloc wma_ie_params");
		goto scan_ie_send_fail;
	}
	wma_ie_params->vdev_id = set_ie_params->session_id;
	wma_ie_params->ie_id = DEFAULT_SCAN_IE_ID;
	wma_ie_params->length = local_ie_len;
	wma_ie_params->data = (uint8_t *)(wma_ie_params)
					+ sizeof(*wma_ie_params);
	qdf_mem_copy(wma_ie_params->data, local_ie_buf, local_ie_len);

	msg_q.type = WMA_SET_IE_INFO;
	msg_q.bodyptr = wma_ie_params;
	msg_q.bodyval = 0;
	ret_code = wma_post_ctrl_msg(mac_ctx, &msg_q);
	if (eSIR_SUCCESS != ret_code) {
		pe_err("fail to send WMA_SET_IE_INFO");
		qdf_mem_free(wma_ie_params);
	}
scan_ie_send_fail:
	qdf_mem_free(local_ie_buf);
}

/**
 * lim_process_hw_mode_trans_ind() - Process set HW mode transition indication
 * @mac: Global MAC pointer
 * @body: Set HW mode response in sir_hw_mode_trans_ind format
 *
 * Process the set HW mode transition indication and post the message
 * to SME to invoke the HDD callback
 * command list
 *
 * Return: None
 */
static void lim_process_hw_mode_trans_ind(tpAniSirGlobal mac, void *body)
{
	struct sir_hw_mode_trans_ind *ind, *param;
	uint32_t len, i;
	struct scheduler_msg msg = {0};

	ind = (struct sir_hw_mode_trans_ind *)body;
	if (!ind) {
		pe_err("Set HW mode trans ind param is NULL");
		return;
	}

	len = sizeof(*param);

	param = qdf_mem_malloc(len);
	if (!param) {
		pe_err("Fail to allocate memory");
		return;
	}

	param->old_hw_mode_index = ind->old_hw_mode_index;
	param->new_hw_mode_index = ind->new_hw_mode_index;
	param->num_vdev_mac_entries = ind->num_vdev_mac_entries;

	for (i = 0; i < ind->num_vdev_mac_entries; i++) {
		param->vdev_mac_map[i].vdev_id =
			ind->vdev_mac_map[i].vdev_id;
		param->vdev_mac_map[i].mac_id =
			ind->vdev_mac_map[i].mac_id;
	}

	/* TODO: Update this HW mode info in any UMAC params, if needed */

	msg.type = eWNI_SME_HW_MODE_TRANS_IND;
	msg.bodyptr = param;
	msg.bodyval = 0;
	pe_err("Send eWNI_SME_HW_MODE_TRANS_IND to SME");
	lim_sys_process_mmh_msg_api(mac, &msg, ePROT);
	return;
}

/** -------------------------------------------------------------
   \fn def_msg_decision
   \brief The function decides whether to defer a message or not in limProcessMessage function
   \param   tpAniSirGlobal pMac
   \param       struct scheduler_msg  limMsg
   \param       tSirMacTspecIE   *ppInfo
   \return none
   -------------------------------------------------------------*/

uint8_t static def_msg_decision(tpAniSirGlobal pMac, struct scheduler_msg *limMsg)
{

/* this function should not changed */
	if (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) {
		/* Defer processsing this message */
		if (lim_defer_msg(pMac, limMsg) != TX_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_PE, LOGE,
					FL("Unable to Defer Msg"));
			lim_log_session_states(pMac);
			lim_handle_defer_msg_error(pMac, limMsg);
		}
		return true;
	}
	/* When defer is requested then defer all the messages except HAL responses. */
	if ((!lim_is_system_in_scan_state(pMac))
	    && (true != GET_LIM_PROCESS_DEFD_MESGS(pMac))
	    && !pMac->lim.gLimSystemInScanLearnMode) {
		if ((limMsg->type != WMA_ADD_BSS_RSP)
		    && (limMsg->type != WMA_DELETE_BSS_RSP)
		    && (limMsg->type != WMA_DELETE_BSS_HO_FAIL_RSP)
		    && (limMsg->type != WMA_ADD_STA_RSP)
		    && (limMsg->type != WMA_DELETE_STA_RSP)
		    && (limMsg->type != WMA_SET_BSSKEY_RSP)
		    && (limMsg->type != WMA_SET_STAKEY_RSP)
		    && (limMsg->type != WMA_SET_STA_BCASTKEY_RSP)
		    && (limMsg->type != WMA_AGGR_QOS_RSP)
		    && (limMsg->type != WMA_SET_MIMOPS_RSP)
		    && (limMsg->type != WMA_SWITCH_CHANNEL_RSP)
		    && (limMsg->type != WMA_P2P_NOA_ATTR_IND)
		    && (limMsg->type != WMA_P2P_NOA_START_IND) &&
		    (limMsg->type != WMA_ADD_TS_RSP) &&
		    /*
		     * LIM won't process any defer queue commands if gLimAddtsSent is
		     * set to TRUE. gLimAddtsSent will be set TRUE to while sending
		     * ADDTS REQ. Say, when deferring is enabled, if
		     * SIR_LIM_ADDTS_RSP_TIMEOUT is posted (because of not receiving ADDTS
		     * RSP) then this command will be added to defer queue and as
		     * gLimAddtsSent is set TRUE LIM will never process any commands from
		     * defer queue, including SIR_LIM_ADDTS_RSP_TIMEOUT. Hence allowing
		     * SIR_LIM_ADDTS_RSP_TIMEOUT command to be processed with deferring
		     * enabled, so that this will be processed immediately and sets
		     * gLimAddtsSent to FALSE.
		     */
		    (limMsg->type != SIR_LIM_ADDTS_RSP_TIMEOUT) &&
		    /* Allow processing of RX frames while awaiting reception
		     * of ADD TS response over the air. This logic particularly
		     * handles the case when host sends ADD BA request to FW
		     * after ADD TS request is sent over the air and
		     * ADD TS response received over the air */
		    !(limMsg->type == SIR_BB_XPORT_MGMT_MSG &&
			pMac->lim.gLimAddtsSent)) {
			pe_debug("Defer the current message %s , gLimProcessDefdMsgs is false and system is not in scan/learn mode",
				       lim_msg_str(limMsg->type));
			/* Defer processsing this message */
			if (lim_defer_msg(pMac, limMsg) != TX_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_PE, LOGE,
					FL("Unable to Defer Msg"));
				lim_log_session_states(pMac);
				lim_handle_defer_msg_error(pMac, limMsg);

			}
			return true;
		}
	}
	return false;
}

#ifdef FEATURE_WLAN_EXTSCAN
static void
__lim_pno_match_fwd_bcn_probepsp(tpAniSirGlobal pmac, uint8_t *rx_pkt_info,
				tSirProbeRespBeacon *frame, uint32_t ie_len,
				uint32_t msg_type)
{
	struct pno_match_found  *result;
	uint8_t                 *body;
	struct scheduler_msg    mmh_msg = {0};
	tpSirMacMgmtHdr         hdr;
	uint32_t num_results = 1, len, i;

	/* Upon receiving every matched beacon, bss info is forwarded to the
	 * the upper layer, hence num_results is set to 1 */
	len = sizeof(*result) + (num_results * sizeof(tSirWifiScanResult)) +
		ie_len;

	result = qdf_mem_malloc(len);
	if (NULL == result) {
		pe_err("Memory allocation failed");
		return;
	}
	hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);

	/* Received frame does not have request id, hence set 0 */
	result->request_id = 0;
	result->more_data = 0;
	result->num_results = num_results;

	for (i = 0; i < result->num_results; i++) {
		result->ap[i].ts = qdf_mc_timer_get_system_time();
		result->ap[i].beaconPeriod = frame->beaconInterval;
		result->ap[i].capability =
			lim_get_u16((uint8_t *) &frame->capabilityInfo);
		result->ap[i].channel = WMA_GET_RX_CH(rx_pkt_info);
		result->ap[i].rssi = WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
		result->ap[i].rtt = 0;
		result->ap[i].rtt_sd = 0;
		result->ap[i].ieLength = ie_len;
		qdf_mem_copy((uint8_t *) &result->ap[i].ssid[0],
			(uint8_t *) frame->ssId.ssId, frame->ssId.length);
		result->ap[i].ssid[frame->ssId.length] = '\0';
		qdf_mem_copy((uint8_t *) &result->ap[i].bssid,
				(uint8_t *) hdr->bssId,
				sizeof(tSirMacAddr));
		/* Copy IE fields */
		qdf_mem_copy((uint8_t *) &result->ap[i].ieData,
				body + SIR_MAC_B_PR_SSID_OFFSET, ie_len);
	}

	mmh_msg.type = msg_type;
	mmh_msg.bodyptr = result;
	mmh_msg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pmac, &mmh_msg, ePROT);
}


static void
__lim_ext_scan_forward_bcn_probe_rsp(tpAniSirGlobal pmac, uint8_t *rx_pkt_info,
					tSirProbeRespBeacon *frame,
					uint32_t ie_len,
					uint32_t msg_type)
{
	tpSirWifiFullScanResultEvent result;
	uint8_t                     *body;
	struct scheduler_msg         mmh_msg = {0};
	tpSirMacMgmtHdr              hdr;
	uint32_t frame_len;
	tSirBssDescription *bssdescr;

	result = qdf_mem_malloc(sizeof(*result) + ie_len);
	if (NULL == result) {
		pe_err("Memory allocation failed");
		return;
	}
	hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);

	/* Received frame does not have request id, hence set 0 */
	result->requestId = 0;

	result->moreData = 0;
	result->ap.ts = qdf_mc_timer_get_system_time();
	result->ap.beaconPeriod = frame->beaconInterval;
	result->ap.capability =
			lim_get_u16((uint8_t *) &frame->capabilityInfo);
	result->ap.channel = WMA_GET_RX_CH(rx_pkt_info);
	result->ap.rssi = WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
	result->ap.rtt = 0;
	result->ap.rtt_sd = 0;
	result->ap.ieLength = ie_len;

	qdf_mem_copy((uint8_t *) &result->ap.ssid[0],
			(uint8_t *) frame->ssId.ssId, frame->ssId.length);
	result->ap.ssid[frame->ssId.length] = '\0';
	qdf_mem_copy((uint8_t *) &result->ap.bssid.bytes,
			(uint8_t *) hdr->bssId,
			QDF_MAC_ADDR_SIZE);
	/* Copy IE fields */
	qdf_mem_copy((uint8_t *) &result->ap.ieData,
			body + SIR_MAC_B_PR_SSID_OFFSET, ie_len);

	frame_len = sizeof(*bssdescr) + ie_len - sizeof(bssdescr->ieFields[1]);
	bssdescr = (tSirBssDescription *) qdf_mem_malloc(frame_len);

	if (NULL == bssdescr) {
		pe_err("qdf_mem_malloc(length=%d) failed", frame_len);
		return;
	}

	qdf_mem_zero(bssdescr, frame_len);

	lim_collect_bss_description(pmac, bssdescr, frame, rx_pkt_info, false);
	/*
	 * Send the beacon to CSR with registered callback routine.
	 * scan_id and flags parameters are currently unused and set to 0.
	 * EXT scan results will also be added to scan cache in SME
	 */
	if (pmac->lim.add_bssdescr_callback) {
		(pmac->lim.add_bssdescr_callback) (pmac, bssdescr, 0, 0);
	} else {
		pe_err("No CSR callback routine to send beacon/probe response");
	}
	qdf_mem_free(bssdescr);

	mmh_msg.type = msg_type;
	mmh_msg.bodyptr = result;
	mmh_msg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pmac, &mmh_msg, ePROT);
}

static void
__lim_process_ext_scan_beacon_probe_rsp(tpAniSirGlobal pmac,
					uint8_t *rx_pkt_info,
					uint8_t sub_type)
{
	tSirProbeRespBeacon  *frame;
	uint8_t              *body;
	uint32_t             frm_len;
	tSirRetStatus        status;

	frm_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
	if (frm_len <= SIR_MAC_B_PR_SSID_OFFSET) {
		pe_err("RX packet has invalid length %d", frm_len);
		return;
	}

	frame = qdf_mem_malloc(sizeof(*frame));
	if (NULL == frame) {
		pe_err("Memory allocation failed");
		return;
	}

	if (sub_type == SIR_MAC_MGMT_BEACON) {
		pe_debug("Beacon due to ExtScan/epno");
		status = sir_convert_beacon_frame2_struct(pmac,
						(uint8_t *)rx_pkt_info,
						frame);
	} else if (sub_type == SIR_MAC_MGMT_PROBE_RSP) {
		pe_debug("Probe Rsp due to ExtScan/epno");
		body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
		status = sir_convert_probe_frame2_struct(pmac, body,
							frm_len, frame);
	} else {
		qdf_mem_free(frame);
		return;
	}

	if (status != eSIR_SUCCESS) {
		pe_err("Frame parsing failed");
		qdf_mem_free(frame);
		return;
	}

	if (WMA_IS_EXTSCAN_SCAN_SRC(rx_pkt_info))
		__lim_ext_scan_forward_bcn_probe_rsp(pmac, rx_pkt_info, frame,
					(frm_len - SIR_MAC_B_PR_SSID_OFFSET),
					eWNI_SME_EXTSCAN_FULL_SCAN_RESULT_IND);

	if (WMA_IS_EPNO_SCAN_SRC(rx_pkt_info))
		__lim_pno_match_fwd_bcn_probepsp(pmac, rx_pkt_info, frame,
					(frm_len - SIR_MAC_B_PR_SSID_OFFSET),
					eWNI_SME_EPNO_NETWORK_FOUND_IND);

	qdf_mem_free(frame);
}
#endif

/*
 * Beacon Handling Cases:
 * during scanning, when no session is active:
 *    handled by lim_handle_frames_in_scan_state before __lim_handle_beacon call is invoked.
 * during scanning, when any session is active, but beacon/Pr does not belong to that session, psessionEntry will be null.
 *    handled by lim_handle_frames_in_scan_state before __lim_handle_beacon call is invoked.
 * during scanning, when any session is active, and beacon/Pr belongs to one of the session, psessionEntry will not be null.
 *    handled by lim_handle_frames_in_scan_state before __lim_handle_beacon call is invoked.
 * Not scanning, no session:
 *    there should not be any beacon coming, if coming, should be dropped.
 * Not Scanning,
 */
static void
__lim_handle_beacon(tpAniSirGlobal pMac, struct scheduler_msg *pMsg,
		    tpPESession psessionEntry)
{
	/* checking for global SME state... */
	uint8_t *pRxPacketInfo;
	lim_get_b_dfrom_rx_packet(pMac, pMsg->bodyptr,
				  (uint32_t **) &pRxPacketInfo);

	/* This function should not be called if beacon is received in scan state. */
	/* So not doing any checks for the global state. */

	if (psessionEntry == NULL) {
		sch_beacon_process(pMac, pRxPacketInfo, NULL);
	} else if ((psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE) ||
		   (psessionEntry->limSmeState == eLIM_SME_NORMAL_STATE)) {
		sch_beacon_process(pMac, pRxPacketInfo, psessionEntry);
	} else
		lim_process_beacon_frame(pMac, pRxPacketInfo, psessionEntry);

	return;
}

/**
 * lim_defer_msg()
 *
 ***FUNCTION:
 * This function is called to defer the messages received
 * during Learn mode
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pMsg of type struct scheduler_msg - Pointer to the message structure
 * @return None
 */

uint32_t lim_defer_msg(tpAniSirGlobal pMac, struct scheduler_msg *pMsg)
{
	uint32_t retCode = TX_SUCCESS;

	retCode = lim_write_deferred_msg_q(pMac, pMsg);

	if (retCode == TX_SUCCESS) {
		MTRACE(mac_trace_msg_rx
			       (pMac, NO_SESSION,
			       LIM_TRACE_MAKE_RXMSG(pMsg->type, LIM_MSG_DEFERRED));
		       )
	} else {
		pe_err("Dropped lim message (0x%X)", pMsg->type);
		MTRACE(mac_trace_msg_rx
			       (pMac, NO_SESSION,
			       LIM_TRACE_MAKE_RXMSG(pMsg->type, LIM_MSG_DROPPED));
		       )
	}

	return retCode;
} /*** end lim_defer_msg() ***/

/**
 * lim_handle_unknown_a2_index_frames() - This function handles Unknown Unicast
 *                                        (A2 Index) packets
 * @mac_ctx:          Pointer to the Global Mac Context.
 * @rx_pkt_buffer:    Pointer to the packet Buffer descriptor.
 * @session_entry:    Pointer to the PE Session Entry.
 *
 * This routine will handle public action frames.
 *
 * Return:      None.
 */
static void lim_handle_unknown_a2_index_frames(tpAniSirGlobal mac_ctx,
	void *rx_pkt_buffer, tpPESession session_entry)
{
#ifdef FEATURE_WLAN_TDLS
	tpSirMacDataHdr3a mac_hdr;
#endif
	if (LIM_IS_P2P_DEVICE_ROLE(session_entry))
		lim_process_action_frame_no_session(mac_ctx,
			(uint8_t *) rx_pkt_buffer);
#ifdef FEATURE_WLAN_TDLS
	mac_hdr = WMA_GET_RX_MPDUHEADER3A(rx_pkt_buffer);

	if (lim_is_group_addr(mac_hdr->addr2)) {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Ignoring A2 Invalid Packet received for MC/BC:"));
		lim_print_mac_addr(mac_ctx, mac_hdr->addr2, LOG2);
		return;
	}
	QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			FL("type=0x%x, subtype=0x%x"),
		mac_hdr->fc.type, mac_hdr->fc.subType);
	/* Currently only following type and subtype are handled.
	 * If there are more combinations, then add switch-case
	 * statements.
	 */
	if (LIM_IS_STA_ROLE(session_entry) &&
		(mac_hdr->fc.type == SIR_MAC_MGMT_FRAME) &&
		(mac_hdr->fc.subType == SIR_MAC_MGMT_ACTION))
		lim_process_action_frame(mac_ctx, rx_pkt_buffer, session_entry);
#endif
	return;
}

/**
 * lim_check_mgmt_registered_frames() - This function handles registered
 *                                      management frames.
 *
 * @mac_ctx:          Pointer to the Global Mac Context.
 * @buff_desc:        Pointer to the packet Buffer descriptor.
 * @session_entry:    Pointer to the PE Session Entry.
 *
 * This function is called to process to check if received frame match with
 * any of the registered frame from HDD. If yes pass this frame to SME.
 *
 * Return:      True or False for Match or Mismatch respectively.
 */
static bool
lim_check_mgmt_registered_frames(tpAniSirGlobal mac_ctx, uint8_t *buff_desc,
				 tpPESession session_entry)
{
	tSirMacFrameCtl fc;
	tpSirMacMgmtHdr hdr;
	uint8_t *body;
	struct mgmt_frm_reg_info *mgmt_frame = NULL;
	struct mgmt_frm_reg_info *next_frm = NULL;
	uint16_t frm_type;
	uint16_t frm_len;
	uint8_t type, sub_type;
	bool match = false;
	QDF_STATUS qdf_status;

	hdr = WMA_GET_RX_MAC_HEADER(buff_desc);
	fc = hdr->fc;
	frm_type = (fc.type << 2) | (fc.subType << 4);
	body = WMA_GET_RX_MPDU_DATA(buff_desc);
	frm_len = WMA_GET_RX_PAYLOAD_LEN(buff_desc);

	qdf_mutex_acquire(&mac_ctx->lim.lim_frame_register_lock);
	qdf_list_peek_front(&mac_ctx->lim.gLimMgmtFrameRegistratinQueue,
			    (qdf_list_node_t **) &mgmt_frame);
	qdf_mutex_release(&mac_ctx->lim.lim_frame_register_lock);

	while (mgmt_frame != NULL) {
		type = (mgmt_frame->frameType >> 2) & 0x03;
		sub_type = (mgmt_frame->frameType >> 4) & 0x0f;
		if ((type == SIR_MAC_MGMT_FRAME)
		    && (fc.type == SIR_MAC_MGMT_FRAME)
		    && (sub_type == SIR_MAC_MGMT_RESERVED15)) {
			QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO_HIGH,
				FL
				("rcvd frm match for SIR_MAC_MGMT_RESERVED15"));
			match = true;
			break;
		}
		if (mgmt_frame->frameType == frm_type) {
			if (mgmt_frame->matchLen <= 0) {
				match = true;
				break;
			}
			if (mgmt_frame->matchLen <= frm_len &&
				(!qdf_mem_cmp(mgmt_frame->matchData, body,
				mgmt_frame->matchLen))) {
				/* found match! */
				match = true;
				break;
			}
		}

		qdf_mutex_acquire(&mac_ctx->lim.lim_frame_register_lock);
		qdf_status =
			qdf_list_peek_next(
			&mac_ctx->lim.gLimMgmtFrameRegistratinQueue,
			(qdf_list_node_t *) mgmt_frame,
			(qdf_list_node_t **) &next_frm);
		qdf_mutex_release(&mac_ctx->lim.lim_frame_register_lock);
		mgmt_frame = next_frm;
		next_frm = NULL;
	}

	if (match) {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			FL("rcvd frame match with registered frame params"));
		/* Indicate this to SME */
		lim_send_sme_mgmt_frame_ind(mac_ctx, hdr->fc.subType,
			(uint8_t *) hdr,
			WMA_GET_RX_PAYLOAD_LEN(buff_desc) +
			sizeof(tSirMacMgmtHdr), mgmt_frame->sessionId,
			WMA_GET_RX_CH(buff_desc), session_entry,
			WMA_GET_RX_RSSI_NORMALIZED(buff_desc));

		if ((type == SIR_MAC_MGMT_FRAME)
		    && (fc.type == SIR_MAC_MGMT_FRAME)
		    && (sub_type == SIR_MAC_MGMT_RESERVED15))
			/* These packets needs to be processed by PE/SME
			 * as well as HDD.If it returns true here,
			 * the packet is forwarded to HDD only.
			 */
			match = false;
	}

	return match;
}

/**
 * lim_handle80211_frames()
 *
 ***FUNCTION:
 * This function is called to process 802.11 frames
 * received by LIM.
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pMsg of type struct scheduler_msg - Pointer to the message structure
 * @return None
 */

static void
lim_handle80211_frames(tpAniSirGlobal pMac, struct scheduler_msg *limMsg,
		       uint8_t *pDeferMsg)
{
	uint8_t *pRxPacketInfo = NULL;
	tSirMacFrameCtl fc;
	tpSirMacMgmtHdr pHdr = NULL;
	tpPESession psessionEntry = NULL;
	uint8_t sessionId;
	tAniBool isFrmFt = false;

	*pDeferMsg = false;
	lim_get_b_dfrom_rx_packet(pMac, limMsg->bodyptr,
				  (uint32_t **) &pRxPacketInfo);

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	isFrmFt = WMA_GET_RX_FT_DONE(pRxPacketInfo);
	fc = pHdr->fc;

	if (pMac->sap.SapDfsInfo.is_dfs_cac_timer_running) {
		psessionEntry = pe_find_session_by_bssid(pMac,
					pHdr->bssId, &sessionId);
		if (psessionEntry &&
		    (QDF_SAP_MODE == psessionEntry->pePersona)) {
			pe_debug("CAC timer running - drop the frame");
			goto end;
		}
	}

#ifdef WLAN_DUMP_MGMTFRAMES
	pe_debug("ProtVersion %d, Type %d, Subtype %d rateIndex=%d",
		fc.protVer, fc.type, fc.subType,
		WMA_GET_RX_MAC_RATE_IDX(pRxPacketInfo));
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR, pHdr,
			   WMA_GET_RX_MPDU_HEADER_LEN(pRxPacketInfo));
#endif
	if (pMac->fEnableDebugLog & 0x1) {
		if ((fc.type == SIR_MAC_MGMT_FRAME) &&
		    (fc.subType != SIR_MAC_MGMT_PROBE_REQ) &&
		    (fc.subType != SIR_MAC_MGMT_PROBE_RSP) &&
		    (fc.subType != SIR_MAC_MGMT_BEACON)) {
			pe_debug("RX MGMT - Type %hu, SubType %hu, seq num[%d]",
				   fc.type,
				   fc.subType,
				   ((pHdr->seqControl.seqNumHi <<
				   HIGH_SEQ_NUM_OFFSET) |
				   pHdr->seqControl.seqNumLo));
		}
	}
#ifdef FEATURE_WLAN_EXTSCAN
	if (WMA_IS_EXTSCAN_SCAN_SRC(pRxPacketInfo) ||
		WMA_IS_EPNO_SCAN_SRC(pRxPacketInfo)) {
		if (fc.subType == SIR_MAC_MGMT_BEACON ||
		    fc.subType == SIR_MAC_MGMT_PROBE_RSP) {
			__lim_process_ext_scan_beacon_probe_rsp(pMac,
								pRxPacketInfo,
								fc.subType);
		} else {
			pe_err("Wrong frameType %d, Subtype %d for %d",
				fc.type, fc.subType,
				WMA_GET_SCAN_SRC(pRxPacketInfo));
		}
		goto end;
	}
#endif
	/* Added For BT-AMP Support */
	psessionEntry = pe_find_session_by_bssid(pMac, pHdr->bssId,
						 &sessionId);
	if (psessionEntry == NULL) {
		if (fc.subType == SIR_MAC_MGMT_AUTH) {
			pe_debug("ProtVersion %d, Type %d, Subtype %d rateIndex=%d",
				fc.protVer, fc.type, fc.subType,
				WMA_GET_RX_MAC_RATE_IDX(pRxPacketInfo));
			lim_print_mac_addr(pMac, pHdr->bssId, LOG1);
			if (lim_process_auth_frame_no_session
				    (pMac, pRxPacketInfo,
				    limMsg->bodyptr) == eSIR_SUCCESS) {
				goto end;
			}
		}
		/* Public action frame can be received from non-assoc stations*/
		if ((fc.subType != SIR_MAC_MGMT_PROBE_RSP) &&
		    (fc.subType != SIR_MAC_MGMT_BEACON) &&
		    (fc.subType != SIR_MAC_MGMT_PROBE_REQ)
		    && (fc.subType != SIR_MAC_MGMT_ACTION)) {

			psessionEntry = pe_find_session_by_peer_sta(pMac,
						pHdr->sa, &sessionId);
			if (psessionEntry == NULL) {
				pe_debug("SessionId does not exist for Bssid");
				lim_print_mac_addr(pMac, pHdr->sa, LOG3);
				goto end;
			} else {
				pe_debug("SessionId:%d exists for given Bssid",
					psessionEntry->peSessionId);
			}
		}
		/*  For p2p resp frames search for valid session with DA as */
		/*  BSSID will be SA and session will be present with DA only */
		if (fc.subType == SIR_MAC_MGMT_ACTION) {
			psessionEntry =
				pe_find_session_by_bssid(pMac, pHdr->da, &sessionId);
		}
	}

	/* Check if frame is registered by HDD */
	if (lim_check_mgmt_registered_frames(pMac, pRxPacketInfo, psessionEntry)) {
		pe_debug("Received frame is passed to SME");
		goto end;
	}

	if (fc.protVer != SIR_MAC_PROTOCOL_VERSION) {   /* Received Frame with non-zero Protocol Version */
		pe_err("Unexpected frame with protVersion %d received",
			fc.protVer);
		lim_pkt_free(pMac, TXRX_FRM_802_11_MGMT, pRxPacketInfo,
			     (void *)limMsg->bodyptr);
#ifdef WLAN_DEBUG
		pMac->lim.numProtErr++;
#endif
		goto end;
	}

/* Chance of crashing : to be done BT-AMP ........happens when broadcast probe req is received */

#ifdef WLAN_DEBUG
	pMac->lim.numMAC[fc.type][fc.subType]++;
#endif

	switch (fc.type) {
	case SIR_MAC_MGMT_FRAME:
	{
		/* Received Management frame */
		switch (fc.subType) {
		case SIR_MAC_MGMT_ASSOC_REQ:
			/* Make sure the role supports Association */
			if (LIM_IS_AP_ROLE(psessionEntry))
				lim_process_assoc_req_frame(pMac,
							    pRxPacketInfo,
							    LIM_ASSOC,
							    psessionEntry);
			else {
				pe_err("unexpected message received %X",
					limMsg->type);
				lim_print_msg_name(pMac, LOGE,
						   limMsg->type);
			}
			break;

		case SIR_MAC_MGMT_ASSOC_RSP:
			lim_process_assoc_rsp_frame(pMac, pRxPacketInfo,
						    LIM_ASSOC,
						    psessionEntry);
			break;

		case SIR_MAC_MGMT_REASSOC_REQ:
			/* Make sure the role supports Reassociation */
			if (LIM_IS_AP_ROLE(psessionEntry)) {
				lim_process_assoc_req_frame(pMac,
							    pRxPacketInfo,
							    LIM_REASSOC,
							    psessionEntry);
			} else {
				pe_err("unexpected message received %X",
					limMsg->type);
				lim_print_msg_name(pMac, LOGE,
					limMsg->type);
			}
			break;

		case SIR_MAC_MGMT_REASSOC_RSP:
			lim_process_assoc_rsp_frame(pMac, pRxPacketInfo,
						    LIM_REASSOC,
						    psessionEntry);
			break;

		case SIR_MAC_MGMT_PROBE_REQ:
			lim_process_probe_req_frame_multiple_bss(pMac,
								 pRxPacketInfo,
								 psessionEntry);
			break;

		case SIR_MAC_MGMT_PROBE_RSP:
			if (psessionEntry)
				lim_process_probe_rsp_frame(pMac,
							    pRxPacketInfo,
							    psessionEntry);
			break;

		case SIR_MAC_MGMT_BEACON:
			__lim_handle_beacon(pMac, limMsg, psessionEntry);
			break;

		case SIR_MAC_MGMT_DISASSOC:
			lim_process_disassoc_frame(pMac, pRxPacketInfo,
						   psessionEntry);
			break;

		case SIR_MAC_MGMT_AUTH:
			lim_process_auth_frame(pMac, pRxPacketInfo,
					       psessionEntry);
			break;

		case SIR_MAC_MGMT_DEAUTH:
			lim_process_deauth_frame(pMac, pRxPacketInfo,
						 psessionEntry);
			break;

		case SIR_MAC_MGMT_ACTION:
			if (psessionEntry == NULL)
				lim_process_action_frame_no_session(pMac,
								    pRxPacketInfo);
			else {
				if (WMA_GET_RX_UNKNOWN_UCAST
					    (pRxPacketInfo))
					lim_handle_unknown_a2_index_frames
						(pMac, pRxPacketInfo,
						psessionEntry);
				else
					lim_process_action_frame(pMac,
								 pRxPacketInfo,
								 psessionEntry);
			}
			break;
		default:
			/* Received Management frame of 'reserved' subtype */
			break;
		} /* switch (fc.subType) */

	}
	break;
	case SIR_MAC_DATA_FRAME:
	{
	}
	break;
	default:
		/* Received frame of type 'reserved' */
		break;

	} /* switch (fc.type) */

end:
	lim_pkt_free(pMac, TXRX_FRM_802_11_MGMT, pRxPacketInfo,
		     (void *)limMsg->bodyptr);
	return;
} /*** end lim_handle80211_frames() ***/

/**
 * lim_send_stop_scan_offload_req()
 *
 ***FUNCTION:
 * This function will be called to abort the ongoing offloaded scan
 * request.
 *
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @return QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 */
static QDF_STATUS lim_send_stop_scan_offload_req(tpAniSirGlobal pMac,
	uint8_t SessionId, uint32_t scan_id, uint32_t scan_requestor_id)
{
	struct scheduler_msg msg = {0};
	tSirRetStatus rc = eSIR_SUCCESS;
	tAbortScanParams *pAbortScanParams;

	pAbortScanParams = qdf_mem_malloc(sizeof(tAbortScanParams));
	if (NULL == pAbortScanParams) {
		pe_err("Memory allocation failed for AbortScanParams");
		return QDF_STATUS_E_NOMEM;
	}

	pAbortScanParams->SessionId = SessionId;
	pAbortScanParams->scan_id = scan_id;
	pAbortScanParams->scan_requestor_id = scan_requestor_id;
	msg.type = WMA_STOP_SCAN_OFFLOAD_REQ;
	msg.bodyptr = pAbortScanParams;
	msg.bodyval = 0;

	rc = wma_post_ctrl_msg(pMac, &msg);
	if (rc != eSIR_SUCCESS) {
		pe_err("wma_post_ctrl_msg() return failure");
		qdf_mem_free(pAbortScanParams);
		return QDF_STATUS_E_FAILURE;
	}

	pe_debug("Abort ongoing offload scan");
	return QDF_STATUS_SUCCESS;

}

/**
 * lim_process_abort_scan_ind() - abort the scan which is presently being run
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: PE session
 * @scan_id: Scan ID from the scan request
 * @scan_requesor_id: Entity requesting the scan
 *
 * @return: None
 */
void lim_process_abort_scan_ind(tpAniSirGlobal mac_ctx,
	uint8_t session_id, uint32_t scan_id, uint32_t scan_requestor_id)
{
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_SCAN_ABORT_IND_EVENT,
		NULL, 0, 0);
#endif

	pe_debug("scan_id %d, scan_requestor_id 0x%x",
		scan_id, scan_requestor_id);

	/* send stop scan cmd to firmware */
	lim_send_stop_scan_offload_req(mac_ctx, session_id, scan_id,
		scan_requestor_id);
	return;
}

static void lim_process_sme_obss_scan_ind(tpAniSirGlobal mac_ctx,
					  struct scheduler_msg *msg)
{
	struct sPESession *session;
	uint8_t session_id;
	struct sme_obss_ht40_scanind_msg *ht40_scanind;

	ht40_scanind = (struct sme_obss_ht40_scanind_msg *)msg->bodyptr;
	session = pe_find_session_by_bssid(mac_ctx,
			ht40_scanind->mac_addr.bytes, &session_id);
	if (session == NULL) {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			"OBSS Scan not started: session id is NULL");
		return;
	}
	if (session->htSupportedChannelWidthSet ==
			WNI_CFG_CHANNEL_BONDING_MODE_ENABLE) {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			"OBSS Scan Start Req: session id %d"
			"htSupportedChannelWidthSet %d",
			session->peSessionId,
			session->htSupportedChannelWidthSet);
		lim_send_ht40_obss_scanind(mac_ctx, session);
	} else {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			"OBSS Scan not started: channel width - %d session %d",
			session->htSupportedChannelWidthSet,
			session->peSessionId);
	}
	return;
}

/**
 * lim_process_messages() - Process messages from upper layers.
 *
 * @mac_ctx: Pointer to the Global Mac Context.
 * @msg: Received message.
 *
 * Return:  None.
 */
static void lim_process_messages(tpAniSirGlobal mac_ctx,
				 struct scheduler_msg *msg)
{
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	uint8_t vdev_id = 0;
	tUpdateBeaconParams beacon_params;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	uint8_t i;
	uint8_t p2p_go_exists = 0;
	tpPESession session_entry = NULL;
	uint8_t defer_msg = false;
	tLinkStateParams *link_state_param;
	uint16_t pkt_len = 0;
	cds_pkt_t *body_ptr = NULL;
	QDF_STATUS qdf_status;
	struct scheduler_msg new_msg = {0};
	tSirSmeScanAbortReq *req_msg = NULL;
	uint8_t session_id;
	uint32_t scan_id;

#ifdef FEATURE_WLAN_TDLS
	tSirTdlsInd *tdls_ind = NULL;
	tpDphHashNode sta_ds = NULL;
	tTdlsLinkEstablishParams *tdls_link_params = NULL;
#endif
	tSirMbMsgP2p *p2p_msg = NULL;
	tSirSetActiveModeSetBncFilterReq *bcn_filter_req = NULL;

	if (ANI_DRIVER_TYPE(mac_ctx) == eDRIVER_TYPE_MFG) {
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		return;
	}
	if (msg == NULL) {
		pe_err("Message pointer is Null");
		QDF_ASSERT(0);
		return;
	}
#ifdef WLAN_DEBUG
	mac_ctx->lim.numTot++;
#endif
	/*
	 * MTRACE logs not captured for events received from SME
	 * SME enums (eWNI_SME_START_REQ) starts with 0x16xx.
	 * Compare received SME events with SIR_SME_MODULE_ID
	 */
	if ((SIR_SME_MODULE_ID ==
	    (uint8_t)MAC_TRACE_GET_MODULE_ID(msg->type)) &&
	    (msg->type != eWNI_SME_REGISTER_MGMT_FRAME_REQ)) {
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_RX_SME_MSG,
				 NO_SESSION, msg->type));
	} else {
		/*
		 * Omitting below message types as these are too frequent
		 * and when crash happens we loose critical trace logs
		 * if these are also logged
		 */
		if (msg->type != SIR_CFG_PARAM_UPDATE_IND &&
		    msg->type != SIR_BB_XPORT_MGMT_MSG &&
		    msg->type != WMA_RX_SCAN_EVENT)
			MTRACE(mac_trace_msg_rx(mac_ctx, NO_SESSION,
				LIM_TRACE_MAKE_RXMSG(msg->type,
				LIM_MSG_PROCESSED));)
	}

	switch (msg->type) {

	case SIR_LIM_UPDATE_BEACON:
		lim_update_beacon(mac_ctx);
		break;
	case SIR_CFG_PARAM_UPDATE_IND:
		if (!lim_is_system_in_scan_state(mac_ctx)) {
			lim_handle_cf_gparam_update(mac_ctx, msg->bodyval);
			break;
		}
		/* System is in DFS (Learn) mode.
		 * Defer processsing this message
		 */
		if (lim_defer_msg(mac_ctx, msg) != TX_SUCCESS) {
			if (!(mac_ctx->lim.deferredMsgCnt & 0xF))
				QDF_TRACE(QDF_MODULE_ID_PE, LOGE,
					FL("Unable to Defer Msg"));
			lim_log_session_states(mac_ctx);
			lim_print_msg_name(mac_ctx, LOGE, msg->type);
		}
		break;
	case WMA_SWITCH_CHANNEL_RSP:
		lim_process_switch_channel_rsp(mac_ctx, msg->bodyptr);
		msg->bodyptr = NULL;
		break;
#ifdef ANI_SIR_IBSS_PEER_CACHING
	case WMA_IBSS_STA_ADD:
		lim_ibss_sta_add(mac_ctx, msg->bodyptr);
		break;
#endif
	case SIR_BB_XPORT_MGMT_MSG:
		/* These messages are from Peer MAC entity. */
#ifdef WLAN_DEBUG
		mac_ctx->lim.numBbt++;
#endif
		/* The original msg which we were deferring have the
		 * bodyPointer point to 'BD' instead of 'cds pkt'. If we
		 * don't make a copy of msg, then overwrite the
		 * msg->bodyPointer and next time when we try to
		 * process the msg, we will try to use 'BD' as
		 * 'cds Pkt' which will cause a crash
		 */
		if (msg->bodyptr == NULL) {
			pe_err("Message bodyptr is Null");
			QDF_ASSERT(0);
			break;
		}
		qdf_mem_copy((uint8_t *) &new_msg,
			(uint8_t *) msg, sizeof(struct scheduler_msg));
		body_ptr = (cds_pkt_t *) new_msg.bodyptr;
		cds_pkt_get_packet_length(body_ptr, &pkt_len);

		qdf_status = wma_ds_peek_rx_packet_info(body_ptr,
			(void **) &new_msg.bodyptr, false);

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			lim_decrement_pending_mgmt_count(mac_ctx);
			cds_pkt_return_packet(body_ptr);
			break;
		}

		pe_debug("roamCandidateInd: %d offloadScanLearn: %d",
				WMA_GET_ROAMCANDIDATEIND(new_msg.bodyptr),
				WMA_GET_OFFLOADSCANLEARN(new_msg.bodyptr));

		lim_handle80211_frames(mac_ctx, &new_msg, &defer_msg);

		if (defer_msg == true) {
			QDF_TRACE(QDF_MODULE_ID_PE, LOG1,
					FL("Defer Msg type=%x"), msg->type);
			if (lim_defer_msg(mac_ctx, msg) != TX_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_PE, LOGE,
						FL("Unable to Defer Msg"));
				lim_log_session_states(mac_ctx);
				lim_decrement_pending_mgmt_count(mac_ctx);
				cds_pkt_return_packet(body_ptr);
			}
		} else {
			/* PE is not deferring this 802.11 frame so we need to
			 * call cds_pkt_return. Asumption here is when Rx mgmt
			 * frame processing is done, cds packet could be
			 * freed here.
			 */
			lim_decrement_pending_mgmt_count(mac_ctx);
			cds_pkt_return_packet(body_ptr);
		}
		break;
	case eWNI_SME_SCAN_REQ:
	case eWNI_SME_REMAIN_ON_CHANNEL_REQ:
	case eWNI_SME_DISASSOC_REQ:
	case eWNI_SME_DEAUTH_REQ:
#ifdef FEATURE_WLAN_TDLS
	case eWNI_SME_TDLS_SEND_MGMT_REQ:
	case eWNI_SME_TDLS_ADD_STA_REQ:
	case eWNI_SME_TDLS_DEL_STA_REQ:
	case eWNI_SME_TDLS_LINK_ESTABLISH_REQ:
#endif
	case eWNI_SME_RESET_AP_CAPS_CHANGED:
	case eWNI_SME_SET_HW_MODE_REQ:
	case eWNI_SME_SET_DUAL_MAC_CFG_REQ:
	case eWNI_SME_SET_ANTENNA_MODE_REQ:
	case eWNI_SME_UPDATE_ACCESS_POLICY_VENDOR_IE:
		/* These messages are from HDD. Need to respond to HDD */
		lim_process_normal_hdd_msg(mac_ctx, msg, true);
		break;
	case eWNI_SME_SEND_DISASSOC_FRAME:
		/* Need to response to hdd */
		lim_process_normal_hdd_msg(mac_ctx, msg, true);
		break;
	case eWNI_SME_SCAN_ABORT_IND:
		req_msg = msg->bodyptr;
		if (req_msg) {
			session_id = req_msg->sessionId;
			scan_id = req_msg->scan_id;
			lim_process_abort_scan_ind(mac_ctx, session_id,
				scan_id, USER_SCAN_REQUESTOR_ID);
			qdf_mem_free((void *)msg->bodyptr);
			msg->bodyptr = NULL;
		}
		break;
	case eWNI_SME_PDEV_SET_HT_VHT_IE:
	case eWNI_SME_SET_VDEV_IES_PER_BAND:
	case eWNI_SME_SYS_READY_IND:
	case eWNI_SME_JOIN_REQ:
	case eWNI_SME_REASSOC_REQ:
	case eWNI_SME_START_BSS_REQ:
	case eWNI_SME_STOP_BSS_REQ:
	case eWNI_SME_SWITCH_CHL_IND:
	case eWNI_SME_SETCONTEXT_REQ:
	case eWNI_SME_DISASSOC_CNF:
	case eWNI_SME_DEAUTH_CNF:
	case eWNI_SME_ASSOC_CNF:
	case eWNI_SME_ADDTS_REQ:
	case eWNI_SME_DELTS_REQ:
	case eWNI_SME_GET_ASSOC_STAS_REQ:
	case eWNI_SME_TKIP_CNTR_MEAS_REQ:
	case eWNI_SME_UPDATE_APWPSIE_REQ:
	case eWNI_SME_SESSION_UPDATE_PARAM:
	case eWNI_SME_GET_WPSPBC_SESSION_REQ:
	case eWNI_SME_SET_APWPARSNIEs_REQ:
	case eWNI_SME_CHNG_MCC_BEACON_INTERVAL:
	case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
	case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
#if defined FEATURE_WLAN_ESE
	case eWNI_SME_ESE_ADJACENT_AP_REPORT:
#endif
	case eWNI_SME_FT_UPDATE_KEY:
	case eWNI_SME_FT_PRE_AUTH_REQ:
	case eWNI_SME_FT_AGGR_QOS_REQ:
	case eWNI_SME_REGISTER_MGMT_FRAME_REQ:
	case eWNI_SME_UPDATE_NOA:
	case eWNI_SME_CLEAR_DFS_CHANNEL_LIST:
	case eWNI_SME_GET_STATISTICS_REQ:
#ifdef FEATURE_WLAN_ESE
	case eWNI_SME_GET_TSM_STATS_REQ:
#endif  /* FEATURE_WLAN_ESE */
	case eWNI_SME_REGISTER_MGMT_FRAME_CB:
	case eWNI_SME_EXT_CHANGE_CHANNEL:
	case eWNI_SME_ROAM_SCAN_OFFLOAD_REQ:
	case eWNI_SME_NDP_INITIATOR_REQ:
	case eWNI_SME_NDP_RESPONDER_REQ:
	case eWNI_SME_NDP_END_REQ:
	case eWNI_SME_REGISTER_P2P_ACK_CB:
		/* These messages are from HDD.No need to respond to HDD */
		lim_process_normal_hdd_msg(mac_ctx, msg, false);
		break;
	case eWNI_SME_SEND_ACTION_FRAME_IND:
		lim_send_p2p_action_frame(mac_ctx, msg);
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_ABORT_REMAIN_ON_CHAN_IND:
		p2p_msg = (tSirMbMsgP2p *) msg->bodyptr;
		lim_process_abort_scan_ind(mac_ctx, p2p_msg->sessionId,
			p2p_msg->scan_id, ROC_SCAN_REQUESTOR_ID);
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_MON_INIT_SESSION:
		lim_mon_init_session(mac_ctx, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case SIR_HAL_P2P_NOA_START_IND:
		session_entry = &mac_ctx->lim.gpSession[0];
		pe_debug("LIM received NOA start %x", msg->type);

		/* Since insert NOA is done and NOA start msg received,
		 * we should deactivate the Insert NOA timer
		 */
		lim_deactivate_and_change_timer(mac_ctx,
			eLIM_INSERT_SINGLESHOT_NOA_TIMER);

		for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
			session_entry = &mac_ctx->lim.gpSession[i];
			if ((session_entry != NULL) && (session_entry->valid) &&
				(session_entry->pePersona == QDF_P2P_GO_MODE)) {
				/* Save P2P NOA start attribute for Go persona*/
				p2p_go_exists = 1;
				qdf_mem_copy(&session_entry->p2pGoPsNoaStartInd,
					msg->bodyptr, sizeof(tSirP2PNoaStart));
				qdf_status =
					session_entry->p2pGoPsNoaStartInd.status;
				if (qdf_status != QDF_STATUS_SUCCESS)
					QDF_TRACE(QDF_MODULE_ID_PE, LOGW,
						FL(
						"GO NOA start status %d by FW"),
						qdf_status);
				break;
			}
		}

		if (p2p_go_exists == 0)
			QDF_TRACE(QDF_MODULE_ID_PE, LOGW,
				FL(
				"GO is removed by the time NOA start recvd"));

		/* We received the NOA start indication. Now we can send down
		 * the SME request which requires off-channel operation */
		lim_process_regd_defd_sme_req_after_noa_start(mac_ctx);
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
#ifdef FEATURE_WLAN_TDLS
	case SIR_HAL_TDLS_IND:
		tdls_ind = (tpSirTdlsInd) msg->bodyptr;
		session_entry = pe_find_session_by_sta_id(mac_ctx,
			tdls_ind->staIdx, &session_id);
		if (session_entry == NULL) {
			pe_err("No session exist for given bssId");
				qdf_mem_free(msg->bodyptr);
				msg->bodyptr = NULL;
				return;
			}
		sta_ds = dph_get_hash_entry(mac_ctx, tdls_ind->assocId,
			&session_entry->dph.dphHashTable);
			if (sta_ds == NULL) {
				pe_debug("No sta_ds exist for given staId");
				qdf_mem_free(msg->bodyptr);
				msg->bodyptr = NULL;
				return;
			}

			if (STA_ENTRY_TDLS_PEER == sta_ds->staType) {
				pe_err("rcvd TDLS IND from FW with RC %d",
					tdls_ind->reasonCode);
				lim_send_sme_tdls_del_sta_ind(mac_ctx, sta_ds,
					session_entry, tdls_ind->reasonCode);
			}
			qdf_mem_free(msg->bodyptr);
			msg->bodyptr = NULL;
		break;
#endif
	case SIR_HAL_P2P_NOA_ATTR_IND:
		session_entry = &mac_ctx->lim.gpSession[0];
		pe_debug("Received message Noa_ATTR %x",
			msg->type);
		for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
			session_entry = &mac_ctx->lim.gpSession[i];
			if ((session_entry != NULL) && (session_entry->valid)
				&& (session_entry->pePersona ==
				QDF_P2P_GO_MODE)) { /* Save P2P attr for Go */
					qdf_mem_copy(
						&session_entry->p2pGoPsUpdate,
						msg->bodyptr,
						sizeof(tSirP2PNoaAttr));
					pe_debug("bssId"
						MAC_ADDRESS_STR
						" ctWin=%d oppPsFlag=%d",
						MAC_ADDR_ARRAY(
							session_entry->bssId),
						session_entry->p2pGoPsUpdate.ctWin,
						session_entry->p2pGoPsUpdate.oppPsFlag);
					pe_debug("uNoa1IntervalCnt=%d uNoa1Duration=%d uNoa1Interval=%d uNoa1StartTime=%d",
						session_entry->p2pGoPsUpdate.uNoa1IntervalCnt,
						session_entry->p2pGoPsUpdate.uNoa1Duration,
						session_entry->p2pGoPsUpdate.uNoa1Interval,
						session_entry->p2pGoPsUpdate.uNoa1StartTime);
					break;
			}
		}
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case WMA_MISSED_BEACON_IND:
		lim_ps_offload_handle_missed_beacon_ind(mac_ctx, msg);
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case SIR_LIM_ADDTS_RSP_TIMEOUT:
		lim_process_sme_req_messages(mac_ctx, msg);
		break;
#ifdef FEATURE_WLAN_ESE
	case WMA_TSM_STATS_RSP:
		lim_send_sme_pe_ese_tsm_rsp(mac_ctx,
			(tAniGetTsmStatsRsp *) msg->bodyptr);
		break;
#endif
	case WMA_ADD_TS_RSP:
		lim_process_hal_add_ts_rsp(mac_ctx, msg);
		break;
	case SIR_LIM_DEL_TS_IND:
		lim_process_del_ts_ind(mac_ctx, msg);
		break;
	case SIR_LIM_BEACON_GEN_IND:
		if (mac_ctx->lim.gLimSystemRole != eLIM_AP_ROLE)
			sch_process_pre_beacon_ind(mac_ctx, msg);
		break;
	case SIR_LIM_DELETE_STA_CONTEXT_IND:
		lim_delete_sta_context(mac_ctx, msg);
		break;
	case SIR_LIM_PERIODIC_PROBE_REQ_TIMEOUT:
	case SIR_LIM_JOIN_FAIL_TIMEOUT:
	case SIR_LIM_PERIODIC_JOIN_PROBE_REQ_TIMEOUT:
	case SIR_LIM_AUTH_FAIL_TIMEOUT:
	case SIR_LIM_AUTH_RSP_TIMEOUT:
	case SIR_LIM_ASSOC_FAIL_TIMEOUT:
	case SIR_LIM_REASSOC_FAIL_TIMEOUT:
	case SIR_LIM_FT_PREAUTH_RSP_TIMEOUT:
	case SIR_LIM_REMAIN_CHN_TIMEOUT:
	case SIR_LIM_INSERT_SINGLESHOT_NOA_TIMEOUT:
	case SIR_LIM_DISASSOC_ACK_TIMEOUT:
	case SIR_LIM_DEAUTH_ACK_TIMEOUT:
	case SIR_LIM_CONVERT_ACTIVE_CHANNEL_TO_PASSIVE:
	case SIR_LIM_AUTH_RETRY_TIMEOUT:
		/* These timeout messages are handled by MLM sub module */
		lim_process_mlm_req_messages(mac_ctx, msg);
		break;
	case SIR_LIM_HEART_BEAT_TIMEOUT:
		/** check if heart beat failed, even if one Beacon
		 * is rcvd within the Heart Beat interval continue
		 * normal processing
		 */
		if (NULL == msg->bodyptr)
			pe_err("Can't Process HB TO - bodyptr is Null");
		else {
			session_entry = (tpPESession) msg->bodyptr;
			pe_err("SIR_LIM_HEART_BEAT_TIMEOUT, Session %d",
				((tpPESession) msg->bodyptr)->peSessionId);
			limResetHBPktCount(session_entry);
			lim_handle_heart_beat_timeout_for_session(mac_ctx,
							session_entry);
		}
		break;
	case SIR_LIM_PROBE_HB_FAILURE_TIMEOUT:
		lim_handle_heart_beat_failure_timeout(mac_ctx);
		break;
		break;
	case SIR_LIM_CNF_WAIT_TIMEOUT:
		/* Does not receive CNF or dummy packet */
		lim_handle_cnf_wait_timeout(mac_ctx, (uint16_t) msg->bodyval);
		break;
	case SIR_LIM_RETRY_INTERRUPT_MSG:
		/* Message from ISR upon TFP's max retry limit interrupt */
		break;
	case SIR_LIM_INV_KEY_INTERRUPT_MSG:
		/* Message from ISR upon SP's Invalid session key interrupt */
		break;
	case SIR_LIM_KEY_ID_INTERRUPT_MSG:
		/* Message from ISR upon SP's Invalid key ID interrupt */
		break;
	case SIR_LIM_REPLAY_THRES_INTERRUPT_MSG:
		/* Message from ISR upon SP's Replay threshold interrupt */
		break;
	case SIR_LIM_CHANNEL_SWITCH_TIMEOUT:
		lim_process_channel_switch_timeout(mac_ctx);
		break;
	case SIR_LIM_QUIET_TIMEOUT:
		lim_process_quiet_timeout(mac_ctx);
		break;
	case SIR_LIM_QUIET_BSS_TIMEOUT:
		lim_process_quiet_bss_timeout(mac_ctx);
		break;
	case SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT:
		lim_handle_update_olbc_cache(mac_ctx);
		break;
#ifdef FEATURE_WLAN_TDLS
	case SIR_HAL_TDLS_SHOULD_DISCOVER:
	case SIR_HAL_TDLS_SHOULD_TEARDOWN:
	case SIR_HAL_TDLS_PEER_DISCONNECTED:
	case SIR_HAL_TDLS_CONNECTION_TRACKER_NOTIFICATION:
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
			("%s received tdls event: 0x%x"), __func__, msg->type);
		lim_send_sme_tdls_event_notify(mac_ctx, msg->type,
					(void *)msg->bodyptr);
		break;
#endif
	case WMA_ADD_BSS_RSP:
		lim_process_mlm_add_bss_rsp(mac_ctx, msg);
		break;
	case WMA_ADD_STA_RSP:
		lim_process_add_sta_rsp(mac_ctx, msg);
		break;
	case WMA_DELETE_STA_RSP:
		lim_process_mlm_del_sta_rsp(mac_ctx, msg);
		break;
	case WMA_DELETE_BSS_RSP:
	case WMA_DELETE_BSS_HO_FAIL_RSP:
		lim_handle_delete_bss_rsp(mac_ctx, msg);
		break;
	case WMA_CSA_OFFLOAD_EVENT:
		lim_handle_csa_offload_msg(mac_ctx, msg);
		break;
	case WMA_SET_BSSKEY_RSP:
	case WMA_SET_STA_BCASTKEY_RSP:
		lim_process_mlm_set_bss_key_rsp(mac_ctx, msg);
		break;
	case WMA_SET_STAKEY_RSP:
		lim_process_mlm_set_sta_key_rsp(mac_ctx, msg);
		break;
	case WMA_GET_STATISTICS_RSP:
		lim_send_sme_pe_statistics_rsp(mac_ctx, msg->type,
					     (void *)msg->bodyptr);
		break;
	case WMA_SET_MIMOPS_RSP:
	case WMA_SET_TX_POWER_RSP:
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case WMA_SET_MAX_TX_POWER_RSP:
		rrm_set_max_tx_power_rsp(mac_ctx, msg);
		if (msg->bodyptr != NULL) {
			qdf_mem_free((void *)msg->bodyptr);
			msg->bodyptr = NULL;
		}
		break;
	case SIR_LIM_ADDR2_MISS_IND:
		pe_err(
			FL("Addr2 mismatch interrupt received %X"), msg->type);
		/* message from HAL indicating addr2 mismatch interrupt occurred
		 * msg->bodyptr contains only pointer to 48-bit addr2 field
		 */
		qdf_mem_free((void *)(msg->bodyptr));
		msg->bodyptr = NULL;
		break;
	case WMA_AGGR_QOS_RSP:
		lim_process_ft_aggr_qo_s_rsp(mac_ctx, msg);
		break;
	case WMA_SET_LINK_STATE_RSP:
		link_state_param = (tLinkStateParams *) msg->bodyptr;
		session_entry = link_state_param->session;
		if (link_state_param->ft
#if defined WLAN_FEATURE_ROAM_OFFLOAD
			&& !session_entry->bRoamSynchInProgress
#endif
		)
			lim_send_reassoc_req_with_ft_ies_mgmt_frame(mac_ctx,
				session_entry->pLimMlmReassocReq,
				session_entry);
		if (link_state_param->callback)
			link_state_param->callback(mac_ctx,
				link_state_param->callbackArg,
				link_state_param->status);
		qdf_mem_free((void *)(msg->bodyptr));
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_SET_BCN_FILTER_REQ:
		bcn_filter_req =
			(tSirSetActiveModeSetBncFilterReq *) msg->bodyptr;
		session_entry = pe_find_session_by_bssid(mac_ctx,
			bcn_filter_req->bssid.bytes, &session_id);
		if ((session_entry != NULL) &&
			(lim_send_beacon_filter_info(mac_ctx, session_entry) !=
			 eSIR_SUCCESS))
			pe_err("Failied to send Beacon Filter Info");
		qdf_mem_free((void *)(msg->bodyptr));
		msg->bodyptr = NULL;
		break;
#ifdef FEATURE_WLAN_TDLS
	case WMA_SET_TDLS_LINK_ESTABLISH_REQ_RSP:
		tdls_link_params = (tTdlsLinkEstablishParams *) msg->bodyptr;
		session_entry = pe_find_session_by_sta_id(mac_ctx,
			tdls_link_params->staIdx, &session_id);
		if (session_entry == NULL) {
			pe_err("session %u does not exist", session_id);
			/* Still send the eWNI_SME_TDLS_LINK_ESTABLISH_RSP
			 * message to SME with session id as zero and status
			 * as FAILURE so, that message queued in SME queue
			 * can be freed to prevent the SME cmd buffer leak
			 */
			lim_send_sme_tdls_link_establish_req_rsp(mac_ctx, 0,
				NULL, NULL, eSIR_FAILURE);
		} else {
			lim_send_sme_tdls_link_establish_req_rsp(mac_ctx,
				session_entry->smeSessionId, NULL, NULL,
				tdls_link_params->status);
		}
		qdf_mem_free((void *)(msg->bodyptr));
		msg->bodyptr = NULL;
		break;
#endif
	case WMA_RX_SCAN_EVENT:
		lim_process_rx_scan_event(mac_ctx, msg->bodyptr);
		break;
	case WMA_IBSS_PEER_INACTIVITY_IND:
		lim_process_ibss_peer_inactivity(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)(msg->bodyptr));
		msg->bodyptr = NULL;
		break;
	case WMA_DFS_BEACON_TX_SUCCESS_IND:
		lim_process_beacon_tx_success_ind(mac_ctx, msg->type,
				(void *)msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case WMA_DISASSOC_TX_COMP:
		lim_disassoc_tx_complete_cnf(mac_ctx, NULL, msg->bodyval, NULL);
		break;
	case WMA_DEAUTH_TX_COMP:
		lim_deauth_tx_complete_cnf(mac_ctx, NULL, msg->bodyval, NULL);
		break;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	case WMA_UPDATE_Q2Q_IE_IND:
		qdf_mem_zero(&beacon_params, sizeof(tUpdateBeaconParams));
		beacon_params.paramChangeBitmap = 0;
		for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
			vdev_id = ((uint8_t *)msg->bodyptr)[i];
			session_entry = pe_find_session_by_sme_session_id(
				mac_ctx, vdev_id);
			if (session_entry == NULL)
				continue;
			session_entry->sap_advertise_avoid_ch_ie =
				(uint8_t)msg->bodyval;
			/*
			 * if message comes for DFS channel, no need to update:
			 * 1) We wont have MCC with DFS channels. so no need to
			 *    add Q2Q IE
			 * 2) We cannot end up in DFS channel SCC by channel
			 *    switch from non DFS MCC scenario, so no need to
			 *    remove Q2Q IE
			 * 3) There is however a case where device start MCC and
			 *    then user modifies hostapd.conf and does SAP
			 *    restart, in such a case, beacon params will be
			 *    reset and thus will not contain Q2Q IE, by default
			 */
			if (wlan_reg_get_channel_state(mac_ctx->pdev,
					session_entry->currentOperChannel)
					!= CHANNEL_STATE_DFS) {
				beacon_params.bssIdx = session_entry->bssIdx;
				beacon_params.beaconInterval =
					session_entry->beaconParams.beaconInterval;
				beacon_params.paramChangeBitmap |=
					PARAM_BCN_INTERVAL_CHANGED;
				sch_set_fixed_beacon_fields(mac_ctx,
					session_entry);
				lim_send_beacon_params(mac_ctx, &beacon_params,
					session_entry);
			}
		}
		qdf_mem_free(msg->bodyptr);
		msg->bodyptr = NULL;
		break;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	case eWNI_SME_NSS_UPDATE_REQ:
	case eWNI_SME_DFS_BEACON_CHAN_SW_IE_REQ:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_CHANNEL_CHANGE_REQ:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_START_BEACON_REQ:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_UPDATE_ADDITIONAL_IES:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_MODIFY_ADDITIONAL_IES:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
#ifdef QCA_HT_2040_COEX
	case eWNI_SME_SET_HT_2040_MODE:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
#endif
	case SIR_HAL_PDEV_SET_HW_MODE_RESP:
		lim_process_set_hw_mode_resp(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case SIR_HAL_PDEV_HW_MODE_TRANS_IND:
		lim_process_hw_mode_trans_ind(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case SIR_HAL_PDEV_MAC_CFG_RESP:
		lim_process_dual_mac_cfg_resp(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_SET_IE_REQ:
		lim_process_sme_req_messages(mac_ctx, msg);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_HT40_OBSS_SCAN_IND:
		lim_process_sme_obss_scan_ind(mac_ctx, msg);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_SOC_ANTENNA_MODE_RESP:
		lim_process_set_antenna_resp(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_DEFAULT_SCAN_IE:
		lim_process_set_default_scan_ie_request(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	case eWNI_SME_DEL_ALL_TDLS_PEERS:
		lim_process_sme_del_all_tdls_peers(mac_ctx, msg->bodyptr);
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		break;
	default:
		qdf_mem_free((void *)msg->bodyptr);
		msg->bodyptr = NULL;
		pe_err("Discarding unexpected message received %X",
			msg->type);
		lim_print_msg_name(mac_ctx, LOGE, msg->type);
		break;

	} /* switch (msg->type) */
} /*** end lim_process_messages() ***/

/**
 * lim_process_deferred_message_queue()
 *
 * This function is called by LIM while exiting from Learn
 * mode. This function fetches messages posted to the LIM
 * deferred message queue limDeferredMsgQ.
 *
 * @pMac: Pointer to Global MAC structure
 * @return None
 */

static void lim_process_deferred_message_queue(tpAniSirGlobal pMac)
{
	struct scheduler_msg limMsg = {0};
	struct scheduler_msg *readMsg;
	uint16_t size;

	/*
	** check any deferred messages need to be processed
	**/
	size = pMac->lim.gLimDeferredMsgQ.size;
	if (size > 0) {
		while ((readMsg = lim_read_deferred_msg_q(pMac)) != NULL) {
			qdf_mem_copy((uint8_t *) &limMsg,
				     (uint8_t *) readMsg,
				     sizeof(struct scheduler_msg));
			size--;
			lim_process_messages(pMac, &limMsg);

			if ((lim_is_system_in_scan_state(pMac))
			    || (true != GET_LIM_PROCESS_DEFD_MESGS(pMac))
			    || (pMac->lim.gLimSystemInScanLearnMode)
			    ||  pMac->lim.gLimAddtsSent)
				break;
		}
	}
} /*** end lim_process_deferred_message_queue() ***/

/**
 * lim_message_processor() - Process messages from LIM.
 * @mac_ctx: Pointer to the Global Mac Context.
 * @msg: Received LIM message.
 *
 * Wrapper function for lim_process_messages when handling messages received by
 * LIM. Could either defer messages or process them.
 *
 * Return:  None.
 */
void lim_message_processor(tpAniSirGlobal mac_ctx, struct scheduler_msg *msg)
{
	if (eLIM_MLM_OFFLINE_STATE == mac_ctx->lim.gLimMlmState) {
		pe_free_msg(mac_ctx, msg);
		return;
	}

	if (!def_msg_decision(mac_ctx, msg)) {
		lim_process_messages(mac_ctx, msg);
		/* process deferred message queue if allowed */
		if ((!(mac_ctx->lim.gLimAddtsSent)) &&
		    (!(lim_is_system_in_scan_state(mac_ctx))) &&
		    (true == GET_LIM_PROCESS_DEFD_MESGS(mac_ctx)))
			lim_process_deferred_message_queue(mac_ctx);
	}
}

/**
 * lim_process_normal_hdd_msg() - Process the message and defer if needed
 * @mac_ctx :     Pointer to Global MAC structure
 * @msg     :     The message need to be processed
 * @rsp_reqd:     whether return result to hdd
 *
 * This function checks the current lim state and decide whether the message
 * passed will be deferred or not.
 *
 * Return: None
 */
static void lim_process_normal_hdd_msg(tpAniSirGlobal mac_ctx,
				       struct scheduler_msg *msg,
				       uint8_t rsp_reqd)
{
	bool defer_msg = true;

	/* Added For BT-AMP Support */
	if ((mac_ctx->lim.gLimSystemRole == eLIM_AP_ROLE)
		|| (mac_ctx->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)) {
		/*
		 * This check is required only for the AP and in 2 cases.
		 * 1. If we are in learn mode and we receive any of these
		 * messages, you have to come out of scan and process the
		 * message, hence dont defer the message here. In handler,
		 * these message could be defered till we actually come out of
		 * scan mode.
		 * 2. If radar is detected, you might have to defer all of
		 * these messages except Stop BSS request/ Switch channel
		 * request. This decision is also made inside its handler.
		 *
		 * Please be careful while using the flag defer_msg. Possibly
		 * you might end up in an infinite loop.
		 */
		if ((msg->type == eWNI_SME_START_BSS_REQ) ||
			(msg->type == eWNI_SME_STOP_BSS_REQ) ||
			(msg->type == eWNI_SME_SWITCH_CHL_IND))
			defer_msg = false;
	}

	if (((mac_ctx->lim.gLimAddtsSent) ||
		(lim_is_system_in_scan_state(mac_ctx))) && defer_msg) {
		/*
		 * System is in DFS (Learn) mode or awaiting addts response or
		 * if radar is detected, Defer processsing this message
		 */
		if (lim_defer_msg(mac_ctx, msg) != TX_SUCCESS) {
#ifdef WLAN_DEBUG
			mac_ctx->lim.numSme++;
#endif
			lim_log_session_states(mac_ctx);
			/* Release body */
			qdf_mem_free(msg->bodyptr);
			msg->bodyptr = NULL;
		}
	} else {
		/*
		 * These messages are from HDD.Since these requests may also be
		 * generated internally within LIM module, need to distinquish
		 * and send response to host
		 */
		if (rsp_reqd)
			mac_ctx->lim.gLimRspReqd = true;
#ifdef WLAN_DEBUG
		mac_ctx->lim.numSme++;
#endif
		if (lim_process_sme_req_messages(mac_ctx, msg)) {
			/*
			 * Release body. limProcessSmeReqMessage consumed the
			 * buffer. We can free it.
			 */
			qdf_mem_free(msg->bodyptr);
			msg->bodyptr = NULL;
		}
	}
}

void
handle_ht_capabilityand_ht_info(struct sAniSirGlobal *pMac,
				tpPESession psessionEntry)
{
	tSirMacHTCapabilityInfo macHTCapabilityInfo;
	tSirMacHTParametersInfo macHTParametersInfo;
	tSirMacHTInfoField1 macHTInfoField1;
	tSirMacHTInfoField2 macHTInfoField2;
	tSirMacHTInfoField3 macHTInfoField3;
	uint32_t cfgValue;
	uint8_t *ptr;

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_CAP_INFO, &cfgValue) !=
	    eSIR_SUCCESS) {
		pe_err("Fail to retrieve WNI_CFG_HT_CAP_INFO value");
		return;
	}
	ptr = (uint8_t *) &macHTCapabilityInfo;
	*((uint16_t *) ptr) = (uint16_t) (cfgValue & 0xffff);
	pMac->lim.gHTLsigTXOPProtection =
		(uint8_t) macHTCapabilityInfo.lsigTXOPProtection;
	pMac->lim.gHTMIMOPSState =
		(tSirMacHTMIMOPowerSaveState) macHTCapabilityInfo.mimoPowerSave;
	pMac->lim.gHTGreenfield = (uint8_t) macHTCapabilityInfo.greenField;
	pMac->lim.gHTMaxAmsduLength =
		(uint8_t) macHTCapabilityInfo.maximalAMSDUsize;
	pMac->lim.gHTShortGI20Mhz = (uint8_t) macHTCapabilityInfo.shortGI20MHz;
	pMac->lim.gHTShortGI40Mhz = (uint8_t) macHTCapabilityInfo.shortGI40MHz;
	pMac->lim.gHTPSMPSupport = (uint8_t) macHTCapabilityInfo.psmp;
	pMac->lim.gHTDsssCckRate40MHzSupport =
		(uint8_t) macHTCapabilityInfo.dsssCckMode40MHz;

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_AMPDU_PARAMS, &cfgValue) !=
	    eSIR_SUCCESS) {
		pe_err("Fail to retrieve WNI_CFG_HT_PARAM_INFO value");
		return;
	}
	ptr = (uint8_t *) &macHTParametersInfo;
	*ptr = (uint8_t) (cfgValue & 0xff);
	pMac->lim.gHTAMpduDensity = (uint8_t) macHTParametersInfo.mpduDensity;
	pMac->lim.gHTMaxRxAMpduFactor =
		(uint8_t) macHTParametersInfo.maxRxAMPDUFactor;

	/* Get HT IE Info */
	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_INFO_FIELD1, &cfgValue) !=
	    eSIR_SUCCESS) {
		pe_err("Fail to retrieve WNI_CFG_HT_INFO_FIELD1 value");
		return;
	}
	ptr = (uint8_t *) &macHTInfoField1;
	*((uint8_t *) ptr) = (uint8_t) (cfgValue & 0xff);
	pMac->lim.gHTServiceIntervalGranularity =
		(uint8_t) macHTInfoField1.serviceIntervalGranularity;
	pMac->lim.gHTControlledAccessOnly =
		(uint8_t) macHTInfoField1.controlledAccessOnly;
	pMac->lim.gHTRifsMode = (uint8_t) macHTInfoField1.rifsMode;

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_INFO_FIELD2, &cfgValue) !=
	    eSIR_SUCCESS) {
		pe_err("Fail to retrieve WNI_CFG_HT_INFO_FIELD2 value");
		return;
	}
	ptr = (uint8_t *) &macHTInfoField2;
	*((uint16_t *) ptr) = (uint16_t) (cfgValue & 0xffff);
	pMac->lim.gHTOperMode = (tSirMacHTOperatingMode) macHTInfoField2.opMode;

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_INFO_FIELD3, &cfgValue) !=
	    eSIR_SUCCESS) {
		pe_err("Fail to retrieve WNI_CFG_HT_INFO_FIELD3 value");
		return;
	}
	ptr = (uint8_t *) &macHTInfoField3;
	*((uint16_t *) ptr) = (uint16_t) (cfgValue & 0xffff);
	pMac->lim.gHTPCOActive = (uint8_t) macHTInfoField3.pcoActive;
	pMac->lim.gHTPCOPhase = (uint8_t) macHTInfoField3.pcoPhase;
	pMac->lim.gHTSecondaryBeacon =
		(uint8_t) macHTInfoField3.secondaryBeacon;
	pMac->lim.gHTDualCTSProtection =
		(uint8_t) macHTInfoField3.dualCTSProtection;
	pMac->lim.gHTSTBCBasicMCS = (uint8_t) macHTInfoField3.basicSTBCMCS;

	/* The lim globals for channelwidth and secondary chnl have been removed and should not be used during no session;
	 * instead direct cfg is read and used when no session for transmission of mgmt frames (same as old);
	 * For now, we might come here during init and join with sessionEntry = NULL; in that case just fill the globals which exist
	 * Sessionized entries values will be filled in join or add bss req. The ones which are missed in join are filled below
	 */
	if (psessionEntry != NULL) {
		psessionEntry->htCapability =
			IS_DOT11_MODE_HT(psessionEntry->dot11mode);
		psessionEntry->beaconParams.fLsigTXOPProtectionFullSupport =
			(uint8_t) macHTInfoField3.lsigTXOPProtectionFullSupport;
		lim_init_obss_params(pMac, psessionEntry);
	}
}

void lim_log_session_states(tpAniSirGlobal mac_ctx)
{
#ifdef WLAN_DEBUG
	int i;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		if (mac_ctx->lim.gpSession[i].valid) {
			QDF_TRACE(QDF_MODULE_ID_PE, LOG1,
				FL("sysRole(%d) Session (%d)"),
				mac_ctx->lim.gLimSystemRole, i);
			QDF_TRACE(QDF_MODULE_ID_PE, LOG1,
				FL("SME: Curr %s,Prev %s,MLM: Curr %s,Prev %s"),
				lim_sme_state_str(
				mac_ctx->lim.gpSession[i].limSmeState),
				lim_sme_state_str(
				mac_ctx->lim.gpSession[i].limPrevSmeState),
				lim_mlm_state_str(
				mac_ctx->lim.gpSession[i].limMlmState),
				lim_mlm_state_str(
				mac_ctx->lim.gpSession[i].limPrevMlmState));
		}
	}
#endif
}
