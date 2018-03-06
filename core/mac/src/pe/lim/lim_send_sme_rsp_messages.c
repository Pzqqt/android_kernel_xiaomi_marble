/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * This file lim_send_sme_rspMessages.cc contains the functions
 * for sending SME response/notification messages to applications
 * above MAC software.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "qdf_types.h"
#include "wni_api.h"
#include "sir_common.h"
#include "ani_global.h"

#include "wni_cfg.h"
#include "sys_def.h"
#include "cfg_api.h"

#include "sch_api.h"
#include "utils_api.h"
#include "lim_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_send_sme_rsp_messages.h"
#include "lim_ibss_peer_mgmt.h"
#include "lim_session_utils.h"
#include "lim_types.h"
#include "sir_api.h"
#include "cds_regdomain.h"
#include "lim_send_messages.h"
#include "nan_datapath.h"
#include "lim_assoc_utils.h"
#include "wlan_reg_services_api.h"
#include "wlan_utility.h"

#include "wlan_tdls_tgt_api.h"
#include "lim_process_fils.h"
#include "wma.h"

static void lim_handle_join_rsp_status(tpAniSirGlobal mac_ctx,
	tpPESession session_entry, tSirResultCodes result_code,
	tpSirSmeJoinRsp sme_join_rsp);

/**
 * lim_send_sme_rsp() - Send Response to upper layers
 * @mac_ctx:          Pointer to Global MAC structure
 * @msg_type:         Indicates message type
 * @result_code:       Indicates the result of previously issued
 *                    eWNI_SME_msg_type_REQ message
 *
 * This function is called by lim_process_sme_req_messages() to send
 * eWNI_SME_START_RSP, eWNI_SME_STOP_BSS_RSP
 * or eWNI_SME_SWITCH_CHL_RSP messages to applications above MAC
 * Software.
 *
 * Return: None
 */

void
lim_send_sme_rsp(tpAniSirGlobal mac_ctx, uint16_t msg_type,
	 tSirResultCodes result_code, uint8_t sme_session_id,
	 uint16_t sme_transaction_id)
{
	struct scheduler_msg msg = {0};
	tSirSmeRsp *sme_rsp;

	pe_debug("Sending message: %s with reasonCode: %s",
		lim_msg_str(msg_type), lim_result_code_str(result_code));

	sme_rsp = qdf_mem_malloc(sizeof(tSirSmeRsp));
	if (NULL == sme_rsp) {
		/* Buffer not available. Log error */
		pe_err("call to AllocateMemory failed for eWNI_SME_*_RSP");
		return;
	}

	sme_rsp->messageType = msg_type;
	sme_rsp->length = sizeof(tSirSmeRsp);
	sme_rsp->statusCode = result_code;

	sme_rsp->sessionId = sme_session_id;
	sme_rsp->transactionId = sme_transaction_id;

	msg.type = msg_type;
	msg.bodyptr = sme_rsp;
	msg.bodyval = 0;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TX_SME_MSG,
			 sme_session_id, msg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	switch (msg_type) {
	case eWNI_SME_STOP_BSS_RSP:
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_STOP_BSS_RSP_EVENT,
				NULL, (uint16_t) result_code, 0);
		break;
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_sys_process_mmh_msg_api(mac_ctx, &msg, ePROT);
}



/**
 * lim_send_sme_roc_rsp() - Send Response to SME
 * @mac_ctx:          Pointer to Global MAC structure
 * @status:           Resume link status
 * @result_code:  Result of the ROC request
 * @sme_session_id:   SME sesson Id
 * @scan_id:  Scan Identifier
 *
 * This function is called to send ROC rsp
 * message to SME.
 *
 * Return: None
 */
void
lim_send_sme_roc_rsp(tpAniSirGlobal mac_ctx, uint16_t msg_type,
	 tSirResultCodes result_code, uint8_t sme_session_id,
	 uint32_t scan_id)
{
	struct scheduler_msg msg = {0};
	struct sir_roc_rsp *sme_rsp;

	pe_debug("Sending message: %s with reasonCode: %s scanId: %d",
		lim_msg_str(msg_type), lim_result_code_str(result_code),
		scan_id);

	sme_rsp = qdf_mem_malloc(sizeof(struct sir_roc_rsp));
	if (NULL == sme_rsp) {
		pe_err("call to AllocateMemory failed for eWNI_SME_*_RSP");
		return;
	}

	sme_rsp->message_type = msg_type;
	sme_rsp->length = sizeof(struct sir_roc_rsp);
	sme_rsp->status = result_code;

	sme_rsp->session_id = sme_session_id;
	sme_rsp->scan_id = scan_id;

	msg.type = msg_type;
	msg.bodyptr = sme_rsp;
	msg.bodyval = 0;
	MTRACE(mac_trace_msg_tx(mac_ctx, sme_session_id, msg.type));
	lim_sys_process_mmh_msg_api(mac_ctx, &msg, ePROT);
}


/**
 * lim_get_max_rate_flags() - Get rate flags
 * @mac_ctx: Pointer to global MAC structure
 * @sta_ds: Pointer to station ds structure
 *
 * This function is called to get the rate flags for a connection
 * from the station ds structure depending on the ht and the vht
 * channel width supported.
 *
 * Return: Returns the populated rate_flags
 */
uint32_t lim_get_max_rate_flags(tpAniSirGlobal mac_ctx, tpDphHashNode sta_ds)
{
	uint32_t rate_flags = 0;

	if (sta_ds == NULL) {
		pe_err("sta_ds is NULL");
		return rate_flags;
	}

	if (!sta_ds->mlmStaContext.htCapability &&
	    !sta_ds->mlmStaContext.vhtCapability) {
		rate_flags |= eHAL_TX_RATE_LEGACY;
	} else {
		if (sta_ds->mlmStaContext.vhtCapability) {
			if (WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ ==
				sta_ds->vhtSupportedChannelWidthSet) {
				rate_flags |= eHAL_TX_RATE_VHT80;
			} else if (WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ ==
					sta_ds->vhtSupportedChannelWidthSet) {
				if (sta_ds->htSupportedChannelWidthSet)
					rate_flags |= eHAL_TX_RATE_VHT40;
				else
					rate_flags |= eHAL_TX_RATE_VHT20;
			}
		} else if (sta_ds->mlmStaContext.htCapability) {
			if (sta_ds->htSupportedChannelWidthSet)
				rate_flags |= eHAL_TX_RATE_HT40;
			else
				rate_flags |= eHAL_TX_RATE_HT20;
		}
	}

	if (sta_ds->htShortGI20Mhz || sta_ds->htShortGI40Mhz)
		rate_flags |= eHAL_TX_RATE_SGI;

	return rate_flags;
}

/**
 * lim_send_sme_join_reassoc_rsp_after_resume() - Send Response to SME
 * @mac_ctx          Pointer to Global MAC structure
 * @status           Resume link status
 * @ctx              context passed while calling resmune link.
 *                   (join response to be sent)
 *
 * This function is called to send Join/Reassoc rsp
 * message to SME after the resume link.
 *
 * Return: None
 */
static void lim_send_sme_join_reassoc_rsp_after_resume(tpAniSirGlobal mac_ctx,
	QDF_STATUS status, uint32_t *ctx)
{
	struct scheduler_msg msg = {0};
	tpSirSmeJoinRsp sme_join_rsp = (tpSirSmeJoinRsp) ctx;

	msg.type = sme_join_rsp->messageType;
	msg.bodyptr = sme_join_rsp;
	msg.bodyval = 0;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TX_SME_MSG, NO_SESSION, msg.type));
	lim_sys_process_mmh_msg_api(mac_ctx, &msg, ePROT);
}

/**
 * lim_handle_join_rsp_status() - Handle the response.
 * @mac_ctx:            Pointer to Global MAC structure
 * @session_entry:      PE Session Info
 * @result_code:        Indicates the result of previously issued
 *                      eWNI_SME_msgType_REQ message
 * @sme_join_rsp        The received response.
 *
 * This function will handle both the success and failure status
 * of the received response.
 *
 * Return: None
 */
static void lim_handle_join_rsp_status(tpAniSirGlobal mac_ctx,
	tpPESession session_entry, tSirResultCodes result_code,
	tpSirSmeJoinRsp sme_join_rsp)
{
	uint16_t bss_ie_len;
	void *bss_ies;
	bool is_vendor_ap_1_present;
	tpSirSmeJoinReq join_reassoc_req = NULL;

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	tSirSmeHTProfile *ht_profile;
#endif
	if (result_code == eSIR_SME_SUCCESS) {
		if (session_entry->beacon != NULL) {
			sme_join_rsp->beaconLength = session_entry->bcnLen;
			qdf_mem_copy(sme_join_rsp->frames,
				session_entry->beacon,
				sme_join_rsp->beaconLength);
			qdf_mem_free(session_entry->beacon);
			session_entry->beacon = NULL;
			session_entry->bcnLen = 0;
			pe_debug("Beacon: %d",
				sme_join_rsp->beaconLength);
		}
		if (session_entry->assocReq != NULL) {
			sme_join_rsp->assocReqLength =
				session_entry->assocReqLen;
			qdf_mem_copy(sme_join_rsp->frames +
				sme_join_rsp->beaconLength,
				session_entry->assocReq,
				sme_join_rsp->assocReqLength);
			qdf_mem_free(session_entry->assocReq);
			session_entry->assocReq = NULL;
			session_entry->assocReqLen = 0;
			pe_debug("AssocReq: %d",
				sme_join_rsp->assocReqLength);
		}
		if (session_entry->assocRsp != NULL) {
			sme_join_rsp->assocRspLength =
				session_entry->assocRspLen;
			qdf_mem_copy(sme_join_rsp->frames +
				sme_join_rsp->beaconLength +
				sme_join_rsp->assocReqLength,
				session_entry->assocRsp,
				sme_join_rsp->assocRspLength);
			qdf_mem_free(session_entry->assocRsp);
			session_entry->assocRsp = NULL;
			session_entry->assocRspLen = 0;
		}
		if (session_entry->ricData != NULL) {
			sme_join_rsp->parsedRicRspLen =
				session_entry->RICDataLen;
			qdf_mem_copy(sme_join_rsp->frames +
				sme_join_rsp->beaconLength +
				sme_join_rsp->assocReqLength +
				sme_join_rsp->assocRspLength,
				session_entry->ricData,
				sme_join_rsp->parsedRicRspLen);
			qdf_mem_free(session_entry->ricData);
			session_entry->ricData = NULL;
			session_entry->RICDataLen = 0;
			pe_debug("RicLength: %d",
				sme_join_rsp->parsedRicRspLen);
		}
#ifdef FEATURE_WLAN_ESE
		if (session_entry->tspecIes != NULL) {
			sme_join_rsp->tspecIeLen =
				session_entry->tspecLen;
			qdf_mem_copy(sme_join_rsp->frames +
				sme_join_rsp->beaconLength +
				sme_join_rsp->assocReqLength +
				sme_join_rsp->assocRspLength +
				sme_join_rsp->parsedRicRspLen,
				session_entry->tspecIes,
				sme_join_rsp->tspecIeLen);
			qdf_mem_free(session_entry->tspecIes);
			session_entry->tspecIes = NULL;
			session_entry->tspecLen = 0;
			pe_debug("ESE-TspecLen: %d",
				sme_join_rsp->tspecIeLen);
		}
#endif
		sme_join_rsp->aid = session_entry->limAID;
		pe_debug("AssocRsp: %d",
			sme_join_rsp->assocRspLength);
		sme_join_rsp->vht_channel_width =
			session_entry->ch_width;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (session_entry->cc_switch_mode !=
				QDF_MCC_TO_SCC_SWITCH_DISABLE) {
			ht_profile = &sme_join_rsp->HTProfile;
			ht_profile->htSupportedChannelWidthSet =
				session_entry->htSupportedChannelWidthSet;
			ht_profile->htRecommendedTxWidthSet =
				session_entry->htRecommendedTxWidthSet;
			ht_profile->htSecondaryChannelOffset =
				session_entry->htSecondaryChannelOffset;
			ht_profile->dot11mode = session_entry->dot11mode;
			ht_profile->htCapability = session_entry->htCapability;
			ht_profile->vhtCapability =
				session_entry->vhtCapability;
			ht_profile->apCenterChan = session_entry->ch_center_freq_seg0;
			ht_profile->apChanWidth = session_entry->ch_width;
		}
#endif
		pe_debug("pLimJoinReq:%pK, pLimReAssocReq:%pK",
			session_entry->pLimJoinReq,
			session_entry->pLimReAssocReq);

		if (session_entry->pLimJoinReq)
			join_reassoc_req = session_entry->pLimJoinReq;

		if (session_entry->pLimReAssocReq)
			join_reassoc_req = session_entry->pLimReAssocReq;

		if (!join_reassoc_req) {
			pe_err("both  pLimJoinReq and pLimReAssocReq NULL");
			return;
		}

		bss_ie_len = lim_get_ielen_from_bss_description(
				&join_reassoc_req->bssDescription);
		bss_ies = &join_reassoc_req->bssDescription.ieFields;
		is_vendor_ap_1_present = (wlan_get_vendor_ie_ptr_from_oui(
			SIR_MAC_VENDOR_AP_1_OUI, SIR_MAC_VENDOR_AP_1_OUI_LEN,
			bss_ies, bss_ie_len) != NULL);

		if (mac_ctx->roam.configParam.is_force_1x1 &&
		    is_vendor_ap_1_present && (session_entry->nss == 2) &&
		    (mac_ctx->lteCoexAntShare == 0 ||
				IS_5G_CH(session_entry->currentOperChannel))) {
			/* SET vdev param */
			pe_debug("sending SMPS intolrent vdev_param");
			wma_cli_set_command(session_entry->smeSessionId,
					   (int)WMI_VDEV_PARAM_SMPS_INTOLERANT,
					    1, VDEV_CMD);

		}
	} else {
		if (session_entry->beacon != NULL) {
			qdf_mem_free(session_entry->beacon);
			session_entry->beacon = NULL;
			session_entry->bcnLen = 0;
		}
		if (session_entry->assocReq != NULL) {
			qdf_mem_free(session_entry->assocReq);
			session_entry->assocReq = NULL;
			session_entry->assocReqLen = 0;
		}
		if (session_entry->assocRsp != NULL) {
			qdf_mem_free(session_entry->assocRsp);
			session_entry->assocRsp = NULL;
			session_entry->assocRspLen = 0;
		}
		if (session_entry->ricData != NULL) {
			qdf_mem_free(session_entry->ricData);
			session_entry->ricData = NULL;
			session_entry->RICDataLen = 0;
		}
#ifdef FEATURE_WLAN_ESE
		if (session_entry->tspecIes != NULL) {
			qdf_mem_free(session_entry->tspecIes);
			session_entry->tspecIes = NULL;
			session_entry->tspecLen = 0;
		}
#endif
	}
}

