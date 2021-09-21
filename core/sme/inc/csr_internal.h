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

/**
 *   \file csr_internal.h
 *
 *   Define internal data structure for MAC.
 */
#ifndef CSRINTERNAL_H__
#define CSRINTERNAL_H__

#include "qdf_status.h"
#include "qdf_lock.h"

#include "qdf_mc_timer.h"
#include "csr_support.h"
#include "cds_reg_service.h"
#include "wlan_scan_public_structs.h"
#include "wlan_cm_roam_api.h"
#include "sir_types.h"
#include "wlan_mlme_public_struct.h"

/* No of sessions to be supported, and a session is for Infra, BT-AMP */
#define CSR_IS_SESSION_VALID(mac, sessionId) \
	((sessionId) < WLAN_MAX_VDEVS && \
	 (mac != NULL) && \
	 ((mac)->roam.roamSession != NULL) && \
	 (mac)->roam.roamSession[(sessionId)].sessionActive)

#define CSR_GET_SESSION(mac, sessionId) \
	(sessionId < WLAN_MAX_VDEVS ? \
	 &(mac)->roam.roamSession[(sessionId)] : NULL)

#define CSR_IS_SESSION_ANY(sessionId) (sessionId == SME_SESSION_ID_ANY)
#define CSR_IS_DFS_CH_ROAM_ALLOWED(mac_ctx) \
	( \
	  ((((mac_ctx)->mlme_cfg->lfr.roaming_dfs_channel) != \
	    ROAMING_DFS_CHANNEL_DISABLED) ? true : false) \
	)
#define CSR_IS_ROAM_PREFER_5GHZ(mac)	\
	( \
	  ((mac)->mlme_cfg->lfr.roam_prefer_5ghz) \
	)

#define CSR_IS_CHANNEL_24GHZ(chnNum) \
	(((chnNum) > 0) && ((chnNum) <= 14))

/* Used to determine what to set to the MLME_DOT11_MODE */
enum csr_cfgdot11mode {
	eCSR_CFG_DOT11_MODE_ABG,
	eCSR_CFG_DOT11_MODE_11A,
	eCSR_CFG_DOT11_MODE_11B,
	eCSR_CFG_DOT11_MODE_11G,
	eCSR_CFG_DOT11_MODE_11N,
	eCSR_CFG_DOT11_MODE_11AC,
	eCSR_CFG_DOT11_MODE_11G_ONLY,
	eCSR_CFG_DOT11_MODE_11N_ONLY,
	eCSR_CFG_DOT11_MODE_11AC_ONLY,
	/* This value can never set to CFG. Its for CSR's internal use */
	eCSR_CFG_DOT11_MODE_AUTO,
	eCSR_CFG_DOT11_MODE_11AX,
	eCSR_CFG_DOT11_MODE_11AX_ONLY,
	eCSR_CFG_DOT11_MODE_11BE,
	eCSR_CFG_DOT11_MODE_11BE_ONLY,
	eCSR_CFG_DOT11_MODE_MAX,
};

enum csr_roam_reason {
	eCsrNoConnection,
	eCsrStartBss,
	eCsrStopBss,
	eCsrForcedDisassocSta,
	eCsrForcedDeauthSta,
};

enum csr_roam_substate {
	eCSR_ROAM_SUBSTATE_NONE = 0,
	eCSR_ROAM_SUBSTATE_START_BSS_REQ,
	eCSR_ROAM_SUBSTATE_DISASSOC_REQ,
	eCSR_ROAM_SUBSTATE_STOP_BSS_REQ,
	eCSR_ROAM_SUBSTATE_CONFIG,
	eCSR_ROAM_SUBSTATE_DEAUTH_REQ,
	eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY,
	/*  max is 15 unless the bitfield is expanded... */
};

enum csr_roam_state {
	eCSR_ROAMING_STATE_STOP = 0,
	eCSR_ROAMING_STATE_IDLE,
	eCSR_ROAMING_STATE_JOINING,
	eCSR_ROAMING_STATE_JOINED,
};

