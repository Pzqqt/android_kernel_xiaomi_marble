/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
 * This file lim_process_sme_req_messages.cc contains the code
 * for processing SME request messages.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "cds_api.h"
#include "wni_api.h"
#include "wni_cfg.h"
#include "cfg_api.h"
#include "sir_api.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_sme_req_utils.h"
#include "lim_ibss_peer_mgmt.h"
#include "lim_admit_control.h"
#include "dph_hash_table.h"
#include "lim_send_messages.h"
#include "lim_api.h"
#include "wmm_apsd.h"
#include "sir_mac_prot_def.h"
#include "rrm_api.h"

#include "sap_api.h"


#if defined WLAN_FEATURE_VOWIFI_11R
#include <lim_ft.h>
#endif
#include "cds_regdomain_common.h"

/*
 * This overhead is time for sending NOA start to host in case of GO/sending
 * NULL data & receiving ACK in case of P2P Client and starting actual scanning
 * with init scan req/rsp plus in case of concurrency, taking care of sending
 * null data and receiving ACK to/from AP/Also SetChannel with calibration
 * is taking around 7ms .
 */
#define SCAN_MESSAGING_OVERHEAD             20  /* in msecs */
#define JOIN_NOA_DURATION                   2000        /* in msecs */
#define OEM_DATA_NOA_DURATION               60  /* in msecs */
#define DEFAULT_PASSIVE_MAX_CHANNEL_TIME    110 /* in msecs */

#define CONV_MS_TO_US 1024      /* conversion factor from ms to us */

/* SME REQ processing function templates */
static bool __lim_process_sme_sys_ready_ind(tpAniSirGlobal, uint32_t *);
static bool __lim_process_sme_start_bss_req(tpAniSirGlobal, tpSirMsgQ pMsg);
static void __lim_process_sme_scan_req(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_join_req(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_reassoc_req(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_disassoc_req(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_disassoc_cnf(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_deauth_req(tpAniSirGlobal, uint32_t *);
static void __lim_process_sme_set_context_req(tpAniSirGlobal, uint32_t *);
static bool __lim_process_sme_stop_bss_req(tpAniSirGlobal, tpSirMsgQ pMsg);
static void lim_process_sme_channel_change_request(tpAniSirGlobal pMac,
						   uint32_t *pMsg);
static void lim_process_sme_start_beacon_req(tpAniSirGlobal pMac, uint32_t *pMsg);
static void lim_process_sme_dfs_csa_ie_request(tpAniSirGlobal pMac, uint32_t *pMsg);
static void lim_process_nss_update_request(tpAniSirGlobal pMac, uint32_t *pMsg);
static void lim_process_set_ie_req(tpAniSirGlobal pMac, uint32_t *pMsg);

static void lim_start_bss_update_add_ie_buffer(tpAniSirGlobal pMac,
					       uint8_t **pDstData_buff,
					       uint16_t *pDstDataLen,
					       uint8_t *pSrcData_buff,
					       uint16_t srcDataLen);

static void lim_update_add_ie_buffer(tpAniSirGlobal pMac,
				     uint8_t **pDstData_buff,
				     uint16_t *pDstDataLen,
				     uint8_t *pSrcData_buff, uint16_t srcDataLen);

static void lim_process_modify_add_ies(tpAniSirGlobal pMac, uint32_t *pMsg);

static void lim_process_update_add_ies(tpAniSirGlobal pMac, uint32_t *pMsg);

extern void pe_register_wma_handle(tpAniSirGlobal pMac);

static void lim_process_ext_change_channel(tpAniSirGlobal mac_ctx,
						uint32_t *msg);


/**
 * lim_process_set_hw_mode() - Send set HW mode command to WMA
 * @mac: Globacl MAC pointer
 * @msg: Message containing the hw mode index
 *
 * Send the set HW mode command to WMA
 *
 * Return: CDF_STATUS_SUCCESS if message posting is successful
 */
static CDF_STATUS lim_process_set_hw_mode(tpAniSirGlobal mac, uint32_t *msg)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cds_msg_t cds_message;
	struct sir_hw_mode *req_msg;
	uint32_t len;
	struct s_sir_set_hw_mode *buf;
	tSirMsgQ resp_msg;
	struct sir_set_hw_mode_resp *param;

	buf = (struct s_sir_set_hw_mode *) msg;
	if (!buf) {
		lim_log(mac, LOGE, FL("Set HW mode param is NULL"));
		/* To free the active command list */
		goto fail;
	}

	len = sizeof(*req_msg);

	req_msg = cdf_mem_malloc(len);
	if (!req_msg) {
		lim_log(mac, LOGE, FL("cdf_mem_malloc failed"));
		/* Free the active command list
		 * Probably the malloc is going to fail there as well?!
		 */
		return CDF_STATUS_E_NOMEM;
	}

	cdf_mem_zero(req_msg, len);

	req_msg->hw_mode_index = buf->set_hw.hw_mode_index;
	req_msg->reason = buf->set_hw.reason;
	/* Other parameters are not needed for WMA */

	cds_message.bodyptr = req_msg;
	cds_message.type    = SIR_HAL_SOC_SET_HW_MODE;

	lim_log(mac, LOG1, FL("Posting SIR_HAL_SOC_SET_HW_MOD to WMA"));
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		lim_log(mac, LOGE,
			FL("vos_mq_post_message failed!(err=%d)"),
			status);
		cdf_mem_free(req_msg);
		goto fail;
	}
	return status;
fail:
	param = cdf_mem_malloc(sizeof(*param));
	if (!param) {
		lim_log(mac, LOGE, FL("HW mode resp failed"));
		return CDF_STATUS_E_FAILURE;
	}
	param->status = SET_HW_MODE_STATUS_ECANCELED;
	param->cfgd_hw_mode_index = 0;
	param->num_vdev_mac_entries = 0;
	resp_msg.type = eWNI_SME_SET_HW_MODE_RESP;
	resp_msg.bodyptr = param;
	resp_msg.bodyval = 0;
	lim_sys_process_mmh_msg_api(mac, &resp_msg, ePROT);
	return CDF_STATUS_SUCCESS;
}

/**
 * lim_process_set_dual_mac_cfg_req() - Set dual mac config command to WMA
 * @mac: Global MAC pointer
 * @msg: Message containing the dual mac config parameter
 *
 * Send the set dual mac config command to WMA
 *
 * Return: CDF_STATUS_SUCCESS if message posting is successful
 */
static CDF_STATUS lim_process_set_dual_mac_cfg_req(tpAniSirGlobal mac,
		uint32_t *msg)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cds_msg_t cds_message;
	struct sir_dual_mac_config *req_msg;
	uint32_t len;
	struct sir_set_dual_mac_cfg *buf;
	tSirMsgQ resp_msg;
	struct sir_dual_mac_config_resp *param;

	buf = (struct sir_set_dual_mac_cfg *) msg;
	if (!buf) {
		lim_log(mac, LOGE, FL("Set Dual mac config is NULL"));
		/* To free the active command list */
		goto fail;
	}

	len = sizeof(*req_msg);

	req_msg = cdf_mem_malloc(len);
	if (!req_msg) {
		lim_log(mac, LOGE, FL("vos_mem_malloc failed"));
		/* Free the active command list
		 * Probably the malloc is going to fail there as well?!
		 */
		return CDF_STATUS_E_NOMEM;
	}

	cdf_mem_zero(req_msg, len);

	req_msg->scan_config = buf->set_dual_mac.scan_config;
	req_msg->fw_mode_config = buf->set_dual_mac.fw_mode_config;
	/* Other parameters are not needed for WMA */

	cds_message.bodyptr = req_msg;
	cds_message.type    = SIR_HAL_SOC_DUAL_MAC_CFG_REQ;

	lim_log(mac, LOG1,
		FL("Post SIR_HAL_SOC_DUAL_MAC_CFG_REQ to WMA: %x %x"),
		req_msg->scan_config, req_msg->fw_mode_config);
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		lim_log(mac, LOGE,
				FL("vos_mq_post_message failed!(err=%d)"),
				status);
		cdf_mem_free(req_msg);
		goto fail;
	}
	return status;
fail:
	param = cdf_mem_malloc(sizeof(*param));
	if (!param) {
		lim_log(mac, LOGE, FL("Dual mac config resp failed"));
		return CDF_STATUS_E_FAILURE;
	}
	param->status = SET_HW_MODE_STATUS_ECANCELED;
	resp_msg.type = eWNI_SME_SET_DUAL_MAC_CFG_RESP;
	resp_msg.bodyptr = param;
	resp_msg.bodyval = 0;
	lim_sys_process_mmh_msg_api(mac, &resp_msg, ePROT);
	return CDF_STATUS_SUCCESS;
}

/**
 * __lim_fresh_scan_reqd() - determine if a fresh scan request must be issued.
 * @mac_ctx: Pointer to Global MAC structure
 * @return_fresh_results: Trigger fresh scan.
 *
 * PE will do fresh scan, if all of the active sessions are in
 * good state (Link Est or BSS Started). If one of the sessions
 * is not in one of the above states, then PE does not do fresh
 * scan. If no session exists (scanning very first time),
 * then PE will always do fresh scan if SME asks it to do that.
 *
 * Return: true for fresh scan results, false if in invalid state.
 */
static uint8_t
__lim_fresh_scan_reqd(tpAniSirGlobal mac_ctx, uint8_t return_fresh_results)
{
	uint8_t valid_state = true;
	int i;

	lim_log(mac_ctx, LOG1, FL("gLimSmeState: %d, returnFreshResults 0x%x"),
		mac_ctx->lim.gLimSmeState, return_fresh_results);

	if (mac_ctx->lim.gLimSmeState != eLIM_SME_IDLE_STATE) {
		lim_log(mac_ctx, LOG1, FL("return FALSE"));
		return false;
	}

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		lim_log(mac_ctx, LOG1,
			FL("session %d, bsstype %d, limSystemRole %d, limSmeState %d"),
			i, mac_ctx->lim.gpSession[i].bssType,
			mac_ctx->lim.gpSession[i].limSystemRole,
			mac_ctx->lim.gpSession[i].limSmeState);
		if (mac_ctx->lim.gpSession[i].valid == true) {
			if (!((((mac_ctx->lim.gpSession[i].bssType ==
					eSIR_INFRASTRUCTURE_MODE) ||
				(mac_ctx->lim.gpSession[i].limSystemRole ==
					eLIM_BT_AMP_STA_ROLE)) &&
				(mac_ctx->lim.gpSession[i].limSmeState ==
					eLIM_SME_LINK_EST_STATE)) ||
			      (((mac_ctx->lim.gpSession[i].bssType ==
					eSIR_IBSS_MODE) ||
				(mac_ctx->lim.gpSession[i].limSystemRole ==
					eLIM_BT_AMP_AP_ROLE) ||
				(mac_ctx->lim.gpSession[i].limSystemRole ==
					eLIM_BT_AMP_STA_ROLE)) &&
			       (mac_ctx->lim.gpSession[i].limSmeState ==
					eLIM_SME_NORMAL_STATE)) ||
			      ((((mac_ctx->lim.gpSession[i].bssType ==
					eSIR_INFRA_AP_MODE) &&
				(mac_ctx->lim.gpSession[i].pePersona ==
					CDF_P2P_GO_MODE)) ||
			      (mac_ctx->lim.gpSession[i].limSystemRole ==
					eLIM_AP_ROLE)) &&
			      (mac_ctx->lim.gpSession[i].limSmeState ==
					eLIM_SME_NORMAL_STATE)))) {
				valid_state = false;
				break;
			}
		}
	}

	lim_log(mac_ctx, LOG1, FL("valid_state: %d"), valid_state);

	if ((valid_state) &&
	    (return_fresh_results & SIR_BG_SCAN_RETURN_FRESH_RESULTS))
		return true;
	else
		return false;
}

/**
 * __lim_is_sme_assoc_cnf_valid()
 *
 ***FUNCTION:
 * This function is called by __lim_process_sme_assoc_cnf_new() upon
 * receiving SME_ASSOC_CNF.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMeasReq  Pointer to Received ASSOC_CNF message
 * @return true      When received SME_ASSOC_CNF is formatted
 *                   correctly
 *         false     otherwise
 */

static inline uint8_t __lim_is_sme_assoc_cnf_valid(tpSirSmeAssocCnf pAssocCnf)
{
	if (lim_is_group_addr(pAssocCnf->peerMacAddr))
		return false;
	else
		return true;
} /*** end __lim_is_sme_assoc_cnf_valid() ***/

/**
 * __lim_get_sme_join_req_size_for_alloc()
 *
 ***FUNCTION:
 * This function is called in various places to get IE length
 * from tSirBssDescription structure
 * number being scanned.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param     pBssDescr
 * @return    Total IE length
 */

static uint16_t __lim_get_sme_join_req_size_for_alloc(uint8_t *pBuf)
{
	uint16_t len = 0;

	if (!pBuf)
		return len;

	pBuf += sizeof(uint16_t);
	len = lim_get_u16(pBuf);
	return len + sizeof(uint16_t);
}

/**
 * __lim_is_defered_msg_for_learn() - message handling in SME learn state
 * @pMac: Global MAC context
 * @pMsg: Pointer to message posted from SME to LIM.
 *
 * Has role only if 11h is enabled. Not used on STA side.
 * Defers the message if SME is in learn state and brings
 * the LIM back to normal mode.
 *
 * Return: true - If defered false - Otherwise
 */

static bool __lim_is_defered_msg_for_learn(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	if (lim_is_system_in_scan_state(pMac)) {
		if (lim_defer_msg(pMac, pMsg) != TX_SUCCESS) {
			lim_log(pMac, LOGE, FL("Could not defer Msg = %d"),
				       pMsg->type);
			return false;
		}
		lim_log(pMac, LOG1,
			FL("Defer the message, in learn mode type = %d"),
			pMsg->type);
		return true;
	}
	return false;
}

/**
 * __lim_is_defered_msg_for_radar() - Defers the message if radar is detected
 * @mac_ctx: Pointer to Global MAC structure
 * @message: Pointer to message posted from SME to LIM.
 *
 * Has role only if 11h is enabled. Not used on STA side.
 * Defers the message if radar is detected.
 *
 * Return: true, if defered otherwise return false.
 */
static bool
__lim_is_defered_msg_for_radar(tpAniSirGlobal mac_ctx, tpSirMsgQ message)
{
	/*
	 * fRadarDetCurOperChan will be set only if we
	 * detect radar in current operating channel and
	 * System Role == AP ROLE
	 *
	 * TODO: Need to take care radar detection.
	 *
	 * if (LIM_IS_RADAR_DETECTED(mac_ctx))
	 */
	if (0) {
		if (lim_defer_msg(mac_ctx, message) != TX_SUCCESS) {
			lim_log(mac_ctx, LOGE, FL("Could not defer Msg = %d"),
				message->type);
			return false;
		}
		lim_log(mac_ctx, LOG1,
			FL("Defer the message, in learn mode type = %d"),
			message->type);
		return true;
	}
	return false;
}

/**
 * __lim_process_sme_sys_ready_ind () - Process ready indication from WMA
 * @pMac: Global MAC context
 * @pMsgBuf: Message from WMA
 *
 * handles the notification from HDD. PE just forwards this message to HAL.
 *
 * Return: true-Posting to HAL failed, so PE will consume the buffer.
 *         false-Posting to HAL successful, so HAL will consume the buffer.
 */

static bool __lim_process_sme_sys_ready_ind(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirMsgQ msg;
	tSirSmeReadyReq *ready_req = (tSirSmeReadyReq *) pMsgBuf;

	msg.type = WMA_SYS_READY_IND;
	msg.reserved = 0;
	msg.bodyptr = pMsgBuf;
	msg.bodyval = 0;

	if (ANI_DRIVER_TYPE(pMac) != eDRIVER_TYPE_MFG) {
		pe_register_wma_handle(pMac);
		pMac->lim.add_bssdescr_callback = ready_req->add_bssdescr_cb;
	}
	PELOGW(lim_log(pMac, LOGW, FL("sending WMA_SYS_READY_IND msg to HAL"));)
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msg.type));

	if (eSIR_SUCCESS != wma_post_ctrl_msg(pMac, &msg)) {
		lim_log(pMac, LOGP, FL("wma_post_ctrl_msg failed"));
		return true;
	}
	return false;
}

#ifdef WLAN_FEATURE_11AC

uint32_t lim_get_center_channel(tpAniSirGlobal pMac, uint8_t primarychanNum,
				ePhyChanBondState secondaryChanOffset,
				uint8_t chanWidth)
{
	if (chanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) {
		switch (secondaryChanOffset) {
		case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
			return primarychanNum;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
			return primarychanNum + 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
			return primarychanNum - 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
			return primarychanNum + 6;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
			return primarychanNum + 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
			return primarychanNum - 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
			return primarychanNum - 6;
		default:
			return eSIR_CFG_INVALID_ID;
		}
	} else if (chanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ) {
		switch (secondaryChanOffset) {
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
			return primarychanNum + 2;
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			return primarychanNum - 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
			return primarychanNum;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
			return primarychanNum + 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
			return primarychanNum - 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
			return primarychanNum + 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
			return primarychanNum - 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
			return primarychanNum + 2;
		case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
			return primarychanNum - 2;
		default:
			return eSIR_CFG_INVALID_ID;
		}
	}
	return primarychanNum;
}

#endif

/**
 *lim_configure_ap_start_bss_session() - Configure the AP Start BSS in session.
 *@mac_ctx: Pointer to Global MAC structure
 *@session: A pointer to session entry
 *@sme_start_bss_req: Start BSS Request from upper layers.
 *
 * This function is used to configure the start bss parameters
 * in to the session.
 *
 * Return: None.
 */
static void
lim_configure_ap_start_bss_session(tpAniSirGlobal mac_ctx, tpPESession session,
			tpSirSmeStartBssReq sme_start_bss_req)
{
	session->limSystemRole = eLIM_AP_ROLE;
	session->privacy = sme_start_bss_req->privacy;
	session->fwdWPSPBCProbeReq = sme_start_bss_req->fwdWPSPBCProbeReq;
	session->authType = sme_start_bss_req->authType;
	/* Store the DTIM period */
	session->dtimPeriod = (uint8_t) sme_start_bss_req->dtimPeriod;
	/* Enable/disable UAPSD */
	session->apUapsdEnable = sme_start_bss_req->apUapsdEnable;
	if (session->pePersona == CDF_P2P_GO_MODE) {
		session->proxyProbeRspEn = 0;
	} else {
		/*
		 * To detect PBC overlap in SAP WPS mode,
		 * Host handles Probe Requests.
		 */
		if (SAP_WPS_DISABLED == sme_start_bss_req->wps_state)
			session->proxyProbeRspEn = 1;
		else
			session->proxyProbeRspEn = 0;
	}
	session->ssidHidden = sme_start_bss_req->ssidHidden;
	session->wps_state = sme_start_bss_req->wps_state;
	session->sap_dot11mc = sme_start_bss_req->sap_dot11mc;
	lim_get_short_slot_from_phy_mode(mac_ctx, session, session->gLimPhyMode,
		&session->shortSlotTimeSupported);
	session->isCoalesingInIBSSAllowed =
		sme_start_bss_req->isCoalesingInIBSSAllowed;

}

/**
 * __lim_handle_sme_start_bss_request() - process SME_START_BSS_REQ message
 *@mac_ctx: Pointer to Global MAC structure
 *@msg_buf: A pointer to the SME message buffer
 *
 * This function is called to process SME_START_BSS_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */
static void
__lim_handle_sme_start_bss_request(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	uint16_t size;
	uint32_t val = 0;
	tSirRetStatus ret_status;
	tSirMacChanNum channel_number;
	tLimMlmStartReq *mlm_start_req = NULL;
	tpSirSmeStartBssReq sme_start_bss_req = NULL;
	tSirResultCodes ret_code = eSIR_SME_SUCCESS;
	/* Flag Used in case of IBSS to Auto generate BSSID. */
	uint32_t auto_gen_bssid = false;
	uint8_t session_id;
	tpPESession session = NULL;
	uint8_t sme_session_id = 0;
	uint16_t sme_transaction_id = 0;
	uint32_t chanwidth;
	tSirRetStatus cfg_get_wmi_dfs_master_param = eSIR_SUCCESS;

/* FEATURE_WLAN_DIAG_SUPPORT */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	/*
	 * Since the session is not created yet, sending NULL.
	 * The response should have the correct state.
	 */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_START_BSS_REQ_EVENT,
			      NULL, 0, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_log(mac_ctx, LOG1, FL("Received START_BSS_REQ"));

	/*
	 * Global Sme state and mlm states are not defined yet,
	 * for BT-AMP Suppoprt . TO BE DONE
	 */
	if ((mac_ctx->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) ||
	    (mac_ctx->lim.gLimSmeState == eLIM_SME_IDLE_STATE)) {
		size = sizeof(tSirSmeStartBssReq);

		sme_start_bss_req = cdf_mem_malloc(size);
		if (NULL == sme_start_bss_req) {
			lim_log(mac_ctx, LOGE,
				FL("Allocate Memory fail for LimStartBssReq"));
			/* Send failure response to host */
			ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
			goto end;
		}

		cdf_mem_set((void *)sme_start_bss_req, size, 0);
		cdf_mem_copy(sme_start_bss_req, msg_buf,
			sizeof(tSirSmeStartBssReq));
		if (!lim_is_sme_start_bss_req_valid(mac_ctx,
					sme_start_bss_req)) {
			lim_log(mac_ctx, LOGW,
				FL("Received invalid eWNI_SME_START_BSS_REQ"));
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			goto free;
		}

		/*
		 * This is the place where PE is going to create a session.
		 * If session is not existed, then create a new session
		 */
		session = pe_find_session_by_bssid(mac_ctx,
				sme_start_bss_req->bssId, &session_id);
		if (session != NULL) {
			lim_log(mac_ctx, LOGW,
				FL("Session Already exists for given BSSID"));
			ret_code = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
			session = NULL;
			goto free;
		} else {
			session = pe_create_session(mac_ctx,
					sme_start_bss_req->bssId,
					&session_id, mac_ctx->lim.maxStation,
					sme_start_bss_req->bssType);
			if (session == NULL) {
				lim_log(mac_ctx, LOGW,
					FL("Session Can not be created "));
				ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
				goto free;
			}
		}

		/* Probe resp add ie */
		lim_start_bss_update_add_ie_buffer(mac_ctx,
			&session->addIeParams.probeRespData_buff,
			&session->addIeParams.probeRespDataLen,
			sme_start_bss_req->addIeParams.probeRespData_buff,
			sme_start_bss_req->addIeParams.probeRespDataLen);

		/* Probe Beacon add ie */
		lim_start_bss_update_add_ie_buffer(mac_ctx,
			&session->addIeParams.probeRespBCNData_buff,
			&session->addIeParams.probeRespBCNDataLen,
			sme_start_bss_req->addIeParams.probeRespBCNData_buff,
			sme_start_bss_req->addIeParams.probeRespBCNDataLen);

		/* Assoc resp IE */
		lim_start_bss_update_add_ie_buffer(mac_ctx,
			&session->addIeParams.assocRespData_buff,
			&session->addIeParams.assocRespDataLen,
			sme_start_bss_req->addIeParams.assocRespData_buff,
			sme_start_bss_req->addIeParams.assocRespDataLen);

		/* Store the session related params in newly created session */
		session->pLimStartBssReq = sme_start_bss_req;

		/* Store PE session_id in session Table  */
		session->peSessionId = session_id;

		/* Store SME session Id in sessionTable */
		session->smeSessionId = sme_start_bss_req->sessionId;

		session->transactionId = sme_start_bss_req->transactionId;

		cdf_mem_copy(&(session->htConfig),
			     &(sme_start_bss_req->htConfig),
			     sizeof(session->htConfig));

		sir_copy_mac_addr(session->selfMacAddr,
				  sme_start_bss_req->selfMacAddr);

		/* Copy SSID to session table */
		cdf_mem_copy((uint8_t *) &session->ssId,
			     (uint8_t *) &sme_start_bss_req->ssId,
			     (sme_start_bss_req->ssId.length + 1));

		session->bssType = sme_start_bss_req->bssType;

		session->nwType = sme_start_bss_req->nwType;

		session->beaconParams.beaconInterval =
			sme_start_bss_req->beaconInterval;

		/* Store the channel number in session Table */
		session->currentOperChannel =
			sme_start_bss_req->channelId;

		/* Store Persona */
		session->pePersona = sme_start_bss_req->bssPersona;
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
			  FL("PE PERSONA=%d"), session->pePersona);

		/* Update the phymode */
		session->gLimPhyMode = sme_start_bss_req->nwType;

		session->maxTxPower =
			cfg_get_regulatory_max_transmit_power(mac_ctx,
				session->currentOperChannel);
		/* Store the dot 11 mode in to the session Table */
		session->dot11mode = sme_start_bss_req->dot11mode;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		session->cc_switch_mode =
			sme_start_bss_req->cc_switch_mode;
#endif
		session->htCapability =
			IS_DOT11_MODE_HT(session->dot11mode);
		session->vhtCapability =
			IS_DOT11_MODE_VHT(session->dot11mode);
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
			  FL("*****session->vhtCapability = %d"),
			  session->vhtCapability);
		session->txLdpcIniFeatureEnabled =
			sme_start_bss_req->txLdpcIniFeatureEnabled;

		if (mac_ctx->roam.configParam.enable2x2)
			session->nss = 2;
		else
			session->nss = 1;
#ifdef WLAN_FEATURE_11W
		session->limRmfEnabled =
			sme_start_bss_req->pmfCapable ? 1 : 0;
		lim_log(mac_ctx, LOG1, FL("Session RMF enabled: %d"),
			session->limRmfEnabled);
