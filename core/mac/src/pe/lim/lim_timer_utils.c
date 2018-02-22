/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * This file lim_timer_utils.cc contains the utility functions
 * LIM uses for handling various timers.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include <lim_api.h>

/* channel Switch Timer in ticks */
#define LIM_CHANNEL_SWITCH_TIMER_TICKS           1
/* Lim Quite timer in ticks */
#define LIM_QUIET_TIMER_TICKS                    100
/* Lim Quite BSS timer interval in ticks */
#define LIM_QUIET_BSS_TIMER_TICK                 100
/* Lim KeepAlive timer default (3000)ms */
#define LIM_KEEPALIVE_TIMER_MS                   3000
/* Lim JoinProbeRequest Retry  timer default (200)ms */
#define LIM_JOIN_PROBE_REQ_TIMER_MS              200
/* Lim Periodic Auth Retry timer default 60 ms */
#define LIM_AUTH_RETRY_TIMER_MS   60

/*
 * SAE auth timer of 5secs. This is required for duration of entire SAE
 * authentication.
 */
#define LIM_AUTH_SAE_TIMER_MS 5000


/* This timer is a periodic timer which expires at every 1 sec to
   convert  ACTIVE DFS channel to DFS channels */
#define ACTIVE_TO_PASSIVE_CONVERISON_TIMEOUT     1000

static bool lim_create_non_ap_timers(tpAniSirGlobal pMac)
{
	uint32_t cfgValue;
	/* Create Channel Switch Timer */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimChannelSwitchTimer,
			    "CHANNEL SWITCH TIMER",
			    lim_channel_switch_timer_handler, 0,
			    LIM_CHANNEL_SWITCH_TIMER_TICKS,
			    0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("failed to create Ch Switch timer");
		return false;
	}
	/* Create Quiet Timer
	 * This is used on the STA to go and shut-off Tx/Rx "after" the
	 * specified quiteInterval
	 */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimQuietTimer,
			    "QUIET TIMER", lim_quiet_timer_handler,
			    SIR_LIM_QUIET_TIMEOUT, LIM_QUIET_TIMER_TICKS,
			    0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("failed to create Quiet Begin Timer");
		return false;
	}
	/* Create Quiet BSS Timer
	 * After the specified quiteInterval, determined by gLimQuietTimer, this
	 * timer, gLimQuietBssTimer, trigger and put the STA to sleep for the
	 * specified gLimQuietDuration
	 */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimQuietBssTimer,
			    "QUIET BSS TIMER", lim_quiet_bss_timer_handler,
			    SIR_LIM_QUIET_BSS_TIMEOUT, LIM_QUIET_BSS_TIMER_TICK,
			    0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("failed to create Quiet Bss Timer");
		return false;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve JoinFailureTimeout value");
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	/* Create Join failure timer and activate it later */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimJoinFailureTimer,
			    "JOIN FAILURE TIMEOUT",
			    lim_timer_handler, SIR_LIM_JOIN_FAIL_TIMEOUT,
			    cfgValue, 0,
			    TX_NO_ACTIVATE) != TX_SUCCESS) {
		/* / Could not create Join failure timer. */
		/* Log error */
		pe_err("could not create Join failure timer");
		return false;
	}
	/* Send unicast probe req frame every 200 ms */
	if (tx_timer_create(pMac,
			    &pMac->lim.limTimers.gLimPeriodicJoinProbeReqTimer,
			    "Periodic Join Probe Request Timer",
			    lim_timer_handler,
			    SIR_LIM_PERIODIC_JOIN_PROBE_REQ_TIMEOUT,
			    SYS_MS_TO_TICKS(LIM_JOIN_PROBE_REQ_TIMER_MS), 0,
			    TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not create Periodic Join Probe Request tmr");
		return false;
	}

	/* Send Auth frame every 60 ms */
	if ((tx_timer_create(pMac,
		&pMac->lim.limTimers.g_lim_periodic_auth_retry_timer,
		"Periodic AUTH Timer",
		lim_timer_handler, SIR_LIM_AUTH_RETRY_TIMEOUT,
		SYS_MS_TO_TICKS(LIM_AUTH_RETRY_TIMER_MS), 0,
		TX_NO_ACTIVATE)) != TX_SUCCESS) {
		pe_err("could not create Periodic AUTH Timer");
		return false;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve AssocFailureTimeout value");

	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	/* Create Association failure timer and activate it later */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimAssocFailureTimer,
			    "ASSOC FAILURE TIMEOUT",
			    lim_assoc_failure_timer_handler, LIM_ASSOC,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not create Association failure timer");
		return false;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_ADDTS_RSP_TIMEOUT, &cfgValue)
			     != eSIR_SUCCESS)
		pe_err("Fail to get WNI_CFG_ADDTS_RSP_TIMEOUT");

	cfgValue = SYS_MS_TO_TICKS(cfgValue);

	/* Create Addts response timer and activate it later */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimAddtsRspTimer,
			    "ADDTS RSP TIMEOUT",
			    lim_addts_response_timer_handler,
			    SIR_LIM_ADDTS_RSP_TIMEOUT,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not create Addts response timer");
		return false;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve AuthFailureTimeout value");

	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	/* Create Auth failure timer and activate it later */
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimAuthFailureTimer,
			    "AUTH FAILURE TIMEOUT",
			    lim_timer_handler,
			    SIR_LIM_AUTH_FAIL_TIMEOUT,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not create Auth failure timer");
		return false;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_PROBE_AFTER_HB_FAIL_TIMEOUT,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve PROBE_AFTER_HB_FAIL_TIMEOUT value");

	/* Change timer to reactivate it in future */
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimProbeAfterHBTimer,
			    "Probe after Heartbeat TIMEOUT",
			    lim_timer_handler,
			    SIR_LIM_PROBE_HB_FAILURE_TIMEOUT,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("unable to create ProbeAfterHBTimer");
		return false;
	}

	/*
	 * SAE auth timer of 5secs. This is required for duration of entire SAE
	 * authentication.
	 */
	if ((tx_timer_create(pMac,
		&pMac->lim.limTimers.sae_auth_timer,
		"SAE AUTH Timer",
		lim_timer_handler, SIR_LIM_AUTH_SAE_TIMEOUT,
		SYS_MS_TO_TICKS(LIM_AUTH_SAE_TIMER_MS), 0,
		TX_NO_ACTIVATE)) != TX_SUCCESS) {
		pe_err("could not create SAE AUTH Timer");
		return false;
	}

	return true;
}
/**
 * lim_create_timers()
 *
 * @pMac : Pointer to Global MAC structure
 *
 * This function is called upon receiving
 * 1. SME_START_REQ for STA in ESS role
 * 2. SME_START_BSS_REQ for AP role & STA in IBSS role
 *
 * @return : status of operation
 */

