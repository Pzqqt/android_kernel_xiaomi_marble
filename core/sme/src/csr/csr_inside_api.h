/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * DOC: csr_inside_api.h
 *
 * Define interface only used by CSR.
 */
#ifndef CSR_INSIDE_API_H__
#define CSR_INSIDE_API_H__

#include "csr_support.h"
#include "sme_inside.h"
#include "cds_reg_service.h"
#include "wlan_objmgr_vdev_obj.h"

bool csr_is_supported_channel(struct mac_context *mac, uint32_t chan_freq);

enum csr_roamcomplete_result {
	eCsrNothingToJoin,
	eCsrStartBssSuccess,
	eCsrStartBssFailure,
	eCsrStopBssSuccess,
	eCsrStopBssFailure,
};

struct tag_csrscan_result {
	tListElem Link;
	/* Preferred Encryption type that matched with profile. */
	eCsrEncryptionType ucEncryptionType;
	eCsrEncryptionType mcEncryptionType;
	/* Preferred auth type that matched with the profile. */
	enum csr_akm_type authType;
	int  bss_score;
	uint8_t retry_count;

	tCsrScanResultInfo Result;
	/*
	 * WARNING - Do not add any element here
	 * This member Result must be the last in the structure because the end
	 * of struct bss_description (inside) is an array with nonknown size at
	 * this time.
	 */
};

struct scan_result_list {
	tDblLinkList List;
	tListElem *pCurEntry;
};

#define CSR_IS_WAIT_FOR_KEY(mac, sessionId) \
		 (CSR_IS_ROAM_JOINED(mac, sessionId) && \
		  CSR_IS_ROAM_SUBSTATE_WAITFORKEY(mac, sessionId))

enum csr_roam_state csr_roam_state_change(struct mac_context *mac,
					  enum csr_roam_state NewRoamState,
					  uint8_t sessionId);
void csr_roaming_state_msg_processor(struct mac_context *mac, void *msg_buf);
void csr_roam_joined_state_msg_processor(struct mac_context *mac,
					 void *msg_buf);

void csr_release_command_roam(struct mac_context *mac, tSmeCmd *pCommand);
void csr_release_command_wm_status_change(struct mac_context *mac,
					  tSmeCmd *pCommand);

QDF_STATUS csr_roam_copy_profile(struct mac_context *mac,
				 struct csr_roam_profile *pDstProfile,
				 struct csr_roam_profile *pSrcProfile,
				 uint8_t vdev_id);
QDF_STATUS csr_scan_open(struct mac_context *mac);
QDF_STATUS csr_scan_close(struct mac_context *mac);

void csr_free_scan_result_entry(struct mac_context *mac, struct tag_csrscan_result
				*pResult);

QDF_STATUS csr_roam_call_callback(struct mac_context *mac, uint32_t sessionId,
				  struct csr_roam_info *roam_info,
				  uint32_t roamId,
				  eRoamCmdStatus u1, eCsrRoamResult u2);
QDF_STATUS csr_issue_bss_start(struct mac_context *mac, uint8_t vdev_id,
			       struct csr_roam_profile *pProfile,
			       uint32_t roamId);
void csr_roam_complete(struct mac_context *mac, enum csr_roamcomplete_result Result,
		       void *Context, uint8_t session_id);

