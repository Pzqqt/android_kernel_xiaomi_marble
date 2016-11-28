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

#include "cds_api.h"
#include "sir_common.h"

#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"
#include "lim_api.h"
#include "lim_send_messages.h"

#include "sch_api.h"
#include "sch_debug.h"

/* / Minimum beacon interval allowed (in Kus) */
#define SCH_BEACON_INTERVAL_MIN  10

/* / Maximum beacon interval allowed (in Kus) */
#define SCH_BEACON_INTERVAL_MAX  10000

/* / convert the CW values into a uint16_t */
#define GET_CW(pCw) ((uint16_t) ((*(pCw) << 8) + *((pCw) + 1)))

/* local functions */
static tSirRetStatus get_wmm_local_params(tpAniSirGlobal pMac,
					  uint32_t
					  params[]
					  [WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN]);
static void set_sch_edca_params(tpAniSirGlobal pMac,
				uint32_t params[][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN],
				tpPESession psessionEntry);

/* -------------------------------------------------------------------- */
/**
 * sch_set_beacon_interval
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

void sch_set_beacon_interval(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t bi;

	bi = psessionEntry->beaconParams.beaconInterval;

	if (bi < SCH_BEACON_INTERVAL_MIN || bi > SCH_BEACON_INTERVAL_MAX) {
		sch_log(pMac, LOGE,
			FL("Invalid beacon interval %d (should be [%d,%d]"), bi,
			SCH_BEACON_INTERVAL_MIN, SCH_BEACON_INTERVAL_MAX);
		return;
	}

	pMac->sch.schObject.gSchBeaconInterval = (uint16_t) bi;
}

/* -------------------------------------------------------------------- */
/**
 * sch_process_message
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

void sch_process_message(tpAniSirGlobal pMac, tpSirMsgQ pSchMsg)
{
	uint32_t val;

	tpPESession psessionEntry = &pMac->lim.gpSession[0];
	PELOG3(sch_log(pMac, LOG3, FL("Received message (%x) "), pSchMsg->type);)

	switch (pSchMsg->type) {

	case SIR_SCH_CHANNEL_SWITCH_REQUEST:
		sch_log(pMac, LOGE, FL("Channel switch request not handled"));
		break;

	case SIR_SCH_START_SCAN_REQ:
		pMac->sch.gSchScanReqRcvd = true;
		if (pMac->sch.gSchHcfEnabled) {
			/* In HCF mode, wait for TFP to stop before sending a response */
			if (pMac->sch.schObject.gSchCFBInitiated ||
			    pMac->sch.schObject.gSchCFPInitiated) {
				PELOG1(sch_log(pMac, LOG1,
					       FL
						       ("Waiting for TFP to halt before sending "
						       "start scan response"));
				       )
			} else
				sch_send_start_scan_rsp(pMac);
		} else {
			/* In eDCF mode, send the response right away */
			sch_send_start_scan_rsp(pMac);
		}
		break;

	case SIR_SCH_END_SCAN_NTF:
		PELOG3(sch_log(pMac, LOG3,
			       FL("Received STOP_SCAN_NTF from LIM"));
		       )
		pMac->sch.gSchScanReqRcvd = false;
		break;

	case SIR_CFG_PARAM_UPDATE_IND:

		if (wlan_cfg_get_int(pMac, (uint16_t) pSchMsg->bodyval, &val) !=
		    eSIR_SUCCESS)
			sch_log(pMac, LOGP, FL("failed to cfg get id %d"),
				pSchMsg->bodyval);

		switch (pSchMsg->bodyval) {
		case WNI_CFG_BEACON_INTERVAL:
			/* What to do for IBSS ?? - TBD */
			if (LIM_IS_AP_ROLE(psessionEntry))
				sch_set_beacon_interval(pMac, psessionEntry);
			break;

		case WNI_CFG_DTIM_PERIOD:
			pMac->sch.schObject.gSchDTIMCount = 0;
			break;

		case WNI_CFG_CFP_PERIOD:
			pMac->sch.schObject.gSchCFPCount = 0;
			break;

		case WNI_CFG_EDCA_PROFILE:
			sch_edca_profile_update(pMac, psessionEntry);
			break;

		case WNI_CFG_EDCA_ANI_ACBK_LOCAL:
		case WNI_CFG_EDCA_ANI_ACBE_LOCAL:
		case WNI_CFG_EDCA_ANI_ACVI_LOCAL:
		case WNI_CFG_EDCA_ANI_ACVO_LOCAL:
		case WNI_CFG_EDCA_WME_ACBK_LOCAL:
		case WNI_CFG_EDCA_WME_ACBE_LOCAL:
		case WNI_CFG_EDCA_WME_ACVI_LOCAL:
		case WNI_CFG_EDCA_WME_ACVO_LOCAL:
			if (LIM_IS_AP_ROLE(psessionEntry))
				sch_qos_update_local(pMac, psessionEntry);
			break;

		case WNI_CFG_EDCA_ANI_ACBK:
		case WNI_CFG_EDCA_ANI_ACBE:
		case WNI_CFG_EDCA_ANI_ACVI:
		case WNI_CFG_EDCA_ANI_ACVO:
		case WNI_CFG_EDCA_WME_ACBK:
		case WNI_CFG_EDCA_WME_ACBE:
		case WNI_CFG_EDCA_WME_ACVI:
		case WNI_CFG_EDCA_WME_ACVO:
			if (LIM_IS_AP_ROLE(psessionEntry)) {
				sch_qos_update_broadcast(pMac, psessionEntry);
			}
			break;

		default:
			sch_log(pMac, LOGE,
				FL("Cfg param %d indication not handled"),
				pSchMsg->bodyval);
		}
		break;

	default:
		sch_log(pMac, LOGE, FL("Unknown message in schMsgQ type %d"),
			pSchMsg->type);
	}

}