#endif

		cdf_mem_copy((void *)&session->rateSet,
			     (void *)&sme_start_bss_req->operationalRateSet,
			     sizeof(tSirMacRateSet));
		cdf_mem_copy((void *)&session->extRateSet,
			     (void *)&sme_start_bss_req->extendedRateSet,
			     sizeof(tSirMacRateSet));

		switch (sme_start_bss_req->bssType) {
		case eSIR_INFRA_AP_MODE:
			lim_configure_ap_start_bss_session(mac_ctx, session,
				sme_start_bss_req);
			break;
		case eSIR_IBSS_MODE:
			session->limSystemRole = eLIM_STA_IN_IBSS_ROLE;
			lim_get_short_slot_from_phy_mode(mac_ctx, session,
				session->gLimPhyMode,
				&session->shortSlotTimeSupported);

			/*
			 * initialize to "OPEN".
			 * will be updated upon key installation
			 */
			session->encryptType = eSIR_ED_NONE;

			break;

		case eSIR_BTAMP_AP_MODE:
			session->limSystemRole = eLIM_BT_AMP_AP_ROLE;
			break;

		case eSIR_BTAMP_STA_MODE:
			session->limSystemRole = eLIM_BT_AMP_STA_ROLE;
			break;

		/*
		 * There is one more mode called auto mode.
		 * which is used no where
		 */

		/* FORBUILD -TEMPFIX.. HOW TO use AUTO MODE????? */

		default:
			/* not used anywhere...used in scan function */
			break;
		}

		/*
		 * BT-AMP: Allocate memory for the array of
		 * parsed (Re)Assoc request structure
		 */
		if ((sme_start_bss_req->bssType == eSIR_BTAMP_AP_MODE) ||
		    (sme_start_bss_req->bssType == eSIR_INFRA_AP_MODE)) {
			session->parsedAssocReq =
				cdf_mem_malloc(session->dph.dphHashTable.
					       size * sizeof(tpSirAssocReq));
			if (NULL == session->parsedAssocReq) {
				lim_log(mac_ctx, LOGW,
					FL("AllocateMemory() failed"));
				ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
				goto free;
			}
			cdf_mem_set(session->parsedAssocReq,
				(session->dph.dphHashTable.size *
				sizeof(tpSirAssocReq)), 0);
		}

		if (!sme_start_bss_req->channelId) {
			lim_log(mac_ctx, LOGE,
				FL("Received invalid eWNI_SME_START_BSS_REQ"));
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			goto free;
		}
		channel_number = sme_start_bss_req->channelId;
#ifdef QCA_HT_2040_COEX
		if (sme_start_bss_req->obssEnabled)
			session->htSupportedChannelWidthSet =
				session->htCapability;
		else
#endif
		session->htSupportedChannelWidthSet =
			(sme_start_bss_req->sec_ch_offset) ? 1 : 0;
		session->htSecondaryChannelOffset =
			sme_start_bss_req->sec_ch_offset;
		session->htRecommendedTxWidthSet =
			(session->htSecondaryChannelOffset) ? 1 : 0;
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
			  FL("cbMode %u"), sme_start_bss_req->cbMode);
		if (session->vhtCapability || session->htCapability) {
			chanwidth = sme_start_bss_req->vht_channel_width;
			lim_log(mac_ctx, LOG1, FL("vht_channel_width %u"),
					sme_start_bss_req->vht_channel_width);
			if (channel_number <= RF_CHAN_14 &&
					chanwidth != eHT_CHANNEL_WIDTH_20MHZ) {
				chanwidth = CH_WIDTH_20MHZ;
				session->htSupportedChannelWidthSet = 0;
				lim_log(mac_ctx, LOG1,
					FL("Set chanwidth to 20Mhz, chan %d"),
					channel_number);
			}
			session->ch_width = chanwidth;
			if (session->htSupportedChannelWidthSet) {
				session->ch_center_freq_seg0 =
					sme_start_bss_req->center_freq_seg0;
				session->ch_center_freq_seg1 =
					sme_start_bss_req->center_freq_seg1;
			} else {
				session->ch_center_freq_seg0 = 0;
				session->ch_center_freq_seg1 = 0;
			}
		}

		if (session->vhtCapability &&
				(CH_WIDTH_160MHZ > session->ch_width)) {
			if (wlan_cfg_get_int(mac_ctx,
					WNI_CFG_VHT_SU_BEAMFORMER_CAP, &val) !=
				eSIR_SUCCESS)
				lim_log(mac_ctx, LOGE, FL(
					"cfg get vht su bformer failed"));

			session->enable_su_tx_bformer = val;
		} else {
			session->nss = 1;
		}
		lim_log(mac_ctx, LOG1, FL("vht su tx bformer %d"), val);

		/* Delete pre-auth list if any */
		lim_delete_pre_auth_list(mac_ctx);

		/*
		 * keep the RSN/WPA IE information in PE Session Entry
		 * later will be using this to check when received (Re)Assoc req
		 */
		lim_set_rs_nie_wp_aiefrom_sme_start_bss_req_message(mac_ctx,
				&sme_start_bss_req->rsnIE, session);

		if (LIM_IS_AP_ROLE(session) || LIM_IS_IBSS_ROLE(session)) {
			session->gLimProtectionControl =
				sme_start_bss_req->protEnabled;
			/*
			 * each byte will have the following info
			 * bit7       bit6    bit5   bit4 bit3   bit2  bit1 bit0
			 * reserved reserved   RIFS   Lsig n-GF   ht20  11g  11b
			 */
			cdf_mem_copy((void *)&session->cfgProtection,
				     (void *)&sme_start_bss_req->ht_capab,
				     sizeof(uint16_t));
			/* Initialize WPS PBC session link list */
			session->pAPWPSPBCSession = NULL;
		}
		/* Prepare and Issue LIM_MLM_START_REQ to MLM */
		mlm_start_req = cdf_mem_malloc(sizeof(tLimMlmStartReq));
		if (NULL == mlm_start_req) {
			lim_log(mac_ctx, LOGP,
				FL("Allocate Memory failed for mlmStartReq"));
			ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
			goto free;
		}

		cdf_mem_set((void *)mlm_start_req, sizeof(tLimMlmStartReq), 0);

		/* Copy SSID to the MLM start structure */
		cdf_mem_copy((uint8_t *) &mlm_start_req->ssId,
			     (uint8_t *) &sme_start_bss_req->ssId,
			     sme_start_bss_req->ssId.length + 1);
		mlm_start_req->ssidHidden = sme_start_bss_req->ssidHidden;
		mlm_start_req->obssProtEnabled =
			sme_start_bss_req->obssProtEnabled;

		mlm_start_req->bssType = session->bssType;

		/* Fill PE session Id from the session Table */
		mlm_start_req->sessionId = session->peSessionId;

		if ((mlm_start_req->bssType == eSIR_BTAMP_STA_MODE) ||
		    (mlm_start_req->bssType == eSIR_BTAMP_AP_MODE) ||
		    (mlm_start_req->bssType == eSIR_INFRA_AP_MODE)) {
			/*
			 * Copy the BSSId from sessionTable to
			 * mlmStartReq struct
			 */
			sir_copy_mac_addr(mlm_start_req->bssId, session->bssId);
		} else {
			/* ibss mode */
			mac_ctx->lim.gLimIbssCoalescingHappened = false;

			ret_status = wlan_cfg_get_int(mac_ctx,
					WNI_CFG_IBSS_AUTO_BSSID,
					&auto_gen_bssid);
			if (ret_status != eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGP,
					FL("Get Auto Gen BSSID fail,Status=%d"),
					ret_status);
				ret_code = eSIR_LOGP_EXCEPTION;
				goto free;
			}

			if (!auto_gen_bssid) {
				/*
				 * We're not auto generating BSSID.
				 * Instead, get it from session entry
				 */
				sir_copy_mac_addr(mlm_start_req->bssId,
						  session->bssId);
				/*
				 * Start IBSS group BSSID
				 * Auto Generating BSSID.
				 */
				auto_gen_bssid = ((mlm_start_req->bssId[0] &
							0x01) ? true : false);
			}

			if (auto_gen_bssid) {
				/*
				 * if BSSID is not any uc id.
				 * then use locally generated BSSID.
				 * Autogenerate the BSSID
				 */
				lim_get_random_bssid(mac_ctx,
						mlm_start_req->bssId);
				mlm_start_req->bssId[0] = 0x02;

				/*
				 * Copy randomly generated BSSID
				 * to the session Table
				 */
				sir_copy_mac_addr(session->bssId,
						  mlm_start_req->bssId);
			}
		}
		/* store the channel num in mlmstart req structure */
		mlm_start_req->channelNumber = session->currentOperChannel;
		mlm_start_req->cbMode = sme_start_bss_req->cbMode;
		mlm_start_req->beaconPeriod =
			session->beaconParams.beaconInterval;

		if (LIM_IS_AP_ROLE(session)) {
			mlm_start_req->dtimPeriod = session->dtimPeriod;
			mlm_start_req->wps_state = session->wps_state;

		} else {
			if (wlan_cfg_get_int(mac_ctx,
				WNI_CFG_DTIM_PERIOD, &val) != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGP,
					FL("could not retrieve DTIM Period"));
			mlm_start_req->dtimPeriod = (uint8_t) val;
		}

		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_CFP_PERIOD, &val) !=
			eSIR_SUCCESS)
			lim_log(mac_ctx, LOGP,
				FL("could not retrieve Beacon interval"));
		mlm_start_req->cfParamSet.cfpPeriod = (uint8_t) val;

		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_CFP_MAX_DURATION, &val) !=
			eSIR_SUCCESS)
			lim_log(mac_ctx, LOGP,
				FL("could not retrieve CFPMaxDuration"));
		mlm_start_req->cfParamSet.cfpMaxDuration = (uint16_t) val;

		/*
		 * this may not be needed anymore now,
		 * as rateSet is now included in the
		 * session entry and MLM has session context.
		 */
		cdf_mem_copy((void *)&mlm_start_req->rateSet,
			     (void *)&session->rateSet,
			     sizeof(tSirMacRateSet));

		/* Now populate the 11n related parameters */
		mlm_start_req->nwType = session->nwType;
		mlm_start_req->htCapable = session->htCapability;

		mlm_start_req->htOperMode = mac_ctx->lim.gHTOperMode;
		/* Unused */
		mlm_start_req->dualCTSProtection =
			mac_ctx->lim.gHTDualCTSProtection;
		mlm_start_req->txChannelWidthSet =
			session->htRecommendedTxWidthSet;

		session->limRFBand = lim_get_rf_band(channel_number);

		/* Initialize 11h Enable Flag */
		session->lim11hEnable = 0;
		if ((mlm_start_req->bssType != eSIR_IBSS_MODE) &&
		    (SIR_BAND_5_GHZ == session->limRFBand)) {
			if (wlan_cfg_get_int(mac_ctx,
				WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGP,
					FL("Fail to get WNI_CFG_11H_ENABLED "));
			else
				session->lim11hEnable = val;

			if (session->lim11hEnable &&
				(eSIR_INFRA_AP_MODE ==
					mlm_start_req->bssType)) {
				cfg_get_wmi_dfs_master_param =
					wlan_cfg_get_int(mac_ctx,
						WNI_CFG_DFS_MASTER_ENABLED,
						&val);
				session->lim11hEnable = val;
			}
			if (cfg_get_wmi_dfs_master_param != eSIR_SUCCESS)
				/* Failed get CFG WNI_CFG_DFS_MASTER_ENABLED */
				lim_log(mac_ctx, LOGE,
					FL("Get Fail, CFG DFS ENABLE"));
		}

		if (!session->lim11hEnable) {
			if (cfg_set_int(mac_ctx,
				WNI_CFG_LOCAL_POWER_CONSTRAINT, 0) !=
				eSIR_SUCCESS)
				/*
				 * Failed to set the CFG param
				 * WNI_CFG_LOCAL_POWER_CONSTRAINT
				 */
				lim_log(mac_ctx, LOGE,
				    FL("Set LOCAL_POWER_CONSTRAINT failed"));
		}

		session->limPrevSmeState = session->limSmeState;
		session->limSmeState = eLIM_SME_WT_START_BSS_STATE;
		MTRACE(mac_trace
			(mac_ctx, TRACE_CODE_SME_STATE,
			session->peSessionId,
			session->limSmeState));

		lim_post_mlm_message(mac_ctx, LIM_MLM_START_REQ,
			(uint32_t *) mlm_start_req);
		return;
	} else {

		lim_log(mac_ctx, LOGE,
			FL("Received unexpected START_BSS_REQ, in state %X"),
			mac_ctx->lim.gLimSmeState);
		ret_code = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
		goto end;
	} /* if (mac_ctx->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) */

free:
	if ((session != NULL) &&
		(session->pLimStartBssReq == sme_start_bss_req)) {
		session->pLimStartBssReq = NULL;
	}
	cdf_mem_free(sme_start_bss_req);
	cdf_mem_free(mlm_start_req);

end:
	if (sme_start_bss_req != NULL) {
		sme_session_id = sme_start_bss_req->sessionId;
		sme_transaction_id = sme_start_bss_req->transactionId;
	}
	if (NULL != session) {
		pe_delete_session(mac_ctx, session);
		session = NULL;
	}
	lim_send_sme_start_bss_rsp(mac_ctx, eWNI_SME_START_BSS_RSP, ret_code,
		session, sme_session_id, sme_transaction_id);
}

/**
 * __lim_process_sme_start_bss_req() - Call handler to start BSS
 *
 * @pMac: Global MAC context
 * @pMsg: Message pointer
 *
 * Wrapper for the function __lim_handle_sme_start_bss_request
 * This message will be defered until softmac come out of
 * scan mode or if we have detected radar on the current
 * operating channel.
 *
 * return true - If we consumed the buffer
 *        false - If have defered the message.
 */
static bool __lim_process_sme_start_bss_req(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	if (__lim_is_defered_msg_for_learn(pMac, pMsg) ||
	    __lim_is_defered_msg_for_radar(pMac, pMsg)) {
		/**
		 * If message defered, buffer is not consumed yet.
		 * So return false
		 */
		return false;
	}

	__lim_handle_sme_start_bss_request(pMac, (uint32_t *) pMsg->bodyptr);
	return true;
}

/**
 *  lim_get_random_bssid()
 *
 *  FUNCTION:This function is called to process generate the random number for bssid
 *  This function is called to process SME_SCAN_REQ message
 *  from HDD or upper layer application.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 * 1. geneartes the unique random number for bssid in ibss
 *
 *  @param  pMac      Pointer to Global MAC structure
 *  @param  *data      Pointer to  bssid  buffer
 *  @return None
 */
void lim_get_random_bssid(tpAniSirGlobal pMac, uint8_t *data)
{
	uint32_t random[2];
	random[0] = tx_time_get();
	random[0] |= (random[0] << 15);
	random[1] = random[0] >> 1;
	cdf_mem_copy(data, (uint8_t *) random, sizeof(tSirMacAddr));
}

static CDF_STATUS lim_send_hal_start_scan_offload_req(tpAniSirGlobal pMac,
						      tpSirSmeScanReq pScanReq)
{
	tSirScanOffloadReq *pScanOffloadReq;
	uint8_t *p;
	uint8_t *ht_cap_ie;
	tSirMsgQ msg;
	uint16_t i, len;
	uint16_t ht_cap_len = 0, addn_ie_len = 0;
#ifdef WLAN_FEATURE_11AC
	uint8_t *vht_cap_ie;
	uint16_t vht_cap_len = 0;
#endif /* WLAN_FEATURE_11AC */
	tSirRetStatus status, rc = eSIR_SUCCESS;
	tDot11fIEExtCap extracted_extcap = {0};
	bool extcap_present = true;

	if (pScanReq->uIEFieldLen) {
		status = lim_strip_extcap_update_struct(pMac,
			     (uint8_t *) pScanReq + pScanReq->uIEFieldOffset,
			     &pScanReq->uIEFieldLen, &extracted_extcap);

		if (eSIR_SUCCESS != status) {
			extcap_present = false;
			lim_log(pMac, LOG1,
			   FL("Unable to Strip ExtCap IE from Scan Req"));
		}

		if (extcap_present) {
			lim_log(pMac, LOG1,
			   FL("Extcap was part of SCAN IE - Updating FW"));
			lim_send_ext_cap_ie(pMac, pScanReq->sessionId,
					    &extracted_extcap, true);
		}
	} else {
	    lim_log(pMac, LOG1,
		    FL("No IEs in the scan request from supplicant"));
	}

	/**
	 * The tSirScanOffloadReq will reserve the space for first channel,
	 * so allocate the memory for (numChannels - 1) and uIEFieldLen
	 */
	len = sizeof(tSirScanOffloadReq) +
		(pScanReq->channelList.numChannels - 1) + pScanReq->uIEFieldLen;

	if (IS_DOT11_MODE_HT(pScanReq->dot11mode)) {
		lim_log(pMac, LOG1,
			FL("Adding HT Caps IE since dot11mode=%d"),
			pScanReq->dot11mode);
		/* 2 bytes for EID and Length */
		ht_cap_len = 2 + sizeof(tHtCaps);
		len += ht_cap_len;
		addn_ie_len += ht_cap_len;
	}

#ifdef WLAN_FEATURE_11AC
	if (IS_DOT11_MODE_VHT(pScanReq->dot11mode)) {
		lim_log(pMac, LOG1,
			FL("Adding VHT Caps IE since dot11mode=%d"),
			pScanReq->dot11mode);
		/* 2 bytes for EID and Length */
		vht_cap_len = 2 + sizeof(tSirMacVHTCapabilityInfo) +
				  sizeof(tSirVhtMcsInfo);
		len += vht_cap_len;
		addn_ie_len += vht_cap_len;
	}
#endif /* WLAN_FEATURE_11AC */

	pScanOffloadReq = cdf_mem_malloc(len);
	if (NULL == pScanOffloadReq) {
		lim_log(pMac, LOGE,
			FL("AllocateMemory failed for pScanOffloadReq"));
		return CDF_STATUS_E_NOMEM;
	}

	cdf_mem_set((uint8_t *) pScanOffloadReq, len, 0);

	msg.type = WMA_START_SCAN_OFFLOAD_REQ;
	msg.bodyptr = pScanOffloadReq;
	msg.bodyval = 0;

	cdf_copy_macaddr(&pScanOffloadReq->bssId, &pScanReq->bssId);

	if (pScanReq->numSsid > SIR_SCAN_MAX_NUM_SSID) {
		lim_log(pMac, LOGE,
			FL("Invalid value (%d) for numSsid"),
			SIR_SCAN_MAX_NUM_SSID);
		cdf_mem_free(pScanOffloadReq);
		return CDF_STATUS_E_FAILURE;
	}

	pScanOffloadReq->numSsid = pScanReq->numSsid;
	for (i = 0; i < pScanOffloadReq->numSsid; i++) {
		pScanOffloadReq->ssId[i].length = pScanReq->ssId[i].length;
		cdf_mem_copy((uint8_t *) pScanOffloadReq->ssId[i].ssId,
			     (uint8_t *) pScanReq->ssId[i].ssId,
			     pScanOffloadReq->ssId[i].length);
	}

	pScanOffloadReq->hiddenSsid = pScanReq->hiddenSsid;
	cdf_copy_macaddr(&pScanOffloadReq->selfMacAddr, &pScanReq->selfMacAddr);
	pScanOffloadReq->bssType = pScanReq->bssType;
	pScanOffloadReq->dot11mode = pScanReq->dot11mode;
	pScanOffloadReq->scanType = pScanReq->scanType;
	pScanOffloadReq->minChannelTime = pScanReq->minChannelTime;
	pScanOffloadReq->maxChannelTime = pScanReq->maxChannelTime;
	pScanOffloadReq->restTime = pScanReq->restTime;

	/* for normal scan, the value for p2pScanType should be 0
	   always */
	if (pScanReq->p2pSearch)
		pScanOffloadReq->p2pScanType = P2P_SCAN_TYPE_SEARCH;

	pScanOffloadReq->sessionId = pScanReq->sessionId;
	pScanOffloadReq->scan_id = pScanReq->scan_id;

	if (pScanOffloadReq->sessionId >= pMac->lim.maxBssId)
		lim_log(pMac, LOGE, FL("Invalid pe sessionID : %d"),
			pScanOffloadReq->sessionId);

	pScanOffloadReq->channelList.numChannels =
		pScanReq->channelList.numChannels;
	p = &(pScanOffloadReq->channelList.channelNumber[0]);
	for (i = 0; i < pScanOffloadReq->channelList.numChannels; i++)
		p[i] = pScanReq->channelList.channelNumber[i];

	pScanOffloadReq->uIEFieldLen = pScanReq->uIEFieldLen;
	pScanOffloadReq->uIEFieldOffset = len - addn_ie_len -
						pScanOffloadReq->uIEFieldLen;
	cdf_mem_copy((uint8_t *) pScanOffloadReq +
		     pScanOffloadReq->uIEFieldOffset,
		     (uint8_t *) pScanReq + pScanReq->uIEFieldOffset,
		     pScanReq->uIEFieldLen);

	/* Copy HT Capability info if dot11mode is HT */
	if (IS_DOT11_MODE_HT(pScanReq->dot11mode)) {
		/* Populate EID and Length field here */
		ht_cap_ie = (uint8_t *) pScanOffloadReq +
					pScanOffloadReq->uIEFieldOffset +
					pScanOffloadReq->uIEFieldLen;
		cdf_mem_set(ht_cap_ie, ht_cap_len, 0);
		*ht_cap_ie = SIR_MAC_HT_CAPABILITIES_EID;
		*(ht_cap_ie + 1) =  ht_cap_len - 2;
		lim_set_ht_caps(pMac, NULL, ht_cap_ie, ht_cap_len);
		pScanOffloadReq->uIEFieldLen += ht_cap_len;
	}

#ifdef WLAN_FEATURE_11AC
	/* Copy VHT Capability info if dot11mode is VHT Capable */
	if (IS_DOT11_MODE_VHT(pScanReq->dot11mode)) {
		/* Populate EID and Length field here */
		vht_cap_ie = (uint8_t *) pScanOffloadReq +
					 pScanOffloadReq->uIEFieldOffset +
					 pScanOffloadReq->uIEFieldLen;
		cdf_mem_set(vht_cap_ie, vht_cap_len, 0);
		*vht_cap_ie = SIR_MAC_VHT_CAPABILITIES_EID;
		*(vht_cap_ie + 1) =  vht_cap_len - 2;
		lim_set_vht_caps(pMac, NULL, vht_cap_ie, vht_cap_len);
		pScanOffloadReq->uIEFieldLen += vht_cap_len;
	}
#endif /* WLAN_FEATURE_11AC */

	rc = wma_post_ctrl_msg(pMac, &msg);
	if (rc != eSIR_SUCCESS) {
		lim_log(pMac, LOGE, FL("wma_post_ctrl_msg() return failure"));
		cdf_mem_free(pScanOffloadReq);
		return CDF_STATUS_E_FAILURE;
	}

	lim_log(pMac, LOG1, FL("Processed Offload Scan Request Successfully"));

	return CDF_STATUS_SUCCESS;
}

/**
 * __lim_process_sme_scan_req() - Process the SME Scan Request
 * @mac_ctx: Global MAC Context
 * @msg_buf: Buffer which contains the request and pertinent parameters
 *
 * This function is called to process SME_SCAN_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */

static void __lim_process_sme_scan_req(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tpSirSmeScanReq scan_req;
	uint8_t valid_req = 0;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_SCAN_REQ_EVENT, NULL,
			      eSIR_SUCCESS, eSIR_SUCCESS);
#endif

	scan_req = (tpSirSmeScanReq) msg_buf;
	lim_log(mac_ctx, LOG1,
		FL("SME SCAN REQ id %d numChan %d min %d max %d IELen %d first %d fresh %d unique %d type %d rsp %d"),
		scan_req->scan_id, scan_req->channelList.numChannels,
		scan_req->minChannelTime, scan_req->maxChannelTime,
		scan_req->uIEFieldLen, scan_req->returnAfterFirstMatch,
		scan_req->returnFreshResults, scan_req->returnUniqueResults,
		scan_req->scanType, mac_ctx->lim.gLimRspReqd ? 1 : 0);
	/*
	 * Since scan req always requires a response, we will overwrite response
	 * required here. This is added esp to take care of the condition where
	 * in p2p go case, we hold the scan req and insert single NOA. We send
	 * the held scan request to FW later on getting start NOA ind from FW so
	 * we lose state of the gLimRspReqd flag for the scan req if any other
	 * request comes by then. e.g. While unit testing, we found when insert
	 * single NOA is done, we see a get stats request which turns the flag
	 * gLimRspReqd to false; now when we actually start the saved scan req
	 * for init scan after getting NOA started, the gLimRspReqd being a
	 * global flag is showing false instead of true value for this saved
	 * scan req. Since all scan reqs coming to lim require a response,
	 * there is no harm in setting the global flag gLimRspReqd to true here.
	 */
	mac_ctx->lim.gLimRspReqd = true;

	/*
	 * copy the Self MAC address from SmeReq to the globalplace,
	 * used for sending probe req
	 */
	sir_copy_mac_addr(mac_ctx->lim.gSelfMacAddr,
			  scan_req->selfMacAddr.bytes);
	valid_req = lim_is_sme_scan_req_valid(mac_ctx, scan_req);

	if (!valid_req || mac_ctx->lim.scan_disabled) {
		lim_log(mac_ctx, LOGE,
			FL("Scan disabled %d, Valid Scan Req %d"),
			mac_ctx->lim.scan_disabled, valid_req);

		if (mac_ctx->lim.gLimRspReqd) {
			mac_ctx->lim.gLimRspReqd = false;

			lim_send_sme_scan_rsp(mac_ctx,
					eSIR_SME_INVALID_PARAMETERS,
					scan_req->sessionId,
					scan_req->transactionId,
					scan_req->scan_id);
		}
		return;
	}

	/*
	 * If scan request is received in idle, joinFailed
	 * states or in link established state (in STA role)
	 * or in normal state (in STA-in-IBSS/AP role) with
	 * 'return fresh scan results' request from HDD or
	 * it is periodic background scanning request,
	 * trigger fresh scan request to MLM
	 */
	if (__lim_fresh_scan_reqd(mac_ctx, scan_req->returnFreshResults)) {

		mac_ctx->lim.gLim24Band11dScanDone = 0;
		mac_ctx->lim.gLim50Band11dScanDone = 0;
		mac_ctx->lim.gLimReturnAfterFirstMatch =
			scan_req->returnAfterFirstMatch;
		mac_ctx->lim.gLimReturnUniqueResults =
			((scan_req->returnUniqueResults) > 0 ? true : false);

		if (CDF_STATUS_SUCCESS !=
			lim_send_hal_start_scan_offload_req(mac_ctx,
			    scan_req)) {
			lim_log(mac_ctx, LOGE, FL(
				"Couldn't send Offload scan request"));
			lim_send_sme_scan_rsp(mac_ctx,
					eSIR_SME_INVALID_PARAMETERS,
					scan_req->sessionId,
					scan_req->transactionId,
					scan_req->scan_id);
			return;
		}
	}
	else {
		/* In all other cases return 'cached' scan results */
		if (mac_ctx->lim.gLimRspReqd) {
			mac_ctx->lim.gLimRspReqd = false;
			lim_send_sme_scan_rsp(mac_ctx, eSIR_SME_SUCCESS,
				scan_req->sessionId,
				scan_req->transactionId, scan_req->scan_id);
		}
	}
}

