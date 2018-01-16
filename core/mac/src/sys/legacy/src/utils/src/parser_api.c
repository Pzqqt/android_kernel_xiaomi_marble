/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * This file parser_api.cc contains the code for parsing
 * 802.11 messages.
 * Author:        Pierre Vandwalle
 * Date:          03/18/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "sir_api.h"
#include "ani_global.h"
#include "parser_api.h"
#include "cfg_api.h"
#include "lim_utils.h"
#include "utils_parser.h"
#include "lim_ser_des_utils.h"
#include "sch_api.h"
#include "wmm_apsd.h"
#include "rrm_api.h"

#include "cds_regdomain.h"
#include "qdf_crypto.h"
#include "lim_process_fils.h"
#include "wlan_utility.h"
#include "wifi_pos_api.h"


#define RSN_OUI_SIZE 4
/* ////////////////////////////////////////////////////////////////////// */
void swap_bit_field16(uint16_t in, uint16_t *out)
{
#ifdef ANI_LITTLE_BIT_ENDIAN
	*out = in;
#else                           /* Big-Endian... */
	*out = ((in & 0x8000) >> 15) |
	       ((in & 0x4000) >> 13) |
	       ((in & 0x2000) >> 11) |
	       ((in & 0x1000) >> 9) |
	       ((in & 0x0800) >> 7) |
	       ((in & 0x0400) >> 5) |
	       ((in & 0x0200) >> 3) |
	       ((in & 0x0100) >> 1) |
	       ((in & 0x0080) << 1) |
	       ((in & 0x0040) << 3) |
	       ((in & 0x0020) << 5) |
	       ((in & 0x0010) << 7) |
	       ((in & 0x0008) << 9) |
	       ((in & 0x0004) << 11) |
	       ((in & 0x0002) << 13) | ((in & 0x0001) << 15);
#endif /* ANI_LITTLE_BIT_ENDIAN */
}

static inline void __print_wmm_params(tpAniSirGlobal pMac,
				      tDot11fIEWMMParams *pWmm)
{
	pe_debug("WMM Parameters Received:");
	pe_debug("BE: aifsn %d, acm %d, aci %d, cwmin %d, cwmax %d, txop %d",
		pWmm->acbe_aifsn, pWmm->acbe_acm, pWmm->acbe_aci,
		pWmm->acbe_acwmin, pWmm->acbe_acwmax, pWmm->acbe_txoplimit);

	pe_debug("BK: aifsn %d, acm %d, aci %d, cwmin %d, cwmax %d, txop %d",
		pWmm->acbk_aifsn, pWmm->acbk_acm, pWmm->acbk_aci,
		pWmm->acbk_acwmin, pWmm->acbk_acwmax, pWmm->acbk_txoplimit);

	pe_debug("VI: aifsn %d, acm %d, aci %d, cwmin %d, cwmax %d, txop %d",
		pWmm->acvi_aifsn, pWmm->acvi_acm, pWmm->acvi_aci,
		pWmm->acvi_acwmin, pWmm->acvi_acwmax, pWmm->acvi_txoplimit);

	pe_debug("VO: aifsn %d, acm %d, aci %d, cwmin %d, cwmax %d, txop %d",
		pWmm->acvo_aifsn, pWmm->acvo_acm, pWmm->acvo_aci,
		pWmm->acvo_acwmin, pWmm->acvo_acwmax, pWmm->acvo_txoplimit);

	return;
}

/* ////////////////////////////////////////////////////////////////////// */
/* Functions for populating "dot11f" style IEs */

/* return: >= 0, the starting location of the IE in rsnIEdata inside tSirRSNie */
/*         < 0, cannot find */
int find_ie_location(tpAniSirGlobal pMac, tpSirRSNie pRsnIe, uint8_t EID)
{
	int idx, ieLen, bytesLeft;
	int ret_val = -1;

	/* Here's what's going on: 'rsnIe' looks like this: */

	/*     typedef struct sSirRSNie */
	/*     { */
	/*         uint16_t       length; */
	/*         uint8_t        rsnIEdata[SIR_MAC_MAX_IE_LENGTH+2]; */
	/*     } tSirRSNie, *tpSirRSNie; */

	/* other code records both the WPA & RSN IEs (including their EIDs & */
	/* lengths) into the array 'rsnIEdata'.  We may have: */

	/*     With WAPI support, there may be 3 IEs here */
	/*     It can be only WPA IE, or only RSN IE or only WAPI IE */
	/*     Or two or all three of them with no particular ordering */

	/* The if/then/else statements that follow are here to figure out */
	/* whether we have the WPA IE, and where it is if we *do* have it. */

	/* Save the first IE length */
	ieLen = pRsnIe->rsnIEdata[1] + 2;
	idx = 0;
	bytesLeft = pRsnIe->length;

	while (1) {
		if (EID == pRsnIe->rsnIEdata[idx]) {
			/* Found it */
			return idx;
		} else if (EID != pRsnIe->rsnIEdata[idx] &&
			/* & if no more IE, */
			   bytesLeft <= (uint16_t) (ieLen)) {
			pe_debug("No IE (%d) in find_ie_location", EID);
			return ret_val;
		}
		bytesLeft -= ieLen;
		ieLen = pRsnIe->rsnIEdata[idx + 1] + 2;
		idx += ieLen;
	}

	return ret_val;
}

tSirRetStatus
populate_dot11f_capabilities(tpAniSirGlobal pMac,
			     tDot11fFfCapabilities *pDot11f,
			     tpPESession psessionEntry)
{
	uint16_t cfg;
	tSirRetStatus nSirStatus;

	nSirStatus = cfg_get_capability_info(pMac, &cfg, psessionEntry);
	if (eSIR_SUCCESS != nSirStatus) {
		pe_err("Failed to retrieve the Capabilities bitfield from CFG status: %d",
			   nSirStatus);
		return nSirStatus;
	}

	swap_bit_field16(cfg, (uint16_t *) pDot11f);

	return eSIR_SUCCESS;
} /* End populate_dot11f_capabilities. */

/**
 * populate_dot_11_f_ext_chann_switch_ann() - Function to populate ECS
 * @mac_ptr:            Pointer to PMAC structure
 * @dot_11_ptr:         ECS element
 * @session_entry:      PE session entry
 *
 * This function is used to populate the extended channel switch element
 *
 * Return: None
 */
void populate_dot_11_f_ext_chann_switch_ann(tpAniSirGlobal mac_ptr,
		tDot11fIEext_chan_switch_ann *dot_11_ptr,
		tpPESession session_entry)
{
	uint8_t ch_offset;

	if (session_entry->gLimChannelSwitch.ch_width == CH_WIDTH_80MHZ)
		ch_offset = BW80;
	else
		ch_offset = session_entry->gLimChannelSwitch.sec_ch_offset;

	dot_11_ptr->switch_mode = session_entry->gLimChannelSwitch.switchMode;
	dot_11_ptr->new_reg_class = wlan_reg_dmn_get_opclass_from_channel(
			mac_ptr->scan.countryCodeCurrent,
			session_entry->gLimChannelSwitch.primaryChannel,
			ch_offset);
	dot_11_ptr->new_channel =
		session_entry->gLimChannelSwitch.primaryChannel;
	dot_11_ptr->switch_count =
		session_entry->gLimChannelSwitch.switchCount;
	dot_11_ptr->present = 1;

	pe_debug("country:%s chan:%d width:%d reg:%d off:%d",
			mac_ptr->scan.countryCodeCurrent,
			session_entry->gLimChannelSwitch.primaryChannel,
			session_entry->gLimChannelSwitch.ch_width,
			dot_11_ptr->new_reg_class,
			session_entry->gLimChannelSwitch.sec_ch_offset);
}

void
populate_dot11f_chan_switch_ann(tpAniSirGlobal pMac,
				tDot11fIEChanSwitchAnn *pDot11f,
				tpPESession psessionEntry)
{
	pDot11f->switchMode = psessionEntry->gLimChannelSwitch.switchMode;
	pDot11f->newChannel = psessionEntry->gLimChannelSwitch.primaryChannel;
	pDot11f->switchCount =
		(uint8_t) psessionEntry->gLimChannelSwitch.switchCount;

	pDot11f->present = 1;
}

/**
 * populate_dot11_supp_operating_classes() - Function to populate supported
 *                      operating class IE
 * @mac_ptr:            Pointer to PMAC structure
 * @dot_11_ptr:         Operating class element
 * @session_entry:      PE session entry
 *
 * Return: None
 */
void
populate_dot11_supp_operating_classes(tpAniSirGlobal mac_ptr,
				tDot11fIESuppOperatingClasses *dot_11_ptr,
				tpPESession session_entry)
{
	uint8_t ch_bandwidth;

	if (session_entry->ch_width == CH_WIDTH_80MHZ) {
		ch_bandwidth = BW80;
	} else {
		switch (session_entry->htSecondaryChannelOffset) {
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			ch_bandwidth = BW40_HIGH_PRIMARY;
			break;
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
			ch_bandwidth = BW40_LOW_PRIMARY;
			break;
		default:
			ch_bandwidth = BW20;
			break;
		}
	}

	wlan_reg_dmn_get_curr_opclasses(&dot_11_ptr->num_classes,
					&dot_11_ptr->classes[1]);
	dot_11_ptr->classes[0] = wlan_reg_dmn_get_opclass_from_channel(
					mac_ptr->scan.countryCodeCurrent,
					session_entry->currentOperChannel,
					ch_bandwidth);
	dot_11_ptr->num_classes++;
	dot_11_ptr->present = 1;
}

void
populate_dot11f_chan_switch_wrapper(tpAniSirGlobal pMac,
				    tDot11fIEChannelSwitchWrapper *pDot11f,
				    tpPESession psessionEntry)
{
	const uint8_t *ie_ptr = NULL;

	/*
	 * The new country subelement is present only when
	 * 1. AP performs Extended Channel switching to new country.
	 * 2. New Operating Class table or a changed set of operating
	 * classes relative to the contents of the country element sent
	 * in the beacons.
	 *
	 * In the current scenario Channel Switch wrapper IE is included
	 * when we a radar is found and the AP does a channel change in
	 * the same regulatory domain(No country change or Operating class
	 * table). So, we do not need to include the New Country IE.
	 *
	 * Transmit Power Envlope Subelement is optional
	 * in Channel Switch Wrapper IE. So, not setting
	 * the TPE subelement. We include only WiderBWChanSwitchAnn.
	 */
	pDot11f->present = 1;

	/*
	 * Add the Wide Channel Bandwidth Sublement.
	 */
	pDot11f->WiderBWChanSwitchAnn.newChanWidth =
		psessionEntry->gLimWiderBWChannelSwitch.newChanWidth;
	pDot11f->WiderBWChanSwitchAnn.newCenterChanFreq0 =
		psessionEntry->gLimWiderBWChannelSwitch.newCenterChanFreq0;
	pDot11f->WiderBWChanSwitchAnn.newCenterChanFreq1 =
		psessionEntry->gLimWiderBWChannelSwitch.newCenterChanFreq1;
	pDot11f->WiderBWChanSwitchAnn.present = 1;

	/*
	 * Add the VHT Transmit power Envelope Sublement.
	 */
	ie_ptr = wlan_get_ie_ptr_from_eid(
			DOT11F_EID_VHT_TRANSMIT_POWER_ENV,
			psessionEntry->addIeParams.probeRespBCNData_buff,
			psessionEntry->addIeParams.probeRespBCNDataLen);
	if (ie_ptr) {
		/* Ignore EID field */
		pDot11f->vht_transmit_power_env.present = 1;
		pDot11f->vht_transmit_power_env.num_bytes = ie_ptr[1];
		qdf_mem_copy(pDot11f->vht_transmit_power_env.bytes,
			&ie_ptr[2], pDot11f->vht_transmit_power_env.num_bytes);
	}

}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
void
populate_dot11f_avoid_channel_ie(tpAniSirGlobal mac_ctx,
				 tDot11fIEQComVendorIE *dot11f,
				 tpPESession pe_session)
{
	if (!pe_session->sap_advertise_avoid_ch_ie)
		return;

	dot11f->present = true;
	dot11f->type = QCOM_VENDOR_IE_MCC_AVOID_CH;
	dot11f->channel = pe_session->currentOperChannel;
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

void
populate_dot11f_wider_bw_chan_switch_ann(tpAniSirGlobal pMac,
					 tDot11fIEWiderBWChanSwitchAnn *pDot11f,
					 tpPESession psessionEntry)
{
	pDot11f->present = 1;
	pDot11f->newChanWidth =
		psessionEntry->gLimWiderBWChannelSwitch.newChanWidth;
	pDot11f->newCenterChanFreq0 =
		psessionEntry->gLimWiderBWChannelSwitch.newCenterChanFreq0;
	pDot11f->newCenterChanFreq1 =
		psessionEntry->gLimWiderBWChannelSwitch.newCenterChanFreq1;
}

tSirRetStatus
populate_dot11f_country(tpAniSirGlobal pMac,
			tDot11fIECountry *pDot11f, tpPESession psessionEntry)
{
	uint32_t len, maxlen;
	uint16_t item;
	tSirRetStatus nSirStatus;
	enum band_info rfBand;
	uint8_t temp[CFG_MAX_STR_LEN], code[3];

	if (psessionEntry->lim11dEnabled) {
		lim_get_rf_band_new(pMac, &rfBand, psessionEntry);
		if (rfBand == BAND_5G) {
			item = WNI_CFG_MAX_TX_POWER_5;
			maxlen = WNI_CFG_MAX_TX_POWER_5_LEN;
		} else {
			item = WNI_CFG_MAX_TX_POWER_2_4;
			maxlen = WNI_CFG_MAX_TX_POWER_2_4_LEN;
		}

		CFG_GET_STR(nSirStatus, pMac, item, temp, len, maxlen);

		if (3 > len) {
			/* no limit on tx power, cannot include the IE because at least */
			/* one (channel,num,tx power) must be present */
			return eSIR_SUCCESS;
		}

		wlan_reg_read_current_country(pMac->psoc, code);

		qdf_mem_copy(pDot11f->country, code, 2);

		if (len > MAX_SIZE_OF_TRIPLETS_IN_COUNTRY_IE) {
			pe_err("len:%d is out of bounds, resetting", len);
			len = MAX_SIZE_OF_TRIPLETS_IN_COUNTRY_IE;
		}

		pDot11f->num_triplets = (uint8_t) (len / 3);
		qdf_mem_copy((uint8_t *) pDot11f->triplets, temp, len);

		pDot11f->present = 1;
	}

	return eSIR_SUCCESS;
} /* End populate_dot11f_country. */

/**
 * populate_dot11f_ds_params() - To populate DS IE params
 * mac_ctx: Pointer to global mac context
 * dot11f_param: pointer to DS params IE
 * channel: channel number
 *
 * This routine will populate DS param in management frame like
 * beacon, probe response, and etc.
 *
 * Return: Overall sucess
 */
tSirRetStatus
populate_dot11f_ds_params(tpAniSirGlobal mac_ctx,
			  tDot11fIEDSParams *dot11f_param, uint8_t channel)
{
	if (IS_24G_CH(channel)) {
		/* .11b/g mode PHY => Include the DS Parameter Set IE: */
		dot11f_param->curr_channel = channel;
		dot11f_param->present = 1;
	}

	return eSIR_SUCCESS;
}

#define SET_AIFSN(aifsn) (((aifsn) < 2) ? 2 : (aifsn))

void
populate_dot11f_edca_param_set(tpAniSirGlobal pMac,
			       tDot11fIEEDCAParamSet *pDot11f,
			       tpPESession psessionEntry)
{

	if (psessionEntry->limQosEnabled) {
		/* change to bitwise operation, after this is fixed in frames. */
		pDot11f->qos =
			(uint8_t) (0xf0 &
				   (psessionEntry->gLimEdcaParamSetCount << 4));

		/* Fill each EDCA parameter set in order: be, bk, vi, vo */
		pDot11f->acbe_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[0].aci.aifsn));
		pDot11f->acbe_acm =
			(0x1 & psessionEntry->gLimEdcaParamsBC[0].aci.acm);
		pDot11f->acbe_aci = (0x3 & SIR_MAC_EDCAACI_BESTEFFORT);
		pDot11f->acbe_acwmin =
			(0xf & psessionEntry->gLimEdcaParamsBC[0].cw.min);
		pDot11f->acbe_acwmax =
			(0xf & psessionEntry->gLimEdcaParamsBC[0].cw.max);
		pDot11f->acbe_txoplimit =
			psessionEntry->gLimEdcaParamsBC[0].txoplimit;

		pDot11f->acbk_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[1].aci.aifsn));
		pDot11f->acbk_acm =
			(0x1 & psessionEntry->gLimEdcaParamsBC[1].aci.acm);
		pDot11f->acbk_aci = (0x3 & SIR_MAC_EDCAACI_BACKGROUND);
		pDot11f->acbk_acwmin =
			(0xf & psessionEntry->gLimEdcaParamsBC[1].cw.min);
		pDot11f->acbk_acwmax =
			(0xf & psessionEntry->gLimEdcaParamsBC[1].cw.max);
		pDot11f->acbk_txoplimit =
			psessionEntry->gLimEdcaParamsBC[1].txoplimit;

		pDot11f->acvi_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[2].aci.aifsn));
		pDot11f->acvi_acm =
			(0x1 & psessionEntry->gLimEdcaParamsBC[2].aci.acm);
		pDot11f->acvi_aci = (0x3 & SIR_MAC_EDCAACI_VIDEO);
		pDot11f->acvi_acwmin =
			(0xf & psessionEntry->gLimEdcaParamsBC[2].cw.min);
		pDot11f->acvi_acwmax =
			(0xf & psessionEntry->gLimEdcaParamsBC[2].cw.max);
		pDot11f->acvi_txoplimit =
			psessionEntry->gLimEdcaParamsBC[2].txoplimit;

		pDot11f->acvo_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[3].aci.aifsn));
		pDot11f->acvo_acm =
			(0x1 & psessionEntry->gLimEdcaParamsBC[3].aci.acm);
		pDot11f->acvo_aci = (0x3 & SIR_MAC_EDCAACI_VOICE);
		pDot11f->acvo_acwmin =
			(0xf & psessionEntry->gLimEdcaParamsBC[3].cw.min);
		pDot11f->acvo_acwmax =
			(0xf & psessionEntry->gLimEdcaParamsBC[3].cw.max);
		pDot11f->acvo_txoplimit =
			psessionEntry->gLimEdcaParamsBC[3].txoplimit;

		pDot11f->present = 1;
	}

} /* End PopluateDot11fEDCAParamSet. */

tSirRetStatus
populate_dot11f_erp_info(tpAniSirGlobal pMac,
			 tDot11fIEERPInfo *pDot11f, tpPESession psessionEntry)
{
	tSirRetStatus nSirStatus;
	uint32_t val;
	enum band_info rfBand = BAND_UNKNOWN;

	lim_get_rf_band_new(pMac, &rfBand, psessionEntry);
	if (BAND_2G == rfBand) {
		pDot11f->present = 1;

		val = psessionEntry->cfgProtection.fromllb;
		if (!val) {
			pe_err("11B protection not enabled. Not populating ERP IE %d",
				val);
			return eSIR_SUCCESS;
		}

		if (psessionEntry->gLim11bParams.protectionEnabled) {
			pDot11f->non_erp_present = 1;
			pDot11f->use_prot = 1;
		}

		if (psessionEntry->gLimOlbcParams.protectionEnabled) {
			/* FIXME_PROTECTION: we should be setting non_erp present also. */
			/* check the test plan first. */
			pDot11f->use_prot = 1;
		}

		if ((psessionEntry->gLimNoShortParams.numNonShortPreambleSta)
		    || !psessionEntry->beaconParams.fShortPreamble) {
			pDot11f->barker_preamble = 1;

		}
		/* if protection always flag is set, advertise protection enabled */
		/* regardless of legacy stations presence */
		CFG_GET_INT(nSirStatus, pMac, WNI_CFG_11G_PROTECTION_ALWAYS,
			    val);

		if (val) {
			pDot11f->use_prot = 1;
		}
	}

	return eSIR_SUCCESS;
} /* End populate_dot11f_erp_info. */

tSirRetStatus
populate_dot11f_ext_supp_rates(tpAniSirGlobal pMac, uint8_t nChannelNum,
			       tDot11fIEExtSuppRates *pDot11f,
			       tpPESession psessionEntry)
{
	tSirRetStatus nSirStatus;
	uint32_t nRates = 0;
	uint8_t rates[SIR_MAC_RATESET_EID_MAX];

	/* Use the ext rates present in session entry whenever nChannelNum is set to OPERATIONAL
	   else use the ext supported rate set from CFG, which is fixed and does not change dynamically and is used for
	   sending mgmt frames (lile probe req) which need to go out before any session is present.
	 */
	if (POPULATE_DOT11F_RATES_OPERATIONAL == nChannelNum) {
		if (psessionEntry != NULL) {
			nRates = psessionEntry->extRateSet.numRates;
			qdf_mem_copy(rates, psessionEntry->extRateSet.rate,
				     nRates);
		} else {
			pe_err("no session context exists while populating Operational Rate Set");
		}
	} else if (HIGHEST_24GHZ_CHANNEL_NUM >= nChannelNum) {
		CFG_GET_STR(nSirStatus, pMac,
			    WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET, rates,
			    nRates, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET_LEN);
	}

	if (0 != nRates) {
		pDot11f->num_rates = (uint8_t) nRates;
		qdf_mem_copy(pDot11f->rates, rates, nRates);
		pDot11f->present = 1;
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_ext_supp_rates. */

tSirRetStatus
populate_dot11f_ext_supp_rates1(tpAniSirGlobal pMac,
				uint8_t nChannelNum,
				tDot11fIEExtSuppRates *pDot11f)
{
	uint32_t nRates;
	tSirRetStatus nSirStatus;
	uint8_t rates[SIR_MAC_MAX_NUMBER_OF_RATES];

	if (14 < nChannelNum) {
		pDot11f->present = 0;
		return eSIR_SUCCESS;
	}
	/* N.B. I have *no* idea why we're calling 'wlan_cfg_get_str' with an argument */
	/* of WNI_CFG_SUPPORTED_RATES_11A here, but that's what was done */
	/* previously & I'm afraid to change it! */
	CFG_GET_STR(nSirStatus, pMac, WNI_CFG_SUPPORTED_RATES_11A,
		    rates, nRates, SIR_MAC_MAX_NUMBER_OF_RATES);

	if (0 != nRates) {
		pDot11f->num_rates = (uint8_t) nRates;
		qdf_mem_copy(pDot11f->rates, rates, nRates);
		pDot11f->present = 1;
	}

	return eSIR_SUCCESS;
} /* populate_dot11f_ext_supp_rates1. */

tSirRetStatus
populate_dot11f_ht_caps(tpAniSirGlobal pMac,
			tpPESession psessionEntry, tDot11fIEHTCaps *pDot11f)
{
	uint32_t nCfgValue, nCfgLen;
	uint8_t nCfgValue8;
	tSirRetStatus nSirStatus;
	tSirMacHTParametersInfo *pHTParametersInfo;
	uint8_t disable_high_ht_mcs_2x2 = 0;
	union {
		uint16_t nCfgValue16;
		tSirMacHTCapabilityInfo htCapInfo;
		tSirMacExtendedHTCapabilityInfo extHtCapInfo;
	} uHTCapabilityInfo;

	tSirMacTxBFCapabilityInfo *pTxBFCapabilityInfo;
	tSirMacASCapabilityInfo *pASCapabilityInfo;

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_CAP_INFO, nCfgValue);

	uHTCapabilityInfo.nCfgValue16 = nCfgValue & 0xFFFF;

	pDot11f->mimoPowerSave = uHTCapabilityInfo.htCapInfo.mimoPowerSave;
	pDot11f->greenField = uHTCapabilityInfo.htCapInfo.greenField;
	pDot11f->delayedBA = uHTCapabilityInfo.htCapInfo.delayedBA;
	pDot11f->maximalAMSDUsize =
		uHTCapabilityInfo.htCapInfo.maximalAMSDUsize;
	pDot11f->dsssCckMode40MHz =
		uHTCapabilityInfo.htCapInfo.dsssCckMode40MHz;
	pDot11f->psmp = uHTCapabilityInfo.htCapInfo.psmp;
	pDot11f->stbcControlFrame =
		uHTCapabilityInfo.htCapInfo.stbcControlFrame;
	pDot11f->lsigTXOPProtection =
		uHTCapabilityInfo.htCapInfo.lsigTXOPProtection;

	/* All sessionized entries will need the check below */
	if (psessionEntry == NULL) {     /* Only in case of NO session */
		pDot11f->supportedChannelWidthSet =
			uHTCapabilityInfo.htCapInfo.supportedChannelWidthSet;
		pDot11f->advCodingCap =
			uHTCapabilityInfo.htCapInfo.advCodingCap;
		pDot11f->txSTBC = uHTCapabilityInfo.htCapInfo.txSTBC;
		pDot11f->rxSTBC = uHTCapabilityInfo.htCapInfo.rxSTBC;
		pDot11f->shortGI20MHz =
			uHTCapabilityInfo.htCapInfo.shortGI20MHz;
		pDot11f->shortGI40MHz =
			uHTCapabilityInfo.htCapInfo.shortGI40MHz;
	} else {
		pDot11f->advCodingCap = psessionEntry->htConfig.ht_rx_ldpc;
		pDot11f->supportedChannelWidthSet =
			psessionEntry->htSupportedChannelWidthSet;
		pDot11f->txSTBC = psessionEntry->htConfig.ht_tx_stbc;
		pDot11f->rxSTBC = psessionEntry->htConfig.ht_rx_stbc;
		pDot11f->shortGI20MHz = psessionEntry->htConfig.ht_sgi20;
		pDot11f->shortGI40MHz = psessionEntry->htConfig.ht_sgi40;
	}

	/* Ensure that shortGI40MHz is Disabled if supportedChannelWidthSet is
	   eHT_CHANNEL_WIDTH_20MHZ */
	if (pDot11f->supportedChannelWidthSet == eHT_CHANNEL_WIDTH_20MHZ) {
		pDot11f->shortGI40MHz = 0;
	}

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_AMPDU_PARAMS, nCfgValue);

	nCfgValue8 = (uint8_t) nCfgValue;
	pHTParametersInfo = (tSirMacHTParametersInfo *) &nCfgValue8;

	pDot11f->maxRxAMPDUFactor = pHTParametersInfo->maxRxAMPDUFactor;
	pDot11f->mpduDensity = pHTParametersInfo->mpduDensity;
	pDot11f->reserved1 = pHTParametersInfo->reserved;

	CFG_GET_STR(nSirStatus, pMac, WNI_CFG_SUPPORTED_MCS_SET,
		    pDot11f->supportedMCSSet, nCfgLen,
		    SIZE_OF_SUPPORTED_MCS_SET);

	if (psessionEntry) {
		disable_high_ht_mcs_2x2 =
				pMac->roam.configParam.disable_high_ht_mcs_2x2;
		pe_debug("disable HT high MCS INI param[%d]",
			 disable_high_ht_mcs_2x2);
		if (psessionEntry->nss == NSS_1x1_MODE) {
			pDot11f->supportedMCSSet[1] = 0;
		} else if (IS_24G_CH(psessionEntry->currentOperChannel) &&
			   disable_high_ht_mcs_2x2 &&
			   (psessionEntry->pePersona == QDF_STA_MODE)) {
				pe_debug("Disabling high HT MCS [%d]",
					 disable_high_ht_mcs_2x2);
				pDot11f->supportedMCSSet[1] =
					(pDot11f->supportedMCSSet[1] >>
						disable_high_ht_mcs_2x2);
		}
	}

	/* If STA mode, session supported NSS > 1 and
	 * SMPS enabled publish HT SMPS IE
	 */
	if (psessionEntry &&
	    LIM_IS_STA_ROLE(psessionEntry) &&
	    (psessionEntry->enableHtSmps) &&
	    (!psessionEntry->supported_nss_1x1)) {
		pe_debug("Add SM power save IE: %d",
			psessionEntry->htSmpsvalue);
		pDot11f->mimoPowerSave = psessionEntry->htSmpsvalue;
	}

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_EXT_HT_CAP_INFO, nCfgValue);

	uHTCapabilityInfo.nCfgValue16 = nCfgValue & 0xFFFF;

	pDot11f->pco = uHTCapabilityInfo.extHtCapInfo.pco;
	pDot11f->transitionTime = uHTCapabilityInfo.extHtCapInfo.transitionTime;
	pDot11f->mcsFeedback = uHTCapabilityInfo.extHtCapInfo.mcsFeedback;

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_TX_BF_CAP, nCfgValue);

	pTxBFCapabilityInfo = (tSirMacTxBFCapabilityInfo *) &nCfgValue;
	pDot11f->txBF = pTxBFCapabilityInfo->txBF;
	pDot11f->rxStaggeredSounding = pTxBFCapabilityInfo->rxStaggeredSounding;
	pDot11f->txStaggeredSounding = pTxBFCapabilityInfo->txStaggeredSounding;
	pDot11f->rxZLF = pTxBFCapabilityInfo->rxZLF;
	pDot11f->txZLF = pTxBFCapabilityInfo->txZLF;
	pDot11f->implicitTxBF = pTxBFCapabilityInfo->implicitTxBF;
	pDot11f->calibration = pTxBFCapabilityInfo->calibration;
	pDot11f->explicitCSITxBF = pTxBFCapabilityInfo->explicitCSITxBF;
	pDot11f->explicitUncompressedSteeringMatrix =
		pTxBFCapabilityInfo->explicitUncompressedSteeringMatrix;
	pDot11f->explicitBFCSIFeedback =
		pTxBFCapabilityInfo->explicitBFCSIFeedback;
	pDot11f->explicitUncompressedSteeringMatrixFeedback =
		pTxBFCapabilityInfo->explicitUncompressedSteeringMatrixFeedback;
	pDot11f->explicitCompressedSteeringMatrixFeedback =
		pTxBFCapabilityInfo->explicitCompressedSteeringMatrixFeedback;
	pDot11f->csiNumBFAntennae = pTxBFCapabilityInfo->csiNumBFAntennae;
	pDot11f->uncompressedSteeringMatrixBFAntennae =
		pTxBFCapabilityInfo->uncompressedSteeringMatrixBFAntennae;
	pDot11f->compressedSteeringMatrixBFAntennae =
		pTxBFCapabilityInfo->compressedSteeringMatrixBFAntennae;

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_AS_CAP, nCfgValue);

	nCfgValue8 = (uint8_t) nCfgValue;

	pASCapabilityInfo = (tSirMacASCapabilityInfo *) &nCfgValue8;
	pDot11f->antennaSelection = pASCapabilityInfo->antennaSelection;
	pDot11f->explicitCSIFeedbackTx =
		pASCapabilityInfo->explicitCSIFeedbackTx;
	pDot11f->antennaIndicesFeedbackTx =
		pASCapabilityInfo->antennaIndicesFeedbackTx;
	pDot11f->explicitCSIFeedback = pASCapabilityInfo->explicitCSIFeedback;
	pDot11f->antennaIndicesFeedback =
		pASCapabilityInfo->antennaIndicesFeedback;
	pDot11f->rxAS = pASCapabilityInfo->rxAS;
	pDot11f->txSoundingPPDUs = pASCapabilityInfo->txSoundingPPDUs;

	pDot11f->present = 1;

	return eSIR_SUCCESS;

} /* End populate_dot11f_ht_caps. */

