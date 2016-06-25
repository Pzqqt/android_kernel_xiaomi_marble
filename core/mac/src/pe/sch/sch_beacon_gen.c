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
 * This file sch_beacon_gen.cc contains beacon generation related
 * functions
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "cds_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "sir_mac_prot_def.h"

#include "lim_utils.h"
#include "lim_api.h"

#include "wma_if.h"
#include "cfg_api.h"
#include "sch_api.h"

#include "parser_api.h"

#include "sch_debug.h"

/* */
/* March 15, 2006 */
/* Temporarily (maybe for all of Alpha-1), assuming TIM = 0 */
/* */

const uint8_t p2p_oui[] = { 0x50, 0x6F, 0x9A, 0x9 };

tSirRetStatus sch_get_p2p_ie_offset(uint8_t *pExtraIe, uint32_t extraIeLen,
				    uint16_t *pP2pIeOffset)
{
	tSirRetStatus status = eSIR_FAILURE;
	*pP2pIeOffset = 0;

	/* Extra IE is not present */
	if (0 == extraIeLen) {
		return status;
	}
	/* Calculate the P2P IE Offset */
	do {
		if (*pExtraIe == 0xDD) {
			if (!qdf_mem_cmp
				    ((void *)(pExtraIe + 2), &p2p_oui, sizeof(p2p_oui))) {
				status = eSIR_SUCCESS;
				break;
			}
		}

		(*pP2pIeOffset)++;
		pExtraIe++;
	} while (--extraIeLen > 0);

	return status;
}

/**
 * sch_append_addn_ie() - adds additional IEs to frame
 * @mac_ctx:       mac global context
 * @session:       pe session pointer
 * @frm:           frame where additional IE is to be added
 * @max_bcn_size:  max beacon size
 * @num_bytes:     final size
 * @addn_ie:       pointer to additional IE
 * @addn_ielen:    lenght of additional IE
 *
 * Return: status of operation
 */
tSirRetStatus
sch_append_addn_ie(tpAniSirGlobal mac_ctx, tpPESession session,
		   uint8_t *frm, uint32_t max_bcn_size, uint32_t *num_bytes,
		   uint8_t *addn_ie, uint16_t addn_ielen)
{
	tSirRetStatus status = eSIR_FAILURE;
	uint8_t add_ie[WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN];
	uint8_t *p2p_ie = NULL;
	uint8_t noa_len = 0;
	uint8_t noa_strm[SIR_MAX_NOA_ATTR_LEN + SIR_P2P_IE_HEADER_LEN];
	bool valid_ie;

	valid_ie = (addn_ielen <= WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN &&
		    addn_ielen && ((addn_ielen + *num_bytes) <= max_bcn_size));

	if (!valid_ie)
		return status;

	qdf_mem_copy(&add_ie[0], addn_ie, addn_ielen);

	p2p_ie = limGetP2pIEPtr(mac_ctx, &add_ie[0], addn_ielen);
	if ((p2p_ie != NULL) && !mac_ctx->beacon_offload) {
		/* get NoA attribute stream P2P IE */
		noa_len = lim_get_noa_attr_stream(mac_ctx, noa_strm, session);
		if (noa_len) {
			if ((noa_len + addn_ielen) <=
			    WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN) {
				qdf_mem_copy(&add_ie[addn_ielen], noa_strm,
					     noa_len);
				addn_ielen += noa_len;
				p2p_ie[1] += noa_len;
			} else {
				sch_log(mac_ctx, LOGE,
					FL("Not able to insert NoA because of length constraint"));
			}
		}
	}
	if (addn_ielen <= WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN) {
		qdf_mem_copy(frm, &add_ie[0], addn_ielen);
		*num_bytes = *num_bytes + addn_ielen;
	} else {
		sch_log(mac_ctx, LOGW,
			FL("Not able to insert because of len constraint %d"),
			addn_ielen);
	}
	return status;
}

/**
 * sch_set_fixed_beacon_fields() - sets the fixed params in beacon frame
 * @mac_ctx:       mac global context
 * @session:       pe session entry
 * @band:          out param, band caclculated
 * @opr_ch:        operating channels
 *
 * Return: status of operation
 */

