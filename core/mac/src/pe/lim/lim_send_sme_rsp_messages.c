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
 * This file lim_send_sme_rspMessages.cc contains the functions
 * for sending SME response/notification messages to applications
 * above MAC software.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "cdf_types.h"
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
	tSirMsgQ msg;
	tSirSmeRsp *sme_rsp;

	lim_log(mac_ctx, LOG1, FL("Sending message %s with reasonCode %s"),
		lim_msg_str(msg_type), lim_result_code_str(result_code));

	sme_rsp = cdf_mem_malloc(sizeof(tSirSmeRsp));
	if (NULL == sme_rsp) {
		/* Buffer not available. Log error */
		CDF_TRACE(CDF_MODULE_ID_PE, LOGP,
			FL("call to AllocateMemory failed for eWNI_SME_*_RSP"));
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
	MTRACE(mac_trace_msg_tx(mac_ctx, sme_session_id, msg.type));

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
	tSirMsgQ msg;
	struct sir_roc_rsp *sme_rsp;

	lim_log(mac_ctx, LOG1,
		FL("Sending message %s with reasonCode %s scanId %d"),
		lim_msg_str(msg_type), lim_result_code_str(result_code),
		scan_id);

	sme_rsp = cdf_mem_malloc(sizeof(struct sir_roc_rsp));
	if (NULL == sme_rsp) {
		CDF_TRACE(CDF_MODULE_ID_PE, LOGP,
			FL("call to AllocateMemory failed for eWNI_SME_*_RSP"));
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
	CDF_STATUS status, uint32_t *ctx)
{
	tSirMsgQ msg;
	tpSirSmeJoinRsp sme_join_rsp = (tpSirSmeJoinRsp) ctx;

	msg.type = sme_join_rsp->messageType;
	msg.bodyptr = sme_join_rsp;
	msg.bodyval = 0;
	MTRACE(mac_trace_msg_tx(mac_ctx, NO_SESSION, msg.type));
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
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	tSirSmeHTProfile *ht_profile;
#endif
	if (result_code == eSIR_SME_SUCCESS) {
		if (session_entry->beacon != NULL) {
			sme_join_rsp->beaconLength = session_entry->bcnLen;
			cdf_mem_copy(sme_join_rsp->frames,
				session_entry->beacon,
				sme_join_rsp->beaconLength);
			cdf_mem_free(session_entry->beacon);
			session_entry->beacon = NULL;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
			lim_log(mac_ctx, LOG1, FL("Beacon=%d"),
				session_entry->bcnLen);
#endif
		}
		if (session_entry->assocReq != NULL) {
			sme_join_rsp->assocReqLength =
				session_entry->assocReqLen;
			cdf_mem_copy(sme_join_rsp->frames +
				session_entry->bcnLen, session_entry->assocReq,
				sme_join_rsp->assocReqLength);
			cdf_mem_free(session_entry->assocReq);
			session_entry->assocReq = NULL;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
			lim_log(mac_ctx,
				LOG1, FL("AssocReq=%d"),
				session_entry->assocReqLen);
#endif
		}
		if (session_entry->assocRsp != NULL) {
			sme_join_rsp->assocRspLength =
				session_entry->assocRspLen;
			cdf_mem_copy(sme_join_rsp->frames +
				session_entry->bcnLen +
				session_entry->assocReqLen,
				session_entry->assocRsp,
				sme_join_rsp->assocRspLength);
			cdf_mem_free(session_entry->assocRsp);
			session_entry->assocRsp = NULL;
		}
#ifdef WLAN_FEATURE_VOWIFI_11R
		if (session_entry->ricData != NULL) {
			sme_join_rsp->parsedRicRspLen =
				session_entry->RICDataLen;
			cdf_mem_copy(sme_join_rsp->frames +
				session_entry->bcnLen +
				session_entry->assocReqLen +
				session_entry->assocRspLen,
				session_entry->ricData,
				sme_join_rsp->parsedRicRspLen);
			cdf_mem_free(session_entry->ricData);
			session_entry->ricData = NULL;
			lim_log(mac_ctx, LOG1, FL("RicLength=%d"),
				sme_join_rsp->parsedRicRspLen);
		}
#endif
#ifdef FEATURE_WLAN_ESE
		if (session_entry->tspecIes != NULL) {
			sme_join_rsp->tspecIeLen =
				session_entry->tspecLen;
			cdf_mem_copy(sme_join_rsp->frames +
				session_entry->bcnLen +
				session_entry->assocReqLen +
				session_entry->assocRspLen +
				session_entry->RICDataLen,
				session_entry->tspecIes,
				sme_join_rsp->tspecIeLen);
			cdf_mem_free(session_entry->tspecIes);
			session_entry->tspecIes = NULL;
			lim_log(mac_ctx, LOG1, FL("ESE-TspecLen=%d"),
				session_entry->tspecLen);
		}
#endif
		sme_join_rsp->aid = session_entry->limAID;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
		lim_log(mac_ctx, LOG1, FL("AssocRsp=%d"),
			session_entry->assocRspLen);
#endif
		sme_join_rsp->vht_channel_width =
			session_entry->ch_width;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (session_entry->cc_switch_mode !=
				CDF_MCC_TO_SCC_SWITCH_DISABLE) {
			ht_profile = &sme_join_rsp->HTProfile;
			ht_profile->htSupportedChannelWidthSet =
				session_entry->htSupportedChannelWidthSet;
			ht_profile->htRecommendedTxWidthSet =
				session_entry->htRecommendedTxWidthSet;
			ht_profile->htSecondaryChannelOffset =
				session_entry->htSecondaryChannelOffset;
			ht_profile->dot11mode = session_entry->dot11mode;
			ht_profile->htCapability = session_entry->htCapability;
#ifdef WLAN_FEATURE_11AC
			ht_profile->vhtCapability =
				session_entry->vhtCapability;
			ht_profile->vhtTxChannelWidthSet =
				session_entry->vhtTxChannelWidthSet;
			ht_profile->apCenterChan = session_entry->ch_center_freq_seg0;
			ht_profile->apChanWidth = session_entry->ch_width;
#endif
		}
#endif
	} else {
		if (session_entry->beacon != NULL) {
			cdf_mem_free(session_entry->beacon);
			session_entry->beacon = NULL;
		}
		if (session_entry->assocReq != NULL) {
			cdf_mem_free(session_entry->assocReq);
			session_entry->assocReq = NULL;
		}
		if (session_entry->assocRsp != NULL) {
			cdf_mem_free(session_entry->assocRsp);
			session_entry->assocRsp = NULL;
		}
#ifdef WLAN_FEATURE_VOWIFI_11R
		if (session_entry->ricData != NULL) {
			cdf_mem_free(session_entry->ricData);
			session_entry->ricData = NULL;
		}
#endif
#ifdef FEATURE_WLAN_ESE
		if (session_entry->tspecIes != NULL) {
			cdf_mem_free(session_entry->tspecIes);
			session_entry->tspecIes = NULL;
		}
#endif
	}
}
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

	lim_log(mac_ctx, LOG1, FL("Sending message %s with reasonCode %s"),
		lim_msg_str(msg_type), lim_result_code_str(result_code));

	if (session_entry == NULL) {
		rsp_len = sizeof(tSirSmeJoinRsp);
		sme_join_rsp = cdf_mem_malloc(rsp_len);
		if (NULL == sme_join_rsp) {
			lim_log(mac_ctx, LOGP,
				FL("Mem Alloc fail - JOIN/REASSOC_RSP"));
			return;
		}

		cdf_mem_set((uint8_t *) sme_join_rsp, rsp_len, 0);
		sme_join_rsp->beaconLength = 0;
		sme_join_rsp->assocReqLength = 0;
		sme_join_rsp->assocRspLength = 0;
	} else {
		rsp_len = session_entry->assocReqLen +
			session_entry->assocRspLen + session_entry->bcnLen +
#ifdef WLAN_FEATURE_VOWIFI_11R
			session_entry->RICDataLen +
#endif
#ifdef FEATURE_WLAN_ESE
			session_entry->tspecLen +
#endif
			sizeof(tSirSmeJoinRsp) - sizeof(uint8_t);
		sme_join_rsp = cdf_mem_malloc(rsp_len);
		if (NULL == sme_join_rsp) {
			lim_log(mac_ctx, LOGP,
				FL("MemAlloc fail - JOIN/REASSOC_RSP"));
			return;
		}
		cdf_mem_set((uint8_t *) sme_join_rsp, rsp_len, 0);
		if (result_code == eSIR_SME_SUCCESS) {
			sta_ds = dph_get_hash_entry(mac_ctx,
				DPH_STA_HASH_INDEX_PEER,
				&session_entry->dph.dphHashTable);
			if (sta_ds == NULL) {
				lim_log(mac_ctx, LOGE,
					FL("Get Self Sta Entry fail"));
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
			}
		}
		sme_join_rsp->beaconLength = 0;
		sme_join_rsp->assocReqLength = 0;
		sme_join_rsp->assocRspLength = 0;
#ifdef WLAN_FEATURE_VOWIFI_11R
		sme_join_rsp->parsedRicRspLen = 0;
#endif
#ifdef FEATURE_WLAN_ESE
		sme_join_rsp->tspecIeLen = 0;
#endif

		lim_handle_join_rsp_status(mac_ctx, session_entry, result_code,
			sme_join_rsp);
	}

	sme_join_rsp->messageType = msg_type;
	sme_join_rsp->length = (uint16_t) rsp_len;
	sme_join_rsp->statusCode = result_code;
	sme_join_rsp->protStatusCode = prot_status_code;

	sme_join_rsp->sessionId = sme_session_id;
	sme_join_rsp->transactionId = sme_transaction_id;

	lim_send_sme_join_reassoc_rsp_after_resume(mac_ctx, CDF_STATUS_SUCCESS,
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
	tSirMsgQ mmhMsg;
	tSirSmeStartBssRsp *pSirSmeRsp;
	uint16_t ieLen;
	uint16_t ieOffset, curLen;

	PELOG1(lim_log(pMac, LOG1, FL("Sending message %s with reasonCode %s"),
		       lim_msg_str(msgType), lim_result_code_str(resultCode));
	       )

	size = sizeof(tSirSmeStartBssRsp);

	if (psessionEntry == NULL) {
		pSirSmeRsp = cdf_mem_malloc(size);
		if (NULL == pSirSmeRsp) {
			/* / Buffer not available. Log error */
			lim_log(pMac, LOGP,
				FL
					("call to AllocateMemory failed for eWNI_SME_START_BSS_RSP"));
			return;
		}
		cdf_mem_set((uint8_t *) pSirSmeRsp, size, 0);

	} else {
		/* subtract size of beaconLength + Mac Hdr + Fixed Fields before SSID */
		ieOffset = sizeof(tAniBeaconStruct) + SIR_MAC_B_PR_SSID_OFFSET;
		ieLen = psessionEntry->schBeaconOffsetBegin
			+ psessionEntry->schBeaconOffsetEnd - ieOffset;
		/* calculate the memory size to allocate */
		size += ieLen;

		pSirSmeRsp = cdf_mem_malloc(size);
		if (NULL == pSirSmeRsp) {
			/* / Buffer not available. Log error */
			lim_log(pMac, LOGP,
				FL
					("call to AllocateMemory failed for eWNI_SME_START_BSS_RSP"));

			return;
		}
		cdf_mem_set((uint8_t *) pSirSmeRsp, size, 0);
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
				lim_log(pMac, LOGP,
					FL
						("could not retrieve Capabilities value"));

			lim_get_phy_mode(pMac,
					 (uint32_t *) &pSirSmeRsp->bssDescription.
					 nwType, psessionEntry);

			pSirSmeRsp->bssDescription.channelId =
				psessionEntry->currentOperChannel;

			curLen = psessionEntry->schBeaconOffsetBegin - ieOffset;
			cdf_mem_copy((uint8_t *) &pSirSmeRsp->bssDescription.
				     ieFields,
				     psessionEntry->pSchBeaconFrameBegin +
				     ieOffset, (uint32_t) curLen);

			cdf_mem_copy(((uint8_t *) &pSirSmeRsp->bssDescription.
				      ieFields) + curLen,
				     psessionEntry->pSchBeaconFrameEnd,
				     (uint32_t) psessionEntry->
				     schBeaconOffsetEnd);

			/* subtracting size of length indicator itself and size of pointer to ieFields */
			pSirSmeRsp->bssDescription.length =
				sizeof(tSirBssDescription) - sizeof(uint16_t) -
				sizeof(uint32_t) + ieLen;
			/* This is the size of the message, subtracting the size of the pointer to ieFields */
			size += ieLen - sizeof(uint32_t);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
			if (psessionEntry->cc_switch_mode
			    != CDF_MCC_TO_SCC_SWITCH_DISABLE) {
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
#ifdef WLAN_FEATURE_11AC
				pSirSmeRsp->HTProfile.vhtCapability =
					psessionEntry->vhtCapability;
				pSirSmeRsp->HTProfile.vhtTxChannelWidthSet =
					psessionEntry->vhtTxChannelWidthSet;
				pSirSmeRsp->HTProfile.apCenterChan =
					psessionEntry->ch_center_freq_seg0;
				pSirSmeRsp->HTProfile.apChanWidth =
					psessionEntry->ch_width;
#endif
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
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace_msg_tx
			       (pMac, psessionEntry->peSessionId, mmhMsg.type));
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_START_BSS_RSP_EVENT,
			      psessionEntry, (uint16_t) resultCode, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
} /*** end lim_send_sme_start_bss_rsp() ***/

#define LIM_MAX_NUM_OF_SCAN_RESULTS_REPORTED  20
#define LIM_SIZE_OF_EACH_BSS  400       /* this is a rough estimate */

/**
 * lim_send_sme_scan_rsp() - Send scan response to SME
 * @pMac:         Pointer to Global MAC structure
 * @length:       Indicates length of message
 * @resultCode:   Indicates the result of previously issued
 *                     eWNI_SME_SCAN_REQ message
 * @scan_id: scan identifier
 *
 * This function is called by lim_process_sme_req_messages() to send
 * eWNI_SME_SCAN_RSP message to applications above MAC
 *
 * return: None
 */

void
lim_send_sme_scan_rsp(tpAniSirGlobal pMac, tSirResultCodes resultCode,
		uint8_t smesessionId, uint16_t smetranscationId,
		uint32_t scan_id)
{
	lim_log(pMac, LOG1,
		FL("Sending message SME_SCAN_RSP reasonCode %s scanId %d"),
		lim_result_code_str(resultCode), scan_id);
	lim_post_sme_scan_rsp_message(pMac, resultCode, smesessionId,
				smetranscationId, scan_id);
}

/**
 * lim_post_sme_scan_rsp_message()
 *
 ***FUNCTION:
 * This function is called by lim_send_sme_scan_rsp() to send
 * eWNI_SME_SCAN_RSP message with failed result code
 *
 ***NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param length       Indicates length of message
 * @param resultCode   failed result code
 *
 * @return None
 */

void
lim_post_sme_scan_rsp_message(tpAniSirGlobal pMac,
			tSirResultCodes resultCode, uint8_t smesessionId,
			uint16_t smetransactionId,
			uint32_t scan_id)
{
	tpSirSmeScanRsp pSirSmeScanRsp;
	tSirMsgQ mmhMsg;

	lim_log(pMac, LOG1, FL("send SME_SCAN_RSP (reasonCode %s)."),
		lim_result_code_str(resultCode));

	pSirSmeScanRsp = cdf_mem_malloc(sizeof(tSirSmeScanRsp));
	if (NULL == pSirSmeScanRsp) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for eWNI_SME_SCAN_RSP"));
		return;
	}
	cdf_mem_set((void *)pSirSmeScanRsp, sizeof(tSirSmeScanRsp), 0);

	pSirSmeScanRsp->messageType = eWNI_SME_SCAN_RSP;
	pSirSmeScanRsp->statusCode = resultCode;

	/*Update SME session Id and transaction Id */
	pSirSmeScanRsp->sessionId = smesessionId;
	pSirSmeScanRsp->transcationId = smetransactionId;
	pSirSmeScanRsp->scan_id = scan_id;

	mmhMsg.type = eWNI_SME_SCAN_RSP;
	mmhMsg.bodyptr = pSirSmeScanRsp;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_SCAN_RSP_EVENT, NULL,
			      (uint16_t) resultCode, 0);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;

} /*** lim_post_sme_scan_rsp_message ***/

