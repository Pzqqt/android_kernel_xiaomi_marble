/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * This file lim_global.h contains the definitions exported by
 * LIM module.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_GLOBAL_H
#define __LIM_GLOBAL_H

#include "wni_api.h"
#include "sir_api.h"
#include "sir_mac_prot_def.h"
#include "sir_mac_prop_exts.h"
#include "sir_common.h"
#include "sir_debug.h"
#include "wni_cfg.h"
#include "csr_api.h"
#include "sap_api.h"
#include "dot11f.h"
#include "wma_if.h"

/* Sending Disassociate frames threshold */
#define LIM_SEND_DISASSOC_FRAME_THRESHOLD       2
#define LIM_HASH_MISS_TIMER_MS                  10000

/* Deferred Message Queue Length */
#define MAX_DEFERRED_QUEUE_LEN                  80

/* Maximum number of PS - TIM's to be sent with out wakeup from STA */
#define LIM_TIM_WAIT_COUNT_FACTOR          5

/*
 * Use this count if (LIM_TIM_WAIT_FACTOR * ListenInterval)
 * is less than LIM_MIN_TIM_WAIT_CNT
 */
#define LIM_MIN_TIM_WAIT_COUNT          50

#define GET_TIM_WAIT_COUNT(LIntrvl) \
	((LIntrvl * LIM_TIM_WAIT_COUNT_FACTOR) > LIM_MIN_TIM_WAIT_COUNT ? \
	(LIntrvl * LIM_TIM_WAIT_COUNT_FACTOR) : LIM_MIN_TIM_WAIT_COUNT)

#define LIM_MAX_CSA_IE_UPDATES    (5)

/* enums exported by LIM are as follows */

/*System role definition */
typedef enum eLimSystemRole {
	eLIM_UNKNOWN_ROLE,
	eLIM_AP_ROLE,
	eLIM_STA_IN_IBSS_ROLE,
	eLIM_STA_ROLE,
	eLIM_P2P_DEVICE_ROLE,
	eLIM_P2P_DEVICE_GO,
	eLIM_P2P_DEVICE_CLIENT,
	eLIM_NDI_ROLE
} tLimSystemRole;

/*
 * SME state definition accessible across all Sirius modules.
 * AP only states are LIM_SME_CHANNEL_SCAN_STATE &
 * LIM_SME_NORMAL_CHANNEL_SCAN_STATE.
 * Note that these states may also be present in STA
 * side too when DFS support is present for a STA in IBSS mode.
 */
typedef enum eLimSmeStates {
	eLIM_SME_OFFLINE_STATE,
	eLIM_SME_IDLE_STATE,
	eLIM_SME_SUSPEND_STATE,
	eLIM_SME_WT_SCAN_STATE,
	eLIM_SME_WT_JOIN_STATE,
	eLIM_SME_WT_AUTH_STATE,
	eLIM_SME_WT_ASSOC_STATE,
	eLIM_SME_WT_REASSOC_STATE,
	eLIM_SME_WT_REASSOC_LINK_FAIL_STATE,
	eLIM_SME_JOIN_FAILURE_STATE,
	eLIM_SME_ASSOCIATED_STATE,
	eLIM_SME_REASSOCIATED_STATE,
	eLIM_SME_LINK_EST_STATE,
	eLIM_SME_LINK_EST_WT_SCAN_STATE,
	eLIM_SME_WT_PRE_AUTH_STATE,
	eLIM_SME_WT_DISASSOC_STATE,
	eLIM_SME_WT_DEAUTH_STATE,
	eLIM_SME_WT_START_BSS_STATE,
	eLIM_SME_WT_STOP_BSS_STATE,
	eLIM_SME_NORMAL_STATE,
	eLIM_SME_CHANNEL_SCAN_STATE,
	eLIM_SME_NORMAL_CHANNEL_SCAN_STATE
} tLimSmeStates;

/*
 * MLM state definition.
 * While these states are present on AP too when it is
 * STA mode, per-STA MLM state exclusive to AP is:
 * eLIM_MLM_WT_AUTH_FRAME3.
 */
