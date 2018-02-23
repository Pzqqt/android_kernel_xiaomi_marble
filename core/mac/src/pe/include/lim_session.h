/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#if !defined(__LIM_SESSION_H)
#define __LIM_SESSION_H

/**=========================================================================

   \file  lim_session.h

   \brief prototype for lim Session related APIs

   \author Sunit Bhatia

   ========================================================================*/

/* Powersave Offload Implementation */
typedef enum ePowersaveState {
	PMM_FULL_POWER,
	PMM_POWER_SAVE
} tPowersaveState;

/* Master Structure: This will be part of PE Session Entry */
typedef struct sPowersaveoffloadInfo {
	tPowersaveState psstate;
	uint8_t bcnmiss;
} tPowersaveoffloadInfo, tpPowersaveoffloadInfo;

#ifdef WLAN_FEATURE_11W
typedef struct tagComebackTimerInfo {
	tpAniSirGlobal pMac;
	uint8_t sessionID;
	tLimMlmStates limPrevMlmState;  /* Previous MLM State */
	tLimMlmStates limMlmState;      /* MLM State */
} tComebackTimerInfo;
#endif /* WLAN_FEATURE_11W */
/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
/* Maximum Number of WEP KEYS */
#define MAX_WEP_KEYS 4

/* Maximum allowable size of a beacon frame */
#define SCH_MAX_BEACON_SIZE    512

#define SCH_MAX_PROBE_RESP_SIZE 512
#define SCH_PROTECTION_RESET_TIME 4000

/*--------------------------------------------------------------------------
   Type declarations
   ------------------------------------------------------------------------*/
typedef struct {
	tSirMacBeaconInterval beaconInterval;
	uint8_t fShortPreamble;
	uint8_t llaCoexist;
	uint8_t llbCoexist;
	uint8_t llgCoexist;
	uint8_t ht20Coexist;
	uint8_t llnNonGFCoexist;
	uint8_t fRIFSMode;
	uint8_t fLsigTXOPProtectionFullSupport;
	uint8_t gHTObssMode;
} tBeaconParams, *tpBeaconParams;

typedef struct join_params {
	uint16_t prot_status_code;
	uint16_t pe_session_id;
	tSirResultCodes result_code;
} join_params;

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
#define MAX_BSS_COLOR_VALUE 63
#define TIME_BEACON_NOT_UPDATED 30000
#define BSS_COLOR_SWITCH_COUNTDOWN 5
#define OBSS_COLOR_COLLISION_DETECTION_STA_PERIOD_MS 10000
#define OBSS_COLOR_COLLISION_DETECTION_AP_PERIOD_MS 5000
#define OBSS_COLOR_COLLISION_SCAN_PERIOD_MS 200
#define OBSS_COLOR_COLLISION_FREE_SLOT_EXPIRY_MS 50000
struct bss_color_info {
	qdf_time_t timestamp;
	uint64_t seen_count;
};
#endif

/**
 * struct obss_detection_cfg - current obss detection cfg set to firmware
 * @obss_11b_ap_detect_mode: detection mode for 11b access point.
 * @obss_11b_sta_detect_mode: detection mode for 11b station.
 * @obss_11g_ap_detect_mode: detection mode for 11g access point.
 * @obss_11a_detect_mode: detection mode for 11a access point.
 * @obss_ht_legacy_detect_mode: detection mode for ht ap with legacy mode.
 * @obss_ht_mixed_detect_mode: detection mode for ht ap with mixed mode.
 * @obss_ht_20mhz_detect_mode: detection mode for ht ap with 20mhz mode.
 */
struct obss_detection_cfg {
	uint8_t obss_11b_ap_detect_mode;
	uint8_t obss_11b_sta_detect_mode;
	uint8_t obss_11g_ap_detect_mode;
	uint8_t obss_11a_detect_mode;
	uint8_t obss_ht_legacy_detect_mode;
	uint8_t obss_ht_mixed_detect_mode;
	uint8_t obss_ht_20mhz_detect_mode;
};

