/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/**=========================================================================

   \file  rrm_api.c

   \brief implementation for PE RRM APIs

   ========================================================================*/

/* $Header$ */


/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "cds_api.h"
#include "wni_api.h"
#include "sir_api.h"
#include "ani_global.h"
#include "wni_cfg.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_send_sme_rsp_messages.h"
#include "parser_api.h"
#include "lim_send_messages.h"
#include "rrm_global.h"
#include "rrm_api.h"

uint8_t
rrm_get_min_of_max_tx_power(tpAniSirGlobal pMac,
			    int8_t regMax, int8_t apTxPower)
{
	uint8_t maxTxPower = 0;
	uint8_t txPower = QDF_MIN(regMax, (apTxPower));
	if ((txPower >= RRM_MIN_TX_PWR_CAP) && (txPower <= RRM_MAX_TX_PWR_CAP))
		maxTxPower = txPower;
	else if (txPower < RRM_MIN_TX_PWR_CAP)
		maxTxPower = RRM_MIN_TX_PWR_CAP;
	else
		maxTxPower = RRM_MAX_TX_PWR_CAP;

	lim_log(pMac, LOG3,
		"%s: regulatoryMax = %d, apTxPwr = %d, maxTxpwr = %d",
		__func__, regMax, apTxPower, maxTxPower);
	return maxTxPower;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_cache_mgmt_tx_power
 **
 * FUNCTION:  Store Tx power for management frames.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pSessionEntry session entry.
 * @return None
 */
void
rrm_cache_mgmt_tx_power(tpAniSirGlobal pMac, int8_t txPower,
			tpPESession pSessionEntry)
{
	lim_log(pMac, LOG3, "Cache Mgmt Tx Power = %d", txPower);

	if (pSessionEntry == NULL) {
		lim_log(pMac, LOG3, "%s: pSessionEntry is NULL", __func__);
		pMac->rrm.rrmPEContext.txMgmtPower = txPower;
	} else
		pSessionEntry->txMgmtPower = txPower;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_get_mgmt_tx_power
 *
 * FUNCTION:  Get the Tx power for management frames.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pSessionEntry session entry.
 * @return txPower
 */
int8_t rrm_get_mgmt_tx_power(tpAniSirGlobal pMac, tpPESession pSessionEntry)
{
	lim_log(pMac, LOG3, "RrmGetMgmtTxPower called");

	if (pSessionEntry == NULL) {
		lim_log(pMac, LOG3, "%s: txpower from rrmPEContext: %d",
			__func__, pMac->rrm.rrmPEContext.txMgmtPower);
		return pMac->rrm.rrmPEContext.txMgmtPower;
	}

	return pSessionEntry->txMgmtPower;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_send_set_max_tx_power_req
 *
 * FUNCTION:  Send WMA_SET_MAX_TX_POWER_REQ message to change the max tx power.
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
rrm_send_set_max_tx_power_req(tpAniSirGlobal pMac, int8_t txPower,
			      tpPESession pSessionEntry)
{
	tpMaxTxPowerParams pMaxTxParams;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	if (pSessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE, FL("Invalid parameters"));)
		return eSIR_FAILURE;
	}
	pMaxTxParams = qdf_mem_malloc(sizeof(tMaxTxPowerParams));
	if (NULL == pMaxTxParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory for pMaxTxParams "));
		return eSIR_MEM_ALLOC_FAILED;

	}
	/* Allocated memory for pMaxTxParams...will be freed in other module */
	pMaxTxParams->power = txPower;
	qdf_mem_copy(pMaxTxParams->bssId.bytes, pSessionEntry->bssId,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(pMaxTxParams->selfStaMacAddr.bytes,
			pSessionEntry->selfMacAddr,
			QDF_MAC_ADDR_SIZE);

	msgQ.type = WMA_SET_MAX_TX_POWER_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pMaxTxParams;
	msgQ.bodyval = 0;

	lim_log(pMac, LOG3,
		FL("Sending WMA_SET_MAX_TX_POWER_REQ with power(%d) to HAL"),
		txPower);

	MTRACE(mac_trace_msg_tx(pMac, pSessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		lim_log(pMac, LOGP,
			FL
				("Posting WMA_SET_MAX_TX_POWER_REQ to HAL failed, reason=%X"),
			retCode);
		qdf_mem_free(pMaxTxParams);
		return retCode;
	}
	return retCode;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_set_max_tx_power_rsp
 *
 * FUNCTION:  Process WMA_SET_MAX_TX_POWER_RSP message.
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
tSirRetStatus rrm_set_max_tx_power_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ)
{
	tSirRetStatus retCode = eSIR_SUCCESS;
	tpMaxTxPowerParams pMaxTxParams = (tpMaxTxPowerParams) limMsgQ->bodyptr;
	tpPESession pSessionEntry;
	uint8_t sessionId, i;

	if (qdf_is_macaddr_broadcast(&pMaxTxParams->bssId)) {
		for (i = 0; i < pMac->lim.maxBssId; i++) {
			if ((pMac->lim.gpSession[i].valid == true)) {
				pSessionEntry = &pMac->lim.gpSession[i];
				rrm_cache_mgmt_tx_power(pMac, pMaxTxParams->power,
							pSessionEntry);
			}
		}
	} else {
		pSessionEntry = pe_find_session_by_bssid(pMac,
							 pMaxTxParams->bssId.bytes,
							 &sessionId);
		if (pSessionEntry == NULL) {
			PELOGE(lim_log
				       (pMac, LOGE, FL("Unable to find session:"));
			       )
			retCode = eSIR_FAILURE;
		} else {
			rrm_cache_mgmt_tx_power(pMac, pMaxTxParams->power,
						pSessionEntry);
		}
	}

	qdf_mem_free(limMsgQ->bodyptr);
	limMsgQ->bodyptr = NULL;
	return retCode;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_process_link_measurement_request
 *
 * FUNCTION:  Processes the Link measurement request and send the report.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pBd pointer to BD to extract RSSI and SNR
 * @param pLinkReq pointer to the Link request frame structure.
 * @param pSessionEntry session entry.
 * @return None
 */
tSirRetStatus
rrm_process_link_measurement_request(tpAniSirGlobal pMac,
				     uint8_t *pRxPacketInfo,
				     tDot11fLinkMeasurementRequest *pLinkReq,
				     tpPESession pSessionEntry)
{
	tSirMacLinkReport LinkReport;
	tpSirMacMgmtHdr pHdr;
	int8_t currentRSSI = 0;

	lim_log(pMac, LOG3, "Received Link measurement request");

	if (pRxPacketInfo == NULL || pLinkReq == NULL || pSessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE,
			       "%s Invalid parameters - Ignoring the request",
			       __func__);
		       )
		return eSIR_FAILURE;
	}
	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);

	LinkReport.txPower = lim_get_max_tx_power(pLinkReq->MaxTxPower.maxTxPower,
						  pLinkReq->MaxTxPower.maxTxPower,
						  pMac->roam.configParam.
						  nTxPowerCap);

	if ((LinkReport.txPower != (uint8_t) (pSessionEntry->maxTxPower)) &&
	    (eSIR_SUCCESS == rrm_send_set_max_tx_power_req(pMac,
							   LinkReport.txPower,
							   pSessionEntry))) {
		PELOGW(lim_log
			       (pMac, LOGW,
			       FL(" maxTx power in link report is not same as local..."
				  " Local = %d Link Request TxPower = %d"
				  " Link Report TxPower = %d"),
			       pSessionEntry->maxTxPower, LinkReport.txPower,
			       pLinkReq->MaxTxPower.maxTxPower);
		       )
			pSessionEntry->maxTxPower =
			LinkReport.txPower;
	}

	LinkReport.dialogToken = pLinkReq->DialogToken.token;
	LinkReport.rxAntenna = 0;
	LinkReport.txAntenna = 0;
	currentRSSI = WMA_GET_RX_RSSI_RAW(pRxPacketInfo);

	lim_log(pMac, LOG1, "Received Link report frame with %d", currentRSSI);

	/* 2008 11k spec reference: 18.4.8.5 RCPI Measurement */
	if ((currentRSSI) <= RCPI_LOW_RSSI_VALUE)
		LinkReport.rcpi = 0;
	else if ((currentRSSI > RCPI_LOW_RSSI_VALUE) && (currentRSSI <= 0))
		LinkReport.rcpi = CALCULATE_RCPI(currentRSSI);
	else
		LinkReport.rcpi = RCPI_MAX_VALUE;

	LinkReport.rsni = WMA_GET_RX_SNR(pRxPacketInfo);

	lim_log(pMac, LOG3, "Sending Link report frame");

	return lim_send_link_report_action_frame(pMac, &LinkReport, pHdr->sa,
						 pSessionEntry);
}

/* -------------------------------------------------------------------- */
/**
 * rrm_process_neighbor_report_response
 *
 * FUNCTION:  Processes the Neighbor Report response from the peer AP.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pNeighborRep pointer to the Neighbor report frame structure.
 * @param pSessionEntry session entry.
 * @return None
 */
tSirRetStatus
rrm_process_neighbor_report_response(tpAniSirGlobal pMac,
				     tDot11fNeighborReportResponse *pNeighborRep,
				     tpPESession pSessionEntry)
{
	tSirRetStatus status = eSIR_FAILURE;
	tpSirNeighborReportInd pSmeNeighborRpt = NULL;
	uint16_t length;
	uint8_t i;
	tSirMsgQ mmhMsg;

	if (pNeighborRep == NULL || pSessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE, FL(" Invalid parameters"));)
		return status;
	}

	lim_log(pMac, LOG3, FL("Neighbor report response received "));

	/* Dialog token */
	if (pMac->rrm.rrmPEContext.DialogToken !=
	    pNeighborRep->DialogToken.token) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       "Dialog token mismatch in the received Neighbor report");
		       )
		return eSIR_FAILURE;
	}
	if (pNeighborRep->num_NeighborReport == 0) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       "No neighbor report in the frame...Dropping it");
		       )
		return eSIR_FAILURE;
	}
	length = (sizeof(tSirNeighborReportInd)) +
		 (sizeof(tSirNeighborBssDescription) *
		  (pNeighborRep->num_NeighborReport - 1));

	/* Prepare the request to send to SME. */
	pSmeNeighborRpt = qdf_mem_malloc(length);
	if (NULL == pSmeNeighborRpt) {
		PELOGE(lim_log(pMac, LOGP, FL("Unable to allocate memory"));)
		return eSIR_MEM_ALLOC_FAILED;

	}
	qdf_mem_set(pSmeNeighborRpt, length, 0);

	/* Allocated memory for pSmeNeighborRpt...will be freed by other module */

	for (i = 0; i < pNeighborRep->num_NeighborReport; i++) {
		pSmeNeighborRpt->sNeighborBssDescription[i].length = sizeof(tSirNeighborBssDescription);        /*+ any optional ies */
		qdf_mem_copy(pSmeNeighborRpt->sNeighborBssDescription[i].bssId,
			     pNeighborRep->NeighborReport[i].bssid,
			     sizeof(tSirMacAddr));
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fApPreauthReachable =
			pNeighborRep->NeighborReport[i].APReachability;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fSameSecurityMode =
			pNeighborRep->NeighborReport[i].Security;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fSameAuthenticator =
			pNeighborRep->NeighborReport[i].KeyScope;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapSpectrumMeasurement =
			pNeighborRep->NeighborReport[i].SpecMgmtCap;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapQos = pNeighborRep->NeighborReport[i].QosCap;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapApsd = pNeighborRep->NeighborReport[i].apsd;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapRadioMeasurement = pNeighborRep->NeighborReport[i].rrm;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapDelayedBlockAck =
			pNeighborRep->NeighborReport[i].DelayedBA;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fCapImmediateBlockAck =
			pNeighborRep->NeighborReport[i].ImmBA;
		pSmeNeighborRpt->sNeighborBssDescription[i].bssidInfo.rrmInfo.
		fMobilityDomain =
			pNeighborRep->NeighborReport[i].MobilityDomain;

		pSmeNeighborRpt->sNeighborBssDescription[i].regClass =
			pNeighborRep->NeighborReport[i].regulatoryClass;
		pSmeNeighborRpt->sNeighborBssDescription[i].channel =
			pNeighborRep->NeighborReport[i].channel;
		pSmeNeighborRpt->sNeighborBssDescription[i].phyType =
			pNeighborRep->NeighborReport[i].PhyType;
	}

	pSmeNeighborRpt->messageType = eWNI_SME_NEIGHBOR_REPORT_IND;
	pSmeNeighborRpt->length = length;
	pSmeNeighborRpt->sessionId = pSessionEntry->smeSessionId;
	pSmeNeighborRpt->numNeighborReports = pNeighborRep->num_NeighborReport;
	qdf_mem_copy(pSmeNeighborRpt->bssId, pSessionEntry->bssId,
		     sizeof(tSirMacAddr));

	/* Send request to SME. */
	mmhMsg.type = pSmeNeighborRpt->messageType;
	mmhMsg.bodyptr = pSmeNeighborRpt;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
			 pSessionEntry->peSessionId, mmhMsg.type));
	status = lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);

	return status;

}

