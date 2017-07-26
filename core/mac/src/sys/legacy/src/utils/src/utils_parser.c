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

/*
 *
 * This file utils_parser.cc contains the code for parsing
 * 802.11 messages.
 * Author:        Pierre Vandwalle
 * Date:          03/18/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "ani_global.h"
#include "utils_parser.h"
#include "lim_ser_des_utils.h"

void convert_ssid(tpAniSirGlobal pMac, tSirMacSSid *pOld, tDot11fIESSID *pNew)
{
	pOld->length = pNew->num_ssid;
	qdf_mem_copy(pOld->ssId, pNew->ssid, pNew->num_ssid);
}

void convert_supp_rates(tpAniSirGlobal pMac,
			tSirMacRateSet *pOld, tDot11fIESuppRates *pNew)
{
	pOld->numRates = pNew->num_rates;
	qdf_mem_copy(pOld->rate, pNew->rates, pNew->num_rates);
}

void convert_ext_supp_rates(tpAniSirGlobal pMac,
			    tSirMacRateSet *pOld, tDot11fIEExtSuppRates *pNew)
{
	pOld->numRates = pNew->num_rates;
	qdf_mem_copy(pOld->rate, pNew->rates, pNew->num_rates);
}

void convert_qos_caps(tpAniSirGlobal pMac,
		      tSirMacQosCapabilityIE *pOld, tDot11fIEQOSCapsAp *pNew)
{
	pOld->type = 46;
	pOld->length = 1;

	pOld->qosInfo.count = pNew->count;
}

void convert_qos_caps_station(tpAniSirGlobal pMac,
			      tSirMacQosCapabilityStaIE *pOld,
			      tDot11fIEQOSCapsStation *pNew)
{
	pOld->type = 46;
	pOld->length = 1;

	pOld->qosInfo.moreDataAck = pNew->more_data_ack;
	pOld->qosInfo.maxSpLen = pNew->max_sp_length;
	pOld->qosInfo.qack = pNew->qack;
	pOld->qosInfo.acbe_uapsd = pNew->acbe_uapsd;
	pOld->qosInfo.acbk_uapsd = pNew->acbk_uapsd;
	pOld->qosInfo.acvi_uapsd = pNew->acvi_uapsd;
	pOld->qosInfo.acvo_uapsd = pNew->acvo_uapsd;
}

tSirRetStatus convert_wpa(tpAniSirGlobal pMac,
			  tSirMacWpaInfo *pOld, tDot11fIEWPA *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into an */
	/* array... */
	uint8_t buffer[257];
	uint32_t status, written = 0, nbuffer = 257;
	status = dot11f_pack_ie_wpa(pMac, pNew, buffer, nbuffer, &written);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to re-pack the WPA IE (0x%0x8)", status);
		return eSIR_FAILURE;
	}

	pOld->length = (uint8_t) written - 2;
	qdf_mem_copy(pOld->info, buffer + 2, pOld->length);

	return eSIR_SUCCESS;
}

tSirRetStatus convert_wpa_opaque(tpAniSirGlobal pMac,
				 tSirMacWpaInfo *pOld, tDot11fIEWPAOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array.  Note that we need to explicitly add the OUI! */
	pOld->length = pNew->num_data + 4;
	pOld->info[0] = 0x00;
	pOld->info[1] = 0x50;
	pOld->info[2] = 0xf2;
	pOld->info[3] = 0x01;
	qdf_mem_copy(pOld->info + 4, pNew->data, pNew->num_data);

	return eSIR_SUCCESS;
}

#ifdef FEATURE_WLAN_WAPI
tSirRetStatus convert_wapi_opaque(tpAniSirGlobal pMac,
				  tSirMacWapiInfo *pOld,
				  tDot11fIEWAPIOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array.  Note that we need to explicitly add the OUI! */
	pOld->length = pNew->num_data;
	qdf_mem_copy(pOld->info, pNew->data, pNew->num_data);

	return eSIR_SUCCESS;
}
#endif

tSirRetStatus convert_wsc_opaque(tpAniSirGlobal pMac,
				 tSirAddie *pOld, tDot11fIEWscIEOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array.  Note that we need to explicitly add the vendorIE and OUI ! */
	uint8_t curAddIELen = pOld->length;

	pOld->length = curAddIELen + pNew->num_data + 6;
	pOld->addIEdata[curAddIELen++] = 0xdd;
	pOld->addIEdata[curAddIELen++] = pNew->num_data + 4;
	pOld->addIEdata[curAddIELen++] = 0x00;
	pOld->addIEdata[curAddIELen++] = 0x50;
	pOld->addIEdata[curAddIELen++] = 0xf2;
	pOld->addIEdata[curAddIELen++] = 0x04;
	qdf_mem_copy(pOld->addIEdata + curAddIELen, pNew->data, pNew->num_data);

	return eSIR_SUCCESS;
}

