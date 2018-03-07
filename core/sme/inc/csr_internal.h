/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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

#include "csr_neighbor_roam.h"

#include "sir_types.h"

/* define scan return criteria. LIM should use these define as well */
#define CSR_SCAN_RETURN_AFTER_ALL_CHANNELS          (0)
#define CSR_SCAN_RETURN_AFTER_FIRST_MATCH           (0x01)
#define CSR_NUM_RSSI_CAT        15
#define CSR_ROAM_SCAN_CHANNEL_SWITCH_TIME        3

/* session ID invalid */
#define CSR_SESSION_ID_INVALID    0xFF
/* No of sessions to be supported, and a session is for Infra, IBSS or BT-AMP */
#define CSR_ROAM_SESSION_MAX      SIR_MAX_SUPPORTED_BSS
#define CSR_IS_SESSION_VALID(pMac, sessionId) \
	(((sessionId) < CSR_ROAM_SESSION_MAX) && \
	 ((pMac)->roam.roamSession[(sessionId)].sessionActive))

#define CSR_GET_SESSION(pMac, sessionId) \
	( \
	  (sessionId < CSR_ROAM_SESSION_MAX) ? \
	  (&(pMac)->roam.roamSession[(sessionId)]) : NULL \
	)

#define CSR_IS_SESSION_ANY(sessionId) (sessionId == SME_SESSION_ID_ANY)
#define CSR_MAX_NUM_COUNTRY_CODE  100
#define CSR_IS_DFS_CH_ROAM_ALLOWED(mac_ctx) \
	( \
	  (((mac_ctx)->roam.configParam.allowDFSChannelRoam) ? true : false) \
	)
#define CSR_IS_SELECT_5GHZ_MARGIN(pMac) \
	( \
	  (((pMac)->roam.configParam.nSelect5GHzMargin) ? true : false) \
	)
#define CSR_IS_SELECT_5G_PREFERRED(pMac) \
	( \
	  (((pMac)->roam.configParam.roam_params.is_5g_pref_enabled) ? \
	   true : false) \
	)
#define CSR_IS_ROAM_PREFER_5GHZ(pMac)	\
	( \
	  (((pMac)->roam.configParam.nRoamPrefer5GHz) ? true : false) \
	)
#define CSR_IS_ROAM_INTRA_BAND_ENABLED(pMac) \
	( \
	  (((pMac)->roam.configParam.nRoamIntraBand) ? true : false) \
	)
#define CSR_IS_FASTROAM_IN_CONCURRENCY_INI_FEATURE_ENABLED(pMac) \
	( \
	  (((pMac)->roam.configParam.bFastRoamInConIniFeatureEnabled) ? \
		true : false) \
	)
#define CSR_IS_CHANNEL_24GHZ(chnNum) \
	(((chnNum) > 0) && ((chnNum) <= 14))
/* Support for "Fast roaming" (i.e., ESE, LFR, or 802.11r.) */
#define CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN 15

/* Used to determine what to set to the WNI_CFG_DOT11_MODE */
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
};

enum csr_scan_reason {
	eCsrScanForSsid,
};

enum csr_roam_reason {
	/* Roaming because we've not established the initial connection. */
	eCsrNoConnection,
	/* roaming because LIM reported a cap change in the associated AP. */
	eCsrCapsChange,
	/* roaming because someone asked us to Disassoc & stay disassociated. */
	eCsrForcedDisassoc,
	/* roaming because an 802.11 request was issued to the driver. */
	eCsrHddIssued,
	/* roaming because we lost link to an associated AP */
	eCsrLostLink1,
	eCsrLostLink2,
	eCsrLostLink3,
	/* roaming because we need to force a Disassoc due to MIC failure */
	eCsrForcedDisassocMICFailure,
	eCsrHddIssuedReassocToSameAP,
	eCsrSmeIssuedReassocToSameAP,
	eCsrSmeIssuedReassocToDiffAP,
	/* roaming becuase someone asked us to deauth and stay disassociated. */
	eCsrForcedDeauth,
	/* will be issued by Handoff logic to disconect from current AP */
	eCsrSmeIssuedDisassocForHandoff,
	/* will be issued by Handoff logic to join a new AP with same profile */
	eCsrSmeIssuedAssocToSimilarAP,
	/* ibss jointimer fired before any peer showedup, so shutdown network */
	eCsrSmeIssuedIbssJoinFailure,
	eCsrForcedIbssLeave,
	eCsrStopBss,
	eCsrSmeIssuedFTReassoc,
	eCsrForcedDisassocSta,
	eCsrForcedDeauthSta,
	eCsrPerformPreauth,
};

enum csr_roam_substate {
	eCSR_ROAM_SUBSTATE_NONE = 0,
	eCSR_ROAM_SUBSTATE_START_BSS_REQ,
	eCSR_ROAM_SUBSTATE_JOIN_REQ,
	eCSR_ROAM_SUBSTATE_REASSOC_REQ,
	eCSR_ROAM_SUBSTATE_DISASSOC_REQ,
	eCSR_ROAM_SUBSTATE_STOP_BSS_REQ,
	/* Continue the current roam command after disconnect */
	eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING,
	eCSR_ROAM_SUBSTATE_AUTH_REQ,
	eCSR_ROAM_SUBSTATE_CONFIG,
	eCSR_ROAM_SUBSTATE_DEAUTH_REQ,
	eCSR_ROAM_SUBSTATE_DISASSOC_NOTHING_TO_JOIN,
	eCSR_ROAM_SUBSTATE_DISASSOC_REASSOC_FAILURE,
	eCSR_ROAM_SUBSTATE_DISASSOC_FORCED,
	eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY,
	eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF,
	eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC,
	eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC,
	eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC,
	eCSR_ROAM_SUBSTATE_DISASSOC_STA_HAS_LEFT,
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
	eCsrStartIbss,
	eCsrStartIbssSameIbss,
	eCsrReassocToSelfNoCapChange,
	eCsrStopRoamingDueToConcurrency,

};

enum csr_roaming_reason {
	eCsrNotRoaming,
	eCsrLostlinkRoamingDisassoc,
	eCsrLostlinkRoamingDeauth,
	eCsrDynamicRoaming,
	eCsrReassocRoaming,
};

enum csr_roam_wmstatus_changetypes {
	eCsrDisassociated,
	eCsrDeauthenticated
};

enum csr_roam_stats_classtypes {
	eCsrSummaryStats = 0,
	eCsrGlobalClassAStats,
	eCsrGlobalClassDStats,
	csr_per_chain_rssi_stats,
	eCsrMaxStats
};

#ifdef FEATURE_WLAN_DIAG_SUPPORT
enum csr_diagwlan_status_eventsubtype {
	eCSR_WLAN_STATUS_CONNECT = 0,
	eCSR_WLAN_STATUS_DISCONNECT
};

enum csr_diagwlan_status_eventreason {
	eCSR_REASON_UNSPECIFIED = 0,
	eCSR_REASON_USER_REQUESTED,
	eCSR_REASON_MIC_ERROR,
	eCSR_REASON_DISASSOC,
	eCSR_REASON_DEAUTH,
	eCSR_REASON_HANDOFF,
	eCSR_REASON_ROAM_SYNCH_IND,
	eCSR_REASON_ROAM_SYNCH_CNF,
	eCSR_REASON_ROAM_HO_FAIL,

};

#endif /* FEATURE_WLAN_DIAG_SUPPORT */

struct csr_channel {
	uint8_t numChannels;
	uint8_t channelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
};