uint32_t lim_create_timers(tpAniSirGlobal pMac)
{
	uint32_t cfgValue, i = 0;
	uint32_t cfgValue1;

	pe_debug("Creating Timers used by LIM module in Role: %d",
	       pMac->lim.gLimSystemRole);
	/* Create timers required for host roaming feature */
	if (TX_SUCCESS != lim_create_timers_host_roam(pMac))
		return TX_TIMER_ERROR;

	if (wlan_cfg_get_int(pMac, WNI_CFG_ACTIVE_MINIMUM_CHANNEL_TIME,
			     &cfgValue) != eSIR_SUCCESS) {
		pe_err("could not retrieve MinChannelTimeout value");
	}
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	/* Periodic probe request timer value is half of the Min channel
	 * timer. Probe request sends periodically till min/max channel
	 * timer expires
	 */
	cfgValue1 = cfgValue / 2;

	if (wlan_cfg_get_int(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve MAXChannelTimeout value");

	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	/* Limiting max numm of probe req for each channel scan */
	pMac->lim.maxProbe = (cfgValue / cfgValue1);

	if (pMac->lim.gLimSystemRole != eLIM_AP_ROLE)
		if (false == lim_create_non_ap_timers(pMac))
			goto err_timer;

	/* Create all CNF_WAIT Timers upfront */
	if (wlan_cfg_get_int(pMac, WNI_CFG_WT_CNF_TIMEOUT, &cfgValue)
		!= eSIR_SUCCESS) {
		pe_err("could not retrieve CNF timeout value");
	}

	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	for (i = 0; i < (pMac->lim.maxStation + 1); i++) {
		if (tx_timer_create(pMac,
				    &pMac->lim.limTimers.gpLimCnfWaitTimer[i],
				    "CNF_MISS_TIMEOUT",
				    lim_cnf_wait_tmer_handler,
				    (uint32_t) i, cfgValue,
				    0, TX_NO_ACTIVATE) != TX_SUCCESS) {
			pe_err("Cannot create CNF wait timer");
			goto err_timer;
		}
	}

	/* Alloc and init table for the preAuth timer list */
	if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_NUM_PRE_AUTH,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve mac preauth value");
	pMac->lim.gLimPreAuthTimerTable.numEntry = cfgValue;
	pMac->lim.gLimPreAuthTimerTable.pTable =
		qdf_mem_malloc(cfgValue * sizeof(tLimPreAuthNode *));

	if (pMac->lim.gLimPreAuthTimerTable.pTable == NULL) {
		pe_err("AllocateMemory failed!");
		goto err_timer;
	}

	for (i = 0; i < cfgValue; i++) {
		pMac->lim.gLimPreAuthTimerTable.pTable[i] =
					qdf_mem_malloc(sizeof(tLimPreAuthNode));
		if (pMac->lim.gLimPreAuthTimerTable.pTable[i] == NULL) {
			pMac->lim.gLimPreAuthTimerTable.numEntry = 0;
			pe_err("AllocateMemory failed!");
			goto err_timer;
		}
	}

	lim_init_pre_auth_timer_table(pMac, &pMac->lim.gLimPreAuthTimerTable);
	pe_debug("alloc and init table for preAuth timers");

	if (wlan_cfg_get_int(pMac, WNI_CFG_OLBC_DETECT_TIMEOUT,
			     &cfgValue) != eSIR_SUCCESS)
		pe_err("could not retrieve OLBD detect timeout value");

	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimUpdateOlbcCacheTimer,
			    "OLBC UPDATE CACHE TIMEOUT",
			    lim_update_olbc_cache_timer_handler,
			    SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT, cfgValue,
			    cfgValue, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("Cannot create update OLBC cache tmr");
		goto err_timer;
	}

	cfgValue = 1000;
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimDisassocAckTimer,
			    "DISASSOC ACK TIMEOUT",
			    lim_timer_handler, SIR_LIM_DISASSOC_ACK_TIMEOUT,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not DISASSOC ACK TIMEOUT timer");
		goto err_timer;
	}

	cfgValue = 1000;
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac, &pMac->lim.limTimers.gLimDeauthAckTimer,
			    "DISASSOC ACK TIMEOUT",
			    lim_timer_handler, SIR_LIM_DEAUTH_ACK_TIMEOUT,
			    cfgValue, 0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("could not create DEAUTH ACK TIMEOUT timer");
		goto err_timer;
	}

	/* (> no of BI* no of TUs per BI * 1TU in msec +
	 * p2p start time offset*1 TU in msec = 2*100*1.024 + 5*1.024
	 * = 204.8 + 5.12 = 209.20)
	 */
	cfgValue = LIM_INSERT_SINGLESHOTNOA_TIMEOUT_VALUE;
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac,
		&pMac->lim.limTimers.gLimP2pSingleShotNoaInsertTimer,
		"Single Shot NOA Insert timeout", lim_timer_handler,
		SIR_LIM_INSERT_SINGLESHOT_NOA_TIMEOUT, cfgValue, 0,
		TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_err("Can't create Single Shot NOA Insert Timeout tmr");
		goto err_timer;
	}

	cfgValue = ACTIVE_TO_PASSIVE_CONVERISON_TIMEOUT;
	cfgValue = SYS_MS_TO_TICKS(cfgValue);
	if (tx_timer_create(pMac,
		&pMac->lim.limTimers.gLimActiveToPassiveChannelTimer,
		"ACTIVE TO PASSIVE CHANNEL", lim_timer_handler,
		SIR_LIM_CONVERT_ACTIVE_CHANNEL_TO_PASSIVE, cfgValue, 0,
		TX_NO_ACTIVATE) != TX_SUCCESS) {
		pe_warn("could not create timer for passive channel to active channel");
		goto err_timer;
	}

	return TX_SUCCESS;