void lim_log_vht_cap(tpAniSirGlobal pMac, tDot11fIEVHTCaps *pDot11f)
{
#ifdef DUMP_MGMT_CNTNTS
	pe_debug("maxMPDULen (2): %d", pDot11f->maxMPDULen);
	pe_debug("supportedChannelWidthSet (2): %d",
		pDot11f->supportedChannelWidthSet);
	pe_debug("ldpcCodingCap (1): %d",
		pDot11f->ldpcCodingCap);
	pe_debug("shortGI80MHz (1): %d", pDot11f->shortGI80MHz);
	pe_debug("shortGI160and80plus80MHz (1): %d",
		pDot11f->shortGI160and80plus80MHz);
	pe_debug("txSTBC (1): %d", pDot11f->txSTBC);
	pe_debug("rxSTBC (3): %d", pDot11f->rxSTBC);
	pe_debug("suBeamFormerCap (1): %d",
		pDot11f->suBeamFormerCap);
	pe_debug("suBeamformeeCap (1): %d",
		pDot11f->suBeamformeeCap);
	pe_debug("csnofBeamformerAntSup (3): %d",
		pDot11f->csnofBeamformerAntSup);
	pe_debug("numSoundingDim (3): %d",
		pDot11f->numSoundingDim);
	pe_debug("muBeamformerCap (1): %d",
		pDot11f->muBeamformerCap);
	pe_debug("muBeamformeeCap (1): %d",
		pDot11f->muBeamformeeCap);
	pe_debug("vhtTXOPPS (1): %d", pDot11f->vhtTXOPPS);
	pe_debug("htcVHTCap (1): %d", pDot11f->htcVHTCap);
	pe_debug("maxAMPDULenExp (3): %d",
		pDot11f->maxAMPDULenExp);
	pe_debug("vhtLinkAdaptCap (2): %d",
		pDot11f->vhtLinkAdaptCap);
	pe_debug("rxAntPattern (1): %d",
		pDot11f->rxAntPattern;
	pe_debug("txAntPattern (1): %d",
		pDot11f->txAntPattern);
	pe_debug("reserved1 (2): %d", pDot11f->reserved1);
	pe_debug("rxMCSMap (16): %d", pDot11f->rxMCSMap);
	pe_debug("rxHighSupDataRate (13): %d",
		pDot11f->rxHighSupDataRate);
	pe_debug("reserved2(3): %d", pDot11f->reserved2);
	pe_debug("txMCSMap (16): %d", pDot11f->txMCSMap);
	pe_debug("txSupDataRate (13): %d"),
		pDot11f->txSupDataRate;
	pe_debug("reserved3 (3): %d", pDot11f->reserved3);
#endif /* DUMP_MGMT_CNTNTS */
}

static void lim_log_vht_operation(tpAniSirGlobal pMac,
				  tDot11fIEVHTOperation *pDot11f)
{
#ifdef DUMP_MGMT_CNTNTS
	pe_debug("chanWidth: %d", pDot11f->chanWidth);
	pe_debug("chanCenterFreqSeg1: %d",
		pDot11f->chanCenterFreqSeg1);
	pe_debug("chanCenterFreqSeg2: %d",
		pDot11f->chanCenterFreqSeg2);
	pe_debug("basicMCSSet: %d", pDot11f->basicMCSSet);
#endif /* DUMP_MGMT_CNTNTS */
}

static void lim_log_vht_ext_bss_load(tpAniSirGlobal pMac,
				     tDot11fIEVHTExtBssLoad *pDot11f)
{
#ifdef DUMP_MGMT_CNTNTS
	pe_debug("muMIMOCapStaCount: %d",
		pDot11f->muMIMOCapStaCount);
	pe_debug("ssUnderUtil: %d", pDot11f->ssUnderUtil);
	pe_debug("FortyMHzUtil: %d", pDot11f->FortyMHzUtil);
	pe_debug("EightyMHzUtil: %d", pDot11f->EightyMHzUtil);
	pe_debug("OneSixtyMHzUtil: %d",
		pDot11f->OneSixtyMHzUtil);
#endif /* DUMP_MGMT_CNTNTS */
}

static void lim_log_operating_mode(tpAniSirGlobal pMac,
				   tDot11fIEOperatingMode *pDot11f)
{
#ifdef DUMP_MGMT_CNTNTS
	pe_debug("ChanWidth: %d", pDot11f->chanWidth);
	pe_debug("reserved: %d", pDot11f->reserved);
	pe_debug("rxNSS: %d", pDot11f->rxNSS);
	pe_debug("rxNSS Type: %d", pDot11f->rxNSSType);
#endif /* DUMP_MGMT_CNTNTS */
}

static void lim_log_qos_map_set(tpAniSirGlobal pMac, tSirQosMapSet *pQosMapSet)
{
	uint8_t i;

	if (pQosMapSet->num_dscp_exceptions > QOS_MAP_MAX_EX)
		pQosMapSet->num_dscp_exceptions = QOS_MAP_MAX_EX;

	pe_debug("num of dscp exceptions: %d",
		pQosMapSet->num_dscp_exceptions);
	for (i = 0; i < pQosMapSet->num_dscp_exceptions; i++) {
		pe_debug("dscp value: %d",
			pQosMapSet->dscp_exceptions[i][0]);
		pe_debug("User priority value: %d",
			pQosMapSet->dscp_exceptions[i][1]);
	}
	for (i = 0; i < 8; i++) {
		pe_debug("dscp low for up %d: %d", i,
			pQosMapSet->dscp_range[i][0]);
		pe_debug("dscp high for up %d: %d", i,
			pQosMapSet->dscp_range[i][1]);
	}
}

tSirRetStatus
populate_dot11f_vht_caps(tpAniSirGlobal pMac,
			 tpPESession psessionEntry, tDot11fIEVHTCaps *pDot11f)
{
	tSirRetStatus nStatus;
	uint32_t nCfgValue = 0;

	pDot11f->present = 1;

	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_MAX_MPDU_LENGTH, nCfgValue);
	pDot11f->maxMPDULen = (nCfgValue & 0x0003);

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_SUPPORTED_CHAN_WIDTH_SET,
		    nCfgValue);
	pDot11f->supportedChannelWidthSet = (nCfgValue & 0x0003);

	nCfgValue = 0;
	/* With VHT it suffices if we just examine HT */
	if (psessionEntry) {
		if (psessionEntry->htConfig.ht_rx_ldpc)
			pDot11f->ldpcCodingCap =
				psessionEntry->vht_config.ldpc_coding;
		if (psessionEntry->ch_width < CH_WIDTH_80MHZ) {
			 pDot11f->shortGI80MHz = 0;
		} else {
			pDot11f->shortGI80MHz =
				psessionEntry->vht_config.shortgi80;
		}

		if (psessionEntry->ch_width < CH_WIDTH_160MHZ) {
			pDot11f->shortGI160and80plus80MHz = 0;
			pDot11f->supportedChannelWidthSet = 0;
		} else {
			pDot11f->shortGI160and80plus80MHz =
				psessionEntry->vht_config.shortgi160and80plus80;
		}

		if (psessionEntry->htConfig.ht_tx_stbc)
			pDot11f->txSTBC = psessionEntry->vht_config.tx_stbc;

		if (psessionEntry->htConfig.ht_rx_stbc)
			pDot11f->rxSTBC = psessionEntry->vht_config.rx_stbc;

		pDot11f->suBeamformeeCap =
			psessionEntry->vht_config.su_beam_formee;
		if (psessionEntry->vht_config.su_beam_formee) {
			pDot11f->muBeamformeeCap =
				psessionEntry->vht_config.mu_beam_formee;
			pDot11f->csnofBeamformerAntSup =
			      psessionEntry->vht_config.csnof_beamformer_antSup;
		} else {
			pDot11f->muBeamformeeCap = 0;
		}
		pDot11f->suBeamFormerCap =
			psessionEntry->vht_config.su_beam_former;

		pDot11f->vhtTXOPPS = psessionEntry->vht_config.vht_txops;

		pDot11f->numSoundingDim =
				psessionEntry->vht_config.num_soundingdim;

		pDot11f->htcVHTCap = psessionEntry->vht_config.htc_vhtcap;

		pDot11f->rxAntPattern = psessionEntry->vht_config.rx_antpattern;

		pDot11f->txAntPattern = psessionEntry->vht_config.tx_antpattern;

		pDot11f->maxAMPDULenExp =
				psessionEntry->vht_config.max_ampdu_lenexp;

		pDot11f->vhtLinkAdaptCap =
				psessionEntry->vht_config.vht_link_adapt;
	} else {
		CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_LDPC_CODING_CAP,
			    nCfgValue);
		pDot11f->ldpcCodingCap = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_SHORT_GI_80MHZ,
			    nCfgValue);
		pDot11f->shortGI80MHz = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
			    WNI_CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ,
			    nCfgValue);
		pDot11f->shortGI160and80plus80MHz = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_TXSTBC, nCfgValue);
		pDot11f->txSTBC = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_RXSTBC, nCfgValue);
		pDot11f->rxSTBC = (nCfgValue & 0x0007);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
			    WNI_CFG_VHT_SU_BEAMFORMEE_CAP, nCfgValue);
		pDot11f->suBeamformeeCap = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
			    WNI_CFG_VHT_MU_BEAMFORMEE_CAP, nCfgValue);
		pDot11f->muBeamformeeCap = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_SU_BEAMFORMER_CAP,
				nCfgValue);
		pDot11f->suBeamFormerCap = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
				nCfgValue);
		pDot11f->csnofBeamformerAntSup = (nCfgValue & 0x0007);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_TXOP_PS,
				nCfgValue);
		pDot11f->vhtTXOPPS = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_NUM_SOUNDING_DIMENSIONS,
				nCfgValue);
		pDot11f->numSoundingDim = (nCfgValue & 0x0007);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_HTC_VHTC_CAP,
				nCfgValue);
		pDot11f->htcVHTCap = (nCfgValue & 0x0001);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_RX_ANT_PATTERN,
				nCfgValue);
		pDot11f->rxAntPattern = nCfgValue;

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_TX_ANT_PATTERN,
				nCfgValue);
		pDot11f->txAntPattern = nCfgValue;

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
				WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
				nCfgValue);
		pDot11f->maxAMPDULenExp = (nCfgValue & 0x0007);

		nCfgValue = 0;
		CFG_GET_INT(nStatus, pMac,
			WNI_CFG_VHT_LINK_ADAPTATION_CAP,
			nCfgValue);
		pDot11f->vhtLinkAdaptCap = (nCfgValue & 0x0003);

	}

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_MU_BEAMFORMER_CAP, nCfgValue);
	pDot11f->muBeamformerCap = (nCfgValue & 0x0001);

	pDot11f->reserved1 = 0;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_RX_MCS_MAP, nCfgValue);
	pDot11f->rxMCSMap = (nCfgValue & 0x0000FFFF);

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE,
		    nCfgValue);
	pDot11f->rxHighSupDataRate = (nCfgValue & 0x00001FFF);

	pDot11f->reserved2 = 0;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_TX_MCS_MAP, nCfgValue);
	pDot11f->txMCSMap = (nCfgValue & 0x0000FFFF);

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE,
			nCfgValue);
	pDot11f->txSupDataRate = (nCfgValue & 0x00001FFF);

	pDot11f->reserved3 = 0;
	if (psessionEntry) {
		if (psessionEntry->nss == NSS_1x1_MODE) {
			pDot11f->txMCSMap |= DISABLE_NSS2_MCS;
			pDot11f->rxMCSMap |= DISABLE_NSS2_MCS;
			pDot11f->txSupDataRate =
				VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
			pDot11f->rxHighSupDataRate =
				VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
		}
	}
	lim_log_vht_cap(pMac, pDot11f);
	return eSIR_SUCCESS;
}

tSirRetStatus
populate_dot11f_vht_operation(tpAniSirGlobal pMac,
			      tpPESession psessionEntry,
			      tDot11fIEVHTOperation *pDot11f)
{
	tSirRetStatus nStatus;
	uint32_t nCfgValue = 0;

	pDot11f->present = 1;

	if (psessionEntry->ch_width > CH_WIDTH_40MHZ) {
		pDot11f->chanWidth = 1;
		pDot11f->chanCenterFreqSeg1 =
			psessionEntry->ch_center_freq_seg0;
		if (psessionEntry->ch_width == CH_WIDTH_80P80MHZ ||
				psessionEntry->ch_width == CH_WIDTH_160MHZ)
			pDot11f->chanCenterFreqSeg2 =
				psessionEntry->ch_center_freq_seg1;
		else
			pDot11f->chanCenterFreqSeg2 = 0;
	} else {
		pDot11f->chanWidth = 0;
		pDot11f->chanCenterFreqSeg1 = 0;
		pDot11f->chanCenterFreqSeg2 = 0;
	}

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_BASIC_MCS_SET, nCfgValue);
	pDot11f->basicMCSSet = (uint16_t) nCfgValue;

	lim_log_vht_operation(pMac, pDot11f);

	return eSIR_SUCCESS;

}

tSirRetStatus
populate_dot11f_vht_ext_bss_load(tpAniSirGlobal pMac,
				 tDot11fIEVHTExtBssLoad *pDot11f)
{
	tSirRetStatus nStatus;
	uint32_t nCfgValue = 0;

	pDot11f->present = 1;

	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_MU_MIMO_CAP_STA_COUNT,
		    nCfgValue);
	pDot11f->muMIMOCapStaCount = (uint8_t) nCfgValue;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_SS_UNDER_UTIL, nCfgValue);
	pDot11f->ssUnderUtil = (uint8_t) nCfgValue;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_40MHZ_UTILIZATION, nCfgValue);
	pDot11f->FortyMHzUtil = (uint8_t) nCfgValue;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_80MHZ_UTILIZATION, nCfgValue);
	pDot11f->EightyMHzUtil = (uint8_t) nCfgValue;

	nCfgValue = 0;
	CFG_GET_INT(nStatus, pMac, WNI_CFG_VHT_160MHZ_UTILIZATION, nCfgValue);
	pDot11f->EightyMHzUtil = (uint8_t) nCfgValue;

	lim_log_vht_ext_bss_load(pMac, pDot11f);

	return eSIR_SUCCESS;
}

tSirRetStatus
populate_dot11f_ext_cap(tpAniSirGlobal pMac,
			bool isVHTEnabled, tDot11fIEExtCap *pDot11f,
			tpPESession psessionEntry)
{
	uint32_t val = 0;
	struct s_ext_cap *p_ext_cap;

	pDot11f->present = 1;

	if (!psessionEntry) {
		pe_debug("11MC - enabled for non-SAP cases");
		pDot11f->num_bytes = DOT11F_IE_EXTCAP_MAX_LEN;
	} else if (psessionEntry->sap_dot11mc) {
		pe_debug("11MC support enabled");
		pDot11f->num_bytes = DOT11F_IE_EXTCAP_MAX_LEN;
	} else {
		if (eLIM_AP_ROLE != psessionEntry->limSystemRole) {
			pe_debug("11MC support enabled");
			pDot11f->num_bytes = DOT11F_IE_EXTCAP_MAX_LEN;
		} else  {
			pe_debug("11MC support disabled");
			pDot11f->num_bytes = DOT11F_IE_EXTCAP_MIN_LEN;
		}
	}

	p_ext_cap = (struct s_ext_cap *)pDot11f->bytes;
	if (isVHTEnabled == true)
		p_ext_cap->oper_mode_notification = 1;

	if (wlan_cfg_get_int(pMac, WNI_CFG_RTT3_ENABLE, &val) != eSIR_SUCCESS) {
		pe_err("could not retrieve RTT3 Variable from DAT File");
		return eSIR_FAILURE;
	}

	if (val) {
		uint32_t ftm = ucfg_wifi_pos_get_ftm_cap(pMac->psoc);
		if (!psessionEntry || LIM_IS_STA_ROLE(psessionEntry)) {
			p_ext_cap->fine_time_meas_initiator =
				(ftm & WMI_FW_STA_RTT_INITR) ? 1 : 0;
			p_ext_cap->fine_time_meas_responder =
				(ftm & WMI_FW_STA_RTT_RESPR) ? 1 : 0;
		} else if (LIM_IS_AP_ROLE(psessionEntry)) {
			p_ext_cap->fine_time_meas_initiator =
				(ftm & WMI_FW_AP_RTT_INITR) ? 1 : 0;
			p_ext_cap->fine_time_meas_responder =
				(ftm & WMI_FW_AP_RTT_RESPR) ? 1 : 0;
		}
	}
#ifdef QCA_HT_2040_COEX
	if (pMac->roam.configParam.obssEnabled)
		p_ext_cap->bss_coexist_mgmt_support = 1;
#endif
	p_ext_cap->ext_chan_switch = 1;

	if (psessionEntry && psessionEntry->enable_bcast_probe_rsp)
		p_ext_cap->fils_capability = 1;

	/* Need to calulate the num_bytes based on bits set */
	if (pDot11f->present)
		pDot11f->num_bytes = lim_compute_ext_cap_ie_length(pDot11f);

	return eSIR_SUCCESS;
}

void populate_dot11f_qcn_ie(tDot11fIEQCN_IE *pDot11f)
{
	pDot11f->present = 1;
	pDot11f->version[0] = QCN_IE_VERSION_SUBATTR_ID;
	pDot11f->version[1] = QCN_IE_VERSION_SUBATTR_DATA_LEN;
	pDot11f->version[2] = QCN_IE_VERSION_SUPPORTED;
	pDot11f->version[3] = QCN_IE_SUBVERSION_SUPPORTED;
}

tSirRetStatus
populate_dot11f_operating_mode(tpAniSirGlobal pMac,
			       tDot11fIEOperatingMode *pDot11f,
			       tpPESession psessionEntry)
{
	pDot11f->present = 1;

	pDot11f->chanWidth = psessionEntry->gLimOperatingMode.chanWidth;
	pDot11f->rxNSS = psessionEntry->gLimOperatingMode.rxNSS;
	pDot11f->rxNSSType = psessionEntry->gLimOperatingMode.rxNSSType;

	return eSIR_SUCCESS;
}

tSirRetStatus
populate_dot11f_ht_info(tpAniSirGlobal pMac,
			tDot11fIEHTInfo *pDot11f, tpPESession psessionEntry)
{
	uint32_t nCfgValue, nCfgLen;
	uint8_t htInfoField1;
	uint16_t htInfoField2;
	tSirRetStatus nSirStatus;
	tSirMacHTInfoField1 *pHTInfoField1;
	tSirMacHTInfoField2 *pHTInfoField2;
	union {
		uint16_t nCfgValue16;
		tSirMacHTInfoField3 infoField3;
	} uHTInfoField;
	union {
		uint16_t nCfgValue16;
		tSirMacHTInfoField2 infoField2;
	} uHTInfoField2 = {
		0
	};

	if (NULL == psessionEntry) {
		pe_err("Invalid session entry in populate_dot11f_ht_info()");
		return eSIR_FAILURE;
	}

	pDot11f->primaryChannel = psessionEntry->currentOperChannel;

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_INFO_FIELD1, nCfgValue);

	htInfoField1 = (uint8_t) nCfgValue;

	pHTInfoField1 = (tSirMacHTInfoField1 *) &htInfoField1;
	pHTInfoField1->rifsMode = psessionEntry->beaconParams.fRIFSMode;
	pHTInfoField1->serviceIntervalGranularity =
		pMac->lim.gHTServiceIntervalGranularity;

		pHTInfoField1->secondaryChannelOffset =
			psessionEntry->htSecondaryChannelOffset;
		pHTInfoField1->recommendedTxWidthSet =
			psessionEntry->htRecommendedTxWidthSet;

	if ((psessionEntry) && LIM_IS_AP_ROLE(psessionEntry)) {
		CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_INFO_FIELD2,
			    nCfgValue);

		uHTInfoField2.nCfgValue16 = nCfgValue & 0xFFFF; /* this is added for fixing CRs on MDM9K platform - 257951, 259577 */

		uHTInfoField2.infoField2.opMode = psessionEntry->htOperMode;
		uHTInfoField2.infoField2.nonGFDevicesPresent =
			psessionEntry->beaconParams.llnNonGFCoexist;
		uHTInfoField2.infoField2.obssNonHTStaPresent = psessionEntry->beaconParams.gHTObssMode; /*added for Obss  */

		uHTInfoField2.infoField2.reserved = 0;

	} else {
		CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_INFO_FIELD2,
			    nCfgValue);

		htInfoField2 = (uint16_t) nCfgValue;

		pHTInfoField2 = (tSirMacHTInfoField2 *) &htInfoField2;
		pHTInfoField2->opMode = pMac->lim.gHTOperMode;
		pHTInfoField2->nonGFDevicesPresent =
			pMac->lim.gHTNonGFDevicesPresent;
		pHTInfoField2->obssNonHTStaPresent = pMac->lim.gHTObssMode;     /*added for Obss  */

		pHTInfoField2->reserved = 0;
	}

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_HT_INFO_FIELD3, nCfgValue);

	uHTInfoField.nCfgValue16 = nCfgValue & 0xFFFF;

	uHTInfoField.infoField3.basicSTBCMCS = pMac->lim.gHTSTBCBasicMCS;
	uHTInfoField.infoField3.dualCTSProtection =
		pMac->lim.gHTDualCTSProtection;
	uHTInfoField.infoField3.secondaryBeacon = pMac->lim.gHTSecondaryBeacon;
	uHTInfoField.infoField3.lsigTXOPProtectionFullSupport =
		psessionEntry->beaconParams.fLsigTXOPProtectionFullSupport;
	uHTInfoField.infoField3.pcoActive = pMac->lim.gHTPCOActive;
	uHTInfoField.infoField3.pcoPhase = pMac->lim.gHTPCOPhase;
	uHTInfoField.infoField3.reserved = 0;

	pDot11f->secondaryChannelOffset = pHTInfoField1->secondaryChannelOffset;
	pDot11f->recommendedTxWidthSet = pHTInfoField1->recommendedTxWidthSet;
	pDot11f->rifsMode = pHTInfoField1->rifsMode;
	pDot11f->controlledAccessOnly = pHTInfoField1->controlledAccessOnly;
	pDot11f->serviceIntervalGranularity =
		pHTInfoField1->serviceIntervalGranularity;

	pDot11f->opMode = uHTInfoField2.infoField2.opMode;
	pDot11f->nonGFDevicesPresent =
		uHTInfoField2.infoField2.nonGFDevicesPresent;
	pDot11f->obssNonHTStaPresent =
		uHTInfoField2.infoField2.obssNonHTStaPresent;
	pDot11f->reserved = uHTInfoField2.infoField2.reserved;

	pDot11f->basicSTBCMCS = uHTInfoField.infoField3.basicSTBCMCS;
	pDot11f->dualCTSProtection = uHTInfoField.infoField3.dualCTSProtection;
	pDot11f->secondaryBeacon = uHTInfoField.infoField3.secondaryBeacon;
	pDot11f->lsigTXOPProtectionFullSupport =
		uHTInfoField.infoField3.lsigTXOPProtectionFullSupport;
	pDot11f->pcoActive = uHTInfoField.infoField3.pcoActive;
	pDot11f->pcoPhase = uHTInfoField.infoField3.pcoPhase;
	pDot11f->reserved2 = uHTInfoField.infoField3.reserved;
	CFG_GET_STR(nSirStatus, pMac, WNI_CFG_BASIC_MCS_SET,
		    pDot11f->basicMCSSet, nCfgLen, SIZE_OF_BASIC_MCS_SET);

	pDot11f->present = 1;

	return eSIR_SUCCESS;

} /* End populate_dot11f_ht_info. */

void
populate_dot11f_ibss_params(tpAniSirGlobal pMac,
			    tDot11fIEIBSSParams *pDot11f,
			    tpPESession psessionEntry)
{
	uint32_t val = 0;

	if (LIM_IS_IBSS_ROLE(psessionEntry)) {
		if (wlan_cfg_get_int(pMac,
				     WNI_CFG_IBSS_ATIM_WIN_SIZE,
				     &val) != eSIR_SUCCESS) {
			pe_err("could not retrieve IBSS ATIM WIN size");
		}
		pDot11f->present = 1;
		/* ATIM duration is always set to 0 */
		pDot11f->atim = val;
	}

} /* End populate_dot11f_ibss_params. */

#ifdef ANI_SUPPORT_11H
tSirRetStatus
populate_dot11f_measurement_report0(tpAniSirGlobal pMac,
				    tpSirMacMeasReqActionFrame pReq,
				    tDot11fIEMeasurementReport *pDot11f)
{
	pDot11f->token = pReq->measReqIE.measToken;
	pDot11f->late = 0;
	pDot11f->incapable = 0;
	pDot11f->refused = 1;
	pDot11f->type = SIR_MAC_BASIC_MEASUREMENT_TYPE;

	pDot11f->present = 1;

	return eSIR_SUCCESS;

} /* End PopulatedDot11fMeasurementReport0. */
tSirRetStatus
populate_dot11f_measurement_report1(tpAniSirGlobal pMac,
				    tpSirMacMeasReqActionFrame pReq,
				    tDot11fIEMeasurementReport *pDot11f)
{
	pDot11f->token = pReq->measReqIE.measToken;
	pDot11f->late = 0;
	pDot11f->incapable = 0;
	pDot11f->refused = 1;
	pDot11f->type = SIR_MAC_CCA_MEASUREMENT_TYPE;
	pDot11f->present = 1;
	return eSIR_SUCCESS;
} /* End PopulatedDot11fMeasurementReport1. */
tSirRetStatus
populate_dot11f_measurement_report2(tpAniSirGlobal pMac,
				    tpSirMacMeasReqActionFrame pReq,
				    tDot11fIEMeasurementReport *pDot11f)
{
	pDot11f->token = pReq->measReqIE.measToken;
	pDot11f->late = 0;
	pDot11f->incapable = 0;
	pDot11f->refused = 1;
	pDot11f->type = SIR_MAC_RPI_MEASUREMENT_TYPE;
	pDot11f->present = 1;
	return eSIR_SUCCESS;
} /* End PopulatedDot11fMeasurementReport2. */
#endif

void
populate_dot11f_power_caps(tpAniSirGlobal pMac,
			   tDot11fIEPowerCaps *pCaps,
			   uint8_t nAssocType, tpPESession psessionEntry)
{
	if (nAssocType == LIM_REASSOC) {
		pCaps->minTxPower =
			psessionEntry->pLimReAssocReq->powerCap.minTxPower;
		pCaps->maxTxPower =
			psessionEntry->pLimReAssocReq->powerCap.maxTxPower;
	} else {
		pCaps->minTxPower =
			psessionEntry->pLimJoinReq->powerCap.minTxPower;
		pCaps->maxTxPower =
			psessionEntry->pLimJoinReq->powerCap.maxTxPower;

	}

	pCaps->present = 1;
} /* End populate_dot11f_power_caps. */

tSirRetStatus
populate_dot11f_power_constraints(tpAniSirGlobal pMac,
				  tDot11fIEPowerConstraints *pDot11f)
{
	uint32_t cfg;
	tSirRetStatus nSirStatus;

	CFG_GET_INT(nSirStatus, pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, cfg);

	pDot11f->localPowerConstraints = (uint8_t) cfg;
	pDot11f->present = 1;

	return eSIR_SUCCESS;
} /* End populate_dot11f_power_constraints. */

void
populate_dot11f_qos_caps_ap(tpAniSirGlobal pMac,
			    tDot11fIEQOSCapsAp *pDot11f, tpPESession psessionEntry)
{
	pDot11f->count = psessionEntry->gLimEdcaParamSetCount;
	pDot11f->reserved = 0;
	pDot11f->txopreq = 0;
	pDot11f->qreq = 0;
	pDot11f->qack = 0;
	pDot11f->present = 1;
} /* End PopulatedDot11fQOSCaps. */

void
populate_dot11f_qos_caps_station(tpAniSirGlobal pMac, tpPESession pe_session,
				 tDot11fIEQOSCapsStation *pDot11f)
{
	uint32_t val = 0;

	if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_SP_LENGTH, &val) != eSIR_SUCCESS)
		pe_err("could not retrieve Max SP Length");

		pDot11f->more_data_ack = 0;
	pDot11f->max_sp_length = (uint8_t) val;
	pDot11f->qack = 0;

	if (pMac->lim.gUapsdEnable) {
		pDot11f->acbe_uapsd =
			LIM_UAPSD_GET(ACBE, pe_session->gUapsdPerAcBitmask);
		pDot11f->acbk_uapsd =
			LIM_UAPSD_GET(ACBK, pe_session->gUapsdPerAcBitmask);
		pDot11f->acvi_uapsd =
			LIM_UAPSD_GET(ACVI, pe_session->gUapsdPerAcBitmask);
		pDot11f->acvo_uapsd =
			LIM_UAPSD_GET(ACVO, pe_session->gUapsdPerAcBitmask);
	}
	pDot11f->present = 1;
} /* End PopulatedDot11fQOSCaps. */

tSirRetStatus
populate_dot11f_rsn(tpAniSirGlobal pMac,
		    tpSirRSNie pRsnIe, tDot11fIERSN *pDot11f)
{
	uint32_t status;
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_RSN);
		if (0 <= idx) {
			status = dot11f_unpack_ie_rsn(pMac, pRsnIe->rsnIEdata + idx + 2,   /* EID, length */
						      pRsnIe->rsnIEdata[idx + 1],
						      pDot11f, false);
			if (DOT11F_FAILED(status)) {
				pe_err("Parse failure in Populate Dot11fRSN (0x%08x)",
					status);
				return eSIR_FAILURE;
			}
			pe_debug("dot11f_unpack_ie_rsn returned 0x%08x in populate_dot11f_rsn",
				   status);
		}

	}

	return eSIR_SUCCESS;
} /* End populate_dot11f_rsn. */

tSirRetStatus populate_dot11f_rsn_opaque(tpAniSirGlobal pMac,
					 tpSirRSNie pRsnIe,
					 tDot11fIERSNOpaque *pDot11f)
{
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_RSN);
		if (0 <= idx) {
			pDot11f->present = 1;
			pDot11f->num_data = pRsnIe->rsnIEdata[idx + 1];
			qdf_mem_copy(pDot11f->data, pRsnIe->rsnIEdata + idx + 2,        /* EID, len */
				     pRsnIe->rsnIEdata[idx + 1]);
		}
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_rsn_opaque. */

#if defined(FEATURE_WLAN_WAPI)

tSirRetStatus
populate_dot11f_wapi(tpAniSirGlobal pMac,
		     tpSirRSNie pRsnIe, tDot11fIEWAPI *pDot11f)
{
	uint32_t status;
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_WAPI);
		if (0 <= idx) {
			status = dot11f_unpack_ie_wapi(pMac, pRsnIe->rsnIEdata + idx + 2,  /* EID, length */
						       pRsnIe->rsnIEdata[idx + 1],
						       pDot11f, false);
			if (DOT11F_FAILED(status)) {
				pe_err("Parse failure in populate_dot11f_wapi (0x%08x)",
					status);
				return eSIR_FAILURE;
			}
			pe_debug("dot11f_unpack_ie_rsn returned 0x%08x in populate_dot11f_wapi",
				   status);
		}
	}

	return eSIR_SUCCESS;
} /* End populate_dot11f_wapi. */

tSirRetStatus populate_dot11f_wapi_opaque(tpAniSirGlobal pMac,
					  tpSirRSNie pRsnIe,
					  tDot11fIEWAPIOpaque *pDot11f)
{
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_WAPI);
		if (0 <= idx) {
			pDot11f->present = 1;
			pDot11f->num_data = pRsnIe->rsnIEdata[idx + 1];
			qdf_mem_copy(pDot11f->data, pRsnIe->rsnIEdata + idx + 2,        /* EID, len */
				     pRsnIe->rsnIEdata[idx + 1]);
		}
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_wapi_opaque. */

#endif /* defined(FEATURE_WLAN_WAPI) */

void
populate_dot11f_ssid(tpAniSirGlobal pMac,
		     tSirMacSSid *pInternal, tDot11fIESSID *pDot11f)
{
	pDot11f->present = 1;
	pDot11f->num_ssid = pInternal->length;
	if (pInternal->length) {
		qdf_mem_copy((uint8_t *) pDot11f->ssid,
			     (uint8_t *) &pInternal->ssId, pInternal->length);
	}
} /* End populate_dot11f_ssid. */

tSirRetStatus populate_dot11f_ssid2(tpAniSirGlobal pMac, tDot11fIESSID *pDot11f)
{
	uint32_t nCfg;
	tSirRetStatus nSirStatus;

	CFG_GET_STR(nSirStatus, pMac, WNI_CFG_SSID, pDot11f->ssid, nCfg, 32);
	pDot11f->num_ssid = (uint8_t) nCfg;
	pDot11f->present = 1;
	return eSIR_SUCCESS;
} /* End populate_dot11f_ssid2. */

