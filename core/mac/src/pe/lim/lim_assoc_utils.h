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

uint32_t lim_cmp_ssid(tSirMacSSid *, struct pe_session *);
uint8_t lim_compare_capabilities(tpAniSirGlobal,
				 tSirAssocReq *,
				 tSirMacCapabilityInfo *, struct pe_session *);
uint8_t lim_check_rx_basic_rates(tpAniSirGlobal, tSirMacRateSet, struct pe_session *);
uint8_t lim_check_rx_rsn_ie_match(tpAniSirGlobal mac_ctx,
				  tDot11fIERSN * const rx_rsn_ie,
				  struct pe_session *session_entry, uint8_t sta_is_ht,
				  bool *pmf_connection);
uint8_t lim_check_rx_wpa_ie_match(tpAniSirGlobal, tDot11fIEWPA *, struct pe_session *,
				  uint8_t);
uint8_t lim_check_mcs_set(tpAniSirGlobal mac, uint8_t *supportedMCSSet);
void limPostDummyToTmRing(tpAniSirGlobal, tpDphHashNode);
void limPostPacketToTdRing(tpAniSirGlobal, tpDphHashNode, uint8_t);
QDF_STATUS lim_cleanup_rx_path(tpAniSirGlobal, tpDphHashNode, struct pe_session *);
void lim_reject_association(tpAniSirGlobal, tSirMacAddr, uint8_t,
			    uint8_t, tAniAuthType, uint16_t, uint8_t,
			    enum eSirMacStatusCodes, struct pe_session *);

QDF_STATUS lim_populate_peer_rate_set(tpAniSirGlobal mac,
					 tpSirSupportedRates pRates,
					 uint8_t *pSupportedMCSSet,
					 uint8_t basicOnly,
					 struct pe_session *psessionEntry,
					 tDot11fIEVHTCaps *pVHTCaps,
					 tDot11fIEhe_cap *he_caps);

QDF_STATUS lim_populate_own_rate_set(tpAniSirGlobal mac,
					tpSirSupportedRates pRates,
					uint8_t *pSupportedMCSSet,
					uint8_t basicOnly,
					struct pe_session *psessionEntry,
					tDot11fIEVHTCaps *pVHTCaps,
					tDot11fIEhe_cap *he_caps);

QDF_STATUS
lim_populate_matching_rate_set(tpAniSirGlobal mac,
			       tpDphHashNode pStaDs,
			       tSirMacRateSet *pOperRateSet,
			       tSirMacRateSet *pExtRateSet,
			       uint8_t *pSupportedMCSSet,
			       struct pe_session *psessionEntry,
			       tDot11fIEVHTCaps *pVHTCaps,
			       tDot11fIEhe_cap *he_caps);

QDF_STATUS lim_add_sta(tpAniSirGlobal, tpDphHashNode, uint8_t, struct pe_session *);
QDF_STATUS lim_del_bss(tpAniSirGlobal, tpDphHashNode, uint16_t, struct pe_session *);
QDF_STATUS lim_del_sta(tpAniSirGlobal, tpDphHashNode, bool, struct pe_session *);
QDF_STATUS lim_add_sta_self(tpAniSirGlobal, uint16_t, uint8_t, struct pe_session *);

void lim_teardown_infra_bss(tpAniSirGlobal, struct pe_session *);
#ifdef WLAN_FEATURE_HOST_ROAM
void lim_restore_pre_reassoc_state(tpAniSirGlobal,
				   tSirResultCodes, uint16_t, struct pe_session *);
void lim_post_reassoc_failure(tpAniSirGlobal,
			      tSirResultCodes, uint16_t, struct pe_session *);
bool lim_is_reassoc_in_progress(tpAniSirGlobal, struct pe_session *);

void lim_handle_add_bss_in_re_assoc_context(tpAniSirGlobal mac,
		tpDphHashNode pStaDs, struct pe_session *psessionEntry);
void lim_handle_del_bss_in_re_assoc_context(tpAniSirGlobal mac,
		   tpDphHashNode pStaDs, struct pe_session *psessionEntry);
void lim_send_retry_reassoc_req_frame(tpAniSirGlobal mac,
	      tLimMlmReassocReq *pMlmReassocReq, struct pe_session *psessionEntry);
QDF_STATUS lim_add_ft_sta_self(tpAniSirGlobal mac, uint16_t assocId,
				  struct pe_session *psessionEntry);