tSirRetStatus convert_p2p_opaque(tpAniSirGlobal pMac,
				  tSirAddie *pOld, tDot11fIEP2PIEOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array.  Note that we need to explicitly add the vendorIE and OUI ! */
	uint8_t curAddIELen = pOld->length;

	pOld->length = curAddIELen + pNew->num_data + 6;
	pOld->addIEdata[curAddIELen++] = 0xdd;
	pOld->addIEdata[curAddIELen++] = pNew->num_data + 4;
	pOld->addIEdata[curAddIELen++] = 0x50;
	pOld->addIEdata[curAddIELen++] = 0x6f;
	pOld->addIEdata[curAddIELen++] = 0x9A;
	pOld->addIEdata[curAddIELen++] = 0x09;
	qdf_mem_copy(pOld->addIEdata + curAddIELen, pNew->data, pNew->num_data);

	return eSIR_SUCCESS;
}

#ifdef WLAN_FEATURE_WFD
tSirRetStatus convert_wfd_opaque(tpAniSirGlobal pMac,
				 tSirAddie *pOld, tDot11fIEWFDIEOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array.  Note that we need to explicitly add the vendorIE and OUI ! */
	uint8_t curAddIELen = pOld->length;

	pOld->length = curAddIELen + pNew->num_data + 6;
	pOld->addIEdata[curAddIELen++] = 0xdd;
	pOld->addIEdata[curAddIELen++] = pNew->num_data + 4;
	pOld->addIEdata[curAddIELen++] = 0x50;
	pOld->addIEdata[curAddIELen++] = 0x6f;
	pOld->addIEdata[curAddIELen++] = 0x9A;
	pOld->addIEdata[curAddIELen++] = 0x0a;
	qdf_mem_copy(pOld->addIEdata + curAddIELen, pNew->data, pNew->num_data);

	return eSIR_SUCCESS;
}
#endif

tSirRetStatus convert_rsn(tpAniSirGlobal pMac,
			  tSirMacRsnInfo *pOld, tDot11fIERSN *pNew)
{
	uint8_t buffer[257];
	uint32_t status, written = 0, nbuffer = 257;
	status = dot11f_pack_ie_rsn(pMac, pNew, buffer, nbuffer, &written);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to re-pack the RSN IE (0x%0x8)", status);
		return eSIR_FAILURE;
	}

	pOld->length = (uint8_t) written - 2;
	qdf_mem_copy(pOld->info, buffer + 2, pOld->length);

	return eSIR_SUCCESS;
}

tSirRetStatus convert_rsn_opaque(tpAniSirGlobal pMac,
				 tSirMacRsnInfo *pOld, tDot11fIERSNOpaque *pNew)
{
	/* This is awful, I know, but the old code just rammed the IE into */
	/* an opaque array. */
	pOld->length = pNew->num_data;
	qdf_mem_copy(pOld->info, pNew->data, pOld->length);

	return eSIR_SUCCESS;
}

void convert_power_caps(tpAniSirGlobal pMac,
			tSirMacPowerCapabilityIE *pOld,
			tDot11fIEPowerCaps *pNew)
{
	pOld->type = 33;
	pOld->length = 2;
	pOld->minTxPower = pNew->minTxPower;
	pOld->maxTxPower = pNew->maxTxPower;
}

void convert_supp_channels(tpAniSirGlobal pMac,
			   tSirMacSupportedChannelIE *pOld,
			   tDot11fIESuppChannels *pNew)
{
	pOld->type = 36;
	pOld->length = (pNew->num_bands * 2);
	qdf_mem_copy((uint8_t *) pOld->supportedChannels,
		     (uint8_t *) pNew->bands, pOld->length);
}

void convert_cf_params(tpAniSirGlobal pMac,
		       tSirMacCfParamSet *pOld, tDot11fIECFParams *pNew)
{
	pOld->cfpCount = pNew->cfp_count;
	pOld->cfpPeriod = pNew->cfp_period;
	pOld->cfpMaxDuration = pNew->cfp_maxduration;
	pOld->cfpDurRemaining = pNew->cfp_durremaining;
}