#ifdef FEATURE_OEM_DATA_SUPPORT

static void __lim_process_sme_oem_data_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirOemDataReq pOemDataReq;
	tLimMlmOemDataReq *pMlmOemDataReq;

	pOemDataReq = (tpSirOemDataReq) pMsgBuf;

	/* post the lim mlm message now */
	pMlmOemDataReq = cdf_mem_malloc(sizeof(tLimMlmOemDataReq));
	if (NULL == pMlmOemDataReq) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for mlmOemDataReq"));
		return;
	}
	/* Initialize this buffer */
	cdf_mem_set(pMlmOemDataReq, (sizeof(tLimMlmOemDataReq)), 0);

	cdf_copy_macaddr(&pMlmOemDataReq->selfMacAddr,
			 &pOemDataReq->selfMacAddr);
	cdf_mem_copy(pMlmOemDataReq->oemDataReq, pOemDataReq->oemDataReq,
		     OEM_DATA_REQ_SIZE);

	/* Issue LIM_MLM_OEM_DATA_REQ to MLM */
	lim_post_mlm_message(pMac, LIM_MLM_OEM_DATA_REQ,
			     (uint32_t *) pMlmOemDataReq);

	return;

} /*** end __lim_process_sme_oem_data_req() ***/

#endif /* FEATURE_OEM_DATA_SUPPORT */

/**
 * __lim_process_clear_dfs_channel_list()
 *
 ***FUNCTION:
 ***Clear DFS channel list  when country is changed/aquired.
   .*This message is sent from SME.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */
static void __lim_process_clear_dfs_channel_list(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	cdf_mem_set(&pMac->lim.dfschannelList, sizeof(tSirDFSChannelList), 0);
}

/**
 * __lim_process_sme_join_req() - process SME_JOIN_REQ message
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: A pointer to the SME message buffer
 *
 * This function is called to process SME_JOIN_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */
static void
__lim_process_sme_join_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tpSirSmeJoinReq sme_join_req = NULL;
	tLimMlmJoinReq *mlm_join_req;
	tSirResultCodes ret_code = eSIR_SME_SUCCESS;
	uint32_t val = 0;
	uint16_t n_size;
	uint8_t session_id;
	tpPESession session = NULL;
	uint8_t sme_session_id;
	uint16_t sme_transaction_id;
	tPowerdBm local_power_constraint = 0, reg_max = 0;
	uint16_t ie_len;
	uint8_t *vendor_ie;
	tSirBssDescription bss_desc;

/* FEATURE_WLAN_DIAG_SUPPORT */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	/*
	 * Not sending any session, since it is not created yet.
	 * The response whould have correct state.
	 */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_JOIN_REQ_EVENT, NULL, 0, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_log(mac_ctx, LOG1, FL("Received SME_JOIN_REQ"));

	/*
	 * Expect Join request in idle state.
	 * Reassociate request is expected in link established state.
	 */

	/* Global SME and LIM states are not defined yet for BT-AMP Support */
	if (mac_ctx->lim.gLimSmeState == eLIM_SME_IDLE_STATE) {
		n_size = __lim_get_sme_join_req_size_for_alloc((uint8_t *)
				msg_buf);

		sme_join_req = cdf_mem_malloc(n_size);
		if (NULL == sme_join_req) {
			lim_log(mac_ctx, LOGP,
				FL("AllocateMemory failed for sme_join_req"));
			ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
			return;
		}
		(void)cdf_mem_set((void *)sme_join_req, n_size, 0);
		(void)cdf_mem_copy((void *)sme_join_req, (void *)msg_buf,
			n_size);

		if (!lim_is_sme_join_req_valid(mac_ctx, sme_join_req)) {
			/* Received invalid eWNI_SME_JOIN_REQ */
			/* Log the event */
			lim_log(mac_ctx, LOGW,
				FL("SessionId:%d JOIN REQ with invalid data"),
				sme_join_req->sessionId);
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}

		/*
		 * Update the capability here itself as this is used in
		 * lim_extract_ap_capability() below. If not updated issues
		 * like not honoring power constraint on 1st association after
		 * driver loading might occur.
		 */
		lim_update_rrm_capability(mac_ctx, sme_join_req);

		bss_desc = sme_join_req->bssDescription;
		/* check for the existence of start BSS session  */
		session = pe_find_session_by_bssid(mac_ctx, bss_desc.bssId,
				&session_id);

		if (session != NULL) {
			lim_log(mac_ctx, LOGE,
				FL("Session(%d) Already exists for BSSID: "
				   MAC_ADDRESS_STR " in limSmeState = %X"),
				session_id,
				MAC_ADDR_ARRAY(bss_desc.bssId),
				session->limSmeState);

			if (session->limSmeState == eLIM_SME_LINK_EST_STATE &&
			    session->smeSessionId == sme_join_req->sessionId) {
				/*
				 * Received eWNI_SME_JOIN_REQ for same
				 * BSS as currently associated.
				 * Log the event and send success
				 */
				lim_log(mac_ctx, LOGW,
					FL("SessionId: %d"), session_id);
				lim_log(mac_ctx, LOGW,
					FL("JOIN_REQ for current joined BSS"));
				/* Send Join success response to host */
				ret_code = eSIR_SME_ALREADY_JOINED_A_BSS;
				session = NULL;
				goto end;
			} else {
				lim_log(mac_ctx, LOGE,
				    FL("JOIN_REQ not for current joined BSS"));
				ret_code = eSIR_SME_REFUSED;
				session = NULL;
				goto end;
			}
		} else {
			/*
			 * Session Entry does not exist for given BSSId
			 * Try to Create a new session
			 */
			session = pe_create_session(mac_ctx, bss_desc.bssId,
					&session_id, mac_ctx->lim.maxStation,
					eSIR_INFRASTRUCTURE_MODE);
			if (session == NULL) {
				lim_log(mac_ctx, LOGE,
					FL("Session Can not be created "));
				ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
				goto end;
			} else
				lim_log(mac_ctx, LOG1,
					FL("SessionId:%d New session created"),
					session_id);
		}
		session->isAmsduSupportInAMPDU =
			sme_join_req->isAmsduSupportInAMPDU;

		/*
		 * Store Session related parameters
		 * Store PE session Id in session Table
		 */
		session->peSessionId = session_id;

		/* store the smejoin req handle in session table */
		session->pLimJoinReq = sme_join_req;

		/* Store SME session Id in sessionTable */
		session->smeSessionId = sme_join_req->sessionId;

		/* Store SME transaction Id in session Table */
		session->transactionId = sme_join_req->transactionId;

		/* Store beaconInterval */
		session->beaconParams.beaconInterval =
			bss_desc.beaconInterval;

		cdf_mem_copy(&(session->htConfig), &(sme_join_req->htConfig),
			sizeof(session->htConfig));

		/* Copying of bssId is already done, while creating session */
		sir_copy_mac_addr(session->selfMacAddr,
			sme_join_req->selfMacAddr);
		session->bssType = sme_join_req->bsstype;

		session->statypeForBss = STA_ENTRY_PEER;
		session->limWmeEnabled = sme_join_req->isWMEenabled;
		session->limQosEnabled = sme_join_req->isQosEnabled;

		/* Store vendor specfic IE for CISCO AP */
		ie_len = (bss_desc.length + sizeof(bss_desc.length) -
			 GET_FIELD_OFFSET(tSirBssDescription, ieFields));

		vendor_ie = cfg_get_vendor_ie_ptr_from_oui(mac_ctx,
				SIR_MAC_CISCO_OUI, SIR_MAC_CISCO_OUI_SIZE,
				((uint8_t *)&bss_desc.ieFields), ie_len);

		if (NULL != vendor_ie) {
			lim_log(mac_ctx, LOG1, FL("Cisco vendor OUI present"));
			session->isCiscoVendorAP = true;
		} else {
			session->isCiscoVendorAP = false;
		}

		/* Copy the dot 11 mode in to the session table */

		session->dot11mode = sme_join_req->dot11mode;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		session->cc_switch_mode = sme_join_req->cc_switch_mode;
#endif
		session->nwType = bss_desc.nwType;
		session->enableAmpduPs = sme_join_req->enableAmpduPs;
		session->enableHtSmps = sme_join_req->enableHtSmps;
		session->htSmpsvalue = sme_join_req->htSmps;

		/*Store Persona */
		session->pePersona = sme_join_req->staPersona;
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
			  FL("PE PERSONA=%d cbMode %u"),
			  session->pePersona, sme_join_req->cbMode);
		if (mac_ctx->roam.configParam.enable2x2)
			session->nss = 2;
		else
			session->nss = 1;
#ifdef WLAN_FEATURE_11AC
		session->vhtCapability =
			IS_DOT11_MODE_VHT(session->dot11mode);
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO_MED,
			  "***__lim_process_sme_join_req: vhtCapability=%d****",
			  session->vhtCapability);
		if (session->vhtCapability) {
			if (session->pePersona == CDF_STA_MODE) {
				session->txBFIniFeatureEnabled =
					sme_join_req->txBFIniFeatureEnabled;
			} else {
				session->txBFIniFeatureEnabled = 0;
			}
			session->txMuBformee = sme_join_req->txMuBformee;
			session->enableVhtpAid =
				sme_join_req->enableVhtpAid;
			session->enableVhtGid =
				sme_join_req->enableVhtGid;

			CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO_MED,
				  FL("***txBFIniFeatureEnabled=%d***"),
				  session->txBFIniFeatureEnabled);
			if (wlan_cfg_get_int(mac_ctx,
					WNI_CFG_VHT_SU_BEAMFORMER_CAP, &val) !=
				eSIR_SUCCESS)
				lim_log(mac_ctx, LOGE, FL(
					"cfg get vht su bformer failed"));

			session->enable_su_tx_bformer = val;
			lim_log(mac_ctx, LOGE, FL("vht su tx bformer %d"), val);
		}
		if (session->vhtCapability && session->txBFIniFeatureEnabled) {
			if (cfg_set_int(mac_ctx, WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
				session->txBFIniFeatureEnabled) !=
				eSIR_SUCCESS) {
				/*
				 * Set failed for
				 * CFG_VHT_SU_BEAMFORMEE_CAP
				 */
				lim_log(mac_ctx, LOGP,
					FL("Failed CFG_VHT_SU_BEAMFORMEE_CAP"));
				ret_code = eSIR_LOGP_EXCEPTION;
				goto end;
			}
			CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO_MED,
				  "%s: txBFCsnValue=%d", __func__,
				  sme_join_req->txBFCsnValue);
			if (cfg_set_int(mac_ctx,
				WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
				sme_join_req->txBFCsnValue) != eSIR_SUCCESS) {
				/*
				 * Set Failed for CFG
				 * CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED
				 */
				lim_log(mac_ctx, LOGP, FL("Set Fail CFG"));
				ret_code = eSIR_LOGP_EXCEPTION;
				goto end;
			}
		}
#endif

		/*Phy mode */
		session->gLimPhyMode = bss_desc.nwType;
		handle_ht_capabilityand_ht_info(mac_ctx, session);
		/* Copy The channel Id to the session Table */
		session->currentOperChannel = bss_desc.channelId;
		/* cbMode is already merged value of peer and self -
		 * done by csr in csr_get_cb_mode_from_ies */
		session->htSupportedChannelWidthSet =
			(sme_join_req->cbMode) ? 1 : 0;
		session->htRecommendedTxWidthSet =
			session->htSupportedChannelWidthSet;
		session->htSecondaryChannelOffset = sme_join_req->cbMode;

		if (PHY_DOUBLE_CHANNEL_HIGH_PRIMARY == sme_join_req->cbMode) {
			session->ch_center_freq_seg0 =
				session->currentOperChannel - 2;
			session->ch_width = CH_WIDTH_40MHZ;
		} else if (PHY_DOUBLE_CHANNEL_LOW_PRIMARY ==
				sme_join_req->cbMode) {
			session->ch_center_freq_seg0 =
				session->currentOperChannel + 2;
			session->ch_width = CH_WIDTH_40MHZ;
		} else {
			session->ch_center_freq_seg0 = 0;
			session->ch_width = CH_WIDTH_20MHZ;
		}

		/* Record if management frames need to be protected */
#ifdef WLAN_FEATURE_11W
		if (eSIR_ED_AES_128_CMAC == sme_join_req->MgmtEncryptionType) {
			CDF_STATUS cdf_status;
			session->limRmfEnabled = 1;
			session->pmfComebackTimerInfo.pMac = mac_ctx;
			session->pmfComebackTimerInfo.sessionID =
				session_id;
			cdf_status = cdf_mc_timer_init(
					&session->pmfComebackTimer,
					CDF_TIMER_TYPE_SW,
					lim_pmf_comeback_timer_callback,
					(void *)&session->pmfComebackTimerInfo);
			if (CDF_STATUS_SUCCESS != cdf_status) {
				lim_log(mac_ctx, LOGP,
					FL("cannot init pmf comeback timer."));
				ret_code = eSIR_LOGP_EXCEPTION;
				goto end;
			}
		} else {
			session->limRmfEnabled = 0;
		}
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
		session->rssi = bss_desc.rssi;
#endif

		/* Copy the SSID from smejoinreq to session entry  */
		session->ssId.length = sme_join_req->ssId.length;
		cdf_mem_copy(session->ssId.ssId, sme_join_req->ssId.ssId,
			session->ssId.length);

		/*
		 * Determin 11r or ESE connection based on input from SME
		 * which inturn is dependent on the profile the user wants
		 * to connect to, So input is coming from supplicant
		 */
#ifdef WLAN_FEATURE_VOWIFI_11R
		session->is11Rconnection = sme_join_req->is11Rconnection;
#endif
#ifdef FEATURE_WLAN_ESE
		session->isESEconnection = sme_join_req->isESEconnection;
#endif
#if defined WLAN_FEATURE_VOWIFI_11R || defined FEATURE_WLAN_ESE || defined(FEATURE_WLAN_LFR)
		session->isFastTransitionEnabled =
			sme_join_req->isFastTransitionEnabled;
#endif

#ifdef FEATURE_WLAN_LFR
		session->isFastRoamIniFeatureEnabled =
			sme_join_req->isFastRoamIniFeatureEnabled;
#endif
		session->txLdpcIniFeatureEnabled =
			sme_join_req->txLdpcIniFeatureEnabled;

		if (session->bssType == eSIR_INFRASTRUCTURE_MODE) {
			session->limSystemRole = eLIM_STA_ROLE;
		} else if (session->bssType == eSIR_BTAMP_AP_MODE) {
			session->limSystemRole = eLIM_BT_AMP_STA_ROLE;
		} else {
			/*
			 * Throw an error and return and make
			 * sure to delete the session.
			 */
			lim_log(mac_ctx, LOGE,
				FL("recvd JOIN_REQ with invalid bss type %d"),
				session->bssType);
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}

		if (sme_join_req->addIEScan.length)
			cdf_mem_copy(&session->pLimJoinReq->addIEScan,
				&sme_join_req->addIEScan, sizeof(tSirAddie));

		if (sme_join_req->addIEAssoc.length)
			cdf_mem_copy(&session->pLimJoinReq->addIEAssoc,
				&sme_join_req->addIEAssoc, sizeof(tSirAddie));

		val = sizeof(tLimMlmJoinReq) +
			session->pLimJoinReq->bssDescription.length + 2;
		mlm_join_req = cdf_mem_malloc(val);
		if (NULL == mlm_join_req) {
			lim_log(mac_ctx, LOGP,
				FL("AllocateMemory failed for mlmJoinReq"));
			return;
		}
		(void)cdf_mem_set((void *)mlm_join_req, val, 0);

		/* PE SessionId is stored as a part of JoinReq */
		mlm_join_req->sessionId = session->peSessionId;

		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_JOIN_FAILURE_TIMEOUT,
			(uint32_t *) &mlm_join_req->joinFailureTimeout) !=
			eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGP,
				FL("couldn't retrieve JoinFailureTimer value"
				" setting to default value"));
			mlm_join_req->joinFailureTimeout =
				WNI_CFG_JOIN_FAILURE_TIMEOUT_STADEF;
		}

		/* copy operational rate from session */
		cdf_mem_copy((void *)&session->rateSet,
			(void *)&sme_join_req->operationalRateSet,
			sizeof(tSirMacRateSet));
		cdf_mem_copy((void *)&session->extRateSet,
			(void *)&sme_join_req->extendedRateSet,
			sizeof(tSirMacRateSet));
		/*
		 * this may not be needed anymore now, as rateSet is now
		 * included in the session entry and MLM has session context.
		 */
		cdf_mem_copy((void *)&mlm_join_req->operationalRateSet,
			(void *)&session->rateSet,
			sizeof(tSirMacRateSet));

		session->encryptType = sme_join_req->UCEncryptionType;

		mlm_join_req->bssDescription.length =
			session->pLimJoinReq->bssDescription.length;

		cdf_mem_copy((uint8_t *) &mlm_join_req->bssDescription.bssId,
			(uint8_t *)
			&session->pLimJoinReq->bssDescription.bssId,
			session->pLimJoinReq->bssDescription.length + 2);

		session->limCurrentBssCaps =
			session->pLimJoinReq->bssDescription.capabilityInfo;

		reg_max = cfg_get_regulatory_max_transmit_power(mac_ctx,
				session->currentOperChannel);
		local_power_constraint = reg_max;

		lim_extract_ap_capability(mac_ctx,
			(uint8_t *)
			session->pLimJoinReq->bssDescription.ieFields,
			lim_get_ielen_from_bss_description(
			&session->pLimJoinReq->bssDescription),
			&session->limCurrentBssQosCaps,
			&session->limCurrentBssPropCap,
			&session->gLimCurrentBssUapsd,
			&local_power_constraint, session);

#ifdef FEATURE_WLAN_ESE
		session->maxTxPower = lim_get_max_tx_power(reg_max,
					local_power_constraint,
					mac_ctx->roam.configParam.nTxPowerCap);
#else
		session->maxTxPower =
			CDF_MIN(reg_max, (local_power_constraint));
#endif
#if defined WLAN_VOWIFI_DEBUG
		lim_log(mac_ctx, LOGE,
			"Regulatory max = %d, local power constraint = %d"
			reg_max, local_power_constraint);
		lim_log(mac_ctx, LOGE, FL(" max tx = %d"),
			session->maxTxPower);
#endif

		if (session->gLimCurrentBssUapsd) {
			session->gUapsdPerAcBitmask =
				session->pLimJoinReq->uapsdPerAcBitmask;
			lim_log(mac_ctx, LOG1,
				FL("UAPSD flag for all AC - 0x%2x"),
				session->gUapsdPerAcBitmask);

			/* resetting the dynamic uapsd mask  */
			session->gUapsdPerAcDeliveryEnableMask = 0;
			session->gUapsdPerAcTriggerEnableMask = 0;
		}

		session->limRFBand =
			lim_get_rf_band(session->currentOperChannel);

		/* Initialize 11h Enable Flag */
		if (SIR_BAND_5_GHZ == session->limRFBand) {
			if (wlan_cfg_get_int(mac_ctx, WNI_CFG_11H_ENABLED,
				&val) != eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGP,
					FL("Fail to get WNI_CFG_11H_ENABLED "));
				session->lim11hEnable =
					WNI_CFG_11H_ENABLED_STADEF;
			} else {
				session->lim11hEnable = val;
			}
		} else {
			session->lim11hEnable = 0;
		}

		/*
		 * To care of the scenario when STA transitions from
		 * IBSS to Infrastructure mode.
		 */
		mac_ctx->lim.gLimIbssCoalescingHappened = false;

		session->limPrevSmeState = session->limSmeState;
		session->limSmeState = eLIM_SME_WT_JOIN_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
				session->peSessionId,
				session->limSmeState));

		lim_log(mac_ctx, LOG1,
			FL("SME JoinReq:Sessionid %d SSID len %d SSID : %s Channel %d, BSSID " MAC_ADDRESS_STR),
			mlm_join_req->sessionId, session->ssId.length,
			session->ssId.ssId, session->currentOperChannel,
			MAC_ADDR_ARRAY(session->bssId));

		/* Indicate whether spectrum management is enabled */
		session->spectrumMgtEnabled =
			sme_join_req->spectrumMgtIndicator;

		/* Enable the spectrum management if this is a DFS channel */
		if (session->country_info_present &&
			lim_isconnected_on_dfs_channel(
					session->currentOperChannel))
			session->spectrumMgtEnabled = true;

		session->isOSENConnection = sme_join_req->isOSENConnection;

		lim_log(mac_ctx, LOG1,
			FL("SessionId:%d MLM_JOIN_REQ is posted to MLM SM"),
			mlm_join_req->sessionId);
		/* Issue LIM_MLM_JOIN_REQ to MLM */
		lim_post_mlm_message(mac_ctx, LIM_MLM_JOIN_REQ,
				     (uint32_t *) mlm_join_req);
		return;

	} else {
		/* Received eWNI_SME_JOIN_REQ un expected state */
		lim_log(mac_ctx, LOGE,
			FL("received unexpected SME_JOIN_REQ in state %X"),
			mac_ctx->lim.gLimSmeState);
		lim_print_sme_state(mac_ctx, LOGE, mac_ctx->lim.gLimSmeState);
		ret_code = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
		session = NULL;
		goto end;
	}

end:
	sme_session_id = sme_join_req->sessionId;
	sme_transaction_id = sme_join_req->transactionId;

	if (sme_join_req) {
		cdf_mem_free(sme_join_req);
		sme_join_req = NULL;
		if (NULL != session)
			session->pLimJoinReq = NULL;
	}
	if (ret_code != eSIR_SME_SUCCESS) {
		if (NULL != session) {
			pe_delete_session(mac_ctx, session);
			session = NULL;
		}
	}
	lim_log(mac_ctx, LOG1,
		FL("Send failure status on sessionid: %d with ret_code = %d"),
		sme_session_id, ret_code);
	lim_send_sme_join_reassoc_rsp(mac_ctx, eWNI_SME_JOIN_RSP, ret_code,
		eSIR_MAC_UNSPEC_FAILURE_STATUS, session, sme_session_id,
		sme_transaction_id);
}

#if defined FEATURE_WLAN_ESE || defined WLAN_FEATURE_VOWIFI
uint8_t lim_get_max_tx_power(tPowerdBm regMax, tPowerdBm apTxPower,
			     uint8_t iniTxPower)
{
	uint8_t maxTxPower = 0;
	uint8_t txPower = CDF_MIN(regMax, (apTxPower));
	txPower = CDF_MIN(txPower, iniTxPower);
	if ((txPower >= MIN_TX_PWR_CAP) && (txPower <= MAX_TX_PWR_CAP))
		maxTxPower = txPower;
	else if (txPower < MIN_TX_PWR_CAP)
		maxTxPower = MIN_TX_PWR_CAP;
	else
		maxTxPower = MAX_TX_PWR_CAP;

	return maxTxPower;
}
#endif

/**
 * __lim_process_sme_reassoc_req() - process reassoc req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process SME_REASSOC_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */

