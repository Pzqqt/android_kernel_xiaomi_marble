/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 *
 * This file sch_api.cc contains functions related to the API exposed
 * by scheduler module
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "ani_global.h"
#include "wni_cfg.h"

#include "sir_mac_prot_def.h"
#include "sir_mac_prop_exts.h"
#include "sir_common.h"

#include "cfg_api.h"

#include "lim_api.h"

#include "sch_api.h"
#include "sch_debug.h"

#include "sch_sys_params.h"
#include "lim_trace.h"
#include "lim_types.h"
#include "lim_utils.h"

#include "wma_types.h"

/* -------------------------------------------------------------------- */
/* */
/*                          Static Variables */
/* */
/* ------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/**
 * sch_get_cfp_count
 *
 * FUNCTION:
 * Function used by other Sirius modules to read CFPcount
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

uint8_t sch_get_cfp_count(tpAniSirGlobal pMac)
{
	return pMac->sch.schObject.gSchCFPCount;
}

/* -------------------------------------------------------------------- */
/**
 * sch_get_cfp_dur_remaining
 *
 * FUNCTION:
 * Function used by other Sirius modules to read CFPDuration remaining
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

uint16_t sch_get_cfp_dur_remaining(tpAniSirGlobal pMac)
{
	return pMac->sch.schObject.gSchCFPDurRemaining;
}

/* -------------------------------------------------------------------- */
/**
 * sch_init_globals
 *
 * FUNCTION:
 * Initialize globals
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

void sch_init_globals(tpAniSirGlobal pMac)
{
	pMac->sch.gSchHcfEnabled = false;

	pMac->sch.gSchScanRequested = false;
	pMac->sch.gSchScanReqRcvd = false;

	pMac->sch.gSchGenBeacon = 1;
	pMac->sch.gSchBeaconsSent = 0;
	pMac->sch.gSchBeaconsWritten = 0;
	pMac->sch.gSchBcnParseErrorCnt = 0;
	pMac->sch.gSchBcnIgnored = 0;
	pMac->sch.gSchBBXportRcvCnt = 0;
	pMac->sch.gSchUnknownRcvCnt = 0;
	pMac->sch.gSchBcnRcvCnt = 0;
	pMac->sch.gSchRRRcvCnt = 0;
	pMac->sch.qosNullCnt = 0;
	pMac->sch.numData = 0;
	pMac->sch.numPoll = 0;
	pMac->sch.numCorrupt = 0;
	pMac->sch.numBogusInt = 0;
	pMac->sch.numTxAct0 = 0;
	pMac->sch.rrTimeout = SCH_RR_TIMEOUT;
	pMac->sch.pollPeriod = SCH_POLL_PERIOD;
	pMac->sch.multipleSched = 1;
	pMac->sch.maxPollTimeouts = 20;
	pMac->sch.checkCfbFlagStuck = 0;
}

/* -------------------------------------------------------------------- */
/**
 * sch_post_message
 *
 * FUNCTION:
 * Post the beacon message to the scheduler message queue
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMsg pointer to message
 * @return None
 */

tSirRetStatus sch_post_message(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
	sch_process_message(pMac, pMsg);

	return eSIR_SUCCESS;
}

/* --------------------------------------------------------------------------- */
/**
 * sch_send_start_scan_rsp
 *
 * FUNCTION:
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

void sch_send_start_scan_rsp(tpAniSirGlobal pMac)
{
	tSirMsgQ msgQ;
	uint32_t retCode;

	PELOG1(sch_log(pMac, LOG1, FL("Sending LIM message to go into scan"));)
	msgQ.type = SIR_SCH_START_SCAN_RSP;
	retCode = lim_post_msg_api(pMac, &msgQ);
	if (retCode != eSIR_SUCCESS)
		sch_log(pMac, LOGE,
			FL("Posting START_SCAN_RSP to LIM failed, reason=%X"),
			retCode);
}

/**
 * sch_send_beacon_req
 *
 * FUNCTION:
 *
 * LOGIC:
 * 1) SCH received SIR_SCH_BEACON_GEN_IND
 * 2) SCH updates TIM IE and other beacon related IE's
 * 3) SCH sends WMA_SEND_BEACON_REQ to HAL. HAL then copies the beacon
 *    template to memory
 *
 * ASSUMPTIONS:
 * Memory allocation is reqd to send this message and SCH allocates memory.
 * The assumption is that HAL will "free" this memory.
 *
 * NOTE:
 *
 * @param pMac global
 *
 * @param beaconPayload
 *
 * @param size - Length of the beacon
 *
 * @return QDF_STATUS
 */