struct scan_profile       {
	uint32_t minChnTime;
	uint32_t maxChnTime;
	/* In units of milliseconds, ignored when not connected */
	uint32_t restTime;
	/* In units of milliseconds, ignored when not connected */
	uint32_t min_rest_time;
	/* In units of milliseconds, ignored when not connected */
	uint32_t idle_time;
	uint32_t numOfChannels;
	uint8_t *pChannelList;
	tSirScanType scanType;
	eCsrRoamBssType bssType;
	uint8_t ssid[WNI_CFG_SSID_LEN];
	uint8_t bReturnAfter1stMatch;
	uint8_t fUniqueResult;
	uint8_t freshScan;
	struct qdf_mac_addr bssid;
};

struct bss_config_param {
	eCsrMediaAccessType qosType;
	tSirMacSSid SSID;
	uint32_t uRTSThresh;
	uint32_t uDeferThresh;
	enum csr_cfgdot11mode uCfgDot11Mode;
	enum band_info eBand;
	uint8_t standardRate[CSR_DOT11_SUPPORTED_RATES_MAX];
	uint8_t extendedRate[CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX];
	eCsrExposedTxRate txRate;
	tAniAuthType authType;
	eCsrEncryptionType encType;
	uint32_t uShortSlotTime;
	uint32_t uHTSupport;
	uint32_t uPowerLimit;
	uint32_t uHeartBeatThresh;
	uint32_t uJoinTimeOut;
	tSirMacCapabilityInfo BssCap;
	bool f11hSupport;
	ePhyChanBondState cbMode;
};

struct csr_roamstart_bssparams {
	tSirMacSSid ssId;

	/*
	 * This is the BSSID for the party we want to
	 * join (only use for IBSS or WDS).
	 */
	struct qdf_mac_addr bssid;
	tSirNwType sirNwType;
	ePhyChanBondState cbMode;
	tSirMacRateSet operationalRateSet;
	tSirMacRateSet extendedRateSet;
	uint8_t operationChn;
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
#ifdef WLAN_FEATURE_11W
	bool mfpCapable;
	bool mfpRequired;
#endif
	tSirAddIeParams addIeParams;
	uint8_t sap_dot11mc;
	uint16_t beacon_tx_rate;
	uint32_t cac_duration_ms;
	uint32_t dfs_regdomain;
};

struct roam_cmd {
	uint32_t roamId;
	enum csr_roam_reason roamReason;
	tCsrRoamProfile roamProfile;
	tScanResultHandle hBSSList;       /* BSS list fits the profile */
	/*
	 * point to the current BSS in the list that is roaming.
	 * It starts from head to tail
	 * */
	tListElem *pRoamBssEntry;
	tSirBssDescription *pLastRoamBss; /* the last BSS we try and failed */
	bool fReleaseBssList;             /* whether to free hBSSList */
	bool fReleaseProfile;             /* whether to free roamProfile */
	bool fReassoc;                    /* whether this cmd is for reassoc */
	/* whether pMac->roam.pCurRoamProfile needs to be updated */
	bool fUpdateCurRoamProfile;
	/*
	 * this is for CSR internal used only. And it should not be assigned
	 * when creating the command. This causes the roam cmd not todo anything
	 */
	bool fReassocToSelfNoCapChange;

	bool fStopWds;
	tSirMacAddr peerMac;
	tSirMacReasonCodes reason;
	eCsrRoamDisconnectReason disconnect_reason;
};

struct setkey_cmd {
	uint32_t roamId;
	eCsrEncryptionType encType;
	eCsrAuthType authType;
	tAniKeyDirection keyDirection;  /* Tx, Rx or Tx-and-Rx */
	struct qdf_mac_addr peermac;    /* Peer's MAC address. ALL 1's for group key */
	uint8_t paeRole;        /* 0 for supplicant */
	uint8_t keyId;          /* Kye index */
	uint8_t keyLength;      /* Number of bytes containing the key in pKey */
	uint8_t Key[CSR_MAX_KEY_LEN];
	uint8_t keyRsc[CSR_MAX_RSC_LEN];
};

struct wmstatus_changecmd {
	enum csr_roam_wmstatus_changetypes Type;
	union {
		tSirSmeDeauthInd DeauthIndMsg;
		tSirSmeDisassocInd DisassocIndMsg;
	} u;

};

struct addstafor_sessioncmd {
	/* Session self mac addr */
	tSirMacAddr selfMacAddr;
	enum QDF_OPMODE currDeviceMode;
	uint32_t type;
	uint32_t subType;
	uint8_t sessionId;
};

struct delstafor_sessionCmd {
	/* Session self mac addr */
	tSirMacAddr selfMacAddr;
};

struct csr_11rconfig {
	bool IsFTResourceReqSupported;
};

struct csr_neighbor_roamconfig {
	uint32_t nNeighborScanTimerPeriod;
	uint32_t neighbor_scan_min_timer_period;
	uint8_t nNeighborLookupRssiThreshold;
	int8_t rssi_thresh_offset_5g;
	uint16_t nNeighborScanMinChanTime;
	uint16_t nNeighborScanMaxChanTime;
	sCsrChannel neighborScanChanList;
	uint8_t nMaxNeighborRetries;
	uint16_t nNeighborResultsRefreshPeriod;
	uint16_t nEmptyScanRefreshPeriod;
	uint8_t nOpportunisticThresholdDiff;
	uint8_t nRoamRescanRssiDiff;
	uint8_t nRoamBmissFirstBcnt;
	uint8_t nRoamBmissFinalBcnt;
	uint8_t nRoamBeaconRssiWeight;
	uint8_t delay_before_vdev_stop;
	uint32_t nhi_rssi_scan_max_count;
	uint32_t nhi_rssi_scan_rssi_delta;
	uint32_t nhi_rssi_scan_delay;
	int32_t nhi_rssi_scan_rssi_ub;
};

/*
 * Neighbor Report Params Bitmask
 */
#define NEIGHBOR_REPORT_PARAMS_TIME_OFFSET            0x01
#define NEIGHBOR_REPORT_PARAMS_LOW_RSSI_OFFSET        0x02
#define NEIGHBOR_REPORT_PARAMS_BMISS_COUNT_TRIGGER    0x04
#define NEIGHBOR_REPORT_PARAMS_PER_THRESHOLD_OFFSET   0x08
#define NEIGHBOR_REPORT_PARAMS_CACHE_TIMEOUT          0x10
#define NEIGHBOR_REPORT_PARAMS_MAX_REQ_CAP            0x20
#define NEIGHBOR_REPORT_PARAMS_ALL                    0x3F

struct csr_config {
	uint32_t agingCount;
	uint32_t FragmentationThreshold;
	uint32_t channelBondingMode24GHz;
	uint32_t channelBondingMode5GHz;
	uint32_t RTSThreshold;
	eCsrPhyMode phyMode;
	enum csr_cfgdot11mode uCfgDot11Mode;
	enum band_info eBand;
	uint32_t HeartbeatThresh50;
	uint32_t HeartbeatThresh24;
	eCsrCBChoice cbChoice;
	enum band_info bandCapability;        /* indicate hw capability */
	eCsrRoamWmmUserModeType WMMSupportMode;
	bool Is11eSupportEnabled;
	bool Is11dSupportEnabled;
	bool Is11dSupportEnabledOriginal;
	bool Is11hSupportEnabled;
	bool shortSlotTime;
	bool ProprietaryRatesEnabled;
	bool fenableMCCMode;
	bool mcc_rts_cts_prot_enable;
	bool mcc_bcast_prob_resp_enable;
	uint8_t fAllowMCCGODiffBI;
	uint8_t AdHocChannel24;
	uint8_t AdHocChannel5G;
	/* each RSSI category has one value */
	uint32_t BssPreferValue[CSR_NUM_RSSI_CAT];
	int RSSICat[CSR_NUM_RSSI_CAT];
	uint8_t bCatRssiOffset; /* to set RSSI difference for each category */
	/*
	 * Whether to limit the channels to the ones set in Csr11dInfo.
	 * If true, the opertaional channels are limited to the default channel
	 * list. It is an "AND" operation between the default channels and
	 * the channels in the 802.11d IE.
	 */
	/* Country Code Priority */
	bool fSupplicantCountryCodeHasPriority;