/**
 * csr_issue_set_context_req_helper  - Function to fill unicast/broadcast keys
 * request to set the keys to fw
 * @mac:         Poiner to mac context
 * @vdev_id:     vdev id
 * @bssid:       Connected BSSID
 * @addkey:      Is add key request to crypto
 * @unicast:     Unicast(1) or broadcast key(0)
 * @key_direction: Key used in TX or RX or both Tx and RX path
 * @key_id:       Key index
 * @key_length:   Key length
 * @key:          Pointer to the key
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
csr_issue_set_context_req_helper(struct mac_context *mac,
				 uint32_t session_id,
				 tSirMacAddr *bssid, bool addkey,
				 bool unicast, tAniKeyDirection key_direction,
				 uint8_t key_id, uint16_t key_length,
				 uint8_t *key);
void csr_roam_check_for_link_status_change(struct mac_context *mac,
					tSirSmeRsp *pSirMsg);

QDF_STATUS csr_roam_issue_start_bss(struct mac_context *mac, uint32_t sessionId,
				    struct csr_roamstart_bssparams *pParam,
				    struct csr_roam_profile *pProfile,
				    uint32_t roamId);
QDF_STATUS csr_roam_issue_stop_bss(struct mac_context *mac, uint32_t sessionId,
				   enum csr_roam_substate NewSubstate);
QDF_STATUS csr_send_mb_disassoc_req_msg(struct mac_context *mac, uint32_t sessionId,
					tSirMacAddr bssId, uint16_t reasonCode);
QDF_STATUS csr_send_mb_deauth_req_msg(struct mac_context *mac, uint32_t sessionId,
				      tSirMacAddr bssId, uint16_t reasonCode);
QDF_STATUS csr_send_mb_disassoc_cnf_msg(struct mac_context *mac,
					struct disassoc_ind *pDisassocInd);
QDF_STATUS csr_send_mb_deauth_cnf_msg(struct mac_context *mac,
				      struct deauth_ind *pDeauthInd);
QDF_STATUS csr_send_assoc_cnf_msg(struct mac_context *mac,
				  struct assoc_ind *pAssocInd,
				  QDF_STATUS status,
				  enum wlan_status_code mac_status_code);
QDF_STATUS csr_send_mb_start_bss_req_msg(struct mac_context *mac,
					 uint32_t sessionId,
					 eCsrRoamBssType bssType,
					 struct csr_roamstart_bssparams *pParam);
QDF_STATUS csr_send_mb_stop_bss_req_msg(struct mac_context *mac,
					uint32_t sessionId);

/**
 * csr_get_cfg_valid_channels() - Get valid channel frequency list
 * @mac: mac context
 * @ch_freq_list: valid channel frequencies
 * @num_ch_freq: valid channel nummber
 *
 * This function returns the valid channel frequencies.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS csr_get_cfg_valid_channels(struct mac_context *mac,
				      uint32_t *ch_freq_list,
				      uint32_t *num_ch_freq);

/* to free memory allocated inside the profile structure */
void csr_release_profile(struct mac_context *mac,
			 struct csr_roam_profile *pProfile);

enum csr_cfgdot11mode
csr_get_cfg_dot11_mode_from_csr_phy_mode(struct csr_roam_profile *pProfile,
					 eCsrPhyMode phyMode,
					 bool fProprietary);

uint32_t csr_translate_to_wni_cfg_dot11_mode(struct mac_context *mac,
				    enum csr_cfgdot11mode csrDot11Mode);
void csr_save_channel_power_for_band(struct mac_context *mac, bool fPopulate5GBand);
void csr_apply_channel_power_info_to_fw(struct mac_context *mac,
					struct csr_channel *pChannelList,
					uint8_t *countryCode);
void csr_apply_power2_current(struct mac_context *mac);
void csr_apply_channel_power_info_wrapper(struct mac_context *mac);
QDF_STATUS csr_save_to_channel_power2_g_5_g(struct mac_context *mac,
					uint32_t tableSize,
					struct pwr_channel_info *channelTable);

/*
 * csr_prepare_vdev_delete() - CSR api to delete vdev
 * @mac_ctx: pointer to mac context
 * @vdev_id: vdev id to be deleted.
 * @cleanup: clean up vdev session on true
 *
 * Return QDF_STATUS
 */
QDF_STATUS csr_prepare_vdev_delete(struct mac_context *mac_ctx,
				   uint8_t vdev_id, bool cleanup);