void
populate_dot11f_schedule(tSirMacScheduleIE *pSchedule,
			 tDot11fIESchedule *pDot11f)
{
	pDot11f->aggregation = pSchedule->info.aggregation;
	pDot11f->tsid = pSchedule->info.tsid;
	pDot11f->direction = pSchedule->info.direction;
	pDot11f->reserved = pSchedule->info.rsvd;
	pDot11f->service_start_time = pSchedule->svcStartTime;
	pDot11f->service_interval = pSchedule->svcInterval;
	pDot11f->max_service_dur = pSchedule->maxSvcDuration;
	pDot11f->spec_interval = pSchedule->specInterval;

	pDot11f->present = 1;
} /* End populate_dot11f_schedule. */

void
populate_dot11f_supp_channels(tpAniSirGlobal pMac,
			      tDot11fIESuppChannels *pDot11f,
			      uint8_t nAssocType, tpPESession psessionEntry)
{
	uint8_t i;
	uint8_t *p;

	if (nAssocType == LIM_REASSOC) {
		p = (uint8_t *) psessionEntry->pLimReAssocReq->
		    supportedChannels.channelList;
		pDot11f->num_bands =
			psessionEntry->pLimReAssocReq->supportedChannels.numChnl;
	} else {
		p = (uint8_t *) psessionEntry->pLimJoinReq->supportedChannels.
		    channelList;
		pDot11f->num_bands =
			psessionEntry->pLimJoinReq->supportedChannels.numChnl;
	}
	for (i = 0U; i < pDot11f->num_bands; ++i, ++p) {
		pDot11f->bands[i][0] = *p;
		pDot11f->bands[i][1] = 1;
	}

	pDot11f->present = 1;

} /* End populate_dot11f_supp_channels. */

tSirRetStatus
populate_dot11f_supp_rates(tpAniSirGlobal pMac,
			   uint8_t nChannelNum,
			   tDot11fIESuppRates *pDot11f, tpPESession psessionEntry)
{
	tSirRetStatus nSirStatus;
	uint32_t nRates;
	uint8_t rates[SIR_MAC_MAX_NUMBER_OF_RATES];

	/* Use the operational rates present in session entry whenever nChannelNum is set to OPERATIONAL
	   else use the supported rate set from CFG, which is fixed and does not change dynamically and is used for
	   sending mgmt frames (lile probe req) which need to go out before any session is present.
	 */
	if (POPULATE_DOT11F_RATES_OPERATIONAL == nChannelNum) {
#if 0
		CFG_GET_STR(nSirStatus, pMac, WNI_CFG_OPERATIONAL_RATE_SET,
			    rates, nRates, SIR_MAC_MAX_NUMBER_OF_RATES);
#endif /* TO SUPPORT BT-AMP */
		if (psessionEntry != NULL) {
			nRates = psessionEntry->rateSet.numRates;
			qdf_mem_copy(rates, psessionEntry->rateSet.rate,
				     nRates);
		} else {
			pe_err("no session context exists while populating Operational Rate Set");
			nRates = 0;
		}
	} else if (14 >= nChannelNum) {
		CFG_GET_STR(nSirStatus, pMac, WNI_CFG_SUPPORTED_RATES_11B,
			    rates, nRates, SIR_MAC_MAX_NUMBER_OF_RATES);
	} else {
		CFG_GET_STR(nSirStatus, pMac, WNI_CFG_SUPPORTED_RATES_11A,
			    rates, nRates, SIR_MAC_MAX_NUMBER_OF_RATES);
	}

	if (0 != nRates) {
		pDot11f->num_rates = (uint8_t) nRates;
		qdf_mem_copy(pDot11f->rates, rates, nRates);
		pDot11f->present = 1;
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_supp_rates. */

/**
 * populate_dot11f_rates_tdls() - populate supported rates and
 *                                extended supported rates IE.
 * @p_mac gloabl - header.
 * @p_supp_rates - pointer to supported rates IE
 * @p_ext_supp_rates - pointer to extended supported rates IE
 * @curr_oper_channel - current operating channel
 *
 * This function populates the supported rates and extended supported
 * rates IE based in the STA capability. If the number of rates
 * supported is less than MAX_NUM_SUPPORTED_RATES, only supported rates
 * IE is populated.
 *
 * Return: tSirRetStatus eSIR_SUCCESS on Success and eSIR_FAILURE
 *         on failure.
 */

tSirRetStatus
populate_dot11f_rates_tdls(tpAniSirGlobal p_mac,
			   tDot11fIESuppRates *p_supp_rates,
			   tDot11fIEExtSuppRates *p_ext_supp_rates,
			   uint8_t curr_oper_channel)
{
	tSirMacRateSet temp_rateset;
	tSirMacRateSet temp_rateset2;
	uint32_t val, i;
	uint32_t self_dot11mode = 0;

	wlan_cfg_get_int(p_mac, WNI_CFG_DOT11_MODE, &self_dot11mode);

	/**
	 * Include 11b rates only when the device configured in
	 * auto, 11a/b/g or 11b_only and also if current base
	 * channel is 5 GHz then no need to advertise the 11b rates.
	 * If devices move to 2.4GHz off-channel then they can communicate
	 * in 11g rates i.e. (6, 9, 12, 18, 24, 36 and 54).
	 */
	pe_debug("Current operating channel %d self_dot11mode = %d",
		curr_oper_channel, self_dot11mode);

	if ((curr_oper_channel <= SIR_11B_CHANNEL_END) &&
	    ((self_dot11mode == WNI_CFG_DOT11_MODE_ALL) ||
	    (self_dot11mode == WNI_CFG_DOT11_MODE_11A) ||
	    (self_dot11mode == WNI_CFG_DOT11_MODE_11AC) ||
	    (self_dot11mode == WNI_CFG_DOT11_MODE_11N) ||
	    (self_dot11mode == WNI_CFG_DOT11_MODE_11G) ||
	    (self_dot11mode == WNI_CFG_DOT11_MODE_11B))) {
		val = WNI_CFG_SUPPORTED_RATES_11B_LEN;
		wlan_cfg_get_str(p_mac, WNI_CFG_SUPPORTED_RATES_11B,
				(uint8_t *)&temp_rateset.rate, &val);
		temp_rateset.numRates = (uint8_t) val;
	} else {
	    temp_rateset.numRates = 0;
	}

	/* Include 11a rates when the device configured in non-11b mode */
	if (!IS_DOT11_MODE_11B(self_dot11mode)) {
		val = WNI_CFG_SUPPORTED_RATES_11A_LEN;
		wlan_cfg_get_str(p_mac, WNI_CFG_SUPPORTED_RATES_11A,
			(uint8_t *)&temp_rateset2.rate, &val);
		temp_rateset2.numRates = (uint8_t) val;
	} else {
		temp_rateset2.numRates = 0;
	}

	if ((temp_rateset.numRates + temp_rateset2.numRates) >
					SIR_MAC_MAX_NUMBER_OF_RATES) {
		pe_err("more than %d rates in CFG",
				SIR_MAC_MAX_NUMBER_OF_RATES);
		return eSIR_FAILURE;
	}

	/**
	 * copy all rates in temp_rateset,
	 * there are SIR_MAC_MAX_NUMBER_OF_RATES rates max
	 */
	for (i = 0; i < temp_rateset2.numRates; i++)
		temp_rateset.rate[i + temp_rateset.numRates] =
						temp_rateset2.rate[i];

	temp_rateset.numRates += temp_rateset2.numRates;

	if (temp_rateset.numRates <= MAX_NUM_SUPPORTED_RATES) {
		p_supp_rates->num_rates = temp_rateset.numRates;
		qdf_mem_copy(p_supp_rates->rates, temp_rateset.rate,
			     p_supp_rates->num_rates);
		p_supp_rates->present = 1;
	}  else { /* Populate extended capability as well */
		p_supp_rates->num_rates = MAX_NUM_SUPPORTED_RATES;
		qdf_mem_copy(p_supp_rates->rates, temp_rateset.rate,
			     p_supp_rates->num_rates);
		p_supp_rates->present = 1;

		p_ext_supp_rates->num_rates = temp_rateset.numRates -
				     MAX_NUM_SUPPORTED_RATES;
		qdf_mem_copy(p_ext_supp_rates->rates,
			     (uint8_t *)temp_rateset.rate +
			     MAX_NUM_SUPPORTED_RATES,
			     p_ext_supp_rates->num_rates);
		p_ext_supp_rates->present = 1;
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_rates_tdls */


tSirRetStatus
populate_dot11f_tpc_report(tpAniSirGlobal pMac,
			   tDot11fIETPCReport *pDot11f, tpPESession psessionEntry)
{
	uint16_t staid, txPower;
	tSirRetStatus nSirStatus;

	nSirStatus = lim_get_mgmt_staid(pMac, &staid, psessionEntry);
	if (eSIR_SUCCESS != nSirStatus) {
		pe_err("Failed to get the STAID in Populate Dot11fTPCReport; lim_get_mgmt_staid returned status %d",
			nSirStatus);
		return eSIR_FAILURE;
	}
	/* FramesToDo: This function was "misplaced" in the move to Gen4_TVM... */
	/* txPower = halGetRateToPwrValue( pMac, staid, pMac->lim.gLimCurrentChannelId, isBeacon ); */
	txPower = 0;
	pDot11f->tx_power = (uint8_t) txPower;
	pDot11f->link_margin = 0;
	pDot11f->present = 1;

	return eSIR_SUCCESS;
} /* End populate_dot11f_tpc_report. */

void populate_dot11f_ts_info(tSirMacTSInfo *pInfo, tDot11fFfTSInfo *pDot11f)
{
	pDot11f->traffic_type = pInfo->traffic.trafficType;
	pDot11f->tsid = pInfo->traffic.tsid;
	pDot11f->direction = pInfo->traffic.direction;
	pDot11f->access_policy = pInfo->traffic.accessPolicy;
	pDot11f->aggregation = pInfo->traffic.aggregation;
	pDot11f->psb = pInfo->traffic.psb;
	pDot11f->user_priority = pInfo->traffic.userPrio;
	pDot11f->tsinfo_ack_pol = pInfo->traffic.ackPolicy;
	pDot11f->schedule = pInfo->schedule.schedule;
} /* End PopulatedDot11fTSInfo. */

void populate_dot11f_wmm(tpAniSirGlobal pMac,
			 tDot11fIEWMMInfoAp *pInfo,
			 tDot11fIEWMMParams *pParams,
			 tDot11fIEWMMCaps *pCaps, tpPESession psessionEntry)
{
	if (psessionEntry->limWmeEnabled) {
		if (LIM_IS_IBSS_ROLE(psessionEntry)) {
			/* if ( ! sirIsPropCapabilityEnabled( pMac, SIR_MAC_PROP_CAPABILITY_WME ) ) */
			{
				populate_dot11f_wmm_info_ap(pMac, pInfo,
							    psessionEntry);
			}
		} else {
			{
				populate_dot11f_wmm_params(pMac, pParams,
							   psessionEntry);
			}

			if (psessionEntry->limWsmEnabled) {
				populate_dot11f_wmm_caps(pCaps);
			}
		}
	}
} /* End populate_dot11f_wmm. */

void populate_dot11f_wmm_caps(tDot11fIEWMMCaps *pCaps)
{
	pCaps->version = SIR_MAC_OUI_VERSION_1;
	pCaps->qack = 0;
	pCaps->queue_request = 1;
	pCaps->txop_request = 0;
	pCaps->more_ack = 0;
	pCaps->present = 1;
} /* End PopulateDot11fWmmCaps. */

#ifdef FEATURE_WLAN_ESE
void populate_dot11f_re_assoc_tspec(tpAniSirGlobal pMac,
				    tDot11fReAssocRequest *pReassoc,
				    tpPESession psessionEntry)
{
	uint8_t numTspecs = 0, idx;
	tTspecInfo *pTspec = NULL;

	numTspecs = psessionEntry->pLimReAssocReq->eseTspecInfo.numTspecs;
	pTspec = &psessionEntry->pLimReAssocReq->eseTspecInfo.tspec[0];
	pReassoc->num_WMMTSPEC = numTspecs;
	if (numTspecs) {
		for (idx = 0; idx < numTspecs; idx++) {
			populate_dot11f_wmmtspec(&pTspec->tspec,
						 &pReassoc->WMMTSPEC[idx]);
			pTspec->tspec.mediumTime = 0;
			pTspec++;
		}
	}
}

void ese_populate_wmm_tspec(tSirMacTspecIE *source,
	ese_wmm_tspec_ie *dest)
{
	dest->traffic_type = source->tsinfo.traffic.trafficType;
	dest->tsid = source->tsinfo.traffic.tsid;
	dest->direction = source->tsinfo.traffic.direction;
	dest->access_policy = source->tsinfo.traffic.accessPolicy;
	dest->aggregation = source->tsinfo.traffic.aggregation;
	dest->psb = source->tsinfo.traffic.psb;
	dest->user_priority = source->tsinfo.traffic.userPrio;
	dest->tsinfo_ack_pol = source->tsinfo.traffic.ackPolicy;
	dest->burst_size_defn = source->tsinfo.traffic.burstSizeDefn;
	/* As defined in IEEE 802.11-2007, section 7.3.2.30
	 * Nominal MSDU size: Bit[0:14]=Size, Bit[15]=Fixed
	 */
	dest->size = (source->nomMsduSz & SIZE_MASK);
	dest->fixed = (source->nomMsduSz & FIXED_MASK) ? 1 : 0;
	dest->max_msdu_size = source->maxMsduSz;
	dest->min_service_int = source->minSvcInterval;
	dest->max_service_int = source->maxSvcInterval;
	dest->inactivity_int = source->inactInterval;
	dest->suspension_int = source->suspendInterval;
	dest->service_start_time = source->svcStartTime;
	dest->min_data_rate = source->minDataRate;
	dest->mean_data_rate = source->meanDataRate;
	dest->peak_data_rate = source->peakDataRate;
	dest->burst_size = source->maxBurstSz;
	dest->delay_bound = source->delayBound;
	dest->min_phy_rate = source->minPhyRate;
	dest->surplus_bw_allowance = source->surplusBw;
	dest->medium_time = source->mediumTime;
}

#endif

void populate_dot11f_wmm_info_ap(tpAniSirGlobal pMac, tDot11fIEWMMInfoAp *pInfo,
				 tpPESession psessionEntry)
{
	pInfo->version = SIR_MAC_OUI_VERSION_1;

	/* WMM Specification 3.1.3, 3.2.3
	 * An IBSS staion shall always use its default WMM parameters.
	 */
	if (LIM_IS_IBSS_ROLE(psessionEntry)) {
		pInfo->param_set_count = 0;
		pInfo->uapsd = 0;
	} else {
		pInfo->param_set_count =
			(0xf & psessionEntry->gLimEdcaParamSetCount);
		if (LIM_IS_AP_ROLE(psessionEntry)) {
			pInfo->uapsd = (0x1 & psessionEntry->apUapsdEnable);
		} else
			pInfo->uapsd = (0x1 & pMac->lim.gUapsdEnable);
	}
	pInfo->present = 1;
}

void populate_dot11f_wmm_info_station_per_session(tpAniSirGlobal pMac,
						  tpPESession psessionEntry,
						  tDot11fIEWMMInfoStation *pInfo)
{
	uint32_t val = 0;

	pInfo->version = SIR_MAC_OUI_VERSION_1;
	pInfo->acvo_uapsd =
		LIM_UAPSD_GET(ACVO, psessionEntry->gUapsdPerAcBitmask);
	pInfo->acvi_uapsd =
		LIM_UAPSD_GET(ACVI, psessionEntry->gUapsdPerAcBitmask);
	pInfo->acbk_uapsd =
		LIM_UAPSD_GET(ACBK, psessionEntry->gUapsdPerAcBitmask);
	pInfo->acbe_uapsd =
		LIM_UAPSD_GET(ACBE, psessionEntry->gUapsdPerAcBitmask);

	if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_SP_LENGTH, &val) != eSIR_SUCCESS)
		pe_err("could not retrieve Max SP Length");

		pInfo->max_sp_length = (uint8_t) val;
	pInfo->present = 1;
}

void populate_dot11f_wmm_params(tpAniSirGlobal pMac,
				tDot11fIEWMMParams *pParams,
				tpPESession psessionEntry)
{
	pParams->version = SIR_MAC_OUI_VERSION_1;

	if (LIM_IS_AP_ROLE(psessionEntry))
		pParams->qosInfo =
			(psessionEntry->
			 apUapsdEnable << 7) | ((uint8_t) (0x0f & psessionEntry->
							   gLimEdcaParamSetCount));
	else
		pParams->qosInfo =
			(pMac->lim.
			 gUapsdEnable << 7) | ((uint8_t) (0x0f & psessionEntry->
							  gLimEdcaParamSetCount));

	/* Fill each EDCA parameter set in order: be, bk, vi, vo */
	pParams->acbe_aifsn =
		(0xf & SET_AIFSN(psessionEntry->gLimEdcaParamsBC[0].aci.aifsn));
	pParams->acbe_acm = (0x1 & psessionEntry->gLimEdcaParamsBC[0].aci.acm);
	pParams->acbe_aci = (0x3 & SIR_MAC_EDCAACI_BESTEFFORT);
	pParams->acbe_acwmin =
		(0xf & psessionEntry->gLimEdcaParamsBC[0].cw.min);
	pParams->acbe_acwmax =
		(0xf & psessionEntry->gLimEdcaParamsBC[0].cw.max);
	pParams->acbe_txoplimit = psessionEntry->gLimEdcaParamsBC[0].txoplimit;

	pParams->acbk_aifsn =
		(0xf & SET_AIFSN(psessionEntry->gLimEdcaParamsBC[1].aci.aifsn));
	pParams->acbk_acm = (0x1 & psessionEntry->gLimEdcaParamsBC[1].aci.acm);
	pParams->acbk_aci = (0x3 & SIR_MAC_EDCAACI_BACKGROUND);
	pParams->acbk_acwmin =
		(0xf & psessionEntry->gLimEdcaParamsBC[1].cw.min);
	pParams->acbk_acwmax =
		(0xf & psessionEntry->gLimEdcaParamsBC[1].cw.max);
	pParams->acbk_txoplimit = psessionEntry->gLimEdcaParamsBC[1].txoplimit;

	if (LIM_IS_AP_ROLE(psessionEntry))
		pParams->acvi_aifsn =
			(0xf & psessionEntry->gLimEdcaParamsBC[2].aci.aifsn);
	else
		pParams->acvi_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[2].aci.aifsn));

	pParams->acvi_acm = (0x1 & psessionEntry->gLimEdcaParamsBC[2].aci.acm);
	pParams->acvi_aci = (0x3 & SIR_MAC_EDCAACI_VIDEO);
	pParams->acvi_acwmin =
		(0xf & psessionEntry->gLimEdcaParamsBC[2].cw.min);
	pParams->acvi_acwmax =
		(0xf & psessionEntry->gLimEdcaParamsBC[2].cw.max);
	pParams->acvi_txoplimit = psessionEntry->gLimEdcaParamsBC[2].txoplimit;

	if (LIM_IS_AP_ROLE(psessionEntry))
		pParams->acvo_aifsn =
			(0xf & psessionEntry->gLimEdcaParamsBC[3].aci.aifsn);
	else
		pParams->acvo_aifsn =
			(0xf &
			 SET_AIFSN(psessionEntry->gLimEdcaParamsBC[3].aci.aifsn));

	pParams->acvo_acm = (0x1 & psessionEntry->gLimEdcaParamsBC[3].aci.acm);
	pParams->acvo_aci = (0x3 & SIR_MAC_EDCAACI_VOICE);
	pParams->acvo_acwmin =
		(0xf & psessionEntry->gLimEdcaParamsBC[3].cw.min);
	pParams->acvo_acwmax =
		(0xf & psessionEntry->gLimEdcaParamsBC[3].cw.max);
	pParams->acvo_txoplimit = psessionEntry->gLimEdcaParamsBC[3].txoplimit;

	pParams->present = 1;

} /* End populate_dot11f_wmm_params. */

void populate_dot11f_wmm_schedule(tSirMacScheduleIE *pSchedule,
				  tDot11fIEWMMSchedule *pDot11f)
{
	pDot11f->version = 1;
	pDot11f->aggregation = pSchedule->info.aggregation;
	pDot11f->tsid = pSchedule->info.tsid;
	pDot11f->direction = pSchedule->info.direction;
	pDot11f->reserved = pSchedule->info.rsvd;
	pDot11f->service_start_time = pSchedule->svcStartTime;
	pDot11f->service_interval = pSchedule->svcInterval;
	pDot11f->max_service_dur = pSchedule->maxSvcDuration;
	pDot11f->spec_interval = pSchedule->specInterval;

	pDot11f->present = 1;
} /* End populate_dot11f_wmm_schedule. */

tSirRetStatus
populate_dot11f_wpa(tpAniSirGlobal pMac,
		    tpSirRSNie pRsnIe, tDot11fIEWPA *pDot11f)
{
	uint32_t status;
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_WPA);
		if (0 <= idx) {
			status = dot11f_unpack_ie_wpa(pMac, pRsnIe->rsnIEdata + idx + 2 + 4,       /* EID, length, OUI */
						      pRsnIe->rsnIEdata[idx + 1] - 4,   /* OUI */
						      pDot11f, false);
			if (DOT11F_FAILED(status)) {
				pe_err("Parse failure in Populate Dot11fWPA (0x%08x)",
					status);
				return eSIR_FAILURE;
			}
		}
	}

	return eSIR_SUCCESS;
} /* End populate_dot11f_wpa. */

tSirRetStatus populate_dot11f_wpa_opaque(tpAniSirGlobal pMac,
					 tpSirRSNie pRsnIe,
					 tDot11fIEWPAOpaque *pDot11f)
{
	int idx;

	if (pRsnIe->length) {
		idx = find_ie_location(pMac, pRsnIe, DOT11F_EID_WPA);
		if (0 <= idx) {
			pDot11f->present = 1;
			pDot11f->num_data = pRsnIe->rsnIEdata[idx + 1] - 4;
			qdf_mem_copy(pDot11f->data, pRsnIe->rsnIEdata + idx + 2 + 4,    /* EID, len, OUI */
				     pRsnIe->rsnIEdata[idx + 1] - 4);   /* OUI */
		}
	}

	return eSIR_SUCCESS;

} /* End populate_dot11f_wpa_opaque. */

/* ////////////////////////////////////////////////////////////////////// */

