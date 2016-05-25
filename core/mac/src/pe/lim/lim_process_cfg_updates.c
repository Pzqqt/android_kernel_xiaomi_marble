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
 * This file lim_process_cfg_updates.cc contains the utility functions
 * to handle various CFG parameter update events
 * Author:        Chandra Modumudi
 * Date:          01/20/03
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "ani_global.h"

#include "wni_cfg.h"
#include "sir_mac_prot_def.h"
#include "cfg_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_prop_exts_utils.h"
#include "sch_api.h"
#include "rrm_api.h"

static void lim_update_config(tpAniSirGlobal pMac, tpPESession psessionEntry);

static void lim_set_default_key_id_and_keys(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN6
	uint32_t val;
	uint32_t dkCfgId;
	PELOG1(lim_log(pMac, LOG1, FL("Setting default keys at SP"));)
	if (wlan_cfg_get_int(pMac, WNI_CFG_WEP_DEFAULT_KEYID,
			     &val) != eSIR_SUCCESS) {
		lim_log(pMac, LOGP,
			FL("Unable to retrieve defaultKeyId from CFG"));
	}
	dkCfgId = limGetCfgIdOfDefaultKeyid(val);
#endif

} /*** end lim_set_default_key_id_and_keys() ***/
/** -------------------------------------------------------------
   \fn lim_set_cfg_protection
   \brief sets lim global cfg cache from the config.
   \param      tpAniSirGlobal    pMac
   \return      None
   -------------------------------------------------------------*/
void lim_set_cfg_protection(tpAniSirGlobal pMac, tpPESession pesessionEntry)
{
	uint32_t val = 0;

	if (pesessionEntry != NULL && LIM_IS_AP_ROLE(pesessionEntry)) {
		if (pesessionEntry->gLimProtectionControl ==
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
			qdf_mem_set((void *)&pesessionEntry->cfgProtection,
				    sizeof(tCfgProtection), 0);
		else {
			lim_log(pMac, LOG1,
				FL(" frm11a = %d, from11b = %d, frm11g = %d, "
				   "ht20 = %d, nongf = %d, lsigTxop = %d, "
				   "rifs = %d, obss = %d"),
				pesessionEntry->cfgProtection.fromlla,
				pesessionEntry->cfgProtection.fromllb,
				pesessionEntry->cfgProtection.fromllg,
				pesessionEntry->cfgProtection.ht20,
				pesessionEntry->cfgProtection.nonGf,
				pesessionEntry->cfgProtection.lsigTxop,
				pesessionEntry->cfgProtection.rifs,
				pesessionEntry->cfgProtection.obss);
		}
	} else {
		if (wlan_cfg_get_int(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, &val)
		    != eSIR_SUCCESS) {
			lim_log(pMac, LOGP,
				FL
					("reading WNI_CFG_FORCE_POLICY_PROTECTION cfg failed"));
			return;
		} else
			pMac->lim.gLimProtectionControl = (uint8_t) val;

		if (wlan_cfg_get_int(pMac, WNI_CFG_PROTECTION_ENABLED, &val) !=
		    eSIR_SUCCESS) {
			lim_log(pMac, LOGP, FL("reading protection cfg failed"));
			return;
		}

		if (pMac->lim.gLimProtectionControl ==
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
			qdf_mem_set((void *)&pMac->lim.cfgProtection,
				    sizeof(tCfgProtection), 0);
		else {
			pMac->lim.cfgProtection.fromlla =
				(val >> WNI_CFG_PROTECTION_ENABLED_FROM_llA) & 1;
			pMac->lim.cfgProtection.fromllb =
				(val >> WNI_CFG_PROTECTION_ENABLED_FROM_llB) & 1;
			pMac->lim.cfgProtection.fromllg =
				(val >> WNI_CFG_PROTECTION_ENABLED_FROM_llG) & 1;
			pMac->lim.cfgProtection.ht20 =
				(val >> WNI_CFG_PROTECTION_ENABLED_HT_20) & 1;
			pMac->lim.cfgProtection.nonGf =
				(val >> WNI_CFG_PROTECTION_ENABLED_NON_GF) & 1;
			pMac->lim.cfgProtection.lsigTxop =
				(val >> WNI_CFG_PROTECTION_ENABLED_LSIG_TXOP) & 1;
			pMac->lim.cfgProtection.rifs =
				(val >> WNI_CFG_PROTECTION_ENABLED_RIFS) & 1;
			pMac->lim.cfgProtection.obss =
				(val >> WNI_CFG_PROTECTION_ENABLED_OBSS) & 1;

		}
	}
}

/**
 * lim_handle_param_update()
 *
 ***FUNCTION:
 * This function is use to post a message whenever need indicate
 * there is update of config parameter.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac  - Pointer to Global MAC structure
 * @param  cfgId - ID of CFG parameter that got updated
 * @return None
 */
void lim_handle_param_update(tpAniSirGlobal pMac, eUpdateIEsType cfgId)
{
	tSirMsgQ msg = { 0 };
	uint32_t status;

	PELOG3(lim_log
		       (pMac, LOG3, FL("Handling CFG parameter id %X update"), cfgId);
	       )
	switch (cfgId) {
	case eUPDATE_IE_PROBE_BCN:
	{
		msg.type = SIR_LIM_UPDATE_BEACON;
		status = lim_post_msg_api(pMac, &msg);

		if (status != TX_SUCCESS)
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("Failed lim_post_msg_api %u"), status);
			       )
			break;
	}
	default:
		break;
	}
}

