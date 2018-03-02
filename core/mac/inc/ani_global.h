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

#ifndef _ANIGLOBAL_H
#define _ANIGLOBAL_H

#include "qdf_types.h"
#include "sir_common.h"
#include "ani_system_defs.h"
#include "sys_def.h"
#include "dph_global.h"
#include "lim_global.h"
#include "sch_global.h"
#include "sys_global.h"
#include "cfg_global.h"
#include "sir_api.h"

#include "csr_api.h"
#include "sme_ft_api.h"
#include "csr_support.h"
#include "sme_internal.h"
#include "sap_api.h"
#include "csr_internal.h"

#include "sme_rrm_internal.h"
#include "rrm_global.h"

#include <lim_ft_defs.h>
#include "wlan_objmgr_psoc_obj.h"

/* Check if this definition can actually move here from halInternal.h even for Volans. In that case */
/* this featurization can be removed. */
#define PMAC_STRUCT(_hHal)  ((tpAniSirGlobal)_hHal)

#define ANI_DRIVER_TYPE(pMac)     (((tpAniSirGlobal)(pMac))->gDriverType)

#define IS_MIRACAST_SESSION_PRESENT(pMac)     (((tpAniSirGlobal)(pMac))->fMiracastSessionPresent ? 1 : 0)
/* ------------------------------------------------------------------- */
/* Bss Qos Caps bit map definition */
#define LIM_BSS_CAPS_OFFSET_HCF 0
#define LIM_BSS_CAPS_OFFSET_WME 1
#define LIM_BSS_CAPS_OFFSET_WSM 2

#define LIM_BSS_CAPS_HCF (1 << LIM_BSS_CAPS_OFFSET_HCF)
#define LIM_BSS_CAPS_WME (1 << LIM_BSS_CAPS_OFFSET_WME)
#define LIM_BSS_CAPS_WSM (1 << LIM_BSS_CAPS_OFFSET_WSM)

/* cap should be one of HCF/WME/WSM */
#define LIM_BSS_CAPS_GET(cap, val) (((val) & (LIM_BSS_CAPS_ ## cap)) >> LIM_BSS_CAPS_OFFSET_ ## cap)
#define LIM_BSS_CAPS_SET(cap, val) ((val) |= (LIM_BSS_CAPS_ ## cap))
#define LIM_BSS_CAPS_CLR(cap, val) ((val) &= (~(LIM_BSS_CAPS_ ## cap)))

/* 40 beacons per heart beat interval is the default + 1 to count the rest */
#define MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL 41

/* max number of legacy bssid we can store during scan on one channel */
#define MAX_NUM_LEGACY_BSSID_PER_CHANNEL    10

#define P2P_WILDCARD_SSID "DIRECT-"     /* TODO Put it in proper place; */
#define P2P_WILDCARD_SSID_LEN 7

#ifdef WLAN_FEATURE_CONCURRENT_P2P
#define MAX_NO_OF_P2P_SESSIONS  5
#endif /* WLAN_FEATURE_CONCURRENT_P2P */

#define SPACE_ASCII_VALUE  32

#define WLAN_HOST_SEQ_NUM_MIN                           2048
#define WLAN_HOST_SEQ_NUM_MAX                           4095
#define LOW_SEQ_NUM_MASK                                0x000F
#define HIGH_SEQ_NUM_MASK                               0x0FF0
#define HIGH_SEQ_NUM_OFFSET                             4

/* vendor element ID */
#define IE_EID_VENDOR        (221) /* 0xDD */
#define IE_LEN_SIZE          (1)
#define IE_EID_SIZE          (1)
/* Minimum size of vendor IE = 3 bytes of oui_data + 1 byte of data */
#define IE_VENDOR_OUI_SIZE   (4)

/*
 * NSS cfg bit definition.
 * STA          BIT[0:1]
 * SAP          BIT[2:3]
 * P2P_GO       BIT[4:5]
 * P2P_CLIENT   BIT[6:7]
 * IBSS         BIT[8:9]
 * TDLS         BIT[10:11]
 * P2P_DEVICE   BIT[12:13]
 * OCB          BIT[14:15]
 */

#define CFG_STA_NSS(_x)     ((((_x) >> 0) & 0x3) ? (((_x) >> 0) & 0x3) : 1)
#define CFG_SAP_NSS(_x)     ((((_x) >> 2) & 0x3) ? (((_x) >> 2) & 0x3) : 1)
#define CFG_P2P_GO_NSS(_x)  ((((_x) >> 4) & 0x3) ? (((_x) >> 4) & 0x3) : 1)
#define CFG_P2P_CLI_NSS(_x) ((((_x) >> 6) & 0x3) ? (((_x) >> 6) & 0x3) : 1)
#define CFG_IBSS_NSS(_x)    ((((_x) >> 8) & 0x3) ? (((_x) >> 8) & 0x3) : 1)
#define CFG_TDLS_NSS(_x)    ((((_x) >> 10) & 0x3) ? (((_x) >> 10) & 0x3) : 1)
#define CFG_P2P_DEV_NSS(_x) ((((_x) >> 12) & 0x3) ? (((_x) >> 12) & 0x3) : 1)
#define CFG_OCB_NSS(_x)     ((((_x) >> 14) & 0x3) ? (((_x) >> 14) & 0x3) : 1)