tSirRetStatus
sir_convert_probe_req_frame2_struct(tpAniSirGlobal pMac,
				    uint8_t *pFrame,
				    uint32_t nFrame, tpSirProbeReq pProbeReq)
{
	uint32_t status;
	tDot11fProbeRequest pr;

	/* Ok, zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pProbeReq, sizeof(tSirProbeReq), 0);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_probe_request(pMac, pFrame, nFrame, &pr, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse a Probe Request (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking a Probe Request (0x%08x, %d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fProbeRequestto' a 'tSirProbeReq'... */
	if (!pr.SSID.present) {
		pe_debug("Mandatory IE SSID not present!");
	} else {
		pProbeReq->ssidPresent = 1;
		convert_ssid(pMac, &pProbeReq->ssId, &pr.SSID);
	}

	if (!pr.SuppRates.present) {
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
		return eSIR_FAILURE;
	} else {
		pProbeReq->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pProbeReq->supportedRates,
				   &pr.SuppRates);
	}

	if (pr.ExtSuppRates.present) {
		pProbeReq->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pProbeReq->extendedRates,
				       &pr.ExtSuppRates);
	}

	if (pr.HTCaps.present) {
		qdf_mem_copy(&pProbeReq->HTCaps, &pr.HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (pr.WscProbeReq.present) {
		pProbeReq->wscIePresent = 1;
		memcpy(&pProbeReq->probeReqWscIeInfo, &pr.WscProbeReq,
		       sizeof(tDot11fIEWscProbeReq));
	}
	if (pr.VHTCaps.present) {
		qdf_mem_copy(&pProbeReq->VHTCaps, &pr.VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	if (pr.P2PProbeReq.present) {
		pProbeReq->p2pIePresent = 1;
	}
	if (pr.he_cap.present) {
		qdf_mem_copy(&pProbeReq->he_cap, &pr.he_cap,
			     sizeof(tDot11fIEhe_cap));
		pe_debug("11AX: HE cap IE present");
	}
	return eSIR_SUCCESS;
} /* End sir_convert_probe_req_frame2_struct. */


/**
 * sir_validate_and_rectify_ies() - API to check malformed frame
 * @mac_ctx: mac context
 * @mgmt_frame: pointer to management frame
 * @frame_bytes: no of bytes in frame
 * @missing_rsn_bytes: missing rsn bytes
 *
 * The frame would contain fixed IEs of 12 bytes followed by variable IEs
 * (Tagged elements). Every Tagged IE has tag number, tag length and data.
 * Tag length indicates the size of data in bytes.
 * This function checks for size of Frame received with the sum of all IEs.
 * And also rectifies missing optional fields in IE.
 *
 * NOTE : Presently this function rectifies RSN capability in RSN IE, can
 * be extended to rectify other optional fields in other IEs.
 *
 * Return: 0 on success, error number otherwise.
 */
tSirRetStatus
sir_validate_and_rectify_ies(tpAniSirGlobal mac_ctx,
				uint8_t *mgmt_frame,
				uint32_t frame_bytes,
				uint32_t *missing_rsn_bytes)
{
	uint32_t length = SIZE_OF_FIXED_PARAM;
	uint8_t *ref_frame;

	/* Frame contains atleast one IE */
	if (frame_bytes > (SIZE_OF_FIXED_PARAM +
			SIZE_OF_TAG_PARAM_NUM + SIZE_OF_TAG_PARAM_LEN)) {
		while (length < frame_bytes) {
			/* ref frame points to next IE */
			ref_frame = mgmt_frame + length;
			length += (uint32_t)(SIZE_OF_TAG_PARAM_NUM +
					SIZE_OF_TAG_PARAM_LEN +
					(*(ref_frame + SIZE_OF_TAG_PARAM_NUM)));
		}
		if (length != frame_bytes) {
			/*
			 * Workaround : Some APs may not include RSN
			 * Capability but the length of which is included in
			 * RSN IE length. This may cause in updating RSN
			 * Capability with junk value. To avoid this, add RSN
			 * Capability value with default value.
			 */
			if ((*ref_frame == RSNIEID) &&
				(length == (frame_bytes +
					RSNIE_CAPABILITY_LEN))) {
				/* Assume RSN Capability as 00 */
				qdf_mem_set((uint8_t *)(mgmt_frame +
					(frame_bytes)),
					RSNIE_CAPABILITY_LEN,
					DEFAULT_RSNIE_CAP_VAL);
				*missing_rsn_bytes = RSNIE_CAPABILITY_LEN;
				pe_debug("Added RSN Capability to RSNIE as 0x00 0x00");
				return eSIR_SUCCESS;
			}
			return eSIR_FAILURE;
		}
	}
	return eSIR_SUCCESS;
}

void sir_copy_caps_info(tpAniSirGlobal mac_ctx, tDot11fFfCapabilities caps,
					    tpSirProbeRespBeacon pProbeResp)
{
	pProbeResp->capabilityInfo.ess = caps.ess;
	pProbeResp->capabilityInfo.ibss = caps.ibss;
	pProbeResp->capabilityInfo.cfPollable = caps.cfPollable;
	pProbeResp->capabilityInfo.cfPollReq = caps.cfPollReq;
	pProbeResp->capabilityInfo.privacy = caps.privacy;
	pProbeResp->capabilityInfo.shortPreamble = caps.shortPreamble;
	pProbeResp->capabilityInfo.pbcc = caps.pbcc;
	pProbeResp->capabilityInfo.channelAgility =	caps.channelAgility;
	pProbeResp->capabilityInfo.spectrumMgt = caps.spectrumMgt;
	pProbeResp->capabilityInfo.qos = caps.qos;
	pProbeResp->capabilityInfo.shortSlotTime = caps.shortSlotTime;
	pProbeResp->capabilityInfo.apsd = caps.apsd;
	pProbeResp->capabilityInfo.rrm = caps.rrm;
	pProbeResp->capabilityInfo.dsssOfdm = caps.dsssOfdm;
	pProbeResp->capabilityInfo.delayedBA = caps.delayedBA;
	pProbeResp->capabilityInfo.immediateBA = caps.immediateBA;
}

#ifdef WLAN_FEATURE_FILS_SK
static void populate_dot11f_fils_rsn(tpAniSirGlobal mac_ctx,
				     tDot11fIERSNOpaque *p_dot11f,
				     uint8_t *rsn_ie)
{
	pe_debug("FILS RSN IE length %d", rsn_ie[1]);
	if (rsn_ie[1]) {
		p_dot11f->present = 1;
		p_dot11f->num_data = rsn_ie[1];
		qdf_mem_copy(p_dot11f->data, &rsn_ie[2], rsn_ie[1]);
	}
}

void populate_dot11f_fils_params(tpAniSirGlobal mac_ctx,
		tDot11fAssocRequest *frm,
		tpPESession pe_session)
{
	struct pe_fils_session *fils_info = pe_session->fils_info;

	/* Populate RSN IE with FILS AKM */
	populate_dot11f_fils_rsn(mac_ctx, &frm->RSNOpaque,
				 fils_info->rsn_ie);

	/* Populate FILS session IE */
	frm->fils_session.present = true;
	qdf_mem_copy(frm->fils_session.session,
		     fils_info->fils_session, FILS_SESSION_LENGTH);

	/* Populate FILS Key confirmation IE */
	if (fils_info->key_auth_len) {
		frm->fils_key_confirmation.present = true;
		frm->fils_key_confirmation.num_key_auth =
						fils_info->key_auth_len;

		qdf_mem_copy(frm->fils_key_confirmation.key_auth,
			     fils_info->key_auth, fils_info->key_auth_len);
	}
}

/**
 * update_fils_data: update fils params from beacon/probe response
 * @fils_ind: pointer to sir_fils_indication
 * @fils_indication: pointer to tDot11fIEfils_indication
 *
 * Return: None
 */
void update_fils_data(struct sir_fils_indication *fils_ind,
		      tDot11fIEfils_indication *fils_indication)
{
	uint8_t *data;

	data = fils_indication->variable_data;
	fils_ind->is_present = true;
	fils_ind->is_ip_config_supported =
			fils_indication->is_ip_config_supported;
	fils_ind->is_fils_sk_auth_supported =
			fils_indication->is_fils_sk_auth_supported;
	fils_ind->is_fils_sk_auth_pfs_supported =
			fils_indication->is_fils_sk_auth_pfs_supported;
	fils_ind->is_pk_auth_supported =
			fils_indication->is_pk_auth_supported;
	if (fils_indication->is_cache_id_present) {
		fils_ind->cache_identifier.is_present = true;
		qdf_mem_copy(fils_ind->cache_identifier.identifier,
				data, SIR_CACHE_IDENTIFIER_LEN);
		data = data + SIR_CACHE_IDENTIFIER_LEN;
	}
	if (fils_indication->is_hessid_present) {
		fils_ind->hessid.is_present = true;
		qdf_mem_copy(fils_ind->hessid.hessid,
				data, SIR_HESSID_LEN);
		data = data + SIR_HESSID_LEN;
	}
	if (fils_indication->realm_identifiers_cnt) {
		fils_ind->realm_identifier.is_present = true;
		fils_ind->realm_identifier.realm_cnt =
			fils_indication->realm_identifiers_cnt;
		qdf_mem_copy(fils_ind->realm_identifier.realm,
			data, fils_ind->realm_identifier.realm_cnt *
					SIR_REALM_LEN);
	}
}
#endif

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
static void update_bss_color_change_ie_from_probe_rsp(
		tDot11fProbeResponse *prb_frm,
		tpSirProbeRespBeacon prb_rsp_struct)
{
	if (prb_frm->bss_color_change.present) {
		pe_debug("11AX: HE BSS color change present");
		qdf_mem_copy(&prb_rsp_struct->vendor_he_bss_color_change,
			     &prb_frm->bss_color_change,
			     sizeof(tDot11fIEbss_color_change));
	}
}
#else
static inline void update_bss_color_change_ie_from_probe_rsp(
		tDot11fProbeResponse *prb_frm,
		tpSirProbeRespBeacon prb_rsp_struct)
{}
#endif
tSirRetStatus sir_convert_probe_frame2_struct(tpAniSirGlobal pMac,
					      uint8_t *pFrame,
					      uint32_t nFrame,
					      tpSirProbeRespBeacon pProbeResp)
{
	uint32_t status;
	tDot11fProbeResponse *pr;

	/* Ok, zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pProbeResp, sizeof(tSirProbeRespBeacon), 0);

	pr = qdf_mem_malloc(sizeof(tDot11fProbeResponse));
	if (NULL == pr) {
		pe_err("Failed to allocate memory");
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_probe_response(pMac, pFrame, nFrame, pr, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse a Probe Response (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
				   pFrame, nFrame);
		qdf_mem_free(pr);
		return eSIR_FAILURE;
	}
	/* & "transliterate" from a 'tDot11fProbeResponse' to a 'tSirProbeRespBeacon'... */

	/* Timestamp */
	qdf_mem_copy((uint8_t *) pProbeResp->timeStamp,
		     (uint8_t *) &pr->TimeStamp, sizeof(tSirMacTimeStamp));

	/* Beacon Interval */
	pProbeResp->beaconInterval = pr->BeaconInterval.interval;

	sir_copy_caps_info(pMac, pr->Capabilities, pProbeResp);

	if (!pr->SSID.present) {
		pe_debug("Mandatory IE SSID not present!");
	} else {
		pProbeResp->ssidPresent = 1;
		convert_ssid(pMac, &pProbeResp->ssId, &pr->SSID);
	}

	if (!pr->SuppRates.present) {
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
	} else {
		pProbeResp->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pProbeResp->supportedRates,
				   &pr->SuppRates);
	}

	if (pr->ExtSuppRates.present) {
		pProbeResp->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pProbeResp->extendedRates,
				       &pr->ExtSuppRates);
	}

	if (pr->CFParams.present) {
		pProbeResp->cfPresent = 1;
		convert_cf_params(pMac, &pProbeResp->cfParamSet, &pr->CFParams);
	}

	if (pr->Country.present) {
		pProbeResp->countryInfoPresent = 1;
		convert_country(pMac, &pProbeResp->countryInfoParam,
				&pr->Country);
	}

	if (pr->EDCAParamSet.present) {
		pProbeResp->edcaPresent = 1;
		convert_edca_param(pMac, &pProbeResp->edcaParams,
				   &pr->EDCAParamSet);
	}

	if (pr->ChanSwitchAnn.present) {
		pProbeResp->channelSwitchPresent = 1;
		qdf_mem_copy(&pProbeResp->channelSwitchIE, &pr->ChanSwitchAnn,
			     sizeof(pProbeResp->channelSwitchIE));
	}

	if (pr->ext_chan_switch_ann.present) {
		pProbeResp->ext_chan_switch_present = 1;
		qdf_mem_copy(&pProbeResp->ext_chan_switch,
			     &pr->ext_chan_switch_ann,
			     sizeof(tDot11fIEext_chan_switch_ann));
	}

	if (pr->SuppOperatingClasses.present) {
		pProbeResp->supp_operating_class_present = 1;
		qdf_mem_copy(&pProbeResp->supp_operating_classes,
			&pr->SuppOperatingClasses,
			sizeof(tDot11fIESuppOperatingClasses));
	}

	if (pr->sec_chan_offset_ele.present) {
		pProbeResp->sec_chan_offset_present = 1;
		qdf_mem_copy(&pProbeResp->sec_chan_offset,
			     &pr->sec_chan_offset_ele,
			     sizeof(pProbeResp->sec_chan_offset));
	}

	if (pr->TPCReport.present) {
		pProbeResp->tpcReportPresent = 1;
		qdf_mem_copy(&pProbeResp->tpcReport, &pr->TPCReport,
			     sizeof(tDot11fIETPCReport));
	}

	if (pr->PowerConstraints.present) {
		pProbeResp->powerConstraintPresent = 1;
		qdf_mem_copy(&pProbeResp->localPowerConstraint,
			     &pr->PowerConstraints,
			     sizeof(tDot11fIEPowerConstraints));
	}

	if (pr->Quiet.present) {
		pProbeResp->quietIEPresent = 1;
		qdf_mem_copy(&pProbeResp->quietIE, &pr->Quiet,
			     sizeof(tDot11fIEQuiet));
	}

	if (pr->HTCaps.present) {
		qdf_mem_copy(&pProbeResp->HTCaps, &pr->HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (pr->HTInfo.present) {
		qdf_mem_copy(&pProbeResp->HTInfo, &pr->HTInfo,
			     sizeof(tDot11fIEHTInfo));
	}

	if (pr->DSParams.present) {
		pProbeResp->dsParamsPresent = 1;
		pProbeResp->channelNumber = pr->DSParams.curr_channel;
	} else if (pr->HTInfo.present) {
		pProbeResp->channelNumber = pr->HTInfo.primaryChannel;
	}

	if (pr->RSNOpaque.present) {
		pProbeResp->rsnPresent = 1;
		convert_rsn_opaque(pMac, &pProbeResp->rsn, &pr->RSNOpaque);
	}

	if (pr->WPA.present) {
		pProbeResp->wpaPresent = 1;
		convert_wpa(pMac, &pProbeResp->wpa, &pr->WPA);
	}

	if (pr->WMMParams.present) {
		pProbeResp->wmeEdcaPresent = 1;
		convert_wmm_params(pMac, &pProbeResp->edcaParams, &pr->WMMParams);
	}

	if (pr->WMMInfoAp.present) {
		pProbeResp->wmeInfoPresent = 1;
		pe_debug("WMM Information Element present in Probe Response Frame!");
	}

	if (pr->WMMCaps.present) {
		pProbeResp->wsmCapablePresent = 1;
	}

	if (pr->ERPInfo.present) {
		pProbeResp->erpPresent = 1;
		convert_erp_info(pMac, &pProbeResp->erpIEInfo, &pr->ERPInfo);
	}
	if (pr->MobilityDomain.present) {
		/* MobilityDomain */
		pProbeResp->mdiePresent = 1;
		qdf_mem_copy((uint8_t *) &(pProbeResp->mdie[0]),
			     (uint8_t *) &(pr->MobilityDomain.MDID),
			     sizeof(uint16_t));
		pProbeResp->mdie[2] =
			((pr->MobilityDomain.overDSCap << 0) | (pr->MobilityDomain.
								resourceReqCap <<
								1));
		pe_debug("mdie=%02x%02x%02x",
			(unsigned int)pProbeResp->mdie[0],
			(unsigned int)pProbeResp->mdie[1],
			(unsigned int)pProbeResp->mdie[2]);
	}

#if defined FEATURE_WLAN_ESE
	if (pr->ESEVersion.present)
		pProbeResp->is_ese_ver_ie_present = 1;
	if (pr->QBSSLoad.present) {
		qdf_mem_copy(&pProbeResp->QBSSLoad, &pr->QBSSLoad,
			     sizeof(tDot11fIEQBSSLoad));
	}
#endif
	if (pr->P2PProbeRes.present) {
		qdf_mem_copy(&pProbeResp->P2PProbeRes, &pr->P2PProbeRes,
			     sizeof(tDot11fIEP2PProbeRes));
	}
	if (pr->VHTCaps.present) {
		qdf_mem_copy(&pProbeResp->VHTCaps, &pr->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	if (pr->VHTOperation.present) {
		qdf_mem_copy(&pProbeResp->VHTOperation, &pr->VHTOperation,
			     sizeof(tDot11fIEVHTOperation));
	}
	if (pr->VHTExtBssLoad.present) {
		qdf_mem_copy(&pProbeResp->VHTExtBssLoad, &pr->VHTExtBssLoad,
			     sizeof(tDot11fIEVHTExtBssLoad));
	}
	pProbeResp->Vendor1IEPresent = pr->Vendor1IE.present;
	pProbeResp->Vendor3IEPresent = pr->Vendor3IE.present;

	pProbeResp->vendor_vht_ie.present = pr->vendor_vht_ie.present;
	if (pr->vendor_vht_ie.present)
		pProbeResp->vendor_vht_ie.sub_type = pr->vendor_vht_ie.sub_type;
	if (pr->vendor_vht_ie.VHTCaps.present) {
		qdf_mem_copy(&pProbeResp->vendor_vht_ie.VHTCaps,
				&pr->vendor_vht_ie.VHTCaps,
				sizeof(tDot11fIEVHTCaps));
	}
	if (pr->vendor_vht_ie.VHTOperation.present) {
		qdf_mem_copy(&pProbeResp->vendor_vht_ie.VHTOperation,
				&pr->vendor_vht_ie.VHTOperation,
				sizeof(tDot11fIEVHTOperation));
	}
	/* Update HS 2.0 Information Element */
	if (pr->hs20vendor_ie.present) {
		pe_debug("HS20 Indication Element Present, rel#:%u, id:%u",
			pr->hs20vendor_ie.release_num,
			pr->hs20vendor_ie.hs_id_present);
		qdf_mem_copy(&pProbeResp->hs20vendor_ie,
			&pr->hs20vendor_ie,
			sizeof(tDot11fIEhs20vendor_ie) -
			sizeof(pr->hs20vendor_ie.hs_id));
		if (pr->hs20vendor_ie.hs_id_present)
			qdf_mem_copy(&pProbeResp->hs20vendor_ie.hs_id,
				&pr->hs20vendor_ie.hs_id,
				sizeof(pr->hs20vendor_ie.hs_id));
	}
	if (pr->MBO_IE.present) {
		pProbeResp->MBO_IE_present = true;
		if (pr->MBO_IE.cellular_data_cap.present)
			pProbeResp->MBO_capability =
				pr->MBO_IE.cellular_data_cap.cellular_connectivity;

		if (pr->MBO_IE.assoc_disallowed.present) {
			pProbeResp->assoc_disallowed = true;
			pProbeResp->assoc_disallowed_reason =
				pr->MBO_IE.assoc_disallowed.reason_code;
		}
	}

	if (pr->QCN_IE.present) {
		pProbeResp->QCN_IE.is_present = true;

		if (pr->QCN_IE.version[0] == QCN_IE_VERSION_SUBATTR_ID) {
			pProbeResp->QCN_IE.version
					= pr->QCN_IE.version[2];
			pProbeResp->QCN_IE.sub_version
					= pr->QCN_IE.version[3];
		}
	}

	if (pr->he_cap.present) {
		pe_debug("11AX: HE cap IE present");
		qdf_mem_copy(&pProbeResp->he_cap, &pr->he_cap,
			     sizeof(tDot11fIEhe_cap));
	}
	if (pr->he_op.present) {
		pe_debug("11AX: HE operation IE present");
		qdf_mem_copy(&pProbeResp->he_op, &pr->he_op,
			     sizeof(tDot11fIEhe_op));
	}

	update_bss_color_change_ie_from_probe_rsp(pr, pProbeResp);

	qdf_mem_free(pr);
	return eSIR_SUCCESS;

} /* End sir_convert_probe_frame2_struct. */

tSirRetStatus
sir_convert_assoc_req_frame2_struct(tpAniSirGlobal pMac,
				    uint8_t *pFrame,
				    uint32_t nFrame, tpSirAssocReq pAssocReq)
{
	tDot11fAssocRequest *ar;
	uint32_t status;

	ar = qdf_mem_malloc(sizeof(tDot11fAssocRequest));
	if (NULL == ar) {
		pe_err("Failed to allocate memory");
		return eSIR_MEM_ALLOC_FAILED;
	}
	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pAssocReq, sizeof(tSirAssocReq), 0);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_assoc_request(pMac, pFrame, nFrame, ar, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Association Request (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		qdf_mem_free(ar);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking an Assoication Request (0x%08x, %d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fAssocRequest' to a 'tSirAssocReq'... */

	/* make sure this is seen as an assoc request */
	pAssocReq->reassocRequest = 0;

	/* Capabilities */
	pAssocReq->capabilityInfo.ess = ar->Capabilities.ess;
	pAssocReq->capabilityInfo.ibss = ar->Capabilities.ibss;
	pAssocReq->capabilityInfo.cfPollable = ar->Capabilities.cfPollable;
	pAssocReq->capabilityInfo.cfPollReq = ar->Capabilities.cfPollReq;
	pAssocReq->capabilityInfo.privacy = ar->Capabilities.privacy;
	pAssocReq->capabilityInfo.shortPreamble =
		ar->Capabilities.shortPreamble;
	pAssocReq->capabilityInfo.pbcc = ar->Capabilities.pbcc;
	pAssocReq->capabilityInfo.channelAgility =
		ar->Capabilities.channelAgility;
	pAssocReq->capabilityInfo.spectrumMgt = ar->Capabilities.spectrumMgt;
	pAssocReq->capabilityInfo.qos = ar->Capabilities.qos;
	pAssocReq->capabilityInfo.shortSlotTime =
		ar->Capabilities.shortSlotTime;
	pAssocReq->capabilityInfo.apsd = ar->Capabilities.apsd;
	pAssocReq->capabilityInfo.rrm = ar->Capabilities.rrm;
	pAssocReq->capabilityInfo.dsssOfdm = ar->Capabilities.dsssOfdm;
	pAssocReq->capabilityInfo.delayedBA = ar->Capabilities.delayedBA;
	pAssocReq->capabilityInfo.immediateBA = ar->Capabilities.immediateBA;

	/* Listen Interval */
	pAssocReq->listenInterval = ar->ListenInterval.interval;

	/* SSID */
	if (ar->SSID.present) {
		pAssocReq->ssidPresent = 1;
		convert_ssid(pMac, &pAssocReq->ssId, &ar->SSID);
	}
	/* Supported Rates */
	if (ar->SuppRates.present) {
		pAssocReq->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pAssocReq->supportedRates,
				   &ar->SuppRates);
	}
	/* Extended Supported Rates */
	if (ar->ExtSuppRates.present) {
		pAssocReq->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pAssocReq->extendedRates,
				       &ar->ExtSuppRates);
	}
	/* QOS Capabilities: */
	if (ar->QOSCapsStation.present) {
		pAssocReq->qosCapabilityPresent = 1;
		convert_qos_caps_station(pMac, &pAssocReq->qosCapability,
					 &ar->QOSCapsStation);
	}
	/* WPA */
	if (ar->WPAOpaque.present) {
		pAssocReq->wpaPresent = 1;
		convert_wpa_opaque(pMac, &pAssocReq->wpa, &ar->WPAOpaque);
	}
#ifdef FEATURE_WLAN_WAPI
	if (ar->WAPIOpaque.present) {
		pAssocReq->wapiPresent = 1;
		convert_wapi_opaque(pMac, &pAssocReq->wapi, &ar->WAPIOpaque);
	}
#endif
	/* RSN */
	if (ar->RSNOpaque.present) {
		pAssocReq->rsnPresent = 1;
		convert_rsn_opaque(pMac, &pAssocReq->rsn, &ar->RSNOpaque);
	}
	/* WSC IE */
	if (ar->WscIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_wsc_opaque(pMac, &pAssocReq->addIE, &ar->WscIEOpaque);
	}

	if (ar->P2PIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_p2p_opaque(pMac, &pAssocReq->addIE, &ar->P2PIEOpaque);
	}
#ifdef WLAN_FEATURE_WFD
	if (ar->WFDIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_wfd_opaque(pMac, &pAssocReq->addIE, &ar->WFDIEOpaque);
	}
#endif

	/* Power Capabilities */
	if (ar->PowerCaps.present) {
		pAssocReq->powerCapabilityPresent = 1;
		convert_power_caps(pMac, &pAssocReq->powerCapability,
				   &ar->PowerCaps);
	}
	/* Supported Channels */
	if (ar->SuppChannels.present) {
		pAssocReq->supportedChannelsPresent = 1;
		convert_supp_channels(pMac, &pAssocReq->supportedChannels,
				      &ar->SuppChannels);
	}

	if (ar->HTCaps.present) {
		qdf_mem_copy(&pAssocReq->HTCaps, &ar->HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (ar->WMMInfoStation.present) {
		pAssocReq->wmeInfoPresent = 1;
		qdf_mem_copy(&pAssocReq->WMMInfoStation, &ar->WMMInfoStation,
			     sizeof(tDot11fIEWMMInfoStation));

	}

	if (ar->WMMCaps.present)
		pAssocReq->wsmCapablePresent = 1;

	if (!pAssocReq->ssidPresent) {
		pe_debug("Received Assoc without SSID IE");
		qdf_mem_free(ar);
		return eSIR_FAILURE;
	}

	if (!pAssocReq->suppRatesPresent && !pAssocReq->extendedRatesPresent) {
		pe_debug("Received Assoc without supp rate IE");
		qdf_mem_free(ar);
		return eSIR_FAILURE;
	}
	if (ar->VHTCaps.present) {
		qdf_mem_copy(&pAssocReq->VHTCaps, &ar->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
		pe_warn("Received Assoc Req with VHT Cap");
		lim_log_vht_cap(pMac, &pAssocReq->VHTCaps);
	}
	if (ar->OperatingMode.present) {
		qdf_mem_copy(&pAssocReq->operMode, &ar->OperatingMode,
			     sizeof(tDot11fIEOperatingMode));
		pe_warn("Received Assoc Req with Operating Mode IE");
		lim_log_operating_mode(pMac, &pAssocReq->operMode);
	}
	if (ar->ExtCap.present) {
		struct s_ext_cap *ext_cap;

		qdf_mem_copy(&pAssocReq->ExtCap, &ar->ExtCap,
			    sizeof(tDot11fIEExtCap));
		ext_cap = (struct s_ext_cap *)&pAssocReq->ExtCap.bytes;
		pe_debug("timingMeas: %d, finetimingMeas Init: %d, Resp: %d",
			ext_cap->timing_meas, ext_cap->fine_time_meas_initiator,
			ext_cap->fine_time_meas_responder);
	}

	pAssocReq->vendor_vht_ie.present = ar->vendor_vht_ie.present;
	if (ar->vendor_vht_ie.present) {
		pAssocReq->vendor_vht_ie.sub_type = ar->vendor_vht_ie.sub_type;
		if (ar->vendor_vht_ie.VHTCaps.present) {
			qdf_mem_copy(&pAssocReq->vendor_vht_ie.VHTCaps,
				     &ar->vendor_vht_ie.VHTCaps,
				     sizeof(tDot11fIEVHTCaps));
			pe_debug("Received Assoc Request with Vendor specific VHT Cap");
			lim_log_vht_cap(pMac, &pAssocReq->VHTCaps);
		}
	}
	if (ar->he_cap.present) {
		qdf_mem_copy(&pAssocReq->he_cap, &ar->he_cap,
			     sizeof(tDot11fIEhe_cap));
		pe_debug("Received Assoc Req with HE Capability IE");
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
				   &pAssocReq->he_cap, sizeof(tDot11fIEhe_cap));
	}
	qdf_mem_free(ar);
	return eSIR_SUCCESS;

} /* End sir_convert_assoc_req_frame2_struct. */

/**
 * dot11f_parse_assoc_response() - API to parse Assoc IE buffer to struct
 * @mac_ctx: MAC context
 * @p_buf: Pointer to the assoc IE buffer
 * @n_buf: length of the @p_buf
 * @p_frm: Struct to populate the IE buffer after parsing
 * @append_ie: Boolean to indicate whether to reset @p_frm or not. If @append_ie
 *             is true, @p_frm struct is not reset to zeros.
 *
 * Return: tSirRetStatus
 */
static tSirRetStatus dot11f_parse_assoc_response(tpAniSirGlobal mac_ctx,
						 uint8_t *p_buf, uint32_t n_buf,
						 tDot11fAssocResponse *p_frm,
						 bool append_ie)
{
	uint32_t status;

	status = dot11f_unpack_assoc_response(mac_ctx, p_buf,
					      n_buf, p_frm, append_ie);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Association Response (0x%08x, %d bytes):",
			status, n_buf);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   p_buf, n_buf);
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * fils_convert_assoc_rsp_frame2_struct() - Copy FILS IE's to Assoc rsp struct
 * @ar: frame parser Assoc response struct
 * @pAssocRsp: LIM Assoc response
 *
 * Return: None
 */
static void fils_convert_assoc_rsp_frame2_struct(tDot11fAssocResponse *ar,
						 tpSirAssocRsp pAssocRsp)
{
	if (ar->fils_session.present) {
		pe_debug("fils session IE present");
		pAssocRsp->fils_session.present = true;
		qdf_mem_copy(pAssocRsp->fils_session.session,
				ar->fils_session.session,
				DOT11F_IE_FILS_SESSION_MAX_LEN);
	}

	if (ar->fils_key_confirmation.present) {
		pe_debug("fils key conf IE present");
		pAssocRsp->fils_key_auth.num_key_auth =
			ar->fils_key_confirmation.num_key_auth;
		qdf_mem_copy(pAssocRsp->fils_key_auth.key_auth,
				ar->fils_key_confirmation.key_auth,
				pAssocRsp->fils_key_auth.num_key_auth);
	}

	if (ar->fils_kde.present) {
		pe_debug("fils kde IE present %d",
				ar->fils_kde.num_kde_list);
		pAssocRsp->fils_kde.num_kde_list =
			ar->fils_kde.num_kde_list;
		qdf_mem_copy(pAssocRsp->fils_kde.key_rsc,
				ar->fils_kde.key_rsc, KEY_RSC_LEN);
		qdf_mem_copy(&pAssocRsp->fils_kde.kde_list,
				&ar->fils_kde.kde_list,
				pAssocRsp->fils_kde.num_kde_list);
	}

	if (ar->fils_hlp_container.present) {
		pe_debug("FILS HLP container IE present");
		sir_copy_mac_addr(pAssocRsp->dst_mac.bytes,
				ar->fils_hlp_container.dest_mac);
		sir_copy_mac_addr(pAssocRsp->src_mac.bytes,
				ar->fils_hlp_container.src_mac);
		pAssocRsp->hlp_data_len = ar->fils_hlp_container.num_hlp_packet;
		qdf_mem_copy(pAssocRsp->hlp_data,
				ar->fils_hlp_container.hlp_packet,
				pAssocRsp->hlp_data_len);

		if (ar->fragment_ie.present) {
			pe_debug("FILS fragment ie present");
			qdf_mem_copy(pAssocRsp->hlp_data +
					pAssocRsp->hlp_data_len,
					ar->fragment_ie.data,
					ar->fragment_ie.num_data);
			pAssocRsp->hlp_data_len += ar->fragment_ie.num_data;
		}
	}
}
#else
static inline void fils_convert_assoc_rsp_frame2_struct(tDot11fAssocResponse
							*ar, tpSirAssocRsp
							pAssocRsp)
{ }
#endif

tSirRetStatus
sir_convert_assoc_resp_frame2_struct(tpAniSirGlobal pMac,
		tpPESession session_entry,
		uint8_t *pFrame, uint32_t nFrame,
		tpSirAssocRsp pAssocRsp)
{
	tDot11fAssocResponse *ar;
	uint32_t status;
	uint8_t cnt = 0;

	ar = qdf_mem_malloc(sizeof(*ar));
	if (!ar) {
		pe_err("Assoc rsp mem alloc fails");
		return eSIR_FAILURE;
	}

	/* decrypt the cipher text using AEAD decryption */
	if (lim_is_fils_connection(session_entry)) {
		status = aead_decrypt_assoc_rsp(pMac, session_entry,
						ar, pFrame, &nFrame);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			pe_err("FILS assoc rsp AEAD decrypt fails");
			qdf_mem_free(ar);
			return eSIR_FAILURE;
		}
	}

	status = dot11f_parse_assoc_response(pMac, pFrame, nFrame, ar, false);
	if (eSIR_SUCCESS != status) {
		qdf_mem_free(ar);
		return status;
	}


	/* Capabilities */
	pAssocRsp->capabilityInfo.ess = ar->Capabilities.ess;
	pAssocRsp->capabilityInfo.ibss = ar->Capabilities.ibss;
	pAssocRsp->capabilityInfo.cfPollable = ar->Capabilities.cfPollable;
	pAssocRsp->capabilityInfo.cfPollReq = ar->Capabilities.cfPollReq;
	pAssocRsp->capabilityInfo.privacy = ar->Capabilities.privacy;
	pAssocRsp->capabilityInfo.shortPreamble =
		ar->Capabilities.shortPreamble;
	pAssocRsp->capabilityInfo.pbcc = ar->Capabilities.pbcc;
	pAssocRsp->capabilityInfo.channelAgility =
		ar->Capabilities.channelAgility;
	pAssocRsp->capabilityInfo.spectrumMgt = ar->Capabilities.spectrumMgt;
	pAssocRsp->capabilityInfo.qos = ar->Capabilities.qos;
	pAssocRsp->capabilityInfo.shortSlotTime =
		ar->Capabilities.shortSlotTime;
	pAssocRsp->capabilityInfo.apsd = ar->Capabilities.apsd;
	pAssocRsp->capabilityInfo.rrm = ar->Capabilities.rrm;
	pAssocRsp->capabilityInfo.dsssOfdm = ar->Capabilities.dsssOfdm;
	pAssocRsp->capabilityInfo.delayedBA = ar->Capabilities.delayedBA;
	pAssocRsp->capabilityInfo.immediateBA = ar->Capabilities.immediateBA;

	pAssocRsp->statusCode = ar->Status.status;
	pAssocRsp->aid = ar->AID.associd;
#ifdef WLAN_FEATURE_11W
	if (ar->TimeoutInterval.present) {
		pAssocRsp->TimeoutInterval.present = 1;
		pAssocRsp->TimeoutInterval.timeoutType =
			ar->TimeoutInterval.timeoutType;
		pAssocRsp->TimeoutInterval.timeoutValue =
			ar->TimeoutInterval.timeoutValue;
	}
#endif

	if (!ar->SuppRates.present) {
		pAssocRsp->suppRatesPresent = 0;
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
	} else {
		pAssocRsp->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pAssocRsp->supportedRates,
				&ar->SuppRates);
	}

	if (ar->ExtSuppRates.present) {
		pAssocRsp->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pAssocRsp->extendedRates,
				&ar->ExtSuppRates);
	}

	if (ar->EDCAParamSet.present) {
		pAssocRsp->edcaPresent = 1;
		convert_edca_param(pMac, &pAssocRsp->edca, &ar->EDCAParamSet);
	}

	if (ar->WMMParams.present) {
		pAssocRsp->wmeEdcaPresent = 1;
		convert_wmm_params(pMac, &pAssocRsp->edca, &ar->WMMParams);
		pe_debug("Received Assoc Resp with WMM Param");
		__print_wmm_params(pMac, &ar->WMMParams);
	}

	if (ar->HTCaps.present) {
		pe_debug("Received Assoc Resp with HT Cap");
		qdf_mem_copy(&pAssocRsp->HTCaps, &ar->HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (ar->HTInfo.present) {
		pe_debug("Received Assoc Resp with HT Info");
		qdf_mem_copy(&pAssocRsp->HTInfo, &ar->HTInfo,
			     sizeof(tDot11fIEHTInfo));
	}
	if (ar->MobilityDomain.present) {
		/* MobilityDomain */
		pAssocRsp->mdiePresent = 1;
		qdf_mem_copy((uint8_t *) &(pAssocRsp->mdie[0]),
				(uint8_t *) &(ar->MobilityDomain.MDID),
				sizeof(uint16_t));
		pAssocRsp->mdie[2] = ((ar->MobilityDomain.overDSCap << 0) |
				      (ar->MobilityDomain.resourceReqCap << 1));
		pe_debug("new mdie=%02x%02x%02x",
			(unsigned int)pAssocRsp->mdie[0],
			(unsigned int)pAssocRsp->mdie[1],
			(unsigned int)pAssocRsp->mdie[2]);
	}

	if (ar->FTInfo.present) {
		pe_debug("FT Info present %d %d %d",
			ar->FTInfo.R0KH_ID.num_PMK_R0_ID,
			ar->FTInfo.R0KH_ID.present, ar->FTInfo.R1KH_ID.present);
		pAssocRsp->ftinfoPresent = 1;
		qdf_mem_copy(&pAssocRsp->FTInfo, &ar->FTInfo,
				sizeof(tDot11fIEFTInfo));
	}

	if (ar->num_RICDataDesc && ar->num_RICDataDesc <= 2) {
		for (cnt = 0; cnt < ar->num_RICDataDesc; cnt++) {
			if (ar->RICDataDesc[cnt].present) {
				qdf_mem_copy(&pAssocRsp->RICData[cnt],
						&ar->RICDataDesc[cnt],
						sizeof(tDot11fIERICDataDesc));
			}
		}
		pAssocRsp->num_RICData = ar->num_RICDataDesc;
		pAssocRsp->ricPresent = true;
	}

#ifdef FEATURE_WLAN_ESE
	if (ar->num_WMMTSPEC) {
		pAssocRsp->num_tspecs = ar->num_WMMTSPEC;
		for (cnt = 0; cnt < ar->num_WMMTSPEC; cnt++) {
			qdf_mem_copy(&pAssocRsp->TSPECInfo[cnt],
					&ar->WMMTSPEC[cnt],
					(sizeof(tDot11fIEWMMTSPEC) *
					 ar->num_WMMTSPEC));
		}
		pAssocRsp->tspecPresent = true;
	}

	if (ar->ESETrafStrmMet.present) {
		pAssocRsp->tsmPresent = 1;
		qdf_mem_copy(&pAssocRsp->tsmIE.tsid,
				&ar->ESETrafStrmMet.tsid,
				sizeof(tSirMacESETSMIE));
	}
#endif

	if (ar->VHTCaps.present) {
		qdf_mem_copy(&pAssocRsp->VHTCaps, &ar->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
		pe_debug("Received Assoc Response with VHT Cap");
		lim_log_vht_cap(pMac, &pAssocRsp->VHTCaps);
	}
	if (ar->VHTOperation.present) {
		qdf_mem_copy(&pAssocRsp->VHTOperation, &ar->VHTOperation,
			     sizeof(tDot11fIEVHTOperation));
		pe_debug("Received Assoc Response with VHT Operation");
		lim_log_vht_operation(pMac, &pAssocRsp->VHTOperation);
	}

	if (ar->ExtCap.present) {
		struct s_ext_cap *ext_cap;

		qdf_mem_copy(&pAssocRsp->ExtCap, &ar->ExtCap,
				sizeof(tDot11fIEExtCap));
		ext_cap = (struct s_ext_cap *)&pAssocRsp->ExtCap.bytes;
		pe_debug("timingMeas: %d, finetimingMeas Init: %d, Resp: %d",
			ext_cap->timing_meas, ext_cap->fine_time_meas_initiator,
			ext_cap->fine_time_meas_responder);
	}

	if (ar->QosMapSet.present) {
		pAssocRsp->QosMapSet.present = 1;
		convert_qos_mapset_frame(pMac, &pAssocRsp->QosMapSet,
					 &ar->QosMapSet);
		pe_debug("Received Assoc Response with Qos Map Set");
		lim_log_qos_map_set(pMac, &pAssocRsp->QosMapSet);
	}

	pAssocRsp->vendor_vht_ie.present = ar->vendor_vht_ie.present;
	if (ar->vendor_vht_ie.present)
		pAssocRsp->vendor_vht_ie.sub_type = ar->vendor_vht_ie.sub_type;
	if (ar->OBSSScanParameters.present) {
		qdf_mem_copy(&pAssocRsp->obss_scanparams,
				&ar->OBSSScanParameters,
				sizeof(struct sDot11fIEOBSSScanParameters));
	}
	if (ar->vendor_vht_ie.VHTCaps.present) {
		qdf_mem_copy(&pAssocRsp->vendor_vht_ie.VHTCaps,
				&ar->vendor_vht_ie.VHTCaps,
				sizeof(tDot11fIEVHTCaps));
		pe_debug("Received Assoc Response with Vendor specific VHT Cap");
		lim_log_vht_cap(pMac, &pAssocRsp->VHTCaps);
	}
	if (ar->vendor_vht_ie.VHTOperation.present) {
		qdf_mem_copy(&pAssocRsp->vendor_vht_ie.VHTOperation,
				&ar->vendor_vht_ie.VHTOperation,
				sizeof(tDot11fIEVHTOperation));
		pe_debug("Received Assoc Response with Vendor specific VHT Oper");
		lim_log_vht_operation(pMac, &pAssocRsp->VHTOperation);
	}

	if (ar->he_cap.present) {
		pe_debug("11AX: HE cap IE present");
		qdf_mem_copy(&pAssocRsp->he_cap, &ar->he_cap,
			     sizeof(tDot11fIEhe_cap));
	}
	if (ar->he_op.present) {
		pe_debug("11AX: HE Operation IE present");
		qdf_mem_copy(&pAssocRsp->he_op, &ar->he_op,
			     sizeof(tDot11fIEhe_op));
		pe_debug("bss_clr %d def_pe %d part_bss_clr %d bss_col_dis %d",
				pAssocRsp->he_op.bss_color,
				pAssocRsp->he_op.default_pe,
				pAssocRsp->he_op.partial_bss_col,
				pAssocRsp->he_op.bss_col_disabled);
	}

	if (ar->MBO_IE.present && ar->MBO_IE.rssi_assoc_rej.present) {
		qdf_mem_copy(&pAssocRsp->rssi_assoc_rej,
				&ar->MBO_IE.rssi_assoc_rej,
				sizeof(tDot11fTLVrssi_assoc_rej));
		pe_debug("Received Assoc Response with rssi based assoc rej");
	}

	fils_convert_assoc_rsp_frame2_struct(ar, pAssocRsp);

	qdf_mem_free(ar);
	return eSIR_SUCCESS;

} /* End sir_convert_assoc_resp_frame2_struct. */

tSirRetStatus
sir_convert_reassoc_req_frame2_struct(tpAniSirGlobal pMac,
				      uint8_t *pFrame,
				      uint32_t nFrame, tpSirAssocReq pAssocReq)
{
	static tDot11fReAssocRequest ar;
	uint32_t status;

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pAssocReq, sizeof(tSirAssocReq), 0);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_re_assoc_request(pMac, pFrame, nFrame,
						&ar, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse a Re-association Request (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking a Re-association Request (0x%08x, %d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fReAssocRequest' to a 'tSirAssocReq'... */

	/* make sure this is seen as a re-assoc request */
	pAssocReq->reassocRequest = 1;

	/* Capabilities */
	pAssocReq->capabilityInfo.ess = ar.Capabilities.ess;
	pAssocReq->capabilityInfo.ibss = ar.Capabilities.ibss;
	pAssocReq->capabilityInfo.cfPollable = ar.Capabilities.cfPollable;
	pAssocReq->capabilityInfo.cfPollReq = ar.Capabilities.cfPollReq;
	pAssocReq->capabilityInfo.privacy = ar.Capabilities.privacy;
	pAssocReq->capabilityInfo.shortPreamble = ar.Capabilities.shortPreamble;
	pAssocReq->capabilityInfo.pbcc = ar.Capabilities.pbcc;
	pAssocReq->capabilityInfo.channelAgility =
		ar.Capabilities.channelAgility;
	pAssocReq->capabilityInfo.spectrumMgt = ar.Capabilities.spectrumMgt;
	pAssocReq->capabilityInfo.qos = ar.Capabilities.qos;
	pAssocReq->capabilityInfo.shortSlotTime = ar.Capabilities.shortSlotTime;
	pAssocReq->capabilityInfo.apsd = ar.Capabilities.apsd;
	pAssocReq->capabilityInfo.rrm = ar.Capabilities.rrm;
	pAssocReq->capabilityInfo.dsssOfdm = ar.Capabilities.dsssOfdm;
	pAssocReq->capabilityInfo.delayedBA = ar.Capabilities.delayedBA;
	pAssocReq->capabilityInfo.immediateBA = ar.Capabilities.immediateBA;

	/* Listen Interval */
	pAssocReq->listenInterval = ar.ListenInterval.interval;

	/* SSID */
	if (ar.SSID.present) {
		pAssocReq->ssidPresent = 1;
		convert_ssid(pMac, &pAssocReq->ssId, &ar.SSID);
	}
	/* Supported Rates */
	if (ar.SuppRates.present) {
		pAssocReq->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pAssocReq->supportedRates,
				   &ar.SuppRates);
	}
	/* Extended Supported Rates */
	if (ar.ExtSuppRates.present) {
		pAssocReq->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pAssocReq->extendedRates,
				       &ar.ExtSuppRates);
	}
	/* QOS Capabilities: */
	if (ar.QOSCapsStation.present) {
		pAssocReq->qosCapabilityPresent = 1;
		convert_qos_caps_station(pMac, &pAssocReq->qosCapability,
					 &ar.QOSCapsStation);
	}
	/* WPA */
	if (ar.WPAOpaque.present) {
		pAssocReq->wpaPresent = 1;
		convert_wpa_opaque(pMac, &pAssocReq->wpa, &ar.WPAOpaque);
	}
	/* RSN */
	if (ar.RSNOpaque.present) {
		pAssocReq->rsnPresent = 1;
		convert_rsn_opaque(pMac, &pAssocReq->rsn, &ar.RSNOpaque);
	}

	/* Power Capabilities */
	if (ar.PowerCaps.present) {
		pAssocReq->powerCapabilityPresent = 1;
		convert_power_caps(pMac, &pAssocReq->powerCapability,
				   &ar.PowerCaps);
	}
	/* Supported Channels */
	if (ar.SuppChannels.present) {
		pAssocReq->supportedChannelsPresent = 1;
		convert_supp_channels(pMac, &pAssocReq->supportedChannels,
				      &ar.SuppChannels);
	}

	if (ar.HTCaps.present) {
		qdf_mem_copy(&pAssocReq->HTCaps, &ar.HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (ar.WMMInfoStation.present) {
		pAssocReq->wmeInfoPresent = 1;
		qdf_mem_copy(&pAssocReq->WMMInfoStation, &ar.WMMInfoStation,
			     sizeof(tDot11fIEWMMInfoStation));

	}

	if (ar.WMMCaps.present)
		pAssocReq->wsmCapablePresent = 1;

	if (!pAssocReq->ssidPresent) {
		pe_debug("Received Assoc without SSID IE");
		return eSIR_FAILURE;
	}

	if (!pAssocReq->suppRatesPresent && !pAssocReq->extendedRatesPresent) {
		pe_debug("Received Assoc without supp rate IE");
		return eSIR_FAILURE;
	}
	/* Why no call to 'updateAssocReqFromPropCapability' here, like */
	/* there is in 'sir_convert_assoc_req_frame2_struct'? */

	/* WSC IE */
	if (ar.WscIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_wsc_opaque(pMac, &pAssocReq->addIE, &ar.WscIEOpaque);
	}

	if (ar.P2PIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_p2p_opaque(pMac, &pAssocReq->addIE, &ar.P2PIEOpaque);
	}
#ifdef WLAN_FEATURE_WFD
	if (ar.WFDIEOpaque.present) {
		pAssocReq->addIEPresent = 1;
		convert_wfd_opaque(pMac, &pAssocReq->addIE, &ar.WFDIEOpaque);
	}
#endif

	if (ar.VHTCaps.present) {
		qdf_mem_copy(&pAssocReq->VHTCaps, &ar.VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	if (ar.OperatingMode.present) {
		qdf_mem_copy(&pAssocReq->operMode, &ar.OperatingMode,
			     sizeof(tDot11fIEOperatingMode));
		pe_warn("Received Assoc Req with Operating Mode IE");
		lim_log_operating_mode(pMac, &pAssocReq->operMode);
	}
	if (ar.ExtCap.present) {
		struct s_ext_cap *ext_cap;

		qdf_mem_copy(&pAssocReq->ExtCap, &ar.ExtCap,
			     sizeof(tDot11fIEExtCap));
		ext_cap = (struct s_ext_cap *)&pAssocReq->ExtCap.bytes;
		pe_debug("timingMeas: %d, finetimingMeas Init: %d, Resp: %d",
			ext_cap->timing_meas, ext_cap->fine_time_meas_initiator,
			ext_cap->fine_time_meas_responder);
	}
	if (ar.he_cap.present) {
		qdf_mem_copy(&pAssocReq->he_cap, &ar.he_cap,
			     sizeof(tDot11fIEhe_cap));
	}

	return eSIR_SUCCESS;

} /* End sir_convert_reassoc_req_frame2_struct. */

#ifdef FEATURE_WLAN_ESE
tSirRetStatus
sir_beacon_ie_ese_bcn_report(tpAniSirGlobal pMac,
	uint8_t *pPayload, const uint32_t nPayload,
	uint8_t **outIeBuf, uint32_t *pOutIeLen)
{
	tDot11fBeaconIEs *pBies = NULL;
	uint32_t status = QDF_STATUS_SUCCESS;
	tSirRetStatus retStatus = eSIR_SUCCESS;
	tSirEseBcnReportMandatoryIe eseBcnReportMandatoryIe;

	/* To store how many bytes are required to be allocated
	   for Bcn report mandatory Ies */
	uint16_t numBytes = 0, freeBytes = 0;
	uint8_t *pos = NULL;

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) &eseBcnReportMandatoryIe,
		    sizeof(eseBcnReportMandatoryIe), 0);
	pBies = qdf_mem_malloc(sizeof(tDot11fBeaconIEs));
	if (NULL == pBies) {
		pe_err("Failed to allocate memory");
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_zero(pBies, sizeof(tDot11fBeaconIEs));
	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_beacon_i_es(pMac, pPayload, nPayload,
					   pBies, false);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse Beacon IEs (0x%08x, %d bytes):",
			status, nPayload);
		qdf_mem_free(pBies);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking Beacon IEs (0x%08x, %d bytes):",
			status, nPayload);
	}
	/* & "transliterate" from a 'tDot11fBeaconIEs' to a 'eseBcnReportMandatoryIe'... */
	if (!pBies->SSID.present) {
		pe_debug("Mandatory IE SSID not present!");
	} else {
		eseBcnReportMandatoryIe.ssidPresent = 1;
		convert_ssid(pMac, &eseBcnReportMandatoryIe.ssId, &pBies->SSID);
		/* 1 for EID, 1 for length and length bytes */
		numBytes += 1 + 1 + eseBcnReportMandatoryIe.ssId.length;
	}

	if (!pBies->SuppRates.present) {
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
	} else {
		eseBcnReportMandatoryIe.suppRatesPresent = 1;
		convert_supp_rates(pMac, &eseBcnReportMandatoryIe.supportedRates,
				   &pBies->SuppRates);
		numBytes +=
			1 + 1 + eseBcnReportMandatoryIe.supportedRates.numRates;
	}

	if (pBies->FHParamSet.present) {
		eseBcnReportMandatoryIe.fhParamPresent = 1;
		convert_fh_params(pMac, &eseBcnReportMandatoryIe.fhParamSet,
				  &pBies->FHParamSet);
		numBytes += 1 + 1 + SIR_MAC_FH_PARAM_SET_EID_MAX;
	}

	if (pBies->DSParams.present) {
		eseBcnReportMandatoryIe.dsParamsPresent = 1;
		eseBcnReportMandatoryIe.dsParamSet.channelNumber =
			pBies->DSParams.curr_channel;
		numBytes += 1 + 1 + SIR_MAC_DS_PARAM_SET_EID_MAX;
	}

	if (pBies->CFParams.present) {
		eseBcnReportMandatoryIe.cfPresent = 1;
		convert_cf_params(pMac, &eseBcnReportMandatoryIe.cfParamSet,
				  &pBies->CFParams);
		numBytes += 1 + 1 + SIR_MAC_CF_PARAM_SET_EID_MAX;
	}

	if (pBies->IBSSParams.present) {
		eseBcnReportMandatoryIe.ibssParamPresent = 1;
		eseBcnReportMandatoryIe.ibssParamSet.atim =
			pBies->IBSSParams.atim;
		numBytes += 1 + 1 + SIR_MAC_IBSS_PARAM_SET_EID_MAX;
	}

	if (pBies->TIM.present) {
		eseBcnReportMandatoryIe.timPresent = 1;
		eseBcnReportMandatoryIe.tim.dtimCount = pBies->TIM.dtim_count;
		eseBcnReportMandatoryIe.tim.dtimPeriod = pBies->TIM.dtim_period;
		eseBcnReportMandatoryIe.tim.bitmapControl = pBies->TIM.bmpctl;
		/* As per the ESE spec, May truncate and report first 4 octets only */
		numBytes += 1 + 1 + SIR_MAC_TIM_EID_MIN;
	}

	if (pBies->RRMEnabledCap.present) {
		eseBcnReportMandatoryIe.rrmPresent = 1;
		qdf_mem_copy(&eseBcnReportMandatoryIe.rmEnabledCapabilities,
			     &pBies->RRMEnabledCap,
			     sizeof(tDot11fIERRMEnabledCap));
		numBytes += 1 + 1 + SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX;
	}

	*outIeBuf = qdf_mem_malloc(numBytes);
	if (NULL == *outIeBuf) {
		pe_err("Memory Allocation failure");
		qdf_mem_free(pBies);
		return eSIR_MEM_ALLOC_FAILED;
	}
	pos = *outIeBuf;
	*pOutIeLen = numBytes;
	freeBytes = numBytes;

	/* Start filling the output Ie with Mandatory IE information */
	/* Fill SSID IE */
	if (eseBcnReportMandatoryIe.ssidPresent) {
		if (freeBytes < (1 + 1 + eseBcnReportMandatoryIe.ssId.length)) {
			pe_err("Insufficient memory to copy SSID");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_SSID_EID;
		pos++;
		*pos = eseBcnReportMandatoryIe.ssId.length;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) eseBcnReportMandatoryIe.ssId.ssId,
			     eseBcnReportMandatoryIe.ssId.length);
		pos += eseBcnReportMandatoryIe.ssId.length;
		freeBytes -= (1 + 1 + eseBcnReportMandatoryIe.ssId.length);
	}

	/* Fill Supported Rates IE */
	if (eseBcnReportMandatoryIe.suppRatesPresent) {
		if (freeBytes <
		    (1 + 1 + eseBcnReportMandatoryIe.supportedRates.numRates)) {
			pe_err("Insufficient memory to copy Rates IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		if (eseBcnReportMandatoryIe.supportedRates.numRates <=
			SIR_MAC_RATESET_EID_MAX) {
			*pos = SIR_MAC_RATESET_EID;
			pos++;
			*pos = eseBcnReportMandatoryIe.supportedRates.numRates;
			pos++;
			qdf_mem_copy(pos,
			     (uint8_t *) eseBcnReportMandatoryIe.supportedRates.
			     rate,
			     eseBcnReportMandatoryIe.supportedRates.numRates);
			pos += eseBcnReportMandatoryIe.supportedRates.numRates;
			freeBytes -=
			(1 + 1 +
			 eseBcnReportMandatoryIe.supportedRates.numRates);
		}
	}

	/* Fill FH Parameter set IE */
	if (eseBcnReportMandatoryIe.fhParamPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_FH_PARAM_SET_EID_MAX)) {
			pe_err("Insufficient memory to copy FHIE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_FH_PARAM_SET_EID;
		pos++;
		*pos = SIR_MAC_FH_PARAM_SET_EID_MAX;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) &eseBcnReportMandatoryIe.fhParamSet,
			     SIR_MAC_FH_PARAM_SET_EID_MAX);
		pos += SIR_MAC_FH_PARAM_SET_EID_MAX;
		freeBytes -= (1 + 1 + SIR_MAC_FH_PARAM_SET_EID_MAX);
	}

	/* Fill DS Parameter set IE */
	if (eseBcnReportMandatoryIe.dsParamsPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_DS_PARAM_SET_EID_MAX)) {
			pe_err("Insufficient memory to copy DS IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_DS_PARAM_SET_EID;
		pos++;
		*pos = SIR_MAC_DS_PARAM_SET_EID_MAX;
		pos++;
		*pos = eseBcnReportMandatoryIe.dsParamSet.channelNumber;
		pos += SIR_MAC_DS_PARAM_SET_EID_MAX;
		freeBytes -= (1 + 1 + SIR_MAC_DS_PARAM_SET_EID_MAX);
	}

	/* Fill CF Parameter set */
	if (eseBcnReportMandatoryIe.cfPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_CF_PARAM_SET_EID_MAX)) {
			pe_err("Insufficient memory to copy CF IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_CF_PARAM_SET_EID;
		pos++;
		*pos = SIR_MAC_CF_PARAM_SET_EID_MAX;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) &eseBcnReportMandatoryIe.cfParamSet,
			     SIR_MAC_CF_PARAM_SET_EID_MAX);
		pos += SIR_MAC_CF_PARAM_SET_EID_MAX;
		freeBytes -= (1 + 1 + SIR_MAC_CF_PARAM_SET_EID_MAX);
	}

	/* Fill IBSS Parameter set IE */
	if (eseBcnReportMandatoryIe.ibssParamPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_IBSS_PARAM_SET_EID_MAX)) {
			pe_err("Insufficient memory to copy IBSS IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_IBSS_PARAM_SET_EID;
		pos++;
		*pos = SIR_MAC_IBSS_PARAM_SET_EID_MAX;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) &eseBcnReportMandatoryIe.ibssParamSet.
			     atim, SIR_MAC_IBSS_PARAM_SET_EID_MAX);
		pos += SIR_MAC_IBSS_PARAM_SET_EID_MAX;
		freeBytes -= (1 + 1 + SIR_MAC_IBSS_PARAM_SET_EID_MAX);
	}

	/* Fill TIM IE */
	if (eseBcnReportMandatoryIe.timPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_TIM_EID_MIN)) {
			pe_err("Insufficient memory to copy TIM IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_TIM_EID;
		pos++;
		*pos = SIR_MAC_TIM_EID_MIN;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) &eseBcnReportMandatoryIe.tim,
			     SIR_MAC_TIM_EID_MIN);
		pos += SIR_MAC_TIM_EID_MIN;
		freeBytes -= (1 + 1 + SIR_MAC_TIM_EID_MIN);
	}

	/* Fill RM Capability IE */
	if (eseBcnReportMandatoryIe.rrmPresent) {
		if (freeBytes < (1 + 1 + SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX)) {
			pe_err("Insufficient memory to copy RRM IE");
			retStatus = eSIR_FAILURE;
			goto err_bcnrep;
		}
		*pos = SIR_MAC_RM_ENABLED_CAPABILITY_EID;
		pos++;
		*pos = SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX;
		pos++;
		qdf_mem_copy(pos,
			     (uint8_t *) &eseBcnReportMandatoryIe.
			     rmEnabledCapabilities,
			     SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX);
		freeBytes -= (1 + 1 + SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX);
	}

	if (freeBytes != 0) {
		pe_err("Mismatch in allocation and copying of IE in Bcn Rep");
		retStatus = eSIR_FAILURE;
	}

