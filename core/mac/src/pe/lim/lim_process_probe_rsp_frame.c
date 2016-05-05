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
 * This file lim_process_probe_rsp_frame.cc contains the code
 * for processing Probe Response Frame.
 * Author:        Chandra Modumudi
 * Date:          03/01/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "wni_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_send_messages.h"

#include "parser_api.h"

tSirRetStatus lim_validate_ie_information_in_probe_rsp_frame(uint8_t *pRxPacketInfo)
{
	tSirRetStatus status = eSIR_SUCCESS;

	if (WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo) <
	    (SIR_MAC_B_PR_SSID_OFFSET + SIR_MAC_MIN_IE_LEN)) {
		status = eSIR_FAILURE;
	}

	return status;
}

/**
 * lim_process_probe_rsp_frame() - processes received Probe Response frame
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_Packet_info: A pointer to Buffer descriptor + associated PDUs
 * @session_entry: Handle to the session.
 *
 * This function processes received Probe Response frame.
 * Frames with out-of-order IEs are dropped.
 * In case of IBSS, join 'success' makes MLM state machine
 * transition into 'BSS started' state. This may have to change
 * depending on supporting what kinda Authentication in IBSS.
 *
 * Return: None
 */
void
lim_process_probe_rsp_frame(tpAniSirGlobal mac_ctx, uint8_t *rx_Packet_info,
			    tpPESession session_entry)
{
	uint8_t *body;
	uint32_t frame_len = 0;
	tSirMacAddr current_bssid;
	tpSirMacMgmtHdr header;
	tSirProbeRespBeacon *probe_rsp;
	uint8_t qos_enabled = false;
	uint8_t wme_enabled = false;

	if (!session_entry) {
		lim_log(mac_ctx, LOGE, FL("session_entry is NULL"));
		return;
	}
	lim_log(mac_ctx, LOG1, "SessionId:%d ProbeRsp Frame is received",
		session_entry->peSessionId);

	probe_rsp = qdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == probe_rsp) {
		lim_log(mac_ctx, LOGE,
			FL
			("Unable to allocate memory "));
		return;
	}

	probe_rsp->ssId.length = 0;
	probe_rsp->wpa.length = 0;

	header = WMA_GET_RX_MAC_HEADER(rx_Packet_info);

	lim_log(mac_ctx, LOG2,
		FL("Rx Probe Response with length = %d from "MAC_ADDRESS_STR),
		WMA_GET_RX_MPDU_LEN(rx_Packet_info),
		MAC_ADDR_ARRAY(header->sa));

	/* Validate IE information before processing Probe Response Frame */
	if (lim_validate_ie_information_in_probe_rsp_frame(rx_Packet_info) !=
		eSIR_SUCCESS) {
		lim_log(mac_ctx, LOG1,
			FL("Parse error ProbeResponse, length=%d"), frame_len);
		qdf_mem_free(probe_rsp);
		return;
	}

	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_Packet_info);
	QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
		FL("Probe Resp Frame Received: BSSID "
		MAC_ADDRESS_STR " (RSSI %d)"),
		MAC_ADDR_ARRAY(header->bssId),
		(uint) abs((int8_t)WMA_GET_RX_RSSI_NORMALIZED(rx_Packet_info)));
	/* Get pointer to Probe Response frame body */
	body = WMA_GET_RX_MPDU_DATA(rx_Packet_info);
		/* Enforce Mandatory IEs */
	if ((sir_convert_probe_frame2_struct(mac_ctx,
		body, frame_len, probe_rsp) == eSIR_FAILURE) ||
		!probe_rsp->ssidPresent) {
		lim_log(mac_ctx, LOG1,
			FL("Parse error ProbeResponse, length=%d"), frame_len);
		qdf_mem_free(probe_rsp);
		return;
	}
	lim_check_and_add_bss_description(mac_ctx, probe_rsp,
			  rx_Packet_info, false, true);
	/* To Support BT-AMP */
	if ((mac_ctx->lim.gLimMlmState ==
			eLIM_MLM_WT_PROBE_RESP_STATE) ||
	    (mac_ctx->lim.gLimMlmState ==
			eLIM_MLM_PASSIVE_SCAN_STATE)) {
		lim_check_and_add_bss_description(mac_ctx, probe_rsp,
			rx_Packet_info, ((mac_ctx->lim.
			gLimHalScanState == eLIM_HAL_SCANNING_STATE)
			? true : false), true);
	} else if (session_entry->limMlmState ==
			eLIM_MLM_WT_JOIN_BEACON_STATE) {
		/*
		 * Either Beacon/probe response is required.
		 * Hence store it in same buffer.
		 */
		if (session_entry->beacon != NULL) {
			qdf_mem_free(session_entry->beacon);
			session_entry->beacon = NULL;
			session_entry->bcnLen = 0;
		}
		session_entry->bcnLen =
			WMA_GET_RX_PAYLOAD_LEN(rx_Packet_info);
			session_entry->beacon =
			qdf_mem_malloc(session_entry->bcnLen);
		if (NULL == session_entry->beacon) {
			lim_log(mac_ctx, LOGE,
				FL("No Memory to store beacon"));
		} else {
			/*
			 * Store the Beacon/ProbeRsp.
			 * This is sent to csr/hdd in join cnf response.
			 */
			qdf_mem_copy(session_entry->beacon,
				     WMA_GET_RX_MPDU_DATA
					     (rx_Packet_info),
				     session_entry->bcnLen);
		}
			/* STA in WT_JOIN_BEACON_STATE */
		lim_check_and_announce_join_success(mac_ctx, probe_rsp,
						header,
						session_entry);
	} else if (session_entry->limMlmState ==
		   eLIM_MLM_LINK_ESTABLISHED_STATE) {
		tpDphHashNode sta_ds = NULL;
		/*
		 * Check if this Probe Response is for
		 * our Probe Request sent upon reaching
		 * heart beat threshold
		 */
		sir_copy_mac_addr(current_bssid, session_entry->bssId);
		if (qdf_mem_cmp(current_bssid, header->bssId,
				sizeof(tSirMacAddr))) {
			qdf_mem_free(probe_rsp);
			return;
		}
		if (!LIM_IS_CONNECTION_ACTIVE(session_entry)) {
			lim_log(mac_ctx, LOGW,
				FL("Recved Probe Resp from AP,AP-alive"));
			if (probe_rsp->HTInfo.present)
				lim_received_hb_handler(mac_ctx,
					probe_rsp->HTInfo.primaryChannel,
					session_entry);
			else
				lim_received_hb_handler(mac_ctx,
					(uint8_t)probe_rsp->channelNumber,
					session_entry);
		}
		if (LIM_IS_STA_ROLE(session_entry)) {
			if (probe_rsp->channelSwitchPresent) {
				/*
				 * on receiving channel switch announcement
				 * from AP, delete all TDLS peers before
				 * leaving BSS and proceed for channel switch
				 */
				lim_delete_tdls_peers(mac_ctx, session_entry);

				lim_update_channel_switch(mac_ctx,
					probe_rsp,
					session_entry);
			} else if (session_entry->gLimSpecMgmt.dot11hChanSwState
				== eLIM_11H_CHANSW_RUNNING) {
				lim_cancel_dot11h_channel_switch(
					mac_ctx, session_entry);
			}
		}
		/*
		 * Now Process EDCA Parameters, if EDCAParamSet
		 * count is different.
		 * -- While processing beacons in link established
		 * state if it is determined that
		 * QoS Info IE has a different count for EDCA Params,
		 * and EDCA IE is not present in beacon,
		 * then probe req is sent out to get the EDCA params.
		 */
		sta_ds = dph_get_hash_entry(mac_ctx,
				DPH_STA_HASH_INDEX_PEER,
				&session_entry->dph.dphHashTable);
		limGetQosMode(session_entry, &qos_enabled);
		limGetWmeMode(session_entry, &wme_enabled);
		lim_log(mac_ctx, LOG2,
			FL("wmeEdcaPresent: %d wme_enabled: %d"),
			probe_rsp->wmeEdcaPresent, wme_enabled);
		lim_log(mac_ctx, LOG2,
			FL("edcaPresent: %d, qos_enabled: %d"),
			probe_rsp->edcaPresent, qos_enabled);
		lim_log(mac_ctx, LOG2,
			FL("edcaParams.qosInfo.count: %d"),
			probe_rsp->edcaParams.qosInfo.count);
		lim_log(mac_ctx, LOG2,
			FL("schObject.gLimEdcaParamSetCount: %d"),
			session_entry->gLimEdcaParamSetCount);
		if (((probe_rsp->wmeEdcaPresent && wme_enabled) ||
		     (probe_rsp->edcaPresent && qos_enabled)) &&
		    (probe_rsp->edcaParams.qosInfo.count !=
		     session_entry->gLimEdcaParamSetCount)) {
			if (sch_beacon_edca_process(mac_ctx,
				&probe_rsp->edcaParams,
				session_entry) != eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("EDCA param process error"));
			} else if (sta_ds != NULL) {
				/*
				 * If needed, downgrade the
				 * EDCA parameters
				 */
				lim_set_active_edca_params(mac_ctx,
						session_entry->
						gLimEdcaParams,
						session_entry);
					lim_send_edca_params(mac_ctx,
					session_entry->
					gLimEdcaParamsActive,
					sta_ds->bssId);
			} else {
				lim_log(mac_ctx, LOGE,
					FL("SelfEntry missing in Hash"));
			}
		}
		if (session_entry->fWaitForProbeRsp == true) {
			lim_log(mac_ctx, LOGW,
				FL("Check probe resp for caps change"));
			lim_detect_change_in_ap_capabilities(
				mac_ctx, probe_rsp, session_entry);
		}
	} else {
		if (LIM_IS_IBSS_ROLE(session_entry) &&
		    (session_entry->limMlmState ==
				eLIM_MLM_BSS_STARTED_STATE))
			lim_handle_ibss_coalescing(mac_ctx, probe_rsp,
					rx_Packet_info, session_entry);
	}
	qdf_mem_free(probe_rsp);

	/* Ignore Probe Response frame in all other states */
	return;
}