tSirRetStatus
sch_set_fixed_beacon_fields(tpAniSirGlobal mac_ctx, tpPESession session)
{
	tpAniBeaconStruct bcn_struct = (tpAniBeaconStruct)
						session->pSchBeaconFrameBegin;
	tpSirMacMgmtHdr mac;
	uint16_t offset;
	uint8_t *ptr;
	tDot11fBeacon1 *bcn_1;
	tDot11fBeacon2 *bcn_2;
	uint32_t i, n_status, n_bytes;
	uint32_t wps_ap_enable = 0, tmp;
	tDot11fIEWscProbeRes *wsc_prb_res;
	uint8_t *extra_ie = NULL;
	uint32_t extra_ie_len = 0;
	uint16_t extra_ie_offset = 0;
	uint16_t p2p_ie_offset = 0;
	tSirRetStatus status = eSIR_SUCCESS;
	bool is_vht_enabled = false;
	uint16_t addn_ielen = 0;
	uint8_t *addn_ie = NULL;
	tDot11fIEExtCap extracted_extcap;
	bool extcap_present = true, addnie_present = false;

	bcn_1 = qdf_mem_malloc(sizeof(tDot11fBeacon1));
	if (NULL == bcn_1) {
		sch_log(mac_ctx, LOGE, FL("Failed to allocate memory"));
		return eSIR_MEM_ALLOC_FAILED;
	}

	bcn_2 = qdf_mem_malloc(sizeof(tDot11fBeacon2));
	if (NULL == bcn_2) {
		sch_log(mac_ctx, LOGE, FL("Failed to allocate memory"));
		qdf_mem_free(bcn_1);
		return eSIR_MEM_ALLOC_FAILED;
	}

	wsc_prb_res = qdf_mem_malloc(sizeof(tDot11fIEWscProbeRes));
	if (NULL == wsc_prb_res) {
		sch_log(mac_ctx, LOGE, FL("Failed to allocate memory"));
		qdf_mem_free(bcn_1);
		qdf_mem_free(bcn_2);
		return eSIR_MEM_ALLOC_FAILED;
	}

	sch_log(mac_ctx, LOG1, FL("Setting fixed beacon fields"));

	/*
	 * First set the fixed fields:
	 * set the TFP headers, set the mac header
	 */
	qdf_mem_set((uint8_t *) &bcn_struct->macHdr, sizeof(tSirMacMgmtHdr), 0);
	mac = (tpSirMacMgmtHdr) &bcn_struct->macHdr;
	mac->fc.type = SIR_MAC_MGMT_FRAME;
	mac->fc.subType = SIR_MAC_MGMT_BEACON;

	for (i = 0; i < 6; i++)
		mac->da[i] = 0xff;

	qdf_mem_copy(mac->sa, session->selfMacAddr,
		     sizeof(session->selfMacAddr));
	qdf_mem_copy(mac->bssId, session->bssId, sizeof(session->bssId));

	mac->fc.fromDS = 0;
	mac->fc.toDS = 0;

	/* Now set the beacon body */
	qdf_mem_set((uint8_t *) bcn_1, sizeof(tDot11fBeacon1), 0);

	/* Skip over the timestamp (it'll be updated later). */
	bcn_1->BeaconInterval.interval =
		mac_ctx->sch.schObject.gSchBeaconInterval;
	populate_dot11f_capabilities(mac_ctx, &bcn_1->Capabilities, session);
	if (session->ssidHidden) {
		bcn_1->SSID.present = 1;
		/* rest of the fileds are 0 for hidden ssid */
		if ((session->ssId.length) &&
		    (session->ssidHidden == eHIDDEN_SSID_ZERO_CONTENTS))
			bcn_1->SSID.num_ssid = session->ssId.length;
	} else {
		populate_dot11f_ssid(mac_ctx, &session->ssId, &bcn_1->SSID);
	}

	populate_dot11f_supp_rates(mac_ctx, POPULATE_DOT11F_RATES_OPERATIONAL,
				   &bcn_1->SuppRates, session);
	populate_dot11f_ds_params(mac_ctx, &bcn_1->DSParams,
				  session->currentOperChannel);
	populate_dot11f_ibss_params(mac_ctx, &bcn_1->IBSSParams, session);

	offset = sizeof(tAniBeaconStruct);
	ptr = session->pSchBeaconFrameBegin + offset;

	if (LIM_IS_AP_ROLE(session)) {
		/* Initialize the default IE bitmap to zero */
		qdf_mem_set((uint8_t *) &(session->DefProbeRspIeBitmap),
			    (sizeof(uint32_t) * 8), 0);

		/* Initialize the default IE bitmap to zero */
		qdf_mem_set((uint8_t *) &(session->probeRespFrame),
			    sizeof(session->probeRespFrame), 0);

		/*
		 * Can be efficiently updated whenever new IE added in Probe
		 * response in future
		 */
		if (lim_update_probe_rsp_template_ie_bitmap_beacon1(mac_ctx,
					bcn_1, session) != eSIR_SUCCESS)
			sch_log(mac_ctx, LOGE,
				FL("Failed to build ProbeRsp template"));
	}

	n_status = dot11f_pack_beacon1(mac_ctx, bcn_1, ptr,
				      SCH_MAX_BEACON_SIZE - offset, &n_bytes);
	if (DOT11F_FAILED(n_status)) {
		sch_log(mac_ctx, LOGE,
			FL("Failed to packed a tDot11fBeacon1 (0x%08x.)."),
			n_status);
		qdf_mem_free(bcn_1);
		qdf_mem_free(bcn_2);
		qdf_mem_free(wsc_prb_res);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(n_status)) {
		sch_log(mac_ctx, LOGE,
			FL("Warnings while packing a tDot11fBeacon1(0x%08x.)."),
			n_status);
	}
	/*changed  to correct beacon corruption */
	qdf_mem_set((uint8_t *) bcn_2, sizeof(tDot11fBeacon2), 0);
	session->schBeaconOffsetBegin = offset + (uint16_t) n_bytes;
	sch_log(mac_ctx, LOG1, FL("Initialized beacon begin, offset %d"),
		offset);

	/* Initialize the 'new' fields at the end of the beacon */

	if ((session->limSystemRole == eLIM_AP_ROLE) &&
		session->dfsIncludeChanSwIe == true) {
		populate_dot_11_f_ext_chann_switch_ann(mac_ctx,
				&bcn_2->ext_chan_switch_ann,
				session);
		sch_log(mac_ctx, LOG1,
			FL("ecsa: mode:%d reg:%d chan:%d count:%d"),
			bcn_2->ext_chan_switch_ann.switch_mode,
			bcn_2->ext_chan_switch_ann.new_reg_class,
			bcn_2->ext_chan_switch_ann.new_channel,
			bcn_2->ext_chan_switch_ann.switch_count);
	}

	populate_dot11_supp_operating_classes(mac_ctx,
		&bcn_2->SuppOperatingClasses, session);
	populate_dot11f_country(mac_ctx, &bcn_2->Country, session);
	if (bcn_1->Capabilities.qos)
		populate_dot11f_edca_param_set(mac_ctx, &bcn_2->EDCAParamSet,
					       session);

	if (session->lim11hEnable) {
		populate_dot11f_power_constraints(mac_ctx,
						  &bcn_2->PowerConstraints);
		populate_dot11f_tpc_report(mac_ctx, &bcn_2->TPCReport, session);
		/* Need to insert channel switch announcement here */
		if ((LIM_IS_AP_ROLE(session)
		    || LIM_IS_P2P_DEVICE_GO(session))
			&& session->dfsIncludeChanSwIe == true) {
			/*
			 * Channel switch announcement only if radar is detected
			 * and SAP has instructed to announce channel switch IEs
			 * in beacon and probe responses
			 */
			populate_dot11f_chan_switch_ann(mac_ctx,
						&bcn_2->ChanSwitchAnn, session);
			sch_log(mac_ctx, LOG1,
				FL("csa: mode:%d chan:%d count:%d"),
				bcn_2->ChanSwitchAnn.switchMode,
				bcn_2->ChanSwitchAnn.newChannel,
				bcn_2->ChanSwitchAnn.switchCount);

			/*
			 * TODO: depending the CB mode, extended channel switch
			 * announcement need to be called
			 */
			/*
			populate_dot11f_ext_chan_switch_ann(mac_ctx,
					&bcn_2->ExtChanSwitchAnn, session);
			*/
			/*
			 * TODO: If in 11AC mode, wider bw channel switch
			 * announcement needs to be called
			 */
			/*
			populate_dot11f_wider_bw_chan_switch_ann(mac_ctx,
					&bcn_2->WiderBWChanSwitchAnn, session);
			*/
			/*
			 * Populate the Channel Switch Wrapper Element if
			 * SAP operates in 40/80 Mhz Channel Width.
			 */
			if (true == session->dfsIncludeChanWrapperIe) {
				populate_dot11f_chan_switch_wrapper(mac_ctx,
					&bcn_2->ChannelSwitchWrapper, session);
				sch_log(mac_ctx, LOG1,
				    FL("wrapper: width:%d f0:%d f1:%d"),
				      bcn_2->ChannelSwitchWrapper.
					WiderBWChanSwitchAnn.newChanWidth,
				      bcn_2->ChannelSwitchWrapper.
					WiderBWChanSwitchAnn.newCenterChanFreq0,
				      bcn_2->ChannelSwitchWrapper.
					WiderBWChanSwitchAnn.newCenterChanFreq1
					);
			}
		}
	}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	/* populate proprietary IE for MDM device operating in AP-MCC */
	populate_dot11f_avoid_channel_ie(mac_ctx, &bcn_2->QComVendorIE,
					 session);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	if (session->dot11mode != WNI_CFG_DOT11_MODE_11B)
		populate_dot11f_erp_info(mac_ctx, &bcn_2->ERPInfo, session);

	if (session->htCapability) {
		populate_dot11f_ht_caps(mac_ctx, session, &bcn_2->HTCaps);
		populate_dot11f_ht_info(mac_ctx, &bcn_2->HTInfo, session);
	}
	if (session->vhtCapability) {
		sch_log(mac_ctx, LOGW, FL("Populate VHT IEs in Beacon"));
		populate_dot11f_vht_caps(mac_ctx, session, &bcn_2->VHTCaps);
		populate_dot11f_vht_operation(mac_ctx, session,
					      &bcn_2->VHTOperation);
		is_vht_enabled = true;
		/* following is for MU MIMO: we do not support it yet */
		/*
		populate_dot11f_vht_ext_bss_load( mac_ctx, &bcn2.VHTExtBssLoad);
		*/
		if (session->gLimOperatingMode.present)
			populate_dot11f_operating_mode(mac_ctx,
						&bcn_2->OperatingMode, session);
	}
	if (session->limSystemRole != eLIM_STA_IN_IBSS_ROLE)
		populate_dot11f_ext_cap(mac_ctx, is_vht_enabled, &bcn_2->ExtCap,
					session);
	populate_dot11f_ext_supp_rates(mac_ctx,
				POPULATE_DOT11F_RATES_OPERATIONAL,
				&bcn_2->ExtSuppRates, session);

	if (session->pLimStartBssReq != NULL) {
		populate_dot11f_wpa(mac_ctx, &session->pLimStartBssReq->rsnIE,
				    &bcn_2->WPA);
		populate_dot11f_rsn_opaque(mac_ctx,
					   &session->pLimStartBssReq->rsnIE,
					   &bcn_2->RSNOpaque);
	}

	if (session->limWmeEnabled)
		populate_dot11f_wmm(mac_ctx, &bcn_2->WMMInfoAp,
				&bcn_2->WMMParams, &bcn_2->WMMCaps, session);
	if (LIM_IS_AP_ROLE(session)) {
		if (session->wps_state != SAP_WPS_DISABLED) {
			populate_dot11f_beacon_wpsi_es(mac_ctx,
						&bcn_2->WscBeacon, session);
		}
	} else {
		if (wlan_cfg_get_int(mac_ctx,
			(uint16_t) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
			sch_log(mac_ctx, LOGP, FL("Failed to cfg get id %d"),
				WNI_CFG_WPS_ENABLE);

		wps_ap_enable = tmp & WNI_CFG_WPS_ENABLE_AP;

		if (wps_ap_enable)
			populate_dot11f_wsc(mac_ctx, &bcn_2->WscBeacon);

		if (mac_ctx->lim.wscIeInfo.wscEnrollmentState ==
						eLIM_WSC_ENROLL_BEGIN) {
			populate_dot11f_wsc_registrar_info(mac_ctx,
						&bcn_2->WscBeacon);
			mac_ctx->lim.wscIeInfo.wscEnrollmentState =
						eLIM_WSC_ENROLL_IN_PROGRESS;
		}

		if (mac_ctx->lim.wscIeInfo.wscEnrollmentState ==
						eLIM_WSC_ENROLL_END) {
			de_populate_dot11f_wsc_registrar_info(mac_ctx,
							&bcn_2->WscBeacon);
			mac_ctx->lim.wscIeInfo.wscEnrollmentState =
							eLIM_WSC_ENROLL_NOOP;
		}
	}

	if ((LIM_IS_AP_ROLE(session))) {
		/*
		 * Can be efficiently updated whenever new IE added  in Probe
		 * response in future
		 */
		lim_update_probe_rsp_template_ie_bitmap_beacon2(mac_ctx, bcn_2,
					&session->DefProbeRspIeBitmap[0],
					&session->probeRespFrame);

		/* update probe response WPS IE instead of beacon WPS IE */
		if (session->wps_state != SAP_WPS_DISABLED) {
			if (session->APWPSIEs.SirWPSProbeRspIE.FieldPresent)
				populate_dot11f_probe_res_wpsi_es(mac_ctx,
							wsc_prb_res, session);
			else
				wsc_prb_res->present = 0;
			if (wsc_prb_res->present) {
				set_probe_rsp_ie_bitmap(
					&session->DefProbeRspIeBitmap[0],
					SIR_MAC_WPA_EID);
				qdf_mem_copy((void *)
					&session->probeRespFrame.WscProbeRes,
					(void *)wsc_prb_res,
					sizeof(tDot11fIEWscProbeRes));
			}
		}

	}

	addnie_present = (session->addIeParams.probeRespBCNDataLen != 0);
	if (addnie_present) {
		addn_ielen = session->addIeParams.probeRespBCNDataLen;
		addn_ie = qdf_mem_malloc(addn_ielen);
		if (!addn_ie) {
			sch_log(mac_ctx, LOGE, FL("addn_ie malloc failed"));
			qdf_mem_free(bcn_1);
			qdf_mem_free(bcn_2);
			qdf_mem_free(wsc_prb_res);
			return eSIR_MEM_ALLOC_FAILED;
		}
		qdf_mem_copy(addn_ie,
			session->addIeParams.probeRespBCNData_buff,
			addn_ielen);

		qdf_mem_zero((uint8_t *)&extracted_extcap,
			     sizeof(tDot11fIEExtCap));
		status = lim_strip_extcap_update_struct(mac_ctx, addn_ie,
				&addn_ielen, &extracted_extcap);
		if (eSIR_SUCCESS != status) {
			extcap_present = false;
			sch_log(mac_ctx, LOG1, FL("extcap not extracted"));
		}
		/* merge extcap IE */
		if (extcap_present &&
			session->limSystemRole != eLIM_STA_IN_IBSS_ROLE)
			lim_merge_extcap_struct(&bcn_2->ExtCap,
						&extracted_extcap);

	}

	n_status = dot11f_pack_beacon2(mac_ctx, bcn_2,
				      session->pSchBeaconFrameEnd,
				      SCH_MAX_BEACON_SIZE, &n_bytes);
	if (DOT11F_FAILED(n_status)) {
		sch_log(mac_ctx, LOGE,
			FL("Failed to packed a tDot11fBeacon2 (0x%08x.)."),
			n_status);
		qdf_mem_free(bcn_1);
		qdf_mem_free(bcn_2);
		qdf_mem_free(wsc_prb_res);
		qdf_mem_free(addn_ie);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(n_status)) {
		sch_log(mac_ctx, LOGE,
			FL("Warnings while packing a tDot11fBeacon2(0x%08x.)."),
			n_status);
	}

	extra_ie = session->pSchBeaconFrameEnd + n_bytes;
	extra_ie_offset = n_bytes;

	/* TODO: Append additional IE here. */
	if (addn_ielen > 0)
		sch_append_addn_ie(mac_ctx, session,
			session->pSchBeaconFrameEnd + n_bytes,
			SCH_MAX_BEACON_SIZE, &n_bytes, addn_ie, addn_ielen);

	session->schBeaconOffsetEnd = (uint16_t) n_bytes;
	extra_ie_len = n_bytes - extra_ie_offset;
	/* Get the p2p Ie Offset */
	status = sch_get_p2p_ie_offset(extra_ie, extra_ie_len, &p2p_ie_offset);
	if (eSIR_SUCCESS == status)
		/* Update the P2P Ie Offset */
		mac_ctx->sch.schObject.p2pIeOffset =
			session->schBeaconOffsetBegin + TIM_IE_SIZE +
			extra_ie_offset + p2p_ie_offset;
	else
		mac_ctx->sch.schObject.p2pIeOffset = 0;

	sch_log(mac_ctx, LOG1, FL("Initialized beacon end, offset %d"),
		session->schBeaconOffsetEnd);
	mac_ctx->sch.schObject.fBeaconChanged = 1;
	qdf_mem_free(bcn_1);
	qdf_mem_free(bcn_2);
	qdf_mem_free(wsc_prb_res);
	qdf_mem_free(addn_ie);
	return eSIR_SUCCESS;
}

tSirRetStatus lim_update_probe_rsp_template_ie_bitmap_beacon1(tpAniSirGlobal pMac,
						     tDot11fBeacon1 *beacon1,
							      tpPESession
							      psessionEntry)
{
	uint32_t *DefProbeRspIeBitmap;
	tDot11fProbeResponse *prb_rsp;
	if (!psessionEntry) {
		sch_log(pMac, LOGE, FL("PESession is null!"));
		return eSIR_FAILURE;
	}
	DefProbeRspIeBitmap = &psessionEntry->DefProbeRspIeBitmap[0];
	prb_rsp = &psessionEntry->probeRespFrame;
	prb_rsp->BeaconInterval = beacon1->BeaconInterval;
	qdf_mem_copy((void *)&prb_rsp->Capabilities,
		     (void *)&beacon1->Capabilities,
		     sizeof(beacon1->Capabilities));

	/* SSID */
	if (beacon1->SSID.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_SSID_EID);
		/* populating it, because probe response has to go with SSID even in hidden case */
		populate_dot11f_ssid(pMac, &psessionEntry->ssId, &prb_rsp->SSID);
	}
	/* supported rates */
	if (beacon1->SuppRates.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_RATESET_EID);
		qdf_mem_copy((void *)&prb_rsp->SuppRates,
			     (void *)&beacon1->SuppRates,
			     sizeof(beacon1->SuppRates));

	}
	/* DS Parameter set */
	if (beacon1->DSParams.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_DS_PARAM_SET_EID);
		qdf_mem_copy((void *)&prb_rsp->DSParams,
			     (void *)&beacon1->DSParams,
			     sizeof(beacon1->DSParams));

	}

	/* IBSS params will not be present in the Beacons transmitted by AP */
	return eSIR_SUCCESS;
}

