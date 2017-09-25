/*
 * Copyright (c) 2012-2014,2016-2017 The Linux Foundation. All rights reserved.
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

/*===========================================================================
			L I M _ P 2 P . C

   OVERVIEW:

   This software unit holds the implementation of the WLAN Protocol Engine for
   P2P.
   ===========================================================================*/

/*===========================================================================

			EDIT HISTORY FOR FILE

   This section contains comments describing changes made to the module.
   Notice that changes are listed in reverse chronological order.

   $Header$$DateTime$$Author$

   when        who     what, where, why
   ----------    ---    --------------------------------------------------------
   2011-05-02    djindal Corrected file indentation and changed remain on channel
			handling for concurrency.
   ===========================================================================*/

#include "lim_utils.h"
#include "lim_session_utils.h"
#include "wma_types.h"
#include "lim_types.h"
#include "wlan_utility.h"

#define   PROBE_RSP_IE_OFFSET    36
#define   BSSID_OFFSET           16
#define   ADDR2_OFFSET           10
#define   ACTION_OFFSET          24

/* A DFS channel can be ACTIVE for max 9000 msec, from the last
   received Beacon/Prpbe Resp. */
#define   MAX_TIME_TO_BE_ACTIVE_CHANNEL 9000

void lim_exit_remain_on_channel(tpAniSirGlobal pMac, QDF_STATUS status,
				uint32_t *data, tpPESession psessionEntry);
extern tSirRetStatus lim_set_link_state(tpAniSirGlobal pMac, tSirLinkState state,
					tSirMacAddr bssId, tSirMacAddr selfMacAddr,
					tpSetLinkStateCallback callback,
					void *callbackArg);

/*------------------------------------------------------------------
 *
 * Below function is called if hdd requests a remain on channel.
 *
 *------------------------------------------------------------------*/