/**
 * lim_process_probe_rsp_frame_no_session() - process Probe Response frame
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_packet_info: A pointer to Buffer descriptor + associated PDUs
 *
 * This function processes received Probe Response frame with no session.
 *
 * Return: None
 */
void
lim_process_probe_rsp_frame_no_session(tpAniSirGlobal mac_ctx,
						uint8_t *rx_packet_info)
{
	uint8_t *body;
	uint32_t frame_len = 0;
	tpSirMacMgmtHdr header;
	tSirProbeRespBeacon *probe_rsp;

	probe_rsp = qdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == probe_rsp) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory"));
		return;
	}

	probe_rsp->ssId.length = 0;
	probe_rsp->wpa.length = 0;

	header = WMA_GET_RX_MAC_HEADER(rx_packet_info);

	lim_log(mac_ctx, LOG2,
		FL("Received Probe Response frame with length=%d from "),
		WMA_GET_RX_MPDU_LEN(rx_packet_info));
	lim_print_mac_addr(mac_ctx, header->sa, LOG2);

	/* Validate IE information before processing Probe Response Frame */
	if (lim_validate_ie_information_in_probe_rsp_frame(rx_packet_info) !=
								eSIR_SUCCESS) {
		lim_log(mac_ctx, LOG1,
			FL("Parse error ProbeResponse, length=%d"), frame_len);
		qdf_mem_free(probe_rsp);
		return;
	}

	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_packet_info);
	lim_log(mac_ctx, LOG2,
		  FL("Probe Resp Frame Received: BSSID "
		  MAC_ADDRESS_STR " (RSSI %d)"),
		  MAC_ADDR_ARRAY(header->bssId),
		  (uint) abs((int8_t)WMA_GET_RX_RSSI_NORMALIZED(
					rx_packet_info)));
	/*
	 * Get pointer to Probe Response frame body
	 */
	body = WMA_GET_RX_MPDU_DATA(rx_packet_info);
	if (sir_convert_probe_frame2_struct(mac_ctx, body, frame_len,
		probe_rsp) == eSIR_FAILURE) {
		lim_log(mac_ctx, LOG1,
			FL("Parse error ProbeResponse, length=%d\n"),
			frame_len);
		qdf_mem_free(probe_rsp);
		return;
	}
	lim_log(mac_ctx, LOG2, FL("Save this probe rsp in LFR cache"));
	lim_check_and_add_bss_description(mac_ctx, probe_rsp,
		  rx_packet_info, false, true);
	qdf_mem_free(probe_rsp);
	return;
}