	uint16_t vccRssiThreshold;
	uint32_t vccUlMacLossThreshold;

	uint32_t nPassiveMinChnTime;    /* in units of milliseconds */
	uint32_t nPassiveMaxChnTime;    /* in units of milliseconds */
	uint32_t nActiveMinChnTime;     /* in units of milliseconds */
	uint32_t nActiveMaxChnTime;     /* in units of milliseconds */

	uint32_t nInitialDwellTime;     /* in units of milliseconds */
	bool initial_scan_no_dfs_chnl;
#ifdef WLAN_AP_STA_CONCURRENCY
	uint32_t nPassiveMinChnTimeConc;/* in units of milliseconds */
	uint32_t nPassiveMaxChnTimeConc;/* in units of milliseconds */
	uint32_t nActiveMinChnTimeConc; /* in units of milliseconds */
	uint32_t nActiveMaxChnTimeConc; /* in units of milliseconds */
	uint32_t nRestTimeConc;         /* in units of milliseconds */
	/* In units of milliseconds */
	uint32_t  min_rest_time_conc;
	/* In units of milliseconds */
	uint32_t  idle_time_conc;

#endif
	/*
	 * in dBm, the max TX power. The actual TX power is the lesser of this
	 * value & 11d. If 11d is disable, the lesser of this & default setting.
	 */
	uint8_t nTxPowerCap;
	bool allow_tpc_from_ap;
	uint32_t statsReqPeriodicity;    /* stats req freq while in fullpower */
	uint32_t statsReqPeriodicityInPS;/* stats req freq while in powersave */
	uint32_t dtimPeriod;
	bool ssidHidden;
	struct csr_11rconfig csr11rConfig;
	uint8_t isFastRoamIniFeatureEnabled;
	struct mawc_params csr_mawc_config;
	uint8_t isRoamOffloadScanEnabled;
	bool bFastRoamInConIniFeatureEnabled;
#ifdef FEATURE_WLAN_ESE
	uint8_t isEseIniFeatureEnabled;
#endif
	uint8_t isFastTransitionEnabled;
	uint8_t RoamRssiDiff;
	int32_t rssi_abs_thresh;
	bool nRoamPrefer5GHz;
	bool nRoamIntraBand;
	bool isWESModeEnabled;
	bool nRoamScanControl;
	uint8_t nProbes;
	uint16_t nRoamScanHomeAwayTime;

	struct csr_neighbor_roamconfig neighborRoamConfig;

	/*
	 * Instead of Reassoc, send ADDTS/DELTS even when ACM is off for
	 * that AC This is mandated by WMM-AC certification
	 */
	bool addTSWhenACMIsOff;
	bool fValidateList;
	/*
	 * Remove this code once SLM_Sessionization is supported
	 * BMPS_WORKAROUND_NOT_NEEDED
	 */
	bool doBMPSWorkaround;
	/* To enable scanning 2g channels twice on single scan req from HDD */
	bool fScanTwice;
	uint32_t nVhtChannelWidth;
	bool enable_subfee_vendor_vhtie;
	uint8_t enable_txbf_sap_mode;
	uint8_t enable2x2;
	bool enableVhtFor24GHz;
	uint8_t enableVhtpAid;
	uint8_t enableVhtGid;
	uint8_t enableAmpduPs;
	uint8_t enableHtSmps;
	uint8_t htSmps;
	bool send_smps_action;
	uint8_t tx_ldpc_enable;
	uint8_t rx_ldpc_enable;
	uint8_t disable_high_ht_mcs_2x2;
	/*
	 * Enable/Disable heartbeat offload
	 */
	bool enableHeartBeatOffload;
	uint8_t max_amsdu_num;
	uint8_t nSelect5GHzMargin;
	uint8_t isCoalesingInIBSSAllowed;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif
	uint8_t allowDFSChannelRoam;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	bool isRoamOffloadEnabled;
#endif
	bool obssEnabled;
	bool ignore_peer_erp_info;
	uint8_t conc_custom_rule1;
	uint8_t conc_custom_rule2;
	uint8_t is_sta_connection_in_5gz_enabled;
	struct roam_ext_params roam_params;
	bool send_deauth_before_con;
	bool early_stop_scan_enable;
	int8_t early_stop_scan_min_threshold;
	int8_t early_stop_scan_max_threshold;
	uint32_t obss_width_interval;
	uint32_t obss_active_dwelltime;
	uint32_t obss_passive_dwelltime;
	bool ignore_peer_ht_opmode;
	bool enable_edca_params;
	uint32_t edca_vo_cwmin;
	uint32_t edca_vi_cwmin;
	uint32_t edca_bk_cwmin;
	uint32_t edca_be_cwmin;
	uint32_t edca_vo_cwmax;
	uint32_t edca_vi_cwmax;
	uint32_t edca_bk_cwmax;
	uint32_t edca_be_cwmax;
	uint32_t edca_vo_aifs;
	uint32_t edca_vi_aifs;
	uint32_t edca_bk_aifs;
	uint32_t edca_be_aifs;
	bool enable_fatal_event;
	bool vendor_vht_sap;
	enum wmi_dwelltime_adaptive_mode scan_adaptive_dwell_mode;
	enum wmi_dwelltime_adaptive_mode scan_adaptive_dwell_mode_nc;
	enum wmi_dwelltime_adaptive_mode roamscan_adaptive_dwell_mode;
	struct csr_sta_roam_policy_params sta_roam_policy;
	uint32_t tx_aggregation_size;
	uint32_t rx_aggregation_size;
	struct wmi_per_roam_config per_roam_config;
	bool enable_bcast_probe_rsp;
	bool is_fils_enabled;
#ifdef WLAN_FEATURE_11AX
	bool enable_ul_ofdma;
	bool enable_ul_mimo;
#endif
	bool qcn_ie_support;
	uint8_t fils_max_chan_guard_time;
	uint16_t pkt_err_disconn_th;
	bool is_force_1x1;
	uint16_t num_11b_tx_chains;
	uint16_t num_11ag_tx_chains;
	uint32_t disallow_duration;
	uint32_t rssi_channel_penalization;
	uint32_t num_disallowed_aps;
	uint16_t wlm_latency_enable;
	uint16_t wlm_latency_level;
	uint32_t wlm_latency_flags[CSR_NUM_WLM_LATENCY_LEVEL];
	struct sir_score_config bss_score_params;
	uint8_t oce_feature_bitmap;
	struct csr_mbo_thresholds mbo_thresholds;
	uint32_t btm_offload_config;
	uint32_t btm_solicited_timeout;
	uint32_t btm_max_attempt_cnt;
	uint32_t btm_sticky_time;
	uint32_t offload_11k_enable_bitmask;
	bool wep_tkip_in_he;
	struct csr_neighbor_report_offload_params neighbor_report_offload;
};

struct csr_channel_powerinfo {
	tListElem link;
	uint8_t firstChannel;
	uint8_t numChannels;
	uint8_t txPower;
	uint8_t interChannelOffset;
};

struct csr_roam_joinstatus {
	tSirResultCodes statusCode;
	/*
	 * this is set to unspecified if statusCode indicates timeout.
	 * Or it is the failed reason from the other BSS(per 802.11 spec)
	 */
	uint32_t reasonCode;
	tSirMacAddr bssId;
};