typedef struct sPESession       /* Added to Support BT-AMP */
{
	/* To check session table is in use or free */
	uint8_t available;
	uint16_t peSessionId;
	uint8_t smeSessionId;
	uint16_t transactionId;

	/* In AP role: BSSID and selfMacAddr will be the same. */
	/* In STA role: they will be different */
	tSirMacAddr bssId;
	tSirMacAddr selfMacAddr;
	tSirMacSSid ssId;
	uint8_t bssIdx;
	uint8_t valid;
	tLimMlmStates limMlmState;      /* MLM State */
	tLimMlmStates limPrevMlmState;  /* Previous MLM State */
	tLimSmeStates limSmeState;      /* SME State */
	tLimSmeStates limPrevSmeState;  /* Previous SME State */
	tLimSystemRole limSystemRole;
	tSirBssType bssType;
	uint8_t operMode;       /* AP - 0; STA - 1 ; */
	tSirNwType nwType;
	tpSirSmeStartBssReq pLimStartBssReq;    /* handle to smestart bss req */
	tpSirSmeJoinReq pLimJoinReq;    /* handle to sme join req */
	tpSirSmeJoinReq pLimReAssocReq; /* handle to sme reassoc req */
	tpLimMlmJoinReq pLimMlmJoinReq; /* handle to MLM join Req */
	void *pLimMlmReassocRetryReq;   /* keep reasoc req for retry */
	void *pLimMlmReassocReq;        /* handle to MLM reassoc Req */
	uint16_t channelChangeReasonCode;
	uint8_t dot11mode;
	uint8_t htCapability;
	/* Supported Channel Width Set: 0-20MHz 1 - 40MHz */
	uint8_t htSupportedChannelWidthSet;
	/* Recommended Tx Width Set
	 * 0 - use 20 MHz channel (control channel)
	 * 1 - use channel width enabled under Supported Channel Width Set
	 */
	uint8_t htRecommendedTxWidthSet;
	/* Identifies the 40 MHz extension channel */
	ePhyChanBondState htSecondaryChannelOffset;
	enum band_info limRFBand;
	uint8_t limIbssActive;  /* TO SUPPORT CONCURRENCY */

	/* These global varibales moved to session Table to support BT-AMP : Oct 9th review */
	tAniAuthType limCurrentAuthType;
	uint16_t limCurrentBssCaps;
	uint8_t limCurrentBssQosCaps;
	uint16_t limCurrentBssPropCap;
	uint8_t limSentCapsChangeNtf;
	uint16_t limAID;

	/* Parameters  For Reassociation */
	tSirMacAddr limReAssocbssId;
	tSirMacChanNum limReassocChannelId;
	/* CB paramaters required/duplicated for Reassoc since re-assoc mantains its own params in lim */
	uint8_t reAssocHtSupportedChannelWidthSet;
	uint8_t reAssocHtRecommendedTxWidthSet;
	ePhyChanBondState reAssocHtSecondaryChannelOffset;
	tSirMacSSid limReassocSSID;
	uint16_t limReassocBssCaps;
	uint8_t limReassocBssQosCaps;
	uint16_t limReassocBssPropCap;

	/* Assoc or ReAssoc Response Data/Frame */
	void *limAssocResponseData;

	/** BSS Table parameters **/

	/*
	 * staId:  Start BSS: this is the  Sta Id for the BSS.
	 * Join: this is the selfStaId
	 * In both cases above, the peer STA ID wll be stored in dph hash table.
	 */
	uint16_t staId;
	uint16_t statypeForBss; /* to know session is for PEER or SELF */
	uint8_t shortSlotTimeSupported;
	uint8_t dtimPeriod;
	tSirMacRateSet rateSet;
	tSirMacRateSet extRateSet;
	tSirMacHTOperatingMode htOperMode;
	uint8_t currentOperChannel;
	uint8_t currentReqChannel;
	uint8_t LimRxedBeaconCntDuringHB;

	/* Time stamp of the last beacon received from the BSS to which STA is connected. */
	uint64_t lastBeaconTimeStamp;
	/* RX Beacon count for the current BSS to which STA is connected. */
	uint32_t currentBssBeaconCnt;
	uint8_t bcon_dtim_period;

	uint32_t bcnLen;
	uint8_t *beacon;        /* Used to store last beacon / probe response before assoc. */

	uint32_t assocReqLen;
	uint8_t *assocReq;      /* Used to store association request frame sent out while associating. */

	uint32_t assocRspLen;
	uint8_t *assocRsp;      /* Used to store association response received while associating */
	tAniSirDph dph;
	void **parsedAssocReq;  /* Used to store parsed assoc req from various requesting station */
	uint32_t RICDataLen;    /* Used to store the Ric data received in the assoc response */
	uint8_t *ricData;
#ifdef FEATURE_WLAN_ESE
	uint32_t tspecLen;      /* Used to store the TSPEC IEs received in the assoc response */
	uint8_t *tspecIes;
#endif
	uint32_t encryptType;

	bool bTkipCntrMeasActive;       /* Used to keep record of TKIP counter measures start/stop */

	uint8_t gLimProtectionControl;  /* used for 11n protection */

	uint8_t gHTNonGFDevicesPresent;

	/* protection related config cache */
	tCfgProtection cfgProtection;

	/* Number of legacy STAs associated */
	tLimProtStaParams gLim11bParams;

	/* Number of 11A STAs associated */
	tLimProtStaParams gLim11aParams;

	/* Number of non-ht non-legacy STAs associated */
	tLimProtStaParams gLim11gParams;

	/* Number of nonGf STA associated */
	tLimProtStaParams gLimNonGfParams;

	/* Number of HT 20 STAs associated */
	tLimProtStaParams gLimHt20Params;

	/* Number of Lsig Txop not supported STAs associated */
	tLimProtStaParams gLimLsigTxopParams;

	/* Number of STAs that do not support short preamble */
	tLimNoShortParams gLimNoShortParams;

	/* Number of STAs that do not support short slot time */
	tLimNoShortSlotParams gLimNoShortSlotParams;

	/* OLBC parameters */
	tLimProtStaParams gLimOlbcParams;

	/* OLBC parameters */
	tLimProtStaParams gLimOverlap11gParams;

	tLimProtStaParams gLimOverlap11aParams;
	tLimProtStaParams gLimOverlapHt20Params;
	tLimProtStaParams gLimOverlapNonGfParams;

	/* cache for each overlap */
	tCacheParams protStaCache[LIM_PROT_STA_CACHE_SIZE];

	uint8_t privacy;
	tAniAuthType authType;
	tSirKeyMaterial WEPKeyMaterial[MAX_WEP_KEYS];

	tDot11fIERSN gStartBssRSNIe;
	tDot11fIEWPA gStartBssWPAIe;
	tSirAPWPSIEs APWPSIEs;
	uint8_t apUapsdEnable;
	tSirWPSPBCSession *pAPWPSPBCSession;
	uint32_t DefProbeRspIeBitmap[8];
	uint32_t proxyProbeRspEn;
	tDot11fProbeResponse probeRespFrame;
	uint8_t ssidHidden;
	bool fwdWPSPBCProbeReq;
	uint8_t wps_state;
	bool wps_registration;

	uint8_t limQosEnabled:1;        /* 11E */
	uint8_t limWmeEnabled:1;        /* WME */
	uint8_t limWsmEnabled:1;        /* WSM */
	uint8_t limHcfEnabled:1;
	uint8_t lim11dEnabled:1;
#ifdef WLAN_FEATURE_11W
	uint8_t limRmfEnabled:1;        /* 11W */
#endif
	uint32_t lim11hEnable;

	int8_t maxTxPower;   /* MIN (Regulatory and local power constraint) */
	enum QDF_OPMODE pePersona;
	int8_t txMgmtPower;
	bool is11Rconnection;

#ifdef FEATURE_WLAN_ESE
	bool isESEconnection;
	tEsePEContext eseContext;
#endif
	bool isFastTransitionEnabled;
	bool isFastRoamIniFeatureEnabled;
	tSirNoAParam p2pNoA;
	tSirP2PNoaAttr p2pGoPsUpdate;
	uint32_t defaultAuthFailureTimeout;
	tSirP2PNoaStart p2pGoPsNoaStartInd;

	/* EDCA QoS parameters
	 * gLimEdcaParams - These EDCA parameters are used locally on AP or STA.
	 * If STA, then these are values taken from the Assoc Rsp when associating,
	 * or Beacons/Probe Response after association.  If AP, then these are
	 * values originally set locally on AP.
	 *
	 * gLimEdcaParamsBC - These EDCA parameters are use by AP to broadcast
	 * to other STATIONs in the BSS.
	 *
	 * gLimEdcaParamsActive: These EDCA parameters are what's actively being
	 * used on station. Specific AC values may be downgraded depending on
	 * admission control for that particular AC.
	 */
	tSirMacEdcaParamRecord gLimEdcaParams[MAX_NUM_AC];      /* used locally */
	tSirMacEdcaParamRecord gLimEdcaParamsBC[MAX_NUM_AC];    /* used for broadcast */
	tSirMacEdcaParamRecord gLimEdcaParamsActive[MAX_NUM_AC];

	uint8_t gLimEdcaParamSetCount;

	tBeaconParams beaconParams;
	uint8_t vhtCapability;
	tLimOperatingModeInfo gLimOperatingMode;
	uint8_t vhtCapabilityPresentInBeacon;
	uint8_t ch_center_freq_seg0;
	enum phy_ch_width ch_width;
	uint8_t ch_center_freq_seg1;
	uint8_t enableVhtpAid;
	uint8_t enableVhtGid;
	tLimWiderBWChannelSwitchInfo gLimWiderBWChannelSwitch;
	uint8_t enableAmpduPs;
	uint8_t enableHtSmps;
	uint8_t htSmpsvalue;
	bool send_smps_action;
	uint8_t spectrumMgtEnabled;
	/* *********************11H related**************************** */
	tLimSpecMgmtInfo gLimSpecMgmt;
	/* CB Primary/Secondary Channel Switch Info */
	tLimChannelSwitchInfo gLimChannelSwitch;
	/* *********************End 11H related**************************** */

	/*Flag to Track Status/Indicate HBFailure on this session */
	bool LimHBFailureStatus;
	uint32_t gLimPhyMode;
	uint8_t amsduSupportedInBA;
	uint8_t txLdpcIniFeatureEnabled;
	/**
	 * Following is the place holder for free peer index pool.
	 * A non-zero value indicates that peer index is available
	 * for assignment.
	 */
	uint8_t *gpLimPeerIdxpool;
	uint8_t freePeerIdxHead;
	uint8_t freePeerIdxTail;
	uint16_t gLimNumOfCurrentSTAs;
#ifdef FEATURE_WLAN_TDLS
	 /* TDLS parameters to check whether TDLS
	  * and TDLS channel switch is allowed in the
	  * AP network
	  */
	uint32_t peerAIDBitmap[2];
	bool tdls_prohibited;
	bool tdls_chan_swit_prohibited;
#endif
	bool fWaitForProbeRsp;
	bool fIgnoreCapsChange;
	bool fDeauthReceived;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	int8_t rssi;
#endif
	uint8_t max_amsdu_num;
	uint8_t isCoalesingInIBSSAllowed;

	tSirHTConfig htConfig;
	struct sir_vht_config vht_config;
	/*
	 * Place holder for StartBssReq message
	 * received by SME state machine
	 */
	uint8_t gLimCurrentBssUapsd;

	/* Used on STA, this is a static UAPSD mask setting
	 * derived from SME_JOIN_REQ and SME_REASSOC_REQ. If a
	 * particular AC bit is set, it means the AC is both
	 * trigger enabled and delivery enabled.
	 */
	uint8_t gUapsdPerAcBitmask;

	/* Used on STA, this is a dynamic UPASD mask setting
	 * derived from AddTS Rsp and DelTS frame. If a
	 * particular AC bit is set, it means AC is trigger
	 * enabled.
	 */
	uint8_t gUapsdPerAcTriggerEnableMask;

	/* Used on STA, dynamic UPASD mask setting
	 * derived from AddTS Rsp and DelTs frame. If
	 * a particular AC bit is set, it means AC is
	 * delivery enabled.
	 */
	uint8_t gUapsdPerAcDeliveryEnableMask;

	/* Flag to skip CSA IE processing when CSA
	 * offload is enabled.
	 */
	uint8_t csaOffloadEnable;

	/* Used on STA for AC downgrade. This is a dynamic mask
	 * setting which keep tracks of ACs being admitted.
	 * If bit is set to 0: That partiular AC is not admitted
	 * If bit is set to 1: That particular AC is admitted
	 */
	uint8_t gAcAdmitMask[SIR_MAC_DIRECTION_DIRECT];

	/* Power Save Off load Parameters */
	tPowersaveoffloadInfo pmmOffloadInfo;
	/* SMPS mode */
	uint8_t smpsMode;

	uint8_t chainMask;

	/* Flag to indicate Chan Sw announcement is required */
	uint8_t dfsIncludeChanSwIe;

	/* Flag to indicate Chan Wrapper Element is required */
	uint8_t dfsIncludeChanWrapperIe;

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif

	bool isCiscoVendorAP;

	tSirAddIeParams addIeParams;

	uint8_t *pSchProbeRspTemplate;
	/* Beginning portion of the beacon frame to be written to TFP */
	uint8_t *pSchBeaconFrameBegin;
	/* Trailing portion of the beacon frame to be written to TFP */
	uint8_t *pSchBeaconFrameEnd;
	/* Size of the beginning portion */
	uint16_t schBeaconOffsetBegin;
	/* Size of the trailing portion */
	uint16_t schBeaconOffsetEnd;
	bool isOSENConnection;
	/*  DSCP to UP mapping for HS 2.0 */
	tSirQosMapSet QosMapSet;

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	bool bRoamSynchInProgress;
#endif

	/* Fast Transition (FT) */
	tftPEContext ftPEContext;
	bool isNonRoamReassoc;
#ifdef WLAN_FEATURE_11W
	qdf_mc_timer_t pmfComebackTimer;
	tComebackTimerInfo pmfComebackTimerInfo;
#endif /* WLAN_FEATURE_11W */
	uint8_t  is_key_installed;
	/* timer for reseting protection fileds at regular intervals */
	qdf_mc_timer_t protection_fields_reset_timer;
	void *mac_ctx;
	/*
	 * variable to store state of various protection struct like
	 * gLimOlbcParams, gLimOverlap11gParams, gLimOverlapHt20Params etc
	 */
	uint16_t old_protection_state;
	tSirMacAddr prev_ap_bssid;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	/* tells if Q2Q IE, from another MDM device in AP MCC mode was recvd */
	bool sap_advertise_avoid_ch_ie;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
#ifdef FEATURE_WLAN_ESE
	uint8_t is_ese_version_ie_present;
#endif
	uint8_t sap_dot11mc;
	bool is_vendor_specific_vhtcaps;
	uint8_t vendor_specific_vht_ie_sub_type;
	bool vendor_vht_sap;
	/* HS 2.0 Indication */
	tDot11fIEhs20vendor_ie hs20vendor_ie;
	/* flag to indicate country code in beacon */
	uint8_t country_info_present;
	uint8_t nss;
	bool add_bss_failed;
	/* To hold OBSS Scan IE Parameters */
	struct obss_scanparam obss_ht40_scanparam;
	uint8_t vdev_nss;
	/* Supported NSS is intersection of self and peer NSS */
	bool supported_nss_1x1;
	bool is_ext_caps_present;
	uint16_t beacon_tx_rate;
	uint8_t *access_policy_vendor_ie;
	uint8_t access_policy;
	bool ignore_assoc_disallowed;
	bool send_p2p_conf_frame;
	bool process_ho_fail;
	/* Number of STAs that do not support ECSA capability */
	uint8_t lim_non_ecsa_cap_num;
#ifdef WLAN_FEATURE_11AX
	bool he_capable;
	tDot11fIEhe_cap he_config;
	tDot11fIEhe_op he_op;
	uint32_t he_sta_obsspd;
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
	tDot11fIEbss_color_change he_bss_color_change;
	struct bss_color_info bss_color_info[MAX_BSS_COLOR_VALUE];
	uint8_t bss_color_changing;
#endif
#endif
	bool enable_bcast_probe_rsp;
	uint8_t ht_client_cnt;
	bool force_24ghz_in_ht20;
	bool ch_switch_in_progress;
	bool he_with_wep_tkip;
#ifdef WLAN_FEATURE_FILS_SK
	struct pe_fils_session *fils_info;
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint16_t hlp_data_len;
	uint8_t *hlp_data;
#endif
	/* previous auth frame's sequence number */
	uint16_t prev_auth_seq_num;
	struct obss_detection_cfg obss_offload_cfg;
	struct obss_detection_cfg current_obss_detection;
	bool is_session_obss_offload_enabled;
	bool is_obss_reset_timer_initialized;
	bool sae_pmk_cached;
	bool fw_roaming_started;
	bool recvd_deauth_while_roaming;
	bool recvd_disassoc_while_roaming;
	bool deauth_disassoc_rc;
	enum wmi_obss_color_collision_evt_type obss_color_collision_dec_evt;
	bool is_session_obss_color_collision_det_enabled;
} tPESession, *tpPESession;

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

