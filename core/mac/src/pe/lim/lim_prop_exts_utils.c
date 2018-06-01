/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "wma.h"
#include "wlan_utility.h"

#define LIM_GET_NOISE_MAX_TRY 5

#ifdef FEATURE_WLAN_ESE
/**
 * get_local_power_constraint_probe_response() - extracts local constraint
 * from probe response
 * @beacon_struct: beacon structure
 * @local_constraint: local constraint pointer
 * @session: A pointer to session entry.
 *
 * Return: None
 */
static void get_local_power_constraint_probe_response(
		tpSirProbeRespBeacon beacon_struct,
		int8_t *local_constraint,
		tpPESession session)
{
	if (beacon_struct->eseTxPwr.present)
		*local_constraint =
			beacon_struct->eseTxPwr.power_limit;
}

/**
 * get_ese_version_ie_probe_response() - extracts ESE version IE
 * from probe response
 * @beacon_struct: beacon structure
 * @session: A pointer to session entry.
 *
 * Return: None
 */
static void get_ese_version_ie_probe_response(tpAniSirGlobal mac_ctx,
					tpSirProbeRespBeacon beacon_struct,
					tpPESession session)
{
	if (mac_ctx->roam.configParam.isEseIniFeatureEnabled)
		session->is_ese_version_ie_present =
			beacon_struct->is_ese_ver_ie_present;
}
#else
static void get_local_power_constraint_probe_response(
		tpSirProbeRespBeacon beacon_struct,
		int8_t *local_constraint,
		tpPESession session)
{

}

static inline void get_ese_version_ie_probe_response(tpAniSirGlobal mac_ctx,
					tpSirProbeRespBeacon beacon_struct,
					tpPESession session)
{
}
#endif

#ifdef WLAN_FEATURE_11AX
static void lim_extract_he_op(tpPESession session,
		tSirProbeRespBeacon *beacon_struct)
{
	if (session->he_capable && beacon_struct->he_op.present) {
		qdf_mem_copy(&session->he_op, &beacon_struct->he_op,
				sizeof(session->he_op));
		pe_debug("he_op.bss_color %d", session->he_op.bss_color);
		pe_debug("he_op.default_pe %d", session->he_op.default_pe);
	}
}

static bool lim_check_he_80_mcs11_supp(tpPESession session,
		tSirProbeRespBeacon *beacon_struct) {
	uint8_t rx_mcs_map;
	uint8_t tx_mcs_map;
	rx_mcs_map = beacon_struct->he_cap.rx_he_mcs_map_lt_80;
	tx_mcs_map = beacon_struct->he_cap.tx_he_mcs_map_lt_80;
	if ((session->nss == NSS_1x1_MODE) &&
		((HE_GET_MCS_4_NSS(rx_mcs_map, 1) == HE_MCS_0_11) ||
		(HE_GET_MCS_4_NSS(tx_mcs_map, 1) == HE_MCS_0_11)))
		return true;

	if ((session->nss == NSS_2x2_MODE) &&
		((HE_GET_MCS_4_NSS(rx_mcs_map, 2) == HE_MCS_0_11) ||
		(HE_GET_MCS_4_NSS(tx_mcs_map, 2) == HE_MCS_0_11)))
		return true;

	return false;
}

static void lim_check_he_ldpc_cap(tpPESession session,
		tSirProbeRespBeacon *beacon_struct)
{
	if (session->he_capable && beacon_struct->he_cap.present) {
		if (beacon_struct->he_cap.ldpc_coding)
			return;
		else if ((session->ch_width == CH_WIDTH_20MHZ) &&
				!lim_check_he_80_mcs11_supp(session,
					beacon_struct))
			return;
		session->he_capable = false;
		pe_err("LDPC check failed for HE operation");
		if (session->vhtCapability) {
			session->dot11mode = WNI_CFG_DOT11_MODE_11AC;
			pe_debug("Update dot11mode to 11ac");
		} else {
			session->dot11mode = WNI_CFG_DOT11_MODE_11N;
			pe_debug("Update dot11mode to 11N");
		}
	}
}
#else
static inline void lim_extract_he_op(tpPESession session,
		tSirProbeRespBeacon *beacon_struct)
{}
static void lim_check_he_ldpc_cap(tpPESession session,
		tSirProbeRespBeacon *beacon_struct)
{}
#endif