err_bcnrep:
	/* The message counter would not be incremented in case of
	 * returning failure and hence next time, this function gets
	 * called, it would be using the same msg ctr for a different
	 * BSS.So, it is good to clear the memory allocated for a BSS
	 * that is returning failure.On success, the caller would take
	 * care of freeing up the memory*/
	if (retStatus == eSIR_FAILURE) {
		qdf_mem_free(*outIeBuf);
		*outIeBuf = NULL;
	}

	qdf_mem_free(pBies);
	return retStatus;
}

#endif /* FEATURE_WLAN_ESE */

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
static void update_bss_color_change_from_beacon_ies(tDot11fBeaconIEs *bcn_ies,
		tpSirProbeRespBeacon bcn_struct)
{
	if (bcn_ies->bss_color_change.present) {
		qdf_mem_copy(&bcn_struct->vendor_he_bss_color_change,
			     &bcn_ies->bss_color_change,
			     sizeof(tDot11fIEbss_color_change));
	}
}
#else
static inline void update_bss_color_change_from_beacon_ies(
		tDot11fBeaconIEs *bcn_ies,
		tpSirProbeRespBeacon bcn_struct)
{}
#endif

tSirRetStatus
sir_parse_beacon_ie(tpAniSirGlobal pMac,
		    tpSirProbeRespBeacon pBeaconStruct,
		    uint8_t *pPayload, uint32_t nPayload)
{
	tDot11fBeaconIEs *pBies;
	uint32_t status;

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pBeaconStruct, sizeof(tSirProbeRespBeacon), 0);

	pBies = qdf_mem_malloc(sizeof(tDot11fBeaconIEs));
	if (NULL == pBies) {
		pe_err("Failed to allocate memory");
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_zero(pBies, sizeof(tDot11fBeaconIEs));
	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_beacon_i_es(pMac, pPayload, nPayload,
					   pBies, false);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse Beacon IEs (0x%08x, %d bytes):",
			status, nPayload);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pPayload, nPayload);
		qdf_mem_free(pBies);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking Beacon IEs (0x%08x, %d bytes):",
			status, nPayload);
	}
	/* & "transliterate" from a 'tDot11fBeaconIEs' to a 'tSirProbeRespBeacon'... */
	if (!pBies->SSID.present) {
		pe_debug("Mandatory IE SSID not present!");
	} else {
		pBeaconStruct->ssidPresent = 1;
		convert_ssid(pMac, &pBeaconStruct->ssId, &pBies->SSID);
	}

	if (!pBies->SuppRates.present) {
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
	} else {
		pBeaconStruct->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pBeaconStruct->supportedRates,
				   &pBies->SuppRates);
	}

	if (pBies->ExtSuppRates.present) {
		pBeaconStruct->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pBeaconStruct->extendedRates,
				       &pBies->ExtSuppRates);
	}

	if (pBies->CFParams.present) {
		pBeaconStruct->cfPresent = 1;
		convert_cf_params(pMac, &pBeaconStruct->cfParamSet,
				  &pBies->CFParams);
	}

	if (pBies->TIM.present) {
		pBeaconStruct->timPresent = 1;
		convert_tim(pMac, &pBeaconStruct->tim, &pBies->TIM);
	}

	if (pBies->Country.present) {
		pBeaconStruct->countryInfoPresent = 1;
		convert_country(pMac, &pBeaconStruct->countryInfoParam,
				&pBies->Country);
	}
	/* 11h IEs */
	if (pBies->TPCReport.present) {
		pBeaconStruct->tpcReportPresent = 1;
		qdf_mem_copy(&pBeaconStruct->tpcReport,
			     &pBies->TPCReport, sizeof(tDot11fIETPCReport));
	}

	if (pBies->PowerConstraints.present) {
		pBeaconStruct->powerConstraintPresent = 1;
		qdf_mem_copy(&pBeaconStruct->localPowerConstraint,
			     &pBies->PowerConstraints,
			     sizeof(tDot11fIEPowerConstraints));
	}
#ifdef FEATURE_WLAN_ESE
	if (pBies->ESEVersion.present)
		pBeaconStruct->is_ese_ver_ie_present = 1;
	if (pBies->ESETxmitPower.present) {
		pBeaconStruct->eseTxPwr.present = 1;
		pBeaconStruct->eseTxPwr.power_limit =
			pBies->ESETxmitPower.power_limit;
	}
	if (pBies->QBSSLoad.present) {
		qdf_mem_copy(&pBeaconStruct->QBSSLoad, &pBies->QBSSLoad,
			     sizeof(tDot11fIEQBSSLoad));
	}
