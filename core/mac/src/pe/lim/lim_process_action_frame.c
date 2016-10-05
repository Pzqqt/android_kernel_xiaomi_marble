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

/*
 * This file lim_process_action_frame.cc contains the code
 * for processing Action Frame.
 * Author:      Michael Lui
 * Date:        05/23/03
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "wni_api.h"
#include "sir_api.h"
#include "ani_global.h"
#include "wni_cfg.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_send_sme_rsp_messages.h"
#include "parser_api.h"
#include "lim_admit_control.h"
#include "wmm_apsd.h"
#include "lim_send_messages.h"
#include "rrm_api.h"
#include "lim_session_utils.h"
#include "cds_concurrency.h"
#include "wma_types.h"
#include "wma.h"

#define BA_DEFAULT_TX_BUFFER_SIZE 64

/* Note: The test passes if the STAUT stops sending any frames, and no further
   frames are transmitted on this channel by the station when the AP has sent
   the last 6 beacons, with the channel switch information elements as seen
   with the sniffer.*/
#define SIR_CHANSW_TX_STOP_MAX_COUNT 6
/**-----------------------------------------------------------------
   \fn     lim_stop_tx_and_switch_channel
   \brief  Stops the transmission if channel switch mode is silent and
   starts the channel switch timer.

   \param  pMac
   \return NONE
   -----------------------------------------------------------------*/
void lim_stop_tx_and_switch_channel(tpAniSirGlobal pMac, uint8_t sessionId)
{
	tpPESession psessionEntry;

	psessionEntry = pe_find_session_by_session_id(pMac, sessionId);

	if (NULL == psessionEntry) {
		lim_log(pMac, LOGE, FL("Session %d not active"), sessionId);
		return;
	}

	PELOG1(lim_log(pMac, LOG1, FL("Channel switch Mode == %d"),
		       psessionEntry->gLimChannelSwitch.switchMode);
	       )

	if (psessionEntry->gLimChannelSwitch.switchMode ==
	    eSIR_CHANSW_MODE_SILENT
	    || psessionEntry->gLimChannelSwitch.switchCount <=
	    SIR_CHANSW_TX_STOP_MAX_COUNT) {
		/* Freeze the transmission */
		lim_frame_transmission_control(pMac, eLIM_TX_ALL, eLIM_STOP_TX);

	} else {
		/* Resume the transmission */
		lim_frame_transmission_control(pMac, eLIM_TX_ALL, eLIM_RESUME_TX);
	}

	pMac->lim.limTimers.gLimChannelSwitchTimer.sessionId = sessionId;
	/* change the channel immediatly only if
	 * the channel switch count is 0
	 */
	if (psessionEntry->gLimChannelSwitch.switchCount == 0) {
		lim_process_channel_switch_timeout(pMac);
		return;
	}
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_TIMER_ACTIVATE, sessionId,
		       eLIM_CHANNEL_SWITCH_TIMER));

	if (tx_timer_activate(&pMac->lim.limTimers.gLimChannelSwitchTimer) !=
	    TX_SUCCESS) {
		lim_log(pMac, LOGP, FL("tx_timer_activate failed"));
	}
	return;
}

/**------------------------------------------------------------
   \fn     lim_start_channel_switch
   \brief  Switches the channel if switch count == 0, otherwise
   starts the timer for channel switch and stops BG scan
   and heartbeat timer tempororily.

   \param  pMac
   \param  psessionEntry
   \return NONE
   ------------------------------------------------------------*/
tSirRetStatus lim_start_channel_switch(tpAniSirGlobal pMac,
				       tpPESession psessionEntry)
{
	PELOG1(lim_log(pMac, LOG1, FL("Starting the channel switch"));)

	/*If channel switch is already running and it is on a different session, just return */
	/*This need to be removed for MCC */
	if ((lim_is_chan_switch_running(pMac) &&
	     psessionEntry->gLimSpecMgmt.dot11hChanSwState !=
	     eLIM_11H_CHANSW_RUNNING) || psessionEntry->csaOffloadEnable) {
		lim_log(pMac, LOGW, FL("Ignoring channel switch on session %d"),
			psessionEntry->peSessionId);
		return eSIR_SUCCESS;
	}

	/* Deactivate and change reconfigure the timeout value */
	/* lim_deactivate_and_change_timer(pMac, eLIM_CHANNEL_SWITCH_TIMER); */
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_TIMER_DEACTIVATE, psessionEntry->peSessionId,
		       eLIM_CHANNEL_SWITCH_TIMER));
	if (tx_timer_deactivate(&pMac->lim.limTimers.gLimChannelSwitchTimer) !=
	    eSIR_SUCCESS) {
		lim_log(pMac, LOGP, FL("tx_timer_deactivate failed!"));
		return eSIR_FAILURE;
	}

	if (tx_timer_change(&pMac->lim.limTimers.gLimChannelSwitchTimer,
			    psessionEntry->gLimChannelSwitch.switchTimeoutValue,
			    0) != TX_SUCCESS) {
		lim_log(pMac, LOGP, FL("tx_timer_change failed "));
		return eSIR_FAILURE;
	}

	/* Follow the channel switch, forget about the previous quiet. */
	/* If quiet is running, chance is there to resume tx on its timeout. */
	/* so stop timer for a safer side. */
	if (psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN) {
		MTRACE(mac_trace
			       (pMac, TRACE_CODE_TIMER_DEACTIVATE,
			       psessionEntry->peSessionId, eLIM_QUIET_TIMER));
		if (tx_timer_deactivate(&pMac->lim.limTimers.gLimQuietTimer) !=
		    TX_SUCCESS) {
			lim_log(pMac, LOGP, FL("tx_timer_deactivate failed"));
			return eSIR_FAILURE;
		}
	} else if (psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_RUNNING) {
		MTRACE(mac_trace
			       (pMac, TRACE_CODE_TIMER_DEACTIVATE,
			       psessionEntry->peSessionId, eLIM_QUIET_BSS_TIMER));
		if (tx_timer_deactivate(&pMac->lim.limTimers.gLimQuietBssTimer)
		    != TX_SUCCESS) {
			lim_log(pMac, LOGP, FL("tx_timer_deactivate failed"));
			return eSIR_FAILURE;
		}
	}
	psessionEntry->gLimSpecMgmt.quietState = eLIM_QUIET_INIT;

	/* Prepare for 11h channel switch */
	lim_prepare_for11h_channel_switch(pMac, psessionEntry);

	/** Dont add any more statements here as we posted finish scan request
	 * to HAL, wait till we get the response
	 */
	return eSIR_SUCCESS;
}

/**
 *  __lim_process_channel_switch_action_frame() - to process channel switch
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_pkt_info: A pointer to packet info structure
 *
 * This routine will be called to process channel switch action frame
 *
 * Return: None
 */

static void __lim_process_channel_switch_action_frame(tpAniSirGlobal mac_ctx,
			  uint8_t *rx_pkt_info, tpPESession session)
{
	tpSirMacMgmtHdr mac_hdr;
	uint8_t *body_ptr;
	tDot11fChannelSwitch *chnl_switch_frame;
	uint16_t bcn_period;
	uint32_t val, frame_len, status;
	tLimChannelSwitchInfo *ch_switch_params;
	struct sDot11fIEWiderBWChanSwitchAnn *wbw_chnlswitch_ie = NULL;
	struct sLimWiderBWChannelSwitch *lim_wbw_chnlswitch_info = NULL;
	struct sDot11fIEsec_chan_offset_ele *sec_chnl_offset = NULL;

	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	PELOG3(lim_log(mac_ctx, LOG3,
		FL("Received Channel switch action frame"));)
	if (!session->lim11hEnable)
		return;

	chnl_switch_frame = qdf_mem_malloc(sizeof(*chnl_switch_frame));
	if (NULL == chnl_switch_frame) {
		lim_log(mac_ctx, LOGE, FL("AllocateMemory failed"));
		return;
	}

	/* Unpack channel switch frame */
	status = dot11f_unpack_channel_switch(mac_ctx, body_ptr, frame_len,
			chnl_switch_frame);

	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Failed to unpack and parse (0x%08x, %d bytes)"),
			status, frame_len);
		qdf_mem_free(chnl_switch_frame);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("warning: unpack 11h-CHANSW Req(0x%08x, %d bytes)"),
			status, frame_len);
	}

	if (qdf_mem_cmp((uint8_t *) &session->bssId,
			(uint8_t *) &mac_hdr->sa, sizeof(tSirMacAddr))) {
		lim_log(mac_ctx, LOG1,
			FL("Rcvd action frame not from our BSS, dropping..."));
		qdf_mem_free(chnl_switch_frame);
		return;
	}
	/* copy the beacon interval from session */
	val = session->beaconParams.beaconInterval;
	ch_switch_params = &session->gLimChannelSwitch;
	bcn_period = (uint16_t)val;
	ch_switch_params->primaryChannel =
		chnl_switch_frame->ChanSwitchAnn.newChannel;
	ch_switch_params->switchCount =
		chnl_switch_frame->ChanSwitchAnn.switchCount;
	ch_switch_params->switchTimeoutValue =
		SYS_MS_TO_TICKS(bcn_period) *
		session->gLimChannelSwitch.switchCount;
	ch_switch_params->switchMode =
		chnl_switch_frame->ChanSwitchAnn.switchMode;

	/* Only primary channel switch element is present */
	ch_switch_params->state = eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
	ch_switch_params->ch_width = CH_WIDTH_20MHZ;

	if (chnl_switch_frame->WiderBWChanSwitchAnn.present
			&& session->vhtCapability) {
		wbw_chnlswitch_ie = &chnl_switch_frame->WiderBWChanSwitchAnn;
		session->gLimWiderBWChannelSwitch.newChanWidth =
			wbw_chnlswitch_ie->newChanWidth;
		session->gLimWiderBWChannelSwitch.newCenterChanFreq0 =
			wbw_chnlswitch_ie->newCenterChanFreq0;
		session->gLimWiderBWChannelSwitch.newCenterChanFreq1 =
			wbw_chnlswitch_ie->newCenterChanFreq1;
	}
	lim_log(mac_ctx, LOG3,
		FL("Rcv Chnl Swtch Frame: Timeout in %d ticks"),
		session->gLimChannelSwitch.switchTimeoutValue);
	if (session->htSupportedChannelWidthSet) {
		sec_chnl_offset = &chnl_switch_frame->sec_chan_offset_ele;
		if (sec_chnl_offset->secondaryChannelOffset ==
				PHY_DOUBLE_CHANNEL_LOW_PRIMARY) {
			ch_switch_params->state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			ch_switch_params->ch_width = CH_WIDTH_40MHZ;
			ch_switch_params->ch_center_freq_seg0 =
				ch_switch_params->primaryChannel + 2;
		} else if (sec_chnl_offset->secondaryChannelOffset ==
				PHY_DOUBLE_CHANNEL_HIGH_PRIMARY) {
			ch_switch_params->state =
				eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
			ch_switch_params->ch_width = CH_WIDTH_40MHZ;
			ch_switch_params->ch_center_freq_seg0 =
				ch_switch_params->primaryChannel - 2;

		}
		if (session->vhtCapability &&
			chnl_switch_frame->WiderBWChanSwitchAnn.present) {
			wbw_chnlswitch_ie =
				&chnl_switch_frame->WiderBWChanSwitchAnn;
			ch_switch_params->ch_width =
				wbw_chnlswitch_ie->newChanWidth + 1;
			lim_wbw_chnlswitch_info =
				&session->gLimWiderBWChannelSwitch;
			ch_switch_params->ch_center_freq_seg0 =
				lim_wbw_chnlswitch_info->newCenterChanFreq0;
			ch_switch_params->ch_center_freq_seg1 =
				lim_wbw_chnlswitch_info->newCenterChanFreq1;

		}
	}

	if (CH_WIDTH_20MHZ == ch_switch_params->ch_width) {
		session->htSupportedChannelWidthSet =
			WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		session->htRecommendedTxWidthSet =
			session->htSupportedChannelWidthSet;
	}

	if (eSIR_SUCCESS != lim_start_channel_switch(mac_ctx, session))
		lim_log(mac_ctx, LOG1,
			FL("Could not start channel switch"));

	qdf_mem_free(chnl_switch_frame);
	return;
}