/*
 * csr_cleanup_vdev_session() - CSR api to cleanup vdev
 * @mac_ctx: pointer to mac context
 * @vdev_id: vdev id to be deleted.
 *
 * This API is used to clean up vdev information gathered during
 * vdev was enabled.
 *
 * Return QDF_STATUS
 */
void csr_cleanup_vdev_session(struct mac_context *mac, uint8_t vdev_id);

QDF_STATUS csr_roam_get_session_id_from_bssid(struct mac_context *mac,
						struct qdf_mac_addr *bssid,
					      uint32_t *pSessionId);

/*
 * csr_scan_get_result() - Return scan results based on filter
 * @mac: Pointer to Global MAC structure
 * @filter: If pFilter is NULL, all cached results are returned
 * @phResult: an object for the result.
 *
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_get_result(struct mac_context *mac,
			       struct scan_filter *filter,
			       tScanResultHandle *phResult);

/**
 * csr_scan_get_result_for_bssid - gets the scan result from scan cache for the
 *      bssid specified
 * @mac_ctx: mac context
 * @bssid: bssid to get the scan result for
 * @res: pointer to tCsrScanResultInfo
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_scan_get_result_for_bssid(struct mac_context *mac_ctx,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res);

/*
 * csr_scan_filter_results() -
 *  Filter scan results based on valid channel list.
 *
 * mac - Pointer to Global MAC structure
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_filter_results(struct mac_context *mac);

/*
 * csr_scan_result_get_first
 * Returns the first element of scan result.
 *
 * hScanResult - returned from csr_scan_get_result
 * tCsrScanResultInfo * - NULL if no result
 */
tCsrScanResultInfo *csr_scan_result_get_first(struct mac_context *mac,
					      tScanResultHandle hScanResult);
/*
 * csr_scan_result_get_next
 * Returns the next element of scan result. It can be called without calling
 * csr_scan_result_get_first first
 *
 * hScanResult - returned from csr_scan_get_result
 * Return Null if no result or reach the end
 */
tCsrScanResultInfo *csr_scan_result_get_next(struct mac_context *mac,
					     tScanResultHandle hScanResult);

/* some support functions */
bool csr_is11h_supported(struct mac_context *mac);
bool csr_is_wmm_supported(struct mac_context *mac);

/* Return SUCCESS is the command is queued, failed */
QDF_STATUS csr_queue_sme_command(struct mac_context *mac, tSmeCmd *pCommand,
				 bool fHighPriority);
tSmeCmd *csr_get_command_buffer(struct mac_context *mac);
void csr_release_command(struct mac_context *mac, tSmeCmd *pCommand);
void csr_release_command_buffer(struct mac_context *mac, tSmeCmd *pCommand);

/**
 * csr_get_vdev_type_nss() - gets the nss value based on vdev type
 * @dev_mode: current device operating mode.
 * @nss2g: Pointer to the 2G Nss parameter.
 * @nss5g: Pointer to the 5G Nss parameter.
 *
 * Fills the 2G and 5G Nss values based on device mode.
 *
 * Return: None
 */
void csr_get_vdev_type_nss(enum QDF_OPMODE dev_mode, uint8_t *nss_2g,
			   uint8_t *nss_5g);

/**
 * csr_send_set_ie  - Send Set IE request to lim
 * @type: Vdev type
 * @sub_type: Vdev sub type
 * @vdev_id: Vdev id
 *
 * Return: None
 */
void csr_send_set_ie(uint8_t type, uint8_t sub_type, uint8_t vdev_id);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

/* Security */
#define WLAN_SECURITY_EVENT_REMOVE_KEY_REQ  5
#define WLAN_SECURITY_EVENT_REMOVE_KEY_RSP  6
#define WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND  7
#define WLAN_SECURITY_EVENT_PMKID_UPDATE    8
#define WLAN_SECURITY_EVENT_MIC_ERROR       9
#define WLAN_SECURITY_EVENT_SET_UNICAST_REQ  10
#define WLAN_SECURITY_EVENT_SET_UNICAST_RSP  11
#define WLAN_SECURITY_EVENT_SET_BCAST_REQ    12
#define WLAN_SECURITY_EVENT_SET_BCAST_RSP    13

