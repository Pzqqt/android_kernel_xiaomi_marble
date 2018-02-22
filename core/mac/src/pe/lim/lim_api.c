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
 * This file lim_api.cc contains the functions that are
 * exported by LIM to other modules.
 *
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "wni_cfg.h"
#include "wni_api.h"
#include "sir_common.h"
#include "sir_debug.h"
#include "cfg_api.h"

#include "sch_api.h"
#include "utils_api.h"
#include "lim_api.h"
#include "lim_global.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_ibss_peer_mgmt.h"
#include "lim_admit_control.h"
#include "lim_send_sme_rsp_messages.h"
#include "wmm_apsd.h"
#include "lim_trace.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "wma_types.h"

#include "rrm_api.h"

#include <lim_ft.h>
#include "qdf_types.h"
#include "cds_packet.h"
#include "cds_utils.h"
#include "sys_startup.h"
#include "cds_api.h"
#include "wlan_policy_mgr_api.h"
#include "nan_datapath.h"
#include "wma.h"
#include "wlan_mgmt_txrx_utils_api.h"
#include "wlan_objmgr_psoc_obj.h"
#include "os_if_nan.h"
#include <wlan_scan_ucfg_api.h>
#include <wlan_p2p_ucfg_api.h>

static void __lim_init_scan_vars(tpAniSirGlobal pMac)
{
	pMac->lim.gLimUseScanModeForLearnMode = 1;

	pMac->lim.gLimSystemInScanLearnMode = 0;

	/* Scan related globals on STA */
	pMac->lim.gLimReturnAfterFirstMatch = 0;
	pMac->lim.gLim24Band11dScanDone = 0;
	pMac->lim.gLim50Band11dScanDone = 0;
	pMac->lim.gLimReturnUniqueResults = 0;

	pMac->lim.gLimCurrentScanChannelId = 0;
	pMac->lim.gDeferMsgTypeForNOA = 0;
	pMac->lim.gpDefdSmeMsgForNOA = NULL;

	pMac->lim.gLimRestoreCBNumScanInterval =
		LIM_RESTORE_CB_NUM_SCAN_INTERVAL_DEFAULT;
	pMac->lim.gLimRestoreCBCount = 0;
	qdf_mem_set(pMac->lim.gLimLegacyBssidList,
		    sizeof(pMac->lim.gLimLegacyBssidList), 0);

	/* Fill in default values */

	/* abort scan is used to abort an on-going scan */
	pMac->lim.abortScan = 0;
	qdf_mem_set(&pMac->lim.scanChnInfo, sizeof(tLimScanChnInfo), 0);
	qdf_mem_set(&pMac->lim.dfschannelList, sizeof(tSirDFSChannelList), 0);

/* WLAN_SUSPEND_LINK Related */
	pMac->lim.gpLimSuspendCallback = NULL;
	pMac->lim.gpLimResumeCallback = NULL;
/* end WLAN_SUSPEND_LINK Related */
}

static void __lim_init_bss_vars(tpAniSirGlobal pMac)
{
	qdf_mem_set((void *)pMac->lim.gpSession,
		    sizeof(*pMac->lim.gpSession) * pMac->lim.maxBssId, 0);

	/* This is for testing purposes only, be default should always be off */
	pMac->lim.gLimForceNoPropIE = 0;
	pMac->lim.gpLimMlmSetKeysReq = NULL;
}

static void __lim_init_stats_vars(tpAniSirGlobal pMac)
{
	pMac->lim.gLimNumBeaconsRcvd = 0;
	pMac->lim.gLimNumBeaconsIgnored = 0;

	pMac->lim.gLimNumDeferredMsgs = 0;

	/* / Variable to keep track of number of currently associated STAs */
	pMac->lim.gLimNumOfAniSTAs = 0; /* count of ANI peers */

	/* Heart-Beat interval value */
	pMac->lim.gLimHeartBeatCount = 0;

	qdf_mem_zero(pMac->lim.gLimHeartBeatApMac[0],
			sizeof(tSirMacAddr));
	qdf_mem_zero(pMac->lim.gLimHeartBeatApMac[1],
			sizeof(tSirMacAddr));
	pMac->lim.gLimHeartBeatApMacIndex = 0;

	/* Statistics to keep track of no. beacons rcvd in heart beat interval */
	qdf_mem_set(pMac->lim.gLimHeartBeatBeaconStats,
		    sizeof(pMac->lim.gLimHeartBeatBeaconStats), 0);

#ifdef WLAN_DEBUG
	/* Debug counters */
	pMac->lim.numTot = 0;
	pMac->lim.numBbt = 0;
	pMac->lim.numProtErr = 0;
	pMac->lim.numLearn = 0;
	pMac->lim.numLearnIgnore = 0;
	pMac->lim.numSme = 0;
	qdf_mem_set(pMac->lim.numMAC, sizeof(pMac->lim.numMAC), 0);
	pMac->lim.gLimNumAssocReqDropInvldState = 0;
	pMac->lim.gLimNumAssocReqDropACRejectTS = 0;
	pMac->lim.gLimNumAssocReqDropACRejectSta = 0;
	pMac->lim.gLimNumReassocReqDropInvldState = 0;
	pMac->lim.gLimNumHashMissIgnored = 0;
	pMac->lim.gLimUnexpBcnCnt = 0;
	pMac->lim.gLimBcnSSIDMismatchCnt = 0;
	pMac->lim.gLimNumLinkEsts = 0;
	pMac->lim.gLimNumRxCleanup = 0;
	pMac->lim.gLim11bStaAssocRejectCount = 0;
#endif
}

static void __lim_init_states(tpAniSirGlobal pMac)
{
	/* Counts Heartbeat failures */
	pMac->lim.gLimHBfailureCntInLinkEstState = 0;
	pMac->lim.gLimProbeFailureAfterHBfailedCnt = 0;
	pMac->lim.gLimHBfailureCntInOtherStates = 0;
	pMac->lim.gLimRspReqd = 0;
	pMac->lim.gLimPrevSmeState = eLIM_SME_OFFLINE_STATE;

	/* / MLM State visible across all Sirius modules */
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, NO_SESSION, eLIM_MLM_IDLE_STATE));
	pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;

	/* / Previous MLM State */
	pMac->lim.gLimPrevMlmState = eLIM_MLM_OFFLINE_STATE;

	/* LIM to HAL SCAN Management Message Interface states */
	pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;

	/**
	 * Initialize state to eLIM_SME_OFFLINE_STATE
	 */
	pMac->lim.gLimSmeState = eLIM_SME_OFFLINE_STATE;

	/**
	 * By default assume 'unknown' role. This will be updated
	 * when SME_START_BSS_REQ is received.
	 */

	qdf_mem_set(&pMac->lim.gLimNoShortParams, sizeof(tLimNoShortParams), 0);
	qdf_mem_set(&pMac->lim.gLimNoShortSlotParams,
		    sizeof(tLimNoShortSlotParams), 0);

	pMac->lim.gLimPhyMode = 0;
	pMac->lim.scanStartTime = 0;    /* used to measure scan time */

	qdf_mem_set(pMac->lim.gLimMyMacAddr, sizeof(pMac->lim.gLimMyMacAddr),
		    0);
	pMac->lim.ackPolicy = 0;

	pMac->lim.gLimProbeRespDisableFlag = 0; /* control over probe resp */
}

static void __lim_init_vars(tpAniSirGlobal pMac)
{
	/* Place holder for Measurement Req/Rsp/Ind related info */


	/* Deferred Queue Paramters */
	qdf_mem_set(&pMac->lim.gLimDeferredMsgQ, sizeof(tSirAddtsReq), 0);

	/* addts request if any - only one can be outstanding at any time */
	qdf_mem_set(&pMac->lim.gLimAddtsReq, sizeof(tSirAddtsReq), 0);
	pMac->lim.gLimAddtsSent = 0;
	pMac->lim.gLimAddtsRspTimerCount = 0;

	/* protection related config cache */
	qdf_mem_set(&pMac->lim.cfgProtection, sizeof(tCfgProtection), 0);
	pMac->lim.gLimProtectionControl = 0;
	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

	/* WMM Related Flag */
	pMac->lim.gUapsdEnable = 0;

	/* QoS-AC Downgrade: Initially, no AC is admitted */
	pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] = 0;
	pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] = 0;

	/* dialogue token List head/tail for Action frames request sent. */
	pMac->lim.pDialogueTokenHead = NULL;
	pMac->lim.pDialogueTokenTail = NULL;

	qdf_mem_set(&pMac->lim.tspecInfo,
		    sizeof(tLimTspecInfo) * LIM_NUM_TSPEC_MAX, 0);

	/* admission control policy information */
	qdf_mem_set(&pMac->lim.admitPolicyInfo, sizeof(tLimAdmitPolicyInfo), 0);

	pMac->lim.gLastBeaconDtimCount = 0;
	pMac->lim.gLastBeaconDtimPeriod = 0;

	/* Scan in Power Save Flag */
	pMac->lim.gScanInPowersave = 0;
	pMac->lim.probeCounter = 0;
	pMac->lim.maxProbe = 0;
}

static void __lim_init_assoc_vars(tpAniSirGlobal pMac)
{
	pMac->lim.gLimAssocStaLimit = 0;
	pMac->lim.gLimIbssStaLimit = 0;
	/* Place holder for current authentication request */
	/* being handled */
	pMac->lim.gpLimMlmAuthReq = NULL;

	/* / MAC level Pre-authentication related globals */
	pMac->lim.gLimPreAuthChannelNumber = 0;
	pMac->lim.gLimPreAuthType = eSIR_OPEN_SYSTEM;
	qdf_mem_set(&pMac->lim.gLimPreAuthPeerAddr, sizeof(tSirMacAddr), 0);
	pMac->lim.gLimNumPreAuthContexts = 0;
	qdf_mem_set(&pMac->lim.gLimPreAuthTimerTable, sizeof(tLimPreAuthTable),
		    0);

	/* Placed holder to deauth reason */
	pMac->lim.gLimDeauthReasonCode = 0;

	/* Place holder for Pre-authentication node list */
	pMac->lim.pLimPreAuthList = NULL;

	/* One cache for each overlap and associated case. */
	qdf_mem_set(pMac->lim.protStaOverlapCache,
		    sizeof(tCacheParams) * LIM_PROT_STA_OVERLAP_CACHE_SIZE, 0);
	qdf_mem_set(pMac->lim.protStaCache,
		    sizeof(tCacheParams) * LIM_PROT_STA_CACHE_SIZE, 0);

	pMac->lim.pSessionEntry = NULL;
	pMac->lim.reAssocRetryAttempt = 0;

}

static void __lim_init_ht_vars(tpAniSirGlobal pMac)
{
	pMac->lim.htCapabilityPresentInBeacon = 0;
	pMac->lim.gHTGreenfield = 0;
	pMac->lim.gHTShortGI40Mhz = 0;
	pMac->lim.gHTShortGI20Mhz = 0;
	pMac->lim.gHTMaxAmsduLength = 0;
	pMac->lim.gHTDsssCckRate40MHzSupport = 0;
	pMac->lim.gHTPSMPSupport = 0;
	pMac->lim.gHTLsigTXOPProtection = 0;
	pMac->lim.gHTMIMOPSState = eSIR_HT_MIMO_PS_STATIC;
	pMac->lim.gHTAMpduDensity = 0;

	pMac->lim.gMaxAmsduSizeEnabled = false;
	pMac->lim.gHTMaxRxAMpduFactor = 0;
	pMac->lim.gHTServiceIntervalGranularity = 0;
	pMac->lim.gHTControlledAccessOnly = 0;
	pMac->lim.gHTOperMode = eSIR_HT_OP_MODE_PURE;
	pMac->lim.gHTPCOActive = 0;

	pMac->lim.gHTPCOPhase = 0;
	pMac->lim.gHTSecondaryBeacon = 0;
	pMac->lim.gHTDualCTSProtection = 0;
	pMac->lim.gHTSTBCBasicMCS = 0;
}