/**
 * lim_add_bss_info() - copy data from session entry to join rsp
 * @sta_ds: Station dph entry
 * @sme_join_rsp: Join response buffer to be filled up
 *
 * Return: None
 */
static void lim_add_bss_info(tpDphHashNode sta_ds, tpSirSmeJoinRsp sme_join_rsp)
{
	struct parsed_ies *parsed_ies = &sta_ds->parsed_ies;

	if (parsed_ies->hs20vendor_ie.present)
		sme_join_rsp->hs20vendor_ie = parsed_ies->hs20vendor_ie;
	if (parsed_ies->vht_caps.present)
		sme_join_rsp->vht_caps = parsed_ies->vht_caps;
	if (parsed_ies->ht_caps.present)
		sme_join_rsp->ht_caps = parsed_ies->ht_caps;
	if (parsed_ies->ht_operation.present)
		sme_join_rsp->ht_operation = parsed_ies->ht_operation;
	if (parsed_ies->vht_operation.present)
		sme_join_rsp->vht_operation = parsed_ies->vht_operation;
}

#ifdef WLAN_FEATURE_FILS_SK
static void lim_update_fils_seq_num(tpSirSmeJoinRsp sme_join_rsp,
				    tpPESession session_entry)
{
	sme_join_rsp->fils_seq_num =
		session_entry->fils_info->sequence_number;
}
#else
static inline void lim_update_fils_seq_num(tpSirSmeJoinRsp sme_join_rsp,
					   tpPESession session_entry)
{}
#endif
/**
 * lim_send_sme_join_reassoc_rsp() - Send Response to Upper Layers
 * @mac_ctx:            Pointer to Global MAC structure
 * @msg_type:           Indicates message type
 * @result_code:        Indicates the result of previously issued
 *                      eWNI_SME_msgType_REQ message
 * @prot_status_code:   Protocol Status Code
 * @session_entry:      PE Session Info
 * @sme_session_id:     SME Session ID
 * @sme_transaction_id: SME Transaction ID
 *
 * This function is called by lim_process_sme_req_messages() to send
 * eWNI_SME_JOIN_RSP or eWNI_SME_REASSOC_RSP messages to applications
 * above MAC Software.
 *
 * Return: None
 */

void
lim_send_sme_join_reassoc_rsp(tpAniSirGlobal mac_ctx, uint16_t msg_type,
	tSirResultCodes result_code, uint16_t prot_status_code,
	tpPESession session_entry, uint8_t sme_session_id,
	uint16_t sme_transaction_id)
{
	tpSirSmeJoinRsp sme_join_rsp;
	uint32_t rsp_len;
	tpDphHashNode sta_ds = NULL;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	if (msg_type == eWNI_SME_REASSOC_RSP)
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_REASSOC_RSP_EVENT,
			session_entry, (uint16_t) result_code, 0);
	else
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_JOIN_RSP_EVENT,
			session_entry, (uint16_t) result_code, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	pe_debug("Sending message: %s with reasonCode: %s",
		lim_msg_str(msg_type), lim_result_code_str(result_code));

	if (session_entry == NULL) {
		rsp_len = sizeof(tSirSmeJoinRsp);
		sme_join_rsp = qdf_mem_malloc(rsp_len);
		if (NULL == sme_join_rsp) {
			pe_err("Mem Alloc fail - JOIN/REASSOC_RSP");
			return;
		}

		sme_join_rsp->beaconLength = 0;
		sme_join_rsp->assocReqLength = 0;
		sme_join_rsp->assocRspLength = 0;
	} else {
		rsp_len = session_entry->assocReqLen +
			session_entry->assocRspLen + session_entry->bcnLen +
			session_entry->RICDataLen +
#ifdef FEATURE_WLAN_ESE
			session_entry->tspecLen +
#endif
			sizeof(tSirSmeJoinRsp) - sizeof(uint8_t);
		sme_join_rsp = qdf_mem_malloc(rsp_len);
		if (NULL == sme_join_rsp) {
			pe_err("MemAlloc fail - JOIN/REASSOC_RSP");
			return;
		}

		if (lim_is_fils_connection(session_entry)) {
			sme_join_rsp->is_fils_connection = true;
			lim_update_fils_seq_num(sme_join_rsp,
						session_entry);
		}

		if (result_code == eSIR_SME_SUCCESS) {
			sta_ds = dph_get_hash_entry(mac_ctx,
				DPH_STA_HASH_INDEX_PEER,
				&session_entry->dph.dphHashTable);
			if (sta_ds == NULL) {
				pe_err("Get Self Sta Entry fail");
			} else {
				/* Pass the peer's staId */
				sme_join_rsp->staId = sta_ds->staIndex;
				sme_join_rsp->ucastSig =
					sta_ds->ucUcastSig;
				sme_join_rsp->bcastSig =
					sta_ds->ucBcastSig;
				sme_join_rsp->timingMeasCap =
					sta_ds->timingMeasCap;
#ifdef FEATURE_WLAN_TDLS
				sme_join_rsp->tdls_prohibited =
					session_entry->tdls_prohibited;
				sme_join_rsp->tdls_chan_swit_prohibited =
				   session_entry->tdls_chan_swit_prohibited;
#endif
				sme_join_rsp->nss = sta_ds->nss;
				sme_join_rsp->max_rate_flags =
					lim_get_max_rate_flags(mac_ctx, sta_ds);
				lim_add_bss_info(sta_ds, sme_join_rsp);

				/* Copy FILS params only for Successful join */
				populate_fils_connect_params(mac_ctx,
						session_entry, sme_join_rsp);
			}
		}

		sme_join_rsp->beaconLength = 0;
		sme_join_rsp->assocReqLength = 0;
		sme_join_rsp->assocRspLength = 0;
		sme_join_rsp->parsedRicRspLen = 0;
#ifdef FEATURE_WLAN_ESE
		sme_join_rsp->tspecIeLen = 0;
#endif
		lim_handle_join_rsp_status(mac_ctx, session_entry, result_code,
			sme_join_rsp);

		/* Send supported NSS 1x1 to SME */
		sme_join_rsp->supported_nss_1x1 =
			session_entry->supported_nss_1x1;
		pe_debug("SME Join Rsp is supported NSS 1X1: %d",
		       sme_join_rsp->supported_nss_1x1);
	}

	sme_join_rsp->messageType = msg_type;
	sme_join_rsp->length = (uint16_t) rsp_len;
	sme_join_rsp->statusCode = result_code;
	sme_join_rsp->protStatusCode = prot_status_code;

	sme_join_rsp->sessionId = sme_session_id;
	sme_join_rsp->transactionId = sme_transaction_id;

	lim_send_sme_join_reassoc_rsp_after_resume(mac_ctx, QDF_STATUS_SUCCESS,
			(uint32_t *)sme_join_rsp);
}

/**
 * lim_send_sme_start_bss_rsp()
 *
 ***FUNCTION:
 * This function is called to send eWNI_SME_START_BSS_RSP
 * message to applications above MAC Software.
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
 * @param pMac         Pointer to Global MAC structure
 * @param msgType      Indicates message type
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_msgType_REQ message
 *
 * @return None
 */