#ifdef FEATURE_OEM_DATA_SUPPORT

/**
 * lim_send_sme_oem_data_rsp()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() to send
 * eWNI_SME_OEM_DATA_RSP message to applications above MAC
 * Software.
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
 * @param pMsgBuf      Indicates the mlm message
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_OEM_DATA_RSP message
 *
 * @return None
 */

void lim_send_sme_oem_data_rsp(tpAniSirGlobal pMac, uint32_t *pMsgBuf,
			       tSirResultCodes resultCode)
{
	tSirMsgQ mmhMsg;
	tSirOemDataRsp *pSirSmeOemDataRsp = NULL;
	tLimMlmOemDataRsp *pMlmOemDataRsp = NULL;
	uint16_t msgLength;

	/* get the pointer to the mlm message */
	pMlmOemDataRsp = (tLimMlmOemDataRsp *) (pMsgBuf);

	msgLength = sizeof(tSirOemDataRsp);

	/* now allocate memory for the char buffer */
	pSirSmeOemDataRsp = cdf_mem_malloc(msgLength);
	if (NULL == pSirSmeOemDataRsp) {
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for pSirSmeOemDataRsp"));
		return;
	}
#if defined (ANI_LITTLE_BYTE_ENDIAN)
	sir_store_u16_n((uint8_t *) &pSirSmeOemDataRsp->length, msgLength);
	sir_store_u16_n((uint8_t *) &pSirSmeOemDataRsp->messageType,
			eWNI_SME_OEM_DATA_RSP);
#else
	pSirSmeOemDataRsp->length = msgLength;
	pSirSmeOemDataRsp->messageType = eWNI_SME_OEM_DATA_RSP;
#endif

	cdf_mem_copy(pSirSmeOemDataRsp->oemDataRsp, pMlmOemDataRsp->oemDataRsp,
		     OEM_DATA_RSP_SIZE);

	/* Now free the memory from MLM Rsp Message */
	cdf_mem_free(pMlmOemDataRsp);

	mmhMsg.type = eWNI_SME_OEM_DATA_RSP;
	mmhMsg.bodyptr = pSirSmeOemDataRsp;
	mmhMsg.bodyval = 0;

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
} /*** lim_send_sme_oem_data_rsp ***/