void lim_update_probe_rsp_template_ie_bitmap_beacon2(tpAniSirGlobal pMac,
						     tDot11fBeacon2 *beacon2,
						     uint32_t *DefProbeRspIeBitmap,
						     tDot11fProbeResponse *prb_rsp)
{
	/* IBSS parameter set - will not be present in probe response tx by AP */
	/* country */
	if (beacon2->Country.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_COUNTRY_EID);
		qdf_mem_copy((void *)&prb_rsp->Country,
			     (void *)&beacon2->Country,
			     sizeof(beacon2->Country));

	}
	/* Power constraint */
	if (beacon2->PowerConstraints.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_PWR_CONSTRAINT_EID);
		qdf_mem_copy((void *)&prb_rsp->PowerConstraints,
			     (void *)&beacon2->PowerConstraints,
			     sizeof(beacon2->PowerConstraints));

	}
	/* Channel Switch Annoouncement SIR_MAC_CHNL_SWITCH_ANN_EID */
	if (beacon2->ChanSwitchAnn.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_CHNL_SWITCH_ANN_EID);
		qdf_mem_copy((void *)&prb_rsp->ChanSwitchAnn,
			     (void *)&beacon2->ChanSwitchAnn,
			     sizeof(beacon2->ChanSwitchAnn));

	}

	/* EXT Channel Switch Announcement CHNL_EXTENDED_SWITCH_ANN_EID*/
	if (beacon2->ext_chan_switch_ann.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
			SIR_MAC_CHNL_EXTENDED_SWITCH_ANN_EID);
		qdf_mem_copy((void *)&prb_rsp->ext_chan_switch_ann,
			(void *)&beacon2->ext_chan_switch_ann,
			sizeof(beacon2->ext_chan_switch_ann));
	}

	/* Supported operating class */
	if (beacon2->SuppOperatingClasses.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_OPERATING_CLASS_EID);
		qdf_mem_copy((void *)&prb_rsp->SuppOperatingClasses,
				(void *)&beacon2->SuppOperatingClasses,
				sizeof(beacon2->SuppOperatingClasses));
	}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (beacon2->QComVendorIE.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_QCOM_VENDOR_EID);
		qdf_mem_copy((void *)&prb_rsp->QComVendorIE,
			     (void *)&beacon2->QComVendorIE,
			     sizeof(beacon2->QComVendorIE));
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	/* ERP information */
	if (beacon2->ERPInfo.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_ERP_INFO_EID);
		qdf_mem_copy((void *)&prb_rsp->ERPInfo,
			     (void *)&beacon2->ERPInfo,
			     sizeof(beacon2->ERPInfo));

	}
	/* Extended supported rates */
	if (beacon2->ExtSuppRates.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_EXTENDED_RATE_EID);
		qdf_mem_copy((void *)&prb_rsp->ExtSuppRates,
			     (void *)&beacon2->ExtSuppRates,
			     sizeof(beacon2->ExtSuppRates));

	}

	/* WPA */
	if (beacon2->WPA.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_WPA_EID);
		qdf_mem_copy((void *)&prb_rsp->WPA, (void *)&beacon2->WPA,
			     sizeof(beacon2->WPA));

	}

	/* RSN */
	if (beacon2->RSNOpaque.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_RSN_EID);
		qdf_mem_copy((void *)&prb_rsp->RSNOpaque,
			     (void *)&beacon2->RSNOpaque,
			     sizeof(beacon2->RSNOpaque));
	}

	/* EDCA Parameter set */
	if (beacon2->EDCAParamSet.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_EDCA_PARAM_SET_EID);
		qdf_mem_copy((void *)&prb_rsp->EDCAParamSet,
			     (void *)&beacon2->EDCAParamSet,
			     sizeof(beacon2->EDCAParamSet));

	}
	/* Vendor specific - currently no vendor specific IEs added */
	/* Requested IEs - currently we are not processing this will be added later */
	/* HT capability IE */
	if (beacon2->HTCaps.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_HT_CAPABILITIES_EID);
		qdf_mem_copy((void *)&prb_rsp->HTCaps, (void *)&beacon2->HTCaps,
			     sizeof(beacon2->HTCaps));
	}
	/* HT Info IE */
	if (beacon2->HTInfo.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_HT_INFO_EID);
		qdf_mem_copy((void *)&prb_rsp->HTInfo, (void *)&beacon2->HTInfo,
			     sizeof(beacon2->HTInfo));
	}
	if (beacon2->VHTCaps.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_VHT_CAPABILITIES_EID);
		qdf_mem_copy((void *)&prb_rsp->VHTCaps,
			     (void *)&beacon2->VHTCaps,
			     sizeof(beacon2->VHTCaps));
	}
	if (beacon2->VHTOperation.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_VHT_OPERATION_EID);
		qdf_mem_copy((void *)&prb_rsp->VHTOperation,
			     (void *)&beacon2->VHTOperation,
			     sizeof(beacon2->VHTOperation));
	}
	if (beacon2->VHTExtBssLoad.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap,
					SIR_MAC_VHT_EXT_BSS_LOAD_EID);
		qdf_mem_copy((void *)&prb_rsp->VHTExtBssLoad,
			     (void *)&beacon2->VHTExtBssLoad,
			     sizeof(beacon2->VHTExtBssLoad));
	}
	/* WMM IE */
	if (beacon2->WMMParams.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_WPA_EID);
		qdf_mem_copy((void *)&prb_rsp->WMMParams,
			     (void *)&beacon2->WMMParams,
			     sizeof(beacon2->WMMParams));
	}
	/* WMM capability - most of the case won't be present */
	if (beacon2->WMMCaps.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, SIR_MAC_WPA_EID);
		qdf_mem_copy((void *)&prb_rsp->WMMCaps,
			     (void *)&beacon2->WMMCaps,
			     sizeof(beacon2->WMMCaps));
	}

	/* Extended Capability */
	if (beacon2->ExtCap.present) {
		set_probe_rsp_ie_bitmap(DefProbeRspIeBitmap, DOT11F_EID_EXTCAP);
		qdf_mem_copy((void *)&prb_rsp->ExtCap,
			     (void *)&beacon2->ExtCap,
			     sizeof(beacon2->ExtCap));
	}

}