static QDF_STATUS lim_send_hal_req_remain_on_chan_offload(tpAniSirGlobal pMac,
							  tSirRemainOnChnReq *
							  pRemOnChnReq)
{
	tSirScanOffloadReq *pScanOffloadReq;
	struct scheduler_msg msg = {0};
	tSirRetStatus rc = eSIR_SUCCESS;

	pScanOffloadReq = qdf_mem_malloc(sizeof(tSirScanOffloadReq));
	if (NULL == pScanOffloadReq) {
		pe_err("Memory allocation failed for pScanOffloadReq");
		return QDF_STATUS_E_NOMEM;
	}

	msg.type = WMA_START_SCAN_OFFLOAD_REQ;
	msg.bodyptr = pScanOffloadReq;
	msg.bodyval = 0;

	qdf_copy_macaddr(&pScanOffloadReq->selfMacAddr,
			 &pRemOnChnReq->selfMacAddr);

	qdf_set_macaddr_broadcast(&pScanOffloadReq->bssId);
	pScanOffloadReq->scanType = eSIR_PASSIVE_SCAN;
	pScanOffloadReq->p2pScanType = P2P_SCAN_TYPE_LISTEN;
	pScanOffloadReq->minChannelTime = pRemOnChnReq->duration;
	pScanOffloadReq->maxChannelTime = pRemOnChnReq->duration;
	pScanOffloadReq->sessionId = pRemOnChnReq->sessionId;
	pScanOffloadReq->channelList.numChannels = 1;
	pScanOffloadReq->channelList.channelNumber[0] = pRemOnChnReq->chnNum;
	pScanOffloadReq->scan_id = pRemOnChnReq->scan_id;
	pScanOffloadReq->scan_requestor_id = ROC_SCAN_REQUESTOR_ID;

	pe_debug("Req-rem-on-channel: duration: %u session: %hu chan: %hu",
		pRemOnChnReq->duration, pRemOnChnReq->sessionId,
		pRemOnChnReq->chnNum);

	rc = wma_post_ctrl_msg(pMac, &msg);
	if (rc != eSIR_SUCCESS) {
		pe_err("wma_post_ctrl_msg() return failure: %u", rc);
		qdf_mem_free(pScanOffloadReq);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*------------------------------------------------------------------
 *
 * Remain on channel req handler. Initiate the INIT_SCAN, CHN_CHANGE
 * and SET_LINK Request from SME, chnNum and duration to remain on channel.
 *
 *------------------------------------------------------------------*/
int lim_process_remain_on_chnl_req(tpAniSirGlobal pMac, uint32_t *pMsg)
{
	tSirRemainOnChnReq *msgbuff = (tSirRemainOnChnReq *) pMsg;
	QDF_STATUS status;

	pMac->lim.gpLimRemainOnChanReq = msgbuff;
	status = lim_send_hal_req_remain_on_chan_offload(pMac, msgbuff);
	if (status != QDF_STATUS_SUCCESS) {
		/* Post the meessage to Sme */
		lim_send_sme_rsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP,
				 eSIR_SME_HAL_SEND_MESSAGE_FAIL,
				 msgbuff->sessionId,
				 msgbuff->scan_id);
		qdf_mem_free(pMac->lim.gpLimRemainOnChanReq);
		pMac->lim.gpLimRemainOnChanReq = NULL;
	}
	return false;
}

/*------------------------------------------------------------------
 *
 * lim Insert NOA timer timeout callback - when timer fires, deactivate it and send
 * scan rsp to csr/hdd
 *
 *------------------------------------------------------------------*/
void lim_process_insert_single_shot_noa_timeout(tpAniSirGlobal pMac)
{
	/* timeout means start NOA did not arrive; we need to deactivate and change the timer for
	 * future activations
	 */
	lim_deactivate_and_change_timer(pMac, eLIM_INSERT_SINGLESHOT_NOA_TIMER);

	/* Even if insert NOA timedout, go ahead and process/send stored SME request */
	lim_process_regd_defd_sme_req_after_noa_start(pMac);

	return;
}

/*-----------------------------------------------------------------
 * lim Insert Timer callback function to check active DFS channels
 * and convert them to passive channels if there was no
 * beacon/proberesp for MAX_TIME_TO_BE_ACTIVE_CHANNEL time
 *------------------------------------------------------------------*/
void lim_convert_active_channel_to_passive_channel(tpAniSirGlobal pMac)
{
	uint64_t currentTime;
	uint64_t lastTime = 0;
	uint64_t timeDiff;
	uint8_t i;
	currentTime = (uint64_t)qdf_mc_timer_get_system_time();
	for (i = 1; i < SIR_MAX_24G_5G_CHANNEL_RANGE; i++) {
		if ((pMac->lim.dfschannelList.timeStamp[i]) != 0) {
			lastTime = pMac->lim.dfschannelList.timeStamp[i];
			if (currentTime >= lastTime) {
				timeDiff = (currentTime - lastTime);
			} else {
				timeDiff =
					(0xFFFFFFFF - lastTime) + currentTime;
			}

			if (timeDiff >= MAX_TIME_TO_BE_ACTIVE_CHANNEL) {
				lim_covert_channel_scan_type(pMac, i, false);
				pMac->lim.dfschannelList.timeStamp[i] = 0;
			}
		}
	}
	/* lastTime is zero if there is no DFS active channels in the list.
	 * If this is non zero then we have active DFS channels so restart the timer.
	 */
	if (lastTime != 0) {
		if (tx_timer_activate
			    (&pMac->lim.limTimers.gLimActiveToPassiveChannelTimer)
		    != TX_SUCCESS) {
			pe_err("Could not activate Active to Passive Channel  timer");
		}
	}

	return;

}

/**
 * lim_process_remain_on_chn_timeout() - ROC timeout handler
 *
 * @mac_ctx: MAC context
 *
 * limchannelchange callback, on success channel change, set the
 * link_state to LISTEN
 *
 * Return: NULL
 */
void lim_process_remain_on_chn_timeout(tpAniSirGlobal mac_ctx)
{
	tpPESession session;
	tSirMacAddr null_bssid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	TX_TIMER *roc_timer;

	roc_timer = &mac_ctx->lim.limTimers.gLimRemainOnChannelTimer;
	/*
	 * Timer might get extended while Sending Action Frame
	 * In that case don't process Channel Timeout
	 */
	if (tx_timer_running(roc_timer)) {
		pe_err("already timer is running and not processing ");
		return;
	}

	lim_deactivate_and_change_timer(mac_ctx, eLIM_REMAIN_CHN_TIMER);
	if (NULL == mac_ctx->lim.gpLimRemainOnChanReq) {
		pe_err("No Remain on channel pending");
		return;
	}

	/* get the previous valid LINK state */
	if (lim_set_link_state(mac_ctx, eSIR_LINK_IDLE_STATE, null_bssid,
		mac_ctx->lim.gSelfMacAddr, NULL, NULL) != eSIR_SUCCESS) {
		pe_err("Unable to change link state");
		return;
	}

	if (mac_ctx->lim.gLimMlmState != eLIM_MLM_P2P_LISTEN_STATE) {
		lim_remain_on_chn_rsp(mac_ctx, QDF_STATUS_SUCCESS, NULL);
	} else {
		session = pe_find_session_by_session_id(mac_ctx,
			roc_timer->sessionId);
		/* get the session */
		if (session == NULL) {
			pe_err("Session Does not exist sessionID: %d",
				roc_timer->sessionId);
			goto error;
		}

		lim_exit_remain_on_channel(mac_ctx, QDF_STATUS_SUCCESS, NULL,
			session);
		return;
error:
		lim_remain_on_chn_rsp(mac_ctx, QDF_STATUS_E_FAILURE, NULL);
	}
	return;
}

/*------------------------------------------------------------------
 *
 * limchannelchange callback, on success channel change, set the link_state
 * to LISTEN
 *
 *------------------------------------------------------------------*/

void lim_exit_remain_on_channel(tpAniSirGlobal pMac, QDF_STATUS status,
				uint32_t *data, tpPESession psessionEntry)
{

	if (status != QDF_STATUS_SUCCESS) {
		pe_err("Remain on Channel Failed");
		goto error;
	}
	/* Set the resume channel to Any valid channel (invalid). */
	/* This will instruct HAL to set it to any previous valid channel. */
	pe_set_resume_channel(pMac, 0, 0);
	lim_remain_on_chn_rsp(pMac, QDF_STATUS_SUCCESS, NULL);
	return;
error:
	lim_remain_on_chn_rsp(pMac, QDF_STATUS_E_FAILURE, NULL);
	return;
}

/*------------------------------------------------------------------
 *
 * Send remain on channel respone: Success/ Failure
 *
 *------------------------------------------------------------------*/
void lim_remain_on_chn_rsp(tpAniSirGlobal pMac, QDF_STATUS status, uint32_t *data)
{
	tpPESession psessionEntry;
	uint8_t sessionId;
	tSirRemainOnChnReq *MsgRemainonChannel = pMac->lim.gpLimRemainOnChanReq;
	tSirMacAddr nullBssid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (NULL == MsgRemainonChannel) {
		pe_err("%s: No Pointer for Remain on Channel Req", __func__);
		return;
	}
	/* Incase of the Remain on Channel Failure Case */
	/* Cleanup Everything */
	if (QDF_STATUS_E_FAILURE == status) {
		/* Deactivate Remain on Channel Timer */
		lim_deactivate_and_change_timer(pMac, eLIM_REMAIN_CHN_TIMER);

		/* Set the Link State to Idle */
		/* get the previous valid LINK state */
		if (lim_set_link_state(pMac, eSIR_LINK_IDLE_STATE, nullBssid,
				       pMac->lim.gSelfMacAddr, NULL,
				       NULL) != eSIR_SUCCESS) {
			pe_err("Unable to change link state");
		}

		pMac->lim.gLimSystemInScanLearnMode = 0;
		pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
		SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	}

	/* delete the session */
	psessionEntry = pe_find_session_by_bssid(pMac,
						      MsgRemainonChannel->
						      selfMacAddr.bytes,
						      &sessionId);
	if (psessionEntry != NULL) {
		if (LIM_IS_P2P_DEVICE_ROLE(psessionEntry)) {
			pe_delete_session(pMac, psessionEntry);
		}
	}

	/* Post the meessage to Sme */
	if (QDF_IS_STATUS_SUCCESS(status))
		lim_send_sme_rsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP,
				eSIR_SME_SUCCESS,
				MsgRemainonChannel->sessionId,
				0);
	else
		lim_send_sme_rsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP,
				eSIR_SME_REFUSED,
				MsgRemainonChannel->sessionId,
				0);

	qdf_mem_free(pMac->lim.gpLimRemainOnChanReq);
	pMac->lim.gpLimRemainOnChanReq = NULL;

	pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;

	/* If remain on channel timer expired and action frame is pending then
	 * indicaiton confirmation with status failure */
	if (pMac->lim.mgmtFrameSessionId != 0xff)
		lim_p2p_action_cnf(pMac, NULL, false, NULL);

	return;
}