void
lim_send_sme_start_bss_rsp(tpAniSirGlobal pMac,
			   uint16_t msgType, tSirResultCodes resultCode,
			   tpPESession psessionEntry, uint8_t smesessionId,
			   uint16_t smetransactionId)
{

	uint16_t size = 0;
	struct scheduler_msg mmhMsg = {0};
	tSirSmeStartBssRsp *pSirSmeRsp;
	uint16_t ieLen;
	uint16_t ieOffset, curLen;

	pe_debug("Sending message: %s with reasonCode: %s",
		       lim_msg_str(msgType), lim_result_code_str(resultCode));

	size = sizeof(tSirSmeStartBssRsp);

	if (psessionEntry == NULL) {
		pSirSmeRsp = qdf_mem_malloc(size);
		if (NULL == pSirSmeRsp) {
			/* / Buffer not available. Log error */
			pe_err("call to AllocateMemory failed for eWNI_SME_START_BSS_RSP");
			return;
		}
	} else {
		/* subtract size of beaconLength + Mac Hdr + Fixed Fields before SSID */
		ieOffset = sizeof(tAniBeaconStruct) + SIR_MAC_B_PR_SSID_OFFSET;
		ieLen = psessionEntry->schBeaconOffsetBegin
			+ psessionEntry->schBeaconOffsetEnd - ieOffset;
		/* calculate the memory size to allocate */
		size += ieLen;

		pSirSmeRsp = qdf_mem_malloc(size);
		if (NULL == pSirSmeRsp) {
			/* / Buffer not available. Log error */
			pe_err("call to AllocateMemory failed for eWNI_SME_START_BSS_RSP");
			return;
		}
		size = sizeof(tSirSmeStartBssRsp);
		if (resultCode == eSIR_SME_SUCCESS) {

			sir_copy_mac_addr(pSirSmeRsp->bssDescription.bssId,
					  psessionEntry->bssId);

			/* Read beacon interval from session */
			pSirSmeRsp->bssDescription.beaconInterval =
				(uint16_t) psessionEntry->beaconParams.
				beaconInterval;
			pSirSmeRsp->bssType = psessionEntry->bssType;

			if (cfg_get_capability_info
				    (pMac, &pSirSmeRsp->bssDescription.capabilityInfo,
				    psessionEntry)
			    != eSIR_SUCCESS)
				pe_err("could not retrieve Capabilities value");

			lim_get_phy_mode(pMac,
					 (uint32_t *) &pSirSmeRsp->bssDescription.
					 nwType, psessionEntry);

			pSirSmeRsp->bssDescription.channelId =
				psessionEntry->currentOperChannel;

		if (!LIM_IS_NDI_ROLE(psessionEntry)) {
			curLen = psessionEntry->schBeaconOffsetBegin - ieOffset;
			qdf_mem_copy((uint8_t *) &pSirSmeRsp->bssDescription.
				     ieFields,
				     psessionEntry->pSchBeaconFrameBegin +
				     ieOffset, (uint32_t) curLen);

			qdf_mem_copy(((uint8_t *) &pSirSmeRsp->bssDescription.
				      ieFields) + curLen,
				     psessionEntry->pSchBeaconFrameEnd,
				     (uint32_t) psessionEntry->
				     schBeaconOffsetEnd);

			pSirSmeRsp->bssDescription.length = (uint16_t)
				(offsetof(tSirBssDescription, ieFields[0])
				- sizeof(pSirSmeRsp->bssDescription.length)
				+ ieLen);
			/* This is the size of the message, subtracting the size of the pointer to ieFields */
			size += ieLen - sizeof(uint32_t);
		}
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
			if (psessionEntry->cc_switch_mode
			    != QDF_MCC_TO_SCC_SWITCH_DISABLE) {
				pSirSmeRsp->HTProfile.
				htSupportedChannelWidthSet =
					psessionEntry->htSupportedChannelWidthSet;
				pSirSmeRsp->HTProfile.htRecommendedTxWidthSet =
					psessionEntry->htRecommendedTxWidthSet;
				pSirSmeRsp->HTProfile.htSecondaryChannelOffset =
					psessionEntry->htSecondaryChannelOffset;
				pSirSmeRsp->HTProfile.dot11mode =
					psessionEntry->dot11mode;
				pSirSmeRsp->HTProfile.htCapability =
					psessionEntry->htCapability;
				pSirSmeRsp->HTProfile.vhtCapability =
					psessionEntry->vhtCapability;
				pSirSmeRsp->HTProfile.apCenterChan =
					psessionEntry->ch_center_freq_seg0;
				pSirSmeRsp->HTProfile.apChanWidth =
					psessionEntry->ch_width;
			}
#endif
		}
	}
	pSirSmeRsp->messageType = msgType;
	pSirSmeRsp->length = size;

	/* Update SME session Id and transaction Id */
	pSirSmeRsp->sessionId = smesessionId;
	pSirSmeRsp->transactionId = smetransactionId;
	pSirSmeRsp->statusCode = resultCode;
	if (psessionEntry != NULL)
		pSirSmeRsp->staId = psessionEntry->staId;       /* else it will be always zero smeRsp StaID = 0 */

	mmhMsg.type = msgType;
	mmhMsg.bodyptr = pSirSmeRsp;
	mmhMsg.bodyval = 0;
	if (psessionEntry == NULL) {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 psessionEntry->peSessionId, mmhMsg.type));
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_START_BSS_RSP_EVENT,
			      psessionEntry, (uint16_t) resultCode, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
} /*** end lim_send_sme_start_bss_rsp() ***/

void lim_send_sme_disassoc_deauth_ntf(tpAniSirGlobal pMac,
				      QDF_STATUS status, uint32_t *pCtx)
{
	struct scheduler_msg mmhMsg = {0};
	struct scheduler_msg *pMsg = (struct scheduler_msg *) pCtx;

	mmhMsg.type = pMsg->type;
	mmhMsg.bodyptr = pMsg;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG, NO_SESSION, mmhMsg.type));

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
}

/**
 * lim_send_sme_disassoc_ntf()
 *
 ***FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_DISASSOC_RSP/IND message to host
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * This function is used for sending eWNI_SME_DISASSOC_CNF,
 * or eWNI_SME_DISASSOC_IND to host depending on
 * disassociation trigger.
 *
 * @param peerMacAddr       Indicates the peer MAC addr to which
 *                          disassociate was initiated
 * @param reasonCode        Indicates the reason for Disassociation
 * @param disassocTrigger   Indicates the trigger for Disassociation
 * @param aid               Indicates the STAID. This parameter is
 *                          present only on AP.
 *
 * @return None
 */
void
lim_send_sme_disassoc_ntf(tpAniSirGlobal pMac,
			  tSirMacAddr peerMacAddr,
			  tSirResultCodes reasonCode,
			  uint16_t disassocTrigger,
			  uint16_t aid,
			  uint8_t smesessionId,
			  uint16_t smetransactionId, tpPESession psessionEntry)
{

	uint8_t *pBuf;
	tSirSmeDisassocRsp *pSirSmeDisassocRsp;
	tSirSmeDisassocInd *pSirSmeDisassocInd;
	uint32_t *pMsg = NULL;
	bool failure = false;
	tpPESession session = NULL;
	uint16_t i, assoc_id;
	tpDphHashNode sta_ds = NULL;
	struct sir_sme_discon_done_ind *sir_sme_dis_ind;

	pe_debug("Disassoc Ntf with trigger : %d reasonCode: %d",
		disassocTrigger, reasonCode);

	switch (disassocTrigger) {
	case eLIM_DUPLICATE_ENTRY:
		/*
		 * Duplicate entry is removed at LIM.
		 * Initiate new entry for other session
		 */
		pe_debug("Rcvd eLIM_DUPLICATE_ENTRY for " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(peerMacAddr));

		for (i = 0; i < pMac->lim.maxBssId; i++) {
			if ((&pMac->lim.gpSession[i] != NULL) &&
					(pMac->lim.gpSession[i].valid) &&
					(pMac->lim.gpSession[i].pePersona ==
								QDF_SAP_MODE)) {
				/* Find the sta ds entry in another session */
				session = &pMac->lim.gpSession[i];
				sta_ds = dph_lookup_hash_entry(pMac,
						peerMacAddr, &assoc_id,
						&session->dph.dphHashTable);
				if (sta_ds)
					break;
			}
		}
		if (sta_ds
#ifdef WLAN_FEATURE_11W
			&& (!sta_ds->rmfEnabled)
#endif
		) {
			if (lim_add_sta(pMac, sta_ds, false, session) !=
					eSIR_SUCCESS)
					pe_err("could not Add STA with assocId: %d",
					sta_ds->assocId);
		}
		failure = true;
		break;

	case eLIM_HOST_DISASSOC:
		/**
		 * Disassociation response due to
		 * host triggered disassociation
		 */

		pSirSmeDisassocRsp = qdf_mem_malloc(sizeof(tSirSmeDisassocRsp));
		if (NULL == pSirSmeDisassocRsp) {
			/* Log error */
			pe_err("Memory allocation failed");
			failure = true;
			goto error;
		}
		pe_debug("send eWNI_SME_DISASSOC_RSP with retCode: %d for " MAC_ADDRESS_STR,
			reasonCode, MAC_ADDR_ARRAY(peerMacAddr));
		pSirSmeDisassocRsp->messageType = eWNI_SME_DISASSOC_RSP;
		pSirSmeDisassocRsp->length = sizeof(tSirSmeDisassocRsp);
		/* sessionId */
		pBuf = (uint8_t *) &pSirSmeDisassocRsp->sessionId;
		*pBuf = smesessionId;
		pBuf++;

		/* transactionId */
		lim_copy_u16(pBuf, smetransactionId);
		pBuf += sizeof(uint16_t);

		/* statusCode */
		lim_copy_u32(pBuf, reasonCode);
		pBuf += sizeof(tSirResultCodes);

		/* peerMacAddr */
		qdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		/* Clear Station Stats */
		/* for sta, it is always 1, IBSS is handled at halInitSta */

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */

		lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_RSP_EVENT,
				      psessionEntry, (uint16_t) reasonCode, 0);
#endif
		pMsg = (uint32_t *) pSirSmeDisassocRsp;
		break;

	case eLIM_PEER_ENTITY_DISASSOC:
	case eLIM_LINK_MONITORING_DISASSOC:
		sir_sme_dis_ind =
			qdf_mem_malloc(sizeof(*sir_sme_dis_ind));
		if (!sir_sme_dis_ind) {
			pe_err("call to AllocateMemory failed for disconnect indication");
			return;
		}

		pe_debug("send  eWNI_SME_DISCONNECT_DONE_IND with retCode: %d",
				reasonCode);

		sir_sme_dis_ind->message_type =
			eWNI_SME_DISCONNECT_DONE_IND;
		sir_sme_dis_ind->length =
			sizeof(*sir_sme_dis_ind);
		qdf_mem_copy(sir_sme_dis_ind->peer_mac, peerMacAddr,
			     sizeof(tSirMacAddr));
		sir_sme_dis_ind->session_id   = smesessionId;
		sir_sme_dis_ind->reason_code  = reasonCode;
		/*
		 * Instead of sending deauth reason code as 505 which is
		 * internal value(eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE)
		 * Send reason code as zero to Supplicant
		 */
		if (reasonCode == eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE)
			sir_sme_dis_ind->reason_code = 0;
		else
			sir_sme_dis_ind->reason_code = reasonCode;

		pMsg = (uint32_t *)sir_sme_dis_ind;

		break;

	default:
		/**
		 * Disassociation indication due to Disassociation
		 * frame reception from peer entity or due to
		 * loss of link with peer entity.
		 */
		pSirSmeDisassocInd = qdf_mem_malloc(sizeof(tSirSmeDisassocInd));
		if (NULL == pSirSmeDisassocInd) {
			/* Log error */
			pe_err("Memory allocation failed");
			failure = true;
			goto error;
		}
		pe_debug("send eWNI_SME_DISASSOC_IND with retCode: %d for " MAC_ADDRESS_STR,
			reasonCode, MAC_ADDR_ARRAY(peerMacAddr));
		pSirSmeDisassocInd->messageType = eWNI_SME_DISASSOC_IND;
		pSirSmeDisassocInd->length = sizeof(tSirSmeDisassocInd);

		/* Update SME session Id and Transaction Id */
		pSirSmeDisassocInd->sessionId = smesessionId;
		pSirSmeDisassocInd->transactionId = smetransactionId;
		pSirSmeDisassocInd->reasonCode = reasonCode;
		pBuf = (uint8_t *) &pSirSmeDisassocInd->statusCode;

		lim_copy_u32(pBuf, reasonCode);
		pBuf += sizeof(tSirResultCodes);

		qdf_mem_copy(pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		qdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_IND_EVENT,
				      psessionEntry, (uint16_t) reasonCode, 0);
#endif
		pMsg = (uint32_t *) pSirSmeDisassocInd;

		break;
	}

error:
	/* Delete the PE session Created */
	if ((psessionEntry != NULL) && LIM_IS_STA_ROLE(psessionEntry))
		pe_delete_session(pMac, psessionEntry);

	if (false == failure)
		lim_send_sme_disassoc_deauth_ntf(pMac, QDF_STATUS_SUCCESS,
						 (uint32_t *) pMsg);
} /*** end lim_send_sme_disassoc_ntf() ***/

/** -----------------------------------------------------------------
   \brief lim_send_sme_disassoc_ind() - sends SME_DISASSOC_IND

   After receiving disassociation frame from peer entity, this
   function sends a eWNI_SME_DISASSOC_IND to SME with a specific
   reason code.

   \param pMac - global mac structure
   \param pStaDs - station dph hash node
   \return none
   \sa
   ----------------------------------------------------------------- */
void
lim_send_sme_disassoc_ind(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
			  tpPESession psessionEntry)
{
	struct scheduler_msg mmhMsg = {0};
	tSirSmeDisassocInd *pSirSmeDisassocInd;

	pSirSmeDisassocInd = qdf_mem_malloc(sizeof(tSirSmeDisassocInd));
	if (NULL == pSirSmeDisassocInd) {
		pe_err("AllocateMemory failed for eWNI_SME_DISASSOC_IND");
		return;
	}

	pSirSmeDisassocInd->messageType = eWNI_SME_DISASSOC_IND;
	pSirSmeDisassocInd->length = sizeof(tSirSmeDisassocInd);

	pSirSmeDisassocInd->sessionId = psessionEntry->smeSessionId;
	pSirSmeDisassocInd->transactionId = psessionEntry->transactionId;
	pSirSmeDisassocInd->statusCode = eSIR_SME_DEAUTH_STATUS;
	pSirSmeDisassocInd->reasonCode = pStaDs->mlmStaContext.disassocReason;

	qdf_mem_copy(pSirSmeDisassocInd->bssid.bytes, psessionEntry->bssId,
		     QDF_MAC_ADDR_SIZE);

	qdf_mem_copy(pSirSmeDisassocInd->peer_macaddr.bytes, pStaDs->staAddr,
		     QDF_MAC_ADDR_SIZE);

	pSirSmeDisassocInd->staId = pStaDs->staIndex;

	mmhMsg.type = eWNI_SME_DISASSOC_IND;
	mmhMsg.bodyptr = pSirSmeDisassocInd;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
			 psessionEntry->peSessionId, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_IND_EVENT, psessionEntry,
			      0, (uint16_t) pStaDs->mlmStaContext.disassocReason);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

} /*** end lim_send_sme_disassoc_ind() ***/