enum csr_join_state {
	eCsrContinueRoaming,
	eCsrStopRoaming,
};

enum csr_roam_wmstatus_changetypes {
	eCsrDisassociated,
	eCsrDeauthenticated
};

struct csr_channel {
	uint8_t numChannels;
	uint32_t channel_freq_list[CFG_VALID_CHANNEL_LIST_LEN];
};

struct bss_config_param {
	tSirMacSSid SSID;
	enum csr_cfgdot11mode uCfgDot11Mode;
	tSirMacCapabilityInfo BssCap;
	ePhyChanBondState cbMode;
};

struct csr_roamstart_bssparams {
	tSirMacSSid ssId;

	/*
	 * This is the BSSID for the party we want to
	 * join (only use for WDS).
	 */
	struct qdf_mac_addr bssid;
	tSirNwType sirNwType;
	ePhyChanBondState cbMode;
	tSirMacRateSet operationalRateSet;
	tSirMacRateSet extendedRateSet;
	uint32_t operation_chan_freq;
	struct ch_params ch_params;
	enum csr_cfgdot11mode uCfgDot11Mode;
	uint8_t privacy;
	bool fwdWPSPBCProbeReq;
	bool protEnabled;
	bool obssProtEnabled;
	tAniAuthType authType;
	uint16_t beaconInterval; /* If this is 0, SME'll fill in for caller */
	uint16_t ht_protection;
	uint32_t dtimPeriod;
	uint8_t ApUapsdEnable;
	uint8_t ssidHidden;
	uint8_t wps_state;
	enum QDF_OPMODE bssPersona;
	uint16_t nRSNIELength;  /* If 0, pRSNIE is ignored. */
	uint8_t *pRSNIE;        /* If not null, it has IE byte stream for RSN */
	/* Flag used to indicate update beaconInterval */
	bool updatebeaconInterval;
	struct add_ie_params add_ie_params;
	uint8_t sap_dot11mc;
	uint16_t beacon_tx_rate;
	uint32_t cac_duration_ms;
	uint32_t dfs_regdomain;
};

struct roam_cmd {
	uint32_t roamId;
	enum csr_roam_reason roamReason;
	struct csr_roam_profile roamProfile;
	bool fReleaseProfile;             /* whether to free roamProfile */
	tSirMacAddr peerMac;
	enum wlan_reason_code reason;
};

struct wmstatus_changecmd {
	enum csr_roam_wmstatus_changetypes Type;
	union {
		struct deauth_ind DeauthIndMsg;
		struct disassoc_ind DisassocIndMsg;
	} u;

};

struct csr_config {
	uint32_t channelBondingMode24GHz;
	uint32_t channelBondingMode5GHz;
	eCsrPhyMode phyMode;
	enum csr_cfgdot11mode uCfgDot11Mode;
	uint32_t HeartbeatThresh50;
	enum wmm_user_mode WMMSupportMode;
	bool Is11eSupportEnabled;
	bool ProprietaryRatesEnabled;
	bool fenableMCCMode;
	bool mcc_rts_cts_prot_enable;
	bool mcc_bcast_prob_resp_enable;
	uint8_t fAllowMCCGODiffBI;
	uint32_t nVhtChannelWidth;
	bool send_smps_action;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif
	bool obssEnabled;
	uint8_t conc_custom_rule1;
	uint8_t conc_custom_rule2;
	uint8_t is_sta_connection_in_5gz_enabled;
	enum force_1x1_type is_force_1x1;
	bool wep_tkip_in_he;
};

struct csr_channel_powerinfo {
	tListElem link;
	uint32_t first_chan_freq;
	uint8_t numChannels;
	uint8_t txPower;
	uint8_t interChannelOffset;
};