/*------------------------------------------------------------------
 *
 * Indicate the Mgmt Frame received to SME to HDD callback
 * handle Probe_req/Action frame currently
 *
 *------------------------------------------------------------------*/
void lim_send_sme_mgmt_frame_ind(tpAniSirGlobal pMac, uint8_t frameType,
				 uint8_t *frame, uint32_t frameLen,
				 uint16_t sessionId, uint32_t rxChannel,
				 tpPESession psessionEntry, int8_t rxRssi)
{
	tpSirSmeMgmtFrameInd pSirSmeMgmtFrame = NULL;
	uint16_t length;

	length = sizeof(tSirSmeMgmtFrameInd) + frameLen;

	pSirSmeMgmtFrame = qdf_mem_malloc(length);
	if (NULL == pSirSmeMgmtFrame) {
		pe_err("AllocateMemory failed for eWNI_SME_LISTEN_RSP");
		return;
	}

	pSirSmeMgmtFrame->frame_len = frameLen;
	pSirSmeMgmtFrame->sessionId = sessionId;
	pSirSmeMgmtFrame->frameType = frameType;
	pSirSmeMgmtFrame->rxRssi = rxRssi;
	pSirSmeMgmtFrame->rxChan = rxChannel;

	qdf_mem_zero(pSirSmeMgmtFrame->frameBuf, frameLen);
	qdf_mem_copy(pSirSmeMgmtFrame->frameBuf, frame, frameLen);

	if (pMac->mgmt_frame_ind_cb)
		pMac->mgmt_frame_ind_cb(pSirSmeMgmtFrame);
	else
		pe_warn("Management indication callback not registered!!");
	qdf_mem_free(pSirSmeMgmtFrame);
	return;
}