static tSirRetStatus __lim_init_config(tpAniSirGlobal pMac)
{
	uint32_t val1, val2, val3;
	uint16_t val16;
	uint8_t val8;
	tSirMacHTCapabilityInfo *pHTCapabilityInfo;
	tSirMacHTInfoField1 *pHTInfoField1;
	tSirMacHTParametersInfo *pAmpduParamInfo;

	/* Read all the CFGs here that were updated before pe_start is called */
	/* All these CFG READS/WRITES are only allowed in init, at start when there is no session
	 * and they will be used throughout when there is no session
	 */

	if (wlan_cfg_get_int(pMac, WNI_CFG_ASSOC_STA_LIMIT, &val1)
		!= eSIR_SUCCESS){
		pe_err("cfg get assoc sta limit failed");
		return eSIR_FAILURE;
	}

	pMac->lim.gLimAssocStaLimit = val1;
	pMac->lim.gLimIbssStaLimit = val1;
	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS) {
		pe_err("could not retrieve HT Cap CFG");
		return eSIR_FAILURE;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_CHANNEL_BONDING_MODE, &val2) !=
	    eSIR_SUCCESS) {
		pe_err("could not retrieve Channel Bonding CFG");
		return eSIR_FAILURE;
	}
	val16 = (uint16_t) val1;
	pHTCapabilityInfo = (tSirMacHTCapabilityInfo *) &val16;

	/* channel bonding mode could be set to anything from 0 to 4(Titan had these */
	/* modes But for Taurus we have only two modes: enable(>0) or disable(=0) */
	pHTCapabilityInfo->supportedChannelWidthSet = val2 ?
						      WNI_CFG_CHANNEL_BONDING_MODE_ENABLE :
						      WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
	if (cfg_set_int
		    (pMac, WNI_CFG_HT_CAP_INFO, *(uint16_t *) pHTCapabilityInfo)
	    != eSIR_SUCCESS) {
		pe_err("could not update HT Cap Info CFG");
		return eSIR_FAILURE;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_INFO_FIELD1, &val1) != eSIR_SUCCESS) {
		pe_err("could not retrieve HT INFO Field1 CFG");
		return eSIR_FAILURE;
	}

	val8 = (uint8_t) val1;
	pHTInfoField1 = (tSirMacHTInfoField1 *) &val8;
	pHTInfoField1->recommendedTxWidthSet =
		(uint8_t) pHTCapabilityInfo->supportedChannelWidthSet;
	if (cfg_set_int(pMac, WNI_CFG_HT_INFO_FIELD1, *(uint8_t *) pHTInfoField1)
	    != eSIR_SUCCESS) {
		pe_err("could not update HT Info Field");
		return eSIR_FAILURE;
	}

	/* WNI_CFG_HEART_BEAT_THRESHOLD */

	if (wlan_cfg_get_int(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &val1) !=
	    eSIR_SUCCESS) {
		pe_err("could not retrieve WNI_CFG_HEART_BEAT_THRESHOLD CFG");
		return eSIR_FAILURE;
	}
	if (!val1) {
		pMac->sys.gSysEnableLinkMonitorMode = 0;
	} else {
		/* No need to activate the timer during init time. */
		pMac->sys.gSysEnableLinkMonitorMode = 1;
	}

	/* WNI_CFG_MAX_RX_AMPDU_FACTOR */

	if (wlan_cfg_get_int(pMac, WNI_CFG_HT_AMPDU_PARAMS, &val1) !=
	    eSIR_SUCCESS) {
		pe_err("could not retrieve HT AMPDU Param");
		return eSIR_FAILURE;
	}
	if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_RX_AMPDU_FACTOR, &val2) !=
	    eSIR_SUCCESS) {
		pe_err("could not retrieve AMPDU Factor CFG");
		return eSIR_FAILURE;
	}
	if (wlan_cfg_get_int(pMac, WNI_CFG_MPDU_DENSITY, &val3) !=
	    eSIR_SUCCESS) {
		pe_err("could not retrieve MPDU Density CFG");
		return eSIR_FAILURE;
	}

	val16 = (uint16_t) val1;
	pAmpduParamInfo = (tSirMacHTParametersInfo *) &val16;
	pAmpduParamInfo->maxRxAMPDUFactor = (uint8_t) val2;
	pAmpduParamInfo->mpduDensity = (uint8_t)val3;
	if (cfg_set_int
		    (pMac, WNI_CFG_HT_AMPDU_PARAMS,
		    *(uint8_t *) pAmpduParamInfo) != eSIR_SUCCESS) {
		pe_err("cfg get short preamble failed");
		return eSIR_FAILURE;
	}

	/* WNI_CFG_SHORT_PREAMBLE - this one is not updated in
	   lim_handle_cf_gparam_update do we want to update this? */
	if (wlan_cfg_get_int(pMac, WNI_CFG_SHORT_PREAMBLE, &val1) != eSIR_SUCCESS) {
		pe_err("cfg get short preamble failed");
		return eSIR_FAILURE;
	}

	/* WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA - not needed */

	/* This was initially done after resume notification from HAL. Now, DAL is
	   started before PE so this can be done here */
	handle_ht_capabilityand_ht_info(pMac, NULL);
	if (eSIR_SUCCESS !=
	    wlan_cfg_get_int(pMac, WNI_CFG_DISABLE_LDPC_WITH_TXBF_AP,
			     (uint32_t *) &pMac->lim.disableLDPCWithTxbfAP)) {
		pe_err("cfg get disableLDPCWithTxbfAP failed");
		return eSIR_FAILURE;
	}
#ifdef FEATURE_WLAN_TDLS
	if (eSIR_SUCCESS != wlan_cfg_get_int(pMac, WNI_CFG_TDLS_BUF_STA_ENABLED,
					     (uint32_t *) &pMac->lim.
					     gLimTDLSBufStaEnabled)) {
		pe_err("cfg get LimTDLSBufStaEnabled failed");
		return eSIR_FAILURE;
	}
	if (eSIR_SUCCESS !=
	    wlan_cfg_get_int(pMac, WNI_CFG_TDLS_QOS_WMM_UAPSD_MASK,
			     (uint32_t *) &pMac->lim.gLimTDLSUapsdMask)) {
		pe_err("cfg get LimTDLSUapsdMask failed");
		return eSIR_FAILURE;
	}
	if (eSIR_SUCCESS !=
	    wlan_cfg_get_int(pMac, WNI_CFG_TDLS_OFF_CHANNEL_ENABLED,
			     (uint32_t *) &pMac->lim.
			     gLimTDLSOffChannelEnabled)) {
		pe_err("cfg get LimTDLSUapsdMask failed");
		return eSIR_FAILURE;
	}

	if (eSIR_SUCCESS != wlan_cfg_get_int(pMac, WNI_CFG_TDLS_WMM_MODE_ENABLED,
					     (uint32_t *) &pMac->lim.
					     gLimTDLSWmmMode)) {
		pe_err("cfg get LimTDLSWmmMode failed");
		return eSIR_FAILURE;
	}
#endif

	if (eSIR_SUCCESS != wlan_cfg_get_int(pMac,
					     WNI_CFG_OBSS_DETECTION_OFFLOAD,
					     (uint32_t *)&pMac->lim.
					     global_obss_offload_enabled)) {
		pe_err("cfg get obss_detection_offloaded failed");
		return eSIR_FAILURE;
	}

	if (eSIR_SUCCESS !=
	    wlan_cfg_get_int(pMac, WNI_CFG_OBSS_COLOR_COLLISION_OFFLOAD,
			     (uint32_t *) &pMac->lim.
			     global_obss_color_collision_det_offload)) {
		pe_err("cfg get obss_color_collision_offload failed");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

/*
   lim_start
   This function is to replace the __lim_process_sme_start_req since there is no
   eWNI_SME_START_REQ post to PE.
 */
tSirRetStatus lim_start(tpAniSirGlobal pMac)
{
	tSirRetStatus retCode = eSIR_SUCCESS;

	pe_debug("enter");

	if (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) {
		pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;

		MTRACE(mac_trace
			       (pMac, TRACE_CODE_SME_STATE, NO_SESSION,
			       pMac->lim.gLimSmeState));

		/* By default do not return after first scan match */
		pMac->lim.gLimReturnAfterFirstMatch = 0;

		/* Initialize MLM state machine */
		if (eSIR_SUCCESS != lim_init_mlm(pMac)) {
			pe_err("Init MLM failed");
			return eSIR_FAILURE;
		}

		/* By default return unique scan results */
		pMac->lim.gLimReturnUniqueResults = true;
	} else {
		/**
		 * Should not have received eWNI_SME_START_REQ in states
		 * other than OFFLINE. Return response to host and
		 * log error
		 */
		pe_warn("Invalid SME state: %X",
			pMac->lim.gLimSmeState);
		retCode = eSIR_FAILURE;
	}

	pMac->lim.req_id =
		ucfg_scan_register_requester(pMac->psoc,
					     "LIM",
					     lim_process_rx_scan_handler,
					     pMac);
	return retCode;
}

/**
 * lim_initialize()
 *
 ***FUNCTION:
 * This function is called from LIM thread entry function.
 * LIM related global data structures are initialized in this function.
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac - Pointer to global MAC structure
 * @return None
 */

tSirRetStatus lim_initialize(tpAniSirGlobal pMac)
{
	tSirRetStatus status = eSIR_SUCCESS;

	pMac->lim.mgmtFrameSessionId = NO_SESSION;
	pMac->lim.tdls_frm_session_id = NO_SESSION;
	pMac->lim.deferredMsgCnt = 0;
	pMac->lim.retry_packet_cnt = 0;
	pMac->lim.ibss_retry_cnt = 0;
	pMac->lim.deauthMsgCnt = 0;
	pMac->lim.disassocMsgCnt = 0;

	if (QDF_IS_STATUS_ERROR(qdf_mutex_create(
			&pMac->lim.lkPeGlobalLock))) {
		pe_err("lim lock init failed!");
		return eSIR_FAILURE;
	}

	__lim_init_assoc_vars(pMac);
	__lim_init_vars(pMac);
	__lim_init_states(pMac);
	__lim_init_stats_vars(pMac);
	__lim_init_bss_vars(pMac);
	__lim_init_scan_vars(pMac);
	__lim_init_ht_vars(pMac);

	/* Initializations for maintaining peers in IBSS */
	lim_ibss_init(pMac);

	rrm_initialize(pMac);

	if (QDF_IS_STATUS_ERROR(qdf_mutex_create(
		&pMac->lim.lim_frame_register_lock))) {
		pe_err("lim lock init failed!");
		qdf_mutex_destroy(&pMac->lim.lkPeGlobalLock);
		return eSIR_FAILURE;
	}

	qdf_list_create(&pMac->lim.gLimMgmtFrameRegistratinQueue, 0);

	/* initialize the TSPEC admission control table. */
	/* Note that this was initially done after resume notification from HAL. */
	/* Now, DAL is started before PE so this can be done here */
	lim_admit_control_init(pMac);
	return status;

} /*** end lim_initialize() ***/

/**
 * lim_cleanup()
 *
 ***FUNCTION:
 * This function is called upon reset or persona change
 * to cleanup LIM state
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void lim_cleanup(tpAniSirGlobal pMac)
{
	uint8_t i;
	qdf_list_node_t *lst_node;

	/*
	 * Before destroying the list making sure all the nodes have been
	 * deleted
	 */
	while (qdf_list_remove_front(
			&pMac->lim.gLimMgmtFrameRegistratinQueue,
			&lst_node) == QDF_STATUS_SUCCESS) {
		qdf_mem_free(lst_node);
	}
	qdf_list_destroy(&pMac->lim.gLimMgmtFrameRegistratinQueue);
	qdf_mutex_destroy(&pMac->lim.lim_frame_register_lock);

	pe_deregister_mgmt_rx_frm_callback(pMac);

	qdf_mem_free(pMac->lim.gpLimRemainOnChanReq);
	pMac->lim.gpLimRemainOnChanReq = NULL;

	/* free up preAuth table */
	if (pMac->lim.gLimPreAuthTimerTable.pTable != NULL) {
		for (i = 0; i < pMac->lim.gLimPreAuthTimerTable.numEntry; i++)
			qdf_mem_free(pMac->lim.gLimPreAuthTimerTable.pTable[i]);
		qdf_mem_free(pMac->lim.gLimPreAuthTimerTable.pTable);
		pMac->lim.gLimPreAuthTimerTable.pTable = NULL;
		pMac->lim.gLimPreAuthTimerTable.numEntry = 0;
	}

	if (NULL != pMac->lim.pDialogueTokenHead) {
		lim_delete_dialogue_token_list(pMac);
	}

	if (NULL != pMac->lim.pDialogueTokenTail) {
		qdf_mem_free(pMac->lim.pDialogueTokenTail);
		pMac->lim.pDialogueTokenTail = NULL;
	}

	if (pMac->lim.gpLimMlmSetKeysReq != NULL) {
		qdf_mem_free(pMac->lim.gpLimMlmSetKeysReq);
		pMac->lim.gpLimMlmSetKeysReq = NULL;
	}

	if (pMac->lim.gpLimMlmAuthReq != NULL) {
		qdf_mem_free(pMac->lim.gpLimMlmAuthReq);
		pMac->lim.gpLimMlmAuthReq = NULL;
	}

	if (pMac->lim.gpDefdSmeMsgForNOA != NULL) {
		qdf_mem_free(pMac->lim.gpDefdSmeMsgForNOA);
		pMac->lim.gpDefdSmeMsgForNOA = NULL;
	}

	if (pMac->lim.limDisassocDeauthCnfReq.pMlmDisassocReq) {
		qdf_mem_free(pMac->lim.limDisassocDeauthCnfReq.pMlmDisassocReq);
		pMac->lim.limDisassocDeauthCnfReq.pMlmDisassocReq = NULL;
	}

	if (pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq) {
		qdf_mem_free(pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq);
		pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq = NULL;
	}

	/* Now, finally reset the deferred message queue pointers */
	lim_reset_deferred_msg_q(pMac);

	rrm_cleanup(pMac);

	lim_ft_cleanup_all_ft_sessions(pMac);

	ucfg_scan_unregister_requester(pMac->psoc, pMac->lim.req_id);
} /*** end lim_cleanup() ***/

