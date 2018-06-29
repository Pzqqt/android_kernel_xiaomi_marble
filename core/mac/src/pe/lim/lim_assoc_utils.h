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
 * This file lim_assoc_utils.h contains the utility definitions
 * LIM uses while processing Re/Association messages.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 05/26/10       js             WPA handling in (Re)Assoc frames
 *
 */
#ifndef __LIM_ASSOC_UTILS_H
#define __LIM_ASSOC_UTILS_H

#include "sir_api.h"
#include "sir_debug.h"
#include "cfg_api.h"

#include "lim_types.h"

#define SIZE_OF_NOA_DESCRIPTOR 13
#define MAX_NOA_PERIOD_IN_MICROSECS 3000000

uint32_t lim_cmp_ssid(tSirMacSSid *, tpPESession);
uint8_t lim_compare_capabilities(tpAniSirGlobal,
				 tSirAssocReq *,
				 tSirMacCapabilityInfo *, tpPESession);
uint8_t lim_check_rx_basic_rates(tpAniSirGlobal, tSirMacRateSet, tpPESession);
uint8_t lim_check_rx_rsn_ie_match(tpAniSirGlobal mac_ctx,
				  tDot11fIERSN * const rx_rsn_ie,
				  tpPESession session_entry, uint8_t sta_is_ht,
				  bool *pmf_connection);
uint8_t lim_check_rx_wpa_ie_match(tpAniSirGlobal, tDot11fIEWPA, tpPESession,
				  uint8_t);
uint8_t lim_check_mcs_set(tpAniSirGlobal pMac, uint8_t *supportedMCSSet);
void limPostDummyToTmRing(tpAniSirGlobal, tpDphHashNode);
void limPostPacketToTdRing(tpAniSirGlobal, tpDphHashNode, uint8_t);
QDF_STATUS lim_cleanup_rx_path(tpAniSirGlobal, tpDphHashNode, tpPESession);
void lim_reject_association(tpAniSirGlobal, tSirMacAddr, uint8_t,
			    uint8_t, tAniAuthType, uint16_t, uint8_t,
			    enum eSirMacStatusCodes, tpPESession);

QDF_STATUS lim_populate_peer_rate_set(tpAniSirGlobal pMac,
					 tpSirSupportedRates pRates,
					 uint8_t *pSupportedMCSSet,
					 uint8_t basicOnly,
					 tpPESession psessionEntry,
					 tDot11fIEVHTCaps *pVHTCaps,
					 tDot11fIEhe_cap *he_caps);

QDF_STATUS lim_populate_own_rate_set(tpAniSirGlobal pMac,
					tpSirSupportedRates pRates,
					uint8_t *pSupportedMCSSet,
					uint8_t basicOnly,
					tpPESession psessionEntry,
					tDot11fIEVHTCaps *pVHTCaps,
					tDot11fIEhe_cap *he_caps);

QDF_STATUS
lim_populate_matching_rate_set(tpAniSirGlobal pMac,
			       tpDphHashNode pStaDs,
			       tSirMacRateSet *pOperRateSet,
			       tSirMacRateSet *pExtRateSet,
			       uint8_t *pSupportedMCSSet,
			       tpPESession psessionEntry,
			       tDot11fIEVHTCaps *pVHTCaps,
			       tDot11fIEhe_cap *he_caps);

QDF_STATUS lim_add_sta(tpAniSirGlobal, tpDphHashNode, uint8_t, tpPESession);
QDF_STATUS lim_del_bss(tpAniSirGlobal, tpDphHashNode, uint16_t, tpPESession);
QDF_STATUS lim_del_sta(tpAniSirGlobal, tpDphHashNode, bool, tpPESession);
QDF_STATUS lim_add_sta_self(tpAniSirGlobal, uint16_t, uint8_t, tpPESession);

void lim_teardown_infra_bss(tpAniSirGlobal, tpPESession);
#ifdef WLAN_FEATURE_HOST_ROAM
void lim_restore_pre_reassoc_state(tpAniSirGlobal,
				   tSirResultCodes, uint16_t, tpPESession);
void lim_post_reassoc_failure(tpAniSirGlobal,
			      tSirResultCodes, uint16_t, tpPESession);
bool lim_is_reassoc_in_progress(tpAniSirGlobal, tpPESession);

void lim_handle_add_bss_in_re_assoc_context(tpAniSirGlobal pMac,
		tpDphHashNode pStaDs, tpPESession psessionEntry);
void lim_handle_del_bss_in_re_assoc_context(tpAniSirGlobal pMac,
		   tpDphHashNode pStaDs, tpPESession psessionEntry);
void lim_send_retry_reassoc_req_frame(tpAniSirGlobal pMac,
	      tLimMlmReassocReq *pMlmReassocReq, tpPESession psessionEntry);
QDF_STATUS lim_add_ft_sta_self(tpAniSirGlobal pMac, uint16_t assocId,
				  tpPESession psessionEntry);