QDF_STATUS lim_p2p_action_cnf(void *context, qdf_nbuf_t buf,
			      uint32_t tx_status, void *params)
{
	QDF_STATUS status;
	uint32_t mgmt_frame_sessionId;
	bool tx_complete_ack = (tx_status) ? true : false;
	tpAniSirGlobal pMac = (tpAniSirGlobal)context;

	status = pe_acquire_global_lock(&pMac->lim);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mgmt_frame_sessionId = pMac->lim.mgmtFrameSessionId;
		pMac->lim.mgmtFrameSessionId = 0xff;
		pe_release_global_lock(&pMac->lim);

		if (mgmt_frame_sessionId != 0xff) {
			/*
			 * The session entry might be invalid(0xff)
			 * action confirmation received after
			 * remain on channel timer expired.
			 */
			pe_debug("mgmt_frame_sessionId: %d",
					 mgmt_frame_sessionId);
			if (pMac->p2p_ack_ind_cb)
				pMac->p2p_ack_ind_cb(mgmt_frame_sessionId,
						     tx_complete_ack);
		}
	}

	if (buf)
		qdf_nbuf_free(buf);

	return status;
}

/**
 * lim_tx_action_frame() - Handles action frame transmission request
 * @mac_ctx:  Pointer to mac context
 * @mb_msg:  message sent from SME
 * @msg_len: Message length
 * @packet: network buffer for TX
 * @frame: frame buffer
 *
 * This function processes the action frame transmission request and
 * posts message to WMA.
 *
 * Return: NULL
 */