/* get the local or broadcast parameters based on the profile sepcified in the config */
/* params are delivered in this order: BK, BE, VI, VO */
static tSirRetStatus
sch_get_params(tpAniSirGlobal pMac,
	       uint32_t params[][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN],
	       uint8_t local)
{
	uint32_t val;
	uint32_t i, idx;
	uint32_t *prf;

	uint32_t ani_l[] = {
	  WNI_CFG_EDCA_ANI_ACBE_LOCAL, WNI_CFG_EDCA_ANI_ACBK_LOCAL,
	  WNI_CFG_EDCA_ANI_ACVI_LOCAL, WNI_CFG_EDCA_ANI_ACVO_LOCAL};
	uint32_t wme_l[] = {
	  WNI_CFG_EDCA_WME_ACBE_LOCAL, WNI_CFG_EDCA_WME_ACBK_LOCAL,
	  WNI_CFG_EDCA_WME_ACVI_LOCAL, WNI_CFG_EDCA_WME_ACVO_LOCAL};
	uint32_t ani_b[] = { WNI_CFG_EDCA_ANI_ACBE, WNI_CFG_EDCA_ANI_ACBK,
			     WNI_CFG_EDCA_ANI_ACVI, WNI_CFG_EDCA_ANI_ACVO};
	uint32_t wme_b[] = { WNI_CFG_EDCA_WME_ACBE, WNI_CFG_EDCA_WME_ACBK,
			     WNI_CFG_EDCA_WME_ACVI, WNI_CFG_EDCA_WME_ACVO};

	if (wlan_cfg_get_int(pMac, WNI_CFG_EDCA_PROFILE, &val) != eSIR_SUCCESS) {
		sch_log(pMac, LOGP, FL("failed to cfg get EDCA_PROFILE id %d"),
			WNI_CFG_EDCA_PROFILE);
		return eSIR_FAILURE;
	}

	if (val >= WNI_CFG_EDCA_PROFILE_MAX) {
		sch_log(pMac, LOGE,
			FL("Invalid EDCA_PROFILE %d, using %d instead"), val,
			WNI_CFG_EDCA_PROFILE_ANI);
		val = WNI_CFG_EDCA_PROFILE_ANI;
	}

	sch_log(pMac, LOGW, FL("EdcaProfile: Using %d (%s)"), val,
		((val == WNI_CFG_EDCA_PROFILE_WMM) ? "WMM" : "HiPerf"));

	if (local) {
		switch (val) {
		case WNI_CFG_EDCA_PROFILE_WMM:
			prf = &wme_l[0];
			break;
		case WNI_CFG_EDCA_PROFILE_ANI:
		default:
			prf = &ani_l[0];
			break;
		}
	} else {
		switch (val) {
		case WNI_CFG_EDCA_PROFILE_WMM:
			prf = &wme_b[0];
			break;
		case WNI_CFG_EDCA_PROFILE_ANI:
		default:
			prf = &ani_b[0];
			break;
		}
	}

	for (i = 0; i < 4; i++) {
		uint8_t data[WNI_CFG_EDCA_ANI_ACBK_LEN];
		uint32_t len = WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN;
		if (wlan_cfg_get_str
			    (pMac, (uint16_t) prf[i], (uint8_t *) &data[0],
			    &len) != eSIR_SUCCESS) {
			sch_log(pMac, LOGP, FL("cfgGet failed for %d"), prf[i]);
			return eSIR_FAILURE;
		}
		if (len > WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN) {
			sch_log(pMac, LOGE,
				FL("cfgGet for %d: length is %d instead of %d"),
				prf[i], len, WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN);
			return eSIR_FAILURE;
		}
		for (idx = 0; idx < len; idx++)
			params[i][idx] = (uint32_t) data[idx];
	}
	PELOG1(sch_log
		       (pMac, LOG1, FL("GetParams: local=%d, profile = %d Done"), local,
		       val);
	       )
	return eSIR_SUCCESS;
}