/**
 * lim_process_ext_channel_switch_action_frame()- Process ECSA Action
 * Frames.
 * @mac_ctx: pointer to global mac structure
 * @rx_packet_info: rx packet meta information
 * @session_entry: Session entry.
 *
 * This function is called when ECSA action frame is received.
 *
 * Return: void
 */
static void
lim_process_ext_channel_switch_action_frame(tpAniSirGlobal mac_ctx,
		uint8_t *rx_packet_info, tpPESession session_entry)
{

	tpSirMacMgmtHdr         hdr;
	uint8_t                 *body;
	tDot11fext_channel_switch_action_frame *ext_channel_switch_frame;
	uint32_t                frame_len;
	uint32_t                status;
	uint8_t                 target_channel;

	hdr = WMA_GET_RX_MAC_HEADER(rx_packet_info);
	body = WMA_GET_RX_MPDU_DATA(rx_packet_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_packet_info);

	lim_log(mac_ctx, LOG1, FL("Received EXT Channel switch action frame"));

	ext_channel_switch_frame =
		 qdf_mem_malloc(sizeof(*ext_channel_switch_frame));
	if (NULL == ext_channel_switch_frame) {
		lim_log(mac_ctx, LOGE, FL("AllocateMemory failed"));
		return;
	}

	/* Unpack channel switch frame */
	status = dot11f_unpack_ext_channel_switch_action_frame(mac_ctx,
			body, frame_len, ext_channel_switch_frame);

	if (DOT11F_FAILED(status)) {

		lim_log(mac_ctx, LOGE,
			FL("Failed to parse CHANSW action frame (0x%08x, len %d):"),
			status, frame_len);
		qdf_mem_free(ext_channel_switch_frame);
		return;
	} else if (DOT11F_WARNED(status)) {

		lim_log(mac_ctx, LOGW,
		  FL("There were warnings while unpacking CHANSW Request (0x%08x, %d bytes):"),
		  status, frame_len);
	}

	target_channel =
	 ext_channel_switch_frame->ext_chan_switch_ann_action.new_channel;

	/* Free ext_channel_switch_frame here as its no longer needed */
	qdf_mem_free(ext_channel_switch_frame);
	/*
	 * Now, validate if channel change is required for the passed
	 * channel and if is valid in the current regulatory domain,
	 * and no concurrent session is running.
	 */
	if (!((session_entry->currentOperChannel != target_channel) &&
	 ((cds_get_channel_state(target_channel)
				== CHANNEL_STATE_ENABLE) ||
	 (cds_get_channel_state(target_channel) == CHANNEL_STATE_DFS &&
	 !cds_concurrent_open_sessions_running())))) {
		lim_log(mac_ctx, LOGE, FL("Channel %d is not valid"),
							target_channel);
		return;
	}

	if (eLIM_AP_ROLE == session_entry->limSystemRole) {

		struct sir_sme_ext_cng_chan_ind *ext_cng_chan_ind;
		tSirMsgQ mmh_msg;

		ext_cng_chan_ind = qdf_mem_malloc(sizeof(*ext_cng_chan_ind));
		if (NULL == ext_cng_chan_ind) {
			lim_log(mac_ctx, LOGP,
			  FL("AllocateMemory failed for ext_cng_chan_ind"));
			return;
		}

		qdf_mem_zero(ext_cng_chan_ind,
			sizeof(*ext_cng_chan_ind));
		ext_cng_chan_ind->session_id =
					session_entry->smeSessionId;

		/* No need to extract op mode as BW will be decided in
		 *  in SAP FSM depending on previous BW.
		 */
		ext_cng_chan_ind->new_channel = target_channel;

		mmh_msg.type = eWNI_SME_EXT_CHANGE_CHANNEL_IND;
		mmh_msg.bodyptr = ext_cng_chan_ind;
		mmh_msg.bodyval = 0;
		lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
	}
	return;
} /*** end lim_process_ext_channel_switch_action_frame() ***/

/**
 * __lim_process_operating_mode_action_frame() - To process op mode frames
 * @mac_ctx: pointer to mac context
 * @rx_pkt_info: pointer to received packet info
 * @session: pointer to session
 *
 * This routine is called to process operating mode action frames
 *
 * Return: None
 */
static void __lim_process_operating_mode_action_frame(tpAniSirGlobal mac_ctx,
			uint8_t *rx_pkt_info, tpPESession session)
{

	tpSirMacMgmtHdr mac_hdr;
	uint8_t *body_ptr;
	tDot11fOperatingMode *operating_mode_frm;
	uint32_t frame_len;
	uint32_t status;
	tpDphHashNode sta_ptr;
	uint16_t aid;
	uint8_t oper_mode;
	uint8_t cb_mode;
	uint8_t ch_bw = 0;
	uint8_t skip_opmode_update = false;

	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	lim_log(mac_ctx, LOG1,
		FL("Received Operating Mode action frame"));
	if (CHAN_ENUM_14 >= session->currentOperChannel)
		cb_mode = mac_ctx->roam.configParam.channelBondingMode24GHz;
	else
		cb_mode = mac_ctx->roam.configParam.channelBondingMode5GHz;
	/* Do not update the channel bonding mode if channel bonding
	 * mode is disabled in INI.
	 */
	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE == cb_mode) {
		lim_log(mac_ctx, LOGW, FL("channel bonding disabled"));
		return;
	}
	operating_mode_frm = qdf_mem_malloc(sizeof(*operating_mode_frm));
	if (NULL == operating_mode_frm) {
		lim_log(mac_ctx, LOGE, FL("AllocateMemory failed"));
		return;
	}
	/* Unpack channel switch frame */
	status = dot11f_unpack_operating_mode(mac_ctx, body_ptr, frame_len,
			operating_mode_frm);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Failed to unpack and parse (0x%08x, %d bytes)"),
			status, frame_len);
		qdf_mem_free(operating_mode_frm);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("warnings while unpacking (0x%08x, %d bytes):"),
			status, frame_len);
	}
	sta_ptr = dph_lookup_hash_entry(mac_ctx, mac_hdr->sa, &aid,
			&session->dph.dphHashTable);

	if (sta_ptr == NULL) {
		lim_log(mac_ctx, LOGE, FL("Station context not found"));
		goto end;
	}

	if (sta_ptr->htSupportedChannelWidthSet) {
		if (WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ <
				sta_ptr->vhtSupportedChannelWidthSet)
			oper_mode = eHT_CHANNEL_WIDTH_160MHZ;
		else
			oper_mode = sta_ptr->vhtSupportedChannelWidthSet + 1;
	} else {
		oper_mode = eHT_CHANNEL_WIDTH_20MHZ;
	}

	if ((oper_mode == eHT_CHANNEL_WIDTH_80MHZ) &&
			(operating_mode_frm->OperatingMode.chanWidth >
				eHT_CHANNEL_WIDTH_80MHZ))
		skip_opmode_update = true;

	if (!skip_opmode_update && (oper_mode !=
		operating_mode_frm->OperatingMode.chanWidth)) {
		uint32_t fw_vht_ch_wd = wma_get_vht_ch_width();

		lim_log(mac_ctx, LOGE,
			FL(" received Chanwidth %d, staIdx = %d"),
			(operating_mode_frm->OperatingMode.chanWidth),
			sta_ptr->staIndex);

		lim_log(mac_ctx, LOGE,
			FL(" MAC - %0x:%0x:%0x:%0x:%0x:%0x"),
			mac_hdr->sa[0], mac_hdr->sa[1], mac_hdr->sa[2],
			mac_hdr->sa[3], mac_hdr->sa[4], mac_hdr->sa[5]);

		if (operating_mode_frm->OperatingMode.chanWidth >=
				eHT_CHANNEL_WIDTH_160MHZ
				&& (fw_vht_ch_wd >= eHT_CHANNEL_WIDTH_160MHZ)) {
			sta_ptr->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;
			sta_ptr->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
			ch_bw = eHT_CHANNEL_WIDTH_160MHZ;
		} else if (operating_mode_frm->OperatingMode.chanWidth >=
				eHT_CHANNEL_WIDTH_80MHZ) {
			sta_ptr->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
			sta_ptr->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
			ch_bw = eHT_CHANNEL_WIDTH_80MHZ;
		} else if (operating_mode_frm->OperatingMode.chanWidth ==
				eHT_CHANNEL_WIDTH_40MHZ) {
			sta_ptr->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
			sta_ptr->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
			ch_bw = eHT_CHANNEL_WIDTH_40MHZ;
		} else if (operating_mode_frm->OperatingMode.chanWidth ==
				eHT_CHANNEL_WIDTH_20MHZ) {
			sta_ptr->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
			sta_ptr->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_20MHZ;
			ch_bw = eHT_CHANNEL_WIDTH_20MHZ;
		}
		lim_check_vht_op_mode_change(mac_ctx, session, ch_bw,
					     sta_ptr->staIndex, mac_hdr->sa);
	}

	if (sta_ptr->vhtSupportedRxNss !=
			(operating_mode_frm->OperatingMode.rxNSS + 1)) {
		sta_ptr->vhtSupportedRxNss =
			operating_mode_frm->OperatingMode.rxNSS + 1;
		lim_set_nss_change(mac_ctx, session,
			operating_mode_frm->OperatingMode.rxNSS,
			sta_ptr->staIndex, mac_hdr->sa);
	}