typedef enum eLimMlmStates {
	eLIM_MLM_OFFLINE_STATE,
	eLIM_MLM_IDLE_STATE,
	eLIM_MLM_WT_PROBE_RESP_STATE,
	eLIM_MLM_PASSIVE_SCAN_STATE,
	eLIM_MLM_WT_JOIN_BEACON_STATE,
	eLIM_MLM_JOINED_STATE,
	eLIM_MLM_BSS_STARTED_STATE,
	eLIM_MLM_WT_AUTH_FRAME2_STATE,
	eLIM_MLM_WT_AUTH_FRAME3_STATE,
	eLIM_MLM_WT_AUTH_FRAME4_STATE,
	eLIM_MLM_AUTH_RSP_TIMEOUT_STATE,
	eLIM_MLM_AUTHENTICATED_STATE,
	eLIM_MLM_WT_ASSOC_RSP_STATE,
	eLIM_MLM_WT_REASSOC_RSP_STATE,
	eLIM_MLM_ASSOCIATED_STATE,
	eLIM_MLM_REASSOCIATED_STATE,
	eLIM_MLM_LINK_ESTABLISHED_STATE,
	eLIM_MLM_WT_ASSOC_CNF_STATE,
	eLIM_MLM_LEARN_STATE,
	eLIM_MLM_WT_ADD_BSS_RSP_STATE,
	eLIM_MLM_WT_DEL_BSS_RSP_STATE,
	eLIM_MLM_WT_ADD_BSS_RSP_ASSOC_STATE,
	eLIM_MLM_WT_ADD_BSS_RSP_REASSOC_STATE,
	eLIM_MLM_WT_ADD_BSS_RSP_PREASSOC_STATE,
	eLIM_MLM_WT_ADD_STA_RSP_STATE,
	eLIM_MLM_WT_DEL_STA_RSP_STATE,
	/*
	 * MLM goes to this state when LIM initiates DELETE_STA
	 * as processing of Assoc req because the entry already exists.
	 * LIM comes out of this state when DELETE_STA response from
	 * HAL is received. LIM needs to maintain this state so that ADD_STA
	 * can be issued while processing DELETE_STA response from HAL.
	 */
	eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE,
	eLIM_MLM_WT_SET_BSS_KEY_STATE,
	eLIM_MLM_WT_SET_STA_KEY_STATE,
	eLIM_MLM_WT_SET_STA_BCASTKEY_STATE,
	eLIM_MLM_WT_SET_MIMOPS_STATE,
	eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE,
	eLIM_MLM_WT_FT_REASSOC_RSP_STATE,
	eLIM_MLM_P2P_LISTEN_STATE,
} tLimMlmStates;

/* 11h channel quiet states */

/*
 * This enum indicates in which state the device is in
 * when it receives quiet element in beacon or probe-response.
 * The default quiet state of the device is always INIT
 * eLIM_QUIET_BEGIN - When Quiet period is started
 * eLIM_QUIET_CHANGED - When Quiet period is updated
 * eLIM_QUIET_RUNNING - Between two successive Quiet updates
 * eLIM_QUIET_END - When quiet period ends
 */
typedef enum eLimQuietStates {
	eLIM_QUIET_INIT,
	eLIM_QUIET_BEGIN,
	eLIM_QUIET_CHANGED,
	eLIM_QUIET_RUNNING,
	eLIM_QUIET_END
} tLimQuietStates;

/* 11h channel switch states */

/*
 * This enum indicates in which state the channel-swith
 * is presently operating.
 * eLIM_11H_CHANSW_INIT - Default state
 * eLIM_11H_CHANSW_RUNNING - When channel switch is running
 * eLIM_11H_CHANSW_END - After channel switch is complete
 */
typedef enum eLimDot11hChanSwStates {
	eLIM_11H_CHANSW_INIT,
	eLIM_11H_CHANSW_RUNNING,
	eLIM_11H_CHANSW_END
} tLimDot11hChanSwStates;


/* WLAN_SUSPEND_LINK Related */
typedef void (*SUSPEND_RESUME_LINK_CALLBACK)(tpAniSirGlobal pMac,
					     QDF_STATUS status,
					     uint32_t *data);