struct csr_scanstruct {
	struct csr_channel channels11d;
	struct channel_power defaultPowerTable[CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t numChannelsDefault;
	struct csr_channel base_channels;  /* The channel base to work on */
	tDblLinkList channelPowerInfoList24;
	tDblLinkList channelPowerInfoList5G;
	uint8_t countryCodeDefault[REG_ALPHA2_LEN + 1];
	uint8_t countryCodeCurrent[REG_ALPHA2_LEN + 1];
	/*
	 * Customer wants to optimize the scan time. Avoiding scans(passive)
	 * on DFS channels while swipping through both bands can save some time
	 * (apprx 1.3 sec)
	 */
	uint8_t fEnableDFSChnlScan;
	bool fcc_constraint;
	bool pending_channel_list_req;
};

/*
 * Save the connected information. This structure + connectedProfile
 * should contain all information about the connection
 */
struct csr_roam_connectedinfo {
	uint32_t nBeaconLength;
	uint32_t nAssocReqLength;
	uint32_t nAssocRspLength;
	/* len of the parsed RIC resp IEs received in reassoc response */
	uint32_t nRICRspLength;
#ifdef FEATURE_WLAN_ESE
	uint32_t nTspecIeLength;
#endif
	/*
	 * Point to a buffer contain the beacon, assoc req, assoc rsp frame, in
	 * that order user needs to use nBeaconLength, nAssocReqLength,
	 * nAssocRspLength to desice where each frame starts and ends.
	 */
	uint8_t *pbFrames;
};

/**
 * struct csr_disconnect_stats - Disconnect Stats per session
 * @disconnection_cnt: total no. of disconnections
 * @disconnection_by_app: diconnections triggered by application
 * @disassoc_by_peer: disassoc sent by peer
 * @deauth_by_peer: deauth sent by peer
 * @bmiss: disconnect triggered by beacon miss
 * @peer_kickout: disconnect triggered by peer kickout
 */
struct csr_disconnect_stats {
	uint32_t disconnection_cnt;
	uint32_t disconnection_by_app;
	uint32_t disassoc_by_peer;
	uint32_t deauth_by_peer;
	uint32_t bmiss;
	uint32_t peer_kickout;
};

/**
 * struct csr_roam_session - CSR per-vdev context
 * @vdev_id: ID of the vdev for which this entry is applicable
 * @is_bcn_recv_start: Allow to process bcn recv indication
 * @beacon_report_do_not_resume: Do not resume the beacon reporting after scan
 */
struct csr_roam_session {
	uint8_t vdev_id;
	bool sessionActive;     /* true if it is used */

	eCsrConnectState connectState;
	struct csr_roam_connectedinfo connectedInfo;
	tCsrRoamModifyProfileFields modifyProfileFields;
	/*
	 * to remember some parameters needed for START_BSS.
	 * All member must be set every time we try to join
	 */
	struct csr_roamstart_bssparams bssParams;
#ifdef WLAN_BCN_RECV_FEATURE
	bool is_bcn_recv_start;
	bool beacon_report_do_not_resume;
#endif
#ifdef FEATURE_WLAN_ESE
	bool isPrevApInfoValid;
	uint32_t roamTS1;
#endif
	bool ch_switch_in_progress;
	uint8_t nss;
	bool dhcp_done;
	struct csr_disconnect_stats disconnect_stats;
};

struct csr_roamstruct {
	uint32_t nextRoamId;
	struct csr_config configParam;
	enum csr_roam_state curState[WLAN_MAX_VDEVS];
	enum csr_roam_substate curSubState[WLAN_MAX_VDEVS];
	/*
	 * This may or may not have the up-to-date valid channel list. It is
	 * used to get CFG_VALID_CHANNEL_LIST and not alloc mem all time
	 */
	int32_t sPendingCommands;
	struct csr_roam_session *roamSession;
#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE) && \
	defined(FEATURE_PKTLOG) && !defined(REMOVE_PKT_LOG)
	qdf_mc_timer_t packetdump_timer;
#endif
	spinlock_t roam_state_lock;
};

#define GET_NEXT_ROAM_ID(pRoamStruct)  (((pRoamStruct)->nextRoamId + 1 == 0) ? \
			1 : (pRoamStruct)->nextRoamId)