/**
 * enum log_event_type - Type of event initiating bug report
 * @WLAN_LOG_TYPE_NON_FATAL: Non fatal event
 * @WLAN_LOG_TYPE_FATAL: Fatal event
 *
 * Enum indicating the type of event that is initiating the bug report
 */
enum log_event_type {
	WLAN_LOG_TYPE_NON_FATAL,
	WLAN_LOG_TYPE_FATAL,
};

/**
 * enum log_event_indicator - Module triggering bug report
 * @WLAN_LOG_INDICATOR_UNUSED: Unused
 * @WLAN_LOG_INDICATOR_FRAMEWORK: Framework triggers bug report
 * @WLAN_LOG_INDICATOR_HOST_DRIVER: Host driver triggers bug report
 * @WLAN_LOG_INDICATOR_FIRMWARE: FW initiates bug report
 * @WLAN_LOG_INDICATOR_HOST_ONLY: Host triggers fatal event bug report
 *
 * Enum indicating the module that triggered the bug report
 */
enum log_event_indicator {
	WLAN_LOG_INDICATOR_UNUSED,
	WLAN_LOG_INDICATOR_FRAMEWORK,
	WLAN_LOG_INDICATOR_HOST_DRIVER,
	WLAN_LOG_INDICATOR_FIRMWARE,
	WLAN_LOG_INDICATOR_HOST_ONLY,
};

/**
 * enum log_event_host_reason_code - Reason code for bug report
 * @WLAN_LOG_REASON_CODE_UNUSED: Unused
 * @WLAN_LOG_REASON_ROAM_FAIL: Driver initiated roam has failed
 * @WLAN_LOG_REASON_DATA_STALL: Unable to send/receive data due to low resource
 * scenario for a prolonged period
 * @WLAN_LOG_REASON_SME_COMMAND_STUCK: SME command is stuck in SME active queue
 * @WLAN_LOG_REASON_QUEUE_FULL: Defer queue becomes full for a prolonged period
 * @WLAN_LOG_REASON_POWER_COLLAPSE_FAIL: Unable to allow apps power collapse
 * for a prolonged period
 * @WLAN_LOG_REASON_MALLOC_FAIL: Memory allocation Fails
 * @WLAN_LOG_REASON_VOS_MSG_UNDER_RUN: VOS Core runs out of message wrapper
 * @WLAN_LOG_REASON_HDD_TIME_OUT: Wait for event Timeout in HDD layer
   @WLAN_LOG_REASON_SME_OUT_OF_CMD_BUFL sme out of cmd buffer
 * @WLAN_LOG_REASON_NO_SCAN_RESULTS: no scan results to report from HDD
 * This enum contains the different reason codes for bug report
 * @WLAN_LOG_REASON_SCAN_NOT_ALLOWED: scan not allowed due to connection states
 * @WLAN_LOG_REASON_HB_FAILURE: station triggered heart beat failure with AP
 * @WLAN_LOG_REASON_ROAM_HO_FAILURE: Handover failed during LFR3 roaming
 * @WLAN_LOG_REASON_DISCONNECT: Disconnect because of some failure
 */
enum log_event_host_reason_code {
	WLAN_LOG_REASON_CODE_UNUSED,
	WLAN_LOG_REASON_ROAM_FAIL,
	WLAN_LOG_REASON_DATA_STALL,
	WLAN_LOG_REASON_SME_COMMAND_STUCK,
	WLAN_LOG_REASON_QUEUE_FULL,
	WLAN_LOG_REASON_POWER_COLLAPSE_FAIL,
	WLAN_LOG_REASON_MALLOC_FAIL,
	WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
	WLAN_LOG_REASON_HDD_TIME_OUT,
	WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF,
	WLAN_LOG_REASON_NO_SCAN_RESULTS,
	WLAN_LOG_REASON_SCAN_NOT_ALLOWED,
	WLAN_LOG_REASON_HB_FAILURE,
	WLAN_LOG_REASON_ROAM_HO_FAILURE,
	WLAN_LOG_REASON_DISCONNECT
};


/**
 * enum userspace_log_level - Log level at userspace
 * @LOG_LEVEL_NO_COLLECTION: verbose_level 0 corresponds to no collection
 * @LOG_LEVEL_NORMAL_COLLECT: verbose_level 1 correspond to normal log level,
 * with minimal user impact. this is the default value
 * @LOG_LEVEL_ISSUE_REPRO: verbose_level 2 are enabled when user is lazily
 * trying to reproduce a problem, wifi performances and power can be impacted
 * but device should not otherwise be significantly impacted
 * @LOG_LEVEL_ACTIVE: verbose_level 3+ are used when trying to
 * actively debug a problem
 *
 * Various log levels defined in the userspace for logging applications
 */