void convert_fh_params(tpAniSirGlobal pMac,
		       tSirMacFHParamSet *pOld, tDot11fIEFHParamSet *pNew)
{
	pOld->dwellTime = pNew->dwell_time;
	pOld->hopSet = pNew->hop_set;
	pOld->hopPattern = pNew->hop_pattern;
	pOld->hopIndex = pNew->hop_index;
}

void convert_tim(tpAniSirGlobal pMac, tSirMacTim *pOld, tDot11fIETIM *pNew)
{
	pOld->dtimCount = pNew->dtim_count;
	pOld->dtimPeriod = pNew->dtim_period;
	pOld->bitmapControl = pNew->bmpctl;
	pOld->bitmapLength = pNew->num_vbmp;

	qdf_mem_copy(pOld->bitmap, pNew->vbmp, pNew->num_vbmp);
}

void convert_country(tpAniSirGlobal pMac,
		     tSirCountryInformation *pOld, tDot11fIECountry *pNew)
{
	int i;

	qdf_mem_copy(pOld->countryString, pNew->country, COUNTRY_STRING_LENGTH);

	pOld->numIntervals = pNew->num_triplets;

	for (i = 0; i < pNew->num_triplets; ++i) {
		pOld->channelTransmitPower[i].channelNumber =
			pNew->triplets[i][0];
		pOld->channelTransmitPower[i].numChannel = pNew->triplets[i][1];
		pOld->channelTransmitPower[i].maxTransmitPower =
			pNew->triplets[i][2];
	}
}

void convert_wmm_params(tpAniSirGlobal pMac,
			tSirMacEdcaParamSetIE *pOld, tDot11fIEWMMParams *pNew)
{
	pOld->type = 221;
	pOld->length = 24;

	qdf_mem_copy((uint8_t *) &pOld->qosInfo, (uint8_t *) &pNew->qosInfo,
		     1);

	pOld->acbe.aci.aifsn = pNew->acbe_aifsn;
	pOld->acbe.aci.acm = pNew->acbe_acm;
	pOld->acbe.aci.aci = pNew->acbe_aci;
	pOld->acbe.cw.min = pNew->acbe_acwmin;
	pOld->acbe.cw.max = pNew->acbe_acwmax;
	pOld->acbe.txoplimit = pNew->acbe_txoplimit;

	pOld->acbk.aci.aifsn = pNew->acbk_aifsn;
	pOld->acbk.aci.acm = pNew->acbk_acm;
	pOld->acbk.aci.aci = pNew->acbk_aci;
	pOld->acbk.cw.min = pNew->acbk_acwmin;
	pOld->acbk.cw.max = pNew->acbk_acwmax;
	pOld->acbk.txoplimit = pNew->acbk_txoplimit;

	pOld->acvi.aci.aifsn = pNew->acvi_aifsn;
	pOld->acvi.aci.acm = pNew->acvi_acm;
	pOld->acvi.aci.aci = pNew->acvi_aci;
	pOld->acvi.cw.min = pNew->acvi_acwmin;
	pOld->acvi.cw.max = pNew->acvi_acwmax;
	pOld->acvi.txoplimit = pNew->acvi_txoplimit;

	pOld->acvo.aci.aifsn = pNew->acvo_aifsn;
	pOld->acvo.aci.acm = pNew->acvo_acm;
	pOld->acvo.aci.aci = pNew->acvo_aci;
	pOld->acvo.cw.min = pNew->acvo_acwmin;
	pOld->acvo.cw.max = pNew->acvo_acwmax;
	pOld->acvo.txoplimit = pNew->acvo_txoplimit;
}

void convert_erp_info(tpAniSirGlobal pMac,
		      tSirMacErpInfo *pOld, tDot11fIEERPInfo *pNew)
{
	pOld->nonErpPresent = pNew->non_erp_present;
	pOld->useProtection = pNew->use_prot;
	pOld->barkerPreambleMode = pNew->barker_preamble;
}