static void __lim_process_sme_reassoc_req(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	uint16_t caps;
	uint32_t val;
	tpSirSmeJoinReq reassoc_req = NULL;
	tLimMlmReassocReq *mlm_reassoc_req;
	tSirResultCodes ret_code = eSIR_SME_SUCCESS;
	tpPESession session_entry = NULL;
	uint8_t session_id;
	uint8_t sme_session_id;
	uint16_t transaction_id;
	tPowerdBm local_pwr_constraint = 0, reg_max = 0;
	uint32_t tele_bcn_en = 0;
	uint16_t size;

	lim_log(mac_ctx, LOG3, FL("Received REASSOC_REQ"));

	size = __lim_get_sme_join_req_size_for_alloc((uint8_t *)msg_buf);
	reassoc_req = cdf_mem_malloc(size);
	if (NULL == reassoc_req) {
		lim_log(mac_ctx, LOGP,
			FL("call to AllocateMemory failed for reassoc_req"));

		ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
		goto end;
	}
	(void)cdf_mem_set((void *)reassoc_req, size, 0);
	(void)cdf_mem_copy((void *)reassoc_req, (void *)msg_buf, size);

	if (!lim_is_sme_join_req_valid(mac_ctx,
				(tpSirSmeJoinReq)reassoc_req)) {
		/*
		 * Received invalid eWNI_SME_REASSOC_REQ
		 */
		lim_log(mac_ctx, LOGW,
			FL("received SME_REASSOC_REQ with invalid data"));

		ret_code = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}

	session_entry = pe_find_session_by_bssid(mac_ctx,
			reassoc_req->bssDescription.bssId,
			&session_id);
	if (session_entry == NULL) {
		lim_print_mac_addr(mac_ctx, reassoc_req->bssDescription.bssId,
				LOGE);
		lim_log(mac_ctx, LOGE,
			FL("Session does not exist for given bssId"));
		ret_code = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_REASSOC_REQ_EVENT,
			session_entry, eSIR_SUCCESS, eSIR_SUCCESS);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
	/* mac_ctx->lim.gpLimReassocReq = reassoc_req;//TO SUPPORT BT-AMP */

	/* Store the reassoc handle in the session Table */
	session_entry->pLimReAssocReq = reassoc_req;

	session_entry->dot11mode = reassoc_req->dot11mode;
	session_entry->vhtCapability =
		IS_DOT11_MODE_VHT(reassoc_req->dot11mode);
	/*
	 * Reassociate request is expected
	 * in link established state only.
	 */

	if (session_entry->limSmeState != eLIM_SME_LINK_EST_STATE) {
#if defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
		defined(FEATURE_WLAN_LFR)
		if (session_entry->limSmeState == eLIM_SME_WT_REASSOC_STATE) {
			/*
			 * May be from 11r FT pre-auth. So lets check it
			 * before we bail out
			 */
			lim_log(mac_ctx, LOG1, FL(
					"Session in reassoc state is %d"),
					session_entry->peSessionId);

			/* Make sure its our preauth bssid */
			if (!cdf_mem_compare(reassoc_req->bssDescription.bssId,
					     session_entry->limReAssocbssId,
					     6)) {
				lim_print_mac_addr(mac_ctx,
						   reassoc_req->bssDescription.
						   bssId, LOGE);
				lim_log(mac_ctx, LOGP,
					FL("Unknown bssId in reassoc state"));
				ret_code = eSIR_SME_INVALID_PARAMETERS;
				goto end;
			}

			lim_process_mlm_ft_reassoc_req(mac_ctx, msg_buf,
					session_entry);
			return;
		}
#endif
		/*
		 * Should not have received eWNI_SME_REASSOC_REQ
		 */
		lim_log(mac_ctx, LOGE,
			FL("received unexpected SME_REASSOC_REQ in state %X"),
			session_entry->limSmeState);
		lim_print_sme_state(mac_ctx, LOGE, session_entry->limSmeState);

		ret_code = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
		goto end;
	}

	cdf_mem_copy(session_entry->limReAssocbssId,
		     session_entry->pLimReAssocReq->bssDescription.bssId,
		     sizeof(tSirMacAddr));

	session_entry->limReassocChannelId =
		session_entry->pLimReAssocReq->bssDescription.channelId;

	session_entry->reAssocHtSupportedChannelWidthSet =
		(session_entry->pLimReAssocReq->cbMode) ? 1 : 0;
	session_entry->reAssocHtRecommendedTxWidthSet =
		session_entry->reAssocHtSupportedChannelWidthSet;
	session_entry->reAssocHtSecondaryChannelOffset =
		session_entry->pLimReAssocReq->cbMode;

	session_entry->limReassocBssCaps =
		session_entry->pLimReAssocReq->bssDescription.capabilityInfo;
	reg_max = cfg_get_regulatory_max_transmit_power(mac_ctx,
			session_entry->currentOperChannel);
	local_pwr_constraint = reg_max;

	lim_extract_ap_capability(mac_ctx,
		(uint8_t *)session_entry->pLimReAssocReq->bssDescription.ieFields,
		lim_get_ielen_from_bss_description(
			&session_entry->pLimReAssocReq->bssDescription),
		&session_entry->limReassocBssQosCaps,
		&session_entry->limReassocBssPropCap,
		&session_entry->gLimCurrentBssUapsd,
		&local_pwr_constraint, session_entry);
	session_entry->maxTxPower = CDF_MIN(reg_max, (local_pwr_constraint));
#if defined WLAN_VOWIFI_DEBUG
	lim_log(mac_ctx, LOGE,
		"Regulatory max = %d, local pwr constraint = %d, max tx = %d",
		reg_max, local_pwr_constraint,
		session_entry->maxTxPower);
#endif
	/* Copy the SSID from session entry to local variable */
	session_entry->limReassocSSID.length = reassoc_req->ssId.length;
	cdf_mem_copy(session_entry->limReassocSSID.ssId,
		     reassoc_req->ssId.ssId,
		     session_entry->limReassocSSID.length);
	if (session_entry->gLimCurrentBssUapsd) {
		session_entry->gUapsdPerAcBitmask =
			session_entry->pLimReAssocReq->uapsdPerAcBitmask;
		lim_log(mac_ctx, LOG1,
			FL("UAPSD flag for all AC - 0x%2x"),
			session_entry->gUapsdPerAcBitmask);
	}

	mlm_reassoc_req = cdf_mem_malloc(sizeof(tLimMlmReassocReq));
	if (NULL == mlm_reassoc_req) {
		lim_log(mac_ctx, LOGP,
			FL("call to AllocateMemory failed for mlmReassocReq"));

		ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
		goto end;
	}

	cdf_mem_copy(mlm_reassoc_req->peerMacAddr,
		     session_entry->limReAssocbssId, sizeof(tSirMacAddr));

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
			(uint32_t *)&mlm_reassoc_req->reassocFailureTimeout) !=
			eSIR_SUCCESS) {
		/*
		 * Could not get ReassocFailureTimeout value
		 * from CFG. Log error.
		 */
		lim_log(mac_ctx, LOGP,
			FL("could not retrieve ReassocFailureTimeout value"));
	}

	if (cfg_get_capability_info(mac_ctx, &caps, session_entry) !=
			eSIR_SUCCESS) {
		/*
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		lim_log(mac_ctx, LOGP, FL(
				"could not retrieve Capabilities value"));
	}
	mlm_reassoc_req->capabilityInfo = caps;

	/* Update PE session_id */
	mlm_reassoc_req->sessionId = session_id;

	/*
	 * If telescopic beaconing is enabled, set listen interval to
	 * WNI_CFG_TELE_BCN_MAX_LI
	 */
	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_TELE_BCN_WAKEUP_EN,
				&tele_bcn_en) != eSIR_SUCCESS)
		lim_log(mac_ctx, LOGP,
			FL("Couldn't get WNI_CFG_TELE_BCN_WAKEUP_EN"));

	val = WNI_CFG_LISTEN_INTERVAL_STADEF;

	if (tele_bcn_en) {
		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_TELE_BCN_MAX_LI, &val) !=
		    eSIR_SUCCESS)
			/*
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			lim_log(mac_ctx, LOGP,
				FL("could not retrieve ListenInterval"));
	} else {
		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_LISTEN_INTERVAL, &val) !=
		    eSIR_SUCCESS)
			/*
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			lim_log(mac_ctx, LOGP,
				FL("could not retrieve ListenInterval"));
	}

	mlm_reassoc_req->listenInterval = (uint16_t) val;

	/* Indicate whether spectrum management is enabled */
	session_entry->spectrumMgtEnabled = reassoc_req->spectrumMgtIndicator;

	/* Enable the spectrum management if this is a DFS channel */
	if (session_entry->country_info_present &&
		lim_isconnected_on_dfs_channel(
				session_entry->currentOperChannel))
		session_entry->spectrumMgtEnabled = true;

	session_entry->limPrevSmeState = session_entry->limSmeState;
	session_entry->limSmeState = eLIM_SME_WT_REASSOC_STATE;

	MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
				session_entry->peSessionId,
				session_entry->limSmeState));

	lim_post_mlm_message(mac_ctx,
			     LIM_MLM_REASSOC_REQ, (uint32_t *)mlm_reassoc_req);
	return;
end:
	if (reassoc_req) {
		cdf_mem_free(reassoc_req);
		if (session_entry)
			session_entry->pLimReAssocReq = NULL;
	}

	if (session_entry) {
		/*
		 * error occurred after we determined the session so extract
		 * session and transaction info from there
		 */
		sme_session_id = session_entry->smeSessionId;
		transaction_id = session_entry->transactionId;
	} else
		/*
		 * error occurred before or during the time we determined
		 * the session so extract the session and transaction info
		 * from the message
		 */
		lim_get_session_info(mac_ctx, (uint8_t *) msg_buf,
				&sme_session_id, &transaction_id);

	/*
	 * Send Reassoc failure response to host
	 * (note session_entry may be NULL, but that's OK)
	 */
	lim_send_sme_join_reassoc_rsp(mac_ctx, eWNI_SME_REASSOC_RSP,
				      ret_code, eSIR_MAC_UNSPEC_FAILURE_STATUS,
				      session_entry, sme_session_id,
				      transaction_id);
}

bool send_disassoc_frame = 1;
/**
 * __lim_process_sme_disassoc_req()
 *
 ***FUNCTION:
 * This function is called to process SME_DISASSOC_REQ message
 * from HDD or upper layer application.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void __lim_process_sme_disassoc_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	uint16_t disassocTrigger, reasonCode;
	tLimMlmDisassocReq *pMlmDisassocReq;
	tSirResultCodes retCode = eSIR_SME_SUCCESS;
	tSirSmeDisassocReq smeDisassocReq;
	tpPESession psessionEntry = NULL;
	uint8_t sessionId;
	uint8_t smesessionId;
	uint16_t smetransactionId;

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	cdf_mem_copy(&smeDisassocReq, pMsgBuf, sizeof(tSirSmeDisassocReq));
	smesessionId = smeDisassocReq.sessionId;
	smetransactionId = smeDisassocReq.transactionId;
	if (!lim_is_sme_disassoc_req_valid(pMac,
					   &smeDisassocReq,
					   psessionEntry)) {
		PELOGE(lim_log(pMac, LOGE,
		       FL("received invalid SME_DISASSOC_REQ message"));)
		if (pMac->lim.gLimRspReqd) {
			pMac->lim.gLimRspReqd = false;

			retCode = eSIR_SME_INVALID_PARAMETERS;
			disassocTrigger = eLIM_HOST_DISASSOC;
			goto sendDisassoc;
		}

		return;
	}

	psessionEntry = pe_find_session_by_bssid(pMac,
				smeDisassocReq.bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("session does not exist for given bssId "
			   MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(smeDisassocReq.bssId));
		retCode = eSIR_SME_INVALID_PARAMETERS;
		disassocTrigger = eLIM_HOST_DISASSOC;
		goto sendDisassoc;
	}
	lim_log(pMac, LOG1,
		FL("received DISASSOC_REQ message on sessionid %d Systemrole %d Reason: %u SmeState: %d from: "
			MAC_ADDRESS_STR), smesessionId,
		GET_LIM_SYSTEM_ROLE(psessionEntry), smeDisassocReq.reasonCode,
		pMac->lim.gLimSmeState,
		MAC_ADDR_ARRAY(smeDisassocReq.peerMacAddr));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_REQ_EVENT, psessionEntry,
			      0, smeDisassocReq.reasonCode);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	/* Update SME session Id and SME transaction ID */

	psessionEntry->smeSessionId = smesessionId;
	psessionEntry->transactionId = smetransactionId;

	switch (GET_LIM_SYSTEM_ROLE(psessionEntry)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:
		switch (psessionEntry->limSmeState) {
		case eLIM_SME_ASSOCIATED_STATE:
		case eLIM_SME_LINK_EST_STATE:
			psessionEntry->limPrevSmeState =
				psessionEntry->limSmeState;
			psessionEntry->limSmeState = eLIM_SME_WT_DISASSOC_STATE;
#ifdef FEATURE_WLAN_TDLS
			/* Delete all TDLS peers connected before leaving BSS */
			lim_delete_tdls_peers(pMac, psessionEntry);
#endif
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_SME_STATE,
				       psessionEntry->peSessionId,
				       psessionEntry->limSmeState));
			lim_log(pMac, LOG1,
				FL("Rcvd SME_DISASSOC_REQ while in limSmeState: %d "),
				psessionEntry->limSmeState);
			break;

		case eLIM_SME_WT_DEAUTH_STATE:
			/* PE shall still process the DISASSOC_REQ and proceed with
			 * link tear down even if it had already sent a DEAUTH_IND to
			 * to SME. pMac->lim.gLimPrevSmeState shall remain the same as
			 * its been set when PE entered WT_DEAUTH_STATE.
			 */
			psessionEntry->limSmeState = eLIM_SME_WT_DISASSOC_STATE;
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_SME_STATE,
				       psessionEntry->peSessionId,
				       psessionEntry->limSmeState));
			lim_log(pMac, LOG1,
				FL("Rcvd SME_DISASSOC_REQ while in SME_WT_DEAUTH_STATE. "));
			break;

		case eLIM_SME_WT_DISASSOC_STATE:
			/* PE Recieved a Disassoc frame. Normally it gets DISASSOC_CNF but it
			 * received DISASSOC_REQ. Which means host is also trying to disconnect.
			 * PE can continue processing DISASSOC_REQ and send the response instead
			 * of failing the request. SME will anyway ignore DEAUTH_IND that was sent
			 * for disassoc frame.
			 *
			 * It will send a disassoc, which is ok. However, we can use the global flag
			 * sendDisassoc to not send disassoc frame.
			 */
			lim_log(pMac, LOG1,
				FL("Rcvd SME_DISASSOC_REQ while in SME_WT_DISASSOC_STATE. "));
			break;

		case eLIM_SME_JOIN_FAILURE_STATE: {
			/* Already in Disconnected State, return success */
			lim_log(pMac, LOG1,
				FL("Rcvd SME_DISASSOC_REQ while in eLIM_SME_JOIN_FAILURE_STATE. "));
			if (pMac->lim.gLimRspReqd) {
				retCode = eSIR_SME_SUCCESS;
				disassocTrigger = eLIM_HOST_DISASSOC;
				goto sendDisassoc;
			}
		}
		break;
		default:
			/**
			 * STA is not currently associated.
			 * Log error and send response to host
			 */
			lim_log(pMac, LOGE,
				FL("received unexpected SME_DISASSOC_REQ in state %X"),
				psessionEntry->limSmeState);
			lim_print_sme_state(pMac, LOGE,
				psessionEntry->limSmeState);

			if (pMac->lim.gLimRspReqd) {
				if (psessionEntry->limSmeState !=
				    eLIM_SME_WT_ASSOC_STATE)
					pMac->lim.gLimRspReqd = false;

				retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
				disassocTrigger = eLIM_HOST_DISASSOC;
				goto sendDisassoc;
			}

			return;
		}

		break;

	case eLIM_AP_ROLE:
	case eLIM_BT_AMP_AP_ROLE:
		/* Fall through */
		break;

	case eLIM_STA_IN_IBSS_ROLE:
	default:
		/* eLIM_UNKNOWN_ROLE */
		lim_log(pMac, LOGE,
			FL("received unexpected SME_DISASSOC_REQ for role %d"),
			GET_LIM_SYSTEM_ROLE(psessionEntry));

		retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
		disassocTrigger = eLIM_HOST_DISASSOC;
		goto sendDisassoc;
	} /* end switch (pMac->lim.gLimSystemRole) */

	disassocTrigger = eLIM_HOST_DISASSOC;
	reasonCode = smeDisassocReq.reasonCode;

	if (smeDisassocReq.doNotSendOverTheAir) {
		lim_log(pMac, LOG1, FL("do not send dissoc over the air"));
		send_disassoc_frame = 0;
	}
	/* Trigger Disassociation frame to peer MAC entity */
	lim_log(pMac, LOG1, FL("Sending Disasscoc with disassoc Trigger"
				" : %d, reasonCode : %d"),
			disassocTrigger, reasonCode);

	pMlmDisassocReq = cdf_mem_malloc(sizeof(tLimMlmDisassocReq));
	if (NULL == pMlmDisassocReq) {
		/* Log error */
		lim_log(pMac, LOGP,
			FL("call to AllocateMemory failed for mlmDisassocReq"));

		return;
	}

	cdf_mem_copy((uint8_t *) &pMlmDisassocReq->peerMacAddr,
		     (uint8_t *) &smeDisassocReq.peerMacAddr,
		     sizeof(tSirMacAddr));

	pMlmDisassocReq->reasonCode = reasonCode;
	pMlmDisassocReq->disassocTrigger = disassocTrigger;

	/* Update PE session ID */
	pMlmDisassocReq->sessionId = sessionId;

	lim_post_mlm_message(pMac,
			     LIM_MLM_DISASSOC_REQ, (uint32_t *) pMlmDisassocReq);
	return;

sendDisassoc:
	if (psessionEntry)
		lim_send_sme_disassoc_ntf(pMac, smeDisassocReq.peerMacAddr,
					  retCode,
					  disassocTrigger,
					  1, smesessionId, smetransactionId,
					  psessionEntry);
	else
		lim_send_sme_disassoc_ntf(pMac, smeDisassocReq.peerMacAddr,
					  retCode,
					  disassocTrigger,
					  1, smesessionId, smetransactionId, NULL);

} /*** end __lim_process_sme_disassoc_req() ***/

/** -----------------------------------------------------------------
   \brief __lim_process_sme_disassoc_cnf() - Process SME_DISASSOC_CNF

   This function is called to process SME_DISASSOC_CNF message
   from HDD or upper layer application.

   \param pMac - global mac structure
   \param pStaDs - station dph hash node
   \return none
   \sa
   ----------------------------------------------------------------- */
static void __lim_process_sme_disassoc_cnf(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirSmeDisassocCnf smeDisassocCnf;
	uint16_t aid;
	tpDphHashNode pStaDs;
	tpPESession psessionEntry;
	uint8_t sessionId;

	PELOG1(lim_log(pMac, LOG1, FL("received DISASSOC_CNF message"));)

	cdf_mem_copy(&smeDisassocCnf, pMsgBuf,
			sizeof(struct sSirSmeDisassocCnf));

	psessionEntry = pe_find_session_by_bssid(pMac,
				smeDisassocCnf.bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("session does not exist for given bssId"));
		return;
	}

	if (!lim_is_sme_disassoc_cnf_valid(pMac, &smeDisassocCnf, psessionEntry)) {
		lim_log(pMac, LOGE,
			FL("received invalid SME_DISASSOC_CNF message"));
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	if (smeDisassocCnf.messageType == eWNI_SME_DISASSOC_CNF)
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_CNF_EVENT,
				      psessionEntry,
				      (uint16_t) smeDisassocCnf.statusCode, 0);
	else if (smeDisassocCnf.messageType == eWNI_SME_DEAUTH_CNF)
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DEAUTH_CNF_EVENT,
				      psessionEntry,
				      (uint16_t) smeDisassocCnf.statusCode, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	switch (GET_LIM_SYSTEM_ROLE(psessionEntry)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:      /* To test reconn */
		if ((psessionEntry->limSmeState != eLIM_SME_IDLE_STATE) &&
		    (psessionEntry->limSmeState != eLIM_SME_WT_DISASSOC_STATE)
		    && (psessionEntry->limSmeState !=
			eLIM_SME_WT_DEAUTH_STATE)) {
			lim_log(pMac, LOGE,
				FL
					("received unexp SME_DISASSOC_CNF in state %X"),
				psessionEntry->limSmeState);
			lim_print_sme_state(pMac, LOGE,
					    psessionEntry->limSmeState);
			return;
		}
		break;

	case eLIM_AP_ROLE:
		/* Fall through */
		break;

	case eLIM_STA_IN_IBSS_ROLE:
	default:                /* eLIM_UNKNOWN_ROLE */
		lim_log(pMac, LOGE,
			FL("received unexpected SME_DISASSOC_CNF role %d"),
			GET_LIM_SYSTEM_ROLE(psessionEntry));

		return;
	}

	if ((psessionEntry->limSmeState == eLIM_SME_WT_DISASSOC_STATE) ||
	    (psessionEntry->limSmeState == eLIM_SME_WT_DEAUTH_STATE) ||
	    LIM_IS_AP_ROLE(psessionEntry)) {
		pStaDs = dph_lookup_hash_entry(pMac,
				smeDisassocCnf.peerMacAddr, &aid,
				&psessionEntry->dph.dphHashTable);
		if (pStaDs == NULL) {
			lim_log(pMac, LOGE,
				FL("DISASSOC_CNF for a STA with no context, addr= "
				MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(smeDisassocCnf.peerMacAddr));
			return;
		}

		if ((pStaDs->mlmStaContext.mlmState ==
				eLIM_MLM_WT_DEL_STA_RSP_STATE) ||
			(pStaDs->mlmStaContext.mlmState ==
				eLIM_MLM_WT_DEL_STA_RSP_STATE)) {
			lim_log(pMac, LOGE,
				FL("No need of cleanup for addr:" MAC_ADDRESS_STR "as MLM state is %d"),
				MAC_ADDR_ARRAY(smeDisassocCnf.peerMacAddr),
				pStaDs->mlmStaContext.mlmState);
			return;
		}

#if defined WLAN_FEATURE_VOWIFI_11R
		/* Delete FT session if there exists one */
		lim_ft_cleanup_pre_auth_info(pMac, psessionEntry);
#endif
		lim_cleanup_rx_path(pMac, pStaDs, psessionEntry);

		lim_clean_up_disassoc_deauth_req(pMac,
						 (char *)&smeDisassocCnf.peerMacAddr,
						 0);
	}

	return;
}

/**
 * __lim_process_sme_deauth_req() - process sme deauth req
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process SME_DEAUTH_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */

static void __lim_process_sme_deauth_req(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	uint16_t deauth_trigger, reason_code;
	tLimMlmDeauthReq *mlm_deauth_req;
	tSirSmeDeauthReq sme_deauth_req;
	tSirResultCodes ret_code = eSIR_SME_SUCCESS;
	tpPESession session_entry;
	uint8_t session_id;      /* PE sessionId */
	uint8_t sme_session_id;
	uint16_t sme_transaction_id;

	lim_log(mac_ctx, LOG1, FL("received DEAUTH_REQ message"));

	cdf_mem_copy(&sme_deauth_req, msg_buf, sizeof(tSirSmeDeauthReq));
	sme_session_id = sme_deauth_req.sessionId;
	sme_transaction_id = sme_deauth_req.transactionId;

	/*
	 * We need to get a session first but we don't even know
	 * if the message is correct.
	 */
	session_entry = pe_find_session_by_bssid(mac_ctx, sme_deauth_req.bssId,
				&session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given bssId"));
		ret_code = eSIR_SME_INVALID_PARAMETERS;
		deauth_trigger = eLIM_HOST_DEAUTH;
		goto send_deauth;
	}

	if (!lim_is_sme_deauth_req_valid(mac_ctx, &sme_deauth_req,
				session_entry)) {
		lim_log(mac_ctx, LOGE,
		       FL("received invalid SME_DEAUTH_REQ message"));
		mac_ctx->lim.gLimRspReqd = false;

		ret_code = eSIR_SME_INVALID_PARAMETERS;
		deauth_trigger = eLIM_HOST_DEAUTH;
		goto send_deauth;
	}
	lim_log(mac_ctx, LOG1,
		FL("received DEAUTH_REQ sessionid %d Systemrole %d reasoncode %u limSmestate %d from "
		MAC_ADDRESS_STR), sme_session_id,
		GET_LIM_SYSTEM_ROLE(session_entry), sme_deauth_req.reasonCode,
		session_entry->limSmeState,
		MAC_ADDR_ARRAY(sme_deauth_req.peerMacAddr));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_DEAUTH_REQ_EVENT,
			session_entry, 0, sme_deauth_req.reasonCode);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	/* Update SME session ID and Transaction ID */
	session_entry->smeSessionId = sme_session_id;
	session_entry->transactionId = sme_transaction_id;

	switch (GET_LIM_SYSTEM_ROLE(session_entry)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:

		switch (session_entry->limSmeState) {
		case eLIM_SME_ASSOCIATED_STATE:
		case eLIM_SME_LINK_EST_STATE:
		case eLIM_SME_WT_ASSOC_STATE:
		case eLIM_SME_JOIN_FAILURE_STATE:
		case eLIM_SME_IDLE_STATE:
			session_entry->limPrevSmeState =
				session_entry->limSmeState;
			session_entry->limSmeState = eLIM_SME_WT_DEAUTH_STATE;
			MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
				       session_entry->peSessionId,
				       session_entry->limSmeState));
			/* Send Deauthentication request to MLM below */
			break;
		case eLIM_SME_WT_DEAUTH_STATE:
		case eLIM_SME_WT_DISASSOC_STATE:
			/*
			 * PE Recieved a Deauth/Disassoc frame. Normally it get
			 * DEAUTH_CNF/DISASSOC_CNF but it received DEAUTH_REQ.
			 * Which means host is also trying to disconnect.
			 * PE can continue processing DEAUTH_REQ and send
			 * the response instead of failing the request.
			 * SME will anyway ignore DEAUTH_IND/DISASSOC_IND that
			 * was sent for deauth/disassoc frame.
			 */
			session_entry->limSmeState = eLIM_SME_WT_DEAUTH_STATE;
			lim_log(mac_ctx, LOG1, FL(
				"Rcvd SME_DEAUTH_REQ while in SME_WT_DEAUTH_STATE"));
			break;
		default:
			/*
			 * STA is not in a state to deauthenticate with
			 * peer. Log error and send response to host.
			 */
			lim_log(mac_ctx, LOGE, FL(
				"received unexp SME_DEAUTH_REQ in state %X"),
				session_entry->limSmeState);
			lim_print_sme_state(mac_ctx, LOGE,
					    session_entry->limSmeState);

			if (mac_ctx->lim.gLimRspReqd) {
				mac_ctx->lim.gLimRspReqd = false;

				ret_code = eSIR_SME_STA_NOT_AUTHENTICATED;
				deauth_trigger = eLIM_HOST_DEAUTH;

			/*
			 * here we received deauth request from AP so sme state
			 * is eLIM_SME_WT_DEAUTH_STATE.if we have ISSUED
			 * delSta then mlm state should be
			 * eLIM_MLM_WT_DEL_STA_RSP_STATE and ifwe got delBSS
			 * rsp then mlm state should be eLIM_MLM_IDLE_STATE
			 * so the below condition captures the state where
			 * delSta not done and firmware still in
			 * connected state.
			 */
			if (session_entry->limSmeState ==
					eLIM_SME_WT_DEAUTH_STATE &&
					session_entry->limMlmState !=
					eLIM_MLM_IDLE_STATE &&
					session_entry->limMlmState !=
					eLIM_MLM_WT_DEL_STA_RSP_STATE)
				ret_code = eSIR_SME_DEAUTH_STATUS;
				goto send_deauth;
			}
			return;
		}
		break;

	case eLIM_STA_IN_IBSS_ROLE:
		lim_log(mac_ctx, LOGE, FL("Deauth not allowed in IBSS"));
		if (mac_ctx->lim.gLimRspReqd) {
			mac_ctx->lim.gLimRspReqd = false;
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			deauth_trigger = eLIM_HOST_DEAUTH;
			goto send_deauth;
		}
		return;
	case eLIM_AP_ROLE:
		break;
	default:
		lim_log(mac_ctx, LOGE,
			FL("received unexpected SME_DEAUTH_REQ for role %X"),
			GET_LIM_SYSTEM_ROLE(session_entry));
		if (mac_ctx->lim.gLimRspReqd) {
			mac_ctx->lim.gLimRspReqd = false;
			ret_code = eSIR_SME_INVALID_PARAMETERS;
			deauth_trigger = eLIM_HOST_DEAUTH;
			goto send_deauth;
		}
		return;
	} /* end switch (mac_ctx->lim.gLimSystemRole) */

	if (sme_deauth_req.reasonCode == eLIM_LINK_MONITORING_DEAUTH) {
		/* Deauthentication is triggered by Link Monitoring */
		lim_log(mac_ctx, LOG1, FL("** Lost link with AP **"));
		deauth_trigger = eLIM_LINK_MONITORING_DEAUTH;
		reason_code = eSIR_MAC_UNSPEC_FAILURE_REASON;
	} else {
		deauth_trigger = eLIM_HOST_DEAUTH;
		reason_code = sme_deauth_req.reasonCode;
	}

	/* Trigger Deauthentication frame to peer MAC entity */
	mlm_deauth_req = cdf_mem_malloc(sizeof(tLimMlmDeauthReq));
	if (NULL == mlm_deauth_req) {
		lim_log(mac_ctx, LOGP,
			FL("call to AllocateMemory failed for mlmDeauthReq"));
		if (mac_ctx->lim.gLimRspReqd) {
			mac_ctx->lim.gLimRspReqd = false;
			ret_code = eSIR_SME_RESOURCES_UNAVAILABLE;
			deauth_trigger = eLIM_HOST_DEAUTH;
			goto send_deauth;
		}
		return;
	}

	cdf_mem_copy((uint8_t *) &mlm_deauth_req->peerMacAddr,
		     (uint8_t *) &sme_deauth_req.peerMacAddr,
		     sizeof(tSirMacAddr));

	mlm_deauth_req->reasonCode = reason_code;
	mlm_deauth_req->deauthTrigger = deauth_trigger;

	/* Update PE session Id */
	mlm_deauth_req->sessionId = session_id;

	lim_post_mlm_message(mac_ctx, LIM_MLM_DEAUTH_REQ,
			(uint32_t *)mlm_deauth_req);
	return;

send_deauth:
	lim_send_sme_deauth_ntf(mac_ctx, sme_deauth_req.peerMacAddr, ret_code,
			deauth_trigger, 1, sme_session_id, sme_transaction_id);
}

/**
 * __lim_process_sme_set_context_req()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process SME_SETCONTEXT_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */

static void
__lim_process_sme_set_context_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tpSirSmeSetContextReq set_context_req;
	tLimMlmSetKeysReq *mlm_set_key_req;
	tpPESession session_entry;
	uint8_t session_id;      /* PE sessionID */
	uint8_t sme_session_id;
	uint16_t sme_transaction_id;

	lim_log(mac_ctx, LOG1, FL("received SETCONTEXT_REQ message"));

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	set_context_req = cdf_mem_malloc(sizeof(struct sSirSmeSetContextReq));
	if (NULL == set_context_req) {
		lim_log(mac_ctx, LOGP, FL(
			"call to AllocateMemory failed for set_context_req"));
		return;
	}
	cdf_mem_copy(set_context_req, msg_buf,
			sizeof(struct sSirSmeSetContextReq));
	sme_session_id = set_context_req->sessionId;
	sme_transaction_id = set_context_req->transactionId;

	if ((!lim_is_sme_set_context_req_valid(mac_ctx, set_context_req))) {
		lim_log(mac_ctx, LOGW,
			FL("received invalid SME_SETCONTEXT_REQ message"));
		goto end;
	}

	if (set_context_req->keyMaterial.numKeys >
			SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS) {
		lim_log(mac_ctx, LOGE, FL(
			"numKeys:%d is more than SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS"),
					set_context_req->keyMaterial.numKeys);
		lim_send_sme_set_context_rsp(mac_ctx,
				set_context_req->peerMacAddr, 1,
				eSIR_SME_INVALID_PARAMETERS, NULL,
				sme_session_id, sme_transaction_id);
		goto end;
	}

	session_entry = pe_find_session_by_bssid(mac_ctx,
			set_context_req->bssId, &session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGW,
			FL("Session does not exist for given BSSID"));
		lim_send_sme_set_context_rsp(mac_ctx,
				set_context_req->peerMacAddr, 1,
				eSIR_SME_INVALID_PARAMETERS, NULL,
				sme_session_id, sme_transaction_id);
		goto end;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_SETCONTEXT_REQ_EVENT,
			      session_entry, 0, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	if (((LIM_IS_STA_ROLE(session_entry) ||
	    LIM_IS_BT_AMP_STA_ROLE(session_entry)) &&
	    (session_entry->limSmeState == eLIM_SME_LINK_EST_STATE)) ||
	    ((LIM_IS_IBSS_ROLE(session_entry) ||
	    LIM_IS_AP_ROLE(session_entry) ||
	    LIM_IS_BT_AMP_AP_ROLE(session_entry)) &&
	    (session_entry->limSmeState == eLIM_SME_NORMAL_STATE))) {
		/* Trigger MLM_SETKEYS_REQ */
		mlm_set_key_req = cdf_mem_malloc(sizeof(tLimMlmSetKeysReq));
		if (NULL == mlm_set_key_req) {
			lim_log(mac_ctx, LOGP, FL(
					"mem alloc failed for mlmSetKeysReq"));
			goto end;
		}
		mlm_set_key_req->edType = set_context_req->keyMaterial.edType;
		mlm_set_key_req->numKeys =
			set_context_req->keyMaterial.numKeys;
		if (mlm_set_key_req->numKeys >
				SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS) {
			lim_log(mac_ctx, LOGP, FL(
				"no.of keys exceeded max num of default keys limit"));
			goto end;
		}
		cdf_mem_copy((uint8_t *) &mlm_set_key_req->peerMacAddr,
			     (uint8_t *) &set_context_req->peerMacAddr,
			     sizeof(tSirMacAddr));

		cdf_mem_copy((uint8_t *) &mlm_set_key_req->key,
			     (uint8_t *) &set_context_req->keyMaterial.key,
			     sizeof(tSirKeys) *
			     (mlm_set_key_req->numKeys ? mlm_set_key_req->
			      numKeys : 1));

		mlm_set_key_req->sessionId = session_id;
		mlm_set_key_req->smesessionId = sme_session_id;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
		lim_log(mac_ctx, LOG1, FL(
			"received SETCONTEXT_REQ message sessionId=%d"),
					mlm_set_key_req->sessionId);
#endif

		if (((set_context_req->keyMaterial.edType == eSIR_ED_WEP40) ||
		    (set_context_req->keyMaterial.edType == eSIR_ED_WEP104)) &&
		    LIM_IS_AP_ROLE(session_entry)) {
			if (set_context_req->keyMaterial.key[0].keyLength) {
				uint8_t key_id;
				key_id =
					set_context_req->keyMaterial.key[0].keyId;
				cdf_mem_copy((uint8_t *)
					&session_entry->WEPKeyMaterial[key_id],
					(uint8_t *) &set_context_req->keyMaterial,
					sizeof(tSirKeyMaterial));
			} else {
				uint32_t i;
				for (i = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
				     i++) {
					cdf_mem_copy((uint8_t *)
						&mlm_set_key_req->key[i],
						(uint8_t *)session_entry->WEPKeyMaterial[i].key,
						sizeof(tSirKeys));
				}
			}
		}
		lim_post_mlm_message(mac_ctx, LIM_MLM_SETKEYS_REQ,
				     (uint32_t *) mlm_set_key_req);
	} else {
		lim_log(mac_ctx, LOGE, FL(
			"rcvd unexpected SME_SETCONTEXT_REQ for role %d, state=%X"),
				GET_LIM_SYSTEM_ROLE(session_entry),
				session_entry->limSmeState);
		lim_print_sme_state(mac_ctx, LOGE, session_entry->limSmeState);

		lim_send_sme_set_context_rsp(mac_ctx,
				set_context_req->peerMacAddr, 1,
				eSIR_SME_UNEXPECTED_REQ_RESULT_CODE,
				session_entry, sme_session_id,
				sme_transaction_id);
	}
end:
	cdf_mem_free(set_context_req);
	return;
}

/**
 * lim_process_sme_get_assoc_sta_info() - process sme assoc sta req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process SME_GET_ASSOC_STAS_REQ message
 * from HDD or upper layer application.
 *
 * Return: None
 */

void lim_process_sme_get_assoc_sta_info(tpAniSirGlobal mac_ctx,
					uint32_t *msg_buf)
{
	tSirSmeGetAssocSTAsReq get_assoc_stas_req;
	tpDphHashNode sta_ds = NULL;
	tpPESession session_entry = NULL;
	tSap_Event sap_event;
	tpWLAN_SAPEventCB sap_event_cb = NULL;
	tpSap_AssocMacAddr assoc_sta_tmp = NULL;
	uint8_t session_id = CSR_SESSION_ID_INVALID;
	uint8_t assoc_id = 0;
	uint8_t sta_cnt = 0;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	cdf_mem_copy(&get_assoc_stas_req, msg_buf,
				sizeof(struct sSirSmeGetAssocSTAsReq));
	/*
	 * Get Associated stations from PE.
	 * Find PE session Entry
	 */
	session_entry = pe_find_session_by_bssid(mac_ctx,
			get_assoc_stas_req.bssId,
			&session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given bssId"));
		goto lim_assoc_sta_end;
	}

	if (!LIM_IS_AP_ROLE(session_entry)) {
		lim_log(mac_ctx, LOGE, FL(
			"Received unexpected message in state %X, in role %X"),
			session_entry->limSmeState,
			GET_LIM_SYSTEM_ROLE(session_entry));
		goto lim_assoc_sta_end;
	}
	/* Retrieve values obtained in the request message */
	sap_event_cb = (tpWLAN_SAPEventCB)get_assoc_stas_req.pSapEventCallback;
	assoc_sta_tmp = (tpSap_AssocMacAddr)get_assoc_stas_req.pAssocStasArray;

	if (NULL == assoc_sta_tmp)
		goto lim_assoc_sta_end;
	for (assoc_id = 0; assoc_id < session_entry->dph.dphHashTable.size;
		assoc_id++) {
		sta_ds = dph_get_hash_entry(mac_ctx, assoc_id,
				&session_entry->dph.dphHashTable);
		if (NULL == sta_ds)
			continue;
		if (sta_ds->valid) {
			cdf_mem_copy((uint8_t *) &assoc_sta_tmp->staMac,
					(uint8_t *) &sta_ds->staAddr,
					 CDF_MAC_ADDR_SIZE);
			assoc_sta_tmp->assocId = (uint8_t) sta_ds->assocId;
			assoc_sta_tmp->staId = (uint8_t) sta_ds->staIndex;

			cdf_mem_copy((uint8_t *)&assoc_sta_tmp->supportedRates,
				     (uint8_t *)&sta_ds->supportedRates,
				     sizeof(tSirSupportedRates));
			assoc_sta_tmp->ShortGI40Mhz = sta_ds->htShortGI40Mhz;
			assoc_sta_tmp->ShortGI20Mhz = sta_ds->htShortGI20Mhz;
			assoc_sta_tmp->Support40Mhz =
				sta_ds->htDsssCckRate40MHzSupport;

			lim_log(mac_ctx, LOG1, FL("dph Station Number = %d"),
				sta_cnt + 1);
			lim_log(mac_ctx, LOG1, FL("MAC = " MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(sta_ds->staAddr));
			lim_log(mac_ctx, LOG1, FL("Association Id = %d"),
				sta_ds->assocId);
			lim_log(mac_ctx, LOG1, FL("Station Index = %d"),
				sta_ds->staIndex);
			assoc_sta_tmp++;
			sta_cnt++;
		}
	}
lim_assoc_sta_end:
	/*
	 * Call hdd callback with sap event to send the list of
	 * associated stations from PE
	 */
	if (sap_event_cb != NULL) {
		sap_event.sapHddEventCode = eSAP_ASSOC_STA_CALLBACK_EVENT;
		sap_event.sapevt.sapAssocStaListEvent.module =
			CDF_MODULE_ID_PE;
		sap_event.sapevt.sapAssocStaListEvent.noOfAssocSta = sta_cnt;
		sap_event.sapevt.sapAssocStaListEvent.pAssocStas =
			(tpSap_AssocMacAddr)get_assoc_stas_req.pAssocStasArray;
		sap_event_cb(&sap_event, get_assoc_stas_req.pUsrContext);
	}
}

/**
 * lim_process_sme_get_wpspbc_sessions - process sme get wpspbc req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to WPS PBC overlap query message
 *
 * This function parses get WPS PBC overlap information
 * message and call callback to pass WPS PBC overlap
 * information back to hdd.
 *
 * Return: None
 */
void lim_process_sme_get_wpspbc_sessions(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tSirSmeGetWPSPBCSessionsReq get_wps_pbc_sessions_req;
	tpPESession session_entry = NULL;
	tSap_Event sap_event;
	tpWLAN_SAPEventCB sap_event_cb = NULL;
	uint8_t session_id = CSR_SESSION_ID_INVALID;
	tSirMacAddr zero_mac = { 0, 0, 0, 0, 0, 0 };
	tSap_GetWPSPBCSessionEvent *sap_get_wpspbc_event;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	sap_get_wpspbc_event = &sap_event.sapevt.sapGetWPSPBCSessionEvent;
	sap_get_wpspbc_event->status = CDF_STATUS_E_FAULT;

	cdf_mem_copy(&get_wps_pbc_sessions_req, msg_buf,
			sizeof(struct sSirSmeGetWPSPBCSessionsReq));
	/*
	 * Get Associated stations from PE
	 * Find PE session Entry
	 */
	session_entry = pe_find_session_by_bssid(mac_ctx,
			get_wps_pbc_sessions_req.bssId, &session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given bssId"));
		goto lim_get_wpspbc_sessions_end;
	}

	if (!LIM_IS_AP_ROLE(session_entry)) {
		lim_log(mac_ctx, LOGE,
			FL("Received unexpected message in role %X"),
			GET_LIM_SYSTEM_ROLE(session_entry));
		goto lim_get_wpspbc_sessions_end;
	}
	/*
	 * Call hdd callback with sap event to send the
	 * WPS PBC overlap information
	 */
	sap_event.sapHddEventCode = eSAP_GET_WPSPBC_SESSION_EVENT;
	sap_get_wpspbc_event->module = CDF_MODULE_ID_PE;

	if (cdf_mem_compare(zero_mac, get_wps_pbc_sessions_req.pRemoveMac,
				sizeof(tSirMacAddr))) {
		lim_get_wpspbc_sessions(mac_ctx,
				sap_get_wpspbc_event->addr.bytes,
				sap_get_wpspbc_event->UUID_E,
				&sap_get_wpspbc_event->wpsPBCOverlap,
				session_entry);
	} else {
		lim_remove_pbc_sessions(mac_ctx,
				get_wps_pbc_sessions_req.pRemoveMac,
				session_entry);
		/* don't have to inform the HDD/Host */
		return;
	}

	lim_log(mac_ctx, LOGE, FL("wpsPBCOverlap %d"),
				sap_get_wpspbc_event->wpsPBCOverlap);
	lim_print_mac_addr(mac_ctx,
				sap_get_wpspbc_event->addr.bytes, LOG4);

	sap_get_wpspbc_event->status = CDF_STATUS_SUCCESS;

lim_get_wpspbc_sessions_end:
	sap_event_cb =
		(tpWLAN_SAPEventCB)get_wps_pbc_sessions_req.pSapEventCallback;
	if (NULL != sap_event_cb)
		sap_event_cb(&sap_event, get_wps_pbc_sessions_req.pUsrContext);
}

/**
 * __lim_counter_measures()
 *
 * FUNCTION:
 * This function is called to "implement" MIC counter measure
 * and is *temporary* only
 *
 * LOGIC: on AP, disassoc all STA associated thru TKIP,
 * we don't do the proper STA disassoc sequence since the
 * BSS will be stoped anyway
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @return None
 */

static void __lim_counter_measures(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	tSirMacAddr mac = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	if (LIM_IS_AP_ROLE(psessionEntry) ||
	    LIM_IS_BT_AMP_AP_ROLE(psessionEntry) ||
	    LIM_IS_BT_AMP_STA_ROLE(psessionEntry))
		lim_send_disassoc_mgmt_frame(pMac, eSIR_MAC_MIC_FAILURE_REASON,
					     mac, psessionEntry, false);
};

void lim_process_tkip_counter_measures(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirSmeTkipCntrMeasReq tkipCntrMeasReq;
	tpPESession psessionEntry;
	uint8_t sessionId;      /* PE sessionId */

	cdf_mem_copy(&tkipCntrMeasReq, pMsgBuf,
			sizeof(struct sSirSmeTkipCntrMeasReq));

	psessionEntry = pe_find_session_by_bssid(pMac,
				tkipCntrMeasReq.bssId.bytes, &sessionId);
	if (NULL == psessionEntry) {
		lim_log(pMac, LOGE,
			FL("session does not exist for given BSSID "));
		return;
	}

	if (tkipCntrMeasReq.bEnable)
		__lim_counter_measures(pMac, psessionEntry);

	psessionEntry->bTkipCntrMeasActive = tkipCntrMeasReq.bEnable;
}

static void
__lim_handle_sme_stop_bss_request(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirSmeStopBssReq stopBssReq;
	tSirRetStatus status;
	tLimSmeStates prevState;
	tpPESession psessionEntry;
	uint8_t smesessionId;
	uint8_t sessionId;
	uint16_t smetransactionId;
	uint8_t i = 0;
	tpDphHashNode pStaDs = NULL;

	cdf_mem_copy(&stopBssReq, pMsgBuf, sizeof(tSirSmeStopBssReq));
	smesessionId = stopBssReq.sessionId;
	smetransactionId = stopBssReq.transactionId;

	if (!lim_is_sme_stop_bss_req_valid(pMsgBuf)) {
		PELOGW(lim_log(pMac, LOGW,
		       FL("received invalid SME_STOP_BSS_REQ message"));)
		/* Send Stop BSS response to host */
		lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP,
				 eSIR_SME_INVALID_PARAMETERS, smesessionId,
				 smetransactionId);
		return;
	}

	psessionEntry = pe_find_session_by_bssid(pMac,
				stopBssReq.bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGW,
			FL("session does not exist for given BSSID "));
		lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP,
				 eSIR_SME_INVALID_PARAMETERS, smesessionId,
				 smetransactionId);
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_STOP_BSS_REQ_EVENT, psessionEntry,
			      0, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	if (psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE ||    /* Added For BT -AMP Support */
	    LIM_IS_STA_ROLE(psessionEntry)) {
		/**
		 * Should not have received STOP_BSS_REQ in states
		 * other than 'normal' state or on STA in Infrastructure
		 * mode. Log error and return response to host.
		 */
		lim_log(pMac, LOGE,
			FL
				("received unexpected SME_STOP_BSS_REQ in state %X, for role %d"),
			psessionEntry->limSmeState,
			GET_LIM_SYSTEM_ROLE(psessionEntry));
		lim_print_sme_state(pMac, LOGE, psessionEntry->limSmeState);
		/* / Send Stop BSS response to host */
		lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP,
				 eSIR_SME_UNEXPECTED_REQ_RESULT_CODE, smesessionId,
				 smetransactionId);
		return;
	}

	if (LIM_IS_AP_ROLE(psessionEntry))
		lim_wpspbc_close(pMac, psessionEntry);

	lim_log(pMac, LOGW,
		FL("RECEIVED STOP_BSS_REQ with reason code=%d"),
		stopBssReq.reasonCode);

	prevState = psessionEntry->limSmeState;

	psessionEntry->limSmeState = eLIM_SME_IDLE_STATE;
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId,
		       psessionEntry->limSmeState));

	/* Update SME session Id and Transaction Id */
	psessionEntry->smeSessionId = smesessionId;
	psessionEntry->transactionId = smetransactionId;

	/* BTAMP_STA and STA_IN_IBSS should NOT send Disassoc frame */
	if (!LIM_IS_IBSS_ROLE(psessionEntry) &&
	    !LIM_IS_BT_AMP_STA_ROLE(psessionEntry)) {
		tSirMacAddr bcAddr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		if (stopBssReq.reasonCode == eSIR_SME_MIC_COUNTER_MEASURES)
			/* Send disassoc all stations associated thru TKIP */
			__lim_counter_measures(pMac, psessionEntry);
		else
			lim_send_disassoc_mgmt_frame(pMac,
						     eSIR_MAC_DEAUTH_LEAVING_BSS_REASON,
						     bcAddr, psessionEntry, false);
	}

	/* Free the buffer allocated in START_BSS_REQ */
	cdf_mem_free(psessionEntry->addIeParams.probeRespData_buff);
	psessionEntry->addIeParams.probeRespDataLen = 0;
	psessionEntry->addIeParams.probeRespData_buff = NULL;

	cdf_mem_free(psessionEntry->addIeParams.assocRespData_buff);
	psessionEntry->addIeParams.assocRespDataLen = 0;
	psessionEntry->addIeParams.assocRespData_buff = NULL;

	cdf_mem_free(psessionEntry->addIeParams.probeRespBCNData_buff);
	psessionEntry->addIeParams.probeRespBCNDataLen = 0;
	psessionEntry->addIeParams.probeRespBCNData_buff = NULL;

	/* lim_del_bss is also called as part of coalescing, when we send DEL BSS followed by Add Bss msg. */
	pMac->lim.gLimIbssCoalescingHappened = false;

	for (i = 1; i < pMac->lim.gLimAssocStaLimit; i++) {
		pStaDs =
			dph_get_hash_entry(pMac, i, &psessionEntry->dph.dphHashTable);
		if (NULL == pStaDs)
			continue;
		status = lim_del_sta(pMac, pStaDs, false, psessionEntry);
		if (eSIR_SUCCESS == status) {
			lim_delete_dph_hash_entry(pMac, pStaDs->staAddr,
						  pStaDs->assocId, psessionEntry);
			lim_release_peer_idx(pMac, pStaDs->assocId, psessionEntry);
		} else {
			lim_log(pMac, LOGE,
				FL("lim_del_sta failed with Status : %d"), status);
			CDF_ASSERT(0);
		}
	}
	/* send a delBss to HAL and wait for a response */
	status = lim_del_bss(pMac, NULL, psessionEntry->bssIdx, psessionEntry);

	if (status != eSIR_SUCCESS) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("delBss failed for bss %d"),
			       psessionEntry->bssIdx);
		       )
		psessionEntry->limSmeState = prevState;

		MTRACE(mac_trace
			       (pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId,
			       psessionEntry->limSmeState));

		lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP,
				 eSIR_SME_STOP_BSS_FAILURE, smesessionId,
				 smetransactionId);
	}
}

/**
 * __lim_process_sme_stop_bss_req() - Process STOP_BSS from SME
 * @pMac: Global MAC context
 * @pMsg: Message from SME
 *
 * Wrapper for the function __lim_handle_sme_stop_bss_request
 * This message will be defered until softmac come out of
 * scan mode. Message should be handled even if we have
 * detected radar in the current operating channel.
 *
 * Return: true - If we consumed the buffer
 *         false - If have defered the message.
 */

static bool __lim_process_sme_stop_bss_req(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	if (__lim_is_defered_msg_for_learn(pMac, pMsg)) {
		/**
		 * If message defered, buffer is not consumed yet.
		 * So return false
		 */
		return false;
	}
	__lim_handle_sme_stop_bss_request(pMac, (uint32_t *) pMsg->bodyptr);
	return true;
} /*** end __lim_process_sme_stop_bss_req() ***/