enum userspace_log_level {
	LOG_LEVEL_NO_COLLECTION,
	LOG_LEVEL_NORMAL_COLLECT,
	LOG_LEVEL_ISSUE_REPRO,
	LOG_LEVEL_ACTIVE,
};

/**
 * enum wifi_driver_log_level - Log level defined in the driver for logging
 * @WLAN_LOG_LEVEL_OFF: No logging
 * @WLAN_LOG_LEVEL_NORMAL: Default logging
 * @WLAN_LOG_LEVEL_REPRO: Normal debug level
 * @WLAN_LOG_LEVEL_ACTIVE: Active debug level
 *
 * Log levels defined for logging by the wifi driver
 */
enum wifi_driver_log_level {
	WLAN_LOG_LEVEL_OFF,
	WLAN_LOG_LEVEL_NORMAL,
	WLAN_LOG_LEVEL_REPRO,
	WLAN_LOG_LEVEL_ACTIVE,
};

/**
 * enum wifi_logging_ring_id - Ring id of logging entities
 * @RING_ID_WAKELOCK:         Power events ring id
 * @RING_ID_CONNECTIVITY:     Connectivity event ring id
 * @RING_ID_PER_PACKET_STATS: Per packet statistic ring id
 * @RING_ID_DRIVER_DEBUG:     Driver debug messages ring id
 * @RING_ID_FIRMWARE_DEBUG:   Firmware debug messages ring id
 *
 * This enum has the ring id values of logging rings
 */
enum wifi_logging_ring_id {
	RING_ID_WAKELOCK,
	RING_ID_CONNECTIVITY,
	RING_ID_PER_PACKET_STATS,
	RING_ID_DRIVER_DEBUG,
	RING_ID_FIRMWARE_DEBUG,
};

/* ------------------------------------------------------------------- */
/* Change channel generic scheme */
typedef void (*CHANGE_CHANNEL_CALLBACK)(tpAniSirGlobal pMac, QDF_STATUS status,
					uint32_t *data,
					tpPESession psessionEntry);

/* / LIM global definitions */
typedef struct sAniSirLimIbss {
	void *pHdr;
	void *pBeacon;
} tAniSirLimIbss;

typedef struct sDialogueToken {
	/* bytes 0-3 */
	uint16_t assocId;
	uint8_t token;
	uint8_t rsvd1;
	/* Bytes 4-7 */
	uint16_t tid;
	uint8_t rsvd2[2];

	struct sDialogueToken *next;
} tDialogueToken, *tpDialogueToken;

typedef struct sLimTimers {
	/* TIMERS IN LIM ARE NOT SUPPOSED TO BE ZEROED OUT DURING RESET. */
	/* DURING lim_initialize DONOT ZERO THEM OUT. */

/* STA SPECIFIC TIMERS */

	TX_TIMER gLimPreAuthClnupTimer;

	/* Association related timers */
	TX_TIMER gLimAssocFailureTimer;
	TX_TIMER gLimReassocFailureTimer;

	/* / Wait for Probe after Heartbeat failure timer on STA */
	TX_TIMER gLimProbeAfterHBTimer;

	/* Authentication related timers */
	TX_TIMER gLimAuthFailureTimer;

	/* Join Failure timeout on STA */
	TX_TIMER gLimJoinFailureTimer;

	/* CNF_WAIT timer */
	TX_TIMER *gpLimCnfWaitTimer;

	TX_TIMER gLimAddtsRspTimer;     /* max wait for a response */

	/* Update OLBC Cache Timer */
	TX_TIMER gLimUpdateOlbcCacheTimer;

	TX_TIMER gLimChannelSwitchTimer;
	/* This TIMER is started on the STA, as indicated by the */
	/* AP in its Quiet BSS IE, for the specified interval */
	TX_TIMER gLimQuietTimer;
	/* This TIMER is started on the AP, prior to the AP going */
	/* into LEARN mode */
	/* This TIMER is started on the STA, for the specified */
	/* quiet duration */
	TX_TIMER gLimQuietBssTimer;

	TX_TIMER gLimFTPreAuthRspTimer;

#ifdef FEATURE_WLAN_ESE
	TX_TIMER gLimEseTsmTimer;
#endif
	TX_TIMER gLimPeriodicJoinProbeReqTimer;
	TX_TIMER gLimDisassocAckTimer;
	TX_TIMER gLimDeauthAckTimer;
	/* This timer is started when single shot NOA insert msg is sent to FW for scan in P2P GO mode */
	TX_TIMER gLimP2pSingleShotNoaInsertTimer;
	/* This timer is used to convert active channel to
	 * passive channel when there is no beacon
	 * for a period of time on a particular DFS channel
	 */
	TX_TIMER gLimActiveToPassiveChannelTimer;
	TX_TIMER g_lim_periodic_auth_retry_timer;

	/* SAE authentication related timer */
	TX_TIMER sae_auth_timer;

/* ********************TIMER SECTION ENDS************************************************** */
/* ALL THE FIELDS BELOW THIS CAN BE ZEROED OUT in lim_initialize */
/* **************************************************************************************** */

} tLimTimers;