/**
 * lim_state_info_dump() - print state information of lim layer
 * @buf: buffer pointer
 * @size: size of buffer to be filled
 *
 * This function is used to print state information of lim layer
 *
 * Return: None
 */
static void lim_state_info_dump(char **buf_ptr, uint16_t *size)
{
	tHalHandle hal;
	tpAniSirGlobal mac;
	uint16_t len = 0;
	char *buf = *buf_ptr;

	hal = cds_get_context(QDF_MODULE_ID_PE);
	if (hal == NULL) {
		QDF_ASSERT(0);
		return;
	}

	mac = PMAC_STRUCT(hal);

	pe_debug("size of buffer: %d", *size);

	len += qdf_scnprintf(buf + len, *size - len,
		"\n SmeState: %d", mac->lim.gLimSmeState);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n PrevSmeState: %d", mac->lim.gLimPrevSmeState);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n MlmState: %d", mac->lim.gLimMlmState);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n PrevMlmState: %d", mac->lim.gLimPrevMlmState);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n SystemInScanLearnMode: %d",
		mac->lim.gLimSystemInScanLearnMode);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n ProcessDefdMsgs: %d", mac->lim.gLimProcessDefdMsgs);
	len += qdf_scnprintf(buf + len, *size - len,
		"\n gLimHalScanState: %d", mac->lim.gLimHalScanState);

	*size -= len;
	*buf_ptr += len;
}

/**
 * lim_register_debug_callback() - registration function for lim layer
 * to print lim state information
 *
 * Return: None
 */
static void lim_register_debug_callback(void)
{
	qdf_register_debug_callback(QDF_MODULE_ID_PE, &lim_state_info_dump);
}

static void lim_nan_register_callbacks(tpAniSirGlobal mac_ctx)
{
	struct nan_callbacks cb_obj = {0};

	cb_obj.add_ndi_peer = lim_add_ndi_peer_converged;
	cb_obj.ndp_delete_peers = lim_ndp_delete_peers_converged;
	cb_obj.delete_peers_by_addr = lim_ndp_delete_peers_by_addr_converged;

	ucfg_nan_register_lim_callbacks(mac_ctx->psoc, &cb_obj);
}

/*
 * pe_shutdown_notifier_cb - Shutdown notifier callback
 * @ctx: Pointer to Global MAC structure
 *
 * Return: None
 */
static void pe_shutdown_notifier_cb(void *ctx)
{
	tpAniSirGlobal mac_ctx = (tpAniSirGlobal)ctx;
	tpPESession session;
	uint8_t i;

	lim_deactivate_timers(mac_ctx);
	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		session = &mac_ctx->lim.gpSession[i];
		if (session->valid == true) {
			if (LIM_IS_AP_ROLE(session))
				qdf_mc_timer_stop(&session->
						 protection_fields_reset_timer);
#ifdef WLAN_FEATURE_11W
			qdf_mc_timer_stop(&session->pmfComebackTimer);
#endif
		}
	}
}

#ifdef WLAN_FEATURE_11W
/**
 * is_mgmt_protected - check RMF enabled for the peer
 * @vdev_id: vdev id
 * @peer_mac_addr: peer mac address
 *
 * The function check the mgmt frame protection enabled or not
 * for station mode and AP mode
 *
 * Return: true, if the connection is RMF enabled.
 */
static bool is_mgmt_protected(uint32_t vdev_id,
				  const uint8_t *peer_mac_addr)
{
	uint16_t aid;
	tpDphHashNode sta_ds;
	tpPESession session;
	bool protected = false;
	tpAniSirGlobal mac_ctx = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac_ctx)
		return false;

	session = pe_find_session_by_sme_session_id(mac_ctx,
						    vdev_id);
	if (!session) {
		/* couldn't find session */
		pe_err("Session not found for vdev_id: %d", vdev_id);
		return false;
	}

	if (LIM_IS_AP_ROLE(session)) {
		sta_ds = dph_lookup_hash_entry(mac_ctx,
					       (uint8_t *)peer_mac_addr, &aid,
					       &session->dph.dphHashTable);
		if (sta_ds) {
			/* rmfenabled will be set at the time of addbss.
			 * but sometimes EAP auth fails and keys are not
			 * installed then if we send any management frame
			 * like deauth/disassoc with this bit set then
			 * firmware crashes. so check for keys are
			 * installed or not also before setting the bit
			 */
			if (sta_ds->rmfEnabled && sta_ds->is_key_installed)
				protected = true;
		}
	} else if (session->limRmfEnabled &&
		   session->is_key_installed) {
		protected = true;
	}

	return protected;
}
#else
/**
 * is_mgmt_protected - check RMF enabled for the peer
 * @vdev_id: vdev id
 * @peer_mac_addr: peer mac address
 *
 * The function check the mgmt frame protection enabled or not
 * for station mode and AP mode
 *
 * Return: true, if the connection is RMF enabled.
 */
static bool is_mgmt_protected(uint32_t vdev_id,
				  const uint8_t *peer_mac_addr)
{
	return false;
}
#endif

static void p2p_register_callbacks(tpAniSirGlobal mac_ctx)
{
	struct p2p_protocol_callbacks p2p_cb = {0};

	p2p_cb.is_mgmt_protected = is_mgmt_protected;
	ucfg_p2p_register_callbacks(mac_ctx->psoc, &p2p_cb);
}

/** -------------------------------------------------------------
   \fn pe_open
   \brief will be called in Open sequence from mac_open
   \param   tpAniSirGlobal pMac
   \param   tHalOpenParameters *pHalOpenParam
   \return  tSirRetStatus
   -------------------------------------------------------------*/

tSirRetStatus pe_open(tpAniSirGlobal pMac, struct cds_config_info *cds_cfg)
{
	tSirRetStatus status = eSIR_SUCCESS;

	if (QDF_DRIVER_TYPE_MFG == cds_cfg->driver_type)
		return eSIR_SUCCESS;

	pMac->lim.maxBssId = cds_cfg->max_bssid;
	pMac->lim.maxStation = cds_cfg->max_station;
	qdf_spinlock_create(&pMac->sys.bbt_mgmt_lock);

	if ((pMac->lim.maxBssId == 0) || (pMac->lim.maxStation == 0)) {
		pe_err("max number of Bssid or Stations cannot be zero!");
		return eSIR_FAILURE;
	}

	pMac->lim.limTimers.gpLimCnfWaitTimer =
		qdf_mem_malloc(sizeof(TX_TIMER) * (pMac->lim.maxStation + 1));
	if (NULL == pMac->lim.limTimers.gpLimCnfWaitTimer) {
		pe_err("gpLimCnfWaitTimer memory allocate failed!");
		return eSIR_MEM_ALLOC_FAILED;
	}

	pMac->lim.gpSession =
		qdf_mem_malloc(sizeof(tPESession) * pMac->lim.maxBssId);
	if (NULL == pMac->lim.gpSession) {
		pe_err("gpSession memory allocate failed!");
		status = eSIR_MEM_ALLOC_FAILED;
		goto pe_open_psession_fail;
	}

	status = lim_initialize(pMac);
	if (eSIR_SUCCESS != status) {
		pe_err("lim_initialize failed!");
		status = eSIR_FAILURE;
		goto  pe_open_lock_fail;
	}

	/*
	 * pe_open is successful by now, so it is right time to initialize
	 * MTRACE for PE module. if LIM_TRACE_RECORD is not defined in build
	 * file then nothing will be logged for PE module.
	 */
#ifdef LIM_TRACE_RECORD
	MTRACE(lim_trace_init(pMac));
#endif
	lim_register_debug_callback();
	lim_nan_register_callbacks(pMac);
	p2p_register_callbacks(pMac);

	if (!QDF_IS_STATUS_SUCCESS(
	    cds_shutdown_notifier_register(pe_shutdown_notifier_cb, pMac))) {
		pe_err("%s: Shutdown notifier register failed", __func__);
	}

	return status; /* status here will be eSIR_SUCCESS */

pe_open_lock_fail:
	qdf_mem_free(pMac->lim.gpSession);
	pMac->lim.gpSession = NULL;
pe_open_psession_fail:
	qdf_mem_free(pMac->lim.limTimers.gpLimCnfWaitTimer);
	pMac->lim.limTimers.gpLimCnfWaitTimer = NULL;

	return status;
}

/** -------------------------------------------------------------
   \fn pe_close
   \brief will be called in close sequence from mac_close
   \param   tpAniSirGlobal pMac
   \return  tSirRetStatus
   -------------------------------------------------------------*/

tSirRetStatus pe_close(tpAniSirGlobal pMac)
{
	uint8_t i;

	if (ANI_DRIVER_TYPE(pMac) == QDF_DRIVER_TYPE_MFG)
		return eSIR_SUCCESS;

	lim_cleanup(pMac);

	if (pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq) {
		qdf_mem_free(pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq);
		pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq = NULL;
	}

	qdf_spinlock_destroy(&pMac->sys.bbt_mgmt_lock);
	for (i = 0; i < pMac->lim.maxBssId; i++) {
		if (pMac->lim.gpSession[i].valid == true)
			pe_delete_session(pMac, &pMac->lim.gpSession[i]);
	}
	qdf_mem_free(pMac->lim.limTimers.gpLimCnfWaitTimer);
	pMac->lim.limTimers.gpLimCnfWaitTimer = NULL;

	qdf_mem_free(pMac->lim.gpSession);
	pMac->lim.gpSession = NULL;
	if (!QDF_IS_STATUS_SUCCESS
		    (qdf_mutex_destroy(&pMac->lim.lkPeGlobalLock))) {
		return eSIR_FAILURE;
	}
	return eSIR_SUCCESS;
}

/** -------------------------------------------------------------
   \fn pe_start
   \brief will be called in start sequence from mac_start
   \param   tpAniSirGlobal pMac
   \return none
   -------------------------------------------------------------*/

tSirRetStatus pe_start(tpAniSirGlobal pMac)
{
	tSirRetStatus status = eSIR_SUCCESS;
	status = lim_start(pMac);
	if (eSIR_SUCCESS != status) {
		pe_err("lim_start failed!");
		return status;
	}
	/* Initialize the configurations needed by PE */
	if (eSIR_FAILURE == __lim_init_config(pMac)) {
		pe_err("lim init config failed!");
		/* We need to undo everything in lim_start */
		lim_cleanup_mlm(pMac);
		return eSIR_FAILURE;
	}
	/* Initialize the configurations needed by PE */
	lim_register_hal_ind_call_back(pMac);
	return status;
}

/** -------------------------------------------------------------
   \fn pe_stop
   \brief will be called in stop sequence from mac_stop
   \param   tpAniSirGlobal pMac
   \return none
   -------------------------------------------------------------*/

void pe_stop(tpAniSirGlobal pMac)
{
	lim_cleanup_mlm(pMac);
	SET_LIM_MLM_STATE(pMac, eLIM_MLM_OFFLINE_STATE);
	return;
}

/** -------------------------------------------------------------
   \fn pe_free_msg
   \brief Called by CDS scheduler (function cds_sched_flush_mc_mqs)
 \      to free a given PE message on the TX and MC thread.
 \      This happens when there are messages pending in the PE
 \      queue when system is being stopped and reset.
   \param   tpAniSirGlobal pMac
   \param   struct scheduler_msg       pMsg
   \return none
   -----------------------------------------------------------------*/
void pe_free_msg(tpAniSirGlobal pMac, struct scheduler_msg *pMsg)
{
	if (pMsg != NULL) {
		if (NULL != pMsg->bodyptr) {
			if (SIR_BB_XPORT_MGMT_MSG == pMsg->type) {
				cds_pkt_return_packet((cds_pkt_t *) pMsg->
						      bodyptr);
			} else {
				qdf_mem_free((void *)pMsg->bodyptr);
			}
		}
		pMsg->bodyptr = 0;
		pMsg->bodyval = 0;
		pMsg->type = 0;
	}
	return;
}

/**
 * lim_post_msg_api()
 *
 ***FUNCTION:
 * This function is called from other thread while posting a
 * message to LIM message Queue gSirLimMsgQ.
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pMsg - Pointer to the message structure
 * @return None
 */

uint32_t lim_post_msg_api(tpAniSirGlobal pMac, struct scheduler_msg *pMsg)
{
	return scheduler_post_msg(QDF_MODULE_ID_PE,  pMsg);

} /*** end lim_post_msg_api() ***/