#define CSR_IS_ROAM_STATE(mac, state, sessionId) \
			((state) == (mac)->roam.curState[sessionId])
#define CSR_IS_ROAM_STOP(mac, sessionId) \
		CSR_IS_ROAM_STATE((mac), eCSR_ROAMING_STATE_STOP, sessionId)
#define CSR_IS_ROAM_INIT(mac, sessionId) \
		 CSR_IS_ROAM_STATE((mac), eCSR_ROAMING_STATE_INIT, sessionId)
#define CSR_IS_ROAM_JOINING(mac, sessionId)  \
		CSR_IS_ROAM_STATE(mac, eCSR_ROAMING_STATE_JOINING, sessionId)
#define CSR_IS_ROAM_IDLE(mac, sessionId) \
		CSR_IS_ROAM_STATE(mac, eCSR_ROAMING_STATE_IDLE, sessionId)
#define CSR_IS_ROAM_JOINED(mac, sessionId) \
		CSR_IS_ROAM_STATE(mac, eCSR_ROAMING_STATE_JOINED, sessionId)
#define CSR_IS_ROAM_SUBSTATE(mac, subState, sessionId) \
		((subState) == (mac)->roam.curSubState[sessionId])
#define CSR_IS_ROAM_SUBSTATE_DISASSOC_REQ(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), eCSR_ROAM_SUBSTATE_DISASSOC_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DEAUTH_REQ(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), \
			eCSR_ROAM_SUBSTATE_DEAUTH_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_START_BSS_REQ(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), \
			eCSR_ROAM_SUBSTATE_START_BSS_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_STOP_BSS_REQ(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), \
			eCSR_ROAM_SUBSTATE_STOP_BSS_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_CONFIG(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), \
		eCSR_ROAM_SUBSTATE_CONFIG, sessionId)
#define CSR_IS_ROAM_SUBSTATE_WAITFORKEY(mac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((mac), \
			eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY, sessionId)

#define CSR_IS_PHY_MODE_DUAL_BAND(phyMode) \
	((eCSR_DOT11_MODE_abg & (phyMode)) || \
	 (eCSR_DOT11_MODE_11n & (phyMode)) || \
	 (eCSR_DOT11_MODE_11ac & (phyMode)) || \
	 (eCSR_DOT11_MODE_11ax & (phyMode)) || \
	 (eCSR_DOT11_MODE_11be & (phyMode)) || \
	 (eCSR_DOT11_MODE_AUTO & (phyMode)))

#define CSR_IS_DOT11_MODE_11N(dot11mode) \
	((dot11mode == eCSR_CFG_DOT11_MODE_AUTO) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11N) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AC) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11N_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AC_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE_ONLY))

#define CSR_IS_DOT11_MODE_11AC(dot11mode) \
	((dot11mode == eCSR_CFG_DOT11_MODE_AUTO) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AC) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AC_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE_ONLY))

#define CSR_IS_DOT11_MODE_11AX(dot11mode) \
	((dot11mode == eCSR_CFG_DOT11_MODE_AUTO) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11AX_ONLY) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE) || \
	 (dot11mode == eCSR_CFG_DOT11_MODE_11BE_ONLY))

#define CSR_IS_DOT11_MODE_11BE(dot11mode) \
	(((dot11mode) == eCSR_CFG_DOT11_MODE_AUTO) || \
	 ((dot11mode) == eCSR_CFG_DOT11_MODE_11BE) || \
	 ((dot11mode) == eCSR_CFG_DOT11_MODE_11BE_ONLY))

#ifdef WLAN_FEATURE_11BE
#define CSR_IS_CFG_DOT11_PHY_MODE_11BE(dot11mode) \
	((dot11mode) == eCSR_CFG_DOT11_MODE_11BE)