typedef struct {
	void *pMlmDisassocReq;
	void *pMlmDeauthReq;
} tLimDisassocDeauthCnfReq;

typedef struct sAniSirLim {
	/* ////////////////////////////////////     TIMER RELATED START /////////////////////////////////////////// */

	tLimTimers limTimers;
	/* / Flag to track if LIM timers are created or not */
	uint32_t gLimTimersCreated;

	/* ////////////////////////////////////     TIMER RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////     SCAN/LEARN RELATED START /////////////////////////////////////////// */
	/**
	 * This flag when set, will use scan mode instead of
	 * Learn mode on BP/AP. By default this flag is set
	 * to true until HIF getting stuck in 0x800 state is
	 * debugged.
	 */
	uint32_t gLimUseScanModeForLearnMode;

	/**
	 * This is useful for modules other than LIM
	 * to see if system is in scan/learn mode or not
	 */
	uint32_t gLimSystemInScanLearnMode;

	/* Scan related globals on STA */
	uint8_t gLimReturnAfterFirstMatch;
	uint8_t gLim24Band11dScanDone;
	uint8_t gLim50Band11dScanDone;
	uint8_t gLimReturnUniqueResults;

	/* / Place holder for current channel ID */
	/* / being scanned */
	uint32_t gLimCurrentScanChannelId;

	/* Hold onto SCAN criteria */
	/* The below is used in P2P GO case when we need to defer processing SME Req
	 * to LIM and insert NOA first and process SME req once SNOA is started
	 */
	uint16_t gDeferMsgTypeForNOA;
	uint32_t *gpDefdSmeMsgForNOA;

	/* Used to store the list of legacy bss sta detected during scan on one channel */
	uint16_t gLimRestoreCBNumScanInterval;
	uint16_t gLimRestoreCBCount;
	tSirMacAddr gLimLegacyBssidList[MAX_NUM_LEGACY_BSSID_PER_CHANNEL];

	/* abort scan is used to abort an on-going scan */
	uint8_t abortScan;
	tLimScanChnInfo scanChnInfo;

	struct lim_scan_channel_status scan_channel_status;

	/* ////////////////////////////////////     SCAN/LEARN RELATED START /////////////////////////////////////////// */
	tSirMacAddr gSelfMacAddr;       /* added for BT-AMP Support */

	/* ////////////////////////////////////////     BSS RELATED END /////////////////////////////////////////// */
	/* Place holder for StartBssReq message */
	/* received by SME state machine */

	uint8_t gLimCurrentBssUapsd;

	/* This is used for testing sta legacy bss detect feature */
	uint8_t gLimForceNoPropIE;

	/* */
	/* Store the BSS Index returned by HAL during */
	/* WMA_ADD_BSS_RSP here. */
	/* */

	/* For now: */
	/* This will be used during WMA_SET_BSSKEY_REQ in */
	/* order to set the GTK */
	/* Later: */
	/* There could be other interfaces needing this info */
	/* */

	/* */
	/* Due to the asynchronous nature of the interface */
	/* between PE <-> HAL, some transient information */
	/* like this needs to be cached. */
	/* This is cached upon receipt of eWNI_SME_SETCONTEXT_REQ. */
	/* This is released while posting LIM_MLM_SETKEYS_CNF */
	/* */
	void *gpLimMlmSetKeysReq;

	/* ////////////////////////////////////////     BSS RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////////     IBSS RELATED START /////////////////////////////////////////// */
	/* This indicates whether this STA coalesced and adapter to peer's capabilities or not. */
	uint8_t gLimIbssCoalescingHappened;

	/* / Definition for storing IBSS peers BSS description */
	tLimIbssPeerNode *gLimIbssPeerList;
	uint32_t gLimNumIbssPeers;
	uint32_t ibss_retry_cnt;

	/* ibss info - params for which ibss to join while coalescing */
	tAniSirLimIbss ibssInfo;

	/* ////////////////////////////////////////     IBSS RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////////     STATS/COUNTER RELATED START /////////////////////////////////////////// */

	uint16_t maxStation;
	uint16_t maxBssId;

	uint32_t gLimNumBeaconsRcvd;
	uint32_t gLimNumBeaconsIgnored;

	uint32_t gLimNumDeferredMsgs;

	/* / Variable to keep track of number of currently associated STAs */
	uint16_t gLimNumOfAniSTAs;      /* count of ANI peers */
	uint16_t gLimAssocStaLimit;

	/* Heart-Beat interval value */
	uint32_t gLimHeartBeatCount;
	tSirMacAddr gLimHeartBeatApMac[2];
	uint8_t gLimHeartBeatApMacIndex;

	/* Statistics to keep track of no. beacons rcvd in heart beat interval */
	uint16_t
		gLimHeartBeatBeaconStats[MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL];

#ifdef WLAN_DEBUG
	/* Debug counters */
	uint32_t numTot, numBbt, numProtErr, numLearn, numLearnIgnore;
	uint32_t numSme, numMAC[4][16];

	/* Debug counter to track number of Assoc Req frame drops */
	/* when received in pStaDs->mlmState other than LINK_ESTABLISED */
	uint32_t gLimNumAssocReqDropInvldState;
	/* counters to track rejection of Assoc Req due to Admission Control */
	uint32_t gLimNumAssocReqDropACRejectTS;
	uint32_t gLimNumAssocReqDropACRejectSta;
	/* Debug counter to track number of Reassoc Req frame drops */
	/* when received in pStaDs->mlmState other than LINK_ESTABLISED */
	uint32_t gLimNumReassocReqDropInvldState;
	/* Debug counter to track number of Hash Miss event that */
	/* will not cause a sending of de-auth/de-associate frame */
	uint32_t gLimNumHashMissIgnored;

	/* Debug counter to track number of Beacon frames */
	/* received in unexpected state */
	uint32_t gLimUnexpBcnCnt;

	/* Debug counter to track number of Beacon frames */
	/* received in wt-join-state that do have SSID mismatch */
	uint32_t gLimBcnSSIDMismatchCnt;

	/* Debug counter to track number of Link establishments on STA/BP */
	uint32_t gLimNumLinkEsts;

	/* Debug counter to track number of Rx cleanup */
	uint32_t gLimNumRxCleanup;

	/* Debug counter to track different parse problem */
	uint32_t gLim11bStaAssocRejectCount;

#endif

	/* Time stamp of the last beacon received from the BSS to which STA is connected. */
	uint64_t gLastBeaconTimeStamp;
	/* RX Beacon count for the current BSS to which STA is connected. */
	uint32_t gCurrentBssBeaconCnt;
	uint8_t gLastBeaconDtimCount;
	uint8_t gLastBeaconDtimPeriod;

	/* ////////////////////////////////////////     STATS/COUNTER RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////////     STATES RELATED START /////////////////////////////////////////// */
	/* Counts Heartbeat failures */
	uint8_t gLimHBfailureCntInLinkEstState;
	uint8_t gLimProbeFailureAfterHBfailedCnt;
	uint8_t gLimHBfailureCntInOtherStates;

	/**
	 * This variable indicates whether LIM module need to
	 * send response to host. Used to identify whether a request
	 * is generated internally within LIM module or by host
	 */
	uint8_t gLimRspReqd;

	/* / Previous SME State */
	tLimSmeStates gLimPrevSmeState;

	/* / MLM State visible across all Sirius modules */
	tLimMlmStates gLimMlmState;

	/* / Previous MLM State */
	tLimMlmStates gLimPrevMlmState;

	/* LIM to HAL SCAN Management Message Interface states */
	tLimLimHalScanState gLimHalScanState;
/* WLAN_SUSPEND_LINK Related */
	SUSPEND_RESUME_LINK_CALLBACK gpLimSuspendCallback;
	uint32_t *gpLimSuspendData;
	SUSPEND_RESUME_LINK_CALLBACK gpLimResumeCallback;
	uint32_t *gpLimResumeData;
/* end WLAN_SUSPEND_LINK Related */
	/* Can be set to invalid channel. If it is invalid, HAL */
	/* should move to previous valid channel or stay in the */
	/* current channel. CB state goes along with channel to resume to */
	uint16_t gResumeChannel;
	ePhyChanBondState gResumePhyCbState;

	/* Change channel generic scheme */
	CHANGE_CHANNEL_CALLBACK gpchangeChannelCallback;
	uint32_t *gpchangeChannelData;

	/* / SME State visible across all Sirius modules */
	tLimSmeStates gLimSmeState;
	/* / This indicates whether we're an AP, STA in BSS/IBSS */
	tLimSystemRole gLimSystemRole;

	/* Number of STAs that do not support short preamble */
	tLimNoShortParams gLimNoShortParams;

	/* Number of STAs that do not support short slot time */
	tLimNoShortSlotParams gLimNoShortSlotParams;

	/* */
	/* ---------------- DPH ----------------------- */
	/* these used to live in DPH but are now moved here (where they belong) */
	uint32_t gLimPhyMode;
	uint32_t propRateAdjustPeriod;
	uint32_t scanStartTime; /* used to measure scan time */

	uint8_t gLimMyMacAddr[6];
	uint8_t ackPolicy;

	uint8_t gLimQosEnabled:1;       /* 11E */
	uint8_t gLimWmeEnabled:1;       /* WME */
	uint8_t gLimWsmEnabled:1;       /* WSM */
	uint8_t gLimHcfEnabled:1;
	uint8_t gLim11dEnabled:1;
	uint8_t gLimProbeRespDisableFlag:1;    /* control over probe response */
	/* ---------------- DPH ----------------------- */

	/* ////////////////////////////////////////     STATES RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////////     MISC RELATED START /////////////////////////////////////////// */

	/* Deferred Queue Parameters */
	tLimDeferredMsgQParams gLimDeferredMsgQ;

	/* addts request if any - only one can be outstanding at any time */
	tSirAddtsReq gLimAddtsReq;
	uint8_t gLimAddtsSent;
	uint8_t gLimAddtsRspTimerCount;

	/* protection related config cache */
	tCfgProtection cfgProtection;

	uint8_t gLimProtectionControl;
	/* This flag will remain to be set except while LIM is waiting for specific response messages */
	/* from HAL. e.g when LIM issues ADD_STA req it will clear this flag and when it will receive */
	/* the response the flag will be set. */
	uint8_t gLimProcessDefdMsgs;

	/* UAPSD flag used on AP */
	uint8_t gUapsdEnable;

	/* Used on STA for AC downgrade. This is a dynamic mask
	 * setting which keep tracks of ACs being admitted.
	 * If bit is set to 0: That partiular AC is not admitted
	 * If bit is set to 1: That particular AC is admitted
	 */
	uint8_t gAcAdmitMask[SIR_MAC_DIRECTION_DIRECT];

	/* dialogue token List head/tail for Action frames request sent. */
	tpDialogueToken pDialogueTokenHead;
	tpDialogueToken pDialogueTokenTail;

	tLimTspecInfo tspecInfo[LIM_NUM_TSPEC_MAX];

	/* admission control policy information */
	tLimAdmitPolicyInfo admitPolicyInfo;
	qdf_mutex_t lkPeGlobalLock;
	uint8_t disableLDPCWithTxbfAP;
#ifdef FEATURE_WLAN_TDLS
	uint8_t gLimTDLSBufStaEnabled;
	uint8_t gLimTDLSUapsdMask;
	uint8_t gLimTDLSOffChannelEnabled;
	uint8_t gLimTDLSWmmMode;
#endif
	/* ////////////////////////////////////////     MISC RELATED END /////////////////////////////////////////// */

	/* ////////////////////////////////////////     ASSOC RELATED START /////////////////////////////////////////// */
	/* Place holder for JoinReq message */
	/* received by SME state machine */
	/* tpSirSmeJoinReq       gpLimJoinReq; */

	/* Place holder for ReassocReq message */
	/* received by SME state machine */
	/* tpSirSmeReassocReq    gpLimReassocReq;  sep23 review */

	/* Current Authentication type used at STA */
	/* tAniAuthType        gLimCurrentAuthType; */

	/* Place holder for current authentication request */
	/* being handled */
	tLimMlmAuthReq *gpLimMlmAuthReq;

	/* Reason code to determine the channel change context while sending */
	/* WMA_CHNL_SWITCH_REQ message to HAL */
	uint32_t channelChangeReasonCode;

	/* / MAC level Pre-authentication related globals */
	tSirMacChanNum gLimPreAuthChannelNumber;
	tAniAuthType gLimPreAuthType;
	tSirMacAddr gLimPreAuthPeerAddr;
	uint32_t gLimNumPreAuthContexts;
	tLimPreAuthTable gLimPreAuthTimerTable;

	/* Placed holder to deauth reason */
	uint16_t gLimDeauthReasonCode;

	/* Place holder for Pre-authentication node list */
	struct tLimPreAuthNode *pLimPreAuthList;

	/* Assoc or ReAssoc Response Data/Frame */
	void *gLimAssocResponseData;

	/* One cache for each overlap and associated case. */
	tCacheParams protStaOverlapCache[LIM_PROT_STA_OVERLAP_CACHE_SIZE];
	tCacheParams protStaCache[LIM_PROT_STA_CACHE_SIZE];

	/* ////////////////////////////////////////     ASSOC RELATED END /////////////////////////////////////////// */

	/* //////////////////////////////  HT RELATED           ////////////////////////////////////////// */
	/* */
	/* The following global LIM variables maintain/manage */
	/* the runtime configurations related to 802.11n */

	/* 802.11n Station detected HT capability in Beacon Frame */
	uint8_t htCapabilityPresentInBeacon;

	/* 802.11 HT capability: Enabled or Disabled */
	uint8_t htCapability;

	uint8_t gHTGreenfield;

	uint8_t gHTShortGI40Mhz;
	uint8_t gHTShortGI20Mhz;

	/* Set to 0 for 3839 octets */
	/* Set to 1 for 7935 octets */
	uint8_t gHTMaxAmsduLength;

	/* DSSS/CCK at 40 MHz: Enabled 1 or Disabled */
	uint8_t gHTDsssCckRate40MHzSupport;

	/* PSMP Support: Enabled 1 or Disabled 0 */
	uint8_t gHTPSMPSupport;

	/* L-SIG TXOP Protection used only if peer support available */
	uint8_t gHTLsigTXOPProtection;

	/* MIMO Power Save */
	tSirMacHTMIMOPowerSaveState gHTMIMOPSState;

	/* Scan In Power Save */
	uint8_t gScanInPowersave;

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
	uint8_t gHTAMpduDensity;

	bool gMaxAmsduSizeEnabled;
	/* Maximum Tx/Rx A-MPDU factor */
	uint8_t gHTMaxRxAMpduFactor;

	/* */
	/* Scheduled PSMP related - Service Interval Granularity */
	/* 000 - 5 ms */
	/* 001 - 10 ms */
	/* 010 - 15 ms */
	/* 011 - 20 ms */
	/* 100 - 25 ms */
	/* 101 - 30 ms */
	/* 110 - 35 ms */
	/* 111 - 40 ms */
	/* */
	uint8_t gHTServiceIntervalGranularity;

	/* Indicates whether an AP wants to associate PSMP enabled Stations */
	uint8_t gHTControlledAccessOnly;

	/* RIFS Mode. Set if no APSD legacy devices associated */
	uint8_t gHTRifsMode;
	/* OBss Mode . set when we have Non HT STA is associated or with in overlap bss */
	uint8_t gHTObssMode;

	/* Identifies the current Operating Mode */
	tSirMacHTOperatingMode gHTOperMode;

	/* Indicates if PCO is activated in the BSS */
	uint8_t gHTPCOActive;

	/* */
	/* If PCO is active, indicates which PCO phase to use */
	/* 0 - switch to 20 MHz phase */
	/* 1 - switch to 40 MHz phase */
	/* */
	uint8_t gHTPCOPhase;

	/* */
	/* Used only in beacons. For PR, this is set to 0 */
	/* 0 - Primary beacon */
	/* 1 - Secondary beacon */
	/* */
	uint8_t gHTSecondaryBeacon;

	/* */
	/* Dual CTS Protection */
	/* 0 - Use RTS/CTS */
	/* 1 - Dual CTS Protection is used */
	/* */
	uint8_t gHTDualCTSProtection;

	/* */
	/* Identifies a single STBC MCS that shall ne used for */
	/* STBC control frames and STBC beacons */
	/* */
	uint8_t gHTSTBCBasicMCS;

	uint8_t gHTNonGFDevicesPresent;

	/* //////////////////////////////  HT RELATED           ////////////////////////////////////////// */

#ifdef FEATURE_WLAN_TDLS
	uint8_t gLimAddStaTdls;
	uint8_t gLimTdlsLinkMode;
	/* //////////////////////////////  TDLS RELATED         ////////////////////////////////////////// */
#endif

	/* wsc info required to form the wsc IE */
	tLimWscIeInfo wscIeInfo;
	tpPESession gpSession;  /* Pointer to  session table */
	/*
	 * sessionID and transactionID from SME is stored here for those messages, for which
	 * there is no session context in PE, e.g. Scan related messages.
	 **/
	uint8_t gSmeSessionId;
	uint16_t gTransactionId;

	tSirRemainOnChnReq *gpLimRemainOnChanReq;       /* hold remain on chan request in this buf */
	qdf_mutex_t lim_frame_register_lock;
	qdf_list_t gLimMgmtFrameRegistratinQueue;
	uint32_t mgmtFrameSessionId;
	uint32_t tdls_frm_session_id;

	tpPESession pSessionEntry;
	uint8_t reAssocRetryAttempt;
	tLimDisassocDeauthCnfReq limDisassocDeauthCnfReq;
	uint8_t deferredMsgCnt;
	tSirDFSChannelList dfschannelList;
	uint8_t deauthMsgCnt;
	uint8_t disassocMsgCnt;
	uint8_t gLimIbssStaLimit;

	/* Number of channel switch IEs sent so far */
	uint8_t gLimDfsChanSwTxCount;
	uint8_t gLimDfsTargetChanNum;
	uint8_t probeCounter;
	uint8_t maxProbe;
	QDF_STATUS(*sme_msg_callback)
		(tHalHandle hal, struct scheduler_msg *msg);
	uint8_t retry_packet_cnt;
	uint8_t scan_disabled;
	uint8_t beacon_probe_rsp_cnt_per_scan;
	wlan_scan_requester req_id;
	bool global_obss_offload_enabled;
	bool global_obss_color_collision_det_offload;
} tAniSirLim, *tpAniSirLim;