/**
 * lim_handle_cf_gparam_update()
 *
 ***FUNCTION:
 * This function is called by lim_process_messages() to
 * whenever SIR_CFG_PARAM_UPDATE_IND message is posted
 * to LIM (due to a set operation on a CFG parameter).
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac  - Pointer to Global MAC structure
 * @param  cfgId - ID of CFG parameter that got updated
 * @return None
 */

void lim_handle_cf_gparam_update(tpAniSirGlobal pMac, uint32_t cfgId)
{
	uint32_t val1, val2;
	uint16_t val16;
	tSirMacHTParametersInfo *pAmpduParamInfo;

	PELOG3(lim_log
		       (pMac, LOG3, FL("Handling CFG parameter id %X update"), cfgId);
	       )
	switch (cfgId) {
	case WNI_CFG_WEP_DEFAULT_KEYID:

		/* !!LAC - when the default KeyID is changed, force all of the */
		/* keys and the keyID to be reprogrammed.  this allows the */
		/* keys to change after the initial setting of the keys when the CFG was */
		/* applied at association time through CFG changes of the keys. */
		lim_set_default_key_id_and_keys(pMac);

		break;

	case WNI_CFG_EXCLUDE_UNENCRYPTED:
		if (wlan_cfg_get_int(pMac, WNI_CFG_EXCLUDE_UNENCRYPTED,
				     &val1) != eSIR_SUCCESS) {
			lim_log(pMac, LOGP,
				FL("Unable to retrieve excludeUnencr from CFG"));
		}
		lim_log(pMac, LOGE,
			FL("Unsupported CFG: WNI_CFG_EXCLUDE_UNENCRYPTED"));

		break;

	case WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT:
		if (pMac->lim.gLimMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE) {
			/* 'Change' timer for future activations */
			lim_deactivate_and_change_timer(pMac,
							eLIM_ASSOC_FAIL_TIMER);
		}

		break;

	case WNI_CFG_PROTECTION_ENABLED:
		lim_set_cfg_protection(pMac, NULL);
		break;
	case WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG:
	{
		tSirMsgQ msg = { 0 };
		uint32_t status;

		msg.type = SIR_LIM_UPDATE_BEACON;

		status = lim_post_msg_api(pMac, &msg);

		if (status != TX_SUCCESS)
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("Failed lim_post_msg_api %u"), status);
			       )
			break;
	}

	case WNI_CFG_MPDU_DENSITY:
		if (wlan_cfg_get_int(pMac, WNI_CFG_HT_AMPDU_PARAMS, &val1) !=
		    eSIR_SUCCESS) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not retrieve HT AMPDU Param CFG"));
			       )
			break;
		}
		if (wlan_cfg_get_int(pMac, WNI_CFG_MPDU_DENSITY, &val2) !=
		    eSIR_SUCCESS) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not retrieve MPDU Density CFG"));
			       )
			break;
		}
		val16 = (uint16_t) val1;
		pAmpduParamInfo = (tSirMacHTParametersInfo *) &val16;
		pAmpduParamInfo->mpduDensity = (uint8_t) val2;
		if (cfg_set_int
			    (pMac, WNI_CFG_HT_AMPDU_PARAMS,
			    *(uint8_t *) pAmpduParamInfo) != eSIR_SUCCESS)
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not update HT AMPDU Param CFG"));
			       )

			break;
	case WNI_CFG_MAX_RX_AMPDU_FACTOR:
		if (wlan_cfg_get_int(pMac, WNI_CFG_HT_AMPDU_PARAMS, &val1) !=
		    eSIR_SUCCESS) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not retrieve HT AMPDU Param CFG"));
			       )
			break;
		}
		if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_RX_AMPDU_FACTOR, &val2) !=
		    eSIR_SUCCESS) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not retrieve AMPDU Factor CFG"));
			       )
			break;
		}
		val16 = (uint16_t) val1;
		pAmpduParamInfo = (tSirMacHTParametersInfo *) &val16;
		pAmpduParamInfo->maxRxAMPDUFactor = (uint8_t) val2;
		if (cfg_set_int
			    (pMac, WNI_CFG_HT_AMPDU_PARAMS,
			    *(uint8_t *) pAmpduParamInfo) != eSIR_SUCCESS)
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not update HT AMPDU Param CFG"));
			       )
			break;

	case WNI_CFG_DOT11_MODE:
		if (wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &val1) !=
		    eSIR_SUCCESS) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("could not retrieve Dot11 Mode  CFG"));
			       )
			break;
		}
		break;

	case WNI_CFG_SCAN_IN_POWERSAVE:
		if (wlan_cfg_get_int(pMac, WNI_CFG_SCAN_IN_POWERSAVE, &val1) !=
		    eSIR_SUCCESS) {
			lim_log(pMac, LOGE,
				FL("Unable to get WNI_CFG_SCAN_IN_POWERSAVE "));
			break;
		}
		pMac->lim.gScanInPowersave = (uint8_t) val1;
		break;

	case WNI_CFG_ASSOC_STA_LIMIT:
		if (wlan_cfg_get_int(pMac, WNI_CFG_ASSOC_STA_LIMIT, &val1) !=
		    eSIR_SUCCESS) {
			lim_log(pMac, LOGE,
				FL("Unable to get WNI_CFG_ASSOC_STA_LIMIT"));
			break;
		}
		pMac->lim.gLimAssocStaLimit = (uint16_t) val1;
		break;
	default:
		break;
	}
} /*** end lim_handle_cf_gparam_update() ***/