#define NO_MATCH    0
#define MATCH       1

#define WLAN_SECURITY_STATUS_SUCCESS        0
#define WLAN_SECURITY_STATUS_FAILURE        1

/* Scan */
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ     1
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP     2
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ    3
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_RSP    4
#define WLAN_SCAN_EVENT_HO_SCAN_REQ         5
#define WLAN_SCAN_EVENT_HO_SCAN_RSP         6

#define WLAN_SCAN_STATUS_SUCCESS        0
#define WLAN_SCAN_STATUS_FAILURE        1
#define WLAN_SCAN_STATUS_ABORT          2

#define AUTO_PICK       0
#define SPECIFIED       1

#define WLAN_IBSS_STATUS_SUCCESS        0
#define WLAN_IBSS_STATUS_FAILURE        1

/* 11d */
#define WLAN_80211D_EVENT_COUNTRY_SET   0
#define WLAN_80211D_EVENT_RESET         1

#define WLAN_80211D_DISABLED         0
#define WLAN_80211D_SUPPORT_MULTI_DOMAIN     1
#define WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN     2
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */
/*
 * csr_scan_result_purge() -
 * Remove all items(tCsrScanResult) in the list and free memory for each item
 * hScanResult - returned from csr_scan_get_result. hScanResult is considered
 * gone by calling this function and even before this function reutrns.
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_result_purge(struct mac_context *mac,
				 tScanResultHandle hScanResult);

/* /////////////////////////////////////////Common Scan ends */

/**
 * csr_bss_start() - A wrapper function to request CSR to inititiate start bss
 * @mac: mac ctx
 * @vdev_id: the vdev id.
 * @profile: description of bss to start
 * @roam_id: to get back the request ID
 *
 * Return QDF_STATUS
 */
QDF_STATUS csr_bss_start(struct mac_context *mac, uint32_t vdev_id,
			 struct csr_roam_profile *profile, uint32_t *roam_id);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*
 * csr_get_pmk_info(): store PMK in pmk_cache
 * @mac_ctx: pointer to global structure for MAC
 * @session_id: Sme session id
 * @pmk_cache: pointer to a structure of Pmk
 *
 * This API gets the PMK from the session and
 * stores it in the pmk_cache
 *
 * Return: none
 */
void csr_get_pmk_info(struct mac_context *mac_ctx, uint8_t session_id,
		      struct wlan_crypto_pmksa *pmk_cache);

/*
 * csr_roam_set_psk_pmk() - store PSK/PMK in CSR session
 * @mac  - pointer to global structure for MAC
 * @pmksa: PMKSA entry
 * @vdev_id - vdev id
 * @update_to_fw - Send RSO update config command to firmware to update
 * PMK
 *
 * Return QDF_STATUS - usually it succeed unless sessionId is not found
 */
QDF_STATUS csr_roam_set_psk_pmk(struct mac_context *mac,
				struct wlan_crypto_pmksa *pmksa,
				uint8_t vdev_id, bool update_to_fw);

/**
 * csr_set_pmk_cache_ft() - store MDID in PMK cache
 *
 * @mac  - pointer to global structure for MAC
 * @session_id - Sme session id
 * @pmk_cache: pointer to a structure of PMK
 *
 * Return QDF_STATUS - usually it succeed unless session_id is not found
 */
QDF_STATUS csr_set_pmk_cache_ft(struct mac_context *mac, uint8_t vdev_id,
				struct wlan_crypto_pmksa *pmk_cache);
#endif