/**
 * pe_create_session() - creates a new PE session given the BSSID
 *
 * @pMac:          pointer to global adapter context
 * @bssid:         BSSID of the new session
 * @sessionId:     session ID is returned here, if session is created.
 * @numSta:        number of stations
 * @bssType:       bss type of new session to do conditional memory allocation.
 *
 * This function returns the session context and the session ID if the session
 * corresponding to the passed BSSID is found in the PE session table.
 *
 * Return: ptr to the session context or NULL if session can not be created.
 */
tpPESession pe_create_session(tpAniSirGlobal pMac,
			      uint8_t *bssid,
			      uint8_t *sessionId,
			      uint16_t numSta, tSirBssType bssType);

/**
 * pe_find_session_by_bssid() - looks up the PE session given the BSSID.
 *
 * @pMac:          pointer to global adapter context
 * @bssid:         BSSID of the new session
 * @sessionId:     session ID is returned here, if session is created.
 *
 * This function returns the session context and the session ID if the session
 * corresponding to the given BSSID is found in the PE session table.
 *
 * Return: pointer to the session context or NULL if session is not found.
 */
tpPESession pe_find_session_by_bssid(tpAniSirGlobal pMac, uint8_t *bssid,
				     uint8_t *sessionId);

/**
 * pe_find_session_by_bss_idx() - looks up the PE session given the bssIdx.
 *
 * @pMac:          pointer to global adapter context
 * @bssIdx:        bss index of the session
 *
 * This function returns the session context  if the session
 * corresponding to the given bssIdx is found in the PE session table.
 *
 * Return: pointer to the session context or NULL if session is not found.
 */