/** -----------------------------------------------------------------
   \brief lim_send_sme_deauth_ind() - sends SME_DEAUTH_IND

   After receiving deauthentication frame from peer entity, this
   function sends a eWNI_SME_DEAUTH_IND to SME with a specific
   reason code.

   \param pMac - global mac structure
   \param pStaDs - station dph hash node
   \return none
   \sa
   ----------------------------------------------------------------- */
void
lim_send_sme_deauth_ind(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
			tpPESession psessionEntry)
{
	struct scheduler_msg mmhMsg = {0};
	tSirSmeDeauthInd *pSirSmeDeauthInd;

	pSirSmeDeauthInd = qdf_mem_malloc(sizeof(tSirSmeDeauthInd));
	if (NULL == pSirSmeDeauthInd) {
		pe_err("AllocateMemory failed for eWNI_SME_DEAUTH_IND");
		return;
	}

	pSirSmeDeauthInd->messageType = eWNI_SME_DEAUTH_IND;
	pSirSmeDeauthInd->length = sizeof(tSirSmeDeauthInd);

	pSirSmeDeauthInd->sessionId = psessionEntry->smeSessionId;
	pSirSmeDeauthInd->transactionId = psessionEntry->transactionId;
	if (eSIR_INFRA_AP_MODE == psessionEntry->bssType) {
		pSirSmeDeauthInd->statusCode =
			(tSirResultCodes) pStaDs->mlmStaContext.cleanupTrigger;
	} else {
		/* Need to indicatet he reascon code over the air */
		pSirSmeDeauthInd->statusCode =
			(tSirResultCodes) pStaDs->mlmStaContext.disassocReason;
	}
	/* BSSID */
	qdf_mem_copy(pSirSmeDeauthInd->bssid.bytes, psessionEntry->bssId,
		     QDF_MAC_ADDR_SIZE);
	/* peerMacAddr */
	qdf_mem_copy(pSirSmeDeauthInd->peer_macaddr.bytes, pStaDs->staAddr,
		     QDF_MAC_ADDR_SIZE);
	pSirSmeDeauthInd->reasonCode = pStaDs->mlmStaContext.disassocReason;

	pSirSmeDeauthInd->staId = pStaDs->staIndex;
	if (eSIR_MAC_PEER_STA_REQ_LEAVING_BSS_REASON ==
		pStaDs->mlmStaContext.disassocReason)
		pSirSmeDeauthInd->rssi = pStaDs->del_sta_ctx_rssi;

	mmhMsg.type = eWNI_SME_DEAUTH_IND;
	mmhMsg.bodyptr = pSirSmeDeauthInd;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DEAUTH_IND_EVENT, psessionEntry,
			      0, pStaDs->mlmStaContext.cleanupTrigger);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
} /*** end lim_send_sme_deauth_ind() ***/

#ifdef FEATURE_WLAN_TDLS
/**
 * lim_send_sme_tdls_del_sta_ind()
 *
 ***FUNCTION:
 * This function is called to send the TDLS STA context deletion to SME.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 * NA
 *
 * @param  pMac   - Pointer to global MAC structure
 * @param  pStaDs - Pointer to internal STA Datastructure
 * @param  psessionEntry - Pointer to the session entry
 * @param  reasonCode - Reason for TDLS sta deletion
 * @return None
 */
void
lim_send_sme_tdls_del_sta_ind(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
			      tpPESession psessionEntry, uint16_t reasonCode)
{
	struct tdls_event_info info;

	pe_debug("Delete TDLS Peer "MAC_ADDRESS_STR "with reason code: %d",
			MAC_ADDR_ARRAY(pStaDs->staAddr), reasonCode);
	info.vdev_id = psessionEntry->smeSessionId;
	qdf_mem_copy(info.peermac.bytes, pStaDs->staAddr, QDF_MAC_ADDR_SIZE);
	info.message_type = TDLS_PEER_DISCONNECTED;
	info.peer_reason = TDLS_DISCONNECTED_PEER_DELETE;

	tgt_tdls_event_handler(pMac->psoc, &info);

	return;
} /*** end lim_send_sme_tdls_del_sta_ind() ***/

/**
 * lim_send_sme_mgmt_tx_completion()
 *
 ***FUNCTION:
 * This function is called to send the eWNI_SME_MGMT_FRM_TX_COMPLETION_IND
 * message to SME.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 * NA
 *
 * @param  pMac   - Pointer to global MAC structure
 * @param  psessionEntry - Pointer to the session entry
 * @param  txCompleteStatus - TX Complete Status of Mgmt Frames
 * @return None
 */
void
lim_send_sme_mgmt_tx_completion(tpAniSirGlobal pMac,
				uint32_t sme_session_id,
				uint32_t txCompleteStatus)
{
	struct scheduler_msg mmhMsg = {0};
	tSirMgmtTxCompletionInd *pSirMgmtTxCompletionInd;

	pSirMgmtTxCompletionInd =
		qdf_mem_malloc(sizeof(tSirMgmtTxCompletionInd));
	if (NULL == pSirMgmtTxCompletionInd) {
		pe_err("AllocateMemory failed for eWNI_SME_MGMT_FRM_TX_COMPLETION_IND");
		return;
	}
	/* messageType */
	pSirMgmtTxCompletionInd->messageType =
		eWNI_SME_MGMT_FRM_TX_COMPLETION_IND;
	pSirMgmtTxCompletionInd->length = sizeof(tSirMgmtTxCompletionInd);

	/* sessionId */
	pSirMgmtTxCompletionInd->sessionId = sme_session_id;

	pSirMgmtTxCompletionInd->txCompleteStatus = txCompleteStatus;

	mmhMsg.type = eWNI_SME_MGMT_FRM_TX_COMPLETION_IND;
	mmhMsg.bodyptr = pSirMgmtTxCompletionInd;
	mmhMsg.bodyval = 0;

	pSirMgmtTxCompletionInd->psoc = pMac->psoc;
	mmhMsg.callback = tgt_tdls_send_mgmt_tx_completion;
	scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &mmhMsg);
	return;
} /*** end lim_send_sme_tdls_delete_all_peer_ind() ***/

#endif /* FEATURE_WLAN_TDLS */

/**
 * lim_send_sme_deauth_ntf()
 *
 ***FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_DISASSOC_RSP/IND message to host
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * This function is used for sending eWNI_SME_DEAUTH_CNF or
 * eWNI_SME_DEAUTH_IND to host depending on deauthentication trigger.
 *
 * @param peerMacAddr       Indicates the peer MAC addr to which
 *                          deauthentication was initiated
 * @param reasonCode        Indicates the reason for Deauthetication
 * @param deauthTrigger     Indicates the trigger for Deauthetication
 * @param aid               Indicates the STAID. This parameter is present
 *                          only on AP.
 *
 * @return None
 */
void
lim_send_sme_deauth_ntf(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
			tSirResultCodes reasonCode, uint16_t deauthTrigger,
			uint16_t aid, uint8_t smesessionId,
			uint16_t smetransactionId)
{
	uint8_t *pBuf;
	tSirSmeDeauthRsp *pSirSmeDeauthRsp;
	tSirSmeDeauthInd *pSirSmeDeauthInd;
	tpPESession psessionEntry;
	uint8_t sessionId;
	uint32_t *pMsg;
	struct sir_sme_discon_done_ind *sir_sme_dis_ind;

	psessionEntry = pe_find_session_by_bssid(pMac, peerMacAddr, &sessionId);
	switch (deauthTrigger) {
	case eLIM_HOST_DEAUTH:
		/**
		 * Deauthentication response to host triggered
		 * deauthentication.
		 */
		pSirSmeDeauthRsp = qdf_mem_malloc(sizeof(tSirSmeDeauthRsp));
		if (NULL == pSirSmeDeauthRsp) {
			/* Log error */
			pe_err("call to AllocateMemory failed for eWNI_SME_DEAUTH_RSP");
			return;
		}
		pe_debug("send eWNI_SME_DEAUTH_RSP with retCode: %d for" MAC_ADDRESS_STR,
			reasonCode, MAC_ADDR_ARRAY(peerMacAddr));
		pSirSmeDeauthRsp->messageType = eWNI_SME_DEAUTH_RSP;
		pSirSmeDeauthRsp->length = sizeof(tSirSmeDeauthRsp);
		pSirSmeDeauthRsp->statusCode = reasonCode;
		pSirSmeDeauthRsp->sessionId = smesessionId;
		pSirSmeDeauthRsp->transactionId = smetransactionId;

		pBuf = (uint8_t *) pSirSmeDeauthRsp->peer_macaddr.bytes;
		qdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DEAUTH_RSP_EVENT,
				      psessionEntry, 0, (uint16_t) reasonCode);
#endif
		pMsg = (uint32_t *) pSirSmeDeauthRsp;

		break;

	case eLIM_PEER_ENTITY_DEAUTH:
	case eLIM_LINK_MONITORING_DEAUTH:
		sir_sme_dis_ind =
			qdf_mem_malloc(sizeof(*sir_sme_dis_ind));
		if (!sir_sme_dis_ind) {
			pe_err("call to AllocateMemory failed for disconnect indication");
			return;
		}

		pe_debug("send eWNI_SME_DISCONNECT_DONE_IND withretCode: %d",
				reasonCode);

		sir_sme_dis_ind->message_type =
			eWNI_SME_DISCONNECT_DONE_IND;
		sir_sme_dis_ind->length =
			sizeof(*sir_sme_dis_ind);
		sir_sme_dis_ind->session_id = smesessionId;
		sir_sme_dis_ind->reason_code = reasonCode;
		qdf_mem_copy(sir_sme_dis_ind->peer_mac, peerMacAddr,
			 ETH_ALEN);
		/*
		 * Instead of sending deauth reason code as 505 which is
		 * internal value(eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE)
		 * Send reason code as zero to Supplicant
		 */
		if (reasonCode == eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE)
			sir_sme_dis_ind->reason_code = 0;
		else
			sir_sme_dis_ind->reason_code = reasonCode;

		pMsg = (uint32_t *)sir_sme_dis_ind;

		break;

	default:
		/**
		 * Deauthentication indication due to Deauthentication
		 * frame reception from peer entity or due to
		 * loss of link with peer entity.
		 */
		pSirSmeDeauthInd = qdf_mem_malloc(sizeof(tSirSmeDeauthInd));
		if (NULL == pSirSmeDeauthInd) {
			/* Log error */
			pe_err("call to AllocateMemory failed for eWNI_SME_DEAUTH_Ind");
			return;
		}
		pe_debug("send eWNI_SME_DEAUTH_IND with retCode: %d for " MAC_ADDRESS_STR,
			reasonCode, MAC_ADDR_ARRAY(peerMacAddr));
		pSirSmeDeauthInd->messageType = eWNI_SME_DEAUTH_IND;
		pSirSmeDeauthInd->length = sizeof(tSirSmeDeauthInd);
		pSirSmeDeauthInd->reasonCode = eSIR_MAC_UNSPEC_FAILURE_REASON;

		/* sessionId */
		pBuf = (uint8_t *) &pSirSmeDeauthInd->sessionId;
		*pBuf++ = smesessionId;

		/* transaction ID */
		lim_copy_u16(pBuf, smetransactionId);
		pBuf += sizeof(uint16_t);

		/* status code */
		lim_copy_u32(pBuf, reasonCode);
		pBuf += sizeof(tSirResultCodes);

		/* bssId */
		qdf_mem_copy(pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		/* peerMacAddr */
		qdf_mem_copy(pSirSmeDeauthInd->peer_macaddr.bytes, peerMacAddr,
			     QDF_MAC_ADDR_SIZE);

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DEAUTH_IND_EVENT,
				      psessionEntry, 0, (uint16_t) reasonCode);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
		pMsg = (uint32_t *) pSirSmeDeauthInd;

		break;
	}

	/*Delete the PE session  created */
	if (psessionEntry != NULL) {
		pe_delete_session(pMac, psessionEntry);
	}

	lim_send_sme_disassoc_deauth_ntf(pMac, QDF_STATUS_SUCCESS,
					 (uint32_t *) pMsg);

} /*** end lim_send_sme_deauth_ntf() ***/