void lim_process_sme_del_bss_rsp(tpAniSirGlobal pMac,
				 uint32_t body, tpPESession psessionEntry)
{

	(void)body;
	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	lim_ibss_delete(pMac, psessionEntry);
	dph_hash_table_class_init(pMac, &psessionEntry->dph.dphHashTable);
	lim_delete_pre_auth_list(pMac);
	lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_SUCCESS,
			 psessionEntry->smeSessionId,
			 psessionEntry->transactionId);
	return;
}

/**
 * __lim_process_sme_assoc_cnf_new() - process sme assoc/reassoc cnf
 *
 * @mac_ctx: pointer to mac context
 * @msg_type: message type
 * @msg_buf: pointer to the SME message buffer
 *
 * This function handles SME_ASSOC_CNF/SME_REASSOC_CNF
 * in BTAMP AP.
 *
 * Return: None
 */

void __lim_process_sme_assoc_cnf_new(tpAniSirGlobal mac_ctx, uint32_t msg_type,
				uint32_t *msg_buf)
{
	tSirSmeAssocCnf assoc_cnf;
	tpDphHashNode sta_ds = NULL;
	tpPESession session_entry = NULL;
	uint8_t session_id;
	tpSirAssocReq assoc_req;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("msg_buf is NULL "));
		goto end;
	}

	cdf_mem_copy(&assoc_cnf, msg_buf, sizeof(struct sSirSmeAssocCnf));
	if (!__lim_is_sme_assoc_cnf_valid(&assoc_cnf)) {
		lim_log(mac_ctx, LOGE,
			FL("Received invalid SME_RE(ASSOC)_CNF message "));
		goto end;
	}

	session_entry = pe_find_session_by_bssid(mac_ctx, assoc_cnf.bssId,
			&session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given bssId"));
		goto end;
	}

	if ((!LIM_IS_AP_ROLE(session_entry) &&
	    !LIM_IS_BT_AMP_AP_ROLE(session_entry)) ||
	    ((session_entry->limSmeState != eLIM_SME_NORMAL_STATE) &&
	    (session_entry->limSmeState !=
			eLIM_SME_NORMAL_CHANNEL_SCAN_STATE))) {
		lim_log(mac_ctx, LOGE, FL(
			"Rcvd unexpected msg %X in state %X, in role %X"),
			msg_type, session_entry->limSmeState,
			GET_LIM_SYSTEM_ROLE(session_entry));
		goto end;
	}
	sta_ds = dph_get_hash_entry(mac_ctx, assoc_cnf.aid,
			&session_entry->dph.dphHashTable);
	if (sta_ds == NULL) {
		lim_log(mac_ctx, LOGE, FL(
			"Rcvd invalid msg %X due to no STA ctx, aid %d, peer "),
				msg_type, assoc_cnf.aid);
		lim_print_mac_addr(mac_ctx, assoc_cnf.peerMacAddr, LOG1);

		/*
		 * send a DISASSOC_IND message to WSM to make sure
		 * the state in WSM and LIM is the same
		 */
		lim_send_sme_disassoc_ntf(mac_ctx, assoc_cnf.peerMacAddr,
				eSIR_SME_STA_NOT_ASSOCIATED,
				eLIM_PEER_ENTITY_DISASSOC, assoc_cnf.aid,
				session_entry->smeSessionId,
				session_entry->transactionId,
				session_entry);
		goto end;
	}
	if (!cdf_mem_compare((uint8_t *)sta_ds->staAddr,
				(uint8_t *) assoc_cnf.peerMacAddr,
				sizeof(tSirMacAddr))) {
		lim_log(mac_ctx, LOG1, FL(
				"peerMacAddr mismatched for aid %d, peer "),
				assoc_cnf.aid);
		lim_print_mac_addr(mac_ctx, assoc_cnf.peerMacAddr, LOG1);
		goto end;
	}

	if ((sta_ds->mlmStaContext.mlmState != eLIM_MLM_WT_ASSOC_CNF_STATE) ||
		((sta_ds->mlmStaContext.subType == LIM_ASSOC) &&
		 (msg_type != eWNI_SME_ASSOC_CNF)) ||
		((sta_ds->mlmStaContext.subType == LIM_REASSOC) &&
		 (msg_type != eWNI_SME_ASSOC_CNF))) {
		lim_log(mac_ctx, LOG1, FL(
			"not in MLM_WT_ASSOC_CNF_STATE, for aid %d, peer"
			"StaD mlmState : %d"),
			assoc_cnf.aid, sta_ds->mlmStaContext.mlmState);
		lim_print_mac_addr(mac_ctx, assoc_cnf.peerMacAddr, LOG1);
		goto end;
	}
	/*
	 * Deactivate/delet CNF_WAIT timer since ASSOC_CNF
	 * has been received
	 */
	lim_log(mac_ctx, LOG1, FL("Received SME_ASSOC_CNF. Delete Timer"));
	lim_deactivate_and_change_per_sta_id_timer(mac_ctx,
			eLIM_CNF_WAIT_TIMER, sta_ds->assocId);

	if (assoc_cnf.statusCode == eSIR_SME_SUCCESS) {
		/*
		 * In BTAMP-AP, PE already finished the WMA_ADD_STA sequence
		 * when it had received Assoc Request frame. Now, PE just needs
		 * to send association rsp frame to the requesting BTAMP-STA.
		 */
		sta_ds->mlmStaContext.mlmState =
			eLIM_MLM_LINK_ESTABLISHED_STATE;
		lim_log(mac_ctx, LOG1,
			FL("sending Assoc Rsp frame to STA (assoc id=%d) "),
			sta_ds->assocId);
		lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_SUCCESS,
					sta_ds->assocId, sta_ds->staAddr,
					sta_ds->mlmStaContext.subType, sta_ds,
					session_entry);
		goto end;
	} else {
		/*
		 * SME_ASSOC_CNF status is non-success, so STA is not allowed
		 * to be associated since the HAL sta entry is created for
		 * denied STA we need to remove this HAL entry.
		 * So to do that set updateContext to 1
		 */
		if (!sta_ds->mlmStaContext.updateContext)
			sta_ds->mlmStaContext.updateContext = 1;
		lim_log(mac_ctx, LOG1,
			FL("Recv Assoc Cnf, status Code : %d(assoc id=%d) "),
			assoc_cnf.statusCode, sta_ds->assocId);
		lim_reject_association(mac_ctx, sta_ds->staAddr,
				       sta_ds->mlmStaContext.subType,
				       true, sta_ds->mlmStaContext.authType,
				       sta_ds->assocId, true,
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       session_entry);
	}
end:
	if (((session_entry != NULL) && (sta_ds != NULL)) &&
		(session_entry->parsedAssocReq[sta_ds->assocId] != NULL)) {
		assoc_req = (tpSirAssocReq)
			session_entry->parsedAssocReq[sta_ds->assocId];
		if (assoc_req->assocReqFrame) {
			cdf_mem_free(assoc_req->assocReqFrame);
			assoc_req->assocReqFrame = NULL;
		}
		cdf_mem_free(session_entry->parsedAssocReq[sta_ds->assocId]);
		session_entry->parsedAssocReq[sta_ds->assocId] = NULL;
	}
}

static void __lim_process_sme_addts_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpDphHashNode pStaDs;
	tSirMacAddr peerMac;
	tpSirAddtsReq pSirAddts;
	uint32_t timeout;
	tpPESession psessionEntry;
	uint8_t sessionId;      /* PE sessionId */
	uint8_t smesessionId;
	uint16_t smetransactionId;

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	lim_get_session_info(pMac, (uint8_t *) pMsgBuf, &smesessionId,
			     &smetransactionId);

	pSirAddts = (tpSirAddtsReq) pMsgBuf;

	psessionEntry = pe_find_session_by_bssid(pMac,
				pSirAddts->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE, "Session Does not exist for given bssId");
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_ADDTS_REQ_EVENT, psessionEntry, 0,
			      0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	/* if sta
	 *  - verify assoc state
	 *  - send addts request to ap
	 *  - wait for addts response from ap
	 * if ap, just ignore with error log
	 */
	PELOG1(lim_log(pMac, LOG1,
		       FL("Received SME_ADDTS_REQ (TSid %d, UP %d)"),
		       pSirAddts->req.tspec.tsinfo.traffic.tsid,
		       pSirAddts->req.tspec.tsinfo.traffic.userPrio);
	       )

	if (!LIM_IS_STA_ROLE(psessionEntry) &&
	    !LIM_IS_BT_AMP_STA_ROLE(psessionEntry)) {
		PELOGE(lim_log(pMac, LOGE, "AddTs received on AP - ignoring");)
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	pStaDs =
		dph_get_hash_entry(pMac, DPH_STA_HASH_INDEX_PEER,
				   &psessionEntry->dph.dphHashTable);

	if (pStaDs == NULL) {
		PELOGE(lim_log
			       (pMac, LOGE, "Cannot find AP context for addts req");
		       )
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	if ((!pStaDs->valid) || (pStaDs->mlmStaContext.mlmState !=
	    eLIM_MLM_LINK_ESTABLISHED_STATE)) {
		lim_log(pMac, LOGE, "AddTs received in invalid MLM state");
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	pSirAddts->req.wsmTspecPresent = 0;
	pSirAddts->req.wmeTspecPresent = 0;
	pSirAddts->req.lleTspecPresent = 0;

	if ((pStaDs->wsmEnabled) &&
	    (pSirAddts->req.tspec.tsinfo.traffic.accessPolicy !=
	     SIR_MAC_ACCESSPOLICY_EDCA))
		pSirAddts->req.wsmTspecPresent = 1;
	else if (pStaDs->wmeEnabled)
		pSirAddts->req.wmeTspecPresent = 1;
	else if (pStaDs->lleEnabled)
		pSirAddts->req.lleTspecPresent = 1;
	else {
		PELOGW(lim_log
			       (pMac, LOGW, FL("ADDTS_REQ ignore - qos is disabled"));
		       )
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	if ((psessionEntry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
	    (psessionEntry->limSmeState != eLIM_SME_LINK_EST_STATE)) {
		lim_log(pMac, LOGE,
			"AddTs received in invalid LIMsme state (%d)",
			psessionEntry->limSmeState);
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	if (pMac->lim.gLimAddtsSent) {
		lim_log(pMac, LOGE,
			"Addts (token %d, tsid %d, up %d) is still pending",
			pMac->lim.gLimAddtsReq.req.dialogToken,
			pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.tsid,
			pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.
			userPrio);
		lim_send_sme_addts_rsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE,
				       psessionEntry, pSirAddts->req.tspec,
				       smesessionId, smetransactionId);
		return;
	}

	sir_copy_mac_addr(peerMac, psessionEntry->bssId);

	/* save the addts request */
	pMac->lim.gLimAddtsSent = true;
	cdf_mem_copy((uint8_t *) &pMac->lim.gLimAddtsReq,
		     (uint8_t *) pSirAddts, sizeof(tSirAddtsReq));

	/* ship out the message now */
	lim_send_addts_req_action_frame(pMac, peerMac, &pSirAddts->req,
					psessionEntry);
	PELOG1(lim_log(pMac, LOG1, "Sent ADDTS request");)
	/* start a timer to wait for the response */
	if (pSirAddts->timeout)
		timeout = pSirAddts->timeout;
	else if (wlan_cfg_get_int(pMac, WNI_CFG_ADDTS_RSP_TIMEOUT, &timeout) !=
		 eSIR_SUCCESS) {
		lim_log(pMac, LOGP,
			FL("Unable to get Cfg param %d (Addts Rsp Timeout)"),
			WNI_CFG_ADDTS_RSP_TIMEOUT);
		return;
	}

	timeout = SYS_MS_TO_TICKS(timeout);
	if (tx_timer_change(&pMac->lim.limTimers.gLimAddtsRspTimer, timeout, 0)
	    != TX_SUCCESS) {
		lim_log(pMac, LOGP, FL("AddtsRsp timer change failed!"));
		return;
	}
	pMac->lim.gLimAddtsRspTimerCount++;
	if (tx_timer_change_context(&pMac->lim.limTimers.gLimAddtsRspTimer,
				    pMac->lim.gLimAddtsRspTimerCount) !=
	    TX_SUCCESS) {
		lim_log(pMac, LOGP, FL("AddtsRsp timer change failed!"));
		return;
	}
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId,
		       eLIM_ADDTS_RSP_TIMER));

	/* add the sessionId to the timer object */
	pMac->lim.limTimers.gLimAddtsRspTimer.sessionId = sessionId;
	if (tx_timer_activate(&pMac->lim.limTimers.gLimAddtsRspTimer) !=
	    TX_SUCCESS) {
		lim_log(pMac, LOGP, FL("AddtsRsp timer activation failed!"));
		return;
	}
	return;
}

static void __lim_process_sme_delts_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirMacAddr peerMacAddr;
	uint8_t ac;
	tSirMacTSInfo *pTsinfo;
	tpSirDeltsReq pDeltsReq = (tpSirDeltsReq) pMsgBuf;
	tpDphHashNode pStaDs = NULL;
	tpPESession psessionEntry;
	uint8_t sessionId;
	uint32_t status = eSIR_SUCCESS;
	uint8_t smesessionId;
	uint16_t smetransactionId;

	lim_get_session_info(pMac, (uint8_t *) pMsgBuf, &smesessionId,
			     &smetransactionId);

	psessionEntry = pe_find_session_by_bssid(pMac,
				pDeltsReq->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE, "Session Does not exist for given bssId");
		status = eSIR_FAILURE;
		goto end;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DELTS_REQ_EVENT, psessionEntry, 0,
			      0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	if (eSIR_SUCCESS !=
	    lim_validate_delts_req(pMac, pDeltsReq, peerMacAddr, psessionEntry)) {
		PELOGE(lim_log(pMac, LOGE, FL("lim_validate_delts_req failed"));)
		status = eSIR_FAILURE;
		lim_send_sme_delts_rsp(pMac, pDeltsReq, eSIR_FAILURE, psessionEntry,
				       smesessionId, smetransactionId);
		return;
	}

	lim_log(pMac, LOG1,
		FL("Sent DELTS request to station with assocId = %d MacAddr = "
		MAC_ADDRESS_STR),
		pDeltsReq->aid, MAC_ADDR_ARRAY(peerMacAddr));

	lim_send_delts_req_action_frame(pMac, peerMacAddr,
					pDeltsReq->req.wmeTspecPresent,
					&pDeltsReq->req.tsinfo,
					&pDeltsReq->req.tspec, psessionEntry);

	pTsinfo =
		pDeltsReq->req.wmeTspecPresent ? &pDeltsReq->req.tspec.
		tsinfo : &pDeltsReq->req.tsinfo;

	/* We've successfully send DELTS frame to AP. Update the
	 * dynamic UAPSD mask. The AC for this TSPEC to be deleted
	 * is no longer trigger enabled or delivery enabled
	 */
	lim_set_tspec_uapsd_mask_per_session(pMac, psessionEntry,
					     pTsinfo, CLEAR_UAPSD_MASK);

	/* We're deleting the TSPEC, so this particular AC is no longer
	 * admitted.  PE needs to downgrade the EDCA
	 * parameters(for the AC for which TS is being deleted) to the
	 * next best AC for which ACM is not enabled, and send the
	 * updated values to HAL.
	 */
	ac = upToAc(pTsinfo->traffic.userPrio);

	if (pTsinfo->traffic.direction == SIR_MAC_DIRECTION_UPLINK) {
		psessionEntry->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &=
			~(1 << ac);
	} else if (pTsinfo->traffic.direction ==
		   SIR_MAC_DIRECTION_DNLINK) {
		psessionEntry->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &=
			~(1 << ac);
	} else if (pTsinfo->traffic.direction ==
		   SIR_MAC_DIRECTION_BIDIR) {
		psessionEntry->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &=
			~(1 << ac);
		psessionEntry->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &=
			~(1 << ac);
	}

	lim_set_active_edca_params(pMac, psessionEntry->gLimEdcaParams,
				   psessionEntry);

	pStaDs =
		dph_get_hash_entry(pMac, DPH_STA_HASH_INDEX_PEER,
				   &psessionEntry->dph.dphHashTable);
	if (pStaDs != NULL) {
		lim_send_edca_params(pMac, psessionEntry->gLimEdcaParamsActive,
				     pStaDs->bssId);
		status = eSIR_SUCCESS;
	} else {
		lim_log(pMac, LOGE, FL("Self entry missing in Hash Table "));
		status = eSIR_FAILURE;
	}
#ifdef FEATURE_WLAN_ESE
#ifdef FEATURE_WLAN_ESE_UPLOAD
	lim_send_sme_tsm_ie_ind(pMac, psessionEntry, 0, 0, 0);
#else
	lim_deactivate_and_change_timer(pMac, eLIM_TSM_TIMER);
#endif /* FEATURE_WLAN_ESE_UPLOAD */
#endif

	/* send an sme response back */
end:
	lim_send_sme_delts_rsp(pMac, pDeltsReq, eSIR_SUCCESS, psessionEntry,
			       smesessionId, smetransactionId);
}

void lim_process_sme_addts_rsp_timeout(tpAniSirGlobal pMac, uint32_t param)
{
	/* fetch the sessionEntry based on the sessionId */
	tpPESession psessionEntry;
	psessionEntry = pe_find_session_by_session_id(pMac,
				pMac->lim.limTimers.gLimAddtsRspTimer.
				sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGP,
			FL("Session Does not exist for given sessionID"));
		return;
	}

	if (!LIM_IS_STA_ROLE(psessionEntry) &&
	    !LIM_IS_BT_AMP_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGW, "AddtsRspTimeout in non-Sta role (%d)",
			GET_LIM_SYSTEM_ROLE(psessionEntry));
		pMac->lim.gLimAddtsSent = false;
		return;
	}

	if (!pMac->lim.gLimAddtsSent) {
		lim_log(pMac, LOGW, "AddtsRspTimeout but no AddtsSent");
		return;
	}

	if (param != pMac->lim.gLimAddtsRspTimerCount) {
		lim_log(pMac, LOGE,
			FL("Invalid AddtsRsp Timer count %d (exp %d)"), param,
			pMac->lim.gLimAddtsRspTimerCount);
		return;
	}
	/* this a real response timeout */
	pMac->lim.gLimAddtsSent = false;
	pMac->lim.gLimAddtsRspTimerCount++;

	lim_send_sme_addts_rsp(pMac, true, eSIR_SME_ADDTS_RSP_TIMEOUT,
			       psessionEntry, pMac->lim.gLimAddtsReq.req.tspec,
			       psessionEntry->smeSessionId,
			       psessionEntry->transactionId);
}

/**
 * __lim_process_sme_get_statistics_request()
 *
 ***FUNCTION:
 *
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */
static void
__lim_process_sme_get_statistics_request(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpAniGetPEStatsReq pPEStatsReq;
	tSirMsgQ msgQ;

	pPEStatsReq = (tpAniGetPEStatsReq) pMsgBuf;

	msgQ.type = WMA_GET_STATISTICS_REQ;

	msgQ.reserved = 0;
	msgQ.bodyptr = pMsgBuf;
	msgQ.bodyval = 0;
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));

	if (eSIR_SUCCESS != (wma_post_ctrl_msg(pMac, &msgQ))) {
		cdf_mem_free(pMsgBuf);
		pMsgBuf = NULL;
		lim_log(pMac, LOGP, "Unable to forward request");
		return;
	}

	return;
}

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
/**
   *FUNCTION: __lim_process_sme_get_tsm_stats_request()
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */
static void
__lim_process_sme_get_tsm_stats_request(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirMsgQ msgQ;

	msgQ.type = WMA_TSM_STATS_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pMsgBuf;
	msgQ.bodyval = 0;
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));

	if (eSIR_SUCCESS != (wma_post_ctrl_msg(pMac, &msgQ))) {
		cdf_mem_free(pMsgBuf);
		pMsgBuf = NULL;
		lim_log(pMac, LOGP, "Unable to forward request");
		return;
	}
}
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

static void
__lim_process_sme_update_apwpsi_es(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirUpdateAPWPSIEsReq pUpdateAPWPSIEsReq;
	tpPESession psessionEntry;
	uint8_t sessionId;      /* PE sessionID */

	PELOG1(lim_log(pMac, LOG1, FL("received UPDATE_APWPSIEs_REQ message")););

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pUpdateAPWPSIEsReq = cdf_mem_malloc(sizeof(tSirUpdateAPWPSIEsReq));
	if (NULL == pUpdateAPWPSIEsReq) {
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for pUpdateAPWPSIEsReq"));
		return;
	}
	cdf_mem_copy(pUpdateAPWPSIEsReq, pMsgBuf,
			sizeof(struct sSirUpdateAPWPSIEsReq));

	psessionEntry = pe_find_session_by_bssid(pMac,
				pUpdateAPWPSIEsReq->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGW,
			FL("Session does not exist for given BSSID"));
		goto end;
	}

	cdf_mem_copy(&psessionEntry->APWPSIEs, &pUpdateAPWPSIEsReq->APWPSIEs,
		     sizeof(tSirAPWPSIEs));

	sch_set_fixed_beacon_fields(pMac, psessionEntry);
	lim_send_beacon_ind(pMac, psessionEntry);

end:
	cdf_mem_free(pUpdateAPWPSIEsReq);
	return;
}

void
lim_send_vdev_restart(tpAniSirGlobal pMac,
		      tpPESession psessionEntry, uint8_t sessionId)
{
	tpHalHiddenSsidVdevRestart pHalHiddenSsidVdevRestart = NULL;
	tSirMsgQ msgQ;
	tSirRetStatus retCode = eSIR_SUCCESS;

	if (psessionEntry == NULL) {
		PELOGE(lim_log
			       (pMac, LOGE, "%s:%d: Invalid parameters", __func__,
			       __LINE__);
		       )
		return;
	}

	pHalHiddenSsidVdevRestart =
		cdf_mem_malloc(sizeof(tHalHiddenSsidVdevRestart));
	if (NULL == pHalHiddenSsidVdevRestart) {
		PELOGE(lim_log
			       (pMac, LOGE, "%s:%d: Unable to allocate memory",
			       __func__, __LINE__);
		       )
		return;
	}

	pHalHiddenSsidVdevRestart->ssidHidden = psessionEntry->ssidHidden;
	pHalHiddenSsidVdevRestart->sessionId = sessionId;

	msgQ.type = WMA_HIDDEN_SSID_VDEV_RESTART;
	msgQ.bodyptr = pHalHiddenSsidVdevRestart;
	msgQ.bodyval = 0;

	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		PELOGE(lim_log
			       (pMac, LOGE, "%s:%d: wma_post_ctrl_msg() failed", __func__,
			       __LINE__);
		       )
		cdf_mem_free(pHalHiddenSsidVdevRestart);
	}
}

static void __lim_process_sme_hide_ssid(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirUpdateParams pUpdateParams;
	tpPESession psessionEntry;

	PELOG1(lim_log(pMac, LOG1, FL("received HIDE_SSID message")););

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pUpdateParams = (tpSirUpdateParams) pMsgBuf;

	psessionEntry = pe_find_session_by_sme_session_id(pMac,
				pUpdateParams->sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGW,
			"Session does not exist for given sessionId %d",
			pUpdateParams->sessionId);
		return;
	}

	/* Update the session entry */
	psessionEntry->ssidHidden = pUpdateParams->ssidHidden;

	/* Send vdev restart */
	lim_send_vdev_restart(pMac, psessionEntry, pUpdateParams->sessionId);

	/* Update beacon */
	sch_set_fixed_beacon_fields(pMac, psessionEntry);
	lim_send_beacon_ind(pMac, psessionEntry);

	return;
} /*** end __lim_process_sme_hide_ssid(tpAniSirGlobal pMac, uint32_t *pMsgBuf) ***/

static void __lim_process_sme_set_wparsni_es(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirUpdateAPWPARSNIEsReq pUpdateAPWPARSNIEsReq;
	tpPESession psessionEntry;
	uint8_t sessionId;      /* PE sessionID */

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pUpdateAPWPARSNIEsReq = cdf_mem_malloc(sizeof(tSirUpdateAPWPSIEsReq));
	if (NULL == pUpdateAPWPARSNIEsReq) {
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for pUpdateAPWPARSNIEsReq"));
		return;
	}
	cdf_mem_copy(pUpdateAPWPARSNIEsReq, pMsgBuf,
			sizeof(struct sSirUpdateAPWPARSNIEsReq));

	psessionEntry = pe_find_session_by_bssid(pMac,
				pUpdateAPWPARSNIEsReq->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGW,
			FL("Session does not exist for given BSSID"));
		goto end;
	}

	cdf_mem_copy(&psessionEntry->pLimStartBssReq->rsnIE,
		     &pUpdateAPWPARSNIEsReq->APWPARSNIEs, sizeof(tSirRSNie));

	lim_set_rs_nie_wp_aiefrom_sme_start_bss_req_message(pMac,
							    &psessionEntry->
							    pLimStartBssReq->rsnIE,
							    psessionEntry);

	psessionEntry->pLimStartBssReq->privacy = 1;
	psessionEntry->privacy = 1;

	sch_set_fixed_beacon_fields(pMac, psessionEntry);
	lim_send_beacon_ind(pMac, psessionEntry);

end:
	cdf_mem_free(pUpdateAPWPARSNIEsReq);
	return;
} /*** end __lim_process_sme_set_wparsni_es(tpAniSirGlobal pMac, uint32_t *pMsgBuf) ***/

/*
   Update the beacon Interval dynamically if beaconInterval is different in MCC
 */