void set_probe_rsp_ie_bitmap(uint32_t *IeBitmap, uint32_t pos)
{
	uint32_t index, temp;

	index = pos >> 5;
	if (index >= 8) {
		return;
	}
	temp = IeBitmap[index];

	temp |= 1 << (pos & 0x1F);

	IeBitmap[index] = temp;
}

/* -------------------------------------------------------------------- */
/**
 * write_beacon_to_memory
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
 * @param size    Size of the beacon to write to memory
 * @param length Length field of the beacon to write to memory
 * @return None
 */

void write_beacon_to_memory(tpAniSirGlobal pMac, uint16_t size, uint16_t length,
			    tpPESession psessionEntry)
{
	uint16_t i;
	tpAniBeaconStruct pBeacon;

	/* copy end of beacon only if length > 0 */
	if (length > 0) {
		for (i = 0; i < psessionEntry->schBeaconOffsetEnd; i++)
			psessionEntry->pSchBeaconFrameBegin[size++] =
				psessionEntry->pSchBeaconFrameEnd[i];
	}
	/* Update the beacon length */
	pBeacon = (tpAniBeaconStruct) psessionEntry->pSchBeaconFrameBegin;
	/* Do not include the beaconLength indicator itself */
	if (length == 0) {
		pBeacon->beaconLength = 0;
		/* Dont copy entire beacon, Copy length field alone */
		size = 4;
	} else
		pBeacon->beaconLength = (uint32_t) size - sizeof(uint32_t);

	/* write size bytes from pSchBeaconFrameBegin */
	PELOG2(sch_log(pMac, LOG2, FL("Beacon size - %d bytes"), size);)
	PELOG2(sir_dump_buf
		       (pMac, SIR_SCH_MODULE_ID, LOG2,
		       psessionEntry->pSchBeaconFrameBegin, size);
	       )

	if (!pMac->sch.schObject.fBeaconChanged)
		return;

	pMac->sch.gSchGenBeacon = 1;
	if (pMac->sch.gSchGenBeacon) {
		pMac->sch.gSchBeaconsSent++;

		/* */
		/* Copy beacon data to SoftMAC shared memory... */
		/* Do this by sending a message to HAL */
		/* */

		size = (size + 3) & (~3);
		if (eSIR_SUCCESS !=
		    sch_send_beacon_req(pMac, psessionEntry->pSchBeaconFrameBegin,
					size, psessionEntry))
			PELOGE(sch_log
				       (pMac, LOGE,
				       FL
					       ("sch_send_beacon_req() returned an error (zsize %d)"),
				       size);
			       )
			else {
				pMac->sch.gSchBeaconsWritten++;
			}
	}
	pMac->sch.schObject.fBeaconChanged = 0;
}