tSirRetStatus sch_send_beacon_req(tpAniSirGlobal pMac, uint8_t *beaconPayload,
				  uint16_t size, tpPESession psessionEntry)
{
	tSirMsgQ msgQ;
	tpSendbeaconParams beaconParams = NULL;
	tSirRetStatus retCode;

	sch_log(pMac, LOG2,
		FL
			("Indicating HAL to copy the beacon template [%d bytes] to memory"),
		size);

	beaconParams = qdf_mem_malloc(sizeof(tSendbeaconParams));
	if (NULL == beaconParams)
		return eSIR_MEM_ALLOC_FAILED;

	msgQ.type = WMA_SEND_BEACON_REQ;

	/* No Dialog Token reqd, as a response is not solicited */
	msgQ.reserved = 0;

	/* Fill in tSendbeaconParams members */
	qdf_mem_copy(beaconParams->bssId, psessionEntry->bssId,
		     sizeof(psessionEntry->bssId));

	if (LIM_IS_IBSS_ROLE(psessionEntry)) {
		beaconParams->timIeOffset = 0;
	} else {
		beaconParams->timIeOffset = psessionEntry->schBeaconOffsetBegin;
	}

	/* p2pIeOffset should be atleast greater than timIeOffset */
	if ((pMac->sch.schObject.p2pIeOffset != 0) &&
	    (pMac->sch.schObject.p2pIeOffset <
	     psessionEntry->schBeaconOffsetBegin)) {
		sch_log(pMac, LOGE, FL("Invalid p2pIeOffset:[%d]"),
			pMac->sch.schObject.p2pIeOffset);
		QDF_ASSERT(0);
		qdf_mem_free(beaconParams);
		return eSIR_FAILURE;
	}
	beaconParams->p2pIeOffset = pMac->sch.schObject.p2pIeOffset;
#ifdef WLAN_SOFTAP_FW_BEACON_TX_PRNT_LOG
	sch_log(pMac, LOGE, FL("TimIeOffset:[%d]"), beaconParams->TimIeOffset);
#endif

	beaconParams->beacon = beaconPayload;
	beaconParams->beaconLength = (uint32_t) size;
	msgQ.bodyptr = beaconParams;
	msgQ.bodyval = 0;

	/* Keep a copy of recent beacon frame sent */

	/* free previous copy of the beacon */
	if (psessionEntry->beacon) {
		qdf_mem_free(psessionEntry->beacon);
	}

	psessionEntry->bcnLen = 0;
	psessionEntry->beacon = NULL;

	psessionEntry->beacon = qdf_mem_malloc(size);
	if (psessionEntry->beacon != NULL) {
		qdf_mem_copy(psessionEntry->beacon, beaconPayload, size);
		psessionEntry->bcnLen = size;
	}

	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		sch_log(pMac, LOGE,
			FL("Posting SEND_BEACON_REQ to HAL failed, reason=%X"),
			retCode);
	} else {
		sch_log(pMac, LOG2,
			FL("Successfully posted WMA_SEND_BEACON_REQ to HAL"));

		if (LIM_IS_AP_ROLE(psessionEntry) &&
		   (pMac->sch.schObject.fBeaconChanged)) {
			retCode = lim_send_probe_rsp_template_to_hal(pMac,
								     psessionEntry,
								     &psessionEntry->
								     DefProbeRspIeBitmap
								     [0]);
			if (eSIR_SUCCESS != retCode) {
				/* check whether we have to free any memory */
				sch_log(pMac, LOGE,
					FL
						("FAILED to send probe response template with retCode %d"),
					retCode);
			}
		}
	}

	return retCode;
}