/* LIM to HAL SCAN Management Message Interface states */
typedef enum eLimHalScanState {
	eLIM_HAL_IDLE_SCAN_STATE,
	eLIM_HAL_INIT_SCAN_WAIT_STATE,
	eLIM_HAL_START_SCAN_WAIT_STATE,
	eLIM_HAL_END_SCAN_WAIT_STATE,
	eLIM_HAL_FINISH_SCAN_WAIT_STATE,
	eLIM_HAL_INIT_LEARN_WAIT_STATE,
	eLIM_HAL_START_LEARN_WAIT_STATE,
	eLIM_HAL_END_LEARN_WAIT_STATE,
	eLIM_HAL_FINISH_LEARN_WAIT_STATE,
	eLIM_HAL_SCANNING_STATE,
/* WLAN_SUSPEND_LINK Related */
	eLIM_HAL_SUSPEND_LINK_WAIT_STATE,
	eLIM_HAL_SUSPEND_LINK_STATE,
	eLIM_HAL_RESUME_LINK_WAIT_STATE,
/* end WLAN_SUSPEND_LINK Related */
} tLimLimHalScanState;

/* MLM Req/Cnf structure definitions */
typedef struct sLimMlmAuthReq {
	tSirMacAddr peerMacAddr;
	tAniAuthType authType;
	uint32_t authFailureTimeout;
	uint8_t sessionId;
} tLimMlmAuthReq, *tpLimMlmAuthReq;

typedef struct sLimMlmJoinReq {
	uint32_t joinFailureTimeout;
	tSirMacRateSet operationalRateSet;
	uint8_t sessionId;
	tSirBssDescription bssDescription;
} tLimMlmJoinReq, *tpLimMlmJoinReq;

typedef struct sLimMlmScanReq {
	tSirBssType bssType;
	tSirMacAddr bssId;
	tSirMacSSid ssId[SIR_SCAN_MAX_NUM_SSID];
	tSirScanType scanType;
	uint32_t minChannelTime;
	uint32_t maxChannelTime;
	uint32_t dot11mode;
	/* Number of SSIDs to scan(send Probe request) */
	uint8_t numSsid;

	bool p2pSearch;
	uint16_t uIEFieldLen;
	uint16_t uIEFieldOffset;

	uint8_t sessionId;
	/* channelList MUST be the last field of this structure */
	tSirChannelList channelList;
	/*-----------------------------
	   tLimMlmScanReq....
	   -----------------------------
	   uIEFiledLen
	   -----------------------------
	   uIEFiledOffset               ----+
	   -----------------------------    |
	   channelList.numChannels          |
	   -----------------------------    |
	   ... variable size up to          |
	   channelNumber[numChannels-1]     |
	   This can be zero, if             |
	   numChannel is zero.              |
	   ----------------------------- <--+
	   ... variable size uIEFiled
	   up to uIEFieldLen (can be 0)
	   -----------------------------*/
} tLimMlmScanReq, *tpLimMlmScanReq;

typedef struct tLimScanResultNode tLimScanResultNode;
struct tLimScanResultNode {
	tLimScanResultNode *next;
	tSirBssDescription bssDescription;
};

#ifdef FEATURE_OEM_DATA_SUPPORT

/* OEM Data related structure definitions */
typedef struct sLimMlmOemDataReq {
	struct qdf_mac_addr selfMacAddr;
	uint32_t data_len;
	uint8_t *data;
} tLimMlmOemDataReq, *tpLimMlmOemDataReq;

typedef struct sLimMlmOemDataRsp {
	bool target_rsp;
	uint32_t rsp_len;
	uint8_t *oem_data_rsp;
} tLimMlmOemDataRsp, *tpLimMlmOemDataRsp;
#endif

/* Pre-authentication structure definition */
typedef struct tLimPreAuthNode {
	struct tLimPreAuthNode *next;
	tSirMacAddr peerMacAddr;
	tAniAuthType authType;
	tLimMlmStates mlmState;
	uint8_t authNodeIdx;
	uint8_t challengeText[SIR_MAC_AUTH_CHALLENGE_LENGTH];
	uint8_t fTimerStarted:1;
	uint8_t fSeen:1;
	uint8_t fFree:1;
	uint8_t rsvd:5;
	TX_TIMER timer;
	uint16_t seq_num;
	unsigned long timestamp;
} tLimPreAuthNode, *tpLimPreAuthNode;

/* Pre-authentication table definition */
typedef struct tLimPreAuthTable {
	uint32_t numEntry;
	tLimPreAuthNode **pTable;
} tLimPreAuthTable, *tpLimPreAuthTable;