/**
 * lim_send_sme_wm_status_change_ntf() - Send Notification
 * @mac_ctx:             Global MAC Context
 * @status_change_code:  Indicates the change in the wireless medium.
 * @status_change_info:  Indicates the information associated with
 *                       change in the wireless medium.
 * @info_len:            Indicates the length of status change information
 *                       being sent.
 * @session_id           SessionID
 *
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_WM_STATUS_CHANGE_NTF message to host.
 *
 * Return: None
 */
void
lim_send_sme_wm_status_change_ntf(tpAniSirGlobal mac_ctx,
	tSirSmeStatusChangeCode status_change_code,
	uint32_t *status_change_info, uint16_t info_len, uint8_t session_id)
{
	struct scheduler_msg msg = {0};
	tSirSmeWmStatusChangeNtf *wm_status_change_ntf;
	uint32_t max_info_len;

	wm_status_change_ntf = qdf_mem_malloc(sizeof(tSirSmeWmStatusChangeNtf));
	if (NULL == wm_status_change_ntf) {
		pe_err("Mem Alloc failed - eWNI_SME_WM_STATUS_CHANGE_NTF");
		return;
	}

	msg.type = eWNI_SME_WM_STATUS_CHANGE_NTF;
	msg.bodyval = 0;
	msg.bodyptr = wm_status_change_ntf;

	switch (status_change_code) {
	case eSIR_SME_AP_CAPS_CHANGED:
		max_info_len = sizeof(tSirSmeApNewCaps);
		break;
	case eSIR_SME_JOINED_NEW_BSS:
		max_info_len = sizeof(tSirSmeNewBssInfo);
		break;
	default:
		max_info_len = sizeof(wm_status_change_ntf->statusChangeInfo);
		break;
	}

	switch (status_change_code) {
	case eSIR_SME_RADAR_DETECTED:
		break;
	default:
		wm_status_change_ntf->messageType =
			eWNI_SME_WM_STATUS_CHANGE_NTF;
		wm_status_change_ntf->statusChangeCode = status_change_code;
		wm_status_change_ntf->length = sizeof(tSirSmeWmStatusChangeNtf);
		wm_status_change_ntf->sessionId = session_id;
		if (info_len <= max_info_len && status_change_info) {
			qdf_mem_copy(
			    (uint8_t *) &wm_status_change_ntf->statusChangeInfo,
			    (uint8_t *) status_change_info, info_len);
		}
		pe_debug("StatusChg code: 0x%x length: %d",
			status_change_code, info_len);
		break;
	}

	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TX_SME_MSG, session_id, msg.type));
	if (eSIR_SUCCESS != lim_sys_process_mmh_msg_api(mac_ctx, &msg, ePROT)) {
		qdf_mem_free(wm_status_change_ntf);
		pe_err("lim_sys_process_mmh_msg_api failed");
	}

} /*** end lim_send_sme_wm_status_change_ntf() ***/

/**
 * lim_send_sme_set_context_rsp()
 *
 ***FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_SETCONTEXT_RSP message to host
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param pMac         Pointer to Global MAC structure
 * @param peerMacAddr  Indicates the peer MAC addr to which
 *                     setContext was performed
 * @param aid          Indicates the aid corresponding to the peer MAC
 *                     address
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_SETCONTEXT_RSP message
 *
 * @return None
 */
void
lim_send_sme_set_context_rsp(tpAniSirGlobal pMac,
			     struct qdf_mac_addr peer_macaddr, uint16_t aid,
			     tSirResultCodes resultCode,
			     tpPESession psessionEntry, uint8_t smesessionId,
			     uint16_t smetransactionId)
{
	struct scheduler_msg mmhMsg = {0};
	tSirSmeSetContextRsp *pSirSmeSetContextRsp;

	pSirSmeSetContextRsp = qdf_mem_malloc(sizeof(tSirSmeSetContextRsp));
	if (NULL == pSirSmeSetContextRsp) {
		/* Log error */
		pe_err("call to AllocateMemory failed for SmeSetContextRsp");
		return;
	}

	pSirSmeSetContextRsp->messageType = eWNI_SME_SETCONTEXT_RSP;
	pSirSmeSetContextRsp->length = sizeof(tSirSmeSetContextRsp);
	pSirSmeSetContextRsp->statusCode = resultCode;

	qdf_copy_macaddr(&pSirSmeSetContextRsp->peer_macaddr, &peer_macaddr);

	/* Update SME session and transaction Id */
	pSirSmeSetContextRsp->sessionId = smesessionId;
	pSirSmeSetContextRsp->transactionId = smetransactionId;

	mmhMsg.type = eWNI_SME_SETCONTEXT_RSP;
	mmhMsg.bodyptr = pSirSmeSetContextRsp;
	mmhMsg.bodyval = 0;
	if (NULL == psessionEntry) {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 psessionEntry->peSessionId, mmhMsg.type));
	}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_SETCONTEXT_RSP_EVENT,
			      psessionEntry, (uint16_t) resultCode, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
} /*** end lim_send_sme_set_context_rsp() ***/

/**
 * lim_send_sme_neighbor_bss_ind()
 *
 ***FUNCTION:
 * This function is called by lim_lookup_nadd_hash_entry() to send
 * eWNI_SME_NEIGHBOR_BSS_IND message to host
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * This function is used for sending eWNI_SME_NEIGHBOR_BSS_IND to
 * host upon detecting new BSS during background scanning if CFG
 * option is enabled for sending such indication
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
lim_send_sme_neighbor_bss_ind(tpAniSirGlobal pMac, tLimScanResultNode *pBssDescr)
{
	struct scheduler_msg msgQ = {0};
	uint32_t val;
	tSirSmeNeighborBssInd *pNewBssInd;

	if ((pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_WT_SCAN_STATE) ||
	    ((pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_WT_SCAN_STATE) &&
	     pMac->lim.gLimRspReqd)) {
		/* LIM is not in background scan state OR */
		/* current scan is initiated by HDD. */
		/* No need to send new BSS indication to HDD */
		return;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_NEW_BSS_FOUND_IND, &val) !=
	    eSIR_SUCCESS) {
		pe_err("could not get NEIGHBOR_BSS_IND from CFG");
		return;
	}

	if (val == 0)
		return;

	/**
	 * Need to indicate new BSSs found during
	 * background scanning to host.
	 * Allocate buffer for sending indication.
	 * Length of buffer is length of BSS description
	 * and length of header itself
	 */
	val = pBssDescr->bssDescription.length + sizeof(uint16_t) +
		sizeof(uint32_t) + sizeof(uint8_t);
	pNewBssInd = qdf_mem_malloc(val);
	if (NULL == pNewBssInd) {
		/* Log error */
		pe_err("call to AllocateMemory failed for eWNI_SME_NEIGHBOR_BSS_IND");
		return;
	}

	pNewBssInd->messageType = eWNI_SME_NEIGHBOR_BSS_IND;
	pNewBssInd->length = (uint16_t) val;
	pNewBssInd->sessionId = 0;

	qdf_mem_copy((uint8_t *) pNewBssInd->bssDescription,
		     (uint8_t *) &pBssDescr->bssDescription,
		     pBssDescr->bssDescription.length + sizeof(uint16_t));

	msgQ.type = eWNI_SME_NEIGHBOR_BSS_IND;
	msgQ.bodyptr = pNewBssInd;
	msgQ.bodyval = 0;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG, NO_SESSION, msgQ.type));
	lim_sys_process_mmh_msg_api(pMac, &msgQ, ePROT);
} /*** end lim_send_sme_neighbor_bss_ind() ***/

/** -----------------------------------------------------------------
   \brief lim_send_sme_addts_rsp() - sends SME ADDTS RSP
 \      This function sends a eWNI_SME_ADDTS_RSP to SME.
 \      SME only looks at rc and tspec field.
   \param pMac - global mac structure
   \param rspReqd - is SmeAddTsRsp required
   \param status - status code of SME_ADD_TS_RSP
   \return tspec
   \sa
   ----------------------------------------------------------------- */
void
lim_send_sme_addts_rsp(tpAniSirGlobal pMac, uint8_t rspReqd, uint32_t status,
		       tpPESession psessionEntry, tSirMacTspecIE tspec,
		       uint8_t smesessionId, uint16_t smetransactionId)
{
	tpSirAddtsRsp rsp;
	struct scheduler_msg mmhMsg = {0};

	if (!rspReqd)
		return;

	rsp = qdf_mem_malloc(sizeof(tSirAddtsRsp));
	if (NULL == rsp) {
		pe_err("AllocateMemory failed for ADDTS_RSP");
		return;
	}

	rsp->messageType = eWNI_SME_ADDTS_RSP;
	rsp->rc = status;
	rsp->rsp.status = (enum eSirMacStatusCodes)status;
	rsp->rsp.tspec = tspec;
	/* Update SME session Id and transcation Id */
	rsp->sessionId = smesessionId;
	rsp->transactionId = smetransactionId;

	mmhMsg.type = eWNI_SME_ADDTS_RSP;
	mmhMsg.bodyptr = rsp;
	mmhMsg.bodyval = 0;
	if (NULL == psessionEntry) {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 psessionEntry->peSessionId, mmhMsg.type));
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_ADDTS_RSP_EVENT, psessionEntry, 0,
			      0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
}

void
lim_send_sme_delts_rsp(tpAniSirGlobal pMac, tpSirDeltsReq delts, uint32_t status,
		       tpPESession psessionEntry, uint8_t smesessionId,
		       uint16_t smetransactionId)
{
	tpSirDeltsRsp rsp;
	struct scheduler_msg mmhMsg = {0};

	pe_debug("SendSmeDeltsRsp aid: %d tsid: %d up: %d status: %d",
		delts->aid,
		delts->req.tsinfo.traffic.tsid,
		delts->req.tsinfo.traffic.userPrio, status);
	if (!delts->rspReqd)
		return;

	rsp = qdf_mem_malloc(sizeof(tSirDeltsRsp));
	if (NULL == rsp) {
		/* Log error */
		pe_err("AllocateMemory failed for DELTS_RSP");
		return;
	}

	if (psessionEntry != NULL) {

		rsp->aid = delts->aid;
		qdf_copy_macaddr(&rsp->macaddr, &delts->macaddr);
		qdf_mem_copy((uint8_t *) &rsp->rsp, (uint8_t *) &delts->req,
			     sizeof(tSirDeltsReqInfo));
	}

	rsp->messageType = eWNI_SME_DELTS_RSP;
	rsp->rc = status;

	/* Update SME session Id and transcation Id */
	rsp->sessionId = smesessionId;
	rsp->transactionId = smetransactionId;

	mmhMsg.type = eWNI_SME_DELTS_RSP;
	mmhMsg.bodyptr = rsp;
	mmhMsg.bodyval = 0;
	if (NULL == psessionEntry) {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				 psessionEntry->peSessionId, mmhMsg.type));
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DELTS_RSP_EVENT, psessionEntry,
			      (uint16_t) status, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
}

void
lim_send_sme_delts_ind(tpAniSirGlobal pMac, tpSirDeltsReqInfo delts, uint16_t aid,
		       tpPESession psessionEntry)
{
	tpSirDeltsRsp rsp;
	struct scheduler_msg mmhMsg = {0};

	pe_debug("SendSmeDeltsInd aid: %d tsid: %d up: %d",
		aid, delts->tsinfo.traffic.tsid, delts->tsinfo.traffic.userPrio);

	rsp = qdf_mem_malloc(sizeof(tSirDeltsRsp));
	if (NULL == rsp) {
		/* Log error */
		pe_err("AllocateMemory failed for DELTS_IND");
		return;
	}

	rsp->messageType = eWNI_SME_DELTS_IND;
	rsp->rc = eSIR_SUCCESS;
	rsp->aid = aid;
	qdf_mem_copy((uint8_t *) &rsp->rsp, (uint8_t *) delts, sizeof(*delts));

	/* Update SME  session Id and SME transaction Id */

	rsp->sessionId = psessionEntry->smeSessionId;
	rsp->transactionId = psessionEntry->transactionId;

	mmhMsg.type = eWNI_SME_DELTS_IND;
	mmhMsg.bodyptr = rsp;
	mmhMsg.bodyval = 0;
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DELTS_IND_EVENT, psessionEntry, 0,
			      0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
}