uint32_t lim_remove_p2p_ie_from_add_ie(tpAniSirGlobal pMac,
					tpPESession psessionEntry,
					uint8_t *addIeWoP2pIe,
					uint32_t *addnIELenWoP2pIe)
{
	uint32_t left = psessionEntry->addIeParams.probeRespDataLen;
	uint8_t *ptr = psessionEntry->addIeParams.probeRespData_buff;
	uint8_t elem_id, elem_len;
	uint32_t offset = 0;
	uint8_t eid = 0xDD;

	qdf_mem_copy(addIeWoP2pIe, ptr, left);
	*addnIELenWoP2pIe = left;

	if (addIeWoP2pIe != NULL) {
		while (left >= 2) {
			elem_id  = ptr[0];
			elem_len = ptr[1];
			left -= 2;
			if (elem_len > left) {
				sch_log(pMac, LOGE, FL("Invalid IEs"));
				return eSIR_FAILURE;
			}
			if ((elem_id == eid) &&
				(!qdf_mem_cmp(&ptr[2],
					"\x50\x6f\x9a\x09", 4))) {
				left -= elem_len;
				ptr += (elem_len + 2);
				qdf_mem_copy(&addIeWoP2pIe[offset], ptr, left);
				*addnIELenWoP2pIe -= (2 + elem_len);
			} else {
				left -= elem_len;
				ptr += (elem_len + 2);
				offset += 2 + elem_len;
			}
		}
	}
	return eSIR_SUCCESS;
}