static void lim_tx_action_frame(tpAniSirGlobal mac_ctx,
	tSirMbMsgP2p *mb_msg, uint32_t msg_len, void *packet, uint8_t *frame)
{
	uint8_t tx_flag = 0;
	tpSirMacFrameCtl fc = (tpSirMacFrameCtl) mb_msg->data;
	QDF_STATUS qdf_status;
	uint8_t sme_session_id = 0;
	uint16_t channel_freq;

	sme_session_id = mb_msg->sessionId;
	channel_freq = mb_msg->channel_freq;
	/*
	 * Use BD rate 2 for all P2P related frames. As these frames
	 * need to go at OFDM rates. And BD rate2 we configured at 6Mbps.
	 */
	tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	mac_ctx->lim.mgmtFrameSessionId = sme_session_id;

	if ((SIR_MAC_MGMT_PROBE_RSP == fc->subType) ||
		(mb_msg->noack)) {
		qdf_status = wma_tx_frame(mac_ctx, packet, (uint16_t) msg_len,
			TXRX_FRM_802_11_MGMT,
			ANI_TXDIR_TODS, 7, lim_tx_complete,
			frame, tx_flag, sme_session_id,
			channel_freq, RATEID_DEFAULT);

		if (!mb_msg->noack)
			lim_p2p_action_cnf(mac_ctx, packet,
				(QDF_IS_STATUS_SUCCESS(qdf_status)) ?
				true : false, NULL);
		mac_ctx->lim.mgmtFrameSessionId = 0xff;
	} else {
		qdf_status =
			wma_tx_frameWithTxComplete(mac_ctx, packet,
				(uint16_t) msg_len,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS, 7, lim_tx_complete,
				frame, lim_p2p_action_cnf, tx_flag,
				sme_session_id, false,
				channel_freq, RATEID_DEFAULT);

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			pe_err("couldn't send action frame");
			lim_p2p_action_cnf(mac_ctx, packet, false, NULL);
			mac_ctx->lim.mgmtFrameSessionId = 0xff;
		} else {
			mac_ctx->lim.mgmtFrameSessionId = mb_msg->sessionId;
			pe_debug("lim.actionFrameSessionId: %u",
				mac_ctx->lim.mgmtFrameSessionId);
		}
	}

	return;
}

#ifdef WLAN_FEATURE_11W
static void lim_check_rmf_and_set_protected(tpAniSirGlobal mac_ctx,
	tSirMbMsgP2p *mb_msg, uint8_t *frame)
{
	uint8_t session_id = 0;
	tpPESession session_entry;
	tpSirMacMgmtHdr mac_hdr;
	tpSirMacActionFrameHdr action_hdr;
	tpSirMacFrameCtl fc = (tpSirMacFrameCtl) mb_msg->data;

	action_hdr = (tpSirMacActionFrameHdr)
		(frame + sizeof(tSirMacMgmtHdr));
	mac_hdr = (tpSirMacMgmtHdr) frame;
	session_entry = pe_find_session_by_bssid(mac_ctx,
		(uint8_t *) mb_msg->data + BSSID_OFFSET,
		&session_id);

	/*
	 * Check for session corresponding to ADDR2 as supplicant
	 * is filling ADDR2  with BSSID
	 */
	if (!session_entry) {
		session_entry = pe_find_session_by_bssid(mac_ctx,
			(uint8_t *) mb_msg->data + ADDR2_OFFSET,
			 &session_id);
	}
	/*
	 * Setting Protected bit only for Robust Action Frames
	 * This has to be based on the current Connection with the
	 * station. lim_set_protected_bit API will set the protected
	 * bit if connection is PMF
	 */
	if (session_entry && (SIR_MAC_MGMT_ACTION == fc->subType) &&
		session_entry->limRmfEnabled &&
		(!lim_is_group_addr(mac_hdr->da)) &&
		lim_is_robust_mgmt_action_frame(action_hdr->category))
		lim_set_protected_bit(mac_ctx, session_entry,
					mac_hdr->da, mac_hdr);
}
#else
static inline void lim_check_rmf_and_set_protected(tpAniSirGlobal mac_ctx,
	tSirMbMsgP2p *mb_msg, uint8_t *frame)
{
}
#endif