/**
 * lim_post_msg_high_priority() - posts high priority pe message
 * @mac: mac context
 * @msg: message to be posted
 *
 * This function is used to post high priority pe message
 *
 * Return: returns value returned by vos_mq_post_message_by_priority
 */
uint32_t lim_post_msg_high_priority(tpAniSirGlobal mac,
				    struct scheduler_msg *msg)
{
	return scheduler_post_msg_by_priority(QDF_MODULE_ID_PE,
					       msg, true);
}

/*--------------------------------------------------------------------------

   \brief pe_post_msg_api() - A wrapper function to post message to Voss msg queues

   This function can be called by legacy code to post message to cds queues OR
   legacy code may keep on invoking 'lim_post_msg_api' to post the message to cds queue
   for dispatching it later.

   \param pMac - Pointer to Global MAC structure
   \param pMsg - Pointer to the message structure

   \return  uint32_t - TX_SUCCESS for success.

   --------------------------------------------------------------------------*/

tSirRetStatus pe_post_msg_api(tpAniSirGlobal pMac, struct scheduler_msg *pMsg)
{
	return (tSirRetStatus) lim_post_msg_api(pMac, pMsg);
}

/*--------------------------------------------------------------------------

   \brief pe_process_messages() - Message Processor for PE

   Voss calls this function to dispatch the message to PE

   \param pMac - Pointer to Global MAC structure
   \param pMsg - Pointer to the message structure

   \return  uint32_t - TX_SUCCESS for success.

   --------------------------------------------------------------------------*/

tSirRetStatus pe_process_messages(tpAniSirGlobal pMac,
				  struct scheduler_msg *pMsg)
{
	if (ANI_DRIVER_TYPE(pMac) == QDF_DRIVER_TYPE_MFG) {
		return eSIR_SUCCESS;
	}
	/**
	 * If the Message to be handled is for CFG Module call the CFG Msg
	 * Handler and for all the other cases post it to LIM
	 */
	if (SIR_CFG_PARAM_UPDATE_IND != pMsg->type && IS_CFG_MSG(pMsg->type))
		cfg_process_mb_msg(pMac, (tSirMbMsg *) pMsg->bodyptr);
	else
		lim_message_processor(pMac, pMsg);
	return eSIR_SUCCESS;
}

QDF_STATUS pe_mc_process_handler(struct scheduler_msg *msg)
{
	tSirRetStatus status;
	tpAniSirGlobal mac_ctx = cds_get_context(QDF_MODULE_ID_PE);

	if (mac_ctx == NULL)
		return QDF_STATUS_E_FAILURE;

	status = pe_process_messages((tHalHandle)mac_ctx, msg);
	if (status == eSIR_SUCCESS)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

/**
 * pe_drop_pending_rx_mgmt_frames: To drop pending RX mgmt frames
 * @mac_ctx: Pointer to global MAC structure
 * @hdr: Management header
 * @cds_pkt: Packet
 *
 * This function is used to drop RX pending mgmt frames if pe mgmt queue
 * reaches threshold
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS pe_drop_pending_rx_mgmt_frames(tpAniSirGlobal mac_ctx,
				tpSirMacMgmtHdr hdr, cds_pkt_t *cds_pkt)
{
	qdf_spin_lock(&mac_ctx->sys.bbt_mgmt_lock);
	if (mac_ctx->sys.sys_bbt_pending_mgmt_count >=
	     MGMT_RX_PACKETS_THRESHOLD) {
		qdf_spin_unlock(&mac_ctx->sys.bbt_mgmt_lock);
		pe_debug("No.of pending RX management frames reaches to threshold, dropping management frames");
		cds_pkt_return_packet(cds_pkt);
		cds_pkt = NULL;
		mac_ctx->rx_packet_drop_counter++;
		return QDF_STATUS_E_FAILURE;
	} else if (mac_ctx->sys.sys_bbt_pending_mgmt_count >
		   (MGMT_RX_PACKETS_THRESHOLD / 2)) {
		/* drop all probereq, proberesp and beacons */
		if (hdr->fc.subType == SIR_MAC_MGMT_BEACON ||
		    hdr->fc.subType == SIR_MAC_MGMT_PROBE_REQ ||
		    hdr->fc.subType == SIR_MAC_MGMT_PROBE_RSP) {
			qdf_spin_unlock(&mac_ctx->sys.bbt_mgmt_lock);
			if (!(mac_ctx->rx_packet_drop_counter % 100))
				pe_debug("No.of pending RX mgmt frames reaches 1/2 thresh, dropping frame subtype: %d rx_packet_drop_counter: %d",
					hdr->fc.subType,
					mac_ctx->rx_packet_drop_counter);
			mac_ctx->rx_packet_drop_counter++;
			cds_pkt_return_packet(cds_pkt);
			cds_pkt = NULL;
			return QDF_STATUS_E_FAILURE;
		}
	}
	mac_ctx->sys.sys_bbt_pending_mgmt_count++;
	qdf_spin_unlock(&mac_ctx->sys.bbt_mgmt_lock);
	if (mac_ctx->sys.sys_bbt_pending_mgmt_count ==
	    (MGMT_RX_PACKETS_THRESHOLD / 4)) {
		if (!(mac_ctx->rx_packet_drop_counter % 100))
			pe_debug("No.of pending RX management frames reaches to 1/4th of threshold, rx_packet_drop_counter: %d",
				mac_ctx->rx_packet_drop_counter);
			mac_ctx->rx_packet_drop_counter++;
	}
	return QDF_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------- */
/**
 * pe_handle_mgmt_frame() - Process the Management frames from TXRX
 * @psoc: psoc context
 * @peer: peer
 * @buf: buffer
 * @mgmt_rx_params; rx event params
 * @frm_type: frame type
 *
 * This function handles the mgmt rx frame from mgmt txrx component and forms
 * a cds packet and schedule it in controller thread for further processing.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
 #define ERROR_LOG_RATE_LIMIT     16
static QDF_STATUS pe_handle_mgmt_frame(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_peer *peer, qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params,
			uint32_t frm_type)
{
	tpAniSirGlobal pMac;
	tpSirMacMgmtHdr mHdr;
	struct scheduler_msg msg = {0};
	cds_pkt_t *pVosPkt;
	QDF_STATUS qdf_status;
	uint8_t *pRxPacketInfo;
	int ret;

	pMac = cds_get_context(QDF_MODULE_ID_PE);
	if (NULL == pMac) {
		/* cannot log a failure without a valid pMac */
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	pVosPkt = qdf_mem_malloc(sizeof(*pVosPkt));
	if (!pVosPkt) {
		pe_err("Failed to allocate rx packet");
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_NOMEM;
	}

	ret = wma_form_rx_packet(buf, mgmt_rx_params, pVosPkt);
	if (ret) {
		pe_err_rate_limited(ERROR_LOG_RATE_LIMIT, "Failed to fill cds packet from event buffer");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_status =
		wma_ds_peek_rx_packet_info(pVosPkt, (void *)&pRxPacketInfo, false);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		cds_pkt_return_packet(pVosPkt);
		pVosPkt = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * The MPDU header is now present at a certain "offset" in
	 * the BD and is specified in the BD itself
	 */

	mHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);

	if (QDF_STATUS_SUCCESS !=
	    pe_drop_pending_rx_mgmt_frames(pMac, mHdr, pVosPkt))
		return QDF_STATUS_E_FAILURE;

	/* Forward to MAC via mesg = SIR_BB_XPORT_MGMT_MSG */
	msg.type = SIR_BB_XPORT_MGMT_MSG;
	msg.bodyptr = pVosPkt;
	msg.bodyval = 0;

	if (eSIR_SUCCESS != sys_bbt_process_message_core(pMac,
							 &msg,
							 mHdr->fc.type,
							 mHdr->fc.subType)) {
		cds_pkt_return_packet(pVosPkt);
		pVosPkt = NULL;
		/*
		 * Decrement sys_bbt_pending_mgmt_count if packet
		 * is dropped before posting to LIM
		 */
		lim_decrement_pending_mgmt_count(pMac);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void pe_register_mgmt_rx_frm_callback(tpAniSirGlobal mac_ctx)
{
	QDF_STATUS status;
	struct mgmt_txrx_mgmt_frame_cb_info frm_cb_info;

	frm_cb_info.frm_type = MGMT_FRAME_TYPE_ALL;
	frm_cb_info.mgmt_rx_cb = (mgmt_frame_rx_callback)
				pe_handle_mgmt_frame;

	status = wlan_mgmt_txrx_register_rx_cb(mac_ctx->psoc,
					 WLAN_UMAC_COMP_MLME, &frm_cb_info, 1);
	if (status != QDF_STATUS_SUCCESS)
		pe_err("Registering the PE Handle with MGMT TXRX layer has failed");

	wma_register_mgmt_frm_client();
}

void pe_deregister_mgmt_rx_frm_callback(tpAniSirGlobal mac_ctx)
{
	QDF_STATUS status;
	struct mgmt_txrx_mgmt_frame_cb_info frm_cb_info;

	frm_cb_info.frm_type = MGMT_FRAME_TYPE_ALL;
	frm_cb_info.mgmt_rx_cb = (mgmt_frame_rx_callback)
				pe_handle_mgmt_frame;

	status = wlan_mgmt_txrx_deregister_rx_cb(mac_ctx->psoc,
					 WLAN_UMAC_COMP_MLME, &frm_cb_info, 1);
	if (status != QDF_STATUS_SUCCESS)
		pe_err("Deregistering the PE Handle with MGMT TXRX layer has failed");

	wma_de_register_mgmt_frm_client();
}


/**
 * pe_register_callbacks_with_wma() - register SME and PE callback functions to
 * WMA.
 * (function documentation in lim_api.h)
 */
void pe_register_callbacks_with_wma(tpAniSirGlobal pMac,
				    tSirSmeReadyReq *ready_req)
{
	QDF_STATUS status;

	status = wma_register_roaming_callbacks(
			ready_req->csr_roam_synch_cb,
			ready_req->pe_roam_synch_cb);
	if (status != QDF_STATUS_SUCCESS)
		pe_err("Registering roaming callbacks with WMA failed");
}

/**
 * lim_is_system_in_scan_state()
 *
 ***FUNCTION:
 * This function is called by various MAC software modules to
 * determine if System is in Scan/Learn state
 *
 ***LOGIC:
 * NA
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac  - Pointer to Global MAC structure
 * @return true  - System is in Scan/Learn state
 *         false - System is NOT in Scan/Learn state
 */

uint8_t lim_is_system_in_scan_state(tpAniSirGlobal pMac)
{
	switch (pMac->lim.gLimSmeState) {
	case eLIM_SME_CHANNEL_SCAN_STATE:
	case eLIM_SME_NORMAL_CHANNEL_SCAN_STATE:
	case eLIM_SME_LINK_EST_WT_SCAN_STATE:
	case eLIM_SME_WT_SCAN_STATE:
		/* System is in Learn mode */
		return true;

	default:
		/* System is NOT in Learn mode */
		return false;
	}
} /*** end lim_is_system_in_scan_state() ***/

/**
 *\brief lim_received_hb_handler()
 *
 * This function is called by sch_beacon_process() upon
 * receiving a Beacon on STA. This also gets called upon
 * receiving Probe Response after heat beat failure is
 * detected.
 *
 * param pMac - global mac structure
 * param channel - channel number indicated in Beacon, Probe Response
 * return - none
 */

void
lim_received_hb_handler(tpAniSirGlobal pMac, uint8_t channelId,
			tpPESession psessionEntry)
{
	if ((channelId == 0)
	    || (channelId == psessionEntry->currentOperChannel))
		psessionEntry->LimRxedBeaconCntDuringHB++;

	psessionEntry->pmmOffloadInfo.bcnmiss = false;
} /*** lim_init_wds_info_params() ***/

/** -------------------------------------------------------------
   \fn lim_update_overlap_sta_param
   \brief Updates overlap cache and param data structure
   \param      tpAniSirGlobal    pMac
   \param      tSirMacAddr bssId
   \param      tpLimProtStaParams pStaParams
   \return      None
   -------------------------------------------------------------*/
void
lim_update_overlap_sta_param(tpAniSirGlobal pMac, tSirMacAddr bssId,
			     tpLimProtStaParams pStaParams)
{
	int i;

	if (!pStaParams->numSta) {
		qdf_mem_copy(pMac->lim.protStaOverlapCache[0].addr,
			     bssId, sizeof(tSirMacAddr));
		pMac->lim.protStaOverlapCache[0].active = true;

		pStaParams->numSta = 1;

		return;
	}

	for (i = 0; i < LIM_PROT_STA_OVERLAP_CACHE_SIZE; i++) {
		if (pMac->lim.protStaOverlapCache[i].active) {
			if (!qdf_mem_cmp
				    (pMac->lim.protStaOverlapCache[i].addr, bssId,
				    sizeof(tSirMacAddr))) {
				return;
			}
		} else
			break;
	}

	if (i == LIM_PROT_STA_OVERLAP_CACHE_SIZE) {
		pe_debug("Overlap cache is full");
	} else {
		qdf_mem_copy(pMac->lim.protStaOverlapCache[i].addr,
			     bssId, sizeof(tSirMacAddr));
		pMac->lim.protStaOverlapCache[i].active = true;

		pStaParams->numSta++;
	}
}

/**
 * lim_ibss_enc_type_matched
 *
 ***FUNCTION:
 * This function compares the encryption type of the peer with self
 * while operating in IBSS mode and detects mismatch.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pBeacon  - Parsed Beacon Frame structure
 * @param  pSession - Pointer to the PE session
 *
 * @return true if encryption type is matched; false otherwise
 */
static bool lim_ibss_enc_type_matched(tpSchBeaconStruct pBeacon,
					  tpPESession pSession)
{
	if (!pBeacon || !pSession)
		return false;

	/* Open case */
	if (pBeacon->capabilityInfo.privacy == 0
	    && pSession->encryptType == eSIR_ED_NONE)
		return true;

	/* WEP case */
	if (pBeacon->capabilityInfo.privacy == 1 && pBeacon->wpaPresent == 0
	    && pBeacon->rsnPresent == 0
	    && (pSession->encryptType == eSIR_ED_WEP40
		|| pSession->encryptType == eSIR_ED_WEP104))
		return true;

	/* WPA-None case */
	if (pBeacon->capabilityInfo.privacy == 1 && pBeacon->wpaPresent == 1
	    && pBeacon->rsnPresent == 0
	    && ((pSession->encryptType == eSIR_ED_CCMP) ||
		(pSession->encryptType == eSIR_ED_GCMP) ||
		(pSession->encryptType == eSIR_ED_GCMP_256) ||
		(pSession->encryptType == eSIR_ED_TKIP)))
		return true;

	return false;
}

/**
 * lim_handle_ibs_scoalescing()
 *
 ***FUNCTION:
 * This function is called upon receiving Beacon/Probe Response
 * while operating in IBSS mode.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pRxPacketInfo - Pointer to RX packet info structure
 *
 * @return Status whether to process or ignore received Beacon Frame
 */

tSirRetStatus
lim_handle_ibss_coalescing(tpAniSirGlobal pMac,
			   tpSchBeaconStruct pBeacon,
			   uint8_t *pRxPacketInfo, tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	tSirRetStatus retCode;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);

	/* Ignore the beacon when any of the conditions below is met:
	   1. The beacon claims no IBSS network
	   2. SSID in the beacon does not match SSID of self station
	   3. Operational channel in the beacon does not match self station
	   4. Encyption type in the beacon does not match with self station
	 */
	if ((!pBeacon->capabilityInfo.ibss) ||
	    lim_cmp_ssid(&pBeacon->ssId, psessionEntry) ||
	    (psessionEntry->currentOperChannel != pBeacon->channelNumber))
		retCode = eSIR_LIM_IGNORE_BEACON;
	else if (lim_ibss_enc_type_matched(pBeacon, psessionEntry) != true) {
		pe_debug("peer privacy: %d peer wpa: %d peer rsn: %d self encType: %d",
			       pBeacon->capabilityInfo.privacy,
			       pBeacon->wpaPresent, pBeacon->rsnPresent,
			       psessionEntry->encryptType);
		retCode = eSIR_LIM_IGNORE_BEACON;
	} else {
		uint32_t ieLen;
		uint16_t tsfLater;
		uint8_t *pIEs;

		ieLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
		tsfLater = WMA_GET_RX_TSF_LATER(pRxPacketInfo);
		pIEs = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
		pe_debug("BEFORE Coalescing tsfLater val: %d", tsfLater);
		retCode =
			lim_ibss_coalesce(pMac, pHdr, pBeacon, pIEs, ieLen, tsfLater,
					  psessionEntry);
	}
	return retCode;
} /*** end lim_handle_ibs_scoalescing() ***/