struct mgmt_frm_reg_info {
	qdf_list_node_t node;   /* MUST be first element */
	uint16_t frameType;
	uint16_t matchLen;
	uint16_t sessionId;
	uint8_t matchData[1];
};

typedef struct sRrmContext {
	tRrmSMEContext rrmSmeContext;
	tRrmPEContext rrmPEContext;
} tRrmContext, *tpRrmContext;

typedef struct sHalMacStartParameters {
	/* parametes for the Firmware */
	enum qdf_driver_type driverType;

} tHalMacStartParameters;

/**
 * enum auth_tx_ack_status - Indicate TX status of AUTH
 * @LIM_AUTH_ACK_NOT_RCD : Default status while waiting for ack status.
 * @LIM_AUTH_ACK_RCD_SUCCESS : Ack is received.
 * @LIM_AUTH_ACK_RCD_FAILURE : No Ack received.
 *
 * Indicate if driver is waiting for ACK status of auth or ACK received for AUTH
 * OR NO ACK is received for the auth sent.
 */
enum auth_tx_ack_status {
	LIM_AUTH_ACK_NOT_RCD,
	LIM_AUTH_ACK_RCD_SUCCESS,
	LIM_AUTH_ACK_RCD_FAILURE,
};
/**
 * struct vdev_type_nss - vdev type nss structure
 * @sta: STA Nss value.
 * @sap: SAP Nss value.
 * @p2p_go: P2P GO Nss value.
 * @p2p_cli: P2P CLI Nss value.
 * @p2p_dev: P2P device Nss value.
 * @ibss: IBSS Nss value.
 * @tdls: TDLS Nss value.
 * @ocb: OCB Nss value.
 *
 * Holds the Nss values of different vdev types.
 */