err_timer:
	lim_delete_timers_host_roam(pMac);
	tx_timer_delete(&pMac->lim.limTimers.gLimDeauthAckTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimDisassocAckTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimUpdateOlbcCacheTimer);
	while (((int32_t)-- i) >= 0) {
		tx_timer_delete(&pMac->lim.limTimers.gpLimCnfWaitTimer[i]);
	}
	tx_timer_delete(&pMac->lim.limTimers.gLimProbeAfterHBTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimAuthFailureTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimAddtsRspTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimAssocFailureTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimJoinFailureTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimPeriodicJoinProbeReqTimer);
	tx_timer_delete(&pMac->lim.limTimers.g_lim_periodic_auth_retry_timer);
	tx_timer_delete(&pMac->lim.limTimers.gLimQuietBssTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimQuietTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimChannelSwitchTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimP2pSingleShotNoaInsertTimer);
	tx_timer_delete(&pMac->lim.limTimers.gLimActiveToPassiveChannelTimer);
	tx_timer_delete(&pMac->lim.limTimers.sae_auth_timer);

	if (NULL != pMac->lim.gLimPreAuthTimerTable.pTable) {
		for (i = 0; i < pMac->lim.gLimPreAuthTimerTable.numEntry; i++)
			qdf_mem_free(pMac->lim.gLimPreAuthTimerTable.pTable[i]);
		qdf_mem_free(pMac->lim.gLimPreAuthTimerTable.pTable);
		pMac->lim.gLimPreAuthTimerTable.pTable = NULL;
	}
	return TX_TIMER_ERROR;
} /****** end lim_create_timers() ******/