/**
 * lim_enc_type_matched() - matches security type of incoming beracon with
 * current
 * @mac_ctx      Pointer to Global MAC structure
 * @bcn          Pointer to parsed Beacon structure
 * @session      PE session entry
 *
 * This function matches security type of incoming beracon with current
 *
 * @return true if matched, false otherwise
 */
static bool
lim_enc_type_matched(tpAniSirGlobal mac_ctx,
		     tpSchBeaconStruct bcn,
		     tpPESession session)
{
	if (!bcn || !session)
		return false;

	pe_debug("Beacon/Probe:: Privacy: %d WPA Present: %d RSN Present: %d",
		bcn->capabilityInfo.privacy, bcn->wpaPresent, bcn->rsnPresent);
	pe_debug("session:: Privacy: %d EncyptionType: %d OSEN: %d WPS: %d",
		SIR_MAC_GET_PRIVACY(session->limCurrentBssCaps),
		session->encryptType, session->isOSENConnection,
		session->wps_registration);

	/*
	 * This is handled by sending probe req due to IOT issues so
	 * return TRUE
	 */
	if ((bcn->capabilityInfo.privacy) !=
		SIR_MAC_GET_PRIVACY(session->limCurrentBssCaps)) {
		pe_warn("Privacy bit miss match");
		return true;
	}

	/* Open */
	if ((bcn->capabilityInfo.privacy == 0) &&
	    (session->encryptType == eSIR_ED_NONE))
		return true;

	/* WEP */
	if ((bcn->capabilityInfo.privacy == 1) &&
	    (bcn->wpaPresent == 0) && (bcn->rsnPresent == 0) &&
	    ((session->encryptType == eSIR_ED_WEP40) ||
		(session->encryptType == eSIR_ED_WEP104)
#ifdef FEATURE_WLAN_WAPI
		|| (session->encryptType == eSIR_ED_WPI)
#endif
	    ))
		return true;

	/* WPA OR RSN*/
	if ((bcn->capabilityInfo.privacy == 1) &&
	    ((bcn->wpaPresent == 1) || (bcn->rsnPresent == 1)) &&
	    ((session->encryptType == eSIR_ED_TKIP) ||
		(session->encryptType == eSIR_ED_CCMP) ||
		(session->encryptType == eSIR_ED_GCMP) ||
		(session->encryptType == eSIR_ED_GCMP_256) ||
		(session->encryptType == eSIR_ED_AES_128_CMAC)))
		return true;

	/*
	 * For HS2.0, RSN ie is not present
	 * in beacon. Therefore no need to
	 * check for security type in case
	 * OSEN session.
	 * For WPS registration session no need to detect
	 * detect security mismatch as it wont match and
	 * driver may end up sending probe request without
	 * WPS IE during WPS registration process.
	 */
	if (session->isOSENConnection ||
	   session->wps_registration)
		return true;

	return false;
}

/**
 * lim_detect_change_in_ap_capabilities()
 *
 ***FUNCTION:
 * This function is called while SCH is processing
 * received Beacon from AP on STA to detect any
 * change in AP's capabilities. If there any change
 * is detected, Roaming is informed of such change
 * so that it can trigger reassociation.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 * Notification is enabled for STA product only since
 * it is not a requirement on BP side.
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  pBeacon   Pointer to parsed Beacon structure
 * @return None
 */

void
lim_detect_change_in_ap_capabilities(tpAniSirGlobal pMac,
				     tpSirProbeRespBeacon pBeacon,
				     tpPESession psessionEntry)
{
	uint8_t len;
	tSirSmeApNewCaps apNewCaps;
	uint8_t newChannel;
	tSirRetStatus status = eSIR_SUCCESS;
	bool security_caps_matched = true;

	apNewCaps.capabilityInfo =
		lim_get_u16((uint8_t *) &pBeacon->capabilityInfo);
	newChannel = (uint8_t) pBeacon->channelNumber;

	security_caps_matched = lim_enc_type_matched(pMac, pBeacon,
						     psessionEntry);
	if ((false == psessionEntry->limSentCapsChangeNtf) &&
	    (((!lim_is_null_ssid(&pBeacon->ssId)) &&
	       lim_cmp_ssid(&pBeacon->ssId, psessionEntry)) ||
	     ((SIR_MAC_GET_ESS(apNewCaps.capabilityInfo) !=
	       SIR_MAC_GET_ESS(psessionEntry->limCurrentBssCaps)) ||
	      (SIR_MAC_GET_PRIVACY(apNewCaps.capabilityInfo) !=
	       SIR_MAC_GET_PRIVACY(psessionEntry->limCurrentBssCaps)) ||
	      (SIR_MAC_GET_SHORT_PREAMBLE(apNewCaps.capabilityInfo) !=
	       SIR_MAC_GET_SHORT_PREAMBLE(psessionEntry->limCurrentBssCaps)) ||
	      (SIR_MAC_GET_QOS(apNewCaps.capabilityInfo) !=
	       SIR_MAC_GET_QOS(psessionEntry->limCurrentBssCaps)) ||
	      ((newChannel != psessionEntry->currentOperChannel) &&
		(newChannel != 0)) ||
	      (false == security_caps_matched)
	     ))) {
		if (false == psessionEntry->fWaitForProbeRsp) {
			/* If Beacon capabilities is not matching with the current capability,
			 * then send unicast probe request to AP and take decision after
			 * receiving probe response */
			if (true == psessionEntry->fIgnoreCapsChange) {
				pe_warn("Ignoring the Capability change as it is false alarm");
				return;
			}
			psessionEntry->fWaitForProbeRsp = true;
			pe_warn("AP capabilities are not matching, sending directed probe request");
			status =
				lim_send_probe_req_mgmt_frame(pMac, &psessionEntry->ssId,
							      psessionEntry->bssId,
							      psessionEntry->
							      currentOperChannel,
							      psessionEntry->selfMacAddr,
							      psessionEntry->dot11mode,
							      0, NULL);

			if (eSIR_SUCCESS != status) {
				pe_err("send ProbeReq failed");
				psessionEntry->fWaitForProbeRsp = false;
			}
			return;
		}
		/**
		 * BSS capabilities have changed.
		 * Inform Roaming.
		 */
		len = sizeof(tSirMacCapabilityInfo) + sizeof(tSirMacAddr) + sizeof(uint8_t) + 3 * sizeof(uint8_t) + /* reserved fields */
		      pBeacon->ssId.length + 1;

		qdf_mem_copy(apNewCaps.bssId.bytes,
			     psessionEntry->bssId, QDF_MAC_ADDR_SIZE);
		if (newChannel != psessionEntry->currentOperChannel) {
			pe_err("Channel Change from %d --> %d Ignoring beacon!",
				psessionEntry->currentOperChannel, newChannel);
			return;
		}

		/**
		 * When Cisco 1262 Enterprise APs are configured with WPA2-PSK with
		 * AES+TKIP Pairwise ciphers and WEP-40 Group cipher, they do not set
		 * the privacy bit in Beacons (wpa/rsnie is still present in beacons),
		 * the privacy bit is set in Probe and association responses.
		 * Due to this anomaly, we detect a change in
		 * AP capabilities when we receive a beacon after association and
		 * disconnect from the AP. The following check makes sure that we can
		 * connect to such APs
		 */
		else if ((SIR_MAC_GET_PRIVACY(apNewCaps.capabilityInfo) == 0) &&
			 (pBeacon->rsnPresent || pBeacon->wpaPresent)) {
			pe_err("BSS Caps (Privacy) bit 0 in beacon, but WPA or RSN IE present, Ignore Beacon!");
			return;
		} else
			apNewCaps.channelId = psessionEntry->currentOperChannel;
		qdf_mem_copy((uint8_t *) &apNewCaps.ssId,
			     (uint8_t *) &pBeacon->ssId,
			     pBeacon->ssId.length + 1);

		psessionEntry->fIgnoreCapsChange = false;
		psessionEntry->fWaitForProbeRsp = false;
		psessionEntry->limSentCapsChangeNtf = true;
		lim_send_sme_wm_status_change_ntf(pMac, eSIR_SME_AP_CAPS_CHANGED,
						  (uint32_t *) &apNewCaps,
						  len, psessionEntry->smeSessionId);
	} else if (true == psessionEntry->fWaitForProbeRsp) {
		/* Only for probe response frames and matching capabilities the control
		 * will come here. If beacon is with broadcast ssid then fWaitForProbeRsp
		 * will be false, the control will not come here*/

		pe_debug("capabilities in probe response are"
				       "matching with the current setting,"
				       "Ignoring subsequent capability"
				       "mismatch");
		psessionEntry->fIgnoreCapsChange = true;
		psessionEntry->fWaitForProbeRsp = false;
	}

} /*** lim_detect_change_in_ap_capabilities() ***/