struct vdev_type_nss {
	uint8_t sta;
	uint8_t sap;
	uint8_t p2p_go;
	uint8_t p2p_cli;
	uint8_t p2p_dev;
	uint8_t ibss;
	uint8_t tdls;
	uint8_t ocb;
};

/* ------------------------------------------------------------------- */
/* / MAC Sirius parameter structure */
typedef struct sAniSirGlobal {
	enum qdf_driver_type gDriverType;

	tAniSirCfg cfg;
	tAniSirLim lim;
	tAniSirSch sch;
	tAniSirSys sys;

	/* PAL/HDD handle */
	tHddHandle hHdd;


	tSmeStruct sme;
	tSapStruct sap;
	struct csr_scanstruct scan;
	struct csr_roamstruct roam;

	tRrmContext rrm;

	uint8_t isCoalesingInIBSSAllowed;

	csr_readyToSuspendCallback readyToSuspendCallback;
	void *readyToSuspendContext;
	uint8_t lteCoexAntShare;
	uint8_t beacon_offload;
	bool pmf_offload;
	bool is_fils_roaming_supported;
	uint32_t fEnableDebugLog;
	uint16_t mgmtSeqNum;
	bool enable5gEBT;
	/* Miracast session 0-Disabled, 1-Source, 2-sink */
	uint8_t fMiracastSessionPresent;
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	csr_readyToExtWoWCallback readyToExtWoWCallback;
	void *readyToExtWoWContext;
#endif
	uint32_t f_sta_miracast_mcc_rest_time_val;
	uint8_t f_prefer_non_dfs_on_radar;
	hdd_ftm_msg_processor ftm_msg_processor_callback;
	struct vdev_type_nss vdev_type_nss_2g;
	struct vdev_type_nss vdev_type_nss_5g;

	/* 802.11p enable */
	bool enable_dot11p;

	/* DBS capability based on INI and FW capability */
	uint8_t hw_dbs_capable;
	/* Based on INI parameter */
	uint32_t dual_mac_feature_disable;
	uint32_t sta_sap_scc_on_dfs_chan;
	sir_mgmt_frame_ind_callback mgmt_frame_ind_cb;
	bool first_scan_done;
	int8_t first_scan_bucket_threshold;
	enum auth_tx_ack_status auth_ack_status;
	uint8_t user_configured_nss;
	bool sta_prefer_80MHz_over_160MHz;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	enum  country_src reg_hint_src;
	bool snr_monitor_enabled;
	void (*chan_info_cb)(struct scan_chan_info *chan_info);
	uint32_t rx_packet_drop_counter;
	bool ignore_assoc_disallowed;
	uint32_t peer_rssi;
	uint32_t peer_txrate;
	uint32_t peer_rxrate;
	/* 11k Offload Support */
	bool is_11k_offload_supported;
	uint8_t reject_addba_req;
	uint16_t usr_cfg_ba_buff_size;
	uint8_t no_ack_policy_cfg[MAX_NUM_AC];
} tAniSirGlobal;


#ifdef FEATURE_WLAN_TDLS

#define RFC1042_HDR_LENGTH      (6)
#define GET_BE16(x)             ((uint16_t) (((x)[0] << 8) | (x)[1]))
#define ETH_TYPE_89_0d          (0x890d)
#define ETH_TYPE_LEN            (2)
#define PAYLOAD_TYPE_TDLS_SIZE  (1)
#define PAYLOAD_TYPE_TDLS       (2)

#endif

#endif /* _ANIGLOBAL_H */