/* function to parse IEs withing P2P IE */
static const uint8_t *lim_parse_p2p_ie(const uint8_t *ie, int len, uint8_t eid)
{
	int left = len;
	const uint8_t *ptr = ie;
	uint8_t elem_id, size_of_len_field = 0;
	uint16_t elem_len;

	/* IE should be at least len(eid + len) = 3 */
	while (left >= 3) {
		elem_id = ptr[0];
		if (elem_id == SIR_P2P_NOA_ATTR) {
			elem_len = ((uint16_t) ptr[1]) | (ptr[2] << 8);
			size_of_len_field = 2;
		} else {
			elem_len = ptr[1];
			size_of_len_field = 1;
		}

		left -= (size_of_len_field + 1);
		if (elem_len > left) {
			pe_err("Invalid IEs eid: %d elem_len: %d left: %d",
				eid, elem_len, left);
			return NULL;
		}

		if (elem_id == eid)
			return ptr;

		left -= elem_len;
		ptr += (elem_len + (size_of_len_field + 1));
	}
	return NULL;
}

/**
 * lim_send_p2p_action_frame() - Process action frame request
 * @mac_ctx:  Pointer to mac context
 * @msg:  message sent from SME
 *
 * This function processes the action frame request sent from the
 * SME and generates the ACTION frame.
 *
 * Return: NULL
 */
void lim_send_p2p_action_frame(tpAniSirGlobal mac_ctx,
		struct scheduler_msg *msg)
{
	tSirMbMsgP2p *mb_msg = (tSirMbMsgP2p *) msg->bodyptr;
	uint32_t msg_len;
	uint8_t *frame;
	void *packet;
	QDF_STATUS qdf_status;
	tpSirMacFrameCtl fc = (tpSirMacFrameCtl) mb_msg->data;
	uint8_t noa_len = 0;
	uint8_t noa_stream[SIR_MAX_NOA_ATTR_LEN + (2 * SIR_P2P_IE_HEADER_LEN)];
	uint8_t orig_len = 0;
	uint8_t *p2p_ie = NULL;
	tpPESession session_entry = NULL;
	const uint8_t *presence_noa_attr = NULL;
	uint8_t *tmp_p2p_ie = NULL;
	uint16_t remain_len = 0;
	uint8_t sme_session_id = 0;

	msg_len = mb_msg->msgLen - sizeof(tSirMbMsgP2p);
	pe_debug("sending fc->type: %d fc->subType: %d",
		fc->type, fc->subType);

	if ((!mac_ctx->lim.gpLimRemainOnChanReq) && (0 != mb_msg->wait)) {
		pe_err("RemainOnChannel is not running");
		mac_ctx->lim.mgmtFrameSessionId = mb_msg->sessionId;
		lim_p2p_action_cnf(mac_ctx, NULL, false, NULL);
		mac_ctx->lim.mgmtFrameSessionId = 0xff;
		return;
	}
	sme_session_id = mb_msg->sessionId;