/* -------------------------------------------------------------------- */
/**
 * rrm_process_neighbor_report_req
 *
 * FUNCTION:
 *
 * LOGIC: Create a Neighbor report request and send it to peer.
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pNeighborReq Neighbor report request params .
 * @return None
 */
tSirRetStatus
rrm_process_neighbor_report_req(tpAniSirGlobal pMac,
				tpSirNeighborReportReqInd pNeighborReq)
{
	tSirRetStatus status = eSIR_SUCCESS;
	tSirMacNeighborReportReq NeighborReportReq;
	tpPESession pSessionEntry;
	uint8_t sessionId;

	if (pNeighborReq == NULL) {
		PELOGE(lim_log(pMac, LOGE, "NeighborReq is NULL");)
		return eSIR_FAILURE;
	}
	pSessionEntry = pe_find_session_by_bssid(pMac, pNeighborReq->bssId,
						 &sessionId);
	if (pSessionEntry == NULL) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       FL("session does not exist for given bssId"));
		       )
		return eSIR_FAILURE;
	}

	lim_log(pMac, LOG1, FL("SSID present = %d "), pNeighborReq->noSSID);

	qdf_mem_set(&NeighborReportReq, sizeof(tSirMacNeighborReportReq), 0);

	NeighborReportReq.dialogToken = ++pMac->rrm.rrmPEContext.DialogToken;
	NeighborReportReq.ssid_present = !pNeighborReq->noSSID;
	if (NeighborReportReq.ssid_present) {
		qdf_mem_copy(&NeighborReportReq.ssid, &pNeighborReq->ucSSID,
			     sizeof(tSirMacSSid));
		PELOGE(sir_dump_buf
			       (pMac, SIR_LIM_MODULE_ID, LOGE,
			       (uint8_t *) NeighborReportReq.ssid.ssId,
			       NeighborReportReq.ssid.length);
		       )
	}

	status =
		lim_send_neighbor_report_request_frame(pMac, &NeighborReportReq,
						       pNeighborReq->bssId,
						       pSessionEntry);

	return status;
}