/*
 * csr_apply_channel_and_power_list() -
 *  HDD calls this function to set the CFG_VALID_CHANNEL_LIST base on the
 * band/mode settings. This function must be called after CFG is downloaded
 * and all the band/mode setting already passed into CSR.

 * Return QDF_STATUS
 */
QDF_STATUS csr_apply_channel_and_power_list(struct mac_context *mac);

/*
 * csr_roam_ndi_stop() - stop ndi
 * @mac: pointer to mac context
 * @vdev_id: vdev ID
 *
 * Return QDF_STATUS
 */
QDF_STATUS csr_roam_ndi_stop(struct mac_context *mac, uint8_t vdev_id);

/* This function is used to stop a BSS. It is similar of csr_roamIssueDisconnect
 * but this function doesn't have any logic other than blindly trying to stop
 * BSS
 */
QDF_STATUS csr_roam_issue_stop_bss_cmd(struct mac_context *mac, uint8_t vdev_id,
				       eCsrRoamBssType bss_type,
				       bool high_priority);

/**
 * csr_roam_issue_disassociate_sta_cmd() - disassociate a associated station
 * @mac:          Pointer to global structure for MAC
 * @sessionId:     Session Id for Soft AP
 * @p_del_sta_params: Pointer to parameters of the station to disassoc
 *
 * CSR function that HDD calls to issue a deauthenticate station command
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_* on error
 */
QDF_STATUS csr_roam_issue_disassociate_sta_cmd(struct mac_context *mac,
					       uint32_t sessionId,
					       struct csr_del_sta_params
					       *p_del_sta_params);
/**
 * csr_roam_issue_deauth_sta_cmd() - issue deauthenticate station command
 * @mac:          Pointer to global structure for MAC
 * @sessionId:     Session Id for Soft AP
 * @pDelStaParams: Pointer to parameters of the station to deauthenticate
 *
 * CSR function that HDD calls to issue a deauthenticate station command
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_** on error
 */
QDF_STATUS csr_roam_issue_deauth_sta_cmd(struct mac_context *mac,
		uint32_t sessionId,
		struct csr_del_sta_params *pDelStaParams);

/*
 * csr_send_chng_mcc_beacon_interval() -
 *   csr function that HDD calls to send Update beacon interval
 *
 * sessionId - session Id for Soft AP
 * Return QDF_STATUS
 */
QDF_STATUS
csr_send_chng_mcc_beacon_interval(struct mac_context *mac, uint32_t sessionId);

#ifdef FEATURE_WLAN_ESE
void csr_update_prev_ap_info(struct csr_roam_session *session,
			     struct wlan_objmgr_vdev *vdev);

#else
static inline void csr_update_prev_ap_info(struct csr_roam_session *session,
					   struct wlan_objmgr_vdev *vdev) {}
#endif

QDF_STATUS csr_update_channel_list(struct mac_context *mac);

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * csr_clear_sae_single_pmk - API to clear single_pmk_info cache
 * @psoc: psoc common object
 * @vdev_id: session id
 * @pmksa: pmk info
 *
 * Return : None
 */
void csr_clear_sae_single_pmk(struct wlan_objmgr_psoc *psoc,
			      uint8_t vdev_id, struct wlan_crypto_pmksa *pmksa);
#else
static inline void
csr_clear_sae_single_pmk(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			 struct wlan_crypto_pmksa *pmksa)
{
}
#endif

QDF_STATUS csr_send_ext_change_freq(struct mac_context *mac_ctx,
				    qdf_freq_t ch_freq, uint8_t session_id);