/**
 * lim_timer_handler()
 *
 ***FUNCTION:
 * This function is called upon
 * 1. MIN_CHANNEL, MAX_CHANNEL timer expiration during scanning
 * 2. JOIN_FAILURE timer expiration while joining a BSS
 * 3. AUTH_FAILURE timer expiration while authenticating with a peer
 * 4. Heartbeat timer expiration on STA
 * 5. Background scan timer expiration on STA
 * 6. AID release, Pre-auth cleanup and Link monitoring timer
 *    expiration on AP
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  param - Message corresponding to the timer that expired
 *
 * @return None
 */

void lim_timer_handler(void *pMacGlobal, uint32_t param)
{
	uint32_t statusCode;
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	/* Prepare and post message to LIM Message Queue */

	msg.type = (uint16_t) param;
	msg.bodyptr = NULL;
	msg.bodyval = 0;

	statusCode = lim_post_msg_high_priority(pMac, &msg);
	if (statusCode != eSIR_SUCCESS)
		pe_err("posting message: %X to LIM failed, reason: %d",
			msg.type, statusCode);
} /****** end lim_timer_handler() ******/

/**
 * lim_addts_response_timer_handler()
 *
 ***FUNCTION:
 * This function is called upon Addts response timer expiration on sta
 *
 ***LOGIC:
 * Message SIR_LIM_ADDTS_RSP_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  param - pointer to pre-auth node
 *
 * @return None
 */

void lim_addts_response_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	/* Prepare and post message to LIM Message Queue */

	msg.type = SIR_LIM_ADDTS_RSP_TIMEOUT;
	msg.bodyval = param;
	msg.bodyptr = NULL;

	lim_post_msg_api(pMac, &msg);
} /****** end lim_auth_response_timer_handler() ******/

/**
 * lim_auth_response_timer_handler()
 *
 ***FUNCTION:
 * This function is called upon Auth response timer expiration on AP
 *
 ***LOGIC:
 * Message SIR_LIM_AUTH_RSP_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  param - pointer to pre-auth node
 *
 * @return None
 */

void lim_auth_response_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	/* Prepare and post message to LIM Message Queue */

	msg.type = SIR_LIM_AUTH_RSP_TIMEOUT;
	msg.bodyptr = NULL;
	msg.bodyval = (uint32_t) param;

	lim_post_msg_api(pMac, &msg);
} /****** end lim_auth_response_timer_handler() ******/

/**
 * lim_assoc_failure_timer_handler()
 *
 * @mac_global  : Pointer to Global MAC structure
 * @param       : Indicates whether this is assoc or reassoc failure timeout
 *
 * This function is called upon Re/Assoc failure timer expiration on STA.
 * Message SIR_LIM_ASSOC_FAIL_TIMEOUT is posted to gSirLimMsgQ when this
 * function is executed.
 *
 * Return void
 */