tpPESession pe_find_session_by_bss_idx(tpAniSirGlobal pMac, uint8_t bssIdx);

/**
 * pe_find_session_by_peer_sta() - looks up the PE session given the Peer
 * Station Address.
 *
 * @pMac:          pointer to global adapter context
 * @sa:            Peer STA Address of the session
 * @sessionId:     session ID is returned here, if session is found.
 *
 * This function returns the session context and the session ID if the session
 * corresponding to the given destination address is found in the PE session
 * table.
 *
 * Return: pointer to the session context or NULL if session is not found.
 */
tpPESession pe_find_session_by_peer_sta(tpAniSirGlobal pMac, uint8_t *sa,
					uint8_t *sessionId);

/**
 * pe_find_session_by_session_id() - looks up the PE session given the session
 * ID.
 *
 * @pMac:          pointer to global adapter context
 * @sessionId:     session ID for which session context needs to be looked up.
 *
 * This function returns the session context  if the session corresponding to
 * the given session ID is found in the PE session table.
 *
 * Return: pointer to the session context or NULL if session is not found.
 */
tpPESession pe_find_session_by_session_id(tpAniSirGlobal pMac,
					  uint8_t sessionId);

/**
 * pe_find_session_by_bssid() - looks up the PE session given staid.
 *
 * @pMac:          pointer to global adapter context
 * @staid:         StaId of the session
 * @sessionId:     session ID is returned here, if session is found.
 *
 * This function returns the session context and the session ID if the session
 * corresponding to the given StaId is found in the PE session table.
 *
 * Return: pointer to the session context or NULL if session is not found.
 */
tpPESession pe_find_session_by_sta_id(tpAniSirGlobal pMac, uint8_t staid,
				      uint8_t *sessionId);

/**
 * pe_delete_session() - deletes the PE session given the session ID.
 *
 * @pMac:          pointer to global adapter context
 * @sessionId:     session ID to delete.
 *
 * Return: void
 */
void pe_delete_session(tpAniSirGlobal pMac, tpPESession psessionEntry);


/**
 * pe_find_session_by_sme_session_id() - looks up the PE session for given sme
 * session id
 * @mac_ctx:          pointer to global adapter context
 * @sme_session_id:   sme session id
 *
 * looks up the PE session for given sme session id
 *
 * Return: pe session entry for given sme session if found else NULL
 */
tpPESession pe_find_session_by_sme_session_id(tpAniSirGlobal mac_ctx,
					      uint8_t sme_session_id);
uint8_t pe_get_active_session_count(tpAniSirGlobal mac_ctx);
#ifdef WLAN_FEATURE_FILS_SK
/**
 * pe_delete_fils_info: API to delete fils session info
 * @session: pe session
 *
 * Return: void
 */
void pe_delete_fils_info(tpPESession session);
#endif
#endif /* #if !defined( __LIM_SESSION_H ) */
