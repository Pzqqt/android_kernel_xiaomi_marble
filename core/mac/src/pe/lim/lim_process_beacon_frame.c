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
 * This file lim_process_beacon_frame.cc contains the code
 * for processing Received Beacon Frame.
 * Author:        Chandra Modumudi
 * Date:          03/01/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_ser_des_utils.h"

/**
 * lim_process_beacon_frame() - to process beacon frames
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_pkt_info: A pointer to RX packet info structure
 * @session: A pointer to session
 *
 * This function is called by limProcessMessageQueue() upon Beacon
 * frame reception.
 * Note:
 * 1. Beacons received in 'normal' state in IBSS are handled by
 *    Beacon Processing module.
 *
 * Return: none
 */

void
lim_process_beacon_frame(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
			 tpPESession session)
{
	tpSirMacMgmtHdr mac_hdr;
	tSchBeaconStruct *bcn_ptr;

	mac_ctx->lim.gLimNumBeaconsRcvd++;

	/*
	 * here is it required to increment session specific heartBeat
	 * beacon counter
	 */
	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	lim_log(mac_ctx, LOG2,
		FL("Received Beacon frame with length=%d from "),
		WMA_GET_RX_MPDU_LEN(rx_pkt_info));
		lim_print_mac_addr(mac_ctx, mac_hdr->sa, LOG2);

	/* Expect Beacon in any state as Scan is independent of LIM state */
	bcn_ptr = qdf_mem_malloc(sizeof(*bcn_ptr));
	if (NULL == bcn_ptr) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory"));
		return;
	}
	/* Parse received Beacon */
	if (sir_convert_beacon_frame2_struct(mac_ctx,
			rx_pkt_info, bcn_ptr) !=
			eSIR_SUCCESS) {
		/*
		 * Received wrongly formatted/invalid Beacon.
		 * Ignore it and move on.
		 */
		lim_log(mac_ctx, LOGW,
			FL("Received invalid Beacon in state %X"),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGW,
			session->limMlmState);
		qdf_mem_free(bcn_ptr);
		return;
	}

	if (bcn_ptr->assoc_disallowed) {
		lim_log(mac_ctx, LOG1,
				FL("Association disallowed in AP "MAC_ADDRESS_STR " Reason code %d"),
				MAC_ADDR_ARRAY(mac_hdr->sa),
				bcn_ptr->assoc_disallowed_reason);
		qdf_mem_free(bcn_ptr);
		return;
	}

	/*
	 * during scanning, when any session is active, and
	 * beacon/Pr belongs to one of the session, fill up the
	 * following, TBD - HB couter
	 */
	if (sir_compare_mac_addr(session->bssId,
				bcn_ptr->bssid)) {
		qdf_mem_copy((uint8_t *)&session->lastBeaconTimeStamp,
			(uint8_t *) bcn_ptr->timeStamp,
			sizeof(uint64_t));
		session->lastBeaconDtimCount =
				bcn_ptr->tim.dtimCount;
		session->currentBssBeaconCnt++;
	}
	MTRACE(mac_trace(mac_ctx,
		TRACE_CODE_RX_MGMT_TSF, 0, bcn_ptr->timeStamp[0]);)
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_RX_MGMT_TSF, 0,
		bcn_ptr->timeStamp[1]);)
	lim_check_and_add_bss_description(mac_ctx, bcn_ptr,
				rx_pkt_info, false, true);

	if ((mac_ctx->lim.gLimMlmState ==
				eLIM_MLM_WT_PROBE_RESP_STATE) ||
		(mac_ctx->lim.gLimMlmState ==
				eLIM_MLM_PASSIVE_SCAN_STATE)) {
		/* If we are scanning for P2P, only accept probe rsp */
		if ((mac_ctx->lim.gLimHalScanState !=
		    eLIM_HAL_SCANNING_STATE) ||
		    (NULL == mac_ctx->lim.gpLimMlmScanReq) ||
		    !mac_ctx->lim.gpLimMlmScanReq->p2pSearch)
			lim_check_and_add_bss_description(mac_ctx, bcn_ptr,
				rx_pkt_info,
				((mac_ctx->lim.gLimHalScanState ==
				 eLIM_HAL_SCANNING_STATE) ? true : false),
				false);
		/*
		 * Calling dfsChannelList which will convert DFS channel
		 * to active channel for x secs if this channel is DFS
		 */
		lim_set_dfs_channel_list(mac_ctx,
			bcn_ptr->channelNumber,
			&mac_ctx->lim.dfschannelList);
	} else if (session->limMlmState ==
			eLIM_MLM_WT_JOIN_BEACON_STATE) {
		if (session->beacon != NULL) {
			qdf_mem_free(session->beacon);
			session->beacon = NULL;
			session->bcnLen = 0;
		}
		session->bcnLen = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
		session->beacon = qdf_mem_malloc(session->bcnLen);
		if (NULL == session->beacon) {
			lim_log(mac_ctx, LOGE,
				FL("fail to alloc mem to store bcn"));
		} else {
			/*
			 * Store the Beacon/ProbeRsp. This is sent to
			 * csr/hdd in join cnf response.
			 */
			qdf_mem_copy(session->beacon,
				WMA_GET_RX_MPDU_DATA(rx_pkt_info),
				session->bcnLen);
		}
		lim_check_and_announce_join_success(mac_ctx, bcn_ptr,
				mac_hdr, session);
	}
	qdf_mem_free(bcn_ptr);
	return;
}