void convert_edca_param(tpAniSirGlobal pMac,
			tSirMacEdcaParamSetIE *pOld,
			tDot11fIEEDCAParamSet *pNew)
{
	pOld->type = 12;
	pOld->length = 20;

	qdf_mem_copy((uint8_t *) &pOld->qosInfo, (uint8_t *) &pNew->qos, 1);

	pOld->acbe.aci.aifsn = pNew->acbe_aifsn;
	pOld->acbe.aci.acm = pNew->acbe_acm;
	pOld->acbe.aci.aci = pNew->acbe_aci;
	pOld->acbe.cw.min = pNew->acbe_acwmin;
	pOld->acbe.cw.max = pNew->acbe_acwmax;
	pOld->acbe.txoplimit = pNew->acbe_txoplimit;

	pOld->acbk.aci.aifsn = pNew->acbk_aifsn;
	pOld->acbk.aci.acm = pNew->acbk_acm;
	pOld->acbk.aci.aci = pNew->acbk_aci;
	pOld->acbk.cw.min = pNew->acbk_acwmin;
	pOld->acbk.cw.max = pNew->acbk_acwmax;
	pOld->acbk.txoplimit = pNew->acbk_txoplimit;

	pOld->acvi.aci.aifsn = pNew->acvi_aifsn;
	pOld->acvi.aci.acm = pNew->acvi_acm;
	pOld->acvi.aci.aci = pNew->acvi_aci;
	pOld->acvi.cw.min = pNew->acvi_acwmin;
	pOld->acvi.cw.max = pNew->acvi_acwmax;
	pOld->acvi.txoplimit = pNew->acvi_txoplimit;

	pOld->acvo.aci.aifsn = pNew->acvo_aifsn;
	pOld->acvo.aci.acm = pNew->acvo_acm;
	pOld->acvo.aci.aci = pNew->acvo_aci;
	pOld->acvo.cw.min = pNew->acvo_acwmin;
	pOld->acvo.cw.max = pNew->acvo_acwmax;
	pOld->acvo.txoplimit = pNew->acvo_txoplimit;

}

void convert_tspec(tpAniSirGlobal pMac,
		   tSirMacTspecIE *pOld, tDot11fIETSPEC *pNew)
{
	pOld->tsinfo.traffic.trafficType = (uint16_t) pNew->traffic_type;
	pOld->tsinfo.traffic.tsid = (uint16_t) pNew->tsid;
	pOld->tsinfo.traffic.direction = (uint16_t) pNew->direction;
	pOld->tsinfo.traffic.accessPolicy = (uint16_t) pNew->access_policy;
	pOld->tsinfo.traffic.aggregation = (uint16_t) pNew->aggregation;
	pOld->tsinfo.traffic.psb = (uint16_t) pNew->psb;
	pOld->tsinfo.traffic.userPrio = (uint16_t) pNew->user_priority;
	pOld->tsinfo.traffic.ackPolicy = (uint16_t) pNew->tsinfo_ack_pol;

	pOld->tsinfo.schedule.schedule = (uint8_t) pNew->schedule;

	pOld->nomMsduSz = pNew->size;
	pOld->maxMsduSz = pNew->max_msdu_size;
	pOld->minSvcInterval = pNew->min_service_int;
	pOld->maxSvcInterval = pNew->max_service_int;
	pOld->inactInterval = pNew->inactivity_int;
	pOld->suspendInterval = pNew->suspension_int;
	pOld->svcStartTime = pNew->service_start_time;
	pOld->minDataRate = pNew->min_data_rate;
	pOld->meanDataRate = pNew->mean_data_rate;
	pOld->peakDataRate = pNew->peak_data_rate;
	pOld->maxBurstSz = pNew->burst_size;
	pOld->delayBound = pNew->delay_bound;
	pOld->minPhyRate = pNew->min_phy_rate;
	pOld->surplusBw = pNew->surplus_bw_allowance;
	pOld->mediumTime = pNew->medium_time;
}