end:
	qdf_mem_free(operating_mode_frm);
	return;
}

/**
 * __lim_process_gid_management_action_frame() - To process group-id mgmt frames
 * @mac_ctx: Pointer to mac context
 * @rx_pkt_info: Rx packet info
 * @session: pointer to session
 *
 * This routine will be called to process group id management frames
 *
 * Return: none
 */
static void __lim_process_gid_management_action_frame(tpAniSirGlobal mac_ctx,
			uint8_t *rx_pkt_info, tpPESession session)
{

	uint8_t *body_ptr;
	uint16_t aid;
	uint32_t frame_len, status, membership = 0, usr_position = 0;
	uint32_t *mem_lower, *mem_upper, *mem_cur;
	tpSirMacMgmtHdr mac_hdr;
	tDot11fVHTGidManagementActionFrame *gid_mgmt_frame;
	tpDphHashNode sta_ptr;
	struct sDot11fFfVhtMembershipStatusArray *vht_member_status = NULL;
	struct sDot11fFfVhtUserPositionArray *vht_user_position = NULL;

	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	lim_log(mac_ctx, LOG3, FL("Received GID Management action frame"));
	gid_mgmt_frame = qdf_mem_malloc(sizeof(*gid_mgmt_frame));
	if (NULL == gid_mgmt_frame) {
		lim_log(mac_ctx, LOGE, FL("AllocateMemory failed"));
		return;
	}

	/* Unpack Gid Mangement Action frame */
	status = dot11f_unpack_vht_gid_management_action_frame(mac_ctx,
			body_ptr, frame_len, gid_mgmt_frame);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Fail to parse an Grp id frame (0x%08x, %d bytes):"),
			status, frame_len);
		qdf_mem_free(gid_mgmt_frame);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
		 FL("warnings while unpacking Grp id frm (0x%08x, %d bytes):"),
		 status, frame_len);
	}
	sta_ptr = dph_lookup_hash_entry(mac_ctx, mac_hdr->sa, &aid,
			&session->dph.dphHashTable);
	lim_log(mac_ctx, LOGE,
		FL("received Gid Management Action Frame , staIdx = %d"),
		sta_ptr->staIndex);

	lim_log(mac_ctx, LOGE,
		FL(" MAC - %0x:%0x:%0x:%0x:%0x:%0x"),
		mac_hdr->sa[0], mac_hdr->sa[1], mac_hdr->sa[2],
		mac_hdr->sa[3], mac_hdr->sa[4], mac_hdr->sa[5]);
	vht_member_status = &gid_mgmt_frame->VhtMembershipStatusArray;
	mem_lower =  (uint32_t *) vht_member_status->membershipStatusArray;
	mem_upper = (uint32_t *) &vht_member_status->membershipStatusArray[4];

	if (*mem_lower && *mem_upper) {
		lim_log(mac_ctx, LOGE,
			FL("rcved frame with mult group ID set, staIdx = %d"),
			sta_ptr->staIndex);
		goto out;
	}
	if (*mem_lower) {
		mem_cur = mem_lower;
	} else if (*mem_upper) {
		mem_cur = mem_upper;
		membership += sizeof(uint32_t);
	} else {
		lim_log(mac_ctx, LOGE,
			FL("rcved Gid frame with no group ID set, staIdx = %d"),
			sta_ptr->staIndex);
		goto out;
	}
	while (!(*mem_cur & 1)) {
		*mem_cur >>= 1;
		++membership;
	}
	if (*mem_cur) {
		lim_log(mac_ctx, LOGE,
			FL("rcved frame with mult group ID set, staIdx = %d"),
			sta_ptr->staIndex);
		goto out;
	}

	/*Just read the last two bits */
	vht_user_position = &gid_mgmt_frame->VhtUserPositionArray;
	usr_position = vht_user_position->userPositionArray[membership] & 0x3;
	lim_check_membership_user_position(mac_ctx, session, membership,
			usr_position, sta_ptr->staIndex);
out:
	qdf_mem_free(gid_mgmt_frame);
	return;
}

static void
__lim_process_add_ts_req(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
			 tpPESession psessionEntry)
{
}

/**
 * __lim_process_add_ts_rsp() - To process add ts response frame
 * @mac_ctx: pointer to mac context
 * @rx_pkt_info: Received packet info
 * @session: pointer to session
 *
 * This routine is to handle add ts response frame
 *
 * Return: none
 */
static void __lim_process_add_ts_rsp(tpAniSirGlobal mac_ctx,
		uint8_t *rx_pkt_info, tpPESession session)
{
	tSirAddtsRspInfo addts;
	tSirRetStatus retval;
	tpSirMacMgmtHdr mac_hdr;
	tpDphHashNode sta_ptr;
	uint16_t aid;
	uint32_t frameLen;
	uint8_t *body_ptr;
	tpLimTspecInfo tspec_info;
	uint8_t ac;
	tpDphHashNode sta_ds_ptr = NULL;
	uint8_t rsp_reqd = 1;
	uint32_t cfg_len;
	tSirMacAddr peer_macaddr;

	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	lim_log(mac_ctx, LOGW, "Recv AddTs Response");
	if (LIM_IS_AP_ROLE(session)) {
		lim_log(mac_ctx, LOGW,
			FL("AddTsRsp recvd at AP: ignoring"));
		return;
	}

	sta_ptr = dph_lookup_hash_entry(mac_ctx, mac_hdr->sa, &aid,
				&session->dph.dphHashTable);
	if (sta_ptr == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Station context not found - ignoring AddTsRsp"));
		return;
	}

	retval = sir_convert_addts_rsp2_struct(mac_ctx, body_ptr,
			frameLen, &addts);
	if (retval != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGW,
			FL("AddTsRsp parsing failed (error %d)"), retval);
		return;
	}
	/*
	 * don't have to check for qos/wme capabilities since we wouldn't have
	 * this flag set otherwise
	 */
	if (!mac_ctx->lim.gLimAddtsSent) {
		/* we never sent an addts request! */
		lim_log(mac_ctx, LOGW,
			FL("rx AddTsRsp but no req was ever sent-ignoring"));
		return;
	}

	if (mac_ctx->lim.gLimAddtsReq.req.dialogToken != addts.dialogToken) {
		lim_log(mac_ctx, LOGW,
			FL("token mismatch (got %d, exp %d) - ignoring"),
			addts.dialogToken,
			mac_ctx->lim.gLimAddtsReq.req.dialogToken);
		return;
	}

	/*
	 * for successful addts reponse, try to add the classifier.
	 * if this fails for any reason, we should send a delts request to the
	 * ap for now, its ok not to send a delts since we are going to add
	 * support for multiple tclas soon and until then we won't send any
	 * addts requests with multiple tclas elements anyway.
	 * In case of addClassifier failure, we just let the addts timer run out
	 */
	if (((addts.tspec.tsinfo.traffic.accessPolicy ==
		SIR_MAC_ACCESSPOLICY_HCCA) ||
		(addts.tspec.tsinfo.traffic.accessPolicy ==
			SIR_MAC_ACCESSPOLICY_BOTH)) &&
		(addts.status == eSIR_MAC_SUCCESS_STATUS)) {
		/* add the classifier - this should always succeed */
		if (addts.numTclas > 1) {
			/* currently no support for multiple tclas elements */
			lim_log(mac_ctx, LOGE,
				FL("Sta %d: Too many Tclas (%d), 1 supported"),
				aid, addts.numTclas);
			return;
		} else if (addts.numTclas == 1) {
			lim_log(mac_ctx, LOGW,
				FL("Response from STA %d: tsid %d, UP %d, OK!"),
				aid, addts.tspec.tsinfo.traffic.tsid,
				addts.tspec.tsinfo.traffic.userPrio);
		}
	}
	lim_log(mac_ctx, LOGW, FL("Recv AddTsRsp: tsid %d, UP %d, status %d "),
		addts.tspec.tsinfo.traffic.tsid,
		addts.tspec.tsinfo.traffic.userPrio, addts.status);

	/* deactivate the response timer */
	lim_deactivate_and_change_timer(mac_ctx, eLIM_ADDTS_RSP_TIMER);

	if (addts.status != eSIR_MAC_SUCCESS_STATUS) {
		lim_log(mac_ctx, LOGW,
			FL("Recv AddTsRsp: tsid %d, UP %d, status %d "),
			addts.tspec.tsinfo.traffic.tsid,
			addts.tspec.tsinfo.traffic.userPrio, addts.status);
		lim_send_sme_addts_rsp(mac_ctx, true, addts.status, session,
				       addts.tspec, session->smeSessionId,
				       session->transactionId);

		/* clear the addts flag */
		mac_ctx->lim.gLimAddtsSent = false;

		return;
	}