static void lim_objmgr_update_vdev_nss(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id, uint8_t nss)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_LEGACY_MAC_ID);
	if (!vdev) {
		pe_err("vdev not found for id: %d", vdev_id);
		return;
	}
	wlan_vdev_obj_lock(vdev);
	wlan_vdev_mlme_set_nss(vdev, nss);
	wlan_vdev_obj_unlock(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_MAC_ID);
}
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
	int8_t *local_constraint, tpPESession session)
{
	tSirProbeRespBeacon *beacon_struct;
	uint32_t enable_txbf_20mhz;
	QDF_STATUS cfg_get_status = QDF_STATUS_E_FAILURE;
	uint8_t ap_bcon_ch_width;
	bool new_ch_width_dfn = false;
	tDot11fIEVHTOperation *vht_op;
	uint8_t fw_vht_ch_wd;
	uint8_t vht_ch_wd;
	uint8_t center_freq_diff;
	struct s_ext_cap *ext_cap;

	beacon_struct = qdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == beacon_struct) {
		pe_err("Unable to allocate memory");
		return;
	}

	*qos_cap = 0;
	*prop_cap = 0;
	*uapsd = 0;
	pe_debug("In lim_extract_ap_capability: The IE's being received:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
			   p_ie, ie_len);
	if (sir_parse_beacon_ie(mac_ctx, beacon_struct, p_ie,
		(uint32_t) ie_len) != QDF_STATUS_SUCCESS) {
		pe_err("sir_parse_beacon_ie failed to parse beacon");
		qdf_mem_free(beacon_struct);
		return;
	}

	if (beacon_struct->wmeInfoPresent ||
	    beacon_struct->wmeEdcaPresent ||
	    beacon_struct->HTCaps.present)
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

	pe_debug("Bcon: VHTCap.present: %d SU Beamformer: %d BSS_VHT_CAPABLE: %d",
		beacon_struct->VHTCaps.present,
		beacon_struct->VHTCaps.suBeamFormerCap,
		IS_BSS_VHT_CAPABLE(beacon_struct->VHTCaps));

	vht_op = &beacon_struct->VHTOperation;
	if (IS_BSS_VHT_CAPABLE(beacon_struct->VHTCaps) &&
			vht_op->present &&
			session->vhtCapability) {
		session->vhtCapabilityPresentInBeacon = 1;
		if (((beacon_struct->Vendor1IEPresent &&
		      beacon_struct->vendor_vht_ie.present &&
		      beacon_struct->Vendor3IEPresent)) &&
		      (((beacon_struct->VHTCaps.txMCSMap & VHT_MCS_3x3_MASK) ==
			VHT_MCS_3x3_MASK) &&
		      ((beacon_struct->VHTCaps.txMCSMap & VHT_MCS_2x2_MASK) !=
		       VHT_MCS_2x2_MASK)))
			session->vht_config.su_beam_formee = 0;
	} else {
		session->vhtCapabilityPresentInBeacon = 0;
	}

	if (session->vhtCapabilityPresentInBeacon == 1 &&
			!session->htSupportedChannelWidthSet) {
		cfg_get_status = wlan_cfg_get_int(mac_ctx,
				WNI_CFG_VHT_ENABLE_TXBF_20MHZ,
				&enable_txbf_20mhz);
		if ((QDF_IS_STATUS_SUCCESS(cfg_get_status)) &&
				(false == enable_txbf_20mhz))
			session->vht_config.su_beam_formee = 0;
	} else if (session->vhtCapabilityPresentInBeacon &&
			vht_op->chanWidth) {
		/* If VHT is supported min 80 MHz support is must */
		ap_bcon_ch_width = vht_op->chanWidth;
		if ((ap_bcon_ch_width == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) &&
		     vht_op->chanCenterFreqSeg2) {
			new_ch_width_dfn = true;
			if (vht_op->chanCenterFreqSeg2 >
			    vht_op->chanCenterFreqSeg1)
			    center_freq_diff = vht_op->chanCenterFreqSeg2 -
				    vht_op->chanCenterFreqSeg1;
			else
			    center_freq_diff = vht_op->chanCenterFreqSeg1 -
				    vht_op->chanCenterFreqSeg2;
			if (center_freq_diff == 8)
				ap_bcon_ch_width =
					WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;
			else if (center_freq_diff > 16)
				ap_bcon_ch_width =
					WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ;
		}

		fw_vht_ch_wd = wma_get_vht_ch_width();
		vht_ch_wd = QDF_MIN(fw_vht_ch_wd, ap_bcon_ch_width);
		/*
		 * If the supported channel width is greater than 80MHz and
		 * AP supports Nss > 1 in 160MHz mode then connect the STA
		 * in 2x2 80MHz mode instead of connecting in 160MHz mode.
		 */
		if ((vht_ch_wd > WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) &&
				mac_ctx->sta_prefer_80MHz_over_160MHz) {
			if (!(IS_VHT_NSS_1x1(beacon_struct->VHTCaps.txMCSMap))
					&&
			   (!IS_VHT_NSS_1x1(beacon_struct->VHTCaps.rxMCSMap)))
				vht_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
		}
		/*
		 * VHT OP IE old definition:
		 * vht_op->chanCenterFreqSeg1: center freq of 80MHz/160MHz/
		 * primary 80 in 80+80MHz.
		 *
		 * vht_op->chanCenterFreqSeg2: center freq of secondary 80
		 * in 80+80MHz.
		 *
		 * VHT OP IE NEW definition:
		 * vht_op->chanCenterFreqSeg1: center freq of 80MHz/primary
		 * 80 in 80+80MHz/center freq of the 80 MHz channel segment
		 * that contains the primary channel in 160MHz mode.
		 *
		 * vht_op->chanCenterFreqSeg2: center freq of secondary 80
		 * in 80+80MHz/center freq of 160MHz.
		 */
		session->ch_center_freq_seg0 = vht_op->chanCenterFreqSeg1;
		session->ch_center_freq_seg1 = vht_op->chanCenterFreqSeg2;
		if (vht_ch_wd == WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ) {
			/* DUT or AP supports only 160MHz */
			if (ap_bcon_ch_width ==
					WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ) {
				/* AP is in 160MHz mode */
				if (!new_ch_width_dfn) {
					session->ch_center_freq_seg1 =
						vht_op->chanCenterFreqSeg1;
					session->ch_center_freq_seg0 =
						lim_get_80Mhz_center_channel(
						beacon_struct->channelNumber);
				}
			} else {
				/* DUT supports only 160MHz and AP is
				 * in 80+80 mode
				 */
				vht_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
				session->ch_center_freq_seg1 = 0;
			}
		} else if (vht_ch_wd == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) {
			/* DUT or AP supports only 80MHz */
			if (ap_bcon_ch_width ==
					WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ &&
					!new_ch_width_dfn)
				/* AP is in 160MHz mode */
				session->ch_center_freq_seg0 =
					lim_get_80Mhz_center_channel(
						beacon_struct->channelNumber);
			else
				session->ch_center_freq_seg1 = 0;
		}
		session->ch_width = vht_ch_wd + 1;
		pe_debug("cntr_freq0: %d cntr_freq1: %d width: %d",
				session->ch_center_freq_seg0,
				session->ch_center_freq_seg1,
				session->ch_width);
		if (CH_WIDTH_80MHZ < session->ch_width) {
			session->vht_config.su_beam_former = 0;
			session->nss = 1;
		}
	}

	if (session->vhtCapability &&
		session->vhtCapabilityPresentInBeacon &&
		beacon_struct->ext_cap.present) {
		ext_cap = (struct s_ext_cap *)beacon_struct->ext_cap.bytes;
		session->gLimOperatingMode.present =
			ext_cap->oper_mode_notification;
		if (ext_cap->oper_mode_notification) {
			if (CH_WIDTH_160MHZ > session->ch_width)
				session->gLimOperatingMode.chanWidth =
						session->ch_width;
			else
				session->gLimOperatingMode.chanWidth =
					CH_WIDTH_160MHZ;
		} else {
			pe_err("AP does not support op_mode rx");
		}
	}
	lim_check_he_ldpc_cap(session, beacon_struct);
	lim_extract_he_op(session, beacon_struct);
	/* Extract the UAPSD flag from WMM Parameter element */
	if (beacon_struct->wmeEdcaPresent)
		*uapsd = beacon_struct->edcaParams.qosInfo.uapsd;

	if (mac_ctx->roam.configParam.allow_tpc_from_ap) {
		if (beacon_struct->powerConstraintPresent) {
			*local_constraint -=
				beacon_struct->localPowerConstraint.
					localPowerConstraints;
		} else {
			get_local_power_constraint_probe_response(
				beacon_struct, local_constraint, session);
		}
	}

	get_ese_version_ie_probe_response(mac_ctx, beacon_struct, session);

	session->country_info_present = false;
	/* Initializing before first use */
	if (beacon_struct->countryInfoPresent)
		session->country_info_present = true;
	/* Check if Extended caps are present in probe resp or not */
	if (beacon_struct->ext_cap.present)
		session->is_ext_caps_present = true;
	/* Update HS 2.0 Information Element */
	if (beacon_struct->hs20vendor_ie.present) {
		pe_debug("HS20 Indication Element Present, rel#: %u id: %u",
			beacon_struct->hs20vendor_ie.release_num,
			beacon_struct->hs20vendor_ie.hs_id_present);
		qdf_mem_copy(&session->hs20vendor_ie,
			&beacon_struct->hs20vendor_ie,
			sizeof(tDot11fIEhs20vendor_ie) -
			sizeof(beacon_struct->hs20vendor_ie.hs_id));
		if (beacon_struct->hs20vendor_ie.hs_id_present)
			qdf_mem_copy(&session->hs20vendor_ie.hs_id,
				&beacon_struct->hs20vendor_ie.hs_id,
				sizeof(beacon_struct->hs20vendor_ie.hs_id));
	}

	lim_objmgr_update_vdev_nss(mac_ctx->psoc, session->smeSessionId,
				   session->nss);
	qdf_mem_free(beacon_struct);
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
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
		return PHY_SINGLE_CHANNEL_CENTERED;
	default:
		return PHY_SINGLE_CHANNEL_CENTERED;
	}
}