/**
 * broadcast_wmm_of_concurrent_sta_session() - broadcasts wmm info
 * @mac_ctx:          mac global context
 * @session:       pesession entry
 *
 * Return: true if wmm param updated, false if wmm param not updated
 */
static bool
broadcast_wmm_of_concurrent_sta_session(tpAniSirGlobal mac_ctx,
					tpPESession session)
{
	uint8_t i, j;
	tpPESession concurrent_session = NULL;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		/*
		 * Find another INFRA STA AP session on same operating channel.
		 * The session entry passed to this API is for GO/SoftAP session
		 * that is getting added currently
		 */
		if (!((mac_ctx->lim.gpSession[i].valid == true) &&
		    (mac_ctx->lim.gpSession[i].peSessionId !=
		     session->peSessionId)
		    && (mac_ctx->lim.gpSession[i].currentOperChannel ==
			session->currentOperChannel)
		    && (mac_ctx->lim.gpSession[i].limSystemRole
			== eLIM_STA_ROLE)))
			continue;

		concurrent_session = &(mac_ctx->lim.gpSession[i]);
		break;
	}

	if (concurrent_session == NULL)
		return false;

	if (!qdf_mem_cmp(session->gLimEdcaParamsBC,
	   concurrent_session->gLimEdcaParams,
	   sizeof(concurrent_session->gLimEdcaParams)))
		return false;

	/*
	 * Once atleast one concurrent session on same channel is found and WMM
	 * broadcast params for current SoftAP/GO session updated, return
	 */
	for (j = 0; j < MAX_NUM_AC; j++) {
		session->gLimEdcaParamsBC[j].aci.acm =
			concurrent_session->gLimEdcaParams[j].aci.acm;
		session->gLimEdcaParamsBC[j].aci.aifsn =
			concurrent_session->gLimEdcaParams[j].aci.aifsn;
		session->gLimEdcaParamsBC[j].cw.min =
			concurrent_session->gLimEdcaParams[j].cw.min;
		session->gLimEdcaParamsBC[j].cw.max =
			concurrent_session->gLimEdcaParams[j].cw.max;
		session->gLimEdcaParamsBC[j].txoplimit =
			concurrent_session->gLimEdcaParams[j].txoplimit;
		PELOG1(sch_log(mac_ctx, LOG1,
		       FL("QoSUpdateBCast changed again due to concurrent INFRA STA session: AC :%d: AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d"),
		       j, session->gLimEdcaParamsBC[j].aci.aifsn,
		       session->gLimEdcaParamsBC[j].aci.acm,
		       session->gLimEdcaParamsBC[j].cw.min,
		       session->gLimEdcaParamsBC[j].cw.max,
		       session->gLimEdcaParamsBC[j].txoplimit);)
	}
	return true;
}