/**
 * sch_generate_tim
 *
 * FUNCTION:
 * Generate TIM
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac pointer to global mac structure
 * @param **pPtr pointer to the buffer, where the TIM bit is to be written.
 * @param *timLength pointer to limLength, which needs to be returned.
 * @return None
 */
void sch_generate_tim(tpAniSirGlobal pMac, uint8_t **pPtr, uint16_t *timLength,
		      uint8_t dtimPeriod)
{
	uint8_t *ptr = *pPtr;
	uint32_t val = 0;
	uint32_t minAid = 1;    /* Always start with AID 1 as minimum */
	uint32_t maxAid = HAL_NUM_STA;

	/* Generate partial virtual bitmap */
	uint8_t N1 = minAid / 8;
	uint8_t N2 = maxAid / 8;
	if (N1 & 1)
		N1--;

	*timLength = N2 - N1 + 4;
	val = dtimPeriod;

	/*
	 * Write 0xFF to firmware's field to detect firmware's mal-function
	 * early. DTIM count and bitmap control usually cannot be 0xFF, so it
	 * is easy to know that firmware never updated DTIM count/bitmap control
	 * field after host driver downloaded beacon template if end-user complaints
	 * that DTIM count and bitmapControl is 0xFF.
	 */
	*ptr++ = SIR_MAC_TIM_EID;
	*ptr++ = (uint8_t) (*timLength);
	/* location for dtimCount. will be filled in by FW. */
	*ptr++ = 0xFF;
	*ptr++ = (uint8_t) val;
	/* location for bitmap control. will be filled in by FW. */
	*ptr++ = 0xFF;
	ptr += (N2 - N1 + 1);

	PELOG2(sir_dump_buf
		       (pMac, SIR_SCH_MODULE_ID, LOG2, *pPtr, (*timLength) + 2);
	       )
	* pPtr = ptr;
}
/* -------------------------------------------------------------------- */
/**
 * @function: SchProcessPreBeaconInd
 *
 * @brief : Process the PreBeacon Indication from the Lim
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param : pMac - tpAniSirGlobal
 *
 * @return None
 */