tSirRetStatus convert_tclas(tpAniSirGlobal pMac,
			    tSirTclasInfo *pOld, tDot11fIETCLAS *pNew)
{
	uint32_t length = 0;

	if (DOT11F_FAILED(dot11f_get_packed_ietclas(pMac, pNew, &length))) {
		return eSIR_FAILURE;
	}

	pOld->tclas.type = DOT11F_EID_TCLAS;
	pOld->tclas.length = (uint8_t) length;
	pOld->tclas.userPrio = pNew->user_priority;
	pOld->tclas.classifierType = pNew->classifier_type;
	pOld->tclas.classifierMask = pNew->classifier_mask;

	switch (pNew->classifier_type) {
	case 0:
		qdf_mem_copy(pOld->tclasParams.eth.srcAddr,
			     pNew->info.EthParams.source, 6);
		qdf_mem_copy(pOld->tclasParams.eth.dstAddr,
			     pNew->info.EthParams.dest, 6);
		pOld->tclasParams.eth.type = pNew->info.EthParams.type;
		break;
	case 1:
		pOld->version = pNew->info.IpParams.version;
		if (4 == pNew->info.IpParams.version) {
			pOld->tclasParams.ipv4.version = 4;
			qdf_mem_copy(pOld->tclasParams.ipv4.srcIpAddr,
				     pNew->info.IpParams.params.IpV4Params.
				     source, 4);
			qdf_mem_copy(pOld->tclasParams.ipv4.dstIpAddr,
				     pNew->info.IpParams.params.IpV4Params.dest,
				     4);
			pOld->tclasParams.ipv4.srcPort =
				pNew->info.IpParams.params.IpV4Params.src_port;
			pOld->tclasParams.ipv4.dstPort =
				pNew->info.IpParams.params.IpV4Params.dest_port;
			pOld->tclasParams.ipv4.dscp =
				pNew->info.IpParams.params.IpV4Params.DSCP;
			pOld->tclasParams.ipv4.protocol =
				pNew->info.IpParams.params.IpV4Params.proto;
			pOld->tclasParams.ipv4.rsvd =
				pNew->info.IpParams.params.IpV4Params.reserved;
		} else if (6 == pNew->info.IpParams.version) {
			pOld->tclasParams.ipv6.version = 6;
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     srcIpAddr,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.source, 16);
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     dstIpAddr,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.dest, 16);
			pOld->tclasParams.ipv6.srcPort =
				pNew->info.IpParams.params.IpV6Params.src_port;
			pOld->tclasParams.ipv6.dstPort =
				pNew->info.IpParams.params.IpV6Params.dest_port;
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     flowLabel,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.flow_label, 3);
		} else {
			return eSIR_FAILURE;
		}
		break;
	case 2:
		pOld->tclasParams.t8021dq.tag =
			pNew->info.Params8021dq.tag_type;
		break;
	default:
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

void convert_wmmtspec(tpAniSirGlobal pMac,
		      tSirMacTspecIE *pOld, tDot11fIEWMMTSPEC *pNew)
{
	pOld->tsinfo.traffic.trafficType = (uint16_t) pNew->traffic_type;
	pOld->tsinfo.traffic.tsid = (uint16_t) pNew->tsid;
	pOld->tsinfo.traffic.direction = (uint16_t) pNew->direction;
	pOld->tsinfo.traffic.accessPolicy = (uint16_t) pNew->access_policy;
	pOld->tsinfo.traffic.aggregation = (uint16_t) pNew->aggregation;
	pOld->tsinfo.traffic.psb = (uint16_t) pNew->psb;
	pOld->tsinfo.traffic.userPrio = (uint16_t) pNew->user_priority;
	pOld->tsinfo.traffic.ackPolicy = (uint16_t) pNew->tsinfo_ack_pol;
	pOld->nomMsduSz = (pNew->fixed << 15) | pNew->size;
	pOld->maxMsduSz = pNew->max_msdu_size;
	pOld->minSvcInterval = pNew->min_service_int;
	pOld->maxSvcInterval = pNew->max_service_int;
	pOld->inactInterval = pNew->inactivity_int;
	pOld->suspendInterval = pNew->suspension_int;
	pOld->svcStartTime = pNew->service_start_time;
	pOld->minDataRate = pNew->min_data_rate;
	pOld->meanDataRate = pNew->mean_data_rate;
	pOld->peakDataRate = pNew->peak_data_rate;
	pOld->maxBurstSz = pNew->burst_size;
	pOld->delayBound = pNew->delay_bound;
	pOld->minPhyRate = pNew->min_phy_rate;
	pOld->surplusBw = pNew->surplus_bw_allowance;
	pOld->mediumTime = pNew->medium_time;
}