#define CSR_IS_CFG_DOT11_PHY_MODE_11BE_ONLY(dot11mode) \
	((dot11mode) == eCSR_CFG_DOT11_MODE_11BE_ONLY)
#else
#define CSR_IS_CFG_DOT11_PHY_MODE_11BE(dot11mode) 0
#define CSR_IS_CFG_DOT11_PHY_MODE_11BE_ONLY(dot11mode) 0
#endif
/*
 * this function returns true if the NIC is operating exclusively in
 * the 2.4 GHz band, meaning. it is NOT operating in the 5.0 GHz band.
 */
#define CSR_IS_24_BAND_ONLY(mac) \
	(BIT(REG_BAND_2G) == (mac)->mlme_cfg->gen.band)

#define CSR_IS_5G_BAND_ONLY(mac) \
	(BIT(REG_BAND_5G) == (mac)->mlme_cfg->gen.band)

#define CSR_IS_RADIO_DUAL_BAND(mac) \
	((BIT(REG_BAND_2G) | BIT(REG_BAND_5G)) == \
		(mac)->mlme_cfg->gen.band_capability)

#define CSR_IS_RADIO_BG_ONLY(mac) \
	(BIT(REG_BAND_2G) == (mac)->mlme_cfg->gen.band_capability)

/*
 * this function returns true if the NIC is operating exclusively in the 5.0 GHz
 * band, meaning. it is NOT operating in the 2.4 GHz band
 */
#define CSR_IS_RADIO_A_ONLY(mac) \
	(BAND_5G == (mac)->mlme_cfg->gen.band_capability)
/* this function returns true if the NIC is operating in both bands. */
#define CSR_IS_OPEARTING_DUAL_BAND(mac) \
	((BAND_ALL == (mac)->mlme_cfg->gen.band_capability) && \
		(BAND_ALL == (mac)->mlme_cfg->gen.band))
/*
 * this function returns true if the NIC can operate in the 5.0 GHz band
 * (could operate in the 2.4 GHz band also)
 */
#define CSR_IS_OPERATING_A_BAND(mac) \
	(CSR_IS_OPEARTING_DUAL_BAND((mac)) || \
		CSR_IS_RADIO_A_ONLY((mac)) || CSR_IS_5G_BAND_ONLY((mac)))

/*
 * this function returns true if the NIC can operate in the 2.4 GHz band
 * (could operate in the 5.0 GHz band also).
 */
#define CSR_IS_OPERATING_BG_BAND(mac) \
	(CSR_IS_OPEARTING_DUAL_BAND((mac)) || \
		CSR_IS_RADIO_BG_ONLY((mac)) || CSR_IS_24_BAND_ONLY((mac)))

#define CSR_IS_ADDTS_WHEN_ACMOFF_SUPPORTED(mac) \
	(mac->mlme_cfg->wmm_params.wmm_tspec_element.ts_acm_is_off)

/**
 * csr_get_vdev_dot11_mode() - get the supported dot11mode by vdev
 * @mac_ctx:  pointer to global mac structure
 * @device_mode: vdev mode
 * @curr_dot11_mode: Current dot11 mode
 *
 * The function return the min of supported dot11 mode and vdev type dot11mode
 * for given vdev type.
 *
 * Return:csr_cfgdot11mode
 */
enum csr_cfgdot11mode
csr_get_vdev_dot11_mode(struct mac_context *mac,
			enum QDF_OPMODE device_mode,
			enum csr_cfgdot11mode curr_dot11_mode);

QDF_STATUS csr_get_channel_and_power_list(struct mac_context *mac);

QDF_STATUS csr_scan_filter_results(struct mac_context *mac);

QDF_STATUS csr_set_modify_profile_fields(struct mac_context *mac,
		uint32_t sessionId, tCsrRoamModifyProfileFields *
		pModifyProfileFields);