#ifdef FEATURE_WLAN_ESE
	if (addts.tsmPresent) {
		lim_log(mac_ctx, LOGW, "TSM IE Present");
		session->eseContext.tsm.tid =
			addts.tspec.tsinfo.traffic.userPrio;
		qdf_mem_copy(&session->eseContext.tsm.tsmInfo,
			     &addts.tsmIE, sizeof(tSirMacESETSMIE));
		lim_send_sme_tsm_ie_ind(mac_ctx, session, addts.tsmIE.tsid,
					addts.tsmIE.state,
					addts.tsmIE.msmt_interval);
	}
#endif
	/*
	 * Since AddTS response was successful, check for the PSB flag
	 * and directional flag inside the TS Info field.
	 * An AC is trigger enabled AC if the PSB subfield is set to 1
	 * in the uplink direction.
	 * An AC is delivery enabled AC if the PSB subfield is set to 1
	 * in the downlink direction.
	 * An AC is trigger and delivery enabled AC if the PSB subfield
	 * is set to 1 in the bi-direction field.
	 */
	if (addts.tspec.tsinfo.traffic.psb == 1)
		lim_set_tspec_uapsd_mask_per_session(mac_ctx, session,
						     &addts.tspec.tsinfo,
						     SET_UAPSD_MASK);
	else
		lim_set_tspec_uapsd_mask_per_session(mac_ctx, session,
						     &addts.tspec.tsinfo,
						     CLEAR_UAPSD_MASK);

	/*
	 * ADDTS success, so AC is now admitted. We shall now use the default
	 * EDCA parameters as advertised by AP and send the updated EDCA params
	 * to HAL.
	 */
	ac = upToAc(addts.tspec.tsinfo.traffic.userPrio);
	if (addts.tspec.tsinfo.traffic.direction ==
	    SIR_MAC_DIRECTION_UPLINK) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] |=
			(1 << ac);
	} else if (addts.tspec.tsinfo.traffic.direction ==
		   SIR_MAC_DIRECTION_DNLINK) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] |=
			(1 << ac);
	} else if (addts.tspec.tsinfo.traffic.direction ==
		   SIR_MAC_DIRECTION_BIDIR) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] |=
			(1 << ac);
		session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] |=
			(1 << ac);
	}
	lim_set_active_edca_params(mac_ctx, session->gLimEdcaParams,
				   session);
	sta_ds_ptr = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
				   &session->dph.dphHashTable);
	if (sta_ds_ptr != NULL)
		lim_send_edca_params(mac_ctx, session->gLimEdcaParamsActive,
				     sta_ds_ptr->bssId);
	else
		lim_log(mac_ctx, LOGE, FL("Self entry missing in Hash Table "));
	sir_copy_mac_addr(peer_macaddr, session->bssId);
	/* if schedule is not present then add TSPEC with svcInterval as 0. */
	if (!addts.schedulePresent)
		addts.schedule.svcInterval = 0;
	if (eSIR_SUCCESS !=
	    lim_tspec_add(mac_ctx, sta_ptr->staAddr, sta_ptr->assocId,
		&addts.tspec, addts.schedule.svcInterval, &tspec_info)) {
		lim_log(mac_ctx, LOGE,
			FL("Adding entry in lim Tspec Table failed "));
		lim_send_delts_req_action_frame(mac_ctx, peer_macaddr, rsp_reqd,
						&addts.tspec.tsinfo,
						&addts.tspec, session);
		mac_ctx->lim.gLimAddtsSent = false;
		return;
		/*
		 * Error handling. send the response with error status.
		 * need to send DelTS to tear down the TSPEC status.
		 */
	}
	if ((addts.tspec.tsinfo.traffic.accessPolicy !=
			SIR_MAC_ACCESSPOLICY_EDCA) ||
		((upToAc(addts.tspec.tsinfo.traffic.userPrio) < MAX_NUM_AC))) {
#ifdef FEATURE_WLAN_ESE
		retval = lim_send_hal_msg_add_ts(mac_ctx,
				sta_ptr->staIndex, tspec_info->idx,
				addts.tspec, session->peSessionId,
				addts.tsmIE.msmt_interval);
#else
		retval = lim_send_hal_msg_add_ts(mac_ctx,
				sta_ptr->staIndex, tspec_info->idx,
				addts.tspec, session->peSessionId);
#endif
		if (eSIR_SUCCESS != retval) {
			lim_admit_control_delete_ts(mac_ctx, sta_ptr->assocId,
				&addts.tspec.tsinfo, NULL, &tspec_info->idx);

			/* Send DELTS action frame to AP */
			cfg_len = sizeof(tSirMacAddr);
			lim_send_delts_req_action_frame(mac_ctx, peer_macaddr,
					rsp_reqd, &addts.tspec.tsinfo,
					&addts.tspec, session);
			lim_send_sme_addts_rsp(mac_ctx, true, retval,
					session, addts.tspec,
					session->smeSessionId,
					session->transactionId);
			mac_ctx->lim.gLimAddtsSent = false;
			return;
		}
		lim_log(mac_ctx, LOGW,
			FL("AddTsRsp received successfully(UP %d, TSID %d)"),
			addts.tspec.tsinfo.traffic.userPrio,
			addts.tspec.tsinfo.traffic.tsid);
	} else {
		lim_log(mac_ctx, LOGW,
			FL("AddTsRsp received successfully(UP %d, TSID %d)"),
			addts.tspec.tsinfo.traffic.userPrio,
			addts.tspec.tsinfo.traffic.tsid);
		lim_log(mac_ctx, LOGW,
			FL("no ACM: Bypass sending WMA_ADD_TS_REQ to HAL "));
		/*
		 * Use the smesessionId and smetransactionId from the PE
		 * session context
		 */
		lim_send_sme_addts_rsp(mac_ctx, true, eSIR_SME_SUCCESS,
			session, addts.tspec, session->smeSessionId,
			session->transactionId);
	}
	/* clear the addts flag */
	mac_ctx->lim.gLimAddtsSent = false;
	return;
}

/**
 * __lim_process_del_ts_req() - To process del ts response frame
 * @mac_ctx: pointer to mac context
 * @rx_pkt_info: Received packet info
 * @session: pointer to session
 *
 * This routine is to handle del ts request frame
 *
 * Return: none
 */