void sch_qos_update_broadcast(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t params[4][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN];
	uint32_t cwminidx, cwmaxidx, txopidx;
	uint32_t phyMode;
	uint8_t i;
	bool updated = false;

	if (sch_get_params(pMac, params, false) != eSIR_SUCCESS) {
		PELOGE(sch_log(pMac, LOGE, FL("QosUpdateBroadcast: failed"));)
		return;
	}
	lim_get_phy_mode(pMac, &phyMode, psessionEntry);

	PELOG1(sch_log(pMac, LOG1, "QosUpdBcast: mode %d", phyMode);)

	if (phyMode == WNI_CFG_PHY_MODE_11G) {
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMING_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXG_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPG_IDX;
	} else if (phyMode == WNI_CFG_PHY_MODE_11B) {
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMINB_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXB_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPB_IDX;
	} else {
		/* This can happen if mode is not set yet, assume 11a mode */
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMINA_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXA_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPA_IDX;
	}

	for (i = 0; i < MAX_NUM_AC; i++) {
		if (psessionEntry->gLimEdcaParamsBC[i].aci.acm !=
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_ACM_IDX]) {
			psessionEntry->gLimEdcaParamsBC[i].aci.acm =
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_ACM_IDX];
			updated = true;
		}
		if (psessionEntry->gLimEdcaParamsBC[i].aci.aifsn !=
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_AIFSN_IDX]) {
			psessionEntry->gLimEdcaParamsBC[i].aci.aifsn =
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_AIFSN_IDX];
			updated = true;
		}
		if (psessionEntry->gLimEdcaParamsBC[i].cw.min !=
			convert_cw(GET_CW(&params[i][cwminidx]))) {
			psessionEntry->gLimEdcaParamsBC[i].cw.min =
			convert_cw(GET_CW(&params[i][cwminidx]));
			updated = true;
		}
		if (psessionEntry->gLimEdcaParamsBC[i].cw.max !=
			convert_cw(GET_CW(&params[i][cwmaxidx]))) {
			psessionEntry->gLimEdcaParamsBC[i].cw.max =
			convert_cw(GET_CW(&params[i][cwmaxidx]));
			updated = true;
		}
		if (psessionEntry->gLimEdcaParamsBC[i].txoplimit !=
			(uint16_t) params[i][txopidx]) {
			psessionEntry->gLimEdcaParamsBC[i].txoplimit =
			(uint16_t) params[i][txopidx];
			updated = true;
		}

		PELOG1(sch_log
			       (pMac, LOG1,
			       "QoSUpdateBCast: AC :%d: AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d",
			       i, psessionEntry->gLimEdcaParamsBC[i].aci.aifsn,
			       psessionEntry->gLimEdcaParamsBC[i].aci.acm,
			       psessionEntry->gLimEdcaParamsBC[i].cw.min,
			       psessionEntry->gLimEdcaParamsBC[i].cw.max,
			       psessionEntry->gLimEdcaParamsBC[i].txoplimit);
		       )

	}

	/* If there exists a concurrent STA-AP session, use its WMM params to broadcast in beacons. WFA Wifi Direct test plan 6.1.14 requirement */
	if (broadcast_wmm_of_concurrent_sta_session(pMac, psessionEntry))
		updated = true;
	if (updated)
		psessionEntry->gLimEdcaParamSetCount++;

	if (sch_set_fixed_beacon_fields(pMac, psessionEntry) != eSIR_SUCCESS)
		PELOGE(sch_log(pMac, LOGE, "Unable to set beacon fields!");)
}

void sch_qos_update_local(tpAniSirGlobal pMac, tpPESession psessionEntry)
{

	uint32_t params[4][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN];

	if (sch_get_params(pMac, params, true /*local */) != eSIR_SUCCESS) {
		PELOGE(sch_log(pMac, LOGE, FL("sch_get_params(local) failed"));)
		return;
	}

	set_sch_edca_params(pMac, params, psessionEntry);

	/* For AP, the bssID is stored in LIM Global context. */
	lim_send_edca_params(pMac, psessionEntry->gLimEdcaParams,
			     psessionEntry->bssIdx);
}

/** ----------------------------------------------------------
   \fn      sch_set_default_edca_params
   \brief   This function sets the gLimEdcaParams to the default
 \        local wmm profile.
   \param   tpAniSirGlobal  pMac
   \return  none
 \ ------------------------------------------------------------ */
void sch_set_default_edca_params(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t params[4][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN];

	if (get_wmm_local_params(pMac, params) != eSIR_SUCCESS) {
		PELOGE(sch_log(pMac, LOGE, FL("get_wmm_local_params() failed"));)
		return;
	}

	set_sch_edca_params(pMac, params, psessionEntry);
	return;
}

/** ----------------------------------------------------------
   \fn      set_sch_edca_params
   \brief   This function fills in the gLimEdcaParams structure
 \        with the given edca params.
   \param   tpAniSirGlobal  pMac
   \return  none
 \ ------------------------------------------------------------ */
