/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
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
 * This file lim_prop_exts_utils.cc contains the utility functions
 * to populate, parse proprietary extensions required to
 * support ANI feature set.
 *
 * Author:        Chandra Modumudi
 * Date:          11/27/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "ani_global.h"
#include "wni_cfg.h"
#include "sir_common.h"
#include "sir_debug.h"
#include "utils_api.h"
#include "cfg_api.h"
#include "lim_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_trace.h"
#ifdef WLAN_FEATURE_VOWIFI_11R
#include "lim_ft_defs.h"
#endif
#include "lim_session.h"
#include "wma.h"

#define LIM_GET_NOISE_MAX_TRY 5
/**
 * lim_extract_ap_capability() - extract AP's HCF/WME/WSM capability
 * @mac_ctx: Pointer to Global MAC structure
 * @p_ie: Pointer to starting IE in Beacon/Probe Response
 * @ie_len: Length of all IEs combined
 * @qos_cap: Bits are set according to capabilities
 * @prop_cap: Pointer to prop info IE.
 * @uapsd: pointer to uapsd
 * @local_constraint: Pointer to local power constraint.
 * @session: A pointer to session entry.
 *
 * This function is called to extract AP's HCF/WME/WSM capability
 * from the IEs received from it in Beacon/Probe Response frames
 *
 * Return: None
 */
void
lim_extract_ap_capability(tpAniSirGlobal mac_ctx, uint8_t *p_ie,
	uint16_t ie_len, uint8_t *qos_cap, uint16_t *prop_cap, uint8_t *uapsd,
	tPowerdBm *local_constraint, tpPESession session)
{
	tSirProbeRespBeacon *beacon_struct;
#if !defined WLAN_FEATURE_VOWIFI
	uint32_t local_power_constraints = 0;
#endif
	uint32_t enable_txbf_20mhz;
	tSirRetStatus cfg_set_status = eSIR_FAILURE;
	tSirRetStatus cfg_get_status = eSIR_FAILURE;

	beacon_struct = cdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == beacon_struct) {
		lim_log(mac_ctx, LOGE, FL("Unable to allocate memory"));
		return;
	}

	cdf_mem_set((uint8_t *) beacon_struct, sizeof(tSirProbeRespBeacon), 0);
	*qos_cap = 0;
	*prop_cap = 0;
	*uapsd = 0;
	lim_log(mac_ctx, LOG3,
		FL("In lim_extract_ap_capability: The IE's being received:"));
	sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG3, p_ie, ie_len);
	if (sir_parse_beacon_ie(mac_ctx, beacon_struct, p_ie,
		(uint32_t) ie_len) == eSIR_SUCCESS) {
		if (beacon_struct->wmeInfoPresent
		    || beacon_struct->wmeEdcaPresent)
			LIM_BSS_CAPS_SET(WME, *qos_cap);
		if (LIM_BSS_CAPS_GET(WME, *qos_cap)
		    && beacon_struct->wsmCapablePresent)
			LIM_BSS_CAPS_SET(WSM, *qos_cap);
		if (beacon_struct->propIEinfo.capabilityPresent)
			*prop_cap = beacon_struct->propIEinfo.capability;
		if (beacon_struct->HTCaps.present)
			mac_ctx->lim.htCapabilityPresentInBeacon = 1;
		else
			mac_ctx->lim.htCapabilityPresentInBeacon = 0;

#ifdef WLAN_FEATURE_11AC
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO_MED,
			  "beacon.VHTCaps.present = %d BSS_VHT_Capable:%d",
			  beacon_struct->VHTCaps.present,
			  IS_BSS_VHT_CAPABLE(beacon_struct->VHTCaps));
		CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO_MED,
			  "***beacon.SU Beamformer Capable*****=%d",
			  beacon_struct->VHTCaps.suBeamFormerCap);

		if (IS_BSS_VHT_CAPABLE(beacon_struct->VHTCaps) &&
			beacon_struct->VHTOperation.present &&
			session->vhtCapability) {
			session->vhtCapabilityPresentInBeacon = 1;
			session->vhtTxChannelWidthSet =
				beacon_struct->VHTOperation.chanWidth;
			if (((beacon_struct->Vendor1IEPresent &&
				beacon_struct->vendor2_ie.present &&
				beacon_struct->Vendor3IEPresent)) &&
				(((beacon_struct->VHTCaps.txMCSMap &
				    VHT_MCS_3x3_MASK) == VHT_MCS_3x3_MASK) &&
				  ((beacon_struct->VHTCaps.txMCSMap &
				    VHT_MCS_2x2_MASK) != VHT_MCS_2x2_MASK))) {
				session->txBFIniFeatureEnabled = 0;
			}
		} else {
			session->vhtCapabilityPresentInBeacon = 0;
		}

		if (session->vhtCapabilityPresentInBeacon == 1 &&
		    session->txBFIniFeatureEnabled == 0) {
			cfg_set_status = cfg_set_int(mac_ctx,
						WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
						0);
			if (cfg_set_status != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGP,
					FL("Set VHT_SU_BEAMFORMEE_CAP Fail"));
		}
		if (session->vhtCapabilityPresentInBeacon == 1 &&
		    !session->htSupportedChannelWidthSet) {
			cfg_get_status = wlan_cfg_get_int(mac_ctx,
						WNI_CFG_VHT_ENABLE_TXBF_20MHZ,
						&enable_txbf_20mhz);
			if ((IS_SIR_STATUS_SUCCESS(cfg_get_status)) &&
			    (false == enable_txbf_20mhz))
				session->txBFIniFeatureEnabled = 0;
		} else if (session->vhtCapabilityPresentInBeacon == 1 &&
			   beacon_struct->VHTOperation.chanWidth) {
			/* If VHT is supported min 80 MHz support is must */
			uint32_t fw_vht_ch_wd = wma_get_vht_ch_width();
			uint32_t vht_ch_wd = CDF_MIN(fw_vht_ch_wd,
					beacon_struct->VHTOperation.chanWidth);
			if (vht_ch_wd == beacon_struct->VHTOperation.chanWidth
			    || vht_ch_wd >= WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ) {
				/*
				 * This block covers 2 cases:
				 * 1) AP and STA both have same vht capab
				 * 2) AP is 160 (80+80), we are 160 only
				 */
				session->ch_center_freq_seg0 =
				 beacon_struct->VHTOperation.chanCenterFreqSeg1;
				session->ch_center_freq_seg1 =
				 beacon_struct->VHTOperation.chanCenterFreqSeg2;
			} else {
				/* when AP was 160 but we were 80 only */
				session->ch_center_freq_seg0 =
					lim_get_80Mhz_center_channel(
						beacon_struct->channelNumber);
			}
			session->ch_width = vht_ch_wd + 1;
			if (CH_WIDTH_80MHZ < session->ch_width) {
				session->enable_su_tx_bformer = 0;
				session->nss = 1;
			}
		}
		if (session->vhtCapabilityPresentInBeacon == 1 &&
		    !session->htSupportedChannelWidthSet &&
		    session->txBFIniFeatureEnabled == 0) {
			cfg_set_status = cfg_set_int(mac_ctx,
						WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
						0);
			if (cfg_set_status != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGP,
					FL("Set VHT_SU_BEAMFORMEE_CAP Fail"));
		}