static void __lim_process_sme_change_bi(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirChangeBIParams pChangeBIParams;
	tpPESession psessionEntry;
	uint8_t sessionId = 0;
	tUpdateBeaconParams beaconParams;

	PELOG1(lim_log(pMac, LOG1,
		       FL("received Update Beacon Interval message"));
	       );

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	cdf_mem_zero(&beaconParams, sizeof(tUpdateBeaconParams));
	pChangeBIParams = (tpSirChangeBIParams) pMsgBuf;

	psessionEntry = pe_find_session_by_bssid(pMac,
				pChangeBIParams->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("Session does not exist for given BSSID"));
		return;
	}

	/*Update sessionEntry Beacon Interval */
	if (psessionEntry->beaconParams.beaconInterval !=
	    pChangeBIParams->beaconInterval) {
		psessionEntry->beaconParams.beaconInterval =
			pChangeBIParams->beaconInterval;
	}

	/*Update sch beaconInterval */
	if (pMac->sch.schObject.gSchBeaconInterval !=
	    pChangeBIParams->beaconInterval) {
		pMac->sch.schObject.gSchBeaconInterval =
			pChangeBIParams->beaconInterval;

		PELOG1(lim_log(pMac, LOG1,
			       FL
				       ("LIM send update BeaconInterval Indication : %d"),
			       pChangeBIParams->beaconInterval);
		       );

		if (false == pMac->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			/* Update beacon */
			sch_set_fixed_beacon_fields(pMac, psessionEntry);

			beaconParams.bssIdx = psessionEntry->bssIdx;
			/* Set change in beacon Interval */
			beaconParams.beaconInterval =
				pChangeBIParams->beaconInterval;
			beaconParams.paramChangeBitmap =
				PARAM_BCN_INTERVAL_CHANGED;
			lim_send_beacon_params(pMac, &beaconParams, psessionEntry);
		}
	}

	return;
} /*** end __lim_process_sme_change_bi(tpAniSirGlobal pMac, uint32_t *pMsgBuf) ***/

#ifdef QCA_HT_2040_COEX
static void __lim_process_sme_set_ht2040_mode(tpAniSirGlobal pMac,
					      uint32_t *pMsgBuf)
{
	tpSirSetHT2040Mode pSetHT2040Mode;
	tpPESession psessionEntry;
	uint8_t sessionId = 0;
	cds_msg_t msg;
	tUpdateVHTOpMode *pHtOpMode = NULL;
	uint16_t staId = 0;
	tpDphHashNode pStaDs = NULL;

	PELOG1(lim_log(pMac, LOG1, FL("received Set HT 20/40 mode message")););
	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pSetHT2040Mode = (tpSirSetHT2040Mode) pMsgBuf;

	psessionEntry = pe_find_session_by_bssid(pMac,
				pSetHT2040Mode->bssId,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOG1,
			FL("Session does not exist for given BSSID "));
		lim_print_mac_addr(pMac, pSetHT2040Mode->bssId, LOG1);
		return;
	}

	lim_log(pMac, LOG1, FL("Update session entry for cbMod=%d"),
		pSetHT2040Mode->cbMode);
	/*Update sessionEntry HT related fields */
	switch (pSetHT2040Mode->cbMode) {
	case PHY_SINGLE_CHANNEL_CENTERED:
		psessionEntry->htSecondaryChannelOffset =
			PHY_SINGLE_CHANNEL_CENTERED;
		psessionEntry->htRecommendedTxWidthSet = 0;
		if (pSetHT2040Mode->obssEnabled)
			psessionEntry->htSupportedChannelWidthSet
					= eHT_CHANNEL_WIDTH_40MHZ;
		else
			psessionEntry->htSupportedChannelWidthSet
					= eHT_CHANNEL_WIDTH_20MHZ;
		break;
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		psessionEntry->htSecondaryChannelOffset =
			PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
		psessionEntry->htRecommendedTxWidthSet = 1;
		break;
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		psessionEntry->htSecondaryChannelOffset =
			PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
		psessionEntry->htRecommendedTxWidthSet = 1;
		break;
	default:
		lim_log(pMac, LOGE, FL("Invalid cbMode"));
		return;
	}

	/* Update beacon */
	sch_set_fixed_beacon_fields(pMac, psessionEntry);
	lim_send_beacon_ind(pMac, psessionEntry);

	/* update OP Mode for each associated peer */
	for (staId = 0; staId < psessionEntry->dph.dphHashTable.size; staId++) {
		pStaDs = dph_get_hash_entry(pMac, staId,
				&psessionEntry->dph.dphHashTable);
		if (NULL == pStaDs)
			continue;

		if (pStaDs->valid && pStaDs->htSupportedChannelWidthSet) {
			pHtOpMode = cdf_mem_malloc(sizeof(tUpdateVHTOpMode));
			if (NULL == pHtOpMode) {
				lim_log(pMac, LOGE,
					FL
						("%s: Not able to allocate memory for setting OP mode"),
					__func__);
				return;
			}
			pHtOpMode->opMode =
				(psessionEntry->htSecondaryChannelOffset ==
				 PHY_SINGLE_CHANNEL_CENTERED) ?
				eHT_CHANNEL_WIDTH_20MHZ : eHT_CHANNEL_WIDTH_40MHZ;
			pHtOpMode->staId = staId;
			cdf_mem_copy(pHtOpMode->peer_mac, &pStaDs->staAddr,
				     sizeof(tSirMacAddr));
			pHtOpMode->smesessionId = sessionId;

			msg.type = WMA_UPDATE_OP_MODE;
			msg.reserved = 0;
			msg.bodyptr = pHtOpMode;
			if (!CDF_IS_STATUS_SUCCESS
				    (cds_mq_post_message(CDF_MODULE_ID_WMA, &msg))) {
				lim_log(pMac, LOGE,
					FL
						("%s: Not able to post WMA_UPDATE_OP_MODE message to WMA"),
					__func__);
				cdf_mem_free(pHtOpMode);
				return;
			}
			lim_log(pMac, LOG1,
				FL
					("%s: Notifed FW about OP mode: %d for staId=%d"),
				__func__, pHtOpMode->opMode, staId);

		} else
			lim_log(pMac, LOG1,
				FL("%s: station %d does not support HT40\n"),
				__func__, staId);
	}

	return;
}
#endif

/* -------------------------------------------------------------------- */
/**
 * __lim_process_report_message
 *
 * FUNCTION:  Processes the next received Radio Resource Management message
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

void __lim_process_report_message(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
#ifdef WLAN_FEATURE_VOWIFI
	switch (pMsg->type) {
	case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
		rrm_process_neighbor_report_req(pMac, pMsg->bodyptr);
		break;
	case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
		rrm_process_beacon_report_xmit(pMac, pMsg->bodyptr);
		break;
	default:
		lim_log(pMac, LOGE, FL("Invalid msg type:%d"), pMsg->type);
	}
#endif
}

#if defined(FEATURE_WLAN_ESE) || defined(WLAN_FEATURE_VOWIFI)
/* -------------------------------------------------------------------- */
/**
 * lim_send_set_max_tx_power_req
 *
 * FUNCTION:  Send SIR_HAL_SET_MAX_TX_POWER_REQ message to change the max tx power.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param txPower txPower to be set.
 * @param pSessionEntry session entry.
 * @return None
 */
tSirRetStatus
lim_send_set_max_tx_power_req(tpAniSirGlobal pMac, tPowerdBm txPower,
			      tpPESession pSessionEntry)
{
	tpMaxTxPowerParams pMaxTxParams = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	if (pSessionEntry == NULL) {
		PELOGE(lim_log
			       (pMac, LOGE, "%s:%d: Inavalid parameters", __func__,
			       __LINE__);
		       )
		return eSIR_FAILURE;
	}

	pMaxTxParams = cdf_mem_malloc(sizeof(tMaxTxPowerParams));
	if (NULL == pMaxTxParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory for pMaxTxParams "));
		return eSIR_MEM_ALLOC_FAILED;

	}
#if defined(WLAN_VOWIFI_DEBUG) || defined(FEATURE_WLAN_ESE)
	lim_log(pMac, LOG1,
		FL("pMaxTxParams allocated...will be freed in other module"));
#endif
	if (pMaxTxParams == NULL) {
		lim_log(pMac, LOGE, FL("pMaxTxParams is NULL"));
		return eSIR_FAILURE;
	}
	pMaxTxParams->power = txPower;
	cdf_mem_copy(pMaxTxParams->bssId.bytes, pSessionEntry->bssId,
		     CDF_MAC_ADDR_SIZE);
	cdf_mem_copy(pMaxTxParams->selfStaMacAddr.bytes,
			pSessionEntry->selfMacAddr,
			CDF_MAC_ADDR_SIZE);

	msgQ.type = WMA_SET_MAX_TX_POWER_REQ;
	msgQ.bodyptr = pMaxTxParams;
	msgQ.bodyval = 0;
	PELOG1(lim_log
		       (pMac, LOG1, FL("Posting WMA_SET_MAX_TX_POWER_REQ to WMA"));
	       )
	MTRACE(mac_trace_msg_tx(pMac, pSessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		lim_log(pMac, LOGE, FL("wma_post_ctrl_msg() failed"));
		cdf_mem_free(pMaxTxParams);
	}
	return retCode;
}
#endif

/**
 * __lim_process_sme_register_mgmt_frame_req() - process sme reg mgmt frame req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process eWNI_SME_REGISTER_MGMT_FRAME_REQ message
 * from SME. It Register this information within PE.
 *
 * Return: None
 */
static void __lim_process_sme_register_mgmt_frame_req(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	CDF_STATUS cdf_status;
	tpSirRegisterMgmtFrame sme_req = (tpSirRegisterMgmtFrame)msg_buf;
	struct mgmt_frm_reg_info *lim_mgmt_regn = NULL;
	struct mgmt_frm_reg_info *next = NULL;
	bool match = false;

	lim_log(mac_ctx, LOG1, FL(
			"registerFrame %d, frameType %d, matchLen %d"),
				sme_req->registerFrame, sme_req->frameType,
				sme_req->matchLen);
	/* First check whether entry exists already */
	cdf_mutex_acquire(&mac_ctx->lim.lim_frame_register_lock);
	cdf_list_peek_front(&mac_ctx->lim.gLimMgmtFrameRegistratinQueue,
			    (cdf_list_node_t **) &lim_mgmt_regn);
	cdf_mutex_release(&mac_ctx->lim.lim_frame_register_lock);

	while (lim_mgmt_regn != NULL) {
		if (lim_mgmt_regn->frameType != sme_req->frameType)
			goto skip_match;
		if (sme_req->matchLen) {
			if ((lim_mgmt_regn->matchLen == sme_req->matchLen) &&
				(cdf_mem_compare(lim_mgmt_regn->matchData,
					sme_req->matchData,
					lim_mgmt_regn->matchLen))) {
					/* found match! */
					match = true;
					break;
			}
		} else {
			/* found match! */
			match = true;
			break;
		}
skip_match:
		cdf_mutex_acquire(&mac_ctx->lim.lim_frame_register_lock);
		cdf_status = cdf_list_peek_next(
				&mac_ctx->lim.gLimMgmtFrameRegistratinQueue,
				(cdf_list_node_t *)lim_mgmt_regn,
				(cdf_list_node_t **)&next);
		cdf_mutex_release(&mac_ctx->lim.lim_frame_register_lock);
		lim_mgmt_regn = next;
		next = NULL;
	}
	if (match) {
		cdf_mutex_acquire(&mac_ctx->lim.lim_frame_register_lock);
		cdf_list_remove_node(
				&mac_ctx->lim.gLimMgmtFrameRegistratinQueue,
				(cdf_list_node_t *)lim_mgmt_regn);
		cdf_mutex_release(&mac_ctx->lim.lim_frame_register_lock);
		cdf_mem_free(lim_mgmt_regn);
	}

	if (sme_req->registerFrame) {
		lim_mgmt_regn =
			cdf_mem_malloc(sizeof(struct mgmt_frm_reg_info) +
					sme_req->matchLen);
		if (lim_mgmt_regn != NULL) {
			cdf_mem_set((void *)lim_mgmt_regn,
				    sizeof(struct mgmt_frm_reg_info) +
				    sme_req->matchLen, 0);
			lim_mgmt_regn->frameType = sme_req->frameType;
			lim_mgmt_regn->matchLen = sme_req->matchLen;
			lim_mgmt_regn->sessionId = sme_req->sessionId;
			if (sme_req->matchLen) {
				cdf_mem_copy(lim_mgmt_regn->matchData,
					     sme_req->matchData,
					     sme_req->matchLen);
			}
			cdf_mutex_acquire(
					&mac_ctx->lim.lim_frame_register_lock);
			cdf_list_insert_front(&mac_ctx->lim.
					      gLimMgmtFrameRegistratinQueue,
					      &lim_mgmt_regn->node);
			cdf_mutex_release(
					&mac_ctx->lim.lim_frame_register_lock);
		}
	}
	return;
}

static void __lim_deregister_deferred_sme_req_after_noa_start(tpAniSirGlobal pMac)
{
	lim_log(pMac, LOG1, FL("Dereg msgType %d"),
		pMac->lim.gDeferMsgTypeForNOA);
	pMac->lim.gDeferMsgTypeForNOA = 0;
	if (pMac->lim.gpDefdSmeMsgForNOA != NULL) {
		/* __lim_process_sme_scan_req consumed the buffer. We can free it. */
		cdf_mem_free(pMac->lim.gpDefdSmeMsgForNOA);
		pMac->lim.gpDefdSmeMsgForNOA = NULL;
	}
}

/**
 * lim_process_regd_defd_sme_req_after_noa_start()
 *
 * mac_ctx: Pointer to Global MAC structure
 *
 * This function is called to process deferred sme req message
 * after noa start.
 *
 * Return: None
 */
void lim_process_regd_defd_sme_req_after_noa_start(tpAniSirGlobal mac_ctx)
{
	bool buf_consumed = true;

	lim_log(mac_ctx, LOG1, FL("Process defd sme req %d"),
		mac_ctx->lim.gDeferMsgTypeForNOA);

	if ((mac_ctx->lim.gDeferMsgTypeForNOA == 0) ||
			(mac_ctx->lim.gpDefdSmeMsgForNOA == NULL)) {
		lim_log(mac_ctx, LOGW,
			FL("start rcvd from FW when no sme deferred msg pending. Do nothing. "));
		lim_log(mac_ctx, LOGW,
			FL("It may happen when NOA start ind and timeout happen at the same time"));
		return;
	}
	switch (mac_ctx->lim.gDeferMsgTypeForNOA) {
	case eWNI_SME_SCAN_REQ:
		__lim_process_sme_scan_req(mac_ctx,
				mac_ctx->lim.gpDefdSmeMsgForNOA);
		break;
#ifdef FEATURE_OEM_DATA_SUPPORT
	case eWNI_SME_OEM_DATA_REQ:
		__lim_process_sme_oem_data_req(mac_ctx,
				mac_ctx->lim.gpDefdSmeMsgForNOA);
		break;
#endif
	case eWNI_SME_REMAIN_ON_CHANNEL_REQ:
		buf_consumed = lim_process_remain_on_chnl_req(mac_ctx,
				mac_ctx->lim.gpDefdSmeMsgForNOA);
		/*
		 * lim_process_remain_on_chnl_req doesnt want us to free
		 * the buffer since it is freed in lim_remain_on_chn_rsp.
		 * this change is to avoid "double free"
		 */
		if (false == buf_consumed)
			mac_ctx->lim.gpDefdSmeMsgForNOA = NULL;
		break;
	case eWNI_SME_JOIN_REQ:
		__lim_process_sme_join_req(mac_ctx,
				mac_ctx->lim.gpDefdSmeMsgForNOA);
		break;
	default:
		lim_log(mac_ctx, LOGE, FL("Unknown deferred msg type %d"),
			mac_ctx->lim.gDeferMsgTypeForNOA);
		break;
	}
	__lim_deregister_deferred_sme_req_after_noa_start(mac_ctx);
}

static void
__lim_process_sme_reset_ap_caps_change(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpSirResetAPCapsChange pResetCapsChange;
	tpPESession psessionEntry;
	uint8_t sessionId = 0;
	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pResetCapsChange = (tpSirResetAPCapsChange) pMsgBuf;
	psessionEntry =
		pe_find_session_by_bssid(pMac, pResetCapsChange->bssId.bytes,
					 &sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("Session does not exist for given BSSID"));
		return;
	}

	psessionEntry->limSentCapsChangeNtf = false;
	return;
}

/**
 * lim_process_sme_req_messages()
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue(). This
 * function processes SME request messages from HDD or upper layer
 * application.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  msgType   Indicates the SME message type
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return Boolean - true - if pMsgBuf is consumed and can be freed.
 *                   false - if pMsgBuf is not to be freed.
 */

bool lim_process_sme_req_messages(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	bool bufConsumed = true;        /* Set this flag to false within case block of any following message, that doesnt want pMsgBuf to be freed. */
	uint32_t *pMsgBuf = pMsg->bodyptr;
	tpSirSmeScanReq pScanReq;
	PELOG1(lim_log
		       (pMac, LOG1,
		       FL
			       ("LIM Received SME Message %s(%d) Global LimSmeState:%s(%d) Global LimMlmState: %s(%d)"),
		       lim_msg_str(pMsg->type), pMsg->type,
		       lim_sme_state_str(pMac->lim.gLimSmeState), pMac->lim.gLimSmeState,
		       lim_mlm_state_str(pMac->lim.gLimMlmState), pMac->lim.gLimMlmState);
	       )

	pScanReq = (tpSirSmeScanReq) pMsgBuf;
	/* If no insert NOA required then execute the code below */

	switch (pMsg->type) {
	case eWNI_SME_SYS_READY_IND:
		bufConsumed = __lim_process_sme_sys_ready_ind(pMac, pMsgBuf);
		break;

	case eWNI_SME_START_BSS_REQ:
		bufConsumed = __lim_process_sme_start_bss_req(pMac, pMsg);
		break;

	case eWNI_SME_SCAN_REQ:
		__lim_process_sme_scan_req(pMac, pMsgBuf);
		break;

#ifdef FEATURE_OEM_DATA_SUPPORT
	case eWNI_SME_OEM_DATA_REQ:
		__lim_process_sme_oem_data_req(pMac, pMsgBuf);
		break;
#endif
	case eWNI_SME_REMAIN_ON_CHANNEL_REQ:
		bufConsumed = lim_process_remain_on_chnl_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_UPDATE_NOA:
		__lim_process_sme_no_a_update(pMac, pMsgBuf);
		break;
	case eWNI_SME_CLEAR_DFS_CHANNEL_LIST:
		__lim_process_clear_dfs_channel_list(pMac, pMsg);
		break;
	case eWNI_SME_JOIN_REQ:
		__lim_process_sme_join_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_REASSOC_REQ:
		__lim_process_sme_reassoc_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_DISASSOC_REQ:
		__lim_process_sme_disassoc_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_DISASSOC_CNF:
	case eWNI_SME_DEAUTH_CNF:
		__lim_process_sme_disassoc_cnf(pMac, pMsgBuf);
		break;

	case eWNI_SME_DEAUTH_REQ:
		__lim_process_sme_deauth_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_SETCONTEXT_REQ:
		__lim_process_sme_set_context_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_STOP_BSS_REQ:
		bufConsumed = __lim_process_sme_stop_bss_req(pMac, pMsg);
		break;

	case eWNI_SME_ASSOC_CNF:
		if (pMsg->type == eWNI_SME_ASSOC_CNF)
			PELOG1(lim_log(pMac,
				LOG1, FL("Received ASSOC_CNF message"));)
			__lim_process_sme_assoc_cnf_new(pMac, pMsg->type,
							pMsgBuf);
		break;

	case eWNI_SME_ADDTS_REQ:
		PELOG1(lim_log(pMac, LOG1, FL("Received ADDTS_REQ message"));)
		__lim_process_sme_addts_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_DELTS_REQ:
		PELOG1(lim_log(pMac, LOG1, FL("Received DELTS_REQ message"));)
		__lim_process_sme_delts_req(pMac, pMsgBuf);
		break;

	case SIR_LIM_ADDTS_RSP_TIMEOUT:
		PELOG1(lim_log
			       (pMac, LOG1,
			       FL("Received SIR_LIM_ADDTS_RSP_TIMEOUT message "));
		       )
		lim_process_sme_addts_rsp_timeout(pMac, pMsg->bodyval);
		break;

	case eWNI_SME_GET_STATISTICS_REQ:
		__lim_process_sme_get_statistics_request(pMac, pMsgBuf);
		/* HAL consumes pMsgBuf. It will be freed there. Set bufConsumed to false. */
		bufConsumed = false;
		break;
#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
	case eWNI_SME_GET_TSM_STATS_REQ:
		__lim_process_sme_get_tsm_stats_request(pMac, pMsgBuf);
		bufConsumed = false;
		break;
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */
	case eWNI_SME_GET_ASSOC_STAS_REQ:
		lim_process_sme_get_assoc_sta_info(pMac, pMsgBuf);
		break;
	case eWNI_SME_TKIP_CNTR_MEAS_REQ:
		lim_process_tkip_counter_measures(pMac, pMsgBuf);
		break;

	case eWNI_SME_HIDE_SSID_REQ:
		__lim_process_sme_hide_ssid(pMac, pMsgBuf);
		break;
	case eWNI_SME_UPDATE_APWPSIE_REQ:
		__lim_process_sme_update_apwpsi_es(pMac, pMsgBuf);
		break;
	case eWNI_SME_GET_WPSPBC_SESSION_REQ:
		lim_process_sme_get_wpspbc_sessions(pMac, pMsgBuf);
		break;

	case eWNI_SME_SET_APWPARSNIEs_REQ:
		__lim_process_sme_set_wparsni_es(pMac, pMsgBuf);
		break;

	case eWNI_SME_CHNG_MCC_BEACON_INTERVAL:
		/* Update the beaconInterval */
		__lim_process_sme_change_bi(pMac, pMsgBuf);
		break;

#ifdef QCA_HT_2040_COEX
	case eWNI_SME_SET_HT_2040_MODE:
		__lim_process_sme_set_ht2040_mode(pMac, pMsgBuf);
		break;
#endif

#if defined WLAN_FEATURE_VOWIFI
	case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
	case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
		__lim_process_report_message(pMac, pMsg);
		break;
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
	case eWNI_SME_FT_PRE_AUTH_REQ:
		bufConsumed = (bool) lim_process_ft_pre_auth_req(pMac, pMsg);
		break;
	case eWNI_SME_FT_UPDATE_KEY:
		lim_process_ft_update_key(pMac, pMsgBuf);
		break;

	case eWNI_SME_FT_AGGR_QOS_REQ:
		lim_process_ft_aggr_qos_req(pMac, pMsgBuf);
		break;
#endif

	case eWNI_SME_REGISTER_MGMT_FRAME_REQ:
		__lim_process_sme_register_mgmt_frame_req(pMac, pMsgBuf);
		break;
#ifdef FEATURE_WLAN_TDLS
	case eWNI_SME_TDLS_SEND_MGMT_REQ:
		lim_process_sme_tdls_mgmt_send_req(pMac, pMsgBuf);
		break;
	case eWNI_SME_TDLS_ADD_STA_REQ:
		lim_process_sme_tdls_add_sta_req(pMac, pMsgBuf);
		break;
	case eWNI_SME_TDLS_DEL_STA_REQ:
		lim_process_sme_tdls_del_sta_req(pMac, pMsgBuf);
		break;
	case eWNI_SME_TDLS_LINK_ESTABLISH_REQ:
		lim_process_sme_tdls_link_establish_req(pMac, pMsgBuf);
		break;
#endif
	case eWNI_SME_RESET_AP_CAPS_CHANGED:
		__lim_process_sme_reset_ap_caps_change(pMac, pMsgBuf);
		break;

	case eWNI_SME_CHANNEL_CHANGE_REQ:
		lim_process_sme_channel_change_request(pMac, pMsgBuf);
		break;

	case eWNI_SME_START_BEACON_REQ:
		lim_process_sme_start_beacon_req(pMac, pMsgBuf);
		break;

	case eWNI_SME_DFS_BEACON_CHAN_SW_IE_REQ:
		lim_process_sme_dfs_csa_ie_request(pMac, pMsgBuf);
		break;

	case eWNI_SME_UPDATE_ADDITIONAL_IES:
		lim_process_update_add_ies(pMac, pMsgBuf);
		break;

	case eWNI_SME_MODIFY_ADDITIONAL_IES:
		lim_process_modify_add_ies(pMac, pMsgBuf);
		break;
	case eWNI_SME_SET_HW_MODE_REQ:
		lim_process_set_hw_mode(pMac, pMsgBuf);
		break;
	case eWNI_SME_NSS_UPDATE_REQ:
		lim_process_nss_update_request(pMac, pMsgBuf);
		break;
	case eWNI_SME_SET_DUAL_MAC_CFG_REQ:
		lim_process_set_dual_mac_cfg_req(pMac, pMsgBuf);
		break;
	case eWNI_SME_SET_IE_REQ:
		lim_process_set_ie_req(pMac, pMsgBuf);
		break;
	case eWNI_SME_EXT_CHANGE_CHANNEL:
		lim_process_ext_change_channel(pMac, pMsgBuf);
		break;
	default:
		cdf_mem_free((void *)pMsg->bodyptr);
		pMsg->bodyptr = NULL;
		break;
	} /* switch (msgType) */

	return bufConsumed;
} /*** end lim_process_sme_req_messages() ***/

/**
 * lim_process_sme_start_beacon_req()
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue(). This
 * function processes SME request messages from HDD or upper layer
 * application.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  msgType   Indicates the SME message type
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return Boolean - true - if pMsgBuf is consumed and can be freed.
 *                   false - if pMsgBuf is not to be freed.
 */