/* / Per STA context structure definition */
typedef struct sLimMlmStaContext {
	tLimMlmStates mlmState;
	tAniAuthType authType;
	uint16_t listenInterval;
	tSirMacCapabilityInfo capabilityInfo;
	tSirMacPropRateSet propRateSet;
	tSirMacReasonCodes disassocReason;
	uint16_t cleanupTrigger;

	tSirResultCodes resultCode;
	uint16_t protStatusCode;

	uint8_t subType:1;      /* Indicates ASSOC (0) or REASSOC (1) */
	uint8_t updateContext:1;
	uint8_t schClean:1;
	/* 802.11n HT Capability in Station: Enabled 1 or DIsabled 0 */
	uint8_t htCapability:1;
	uint8_t vhtCapability:1;
} tLimMlmStaContext, *tpLimMlmStaContext;

/* Structure definition to hold deferred messages queue parameters */
typedef struct sLimDeferredMsgQParams {
	tSirMsgQ deferredQueue[MAX_DEFERRED_QUEUE_LEN];
	uint16_t size;
	uint16_t read;
	uint16_t write;
} tLimDeferredMsgQParams, *tpLimDeferredMsgQParams;

typedef struct sCfgProtection {
	uint32_t overlapFromlla:1;
	uint32_t overlapFromllb:1;
	uint32_t overlapFromllg:1;
	uint32_t overlapHt20:1;
	uint32_t overlapNonGf:1;
	uint32_t overlapLsigTxop:1;
	uint32_t overlapRifs:1;
	uint32_t overlapOBSS:1; /* added for obss */
	uint32_t fromlla:1;
	uint32_t fromllb:1;
	uint32_t fromllg:1;
	uint32_t ht20:1;
	uint32_t nonGf:1;
	uint32_t lsigTxop:1;
	uint32_t rifs:1;
	uint32_t obss:1;        /* added for Obss */
} tCfgProtection, *tpCfgProtection;

typedef enum eLimProtStaCacheType {
	eLIM_PROT_STA_CACHE_TYPE_INVALID,
	eLIM_PROT_STA_CACHE_TYPE_llB,
	eLIM_PROT_STA_CACHE_TYPE_llG,
	eLIM_PROT_STA_CACHE_TYPE_HT20
} tLimProtStaCacheType;

typedef struct sCacheParams {
	uint8_t active;
	tSirMacAddr addr;
	tLimProtStaCacheType protStaCacheType;

} tCacheParams, *tpCacheParams;

#define LIM_PROT_STA_OVERLAP_CACHE_SIZE    HAL_NUM_ASSOC_STA
#define LIM_PROT_STA_CACHE_SIZE            HAL_NUM_ASSOC_STA

typedef struct sLimProtStaParams {
	uint8_t numSta;
	uint8_t protectionEnabled;
} tLimProtStaParams, *tpLimProtStaParams;

typedef struct sLimNoShortParams {
	uint8_t numNonShortPreambleSta;
	tCacheParams staNoShortCache[LIM_PROT_STA_CACHE_SIZE];
} tLimNoShortParams, *tpLimNoShortParams;

typedef struct sLimNoShortSlotParams {
	uint8_t numNonShortSlotSta;
	tCacheParams staNoShortSlotCache[LIM_PROT_STA_CACHE_SIZE];
} tLimNoShortSlotParams, *tpLimNoShortSlotParams;

typedef struct tLimIbssPeerNode tLimIbssPeerNode;
struct tLimIbssPeerNode {
	tLimIbssPeerNode *next;
	tSirMacAddr peerMacAddr;
	uint8_t extendedRatesPresent:1;
	uint8_t edcaPresent:1;
	uint8_t wmeEdcaPresent:1;
	uint8_t wmeInfoPresent:1;
	uint8_t htCapable:1;
	uint8_t vhtCapable:1;
	uint8_t rsvd:2;
	uint8_t htSecondaryChannelOffset;
	tSirMacCapabilityInfo capabilityInfo;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;
	uint8_t supportedMCSSet[SIZE_OF_SUPPORTED_MCS_SET];
	tSirMacEdcaParamSetIE edcaParams;
	uint8_t erpIePresent;

	/* HT Capabilities of IBSS Peer */
	uint8_t htGreenfield;
	uint8_t htShortGI40Mhz;
	uint8_t htShortGI20Mhz;