void lim_assoc_failure_timer_handler(void *mac_global, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal mac_ctx = (tpAniSirGlobal) mac_global;
	tpPESession session = NULL;

	session = mac_ctx->lim.pSessionEntry;
	if (LIM_REASSOC == param && NULL != session
	    && session->limMlmState == eLIM_MLM_WT_FT_REASSOC_RSP_STATE) {
		pe_err("Reassoc timeout happened");
		if (mac_ctx->lim.reAssocRetryAttempt <
		    LIM_MAX_REASSOC_RETRY_LIMIT) {
			lim_send_retry_reassoc_req_frame(mac_ctx,
			    session->pLimMlmReassocRetryReq, session);
			mac_ctx->lim.reAssocRetryAttempt++;
			pe_warn("Reassoc request retry is sent %d times",
				mac_ctx->lim.reAssocRetryAttempt);
			return;
		} else {
			pe_warn("Reassoc request retry MAX: %d reached",
				LIM_MAX_REASSOC_RETRY_LIMIT);
			if (NULL != session->pLimMlmReassocRetryReq) {
				qdf_mem_free(session->pLimMlmReassocRetryReq);
				session->pLimMlmReassocRetryReq = NULL;
			}
		}
	}
	/* Prepare and post message to LIM Message Queue */
	msg.type = SIR_LIM_ASSOC_FAIL_TIMEOUT;
	msg.bodyval = (uint32_t) param;
	msg.bodyptr = NULL;
	lim_post_msg_api(mac_ctx, &msg);
} /****** end lim_assoc_failure_timer_handler() ******/

/**
 * lim_update_olbc_cache_timer_handler()
 *
 ***FUNCTION:
 * This function is called upon update olbc cache timer expiration
 * on STA
 *
 ***LOGIC:
 * Message SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */
void lim_update_olbc_cache_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	/* Prepare and post message to LIM Message Queue */

	msg.type = SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT;
	msg.bodyval = 0;
	msg.bodyptr = NULL;

	lim_post_msg_api(pMac, &msg);
} /****** end lim_update_olbc_cache_timer_handler() ******/

/**
 * lim_deactivate_and_change_timer()
 *
 ***FUNCTION:
 * This function is called to deactivate and change a timer
 * for future re-activation
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in lim_utils.h file
 *
 * @return None
 */