static void __lim_process_del_ts_req(tpAniSirGlobal mac_ctx,
		uint8_t *rx_pkt_info, tpPESession session)
{
	tSirRetStatus retval;
	tSirDeltsReqInfo delts;
	tpSirMacMgmtHdr mac_hdr;
	tpDphHashNode sta_ptr;
	uint32_t frame_len;
	uint16_t aid;
	uint8_t *body_ptr;
	uint8_t ts_status;
	tSirMacTSInfo *tsinfo;
	uint8_t tspec_idx;
	uint8_t ac;
	tpDphHashNode sta_ds_ptr = NULL;

	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	sta_ptr = dph_lookup_hash_entry(mac_ctx, mac_hdr->sa, &aid,
				      &session->dph.dphHashTable);
	if (sta_ptr == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Station context not found - ignoring DelTs"));
		return;
	}
	/* parse the delts request */
	retval = sir_convert_delts_req2_struct(mac_ctx, body_ptr,
			frame_len, &delts);
	if (retval != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGW,
			FL("DelTs parsing failed (error %d)"), retval);
		return;
	}

	if (delts.wmeTspecPresent) {
		if ((!session->limWmeEnabled) || (!sta_ptr->wmeEnabled)) {
			lim_log(mac_ctx, LOGW,
				FL("Ignore delts req: wme not enabled"));
			return;
		}
		lim_log(mac_ctx, LOG2, FL("WME Delts received"));
	} else if ((session->limQosEnabled) && sta_ptr->lleEnabled) {
		lim_log(mac_ctx, LOG2, FL("11e QoS Delts received"));
	} else if ((session->limWsmEnabled) && sta_ptr->wsmEnabled) {
		lim_log(mac_ctx, LOG2, FL("WSM Delts received"));
	} else {
		lim_log(mac_ctx, LOGW,
			FL("Ignoring delts request: qos not enabled/capable"));
		return;
	}

	tsinfo = delts.wmeTspecPresent ? &delts.tspec.tsinfo : &delts.tsinfo;

	/* if no Admit Control, ignore the request */
	if ((tsinfo->traffic.accessPolicy == SIR_MAC_ACCESSPOLICY_EDCA)) {

		if (upToAc(tsinfo->traffic.userPrio) >= MAX_NUM_AC) {
			lim_log(mac_ctx, LOGW,
				FL("DelTs with UP %d has no AC - ignoring req"),
				tsinfo->traffic.userPrio);
			return;
		}
	}

	if (!LIM_IS_AP_ROLE(session))
		lim_send_sme_delts_ind(mac_ctx, &delts, aid, session);

	/* try to delete the TS */
	if (eSIR_SUCCESS !=
	    lim_admit_control_delete_ts(mac_ctx, sta_ptr->assocId, tsinfo,
				&ts_status, &tspec_idx)) {
		lim_log(mac_ctx, LOGW, FL("Unable to Delete TS"));
		return;
	} else if (!((tsinfo->traffic.accessPolicy == SIR_MAC_ACCESSPOLICY_HCCA)
			|| (tsinfo->traffic.accessPolicy ==
					SIR_MAC_ACCESSPOLICY_BOTH))){
		/* send message to HAL to delete TS */
		if (eSIR_SUCCESS != lim_send_hal_msg_del_ts(mac_ctx,
						sta_ptr->staIndex, tspec_idx,
						delts, session->peSessionId,
						session->bssId)) {
			lim_log(mac_ctx, LOGW,
				FL("DelTs with UP %d failed ignoring request"),
				tsinfo->traffic.userPrio);
			return;
		}
	}
	/*
	 * We successfully deleted the TSPEC. Update the dynamic UAPSD Mask.
	 * The AC for this TSPEC is no longer trigger enabled if this Tspec
	 * was set-up in uplink direction only.
	 * The AC for this TSPEC is no longer delivery enabled if this Tspec
	 * was set-up in downlink direction only.
	 * The AC for this TSPEC is no longer triiger enabled and delivery
	 * enabled if this Tspec was a bidirectional TSPEC.
	 */
	lim_set_tspec_uapsd_mask_per_session(mac_ctx, session,
					     tsinfo, CLEAR_UAPSD_MASK);
	/*
	 * We're deleting the TSPEC.
	 * The AC for this TSPEC is no longer admitted in uplink/downlink
	 * direction if this TSPEC was set-up in uplink/downlink direction only.
	 * The AC for this TSPEC is no longer admitted in both uplink and
	 * downlink directions if this TSPEC was a bi-directional TSPEC.
	 * If ACM is set for this AC and this AC is admitted only in downlink
	 * direction, PE needs to downgrade the EDCA parameter
	 * (for the AC for which TS is being deleted) to the
	 * next best AC for which ACM is not enabled, and send the
	 * updated values to HAL.
	 */
	ac = upToAc(tsinfo->traffic.userPrio);
	if (tsinfo->traffic.direction == SIR_MAC_DIRECTION_UPLINK) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &=
			~(1 << ac);
	} else if (tsinfo->traffic.direction ==
		   SIR_MAC_DIRECTION_DNLINK) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &=
			~(1 << ac);
	} else if (tsinfo->traffic.direction == SIR_MAC_DIRECTION_BIDIR) {
		session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &=
			~(1 << ac);
		session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &=
			~(1 << ac);
	}
	lim_set_active_edca_params(mac_ctx, session->gLimEdcaParams,
				   session);
	sta_ds_ptr = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
				   &session->dph.dphHashTable);
	if (sta_ds_ptr != NULL)
		lim_send_edca_params(mac_ctx, session->gLimEdcaParamsActive,
				     sta_ds_ptr->bssId);
	else
		lim_log(mac_ctx, LOGE, FL("Self entry missing in Hash Table "));

	lim_log(mac_ctx, LOG1, FL("DeleteTS succeeded"));
#ifdef FEATURE_WLAN_ESE
	lim_send_sme_tsm_ie_ind(mac_ctx, session, 0, 0, 0);
#endif
}

/**
 * __lim_process_qos_map_configure_frame() - to process QoS map configure frame
 * @mac_ctx: pointer to mac context
 * @rx_pkt_info: pointer to received packet info
 * @session: pointer to session
 *
 * This routine will called to process qos map configure frame
 *
 * Return: none
 */
static void __lim_process_qos_map_configure_frame(tpAniSirGlobal mac_ctx,
			uint8_t *rx_pkt_info, tpPESession session)
{
	tpSirMacMgmtHdr mac_hdr;
	uint32_t frame_len;
	uint8_t *body_ptr;
	tSirRetStatus retval;
	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
	retval = sir_convert_qos_map_configure_frame2_struct(mac_ctx,
				body_ptr, frame_len, &session->QosMapSet);
	if (retval != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("QosMapConfigure frame parsing fail(error %d)"),
			retval);
		return;
	}
	lim_send_sme_mgmt_frame_ind(mac_ctx, mac_hdr->fc.subType,
			(uint8_t *) mac_hdr,
			frame_len + sizeof(tSirMacMgmtHdr), 0,
			WMA_GET_RX_CH(rx_pkt_info), session, 0);
}

#ifdef ANI_SUPPORT_11H
static void
__lim_process_basic_meas_req(tpAniSirGlobal pMac,
			     tpSirMacMeasReqActionFrame pMeasReqFrame,
			     tSirMacAddr peerMacAddr, tpPESession psessionEntry)
{
	if (lim_send_meas_report_frame(pMac, pMeasReqFrame,
				       peerMacAddr, psessionEntry) !=
					 eSIR_SUCCESS) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("fail to send Basic Meas report "));
		       )
		return;
	}
}
static void
__lim_process_cca_meas_req(tpAniSirGlobal pMac,
			   tpSirMacMeasReqActionFrame pMeasReqFrame,
			   tSirMacAddr peerMacAddr, tpPESession psessionEntry)
{
	if (lim_send_meas_report_frame(pMac, pMeasReqFrame,
				       peerMacAddr, psessionEntry) !=
					 eSIR_SUCCESS) {
		PELOGE(lim_log(pMac, LOGE, FL("fail to send CCA Meas report "));)
		return;
	}
}
static void
__lim_process_rpi_meas_req(tpAniSirGlobal pMac,
			   tpSirMacMeasReqActionFrame pMeasReqFrame,
			   tSirMacAddr peerMacAddr, tpPESession psessionEntry)
{
	if (lim_send_meas_report_frame(pMac, pMeasReqFrame,
				       peerMacAddr, psessionEntry) !=
					 eSIR_SUCCESS) {
		PELOGE(lim_log(pMac, LOGE, FL("fail to send RPI Meas report "));)
		return;
	}
}
static void
__lim_process_measurement_request_frame(tpAniSirGlobal pMac,
					uint8_t *pRxPacketInfo,
					tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	uint8_t *pBody;
	tpSirMacMeasReqActionFrame pMeasReqFrame;
	uint32_t frameLen;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	pMeasReqFrame = qdf_mem_malloc(sizeof(tSirMacMeasReqActionFrame));
	if (NULL == pMeasReqFrame) {
		lim_log(pMac, LOGE,
			FL
				("limProcessMeasurementRequestFrame: AllocateMemory failed "));
		return;
	}

	if (sir_convert_meas_req_frame2_struct(pMac, pBody, pMeasReqFrame, frameLen)
	    != eSIR_SUCCESS) {
		PELOGW(lim_log
			       (pMac, LOGW,
			       FL("Rcv invalid Measurement Request Action Frame "));
		       )
		return;
	}
	switch (pMeasReqFrame->measReqIE.measType) {
	case SIR_MAC_BASIC_MEASUREMENT_TYPE:
		__lim_process_basic_meas_req(pMac, pMeasReqFrame, pHdr->sa,
					     psessionEntry);
		break;
	case SIR_MAC_CCA_MEASUREMENT_TYPE:
		__lim_process_cca_meas_req(pMac, pMeasReqFrame, pHdr->sa,
					   psessionEntry);
		break;
	case SIR_MAC_RPI_MEASUREMENT_TYPE:
		__lim_process_rpi_meas_req(pMac, pMeasReqFrame, pHdr->sa,
					   psessionEntry);
		break;
	default:
		PELOG1(lim_log(pMac, LOG1, FL("Unknown Measurement Type %d "),
			       pMeasReqFrame->measReqIE.measType);
		       )
		break;
	}
} /*** end limProcessMeasurementRequestFrame ***/
static void
__lim_process_tpc_request_frame(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
				tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	uint8_t *pBody;
	tpSirMacTpcReqActionFrame pTpcReqFrame;
	uint32_t frameLen;
	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
	PELOG1(lim_log
		       (pMac, LOG1,
		       FL("****LIM: Processing TPC Request from peer ****"));
	       )
	pTpcReqFrame = qdf_mem_malloc(sizeof(tSirMacTpcReqActionFrame));
	if (NULL == pTpcReqFrame) {
		PELOGE(lim_log(pMac, LOGE, FL("AllocateMemory failed "));)
		return;
	}
	if (sir_convert_tpc_req_frame2_struct(pMac, pBody, pTpcReqFrame, frameLen) !=
	    eSIR_SUCCESS) {
		PELOGW(lim_log
			       (pMac, LOGW, FL("Rcv invalid TPC Req Action Frame "));
		       )
		return;
	}
	if (lim_send_tpc_report_frame(pMac,
				      pTpcReqFrame,
				      pHdr->sa, psessionEntry) != eSIR_SUCCESS) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("fail to send TPC Report Frame. "));
		       )
		return;
	}
}
#endif