/**
 * lim_apply_configuration()
 *
 ***FUNCTION:
 * This function is called to apply the configured parameters
 * before joining or reassociating with a BSS or starting a BSS.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac  - Pointer to Global MAC structure
 * @return None
 */

void lim_apply_configuration(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t val = 0, phyMode;

	PELOG2(lim_log(pMac, LOG2, FL("Applying config"));)

	psessionEntry->limSentCapsChangeNtf = false;

	lim_get_phy_mode(pMac, &phyMode, psessionEntry);

	/* Set default keyId and keys */
	lim_set_default_key_id_and_keys(pMac);

	lim_update_config(pMac, psessionEntry);

	lim_get_short_slot_from_phy_mode(pMac, psessionEntry, phyMode,
					 &psessionEntry->shortSlotTimeSupported);

	lim_set_cfg_protection(pMac, psessionEntry);

	/* Added for BT - AMP Support */
	if (LIM_IS_AP_ROLE(psessionEntry) ||
	    LIM_IS_IBSS_ROLE(psessionEntry)) {
		/* This check is required to ensure the beacon generation is not done
		   as a part of join request for a BT-AMP station */

		if (psessionEntry->statypeForBss == STA_ENTRY_SELF) {
			PELOG1(lim_log
				       (pMac, LOG1,
				       FL("Initializing BT-AMP beacon generation"));
			       )
			sch_set_beacon_interval(pMac, psessionEntry);
			sch_set_fixed_beacon_fields(pMac, psessionEntry);
		}
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_SCAN_IN_POWERSAVE, &val) !=
	    eSIR_SUCCESS) {
		lim_log(pMac, LOGP,
			FL("could not retrieve WNI_CFG_SCAN_IN_POWERSAVE"));
		return;
	}

	PELOG1(lim_log(pMac, LOG1, FL("pMac->lim.gScanInPowersave = %hu"),
		       pMac->lim.gScanInPowersave);
	       )
	pMac->lim.gScanInPowersave = (uint8_t) val;

} /*** end lim_apply_configuration() ***/