void lim_deactivate_and_change_timer(tpAniSirGlobal pMac, uint32_t timerId)
{
	uint32_t val = 0;
	tpPESession  session_entry;

	switch (timerId) {
	case eLIM_REASSOC_FAIL_TIMER:
	case eLIM_FT_PREAUTH_RSP_TIMER:
		lim_deactivate_and_change_timer_host_roam(pMac, timerId);
		break;

	case eLIM_ADDTS_RSP_TIMER:
		pMac->lim.gLimAddtsRspTimerCount++;
		if (tx_timer_deactivate(&pMac->lim.limTimers.gLimAddtsRspTimer)
		    != TX_SUCCESS) {
			/* Could not deactivate AddtsRsp Timer */
			/* Log error */
			pe_err("Unable to deactivate AddtsRsp timer");
		}
		break;

	case eLIM_JOIN_FAIL_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimJoinFailureTimer)
		    != TX_SUCCESS) {
			/**
			 * Could not deactivate Join Failure
			 * timer. Log error.
			 */
			pe_err("Unable to deactivate Join Failure timer");
		}

		if (wlan_cfg_get_int(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get JoinFailureTimeout value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve JoinFailureTimeout value");
		}
		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change(&pMac->lim.limTimers.gLimJoinFailureTimer,
				    val, 0) != TX_SUCCESS) {
			/**
			 * Could not change Join Failure
			 * timer. Log error.
			 */
			pe_err("Unable to change Join Failure timer");
		}

		break;

	case eLIM_PERIODIC_JOIN_PROBE_REQ_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimPeriodicJoinProbeReqTimer)
		    != TX_SUCCESS) {
			/* Could not deactivate periodic join req Times. */
			pe_err("Unable to deactivate periodic join request timer");
		}

		val = SYS_MS_TO_TICKS(LIM_JOIN_PROBE_REQ_TIMER_MS);
		if (tx_timer_change
			    (&pMac->lim.limTimers.gLimPeriodicJoinProbeReqTimer, val,
			    0) != TX_SUCCESS) {
			/* Could not change periodic join req times. */
			/* Log error */
			pe_err("Unable to change periodic join request timer");
		}

		break;

	case eLIM_AUTH_FAIL_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimAuthFailureTimer)
		    != TX_SUCCESS) {
			/* Could not deactivate Auth failure timer. */
			/* Log error */
			pe_err("Unable to deactivate auth failure timer");
		}
		/* Change timer to reactivate it in future */
		if (wlan_cfg_get_int(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get AuthFailureTimeout value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve AuthFailureTimeout value");
		}
		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change(&pMac->lim.limTimers.gLimAuthFailureTimer,
				    val, 0) != TX_SUCCESS) {
			/* Could not change Authentication failure timer. */
			/* Log error */
			pe_err("unable to change Auth failure timer");
		}

		break;

	case eLIM_AUTH_RETRY_TIMER:

		if (tx_timer_deactivate
			  (&pMac->lim.limTimers.g_lim_periodic_auth_retry_timer)
							 != TX_SUCCESS) {
			/* Could not deactivate Auth Retry Timer. */
			pe_err("Unable to deactivate Auth Retry timer");
		}
		session_entry = pe_find_session_by_session_id(pMac,
			pMac->lim.limTimers.
				g_lim_periodic_auth_retry_timer.sessionId);
		if (NULL == session_entry) {
			pe_err("session does not exist for given SessionId : %d",
			pMac->lim.limTimers.
				g_lim_periodic_auth_retry_timer.sessionId);
			break;
		}
		/* 3/5 of the beacon interval */
		val = (session_entry->beaconParams.beaconInterval * 3) / 5;
		val = SYS_MS_TO_TICKS(val);
		if (tx_timer_change
			 (&pMac->lim.limTimers.g_lim_periodic_auth_retry_timer,
							val, 0) != TX_SUCCESS) {
			/* Could not change Auth Retry timer. */
			pe_err("Unable to change Auth Retry timer");
		}
		break;

	case eLIM_ASSOC_FAIL_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimAssocFailureTimer) !=
		    TX_SUCCESS) {
			/* Could not deactivate Association failure timer. */
			/* Log error */
			pe_err("unable to deactivate Association failure timer");
		}
		/* Change timer to reactivate it in future */
		if (wlan_cfg_get_int(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get AssocFailureTimeout value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve AssocFailureTimeout value");
		}
		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change(&pMac->lim.limTimers.gLimAssocFailureTimer,
				    val, 0) != TX_SUCCESS) {
			/* Could not change Association failure timer. */
			/* Log error */
			pe_err("unable to change Assoc failure timer");
		}

		break;

	case eLIM_PROBE_AFTER_HB_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimProbeAfterHBTimer) !=
		    TX_SUCCESS) {
			/* Could not deactivate Heartbeat timer. */
			/* Log error */
			pe_err("unable to deactivate probeAfterHBTimer");
		} else {
			pe_debug("Deactivated probe after hb timer");
		}

		if (wlan_cfg_get_int(pMac, WNI_CFG_PROBE_AFTER_HB_FAIL_TIMEOUT,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get PROBE_AFTER_HB_FAILURE
			 * value from CFG. Log error.
			 */
			pe_err("could not retrieve PROBE_AFTER_HB_FAIL_TIMEOUT value");
		}
		/* Change timer to reactivate it in future */
		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change(&pMac->lim.limTimers.gLimProbeAfterHBTimer,
				    val, 0) != TX_SUCCESS) {
			/* Could not change HeartBeat timer. */
			/* Log error */
			pe_err("unable to change ProbeAfterHBTimer");
		} else {
			pe_debug("Probe after HB timer value is changed: %u",
				val);
		}

		break;

	case eLIM_LEARN_DURATION_TIMER:
		break;

	case eLIM_CONVERT_ACTIVE_CHANNEL_TO_PASSIVE:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimActiveToPassiveChannelTimer) !=
		    TX_SUCCESS) {
			/**
			** Could not deactivate Active to passive channel timer.
			** Log error.
			**/
			pe_err("Unable to Deactivate Active to passive channel timer");
			return;
		}
		val = ACTIVE_TO_PASSIVE_CONVERISON_TIMEOUT;
		val = SYS_MS_TO_TICKS(val);
		if (tx_timer_change
			    (&pMac->lim.limTimers.gLimActiveToPassiveChannelTimer, val,
			    0) != TX_SUCCESS) {
			/**
			 * Could not change timer to check scan type for passive channel.
			 * timer. Log error.
			 */
			pe_err("Unable to change timer");
			return;
		}
		break;

	case eLIM_DISASSOC_ACK_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimDisassocAckTimer) != TX_SUCCESS) {
			/**
			** Could not deactivate Join Failure
			** timer. Log error.
			**/
			pe_err("Unable to deactivate Disassoc ack timer");
			return;
		}
		val = 1000;
		val = SYS_MS_TO_TICKS(val);
		if (tx_timer_change(&pMac->lim.limTimers.gLimDisassocAckTimer,
				    val, 0) != TX_SUCCESS) {
			/**
			 * Could not change Join Failure
			 * timer. Log error.
			 */
			pe_err("Unable to change timer");
			return;
		}
		break;

	case eLIM_DEAUTH_ACK_TIMER:
		if (tx_timer_deactivate(&pMac->lim.limTimers.gLimDeauthAckTimer)
		    != TX_SUCCESS) {
			/**
			** Could not deactivate Join Failure
			** timer. Log error.
			**/
			pe_err("Unable to deactivate Deauth ack timer");
			return;
		}
		val = 1000;
		val = SYS_MS_TO_TICKS(val);
		if (tx_timer_change(&pMac->lim.limTimers.gLimDeauthAckTimer,
				    val, 0) != TX_SUCCESS) {
			/**
			 * Could not change Join Failure
			 * timer. Log error.
			 */
			pe_err("Unable to change timer");
			return;
		}
		break;

	case eLIM_INSERT_SINGLESHOT_NOA_TIMER:
		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gLimP2pSingleShotNoaInsertTimer) !=
		    TX_SUCCESS) {
			/**
			** Could not deactivate SingleShot NOA Insert
			** timer. Log error.
			**/
			pe_err("Unable to deactivate SingleShot NOA Insert timer");
			return;
		}
		val = LIM_INSERT_SINGLESHOTNOA_TIMEOUT_VALUE;
		val = SYS_MS_TO_TICKS(val);
		if (tx_timer_change
			    (&pMac->lim.limTimers.gLimP2pSingleShotNoaInsertTimer, val,
			    0) != TX_SUCCESS) {
			/**
			 * Could not change Single Shot NOA Insert
			 * timer. Log error.
			 */
			pe_err("Unable to change timer");
			return;
		}
		break;

	case eLIM_AUTH_SAE_TIMER:
		if (tx_timer_deactivate
		   (&pMac->lim.limTimers.sae_auth_timer)
		    != TX_SUCCESS)
			pe_err("Unable to deactivate SAE auth timer");

		/* Change timer to reactivate it in future */
		val = SYS_MS_TO_TICKS(LIM_AUTH_SAE_TIMER_MS);

		if (tx_timer_change(&pMac->lim.limTimers.sae_auth_timer,
				    val, 0) != TX_SUCCESS)
			pe_err("unable to change SAE auth timer");

		break;

	default:
		/* Invalid timerId. Log error */
		break;
	}
} /****** end lim_deactivate_and_change_timer() ******/