#endif

void lim_send_sme_disassoc_deauth_ntf(tpAniSirGlobal pMac,
				      CDF_STATUS status, uint32_t *pCtx)
{
	tSirMsgQ mmhMsg;
	tSirMsgQ *pMsg = (tSirMsgQ *) pCtx;

	mmhMsg.type = pMsg->type;
	mmhMsg.bodyptr = pMsg;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));

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
	uint32_t *pMsg;
	bool failure = false;

	lim_log(pMac, LOG1, FL("Disassoc Ntf with trigger : %d reasonCode: %d"),
		disassocTrigger, reasonCode);

	switch (disassocTrigger) {
	case eLIM_PEER_ENTITY_DISASSOC:
		if (reasonCode != eSIR_SME_STA_NOT_ASSOCIATED) {
			failure = true;
			goto error;
		}

	case eLIM_HOST_DISASSOC:
		/**
		 * Disassociation response due to
		 * host triggered disassociation
		 */

		pSirSmeDisassocRsp = cdf_mem_malloc(sizeof(tSirSmeDisassocRsp));
		if (NULL == pSirSmeDisassocRsp) {
			/* Log error */
			lim_log(pMac, LOGP, FL("Memory allocation failed"));
			failure = true;
			goto error;
		}
		lim_log(pMac, LOG1, FL("send eWNI_SME_DISASSOC_RSP with "
				       "retCode: %d for " MAC_ADDRESS_STR),
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
		cdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		/* Clear Station Stats */
		/* for sta, it is always 1, IBSS is handled at halInitSta */

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */

		lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_RSP_EVENT,
				      psessionEntry, (uint16_t) reasonCode, 0);
#endif
		pMsg = (uint32_t *) pSirSmeDisassocRsp;
		break;

	default:
		/**
		 * Disassociation indication due to Disassociation
		 * frame reception from peer entity or due to
		 * loss of link with peer entity.
		 */
		pSirSmeDisassocInd = cdf_mem_malloc(sizeof(tSirSmeDisassocInd));
		if (NULL == pSirSmeDisassocInd) {
			/* Log error */
			lim_log(pMac, LOGP, FL("Memory allocation failed"));
			failure = true;
			goto error;
		}
		lim_log(pMac, LOG1, FL("send eWNI_SME_DISASSOC_IND with "
				       "retCode: %d for " MAC_ADDRESS_STR),
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

		cdf_mem_copy(pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		cdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_IND_EVENT,
				      psessionEntry, (uint16_t) reasonCode, 0);
#endif
		pMsg = (uint32_t *) pSirSmeDisassocInd;

		break;
	}

error:
	/* Delete the PE session Created */
	if ((psessionEntry != NULL) &&
	    (LIM_IS_STA_ROLE(psessionEntry) ||
	    LIM_IS_BT_AMP_STA_ROLE(psessionEntry))) {
		pe_delete_session(pMac, psessionEntry);
	}

	if (false == failure)
		lim_send_sme_disassoc_deauth_ntf(pMac, CDF_STATUS_SUCCESS,
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
	tSirMsgQ mmhMsg;
	tSirSmeDisassocInd *pSirSmeDisassocInd;

	pSirSmeDisassocInd = cdf_mem_malloc(sizeof(tSirSmeDisassocInd));
	if (NULL == pSirSmeDisassocInd) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for eWNI_SME_DISASSOC_IND"));
		return;
	}

	pSirSmeDisassocInd->messageType = eWNI_SME_DISASSOC_IND;
	pSirSmeDisassocInd->length = sizeof(tSirSmeDisassocInd);

	pSirSmeDisassocInd->sessionId = psessionEntry->smeSessionId;
	pSirSmeDisassocInd->transactionId = psessionEntry->transactionId;
	pSirSmeDisassocInd->statusCode = pStaDs->mlmStaContext.disassocReason;
	pSirSmeDisassocInd->reasonCode = pStaDs->mlmStaContext.disassocReason;

	cdf_mem_copy(pSirSmeDisassocInd->bssId, psessionEntry->bssId,
		     sizeof(tSirMacAddr));

	cdf_mem_copy(pSirSmeDisassocInd->peerMacAddr, pStaDs->staAddr,
		     sizeof(tSirMacAddr));

	pSirSmeDisassocInd->staId = pStaDs->staIndex;

	mmhMsg.type = eWNI_SME_DISASSOC_IND;
	mmhMsg.bodyptr = pSirSmeDisassocInd;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, mmhMsg.type));
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
	tSirMsgQ mmhMsg;
	tSirSmeDeauthInd *pSirSmeDeauthInd;

	pSirSmeDeauthInd = cdf_mem_malloc(sizeof(tSirSmeDeauthInd));
	if (NULL == pSirSmeDeauthInd) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for eWNI_SME_DEAUTH_IND "));
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
	cdf_mem_copy(pSirSmeDeauthInd->bssId, psessionEntry->bssId,
		     sizeof(tSirMacAddr));
	/* peerMacAddr */
	cdf_mem_copy(pSirSmeDeauthInd->peerMacAddr, pStaDs->staAddr,
		     sizeof(tSirMacAddr));
	pSirSmeDeauthInd->reasonCode = pStaDs->mlmStaContext.disassocReason;

	pSirSmeDeauthInd->staId = pStaDs->staIndex;

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
	tSirMsgQ mmhMsg;
	tSirTdlsDelStaInd *pSirTdlsDelStaInd;

	pSirTdlsDelStaInd = cdf_mem_malloc(sizeof(tSirTdlsDelStaInd));
	if (NULL == pSirTdlsDelStaInd) {
		lim_log(pMac, LOGP,
			FL
				("AllocateMemory failed for eWNI_SME_TDLS_DEL_STA_IND "));
		return;
	}
	/* messageType */
	pSirTdlsDelStaInd->messageType = eWNI_SME_TDLS_DEL_STA_IND;
	pSirTdlsDelStaInd->length = sizeof(tSirTdlsDelStaInd);

	/* sessionId */
	pSirTdlsDelStaInd->sessionId = psessionEntry->smeSessionId;

	/* peerMacAddr */
	cdf_mem_copy(pSirTdlsDelStaInd->peerMac, pStaDs->staAddr,
		     sizeof(tSirMacAddr));

	/* staId */
	lim_copy_u16((uint8_t *) (&pSirTdlsDelStaInd->staId),
		     (uint16_t) pStaDs->staIndex);

	/* reasonCode */
	lim_copy_u16((uint8_t *) (&pSirTdlsDelStaInd->reasonCode), reasonCode);

	mmhMsg.type = eWNI_SME_TDLS_DEL_STA_IND;
	mmhMsg.bodyptr = pSirTdlsDelStaInd;
	mmhMsg.bodyval = 0;

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
} /*** end lim_send_sme_tdls_del_sta_ind() ***/