struct csr_oschannel_mask {
	uint8_t numChannels;
	bool scanEnabled[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t channelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
};

struct csr_votes11d {
	uint8_t votes;
	uint8_t countryCode[WNI_CFG_COUNTRY_CODE_LEN];
};

struct csr_scanstruct {
	struct scan_profile scanProfile;
	uint8_t scanResultCfgAgingTime;
	tSirScanType curScanType;
	struct csr_channel channels11d;
	struct channel_power defaultPowerTable[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t numChannelsDefault;
	struct csr_channel base_channels;  /* The channel base to work on */
	tDblLinkList channelPowerInfoList24;
	tDblLinkList channelPowerInfoList5G;
	uint32_t nLastAgeTimeOut;
	uint32_t nAgingCountDown;
	uint8_t countryCodeDefault[WNI_CFG_COUNTRY_CODE_LEN];
	uint8_t countryCodeCurrent[WNI_CFG_COUNTRY_CODE_LEN];
	uint8_t countryCode11d[WNI_CFG_COUNTRY_CODE_LEN];
	v_REGDOMAIN_t domainIdDefault;  /* default regulatory domain */
	v_REGDOMAIN_t domainIdCurrent;  /* current regulatory domain */

	/* Bssid for current country code */
	struct qdf_mac_addr currentCountryBssid;

	int8_t currentCountryRSSI;      /* RSSI for current country code */
	bool fCancelIdleScan;
	uint8_t countryCodeCount;
	/* counts for various advertized country codes */
	struct csr_votes11d votes11d[CSR_MAX_NUM_COUNTRY_CODE];
	/*
	 * in 11d IE from probe rsp or beacons of neighboring APs
	 * will use the most popular one (max count)
	 */
	uint8_t countryCodeElected[WNI_CFG_COUNTRY_CODE_LEN];
	bool fRestartIdleScan;
	uint32_t nIdleScanTimeGap;
	/* keep a track of channels to be scnned while in traffic condition */
	struct csr_oschannel_mask osScanChannelMask;
	uint16_t nBssLimit;     /* the maximum number of BSS in scan cache */
	/*
	 * channelPowerInfoList24 has been seen corrupted. Set this flag to true
	 * trying to detect when it happens. Adding this into code because we
	 * can't reproduce it easily. We don't know when it happens.
	 */
	bool fValidateList;
	/*
	 * Customer wants to start with an active scan based on the default
	 * country code. This optimization will minimize the driver load to
	 * association time. Based on this flag we will bypass the initial
	 * passive scan needed for 11d to determine the country code & domain
	 */
	bool fEnableBypass11d;
	/*
	 * Customer wants to optimize the scan time. Avoiding scans(passive)
	 * on DFS channels while swipping through both bands can save some time
	 * (apprx 1.3 sec)
	 */
	uint8_t fEnableDFSChnlScan;
	/*
	 * To enable/disable scanning only 2.4Ghz channels on first scan
	 */
	bool fFirstScanOnly2GChnl;
	bool fDropScanCmd;      /* true means we don't accept scan commands */

#ifdef WLAN_AP_STA_CONCURRENCY
	tDblLinkList scanCmdPendingList;
#endif
	/* This includes all channels on which candidate APs are found */
	struct csr_channel occupiedChannels[CSR_ROAM_SESSION_MAX];
	int8_t roam_candidate_count[CSR_ROAM_SESSION_MAX];
	int8_t inScanResultBestAPRssi;
	csr_scan_completeCallback callback11dScanDone;
	bool fcc_constraint;
	uint8_t max_scan_count;
	bool defer_update_channel_list;
	wlan_scan_requester requester_id;
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
	uint8_t staId;
};

struct csr_linkquality_indinfo {
	csr_roamLinkQualityIndCallback callback;
	void *context;
};

struct csr_pestats_reqinfo {
	tListElem link;         /* list links */
	uint32_t statsMask;
	bool rspPending;
	uint8_t staId;
	uint8_t numClient;
	tpAniSirGlobal pMac;
	uint8_t sessionId;
};

struct csr_statsclient_reqinfo {
	tListElem link;         /* list links */
	eCsrStatsRequesterType requesterId;
	tCsrStatsCallback callback;
	void *pContext;
	uint32_t statsMask;
	struct csr_pestats_reqinfo *pPeStaEntry;
	uint8_t staId;
	qdf_mc_timer_t timer;
	bool timerExpired;
	tpAniSirGlobal pMac;    /* TODO: Confirm this change BTAMP */
	uint8_t sessionId;
};

struct csr_tlstats_reqinfo {
	uint8_t numClient;
};

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
enum csr_roamoffload_authstatus {
	/* reassociation is done but couldn't finish security handshake */
	eSIR_ROAM_AUTH_STATUS_CONNECTED = 1,
	/* roam successfully completed by firmware */
	eSIR_ROAM_AUTH_STATUS_AUTHENTICATED = 2,
	/* unknown error */
	eSIR_ROAM_AUTH_STATUS_UNKNOWN = 0xff
};
struct csr_roam_offload_synch_params {
	uint8_t roamedVdevId;   /* vdevId after roaming */
	int8_t txMgmtPower;     /* HAL fills in the tx power used for */
	uint8_t rssi;           /* RSSI */
	uint8_t roamReason;     /* Roam reason */
	uint8_t nss;            /* no of spatial streams */
	uint16_t chainMask;     /* chainmask */
	uint16_t smpsMode;      /* smps.mode */
	struct qdf_mac_addr bssid;      /* MAC address of roamed AP */
	enum csr_roamoffload_authstatus authStatus;   /* auth status */
	uint8_t kck[SIR_KCK_KEY_LEN];
	uint8_t kek[SIR_KEK_KEY_LEN_FILS];
	uint32_t kek_len;
	uint32_t pmk_len;
	uint8_t pmk[SIR_PMK_LEN];
	uint8_t pmkid[SIR_PMKID_LEN];
	bool update_erp_next_seq_num;
	uint16_t next_erp_seq_num;
	uint8_t replay_ctr[SIR_REPLAY_CTR_LEN];
	tpSirBssDescription  bss_desc_ptr;      /*BSS descriptor*/
};
#endif

struct csr_roam_stored_profile {
	uint32_t session_id;
	tCsrRoamProfile profile;
	tScanResultHandle bsslist_handle;
	enum csr_roam_reason reason;
	uint32_t roam_id;
	bool imediate_flag;
	bool clear_flag;
};

/**
 * struct scan_cmd_info - Scan cache entry node
 * @scan_id: scan id
 * @scan_reason: scan reason
 * @profile: roam profile
 * @roam_id: Roam id
 * @roambssentry: scan entries
 */
struct scan_cmd_info {
	wlan_scan_id scan_id;
	enum csr_scan_reason scan_reason;
	tCsrRoamProfile *profile;
	uint32_t roam_id;
	tListElem *roambssentry;
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

struct csr_roam_session {
	uint8_t sessionId;      /* Session ID */
	bool sessionActive;     /* true if it is used */

	/* For BT-AMP station, this serve as BSSID for self-BSS. */
	struct qdf_mac_addr selfMacAddr;