static void
__lim_process_sm_power_save_update(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
				   tpPESession psessionEntry)
{

	tpSirMacMgmtHdr pHdr;
	tDot11fSMPowerSave frmSMPower;
	tSirMacHTMIMOPowerSaveState state;
	tpDphHashNode pSta;
	uint16_t aid;
	uint32_t frameLen, nStatus;
	uint8_t *pBody;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	pSta =
		dph_lookup_hash_entry(pMac, pHdr->sa, &aid,
				      &psessionEntry->dph.dphHashTable);
	if (pSta == NULL) {
		lim_log(pMac, LOGE,
			FL
				("STA context not found - ignoring UpdateSM PSave Mode from "));
		lim_print_mac_addr(pMac, pHdr->sa, LOGW);
		return;
	}

	/**Unpack the received frame */
	nStatus = dot11f_unpack_sm_power_save(pMac, pBody, frameLen, &frmSMPower);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL
				("Failed to unpack and parse a Update SM Power (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
		return;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while unpacking a SMPower Save update (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
	}

	lim_log(pMac, LOGW,
		FL("Received SM Power save Mode update Frame with PS_Enable:%d"
		   "PS Mode: %d"), frmSMPower.SMPowerModeSet.PowerSave_En,
		frmSMPower.SMPowerModeSet.Mode);

	/** Update in the DPH Table about the Update in the SM Power Save mode*/
	if (frmSMPower.SMPowerModeSet.PowerSave_En
	    && frmSMPower.SMPowerModeSet.Mode)
		state = eSIR_HT_MIMO_PS_DYNAMIC;
	else if ((frmSMPower.SMPowerModeSet.PowerSave_En)
		 && (frmSMPower.SMPowerModeSet.Mode == 0))
		state = eSIR_HT_MIMO_PS_STATIC;
	else if ((frmSMPower.SMPowerModeSet.PowerSave_En == 0)
		 && (frmSMPower.SMPowerModeSet.Mode == 0))
		state = eSIR_HT_MIMO_PS_NO_LIMIT;
	else {
		PELOGW(lim_log
			       (pMac, LOGW,
			       FL
				       ("Received SM Power save Mode update Frame with invalid mode"));
		       )
		return;
	}

	if (state == pSta->htMIMOPSState) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       FL("The PEER is already set in the same mode"));
		       )
		return;
	}

	/** Update in the HAL Station Table for the Update of the Protection Mode */
	pSta->htMIMOPSState = state;
	lim_post_sm_state_update(pMac, pSta->staIndex, pSta->htMIMOPSState,
				 pSta->staAddr, psessionEntry->smeSessionId);
}


static void
__lim_process_radio_measure_request(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
				    tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	tDot11fRadioMeasurementRequest *frm;
	uint32_t frameLen, nStatus;
	uint8_t *pBody;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	if (psessionEntry == NULL) {
		return;
	}

	lim_send_sme_mgmt_frame_ind(pMac, pHdr->fc.subType, (uint8_t *)pHdr,
		frameLen + sizeof(tSirMacMgmtHdr), 0,
		WMA_GET_RX_CH(pRxPacketInfo), psessionEntry, 0);

	frm = qdf_mem_malloc(sizeof(*frm));
	if (frm == NULL) {
		lim_log(pMac, LOGE,
			FL("Failed to alloc memory for tDot11fRadioMeasurementRequest"));
		return;
	}

	/**Unpack the received frame */
	nStatus = dot11f_unpack_radio_measurement_request(pMac,
								pBody,
								frameLen, frm);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to unpack and parse a Radio Measure request (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
		       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);)
		    goto err;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL("There were warnings while unpacking a Radio Measure request (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
		       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);)
	}
	/* Call rrm function to handle the request. */

	rrm_process_radio_measurement_request(pMac, pHdr->sa, frm,
					      psessionEntry);
err:
	qdf_mem_free(frm);
}

static void
__lim_process_link_measurement_req(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
				   tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	tDot11fLinkMeasurementRequest frm;
	uint32_t frameLen, nStatus;
	uint8_t *pBody;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	if (psessionEntry == NULL) {
		return;
	}

	/**Unpack the received frame */
	nStatus =
		dot11f_unpack_link_measurement_request(pMac, pBody, frameLen, &frm);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL
				("Failed to unpack and parse a Link Measure request (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
		return;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while unpacking a Link Measure request (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
	}
	/* Call rrm function to handle the request. */

	rrm_process_link_measurement_request(pMac, pRxPacketInfo, &frm,
					     psessionEntry);

}

static void
__lim_process_neighbor_report(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
			      tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	tDot11fNeighborReportResponse *pFrm;
	uint32_t frameLen, nStatus;
	uint8_t *pBody;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	pFrm = qdf_mem_malloc(sizeof(tDot11fNeighborReportResponse));
	if (NULL == pFrm) {
		lim_log(pMac, LOGE,
			FL
				("Unable to allocate memory in __lim_process_neighbor_report"));
		return;
	}

	if (psessionEntry == NULL) {
		qdf_mem_free(pFrm);
		return;
	}

	/**Unpack the received frame */
	nStatus =
		dot11f_unpack_neighbor_report_response(pMac, pBody, frameLen, pFrm);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL
				("Failed to unpack and parse a Neighbor report response (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
		qdf_mem_free(pFrm);
		return;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while unpacking a Neighbor report response (0x%08x, %d bytes):"),
			nStatus, frameLen);
		PELOG2(sir_dump_buf
			       (pMac, SIR_DBG_MODULE_ID, LOG2, pBody, frameLen);
		       )
	}
	/* Call rrm function to handle the request. */
	rrm_process_neighbor_report_response(pMac, pFrm, psessionEntry);

	qdf_mem_free(pFrm);
}


#ifdef WLAN_FEATURE_11W
/**
 * limProcessSAQueryRequestActionFrame
 *
 ***FUNCTION:
 * This function is called by lim_process_action_frame() upon
 * SA query request Action frame reception.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pRxPacketInfo - Handle to the Rx packet info
 * @param  psessionEntry - PE session entry
 *
 * @return None
 */
static void __lim_process_sa_query_request_action_frame(tpAniSirGlobal pMac,
							uint8_t *pRxPacketInfo,
							tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	uint8_t *pBody;
	uint8_t transId[2];

	/* Prima  --- Below Macro not available in prima
	   pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);
	   pBody = SIR_MAC_BD_TO_MPDUDATA(pBd); */

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);

	/* If this is an unprotected SA Query Request, then ignore it. */
	if (pHdr->fc.wep == 0)
		return;

	/* 11w offload is enabled then firmware should not fwd this frame */
	if (pMac->pmf_offload) {
		lim_log(pMac, LOGE,
			FL("11w offload is enabled, SA Query request is not expected ")
			);
		return;
	}

	/*Extract 11w trsansId from SA query request action frame
	   In SA query response action frame we will send same transId
	   In SA query request action frame:
	   Category       : 1 byte
	   Action         : 1 byte
	   Transaction ID : 2 bytes */
	qdf_mem_copy(&transId[0], &pBody[2], 2);

	/* Send 11w SA query response action frame */
	if (lim_send_sa_query_response_frame(pMac,
					     transId,
					     pHdr->sa,
					     psessionEntry) != eSIR_SUCCESS) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       FL("fail to send SA query response action frame."));
		       )
		return;
	}
}

/**
 * __lim_process_sa_query_response_action_frame
 *
 ***FUNCTION:
 * This function is called by lim_process_action_frame() upon
 * SA query response Action frame reception.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pRxPacketInfo - Handle to the Rx packet info
 * @param  psessionEntry - PE session entry
 * @return None
 */
static void __lim_process_sa_query_response_action_frame(tpAniSirGlobal pMac,
							 uint8_t *pRxPacketInfo,
							 tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	uint32_t frameLen;
	uint8_t *pBody;
	tpDphHashNode pSta;
	uint16_t aid;
	uint16_t transId;
	uint8_t retryNum;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
		  ("SA Query Response received..."));

	/* When a station, supplicant handles SA Query Response.
	 * Forward to SME to HDD to wpa_supplicant.
	 */
	if (LIM_IS_STA_ROLE(psessionEntry)) {
		lim_send_sme_mgmt_frame_ind(pMac, pHdr->fc.subType, (uint8_t *) pHdr,
					    frameLen + sizeof(tSirMacMgmtHdr), 0,
					    WMA_GET_RX_CH(pRxPacketInfo),
					    psessionEntry, 0);
		return;
	}

	/* If this is an unprotected SA Query Response, then ignore it. */
	if (pHdr->fc.wep == 0)
		return;

	pSta =
		dph_lookup_hash_entry(pMac, pHdr->sa, &aid,
				      &psessionEntry->dph.dphHashTable);
	if (NULL == pSta)
		return;

	lim_log(pMac, LOG1,
		FL("SA Query Response source addr - %0x:%0x:%0x:%0x:%0x:%0x"),
		pHdr->sa[0], pHdr->sa[1], pHdr->sa[2], pHdr->sa[3],
		pHdr->sa[4], pHdr->sa[5]);
	lim_log(pMac, LOG1,
		FL("SA Query state for station - %d"), pSta->pmfSaQueryState);

	if (DPH_SA_QUERY_IN_PROGRESS != pSta->pmfSaQueryState)
		return;

	/* Extract 11w trsansId from SA query reponse action frame
	   In SA query response action frame:
	   Category       : 1 byte
	   Action         : 1 byte
	   Transaction ID : 2 bytes */
	qdf_mem_copy(&transId, &pBody[2], 2);

	/* If SA Query is in progress with the station and the station
	   responds then the association request that triggered the SA
	   query is from a rogue station, just go back to initial state. */
	for (retryNum = 0; retryNum <= pSta->pmfSaQueryRetryCount; retryNum++)
		if (transId == pSta->pmfSaQueryStartTransId + retryNum) {
			lim_log(pMac, LOG1,
				FL
					("Found matching SA Query Request - transaction ID %d"),
				transId);
			tx_timer_deactivate(&pSta->pmfSaQueryTimer);
			pSta->pmfSaQueryState = DPH_SA_QUERY_NOT_IN_PROGRESS;
			break;
		}
}
#endif