QDF_STATUS csr_get_modify_profile_fields(struct mac_context *mac,
		uint32_t sessionId, tCsrRoamModifyProfileFields *
		pModifyProfileFields);
void csr_set_global_cfgs(struct mac_context *mac);
void csr_set_default_dot11_mode(struct mac_context *mac);
bool csr_is_conn_state_disconnected(struct mac_context *mac,
				    uint8_t vdev_id);
bool csr_is_conn_state_connected(struct mac_context *mac,
					       uint32_t sessionId);
bool csr_is_conn_state_wds(struct mac_context *mac, uint32_t sessionId);
bool csr_is_conn_state_connected_wds(struct mac_context *mac,
						    uint32_t sessionId);
bool csr_is_conn_state_disconnected_wds(struct mac_context *mac,
		uint32_t sessionId);
bool csr_is_any_session_in_connect_state(struct mac_context *mac);
bool csr_is_all_session_disconnected(struct mac_context *mac);

bool csr_is_infra_ap_started(struct mac_context *mac);
bool csr_is_conn_state_connected_infra_ap(struct mac_context *mac,
		uint32_t sessionId);
QDF_STATUS csr_get_snr(struct mac_context *mac, tCsrSnrCallback callback,
			  struct qdf_mac_addr bssId, void *pContext);
QDF_STATUS csr_get_config_param(struct mac_context *mac,
					  struct csr_config_params *pParam);
QDF_STATUS csr_change_default_config_param(struct mac_context *mac,
		struct csr_config_params *pParam);
QDF_STATUS csr_msg_processor(struct mac_context *mac, void *msg_buf);
QDF_STATUS csr_open(struct mac_context *mac);
QDF_STATUS csr_init_chan_list(struct mac_context *mac, uint8_t *alpha2);
QDF_STATUS csr_close(struct mac_context *mac);
QDF_STATUS csr_start(struct mac_context *mac);
QDF_STATUS csr_stop(struct mac_context *mac);
QDF_STATUS csr_ready(struct mac_context *mac);

/**
 * csr_get_concurrent_operation_freq() - To get concurrent operating freq
 * @mac_ctx: Pointer to mac context
 *
 * This routine will return operating freq on FIRST BSS that is
 * active/operating to be used for concurrency mode.
 * If other BSS is not up or not connected it will return 0
 *
 * Return: uint32_t
 */
uint32_t csr_get_concurrent_operation_freq(struct mac_context *mac_ctx);

/**
 * csr_get_beaconing_concurrent_channel() - To get concurrent operating channel
 * frequency of beaconing interface
 * @mac_ctx: Pointer to mac context
 * @vdev_id_to_skip: channel of which vdev id to skip
 *
 * This routine will return operating channel of active AP/GO channel
 * and will skip the channel of vdev_id_to_skip.
 * If other no reqested mode is active it will return 0
 *
 * Return: uint32_t
 */
uint32_t csr_get_beaconing_concurrent_channel(struct mac_context *mac_ctx,
					      uint8_t vdev_id_to_skip);

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t csr_check_concurrent_channel_overlap(
		struct mac_context *mac,
		uint32_t sap_ch_freq, eCsrPhyMode sap_phymode,
		uint8_t cc_switch_mode);
#endif

/* Returns whether the current association is a 11r assoc or not */
bool csr_roam_is11r_assoc(struct mac_context *mac, uint8_t sessionId);

#ifdef FEATURE_WLAN_ESE
/* Returns whether the current association is a ESE assoc or not */
bool csr_roam_is_ese_assoc(struct mac_context *mac, uint32_t sessionId);
bool csr_roam_is_ese_ini_feature_enabled(struct mac_context *mac);
QDF_STATUS csr_get_tsm_stats(struct mac_context *mac,
		tCsrTsmStatsCallback callback,
		struct qdf_mac_addr bssId,
		void *pContext, uint8_t tid);