#endif

	if (pBies->EDCAParamSet.present) {
		pBeaconStruct->edcaPresent = 1;
		convert_edca_param(pMac, &pBeaconStruct->edcaParams,
				   &pBies->EDCAParamSet);
	}
	/* QOS Capabilities: */
	if (pBies->QOSCapsAp.present) {
		pBeaconStruct->qosCapabilityPresent = 1;
		convert_qos_caps(pMac, &pBeaconStruct->qosCapability,
				 &pBies->QOSCapsAp);
	}

	if (pBies->ChanSwitchAnn.present) {
		pBeaconStruct->channelSwitchPresent = 1;
		qdf_mem_copy(&pBeaconStruct->channelSwitchIE,
			     &pBies->ChanSwitchAnn,
			     sizeof(pBeaconStruct->channelSwitchIE));
	}

	if (pBies->SuppOperatingClasses.present) {
		pBeaconStruct->supp_operating_class_present = 1;
		qdf_mem_copy(&pBeaconStruct->supp_operating_classes,
			&pBies->SuppOperatingClasses,
			sizeof(tDot11fIESuppOperatingClasses));
	}

	if (pBies->ext_chan_switch_ann.present) {
		pBeaconStruct->ext_chan_switch_present = 1;
		qdf_mem_copy(&pBeaconStruct->ext_chan_switch,
			     &pBies->ext_chan_switch_ann,
			     sizeof(tDot11fIEext_chan_switch_ann));
	}

	if (pBies->sec_chan_offset_ele.present) {
		pBeaconStruct->sec_chan_offset_present = 1;
		qdf_mem_copy(&pBeaconStruct->sec_chan_offset,
			     &pBies->sec_chan_offset_ele,
			     sizeof(pBeaconStruct->sec_chan_offset));
	}

	if (pBies->Quiet.present) {
		pBeaconStruct->quietIEPresent = 1;
		qdf_mem_copy(&pBeaconStruct->quietIE, &pBies->Quiet,
			     sizeof(tDot11fIEQuiet));
	}

	if (pBies->HTCaps.present) {
		qdf_mem_copy(&pBeaconStruct->HTCaps, &pBies->HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (pBies->HTInfo.present) {
		qdf_mem_copy(&pBeaconStruct->HTInfo, &pBies->HTInfo,
			     sizeof(tDot11fIEHTInfo));
	}

	if (pBies->DSParams.present) {
		pBeaconStruct->dsParamsPresent = 1;
		pBeaconStruct->channelNumber = pBies->DSParams.curr_channel;
	} else if (pBies->HTInfo.present) {
		pBeaconStruct->channelNumber = pBies->HTInfo.primaryChannel;
	}

	if (pBies->RSN.present) {
		pBeaconStruct->rsnPresent = 1;
		convert_rsn(pMac, &pBeaconStruct->rsn, &pBies->RSN);
	}

	if (pBies->WPA.present) {
		pBeaconStruct->wpaPresent = 1;
		convert_wpa(pMac, &pBeaconStruct->wpa, &pBies->WPA);
	}

	if (pBies->WMMParams.present) {
		pBeaconStruct->wmeEdcaPresent = 1;
		convert_wmm_params(pMac, &pBeaconStruct->edcaParams,
				   &pBies->WMMParams);
	}

	if (pBies->WMMInfoAp.present) {
		pBeaconStruct->wmeInfoPresent = 1;
	}

	if (pBies->WMMCaps.present) {
		pBeaconStruct->wsmCapablePresent = 1;
	}

	if (pBies->ERPInfo.present) {
		pBeaconStruct->erpPresent = 1;
		convert_erp_info(pMac, &pBeaconStruct->erpIEInfo,
				 &pBies->ERPInfo);
	}
	if (pBies->VHTCaps.present) {
		pBeaconStruct->VHTCaps.present = 1;
		qdf_mem_copy(&pBeaconStruct->VHTCaps, &pBies->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	if (pBies->VHTOperation.present) {
		pBeaconStruct->VHTOperation.present = 1;
		qdf_mem_copy(&pBeaconStruct->VHTOperation, &pBies->VHTOperation,
			     sizeof(tDot11fIEVHTOperation));
	}
	if (pBies->VHTExtBssLoad.present) {
		pBeaconStruct->VHTExtBssLoad.present = 1;
		qdf_mem_copy(&pBeaconStruct->VHTExtBssLoad,
			     &pBies->VHTExtBssLoad,
			     sizeof(tDot11fIEVHTExtBssLoad));
	}
	if (pBies->OperatingMode.present) {
		pBeaconStruct->OperatingMode.present = 1;
		qdf_mem_copy(&pBeaconStruct->OperatingMode,
			     &pBies->OperatingMode,
			     sizeof(tDot11fIEOperatingMode));
	}
	if (pBies->MobilityDomain.present) {
		pBeaconStruct->mdiePresent = 1;
		qdf_mem_copy(pBeaconStruct->mdie, &pBies->MobilityDomain.MDID,
			     SIR_MDIE_SIZE);
	}

	pBeaconStruct->Vendor1IEPresent = pBies->Vendor1IE.present;
	pBeaconStruct->Vendor3IEPresent = pBies->Vendor3IE.present;
	pBeaconStruct->vendor_vht_ie.present = pBies->vendor_vht_ie.present;
	if (pBies->vendor_vht_ie.present)
		pBeaconStruct->vendor_vht_ie.sub_type =
						pBies->vendor_vht_ie.sub_type;

	if (pBies->vendor_vht_ie.VHTCaps.present) {
		pBeaconStruct->vendor_vht_ie.VHTCaps.present = 1;
		qdf_mem_copy(&pBeaconStruct->vendor_vht_ie.VHTCaps,
				&pBies->vendor_vht_ie.VHTCaps,
				sizeof(tDot11fIEVHTCaps));
	}
	if (pBies->vendor_vht_ie.VHTOperation.present) {
		pBeaconStruct->vendor_vht_ie.VHTOperation.present = 1;
		qdf_mem_copy(&pBeaconStruct->vendor_vht_ie.VHTOperation,
				&pBies->vendor_vht_ie.VHTOperation,
				sizeof(tDot11fIEVHTOperation));
	}
	if (pBies->ExtCap.present) {
		qdf_mem_copy(&pBeaconStruct->ext_cap, &pBies->ExtCap,
				sizeof(tDot11fIEExtCap));
	}
	/* Update HS 2.0 Information Element */
	if (pBies->hs20vendor_ie.present) {
		pe_debug("HS20 Indication Element Present, rel#:%u, id:%u",
			pBies->hs20vendor_ie.release_num,
			pBies->hs20vendor_ie.hs_id_present);
		qdf_mem_copy(&pBeaconStruct->hs20vendor_ie,
			&pBies->hs20vendor_ie,
			sizeof(tDot11fIEhs20vendor_ie) -
			sizeof(pBies->hs20vendor_ie.hs_id));
		if (pBies->hs20vendor_ie.hs_id_present)
			qdf_mem_copy(&pBeaconStruct->hs20vendor_ie.hs_id,
				&pBies->hs20vendor_ie.hs_id,
				sizeof(pBies->hs20vendor_ie.hs_id));
	}

	if (pBies->MBO_IE.present) {
		pBeaconStruct->MBO_IE_present = true;
		if (pBies->MBO_IE.cellular_data_cap.present)
			pBeaconStruct->MBO_capability =
				pBies->MBO_IE.cellular_data_cap.cellular_connectivity;

		if (pBies->MBO_IE.assoc_disallowed.present) {
			pBeaconStruct->assoc_disallowed = true;
			pBeaconStruct->assoc_disallowed_reason =
				pBies->MBO_IE.assoc_disallowed.reason_code;
		}
	}

	if (pBies->QCN_IE.present) {
		pBeaconStruct->QCN_IE.is_present = true;
		if (pBies->QCN_IE.version[0] == QCN_IE_VERSION_SUBATTR_ID) {
			pBeaconStruct->QCN_IE.version
					= pBies->QCN_IE.version[2];
			pBeaconStruct->QCN_IE.sub_version
					= pBies->QCN_IE.version[3];
		}
	}

	if (pBies->he_cap.present) {
		qdf_mem_copy(&pBeaconStruct->he_cap, &pBies->he_cap,
			     sizeof(tDot11fIEhe_cap));
	}
	if (pBies->he_op.present) {
		qdf_mem_copy(&pBeaconStruct->he_op, &pBies->he_op,
			     sizeof(tDot11fIEhe_op));
	}

	update_bss_color_change_from_beacon_ies(pBies, pBeaconStruct);

	qdf_mem_free(pBies);
	return eSIR_SUCCESS;
} /* End sir_parse_beacon_ie. */

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
static void convert_bcon_bss_color_change_ie(tDot11fBeacon *bcn_frm,
		tpSirProbeRespBeacon bcn_struct)
{
	if (bcn_frm->bss_color_change.present) {
		pe_debug("11AX: HE BSS color change present");
		qdf_mem_copy(&bcn_struct->vendor_he_bss_color_change,
			     &bcn_frm->bss_color_change,
			     sizeof(tDot11fIEbss_color_change));
	}
}
#else
static inline void convert_bcon_bss_color_change_ie(tDot11fBeacon *bcn_frm,
		tpSirProbeRespBeacon bcn_struct)
{}
#endif

tSirRetStatus
sir_convert_beacon_frame2_struct(tpAniSirGlobal pMac,
				 uint8_t *pFrame,
				 tpSirProbeRespBeacon pBeaconStruct)
{
	tDot11fBeacon *pBeacon;
	uint32_t status, nPayload;
	uint8_t *pPayload;
	tpSirMacMgmtHdr pHdr;
	uint8_t mappedRXCh;
	uint8_t rfBand;

	pPayload = WMA_GET_RX_MPDU_DATA(pFrame);
	nPayload = WMA_GET_RX_PAYLOAD_LEN(pFrame);
	pHdr = WMA_GET_RX_MAC_HEADER(pFrame);
	mappedRXCh = WMA_GET_RX_CH(pFrame);
	rfBand = WMA_GET_RX_RFBAND(pFrame);

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pBeaconStruct, sizeof(tSirProbeRespBeacon), 0);

	pBeacon = qdf_mem_malloc(sizeof(tDot11fBeacon));
	if (NULL == pBeacon) {
		pe_err("Failed to allocate memory");
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* get the MAC address out of the BD, */
	qdf_mem_copy(pBeaconStruct->bssid, pHdr->sa, 6);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_beacon(pMac, pPayload, nPayload, pBeacon, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse Beacon IEs (0x%08x, %d bytes):",
			status, nPayload);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
				   pPayload, nPayload);
		qdf_mem_free(pBeacon);
		return eSIR_FAILURE;
	}
	/* & "transliterate" from a 'tDot11fBeacon' to a 'tSirProbeRespBeacon'... */
	/* Timestamp */
	qdf_mem_copy((uint8_t *) pBeaconStruct->timeStamp,
		     (uint8_t *) &pBeacon->TimeStamp,
		     sizeof(tSirMacTimeStamp));

	/* Beacon Interval */
	pBeaconStruct->beaconInterval = pBeacon->BeaconInterval.interval;

	/* Capabilities */
	pBeaconStruct->capabilityInfo.ess = pBeacon->Capabilities.ess;
	pBeaconStruct->capabilityInfo.ibss = pBeacon->Capabilities.ibss;
	pBeaconStruct->capabilityInfo.cfPollable =
		pBeacon->Capabilities.cfPollable;
	pBeaconStruct->capabilityInfo.cfPollReq =
		pBeacon->Capabilities.cfPollReq;
	pBeaconStruct->capabilityInfo.privacy = pBeacon->Capabilities.privacy;
	pBeaconStruct->capabilityInfo.shortPreamble =
		pBeacon->Capabilities.shortPreamble;
	pBeaconStruct->capabilityInfo.pbcc = pBeacon->Capabilities.pbcc;
	pBeaconStruct->capabilityInfo.channelAgility =
		pBeacon->Capabilities.channelAgility;
	pBeaconStruct->capabilityInfo.spectrumMgt =
		pBeacon->Capabilities.spectrumMgt;
	pBeaconStruct->capabilityInfo.qos = pBeacon->Capabilities.qos;
	pBeaconStruct->capabilityInfo.shortSlotTime =
		pBeacon->Capabilities.shortSlotTime;
	pBeaconStruct->capabilityInfo.apsd = pBeacon->Capabilities.apsd;
	pBeaconStruct->capabilityInfo.rrm = pBeacon->Capabilities.rrm;
	pBeaconStruct->capabilityInfo.dsssOfdm = pBeacon->Capabilities.dsssOfdm;
	pBeaconStruct->capabilityInfo.delayedBA =
		pBeacon->Capabilities.delayedBA;
	pBeaconStruct->capabilityInfo.immediateBA =
		pBeacon->Capabilities.immediateBA;

	if (!pBeacon->SSID.present) {
		pe_debug("Mandatory IE SSID not present!");
	} else {
		pBeaconStruct->ssidPresent = 1;
		convert_ssid(pMac, &pBeaconStruct->ssId, &pBeacon->SSID);
	}

	if (!pBeacon->SuppRates.present) {
		pe_debug_rate_limited(30,
				"Mandatory IE Supported Rates not present!");
	} else {
		pBeaconStruct->suppRatesPresent = 1;
		convert_supp_rates(pMac, &pBeaconStruct->supportedRates,
				   &pBeacon->SuppRates);
	}

	if (pBeacon->ExtSuppRates.present) {
		pBeaconStruct->extendedRatesPresent = 1;
		convert_ext_supp_rates(pMac, &pBeaconStruct->extendedRates,
				       &pBeacon->ExtSuppRates);
	}

	if (pBeacon->CFParams.present) {
		pBeaconStruct->cfPresent = 1;
		convert_cf_params(pMac, &pBeaconStruct->cfParamSet,
				  &pBeacon->CFParams);
	}

	if (pBeacon->TIM.present) {
		pBeaconStruct->timPresent = 1;
		convert_tim(pMac, &pBeaconStruct->tim, &pBeacon->TIM);
	}

	if (pBeacon->Country.present) {
		pBeaconStruct->countryInfoPresent = 1;
		convert_country(pMac, &pBeaconStruct->countryInfoParam,
				&pBeacon->Country);
	}
	/* QOS Capabilities: */
	if (pBeacon->QOSCapsAp.present) {
		pBeaconStruct->qosCapabilityPresent = 1;
		convert_qos_caps(pMac, &pBeaconStruct->qosCapability,
				 &pBeacon->QOSCapsAp);
	}

	if (pBeacon->EDCAParamSet.present) {
		pBeaconStruct->edcaPresent = 1;
		convert_edca_param(pMac, &pBeaconStruct->edcaParams,
				   &pBeacon->EDCAParamSet);
	}

	if (pBeacon->ChanSwitchAnn.present) {
		pBeaconStruct->channelSwitchPresent = 1;
		qdf_mem_copy(&pBeaconStruct->channelSwitchIE,
			     &pBeacon->ChanSwitchAnn,
			     sizeof(pBeaconStruct->channelSwitchIE));
	}

	if (pBeacon->ext_chan_switch_ann.present) {
		pBeaconStruct->ext_chan_switch_present = 1;
		qdf_mem_copy(&pBeaconStruct->ext_chan_switch,
			     &pBeacon->ext_chan_switch_ann,
			     sizeof(tDot11fIEext_chan_switch_ann));
	}

	if (pBeacon->sec_chan_offset_ele.present) {
		pBeaconStruct->sec_chan_offset_present = 1;
		qdf_mem_copy(&pBeaconStruct->sec_chan_offset,
			     &pBeacon->sec_chan_offset_ele,
			     sizeof(pBeaconStruct->sec_chan_offset));
	}

	if (pBeacon->TPCReport.present) {
		pBeaconStruct->tpcReportPresent = 1;
		qdf_mem_copy(&pBeaconStruct->tpcReport, &pBeacon->TPCReport,
			     sizeof(tDot11fIETPCReport));
	}

	if (pBeacon->PowerConstraints.present) {
		pBeaconStruct->powerConstraintPresent = 1;
		qdf_mem_copy(&pBeaconStruct->localPowerConstraint,
			     &pBeacon->PowerConstraints,
			     sizeof(tDot11fIEPowerConstraints));
	}

	if (pBeacon->Quiet.present) {
		pBeaconStruct->quietIEPresent = 1;
		qdf_mem_copy(&pBeaconStruct->quietIE, &pBeacon->Quiet,
			     sizeof(tDot11fIEQuiet));
	}

	if (pBeacon->HTCaps.present) {
		qdf_mem_copy(&pBeaconStruct->HTCaps, &pBeacon->HTCaps,
			     sizeof(tDot11fIEHTCaps));
	}

	if (pBeacon->HTInfo.present) {
		qdf_mem_copy(&pBeaconStruct->HTInfo, &pBeacon->HTInfo,
			     sizeof(tDot11fIEHTInfo));

	}

	if (pBeacon->DSParams.present) {
		pBeaconStruct->dsParamsPresent = 1;
		pBeaconStruct->channelNumber = pBeacon->DSParams.curr_channel;
	} else if (pBeacon->HTInfo.present) {
		pBeaconStruct->channelNumber = pBeacon->HTInfo.primaryChannel;
	} else {
		pBeaconStruct->channelNumber = mappedRXCh;
		pe_debug_rate_limited(30, "In Beacon No Channel info");
	}

	if (pBeacon->RSN.present) {
		pBeaconStruct->rsnPresent = 1;
		convert_rsn(pMac, &pBeaconStruct->rsn, &pBeacon->RSN);
	}

	if (pBeacon->WPA.present) {
		pBeaconStruct->wpaPresent = 1;
		convert_wpa(pMac, &pBeaconStruct->wpa, &pBeacon->WPA);
	}

	if (pBeacon->WMMParams.present) {
		pBeaconStruct->wmeEdcaPresent = 1;
		convert_wmm_params(pMac, &pBeaconStruct->edcaParams,
				   &pBeacon->WMMParams);
	}

	if (pBeacon->WMMInfoAp.present) {
		pBeaconStruct->wmeInfoPresent = 1;
		pe_debug("WMM Info present in Beacon Frame!");
	}

	if (pBeacon->WMMCaps.present) {
		pBeaconStruct->wsmCapablePresent = 1;
	}

	if (pBeacon->ERPInfo.present) {
		pBeaconStruct->erpPresent = 1;
		convert_erp_info(pMac, &pBeaconStruct->erpIEInfo,
				 &pBeacon->ERPInfo);
	}
	if (pBeacon->MobilityDomain.present) {
		/* MobilityDomain */
		pBeaconStruct->mdiePresent = 1;
		qdf_mem_copy((uint8_t *) &(pBeaconStruct->mdie[0]),
			     (uint8_t *) &(pBeacon->MobilityDomain.MDID),
			     sizeof(uint16_t));
		pBeaconStruct->mdie[2] =
			((pBeacon->MobilityDomain.overDSCap << 0) | (pBeacon->
								     MobilityDomain.
								     resourceReqCap
								     << 1));

	}

#ifdef FEATURE_WLAN_ESE
	if (pBeacon->ESEVersion.present)
		pBeaconStruct->is_ese_ver_ie_present = 1;
	if (pBeacon->ESETxmitPower.present) {
		/* copy ESE TPC info element */
		pBeaconStruct->eseTxPwr.present = 1;
		qdf_mem_copy(&pBeaconStruct->eseTxPwr,
			     &pBeacon->ESETxmitPower,
			     sizeof(tDot11fIEESETxmitPower));
	}
	if (pBeacon->QBSSLoad.present) {
		qdf_mem_copy(&pBeaconStruct->QBSSLoad,
			     &pBeacon->QBSSLoad, sizeof(tDot11fIEQBSSLoad));
	}
#endif
	if (pBeacon->VHTCaps.present) {
		qdf_mem_copy(&pBeaconStruct->VHTCaps, &pBeacon->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	if (pBeacon->VHTOperation.present) {
		qdf_mem_copy(&pBeaconStruct->VHTOperation,
			     &pBeacon->VHTOperation,
			     sizeof(tDot11fIEVHTOperation));
	}
	if (pBeacon->VHTExtBssLoad.present) {
		qdf_mem_copy(&pBeaconStruct->VHTExtBssLoad,
			     &pBeacon->VHTExtBssLoad,
			     sizeof(tDot11fIEVHTExtBssLoad));
	}
	if (pBeacon->OperatingMode.present) {
		qdf_mem_copy(&pBeaconStruct->OperatingMode,
			     &pBeacon->OperatingMode,
			     sizeof(tDot11fIEOperatingMode));
	}
	if (pBeacon->WiderBWChanSwitchAnn.present) {
		pBeaconStruct->WiderBWChanSwitchAnnPresent = 1;
		qdf_mem_copy(&pBeaconStruct->WiderBWChanSwitchAnn,
			     &pBeacon->WiderBWChanSwitchAnn,
			     sizeof(tDot11fIEWiderBWChanSwitchAnn));
	}
	/* IBSS Peer Params */
	if (pBeacon->IBSSParams.present) {
		pBeaconStruct->IBSSParams.present = 1;
		qdf_mem_copy(&pBeaconStruct->IBSSParams, &pBeacon->IBSSParams,
			     sizeof(tDot11fIEIBSSParams));
	}

	pBeaconStruct->Vendor1IEPresent = pBeacon->Vendor1IE.present;
	pBeaconStruct->Vendor3IEPresent = pBeacon->Vendor3IE.present;

	pBeaconStruct->vendor_vht_ie.present = pBeacon->vendor_vht_ie.present;
	if (pBeacon->vendor_vht_ie.present) {
		pBeaconStruct->vendor_vht_ie.sub_type =
			pBeacon->vendor_vht_ie.sub_type;
	}

	if (pBeacon->vendor_vht_ie.VHTCaps.present) {
		qdf_mem_copy(&pBeaconStruct->vendor_vht_ie.VHTCaps,
				&pBeacon->vendor_vht_ie.VHTCaps,
				sizeof(tDot11fIEVHTCaps));
	}
	if (pBeacon->vendor_vht_ie.VHTOperation.present) {
		qdf_mem_copy(&pBeaconStruct->vendor_vht_ie.VHTOperation,
				&pBeacon->VHTOperation,
				sizeof(tDot11fIEVHTOperation));
	}
	/* Update HS 2.0 Information Element */
	if (pBeacon->hs20vendor_ie.present) {
		qdf_mem_copy(&pBeaconStruct->hs20vendor_ie,
			&pBeacon->hs20vendor_ie,
			sizeof(tDot11fIEhs20vendor_ie) -
			sizeof(pBeacon->hs20vendor_ie.hs_id));
		if (pBeacon->hs20vendor_ie.hs_id_present)
			qdf_mem_copy(&pBeaconStruct->hs20vendor_ie.hs_id,
				&pBeacon->hs20vendor_ie.hs_id,
				sizeof(pBeacon->hs20vendor_ie.hs_id));
	}
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (pBeacon->QComVendorIE.present) {
		pBeaconStruct->AvoidChannelIE.present =
			pBeacon->QComVendorIE.present;
		pBeaconStruct->AvoidChannelIE.type =
			pBeacon->QComVendorIE.type;
		pBeaconStruct->AvoidChannelIE.channel =
			pBeacon->QComVendorIE.channel;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	if (pBeacon->OBSSScanParameters.present) {
		qdf_mem_copy(&pBeaconStruct->obss_scanparams,
			&pBeacon->OBSSScanParameters,
			sizeof(struct sDot11fIEOBSSScanParameters));
	}
	if (pBeacon->MBO_IE.present) {
		pBeaconStruct->MBO_IE_present = true;
		if (pBeacon->MBO_IE.cellular_data_cap.present)
			pBeaconStruct->MBO_capability =
				pBeacon->MBO_IE.cellular_data_cap.cellular_connectivity;

		if (pBeacon->MBO_IE.assoc_disallowed.present) {
			pBeaconStruct->assoc_disallowed = true;
			pBeaconStruct->assoc_disallowed_reason =
				pBeacon->MBO_IE.assoc_disallowed.reason_code;
		}
	}

	if (pBeacon->QCN_IE.present) {
		pBeaconStruct->QCN_IE.is_present = true;
		if (pBeacon->QCN_IE.version[0]
					== QCN_IE_VERSION_SUBATTR_ID) {
			pBeaconStruct->QCN_IE.version
					= pBeacon->QCN_IE.version[2];
			pBeaconStruct->QCN_IE.sub_version
					= pBeacon->QCN_IE.version[3];
		}
	}

	if (pBeacon->he_cap.present) {
		pe_debug("11AX: HE cap IE present");
		qdf_mem_copy(&pBeaconStruct->he_cap,
			     &pBeacon->he_cap,
			     sizeof(tDot11fIEhe_cap));
	}
	if (pBeacon->he_op.present) {
		pe_debug("11AX: HE operation IE present");
		qdf_mem_copy(&pBeaconStruct->he_op,
			     &pBeacon->he_op,
			     sizeof(tDot11fIEhe_op));
	}

	convert_bcon_bss_color_change_ie(pBeacon, pBeaconStruct);

	qdf_mem_free(pBeacon);
	return eSIR_SUCCESS;

} /* End sir_convert_beacon_frame2_struct. */

#ifdef WLAN_FEATURE_FILS_SK
/* sir_update_auth_frame2_struct_fils_conf: API to update fils info from auth
 * packet type 2
 * @auth: auth packet pointer received from AP
 * @auth_frame: data structure needs to be updated
 *
 * Return: None
 */
static void sir_update_auth_frame2_struct_fils_conf(tDot11fAuthentication *auth,
				tpSirMacAuthFrameBody auth_frame)
{
	if (auth->AuthAlgo.algo != SIR_FILS_SK_WITHOUT_PFS)
		return;

	if (auth->fils_assoc_delay_info.present)
		auth_frame->assoc_delay_info =
			auth->fils_assoc_delay_info.assoc_delay_info;

	if (auth->fils_session.present)
		qdf_mem_copy(auth_frame->session, auth->fils_session.session,
			SIR_FILS_SESSION_LENGTH);

	if (auth->fils_nonce.present)
		qdf_mem_copy(auth_frame->nonce, auth->fils_nonce.nonce,
			SIR_FILS_NONCE_LENGTH);

	if (auth->fils_wrapped_data.present) {
		qdf_mem_copy(auth_frame->wrapped_data,
			auth->fils_wrapped_data.wrapped_data,
			auth->fils_wrapped_data.num_wrapped_data);
		auth_frame->wrapped_data_len =
			auth->fils_wrapped_data.num_wrapped_data;
	}
	if (auth->RSNOpaque.present) {
		qdf_mem_copy(auth_frame->rsn_ie.info, auth->RSNOpaque.data,
			auth->RSNOpaque.num_data);
		auth_frame->rsn_ie.length = auth->RSNOpaque.num_data;
	}
}
#else
static void sir_update_auth_frame2_struct_fils_conf(tDot11fAuthentication *auth,
				tpSirMacAuthFrameBody auth_frame)
{ }
#endif

tSirRetStatus
sir_convert_auth_frame2_struct(tpAniSirGlobal pMac,
			       uint8_t *pFrame,
			       uint32_t nFrame, tpSirMacAuthFrameBody pAuth)
{
	static tDot11fAuthentication auth;
	uint32_t status;

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pAuth, sizeof(tSirMacAuthFrameBody), 0);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_authentication(pMac, pFrame, nFrame,
					      &auth, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Authentication frame (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking an Authentication frame (0x%08x, %d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fAuthentication' to a 'tSirMacAuthFrameBody'... */
	pAuth->authAlgoNumber = auth.AuthAlgo.algo;
	pAuth->authTransactionSeqNumber = auth.AuthSeqNo.no;
	pAuth->authStatusCode = auth.Status.status;

	if (auth.ChallengeText.present) {
		pAuth->type = SIR_MAC_CHALLENGE_TEXT_EID;
		pAuth->length = auth.ChallengeText.num_text;
		qdf_mem_copy(pAuth->challengeText, auth.ChallengeText.text,
			     auth.ChallengeText.num_text);
	}
	sir_update_auth_frame2_struct_fils_conf(&auth, pAuth);

	return eSIR_SUCCESS;

} /* End sir_convert_auth_frame2_struct. */

tSirRetStatus
sir_convert_addts_req2_struct(tpAniSirGlobal pMac,
			      uint8_t *pFrame,
			      uint32_t nFrame, tSirAddtsReqInfo *pAddTs)
{
	tDot11fAddTSRequest addts = { {0} };
	tDot11fWMMAddTSRequest wmmaddts = { {0} };
	uint8_t j;
	uint16_t i;
	uint32_t status;

	if (SIR_MAC_QOS_ADD_TS_REQ != *(pFrame + 1)) {
		pe_err("sir_convert_addts_req2_struct invoked "
		       "with an Action of %d; this is not "
		       "supported & is probably an error",
			*(pFrame + 1));
		return eSIR_FAILURE;
	}
	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pAddTs, sizeof(tSirAddtsReqInfo), 0);

	/* delegate to the framesc-generated code, */
	switch (*pFrame) {
	case SIR_MAC_ACTION_QOS_MGMT:
		status = dot11f_unpack_add_ts_request(pMac, pFrame, nFrame,
						      &addts, false);
		break;
	case SIR_MAC_ACTION_WME:
		status =
			dot11f_unpack_wmm_add_ts_request(pMac, pFrame, nFrame,
							 &wmmaddts, false);
		break;
	default:
		pe_err("sir_convert_addts_req2_struct invoked "
		       "with a Category of %d; this is not"
		       " supported & is probably an error",
			*pFrame);
		return eSIR_FAILURE;
	}

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Add TS Request frame (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking an Add TS Request frame (0x%08x,%d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fAddTSRequest' or a */
	/* 'tDot11WMMAddTSRequest' to a 'tSirMacAddtsReqInfo'... */
	if (SIR_MAC_ACTION_QOS_MGMT == *pFrame) {
		pAddTs->dialogToken = addts.DialogToken.token;

		if (addts.TSPEC.present) {
			convert_tspec(pMac, &pAddTs->tspec, &addts.TSPEC);
		} else {
			pe_err("Mandatory TSPEC element missing in Add TS Request");
			return eSIR_FAILURE;
		}

		if (addts.num_TCLAS) {
			pAddTs->numTclas = (uint8_t) addts.num_TCLAS;

			for (i = 0U; i < addts.num_TCLAS; ++i) {
				if (eSIR_SUCCESS !=
				    convert_tclas(pMac, &(pAddTs->tclasInfo[i]),
						  &(addts.TCLAS[i]))) {
					pe_err("Failed to convert a TCLAS IE");
					return eSIR_FAILURE;
				}
			}
		}

		if (addts.TCLASSPROC.present) {
			pAddTs->tclasProcPresent = 1;
			pAddTs->tclasProc = addts.TCLASSPROC.processing;
		}

		if (addts.WMMTSPEC.present) {
			pAddTs->wsmTspecPresent = 1;
			convert_wmmtspec(pMac, &pAddTs->tspec, &addts.WMMTSPEC);
		}

		if (addts.num_WMMTCLAS) {
			j = (uint8_t) (pAddTs->numTclas + addts.num_WMMTCLAS);
			if (SIR_MAC_TCLASIE_MAXNUM > j)
				j = SIR_MAC_TCLASIE_MAXNUM;

			for (i = pAddTs->numTclas; i < j; ++i) {
				if (eSIR_SUCCESS !=
				    convert_wmmtclas(pMac,
						     &(pAddTs->tclasInfo[i]),
						     &(addts.WMMTCLAS[i]))) {
					pe_err("Failed to convert a TCLAS IE");
					return eSIR_FAILURE;
				}
			}
		}

		if (addts.WMMTCLASPROC.present) {
			pAddTs->tclasProcPresent = 1;
			pAddTs->tclasProc = addts.WMMTCLASPROC.processing;
		}

		if (1 < pAddTs->numTclas && (!pAddTs->tclasProcPresent)) {
			pe_err("%d TCLAS IE but not TCLASPROC IE",
				pAddTs->numTclas);
			return eSIR_FAILURE;
		}
	} else {
		pAddTs->dialogToken = wmmaddts.DialogToken.token;

		if (wmmaddts.WMMTSPEC.present) {
			pAddTs->wmeTspecPresent = 1;
			convert_wmmtspec(pMac, &pAddTs->tspec,
					 &wmmaddts.WMMTSPEC);
		} else {
			pe_err("Mandatory WME TSPEC element missing!");
			return eSIR_FAILURE;
		}
	}

	return eSIR_SUCCESS;

} /* End sir_convert_addts_req2_struct. */

tSirRetStatus
sir_convert_addts_rsp2_struct(tpAniSirGlobal pMac,
			      uint8_t *pFrame,
			      uint32_t nFrame, tSirAddtsRspInfo *pAddTs)
{
	tDot11fAddTSResponse addts = { {0} };
	tDot11fWMMAddTSResponse wmmaddts = { {0} };
	uint8_t j;
	uint16_t i;
	uint32_t status;

	if (SIR_MAC_QOS_ADD_TS_RSP != *(pFrame + 1)) {
		pe_err("sir_convert_addts_rsp2_struct invoked "
		       "with an Action of %d; this is not "
		       "supported & is probably an error",
			*(pFrame + 1));
		return eSIR_FAILURE;
	}
	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pAddTs, sizeof(tSirAddtsRspInfo), 0);
	qdf_mem_set((uint8_t *) &addts, sizeof(tDot11fAddTSResponse), 0);
	qdf_mem_set((uint8_t *) &wmmaddts, sizeof(tDot11fWMMAddTSResponse), 0);

	/* delegate to the framesc-generated code, */
	switch (*pFrame) {
	case SIR_MAC_ACTION_QOS_MGMT:
		status =
			dot11f_unpack_add_ts_response(pMac, pFrame, nFrame,
						      &addts, false);
		break;
	case SIR_MAC_ACTION_WME:
		status =
			dot11f_unpack_wmm_add_ts_response(pMac, pFrame, nFrame,
							  &wmmaddts, false);
		break;
	default:
		pe_err("sir_convert_addts_rsp2_struct invoked "
		       "with a Category of %d; this is not"
		       " supported & is probably an error",
			*pFrame);
		return eSIR_FAILURE;
	}

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Add TS Response frame (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking an Add TS Response frame (0x%08x,%d bytes):",
			status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fAddTSResponse' or a */
	/* 'tDot11WMMAddTSResponse' to a 'tSirMacAddtsRspInfo'... */
	if (SIR_MAC_ACTION_QOS_MGMT == *pFrame) {
		pAddTs->dialogToken = addts.DialogToken.token;
		pAddTs->status = (tSirMacStatusCodes) addts.Status.status;

		if (addts.TSDelay.present) {
			convert_ts_delay(pMac, &pAddTs->delay, &addts.TSDelay);
		}
		/* TS Delay is present iff status indicates its presence */
		if (eSIR_MAC_TS_NOT_CREATED_STATUS == pAddTs->status
		    && !addts.TSDelay.present) {
			pe_warn("Missing TSDelay IE");
		}

		if (addts.TSPEC.present) {
			convert_tspec(pMac, &pAddTs->tspec, &addts.TSPEC);
		} else {
			pe_err("Mandatory TSPEC element missing in Add TS Response");
			return eSIR_FAILURE;
		}

		if (addts.num_TCLAS) {
			pAddTs->numTclas = (uint8_t) addts.num_TCLAS;

			for (i = 0U; i < addts.num_TCLAS; ++i) {
				if (eSIR_SUCCESS !=
				    convert_tclas(pMac, &(pAddTs->tclasInfo[i]),
						  &(addts.TCLAS[i]))) {
					pe_err("Failed to convert a TCLAS IE");
					return eSIR_FAILURE;
				}
			}
		}

		if (addts.TCLASSPROC.present) {
			pAddTs->tclasProcPresent = 1;
			pAddTs->tclasProc = addts.TCLASSPROC.processing;
		}
#ifdef FEATURE_WLAN_ESE
		if (addts.ESETrafStrmMet.present) {
			pAddTs->tsmPresent = 1;
			qdf_mem_copy(&pAddTs->tsmIE.tsid,
				     &addts.ESETrafStrmMet.tsid,
				     sizeof(tSirMacESETSMIE));
		}
#endif
		if (addts.Schedule.present) {
			pAddTs->schedulePresent = 1;
			convert_schedule(pMac, &pAddTs->schedule,
					 &addts.Schedule);
		}

		if (addts.WMMSchedule.present) {
			pAddTs->schedulePresent = 1;
			convert_wmm_schedule(pMac, &pAddTs->schedule,
					     &addts.WMMSchedule);
		}

		if (addts.WMMTSPEC.present) {
			pAddTs->wsmTspecPresent = 1;
			convert_wmmtspec(pMac, &pAddTs->tspec, &addts.WMMTSPEC);
		}

		if (addts.num_WMMTCLAS) {
			j = (uint8_t) (pAddTs->numTclas + addts.num_WMMTCLAS);
			if (SIR_MAC_TCLASIE_MAXNUM > j)
				j = SIR_MAC_TCLASIE_MAXNUM;

			for (i = pAddTs->numTclas; i < j; ++i) {
				if (eSIR_SUCCESS !=
				    convert_wmmtclas(pMac,
						     &(pAddTs->tclasInfo[i]),
						     &(addts.WMMTCLAS[i]))) {
					pe_err("Failed to convert a TCLAS IE");
					return eSIR_FAILURE;
				}
			}
		}

		if (addts.WMMTCLASPROC.present) {
			pAddTs->tclasProcPresent = 1;
			pAddTs->tclasProc = addts.WMMTCLASPROC.processing;
		}

		if (1 < pAddTs->numTclas && (!pAddTs->tclasProcPresent)) {
			pe_err("%d TCLAS IE but not TCLASPROC IE",
				pAddTs->numTclas);
			return eSIR_FAILURE;
		}
	} else {
		pAddTs->dialogToken = wmmaddts.DialogToken.token;
		pAddTs->status =
			(tSirMacStatusCodes) wmmaddts.StatusCode.statusCode;

		if (wmmaddts.WMMTSPEC.present) {
			pAddTs->wmeTspecPresent = 1;
			convert_wmmtspec(pMac, &pAddTs->tspec,
					 &wmmaddts.WMMTSPEC);
		} else {
			pe_err("Mandatory WME TSPEC element missing!");
			return eSIR_FAILURE;
		}

#ifdef FEATURE_WLAN_ESE
		if (wmmaddts.ESETrafStrmMet.present) {
			pAddTs->tsmPresent = 1;
			qdf_mem_copy(&pAddTs->tsmIE.tsid,
				     &wmmaddts.ESETrafStrmMet.tsid,
				     sizeof(tSirMacESETSMIE));
		}
#endif

	}

	return eSIR_SUCCESS;

} /* End sir_convert_addts_rsp2_struct. */

tSirRetStatus
sir_convert_delts_req2_struct(tpAniSirGlobal pMac,
			      uint8_t *pFrame,
			      uint32_t nFrame, tSirDeltsReqInfo *pDelTs)
{
	tDot11fDelTS delts = { {0} };
	tDot11fWMMDelTS wmmdelts = { {0} };
	uint32_t status;

	if (SIR_MAC_QOS_DEL_TS_REQ != *(pFrame + 1)) {
		pe_err("sirConvertDeltsRsp2Struct invoked "
			"with an Action of %d; this is not "
			"supported & is probably an error",
			*(pFrame + 1));
		return eSIR_FAILURE;
	}
	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pDelTs, sizeof(tSirDeltsReqInfo), 0);

	/* delegate to the framesc-generated code, */
	switch (*pFrame) {
	case SIR_MAC_ACTION_QOS_MGMT:
		status = dot11f_unpack_del_ts(pMac, pFrame, nFrame,
					      &delts, false);
		break;
	case SIR_MAC_ACTION_WME:
		status = dot11f_unpack_wmm_del_ts(pMac, pFrame, nFrame,
						  &wmmdelts, false);
		break;
	default:
		pe_err("sirConvertDeltsRsp2Struct invoked "
		       "with a Category of %d; this is not"
		       " supported & is probably an error",
			*pFrame);
		return eSIR_FAILURE;
	}

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse an Del TS Request frame (0x%08x, %d bytes):",
			status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking an Del TS Request frame (0x%08x,%d bytes):",
			   status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fDelTSResponse' or a */
	/* 'tDot11WMMDelTSResponse' to a 'tSirMacDeltsReqInfo'... */
	if (SIR_MAC_ACTION_QOS_MGMT == *pFrame) {
		pDelTs->tsinfo.traffic.trafficType =
			(uint16_t) delts.TSInfo.traffic_type;
		pDelTs->tsinfo.traffic.tsid = (uint16_t) delts.TSInfo.tsid;
		pDelTs->tsinfo.traffic.direction =
			(uint16_t) delts.TSInfo.direction;
		pDelTs->tsinfo.traffic.accessPolicy =
			(uint16_t) delts.TSInfo.access_policy;
		pDelTs->tsinfo.traffic.aggregation =
			(uint16_t) delts.TSInfo.aggregation;
		pDelTs->tsinfo.traffic.psb = (uint16_t) delts.TSInfo.psb;
		pDelTs->tsinfo.traffic.userPrio =
			(uint16_t) delts.TSInfo.user_priority;
		pDelTs->tsinfo.traffic.ackPolicy =
			(uint16_t) delts.TSInfo.tsinfo_ack_pol;

		pDelTs->tsinfo.schedule.schedule =
			(uint8_t) delts.TSInfo.schedule;
	} else {
		if (wmmdelts.WMMTSPEC.present) {
			pDelTs->wmeTspecPresent = 1;
			convert_wmmtspec(pMac, &pDelTs->tspec,
					 &wmmdelts.WMMTSPEC);
		} else {
			pe_err("Mandatory WME TSPEC element missing!");
			return eSIR_FAILURE;
		}
	}

	return eSIR_SUCCESS;

} /* End sir_convert_delts_req2_struct. */

tSirRetStatus
sir_convert_qos_map_configure_frame2_struct(tpAniSirGlobal pMac,
					    uint8_t *pFrame,
					    uint32_t nFrame,
					    tSirQosMapSet *pQosMapSet)
{
	tDot11fQosMapConfigure mapConfigure;
	uint32_t status;

	status =
		dot11f_unpack_qos_map_configure(pMac, pFrame, nFrame,
						&mapConfigure, false);
	if (DOT11F_FAILED(status) || !mapConfigure.QosMapSet.present) {
		pe_err("Failed to parse Qos Map Configure frame (0x%08x, %d bytes):",
			   status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking Qos Map Configure frame (0x%08x, %d bytes):",
			   status, nFrame);
	}
	pQosMapSet->present = mapConfigure.QosMapSet.present;
	convert_qos_mapset_frame(pMac->hHdd, pQosMapSet, &mapConfigure.QosMapSet);
	lim_log_qos_map_set(pMac, pQosMapSet);
	return eSIR_SUCCESS;
}

#ifdef ANI_SUPPORT_11H
tSirRetStatus
sir_convert_tpc_req_frame2_struct(tpAniSirGlobal pMac,
				  uint8_t *pFrame,
				  tpSirMacTpcReqActionFrame pTpcReqFrame,
				  uint32_t nFrame)
{
	tDot11fTPCRequest req;
	uint32_t status;

	qdf_mem_set((uint8_t *) pTpcReqFrame, sizeof(tSirMacTpcReqActionFrame),
		    0);
	status = dot11f_unpack_tpc_request(pMac, pFrame, nFrame, &req, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse a TPC Request frame (0x%08x, %d bytes):",
			   status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking a TPC Request frame (0x%08x, %d bytes):",
			   status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fTPCRequest' to a */
	/* 'tSirMacTpcReqActionFrame'... */
	pTpcReqFrame->actionHeader.category = req.Category.category;
	pTpcReqFrame->actionHeader.actionID = req.Action.action;
	pTpcReqFrame->actionHeader.dialogToken = req.DialogToken.token;
	if (req.TPCRequest.present) {
		pTpcReqFrame->type = DOT11F_EID_TPCREQUEST;
		pTpcReqFrame->length = 0;
	} else {
		pe_warn("!!!Rcv TPC Req of inalid type!");
		return eSIR_FAILURE;
	}
	return eSIR_SUCCESS;
} /* End sir_convert_tpc_req_frame2_struct. */
tSirRetStatus
sir_convert_meas_req_frame2_struct(tpAniSirGlobal pMac,
				   uint8_t *pFrame,
				   tpSirMacMeasReqActionFrame pMeasReqFrame,
				   uint32_t nFrame)
{
	tDot11fMeasurementRequest mr;
	uint32_t status;

	/* Zero-init our [out] parameter, */
	qdf_mem_set((uint8_t *) pMeasReqFrame,
		    sizeof(tpSirMacMeasReqActionFrame), 0);

	/* delegate to the framesc-generated code, */
	status = dot11f_unpack_measurement_request(pMac, pFrame,
						   nFrame, &mr, false);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to parse a Measurement Request frame (0x%08x, %d bytes):",
			   status, nFrame);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
				   pFrame, nFrame);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_debug("There were warnings while unpacking a Measurement Request frame (0x%08x, %d bytes):",
			   status, nFrame);
	}
	/* & "transliterate" from a 'tDot11fMeasurementRequest' to a */
	/* 'tpSirMacMeasReqActionFrame'... */
	pMeasReqFrame->actionHeader.category = mr.Category.category;
	pMeasReqFrame->actionHeader.actionID = mr.Action.action;
	pMeasReqFrame->actionHeader.dialogToken = mr.DialogToken.token;

	if (0 == mr.num_MeasurementRequest) {
		pe_err("Missing mandatory IE in Measurement Request Frame");
		return eSIR_FAILURE;
	} else if (1 < mr.num_MeasurementRequest) {
		pe_warn(
			FL("Warning: dropping extra Measurement Request IEs!"));
	}

	pMeasReqFrame->measReqIE.type = DOT11F_EID_MEASUREMENTREQUEST;
	pMeasReqFrame->measReqIE.length = DOT11F_IE_MEASUREMENTREQUEST_MIN_LEN;
	pMeasReqFrame->measReqIE.measToken =
		mr.MeasurementRequest[0].measurement_token;
	pMeasReqFrame->measReqIE.measReqMode =
		(mr.MeasurementRequest[0].reserved << 3) | (mr.
							    MeasurementRequest[0].
							    enable << 2) | (mr.
									    MeasurementRequest
									    [0].
									    request
									    << 1) |
		(mr.MeasurementRequest[0].report /*<< 0 */);
	pMeasReqFrame->measReqIE.measType =
		mr.MeasurementRequest[0].measurement_type;

	pMeasReqFrame->measReqIE.measReqField.channelNumber =
		mr.MeasurementRequest[0].channel_no;

	qdf_mem_copy(pMeasReqFrame->measReqIE.measReqField.measStartTime,
		     mr.MeasurementRequest[0].meas_start_time, 8);

	pMeasReqFrame->measReqIE.measReqField.measDuration =
		mr.MeasurementRequest[0].meas_duration;

	return eSIR_SUCCESS;

} /* End sir_convert_meas_req_frame2_struct. */
#endif

void populate_dot11f_tspec(tSirMacTspecIE *pOld, tDot11fIETSPEC *pDot11f)
{
	pDot11f->traffic_type = pOld->tsinfo.traffic.trafficType;
	pDot11f->tsid = pOld->tsinfo.traffic.tsid;
	pDot11f->direction = pOld->tsinfo.traffic.direction;
	pDot11f->access_policy = pOld->tsinfo.traffic.accessPolicy;
	pDot11f->aggregation = pOld->tsinfo.traffic.aggregation;
	pDot11f->psb = pOld->tsinfo.traffic.psb;
	pDot11f->user_priority = pOld->tsinfo.traffic.userPrio;
	pDot11f->tsinfo_ack_pol = pOld->tsinfo.traffic.ackPolicy;
	pDot11f->schedule = pOld->tsinfo.schedule.schedule;
	/* As defined in IEEE 802.11-2007, section 7.3.2.30
	 * Nominal MSDU size: Bit[0:14]=Size, Bit[15]=Fixed
	 */
	pDot11f->size = (pOld->nomMsduSz & 0x7fff);
	pDot11f->fixed = (pOld->nomMsduSz & 0x8000) ? 1 : 0;
	pDot11f->max_msdu_size = pOld->maxMsduSz;
	pDot11f->min_service_int = pOld->minSvcInterval;
	pDot11f->max_service_int = pOld->maxSvcInterval;
	pDot11f->inactivity_int = pOld->inactInterval;
	pDot11f->suspension_int = pOld->suspendInterval;
	pDot11f->service_start_time = pOld->svcStartTime;
	pDot11f->min_data_rate = pOld->minDataRate;
	pDot11f->mean_data_rate = pOld->meanDataRate;
	pDot11f->peak_data_rate = pOld->peakDataRate;
	pDot11f->burst_size = pOld->maxBurstSz;
	pDot11f->delay_bound = pOld->delayBound;
	pDot11f->min_phy_rate = pOld->minPhyRate;
	pDot11f->surplus_bw_allowance = pOld->surplusBw;
	pDot11f->medium_time = pOld->mediumTime;

	pDot11f->present = 1;

} /* End populate_dot11f_tspec. */

void populate_dot11f_wmmtspec(tSirMacTspecIE *pOld, tDot11fIEWMMTSPEC *pDot11f)
{
	pDot11f->traffic_type = pOld->tsinfo.traffic.trafficType;
	pDot11f->tsid = pOld->tsinfo.traffic.tsid;
	pDot11f->direction = pOld->tsinfo.traffic.direction;
	pDot11f->access_policy = pOld->tsinfo.traffic.accessPolicy;
	pDot11f->aggregation = pOld->tsinfo.traffic.aggregation;
	pDot11f->psb = pOld->tsinfo.traffic.psb;
	pDot11f->user_priority = pOld->tsinfo.traffic.userPrio;
	pDot11f->tsinfo_ack_pol = pOld->tsinfo.traffic.ackPolicy;
	pDot11f->burst_size_defn = pOld->tsinfo.traffic.burstSizeDefn;
	/* As defined in IEEE 802.11-2007, section 7.3.2.30
	 * Nominal MSDU size: Bit[0:14]=Size, Bit[15]=Fixed
	 */
	pDot11f->size = (pOld->nomMsduSz & 0x7fff);
	pDot11f->fixed = (pOld->nomMsduSz & 0x8000) ? 1 : 0;
	pDot11f->max_msdu_size = pOld->maxMsduSz;
	pDot11f->min_service_int = pOld->minSvcInterval;
	pDot11f->max_service_int = pOld->maxSvcInterval;
	pDot11f->inactivity_int = pOld->inactInterval;
	pDot11f->suspension_int = pOld->suspendInterval;
	pDot11f->service_start_time = pOld->svcStartTime;
	pDot11f->min_data_rate = pOld->minDataRate;
	pDot11f->mean_data_rate = pOld->meanDataRate;
	pDot11f->peak_data_rate = pOld->peakDataRate;
	pDot11f->burst_size = pOld->maxBurstSz;
	pDot11f->delay_bound = pOld->delayBound;
	pDot11f->min_phy_rate = pOld->minPhyRate;
	pDot11f->surplus_bw_allowance = pOld->surplusBw;
	pDot11f->medium_time = pOld->mediumTime;

	pDot11f->version = 1;
	pDot11f->present = 1;

} /* End populate_dot11f_wmmtspec. */

#if defined(FEATURE_WLAN_ESE)
/* Fill the ESE version currently supported */
void populate_dot11f_ese_version(tDot11fIEESEVersion *pESEVersion)
{
	pESEVersion->present = 1;
	pESEVersion->version = ESE_VERSION_SUPPORTED;
}

/* Fill the ESE ie for the station. */
/* The State is Normal (1) */
/* The MBSSID for station is set to 0. */
void populate_dot11f_ese_rad_mgmt_cap(tDot11fIEESERadMgmtCap *pESERadMgmtCap)
{
	pESERadMgmtCap->present = 1;
	pESERadMgmtCap->mgmt_state = RM_STATE_NORMAL;
	pESERadMgmtCap->mbssid_mask = 0;
	pESERadMgmtCap->reserved = 0;
}

tSirRetStatus populate_dot11f_ese_cckm_opaque(tpAniSirGlobal pMac,
					      tpSirCCKMie pCCKMie,
					      tDot11fIEESECckmOpaque *pDot11f)
{
	int idx;

	if (pCCKMie->length) {
		idx = find_ie_location(pMac, (tpSirRSNie) pCCKMie,
						 DOT11F_EID_ESECCKMOPAQUE);
		if (0 <= idx) {
			pDot11f->present = 1;
			/* Dont include OUI */
			pDot11f->num_data = pCCKMie->cckmIEdata[idx + 1] - 4;
			qdf_mem_copy(pDot11f->data, pCCKMie->cckmIEdata + idx + 2 + 4,  /* EID,len,OUI */
				     pCCKMie->cckmIEdata[idx + 1] - 4); /* Skip OUI */
		}
	}
	return eSIR_SUCCESS;
} /* End populate_dot11f_ese_cckm_opaque. */

void populate_dot11_tsrsie(tpAniSirGlobal pMac,
			   tSirMacESETSRSIE *pOld,
			   tDot11fIEESETrafStrmRateSet *pDot11f,
			   uint8_t rate_length)
{
	pDot11f->tsid = pOld->tsid;
	qdf_mem_copy(pDot11f->tsrates, pOld->rates, rate_length);
	pDot11f->num_tsrates = rate_length;
	pDot11f->present = 1;
}
#endif

tSirRetStatus
populate_dot11f_tclas(tpAniSirGlobal pMac,
		      tSirTclasInfo *pOld, tDot11fIETCLAS *pDot11f)
{
	pDot11f->user_priority = pOld->tclas.userPrio;
	pDot11f->classifier_type = pOld->tclas.classifierType;
	pDot11f->classifier_mask = pOld->tclas.classifierMask;

	switch (pDot11f->classifier_type) {
	case SIR_MAC_TCLASTYPE_ETHERNET:
		qdf_mem_copy((uint8_t *) &pDot11f->info.EthParams.source,
			     (uint8_t *) &pOld->tclasParams.eth.srcAddr, 6);
		qdf_mem_copy((uint8_t *) &pDot11f->info.EthParams.dest,
			     (uint8_t *) &pOld->tclasParams.eth.dstAddr, 6);
		pDot11f->info.EthParams.type = pOld->tclasParams.eth.type;
		break;
	case SIR_MAC_TCLASTYPE_TCPUDPIP:
		pDot11f->info.IpParams.version = pOld->version;
		if (SIR_MAC_TCLAS_IPV4 == pDot11f->info.IpParams.version) {
			qdf_mem_copy(pDot11f->info.IpParams.params.IpV4Params.
				     source, pOld->tclasParams.ipv4.srcIpAddr,
				     4);
			qdf_mem_copy(pDot11f->info.IpParams.params.IpV4Params.
				     dest, pOld->tclasParams.ipv4.dstIpAddr, 4);
			pDot11f->info.IpParams.params.IpV4Params.src_port =
				pOld->tclasParams.ipv4.srcPort;
			pDot11f->info.IpParams.params.IpV4Params.dest_port =
				pOld->tclasParams.ipv4.dstPort;
			pDot11f->info.IpParams.params.IpV4Params.DSCP =
				pOld->tclasParams.ipv4.dscp;
			pDot11f->info.IpParams.params.IpV4Params.proto =
				pOld->tclasParams.ipv4.protocol;
			pDot11f->info.IpParams.params.IpV4Params.reserved =
				pOld->tclasParams.ipv4.rsvd;
		} else {
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.source,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     srcIpAddr, 16);
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.dest,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     dstIpAddr, 16);
			pDot11f->info.IpParams.params.IpV6Params.src_port =
				pOld->tclasParams.ipv6.srcPort;
			pDot11f->info.IpParams.params.IpV6Params.dest_port =
				pOld->tclasParams.ipv6.dstPort;
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.flow_label,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     flowLabel, 3);
		}
		break;
	case SIR_MAC_TCLASTYPE_8021DQ:
		pDot11f->info.Params8021dq.tag_type =
			pOld->tclasParams.t8021dq.tag;
		break;
	default:
		pe_err("Bad TCLAS type %d in populate_dot11f_tclas",
			pDot11f->classifier_type);
		return eSIR_FAILURE;
	}

	pDot11f->present = 1;

	return eSIR_SUCCESS;

} /* End populate_dot11f_tclas. */

tSirRetStatus
populate_dot11f_wmmtclas(tpAniSirGlobal pMac,
			 tSirTclasInfo *pOld, tDot11fIEWMMTCLAS *pDot11f)
{
	pDot11f->version = 1;
	pDot11f->user_priority = pOld->tclas.userPrio;
	pDot11f->classifier_type = pOld->tclas.classifierType;
	pDot11f->classifier_mask = pOld->tclas.classifierMask;

	switch (pDot11f->classifier_type) {
	case SIR_MAC_TCLASTYPE_ETHERNET:
		qdf_mem_copy((uint8_t *) &pDot11f->info.EthParams.source,
			     (uint8_t *) &pOld->tclasParams.eth.srcAddr, 6);
		qdf_mem_copy((uint8_t *) &pDot11f->info.EthParams.dest,
			     (uint8_t *) &pOld->tclasParams.eth.dstAddr, 6);
		pDot11f->info.EthParams.type = pOld->tclasParams.eth.type;
		break;
	case SIR_MAC_TCLASTYPE_TCPUDPIP:
		pDot11f->info.IpParams.version = pOld->version;
		if (SIR_MAC_TCLAS_IPV4 == pDot11f->info.IpParams.version) {
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV4Params.source,
				     (uint8_t *) pOld->tclasParams.ipv4.
				     srcIpAddr, 4);
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV4Params.dest,
				     (uint8_t *) pOld->tclasParams.ipv4.
				     dstIpAddr, 4);
			pDot11f->info.IpParams.params.IpV4Params.src_port =
				pOld->tclasParams.ipv4.srcPort;
			pDot11f->info.IpParams.params.IpV4Params.dest_port =
				pOld->tclasParams.ipv4.dstPort;
			pDot11f->info.IpParams.params.IpV4Params.DSCP =
				pOld->tclasParams.ipv4.dscp;
			pDot11f->info.IpParams.params.IpV4Params.proto =
				pOld->tclasParams.ipv4.protocol;
			pDot11f->info.IpParams.params.IpV4Params.reserved =
				pOld->tclasParams.ipv4.rsvd;
		} else {
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.source,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     srcIpAddr, 16);
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.dest,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     dstIpAddr, 16);
			pDot11f->info.IpParams.params.IpV6Params.src_port =
				pOld->tclasParams.ipv6.srcPort;
			pDot11f->info.IpParams.params.IpV6Params.dest_port =
				pOld->tclasParams.ipv6.dstPort;
			qdf_mem_copy((uint8_t *) &pDot11f->info.IpParams.
				     params.IpV6Params.flow_label,
				     (uint8_t *) pOld->tclasParams.ipv6.
				     flowLabel, 3);
		}
		break;
	case SIR_MAC_TCLASTYPE_8021DQ:
		pDot11f->info.Params8021dq.tag_type =
			pOld->tclasParams.t8021dq.tag;
		break;
	default:
		pe_err("Bad TCLAS type %d in populate_dot11f_tclas",
			pDot11f->classifier_type);
		return eSIR_FAILURE;
	}

	pDot11f->present = 1;

	return eSIR_SUCCESS;

} /* End populate_dot11f_wmmtclas. */