#define ABS(x)      ((x < 0) ? -x : x)
/* -------------------------------------------------------------------- */
/**
 * rrm_process_beacon_report_req
 *
 * FUNCTION:  Processes the Beacon report request from the peer AP.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pCurrentReq pointer to the current Req comtext.
 * @param pBeaconReq pointer to the beacon report request IE from the peer.
 * @param pSessionEntry session entry.
 * @return None
 */
static tRrmRetStatus
rrm_process_beacon_report_req(tpAniSirGlobal pMac,
			      tpRRMReq pCurrentReq,
			      tDot11fIEMeasurementRequest *pBeaconReq,
			      tpPESession pSessionEntry)
{
	tSirMsgQ mmhMsg;
	tpSirBeaconReportReqInd pSmeBcnReportReq;
	uint8_t num_channels = 0, num_APChanReport;
	uint16_t measDuration, maxMeasduration;
	int8_t maxDuration;
	uint8_t sign;

	if (pBeaconReq->measurement_request.Beacon.BeaconReporting.present &&
	    (pBeaconReq->measurement_request.Beacon.BeaconReporting.
	     reportingCondition != 0)) {
		/* Repeated measurement is not supported. This means number of repetitions should be zero.(Already checked) */
		/* All test case in VoWifi(as of version 0.36)  use zero for number of repetitions. */
		/* Beacon reporting should not be included in request if number of repetitons is zero. */
		/* IEEE Std 802.11k-2008 Table 7-29g and section 11.10.8.1 */

		PELOGE(lim_log
			       (pMac, LOGE,
			       "Dropping the request: Reporting condition included in beacon report request and it is not zero");
		       )
		return eRRM_INCAPABLE;
	}

	/* The logic here is to check the measurement duration passed in the beacon request. Following are the cases handled.
	   Case 1: If measurement duration received in the beacon request is greater than the max measurement duration advertised
	   in the RRM capabilities(Assoc Req), and Duration Mandatory bit is set to 1, REFUSE the beacon request
	   Case 2: If measurement duration received in the beacon request is greater than the max measurement duration advertised
	   in the RRM capabilities(Assoc Req), and Duration Mandatory bit is set to 0, perform measurement for
	   the duration advertised in the RRM capabilities

	   maxMeasurementDuration = 2^(nonOperatingChanMax - 4) * BeaconInterval
	 */
	maxDuration =
		pMac->rrm.rrmPEContext.rrmEnabledCaps.nonOperatingChanMax - 4;
	sign = (maxDuration < 0) ? 1 : 0;
	maxDuration = (1L << ABS(maxDuration));
	if (!sign)
		maxMeasduration =
			maxDuration * pSessionEntry->beaconParams.beaconInterval;
	else
		maxMeasduration =
			pSessionEntry->beaconParams.beaconInterval / maxDuration;

	measDuration = pBeaconReq->measurement_request.Beacon.meas_duration;

	lim_log(pMac, LOG3,
		"maxDuration = %d sign = %d maxMeasduration = %d measDuration = %d",
		maxDuration, sign, maxMeasduration, measDuration);

	if (maxMeasduration < measDuration) {
		if (pBeaconReq->durationMandatory) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       "Dropping the request: duration mandatory and maxduration > measduration");
			       )
			return eRRM_REFUSED;
		} else
			measDuration = maxMeasduration;
	}
	/* Cache the data required for sending report. */
	pCurrentReq->request.Beacon.reportingDetail =
		pBeaconReq->measurement_request.Beacon.BcnReportingDetail.
		present ? pBeaconReq->measurement_request.Beacon.BcnReportingDetail.
		reportingDetail : BEACON_REPORTING_DETAIL_ALL_FF_IE;

	if (pBeaconReq->measurement_request.Beacon.RequestedInfo.present) {
		pCurrentReq->request.Beacon.reqIes.pElementIds =
			qdf_mem_malloc(sizeof(uint8_t) *
				       pBeaconReq->measurement_request.Beacon.
				       RequestedInfo.num_requested_eids);
		if (NULL == pCurrentReq->request.Beacon.reqIes.pElementIds) {
			lim_log(pMac, LOGP,
				FL
					("Unable to allocate memory for request IEs buffer"));
			return eRRM_FAILURE;
		}
		lim_log(pMac, LOG3, FL(" Allocated memory for pElementIds"));

		pCurrentReq->request.Beacon.reqIes.num =
			pBeaconReq->measurement_request.Beacon.RequestedInfo.
			num_requested_eids;
		qdf_mem_copy(pCurrentReq->request.Beacon.reqIes.pElementIds,
			     pBeaconReq->measurement_request.Beacon.
			     RequestedInfo.requested_eids,
			     pCurrentReq->request.Beacon.reqIes.num);
	}

	if (pBeaconReq->measurement_request.Beacon.num_APChannelReport) {
		for (num_APChanReport = 0;
		     num_APChanReport <
		     pBeaconReq->measurement_request.Beacon.num_APChannelReport;
		     num_APChanReport++)
			num_channels +=
				pBeaconReq->measurement_request.Beacon.
				APChannelReport[num_APChanReport].num_channelList;
	}
	/* Prepare the request to send to SME. */
	pSmeBcnReportReq = qdf_mem_malloc(sizeof(tSirBeaconReportReqInd));
	if (NULL == pSmeBcnReportReq) {
		lim_log(pMac, LOGP,
			FL
				("Unable to allocate memory during Beacon Report Req Ind to SME"));

		return eRRM_FAILURE;

	}

	qdf_mem_set(pSmeBcnReportReq, sizeof(tSirBeaconReportReqInd), 0);

	/* Allocated memory for pSmeBcnReportReq....will be freed by other modulea */
	qdf_mem_copy(pSmeBcnReportReq->bssId, pSessionEntry->bssId,
		     sizeof(tSirMacAddr));
	pSmeBcnReportReq->messageType = eWNI_SME_BEACON_REPORT_REQ_IND;
	pSmeBcnReportReq->length = sizeof(tSirBeaconReportReqInd);
	pSmeBcnReportReq->uDialogToken = pBeaconReq->measurement_token;
	pSmeBcnReportReq->msgSource = eRRM_MSG_SOURCE_11K;
	pSmeBcnReportReq->randomizationInterval =
		SYS_TU_TO_MS(pBeaconReq->measurement_request.Beacon.randomization);
	pSmeBcnReportReq->channelInfo.regulatoryClass =
		pBeaconReq->measurement_request.Beacon.regClass;
	pSmeBcnReportReq->channelInfo.channelNum =
		pBeaconReq->measurement_request.Beacon.channel;
	pSmeBcnReportReq->measurementDuration[0] = SYS_TU_TO_MS(measDuration);
	pSmeBcnReportReq->fMeasurementtype[0] =
		pBeaconReq->measurement_request.Beacon.meas_mode;
	qdf_mem_copy(pSmeBcnReportReq->macaddrBssid,
		     pBeaconReq->measurement_request.Beacon.BSSID,
		     sizeof(tSirMacAddr));

	if (pBeaconReq->measurement_request.Beacon.SSID.present) {
		pSmeBcnReportReq->ssId.length =
			pBeaconReq->measurement_request.Beacon.SSID.num_ssid;
		qdf_mem_copy(pSmeBcnReportReq->ssId.ssId,
			     pBeaconReq->measurement_request.Beacon.SSID.ssid,
			     pSmeBcnReportReq->ssId.length);
	}

	pCurrentReq->token = pBeaconReq->measurement_token;

	pSmeBcnReportReq->channelList.numChannels = num_channels;
	if (pBeaconReq->measurement_request.Beacon.num_APChannelReport) {
		uint8_t *pChanList =
			pSmeBcnReportReq->channelList.channelNumber;
		for (num_APChanReport = 0;
		     num_APChanReport <
		     pBeaconReq->measurement_request.Beacon.num_APChannelReport;
		     num_APChanReport++) {
			qdf_mem_copy(pChanList,
				     pBeaconReq->measurement_request.Beacon.
				     APChannelReport[num_APChanReport].
				     channelList,
				     pBeaconReq->measurement_request.Beacon.
				     APChannelReport[num_APChanReport].
				     num_channelList);

			pChanList +=
				pBeaconReq->measurement_request.Beacon.
				APChannelReport[num_APChanReport].num_channelList;
		}
	}
	/* Send request to SME. */
	mmhMsg.type = eWNI_SME_BEACON_REPORT_REQ_IND;
	mmhMsg.bodyptr = pSmeBcnReportReq;
	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
			 pSessionEntry->peSessionId, mmhMsg.type));
	return lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
}