/**
 * lim_deactivate_and_change_per_sta_id_timer()
 *
 *
 * @brief: This function is called to deactivate and change a per STA timer
 * for future re-activation
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 * @note   staId for eLIM_AUTH_RSP_TIMER is auth Node Index.
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in lim_utils.h file
 * @param  staId   - staId
 *
 * @return None
 */

void
lim_deactivate_and_change_per_sta_id_timer(tpAniSirGlobal pMac, uint32_t timerId,
					   uint16_t staId)
{
	uint32_t val;

	switch (timerId) {
	case eLIM_CNF_WAIT_TIMER:

		if (tx_timer_deactivate
			    (&pMac->lim.limTimers.gpLimCnfWaitTimer[staId])
		    != TX_SUCCESS) {
			pe_err("unable to deactivate CNF wait timer");
		}
		/* Change timer to reactivate it in future */

		if (wlan_cfg_get_int(pMac, WNI_CFG_WT_CNF_TIMEOUT,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get cnf timeout value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve cnf timeout value");
		}
		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change
			    (&pMac->lim.limTimers.gpLimCnfWaitTimer[staId], val,
			    val) != TX_SUCCESS) {
			/* Could not change cnf timer. */
			/* Log error */
			pe_err("unable to change cnf wait timer");
		}

		break;

	case eLIM_AUTH_RSP_TIMER:
	{
		tLimPreAuthNode *pAuthNode;

		pAuthNode =
			lim_get_pre_auth_node_from_index(pMac,
							 &pMac->lim.
							 gLimPreAuthTimerTable,
							 staId);

		if (pAuthNode == NULL) {
			pe_err("Invalid Pre Auth Index passed :%d",
				staId);
			break;
		}

		if (tx_timer_deactivate(&pAuthNode->timer) !=
		    TX_SUCCESS) {
			/* Could not deactivate auth response timer. */
			/* Log error */
			pe_err("unable to deactivate auth response timer");
		}
		/* Change timer to reactivate it in future */

		if (wlan_cfg_get_int
			    (pMac, WNI_CFG_AUTHENTICATE_RSP_TIMEOUT,
			    &val) != eSIR_SUCCESS) {
			/**
			 * Could not get auth rsp timeout value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve auth response timeout value");
		}

		val = SYS_MS_TO_TICKS(val);

		if (tx_timer_change(&pAuthNode->timer, val, 0) !=
		    TX_SUCCESS) {
			/* Could not change auth rsp timer. */
			/* Log error */
			pe_err("unable to change auth rsp timer");
		}
	}
	break;

	default:
		/* Invalid timerId. Log error */
		break;

	}
}