uint32_t lim_send_probe_rsp_template_to_hal(tpAniSirGlobal pMac,
					    tpPESession psessionEntry,
					    uint32_t *IeBitmap)
{
	tSirMsgQ msgQ;
	uint8_t *pFrame2Hal = psessionEntry->pSchProbeRspTemplate;
	tpSendProbeRespParams pprobeRespParams = NULL;
	uint32_t retCode = eSIR_FAILURE;
	uint32_t nPayload, nBytes, nStatus;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t addnIEPresent = false;
	uint8_t *addIE = NULL;
	uint8_t *addIeWoP2pIe = NULL;
	uint32_t addnIELenWoP2pIe = 0;
	uint32_t retStatus;
	tDot11fIEExtCap extracted_extcap;
	bool extcap_present = false;
	tDot11fProbeResponse *prb_rsp_frm;
	tSirRetStatus status;
	uint16_t addn_ielen = 0;

	nStatus = dot11f_get_packed_probe_response_size(pMac,
			&psessionEntry->probeRespFrame, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		sch_log(pMac, LOGE, FL("Failed to calculate the packed size f"
				       "or a Probe Response (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeResponse);
	} else if (DOT11F_WARNED(nStatus)) {
		sch_log(pMac, LOGE, FL("There were warnings while calculating"
				       "the packed size for a Probe Response "
				       "(0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	/* Check if probe response IE is present or not */
	addnIEPresent = (psessionEntry->addIeParams.probeRespDataLen != 0);
	if (addnIEPresent) {
		/*
		* probe response template should not have P2P IE.
		* In case probe request has P2P IE or WPS IE, the
		* probe request will be forwarded to the Host and
		* Host will send the probe response. In other cases
		* FW will send the probe response. So, if the template
		* has P2P IE, the probe response sent to non P2P devices
		* by the FW, may also have P2P IE which will fail
		* P2P cert case 6.1.3
		*/
		addIeWoP2pIe = qdf_mem_malloc(psessionEntry->addIeParams.
						probeRespDataLen);
		if (NULL == addIeWoP2pIe) {
			sch_log(pMac, LOGE,
				FL("FAILED to alloc memory when removing P2P IE"));
			return eSIR_MEM_ALLOC_FAILED;
		}

		retStatus = lim_remove_p2p_ie_from_add_ie(pMac, psessionEntry,
					addIeWoP2pIe, &addnIELenWoP2pIe);
		if (retStatus != eSIR_SUCCESS) {
			qdf_mem_free(addIeWoP2pIe);
			return eSIR_FAILURE;
		}

		/* Probe rsp IE available */
		/*need to check the data length */
		addIE = qdf_mem_malloc(addnIELenWoP2pIe);
		if (NULL == addIE) {
			sch_log(pMac, LOGE,
				FL("Unable to get WNI_CFG_PROBE_RSP_ADDNIE_DATA1 length"));
			qdf_mem_free(addIeWoP2pIe);
			return eSIR_MEM_ALLOC_FAILED;
		}
		addn_ielen = addnIELenWoP2pIe;

		if (addn_ielen <= WNI_CFG_PROBE_RSP_ADDNIE_DATA1_LEN &&
		    addn_ielen && (nBytes + addn_ielen) <= SIR_MAX_PACKET_SIZE)
			qdf_mem_copy(addIE, addIeWoP2pIe, addnIELenWoP2pIe);

		qdf_mem_free(addIeWoP2pIe);

		qdf_mem_zero((uint8_t *)&extracted_extcap,
			     sizeof(tDot11fIEExtCap));
		status = lim_strip_extcap_update_struct(pMac, addIE,
				&addn_ielen, &extracted_extcap);
		if (eSIR_SUCCESS != status) {
			sch_log(pMac, LOG1, FL("extcap not extracted"));
		} else {
			extcap_present = true;
		}
	}

	if (addnIEPresent) {
		if ((nBytes + addn_ielen) <= SIR_MAX_PACKET_SIZE)
			nBytes += addn_ielen;
		else
			addnIEPresent = false;  /* Dont include the IE. */
	}
	/* Paranoia: */
	qdf_mem_set(pFrame2Hal, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame2Hal, SIR_MAC_MGMT_FRAME,
					     SIR_MAC_MGMT_PROBE_RSP,
					     psessionEntry->selfMacAddr,
					     psessionEntry->selfMacAddr);

	pMacHdr = (tpSirMacMgmtHdr) pFrame2Hal;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

	/* merge extcap IE */
	prb_rsp_frm = &psessionEntry->probeRespFrame;
	if (extcap_present)
		lim_merge_extcap_struct(&prb_rsp_frm->ExtCap,
					&extracted_extcap);

	/* That done, pack the Probe Response: */
	nStatus =
		dot11f_pack_probe_response(pMac, &psessionEntry->probeRespFrame,
					   pFrame2Hal + sizeof(tSirMacMgmtHdr),
					   nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		sch_log(pMac, LOGE,
			FL("Failed to pack a Probe Response (0x%08x)."),
			nStatus);

		qdf_mem_free(addIE);
		return retCode; /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		sch_log(pMac, LOGE, FL("There were warnings while packing a P"
				       "robe Response (0x%08x)."), nStatus);
	}

	if (addnIEPresent) {
		qdf_mem_copy(&pFrame2Hal[nBytes - addn_ielen],
			     &addIE[0], addn_ielen);
	}

	/* free the allocated Memory */
	qdf_mem_free(addIE);

	pprobeRespParams = qdf_mem_malloc(sizeof(tSendProbeRespParams));
	if (NULL == pprobeRespParams) {
		sch_log(pMac, LOGE,
			FL
				("lim_send_probe_rsp_template_to_hal: HAL probe response params malloc failed for bytes %d"),
			nBytes);
	} else {
		sir_copy_mac_addr(pprobeRespParams->bssId, psessionEntry->bssId);
		pprobeRespParams->pProbeRespTemplate = pFrame2Hal;
		pprobeRespParams->probeRespTemplateLen = nBytes;
		qdf_mem_copy(pprobeRespParams->ucProxyProbeReqValidIEBmap,
			     IeBitmap, (sizeof(uint32_t) * 8));
		msgQ.type = WMA_SEND_PROBE_RSP_TMPL;
		msgQ.reserved = 0;
		msgQ.bodyptr = pprobeRespParams;
		msgQ.bodyval = 0;

		retCode = wma_post_ctrl_msg(pMac, &msgQ);
		if (eSIR_SUCCESS != retCode) {
			/* free the allocated Memory */
			sch_log(pMac, LOGE,
				FL
					("lim_send_probe_rsp_template_to_hal: FAIL bytes %d retcode[%X]"),
				nBytes, retCode);
			qdf_mem_free(pprobeRespParams);
		} else {
			sch_log(pMac, LOG1,
				FL
					("lim_send_probe_rsp_template_to_hal: Probe response template msg posted to HAL of bytes %d"),
				nBytes);
		}
	}

	return retCode;
}

/**
 * sch_gen_timing_advert_frame() - Generate the TA frame and populate the buffer
 * @pMac: the global MAC context
 * @self_addr: the self MAC address
 * @buf: the buffer that will contain the frame
 * @timestamp_offset: return for the offset of the timestamp field
 * @time_value_offset: return for the time_value field in the TA IE
 *
 * Return: the length of the buffer.
 */
int sch_gen_timing_advert_frame(tpAniSirGlobal mac_ctx, tSirMacAddr self_addr,
	uint8_t **buf, uint32_t *timestamp_offset, uint32_t *time_value_offset)
{
	tDot11fTimingAdvertisementFrame frame;
	uint32_t payload_size, buf_size;
	int status;
	struct qdf_mac_addr wildcard_bssid = {
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	};

	qdf_mem_zero((uint8_t *)&frame, sizeof(tDot11fTimingAdvertisementFrame));

	/* Populate the TA fields */
	status = populate_dot11f_timing_advert_frame(mac_ctx, &frame);
	if (status) {
		sch_log(mac_ctx, LOGE, FL("Error populating TA frame %x"),
			status);
		return status;
	}

	status = dot11f_get_packed_timing_advertisement_frame_size(mac_ctx,
		&frame, &payload_size);
	if (DOT11F_FAILED(status)) {
		sch_log(mac_ctx, LOGE, FL("Error getting packed frame size %x"),
			status);
		return status;
	} else if (DOT11F_WARNED(status)) {
		sch_log(mac_ctx, LOGW, FL("Warning getting packed frame size"));
	}

	buf_size = sizeof(tSirMacMgmtHdr) + payload_size;
	*buf = qdf_mem_malloc(buf_size);
	if (*buf == NULL) {
		sch_log(mac_ctx, LOGE, FL("Cannot allocate memory"));
		return eSIR_FAILURE;
	}
	qdf_mem_zero(*buf, buf_size);

	payload_size = 0;
	status = dot11f_pack_timing_advertisement_frame(mac_ctx, &frame,
		*buf + sizeof(tSirMacMgmtHdr), buf_size -
		sizeof(tSirMacMgmtHdr), &payload_size);
	sch_log(mac_ctx, LOGE, FL("TA payload size2 = %d"), payload_size);
	if (DOT11F_FAILED(status)) {
		sch_log(mac_ctx, LOGE, FL("Error packing frame %x"), status);
		goto fail;
	} else if (DOT11F_WARNED(status)) {
		sch_log(mac_ctx, LOGE, FL("Warning packing frame"));
	}

	lim_populate_mac_header(mac_ctx, *buf, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_TIME_ADVERT, wildcard_bssid.bytes, self_addr);

	/* The timestamp field is right after the header */
	*timestamp_offset = sizeof(tSirMacMgmtHdr);

	*time_value_offset = sizeof(tSirMacMgmtHdr) +
		sizeof(tDot11fFfTimeStamp) + sizeof(tDot11fFfCapabilities);

	/* Add the Country IE length */
	dot11f_get_packed_ie_country(mac_ctx, &frame.Country,
		time_value_offset);
	/* Add 2 for Country IE EID and Length fields */
	*time_value_offset += 2;

	/* Add the PowerConstraint IE size */
	if (frame.Country.present == 1)
		*time_value_offset += 3;

	/* Add the offset inside TA IE */
	*time_value_offset += 3;

	return payload_size + sizeof(tSirMacMgmtHdr);

fail:
	if (*buf)
		qdf_mem_free(*buf);
	return status;
}