/* -------------------------------------------------------------------- */
/**
 * rrm_fill_beacon_ies
 *
 * FUNCTION:
 *
 * LOGIC: Fills Fixed fields and Ies in bss description to an array of uint8_t.
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pIes - pointer to the buffer that should be populated with ies.
 * @param pNumIes - returns the num of ies filled in this param.
 * @param pIesMaxSize - Max size of the buffer pIes.
 * @param eids - pointer to array of eids. If NULL, all ies will be populated.
 * @param numEids - number of elements in array eids.
 * @param pBssDesc - pointer to Bss Description.
 * @return None
 */
static void
rrm_fill_beacon_ies(tpAniSirGlobal pMac,
		    uint8_t *pIes, uint8_t *pNumIes, uint8_t pIesMaxSize,
		    uint8_t *eids, uint8_t numEids, tpSirBssDescription pBssDesc)
{
	uint8_t len, *pBcnIes, count = 0, i;
	uint8_t BcnNumIes;

	if ((pIes == NULL) || (pNumIes == NULL) || (pBssDesc == NULL)) {
		PELOGE(lim_log(pMac, LOGE, FL(" Invalid parameters"));)
		return;
	}
	/* Make sure that if eid is null, numEids is set to zero. */
	numEids = (eids == NULL) ? 0 : numEids;

	pBcnIes = (uint8_t *) &pBssDesc->ieFields[0];
	BcnNumIes = (uint8_t) GET_IE_LEN_IN_BSS(pBssDesc->length);

	*pNumIes = 0;

	*((uint32_t *) pIes) = pBssDesc->timeStamp[0];
	*pNumIes += sizeof(uint32_t);
	pIes += sizeof(uint32_t);
	*((uint32_t *) pIes) = pBssDesc->timeStamp[1];
	*pNumIes += sizeof(uint32_t);
	pIes += sizeof(uint32_t);
	*((uint16_t *) pIes) = pBssDesc->beaconInterval;
	*pNumIes += sizeof(uint16_t);
	pIes += sizeof(uint16_t);
	*((uint16_t *) pIes) = pBssDesc->capabilityInfo;
	*pNumIes += sizeof(uint16_t);
	pIes += sizeof(uint16_t);

	while (BcnNumIes > 0) {
		len = *(pBcnIes + 1) + 2;       /* element id + length. */
		lim_log(pMac, LOG3, "EID = %d, len = %d total = %d",
			*pBcnIes, *(pBcnIes + 1), len);

		i = 0;
		do {
			if (((eids == NULL) || (*pBcnIes == eids[i])) &&
			    ((*pNumIes) + len) < pIesMaxSize) {
				lim_log(pMac, LOG3, "Adding Eid %d, len=%d",
					*pBcnIes, len);

				qdf_mem_copy(pIes, pBcnIes, len);
				pIes += len;
				*pNumIes += len;
				count++;
				break;
			}
			i++;
		} while (i < numEids);

		pBcnIes += len;
		BcnNumIes -= len;
	}
	lim_log(pMac, LOG1, "Total length of Ies added = %d", *pNumIes);
}