tSirRetStatus convert_wmmtclas(tpAniSirGlobal pMac,
			       tSirTclasInfo *pOld, tDot11fIEWMMTCLAS *pNew)
{
	uint32_t length = 0;

	if (DOT11F_FAILED(dot11f_get_packed_iewmmtclas(pMac, pNew, &length))) {
		return eSIR_FAILURE;
	}

	pOld->tclas.type = DOT11F_EID_WMMTCLAS;
	pOld->tclas.length = (uint8_t) length;
	pOld->tclas.userPrio = pNew->user_priority;
	pOld->tclas.classifierType = pNew->classifier_type;
	pOld->tclas.classifierMask = pNew->classifier_mask;

	switch (pNew->classifier_type) {
	case 0:
		qdf_mem_copy(pOld->tclasParams.eth.srcAddr,
			     pNew->info.EthParams.source, 6);
		qdf_mem_copy(pOld->tclasParams.eth.dstAddr,
			     pNew->info.EthParams.dest, 6);
		pOld->tclasParams.eth.type = pNew->info.EthParams.type;
		break;
	case 1:
		pOld->version = pNew->info.IpParams.version;
		if (4 == pNew->info.IpParams.version) {
			pOld->tclasParams.ipv4.version = 4;
			qdf_mem_copy(pOld->tclasParams.ipv4.srcIpAddr,
				     pNew->info.IpParams.params.IpV4Params.
				     source, 4);
			qdf_mem_copy(pOld->tclasParams.ipv4.dstIpAddr,
				     pNew->info.IpParams.params.IpV4Params.dest,
				     4);
			pOld->tclasParams.ipv4.srcPort =
				pNew->info.IpParams.params.IpV4Params.src_port;
			pOld->tclasParams.ipv4.dstPort =
				pNew->info.IpParams.params.IpV4Params.dest_port;
			pOld->tclasParams.ipv4.dscp =
				pNew->info.IpParams.params.IpV4Params.DSCP;
			pOld->tclasParams.ipv4.protocol =
				pNew->info.IpParams.params.IpV4Params.proto;
			pOld->tclasParams.ipv4.rsvd =
				pNew->info.IpParams.params.IpV4Params.reserved;
		} else if (6 == pNew->info.IpParams.version) {
			pOld->tclasParams.ipv6.version = 6;
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     srcIpAddr,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.source, 16);
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     dstIpAddr,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.dest, 16);
			pOld->tclasParams.ipv6.srcPort =
				pNew->info.IpParams.params.IpV6Params.src_port;
			pOld->tclasParams.ipv6.dstPort =
				pNew->info.IpParams.params.IpV6Params.dest_port;
			qdf_mem_copy((uint8_t *) pOld->tclasParams.ipv6.
				     flowLabel,
				     (uint8_t *) pNew->info.IpParams.params.
				     IpV6Params.flow_label, 3);
		} else {
			return eSIR_FAILURE;
		}
		break;
	case 2:
		pOld->tclasParams.t8021dq.tag =
			pNew->info.Params8021dq.tag_type;
		break;
	default:
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

void convert_ts_delay(tpAniSirGlobal pMac,
		      tSirMacTsDelayIE *pOld, tDot11fIETSDelay *pNew)
{
	pOld->type = DOT11F_EID_TSDELAY;
	pOld->length = 4U;
	pOld->delay = pNew->delay;
}

void convert_schedule(tpAniSirGlobal pMac,
		      tSirMacScheduleIE *pOld, tDot11fIESchedule *pNew)
{
	pOld->type = DOT11F_EID_SCHEDULE;
	pOld->length = DOT11F_IE_SCHEDULE_MIN_LEN;

	pOld->info.aggregation = pNew->aggregation;
	pOld->info.tsid = pNew->tsid;
	pOld->info.direction = pNew->direction;

	pOld->svcStartTime = pNew->service_start_time;
	pOld->svcInterval = pNew->service_interval;
	pOld->specInterval = pNew->spec_interval;
}

void convert_wmm_schedule(tpAniSirGlobal pMac,
			  tSirMacScheduleIE *pOld, tDot11fIEWMMSchedule *pNew)
{
	pOld->type = DOT11F_EID_WMMSCHEDULE;
	pOld->length = DOT11F_IE_WMMSCHEDULE_MIN_LEN;

	pOld->info.aggregation = pNew->aggregation;
	pOld->info.tsid = pNew->tsid;
	pOld->info.direction = pNew->direction;

	pOld->svcStartTime = pNew->service_start_time;
	pOld->svcInterval = pNew->service_interval;
	pOld->specInterval = pNew->spec_interval;
}

/**
    @brief   :    This functions converts the given buffer till given size to
    Big endian format assuming the bus is 32 bit. The size should
    be four byte aligned.
    @param :    ptr to be converted, size
    @return  :    void
 */

void convertto_big_endian(void *ptr, uint16_t size)
{
	uint8_t *temp_ptr;
	uint32_t *dest_ptr;

	dest_ptr = (uint32_t *) ptr;
	while (size) {
		temp_ptr = (uint8_t *) dest_ptr;
		*dest_ptr =
			(temp_ptr[0] << 24) | (temp_ptr[1] << 16) | (temp_ptr[2] <<
								     8) |
			temp_ptr[3];
		dest_ptr++;
		size -= 4;
	}
}