/**
 * lim_send_sme_pe_statistics_rsp()
 *
 ***FUNCTION:
 * This function is called to send 802.11 statistics response to HDD.
 * This function posts the result back to HDD. This is a response to
 * HDD's request for statistics.
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
 * @param pMac         Pointer to Global MAC structure
 * @param p80211Stats  Statistics sent in response
 * @param resultCode   TODO:
 *
 *
 * @return none
 */

void
lim_send_sme_pe_statistics_rsp(tpAniSirGlobal pMac, uint16_t msgType, void *stats)
{
	struct scheduler_msg mmhMsg = {0};
	uint8_t sessionId;
	tAniGetPEStatsRsp *pPeStats = (tAniGetPEStatsRsp *) stats;
	tpPESession pPeSessionEntry;

	/* Get the Session Id based on Sta Id */
	pPeSessionEntry =
		pe_find_session_by_sta_id(pMac, pPeStats->staId, &sessionId);

	/* Fill the Session Id */
	if (NULL != pPeSessionEntry) {
		/* Fill the Session Id */
		pPeStats->sessionId = pPeSessionEntry->smeSessionId;
	}

	pPeStats->msgType = eWNI_SME_GET_STATISTICS_RSP;

	/* msgType should be WMA_GET_STATISTICS_RSP */
	mmhMsg.type = eWNI_SME_GET_STATISTICS_RSP;

	mmhMsg.bodyptr = stats;
	mmhMsg.bodyval = 0;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG, NO_SESSION, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;

} /*** end lim_send_sme_pe_statistics_rsp() ***/

#ifdef FEATURE_WLAN_ESE
/**
 * lim_send_sme_pe_ese_tsm_rsp() - send tsm response
 * @pMac:   Pointer to global pMac structure
 * @pStats: Pointer to TSM Stats
 *
 * This function is called to send tsm stats response to HDD.
 * This function posts the result back to HDD. This is a response to
 * HDD's request to get tsm stats.
 *
 * Return: None
 */
void lim_send_sme_pe_ese_tsm_rsp(tpAniSirGlobal pMac,
				 tAniGetTsmStatsRsp *pStats)
{
	struct scheduler_msg mmhMsg = {0};
	uint8_t sessionId;
	tAniGetTsmStatsRsp *pPeStats = (tAniGetTsmStatsRsp *) pStats;
	tpPESession pPeSessionEntry = NULL;

	/* Get the Session Id based on Sta Id */
	pPeSessionEntry =
		pe_find_session_by_sta_id(pMac, pPeStats->staId, &sessionId);

	/* Fill the Session Id */
	if (NULL != pPeSessionEntry) {
		/* Fill the Session Id */
		pPeStats->sessionId = pPeSessionEntry->smeSessionId;
	} else {
		pe_err("Session not found for the Sta id: %d",
		       pPeStats->staId);
		qdf_mem_free(pPeStats->tsmStatsReq);
		qdf_mem_free(pPeStats);
		return;
	}

	pPeStats->msgType = eWNI_SME_GET_TSM_STATS_RSP;
	pPeStats->tsmMetrics.RoamingCount
		= pPeSessionEntry->eseContext.tsm.tsmMetrics.RoamingCount;
	pPeStats->tsmMetrics.RoamingDly
		= pPeSessionEntry->eseContext.tsm.tsmMetrics.RoamingDly;

	mmhMsg.type = eWNI_SME_GET_TSM_STATS_RSP;
	mmhMsg.bodyptr = pStats;
	mmhMsg.bodyval = 0;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG, sessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
} /*** end lim_send_sme_pe_ese_tsm_rsp() ***/

#endif /* FEATURE_WLAN_ESE */

void
lim_send_sme_ibss_peer_ind(tpAniSirGlobal pMac,
			   tSirMacAddr peerMacAddr,
			   uint16_t staIndex,
			   uint8_t ucastIdx,
			   uint8_t bcastIdx,
			   uint8_t *beacon,
			   uint16_t beaconLen, uint16_t msgType, uint8_t sessionId)
{
	struct scheduler_msg mmhMsg = {0};
	tSmeIbssPeerInd *pNewPeerInd;

	pNewPeerInd = qdf_mem_malloc(sizeof(tSmeIbssPeerInd) + beaconLen);
	if (NULL == pNewPeerInd) {
		pe_err("Failed to allocate memory");
		return;
	}

	qdf_mem_copy((uint8_t *) pNewPeerInd->peer_addr.bytes,
		     peerMacAddr, QDF_MAC_ADDR_SIZE);
	pNewPeerInd->staId = staIndex;
	pNewPeerInd->ucastSig = ucastIdx;
	pNewPeerInd->bcastSig = bcastIdx;
	pNewPeerInd->mesgLen = sizeof(tSmeIbssPeerInd) + beaconLen;
	pNewPeerInd->mesgType = msgType;
	pNewPeerInd->sessionId = sessionId;

	if (beacon != NULL) {
		qdf_mem_copy((void *)((uint8_t *) pNewPeerInd +
				      sizeof(tSmeIbssPeerInd)), (void *)beacon,
			     beaconLen);
	}

	mmhMsg.type = msgType;
	mmhMsg.bodyptr = pNewPeerInd;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG, sessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

}

/**
 * lim_process_csa_wbw_ie() - Process CSA Wide BW IE
 * @mac_ctx:         pointer to global adapter context
 * @csa_params:      pointer to CSA parameters
 * @chnl_switch_info:pointer to channel switch parameters
 * @session_entry:   session pointer
 *
 * Return: None
 */
static QDF_STATUS lim_process_csa_wbw_ie(tpAniSirGlobal mac_ctx,
		struct csa_offload_params *csa_params,
		tLimWiderBWChannelSwitchInfo *chnl_switch_info,
		tpPESession session_entry)
{
	struct ch_params ch_params = {0};
	uint8_t ap_new_ch_width;
	bool new_ch_width_dfn = false;
	uint8_t center_freq_diff;
	uint32_t fw_vht_ch_wd = wma_get_vht_ch_width() + 1;

	ap_new_ch_width = csa_params->new_ch_width + 1;

	pe_info("new channel: %d new_ch_width: %d seg0: %d seg1: %d",
			csa_params->channel, ap_new_ch_width,
			csa_params->new_ch_freq_seg1,
			csa_params->new_ch_freq_seg2);

	if ((ap_new_ch_width != CH_WIDTH_80MHZ) &&
			(ap_new_ch_width != CH_WIDTH_160MHZ) &&
			(ap_new_ch_width != CH_WIDTH_80P80MHZ)) {
		pe_err("CSA wide BW IE has wrong ch_width %d",
				csa_params->new_ch_width);
		return QDF_STATUS_E_INVAL;
	}

	if (!csa_params->new_ch_freq_seg1 && !csa_params->new_ch_freq_seg2) {
		pe_err("CSA wide BW IE has invalid center freq");
		return QDF_STATUS_E_INVAL;
	}
	if ((ap_new_ch_width == CH_WIDTH_80MHZ) &&
			csa_params->new_ch_freq_seg2) {
		new_ch_width_dfn = true;
		if (csa_params->new_ch_freq_seg2 >
				csa_params->new_ch_freq_seg1)
			center_freq_diff = csa_params->new_ch_freq_seg2 -
				csa_params->new_ch_freq_seg1;
		else
			center_freq_diff = csa_params->new_ch_freq_seg1 -
				csa_params->new_ch_freq_seg2;
		if (center_freq_diff == CENTER_FREQ_DIFF_160MHz)
			ap_new_ch_width = CH_WIDTH_160MHZ;
		else if (center_freq_diff > CENTER_FREQ_DIFF_80P80MHz)
			ap_new_ch_width = CH_WIDTH_80P80MHZ;
		else
			ap_new_ch_width = CH_WIDTH_80MHZ;
	}
	session_entry->gLimChannelSwitch.state =
		eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
	if ((ap_new_ch_width == CH_WIDTH_160MHZ) &&
			!new_ch_width_dfn) {
		if (csa_params->new_ch_freq_seg1 != csa_params->channel +
				CH_TO_CNTR_FREQ_DIFF_160MHz) {
			pe_err("CSA wide BW IE has invalid center freq");
			return QDF_STATUS_E_INVAL;
		}

		if (ap_new_ch_width > fw_vht_ch_wd) {
			pe_info("New BW is not supported, setting BW to %d",
					fw_vht_ch_wd);
			ap_new_ch_width = fw_vht_ch_wd;
		}
		ch_params.ch_width = ap_new_ch_width ;
		wlan_reg_set_channel_params(mac_ctx->pdev,
					    csa_params->channel, 0, &ch_params);
		ap_new_ch_width = ch_params.ch_width;
		csa_params->new_ch_freq_seg1 = ch_params.center_freq_seg0;
		csa_params->new_ch_freq_seg2 = ch_params.center_freq_seg1;
	} else if (!new_ch_width_dfn) {
		if (ap_new_ch_width > fw_vht_ch_wd) {
			pe_info("New BW is not supported, setting BW to %d",
					fw_vht_ch_wd);
			ap_new_ch_width = fw_vht_ch_wd;
		}
		if (csa_params->new_ch_freq_seg1 != csa_params->channel +
				CH_TO_CNTR_FREQ_DIFF_80MHz) {
			pe_err("CSA wide BW IE has invalid center freq");
			return QDF_STATUS_E_INVAL;
		}
		csa_params->new_ch_freq_seg2 = 0;
	}
	if (new_ch_width_dfn) {
		if (csa_params->new_ch_freq_seg1 != csa_params->channel +
				CH_TO_CNTR_FREQ_DIFF_80MHz) {
			pe_err("CSA wide BW IE has invalid center freq");
			return QDF_STATUS_E_INVAL;
		}
		if (ap_new_ch_width > fw_vht_ch_wd) {
			pe_info("New width is not supported, setting BW to %d",
					fw_vht_ch_wd);
			ap_new_ch_width = fw_vht_ch_wd;
		}
		if ((ap_new_ch_width == CH_WIDTH_160MHZ) &&
				(csa_params->new_ch_freq_seg1 !=
				 csa_params->channel +
				 CH_TO_CNTR_FREQ_DIFF_160MHz)) {
			pe_err("wide BW IE has invalid 160M center freq");
			csa_params->new_ch_freq_seg2 = 0;
			ap_new_ch_width = CH_WIDTH_80MHZ;
		}
	}
	chnl_switch_info->newChanWidth = ap_new_ch_width;
	chnl_switch_info->newCenterChanFreq0 = csa_params->new_ch_freq_seg1;
	chnl_switch_info->newCenterChanFreq1 = csa_params->new_ch_freq_seg2;

	if (session_entry->ch_width == ap_new_ch_width)
		goto prnt_log;

	if (session_entry->ch_width == CH_WIDTH_80MHZ) {
		chnl_switch_info->newChanWidth = CH_WIDTH_80MHZ;
		chnl_switch_info->newCenterChanFreq1 = 0;
	} else {
		session_entry->ch_width = ap_new_ch_width;
		chnl_switch_info->newChanWidth = ap_new_ch_width;
	}
prnt_log:
	pe_debug("new channel: %d new_ch_width: %d seg0: %d seg1: %d",
			csa_params->channel,
			chnl_switch_info->newChanWidth,
			chnl_switch_info->newCenterChanFreq0,
			chnl_switch_info->newCenterChanFreq1);

	return QDF_STATUS_SUCCESS;
}

/**
 * lim_handle_csa_offload_msg() - Handle CSA offload message
 * @mac_ctx:         pointer to global adapter context
 * @msg:             Message pointer.
 *
 * Return: None
 */