/**
 * rrm_process_beacon_report_xmit() - create a rrm action frame
 * @mac_ctx: Global pointer to MAC context
 * @beacon_xmit_ind: Data for beacon report IE from SME.
 *
 * Create a Radio measurement report action frame and send it to peer.
 *
 * Return: tSirRetStatus
 */
tSirRetStatus
rrm_process_beacon_report_xmit(tpAniSirGlobal mac_ctx,
			       tpSirBeaconReportXmitInd beacon_xmit_ind)
{
	tSirRetStatus status = eSIR_SUCCESS;
	tSirMacRadioMeasureReport *report = NULL;
	tSirMacBeaconReport *beacon_report;
	tpSirBssDescription bss_desc;
	tpRRMReq curr_req = mac_ctx->rrm.rrmPEContext.pCurrentReq;
	tpPESession session_entry;
	uint8_t session_id;
	uint8_t bss_desc_count = 0;

	lim_log(mac_ctx, LOG1, FL("Received beacon report xmit indication"));

	if (NULL == beacon_xmit_ind) {
		lim_log(mac_ctx, LOGE,
			FL("Received beacon_xmit_ind is NULL in PE"));
		return eSIR_FAILURE;
	}

	if (NULL == curr_req) {
		lim_log(mac_ctx, LOGE,
			FL("Received report xmit while there is no request pending in PE"));
		return eSIR_FAILURE;
	}

	if ((beacon_xmit_ind->numBssDesc) || curr_req->sendEmptyBcnRpt) {
		beacon_xmit_ind->numBssDesc = (beacon_xmit_ind->numBssDesc ==
			RRM_BCN_RPT_NO_BSS_INFO) ? RRM_BCN_RPT_MIN_RPT :
			beacon_xmit_ind->numBssDesc;

		session_entry = pe_find_session_by_bssid(mac_ctx,
				beacon_xmit_ind->bssId, &session_id);
		if (NULL == session_entry) {
			lim_log(mac_ctx, LOGE, FL("session does not exist for given bssId"));
			return eSIR_FAILURE;
		}

		report = qdf_mem_malloc(beacon_xmit_ind->numBssDesc *
			 sizeof(*report));

		if (NULL == report) {
			lim_log(mac_ctx, LOGE, FL("RRM Report is NULL, allocation failed"));
			return eSIR_MEM_ALLOC_FAILED;
		}

		qdf_mem_zero(report, beacon_xmit_ind->numBssDesc *
			sizeof(*report));

		for (bss_desc_count = 0; bss_desc_count <
		     beacon_xmit_ind->numBssDesc; bss_desc_count++) {
			beacon_report =
				&report[bss_desc_count].report.beaconReport;
			/*
			 * If the scan result is NULL then send report request
			 * with option subelement as NULL.
			 */
			bss_desc = beacon_xmit_ind->
				   pBssDescription[bss_desc_count];
			/* Prepare the beacon report and send it to the peer.*/
			report[bss_desc_count].token =
				beacon_xmit_ind->uDialogToken;
			report[bss_desc_count].refused = 0;
			report[bss_desc_count].incapable = 0;
			report[bss_desc_count].type = SIR_MAC_RRM_BEACON_TYPE;

			/*
			 * Valid response is included if the size of
			 * becon xmit is == size of beacon xmit ind + ies
			 */
			if (beacon_xmit_ind->length < sizeof(*beacon_xmit_ind))
				continue;
			beacon_report->regClass = beacon_xmit_ind->regClass;
			if (bss_desc) {
				beacon_report->channel = bss_desc->channelId;
				qdf_mem_copy(beacon_report->measStartTime,
					bss_desc->startTSF,
					sizeof(bss_desc->startTSF));
				beacon_report->measDuration =
					SYS_MS_TO_TU(beacon_xmit_ind->duration);
				beacon_report->phyType = bss_desc->nwType;
				beacon_report->bcnProbeRsp = 1;
				beacon_report->rsni = bss_desc->sinr;
				beacon_report->rcpi = bss_desc->rssi;
				beacon_report->antennaId = 0;
				beacon_report->parentTSF = bss_desc->parentTSF;
				qdf_mem_copy(beacon_report->bssid,
					bss_desc->bssId, sizeof(tSirMacAddr));
			}

			switch (curr_req->request.Beacon.reportingDetail) {
			case BEACON_REPORTING_DETAIL_NO_FF_IE:
				/* 0: No need to include any elements. */
				lim_log(mac_ctx, LOG3,
					FL("No reporting detail requested"));
				break;
			case BEACON_REPORTING_DETAIL_ALL_FF_REQ_IE:
				/* 1: Include all FFs and Requested Ies. */
				lim_log(mac_ctx, LOG3,
					FL("Only requested IEs in reporting detail requested"));

				if (bss_desc) {
					rrm_fill_beacon_ies(mac_ctx,
					    (uint8_t *) &beacon_report->Ies[0],
					    (uint8_t *) &beacon_report->numIes,
					    BEACON_REPORT_MAX_IES,
					    curr_req->request.Beacon.reqIes.
					    pElementIds,
					    curr_req->request.Beacon.reqIes.num,
					    bss_desc);
				}
				break;
			case BEACON_REPORTING_DETAIL_ALL_FF_IE:
				/* 2: default - Include all FFs and all Ies. */
			default:
				lim_log(mac_ctx, LOG3, FL("Default all IEs and FFs"));
				if (bss_desc) {
					rrm_fill_beacon_ies(mac_ctx,
					    (uint8_t *) &beacon_report->Ies[0],
					    (uint8_t *) &beacon_report->numIes,
					    BEACON_REPORT_MAX_IES,
					    NULL,
					    0,
					    bss_desc);
				}
				break;
			}
		}

		lim_log(mac_ctx, LOG1, FL("Sending Action frame with %d bss info"),
			bss_desc_count);
		lim_send_radio_measure_report_action_frame(mac_ctx,
			curr_req->dialog_token, bss_desc_count, report,
			beacon_xmit_ind->bssId, session_entry);

		curr_req->sendEmptyBcnRpt = false;
	}

	if (beacon_xmit_ind->fMeasureDone) {
		lim_log(mac_ctx, LOG3, FL("Measurement done....cleanup the context"));
		rrm_cleanup(mac_ctx);
	}

	if (NULL != report)
		qdf_mem_free(report);

	return status;
}