static void lim_process_sme_start_beacon_req(tpAniSirGlobal pMac, uint32_t *pMsg)
{
	tpSirStartBeaconIndication pBeaconStartInd;
	tpPESession psessionEntry;
	uint8_t sessionId;      /* PE sessionID */

	if (pMsg == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	pBeaconStartInd = (tpSirStartBeaconIndication) pMsg;
	psessionEntry = pe_find_session_by_bssid(pMac,
				pBeaconStartInd->bssid,
				&sessionId);
	if (psessionEntry == NULL) {
		lim_print_mac_addr(pMac, pBeaconStartInd->bssid, LOGE);
		lim_log(pMac, LOGE,
			FL("Session does not exist for given bssId"));
		return;
	}

	if (pBeaconStartInd->beaconStartStatus == true) {
		/*
		 * Currently this Indication comes from SAP
		 * to start Beacon Tx on a DFS channel
		 * since beaconing has to be done on DFS
		 * channel only after CAC WAIT is completed.
		 * On a DFS Channel LIM does not start beacon
		 * Tx right after the WMA_ADD_BSS_RSP.
		 */
		lim_apply_configuration(pMac, psessionEntry);
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
			  FL("Start Beacon with ssid %s Ch %d"),
			  psessionEntry->ssId.ssId,
			  psessionEntry->currentOperChannel);
		lim_send_beacon_ind(pMac, psessionEntry);
	} else {
		lim_log(pMac, LOGE, FL("Invalid Beacon Start Indication"));
		return;
	}
}

/**
 * lim_process_sme_channel_change_request() - process sme ch change req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function is called to process SME_CHANNEL_CHANGE_REQ message
 *
 * Return: None
 */
static void lim_process_sme_channel_change_request(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tpSirChanChangeRequest ch_change_req;
	tpPESession session_entry;
	uint8_t session_id;      /* PE session_id */
	tPowerdBm max_tx_pwr;
	uint32_t val = 0;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("msg_buf is NULL"));
		return;
	}
	ch_change_req = (tpSirChanChangeRequest)msg_buf;

	max_tx_pwr = cfg_get_regulatory_max_transmit_power(mac_ctx,
			ch_change_req->targetChannel);

	if ((ch_change_req->messageType != eWNI_SME_CHANNEL_CHANGE_REQ) ||
			(max_tx_pwr == WMA_MAX_TXPOWER_INVALID)) {
		lim_log(mac_ctx, LOGE, FL("Invalid Request/max_tx_pwr"));
		return;
	}

	session_entry = pe_find_session_by_bssid(mac_ctx,
			ch_change_req->bssid, &session_id);
	if (session_entry == NULL) {
		lim_print_mac_addr(mac_ctx, ch_change_req->bssid, LOGE);
		lim_log(mac_ctx, LOGE, FL(
			"Session does not exist for given bssId"));
		return;
	}

	if (session_entry->currentOperChannel ==
			ch_change_req->targetChannel) {
		lim_log(mac_ctx, LOGE, FL("target CH is same as current CH"));
		return;
	}

	if (LIM_IS_AP_ROLE(session_entry))
		session_entry->channelChangeReasonCode =
			LIM_SWITCH_CHANNEL_SAP_DFS;
	else
		session_entry->channelChangeReasonCode =
			LIM_SWITCH_CHANNEL_OPERATION;

	lim_log(mac_ctx, LOGW, FL(
			"switch old chnl %d to new chnl %d, ch_bw %d"),
			session_entry->currentOperChannel,
			ch_change_req->targetChannel,
			ch_change_req->channel_width);

	/* Store the New Channel Params in session_entry */
	session_entry->ch_width = ch_change_req->channel_width;
	session_entry->ch_center_freq_seg0 =
		ch_change_req->center_freq_seg_0;
	session_entry->ch_center_freq_seg1 =
		ch_change_req->center_freq_seg_1;
	session_entry->htSecondaryChannelOffset = ch_change_req->cbMode;
	session_entry->htSupportedChannelWidthSet =
		(ch_change_req->channel_width ? 1 : 0);
	session_entry->htRecommendedTxWidthSet =
		session_entry->htSupportedChannelWidthSet;
	session_entry->currentOperChannel =
		ch_change_req->targetChannel;
	session_entry->limRFBand =
		lim_get_rf_band(session_entry->currentOperChannel);
	/* Initialize 11h Enable Flag */
	if (SIR_BAND_5_GHZ == session_entry->limRFBand) {
		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_11H_ENABLED, &val) !=
				eSIR_SUCCESS)
			lim_log(mac_ctx, LOGP,
				FL("Fail to get WNI_CFG_11H_ENABLED"));
	}

	session_entry->lim11hEnable = val;
	session_entry->dot11mode = ch_change_req->dot11mode;
	cdf_mem_copy(&session_entry->rateSet,
			&ch_change_req->operational_rateset,
			sizeof(session_entry->rateSet));
	cdf_mem_copy(&session_entry->extRateSet,
			&ch_change_req->extended_rateset,
			sizeof(session_entry->extRateSet));
	lim_set_channel(mac_ctx, ch_change_req->targetChannel,
			session_entry->ch_center_freq_seg0,
			session_entry->ch_center_freq_seg1,
			session_entry->ch_width,
			max_tx_pwr, session_entry->peSessionId);
}

/******************************************************************************
* lim_start_bss_update_add_ie_buffer()
*
***FUNCTION:
* This function checks the src buffer and its length and then malloc for
* dst buffer update the same
*
***LOGIC:
*
***ASSUMPTIONS:
*
***NOTE:
*
* @param  pMac      Pointer to Global MAC structure
* @param  **pDstData_buff  A pointer to pointer of  uint8_t dst buffer
* @param  *pDstDataLen  A pointer to pointer of  uint16_t dst buffer length
* @param  *pSrcData_buff  A pointer of  uint8_t  src buffer
* @param  srcDataLen  src buffer length
******************************************************************************/

static void
lim_start_bss_update_add_ie_buffer(tpAniSirGlobal pMac,
				   uint8_t **pDstData_buff,
				   uint16_t *pDstDataLen,
				   uint8_t *pSrcData_buff, uint16_t srcDataLen)
{

	if (srcDataLen > 0 && pSrcData_buff != NULL) {
		*pDstDataLen = srcDataLen;

		*pDstData_buff = cdf_mem_malloc(*pDstDataLen);

		if (NULL == *pDstData_buff) {
			lim_log(pMac, LOGE,
				FL("AllocateMemory failed for pDstData_buff"));
			return;
		}
		cdf_mem_copy(*pDstData_buff, pSrcData_buff, *pDstDataLen);
	} else {
		*pDstData_buff = NULL;
		*pDstDataLen = 0;
	}
}

/******************************************************************************
* lim_update_add_ie_buffer()
*
***FUNCTION:
* This function checks the src buffer and length if src buffer length more
* than dst buffer length then free the dst buffer and malloc for the new src
* length, and update the dst buffer and length. But if dst buffer is bigger
* than src buffer length then it just update the dst buffer and length
*
***LOGIC:
*
***ASSUMPTIONS:
*
***NOTE:
*
* @param  pMac      Pointer to Global MAC structure
* @param  **pDstData_buff  A pointer to pointer of  uint8_t dst buffer
* @param  *pDstDataLen  A pointer to pointer of  uint16_t dst buffer length
* @param  *pSrcData_buff  A pointer of  uint8_t  src buffer
* @param  srcDataLen  src buffer length
******************************************************************************/

static void
lim_update_add_ie_buffer(tpAniSirGlobal pMac,
			 uint8_t **pDstData_buff,
			 uint16_t *pDstDataLen,
			 uint8_t *pSrcData_buff, uint16_t srcDataLen)
{

	if (NULL == pSrcData_buff) {
		lim_log(pMac, LOGE, FL("src buffer is null."));
		return;
	}

	if (srcDataLen > *pDstDataLen) {
		*pDstDataLen = srcDataLen;
		/* free old buffer */
		cdf_mem_free(*pDstData_buff);
		/* allocate a new */
		*pDstData_buff = cdf_mem_malloc(*pDstDataLen);

		if (NULL == *pDstData_buff) {
			lim_log(pMac, LOGE, FL("Memory allocation failed."));
			*pDstDataLen = 0;
			return;
		}
	}

	/* copy the content of buffer into dst buffer
	 */
	*pDstDataLen = srcDataLen;
	cdf_mem_copy(*pDstData_buff, pSrcData_buff, *pDstDataLen);

}

/*
* lim_process_modify_add_ies() - process modify additional IE req.
*
* @mac_ctx: Pointer to Global MAC structure
* @msg_buf: pointer to the SME message buffer
*
* This function update the PE buffers for additional IEs.
*
* Return: None
*/
static void lim_process_modify_add_ies(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tpSirModifyIEsInd modify_add_ies;
	tpPESession session_entry;
	uint8_t session_id;
	bool ret = false;
	tSirAddIeParams *add_ie_params;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("msg_buf is NULL"));
		return;
	}

	modify_add_ies = (tpSirModifyIEsInd)msg_buf;
	/* Incoming message has smeSession, use BSSID to find PE session */
	session_entry = pe_find_session_by_bssid(mac_ctx,
			modify_add_ies->modifyIE.bssid.bytes, &session_id);

	if (NULL == session_entry) {
		lim_log(mac_ctx, LOGE, FL("Session not found for given bssid. "
					MAC_ADDRESS_STR),
		MAC_ADDR_ARRAY(modify_add_ies->modifyIE.bssid.bytes));
		goto end;
	}
	if ((0 == modify_add_ies->modifyIE.ieBufferlength) ||
		(0 == modify_add_ies->modifyIE.ieIDLen) ||
		(NULL == modify_add_ies->modifyIE.pIEBuffer)) {
		lim_log(mac_ctx, LOGE,
			FL("Invalid request pIEBuffer %p ieBufferlength %d ieIDLen %d ieID %d. update Type %d"),
				modify_add_ies->modifyIE.pIEBuffer,
				modify_add_ies->modifyIE.ieBufferlength,
				modify_add_ies->modifyIE.ieID,
				modify_add_ies->modifyIE.ieIDLen,
				modify_add_ies->updateType);
		goto end;
	}
	add_ie_params = &session_entry->addIeParams;
	switch (modify_add_ies->updateType) {
	case eUPDATE_IE_PROBE_RESP:
		/* Probe resp */
		break;
	case eUPDATE_IE_ASSOC_RESP:
		/* assoc resp IE */
		if (add_ie_params->assocRespDataLen == 0) {
			CDF_TRACE(CDF_MODULE_ID_PE,
					CDF_TRACE_LEVEL_ERROR, FL(
				"assoc resp add ie not present %d"),
				add_ie_params->assocRespDataLen);
		}
		/* search through the buffer and modify the IE */
		break;
	case eUPDATE_IE_PROBE_BCN:
		/*probe beacon IE */
		if (ret == true && modify_add_ies->modifyIE.notify) {
			lim_handle_param_update(mac_ctx,
					modify_add_ies->updateType);
		}
		break;
	default:
		lim_log(mac_ctx, LOGE, FL("unhandled buffer type %d"),
				modify_add_ies->updateType);
		break;
	}
end:
	cdf_mem_free(modify_add_ies->modifyIE.pIEBuffer);
	modify_add_ies->modifyIE.pIEBuffer = NULL;
}

/*
* lim_process_update_add_ies() - process additional IE update req
*
* @mac_ctx: Pointer to Global MAC structure
* @msg_buf: pointer to the SME message buffer
*
* This function update the PE buffers for additional IEs.
*
* Return: None
*/
static void lim_process_update_add_ies(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tpSirUpdateIEsInd update_add_ies = (tpSirUpdateIEsInd)msg_buf;
	uint8_t session_id;
	tpPESession session_entry;
	tSirAddIeParams *addn_ie;
	uint16_t new_length = 0;
	uint8_t *new_ptr = NULL;
	tSirUpdateIE *update_ie;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("msg_buf is NULL"));
		return;
	}
	update_ie = &update_add_ies->updateIE;
	/* incoming message has smeSession, use BSSID to find PE session */
	session_entry = pe_find_session_by_bssid(mac_ctx,
			update_ie->bssid.bytes, &session_id);

	if (NULL == session_entry) {
		lim_log(mac_ctx, LOGE, FL("Session not found for given bssid. "
				       MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(update_ie->bssid.bytes));
		goto end;
	}
	addn_ie = &session_entry->addIeParams;
	/* if len is 0, upper layer requested freeing of buffer */
	if (0 == update_ie->ieBufferlength) {
		switch (update_add_ies->updateType) {
		case eUPDATE_IE_PROBE_RESP:
			cdf_mem_free(addn_ie->probeRespData_buff);
			addn_ie->probeRespData_buff = NULL;
			addn_ie->probeRespDataLen = 0;
			break;
		case eUPDATE_IE_ASSOC_RESP:
			cdf_mem_free(addn_ie->assocRespData_buff);
			addn_ie->assocRespData_buff = NULL;
			addn_ie->assocRespDataLen = 0;
			break;
		case eUPDATE_IE_PROBE_BCN:
			cdf_mem_free(addn_ie->probeRespBCNData_buff);
			addn_ie->probeRespBCNData_buff = NULL;
			addn_ie->probeRespBCNDataLen = 0;

			if (update_ie->notify)
				lim_handle_param_update(mac_ctx,
						update_add_ies->updateType);
			break;
		default:
			break;
		}
		return;
	}
	switch (update_add_ies->updateType) {
	case eUPDATE_IE_PROBE_RESP:
		if (update_ie->append) {
			/*
			 * In case of append, allocate new memory
			 * with combined length
			 */
			new_length = update_ie->ieBufferlength +
				addn_ie->probeRespDataLen;
			new_ptr = cdf_mem_malloc(new_length);
			if (NULL == new_ptr) {
				lim_log(mac_ctx, LOGE, FL(
						"Memory allocation failed."));
				goto end;
			}
			/* append buffer to end of local buffers */
			cdf_mem_copy(new_ptr, addn_ie->probeRespData_buff,
					addn_ie->probeRespDataLen);
			cdf_mem_copy(&new_ptr[addn_ie->probeRespDataLen],
				     update_ie->pAdditionIEBuffer,
				     update_ie->ieBufferlength);
			/* free old memory */
			cdf_mem_free(addn_ie->probeRespData_buff);
			/* adjust length accordingly */
			addn_ie->probeRespDataLen = new_length;
			/* save refernece of local buffer in PE session */
			addn_ie->probeRespData_buff = new_ptr;
			goto end;
		}
		lim_update_add_ie_buffer(mac_ctx, &addn_ie->probeRespData_buff,
				&addn_ie->probeRespDataLen,
				update_ie->pAdditionIEBuffer,
				update_ie->ieBufferlength);
		break;
	case eUPDATE_IE_ASSOC_RESP:
		/* assoc resp IE */
		lim_update_add_ie_buffer(mac_ctx, &addn_ie->assocRespData_buff,
				&addn_ie->assocRespDataLen,
				update_ie->pAdditionIEBuffer,
				update_ie->ieBufferlength);
		break;
	case eUPDATE_IE_PROBE_BCN:
		/* probe resp Bcn IE */
		lim_update_add_ie_buffer(mac_ctx,
				&addn_ie->probeRespBCNData_buff,
				&addn_ie->probeRespBCNDataLen,
				update_ie->pAdditionIEBuffer,
				update_ie->ieBufferlength);
		if (update_ie->notify)
			lim_handle_param_update(mac_ctx,
					update_add_ies->updateType);
		break;
	default:
		lim_log(mac_ctx, LOGE, FL("unhandled buffer type %d."),
			update_add_ies->updateType);
		break;
	}
end:
	cdf_mem_free(update_ie->pAdditionIEBuffer);
	update_ie->pAdditionIEBuffer = NULL;
}

/**
 * send_extended_chan_switch_action_frame()- function to send ECSA
 * action frame for each sta connected to SAP/GO and AP in case of
 * STA .
 * @mac_ctx: pointer to global mac structure
 * @new_channel: new channel to switch to.
 * @ch_bandwidth: BW of channel to calculate op_class
 * @session_entry: pe session
 *
 * This function is called to send ECSA frame for STA/CLI and SAP/GO.
 *
 * Return: void
 */

static void send_extended_chan_switch_action_frame(tpAniSirGlobal mac_ctx,
				uint16_t new_channel, uint8_t ch_bandwidth,
						tpPESession session_entry)
{
	uint16_t op_class;
	uint8_t switch_mode = 0, i;
	tpDphHashNode psta;


	op_class = cds_regdm_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				new_channel,
				ch_bandwidth);

	if (LIM_IS_AP_ROLE(session_entry) &&
		(mac_ctx->sap.SapDfsInfo.disable_dfs_ch_switch == false))
		switch_mode = 1;

	if (LIM_IS_AP_ROLE(session_entry)) {
		for (i = 0; i < mac_ctx->lim.maxStation; i++) {
			psta =
			  session_entry->dph.dphHashTable.pDphNodeArray + i;
			if (psta && psta->added)
				lim_send_extended_chan_switch_action_frame(
					mac_ctx,
					psta->staAddr,
					switch_mode, op_class, new_channel,
					LIM_MAX_CSA_IE_UPDATES, session_entry);
		}
	} else if (LIM_IS_STA_ROLE(session_entry)) {
		lim_send_extended_chan_switch_action_frame(mac_ctx,
					session_entry->bssId,
					switch_mode, op_class, new_channel,
					LIM_MAX_CSA_IE_UPDATES, session_entry);
	}

}

/**
 * lim_process_sme_dfs_csa_ie_request() - process sme dfs csa ie req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function processes SME request messages from HDD or upper layer
 * application.
 *
 * Return: None
 */
static void lim_process_sme_dfs_csa_ie_request(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	tpSirDfsCsaIeRequest dfs_csa_ie_req;
	tpPESession session_entry = NULL;
	uint32_t ch_width = 0;
	uint8_t session_id;
	tLimWiderBWChannelSwitchInfo *wider_bw_ch_switch;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	dfs_csa_ie_req = (tSirDfsCsaIeRequest *)msg_buf;
	session_entry = pe_find_session_by_bssid(mac_ctx,
			dfs_csa_ie_req->bssid, &session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE, FL(
			"Session not found for given BSSID" MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(dfs_csa_ie_req->bssid));
		return;
	}

	if (session_entry->valid && !LIM_IS_AP_ROLE(session_entry)) {
		lim_log(mac_ctx, LOGE, FL("Invalid SystemRole %d"),
			GET_LIM_SYSTEM_ROLE(session_entry));
		return;
	}

	/* target channel */
	session_entry->gLimChannelSwitch.primaryChannel =
		dfs_csa_ie_req->targetChannel;

	/* Channel switch announcement needs to be included in beacon */
	session_entry->dfsIncludeChanSwIe = true;
	session_entry->gLimChannelSwitch.switchCount = LIM_MAX_CSA_IE_UPDATES;
	session_entry->gLimChannelSwitch.ch_width =
				 dfs_csa_ie_req->ch_bandwidth;
	if (mac_ctx->sap.SapDfsInfo.disable_dfs_ch_switch == false)
		session_entry->gLimChannelSwitch.switchMode = 1;

	/*
	 * Validate if SAP is operating HT or VHT mode and set the Channel
	 * Switch Wrapper element with the Wide Band Switch subelement.
	 */
	if (true != session_entry->vhtCapability)
		goto skip_vht;

	if (WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ ==
			session_entry->vhtTxChannelWidthSet)
		ch_width = eHT_CHANNEL_WIDTH_80MHZ;
	else if (WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ ==
			session_entry->vhtTxChannelWidthSet)
		ch_width = session_entry->htSupportedChannelWidthSet;
	/* Now encode the Wider Ch BW element depending on the ch width */
	wider_bw_ch_switch = &session_entry->gLimWiderBWChannelSwitch;
	switch (ch_width) {
	case eHT_CHANNEL_WIDTH_20MHZ:
		/*
		 * Wide channel BW sublement in channel wrapper element is not
		 * required in case of 20 Mhz operation. Currently It is set
		 * only set in case of 40/80 Mhz Operation.
		 */
		session_entry->dfsIncludeChanWrapperIe = false;
		wider_bw_ch_switch->newChanWidth =
			WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
		break;
	case eHT_CHANNEL_WIDTH_40MHZ:
		session_entry->dfsIncludeChanWrapperIe = true;
		wider_bw_ch_switch->newChanWidth =
			WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
		break;
	case eHT_CHANNEL_WIDTH_80MHZ:
		session_entry->dfsIncludeChanWrapperIe = true;
		wider_bw_ch_switch->newChanWidth =
			WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
		break;
	case eHT_CHANNEL_WIDTH_160MHZ:
		session_entry->dfsIncludeChanWrapperIe = true;
		wider_bw_ch_switch->newChanWidth =
			WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;
		break;
	default:
		session_entry->dfsIncludeChanWrapperIe = false;
		/*
		 * Need to handle 80+80 Mhz Scenario. When 80+80 is supported
		 * set the gLimWiderBWChannelSwitch.newChanWidth to 3
		 */
		lim_log(mac_ctx, LOGE, FL("Invalid Channel Width"));
		break;
	}
	/* Fetch the center channel based on the channel width */
	wider_bw_ch_switch->newCenterChanFreq0 =
		lim_get_center_channel(mac_ctx, dfs_csa_ie_req->targetChannel,
				       session_entry->htSecondaryChannelOffset,
				       wider_bw_ch_switch->newChanWidth);
	/*
	 * This is not applicable for 20/40/80 Mhz.Only used when we support
	 * 80+80 Mhz operation. In case of 80+80 Mhz, this parameter indicates
	 * center channel frequency index of 80 Mhz channel of
	 * frequency segment 1.
	 */
	wider_bw_ch_switch->newCenterChanFreq1 = 0;
skip_vht:
	/* Send CSA IE request from here */
	if (sch_set_fixed_beacon_fields(mac_ctx, session_entry) !=
			eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE, FL("Unable to set CSA IE in beacon"));
		return;
	}

	/*
	 * First beacon update request is sent here, the remaining updates are
	 * done when the FW responds back after sending the first beacon after
	 * the template update
	 */
	lim_send_beacon_ind(mac_ctx, session_entry);
	lim_log(mac_ctx, LOG1, FL("Updated CSA IE, IE COUNT = %d"),
		       session_entry->gLimChannelSwitch.switchCount);
	/* Send ECSA Action frame after updating the beacon */
	send_extended_chan_switch_action_frame(mac_ctx,
		session_entry->gLimChannelSwitch.primaryChannel,
		session_entry->gLimChannelSwitch.ch_width,
					   session_entry);
	session_entry->gLimChannelSwitch.switchCount--;
}

/**
 * lim_process_ext_change_channel()- function to send ECSA
 * action frame for STA/CLI .
 * @mac_ctx: pointer to global mac structure
 * @msg: params from sme for new channel.
 *
 * This function is called to send ECSA frame for STA/CLI.
 *
 * Return: void
 */

static void lim_process_ext_change_channel(tpAniSirGlobal mac_ctx,
							uint32_t *msg)
{
	struct sir_sme_ext_cng_chan_req *ext_chng_channel =
				(struct sir_sme_ext_cng_chan_req *) msg;
	tpPESession session_entry = NULL;

	if (NULL == msg) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}
	session_entry =
		pe_find_session_by_sme_session_id(mac_ctx,
						ext_chng_channel->session_id);
	if (NULL == session_entry) {
		lim_log(mac_ctx, LOGE,
			FL("Session not found for given session %d"),
			ext_chng_channel->session_id);
		return;
	}
	if (LIM_IS_AP_ROLE(session_entry)) {
		lim_log(mac_ctx, LOGE,
			FL("not an STA/CLI session"));
		return;
	}
	send_extended_chan_switch_action_frame(mac_ctx,
			ext_chng_channel->new_channel,
				0, session_entry);
}

/**
 * lim_process_nss_update_request() - process sme nss update req
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function processes SME request messages from HDD or upper layer
 * application.
 *
 * Return: None
 */
static void lim_process_nss_update_request(tpAniSirGlobal mac_ctx,
		uint32_t *msg_buf)
{
	struct sir_nss_update_request *nss_update_req_ptr;
	tpPESession session_entry = NULL;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	nss_update_req_ptr = (struct sir_nss_update_request *)msg_buf;
	session_entry = pe_find_session_by_session_id(mac_ctx,
				nss_update_req_ptr->vdev_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE, FL(
			"Session not found for given session_id %d"),
			nss_update_req_ptr->vdev_id);
		return;
	}

	if (session_entry->valid && !LIM_IS_AP_ROLE(session_entry)) {
		lim_log(mac_ctx, LOGE, FL("Invalid SystemRole %d"),
			GET_LIM_SYSTEM_ROLE(session_entry));
		return;
	}

	/* populate nss field in the beacon */
	session_entry->gLimOperatingMode.present = 1;
	session_entry->gLimOperatingMode.rxNSS = nss_update_req_ptr->new_nss;
	/* Send nss update request from here */
	if (sch_set_fixed_beacon_fields(mac_ctx, session_entry) !=
			eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to set op mode IE in beacon"));
		return;
	}

	lim_send_beacon_ind(mac_ctx, session_entry);
}

/**
 * lim_process_set_ie_req() - process sme set IE request
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to the SME message buffer
 *
 * This function processes SME request messages from HDD or upper layer
 * application.
 *
 * Return: None
 */
static void lim_process_set_ie_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	struct send_extcap_ie *msg;
	CDF_STATUS status;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	msg = (struct send_extcap_ie *)msg_buf;
	status = lim_send_ext_cap_ie(mac_ctx, msg->session_id, NULL, false);
	if (CDF_STATUS_SUCCESS != status)
		lim_log(mac_ctx, LOGE, FL("Unable to send ExtCap to FW"));

}