/**
 * lim_activate_cnf_timer()
 *
 ***FUNCTION:
 * This function is called to activate a per STA timer
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  StaId   - staId
 *
 * @return None
 */

void lim_activate_cnf_timer(tpAniSirGlobal pMac, uint16_t staId,
			    tpPESession psessionEntry)
{
	pMac->lim.limTimers.gpLimCnfWaitTimer[staId].sessionId =
		psessionEntry->peSessionId;
	if (tx_timer_activate(&pMac->lim.limTimers.gpLimCnfWaitTimer[staId])
	    != TX_SUCCESS) {
		pe_err("could not activate cnf wait timer");
	}
}

/**
 * lim_activate_auth_rsp_timer()
 *
 ***FUNCTION:
 * This function is called to activate a per STA timer
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  id      - id
 *
 * @return None
 */

void lim_activate_auth_rsp_timer(tpAniSirGlobal pMac, tLimPreAuthNode *pAuthNode)
{
	if (tx_timer_activate(&pAuthNode->timer) != TX_SUCCESS) {
		/* / Could not activate auth rsp timer. */
		/* Log error */
		pe_err("could not activate auth rsp timer");
	}
}

/**
 * limAssocCnfWaitTmerHandler()
 *
 ***FUNCTION:
 *        This function post a message to send a disassociate frame out.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */

void lim_cnf_wait_tmer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	uint32_t statusCode;
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	msg.type = SIR_LIM_CNF_WAIT_TIMEOUT;
	msg.bodyval = (uint32_t) param;
	msg.bodyptr = NULL;

	statusCode = lim_post_msg_api(pMac, &msg);
	if (statusCode != eSIR_SUCCESS)
		pe_err("posting to LIM failed, reason: %d", statusCode);

}

void lim_channel_switch_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	pe_debug("ChannelSwitch Timer expired.  Posting msg to LIM");

	msg.type = SIR_LIM_CHANNEL_SWITCH_TIMEOUT;
	msg.bodyval = (uint32_t) param;
	msg.bodyptr = NULL;

	lim_post_msg_api(pMac, &msg);
}

void lim_quiet_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	msg.type = SIR_LIM_QUIET_TIMEOUT;
	msg.bodyval = (uint32_t) param;
	msg.bodyptr = NULL;

	pe_debug("Post SIR_LIM_QUIET_TIMEOUT msg");
	lim_post_msg_api(pMac, &msg);
}

void lim_quiet_bss_timer_handler(void *pMacGlobal, uint32_t param)
{
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = (tpAniSirGlobal) pMacGlobal;

	msg.type = SIR_LIM_QUIET_BSS_TIMEOUT;
	msg.bodyval = (uint32_t) param;
	msg.bodyptr = NULL;
	pe_debug("Post SIR_LIM_QUIET_BSS_TIMEOUT msg");
	lim_post_msg_api(pMac, &msg);
}