void rrm_process_beacon_request_failure(tpAniSirGlobal pMac,
					tpPESession pSessionEntry, tSirMacAddr peer,
					tRrmRetStatus status)
{
	tpSirMacRadioMeasureReport pReport = NULL;
	tpRRMReq pCurrentReq = pMac->rrm.rrmPEContext.pCurrentReq;

	pReport = qdf_mem_malloc(sizeof(tSirMacRadioMeasureReport));
	if (NULL == pReport) {
		lim_log(pMac, LOGP,
			FL
				("Unable to allocate memory during RRM Req processing"));
		return;
	}
	qdf_mem_set(pReport, sizeof(tSirMacRadioMeasureReport), 0);
	pReport->token = pCurrentReq->token;
	pReport->type = SIR_MAC_RRM_BEACON_TYPE;

	switch (status) {
	case eRRM_REFUSED:
		pReport->refused = 1;
		break;
	case eRRM_INCAPABLE:
		pReport->incapable = 1;
		break;
	default:
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       (" Beacon request processing failed no report sent with status %d "),
			       status);
		       );
		qdf_mem_free(pReport);
		return;
	}

	lim_send_radio_measure_report_action_frame(pMac, pCurrentReq->dialog_token, 1,
						   pReport, peer, pSessionEntry);

	qdf_mem_free(pReport);
	lim_log(pMac, LOG3, FL(" Free memory for pReport"));
	return;
}

/**
 * rrm_process_beacon_req() - Update curr_req and report
 * @mac_ctx: Global pointer to MAC context
 * @peer: Macaddress of the peer requesting the radio measurement
 * @session_entry: session entry
 * @curr_req: Pointer to RRM request
 * @report: Pointer to radio measurement report
 * @rrm_req: Array of Measurement request IEs
 * @num_report: No.of reports
 * @index: Index for Measurement request
 *
 * Update structure sRRMReq and sSirMacRadioMeasureReport and pass it to
 * rrm_process_beacon_report_req().
 *
 * Return: tSirRetStatus
 */