#ifdef WLAN_FEATURE_11W
/**
 * lim_drop_unprotected_action_frame
 *
 ***FUNCTION:
 * This function checks if an Action frame should be dropped since it is
 * a Robust Managment Frame, it is unprotected, and it is received on a
 * connection where PMF is enabled.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Global MAC structure
 * @param  psessionEntry - PE session entry
 * @param  pHdr - Frame header
 * @param  category - Action frame category
 * @return true if frame should be dropped
 */

static bool
lim_drop_unprotected_action_frame(tpAniSirGlobal pMac, tpPESession psessionEntry,
				  tpSirMacMgmtHdr pHdr, uint8_t category)
{
	uint16_t aid;
	tpDphHashNode pStaDs;
	bool rmfConnection = false;

	if (LIM_IS_AP_ROLE(psessionEntry)) {
		pStaDs =
			dph_lookup_hash_entry(pMac, pHdr->sa, &aid,
					      &psessionEntry->dph.dphHashTable);
		if (pStaDs != NULL)
			if (pStaDs->rmfEnabled)
				rmfConnection = true;
	} else if (psessionEntry->limRmfEnabled)
		rmfConnection = true;

	if (rmfConnection && (pHdr->fc.wep == 0)) {
		lim_log(pMac, LOGE,
			       FL("Dropping unprotected Action category %d frame since RMF is enabled."), category);
		return true;
	} else
		return false;
}
#endif

/**
 * lim_process_action_frame() - to process action frames
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_pkt_info: A pointer to packet info structure
 *
 * This function is called by limProcessMessageQueue() upon
 * Action frame reception.
 *
 * Return: none
 */