/**---------------------------------------------------------------
   \fn     lim_process_beacon_frame_no_session
   \brief  This function is called by limProcessMessageQueue()
 \       upon Beacon reception.
 \
   \param pMac
   \param *pRxPacketInfo    - A pointer to Rx packet info structure
   \return None
   ------------------------------------------------------------------*/
void
lim_process_beacon_frame_no_session(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo)
{
	tpSirMacMgmtHdr pHdr;
	tSchBeaconStruct *pBeacon;

	pMac->lim.gLimNumBeaconsRcvd++;
	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);

	lim_log(pMac, LOG2, FL("Received Beacon frame with length=%d from "),
		WMA_GET_RX_MPDU_LEN(pRxPacketInfo));
	lim_print_mac_addr(pMac, pHdr->sa, LOG2);


	/**
	 * No session has been established. Expect Beacon only when
	 * 1. STA is in Scan mode waiting for Beacon/Probe response or
	 * 2. STA/AP is in Learn mode
	 */
	if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
	    (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||
	    (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)) {
		pBeacon = qdf_mem_malloc(sizeof(tSchBeaconStruct));
		if (NULL == pBeacon) {
			lim_log(pMac, LOGE,
				FL
					("Unable to allocate memory in lim_process_beacon_frame_no_session"));
			return;
		}

		if (sir_convert_beacon_frame2_struct
			    (pMac, (uint8_t *) pRxPacketInfo,
			    pBeacon) != eSIR_SUCCESS) {
			/* Received wrongly formatted/invalid Beacon. Ignore and move on. */
			lim_log(pMac, LOGW,
				FL
					("Received invalid Beacon in global MLM state %X"),
				pMac->lim.gLimMlmState);
			lim_print_mlm_state(pMac, LOGW, pMac->lim.gLimMlmState);
			qdf_mem_free(pBeacon);
			return;
		}

		if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
		    (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE)) {
			/*If we are scanning for P2P, only accept probe rsp */
			if ((pMac->lim.gLimHalScanState !=
			    eLIM_HAL_SCANNING_STATE) ||
			    (NULL == pMac->lim.gpLimMlmScanReq) ||
			    !pMac->lim.gpLimMlmScanReq->p2pSearch)
				lim_check_and_add_bss_description(pMac, pBeacon,
					pRxPacketInfo, true, false);
			/* Calling dfsChannelList which will convert DFS channel
			 * to Active channel for x secs if this channel is DFS channel */
			lim_set_dfs_channel_list(pMac, pBeacon->channelNumber,
						 &pMac->lim.dfschannelList);
		} else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE) {
		} /* end of eLIM_MLM_LEARN_STATE) */
		qdf_mem_free(pBeacon);
	} /* end of (eLIM_MLM_WT_PROBE_RESP_STATE) || (eLIM_MLM_PASSIVE_SCAN_STATE) */
	else {
		lim_log(pMac, LOG1, FL("Rcvd Beacon in unexpected MLM state %s (%d)"),
			lim_mlm_state_str(pMac->lim.gLimMlmState),
			pMac->lim.gLimMlmState);
#ifdef WLAN_DEBUG
		pMac->lim.gLimUnexpBcnCnt++;
#endif
	}

	return;
} /*** end lim_process_beacon_frame_no_session() ***/