	/* DSSS/CCK at 40 MHz: Enabled 1 or Disabled */
	uint8_t htDsssCckRate40MHzSupport;

	/* MIMO Power Save */
	tSirMacHTMIMOPowerSaveState htMIMOPSState;

	/* */
	/* A-MPDU Density */
	/* 000 - No restriction */
	/* 001 - 1/8 usec */
	/* 010 - 1/4 usec */
	/* 011 - 1/2 usec */
	/* 100 - 1 usec */
	/* 101 - 2 usec */
	/* 110 - 4 usec */
	/* 111 - 8 usec */
	/* */
	uint8_t htAMpduDensity;

	/* Maximum Rx A-MPDU factor */
	uint8_t htMaxRxAMpduFactor;

	/* Set to 0 for 3839 octets */
	/* Set to 1 for 7935 octets */
	uint8_t htMaxAmsduLength;

	/* */
	/* Recommended Tx Width Set */
	/* 0 - use 20 MHz channel (control channel) */
	/* 1 - use 40 Mhz channel */
	/* */
	uint8_t htSupportedChannelWidthSet;

	uint8_t htLdpcCapable;

	uint8_t beaconHBCount;
	uint8_t heartbeatFailure;

	uint8_t *beacon;        /* Hold beacon to be sent to HDD/CSR */
	uint16_t beaconLen;

	tDot11fIEVHTCaps VHTCaps;
	uint8_t vhtSupportedChannelWidthSet;
	uint8_t vhtBeamFormerCapable;
	/*
	 * Peer Atim Info
	 */
	uint8_t atimIePresent;
	uint32_t peerAtimWindowLength;
};

/* Enums used for channel switching. */
typedef enum eLimChannelSwitchState {
	eLIM_CHANNEL_SWITCH_IDLE,
	eLIM_CHANNEL_SWITCH_PRIMARY_ONLY,
	eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY
} tLimChannelSwitchState;

/* Channel Switch Info */
typedef struct sLimChannelSwitchInfo {
	tLimChannelSwitchState state;
	uint8_t primaryChannel;
	uint8_t ch_center_freq_seg0;
	uint8_t ch_center_freq_seg1;
	uint8_t sec_ch_offset;
	enum phy_ch_width ch_width;
	int8_t switchCount;
	uint32_t switchTimeoutValue;
	uint8_t switchMode;
} tLimChannelSwitchInfo, *tpLimChannelSwitchInfo;

typedef struct sLimOperatingModeInfo {
	uint8_t present;
	uint8_t chanWidth:2;
	uint8_t reserved:2;
	uint8_t rxNSS:3;
	uint8_t rxNSSType:1;
} tLimOperatingModeInfo, *tpLimOperatingModeInfo;

typedef struct sLimWiderBWChannelSwitch {
	uint8_t newChanWidth;
	uint8_t newCenterChanFreq0;
	uint8_t newCenterChanFreq1;
} tLimWiderBWChannelSwitchInfo, *tpLimWiderBWChannelSwitchInfo;

/* Enums used when stopping the Tx. */
typedef enum eLimQuietTxMode {
	/* Stop/resume transmission of all stations,Uses the global flag */
	eLIM_TX_ALL = 0,
	/*
	 * Stops/resumes the transmission of specific stations identified
	 * by staId.
	 */
	eLIM_TX_STA,
	/* Stops/resumes the transmission of all the packets in BSS */
	eLIM_TX_BSS,
	/*
	 * Stops/resumes the transmission of all packets except beacons in BSS
	 * This is used when radar is detected in the current operating channel.
	 * Beacon has to be sent to notify the stations associated about the
	 * scheduled channel switch
	 */
	eLIM_TX_BSS_BUT_BEACON
} tLimQuietTxMode;

typedef enum eLimControlTx {
	eLIM_RESUME_TX = 0,
	eLIM_STOP_TX
} tLimControlTx;

/* -------------------------------------------------------------------- */