	if ((SIR_MAC_MGMT_FRAME == fc->type) &&
		(SIR_MAC_MGMT_PROBE_RSP == fc->subType)) {
		/* get proper offset for Probe RSP */
		p2p_ie = (uint8_t *)limGetP2pIEPtr(mac_ctx,
				(uint8_t *) mb_msg->data +
				PROBE_RSP_IE_OFFSET,
				msg_len - PROBE_RSP_IE_OFFSET);
		while ((NULL != p2p_ie)
			&& (SIR_MAC_MAX_IE_LENGTH == p2p_ie[1])) {
			remain_len =
				msg_len - (p2p_ie -
				(uint8_t *) mb_msg->data);
			if (remain_len > 2) {
				tmp_p2p_ie = (uint8_t *)limGetP2pIEPtr(mac_ctx,
					p2p_ie + SIR_MAC_MAX_IE_LENGTH + 2,
					remain_len);
			}
			if (tmp_p2p_ie) {
				p2p_ie = (uint8_t *)tmp_p2p_ie;
				tmp_p2p_ie = NULL;
			} else {
				break;
			}
		} /* end of while */
	} else if ((SIR_MAC_MGMT_FRAME == fc->type) &&
		(SIR_MAC_MGMT_ACTION == fc->subType) &&
		(SIR_MAC_ACTION_VENDOR_SPECIFIC_CATEGORY ==
		*((uint8_t *) mb_msg->data + ACTION_OFFSET))) {
		tpSirMacP2PActionFrameHdr action_hdr =
			(tpSirMacP2PActionFrameHdr) ((uint8_t *)
			mb_msg->data + ACTION_OFFSET);
		if ((!qdf_mem_cmp(action_hdr->Oui, SIR_MAC_P2P_OUI,
			SIR_MAC_P2P_OUI_SIZE)) &&
			(SIR_MAC_ACTION_P2P_SUBTYPE_PRESENCE_RSP ==
			action_hdr->OuiSubType)) {

			/* In case of Presence RSP response */
			p2p_ie = (uint8_t *)limGetP2pIEPtr(mac_ctx,
				(uint8_t *)mb_msg->data +
				ACTION_OFFSET +
				sizeof(tSirMacP2PActionFrameHdr),
				(msg_len - ACTION_OFFSET
				- sizeof(tSirMacP2PActionFrameHdr)));
			if (NULL != p2p_ie) {
				/* extract the presence of NoA attribute inside
				 * P2P IE */
				presence_noa_attr =  lim_parse_p2p_ie(
					p2p_ie + SIR_P2P_IE_HEADER_LEN,
					p2p_ie[1], SIR_P2P_NOA_ATTR);
			}
		}
	}

	if ((SIR_MAC_MGMT_FRAME == fc->type) &&
		(SIR_MAC_MGMT_PROBE_RSP == fc->subType ||
		SIR_MAC_MGMT_ACTION == fc->subType) && p2p_ie != NULL) {
		/* get NoA attribute stream P2P IE */
		noa_len =
			lim_get_noa_attr_stream(mac_ctx, noa_stream,
				session_entry);
		/* need to append NoA attribute in P2P IE */
		if (noa_len > 0) {
			orig_len = p2p_ie[1];
			/* if Presence Rsp has NoAttr */
			if (presence_noa_attr) {
				uint16_t noa_len =
					presence_noa_attr[1] |
					(presence_noa_attr[2] << 8);
				/*One byte for attribute, 2bytes for length */
				orig_len -= (noa_len + 1 + 2);
				/* remove those bytes to copy */
				msg_len -= (noa_len + 1 + 2);
				/* remove NoA from original Len */
				p2p_ie[1] = orig_len;
			}
			if ((p2p_ie[1] + (uint16_t) noa_len) >
				SIR_MAC_MAX_IE_LENGTH) {
				/*
				 * Form the new NoA Byte array in multiple
				 * P2P IEs
				 */
				noa_len =
					lim_get_noa_attr_stream_in_mult_p2p_ies
					(mac_ctx, noa_stream, noa_len,
					((p2p_ie[1] + (uint16_t)noa_len)
					- SIR_MAC_MAX_IE_LENGTH));
				p2p_ie[1] = SIR_MAC_MAX_IE_LENGTH;
			} else {
				/* increment the length of P2P IE */
				p2p_ie[1] += noa_len;
			}
			msg_len += noa_len;
			pe_debug("noa_len: %d orig_len: %d p2p_ie: %pK msg_len: %d nBytesToCopy: %zu ",
				noa_len, orig_len, p2p_ie, msg_len,
				((p2p_ie + orig_len + 2) -
				 (uint8_t *) mb_msg->data));
		}
	}