tSirRetStatus rrm_process_beacon_req(tpAniSirGlobal mac_ctx, tSirMacAddr peer,
			     tpPESession session_entry, tpRRMReq curr_req,
			     tpSirMacRadioMeasureReport report,
			     tDot11fRadioMeasurementRequest *rrm_req,
			     uint8_t *num_report, int index)
{
	tRrmRetStatus rrm_status = eRRM_SUCCESS;

	if (curr_req) {
		if (report == NULL) {
			/*
			 * Allocate memory to send reports for
			 * any subsequent requests.
			 */
			report = qdf_mem_malloc(sizeof(*report) *
				(rrm_req->num_MeasurementRequest - index));
			if (NULL == report) {
				lim_log(mac_ctx, LOGP,
					FL("Unable to allocate memory during RRM Req processing"));
				return eSIR_MEM_ALLOC_FAILED;
			}
			qdf_mem_set(report, sizeof(*report) *
				(rrm_req->num_MeasurementRequest - index), 0);
			lim_log(mac_ctx, LOG3,
				FL("rrm beacon type refused of %d report in beacon table"),
				*num_report);
		}
		report[*num_report].refused = 1;
		report[*num_report].type = SIR_MAC_RRM_BEACON_TYPE;
		report[*num_report].token =
			rrm_req->MeasurementRequest[index].measurement_token;
		(*num_report)++;
		return eSIR_SUCCESS;
	} else {
		curr_req = qdf_mem_malloc(sizeof(*curr_req));
		if (NULL == curr_req) {
			lim_log(mac_ctx, LOGP,
				FL("Unable to allocate memory during RRM Req processing"));
				qdf_mem_free(report);
			return eSIR_MEM_ALLOC_FAILED;
		}
		lim_log(mac_ctx, LOG3, FL(" Processing Beacon Report request"));
		qdf_mem_set(curr_req, sizeof(*curr_req), 0);
		curr_req->dialog_token = rrm_req->DialogToken.token;
		curr_req->token = rrm_req->
				  MeasurementRequest[index].measurement_token;
		curr_req->sendEmptyBcnRpt = true;
		mac_ctx->rrm.rrmPEContext.pCurrentReq = curr_req;
		rrm_status = rrm_process_beacon_report_req(mac_ctx, curr_req,
			&rrm_req->MeasurementRequest[index], session_entry);
		if (eRRM_SUCCESS != rrm_status) {
			rrm_process_beacon_request_failure(mac_ctx,
				session_entry, peer, rrm_status);
			rrm_cleanup(mac_ctx);
		}
	}
	return eSIR_SUCCESS;
}

/**
 * update_rrm_report() - Set incapable bit
 * @mac_ctx: Global pointer to MAC context
 * @report: Pointer to radio measurement report
 * @rrm_req: Array of Measurement request IEs
 * @num_report: No.of reports
 * @index: Index for Measurement request
 *
 * Send a report with incapabale bit set
 *
 * Return: tSirRetStatus
 */