/**
 * csr_csa_start() - request CSA IE transmission from PE
 * @mac_ctx: handle returned by mac_open
 * @session_id: SAP session id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_csa_restart(struct mac_context *mac_ctx, uint8_t session_id);

/**
 * csr_sta_continue_csa() - Continue for CSA for STA after HW mode change
 * @mac_ctx: handle returned by mac_open
 * @vdev_id: STA VDEV ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_sta_continue_csa(struct mac_context *mac_ctx,
				uint8_t vdev_id);

#ifdef QCA_HT_2040_COEX
QDF_STATUS csr_set_ht2040_mode(struct mac_context *mac, uint32_t sessionId,
			       ePhyChanBondState cbMode, bool obssEnabled);
#endif
QDF_STATUS
csr_roam_prepare_bss_config_from_profile(struct mac_context *mac_ctx,
					 struct csr_roam_profile *profile,
					 uint8_t vdev_id,
					 struct bss_config_param *bss_cfg);

void
csr_roam_prepare_bss_params(struct mac_context *mac_ctx, uint32_t session_id,
			    struct csr_roam_profile *profile,
			    struct bss_config_param *bss_cfg);

/**
 * csr_remove_bssid_from_scan_list() - remove the bssid from
 * scan list
 * @mac_tx: mac context.
 * @bssid: bssid to be removed
 *
 * This function remove the given bssid from scan list.
 *
 * Return: void.
 */
void csr_remove_bssid_from_scan_list(struct mac_context *mac_ctx,
				     tSirMacAddr bssid);

QDF_STATUS
csr_roam_set_bss_config_cfg(struct mac_context *mac_ctx, uint32_t session_id,
			    struct csr_roam_profile *profile,
			    struct bss_config_param *bss_cfg);

void csr_prune_channel_list_for_mode(struct mac_context *mac,
				     struct csr_channel *pChannelList);

/**
 * csr_is_pmkid_found_for_peer() - check if pmkid sent by peer is present
				   in PMK cache. Used in SAP mode.
 * @mac: pointer to mac
 * @session: sme session pointer
 * @peer_mac_addr: mac address of the connecting peer
 * @pmkid: pointer to pmkid(s) send by peer
 * @pmkid_count: number of pmkids sent by peer
 *
 * Return: true if pmkid is found else false
 */
bool csr_is_pmkid_found_for_peer(struct mac_context *mac,
				 struct csr_roam_session *session,
				 tSirMacAddr peer_mac_addr,
				 uint8_t *pmkid, uint16_t pmkid_count);
#ifdef WLAN_FEATURE_11BE

/**
 * csr_update_session_eht_cap() - update sme session eht capabilities
 * @mac_ctx: pointer to mac
 * @session: sme session pointer
 *
 * Return: None
 */
void csr_update_session_eht_cap(struct mac_context *mac_ctx,
				struct csr_roam_session *session);
#else
static inline void csr_update_session_eht_cap(struct mac_context *mac_ctx,
					      struct csr_roam_session *session)
{
}
#endif

#ifdef WLAN_FEATURE_11AX
void csr_update_session_he_cap(struct mac_context *mac_ctx,
			struct csr_roam_session *session);
#else
static inline void csr_update_session_he_cap(struct mac_context *mac_ctx,
			struct csr_roam_session *session)
{
}
#endif

/**
 * csr_setup_vdev_session() - API to setup vdev mac session
 * @vdev_mlme: vdev mlme private object
 *
 * This API setsup the vdev session for the mac layer
 *
 * Returns: QDF_STATUS
 */
QDF_STATUS csr_setup_vdev_session(struct vdev_mlme_obj *vdev_mlme);


#ifdef WLAN_UNIT_TEST
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
/**
 * csr_cm_get_sta_cxn_info() - This function populates all the connection
 *			    information which is formed by DUT-STA to AP
 * @mac_ctx: pointer to mac context
 * @vdev_id: vdev id
 * @buf: pointer to char buffer to write all the connection information.
 * @buf_size: maximum size of the provided buffer
 *
 * Returns: None (information gets populated in buffer)
 */
void csr_cm_get_sta_cxn_info(struct mac_context *mac_ctx, uint8_t vdev_id,
			     char *buf, uint32_t buf_sz);

#endif
#endif

#endif /* CSR_INSIDE_API_H__ */