/* --------------------------------------------------------------------- */
/**
 * lim_update_short_slot
 *
 * FUNCTION:
 * Enable/Disable short slot
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param enable        Flag to enable/disable short slot
 * @return None
 */

tSirRetStatus lim_update_short_slot(tpAniSirGlobal pMac,
				    tpSirProbeRespBeacon pBeacon,
				    tpUpdateBeaconParams pBeaconParams,
				    tpPESession psessionEntry)
{

	tSirSmeApNewCaps apNewCaps;
	uint32_t nShortSlot;
	uint32_t val = 0;
	uint32_t phyMode;

	/* Check Admin mode first. If it is disabled just return */
	if (wlan_cfg_get_int(pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED, &val)
	    != eSIR_SUCCESS) {
		pe_err("cfg get WNI_CFG_11G_SHORT_SLOT_TIME failed");
		return eSIR_FAILURE;
	}
	if (val == false)
		return eSIR_SUCCESS;

	/* Check for 11a mode or 11b mode. In both cases return since slot time is constant and cannot/should not change in beacon */
	lim_get_phy_mode(pMac, &phyMode, psessionEntry);
	if ((phyMode == WNI_CFG_PHY_MODE_11A)
	    || (phyMode == WNI_CFG_PHY_MODE_11B))
		return eSIR_SUCCESS;

	apNewCaps.capabilityInfo =
		lim_get_u16((uint8_t *) &pBeacon->capabilityInfo);

	/*  Earlier implementation: determine the appropriate short slot mode based on AP advertised modes */
	/* when erp is present, apply short slot always unless, prot=on  && shortSlot=off */
	/* if no erp present, use short slot based on current ap caps */

	/* Issue with earlier implementation : Cisco 1231 BG has shortSlot = 0, erpIEPresent and useProtection = 0 (Case4); */

	/* Resolution : always use the shortSlot setting the capability info to decide slot time. */
	/* The difference between the earlier implementation and the new one is only Case4. */
	/*
	   ERP IE Present  |   useProtection   |   shortSlot   =   QC STA Short Slot
	   Case1        1                                   1                       1                       1           //AP should not advertise this combination.
	   Case2        1                                   1                       0                       0
	   Case3        1                                   0                       1                       1
	   Case4        1                                   0                       0                       0
	   Case5        0                                   1                       1                       1
	   Case6        0                                   1                       0                       0
	   Case7        0                                   0                       1                       1
	   Case8        0                                   0                       0                       0
	 */
	nShortSlot = SIR_MAC_GET_SHORT_SLOT_TIME(apNewCaps.capabilityInfo);

	if (nShortSlot != psessionEntry->shortSlotTimeSupported) {
		/* Short slot time capability of AP has changed. Adopt to it. */
		pe_debug("Shortslot capability of AP changed: %d",
			       nShortSlot);
			((tpSirMacCapabilityInfo) & psessionEntry->
			limCurrentBssCaps)->shortSlotTime = (uint16_t) nShortSlot;
		psessionEntry->shortSlotTimeSupported = nShortSlot;
		pBeaconParams->fShortSlotTime = (uint8_t) nShortSlot;
		pBeaconParams->paramChangeBitmap |=
			PARAM_SHORT_SLOT_TIME_CHANGED;
	}
	return eSIR_SUCCESS;
}


void lim_send_heart_beat_timeout_ind(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t statusCode;
	struct scheduler_msg msg = {0};

	/* Prepare and post message to LIM Message Queue */
	msg.type = (uint16_t) SIR_LIM_HEART_BEAT_TIMEOUT;
	msg.bodyptr = psessionEntry;
	msg.bodyval = 0;
	pe_err("Heartbeat failure from Fw");

	statusCode = lim_post_msg_api(pMac, &msg);

	if (statusCode != eSIR_SUCCESS) {
		pe_err("posting message: %X to LIM failed, reason: %d",
			msg.type, statusCode);
	}
}

/**
 * lim_ps_offload_handle_missed_beacon_ind(): handles missed beacon indication
 * @pMac : global mac context
 * @pMsg: message
 *
 * This function process the SIR_HAL_MISSED_BEACON_IND
 * message from HAL, to do active AP probing.
 *
 * Return: void
 */
void lim_ps_offload_handle_missed_beacon_ind(tpAniSirGlobal pMac,
					     struct scheduler_msg *pMsg)
{
	tpSirSmeMissedBeaconInd pSirMissedBeaconInd =
		(tpSirSmeMissedBeaconInd) pMsg->bodyptr;
	tpPESession psessionEntry =
		pe_find_session_by_bss_idx(pMac, pSirMissedBeaconInd->bssIdx);

	if (!psessionEntry) {
		pe_err("session does not exist for given BSSId");
		return;
	}

	/* Set Beacon Miss in Powersave Offload */
	psessionEntry->pmmOffloadInfo.bcnmiss = true;
	pe_err("Received Heart Beat Failure");

	/*  Do AP probing immediately */
	lim_send_heart_beat_timeout_ind(pMac, psessionEntry);
	return;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * lim_fill_join_rsp_ht_caps() - Fill the HT caps in join response
 * @session: PE Session
 * @join_rsp: Join response buffer to be filled up.
 *
 * Return: None
 */
void lim_fill_join_rsp_ht_caps(tpPESession session, tpSirSmeJoinRsp join_rsp)
{
	tSirSmeHTProfile *ht_profile;

	if (session == NULL) {
		pe_err("Invalid Session");
		return;
	}
	if (join_rsp == NULL) {
		pe_err("Invalid Join Response");
		return;
	}

	if (session->cc_switch_mode == QDF_MCC_TO_SCC_SWITCH_DISABLE)
		return;

	ht_profile = &join_rsp->HTProfile;
	ht_profile->htSupportedChannelWidthSet =
		session->htSupportedChannelWidthSet;
	ht_profile->htRecommendedTxWidthSet =
		session->htRecommendedTxWidthSet;
	ht_profile->htSecondaryChannelOffset =
		session->htSecondaryChannelOffset;
	ht_profile->dot11mode = session->dot11mode;
	ht_profile->htCapability = session->htCapability;
	ht_profile->vhtCapability = session->vhtCapability;
	ht_profile->apCenterChan = session->ch_center_freq_seg0;
	ht_profile->apChanWidth = session->ch_width;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sir_parse_bcn_fixed_fields() - Parse fixed fields in Beacon IE's
 *
 * @mac_ctx: MAC Context
 * @beacon_struct: Beacon/Probe Response structure
 * @buf: Fixed Fields buffer
 */
static void sir_parse_bcn_fixed_fields(tpAniSirGlobal mac_ctx,
					tpSirProbeRespBeacon beacon_struct,
					uint8_t *buf)
{
	tDot11fFfCapabilities dst;

	beacon_struct->timeStamp[0] = lim_get_u32(buf);
	beacon_struct->timeStamp[1] = lim_get_u32(buf + 4);
	buf += 8;

	beacon_struct->beaconInterval = lim_get_u16(buf);
	buf += 2;

	dot11f_unpack_ff_capabilities(mac_ctx, buf, &dst);

	sir_copy_caps_info(mac_ctx, dst, beacon_struct);
}

static QDF_STATUS
lim_roam_fill_bss_descr(tpAniSirGlobal pMac,
			roam_offload_synch_ind *roam_offload_synch_ind_ptr,
			tpSirBssDescription  bss_desc_ptr)
{
	uint32_t ie_len = 0;
	tpSirProbeRespBeacon parsed_frm_ptr;
	tpSirMacMgmtHdr mac_hdr;
	uint8_t *bcn_proberesp_ptr;

	bcn_proberesp_ptr = (uint8_t *)roam_offload_synch_ind_ptr +
		roam_offload_synch_ind_ptr->beaconProbeRespOffset;
	mac_hdr = (tpSirMacMgmtHdr)bcn_proberesp_ptr;
	parsed_frm_ptr =
	(tpSirProbeRespBeacon) qdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == parsed_frm_ptr) {
		pe_err("fail to allocate memory for frame");
		return QDF_STATUS_E_NOMEM;
	}

	if (roam_offload_synch_ind_ptr->beaconProbeRespLength <=
			SIR_MAC_HDR_LEN_3A) {
		pe_err("%s: very few bytes in synchInd beacon / probe resp frame! length: %d",
		__func__, roam_offload_synch_ind_ptr->beaconProbeRespLength);
		qdf_mem_free(parsed_frm_ptr);
		return QDF_STATUS_E_FAILURE;
	}

	pe_debug("LFR3:Beacon/Prb Rsp: %d", roam_offload_synch_ind_ptr->isBeacon);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
	bcn_proberesp_ptr, roam_offload_synch_ind_ptr->beaconProbeRespLength);
	if (roam_offload_synch_ind_ptr->isBeacon) {
		if (sir_parse_beacon_ie(pMac, parsed_frm_ptr,
			&bcn_proberesp_ptr[SIR_MAC_HDR_LEN_3A +
			SIR_MAC_B_PR_SSID_OFFSET],
			roam_offload_synch_ind_ptr->beaconProbeRespLength -
			SIR_MAC_HDR_LEN_3A) != eSIR_SUCCESS ||
			!parsed_frm_ptr->ssidPresent) {
			pe_err("Parse error Beacon, length: %d",
			roam_offload_synch_ind_ptr->beaconProbeRespLength);
			qdf_mem_free(parsed_frm_ptr);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (sir_convert_probe_frame2_struct(pMac,
			&bcn_proberesp_ptr[SIR_MAC_HDR_LEN_3A],
			roam_offload_synch_ind_ptr->beaconProbeRespLength -
			SIR_MAC_HDR_LEN_3A, parsed_frm_ptr) != eSIR_SUCCESS ||
			!parsed_frm_ptr->ssidPresent) {
			pe_err("Parse error ProbeResponse, length: %d",
			roam_offload_synch_ind_ptr->beaconProbeRespLength);
			qdf_mem_free(parsed_frm_ptr);
			return QDF_STATUS_E_FAILURE;
		}
	}

	/*
	 * For probe response, unpack core parses beacon interval, capabilities,
	 * timestamp. For beacon IEs, these fields are not parsed.
	 */
	if (roam_offload_synch_ind_ptr->isBeacon)
		sir_parse_bcn_fixed_fields(pMac, parsed_frm_ptr,
			&bcn_proberesp_ptr[SIR_MAC_HDR_LEN_3A]);

	/* 24 byte MAC header and 12 byte to ssid IE */
	if (roam_offload_synch_ind_ptr->beaconProbeRespLength >
		(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET)) {
		ie_len = roam_offload_synch_ind_ptr->beaconProbeRespLength -
			(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET);
	}
	/*
	 * Length of BSS desription is without length of
	 * length itself and length of pointer
	 * that holds ieFields
	 *
	 * tSirBssDescription
	 * +--------+---------------------------------+---------------+
	 * | length | other fields                    | pointer to IEs|
	 * +--------+---------------------------------+---------------+
	 *                                            ^
	 *                                            ieFields
	 */
	bss_desc_ptr->length = (uint16_t) (offsetof(tSirBssDescription,
					   ieFields[0]) -
				sizeof(bss_desc_ptr->length) + ie_len);

	bss_desc_ptr->fProbeRsp = !roam_offload_synch_ind_ptr->isBeacon;
	/* Copy Timestamp */
	bss_desc_ptr->scansystimensec = qdf_get_monotonic_boottime_ns();
	if (parsed_frm_ptr->dsParamsPresent) {
		bss_desc_ptr->channelId = parsed_frm_ptr->channelNumber;
	} else if (parsed_frm_ptr->HTInfo.present) {
		bss_desc_ptr->channelId = parsed_frm_ptr->HTInfo.primaryChannel;
	} else {
		/*
		 * If DS Params or HTIE is not present in the probe resp or
		 * beacon, then use the channel frequency provided by firmware
		 * to fill the channel in the BSS descriptor.*/
		bss_desc_ptr->channelId =
			cds_freq_to_chan(roam_offload_synch_ind_ptr->chan_freq);
	}
	bss_desc_ptr->channelIdSelf = bss_desc_ptr->channelId;

	bss_desc_ptr->nwType = lim_get_nw_type(pMac, bss_desc_ptr->channelId,
					       SIR_MAC_MGMT_FRAME,
					       parsed_frm_ptr);

	bss_desc_ptr->sinr = 0;
	bss_desc_ptr->beaconInterval = parsed_frm_ptr->beaconInterval;
	bss_desc_ptr->timeStamp[0]   = parsed_frm_ptr->timeStamp[0];
	bss_desc_ptr->timeStamp[1]   = parsed_frm_ptr->timeStamp[1];
	qdf_mem_copy(&bss_desc_ptr->capabilityInfo,
	&bcn_proberesp_ptr[SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_CAPAB_OFFSET], 2);

	if (qdf_is_macaddr_zero((struct qdf_mac_addr *)mac_hdr->bssId)) {
		pe_debug("bssid is 0 in beacon/probe update it with bssId %pM in sync ind",
			roam_offload_synch_ind_ptr->bssid.bytes);
		qdf_mem_copy(mac_hdr->bssId,
			roam_offload_synch_ind_ptr->bssid.bytes,
			sizeof(tSirMacAddr));
	}

	qdf_mem_copy((uint8_t *) &bss_desc_ptr->bssId,
			(uint8_t *) mac_hdr->bssId,
			sizeof(tSirMacAddr));
	bss_desc_ptr->received_time =
		      (uint64_t)qdf_mc_timer_get_system_time();
	if (parsed_frm_ptr->mdiePresent) {
		bss_desc_ptr->mdiePresent = parsed_frm_ptr->mdiePresent;
		qdf_mem_copy((uint8_t *)bss_desc_ptr->mdie,
				(uint8_t *)parsed_frm_ptr->mdie,
				SIR_MDIE_SIZE);
	}
	pe_debug("LFR3: BssDescr Info:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
			bss_desc_ptr->bssId, sizeof(tSirMacAddr));
	pe_debug("chan: %d rssi: %d", bss_desc_ptr->channelId,
			bss_desc_ptr->rssi);
	if (ie_len) {
		qdf_mem_copy(&bss_desc_ptr->ieFields,
			bcn_proberesp_ptr +
			(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET),
			ie_len);
	}
	qdf_mem_free(parsed_frm_ptr);
	return QDF_STATUS_SUCCESS;
}

#if defined(WLAN_FEATURE_FILS_SK)
/**
 * lim_copy_and_free_hlp_data_from_session - Copy HLP info
 * @session_ptr: PE session
 * @roam_sync_ind_ptr: Roam Synch Indication pointer
 *
 * This API is used to copy the parsed HLP info from PE session
 * to roam synch indication data. THe HLP info is expected to be
 * parsed/stored in PE session already from assoc IE's received
 * from fw as part of Roam Synch Indication.
 *
 * Return: None
 */
static void lim_copy_and_free_hlp_data_from_session(tpPESession session_ptr,
				    roam_offload_synch_ind *roam_sync_ind_ptr)
{
	if (session_ptr->hlp_data && session_ptr->hlp_data_len) {
		cds_copy_hlp_info(&session_ptr->dst_mac,
				&session_ptr->src_mac,
				session_ptr->hlp_data_len,
				session_ptr->hlp_data,
				&roam_sync_ind_ptr->dst_mac,
				&roam_sync_ind_ptr->src_mac,
				&roam_sync_ind_ptr->hlp_data_len,
				roam_sync_ind_ptr->hlp_data);
		qdf_mem_free(session_ptr->hlp_data);
		session_ptr->hlp_data = NULL;
		session_ptr->hlp_data_len = 0;
	}
}
#else
static inline void lim_copy_and_free_hlp_data_from_session(
					tpPESession session_ptr,
					roam_offload_synch_ind
					*roam_sync_ind_ptr)
{}
#endif

/**
 * pe_roam_synch_callback() - PE level callback for roam synch propagation
 * @mac_ctx: MAC Context
 * @roam_sync_ind_ptr: Roam synch indication buffer pointer
 * @bss_desc: BSS Descriptor pointer
 * @reason: Reason for calling callback which decides the action to be taken.
 *
 * This is a PE level callback called from WMA to complete the roam synch
 * propagation at PE level and also fill the BSS descriptor which will be
 * helpful further to complete the roam synch propagation.
 *
 * Return: Success or Failure status
 */
QDF_STATUS pe_roam_synch_callback(tpAniSirGlobal mac_ctx,
	roam_offload_synch_ind *roam_sync_ind_ptr,
	tpSirBssDescription  bss_desc, enum sir_roam_op_code reason)
{
	tpPESession session_ptr;
	tpPESession ft_session_ptr;
	uint8_t session_id;
	tpDphHashNode curr_sta_ds;
	uint16_t aid;
	tpAddBssParams add_bss_params;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint16_t join_rsp_len;

	if (!roam_sync_ind_ptr) {
		pe_err("LFR3:roam_sync_ind_ptr is NULL");
		return status;
	}
	session_ptr = pe_find_session_by_sme_session_id(mac_ctx,
				roam_sync_ind_ptr->roamedVdevId);
	if (session_ptr == NULL) {
		pe_err("LFR3:Unable to find session");
		return status;
	}

	if (!LIM_IS_STA_ROLE(session_ptr)) {
		pe_err("LFR3:session is not in STA mode");
		return status;
	}

	pe_debug("LFR3: PE callback reason: %d", reason);
	switch (reason) {
	case SIR_ROAMING_START:
		session_ptr->fw_roaming_started = true;
		return QDF_STATUS_SUCCESS;
	case SIR_ROAMING_ABORT:
		session_ptr->fw_roaming_started = false;
		/*
		 * If there was a disassoc or deauth that was received
		 * during roaming and it was not honored, then we have
		 * to internally initiate a disconnect because with
		 * ROAM_ABORT we come back to original AP.
		 */
		if (session_ptr->recvd_deauth_while_roaming)
			lim_perform_deauth(mac_ctx, session_ptr,
					   session_ptr->deauth_disassoc_rc,
					   session_ptr->bssId, 0);
		if (session_ptr->recvd_disassoc_while_roaming) {
			lim_disassoc_tdls_peers(mac_ctx, session_ptr,
						session_ptr->bssId);
			lim_perform_disassoc(mac_ctx, 0,
					     session_ptr->deauth_disassoc_rc,
					     session_ptr, session_ptr->bssId);
		}
		return QDF_STATUS_SUCCESS;
	case SIR_ROAM_SYNCH_PROPAGATION:
		session_ptr->fw_roaming_started = false;
		break;
	default:
		return status;
	}

	pe_debug("LFR3:Received WMA_ROAM_OFFLOAD_SYNCH_IND LFR3:auth: %d vdevId: %d",
		roam_sync_ind_ptr->authStatus, roam_sync_ind_ptr->roamedVdevId);
	lim_print_mac_addr(mac_ctx, roam_sync_ind_ptr->bssid.bytes,
			QDF_TRACE_LEVEL_DEBUG);
	/*
	 * If deauth from AP already in progress, ignore Roam Synch Indication
	 * from firmware.
	 */
	if (session_ptr->limSmeState != eLIM_SME_LINK_EST_STATE) {
		pe_err("LFR3: Not in Link est state");
		return status;
	}
	status = lim_roam_fill_bss_descr(mac_ctx, roam_sync_ind_ptr, bss_desc);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		pe_err("LFR3:Failed to fill Bss Descr");
		return status;
	}
	status = QDF_STATUS_E_FAILURE;
	ft_session_ptr = pe_create_session(mac_ctx, bss_desc->bssId,
			&session_id, mac_ctx->lim.maxStation,
			eSIR_INFRASTRUCTURE_MODE);
	if (ft_session_ptr == NULL) {
		pe_err("LFR3:Cannot create PE Session");
		lim_print_mac_addr(mac_ctx, bss_desc->bssId, LOGE);
		return status;
	}
	ft_session_ptr->peSessionId = session_id;
	sir_copy_mac_addr(ft_session_ptr->selfMacAddr, session_ptr->selfMacAddr);
	sir_copy_mac_addr(roam_sync_ind_ptr->self_mac.bytes,
			session_ptr->selfMacAddr);
	sir_copy_mac_addr(ft_session_ptr->limReAssocbssId, bss_desc->bssId);
	ft_session_ptr->bssType = eSIR_INFRASTRUCTURE_MODE;
	session_ptr->bRoamSynchInProgress = true;
	ft_session_ptr->bRoamSynchInProgress = true;
	ft_session_ptr->limSystemRole = eLIM_STA_ROLE;
	sir_copy_mac_addr(session_ptr->limReAssocbssId, bss_desc->bssId);
	ft_session_ptr->csaOffloadEnable = session_ptr->csaOffloadEnable;