/**
 * lim_update_config
 *
 * FUNCTION:
 * Update the local state from CFG database
 * (This used to be dphUpdateConfig)
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

static void lim_update_config(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	uint32_t val;

	sir_copy_mac_addr(pMac->lim.gLimMyMacAddr, psessionEntry->selfMacAddr);

	if (wlan_cfg_get_int(pMac, WNI_CFG_SHORT_PREAMBLE, &val) != eSIR_SUCCESS)
		lim_log(pMac, LOGP, FL("cfg get short preamble failed"));
	psessionEntry->beaconParams.fShortPreamble = (val) ? 1 : 0;

	/* In STA case this parameter is filled during the join request */
	if (LIM_IS_AP_ROLE(psessionEntry) ||
	    LIM_IS_IBSS_ROLE(psessionEntry)) {
		if (wlan_cfg_get_int(pMac, WNI_CFG_WME_ENABLED, &val) !=
		    eSIR_SUCCESS)
			lim_log(pMac, LOGP, FL("cfg get wme enabled failed"));
		psessionEntry->limWmeEnabled = (val) ? 1 : 0;
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_WSM_ENABLED, &val) != eSIR_SUCCESS)
		lim_log(pMac, LOGP, FL("cfg get wsm enabled failed"));
	psessionEntry->limWsmEnabled = (val) ? 1 : 0;

	if ((!psessionEntry->limWmeEnabled) && (psessionEntry->limWsmEnabled)) {
		PELOGE(lim_log(pMac, LOGE, FL("Can't enable WSM without WME"));)
		psessionEntry->limWsmEnabled = 0;
	}
	/* In STA , this parameter is filled during the join request */
	if (LIM_IS_AP_ROLE(psessionEntry) || LIM_IS_IBSS_ROLE(psessionEntry)) {
		if (wlan_cfg_get_int(pMac, WNI_CFG_QOS_ENABLED, &val) !=
		    eSIR_SUCCESS)
			lim_log(pMac, LOGP, FL("cfg get qos enabled failed"));
		psessionEntry->limQosEnabled = (val) ? 1 : 0;
	}
	if (wlan_cfg_get_int(pMac, WNI_CFG_HCF_ENABLED, &val) != eSIR_SUCCESS)
		lim_log(pMac, LOGP, FL("cfg get hcf enabled failed"));
	psessionEntry->limHcfEnabled = (val) ? 1 : 0;

	/* AP: WSM should enable HCF as well, for STA enable WSM only after */
	/* association response is received */
	if (psessionEntry->limWsmEnabled && LIM_IS_AP_ROLE(psessionEntry))
		psessionEntry->limHcfEnabled = 1;

	if (wlan_cfg_get_int(pMac, WNI_CFG_11D_ENABLED, &val) != eSIR_SUCCESS)
		lim_log(pMac, LOGP, FL("cfg get 11d enabled failed"));
	psessionEntry->lim11dEnabled = (val) ? 1 : 0;

	if (wlan_cfg_get_int(pMac, WNI_CFG_ASSOC_STA_LIMIT, &val) != eSIR_SUCCESS) {
		lim_log(pMac, LOGP, FL("cfg get assoc sta limit failed"));
	}
	pMac->lim.gLimAssocStaLimit = (uint16_t) val;

	PELOG1(lim_log(pMac, LOG1, FL("Updated Lim shadow state based on CFG"));)
}