tSirRetStatus populate_dot11f_wsc(tpAniSirGlobal pMac,
				  tDot11fIEWscBeacon *pDot11f)
{

	uint32_t wpsState;

	pDot11f->Version.present = 1;
	pDot11f->Version.major = 0x01;
	pDot11f->Version.minor = 0x00;

	if (wlan_cfg_get_int(pMac, (uint16_t) WNI_CFG_WPS_STATE, &wpsState) !=
	    eSIR_SUCCESS)
		pe_err("Failed to cfg get id %d", WNI_CFG_WPS_STATE);

	pDot11f->WPSState.present = 1;
	pDot11f->WPSState.state = (uint8_t) wpsState;

	pDot11f->APSetupLocked.present = 0;

	pDot11f->SelectedRegistrar.present = 0;

	pDot11f->DevicePasswordID.present = 0;

	pDot11f->SelectedRegistrarConfigMethods.present = 0;

	pDot11f->UUID_E.present = 0;

	pDot11f->RFBands.present = 0;

	pDot11f->present = 1;
	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_wsc_registrar_info(tpAniSirGlobal pMac,
						 tDot11fIEWscBeacon *pDot11f)
{
	const struct sLimWscIeInfo *const pWscIeInfo = &(pMac->lim.wscIeInfo);
	uint32_t devicepasswdId;

	pDot11f->APSetupLocked.present = 1;
	pDot11f->APSetupLocked.fLocked = pWscIeInfo->apSetupLocked;

	pDot11f->SelectedRegistrar.present = 1;
	pDot11f->SelectedRegistrar.selected = pWscIeInfo->selectedRegistrar;

	if (wlan_cfg_get_int
		    (pMac, (uint16_t) WNI_CFG_WPS_DEVICE_PASSWORD_ID,
		    &devicepasswdId) != eSIR_SUCCESS)
		pe_err("Failed to cfg get id %d",
			WNI_CFG_WPS_DEVICE_PASSWORD_ID);

	pDot11f->DevicePasswordID.present = 1;
	pDot11f->DevicePasswordID.id = (uint16_t) devicepasswdId;

	pDot11f->SelectedRegistrarConfigMethods.present = 1;
	pDot11f->SelectedRegistrarConfigMethods.methods =
		pWscIeInfo->selectedRegistrarConfigMethods;

	/* UUID_E and RF Bands are applicable only for dual band AP */

	return eSIR_SUCCESS;
}

tSirRetStatus de_populate_dot11f_wsc_registrar_info(tpAniSirGlobal pMac,
						    tDot11fIEWscBeacon *pDot11f)
{
	pDot11f->APSetupLocked.present = 0;
	pDot11f->SelectedRegistrar.present = 0;
	pDot11f->DevicePasswordID.present = 0;
	pDot11f->SelectedRegistrarConfigMethods.present = 0;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_probe_res_wpsi_es(tpAniSirGlobal pMac,
						tDot11fIEWscProbeRes *pDot11f,
						tpPESession psessionEntry)
{

	tSirWPSProbeRspIE *pSirWPSProbeRspIE;

	pSirWPSProbeRspIE = &psessionEntry->APWPSIEs.SirWPSProbeRspIE;

	if (pSirWPSProbeRspIE->FieldPresent & SIR_WPS_PROBRSP_VER_PRESENT) {
		pDot11f->present = 1;
		pDot11f->Version.present = 1;
		pDot11f->Version.major =
			(uint8_t) ((pSirWPSProbeRspIE->Version & 0xF0) >> 4);
		pDot11f->Version.minor =
			(uint8_t) (pSirWPSProbeRspIE->Version & 0x0F);
	} else {
		pDot11f->present = 0;
		pDot11f->Version.present = 0;
	}

	if (pSirWPSProbeRspIE->FieldPresent & SIR_WPS_PROBRSP_STATE_PRESENT) {

		pDot11f->WPSState.present = 1;
		pDot11f->WPSState.state = (uint8_t) pSirWPSProbeRspIE->wpsState;
	} else
		pDot11f->WPSState.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_APSETUPLOCK_PRESENT) {
		pDot11f->APSetupLocked.present = 1;
		pDot11f->APSetupLocked.fLocked =
			pSirWPSProbeRspIE->APSetupLocked;
	} else
		pDot11f->APSetupLocked.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_SELECTEDREGISTRA_PRESENT) {
		pDot11f->SelectedRegistrar.present = 1;
		pDot11f->SelectedRegistrar.selected =
			pSirWPSProbeRspIE->SelectedRegistra;
	} else
		pDot11f->SelectedRegistrar.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_DEVICEPASSWORDID_PRESENT) {
		pDot11f->DevicePasswordID.present = 1;
		pDot11f->DevicePasswordID.id =
			pSirWPSProbeRspIE->DevicePasswordID;
	} else
		pDot11f->DevicePasswordID.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_SELECTEDREGISTRACFGMETHOD_PRESENT) {
		pDot11f->SelectedRegistrarConfigMethods.present = 1;
		pDot11f->SelectedRegistrarConfigMethods.methods =
			pSirWPSProbeRspIE->SelectedRegistraCfgMethod;
	} else
		pDot11f->SelectedRegistrarConfigMethods.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_RESPONSETYPE_PRESENT) {
		pDot11f->ResponseType.present = 1;
		pDot11f->ResponseType.resType = pSirWPSProbeRspIE->ResponseType;
	} else
		pDot11f->ResponseType.present = 0;

	if (pSirWPSProbeRspIE->FieldPresent & SIR_WPS_PROBRSP_UUIDE_PRESENT) {
		pDot11f->UUID_E.present = 1;
		qdf_mem_copy(pDot11f->UUID_E.uuid, pSirWPSProbeRspIE->UUID_E,
			     WNI_CFG_WPS_UUID_LEN);
	} else
		pDot11f->UUID_E.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_MANUFACTURE_PRESENT) {
		pDot11f->Manufacturer.present = 1;
		pDot11f->Manufacturer.num_name =
			pSirWPSProbeRspIE->Manufacture.num_name;
		qdf_mem_copy(pDot11f->Manufacturer.name,
			     pSirWPSProbeRspIE->Manufacture.name,
			     pSirWPSProbeRspIE->Manufacture.num_name);
	} else
		pDot11f->Manufacturer.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_MODELNUMBER_PRESENT) {
		pDot11f->ModelName.present = 1;
		pDot11f->ModelName.num_text =
			pSirWPSProbeRspIE->ModelName.num_text;
		qdf_mem_copy(pDot11f->ModelName.text,
			     pSirWPSProbeRspIE->ModelName.text,
			     pDot11f->ModelName.num_text);
	} else
		pDot11f->ModelName.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_MODELNUMBER_PRESENT) {
		pDot11f->ModelNumber.present = 1;
		pDot11f->ModelNumber.num_text =
			pSirWPSProbeRspIE->ModelNumber.num_text;
		qdf_mem_copy(pDot11f->ModelNumber.text,
			     pSirWPSProbeRspIE->ModelNumber.text,
			     pDot11f->ModelNumber.num_text);
	} else
		pDot11f->ModelNumber.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_SERIALNUMBER_PRESENT) {
		pDot11f->SerialNumber.present = 1;
		pDot11f->SerialNumber.num_text =
			pSirWPSProbeRspIE->SerialNumber.num_text;
		qdf_mem_copy(pDot11f->SerialNumber.text,
			     pSirWPSProbeRspIE->SerialNumber.text,
			     pDot11f->SerialNumber.num_text);
	} else
		pDot11f->SerialNumber.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_PRIMARYDEVICETYPE_PRESENT) {
		pDot11f->PrimaryDeviceType.present = 1;
		qdf_mem_copy(pDot11f->PrimaryDeviceType.oui,
			     pSirWPSProbeRspIE->PrimaryDeviceOUI,
			     sizeof(pSirWPSProbeRspIE->PrimaryDeviceOUI));
		pDot11f->PrimaryDeviceType.primary_category =
			(uint16_t) pSirWPSProbeRspIE->PrimaryDeviceCategory;
		pDot11f->PrimaryDeviceType.sub_category =
			(uint16_t) pSirWPSProbeRspIE->DeviceSubCategory;
	} else
		pDot11f->PrimaryDeviceType.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_DEVICENAME_PRESENT) {
		pDot11f->DeviceName.present = 1;
		pDot11f->DeviceName.num_text =
			pSirWPSProbeRspIE->DeviceName.num_text;
		qdf_mem_copy(pDot11f->DeviceName.text,
			     pSirWPSProbeRspIE->DeviceName.text,
			     pDot11f->DeviceName.num_text);
	} else
		pDot11f->DeviceName.present = 0;

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_CONFIGMETHODS_PRESENT) {
		pDot11f->ConfigMethods.present = 1;
		pDot11f->ConfigMethods.methods =
			pSirWPSProbeRspIE->ConfigMethod;
	} else
		pDot11f->ConfigMethods.present = 0;

	if (pSirWPSProbeRspIE->FieldPresent & SIR_WPS_PROBRSP_RF_BANDS_PRESENT) {
		pDot11f->RFBands.present = 1;
		pDot11f->RFBands.bands = pSirWPSProbeRspIE->RFBand;
	} else
		pDot11f->RFBands.present = 0;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_assoc_res_wpsi_es(tpAniSirGlobal pMac,
						tDot11fIEWscAssocRes *pDot11f,
						tpPESession psessionEntry)
{
	tSirWPSProbeRspIE *pSirWPSProbeRspIE;

	pSirWPSProbeRspIE = &psessionEntry->APWPSIEs.SirWPSProbeRspIE;

	if (pSirWPSProbeRspIE->FieldPresent & SIR_WPS_PROBRSP_VER_PRESENT) {
		pDot11f->present = 1;
		pDot11f->Version.present = 1;
		pDot11f->Version.major =
			(uint8_t) ((pSirWPSProbeRspIE->Version & 0xF0) >> 4);
		pDot11f->Version.minor =
			(uint8_t) (pSirWPSProbeRspIE->Version & 0x0F);
	} else {
		pDot11f->present = 0;
		pDot11f->Version.present = 0;
	}

	if (pSirWPSProbeRspIE->
	    FieldPresent & SIR_WPS_PROBRSP_RESPONSETYPE_PRESENT) {
		pDot11f->ResponseType.present = 1;
		pDot11f->ResponseType.resType = pSirWPSProbeRspIE->ResponseType;
	} else
		pDot11f->ResponseType.present = 0;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_beacon_wpsi_es(tpAniSirGlobal pMac,
					     tDot11fIEWscBeacon *pDot11f,
					     tpPESession psessionEntry)
{

	tSirWPSBeaconIE *pSirWPSBeaconIE;

	pSirWPSBeaconIE = &psessionEntry->APWPSIEs.SirWPSBeaconIE;

	if (pSirWPSBeaconIE->FieldPresent & SIR_WPS_PROBRSP_VER_PRESENT) {
		pDot11f->present = 1;
		pDot11f->Version.present = 1;
		pDot11f->Version.major =
			(uint8_t) ((pSirWPSBeaconIE->Version & 0xF0) >> 4);
		pDot11f->Version.minor =
			(uint8_t) (pSirWPSBeaconIE->Version & 0x0F);
	} else {
		pDot11f->present = 0;
		pDot11f->Version.present = 0;
	}

	if (pSirWPSBeaconIE->FieldPresent & SIR_WPS_BEACON_STATE_PRESENT) {

		pDot11f->WPSState.present = 1;
		pDot11f->WPSState.state = (uint8_t) pSirWPSBeaconIE->wpsState;
	} else
		pDot11f->WPSState.present = 0;

	if (pSirWPSBeaconIE->FieldPresent & SIR_WPS_BEACON_APSETUPLOCK_PRESENT) {
		pDot11f->APSetupLocked.present = 1;
		pDot11f->APSetupLocked.fLocked = pSirWPSBeaconIE->APSetupLocked;
	} else
		pDot11f->APSetupLocked.present = 0;

	if (pSirWPSBeaconIE->
	    FieldPresent & SIR_WPS_BEACON_SELECTEDREGISTRA_PRESENT) {
		pDot11f->SelectedRegistrar.present = 1;
		pDot11f->SelectedRegistrar.selected =
			pSirWPSBeaconIE->SelectedRegistra;
	} else
		pDot11f->SelectedRegistrar.present = 0;

	if (pSirWPSBeaconIE->
	    FieldPresent & SIR_WPS_BEACON_DEVICEPASSWORDID_PRESENT) {
		pDot11f->DevicePasswordID.present = 1;
		pDot11f->DevicePasswordID.id =
			pSirWPSBeaconIE->DevicePasswordID;
	} else
		pDot11f->DevicePasswordID.present = 0;

	if (pSirWPSBeaconIE->
	    FieldPresent & SIR_WPS_BEACON_SELECTEDREGISTRACFGMETHOD_PRESENT) {
		pDot11f->SelectedRegistrarConfigMethods.present = 1;
		pDot11f->SelectedRegistrarConfigMethods.methods =
			pSirWPSBeaconIE->SelectedRegistraCfgMethod;
	} else
		pDot11f->SelectedRegistrarConfigMethods.present = 0;

	if (pSirWPSBeaconIE->FieldPresent & SIR_WPS_BEACON_UUIDE_PRESENT) {
		pDot11f->UUID_E.present = 1;
		qdf_mem_copy(pDot11f->UUID_E.uuid, pSirWPSBeaconIE->UUID_E,
			     WNI_CFG_WPS_UUID_LEN);
	} else
		pDot11f->UUID_E.present = 0;

	if (pSirWPSBeaconIE->FieldPresent & SIR_WPS_BEACON_RF_BANDS_PRESENT) {
		pDot11f->RFBands.present = 1;
		pDot11f->RFBands.bands = pSirWPSBeaconIE->RFBand;
	} else
		pDot11f->RFBands.present = 0;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_wsc_in_probe_res(tpAniSirGlobal pMac,
					       tDot11fIEWscProbeRes *pDot11f)
{
	uint32_t cfgMethods;
	uint32_t cfgStrLen;
	uint32_t val;
	uint32_t wpsVersion, wpsState;

	if (wlan_cfg_get_int(pMac, (uint16_t) WNI_CFG_WPS_VERSION, &wpsVersion) !=
	    eSIR_SUCCESS)
		pe_err("Failed to cfg get id %d", WNI_CFG_WPS_VERSION);

	pDot11f->Version.present = 1;
	pDot11f->Version.major = (uint8_t) ((wpsVersion & 0xF0) >> 4);
	pDot11f->Version.minor = (uint8_t) (wpsVersion & 0x0F);

	if (wlan_cfg_get_int(pMac, (uint16_t) WNI_CFG_WPS_STATE, &wpsState) !=
	    eSIR_SUCCESS)
		pe_err("Failed to cfg get id %d", WNI_CFG_WPS_STATE);

	pDot11f->WPSState.present = 1;
	pDot11f->WPSState.state = (uint8_t) wpsState;

	pDot11f->APSetupLocked.present = 0;

	pDot11f->SelectedRegistrar.present = 0;

	pDot11f->DevicePasswordID.present = 0;

	pDot11f->SelectedRegistrarConfigMethods.present = 0;

	pDot11f->ResponseType.present = 1;
	if ((pMac->lim.wscIeInfo.reqType == REQ_TYPE_REGISTRAR) ||
	    (pMac->lim.wscIeInfo.reqType == REQ_TYPE_WLAN_MANAGER_REGISTRAR)) {
		pDot11f->ResponseType.resType = RESP_TYPE_ENROLLEE_OPEN_8021X;
	} else {
		pDot11f->ResponseType.resType = RESP_TYPE_AP;
	}

	/* UUID is a 16 byte long binary. Still use wlan_cfg_get_str to get it. */
	pDot11f->UUID_E.present = 1;
	cfgStrLen = WNI_CFG_WPS_UUID_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_WPS_UUID,
			     pDot11f->UUID_E.uuid, &cfgStrLen) != eSIR_SUCCESS) {
		*(pDot11f->UUID_E.uuid) = '\0';
	}

	pDot11f->Manufacturer.present = 1;
	cfgStrLen = WNI_CFG_MANUFACTURER_NAME_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_MANUFACTURER_NAME,
			     pDot11f->Manufacturer.name,
			     &cfgStrLen) != eSIR_SUCCESS) {
		pDot11f->Manufacturer.num_name = 0;
	} else {
		pDot11f->Manufacturer.num_name =
			(uint8_t) (cfgStrLen & 0x000000FF);
	}

	pDot11f->ModelName.present = 1;
	cfgStrLen = WNI_CFG_MODEL_NAME_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_MODEL_NAME,
			     pDot11f->ModelName.text,
			     &cfgStrLen) != eSIR_SUCCESS) {
		pDot11f->ModelName.num_text = 0;
	} else {
		pDot11f->ModelName.num_text =
			(uint8_t) (cfgStrLen & 0x000000FF);
	}

	pDot11f->ModelNumber.present = 1;
	cfgStrLen = WNI_CFG_MODEL_NUMBER_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_MODEL_NUMBER,
			     pDot11f->ModelNumber.text,
			     &cfgStrLen) != eSIR_SUCCESS) {
		pDot11f->ModelNumber.num_text = 0;
	} else {
		pDot11f->ModelNumber.num_text =
			(uint8_t) (cfgStrLen & 0x000000FF);
	}

	pDot11f->SerialNumber.present = 1;
	cfgStrLen = WNI_CFG_MANUFACTURER_PRODUCT_VERSION_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_MANUFACTURER_PRODUCT_VERSION,
			     pDot11f->SerialNumber.text,
			     &cfgStrLen) != eSIR_SUCCESS) {
		pDot11f->SerialNumber.num_text = 0;
	} else {
		pDot11f->SerialNumber.num_text =
			(uint8_t) (cfgStrLen & 0x000000FF);
	}

	pDot11f->PrimaryDeviceType.present = 1;

	if (wlan_cfg_get_int(pMac, WNI_CFG_WPS_PRIMARY_DEVICE_CATEGORY, &val) !=
	    eSIR_SUCCESS) {
		pe_err("cfg get prim device category failed");
	} else
		pDot11f->PrimaryDeviceType.primary_category = (uint16_t) val;

	if (wlan_cfg_get_int(pMac, WNI_CFG_WPS_PIMARY_DEVICE_OUI, &val) !=
	    eSIR_SUCCESS) {
		pe_err("cfg get prim device OUI failed");
	} else {
		*(pDot11f->PrimaryDeviceType.oui) =
			(uint8_t) ((val >> 24) & 0xff);
		*(pDot11f->PrimaryDeviceType.oui + 1) =
			(uint8_t) ((val >> 16) & 0xff);
		*(pDot11f->PrimaryDeviceType.oui + 2) =
			(uint8_t) ((val >> 8) & 0xff);
		*(pDot11f->PrimaryDeviceType.oui + 3) =
			(uint8_t) ((val & 0xff));
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_WPS_DEVICE_SUB_CATEGORY, &val) !=
	    eSIR_SUCCESS) {
		pe_err("cfg get prim device sub category failed");
	} else
		pDot11f->PrimaryDeviceType.sub_category = (uint16_t) val;

	pDot11f->DeviceName.present = 1;
	cfgStrLen = WNI_CFG_MANUFACTURER_PRODUCT_NAME_LEN;
	if (wlan_cfg_get_str(pMac,
			     WNI_CFG_MANUFACTURER_PRODUCT_NAME,
			     pDot11f->DeviceName.text,
			     &cfgStrLen) != eSIR_SUCCESS) {
		pDot11f->DeviceName.num_text = 0;
	} else {
		pDot11f->DeviceName.num_text =
			(uint8_t) (cfgStrLen & 0x000000FF);
	}

	if (wlan_cfg_get_int(pMac,
			     WNI_CFG_WPS_CFG_METHOD,
			     &cfgMethods) != eSIR_SUCCESS) {
		pDot11f->ConfigMethods.present = 0;
		pDot11f->ConfigMethods.methods = 0;
	} else {
		pDot11f->ConfigMethods.present = 1;
		pDot11f->ConfigMethods.methods =
			(uint16_t) (cfgMethods & 0x0000FFFF);
	}

	pDot11f->RFBands.present = 0;

	pDot11f->present = 1;
	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_wsc_registrar_info_in_probe_res(tpAniSirGlobal pMac,
							      tDot11fIEWscProbeRes *
							      pDot11f)
{
	const struct sLimWscIeInfo *const pWscIeInfo = &(pMac->lim.wscIeInfo);
	uint32_t devicepasswdId;

	pDot11f->APSetupLocked.present = 1;
	pDot11f->APSetupLocked.fLocked = pWscIeInfo->apSetupLocked;

	pDot11f->SelectedRegistrar.present = 1;
	pDot11f->SelectedRegistrar.selected = pWscIeInfo->selectedRegistrar;

	if (wlan_cfg_get_int
		    (pMac, (uint16_t) WNI_CFG_WPS_DEVICE_PASSWORD_ID,
		    &devicepasswdId) != eSIR_SUCCESS)
		pe_err("Failed to cfg get id %d",
			WNI_CFG_WPS_DEVICE_PASSWORD_ID);

	pDot11f->DevicePasswordID.present = 1;
	pDot11f->DevicePasswordID.id = (uint16_t) devicepasswdId;

	pDot11f->SelectedRegistrarConfigMethods.present = 1;
	pDot11f->SelectedRegistrarConfigMethods.methods =
		pWscIeInfo->selectedRegistrarConfigMethods;

	/* UUID_E and RF Bands are applicable only for dual band AP */

	return eSIR_SUCCESS;
}