	/* Assign default configured nss value in the new session */
	if (IS_5G_CH(ft_session_ptr->currentOperChannel))
		ft_session_ptr->vdev_nss = mac_ctx->vdev_type_nss_5g.sta;
	else
		ft_session_ptr->vdev_nss = mac_ctx->vdev_type_nss_2g.sta;

	ft_session_ptr->nss = ft_session_ptr->vdev_nss;

	/* Next routine will update nss and vdev_nss with AP's capabilities */
	lim_fill_ft_session(mac_ctx, bss_desc, ft_session_ptr, session_ptr);

	/* Next routine may update nss based on dot11Mode */
	lim_ft_prepare_add_bss_req(mac_ctx, false, ft_session_ptr, bss_desc);
	roam_sync_ind_ptr->add_bss_params =
		(tpAddBssParams) ft_session_ptr->ftPEContext.pAddBssReq;
	add_bss_params = ft_session_ptr->ftPEContext.pAddBssReq;
	lim_delete_tdls_peers(mac_ctx, session_ptr);
	curr_sta_ds = dph_lookup_hash_entry(mac_ctx, session_ptr->bssId,
			&aid, &session_ptr->dph.dphHashTable);
	if (curr_sta_ds == NULL) {
		pe_err("LFR3:failed to lookup hash entry");
		ft_session_ptr->bRoamSynchInProgress = false;
		return status;
	}
	session_ptr->limSmeState = eLIM_SME_IDLE_STATE;
	lim_cleanup_rx_path(mac_ctx, curr_sta_ds, session_ptr);
	lim_delete_dph_hash_entry(mac_ctx, curr_sta_ds->staAddr,
			aid, session_ptr);
	pe_delete_session(mac_ctx, session_ptr);
	session_ptr = NULL;
	curr_sta_ds = dph_add_hash_entry(mac_ctx,
			roam_sync_ind_ptr->bssid.bytes, DPH_STA_HASH_INDEX_PEER,
			&ft_session_ptr->dph.dphHashTable);
	if (curr_sta_ds == NULL) {
		pe_err("LFR3:failed to add hash entry for");
		lim_print_mac_addr(mac_ctx,
				add_bss_params->staContext.staMac, LOGE);
		ft_session_ptr->bRoamSynchInProgress = false;
		return status;
	}

	add_bss_params->bssIdx = roam_sync_ind_ptr->roamedVdevId;
	ft_session_ptr->bssIdx = (uint8_t) add_bss_params->bssIdx;