#endif
		/* Extract the UAPSD flag from WMM Parameter element */
		if (beacon_struct->wmeEdcaPresent)
			*uapsd = beacon_struct->edcaParams.qosInfo.uapsd;
#if defined FEATURE_WLAN_ESE
		/* If there is Power Constraint Element specifically,
		 * adapt to it. Hence there is else condition check
		 * for this if statement.
		 */
		if (beacon_struct->eseTxPwr.present)
			*local_constraint = beacon_struct->eseTxPwr.power_limit;
		session->is_ese_version_ie_present =
			beacon_struct->is_ese_ver_ie_present;
#endif
		if (beacon_struct->powerConstraintPresent) {
#if defined WLAN_FEATURE_VOWIFI
			*local_constraint -=
				beacon_struct->localPowerConstraint.
				localPowerConstraints;
#else
			local_power_constraints =
				(uint32_t) beacon_struct->localPowerConstraint.
				localPowerConstraints;
#endif
		}
#if !defined WLAN_FEATURE_VOWIFI
		if (cfg_set_int
			    (mac_ctx, WNI_CFG_LOCAL_POWER_CONSTRAINT,
			    local_power_constraints) != eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGP,
				FL
					("Could not update local power constraint to cfg."));
		}
#endif
		session->country_info_present = false;
		/* Initializing before first use */
		if (beacon_struct->countryInfoPresent)
			session->country_info_present = true;
	}
	cdf_mem_free(beacon_struct);
	return;
} /****** end lim_extract_ap_capability() ******/

/**
 * lim_get_htcb_state
 *
 ***FUNCTION:
 * This routing provides the translation of Airgo Enum to HT enum for determining
 * secondary channel offset.
 * Airgo Enum is required for backward compatibility purposes.
 *
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return The corresponding HT enumeration
 */
ePhyChanBondState lim_get_htcb_state(ePhyChanBondState aniCBMode)
{
	switch (aniCBMode) {
#ifdef WLAN_FEATURE_11AC
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
#endif
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
#ifdef WLAN_FEATURE_11AC
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
#endif
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
#ifdef WLAN_FEATURE_11AC
	case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
		return PHY_SINGLE_CHANNEL_CENTERED;
#endif
	default:
		return PHY_SINGLE_CHANNEL_CENTERED;
	}
}

/*
 * lim_get_sta_peer_type
 *
 ***FUNCTION:
 * This API returns STA peer type
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pStaDs - Pointer to the tpDphHashNode of the STA
 *         under consideration
 * @return tStaRateMode
 */
tStaRateMode lim_get_sta_peer_type(tpAniSirGlobal pMac,
				   tpDphHashNode pStaDs, tpPESession psessionEntry)
{
	tStaRateMode staPeerType = eSTA_11b;
#ifdef WLAN_FEATURE_11AC
	if (pStaDs->mlmStaContext.vhtCapability)
		staPeerType = eSTA_11ac;
#endif
	else if (pStaDs->mlmStaContext.htCapability)
		staPeerType = eSTA_11n;
	else if (pStaDs->erpEnabled)
		staPeerType = eSTA_11bg;
	else if (psessionEntry->limRFBand == SIR_BAND_5_GHZ)
		staPeerType = eSTA_11a;
	return staPeerType;
}