void sch_process_pre_beacon_ind(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
	tpBeaconGenParams pMsg = (tpBeaconGenParams) limMsg->bodyptr;
	uint32_t beaconSize;
	tpPESession psessionEntry;
	uint8_t sessionId;

	psessionEntry = pe_find_session_by_bssid(pMac, pMsg->bssId, &sessionId);
	if (psessionEntry == NULL) {
		PELOGE(sch_log(pMac, LOGE, FL("session lookup fails"));)
		goto end;
	}

	beaconSize = psessionEntry->schBeaconOffsetBegin;

	/* If SME is not in normal mode, no need to generate beacon */
	if (psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE) {
		PELOGE(sch_log
			       (pMac, LOG1,
			       FL("PreBeaconInd received in invalid state: %d"),
			       psessionEntry->limSmeState);
		       )
		goto end;
	}

	switch (GET_LIM_SYSTEM_ROLE(psessionEntry)) {

	case eLIM_STA_IN_IBSS_ROLE:
		/* generate IBSS parameter set */
		if (psessionEntry->statypeForBss == STA_ENTRY_SELF)
			write_beacon_to_memory(pMac, (uint16_t) beaconSize,
					       (uint16_t) beaconSize,
					       psessionEntry);
		else
			PELOGE(sch_log
				       (pMac, LOGE,
				       FL
					       ("can not send beacon for PEER session entry"));
			       )
			break;

	case eLIM_AP_ROLE: {
		uint8_t *ptr =
			&psessionEntry->pSchBeaconFrameBegin[psessionEntry->
							     schBeaconOffsetBegin];
		uint16_t timLength = 0;
		if (psessionEntry->statypeForBss == STA_ENTRY_SELF) {
			sch_generate_tim(pMac, &ptr, &timLength,
					 psessionEntry->dtimPeriod);
			beaconSize += 2 + timLength;
			write_beacon_to_memory(pMac, (uint16_t) beaconSize,
					       (uint16_t) beaconSize,
					       psessionEntry);
		} else
			PELOGE(sch_log
				       (pMac, LOGE,
				       FL
					       ("can not send beacon for PEER session entry"));
			       )
			}
			break;

	default:
		PELOGE(sch_log
			       (pMac, LOGE,
			       FL
				       ("Error-PE has Receive PreBeconGenIndication when System is in %d role"),
			       GET_LIM_SYSTEM_ROLE(psessionEntry));
		       )
	}

end:
		qdf_mem_free(pMsg);

	}