	curr_sta_ds->bssId = add_bss_params->bssIdx;
	curr_sta_ds->staIndex =
		add_bss_params->staContext.staIdx;
	curr_sta_ds->ucUcastSig =
		add_bss_params->staContext.ucUcastSig;
	curr_sta_ds->ucBcastSig =
		add_bss_params->staContext.ucBcastSig;
	rrm_cache_mgmt_tx_power(mac_ctx,
		add_bss_params->txMgmtPower, ft_session_ptr);
	mac_ctx->roam.reassocRespLen = roam_sync_ind_ptr->reassocRespLength;
	mac_ctx->roam.pReassocResp =
		qdf_mem_malloc(mac_ctx->roam.reassocRespLen);
	if (NULL == mac_ctx->roam.pReassocResp) {
		pe_err("LFR3:assoc resp mem alloc failed");
		ft_session_ptr->bRoamSynchInProgress = false;
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(mac_ctx->roam.pReassocResp,
			(uint8_t *)roam_sync_ind_ptr +
			roam_sync_ind_ptr->reassocRespOffset,
			mac_ctx->roam.reassocRespLen);

	pe_debug("LFR3:the reassoc resp frame data:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			mac_ctx->roam.pReassocResp,
			mac_ctx->roam.reassocRespLen);
	ft_session_ptr->bRoamSynchInProgress = true;

	lim_process_assoc_rsp_frame(mac_ctx, mac_ctx->roam.pReassocResp,
			LIM_REASSOC, ft_session_ptr);

	lim_copy_and_free_hlp_data_from_session(ft_session_ptr,
						roam_sync_ind_ptr);

	roam_sync_ind_ptr->aid = ft_session_ptr->limAID;
	curr_sta_ds->mlmStaContext.mlmState =
		eLIM_MLM_LINK_ESTABLISHED_STATE;
	curr_sta_ds->nss = ft_session_ptr->nss;
	roam_sync_ind_ptr->nss = ft_session_ptr->nss;
	ft_session_ptr->limMlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
	lim_init_tdls_data(mac_ctx, ft_session_ptr);
	join_rsp_len = ft_session_ptr->RICDataLen +
			sizeof(tSirSmeJoinRsp) - sizeof(uint8_t);

#ifdef FEATURE_WLAN_ESE
	join_rsp_len += ft_session_ptr->tspecLen;
	pe_debug("tspecLen: %d", ft_session_ptr->tspecLen);
#endif

	roam_sync_ind_ptr->join_rsp = qdf_mem_malloc(join_rsp_len);
	if (NULL == roam_sync_ind_ptr->join_rsp) {
		pe_err("LFR3:mem alloc failed");
		ft_session_ptr->bRoamSynchInProgress = false;
		if (mac_ctx->roam.pReassocResp)
			qdf_mem_free(mac_ctx->roam.pReassocResp);
		mac_ctx->roam.pReassocResp = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	pe_debug("Session RicLength: %d", ft_session_ptr->RICDataLen);
	if (ft_session_ptr->ricData != NULL) {
		roam_sync_ind_ptr->join_rsp->parsedRicRspLen =
			ft_session_ptr->RICDataLen;
		qdf_mem_copy(roam_sync_ind_ptr->join_rsp->frames,
				ft_session_ptr->ricData,
				roam_sync_ind_ptr->join_rsp->parsedRicRspLen);
		qdf_mem_free(ft_session_ptr->ricData);
		ft_session_ptr->ricData = NULL;
		ft_session_ptr->RICDataLen = 0;
	}

#ifdef FEATURE_WLAN_ESE
	if (ft_session_ptr->tspecIes != NULL) {
		roam_sync_ind_ptr->join_rsp->tspecIeLen =
			ft_session_ptr->tspecLen;
		qdf_mem_copy(roam_sync_ind_ptr->join_rsp->frames +
				roam_sync_ind_ptr->join_rsp->parsedRicRspLen,
				ft_session_ptr->tspecIes,
				roam_sync_ind_ptr->join_rsp->tspecIeLen);
		qdf_mem_free(ft_session_ptr->tspecIes);
		ft_session_ptr->tspecIes = NULL;
		ft_session_ptr->tspecLen = 0;
	}
#endif

	roam_sync_ind_ptr->join_rsp->vht_channel_width =
		ft_session_ptr->ch_width;
	roam_sync_ind_ptr->join_rsp->staId = curr_sta_ds->staIndex;
	roam_sync_ind_ptr->join_rsp->ucastSig = curr_sta_ds->ucUcastSig;
	roam_sync_ind_ptr->join_rsp->bcastSig = curr_sta_ds->ucBcastSig;
	roam_sync_ind_ptr->join_rsp->timingMeasCap = curr_sta_ds->timingMeasCap;
	roam_sync_ind_ptr->join_rsp->nss = curr_sta_ds->nss;
	roam_sync_ind_ptr->join_rsp->max_rate_flags =
		lim_get_max_rate_flags(mac_ctx, curr_sta_ds);
	lim_set_tdls_flags(roam_sync_ind_ptr, ft_session_ptr);
	roam_sync_ind_ptr->join_rsp->aid = ft_session_ptr->limAID;
	lim_fill_join_rsp_ht_caps(ft_session_ptr, roam_sync_ind_ptr->join_rsp);
	ft_session_ptr->limPrevSmeState = ft_session_ptr->limSmeState;
	ft_session_ptr->limSmeState = eLIM_SME_LINK_EST_STATE;
	ft_session_ptr->bRoamSynchInProgress = false;
	if (mac_ctx->roam.pReassocResp)
		qdf_mem_free(mac_ctx->roam.pReassocResp);
	mac_ctx->roam.pReassocResp = NULL;

	if (roam_sync_ind_ptr->authStatus == CSR_ROAM_AUTH_STATUS_AUTHENTICATED)
		ft_session_ptr->is_key_installed = true;

	return QDF_STATUS_SUCCESS;
}
#endif

static bool lim_is_beacon_miss_scenario(tpAniSirGlobal pMac,
					uint8_t *pRxPacketInfo)
{
	tpSirMacMgmtHdr pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	uint8_t sessionId;
	tpPESession psessionEntry =
		pe_find_session_by_bssid(pMac, pHdr->bssId, &sessionId);

	if (psessionEntry && psessionEntry->pmmOffloadInfo.bcnmiss)
		return true;
	return false;
}

/** -----------------------------------------------------------------
   \brief lim_is_pkt_candidate_for_drop() - decides whether to drop the frame or not

   This function is called before enqueuing the frame to PE queue for further processing.
   This prevents unnecessary frames getting into PE Queue and drops them right away.
   Frames will be droped in the following scenarios:

   - In Scan State, drop the frames which are not marked as scan frames
   - In non-Scan state, drop the frames which are marked as scan frames.
   - Drop INFRA Beacons and Probe Responses in IBSS Mode
   - Drop the Probe Request in IBSS mode, if STA did not send out the last beacon

   \param pMac - global mac structure
   \return - none
   \sa
   ----------------------------------------------------------------- */

tMgmtFrmDropReason lim_is_pkt_candidate_for_drop(tpAniSirGlobal pMac,
						 uint8_t *pRxPacketInfo,
						 uint32_t subType)
{
	uint32_t framelen;
	uint8_t *pBody;
	tSirMacCapabilityInfo capabilityInfo;
	tpSirMacMgmtHdr pHdr = NULL;
	tpPESession psessionEntry = NULL;
	uint8_t sessionId;

	/*
	 *
	 * In scan mode, drop only Beacon/Probe Response which are NOT marked as scan-frames.
	 * In non-scan mode, drop only Beacon/Probe Response which are marked as scan frames.
	 * Allow other mgmt frames, they must be from our own AP, as we don't allow
	 * other than beacons or probe responses in scan state.
	 */
	if ((subType == SIR_MAC_MGMT_BEACON) ||
	    (subType == SIR_MAC_MGMT_PROBE_RSP)) {
		if (lim_is_beacon_miss_scenario(pMac, pRxPacketInfo)) {
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_INFO_LOG, 0,
				       eLOG_NODROP_MISSED_BEACON_SCENARIO));
			return eMGMT_DROP_NO_DROP;
		}
		if (lim_is_system_in_scan_state(pMac)) {
			return eMGMT_DROP_NO_DROP;
		} else if (WMA_IS_RX_IN_SCAN(pRxPacketInfo)) {
			return eMGMT_DROP_SCAN_MODE_FRAME;
		} else {
			/* Beacons and probe responses can come in any time
			 * because of parallel scans. Don't drop them.
			 */
			return eMGMT_DROP_NO_DROP;
		}
	}

	framelen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);

	/* Drop INFRA Beacons and Probe Responses in IBSS Mode */
	if ((subType == SIR_MAC_MGMT_BEACON) ||
	    (subType == SIR_MAC_MGMT_PROBE_RSP)) {
		/* drop the frame if length is less than 12 */
		if (framelen < LIM_MIN_BCN_PR_LENGTH)
			return eMGMT_DROP_INVALID_SIZE;

		*((uint16_t *) &capabilityInfo) =
			sir_read_u16(pBody + LIM_BCN_PR_CAPABILITY_OFFSET);

		/* Note sure if this is sufficient, basically this condition allows all probe responses and
		 *   beacons from an infrastructure network
		 */
		if (!capabilityInfo.ibss)
			return eMGMT_DROP_NO_DROP;

		/* This can be enhanced to even check the SSID before deciding to enque the frame. */
		if (capabilityInfo.ess)
			return eMGMT_DROP_INFRA_BCN_IN_IBSS;
	} else if ((subType == SIR_MAC_MGMT_PROBE_REQ) &&
		   (!WMA_GET_RX_BEACON_SENT(pRxPacketInfo))) {
		pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
		psessionEntry =
			pe_find_session_by_bssid(pMac, pHdr->bssId, &sessionId);
		if ((psessionEntry && !LIM_IS_IBSS_ROLE(psessionEntry)) ||
		    (!psessionEntry))
			return eMGMT_DROP_NO_DROP;

		/* Drop the Probe Request in IBSS mode, if STA did not send out the last beacon */
		/* In IBSS, the node which sends out the beacon, is supposed to respond to ProbeReq */
		return eMGMT_DROP_NOT_LAST_IBSS_BCN;
	}

	return eMGMT_DROP_NO_DROP;
}

void lim_update_lost_link_info(tpAniSirGlobal mac, tpPESession session,
				int32_t rssi)
{
	struct sir_lost_link_info *lost_link_info;
	struct scheduler_msg mmh_msg = {0};

	if ((NULL == mac) || (NULL == session)) {
		pe_err("parameter NULL");
		return;
	}
	if (!LIM_IS_STA_ROLE(session)) {
		pe_err("not STA mode, do nothing");
		return;
	}

	lost_link_info = qdf_mem_malloc(sizeof(*lost_link_info));
	if (NULL == lost_link_info) {
		pe_err("lost_link_info allocation failure");
		return;
	}

	lost_link_info->vdev_id = session->smeSessionId;
	lost_link_info->rssi = rssi;
	mmh_msg.type = eWNI_SME_LOST_LINK_INFO_IND;
	mmh_msg.bodyptr = lost_link_info;
	mmh_msg.bodyval = 0;
	pe_debug("post eWNI_SME_LOST_LINK_INFO_IND, bss_idx: %d rssi: %d",
		lost_link_info->vdev_id, lost_link_info->rssi);

	lim_sys_process_mmh_msg_api(mac, &mmh_msg, ePROT);
}

QDF_STATUS pe_acquire_global_lock(tAniSirLim *psPe)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psPe) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_acquire(&psPe->lkPeGlobalLock))) {
			status = QDF_STATUS_SUCCESS;
		}
	}
	return status;
}

QDF_STATUS pe_release_global_lock(tAniSirLim *psPe)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psPe) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_release(&psPe->lkPeGlobalLock))) {
			status = QDF_STATUS_SUCCESS;
		}
	}
	return status;
}

/**
 * lim_mon_init_session() - create PE session for monitor mode operation
 * @mac_ptr: mac pointer
 * @msg: Pointer to struct sir_create_session type.
 *
 * Return: NONE
 */
void lim_mon_init_session(tpAniSirGlobal mac_ptr,
			  struct sir_create_session *msg)
{
	tpPESession psession_entry;
	uint8_t session_id;

	psession_entry = pe_create_session(mac_ptr, msg->bss_id.bytes,
					   &session_id,
					   mac_ptr->lim.maxStation,
					   eSIR_MONITOR_MODE);
	if (psession_entry == NULL) {
		pe_err("Monitor mode: Session Can not be created");
		lim_print_mac_addr(mac_ptr, msg->bss_id.bytes, LOGE);
		return;
	}
	psession_entry->vhtCapability = 1;
}

/**
 * lim_update_ext_cap_ie() - Update Extended capabilities IE(if present)
 *          with capabilities of Fine Time measurements(FTM) if set in driver
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @ie_data: Default Scan IE data
 * @local_ie_buf: Local Scan IE data
 * @local_ie_len: Pointer to length of @ie_data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS lim_update_ext_cap_ie(tpAniSirGlobal mac_ctx,
		uint8_t *ie_data, uint8_t *local_ie_buf, uint16_t *local_ie_len)
{
	uint32_t dot11mode;
	bool vht_enabled = false;
	tDot11fIEExtCap default_scan_ext_cap = {0}, driver_ext_cap = {0};
	tSirRetStatus status;

	status = lim_strip_extcap_update_struct(mac_ctx, ie_data,
				   local_ie_len, &default_scan_ext_cap);
	if (eSIR_SUCCESS != status) {
		pe_err("Strip ext cap fails %d", status);
		return QDF_STATUS_E_FAILURE;
	}

	/* copy ie prior to ext cap to local buffer */
	qdf_mem_copy(local_ie_buf, ie_data, (*local_ie_len));

	/* from here ext cap ie starts, set EID */
	local_ie_buf[*local_ie_len] = DOT11F_EID_EXTCAP;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_DOT11_MODE, &dot11mode);
	if (IS_DOT11_MODE_VHT(dot11mode))
		vht_enabled = true;

	status = populate_dot11f_ext_cap(mac_ctx, vht_enabled,
					&driver_ext_cap, NULL);
	if (eSIR_SUCCESS != status) {
		pe_err("Failed %d to create ext cap IE. Use default value instead",
				status);
		local_ie_buf[*local_ie_len + 1] = DOT11F_IE_EXTCAP_MAX_LEN;
		(*local_ie_len) += EXT_CAP_IE_HDR_LEN;
		qdf_mem_copy(local_ie_buf + (*local_ie_len),
				default_scan_ext_cap.bytes,
				DOT11F_IE_EXTCAP_MAX_LEN);
		(*local_ie_len) += DOT11F_IE_EXTCAP_MAX_LEN;
		return QDF_STATUS_SUCCESS;
	}
	lim_merge_extcap_struct(&driver_ext_cap, &default_scan_ext_cap, true);
	local_ie_buf[*local_ie_len + 1] = driver_ext_cap.num_bytes;
	(*local_ie_len) += EXT_CAP_IE_HDR_LEN;
	qdf_mem_copy(local_ie_buf + (*local_ie_len),
			driver_ext_cap.bytes, driver_ext_cap.num_bytes);
	(*local_ie_len) += driver_ext_cap.num_bytes;
	return QDF_STATUS_SUCCESS;
}