/**
 * lim_send_sme_tdls_delete_all_peer_ind()
 *
 ***FUNCTION:
 * This function is called to send the eWNI_SME_TDLS_DEL_ALL_PEER_IND
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
 * @return None
 */
void
lim_send_sme_tdls_delete_all_peer_ind(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	tSirMsgQ mmhMsg;
	tSirTdlsDelAllPeerInd *pSirTdlsDelAllPeerInd;

	pSirTdlsDelAllPeerInd = cdf_mem_malloc(sizeof(tSirTdlsDelAllPeerInd));
	if (NULL == pSirTdlsDelAllPeerInd) {
		lim_log(pMac, LOGP,
			FL
				("AllocateMemory failed for eWNI_SME_TDLS_DEL_ALL_PEER_IND"));
		return;
	}
	/* messageType */
	pSirTdlsDelAllPeerInd->messageType = eWNI_SME_TDLS_DEL_ALL_PEER_IND;
	pSirTdlsDelAllPeerInd->length = sizeof(tSirTdlsDelAllPeerInd);

	/* sessionId */
	pSirTdlsDelAllPeerInd->sessionId = psessionEntry->smeSessionId;

	mmhMsg.type = eWNI_SME_TDLS_DEL_ALL_PEER_IND;
	mmhMsg.bodyptr = pSirTdlsDelAllPeerInd;
	mmhMsg.bodyval = 0;

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
} /*** end lim_send_sme_tdls_delete_all_peer_ind() ***/

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
				tpPESession psessionEntry, uint32_t txCompleteStatus)
{
	tSirMsgQ mmhMsg;
	tSirMgmtTxCompletionInd *pSirMgmtTxCompletionInd;

	pSirMgmtTxCompletionInd =
		cdf_mem_malloc(sizeof(tSirMgmtTxCompletionInd));
	if (NULL == pSirMgmtTxCompletionInd) {
		lim_log(pMac, LOGP,
			FL
				("AllocateMemory failed for eWNI_SME_MGMT_FRM_TX_COMPLETION_IND"));
		return;
	}
	/* messageType */
	pSirMgmtTxCompletionInd->messageType =
		eWNI_SME_MGMT_FRM_TX_COMPLETION_IND;
	pSirMgmtTxCompletionInd->length = sizeof(tSirMgmtTxCompletionInd);

	/* sessionId */
	pSirMgmtTxCompletionInd->sessionId = psessionEntry->smeSessionId;

	pSirMgmtTxCompletionInd->txCompleteStatus = txCompleteStatus;

	mmhMsg.type = eWNI_SME_MGMT_FRM_TX_COMPLETION_IND;
	mmhMsg.bodyptr = pSirMgmtTxCompletionInd;
	mmhMsg.bodyval = 0;

	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
} /*** end lim_send_sme_tdls_delete_all_peer_ind() ***/