typedef struct sLimTspecInfo {
	/* 0==free, else used */
	uint8_t inuse;
	/* index in list */
	uint8_t idx;
	tSirMacAddr staAddr;
	uint16_t assocId;
	tSirMacTspecIE tspec;
	/* number of Tclas elements */
	uint8_t numTclas;
	tSirTclasInfo tclasInfo[SIR_MAC_TCLASIE_MAXNUM];
	uint8_t tclasProc;
	/* tclassProc is valid only if this is set to 1. */
	uint8_t tclasProcPresent:1;
} qdf_packed tLimTspecInfo, *tpLimTspecInfo;

typedef struct sLimAdmitPolicyInfo {
	/* admit control policy type */
	uint8_t type;
	/* oversubscription factor : 0 means nothing is allowed */
	uint8_t bw_factor;
	/* valid only when 'type' is set BW_FACTOR */
} tLimAdmitPolicyInfo, *tpLimAdmitPolicyInfo;

typedef enum eLimWscEnrollState {
	eLIM_WSC_ENROLL_NOOP,
	eLIM_WSC_ENROLL_BEGIN,
	eLIM_WSC_ENROLL_IN_PROGRESS,
	eLIM_WSC_ENROLL_END
} tLimWscEnrollState;

#define WSC_PASSWD_ID_PUSH_BUTTON         (0x0004)

typedef struct sLimWscIeInfo {
	bool apSetupLocked;
	bool selectedRegistrar;
	uint16_t selectedRegistrarConfigMethods;
	tLimWscEnrollState wscEnrollmentState;
	tLimWscEnrollState probeRespWscEnrollmentState;
	uint8_t reqType;
	uint8_t respType;
} tLimWscIeInfo, *tpLimWscIeInfo;

/* maximum number of tspec's supported */
#define LIM_NUM_TSPEC_MAX      15

/* structure to hold all 11h specific data */
typedef struct sLimSpecMgmtInfo {
	tLimQuietStates quietState;
	uint32_t quietCount;
	/* This is in units of system TICKS */
	uint32_t quietDuration;
	/* This is in units of TU, for over the air transmission */
	uint32_t quietDuration_TU;
	/* After this timeout, actual quiet starts */
	uint32_t quietTimeoutValue;
	/* Used on AP, if quiet is enabled during learning */
	bool fQuietEnabled;
	tLimDot11hChanSwStates dot11hChanSwState;
	/* Radar detected in cur oper chan on AP */
	bool fRadarDetCurOperChan;
	/* Whether radar interrupt has been configured */
	bool fRadarIntrConfigured;
} tLimSpecMgmtInfo, *tpLimSpecMgmtInfo;

#ifdef FEATURE_WLAN_TDLS
/*
 * Peer info needed for TDLS setup..
 */
typedef struct tLimTDLSPeerSta {
	struct tLimTDLSPeerSta *next;
	uint8_t dialog;
	tSirMacAddr peerMac;
	tSirMacCapabilityInfo capabilityInfo;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;
	tSirMacQosCapabilityStaIE qosCaps;
	tSirMacEdcaParamSetIE edcaParams;
	uint8_t mcsSet[SIZE_OF_SUPPORTED_MCS_SET];
	uint8_t tdls_bIsResponder;
	/* HT Capabilties */
	tDot11fIEHTCaps tdlsPeerHTCaps;
	tDot11fIEExtCap tdlsPeerExtCaps;
	uint8_t tdls_flags;
	uint8_t tdls_link_state;
	uint8_t tdls_prev_link_state;
	uint8_t tdls_sessionId;
	uint8_t ExtRatesPresent;
	TX_TIMER gLimTdlsLinkSetupRspTimeoutTimer;
	TX_TIMER gLimTdlsLinkSetupCnfTimeoutTimer;
} tLimTdlsLinkSetupPeer, *tpLimTdlsLinkSetupPeer;

typedef struct tLimTdlsLinkSetupInfo {
	tLimTdlsLinkSetupPeer *tdlsLinkSetupList;
	uint8_t num_tdls_peers;
	uint8_t tdls_flags;
	uint8_t tdls_state;
	uint8_t tdls_prev_state;
} tLimTdlsLinkSetupInfo, *tpLimTdlsLinkSetupInfo;

typedef enum tdlsLinkMode {
	TDLS_LINK_MODE_BG,
	TDLS_LINK_MODE_N,
	TDLS_LINK_MODE_AC,
	TDLS_LINK_MODE_NONE
} eLimTdlsLinkMode;
#endif /* FEATURE_WLAN_TDLS */

#endif