	if (SIR_MAC_MGMT_PROBE_RSP == fc->subType)
		lim_set_ht_caps(mac_ctx, session_entry,
			(uint8_t *) mb_msg->data + PROBE_RSP_IE_OFFSET,
			msg_len - PROBE_RSP_IE_OFFSET);

	/* Ok-- try to allocate some memory: */
	qdf_status = cds_packet_alloc((uint16_t) msg_len, (void **)&frame,
		(void **)&packet);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate: %d bytes for a Probe Request",
			msg_len);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(frame, msg_len, 0);

	/*
	 * Add sequence number to action frames
	 * Frames are handed over in .11 format by supplicant already
	 */
	lim_populate_p2p_mac_header(mac_ctx, (uint8_t *) mb_msg->data);

	if ((noa_len > 0)
		&& (noa_len < (SIR_MAX_NOA_ATTR_LEN + SIR_P2P_IE_HEADER_LEN))) {
		/* Add 2 bytes for length and Arribute field */
		uint32_t nBytesToCopy = ((p2p_ie + orig_len + 2) -
			 (uint8_t *) mb_msg->data);
		qdf_mem_copy(frame, mb_msg->data, nBytesToCopy);
		qdf_mem_copy((frame + nBytesToCopy), noa_stream, noa_len);
		qdf_mem_copy((frame + nBytesToCopy + noa_len),
			mb_msg->data + nBytesToCopy,
			msg_len - nBytesToCopy - noa_len);

	} else {
		qdf_mem_copy(frame, mb_msg->data, msg_len);
	}

	lim_check_rmf_and_set_protected(mac_ctx, mb_msg, frame);

	lim_tx_action_frame(mac_ctx, mb_msg, msg_len, packet, frame);
	return;
}

/* Power Save Related Functions */
tSirRetStatus __lim_process_sme_no_a_update(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tpP2pPsConfig pNoA;
	tpP2pPsParams pMsgNoA;
	struct scheduler_msg msg = {0};

	pNoA = (tpP2pPsConfig) pMsgBuf;

	pMsgNoA = qdf_mem_malloc(sizeof(tP2pPsConfig));
	if (NULL == pMsgNoA) {
		pe_err("Unable to allocate memory during NoA Update");
		return eSIR_MEM_ALLOC_FAILED;
	}

	pMsgNoA->opp_ps = pNoA->opp_ps;
	pMsgNoA->ctWindow = pNoA->ctWindow;
	pMsgNoA->duration = pNoA->duration;
	pMsgNoA->interval = pNoA->interval;
	pMsgNoA->count = pNoA->count;
	pMsgNoA->single_noa_duration = pNoA->single_noa_duration;
	pMsgNoA->psSelection = pNoA->psSelection;
	pMsgNoA->sessionId = pNoA->sessionid;

	msg.type = WMA_SET_P2P_GO_NOA_REQ;
	msg.reserved = 0;
	msg.bodyptr = pMsgNoA;
	msg.bodyval = 0;

	if (eSIR_SUCCESS != wma_post_ctrl_msg(pMac, &msg)) {
		pe_err("halPostMsgApi failed");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
} /*** end __limProcessSmeGoNegReq() ***/