void create_scan_data_null_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
				 uint8_t pwrMgmt, tSirMacAddr bssid,
				 tSirMacAddr selfMacAddr)
{

	macMgmtHdr->fc.type = SIR_MAC_DATA_FRAME;
	macMgmtHdr->fc.subType = SIR_MAC_DATA_NULL;
	macMgmtHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
	macMgmtHdr->fc.order = 0;
	macMgmtHdr->fc.wep = 0;
	macMgmtHdr->fc.moreData = 0;
	macMgmtHdr->fc.powerMgmt = pwrMgmt;
	macMgmtHdr->fc.retry = 0;
	macMgmtHdr->fc.moreFrag = 0;
	macMgmtHdr->fc.fromDS = 0;
	macMgmtHdr->fc.toDS = 0;
	macMgmtHdr->durationLo =
		(uint8_t) (SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff);
	macMgmtHdr->durationHi =
		(uint8_t) ((SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff00) >> 8);
	macMgmtHdr->seqControl.fragNum = 0;
	macMgmtHdr->seqControl.seqNumLo = 0;
	macMgmtHdr->seqControl.seqNumHi = 2;
	qdf_mem_copy((void *)&macMgmtHdr->da,
		     (void *)bssid, sizeof(tSirMacAddr));
	qdf_mem_copy((void *)&macMgmtHdr->sa,
		     (void *)selfMacAddr, sizeof(tSirMacAddr));
	qdf_mem_copy((void *)&macMgmtHdr->bssId,
		     (void *)bssid, sizeof(tSirMacAddr));

	return;
}

void create_scan_cts_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
			   tSirMacAddr selfMac)
{
	macMgmtHdr->fc.type = SIR_MAC_CTRL_FRAME;
	macMgmtHdr->fc.subType = SIR_MAC_CTRL_CTS;
	macMgmtHdr->fc.order = 0;
	macMgmtHdr->fc.wep = 0;
	macMgmtHdr->fc.moreData = 0;
	macMgmtHdr->fc.powerMgmt = 0;
	macMgmtHdr->fc.retry = 0;
	macMgmtHdr->fc.moreFrag = 0;
	macMgmtHdr->fc.fromDS = 0;
	macMgmtHdr->fc.toDS = 0;
	macMgmtHdr->durationLo =
		(uint8_t) (SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff);
	macMgmtHdr->durationHi =
		(uint8_t) ((SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff00) >> 8);
	qdf_mem_copy((void *)macMgmtHdr->da, (void *)selfMac,
		     sizeof(tSirMacAddr));

	return;
}

void convert_qos_mapset_frame(tpAniSirGlobal pMac, tSirQosMapSet *Qos,
			      tDot11fIEQosMapSet *dot11fIE)
{
	uint8_t i, j = 0;
	uint8_t qos_dscp_sz, dot11_dscp_sz;

	qos_dscp_sz = (sizeof(Qos->dscp_exceptions)/2);
	dot11_dscp_sz = sizeof(dot11fIE->dscp_exceptions);
	if (dot11fIE->num_dscp_exceptions > QOS_MAP_LEN_MAX)
		dot11fIE->num_dscp_exceptions = QOS_MAP_LEN_MAX;
	if (dot11fIE->num_dscp_exceptions < QOS_MAP_LEN_MIN)
		return;
	Qos->num_dscp_exceptions =
		(dot11fIE->num_dscp_exceptions - QOS_MAP_LEN_MIN) / 2;

	for (i = 0;
			i < Qos->num_dscp_exceptions &&
			i < qos_dscp_sz && j < dot11_dscp_sz;
			i++) {
		Qos->dscp_exceptions[i][0] = dot11fIE->dscp_exceptions[j];
		j++;
		Qos->dscp_exceptions[i][1] = dot11fIE->dscp_exceptions[j];
		j++;
	}
	for (i = 0; i < 8 && j < dot11_dscp_sz; i++) {
		Qos->dscp_range[i][0] = dot11fIE->dscp_exceptions[j];
		j++;
		Qos->dscp_range[i][1] = dot11fIE->dscp_exceptions[j];
		j++;
	}
}

/**
    @brief    :    This functions creates a DATA_NULL/CTS2SELF frame in Big endian format
    @param    :    Global MAC structure, pointer to return the created packet, role which is Station/AP
    @return    :    void
 */