tSirRetStatus update_rrm_report(tpAniSirGlobal mac_ctx,
				tpSirMacRadioMeasureReport report,
				tDot11fRadioMeasurementRequest *rrm_req,
				uint8_t *num_report, int index)
{
	if (report == NULL) {
		/*
		 * Allocate memory to send reports for
		 * any subsequent requests.
		 */
		report = qdf_mem_malloc(sizeof(*report) *
			 (rrm_req->num_MeasurementRequest - index));
		if (NULL == report) {
			lim_log(mac_ctx, LOGP, FL("Unable to allocate memory during RRM Req processing"));
			return eSIR_MEM_ALLOC_FAILED;
		}
		qdf_mem_set(report, sizeof(*report) *
			(rrm_req->num_MeasurementRequest - index), 0);
		lim_log(mac_ctx, LOG3, FL("rrm beacon type incapable of %d report "),
			*num_report);
	}
	report[*num_report].incapable = 1;
	report[*num_report].type =
		rrm_req->MeasurementRequest[index].measurement_type;
	report[*num_report].token =
		 rrm_req->MeasurementRequest[index].measurement_token;
	(*num_report)++;
	return eSIR_SUCCESS;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_process_radio_measurement_request - Process rrm request
 * @mac_ctx: Global pointer to MAC context
 * @peer: Macaddress of the peer requesting the radio measurement.
 * @rrm_req: Array of Measurement request IEs
 * @session_entry: session entry.
 *
 * Processes the Radio Resource Measurement request.
 *
 * Return: tSirRetStatus
 */
tSirRetStatus
rrm_process_radio_measurement_request(tpAniSirGlobal mac_ctx,
				      tSirMacAddr peer,
				      tDot11fRadioMeasurementRequest *rrm_req,
				      tpPESession session_entry)
{
	uint8_t i;
	tSirRetStatus status = eSIR_SUCCESS;
	tpSirMacRadioMeasureReport report = NULL;
	uint8_t num_report = 0;
	tpRRMReq curr_req = mac_ctx->rrm.rrmPEContext.pCurrentReq;

	if (!rrm_req->num_MeasurementRequest) {
		report = qdf_mem_malloc(sizeof(tSirMacRadioMeasureReport));
		if (NULL == report) {
			lim_log(mac_ctx, LOGP, FL("Unable to allocate memory during RRM Req processing"));
			return eSIR_MEM_ALLOC_FAILED;
		}
		qdf_mem_set(report, sizeof(tSirMacRadioMeasureReport), 0);
		lim_log(mac_ctx, LOGE, FL("No requestIes in the measurement request, sending incapable report"));
		report->incapable = 1;
		num_report = 1;
		lim_send_radio_measure_report_action_frame(mac_ctx,
			rrm_req->DialogToken.token, num_report, report, peer,
			session_entry);
		qdf_mem_free(report);
		return eSIR_FAILURE;
	}
	/* PF Fix */
	if (rrm_req->NumOfRepetitions.repetitions > 0) {
		lim_log(mac_ctx, LOG1, FL(" number of repetitions %d"),
			rrm_req->NumOfRepetitions.repetitions);
		/*
		 * Send a report with incapable bit set.
		 * Not supporting repetitions.
		 */
		report = qdf_mem_malloc(sizeof(tSirMacRadioMeasureReport));
		if (NULL == report) {
			lim_log(mac_ctx, LOGP, FL("Unable to allocate memory during RRM Req processing"));
			return eSIR_MEM_ALLOC_FAILED;
		}
		qdf_mem_set(report, sizeof(tSirMacRadioMeasureReport), 0);
		lim_log(mac_ctx, LOGE, FL("Allocated memory for report"));
		report->incapable = 1;
		report->type = rrm_req->MeasurementRequest[0].measurement_type;
		num_report = 1;
		goto end;
	}

	for (i = 0; i < rrm_req->num_MeasurementRequest; i++) {
		switch (rrm_req->MeasurementRequest[i].measurement_type) {
		case SIR_MAC_RRM_BEACON_TYPE:
			/* Process beacon request. */
			status = rrm_process_beacon_req(mac_ctx, peer,
				 session_entry, curr_req, report, rrm_req,
				 &num_report, i);
			if (eSIR_SUCCESS != status)
				return status;
			break;
		case SIR_MAC_RRM_LCI_TYPE:
		case SIR_MAC_RRM_LOCATION_CIVIC_TYPE:
		case SIR_MAC_RRM_FINE_TIME_MEAS_TYPE:
			lim_log(mac_ctx, LOG1,
			    FL("RRM with type: %d sent to userspace"),
			    rrm_req->MeasurementRequest[i].measurement_type);
			break;
		default:
			/* Send a report with incapabale bit set. */
			status = update_rrm_report(mac_ctx, report, rrm_req,
						   &num_report, i);
			if (eSIR_SUCCESS != status)
				return status;
			break;
		}
	}

end:
	if (report) {
		lim_send_radio_measure_report_action_frame(mac_ctx,
			rrm_req->DialogToken.token, num_report, report,
			peer, session_entry);
		qdf_mem_free(report);
		lim_log(mac_ctx, LOG3, FL(" Free memory for report"));
	}
	return status;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_update_start_tsf
 **
 * FUNCTION:  Store start TSF of measurement.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param startTSF - TSF value at the start of measurement.
 * @return None
 */
void rrm_update_start_tsf(tpAniSirGlobal pMac, uint32_t startTSF[2])
{
	pMac->rrm.rrmPEContext.startTSF[0] = startTSF[0];
	pMac->rrm.rrmPEContext.startTSF[1] = startTSF[1];
}

/* -------------------------------------------------------------------- */
/**
 * rrm_get_start_tsf
 *
 * FUNCTION:  Get the Start TSF.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param startTSF - store star TSF in this buffer.
 * @return txPower
 */
void rrm_get_start_tsf(tpAniSirGlobal pMac, uint32_t *pStartTSF)
{
	pStartTSF[0] = pMac->rrm.rrmPEContext.startTSF[0];
	pStartTSF[1] = pMac->rrm.rrmPEContext.startTSF[1];

}

/* -------------------------------------------------------------------- */
/**
 * rrm_get_capabilities
 *
 * FUNCTION:
 * Returns a pointer to tpRRMCaps with all the caps enabled in RRM
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pSessionEntry
 * @return pointer to tRRMCaps
 */
tpRRMCaps rrm_get_capabilities(tpAniSirGlobal pMac, tpPESession pSessionEntry)
{
	return &pMac->rrm.rrmPEContext.rrmEnabledCaps;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_initialize
 *
 * FUNCTION:
 * Initialize RRM module
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @return None
 */

tSirRetStatus rrm_initialize(tpAniSirGlobal pMac)
{
	tpRRMCaps pRRMCaps = &pMac->rrm.rrmPEContext.rrmEnabledCaps;

	pMac->rrm.rrmPEContext.pCurrentReq = NULL;
	pMac->rrm.rrmPEContext.txMgmtPower = 0;
	pMac->rrm.rrmPEContext.DialogToken = 0;

	pMac->rrm.rrmPEContext.rrmEnable = 0;

	qdf_mem_set(pRRMCaps, sizeof(tRRMCaps), 0);
	pRRMCaps->LinkMeasurement = 1;
	pRRMCaps->NeighborRpt = 1;
	pRRMCaps->BeaconPassive = 1;
	pRRMCaps->BeaconActive = 1;
	pRRMCaps->BeaconTable = 1;
	pRRMCaps->APChanReport = 1;
	pRRMCaps->fine_time_meas_rpt = 1;
	pRRMCaps->lci_capability = 1;

	pRRMCaps->operatingChanMax = 3;
	pRRMCaps->nonOperatingChanMax = 3;

	return eSIR_SUCCESS;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_cleanup
 *
 * FUNCTION:
 * cleanup RRM module
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param mode
 * @param rate
 * @return None
 */

tSirRetStatus rrm_cleanup(tpAniSirGlobal pMac)
{
	if (pMac->rrm.rrmPEContext.pCurrentReq) {
		if (pMac->rrm.rrmPEContext.pCurrentReq->request.Beacon.reqIes.
		    pElementIds) {
			qdf_mem_free(pMac->rrm.rrmPEContext.pCurrentReq->
				     request.Beacon.reqIes.pElementIds);
			lim_log(pMac, LOG4, FL(" Free memory for pElementIds"));
		}

		qdf_mem_free(pMac->rrm.rrmPEContext.pCurrentReq);
		lim_log(pMac, LOG4, FL(" Free memory for pCurrentReq"));
	}

	pMac->rrm.rrmPEContext.pCurrentReq = NULL;
	return eSIR_SUCCESS;
}

/* -------------------------------------------------------------------- */
/**
 * rrm_process_message
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

void rrm_process_message(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
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

}

/**
 * lim_update_rrm_capability() - Update PE context's rrm capability
 * @mac_ctx: Global pointer to MAC context
 * @join_req: Pointer to SME join request.
 *
 * Update PE context's rrm capability based on SME join request.
 *
 * Return: None
 */
void lim_update_rrm_capability(tpAniSirGlobal mac_ctx,
			       tpSirSmeJoinReq join_req)
{
	mac_ctx->rrm.rrmPEContext.rrmEnable = join_req->rrm_config.rrm_enabled;
	qdf_mem_copy(&mac_ctx->rrm.rrmPEContext.rrmEnabledCaps,
		     &join_req->rrm_config.rm_capability,
		     RMENABLEDCAP_MAX_LEN);

	return;
}