#else
static inline void lim_restore_pre_reassoc_state(tpAniSirGlobal mac_ctx,
			tSirResultCodes res_code, uint16_t prot_status,
			tpPESession pe_session)
{}
static inline void lim_post_reassoc_failure(tpAniSirGlobal mac_ctx,
			      tSirResultCodes res_code, uint16_t prot_status,
			      tpPESession pe_session)
{}
static inline void lim_handle_add_bss_in_re_assoc_context(tpAniSirGlobal pMac,
		tpDphHashNode pStaDs, tpPESession psessionEntry)
{}
static inline void lim_handle_del_bss_in_re_assoc_context(tpAniSirGlobal pMac,
		   tpDphHashNode pStaDs, tpPESession psessionEntry)
{}
static inline void lim_send_retry_reassoc_req_frame(tpAniSirGlobal pMac,
	      tLimMlmReassocReq *pMlmReassocReq, tpPESession psessionEntry)
{}
static inline bool lim_is_reassoc_in_progress(tpAniSirGlobal mac_ctx,
		tpPESession pe_session)
{
	return false;
}
static inline QDF_STATUS lim_add_ft_sta_self(tpAniSirGlobal pMac,
		uint16_t assocId, tpPESession psessionEntry)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline bool lim_is_roam_synch_in_progress(tpPESession pe_session)
{
	return pe_session->bRoamSynchInProgress;
}
#else
static inline bool lim_is_roam_synch_in_progress(tpPESession pe_session)
{
	return false;
}
#endif

void
lim_send_del_sta_cnf(tpAniSirGlobal pMac, struct qdf_mac_addr sta_dsaddr,
		     uint16_t staDsAssocId, tLimMlmStaContext mlmStaContext,
		     tSirResultCodes statusCode, tpPESession psessionEntry);

void lim_handle_cnf_wait_timeout(tpAniSirGlobal pMac, uint16_t staId);
void lim_delete_dph_hash_entry(tpAniSirGlobal, tSirMacAddr, uint16_t, tpPESession);
void lim_check_and_announce_join_success(tpAniSirGlobal,
					 tSirProbeRespBeacon *,
					 tpSirMacMgmtHdr, tpPESession);
void lim_update_re_assoc_globals(tpAniSirGlobal pMac,
				 tpSirAssocRsp pAssocRsp,
				 tpPESession psessionEntry);

void lim_update_assoc_sta_datas(tpAniSirGlobal pMac,
				tpDphHashNode pStaDs, tpSirAssocRsp pAssocRsp,
				tpPESession psessionEntry);

QDF_STATUS lim_sta_send_add_bss(tpAniSirGlobal pMac, tpSirAssocRsp pAssocRsp,
				   tpSchBeaconStruct pBeaconStruct,
				   tpSirBssDescription bssDescription,
				   uint8_t updateEntry, tpPESession psessionEntry);
QDF_STATUS lim_sta_send_add_bss_pre_assoc(tpAniSirGlobal pMac, uint8_t updateEntry,
					     tpPESession psessionEntry);

void lim_prepare_and_send_del_sta_cnf(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
				      tSirResultCodes statusCode, tpPESession);
QDF_STATUS lim_extract_ap_capabilities(tpAniSirGlobal pMac, uint8_t *pIE,
					  uint16_t ieLen,
					  tpSirProbeRespBeacon beaconStruct);
void lim_init_pre_auth_timer_table(tpAniSirGlobal pMac,
				   tpLimPreAuthTable pPreAuthTimerTable);
tpLimPreAuthNode lim_acquire_free_pre_auth_node(tpAniSirGlobal pMac,
						tpLimPreAuthTable
						pPreAuthTimerTable);
tpLimPreAuthNode lim_get_pre_auth_node_from_index(tpAniSirGlobal pMac,
						  tpLimPreAuthTable pAuthTable,
						  uint32_t authNodeIdx);

/* Util API to check if the channels supported by STA is within range */
QDF_STATUS lim_is_dot11h_supported_channels_valid(tpAniSirGlobal pMac,
						     tSirAssocReq *assoc);

/* Util API to check if the txpower supported by STA is within range */
QDF_STATUS lim_is_dot11h_power_capabilities_in_range(tpAniSirGlobal pMac,
							tSirAssocReq *assoc,
							tpPESession);
/* API to fill in RX Highest Supported data Rate */
void lim_fill_rx_highest_supported_rate(tpAniSirGlobal pMac,
					uint16_t *rxHighestRate,
					uint8_t *pSupportedMCSSet);
#ifdef WLAN_FEATURE_11W
void lim_send_sme_unprotected_mgmt_frame_ind(tpAniSirGlobal pMac, uint8_t frameType,
					     uint8_t *frame, uint32_t frameLen,
					     uint16_t sessionId,
					     tpPESession psessionEntry);
#endif

#ifdef FEATURE_WLAN_ESE
void lim_send_sme_tsm_ie_ind(tpAniSirGlobal pMac, tpPESession psessionEntry,
			     uint8_t tid, uint8_t state, uint16_t measInterval);
#else
static inline void lim_send_sme_tsm_ie_ind(tpAniSirGlobal pMac,
	tpPESession psessionEntry, uint8_t tid,
	uint8_t state, uint16_t measInterval)
{}
#endif /* FEATURE_WLAN_ESE */

QDF_STATUS lim_populate_vht_mcs_set(tpAniSirGlobal pMac,
				       tpSirSupportedRates pRates,
				       tDot11fIEVHTCaps *pPeerVHTCaps,
				       tpPESession psessionEntry,
				       uint8_t nss);

#endif /* __LIM_ASSOC_UTILS_H */