#endif

/**
 * csr_roam_channel_change_req() - Post channel change request to LIM
 * @mac: mac context
 * @bssid: SAP bssid
 * @vdev_id: vdev_id
 * @ch_params: channel information
 * @profile: CSR profile
 *
 * This API is primarily used to post Channel Change Req for SAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_roam_channel_change_req(struct mac_context *mac,
				       struct qdf_mac_addr bssid,
				       uint8_t vdev_id,
				       struct ch_params *ch_params,
				       struct csr_roam_profile *profile);

/* Post Beacon Tx Start Indication */
QDF_STATUS csr_roam_start_beacon_req(struct mac_context *mac,
		struct qdf_mac_addr bssid, uint8_t dfsCacWaitStatus);

QDF_STATUS csr_roam_send_chan_sw_ie_request(struct mac_context *mac,
					    struct qdf_mac_addr bssid,
					    uint32_t target_chan_freq,
					    uint8_t csaIeReqd,
					    struct ch_params *ch_params);
QDF_STATUS csr_roam_modify_add_ies(struct mac_context *mac,
					tSirModifyIE *pModifyIE,
				   eUpdateIEsType updateType);
QDF_STATUS
csr_roam_update_add_ies(struct mac_context *mac,
		tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType);

bool csr_nonscan_active_ll_remove_entry(
			struct mac_context *mac_ctx,
			tListElem *pEntryToRemove, bool inter_locked);
tListElem *csr_nonscan_active_ll_peek_head(
			struct mac_context *mac_ctx,
			bool inter_locked);
tListElem *csr_nonscan_pending_ll_peek_head(
			struct mac_context *mac_ctx,
			bool inter_locked);
tListElem *csr_nonscan_pending_ll_next(
			struct mac_context *mac_ctx,
		tListElem *entry, bool inter_locked);

/**
 * csr_purge_pdev_all_ser_cmd_list() - purge all scan and non-scan
 * active and pending cmds for all vdevs in pdev
 * @mac_ctx: pointer to global MAC context
 *
 * Return : none
 */
void csr_purge_pdev_all_ser_cmd_list(struct mac_context *mac_ctx);

void csr_roam_substate_change(
			struct mac_context *mac, enum csr_roam_substate
					NewSubstate, uint32_t sessionId);

bool csr_is_ndi_started(struct mac_context *mac_ctx, uint32_t session_id);

QDF_STATUS csr_roam_update_config(
			struct mac_context *mac_ctx, uint8_t session_id,
				  uint16_t capab, uint32_t value);

/**
 * csr_is_mcc_channel() - check if using the channel results into MCC
 * @mac_ctx: pointer to global MAC context
 * @chan_freq: channel frequency to check for MCC scenario
 *
 * Return : true if channel causes MCC, else false
 */
bool csr_is_mcc_channel(struct mac_context *mac_ctx, uint32_t chan_freq);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * csr_roam_auth_offload_callback() - Registered CSR Callback function to handle
 * WPA3 roam pre-auth event from firmware.
 * @mac_ctx: Global mac context pointer
 * @vdev_id: Vdev id
 * @bssid: candidate AP bssid
 */
QDF_STATUS
csr_roam_auth_offload_callback(struct mac_context *mac_ctx,
			       uint8_t vdev_id,
			       struct qdf_mac_addr bssid);
#else
static inline QDF_STATUS
csr_roam_auth_offload_callback(struct mac_context *mac_ctx,
			       uint8_t vdev_id,
			       struct qdf_mac_addr bssid)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * csr_invoke_neighbor_report_request - Send neighbor report invoke command to
 *					WMA
 * @mac_ctx: MAC context
 * @session_id: session id
 *
 * API called from IW to invoke neighbor report request to WMA then to FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_invoke_neighbor_report_request(uint8_t session_id,
				struct sRrmNeighborReq *neighbor_report_req,
				bool send_resp_to_host);
#endif