void lim_process_action_frame(tpAniSirGlobal mac_ctx,
		uint8_t *rx_pkt_info, tpPESession session)
{
	uint8_t *body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	tpSirMacActionFrameHdr action_hdr = (tpSirMacActionFrameHdr) body_ptr;
#ifdef WLAN_FEATURE_11W
	tpSirMacMgmtHdr mac_hdr_11w = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
#endif
	tpSirMacMgmtHdr mac_hdr = NULL;
	int8_t rssi;
	uint32_t frame_len;
	tpSirMacVendorSpecificFrameHdr vendor_specific;
	uint8_t oui[] = { 0x00, 0x00, 0xf0 };
	tpSirMacVendorSpecificPublicActionFrameHdr pub_action;
	uint8_t p2p_oui[] = { 0x50, 0x6F, 0x9A, 0x09 };

#ifdef WLAN_FEATURE_11W
	if (lim_is_robust_mgmt_action_frame(action_hdr->category) &&
	   lim_drop_unprotected_action_frame(mac_ctx, session,
			mac_hdr_11w, action_hdr->category))
		return;
#endif

	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	switch (action_hdr->category) {
	case SIR_MAC_ACTION_QOS_MGMT:
		if ((session->limQosEnabled) ||
		    (action_hdr->actionID == SIR_MAC_QOS_MAP_CONFIGURE)) {
			switch (action_hdr->actionID) {
			case SIR_MAC_QOS_ADD_TS_REQ:
				__lim_process_add_ts_req(mac_ctx,
						(uint8_t *) rx_pkt_info,
						session);
				break;

			case SIR_MAC_QOS_ADD_TS_RSP:
				__lim_process_add_ts_rsp(mac_ctx,
						 (uint8_t *) rx_pkt_info,
						 session);
				break;

			case SIR_MAC_QOS_DEL_TS_REQ:
				__lim_process_del_ts_req(mac_ctx,
						(uint8_t *) rx_pkt_info,
						session);
				break;

			case SIR_MAC_QOS_MAP_CONFIGURE:
				__lim_process_qos_map_configure_frame(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
				break;
			default:
				lim_log(mac_ctx, LOG1,
					FL("Qos action %d not handled"),
					action_hdr->actionID);
				break;
			}
			break;
		}
		break;

	case SIR_MAC_ACTION_SPECTRUM_MGMT:
		switch (action_hdr->actionID) {
#ifdef ANI_SUPPORT_11H
		case SIR_MAC_ACTION_MEASURE_REQUEST_ID:
			if (session->lim11hEnable)
				__lim_process_measurement_request_frame(mac_ctx,
							rx_pkt_info,
							session);
			break;
		case SIR_MAC_ACTION_TPC_REQUEST_ID:
			if ((LIM_IS_STA_ROLE(session) ||
				LIM_IS_AP_ROLE(session)) &&
				session->lim11hEnable)
					__lim_process_tpc_request_frame(mac_ctx,
						rx_pkt_info, session);
			break;
#endif
		case SIR_MAC_ACTION_CHANNEL_SWITCH_ID:
			if (LIM_IS_STA_ROLE(session))
				__lim_process_channel_switch_action_frame(
					mac_ctx, rx_pkt_info, session);
			break;
		default:
			lim_log(mac_ctx, LOG1,
				FL("Spectrum mgmt action id %d not handled"),
				action_hdr->actionID);
			break;
		}
		break;

	case SIR_MAC_ACTION_WME:
		if (!session->limWmeEnabled) {
			lim_log(mac_ctx, LOGW,
				FL("WME mode disabled - dropping frame %d"),
				action_hdr->actionID);
			break;
		}
		switch (action_hdr->actionID) {
		case SIR_MAC_QOS_ADD_TS_REQ:
			__lim_process_add_ts_req(mac_ctx,
				(uint8_t *) rx_pkt_info, session);
			break;

		case SIR_MAC_QOS_ADD_TS_RSP:
			__lim_process_add_ts_rsp(mac_ctx,
				(uint8_t *) rx_pkt_info, session);
			break;

		case SIR_MAC_QOS_DEL_TS_REQ:
			__lim_process_del_ts_req(mac_ctx,
				(uint8_t *) rx_pkt_info, session);
			break;

		case SIR_MAC_QOS_MAP_CONFIGURE:
			__lim_process_qos_map_configure_frame(mac_ctx,
				(uint8_t *)rx_pkt_info, session);
			break;

		default:
			lim_log(mac_ctx, LOG1,
				FL("WME action %d not handled"),
				action_hdr->actionID);
			break;
		}
		break;

	case SIR_MAC_ACTION_HT:
		/** Type of HT Action to be performed*/
		switch (action_hdr->actionID) {
		case SIR_MAC_SM_POWER_SAVE:
			if (LIM_IS_AP_ROLE(session))
				__lim_process_sm_power_save_update(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
			break;
		default:
			lim_log(mac_ctx, LOG1,
				FL("Action ID %d not handled in HT category"),
				action_hdr->actionID);
			break;
		}
		break;

	case SIR_MAC_ACTION_WNM:
		lim_log(mac_ctx, LOG1,
			FL("WNM Action category %d action %d."),
			action_hdr->category, action_hdr->actionID);
		switch (action_hdr->actionID) {
		case SIR_MAC_WNM_BSS_TM_QUERY:
		case SIR_MAC_WNM_BSS_TM_REQUEST:
		case SIR_MAC_WNM_BSS_TM_RESPONSE:
		case SIR_MAC_WNM_NOTIF_REQUEST:
		case SIR_MAC_WNM_NOTIF_RESPONSE:
			rssi = WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
			mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
			/* Forward to the SME to HDD to wpa_supplicant */
			lim_send_sme_mgmt_frame_ind(mac_ctx,
					mac_hdr->fc.subType,
					(uint8_t *) mac_hdr,
					frame_len + sizeof(tSirMacMgmtHdr),
					session->smeSessionId,
					WMA_GET_RX_CH(rx_pkt_info),
					session, rssi);
			break;
		default:
			lim_log(mac_ctx, LOG1,
				FL("Action ID %d not handled in WNM category"),
				action_hdr->actionID);
			break;
		}
		break;

	case SIR_MAC_ACTION_RRM:
		/* Ignore RRM measurement request until DHCP is set */
		if (mac_ctx->rrm.rrmPEContext.rrmEnable &&
		    mac_ctx->roam.roamSession
		    [session->smeSessionId].dhcp_done) {
			switch (action_hdr->actionID) {
			case SIR_MAC_RRM_RADIO_MEASURE_REQ:
				__lim_process_radio_measure_request(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
				break;
			case SIR_MAC_RRM_LINK_MEASUREMENT_REQ:
				__lim_process_link_measurement_req(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
				break;
			case SIR_MAC_RRM_NEIGHBOR_RPT:
				__lim_process_neighbor_report(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
				break;
			default:
				lim_log(mac_ctx, LOG1,
					FL("Action ID %d not handled in RRM"),
					action_hdr->actionID);
				break;

			}
		} else {
			/* Else we will just ignore the RRM messages. */
			lim_log(mac_ctx, LOG1,
			  FL("RRM frm ignored, it is disabled in cfg %d or DHCP not completed %d"),
			  mac_ctx->rrm.rrmPEContext.rrmEnable,
			  mac_ctx->roam.roamSession
			  [session->smeSessionId].dhcp_done);
		}
		break;

	case SIR_MAC_ACTION_VENDOR_SPECIFIC_CATEGORY:
		vendor_specific = (tpSirMacVendorSpecificFrameHdr) action_hdr;
		mac_hdr = NULL;
		frame_len = 0;

		mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
		frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

		/* Check if it is a vendor specific action frame. */
		if (LIM_IS_STA_ROLE(session) &&
		    (!qdf_mem_cmp(session->selfMacAddr,
					&mac_hdr->da[0], sizeof(tSirMacAddr)))
		    && IS_WES_MODE_ENABLED(mac_ctx)
		    && !qdf_mem_cmp(vendor_specific->Oui, oui, 3)) {
			lim_log(mac_ctx, LOGW,
				FL("Rcvd Vendor specific frame, OUI %x %x %x"),
				vendor_specific->Oui[0],
				vendor_specific->Oui[1],
				vendor_specific->Oui[2]);
			/*
			 * Forward to the SME to HDD to wpa_supplicant
			 * type is ACTION
			 */
			lim_send_sme_mgmt_frame_ind(mac_ctx,
					mac_hdr->fc.subType,
					(uint8_t *) mac_hdr,
					frame_len +
					sizeof(tSirMacMgmtHdr),
					session->smeSessionId,
					WMA_GET_RX_CH(rx_pkt_info),
					session, 0);
		} else {
			lim_log(mac_ctx, LOG1,
				FL("Dropping the vendor specific action frame"
					"beacause of (WES Mode not enabled "
					"(WESMODE = %d) or OUI mismatch "
					"(%02x %02x %02x) or not received with"
					"SelfSta address) system role = %d"),
				IS_WES_MODE_ENABLED(mac_ctx),
				vendor_specific->Oui[0],
				vendor_specific->Oui[1],
				vendor_specific->Oui[2],
				GET_LIM_SYSTEM_ROLE(session));
		}
	break;
	case SIR_MAC_ACTION_PUBLIC_USAGE:
		switch (action_hdr->actionID) {
		case SIR_MAC_ACTION_VENDOR_SPECIFIC:
			pub_action =
				(tpSirMacVendorSpecificPublicActionFrameHdr)
				action_hdr;
			mac_hdr = NULL;
			frame_len = 0;

			mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
			frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
			/* Check if it is a P2P public action frame. */
			if (!qdf_mem_cmp(pub_action->Oui, p2p_oui, 4)) {
				/*
				 * Forward to the SME to HDD to wpa_supplicant
				 * type is ACTION
				 */
				lim_send_sme_mgmt_frame_ind(mac_ctx,
						mac_hdr->fc.subType,
						(uint8_t *) mac_hdr,
						frame_len +
						sizeof(tSirMacMgmtHdr),
						session->smeSessionId,
						WMA_GET_RX_CH(rx_pkt_info),
						session, 0);
			} else {
				lim_log(mac_ctx, LOG1,
					FL("Unhandled public action frame (Vendor specific). OUI %x %x %x %x"),
					pub_action->Oui[0], pub_action->Oui[1],
					pub_action->Oui[2], pub_action->Oui[3]);
			}
		break;

		case SIR_MAC_ACTION_2040_BSS_COEXISTENCE:
			mac_hdr = NULL;
			frame_len = 0;

			mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
			frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

			lim_send_sme_mgmt_frame_ind(mac_ctx,
					mac_hdr->fc.subType,
					(uint8_t *) mac_hdr,
					frame_len + sizeof(tSirMacMgmtHdr),
					session->smeSessionId,
					WMA_GET_RX_CH(rx_pkt_info), session, 0);
		break;
#ifdef FEATURE_WLAN_TDLS
		case SIR_MAC_TDLS_DIS_RSP:
			mac_hdr = NULL;
			frame_len = 0;
			rssi = 0;

			mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
			frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
			rssi = WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
			QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_INFO,
				  ("Public Action TDLS Discovery RSP .."));
			lim_send_sme_mgmt_frame_ind(mac_ctx,
				mac_hdr->fc.subType, (uint8_t *) mac_hdr,
				frame_len + sizeof(tSirMacMgmtHdr),
				session->smeSessionId,
				WMA_GET_RX_CH(rx_pkt_info), session, rssi);
		break;
#endif
		case SIR_MAC_ACTION_EXT_CHANNEL_SWITCH_ID:
			lim_process_ext_channel_switch_action_frame(mac_ctx,
							rx_pkt_info, session);
			break;
		default:
			lim_log(mac_ctx, LOG1,
				FL("Unhandled public action frame -- %x "),
				action_hdr->actionID);
			break;
		}
		break;

#ifdef WLAN_FEATURE_11W
	case SIR_MAC_ACTION_SA_QUERY:
		lim_log(mac_ctx, LOG1,
			FL("SA Query Action category %d action %d."),
			action_hdr->category, action_hdr->actionID);
		switch (action_hdr->actionID) {
		case SIR_MAC_SA_QUERY_REQ:
			/**11w SA query request action frame received**/
			/* Respond directly to the incoming request in LIM */
			__lim_process_sa_query_request_action_frame(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
			break;
		case SIR_MAC_SA_QUERY_RSP:
			/**11w SA query response action frame received**/
			/* Handle based on the current SA Query state */
			__lim_process_sa_query_response_action_frame(mac_ctx,
						(uint8_t *)rx_pkt_info,
						session);
			break;
		default:
			break;
		}
		break;
#endif
	case SIR_MAC_ACTION_VHT:
		if (!session->vhtCapability)
			break;
		switch (action_hdr->actionID) {
		case SIR_MAC_VHT_OPMODE_NOTIFICATION:
			__lim_process_operating_mode_action_frame(mac_ctx,
					rx_pkt_info, session);
			break;
		case SIR_MAC_VHT_GID_NOTIFICATION:
			/* Only if ini supports it */
			if (session->enableVhtGid)
				__lim_process_gid_management_action_frame(
					mac_ctx, rx_pkt_info, session);
			break;
		default:
			break;
		}
		break;
	case SIR_MAC_ACTION_FST: {
		tpSirMacMgmtHdr     hdr;
		uint32_t            frame_len;

		hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
		frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

		lim_log(mac_ctx, LOG1, FL("Received FST MGMT action frame"));
		/* Forward to the SME to HDD */
		lim_send_sme_mgmt_frame_ind(mac_ctx, hdr->fc.subType,
					    (uint8_t *)hdr,
					    frame_len + sizeof(tSirMacMgmtHdr),
					    session->smeSessionId,
					    WMA_GET_RX_CH(rx_pkt_info),
					    session, 0);
		break;
	}
	case SIR_MAC_ACTION_PROT_DUAL_PUB:
		lim_log(mac_ctx, LOG1,
			FL("Rcvd Protected Dual of Public Action; action %d."),
			action_hdr->actionID);
		switch (action_hdr->actionID) {
		case SIR_MAC_PDPA_GAS_INIT_REQ:
		case SIR_MAC_PDPA_GAS_INIT_RSP:
		case SIR_MAC_PDPA_GAS_COMEBACK_REQ:
		case SIR_MAC_PDPA_GAS_COMEBACK_RSP:
			mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
			frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
			rssi = WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
			lim_send_sme_mgmt_frame_ind(mac_ctx,
				mac_hdr->fc.subType, (uint8_t *) mac_hdr,
				frame_len + sizeof(tSirMacMgmtHdr),
				session->smeSessionId,
				WMA_GET_RX_CH(rx_pkt_info), session, rssi);
			break;
		default:
			lim_log(mac_ctx, LOG1,
				FL("Unhandled - Protected Dual Public Action"));
			break;
		}
		break;
	default:
		lim_log(mac_ctx, LOG1,
			FL("Action category %d not handled"),
			action_hdr->category);
		break;
	}
}

/**
 * lim_process_action_frame_no_session
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Action frame reception and no session.
 * Currently only public action frames can be received from
 * a non-associated station.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pBd - A pointer to Buffer descriptor + associated PDUs
 * @return None
 */

void lim_process_action_frame_no_session(tpAniSirGlobal pMac, uint8_t *pBd)
{
	uint8_t *pBody = WMA_GET_RX_MPDU_DATA(pBd);
	tpSirMacVendorSpecificPublicActionFrameHdr pActionHdr =
		(tpSirMacVendorSpecificPublicActionFrameHdr) pBody;

	lim_log(pMac, LOG1, "Received a Action frame -- no session");

	switch (pActionHdr->category) {
	case SIR_MAC_ACTION_PUBLIC_USAGE:
		switch (pActionHdr->actionID) {
		case SIR_MAC_ACTION_VENDOR_SPECIFIC:
		{
			tpSirMacMgmtHdr pHdr;
			uint32_t frameLen;
			uint8_t P2POui[] = { 0x50, 0x6F, 0x9A, 0x09 };

			pHdr = WMA_GET_RX_MAC_HEADER(pBd);
			frameLen = WMA_GET_RX_PAYLOAD_LEN(pBd);

			/* Check if it is a P2P public action frame. */
			if (!qdf_mem_cmp(pActionHdr->Oui, P2POui, 4)) {
				/* Forward to the SME to HDD to wpa_supplicant */
				/* type is ACTION */
				lim_send_sme_mgmt_frame_ind(pMac,
							    pHdr->fc.subType,
							    (uint8_t *) pHdr,
							    frameLen +
							    sizeof
							    (tSirMacMgmtHdr),
							    0,
							    WMA_GET_RX_CH
								    (pBd), NULL, 0);
			} else {
				lim_log(pMac, LOG1,
					FL
						("Unhandled public action frame (Vendor specific). OUI %x %x %x %x"),
					pActionHdr->Oui[0],
					pActionHdr->Oui[1],
					pActionHdr->Oui[2],
					pActionHdr->Oui[3]);
			}
		}
		break;
		default:
			PELOGE(lim_log
				       (pMac, LOG1,
				       FL("Unhandled public action frame -- %x "),
				       pActionHdr->actionID);
			       )
			break;
		}
		break;
	default:
		PELOGE(lim_log
			       (pMac, LOG1,
			       FL("Unhandled action frame without session -- %x "),
			       pActionHdr->category);
		       )
		break;

	}
}