	csr_session_open_cb  session_open_cb;
	csr_session_close_cb session_close_cb;
	csr_roam_completeCallback callback;
	void *pContext;
	eCsrConnectState connectState;
	tCsrRoamConnectedProfile connectedProfile;
	struct csr_roam_connectedinfo connectedInfo;
	tCsrRoamProfile *pCurRoamProfile;
	tSirBssDescription *pConnectBssDesc;
	uint16_t NumPmkidCache; /* valid number of pmkid in the cache*/
	uint16_t curr_cache_idx; /* the index in pmkidcache to write next to */
	tPmkidCacheInfo PmkidCacheInfo[CSR_MAX_PMKID_ALLOWED];
	uint8_t cJoinAttemps;
	/*
	 * This may or may not have the up-to-date valid channel list. It is
	 * used to get WNI_CFG_VALID_CHANNEL_LIST and not alloc memory all time
	 */
	tSirMacChanNum validChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	int32_t sPendingCommands;   /* 0 means CSR is ok to low power */
#ifdef FEATURE_WLAN_WAPI
	uint16_t NumBkidCache;
	tBkidCacheInfo BkidCacheInfo[CSR_MAX_BKID_ALLOWED];
#endif /* FEATURE_WLAN_WAPI */
	/*
	 * indicate whether CSR is roaming
	 * (either via lostlink or dynamic roaming)
	 */
	bool fRoaming;
	/*
	 * to remember some parameters needed for START_BSS.
	 * All member must be set every time we try to join or start an IBSS
	 */
	struct csr_roamstart_bssparams bssParams;
	/* the byte count of pWpaRsnIE; */
	uint32_t nWpaRsnReqIeLength;
	/* contain the WPA/RSN IE in assoc req or one sent in beacon(IBSS) */
	uint8_t *pWpaRsnReqIE;
	/* the byte count for pWpaRsnRspIE */
	uint32_t nWpaRsnRspIeLength;
	/* this contain the WPA/RSN IE in beacon/probe rsp */
	uint8_t *pWpaRsnRspIE;
#ifdef FEATURE_WLAN_WAPI
	/* the byte count of pWapiReqIE; */
	uint32_t nWapiReqIeLength;
	/* this contain the WAPI IE in assoc req or one sent in beacon (IBSS) */
	uint8_t *pWapiReqIE;
	/* the byte count for pWapiRspIE */
	uint32_t nWapiRspIeLength;
	/* this contain the WAPI IE in beacon/probe rsp */
	uint8_t *pWapiRspIE;
#endif /* FEATURE_WLAN_WAPI */
	uint32_t nAddIEScanLength;      /* the byte count of pAddIeScanIE; */
	/* contains the additional IE in (unicast) probe req at time of join */
	uint8_t *pAddIEScan;
	uint32_t nAddIEAssocLength;     /* the byte count for pAddIeAssocIE */
	uint8_t *pAddIEAssoc;
	tCsrTimerInfo roamingTimerInfo;
	enum csr_roaming_reason roamingReason;
	bool fCancelRoaming;
	qdf_mc_timer_t hTimerRoaming;
	/* the roamResult that is used when the roaming timer fires */
	eCsrRoamResult roamResult;
	/* This is the reason code for join(assoc) failure */
	struct csr_roam_joinstatus joinFailStatusCode;
	/* status from PE for deauth/disassoc(lostlink) or our own dyn roam */
	uint32_t roamingStatusCode;
	uint16_t NumPmkidCandidate;
	tPmkidCandidateInfo PmkidCandidateInfo[CSR_MAX_PMKID_ALLOWED];
#ifdef FEATURE_WLAN_WAPI
	uint16_t NumBkidCandidate;
	tBkidCandidateInfo BkidCandidateInfo[CSR_MAX_BKID_ALLOWED];
#endif
	bool fWMMConnection;
	bool fQOSConnection;
#ifdef FEATURE_WLAN_ESE
	tCsrEseCckmInfo eseCckmInfo;
	bool isPrevApInfoValid;
	tSirMacSSid prevApSSID;
	struct qdf_mac_addr prevApBssid;
	uint8_t prevOpChannel;
	uint16_t clientDissSecs;
	uint32_t roamTS1;
	tCsrEseCckmIe suppCckmIeInfo;
#endif
	uint8_t bRefAssocStartCnt;      /* Tracking assoc start indication */
	tSirHTConfig htConfig;
	struct sir_vht_config vht_config;
#ifdef WLAN_FEATURE_11AX
	tDot11fIEhe_cap he_config;
	uint32_t he_sta_obsspd;
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	struct csr_roam_offload_synch_params roamOffloadSynchParams;
	uint8_t psk_pmk[SIR_ROAM_SCAN_PSK_SIZE];
	size_t pmk_len;
	uint8_t RoamKeyMgmtOffloadEnabled;
	roam_offload_synch_ind *roam_synch_data;
	struct pmkid_mode_bits pmkid_modes;
#endif
	tftSMEContext ftSmeContext;
	/* This count represents the number of bssid's we try to join. */
	uint8_t join_bssid_count;
	struct csr_roam_stored_profile stored_roam_profile;
	bool ch_switch_in_progress;
	bool roam_synch_in_progress;
	bool supported_nss_1x1;
	bool disable_hi_rssi;
	bool dhcp_done;
	uint8_t disconnect_reason;
	uint8_t uapsd_mask;
	struct scan_cmd_info scan_info;
	qdf_mc_timer_t roaming_offload_timer;
	bool is_fils_connection;
	uint16_t fils_seq_num;
	bool discon_in_progress;
	struct csr_disconnect_stats disconnect_stats;
};

struct csr_roamstruct {
	uint32_t nextRoamId;
	tDblLinkList channelList5G;
	tDblLinkList channelList24;
	struct csr_config configParam;
	uint32_t numChannelsEeprom;     /* total channels of eeprom */
	struct csr_channel baseChannels;  /* The channel base to work on */
	enum csr_roam_state curState[CSR_ROAM_SESSION_MAX];
	enum csr_roam_substate curSubState[CSR_ROAM_SESSION_MAX];
	/*
	 * This may or may not have the up-to-date valid channel list. It is
	 * used to get WNI_CFG_VALID_CHANNEL_LIST and not alloc mem all time
	 */
	tSirMacChanNum validChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t numValidChannels;       /* total number of channels in CFG */
	int32_t sPendingCommands;
	qdf_mc_timer_t hTimerWaitForKey; /* support timeout for WaitForKey */
	tCsrSummaryStatsInfo summaryStatsInfo;
	tCsrGlobalClassAStatsInfo classAStatsInfo;
	tCsrGlobalClassDStatsInfo classDStatsInfo;
	struct csr_per_chain_rssi_stats_info  per_chain_rssi_stats;
	tDblLinkList statsClientReqList;
	tDblLinkList peStatsReqList;
	struct csr_tlstats_reqinfo tlStatsReqInfo;
	eCsrRoamLinkQualityInd vccLinkQuality;
	struct csr_linkquality_indinfo linkQualityIndInfo;
	tCsrTimerInfo WaitForKeyTimerInfo;
	struct csr_roam_session *roamSession;
	uint32_t transactionId;  /* Current transaction ID for internal use. */
	tCsrNeighborRoamControlInfo neighborRoamInfo[CSR_ROAM_SESSION_MAX];
	uint8_t isFastRoamIniFeatureEnabled;
#ifdef FEATURE_WLAN_ESE
	uint8_t isEseIniFeatureEnabled;
#endif
	uint8_t RoamRssiDiff;
	bool isWESModeEnabled;
	uint32_t deauthRspStatus;
	uint8_t *pReassocResp;          /* reassociation response from new AP */
	uint16_t reassocRespLen;        /* length of reassociation response */
	qdf_mc_timer_t packetdump_timer;
	qdf_list_t rssi_disallow_bssid;
	spinlock_t roam_state_lock;
};

#define GET_NEXT_ROAM_ID(pRoamStruct)  (((pRoamStruct)->nextRoamId + 1 == 0) ? \
			1 : (pRoamStruct)->nextRoamId)
#define CSR_IS_ROAM_STATE(pMac, state, sessionId) \
			((state) == (pMac)->roam.curState[sessionId])
#define CSR_IS_ROAM_STOP(pMac, sessionId) \
		CSR_IS_ROAM_STATE((pMac), eCSR_ROAMING_STATE_STOP, sessionId)
#define CSR_IS_ROAM_INIT(pMac, sessionId) \
		 CSR_IS_ROAM_STATE((pMac), eCSR_ROAMING_STATE_INIT, sessionId)
#define CSR_IS_ROAM_JOINING(pMac, sessionId)  \
		CSR_IS_ROAM_STATE(pMac, eCSR_ROAMING_STATE_JOINING, sessionId)
#define CSR_IS_ROAM_IDLE(pMac, sessionId) \
		CSR_IS_ROAM_STATE(pMac, eCSR_ROAMING_STATE_IDLE, sessionId)
#define CSR_IS_ROAM_JOINED(pMac, sessionId) \
		CSR_IS_ROAM_STATE(pMac, eCSR_ROAMING_STATE_JOINED, sessionId)
#define CSR_IS_ROAM_SUBSTATE(pMac, subState, sessionId) \
		((subState) == (pMac)->roam.curSubState[sessionId])
#define CSR_IS_ROAM_SUBSTATE_JOIN_REQ(pMac, sessionId) \
	CSR_IS_ROAM_SUBSTATE((pMac), eCSR_ROAM_SUBSTATE_JOIN_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_AUTH_REQ(pMac, sessionId) \
	CSR_IS_ROAM_SUBSTATE((pMac), eCSR_ROAM_SUBSTATE_AUTH_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_REASSOC_REQ(pMac, sessionId) \
	CSR_IS_ROAM_SUBSTATE((pMac), eCSR_ROAM_SUBSTATE_REASSOC_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DISASSOC_REQ(pMac, sessionId) \
	CSR_IS_ROAM_SUBSTATE((pMac), eCSR_ROAM_SUBSTATE_DISASSOC_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DISASSOC_NO_JOIN(pMac, sessionId) \
	CSR_IS_ROAM_SUBSTATE((pMac), \
		eCSR_ROAM_SUBSTATE_DISASSOC_NOTHING_TO_JOIN, sessionId)
#define CSR_IS_ROAM_SUBSTATE_REASSOC_FAIL(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_DISASSOC_REASSOC_FAILURE, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DISASSOC_FORCED(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_DISASSOC_FORCED, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DEAUTH_REQ(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_DEAUTH_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_START_BSS_REQ(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_START_BSS_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_STOP_BSS_REQ(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_STOP_BSS_REQ, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DISCONNECT_CONTINUE(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
		eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING, sessionId)
#define CSR_IS_ROAM_SUBSTATE_CONFIG(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
		eCSR_ROAM_SUBSTATE_CONFIG, sessionId)
#define CSR_IS_ROAM_SUBSTATE_WAITFORKEY(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY, sessionId)
#define CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF, sessionId)
#define CSR_IS_ROAM_SUBSTATE_HO_NT(pMac, sessionId) \
		CSR_IS_ROAM_SUBSTATE((pMac), \
			eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC, sessionId)
#define CSR_IS_ROAM_SUBSTATE_HO_NRT(pMac, sessionId)  \
			CSR_IS_ROAM_SUBSTATE((pMac), \
				eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC,\
					sessionId)
#define CSR_IS_ROAM_SUBSTATE_HO_RT(pMac, sessionId) \
			CSR_IS_ROAM_SUBSTATE((pMac),\
			eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC, sessionId)
#define CSR_IS_PHY_MODE_B_ONLY(pMac) \
	((eCSR_DOT11_MODE_11b == (pMac)->roam.configParam.phyMode) || \
	 (eCSR_DOT11_MODE_11b_ONLY == (pMac)->roam.configParam.phyMode))

#define CSR_IS_PHY_MODE_G_ONLY(pMac) \
	(eCSR_DOT11_MODE_11g == (pMac)->roam.configParam.phyMode \
		|| eCSR_DOT11_MODE_11g_ONLY == (pMac)->roam.configParam.phyMode)

#define CSR_IS_PHY_MODE_A_ONLY(pMac) \
	(eCSR_DOT11_MODE_11a == (pMac)->roam.configParam.phyMode)

#define CSR_IS_PHY_MODE_DUAL_BAND(phyMode) \
	((eCSR_DOT11_MODE_abg & (phyMode)) || \
	 (eCSR_DOT11_MODE_11n & (phyMode)) || \
	 (eCSR_DOT11_MODE_11ac & (phyMode)) || \
	 (eCSR_DOT11_MODE_11ax & (phyMode)) || \
	 (eCSR_DOT11_MODE_AUTO & (phyMode)))

#define CSR_IS_PHY_MODE_11n(phy_mode) \
	((eCSR_DOT11_MODE_11n == phy_mode) || \
	 (eCSR_DOT11_MODE_11n_ONLY == phy_mode) || \
	 (eCSR_DOT11_MODE_11ac == phy_mode) || \
	 (eCSR_DOT11_MODE_11ac_ONLY == phy_mode))

#define CSR_IS_PHY_MODE_11ac(phy_mode) \
	((eCSR_DOT11_MODE_11ac == phy_mode) || \
	 (eCSR_DOT11_MODE_11ac_ONLY == phy_mode))

/*
 * this function returns true if the NIC is operating exclusively in
 * the 2.4 GHz band, meaning. it is NOT operating in the 5.0 GHz band.
 */
#define CSR_IS_24_BAND_ONLY(pMac) \
	(BAND_2G == (pMac)->roam.configParam.eBand)

#define CSR_IS_5G_BAND_ONLY(pMac) \
	(BAND_5G == (pMac)->roam.configParam.eBand)

#define CSR_IS_RADIO_DUAL_BAND(pMac) \
	(BAND_ALL == (pMac)->roam.configParam.bandCapability)

#define CSR_IS_RADIO_BG_ONLY(pMac) \
	(BAND_2G == (pMac)->roam.configParam.bandCapability)

/*
 * this function returns true if the NIC is operating exclusively in the 5.0 GHz
 * band, meaning. it is NOT operating in the 2.4 GHz band
 */
#define CSR_IS_RADIO_A_ONLY(pMac) \
	(BAND_5G == (pMac)->roam.configParam.bandCapability)
/* this function returns true if the NIC is operating in both bands. */
#define CSR_IS_OPEARTING_DUAL_BAND(pMac) \
	((BAND_ALL == (pMac)->roam.configParam.bandCapability) && \
		(BAND_ALL == (pMac)->roam.configParam.eBand))
/*
 * this function returns true if the NIC can operate in the 5.0 GHz band
 * (could operate in the 2.4 GHz band also)
 */
#define CSR_IS_OPERATING_A_BAND(pMac) \
	(CSR_IS_OPEARTING_DUAL_BAND((pMac)) || \
		CSR_IS_RADIO_A_ONLY((pMac)) || CSR_IS_5G_BAND_ONLY((pMac)))

/*
 * this function returns true if the NIC can operate in the 2.4 GHz band
 * (could operate in the 5.0 GHz band also).
 */
#define CSR_IS_OPERATING_BG_BAND(pMac) \
	(CSR_IS_OPEARTING_DUAL_BAND((pMac)) || \
		CSR_IS_RADIO_BG_ONLY((pMac)) || CSR_IS_24_BAND_ONLY((pMac)))
#define CSR_GET_BAND(ch_num) \
	((WLAN_REG_IS_24GHZ_CH(ch_num)) ? BAND_2G : BAND_5G)
#define CSR_IS_11D_INFO_FOUND(pMac) \
	(0 != (pMac)->scan.channelOf11dInfo)
#define CSR_IS_ROAMING(pSession) \
	((CSR_IS_LOSTLINK_ROAMING((pSession)->roamingReason)) || \
		(eCsrDynamicRoaming == (pSession)->roamingReason)  ||	\
		(eCsrReassocRoaming == (pSession)->roamingReason))
#define CSR_IS_ADDTS_WHEN_ACMOFF_SUPPORTED(pMac) \
	(pMac->roam.configParam.addTSWhenACMIsOff)
#define CSR_IS_LOSTLINK_ROAMING(reason) \
	((eCsrLostlinkRoamingDisassoc == (reason)) || \
		(eCsrLostlinkRoamingDeauth == (reason)))

#define CSR_IS_ROAMING_COMMAND(pCommand) \
		((eCsrLostLink1 == (pCommand)->u.roamCmd.roamReason) || \
			(eCsrLostLink2 == (pCommand)->u.roamCmd.roamReason) || \
			(eCsrLostLink3 == (pCommand)->u.roamCmd.roamReason))

#ifdef FEATURE_LFR_SUBNET_DETECTION
/* bit-4 and bit-5 indicate the subnet status */
#define CSR_GET_SUBNET_STATUS(roam_reason) (((roam_reason) & 0x30) >> 4)
#else
#define CSR_GET_SUBNET_STATUS(roam_reason) (0)
#endif

QDF_STATUS csr_get_channel_and_power_list(tpAniSirGlobal pMac);

QDF_STATUS csr_scan_filter_results(tpAniSirGlobal pMac);

QDF_STATUS csr_set_modify_profile_fields(tpAniSirGlobal pMac,
		uint32_t sessionId, tCsrRoamModifyProfileFields *
		pModifyProfileFields);
QDF_STATUS csr_get_modify_profile_fields(tpAniSirGlobal pMac,
		uint32_t sessionId, tCsrRoamModifyProfileFields *
		pModifyProfileFields);
void csr_set_global_cfgs(tpAniSirGlobal pMac);
void csr_set_default_dot11_mode(tpAniSirGlobal pMac);
bool csr_is_conn_state_disconnected(tpAniSirGlobal pMac,
						   uint32_t sessionId);
bool csr_is_conn_state_connected_ibss(tpAniSirGlobal pMac,
						      uint32_t sessionId);
bool csr_is_conn_state_disconnected_ibss(tpAniSirGlobal pMac,
							   uint32_t sessionId);
bool csr_is_conn_state_connected_infra(tpAniSirGlobal pMac,
							uint32_t sessionId);
bool csr_is_conn_state_connected(tpAniSirGlobal pMac,
					       uint32_t sessionId);
bool csr_is_conn_state_infra(tpAniSirGlobal pMac,
					uint32_t sessionId);
bool csr_is_conn_state_ibss(tpAniSirGlobal pMac, uint32_t sessionId);
bool csr_is_conn_state_wds(tpAniSirGlobal pMac, uint32_t sessionId);
bool csr_is_conn_state_connected_wds(tpAniSirGlobal pMac,
						    uint32_t sessionId);
bool csr_is_conn_state_disconnected_wds(tpAniSirGlobal pMac,
		uint32_t sessionId);
bool csr_is_any_session_in_connect_state(tpAniSirGlobal pMac);
bool csr_is_all_session_disconnected(tpAniSirGlobal pMac);
bool csr_is_sta_session_connected(tpAniSirGlobal pMac);
bool csr_is_p2p_session_connected(tpAniSirGlobal pMac);
bool csr_is_any_session_connected(tpAniSirGlobal pMac);
bool csr_is_infra_connected(tpAniSirGlobal pMac);

/**
 * csr_get_connected_infra() - get the session id of the connected infra
 * @mac_ctx:  pointer to global mac structure
 *
 * The function check if any infra is present in connected state and if present
 * return the session id of the connected infra else if no infra is in connected
 * state return CSR_SESSION_ID_INVALID
 *
 * Return: session id of the connected infra
 */
uint8_t csr_get_connected_infra(tpAniSirGlobal mac_ctx);
bool csr_is_concurrent_infra_connected(tpAniSirGlobal pMac);
bool csr_is_concurrent_session_running(tpAniSirGlobal pMac);
bool csr_is_infra_ap_started(tpAniSirGlobal pMac);
bool csr_is_ibss_started(tpAniSirGlobal pMac);
bool csr_is_valid_mc_concurrent_session(tpAniSirGlobal pMac,
							 uint32_t sessionId,
						tSirBssDescription *pBssDesc);
bool csr_is_conn_state_connected_infra_ap(tpAniSirGlobal pMac,
		uint32_t sessionId);
QDF_STATUS csr_get_statistics(tpAniSirGlobal pMac,
		eCsrStatsRequesterType requesterId,
		uint32_t statsMask, tCsrStatsCallback callback,
		uint8_t staId, void *pContext, uint8_t sessionId);
QDF_STATUS csr_get_rssi(tpAniSirGlobal pMac, tCsrRssiCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, int8_t lastRSSI,
		void *pContext);
QDF_STATUS csr_get_snr(tpAniSirGlobal pMac, tCsrSnrCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, void *pContext);
QDF_STATUS csr_get_config_param(tpAniSirGlobal pMac,
					  tCsrConfigParam *pParam);
QDF_STATUS csr_change_default_config_param(tpAniSirGlobal pMac,
		tCsrConfigParam *pParam);
QDF_STATUS csr_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
QDF_STATUS csr_open(tpAniSirGlobal pMac);
QDF_STATUS csr_init_chan_list(tpAniSirGlobal mac, uint8_t *alpha2);
QDF_STATUS csr_close(tpAniSirGlobal pMac);
QDF_STATUS csr_start(tpAniSirGlobal pMac);
QDF_STATUS csr_stop(tpAniSirGlobal pMac, tHalStopType stopType);
QDF_STATUS csr_ready(tpAniSirGlobal pMac);

#ifdef FEATURE_WLAN_WAPI
QDF_STATUS csr_roam_get_wapi_req_ie(tpAniSirGlobal pMac,
		uint32_t sessionId, uint32_t *pLen, uint8_t *pBuf);
QDF_STATUS csr_roam_get_wapi_rsp_ie(tpAniSirGlobal pMac,
						uint32_t sessionId,
		uint32_t *pLen, uint8_t *pBuf);
uint8_t csr_construct_wapi_ie(tpAniSirGlobal pMac, uint32_t sessionId,
		tCsrRoamProfile *pProfile,
		tSirBssDescription *pSirBssDesc,
		tDot11fBeaconIEs *pIes, tCsrWapiIe *pWapiIe);
#endif /* FEATURE_WLAN_WAPI */

void csr_set_cfg_privacy(tpAniSirGlobal pMac,
						tCsrRoamProfile *pProfile,
						bool fPrivacy);
int8_t csr_get_infra_session_id(tpAniSirGlobal pMac);
uint8_t csr_get_infra_operation_channel(tpAniSirGlobal pMac,
							uint8_t sessionId);
bool csr_is_session_client_and_connected(tpAniSirGlobal pMac,
		uint8_t sessionId);
uint8_t csr_get_concurrent_operation_channel(
	tpAniSirGlobal pMac);

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t csr_check_concurrent_channel_overlap(
		tpAniSirGlobal pMac,
		uint16_t sap_ch, eCsrPhyMode sap_phymode,
		uint8_t cc_switch_mode);
#endif
QDF_STATUS csr_roam_copy_connect_profile(tpAniSirGlobal pMac,
		uint32_t sessionId, tCsrRoamConnectedProfile *pProfile);
bool csr_is_set_key_allowed(tpAniSirGlobal pMac, uint32_t sessionId);

/* Returns whether the current association is a 11r assoc or not */
bool csr_roam_is11r_assoc(tpAniSirGlobal pMac, uint8_t sessionId);

#ifdef FEATURE_WLAN_ESE
/* Returns whether the current association is a ESE assoc or not */
bool csr_roam_is_ese_assoc(tpAniSirGlobal pMac, uint32_t sessionId);
bool csr_roam_is_ese_ini_feature_enabled(tpAniSirGlobal pMac);
QDF_STATUS csr_get_tsm_stats(tpAniSirGlobal pMac,
		tCsrTsmStatsCallback callback,
		uint8_t staId,
		struct qdf_mac_addr bssId,
		void *pContext, uint8_t tid);
#endif

/* Remove this code once SLM_Sessionization is supported */
void csr_disconnect_all_active_sessions(tpAniSirGlobal pMac);

/* Returns whether "Legacy Fast Roaming" is enabled...or not */
bool csr_roam_is_fast_roam_enabled(tpAniSirGlobal pMac,
						uint32_t sessionId);
bool csr_roam_is_roam_offload_scan_enabled(
	tpAniSirGlobal pMac);
bool csr_is_channel_present_in_list(uint8_t *pChannelList,
						   int numChannels,
						   uint8_t channel);
QDF_STATUS csr_add_to_channel_list_front(uint8_t *pChannelList,
							int numChannels,
		uint8_t channel);
#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
QDF_STATUS csr_roam_offload_scan_rsp_hdlr(tpAniSirGlobal pMac,
		tpSirRoamOffloadScanRsp scanOffloadRsp);
#else
static inline QDF_STATUS csr_roam_offload_scan_rsp_hdlr(
		tpAniSirGlobal pMac,
		tpSirRoamOffloadScanRsp scanOffloadRsp)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
QDF_STATUS csr_handoff_request(tpAniSirGlobal pMac, uint8_t sessionId,
		tCsrHandoffRequest
		*pHandoffInfo);
bool csr_roam_is_sta_mode(tpAniSirGlobal pMac, uint32_t sessionId);

/* Post Channel Change Indication */
QDF_STATUS csr_roam_channel_change_req(tpAniSirGlobal pMac,
					struct qdf_mac_addr
				       bssid, struct ch_params *ch_params,
				       tCsrRoamProfile *profile);

/* Post Beacon Tx Start Indication */
QDF_STATUS csr_roam_start_beacon_req(tpAniSirGlobal pMac,
		struct qdf_mac_addr bssid, uint8_t dfsCacWaitStatus);

QDF_STATUS csr_roam_send_chan_sw_ie_request(tpAniSirGlobal pMac,
					    struct qdf_mac_addr bssid,
					    uint8_t targetChannel,
					    uint8_t csaIeReqd,
					    struct ch_params *ch_params);
QDF_STATUS csr_roam_modify_add_ies(tpAniSirGlobal pMac,
					tSirModifyIE *pModifyIE,
				   eUpdateIEsType updateType);
QDF_STATUS
csr_roam_update_add_ies(tpAniSirGlobal pMac,
		tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS csr_scan_save_roam_offload_ap_to_scan_cache(
		tpAniSirGlobal pMac,
		struct sSirSmeRoamOffloadSynchInd *roam_synch_ind_ptr,
		tpSirBssDescription  bss_desc_ptr);
void csr_process_ho_fail_ind(tpAniSirGlobal pMac, void *pMsgBuf);
#endif
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
void csr_roaming_report_diag_event(tpAniSirGlobal mac_ctx,
		roam_offload_synch_ind *roam_synch_ind_ptr,
		enum csr_diagwlan_status_eventreason reason);
#else
static inline void csr_roaming_report_diag_event(
		tpAniSirGlobal mac_ctx,
		roam_offload_synch_ind *roam_synch_ind_ptr,
		enum csr_diagwlan_status_eventreason reason)
{}
#endif

bool csr_store_joinreq_param(tpAniSirGlobal mac_ctx,
		tCsrRoamProfile *profile,
		tScanResultHandle scan_cache,
		uint32_t *roam_id,
		uint32_t session_id);
bool csr_clear_joinreq_param(tpAniSirGlobal mac_ctx,
		uint32_t session_id);
QDF_STATUS csr_issue_stored_joinreq(tpAniSirGlobal mac_ctx,
		uint32_t *roam_id,
		uint32_t session_id);
QDF_STATUS csr_get_channels_and_power(tpAniSirGlobal pMac);

void csr_nonscan_pending_ll_unlock(struct sAniSirGlobal *mac_ctx);
void csr_nonscan_active_ll_unlock(struct sAniSirGlobal *mac_ctx);
void csr_nonscan_pending_ll_lock(struct sAniSirGlobal *mac_ctx);
void csr_nonscan_active_ll_lock(struct sAniSirGlobal *mac_ctx);
bool csr_nonscan_active_ll_is_list_empty(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
bool csr_nonscan_pending_ll_is_list_empty(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
bool csr_nonscan_active_ll_remove_entry(
			struct sAniSirGlobal *mac_ctx,
			tListElem *pEntryToRemove, bool inter_locked);
bool csr_nonscan_pending_ll_remove_entry(
			struct sAniSirGlobal *mac_ctx,
			tListElem *pEntryToRemove, bool inter_locked);
tListElem *csr_nonscan_active_ll_peek_head(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
tListElem *csr_nonscan_pending_ll_peek_head(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
tListElem *csr_nonscan_active_ll_remove_head(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
tListElem *csr_nonscan_pending_ll_remove_head(
			struct sAniSirGlobal *mac_ctx,
			bool inter_locked);
uint32_t csr_nonscan_pending_ll_count(
			struct sAniSirGlobal *mac_ctx);
void csr_nonscan_pending_ll_insert_head(
			struct sAniSirGlobal *mac_ctx,
		tListElem *entry, bool inter_locked);
void csr_nonscan_pending_ll_insert_tail(
			struct sAniSirGlobal *mac_ctx,
		tListElem *entry, bool inter_locked);
uint32_t csr_nonscan_active_ll_count(
			struct sAniSirGlobal *mac_ctx);
void csr_nonscan_active_ll_insert_head(
			struct sAniSirGlobal *mac_ctx,
		tListElem *entry, bool inter_locked);
tListElem *csr_nonscan_pending_ll_next(
			struct sAniSirGlobal *mac_ctx,
		tListElem *entry, bool inter_locked);
void purge_sme_session_pending_cmd_list(
			struct sAniSirGlobal *mac_ctx,
		uint32_t session_id);
void purge_sme_session_active_cmd_list(
			struct sAniSirGlobal *mac_ctx,
		uint32_t session_id);
void purge_sme_session_pending_scan_cmd_list(struct sAniSirGlobal *mac_ctx,
		uint32_t session_id);
void purge_sme_session_active_scan_cmd_list(struct sAniSirGlobal *mac_ctx,
		uint32_t session_id);
bool csr_wait_for_connection_update(tpAniSirGlobal mac,
		bool do_release_reacquire_lock);
enum QDF_OPMODE csr_get_session_persona(tpAniSirGlobal pmac,
					uint32_t session_id);
void csr_roam_substate_change(
			tpAniSirGlobal pMac, enum csr_roam_substate
					NewSubstate, uint32_t sessionId);

void csr_neighbor_roam_process_scan_results(
		tpAniSirGlobal mac_ctx,
		uint8_t sessionid, tScanResultHandle *scan_results_list);

void csr_neighbor_roam_trigger_handoff(tpAniSirGlobal mac_ctx,
					uint8_t session_id);
bool csr_is_ndi_started(tpAniSirGlobal mac_ctx, uint32_t session_id);

QDF_STATUS csr_roam_update_config(
			tpAniSirGlobal mac_ctx, uint8_t session_id,
				  uint16_t capab, uint32_t value);

/**
 * csr_is_mcc_channel() - check if using the channel results into MCC
 * @hal: pointer to HAL
 * @channel : channel number to check for MCC scenario
 *
 * Return : true if channel causes MCC, else false
 */
bool csr_is_mcc_channel(tHalHandle hal, uint8_t channel);

#endif