static void
set_sch_edca_params(tpAniSirGlobal pMac,
		    uint32_t params[][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN],
		    tpPESession psessionEntry)
{
	uint32_t i;
	uint32_t cwminidx, cwmaxidx, txopidx;
	uint32_t phyMode;

	lim_get_phy_mode(pMac, &phyMode, psessionEntry);

	PELOG1(sch_log(pMac, LOG1, FL("lim_get_phy_mode() = %d"), phyMode);)
	/* if (pMac->lim.gLimPhyMode == WNI_CFG_PHY_MODE_11G) */
	if (phyMode == WNI_CFG_PHY_MODE_11G) {
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMING_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXG_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPG_IDX;
	}
	/* else if (pMac->lim.gLimPhyMode == WNI_CFG_PHY_MODE_11B) */
	else if (phyMode == WNI_CFG_PHY_MODE_11B) {
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMINB_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXB_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPB_IDX;
	} else {
		/* This can happen if mode is not set yet, assume 11a mode */
		cwminidx = WNI_CFG_EDCA_PROFILE_CWMINA_IDX;
		cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXA_IDX;
		txopidx = WNI_CFG_EDCA_PROFILE_TXOPA_IDX;
	}

	for (i = 0; i < MAX_NUM_AC; i++) {
		psessionEntry->gLimEdcaParams[i].aci.acm =
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_ACM_IDX];
		psessionEntry->gLimEdcaParams[i].aci.aifsn =
			(uint8_t) params[i][WNI_CFG_EDCA_PROFILE_AIFSN_IDX];
		psessionEntry->gLimEdcaParams[i].cw.min =
			convert_cw(GET_CW(&params[i][cwminidx]));
		psessionEntry->gLimEdcaParams[i].cw.max =
			convert_cw(GET_CW(&params[i][cwmaxidx]));
		psessionEntry->gLimEdcaParams[i].txoplimit =
			(uint16_t) params[i][txopidx];

		PELOG1(sch_log
			       (pMac, LOG1,
			       FL
				       ("AC :%d: AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d"),
			       i, psessionEntry->gLimEdcaParams[i].aci.aifsn,
			       psessionEntry->gLimEdcaParams[i].aci.acm,
			       psessionEntry->gLimEdcaParams[i].cw.min,
			       psessionEntry->gLimEdcaParams[i].cw.max,
			       psessionEntry->gLimEdcaParams[i].txoplimit);
		       )

	}
	return;
}

/** ----------------------------------------------------------
   \fn      get_wmm_local_params
   \brief   This function gets the WMM local edca parameters.
   \param   tpAniSirGlobal  pMac
   \param   uint32_t params[][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN]
   \return  none
 \ ------------------------------------------------------------ */
static tSirRetStatus
get_wmm_local_params(tpAniSirGlobal pMac,
		     uint32_t params[][WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN])
{
	uint32_t i, idx;
	uint32_t *prf;
	uint32_t wme_l[] = {
	  WNI_CFG_EDCA_WME_ACBE_LOCAL, WNI_CFG_EDCA_WME_ACBK_LOCAL,
	  WNI_CFG_EDCA_WME_ACVI_LOCAL, WNI_CFG_EDCA_WME_ACVO_LOCAL};

	prf = &wme_l[0];
	for (i = 0; i < 4; i++) {
		uint8_t data[WNI_CFG_EDCA_ANI_ACBK_LEN];
		uint32_t len = WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN;
		if (wlan_cfg_get_str
			    (pMac, (uint16_t) prf[i], (uint8_t *) &data[0],
			    &len) != eSIR_SUCCESS) {
			sch_log(pMac, LOGP, FL("cfgGet failed for %d"), prf[i]);
			return eSIR_FAILURE;
		}
		if (len > WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN) {
			sch_log(pMac, LOGE,
				FL("cfgGet for %d: length is %d instead of %d"),
				prf[i], len, WNI_CFG_EDCA_ANI_ACBK_LOCAL_LEN);
			return eSIR_FAILURE;
		}
		for (idx = 0; idx < len; idx++)
			params[i][idx] = (uint32_t) data[idx];
	}
	return eSIR_SUCCESS;
}

/** ----------------------------------------------------------
   \fn      sch_edca_profile_update
   \brief   This function updates the local and broadcast
 \        EDCA params in the gLimEdcaParams structure. It also
 \        updates the edcaParamSetCount.
   \param   tpAniSirGlobal  pMac
   \return  none
 \ ------------------------------------------------------------ */
void sch_edca_profile_update(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	if (LIM_IS_AP_ROLE(psessionEntry) ||
	    LIM_IS_IBSS_ROLE(psessionEntry)) {
		sch_qos_update_local(pMac, psessionEntry);
		sch_qos_update_broadcast(pMac, psessionEntry);
	}
}

/* -------------------------------------------------------------------- */