tSirRetStatus de_populate_dot11f_wsc_registrar_info_in_probe_res(tpAniSirGlobal pMac,
								 tDot11fIEWscProbeRes *
								 pDot11f)
{
	pDot11f->APSetupLocked.present = 0;
	pDot11f->SelectedRegistrar.present = 0;
	pDot11f->DevicePasswordID.present = 0;
	pDot11f->SelectedRegistrarConfigMethods.present = 0;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_assoc_res_wsc_ie(tpAniSirGlobal pMac,
					       tDot11fIEWscAssocRes *pDot11f,
					       tpSirAssocReq pRcvdAssocReq)
{
	tDot11fIEWscAssocReq parsedWscAssocReq = { 0, };
	const uint8_t *wscIe;

	wscIe = limGetWscIEPtr(pMac, pRcvdAssocReq->addIE.addIEdata,
			       pRcvdAssocReq->addIE.length);
	if (wscIe != NULL) {
		/* retreive WSC IE from given AssocReq */
		dot11f_unpack_ie_wsc_assoc_req(pMac,
			(uint8_t *)wscIe + 2 + 4, /* EID, length, OUI */
			wscIe[1] - 4, /* length without OUI */
			&parsedWscAssocReq, false);
		pDot11f->present = 1;
		/* version has to be 0x10 */
		pDot11f->Version.present = 1;
		pDot11f->Version.major = 0x1;
		pDot11f->Version.minor = 0x0;

		pDot11f->ResponseType.present = 1;

		if ((parsedWscAssocReq.RequestType.reqType ==
		     REQ_TYPE_REGISTRAR)
		    || (parsedWscAssocReq.RequestType.reqType ==
			REQ_TYPE_WLAN_MANAGER_REGISTRAR)) {
			pDot11f->ResponseType.resType =
				RESP_TYPE_ENROLLEE_OPEN_8021X;
		} else {
			pDot11f->ResponseType.resType = RESP_TYPE_AP;
		}
		/* Version infomration should be taken from our capability as well as peers */
		/* TODO: currently it takes from peers only */
		if (parsedWscAssocReq.VendorExtension.present &&
		    parsedWscAssocReq.VendorExtension.Version2.present) {
			pDot11f->VendorExtension.present = 1;
			pDot11f->VendorExtension.vendorId[0] = 0x00;
			pDot11f->VendorExtension.vendorId[1] = 0x37;
			pDot11f->VendorExtension.vendorId[2] = 0x2A;
			pDot11f->VendorExtension.Version2.present = 1;
			pDot11f->VendorExtension.Version2.major =
				parsedWscAssocReq.VendorExtension.Version2.major;
			pDot11f->VendorExtension.Version2.minor =
				parsedWscAssocReq.VendorExtension.Version2.minor;
		}
	}
	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11_assoc_res_p2p_ie(tpAniSirGlobal pMac,
					      tDot11fIEP2PAssocRes *pDot11f,
					      tpSirAssocReq pRcvdAssocReq)
{
	const uint8_t *p2pIe;

	p2pIe = limGetP2pIEPtr(pMac, pRcvdAssocReq->addIE.addIEdata,
			       pRcvdAssocReq->addIE.length);
	if (p2pIe != NULL) {
		pDot11f->present = 1;
		pDot11f->P2PStatus.present = 1;
		pDot11f->P2PStatus.status = eSIR_SUCCESS;
		pDot11f->ExtendedListenTiming.present = 0;
	}
	return eSIR_SUCCESS;
}


tSirRetStatus populate_dot11f_wfatpc(tpAniSirGlobal pMac,
				     tDot11fIEWFATPC *pDot11f, uint8_t txPower,
				     uint8_t linkMargin)
{
	pDot11f->txPower = txPower;
	pDot11f->linkMargin = linkMargin;
	pDot11f->present = 1;

	return eSIR_SUCCESS;
}

tSirRetStatus populate_dot11f_beacon_report(tpAniSirGlobal pMac,
					    tDot11fIEMeasurementReport *pDot11f,
					    tSirMacBeaconReport *pBeaconReport)
{

	pDot11f->report.Beacon.regClass = pBeaconReport->regClass;
	pDot11f->report.Beacon.channel = pBeaconReport->channel;
	qdf_mem_copy(pDot11f->report.Beacon.meas_start_time,
		     pBeaconReport->measStartTime,
		     sizeof(pDot11f->report.Beacon.meas_start_time));
	pDot11f->report.Beacon.meas_duration = pBeaconReport->measDuration;
	pDot11f->report.Beacon.condensed_PHY = pBeaconReport->phyType;
	pDot11f->report.Beacon.reported_frame_type =
		!pBeaconReport->bcnProbeRsp;
	pDot11f->report.Beacon.RCPI = pBeaconReport->rcpi;
	pDot11f->report.Beacon.RSNI = pBeaconReport->rsni;
	qdf_mem_copy(pDot11f->report.Beacon.BSSID, pBeaconReport->bssid,
		     sizeof(tSirMacAddr));
	pDot11f->report.Beacon.antenna_id = pBeaconReport->antennaId;
	pDot11f->report.Beacon.parent_TSF = pBeaconReport->parentTSF;

	if (pBeaconReport->numIes) {
		pDot11f->report.Beacon.BeaconReportFrmBody.present = 1;
		qdf_mem_copy(pDot11f->report.Beacon.BeaconReportFrmBody.
			     reportedFields, pBeaconReport->Ies,
			     pBeaconReport->numIes);
		pDot11f->report.Beacon.BeaconReportFrmBody.num_reportedFields =
			pBeaconReport->numIes;
	}

	return eSIR_SUCCESS;

}

tSirRetStatus populate_dot11f_rrm_ie(tpAniSirGlobal pMac,
				     tDot11fIERRMEnabledCap *pDot11f,
				     tpPESession psessionEntry)
{
	tpRRMCaps pRrmCaps;
	uint8_t *bytes;

	pRrmCaps = rrm_get_capabilities(pMac, psessionEntry);

	pDot11f->LinkMeasurement = pRrmCaps->LinkMeasurement;
	pDot11f->NeighborRpt = pRrmCaps->NeighborRpt;
	pDot11f->parallel = pRrmCaps->parallel;
	pDot11f->repeated = pRrmCaps->repeated;
	pDot11f->BeaconPassive = pRrmCaps->BeaconPassive;
	pDot11f->BeaconActive = pRrmCaps->BeaconActive;
	pDot11f->BeaconTable = pRrmCaps->BeaconTable;
	pDot11f->BeaconRepCond = pRrmCaps->BeaconRepCond;
	pDot11f->FrameMeasurement = pRrmCaps->FrameMeasurement;
	pDot11f->ChannelLoad = pRrmCaps->ChannelLoad;
	pDot11f->NoiseHistogram = pRrmCaps->NoiseHistogram;
	pDot11f->statistics = pRrmCaps->statistics;
	pDot11f->LCIMeasurement = pRrmCaps->LCIMeasurement;
	pDot11f->LCIAzimuth = pRrmCaps->LCIAzimuth;
	pDot11f->TCMCapability = pRrmCaps->TCMCapability;
	pDot11f->triggeredTCM = pRrmCaps->triggeredTCM;
	pDot11f->APChanReport = pRrmCaps->APChanReport;
	pDot11f->RRMMIBEnabled = pRrmCaps->RRMMIBEnabled;
	pDot11f->operatingChanMax = pRrmCaps->operatingChanMax;
	pDot11f->nonOperatinChanMax = pRrmCaps->nonOperatingChanMax;
	pDot11f->MeasurementPilot = pRrmCaps->MeasurementPilot;
	pDot11f->MeasurementPilotEnabled = pRrmCaps->MeasurementPilotEnabled;
	pDot11f->NeighborTSFOffset = pRrmCaps->NeighborTSFOffset;
	pDot11f->RCPIMeasurement = pRrmCaps->RCPIMeasurement;
	pDot11f->RSNIMeasurement = pRrmCaps->RSNIMeasurement;
	pDot11f->BssAvgAccessDelay = pRrmCaps->BssAvgAccessDelay;
	pDot11f->BSSAvailAdmission = pRrmCaps->BSSAvailAdmission;
	pDot11f->AntennaInformation = pRrmCaps->AntennaInformation;
	pDot11f->fine_time_meas_rpt = pRrmCaps->fine_time_meas_rpt;
	pDot11f->lci_capability = pRrmCaps->lci_capability;
	pDot11f->reserved = pRrmCaps->reserved;

	bytes = (uint8_t *) pDot11f + 1; /* ignore present field */
	pe_debug("RRM Enabled Cap IE: %02x %02x %02x %02x %02x",
			   bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);

	pDot11f->present = 1;
	return eSIR_SUCCESS;
}

void populate_mdie(tpAniSirGlobal pMac,
		   tDot11fIEMobilityDomain *pDot11f,
		   uint8_t mdie[SIR_MDIE_SIZE])
{
	pDot11f->present = 1;
	pDot11f->MDID = (uint16_t) ((mdie[1] << 8) | (mdie[0]));

	/* Plugfest fix */
	pDot11f->overDSCap = (mdie[2] & 0x01);
	pDot11f->resourceReqCap = ((mdie[2] >> 1) & 0x01);

}

void populate_ft_info(tpAniSirGlobal pMac, tDot11fIEFTInfo *pDot11f)
{
	pDot11f->present = 1;
	pDot11f->IECount = 0;   /* TODO: put valid data during reassoc. */
	/* All other info is zero. */

}

void populate_dot11f_assoc_rsp_rates(tpAniSirGlobal pMac,
				     tDot11fIESuppRates *pSupp,
				     tDot11fIEExtSuppRates *pExt,
				     uint16_t *_11bRates, uint16_t *_11aRates)
{
	uint8_t num_supp = 0, num_ext = 0;
	uint8_t i, j;

	for (i = 0; (i < SIR_NUM_11B_RATES && _11bRates[i]); i++, num_supp++) {
		pSupp->rates[num_supp] = (uint8_t) _11bRates[i];
	}
	for (j = 0; (j < SIR_NUM_11A_RATES && _11aRates[j]); j++) {
		if (num_supp < 8)
			pSupp->rates[num_supp++] = (uint8_t) _11aRates[j];
		else
			pExt->rates[num_ext++] = (uint8_t) _11aRates[j];
	}

	if (num_supp) {
		pSupp->num_rates = num_supp;
		pSupp->present = 1;
	}
	if (num_ext) {
		pExt->num_rates = num_ext;
		pExt->present = 1;
	}
}

void populate_dot11f_timeout_interval(tpAniSirGlobal pMac,
				      tDot11fIETimeoutInterval *pDot11f,
				      uint8_t type, uint32_t value)
{
	pDot11f->present = 1;
	pDot11f->timeoutType = type;
	pDot11f->timeoutValue = value;
}

/**
 * populate_dot11f_timing_advert_frame() - Populate the TA mgmt frame fields
 * @pMac: the MAC context
 * @frame: pointer to the TA frame
 *
 * Return: The SIR status.
 */
tSirRetStatus populate_dot11f_timing_advert_frame(tpAniSirGlobal mac_ctx,
	tDot11fTimingAdvertisementFrame *frame)
{
	uint32_t val, len;
	uint16_t item;
	uint8_t temp[CFG_MAX_STR_LEN], code[3];
	tSirRetStatus nSirStatus;

	/* Capabilities */
	wlan_cfg_get_int(mac_ctx, WNI_CFG_PRIVACY_ENABLED, &val);
	if (val)
		frame->Capabilities.privacy = 1;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_SHORT_PREAMBLE, &val);
	if (val)
		frame->Capabilities.shortPreamble = 1;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_11H_ENABLED, &val);
	if (val)
		frame->Capabilities.spectrumMgt = 1;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_QOS_ENABLED, &val);
	if (val)
		frame->Capabilities.qos = 1;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_APSD_ENABLED, &val);
	if (val)
		frame->Capabilities.apsd = 1;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_BLOCK_ACK_ENABLED, &val);
	frame->Capabilities.delayedBA =
		(uint16_t)((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
	frame->Capabilities.immediateBA =
		(uint16_t)((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);

	/* Country */
	item = WNI_CFG_MAX_TX_POWER_5;
	CFG_GET_STR(nSirStatus, mac_ctx, item, temp, len,
		WNI_CFG_MAX_TX_POWER_5_LEN);
	wlan_reg_read_current_country(mac_ctx->psoc, code);
	qdf_mem_copy(&frame->Country, code, 2);
	if (len > MAX_SIZE_OF_TRIPLETS_IN_COUNTRY_IE)
		len = MAX_SIZE_OF_TRIPLETS_IN_COUNTRY_IE;

	frame->Country.num_triplets = (uint8_t)(len / 3);
	qdf_mem_copy((uint8_t *)&frame->Country.triplets, temp, len);
	frame->Country.present = 1;

	/* PowerConstraints */
	wlan_cfg_get_int(mac_ctx, WNI_CFG_LOCAL_POWER_CONSTRAINT, &val);
	frame->PowerConstraints.localPowerConstraints = (uint8_t)val;
	frame->PowerConstraints.present = 1;

	/* TimeAdvertisement */
	frame->TimeAdvertisement.present = 1;
	frame->TimeAdvertisement.timing_capabilities = 1;

	return nSirStatus;
}

#ifdef WLAN_FEATURE_11AX
/**
 * populate_dot11f_he_caps() - pouldate HE Capability IE
 * @mac_ctx: Global MAC context
 * @session: PE session
 * @he_cap: pointer to HE capability IE
 *
 * Populdate the HE capability IE based on the session.
 */
QDF_STATUS populate_dot11f_he_caps(tpAniSirGlobal mac_ctx, tpPESession session,
				   tDot11fIEhe_cap *he_cap)
{
	uint8_t *ppet;
	uint32_t value = 0;
	tSirRetStatus status;

	he_cap->present = 1;

	if (!session) {
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_CONTROL, value);
		he_cap->htc_he = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TWT_REQUESTOR, value);
		he_cap->twt_request = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TWT_RESPONDER, value);
		he_cap->twt_responder = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_FRAGMENTATION, value);
		he_cap->fragmentation = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MAX_FRAG_MSDU, value);
		he_cap->max_num_frag_msdu = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MIN_FRAG_SIZE, value);
		he_cap->min_frag_size = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TRIG_PAD, value);
		he_cap->trigger_frm_mac_pad = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MTID_AGGR, value);
		he_cap->multi_tid_aggr = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_LINK_ADAPTATION, value);
		he_cap->he_link_adaptation = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_ALL_ACK, value);
		he_cap->all_ack = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_UL_MU_RSP_SCHEDULING,
			    value);
		he_cap->ul_mu_rsp_sched = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BUFFER_STATUS_RPT,
			    value);
		he_cap->a_bsr = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BCAST_TWT, value);
		he_cap->broadcast_twt = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BA_32BIT, value);
		he_cap->ba_32bit_bitmap = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MU_CASCADING, value);
		he_cap->mu_cascade = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MULTI_TID, value);
		he_cap->ack_enabled_multitid = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DL_MU_BA, value);
		he_cap->dl_mu_ba = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_OMI, value);
		he_cap->omi_a_ctrl = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_OFDMA_RA, value);
		he_cap->ofdma_ra = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MAX_AMPDU_LEN, value);
		he_cap->max_ampdu_len = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_AMSDU_FRAG, value);
		he_cap->amsdu_frag = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_FLEX_TWT_SCHED, value);
		he_cap->flex_twt_sched = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_RX_CTRL, value);
		he_cap->rx_ctrl_frame = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BSRP_AMPDU_AGGR, value);
		he_cap->bsrp_ampdu_aggr = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_QTP, value);
		he_cap->qtp = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_A_BQR, value);
		he_cap->a_bqr = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_SR_RESPONDER, value);
		he_cap->sr_responder = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_NDP_FEEDBACK_SUPP,
					     value);
		he_cap->ndp_feedback_supp = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_OPS_SUPP, value);
		he_cap->ops_supp = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_AMSDU_IN_AMPDU, value);
		he_cap->amsdu_in_ampdu = value;

		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DUAL_BAND, value);
		he_cap->dual_band = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_CHAN_WIDTH, value);
		he_cap->chan_width_0 = HE_CH_WIDTH_GET_BIT(value, 0);
		he_cap->chan_width_1 = HE_CH_WIDTH_GET_BIT(value, 1);
		he_cap->chan_width_2 = HE_CH_WIDTH_GET_BIT(value, 2);
		he_cap->chan_width_3 = HE_CH_WIDTH_GET_BIT(value, 3);
		he_cap->chan_width_4 = HE_CH_WIDTH_GET_BIT(value, 4);
		he_cap->chan_width_5 = HE_CH_WIDTH_GET_BIT(value, 5);
		he_cap->chan_width_6 = HE_CH_WIDTH_GET_BIT(value, 6);

		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_RX_PREAM_PUNC, value);
		he_cap->rx_pream_puncturing = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_CLASS_OF_DEVICE, value);
		he_cap->device_class = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_LDPC, value);
		he_cap->ldpc_coding = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_LTF_PPDU, value);
		he_cap->he_1x_ltf_800_gi_ppdu = value;
		CFG_GET_INT(status, mac_ctx,
			    WNI_CFG_HE_MIDAMBLE_RX_MAX_NSTS, value);
		he_cap->midamble_rx_max_nsts = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_LTF_NDP, value);
		he_cap->he_4x_ltf_3200_gi_ndp = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TX_STBC_LT80, value);
		he_cap->tx_stbc_lt_80mhz = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_RX_STBC_LT80, value);
		he_cap->rx_stbc_lt_80mhz = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DOPPLER, value);
		he_cap->doppler = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_UL_MUMIMO, value);
		he_cap->ul_mu = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DCM_TX, value);
		he_cap->dcm_enc_tx = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DCM_RX, value);
		he_cap->dcm_enc_rx = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MU_PPDU, value);
		he_cap->ul_he_mu = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_SU_BEAMFORMER, value);
		he_cap->su_beamformer = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_SU_BEAMFORMEE, value);
		he_cap->su_beamformee = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MU_BEAMFORMER, value);
		he_cap->mu_beamformer = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BFEE_STS_LT80, value);
		he_cap->bfee_sts_lt_80 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BFEE_STS_GT80, value);
		he_cap->bfee_sts_gt_80 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_NUM_SOUND_LT80, value);
		he_cap->num_sounding_lt_80 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_NUM_SOUND_GT80, value);
		he_cap->num_sounding_gt_80 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_SU_FEED_TONE16, value);
		he_cap->su_feedback_tone16 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MU_FEED_TONE16, value);
		he_cap->mu_feedback_tone16 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_CODEBOOK_SU, value);
		he_cap->codebook_su = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_CODEBOOK_MU, value);
		he_cap->codebook_mu = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_BFRM_FEED, value);
		he_cap->beamforming_feedback = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_ER_SU_PPDU, value);
		he_cap->he_er_su_ppdu = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_DL_PART_BW, value);
		he_cap->dl_mu_mimo_part_bw = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_PPET_PRESENT, value);
		he_cap->ppet_present = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_SRP, value);
		he_cap->srp = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_POWER_BOOST, value);
		he_cap->power_boost = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_4x_LTF_GI, value);
		he_cap->he_ltf_800_gi_4x = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_MAX_NC, value);
		he_cap->max_nc = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TX_STBC_GT80, value);
		he_cap->tx_stbc_gt_80mhz = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_RX_STBC_GT80, value);
		he_cap->rx_stbc_gt_80mhz = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_ER_4x_LTF_GI, value);
		he_cap->er_he_ltf_800_gi_4x = value;
		CFG_GET_INT(status, mac_ctx,
			    WNI_CFG_HE_PPDU_20_IN_40MHZ_2G, value);
		he_cap->he_ppdu_20_in_40Mhz_2G = value;
		CFG_GET_INT(status, mac_ctx,
			    WNI_CFG_HE_PPDU_20_IN_160_80P80MHZ, value);
		he_cap->he_ppdu_20_in_160_80p80Mhz = value;
		CFG_GET_INT(status, mac_ctx,
			    WNI_CFG_HE_PPDU_80_IN_160_80P80MHZ, value);
		he_cap->he_ppdu_80_in_160_80p80Mhz = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_ER_1X_HE_LTF_GI, value);
		he_cap->er_1x_he_ltf_gi = value;
		CFG_GET_INT(status, mac_ctx,
			    WNI_CFG_HE_MIDAMBLE_RX_1X_HE_LTF, value);
		he_cap->midamble_rx_1x_he_ltf = value;
		CFG_GET_INT(status, mac_ctx,
			WNI_CFG_HE_RX_MCS_MAP_LT_80, value);
		he_cap->rx_he_mcs_map_lt_80 = value;
		CFG_GET_INT(status, mac_ctx,
			WNI_CFG_HE_TX_MCS_MAP_LT_80, value);
		he_cap->tx_he_mcs_map_lt_80 = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_RX_MCS_MAP_160, value);
		*((uint16_t *)he_cap->rx_he_mcs_map_160) = value;
		CFG_GET_INT(status, mac_ctx, WNI_CFG_HE_TX_MCS_MAP_160, value);
		*((uint16_t *)he_cap->tx_he_mcs_map_160) = value;
		CFG_GET_INT(status, mac_ctx,
			WNI_CFG_HE_RX_MCS_MAP_80_80, value);
		*((uint16_t *)he_cap->rx_he_mcs_map_80_80) = value;
		CFG_GET_INT(status, mac_ctx,
			WNI_CFG_HE_TX_MCS_MAP_80_80, value);
		*((uint16_t *)he_cap->tx_he_mcs_map_80_80) = value;

		if (he_cap->ppet_present) {
			value = WNI_CFG_HE_PPET_LEN;
			/* if session less then take 5g cap */
			CFG_GET_STR(status, mac_ctx, WNI_CFG_HE_PPET_5G,
				    he_cap->ppet.ppe_threshold.ppe_th,
				    value, value);
			ppet = he_cap->ppet.ppe_threshold.ppe_th;
			he_cap->ppet.ppe_threshold.num_ppe_th =
						lim_truncate_ppet(ppet, value);
		} else {
			he_cap->ppet.ppe_threshold.num_ppe_th = 0;
		}

		return QDF_STATUS_SUCCESS;
	}

	qdf_mem_copy(he_cap, &session->he_config, sizeof(*he_cap));
	if (he_cap->ppet_present) {
		value = WNI_CFG_HE_PPET_LEN;
		/* if session is present, populate PPET based on band */
		if (IS_5G_CH(session->currentOperChannel))
			CFG_GET_STR(status, mac_ctx, WNI_CFG_HE_PPET_5G,
				    he_cap->ppet.ppe_threshold.ppe_th,
				    value, value);
		else
			CFG_GET_STR(status, mac_ctx, WNI_CFG_HE_PPET_2G,
				    he_cap->ppet.ppe_threshold.ppe_th,
				    value, value);
		ppet = he_cap->ppet.ppe_threshold.ppe_th;
		he_cap->ppet.ppe_threshold.num_ppe_th =
						lim_truncate_ppet(ppet, value);
	} else {
		he_cap->ppet.ppe_threshold.num_ppe_th = 0;
	}

	lim_log_he_cap(mac_ctx, he_cap);

	return QDF_STATUS_SUCCESS;
}

/**
 * populate_dot11f_he_operation() - pouldate HE Operation IE
 * @mac_ctx: Global MAC context
 * @session: PE session
 * @he_op: pointer to HE Operation IE
 *
 * Populdate the HE Operation IE based on the session.
 */
QDF_STATUS populate_dot11f_he_operation(tpAniSirGlobal mac_ctx,
			tpPESession session, tDot11fIEhe_op *he_op)
{
	qdf_mem_copy(he_op, &session->he_op, sizeof(*he_op));

	he_op->vht_oper_present = 1;
	he_op->present = 1;
	if (session->ch_width > CH_WIDTH_40MHZ) {
		he_op->vht_oper.info.chan_width = 1;
		he_op->vht_oper.info.center_freq_seg0 =
			session->ch_center_freq_seg0;
		if (session->ch_width == CH_WIDTH_80P80MHZ ||
				session->ch_width == CH_WIDTH_160MHZ)
			he_op->vht_oper.info.center_freq_seg1 =
				session->ch_center_freq_seg1;
		else
			he_op->vht_oper.info.center_freq_seg1 = 0;
	} else {
		he_op->vht_oper.info.chan_width = 0;
		he_op->vht_oper.info.center_freq_seg0 = 0;
		he_op->vht_oper.info.center_freq_seg1 = 0;
	}
	lim_log_he_op(mac_ctx, he_op);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
/**
 * populate_dot11f_he_bss_color_change() - pouldate HE BSS color change IE
 * @mac_ctx: Global MAC context
 * @session: PE session
 * @he_bss_color: pointer to HE BSS color change IE
 *
 * Populdate the HE BSS color change IE based on the session.
 */
QDF_STATUS populate_dot11f_he_bss_color_change(tpAniSirGlobal mac_ctx,
				tpPESession session,
				tDot11fIEbss_color_change *he_bss_color)
{
	if (!session->bss_color_changing) {
		he_bss_color->present = 0;
		return QDF_STATUS_SUCCESS;
	}

	he_bss_color->present = 1;
	he_bss_color->countdown = session->he_bss_color_change.countdown;
	he_bss_color->new_color = session->he_bss_color_change.new_color;

	lim_log_he_bss_color(mac_ctx, he_bss_color);

	return QDF_STATUS_SUCCESS;
}
#endif
#endif

/* parser_api.c ends here. */