void lim_send_sme_tdls_event_notify(tpAniSirGlobal pMac, uint16_t msgType,
				    void *events)
{
	tSirMsgQ mmhMsg;

	switch (msgType) {
	case SIR_HAL_TDLS_SHOULD_DISCOVER:
		mmhMsg.type = eWNI_SME_TDLS_SHOULD_DISCOVER;
		break;
	case SIR_HAL_TDLS_SHOULD_TEARDOWN:
		mmhMsg.type = eWNI_SME_TDLS_SHOULD_TEARDOWN;
		break;
	case SIR_HAL_TDLS_PEER_DISCONNECTED:
		mmhMsg.type = eWNI_SME_TDLS_PEER_DISCONNECTED;
		break;
	}

	mmhMsg.bodyptr = events;
	mmhMsg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;
}
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

	psessionEntry = pe_find_session_by_bssid(pMac, peerMacAddr, &sessionId);
	switch (deauthTrigger) {
	case eLIM_PEER_ENTITY_DEAUTH:
		return;

	case eLIM_HOST_DEAUTH:
		/**
		 * Deauthentication response to host triggered
		 * deauthentication.
		 */
		pSirSmeDeauthRsp = cdf_mem_malloc(sizeof(tSirSmeDeauthRsp));
		if (NULL == pSirSmeDeauthRsp) {
			/* Log error */
			lim_log(pMac, LOGP,
				FL
					("call to AllocateMemory failed for eWNI_SME_DEAUTH_RSP"));

			return;
		}
		lim_log(pMac, LOG1, FL("send eWNI_SME_DEAUTH_RSP with "
				       "retCode: %d for" MAC_ADDRESS_STR),
			reasonCode, MAC_ADDR_ARRAY(peerMacAddr));
		pSirSmeDeauthRsp->messageType = eWNI_SME_DEAUTH_RSP;
		pSirSmeDeauthRsp->length = sizeof(tSirSmeDeauthRsp);
		pSirSmeDeauthRsp->statusCode = reasonCode;
		pSirSmeDeauthRsp->sessionId = smesessionId;
		pSirSmeDeauthRsp->transactionId = smetransactionId;

		pBuf = (uint8_t *) pSirSmeDeauthRsp->peerMacAddr;
		cdf_mem_copy(pBuf, peerMacAddr, sizeof(tSirMacAddr));

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
		lim_diag_event_report(pMac, WLAN_PE_DIAG_DEAUTH_RSP_EVENT,
				      psessionEntry, 0, (uint16_t) reasonCode);
#endif
		pMsg = (uint32_t *) pSirSmeDeauthRsp;

		break;

	default:
		/**
		 * Deauthentication indication due to Deauthentication
		 * frame reception from peer entity or due to
		 * loss of link with peer entity.
		 */
		pSirSmeDeauthInd = cdf_mem_malloc(sizeof(tSirSmeDeauthInd));
		if (NULL == pSirSmeDeauthInd) {
			/* Log error */
			lim_log(pMac, LOGP,
				FL
					("call to AllocateMemory failed for eWNI_SME_DEAUTH_Ind"));

			return;
		}
		lim_log(pMac, LOG1, FL("send eWNI_SME_DEAUTH_IND with "
				       "retCode: %d for " MAC_ADDRESS_STR),
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
		cdf_mem_copy(pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);

		/* peerMacAddr */
		cdf_mem_copy(pSirSmeDeauthInd->peerMacAddr, peerMacAddr,
			     sizeof(tSirMacAddr));

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

	lim_send_sme_disassoc_deauth_ntf(pMac, CDF_STATUS_SUCCESS,
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
	tSirMsgQ msg;
	tSirSmeWmStatusChangeNtf *wm_status_change_ntf;

	wm_status_change_ntf = cdf_mem_malloc(sizeof(tSirSmeWmStatusChangeNtf));
	if (NULL == wm_status_change_ntf) {
		lim_log(mac_ctx, LOGE,
			FL("Mem Alloc failed - eWNI_SME_WM_STATUS_CHANGE_NTF"));
		return;
	}

	msg.type = eWNI_SME_WM_STATUS_CHANGE_NTF;
	msg.bodyval = 0;
	msg.bodyptr = wm_status_change_ntf;

	switch (status_change_code) {
	case eSIR_SME_RADAR_DETECTED:
		break;
	default:
		wm_status_change_ntf->messageType =
			eWNI_SME_WM_STATUS_CHANGE_NTF;
		wm_status_change_ntf->statusChangeCode = status_change_code;
		wm_status_change_ntf->length = sizeof(tSirSmeWmStatusChangeNtf);
		wm_status_change_ntf->sessionId = session_id;
		if (sizeof(wm_status_change_ntf->statusChangeInfo) >=
			info_len) {
			cdf_mem_copy(
			    (uint8_t *) &wm_status_change_ntf->statusChangeInfo,
			    (uint8_t *) status_change_info, info_len);
		}
		lim_log(mac_ctx, LOGE,
			FL("**---** StatusChg: code 0x%x, length %d **---**"),
			status_change_code, info_len);
		break;
	}

	MTRACE(mac_trace_msg_tx(mac_ctx, session_id, msg.type));
	if (eSIR_SUCCESS != lim_sys_process_mmh_msg_api(mac_ctx, &msg, ePROT)) {
		cdf_mem_free(wm_status_change_ntf);
		lim_log(mac_ctx, LOGP,
			FL("lim_sys_process_mmh_msg_api failed"));
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
			     tSirMacAddr peerMacAddr, uint16_t aid,
			     tSirResultCodes resultCode,
			     tpPESession psessionEntry, uint8_t smesessionId,
			     uint16_t smetransactionId)
{

	uint8_t *pBuf;
	tSirMsgQ mmhMsg;
	tSirSmeSetContextRsp *pSirSmeSetContextRsp;

	pSirSmeSetContextRsp = cdf_mem_malloc(sizeof(tSirSmeSetContextRsp));
	if (NULL == pSirSmeSetContextRsp) {
		/* Log error */
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for SmeSetContextRsp"));

		return;
	}

	pSirSmeSetContextRsp->messageType = eWNI_SME_SETCONTEXT_RSP;
	pSirSmeSetContextRsp->length = sizeof(tSirSmeSetContextRsp);
	pSirSmeSetContextRsp->statusCode = resultCode;

	pBuf = pSirSmeSetContextRsp->peerMacAddr;

	cdf_mem_copy(pBuf, (uint8_t *) peerMacAddr, sizeof(tSirMacAddr));
	pBuf += sizeof(tSirMacAddr);

	/* Update SME session and transaction Id */
	pSirSmeSetContextRsp->sessionId = smesessionId;
	pSirSmeSetContextRsp->transactionId = smetransactionId;

	mmhMsg.type = eWNI_SME_SETCONTEXT_RSP;
	mmhMsg.bodyptr = pSirSmeSetContextRsp;
	mmhMsg.bodyval = 0;
	if (NULL == psessionEntry) {
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace_msg_tx
			       (pMac, psessionEntry->peSessionId, mmhMsg.type));
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
	tSirMsgQ msgQ;
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
		lim_log(pMac, LOGP,
			FL("could not get NEIGHBOR_BSS_IND from CFG"));

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
	pNewBssInd = cdf_mem_malloc(val);
	if (NULL == pNewBssInd) {
		/* Log error */
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for eWNI_SME_NEIGHBOR_BSS_IND"));

		return;
	}

	pNewBssInd->messageType = eWNI_SME_NEIGHBOR_BSS_IND;
	pNewBssInd->length = (uint16_t) val;
	pNewBssInd->sessionId = 0;

	cdf_mem_copy((uint8_t *) pNewBssInd->bssDescription,
		     (uint8_t *) &pBssDescr->bssDescription,
		     pBssDescr->bssDescription.length + sizeof(uint16_t));

	msgQ.type = eWNI_SME_NEIGHBOR_BSS_IND;
	msgQ.bodyptr = pNewBssInd;
	msgQ.bodyval = 0;
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
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
	tSirMsgQ mmhMsg;

	if (!rspReqd)
		return;

	rsp = cdf_mem_malloc(sizeof(tSirAddtsRsp));
	if (NULL == rsp) {
		lim_log(pMac, LOGP, FL("AllocateMemory failed for ADDTS_RSP"));
		return;
	}

	cdf_mem_set((uint8_t *) rsp, sizeof(*rsp), 0);
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
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace_msg_tx
			       (pMac, psessionEntry->peSessionId, mmhMsg.type));
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
	tSirMsgQ mmhMsg;

	lim_log(pMac, LOGW, "SendSmeDeltsRsp (aid %d, tsid %d, up %d) status %d",
		delts->aid,
		delts->req.tsinfo.traffic.tsid,
		delts->req.tsinfo.traffic.userPrio, status);
	if (!delts->rspReqd)
		return;

	rsp = cdf_mem_malloc(sizeof(tSirDeltsRsp));
	if (NULL == rsp) {
		/* Log error */
		lim_log(pMac, LOGP, FL("AllocateMemory failed for DELTS_RSP"));
		return;
	}
	cdf_mem_set((uint8_t *) rsp, sizeof(*rsp), 0);

	if (psessionEntry != NULL) {

		rsp->aid = delts->aid;
		cdf_mem_copy((uint8_t *) &rsp->macAddr[0],
			     (uint8_t *) &delts->macAddr[0], 6);
		cdf_mem_copy((uint8_t *) &rsp->rsp, (uint8_t *) &delts->req,
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
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	} else {
		MTRACE(mac_trace_msg_tx
			       (pMac, psessionEntry->peSessionId, mmhMsg.type));
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
	tSirMsgQ mmhMsg;

	lim_log(pMac, LOGW, "SendSmeDeltsInd (aid %d, tsid %d, up %d)",
		aid, delts->tsinfo.traffic.tsid, delts->tsinfo.traffic.userPrio);

	rsp = cdf_mem_malloc(sizeof(tSirDeltsRsp));
	if (NULL == rsp) {
		/* Log error */
		lim_log(pMac, LOGP, FL("AllocateMemory failed for DELTS_IND"));
		return;
	}
	cdf_mem_set((uint8_t *) rsp, sizeof(*rsp), 0);

	rsp->messageType = eWNI_SME_DELTS_IND;
	rsp->rc = eSIR_SUCCESS;
	rsp->aid = aid;
	cdf_mem_copy((uint8_t *) &rsp->rsp, (uint8_t *) delts, sizeof(*delts));

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
	tSirMsgQ mmhMsg;
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
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;

} /*** end lim_send_sme_pe_statistics_rsp() ***/

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
/**
 * lim_send_sme_pe_ese_tsm_rsp()
 *
 ***FUNCTION:
 * This function is called to send tsm stats response to HDD.
 * This function posts the result back to HDD. This is a response to
 * HDD's request to get tsm stats.
 *
 ***PARAMS:
 * @param pMac   - Pointer to global pMac structure
 * @param pStats - Pointer to TSM Stats
 *
 * @return none
 */

void lim_send_sme_pe_ese_tsm_rsp(tpAniSirGlobal pMac, tAniGetTsmStatsRsp *pStats)
{
	tSirMsgQ mmhMsg;
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
		PELOGE(lim_log
			       (pMac, LOGE, FL("Session not found for the Sta id(%d)"),
			       pPeStats->staId);
		       )
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
	MTRACE(mac_trace_msg_tx(pMac, sessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
} /*** end lim_send_sme_pe_ese_tsm_rsp() ***/

#endif /* FEATURE_WLAN_ESE) && FEATURE_WLAN_ESE_UPLOAD */

void
lim_send_sme_ibss_peer_ind(tpAniSirGlobal pMac,
			   tSirMacAddr peerMacAddr,
			   uint16_t staIndex,
			   uint8_t ucastIdx,
			   uint8_t bcastIdx,
			   uint8_t *beacon,
			   uint16_t beaconLen, uint16_t msgType, uint8_t sessionId)
{
	tSirMsgQ mmhMsg;
	tSmeIbssPeerInd *pNewPeerInd;

	pNewPeerInd = cdf_mem_malloc(sizeof(tSmeIbssPeerInd) + beaconLen);
	if (NULL == pNewPeerInd) {
		PELOGE(lim_log(pMac, LOGE, FL("Failed to allocate memory"));)
		return;
	}

	cdf_mem_set((void *)pNewPeerInd, (sizeof(tSmeIbssPeerInd) + beaconLen),
		    0);

	cdf_mem_copy((uint8_t *) pNewPeerInd->peerAddr,
		     peerMacAddr, sizeof(tSirMacAddr));
	pNewPeerInd->staId = staIndex;
	pNewPeerInd->ucastSig = ucastIdx;
	pNewPeerInd->bcastSig = bcastIdx;
	pNewPeerInd->mesgLen = sizeof(tSmeIbssPeerInd) + beaconLen;
	pNewPeerInd->mesgType = msgType;
	pNewPeerInd->sessionId = sessionId;

	if (beacon != NULL) {
		cdf_mem_copy((void *)((uint8_t *) pNewPeerInd +
				      sizeof(tSmeIbssPeerInd)), (void *)beacon,
			     beaconLen);
	}

	mmhMsg.type = msgType;
	mmhMsg.bodyptr = pNewPeerInd;
	MTRACE(mac_trace_msg_tx(pMac, sessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

}

/**
 * lim_handle_csa_offload_msg() - Handle CSA offload message
 * @mac_ctx:         pointer to global adapter context
 * @msg:             Message pointer.
 *
 * Return: None
 */
void lim_handle_csa_offload_msg(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	tpPESession session_entry;
	tSirMsgQ mmh_msg;
	tpCSAOffloadParams csa_params = (tpCSAOffloadParams) (msg->bodyptr);
	tpSmeCsaOffloadInd csa_offload_ind;
	tpDphHashNode sta_ds = NULL;
	uint8_t session_id;
	uint16_t aid = 0;
	tLimWiderBWChannelSwitchInfo *chnl_switch_info = NULL;

	if (!csa_params) {
		lim_log(mac_ctx, LOGE, FL("limMsgQ body ptr is NULL"));
		return;
	}

	session_entry =
		pe_find_session_by_bssid(mac_ctx,
			csa_params->bssId, &session_id);
	if (!session_entry) {
		lim_log(mac_ctx, LOGE,
			FL("Session does not exist"));
		goto err;
	}

	sta_ds = dph_lookup_hash_entry(mac_ctx, session_entry->bssId, &aid,
		&session_entry->dph.dphHashTable);

	if (!sta_ds) {
		lim_log(mac_ctx, LOGE,
			FL("sta_ds does not exist"));
		goto err;
	}

	if (LIM_IS_STA_ROLE(session_entry)) {
		/*
		 * on receiving channel switch announcement from AP, delete all
		 * TDLS peers before leaving BSS and proceed for channel switch
		 */
		lim_delete_tdls_peers(mac_ctx, session_entry);

		session_entry->gLimChannelSwitch.switchMode =
			csa_params->switchmode;
		/* timer already started by firmware, switch immediately */
		session_entry->gLimChannelSwitch.switchCount = 0;
		session_entry->gLimChannelSwitch.primaryChannel =
			csa_params->channel;
		session_entry->gLimChannelSwitch.state =
			eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
		session_entry->gLimChannelSwitch.ch_width = CH_WIDTH_20MHZ;

		if (session_entry->vhtCapability &&
				session_entry->htSupportedChannelWidthSet) {
			chnl_switch_info =
				&session_entry->gLimWiderBWChannelSwitch;
			if (csa_params->ies_present_flag & lim_wbw_ie_present) {
				chnl_switch_info->newChanWidth =
					csa_params->new_ch_width;
				chnl_switch_info->newCenterChanFreq0 =
					csa_params->new_ch_freq_seg1;
				chnl_switch_info->newCenterChanFreq1 =
					csa_params->new_ch_freq_seg2;
				session_entry->gLimChannelSwitch.state =
				   eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
				session_entry->gLimChannelSwitch.ch_width =
					csa_params->new_ch_width + 1;
			}
		} else if (session_entry->htSupportedChannelWidthSet) {
			if (csa_params->sec_chan_offset) {
				session_entry->gLimChannelSwitch.ch_width =
					CH_WIDTH_40MHZ;
				session_entry->gLimChannelSwitch.state =
				     eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			} else {
				session_entry->htSupportedChannelWidthSet =
					WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
				session_entry->htRecommendedTxWidthSet =
				    session_entry->htSupportedChannelWidthSet;
			}
		}
		lim_log(mac_ctx, LOG1, FL("new ch width = %d"),
			session_entry->gLimChannelSwitch.ch_width);

		lim_prepare_for11h_channel_switch(mac_ctx, session_entry);
		csa_offload_ind = cdf_mem_malloc(sizeof(tSmeCsaOffloadInd));
		if (NULL == csa_offload_ind) {
			lim_log(mac_ctx, LOGE,
				FL("memalloc fail eWNI_SME_CSA_OFFLOAD_EVENT"));
			goto err;
		}

		cdf_mem_set(csa_offload_ind, sizeof(tSmeCsaOffloadInd), 0);
		csa_offload_ind->mesgType = eWNI_SME_CSA_OFFLOAD_EVENT;
		csa_offload_ind->mesgLen = sizeof(tSmeCsaOffloadInd);
		cdf_mem_copy(csa_offload_ind->bssId, session_entry->bssId,
			sizeof(tSirMacAddr));
		mmh_msg.type = eWNI_SME_CSA_OFFLOAD_EVENT;
		mmh_msg.bodyptr = csa_offload_ind;
		mmh_msg.bodyval = 0;
		lim_log(mac_ctx, LOG1,
			FL("Sending eWNI_SME_CSA_OFFLOAD_EVENT to SME. "));
		MTRACE(mac_trace_msg_tx
			(mac_ctx, session_entry->peSessionId, mmh_msg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT
		lim_diag_event_report(mac_ctx,
			WLAN_PE_DIAG_SWITCH_CHL_IND_EVENT, session_entry,
			eSIR_SUCCESS, eSIR_SUCCESS);
#endif
		lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
	}

err:
	cdf_mem_free(csa_params);
}

/*--------------------------------------------------------------------------
   \brief pe_delete_session() - Handle the Delete BSS Response from HAL.

   \param pMac                   - pointer to global adapter context
   \param sessionId             - Message pointer.

   \sa
   --------------------------------------------------------------------------*/

void lim_handle_delete_bss_rsp(tpAniSirGlobal pMac, tpSirMsgQ MsgQ)
{
	tpPESession psessionEntry;
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) (MsgQ->bodyptr);

	psessionEntry =
		pe_find_session_by_session_id(pMac, pDelBss->sessionId);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("Session Does not exist for given sessionID %d"),
			pDelBss->sessionId);
		return;
	}
	if (LIM_IS_IBSS_ROLE(psessionEntry)) {
		lim_ibss_del_bss_rsp(pMac, MsgQ->bodyptr, psessionEntry);
	} else if (LIM_IS_UNKNOWN_ROLE(psessionEntry)) {
		lim_process_sme_del_bss_rsp(pMac, MsgQ->bodyval, psessionEntry);
	}

	else
		lim_process_mlm_del_bss_rsp(pMac, MsgQ, psessionEntry);

}

#ifdef WLAN_FEATURE_VOWIFI_11R
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
	tSirMsgQ mmhMsg;

	mmhMsg.type = eWNI_SME_FT_AGGR_QOS_RSP;
	mmhMsg.bodyptr = aggrQosRsp;
	mmhMsg.bodyval = 0;
	MTRACE(mac_trace_msg_tx(pMac, smesessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
}
#endif

void lim_send_sme_max_assoc_exceeded_ntf(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
					 uint8_t smesessionId)
{
	tSirMsgQ mmhMsg;
	tSmeMaxAssocInd *pSmeMaxAssocInd;

	pSmeMaxAssocInd = cdf_mem_malloc(sizeof(tSmeMaxAssocInd));
	if (NULL == pSmeMaxAssocInd) {
		PELOGE(lim_log(pMac, LOGE, FL("Failed to allocate memory"));)
		return;
	}
	cdf_mem_set((void *)pSmeMaxAssocInd, sizeof(tSmeMaxAssocInd), 0);
	cdf_mem_copy((uint8_t *) pSmeMaxAssocInd->peerMac,
		     (uint8_t *) peerMacAddr, sizeof(tSirMacAddr));
	pSmeMaxAssocInd->mesgType = eWNI_SME_MAX_ASSOC_EXCEEDED;
	pSmeMaxAssocInd->mesgLen = sizeof(tSmeMaxAssocInd);
	pSmeMaxAssocInd->sessionId = smesessionId;
	mmhMsg.type = pSmeMaxAssocInd->mesgType;
	mmhMsg.bodyptr = pSmeMaxAssocInd;
	PELOG1(lim_log(pMac, LOG1, FL("msgType %s peerMacAddr " MAC_ADDRESS_STR
				      " sme session id %d"),
		       "eWNI_SME_MAX_ASSOC_EXCEEDED",
		       MAC_ADDR_ARRAY(peerMacAddr));
	       )
	MTRACE(mac_trace_msg_tx(pMac, smesessionId, mmhMsg.type));
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return;
}

/** -----------------------------------------------------------------
   \brief lim_send_sme_dfs_event_notify() - sends
   eWNI_SME_DFS_RADAR_FOUND
   After receiving WMI_PHYERR_EVENTID indication frame from FW, this
   function sends a eWNI_SME_DFS_RADAR_FOUND to SME to notify
   that a RADAR is found on current operating channel and SAP-
   has to move to a new channel.
   \param pMac - global mac structure
   \param msgType - message type received from lower layer
   \param event - event data received from lower layer
   \return none
   \sa
   ----------------------------------------------------------------- */
void
lim_send_sme_dfs_event_notify(tpAniSirGlobal pMac, uint16_t msgType, void *event)
{
	tSirMsgQ mmhMsg;
	mmhMsg.type = eWNI_SME_DFS_RADAR_FOUND;
	mmhMsg.bodyptr = event;
	mmhMsg.bodyval = 0;
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
		PELOGE(lim_log(pMac, LOGE, FL("Unable to set CSA IE in beacon"));)
		return;
	}

	/* Send update beacon template message */
	lim_send_beacon_ind(pMac, psessionEntry);
	PELOG1(lim_log(pMac, LOG1,
		       FL(" Updated CSA IE, IE COUNT = %d"),
		       psessionEntry->gLimChannelSwitch.switchCount);
	       )

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
	tSirMsgQ mmhMsg;
	tpSwitchChannelParams pSmeSwithChnlParams;
	uint8_t channelId;

	pSmeSwithChnlParams = (tSwitchChannelParams *)
			      cdf_mem_malloc(sizeof(tSwitchChannelParams));
	if (NULL == pSmeSwithChnlParams) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for pSmeSwithChnlParams\n"));
		return;
	}

	cdf_mem_set((void *)pSmeSwithChnlParams,
		    sizeof(tSwitchChannelParams), 0);

	cdf_mem_copy(pSmeSwithChnlParams, pChnlParams,
		     sizeof(tSwitchChannelParams));

	channelId = pSmeSwithChnlParams->channelNumber;

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

	if (CHANNEL_STATE_DFS != cds_get_channel_state(channelId)) {
		if (channelId == psessionEntry->currentOperChannel) {
			lim_apply_configuration(pMac, psessionEntry);
			lim_send_beacon_ind(pMac, psessionEntry);
		} else {
			PELOG1(lim_log(pMac, LOG1,
				       FL
					       ("Failed to Transmit Beacons on channel = %d"
					       "after AP channel change response"),
				       psessionEntry->bcnLen);
			       )
		}
	}
	return;
}

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
	tSirMsgQ mmhMsg;
	tSirSmeCSAIeTxCompleteRsp *pChanSwTxResponse;
	struct sir_beacon_tx_complete_rsp *beacon_tx_comp_rsp_ptr;
	uint8_t length = sizeof(tSirSmeCSAIeTxCompleteRsp);
	tpSirFirstBeaconTxCompleteInd pBcnTxInd =
		(tSirFirstBeaconTxCompleteInd *) event;

	psessionEntry = pe_find_session_by_bss_idx(pMac, pBcnTxInd->bssIdx);
	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL("Session Does not exist for given sessionID"));
		return;
	}

	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    true == psessionEntry->dfsIncludeChanSwIe) {
		/* Send only 5 beacons with CSA IE Set in when a radar is detected */
		if (psessionEntry->gLimChannelSwitch.switchCount > 0) {
			/*
			 * Send the next beacon with updated CSA IE count
			 */
			lim_send_dfs_chan_sw_ie_update(pMac, psessionEntry);
			/* Decrement the IE count */
			psessionEntry->gLimChannelSwitch.switchCount--;
		} else {
			/* Done with CSA IE update, send response back to SME */
			psessionEntry->gLimChannelSwitch.switchCount = 0;
			if (pMac->sap.SapDfsInfo.disable_dfs_ch_switch == false)
				psessionEntry->gLimChannelSwitch.switchMode = 0;
			psessionEntry->dfsIncludeChanSwIe = false;
			psessionEntry->dfsIncludeChanWrapperIe = false;

			pChanSwTxResponse = (tSirSmeCSAIeTxCompleteRsp *)
					    cdf_mem_malloc(length);

			if (NULL == pChanSwTxResponse) {
				lim_log(pMac, LOGP,
					FL
						("AllocateMemory failed for tSirSmeCSAIeTxCompleteRsp"));
				return;
			}

			cdf_mem_set((void *)pChanSwTxResponse, length, 0);
			pChanSwTxResponse->sessionId =
				psessionEntry->smeSessionId;
			pChanSwTxResponse->chanSwIeTxStatus =
				CDF_STATUS_SUCCESS;

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
				cdf_mem_malloc(sizeof(*beacon_tx_comp_rsp_ptr));
		if (NULL == beacon_tx_comp_rsp_ptr) {
			lim_log(pMac, LOGP,
				FL
				("AllocateMemory failed for beacon_tx_comp_rsp_ptr"));
			return;
		}
		cdf_mem_set((void *)beacon_tx_comp_rsp_ptr,
					sizeof(*beacon_tx_comp_rsp_ptr), 0);
		beacon_tx_comp_rsp_ptr->session_id =
			psessionEntry->smeSessionId;
		beacon_tx_comp_rsp_ptr->tx_status = CDF_STATUS_SUCCESS;
		mmhMsg.type = eWNI_SME_NSS_UPDATE_RSP;
		mmhMsg.bodyptr = beacon_tx_comp_rsp_ptr;
		mmhMsg.bodyval = 0;
		lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	}
	return;
}
