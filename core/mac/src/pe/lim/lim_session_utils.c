/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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

   \file  lim_session_utils.c
   \brief implementation for lim Session Utility  APIs
   \author Sunit Bhatia
   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "ani_global.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "lim_session_utils.h"
#include "lim_utils.h"

/**
 * is_lim_session_off_channel() - checks if any other off channel session exists
 * @mac_ctx: Global MAC context.
 * @sessionId: PE session ID.
 *
 * Return: This function returns true if the session Id passed needs to be on
 *         a different channel than atleast one session already active.
 **/
uint8_t is_lim_session_off_channel(tpAniSirGlobal mac_ctx, uint8_t session_id)
{
	uint8_t i;

	if (session_id >= mac_ctx->lim.maxBssId) {
		pe_warn("Invalid session_id: %d", session_id);
		return false;
	}

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		/* Skip the session_id that is to be joined. */
		if (i == session_id)
			continue;
		/*
		 * if another session is valid and it is on different channel
		 * then it is an off channel operation.
		 */
		if ((mac_ctx->lim.gpSession[i].valid) &&
		    (mac_ctx->lim.gpSession[i].currentOperChannel !=
		     mac_ctx->lim.gpSession[session_id].currentOperChannel))
			return true;
	}
	return false;

}

/**
 * lim_is_chan_switch_running() - check if channel switch is happening
 * @mac_ctx: Global MAC context.
 *
 * Return: 1 - if channel switch is happening on any session.
 *         0 - if channel switch is not happening.
 **/
uint8_t lim_is_chan_switch_running(tpAniSirGlobal mac_ctx)
{
	uint8_t i;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++)
		if (mac_ctx->lim.gpSession[i].valid &&
			mac_ctx->lim.gpSession[i].gLimSpecMgmt.dot11hChanSwState
			== eLIM_11H_CHANSW_RUNNING)
			return 1;
	return 0;
}

/**
 * lim_is_in_mcc() - check if device is in MCC
 * @mac_ctx: Global MAC context.
 *
 * Return: true - if in MCC.
 *         false - Not in MCC
 **/
uint8_t lim_is_in_mcc(tpAniSirGlobal mac_ctx)
{
	uint8_t i;
	uint8_t chan = 0;
	uint8_t curr_oper_channel = 0;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		/*
		 * if another session is valid and it is on different channel
		 * it is an off channel operation.
		 */
		if ((mac_ctx->lim.gpSession[i].valid)) {
			curr_oper_channel =
				mac_ctx->lim.gpSession[i].currentOperChannel;
			if (chan == 0)
				chan = curr_oper_channel;
			else if (chan != curr_oper_channel)
				return true;
		}
	}
	return false;
}

/**
 * pe_get_current_stas_count() - Total stations associated on all sessions.
 * @mac_ctx: Global MAC context.
 *
 * Return: true - Number of stations active on all sessions.
 **/
uint8_t pe_get_current_stas_count(tpAniSirGlobal mac_ctx)
{
	uint8_t i;
	uint8_t stacount = 0;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++)
		if (mac_ctx->lim.gpSession[i].valid == true)
			stacount +=
				mac_ctx->lim.gpSession[i].gLimNumOfCurrentSTAs;
	return stacount;
}