void lim_handle_csa_offload_msg(tpAniSirGlobal mac_ctx,
				struct scheduler_msg *msg)
{
	tpPESession session_entry;
	struct scheduler_msg mmh_msg = {0};
	struct csa_offload_params *csa_params =
				(struct csa_offload_params *) (msg->bodyptr);
	tpSmeCsaOffloadInd csa_offload_ind;
	tpDphHashNode sta_ds = NULL;
	uint8_t session_id;
	uint16_t aid = 0;
	uint16_t chan_space = 0;
	struct ch_params ch_params;

	tLimWiderBWChannelSwitchInfo *chnl_switch_info = NULL;
	tLimChannelSwitchInfo *lim_ch_switch = NULL;

	pe_debug("handle csa offload msg");

	if (!csa_params) {
		pe_err("limMsgQ body ptr is NULL");
		return;
	}

	session_entry =
		pe_find_session_by_bssid(mac_ctx,
			csa_params->bssId, &session_id);
	if (!session_entry) {
		pe_err("Session does not exists for %pM",
				csa_params->bssId);
		goto err;
	}

	sta_ds = dph_lookup_hash_entry(mac_ctx, session_entry->bssId, &aid,
		&session_entry->dph.dphHashTable);

	if (!sta_ds) {
		pe_err("sta_ds does not exist");
		goto err;
	}

	if (!LIM_IS_STA_ROLE(session_entry)) {
		pe_debug("Invalid role to handle CSA");
		goto err;
	}

	/*
	 * on receiving channel switch announcement from AP, delete all
	 * TDLS peers before leaving BSS and proceed for channel switch
	 */
	lim_delete_tdls_peers(mac_ctx, session_entry);

	lim_ch_switch = &session_entry->gLimChannelSwitch;
	session_entry->gLimChannelSwitch.switchMode =
		csa_params->switch_mode;
	/* timer already started by firmware, switch immediately */
	session_entry->gLimChannelSwitch.switchCount = 0;
	session_entry->gLimChannelSwitch.primaryChannel =
		csa_params->channel;
	session_entry->gLimChannelSwitch.state =
		eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
	session_entry->gLimChannelSwitch.ch_width = CH_WIDTH_20MHZ;
	lim_ch_switch->sec_ch_offset =
		session_entry->htSecondaryChannelOffset;
	session_entry->gLimChannelSwitch.ch_center_freq_seg0 = 0;
	session_entry->gLimChannelSwitch.ch_center_freq_seg1 = 0;
	chnl_switch_info =
		&session_entry->gLimWiderBWChannelSwitch;

	pe_info("vht: %d ht: %d flag: %x chan: %d, sec_ch_offset %d",
			session_entry->vhtCapability,
			session_entry->htSupportedChannelWidthSet,
			csa_params->ies_present_flag,
			csa_params->channel,
			csa_params->sec_chan_offset);
	pe_info("seg1: %d seg2: %d width: %d country: %s class: %d",
			csa_params->new_ch_freq_seg1,
			csa_params->new_ch_freq_seg2,
			csa_params->new_ch_width,
			mac_ctx->scan.countryCodeCurrent,
			csa_params->new_op_class);

	if (session_entry->vhtCapability &&
			session_entry->htSupportedChannelWidthSet) {
		if ((csa_params->ies_present_flag & lim_wbw_ie_present) &&
			(QDF_STATUS_SUCCESS == lim_process_csa_wbw_ie(mac_ctx,
					csa_params, chnl_switch_info,
					session_entry))) {
			pe_debug("CSA wide BW IE process successful");
			lim_ch_switch->sec_ch_offset =
				PHY_SINGLE_CHANNEL_CENTERED;
			if (chnl_switch_info->newChanWidth) {
				if (csa_params->channel <
				  csa_params->new_ch_freq_seg1)
					lim_ch_switch->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
				else
					lim_ch_switch->sec_ch_offset =
						PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
			}
		} else if (csa_params->ies_present_flag
				& lim_xcsa_ie_present) {
			chan_space =
				wlan_reg_dmn_get_chanwidth_from_opclass(
						mac_ctx->scan.countryCodeCurrent,
						csa_params->channel,
						csa_params->new_op_class);
			session_entry->gLimChannelSwitch.state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;

			if (chan_space == 80) {
				chnl_switch_info->newChanWidth =
					CH_WIDTH_80MHZ;
			} else if (chan_space == 40) {
				chnl_switch_info->newChanWidth =
					CH_WIDTH_40MHZ;
			} else {
				chnl_switch_info->newChanWidth =
					CH_WIDTH_20MHZ;
				lim_ch_switch->state =
					eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
			}

			ch_params.ch_width =
				chnl_switch_info->newChanWidth;
			wlan_reg_set_channel_params(mac_ctx->pdev,
					csa_params->channel, 0, &ch_params);
			chnl_switch_info->newCenterChanFreq0 =
				ch_params.center_freq_seg0;
			/*
			 * This is not applicable for 20/40/80 MHz.
			 * Only used when we support 80+80 MHz operation.
			 * In case of 80+80 MHz, this parameter indicates
			 * center channel frequency index of 80 MHz
			 * channel offrequency segment 1.
			 */
			chnl_switch_info->newCenterChanFreq1 =
				ch_params.center_freq_seg1;
			lim_ch_switch->sec_ch_offset =
				ch_params.sec_ch_offset;

		} else {
			lim_ch_switch->state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			ch_params.ch_width = CH_WIDTH_40MHZ;
			wlan_reg_set_channel_params(mac_ctx->pdev,
					csa_params->channel, 0, &ch_params);
			lim_ch_switch->sec_ch_offset =
				ch_params.sec_ch_offset;
			chnl_switch_info->newChanWidth = CH_WIDTH_40MHZ;
			chnl_switch_info->newCenterChanFreq0 =
				ch_params.center_freq_seg0;
			chnl_switch_info->newCenterChanFreq1 = 0;
		}
		session_entry->gLimChannelSwitch.ch_center_freq_seg0 =
			chnl_switch_info->newCenterChanFreq0;
		session_entry->gLimChannelSwitch.ch_center_freq_seg1 =
			chnl_switch_info->newCenterChanFreq1;
		session_entry->gLimChannelSwitch.ch_width =
			chnl_switch_info->newChanWidth;

	} else if (session_entry->htSupportedChannelWidthSet) {
		if (csa_params->ies_present_flag
				& lim_xcsa_ie_present) {
			chan_space =
				wlan_reg_dmn_get_chanwidth_from_opclass(
						mac_ctx->scan.countryCodeCurrent,
						csa_params->channel,
						csa_params->new_op_class);
			lim_ch_switch->state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			if (chan_space == 40) {
				lim_ch_switch->ch_width =
					CH_WIDTH_40MHZ;
				chnl_switch_info->newChanWidth =
					CH_WIDTH_40MHZ;
				ch_params.ch_width =
					chnl_switch_info->newChanWidth;
				wlan_reg_set_channel_params(mac_ctx->pdev,
						csa_params->channel,
						0, &ch_params);
				lim_ch_switch->ch_center_freq_seg0 =
					ch_params.center_freq_seg0;
				lim_ch_switch->sec_ch_offset =
					ch_params.sec_ch_offset;
			} else {
				lim_ch_switch->ch_width =
					CH_WIDTH_20MHZ;
				chnl_switch_info->newChanWidth =
					CH_WIDTH_40MHZ;
				lim_ch_switch->state =
					eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
				lim_ch_switch->sec_ch_offset =
					PHY_SINGLE_CHANNEL_CENTERED;
			}
		} else {
			lim_ch_switch->ch_width =
				CH_WIDTH_40MHZ;
			lim_ch_switch->state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			ch_params.ch_width = CH_WIDTH_40MHZ;
			wlan_reg_set_channel_params(mac_ctx->pdev,
					csa_params->channel, 0, &ch_params);
			lim_ch_switch->ch_center_freq_seg0 =
				ch_params.center_freq_seg0;
			lim_ch_switch->sec_ch_offset =
				ch_params.sec_ch_offset;
		}

	}
	pe_debug("new ch width: %d space: %d",
			session_entry->gLimChannelSwitch.ch_width, chan_space);
	if ((session_entry->currentOperChannel == csa_params->channel) &&
		(session_entry->ch_width ==
		 session_entry->gLimChannelSwitch.ch_width)) {
		pe_debug("Ignore CSA, no change in ch and bw");
		goto err;
	}

	lim_prepare_for11h_channel_switch(mac_ctx, session_entry);
	csa_offload_ind = qdf_mem_malloc(sizeof(tSmeCsaOffloadInd));
	if (NULL == csa_offload_ind) {
		pe_err("memalloc fail eWNI_SME_CSA_OFFLOAD_EVENT");
		goto err;
	}

	csa_offload_ind->mesgType = eWNI_SME_CSA_OFFLOAD_EVENT;
	csa_offload_ind->mesgLen = sizeof(tSmeCsaOffloadInd);
	qdf_mem_copy(csa_offload_ind->bssid.bytes, session_entry->bssId,
			QDF_MAC_ADDR_SIZE);
	mmh_msg.type = eWNI_SME_CSA_OFFLOAD_EVENT;
	mmh_msg.bodyptr = csa_offload_ind;
	mmh_msg.bodyval = 0;
	pe_debug("Sending eWNI_SME_CSA_OFFLOAD_EVENT to SME");
	MTRACE(mac_trace_msg_tx
			(mac_ctx, session_entry->peSessionId, mmh_msg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac_ctx,
			WLAN_PE_DIAG_SWITCH_CHL_IND_EVENT, session_entry,
			eSIR_SUCCESS, eSIR_SUCCESS);
#endif
	lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);

err:
	qdf_mem_free(csa_params);
}

/*--------------------------------------------------------------------------
   \brief pe_delete_session() - Handle the Delete BSS Response from HAL.

   \param pMac                   - pointer to global adapter context
   \param sessionId             - Message pointer.

   \sa
   --------------------------------------------------------------------------*/

void lim_handle_delete_bss_rsp(tpAniSirGlobal pMac, struct scheduler_msg *MsgQ)
{
	tpPESession psessionEntry;
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) (MsgQ->bodyptr);

	psessionEntry =
		pe_find_session_by_session_id(pMac, pDelBss->sessionId);
	if (psessionEntry == NULL) {
		pe_err("Session Does not exist for given sessionID: %d",
			pDelBss->sessionId);
		qdf_mem_free(MsgQ->bodyptr);
		return;
	}

	/*
	 * During DEL BSS handling, the PE Session will be deleted, but it is
	 * better to clear this flag if the session is hanging around due
	 * to some error conditions so that the next DEL_BSS request does
	 * not take the HO_FAIL path
	 */
	psessionEntry->process_ho_fail = false;
	if (LIM_IS_IBSS_ROLE(psessionEntry))
		lim_ibss_del_bss_rsp(pMac, MsgQ->bodyptr, psessionEntry);
	else if (LIM_IS_UNKNOWN_ROLE(psessionEntry))
		lim_process_sme_del_bss_rsp(pMac, MsgQ->bodyval, psessionEntry);
	else if (LIM_IS_NDI_ROLE(psessionEntry))
		lim_ndi_del_bss_rsp(pMac, MsgQ->bodyptr, psessionEntry);
	else
		lim_process_mlm_del_bss_rsp(pMac, MsgQ, psessionEntry);

}

/** -----------------------------------------------------------------
   \brief lim_send_sme_aggr_qos_rsp() - sends SME FT AGGR QOS RSP
 \      This function sends a eWNI_SME_FT_AGGR_QOS_RSP to SME.
 \      SME only looks at rc and tspec field.
   \param pMac - global mac structure
   \param rspReqd - is SmeAddTsRsp required
   \param status - status code of eWNI_SME_FT_AGGR_QOS_RSP
   \return tspec
   \sa
   ----------------------------------------------------------------- */
void
lim_send_sme_aggr_qos_rsp(tpAniSirGlobal pMac, tpSirAggrQosRsp aggrQosRsp,
			  uint8_t smesessionId)
{
	struct scheduler_msg mmhMsg = {0};

	mmhMsg.type = eWNI_SME_FT_AGGR_QOS_RSP;
	mmhMsg.bodyptr = aggrQosRsp;
	mmhMsg.bodyval = 0;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
			 smesessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
}

