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

/* A DFS channel can be ACTIVE for max 9000 msec, from the last
   received Beacon/Prpbe Resp. */
#define   MAX_TIME_TO_BE_ACTIVE_CHANNEL 9000

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
