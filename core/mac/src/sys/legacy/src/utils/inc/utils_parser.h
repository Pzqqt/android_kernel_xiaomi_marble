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
 * This file utils_parser.h contains the utility function protos
 * used internally by the parser
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __UTILS_PARSE_H__
#define __UTILS_PARSE_H__

#include <stdarg.h>
#include "sir_api.h"
#include "dot11f.h"
#include "utils_api.h"

void convert_ssid(tpAniSirGlobal, tSirMacSSid *, tDot11fIESSID *);
void convert_supp_rates(tpAniSirGlobal, tSirMacRateSet *, tDot11fIESuppRates *);
void convert_fh_params(tpAniSirGlobal, tSirMacFHParamSet *,
		       tDot11fIEFHParamSet *);
void convert_ext_supp_rates(tpAniSirGlobal, tSirMacRateSet *,
			    tDot11fIEExtSuppRates *);
void convert_qos_caps(tpAniSirGlobal, tSirMacQosCapabilityIE *,
		      tDot11fIEQOSCapsAp *);
void convert_qos_caps_station(tpAniSirGlobal, tSirMacQosCapabilityStaIE *,
			      tDot11fIEQOSCapsStation *);
QDF_STATUS convert_wpa(tpAniSirGlobal, tSirMacWpaInfo *, tDot11fIEWPA *);
QDF_STATUS convert_wpa_opaque(tpAniSirGlobal, tSirMacWpaInfo *,
			      tDot11fIEWPAOpaque *);
QDF_STATUS convert_wapi_opaque(tpAniSirGlobal, tSirMacWapiInfo *,
			       tDot11fIEWAPIOpaque *);
QDF_STATUS convert_rsn(tpAniSirGlobal, tSirMacRsnInfo *, tDot11fIERSN *);
QDF_STATUS convert_rsn_opaque(tpAniSirGlobal, tSirMacRsnInfo *,
			      tDot11fIERSNOpaque *);
void convert_power_caps(tpAniSirGlobal, tSirMacPowerCapabilityIE *,
			tDot11fIEPowerCaps *);
void convert_supp_channels(tpAniSirGlobal, tSirMacSupportedChannelIE *,
			   tDot11fIESuppChannels *);
void convert_cf_params(tpAniSirGlobal, tSirMacCfParamSet *, tDot11fIECFParams *);
void convert_tim(tpAniSirGlobal, tSirMacTim *, tDot11fIETIM *);
void convert_country(tpAniSirGlobal, tSirCountryInformation *,
		     tDot11fIECountry *);
void convert_wmm_params(tpAniSirGlobal, tSirMacEdcaParamSetIE *,
			tDot11fIEWMMParams *);
void convert_erp_info(tpAniSirGlobal, tSirMacErpInfo *, tDot11fIEERPInfo *);
void convert_edca_param(tpAniSirGlobal, tSirMacEdcaParamSetIE *,
			tDot11fIEEDCAParamSet *);
void convert_mu_edca_param(tpAniSirGlobal mac_ctx,
			tSirMacEdcaParamSetIE *mu_edca,
			tDot11fIEmu_edca_param_set *ie);
void convert_tspec(tpAniSirGlobal, tSirMacTspecIE *, tDot11fIETSPEC *);
QDF_STATUS convert_tclas(tpAniSirGlobal, tSirTclasInfo *, tDot11fIETCLAS *);
void convert_wmmtspec(tpAniSirGlobal, tSirMacTspecIE *, tDot11fIEWMMTSPEC *);
QDF_STATUS convert_wmmtclas(tpAniSirGlobal, tSirTclasInfo *,
			    tDot11fIEWMMTCLAS *);
void convert_ts_delay(tpAniSirGlobal, tSirMacTsDelayIE *, tDot11fIETSDelay *);
void convert_schedule(tpAniSirGlobal, tSirMacScheduleIE *, tDot11fIESchedule *);
void convert_wmm_schedule(tpAniSirGlobal, tSirMacScheduleIE *,
			  tDot11fIEWMMSchedule *);
QDF_STATUS convert_wsc_opaque(tpAniSirGlobal, tSirAddie *,
			      tDot11fIEWscIEOpaque *);
QDF_STATUS convert_p2p_opaque(tpAniSirGlobal, tSirAddie *,
			      tDot11fIEP2PIEOpaque *);
#ifdef WLAN_FEATURE_WFD
QDF_STATUS convert_wfd_opaque(tpAniSirGlobal, tSirAddie *,
			      tDot11fIEWFDIEOpaque *);
#endif
void convert_qos_mapset_frame(tpAniSirGlobal, tSirQosMapSet *,
			      tDot11fIEQosMapSet *);

#endif