void lim_send_sme_max_assoc_exceeded_ntf(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
					 uint8_t smesessionId)
{
	struct scheduler_msg mmhMsg = {0};
	tSmeMaxAssocInd *pSmeMaxAssocInd;

	pSmeMaxAssocInd = qdf_mem_malloc(sizeof(tSmeMaxAssocInd));
	if (NULL == pSmeMaxAssocInd) {
		pe_err("Failed to allocate memory");
		return;
	}
	qdf_mem_copy((uint8_t *) pSmeMaxAssocInd->peer_mac.bytes,
		     (uint8_t *) peerMacAddr, QDF_MAC_ADDR_SIZE);
	pSmeMaxAssocInd->mesgType = eWNI_SME_MAX_ASSOC_EXCEEDED;
	pSmeMaxAssocInd->mesgLen = sizeof(tSmeMaxAssocInd);
	pSmeMaxAssocInd->sessionId = smesessionId;
	mmhMsg.type = pSmeMaxAssocInd->mesgType;
	mmhMsg.bodyptr = pSmeMaxAssocInd;
	pe_debug("msgType: %s peerMacAddr "MAC_ADDRESS_STR "sme session id %d",
		"eWNI_SME_MAX_ASSOC_EXCEEDED", MAC_ADDR_ARRAY(peerMacAddr),
		pSmeMaxAssocInd->sessionId);
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
			 smesessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
}

/*--------------------------------------------------------------------------
   \brief lim_send_dfs_chan_sw_ie_update()
   This timer handler updates the channel switch IE in beacon template

   \param pMac - pointer to global adapter context
   \return     - channel to scan from valid session else zero.
   \sa
   --------------------------------------------------------------------------*/
static void
lim_send_dfs_chan_sw_ie_update(tpAniSirGlobal pMac, tpPESession psessionEntry)
{

	/* Update the beacon template and send to FW */
	if (sch_set_fixed_beacon_fields(pMac, psessionEntry) != eSIR_SUCCESS) {
		pe_err("Unable to set CSA IE in beacon");
		return;
	}

	/* Send update beacon template message */
	lim_send_beacon_ind(pMac, psessionEntry);
	pe_debug("Updated CSA IE, IE COUNT: %d",
		       psessionEntry->gLimChannelSwitch.switchCount);

	return;
}

/** -----------------------------------------------------------------
   \brief lim_send_sme_ap_channel_switch_resp() - sends
   eWNI_SME_CHANNEL_CHANGE_RSP
   After receiving WMA_SWITCH_CHANNEL_RSP indication this
   function sends a eWNI_SME_CHANNEL_CHANGE_RSP to SME to notify
   that the Channel change has been done to the specified target
   channel in the Channel change request
   \param pMac - global mac structure
   \param psessionEntry - session info
   \param pChnlParams - Channel switch params
   --------------------------------------------------------------------*/
void
lim_send_sme_ap_channel_switch_resp(tpAniSirGlobal pMac,
				    tpPESession psessionEntry,
				    tpSwitchChannelParams pChnlParams)
{
	struct scheduler_msg mmhMsg = {0};
	tpSwitchChannelParams pSmeSwithChnlParams;
	uint8_t channelId;
	bool is_ch_dfs = false;
	enum phy_ch_width ch_width;
	uint8_t ch_center_freq_seg1;

	pSmeSwithChnlParams = (tSwitchChannelParams *)
			      qdf_mem_malloc(sizeof(tSwitchChannelParams));
	if (NULL == pSmeSwithChnlParams) {
		pe_err("AllocateMemory failed for pSmeSwithChnlParams");
		return;
	}

	qdf_mem_copy(pSmeSwithChnlParams, pChnlParams,
		     sizeof(tSwitchChannelParams));

	channelId = pSmeSwithChnlParams->channelNumber;
	ch_width = pSmeSwithChnlParams->ch_width;
	ch_center_freq_seg1 = pSmeSwithChnlParams->ch_center_freq_seg1;

	/*
	 * Pass the sme sessionID to SME instead
	 * PE session ID.
	 */
	pSmeSwithChnlParams->peSessionId = psessionEntry->smeSessionId;

	mmhMsg.type = eWNI_SME_CHANNEL_CHANGE_RSP;
	mmhMsg.bodyptr = (void *)pSmeSwithChnlParams;
	mmhMsg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	/*
	 * We should start beacon transmission only if the new
	 * channel after channel change is Non-DFS. For a DFS
	 * channel, PE will receive an explicit request from
	 * upper layers to start the beacon transmission .
	 */

	if (ch_width == CH_WIDTH_160MHZ) {
		is_ch_dfs = true;
	} else if (ch_width == CH_WIDTH_80P80MHZ) {
		if (wlan_reg_get_channel_state(pMac->pdev, channelId) ==
				CHANNEL_STATE_DFS ||
		    wlan_reg_get_channel_state(pMac->pdev,
			    ch_center_freq_seg1 -
			    SIR_80MHZ_START_CENTER_CH_DIFF) ==
							CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	} else {
		if (wlan_reg_get_channel_state(pMac->pdev, channelId) ==
				CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	}

	if (!is_ch_dfs) {
		if (channelId == psessionEntry->currentOperChannel) {
			lim_apply_configuration(pMac, psessionEntry);
			lim_send_beacon_ind(pMac, psessionEntry);
		} else {
			pe_debug("Failed to Transmit Beacons on channel: %d after AP channel change response",
				       psessionEntry->bcnLen);
		}

		lim_obss_send_detection_cfg(pMac, psessionEntry, true);
	}
	return;
}

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
/**
 *  lim_send_bss_color_change_ie_update() - update bss color change IE in
 *   beacon template
 *
 *  @mac_ctx: pointer to global adapter context
 *  @session: session pointer
 *
 *  Return: none
 */
static void
lim_send_bss_color_change_ie_update(tpAniSirGlobal mac_ctx,
						tpPESession session)
{
	/* Update the beacon template and send to FW */
	if (sch_set_fixed_beacon_fields(mac_ctx, session) != eSIR_SUCCESS) {
		pe_err("Unable to set BSS color change IE in beacon");
	       return;
	}

	/* Send update beacon template message */
	lim_send_beacon_ind(mac_ctx, session);
	pe_info("Updated BSS color change countdown = %d",
		session->he_bss_color_change.countdown);
}

static void
lim_handle_bss_color_change_ie(tpAniSirGlobal mac_ctx,
					tpPESession session)
{
	tUpdateBeaconParams beacon_params;

	/* handle bss color change IE */
	if (LIM_IS_AP_ROLE(session) &&
			session->he_op.bss_col_disabled) {
		if (session->he_bss_color_change.countdown > 0) {
			session->he_bss_color_change.countdown--;
		} else {
			session->bss_color_changing = 0;
			qdf_mem_zero(&beacon_params, sizeof(beacon_params));
			if (session->he_bss_color_change.new_color != 0) {
				session->he_op.bss_col_disabled = 0;
				session->he_op.bss_color =
					session->he_bss_color_change.new_color;
				beacon_params.paramChangeBitmap |=
					PARAM_BSS_COLOR_CHANGED;
				beacon_params.bss_color_disabled = 0;
				beacon_params.bss_color =
					session->he_op.bss_color;
				lim_send_beacon_params(mac_ctx,
						       &beacon_params,
						       session);
				lim_send_obss_color_collision_cfg(mac_ctx,
						session,
						OBSS_COLOR_COLLISION_DETECTION);
			}
		}

		lim_send_bss_color_change_ie_update(mac_ctx, session);
	}
}

#else
static void
lim_handle_bss_color_change_ie(tpAniSirGlobal mac_ctx,
					tpPESession session)
{
}
#endif

/** -----------------------------------------------------------------
   \brief lim_process_beacon_tx_success_ind() - This function is used
   explicitely to handle successful beacon transmission indication
   from the FW. This is a generic event generated by the FW afer the
   first beacon is sent out after the beacon template update by the
   host
   \param pMac - global mac structure
   \param psessionEntry - session info
   \return none
   \sa
   ----------------------------------------------------------------- */
void
lim_process_beacon_tx_success_ind(tpAniSirGlobal pMac, uint16_t msgType, void *event)
{
	/* Currently, this event is used only for DFS channel switch announcement
	 * IE update in the template. If required to be used for other IE updates
	 * add appropriate code by introducing a state variable
	 */
	tpPESession psessionEntry;
	struct scheduler_msg mmhMsg = {0};
	tSirSmeCSAIeTxCompleteRsp *pChanSwTxResponse;
	struct sir_beacon_tx_complete_rsp *beacon_tx_comp_rsp_ptr;
	uint8_t length = sizeof(tSirSmeCSAIeTxCompleteRsp);
	tpSirFirstBeaconTxCompleteInd pBcnTxInd =
		(tSirFirstBeaconTxCompleteInd *) event;
	uint8_t ch, ch_width;

	psessionEntry = pe_find_session_by_bss_idx(pMac, pBcnTxInd->bssIdx);
	if (psessionEntry == NULL) {
		pe_err("Session Does not exist for given sessionID");
		return;
	}

	pe_debug("role: %d swIe: %d opIe: %d",
		GET_LIM_SYSTEM_ROLE(psessionEntry),
		psessionEntry->dfsIncludeChanSwIe,
		psessionEntry->gLimOperatingMode.present);

	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    true == psessionEntry->dfsIncludeChanSwIe) {

		if (psessionEntry->gLimChannelSwitch.switchCount) {
			/* Decrement the beacon switch count */
			psessionEntry->gLimChannelSwitch.switchCount--;
			pe_debug("current beacon count %d",
				psessionEntry->gLimChannelSwitch.switchCount);
		}

		/* Send only 5 beacons with CSA IE Set in when a radar is detected */
		if (psessionEntry->gLimChannelSwitch.switchCount > 0) {
			/*
			 * Send the next beacon with updated CSA IE count
			 */
			lim_send_dfs_chan_sw_ie_update(pMac, psessionEntry);

			ch = psessionEntry->gLimChannelSwitch.primaryChannel;
			ch_width = psessionEntry->gLimChannelSwitch.ch_width;
			if (pMac->sap.SapDfsInfo.dfs_beacon_tx_enhanced)
				/* Send Action frame after updating beacon */
				lim_send_chan_switch_action_frame(pMac,
					ch, ch_width, psessionEntry);

		} else {
			/* Done with CSA IE update, send response back to SME */
			psessionEntry->gLimChannelSwitch.switchCount = 0;
			if (pMac->sap.SapDfsInfo.disable_dfs_ch_switch == false)
				psessionEntry->gLimChannelSwitch.switchMode = 0;
			psessionEntry->dfsIncludeChanSwIe = false;
			psessionEntry->dfsIncludeChanWrapperIe = false;

			pChanSwTxResponse = (tSirSmeCSAIeTxCompleteRsp *)
					    qdf_mem_malloc(length);

			if (NULL == pChanSwTxResponse) {
				pe_err("AllocateMemory failed for tSirSmeCSAIeTxCompleteRsp");
				return;
			}

			pChanSwTxResponse->sessionId =
				psessionEntry->smeSessionId;
			pChanSwTxResponse->chanSwIeTxStatus =
				QDF_STATUS_SUCCESS;

			mmhMsg.type = eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND;
			mmhMsg.bodyptr = pChanSwTxResponse;
			mmhMsg.bodyval = 0;
			lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
		}
	}

	if (LIM_IS_AP_ROLE(psessionEntry) &&
		psessionEntry->gLimOperatingMode.present) {
		/* Done with nss update, send response back to SME */
		psessionEntry->gLimOperatingMode.present = 0;
		beacon_tx_comp_rsp_ptr = (struct sir_beacon_tx_complete_rsp *)
				qdf_mem_malloc(sizeof(*beacon_tx_comp_rsp_ptr));
		if (NULL == beacon_tx_comp_rsp_ptr) {
			pe_err("AllocateMemory failed for beacon_tx_comp_rsp_ptr");
			return;
		}
		beacon_tx_comp_rsp_ptr->session_id =
			psessionEntry->smeSessionId;
		beacon_tx_comp_rsp_ptr->tx_status = QDF_STATUS_SUCCESS;
		mmhMsg.type = eWNI_SME_NSS_UPDATE_RSP;
		mmhMsg.bodyptr = beacon_tx_comp_rsp_ptr;
		mmhMsg.bodyval = 0;
		lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	}

	lim_handle_bss_color_change_ie(pMac, psessionEntry);

	return;
}