void create_init_scan_raw_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
				tBssSystemRole role)
{
#if 0
	tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

	if (role == eSYSTEM_STA_ROLE) {
		macMgmtHdr->fc.type = SIR_MAC_DATA_FRAME;
		macMgmtHdr->fc.subType = SIR_MAC_DATA_NULL;
		macMgmtHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
		macMgmtHdr->fc.order = 0;
		macMgmtHdr->fc.wep = 0;
		macMgmtHdr->fc.moreData = 0;
		macMgmtHdr->fc.powerMgmt = 1;   /* Needed for station */
		macMgmtHdr->fc.retry = 0;
		macMgmtHdr->fc.moreFrag = 0;
		macMgmtHdr->fc.fromDS = 0;
		macMgmtHdr->fc.toDS = 1;
		macMgmtHdr->durationLo =
			(uint8_t) (SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff);
		macMgmtHdr->durationHi =
			(uint8_t) ((SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff00) >>
				   8);
		macMgmtHdr->seqControl.fragNum = 0;
		macMgmtHdr->seqControl.seqNumLo = 0;
		macMgmtHdr->seqControl.seqNumHi = 2;
		qdf_mem_copy((void *)&macMgmtHdr->da, (void *)pSta[0].bssId, 6);
		qdf_mem_copy(&macMgmtHdr->sa, pSta[0].staAddr, 6);
		qdf_mem_copy((void *)&macMgmtHdr->bssId, (void *)pSta[0].bssId,
			     6);
	} else if (role == eSYSTEM_AP_ROLE || role == eSYSTEM_STA_IN_IBSS_ROLE) {
		macMgmtHdr->fc.type = SIR_MAC_CTRL_FRAME;
		macMgmtHdr->fc.subType = SIR_MAC_CTRL_CTS;
		macMgmtHdr->fc.order = 0;
		macMgmtHdr->fc.wep = 0;
		macMgmtHdr->fc.moreData = 0;
		macMgmtHdr->fc.powerMgmt = 0;   /* Needed for station */
		macMgmtHdr->fc.retry = 0;
		macMgmtHdr->fc.moreFrag = 0;
		macMgmtHdr->fc.fromDS = 0;
		macMgmtHdr->fc.toDS = 0;
		macMgmtHdr->durationLo =
			(uint8_t) (SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff);
		macMgmtHdr->durationHi =
			(uint8_t) ((SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff00) >>
				   8);
		qdf_mem_copy((void *)macMgmtHdr->da, (void *)pSta[0].staAddr,
			     6);
	}
	return;
#endif
}

/**
    @brief    :    This functions creates a DATA_NULL frame in Big endian format
    @param    :    Global MAC structure, pointer to return the created packet, role which is Station/AP
    @return    :    void
 */

void create_finish_scan_raw_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
				  tBssSystemRole role)
{
#if 0
	tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

	if (role == eSYSTEM_STA_ROLE) {
		macMgmtHdr->fc.type = SIR_MAC_DATA_FRAME;
		macMgmtHdr->fc.subType = SIR_MAC_DATA_NULL;
		macMgmtHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
		macMgmtHdr->fc.order = 0;
		macMgmtHdr->fc.wep = 0;
		macMgmtHdr->fc.moreData = 0;
		macMgmtHdr->fc.powerMgmt = 0;   /* Needed for station */
		macMgmtHdr->fc.retry = 0;
		macMgmtHdr->fc.moreFrag = 0;
		macMgmtHdr->fc.fromDS = 0;
		macMgmtHdr->fc.toDS = 1;
		macMgmtHdr->durationLo =
			(uint8_t) (SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff);
		macMgmtHdr->durationHi =
			(uint8_t) ((SIR_MAC_MAX_DURATION_MICRO_SECONDS & 0xff00) >>
				   8);
		macMgmtHdr->seqControl.fragNum = 0;
		macMgmtHdr->seqControl.seqNumLo = 0;
		macMgmtHdr->seqControl.seqNumHi = 2;
		qdf_mem_copy((void *)macMgmtHdr->da, (void *)pSta[0].bssId, 6);
		qdf_mem_copy(macMgmtHdr->sa, pSta[0].staAddr, 6);
		qdf_mem_copy((void *)macMgmtHdr->bssId, (void *)pSta[0].bssId,
			     6);

	}

	return;
#endif
}

/* utils_parser.c ends here. */