#else
static inline void lim_restore_pre_reassoc_state(tpAniSirGlobal mac_ctx,
			tSirResultCodes res_code, uint16_t prot_status,
			struct pe_session *pe_session)
{}
static inline void lim_post_reassoc_failure(tpAniSirGlobal mac_ctx,
			      tSirResultCodes res_code, uint16_t prot_status,
			      struct pe_session *pe_session)
{}
static inline void lim_handle_add_bss_in_re_assoc_context(tpAniSirGlobal mac,
		tpDphHashNode pStaDs, struct pe_session *psessionEntry)
{}
static inline void lim_handle_del_bss_in_re_assoc_context(tpAniSirGlobal mac,
		   tpDphHashNode pStaDs, struct pe_session *psessionEntry)
{}
static inline void lim_send_retry_reassoc_req_frame(tpAniSirGlobal mac,
	      tLimMlmReassocReq *pMlmReassocReq, struct pe_session *psessionEntry)
{}
static inline bool lim_is_reassoc_in_progress(tpAniSirGlobal mac_ctx,
		struct pe_session *pe_session)
{
	return false;
}
static inline QDF_STATUS lim_add_ft_sta_self(tpAniSirGlobal mac,
		uint16_t assocId, struct pe_session *psessionEntry)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline bool lim_is_roam_synch_in_progress(struct pe_session *pe_session)
{
	return pe_session->bRoamSynchInProgress;
}
#else
static inline bool lim_is_roam_synch_in_progress(struct pe_session *pe_session)
{
	return false;
}
#endif

void
lim_send_del_sta_cnf(tpAniSirGlobal mac, struct qdf_mac_addr sta_dsaddr,
		     uint16_t staDsAssocId, tLimMlmStaContext mlmStaContext,
		     tSirResultCodes statusCode, struct pe_session *psessionEntry);

void lim_handle_cnf_wait_timeout(tpAniSirGlobal mac, uint16_t staId);
void lim_delete_dph_hash_entry(tpAniSirGlobal, tSirMacAddr, uint16_t, struct pe_session *);
void lim_check_and_announce_join_success(tpAniSirGlobal,
					 tSirProbeRespBeacon *,
					 tpSirMacMgmtHdr, struct pe_session *);
void lim_update_re_assoc_globals(tpAniSirGlobal mac,
				 tpSirAssocRsp pAssocRsp,
				 struct pe_session *psessionEntry);

void lim_update_assoc_sta_datas(tpAniSirGlobal mac,
				tpDphHashNode pStaDs, tpSirAssocRsp pAssocRsp,
				struct pe_session *psessionEntry);

QDF_STATUS lim_sta_send_add_bss(tpAniSirGlobal mac, tpSirAssocRsp pAssocRsp,
				   tpSchBeaconStruct pBeaconStruct,
				   tpSirBssDescription bssDescription,
				   uint8_t updateEntry, struct pe_session *psessionEntry);
QDF_STATUS lim_sta_send_add_bss_pre_assoc(tpAniSirGlobal mac, uint8_t updateEntry,
					     struct pe_session *psessionEntry);

void lim_prepare_and_send_del_sta_cnf(tpAniSirGlobal mac, tpDphHashNode pStaDs,
				      tSirResultCodes statusCode, struct pe_session *);
QDF_STATUS lim_extract_ap_capabilities(tpAniSirGlobal mac, uint8_t *pIE,
					  uint16_t ieLen,
					  tpSirProbeRespBeacon beaconStruct);
void lim_init_pre_auth_timer_table(tpAniSirGlobal mac,
				   tpLimPreAuthTable pPreAuthTimerTable);
tpLimPreAuthNode lim_acquire_free_pre_auth_node(tpAniSirGlobal mac,
						tpLimPreAuthTable
						pPreAuthTimerTable);
tpLimPreAuthNode lim_get_pre_auth_node_from_index(tpAniSirGlobal mac,
						  tpLimPreAuthTable pAuthTable,
						  uint32_t authNodeIdx);

/* Util API to check if the channels supported by STA is within range */
QDF_STATUS lim_is_dot11h_supported_channels_valid(tpAniSirGlobal mac,
						     tSirAssocReq *assoc);

/* Util API to check if the txpower supported by STA is within range */
QDF_STATUS lim_is_dot11h_power_capabilities_in_range(tpAniSirGlobal mac,
							tSirAssocReq *assoc,
							struct pe_session *);
/* API to fill in RX Highest Supported data Rate */
void lim_fill_rx_highest_supported_rate(tpAniSirGlobal mac,
					uint16_t *rxHighestRate,
					uint8_t *pSupportedMCSSet);
#ifdef WLAN_FEATURE_11W
void lim_send_sme_unprotected_mgmt_frame_ind(tpAniSirGlobal mac, uint8_t frameType,
					     uint8_t *frame, uint32_t frameLen,
					     uint16_t sessionId,
					     struct pe_session *psessionEntry);
#endif

#ifdef FEATURE_WLAN_ESE
void lim_send_sme_tsm_ie_ind(tpAniSirGlobal mac, struct pe_session *psessionEntry,
			     uint8_t tid, uint8_t state, uint16_t measInterval);
#else
static inline void lim_send_sme_tsm_ie_ind(tpAniSirGlobal mac,
	struct pe_session *psessionEntry, uint8_t tid,
	uint8_t state, uint16_t measInterval)
{}
#endif /* FEATURE_WLAN_ESE */

QDF_STATUS lim_populate_vht_mcs_set(tpAniSirGlobal mac,
				       tpSirSupportedRates pRates,
				       tDot11fIEVHTCaps *pPeerVHTCaps,
				       struct pe_session *psessionEntry,
				       uint8_t nss);

#endif /* __LIM_ASSOC_UTILS_H */
