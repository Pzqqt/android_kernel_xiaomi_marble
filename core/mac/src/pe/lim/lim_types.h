/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
 * This file lim_types.h contains the definitions used by all
 * all LIM modules.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_TYPES_H
#define __LIM_TYPES_H

#include "wni_api.h"
#include "sir_api.h"
#include "sir_common.h"
#include "sir_mac_prot_def.h"
#include "utils_api.h"

#include "lim_api.h"
#include "lim_debug.h"
#include "lim_send_sme_rsp_messages.h"
#include "sys_global.h"
#include "dph_global.h"
#include "parser_api.h"
#include "wma_if.h"

#define LINK_TEST_DEFER 1

#define TRACE_EVENT_CNF_TIMER_DEACT        0x6600
#define TRACE_EVENT_CNF_TIMER_ACT          0x6601
#define TRACE_EVENT_AUTH_RSP_TIMER_DEACT   0x6602
#define TRACE_EVENT_AUTH_RSP_TIMER_ACT     0x6603

/* MLM message types */
#define LIM_MLM_MSG_START           1000
#define LIM_MLM_SCAN_REQ            LIM_MLM_MSG_START
#define LIM_MLM_SCAN_CNF            (LIM_MLM_MSG_START + 1)
#define LIM_MLM_START_REQ           (LIM_MLM_MSG_START + 2)
#define LIM_MLM_START_CNF           (LIM_MLM_MSG_START + 3)
#define LIM_MLM_JOIN_REQ            (LIM_MLM_MSG_START + 4)
#define LIM_MLM_JOIN_CNF            (LIM_MLM_MSG_START + 5)
#define LIM_MLM_AUTH_REQ            (LIM_MLM_MSG_START + 6)
#define LIM_MLM_AUTH_CNF            (LIM_MLM_MSG_START + 7)
#define LIM_MLM_AUTH_IND            (LIM_MLM_MSG_START + 8)
#define LIM_MLM_ASSOC_REQ           (LIM_MLM_MSG_START + 9)
#define LIM_MLM_ASSOC_CNF           (LIM_MLM_MSG_START + 10)
#define LIM_MLM_ASSOC_IND           (LIM_MLM_MSG_START + 11)
#define LIM_MLM_DISASSOC_REQ        (LIM_MLM_MSG_START + 12)
#define LIM_MLM_DISASSOC_CNF        (LIM_MLM_MSG_START + 13)
#define LIM_MLM_DISASSOC_IND        (LIM_MLM_MSG_START + 14)
#define LIM_MLM_REASSOC_REQ         (LIM_MLM_MSG_START + 15)
#define LIM_MLM_REASSOC_CNF         (LIM_MLM_MSG_START + 16)
#define LIM_MLM_REASSOC_IND         (LIM_MLM_MSG_START + 17)
#define LIM_MLM_DEAUTH_REQ          (LIM_MLM_MSG_START + 18)
#define LIM_MLM_DEAUTH_CNF          (LIM_MLM_MSG_START + 19)
#define LIM_MLM_DEAUTH_IND          (LIM_MLM_MSG_START + 20)
#define LIM_MLM_TSPEC_REQ           (LIM_MLM_MSG_START + 21)
#define LIM_MLM_TSPEC_CNF           (LIM_MLM_MSG_START + 22)
#define LIM_MLM_TSPEC_IND           (LIM_MLM_MSG_START + 23)
#define LIM_MLM_SETKEYS_REQ         (LIM_MLM_MSG_START + 24)
#define LIM_MLM_SETKEYS_CNF         (LIM_MLM_MSG_START + 25)
#define LIM_MLM_LINK_TEST_STOP_REQ  (LIM_MLM_MSG_START + 30)
#define LIM_MLM_PURGE_STA_IND       (LIM_MLM_MSG_START + 31)
/*
 * Values (LIM_MLM_MSG_START + 32) through
 * (LIM_MLM_MSG_START + 40) are unused.
 */
#ifdef FEATURE_OEM_DATA_SUPPORT
#define LIM_MLM_OEM_DATA_REQ            (LIM_MLM_MSG_START + 41)
#define LIM_MLM_OEM_DATA_CNF            (LIM_MLM_MSG_START + 42)
#endif

#define LIM_HASH_ADD            0
#define LIM_HASH_UPDATE         1

#define LIM_WEP_IN_FC           1
#define LIM_NO_WEP_IN_FC        0

#define LIM_DECRYPT_ICV_FAIL    1

/* / Definitions to distinquish between Association/Reassociaton */
#define LIM_ASSOC    0
#define LIM_REASSOC  1

/* / Minimum Memory blocks require for different scenario */
#define LIM_MIN_MEM_ASSOC       4

/* / Verifies whether given mac addr matches the CURRENT Bssid */
#define IS_CURRENT_BSSID(pMac, addr, psessionEntry)  (cdf_mem_compare(addr, \
								      psessionEntry->bssId, \
								      sizeof(psessionEntry->bssId)))
/* / Verifies whether given addr matches the REASSOC Bssid */
#define IS_REASSOC_BSSID(pMac, addr, psessionEntry)  (cdf_mem_compare(addr, \
								      psessionEntry->limReAssocbssId, \
								      sizeof(psessionEntry->limReAssocbssId)))

#define REQ_TYPE_REGISTRAR                   (0x2)
#define REQ_TYPE_WLAN_MANAGER_REGISTRAR      (0x3)

#define RESP_TYPE_REGISTRAR                  (0x2)
#define RESP_TYPE_ENROLLEE_INFO_ONLY         (0x0)
#define RESP_TYPE_ENROLLEE_OPEN_8021X        (0x1)
#define RESP_TYPE_AP                         (0x3)
#define LIM_TX_FRAMES_THRESHOLD_ON_CHIP       300


#define HAL_TXCOMP_REQUESTED_MASK           0x1 /* bit 0 for TxComp intr requested. */
#define HAL_USE_SELF_STA_REQUESTED_MASK     0x2 /* bit 1 for STA overwrite with selfSta Requested. */
#define HAL_TX_NO_ENCRYPTION_MASK           0x4 /* bit 2. If set, the frame is not to be encrypted */
#if defined(LIBRA_WAPI_SUPPORT)
#define HAL_WAPI_STA_MASK            0x8        /* bit 3. If set, this frame is for WAPI station */
#endif

#define HAL_TRIGGER_ENABLED_AC_MASK         0x10        /* bit 4 for data frames belonging to trigger enabled AC */
#define HAL_USE_NO_ACK_REQUESTED_MASK       0x20

#define HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME 0x40      /* Bit 6 will be used to control BD rate for Management frames */
#define HAL_USE_PEER_STA_REQUESTED_MASK   0x80  /* bit 7 will be used to control frames for p2p interface */

#ifdef FEATURE_WLAN_TDLS
#define HAL_TDLS_PEER_STA_MASK              0x80        /* bit 7 set for TDLS peer station */
#endif


/* enums used by LIM are as follows */

enum eLimDisassocTrigger {
	eLIM_HOST_DISASSOC,
	eLIM_PEER_ENTITY_DISASSOC,
	eLIM_LINK_MONITORING_DISASSOC,
	eLIM_PROMISCUOUS_MODE_DISASSOC,
	eLIM_HOST_DEAUTH,
	eLIM_PEER_ENTITY_DEAUTH,
	eLIM_LINK_MONITORING_DEAUTH,
	eLIM_JOIN_FAILURE,
	eLIM_REASSOC_REJECT
};

/* Reason code to determine the channel change context while sending
 * WMA_CHNL_SWITCH_REQ message to HAL
 */
enum eChannelChangeReasonCodes {
	LIM_SWITCH_CHANNEL_REASSOC,
	LIM_SWITCH_CHANNEL_JOIN,
	LIM_SWITCH_CHANNEL_OPERATION,   /* Generic change channel */
	LIM_SWITCH_CHANNEL_SAP_DFS,     /* DFS channel change */
};

typedef struct sLimAuthRspTimeout {
	tSirMacAddr peerMacAddr;
} tLimAuthRspTimeout;

typedef struct sLimMlmStartReq {
	tSirMacSSid ssId;
	tSirBssType bssType;
	tSirMacAddr bssId;
	tSirMacBeaconInterval beaconPeriod;
	uint8_t dtimPeriod;
	tSirMacCfParamSet cfParamSet;
	tSirMacChanNum channelNumber;
	ePhyChanBondState cbMode;
	uint16_t atimWindow;
	tSirMacRateSet rateSet;
	uint8_t sessionId;      /* Added For BT-AMP Support */

	/* Parameters reqd for new HAL (message) interface */
	tSirNwType nwType;
	uint8_t htCapable;
	tSirMacHTOperatingMode htOperMode;
	uint8_t dualCTSProtection;
	uint8_t txChannelWidthSet;
	uint8_t ssidHidden;
	uint8_t wps_state;
	uint8_t obssProtEnabled;
} tLimMlmStartReq, *tpLimMlmStartReq;

typedef struct sLimMlmStartCnf {
	tSirResultCodes resultCode;
	uint8_t sessionId;
} tLimMlmStartCnf, *tpLimMlmStartCnf;

typedef struct sLimMlmScanCnf {
	tSirResultCodes resultCode;
	uint16_t scanResultLength;
	tSirBssDescription bssDescription[1];
	uint8_t sessionId;
} tLimMlmScanCnf, *tpLimMlmScanCnf;

typedef struct sLimScanResult {
	uint16_t numBssDescriptions;
	tSirBssDescription bssDescription[1];
} tLimScanResult;

typedef struct sLimMlmJoinCnf {
	tSirResultCodes resultCode;
	uint16_t protStatusCode;
	uint8_t sessionId;
} tLimMlmJoinCnf, *tpLimMlmJoinCnf;

typedef struct sLimMlmAssocReq {
	tSirMacAddr peerMacAddr;
	uint32_t assocFailureTimeout;
	uint16_t capabilityInfo;
	tSirMacListenInterval listenInterval;
	uint8_t sessionId;
} tLimMlmAssocReq, *tpLimMlmAssocReq;

typedef struct sLimMlmAssocCnf {
	tSirResultCodes resultCode;     /* Internal status code. */
	uint16_t protStatusCode;        /* Protocol Status code. */
	uint8_t sessionId;
} tLimMlmAssocCnf, *tpLimMlmAssocCnf;

typedef struct sLimMlmAssocInd {
	tSirMacAddr peerMacAddr;
	uint16_t aid;
	tAniAuthType authType;
	tAniSSID ssId;
	tSirRSNie rsnIE;
	tSirWAPIie wapiIE;
	tSirAddie addIE;        /* additional IE received from the peer, which possibly includes WSC IE and/or P2P IE. */
	tSirMacCapabilityInfo capabilityInfo;
	tAniBool spectrumMgtIndicator;
	tSirMacPowerCapInfo powerCap;
	tSirSupChnl supportedChannels;
	uint8_t sessionId;

	tAniBool WmmStaInfoPresent;

	/* Required for indicating the frames to upper layer */
	uint32_t beaconLength;
	uint8_t *beaconPtr;
	uint32_t assocReqLength;
	uint8_t *assocReqPtr;
	tSirSmeChanInfo chan_info;
} tLimMlmAssocInd, *tpLimMlmAssocInd;

typedef struct sLimMlmReassocReq {
	tSirMacAddr peerMacAddr;
	uint32_t reassocFailureTimeout;
	uint16_t capabilityInfo;
	tSirMacListenInterval listenInterval;
	uint8_t sessionId;
} tLimMlmReassocReq, *tpLimMlmReassocReq;

typedef struct sLimMlmReassocCnf {
	tSirResultCodes resultCode;
	uint16_t protStatusCode;        /* Protocol Status code. */
	uint8_t sessionId;
} tLimMlmReassocCnf, *tpLimMlmReassocCnf;

typedef struct sLimMlmReassocInd {
	tSirMacAddr peerMacAddr;
	tSirMacAddr currentApAddr;
	uint16_t aid;
	tAniAuthType authType;
	tAniSSID ssId;
	tSirRSNie rsnIE;
	tSirWAPIie wapiIE;
	tSirAddie addIE;        /* additional IE received from the peer, which can be WSC IE and/or P2P IE. */
	tSirMacCapabilityInfo capabilityInfo;
	tAniBool spectrumMgtIndicator;
	tSirMacPowerCapInfo powerCap;
	tSirSupChnl supportedChannels;

	tAniBool WmmStaInfoPresent;

	/* Required for indicating the frames to upper layer */
	uint32_t beaconLength;
	uint8_t *beaconPtr;
	uint32_t assocReqLength;
	uint8_t *assocReqPtr;
} tLimMlmReassocInd, *tpLimMlmReassocInd;

typedef struct sLimMlmAuthCnf {
	tSirMacAddr peerMacAddr;
	tAniAuthType authType;
	tSirResultCodes resultCode;
	uint16_t protStatusCode;
	uint8_t sessionId;
} tLimMlmAuthCnf, *tpLimMlmAuthCnf;

typedef struct sLimMlmDeauthReq {
	tSirMacAddr peerMacAddr;
	uint16_t reasonCode;
	uint16_t deauthTrigger;
	uint16_t aid;
	uint8_t sessionId;      /* Added for BT-AMP SUPPORT */

} tLimMlmDeauthReq, *tpLimMlmDeauthReq;

typedef struct sLimMlmDeauthCnf {
	tSirMacAddr peerMacAddr;
	tSirResultCodes resultCode;
	uint16_t deauthTrigger;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmDeauthCnf, *tpLimMLmDeauthCnf;

typedef struct sLimMlmDeauthInd {
	tSirMacAddr peerMacAddr;
	uint16_t reasonCode;
	uint16_t deauthTrigger;
	uint16_t aid;
} tLimMlmDeauthInd, *tpLimMlmDeauthInd;

typedef struct sLimMlmDisassocReq {
	tSirMacAddr peerMacAddr;
	uint16_t reasonCode;
	uint16_t disassocTrigger;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmDisassocReq, *tpLimMlmDisassocReq;

typedef struct sLimMlmDisassocCnf {
	tSirMacAddr peerMacAddr;
	tSirResultCodes resultCode;
	uint16_t disassocTrigger;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmDisassocCnf, *tpLimMlmDisassocCnf;

typedef struct sLimMlmDisassocInd {
	tSirMacAddr peerMacAddr;
	uint16_t reasonCode;
	uint16_t disassocTrigger;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmDisassocInd, *tpLimMlmDisassocInd;

typedef struct sLimMlmPurgeStaReq {
	tSirMacAddr peerMacAddr;
	uint16_t aid;
	uint8_t sessionId;      /* Added For BT-AMP Support */
} tLimMlmPurgeStaReq, *tpLimMlmPurgeStaReq;

typedef struct sLimMlmPurgeStaInd {
	tSirMacAddr peerMacAddr;
	uint16_t reasonCode;
	uint16_t purgeTrigger;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmPurgeStaInd, *tpLimMlmPurgeStaInd;

typedef struct sLimMlmSetKeysCnf {
	tSirMacAddr peerMacAddr;
	uint16_t resultCode;
	uint16_t aid;
	uint8_t sessionId;
} tLimMlmSetKeysCnf, *tpLimMlmSetKeysCnf;

typedef struct sLimMlmResetReq {
	tSirMacAddr macAddr;
	uint8_t performCleanup;
	uint8_t sessionId;
} tLimMlmResetReq, *tpLimMlmResetReq;

typedef struct sLimMlmResetCnf {
	tSirMacAddr macAddr;
	tSirResultCodes resultCode;
	uint8_t sessionId;
} tLimMlmResetCnf, *tpLimMlmResetCnf;

typedef struct sLimMlmLinkTestStopReq {
	tSirMacAddr peerMacAddr;
	uint8_t sessionId;
} tLimMlmLinkTestStopReq, *tpLimMlmLinkTestStopReq;

/* Function templates */

bool lim_process_sme_req_messages(tpAniSirGlobal, tpSirMsgQ);
void lim_process_mlm_req_messages(tpAniSirGlobal, tpSirMsgQ);
void lim_process_mlm_rsp_messages(tpAniSirGlobal, uint32_t, uint32_t *);
void lim_process_sme_del_bss_rsp(tpAniSirGlobal, uint32_t, tpPESession);

void lim_get_random_bssid(tpAniSirGlobal pMac, uint8_t *data);

/* Function to handle HT and HT IE CFG parameter intializations */
void handle_ht_capabilityand_ht_info(struct sAniSirGlobal *pMac,
				     tpPESession psessionEntry);

/* Function to handle CFG parameter updates */
void lim_handle_cf_gparam_update(tpAniSirGlobal, uint32_t);

void lim_handle_param_update(tpAniSirGlobal pMac, eUpdateIEsType cfgId);

/* Function to apply CFG parameters before join/reassoc/start BSS */
void lim_apply_configuration(tpAniSirGlobal, tpPESession);

void lim_set_cfg_protection(tpAniSirGlobal pMac, tpPESession pesessionEntry);

/* Function to Initialize MLM state machine on STA */
void lim_init_mlm(tpAniSirGlobal);

/* Function to cleanup MLM state machine */
void lim_cleanup_mlm(tpAniSirGlobal);

/* Function to cleanup LMM state machine */
void lim_cleanup_lmm(tpAniSirGlobal);

/* Management frame handling functions */
void lim_process_beacon_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_beacon_frame_no_session(tpAniSirGlobal, uint8_t *);
void lim_process_probe_req_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_probe_rsp_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_probe_rsp_frame_no_session(tpAniSirGlobal, uint8_t *);
void lim_process_probe_req_frame_multiple_bss(tpAniSirGlobal, uint8_t *,
					      tpPESession);

/* Process Auth frame when we have a session in progress. */
void lim_process_auth_frame(tpAniSirGlobal, uint8_t *, tpPESession);
#ifdef WLAN_FEATURE_VOWIFI_11R
tSirRetStatus lim_process_auth_frame_no_session(tpAniSirGlobal pMac, uint8_t *,
						void *body);
#endif

void lim_process_assoc_req_frame(tpAniSirGlobal, uint8_t *, uint8_t, tpPESession);
void lim_send_mlm_assoc_ind(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
			    tpPESession psessionEntry);

void lim_process_assoc_rsp_frame(tpAniSirGlobal, uint8_t *, uint8_t, tpPESession);
void lim_process_disassoc_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_deauth_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_action_frame(tpAniSirGlobal, uint8_t *, tpPESession);
void lim_process_action_frame_no_session(tpAniSirGlobal pMac, uint8_t *pRxMetaInfo);

void lim_populate_p2p_mac_header(tpAniSirGlobal, uint8_t *);
void lim_populate_mac_header(tpAniSirGlobal, uint8_t *, uint8_t, uint8_t,
				      tSirMacAddr, tSirMacAddr);
tSirRetStatus lim_send_probe_req_mgmt_frame(tpAniSirGlobal, tSirMacSSid *,
					    tSirMacAddr, uint8_t, tSirMacAddr,
					    uint32_t, uint32_t, uint8_t *);
void lim_send_probe_rsp_mgmt_frame(tpAniSirGlobal, tSirMacAddr, tpAniSSID, short,
				   uint8_t, tpPESession, uint8_t);
void lim_send_auth_mgmt_frame(tpAniSirGlobal, tSirMacAuthFrameBody *, tSirMacAddr,
			      uint8_t, tpPESession);
void lim_send_assoc_req_mgmt_frame(tpAniSirGlobal, tLimMlmAssocReq *, tpPESession);
void lim_send_reassoc_req_mgmt_frame(tpAniSirGlobal, tLimMlmReassocReq *,
				     tpPESession);
#ifdef WLAN_FEATURE_VOWIFI_11R
void lim_send_reassoc_req_with_ft_ies_mgmt_frame(tpAniSirGlobal pMac,
						 tLimMlmReassocReq *pMlmReassocReq,
						 tpPESession psessionEntry);
#endif
void lim_send_delts_req_action_frame(tpAniSirGlobal pMac, tSirMacAddr peer,
				     uint8_t wmmTspecPresent,
				     tSirMacTSInfo *pTsinfo,
				     tSirMacTspecIE *pTspecIe,
				     tpPESession psessionEntry);
void lim_send_addts_req_action_frame(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
				     tSirAddtsReqInfo *addts, tpPESession);
void lim_send_addts_rsp_action_frame(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
				     uint16_t statusCode, tSirAddtsReqInfo *addts,
				     tSirMacScheduleIE *pSchedule, tpPESession);

void lim_send_assoc_rsp_mgmt_frame(tpAniSirGlobal, uint16_t, uint16_t, tSirMacAddr,
				   uint8_t, tpDphHashNode pSta, tpPESession);

void lim_send_disassoc_mgmt_frame(tpAniSirGlobal, uint16_t, tSirMacAddr,
				  tpPESession, bool waitForAck);
void lim_send_deauth_mgmt_frame(tpAniSirGlobal, uint16_t, tSirMacAddr, tpPESession,
				bool waitForAck);

tSirResultCodes lim_mlm_add_bss(tpAniSirGlobal, tLimMlmStartReq *,
				tpPESession psessionEntry);

tSirRetStatus lim_send_channel_switch_mgmt_frame(tpAniSirGlobal, tSirMacAddr,
						 uint8_t, uint8_t, uint8_t,
						 tpPESession);

tSirRetStatus lim_send_extended_chan_switch_action_frame(tpAniSirGlobal mac_ctx,
	tSirMacAddr peer, uint8_t mode, uint8_t new_op_class,
	uint8_t new_channel, uint8_t count, tpPESession session_entry);

#ifdef WLAN_FEATURE_11AC
tSirRetStatus lim_send_vht_opmode_notification_frame(tpAniSirGlobal pMac,
						     tSirMacAddr peer, uint8_t nMode,
						     tpPESession psessionEntry);
tSirRetStatus lim_send_vht_channel_switch_mgmt_frame(tpAniSirGlobal pMac,
						     tSirMacAddr peer,
						     uint8_t nChanWidth,
						     uint8_t nNewChannel,
						     uint8_t ncbMode,
						     tpPESession psessionEntry);
#endif

#if defined WLAN_FEATURE_VOWIFI
tSirRetStatus lim_send_neighbor_report_request_frame(tpAniSirGlobal,
						     tpSirMacNeighborReportReq,
						     tSirMacAddr, tpPESession);
tSirRetStatus lim_send_link_report_action_frame(tpAniSirGlobal, tpSirMacLinkReport,
						tSirMacAddr, tpPESession);
tSirRetStatus lim_send_radio_measure_report_action_frame(tpAniSirGlobal, uint8_t,
							 uint8_t,
							 tpSirMacRadioMeasureReport,
							 tSirMacAddr, tpPESession);
#endif


#ifdef FEATURE_WLAN_TDLS
void lim_init_tdls_data(tpAniSirGlobal, tpPESession);
tSirRetStatus lim_process_sme_tdls_mgmt_send_req(tpAniSirGlobal pMac,
						 uint32_t *pMsgBuf);
tSirRetStatus lim_process_sme_tdls_add_sta_req(tpAniSirGlobal pMac,
					       uint32_t *pMsgBuf);
tSirRetStatus lim_process_sme_tdls_link_establish_req(tpAniSirGlobal pMac,
						     uint32_t *pMsgBuf);
tSirRetStatus lim_process_sme_tdls_del_sta_req(tpAniSirGlobal pMac,
					       uint32_t *pMsgBuf);
void lim_send_sme_tdls_delete_all_peer_ind(tpAniSirGlobal pMac,
					   tpPESession psessionEntry);
void lim_send_sme_mgmt_tx_completion(tpAniSirGlobal pMac, tpPESession psessionEntry,
				     uint32_t txCompleteStatus);
tSirRetStatus lim_delete_tdls_peers(tpAniSirGlobal pMac,
				    tpPESession psessionEntry);
CDF_STATUS lim_process_tdls_add_sta_rsp(tpAniSirGlobal pMac, void *msg, tpPESession);
tSirRetStatus lim_send_tdls_teardown_frame(tpAniSirGlobal pMac,
					   tSirMacAddr peerMac, uint16_t reason,
					   uint8_t responder,
					   tpPESession psessionEntry,
					   uint8_t *addIe, uint16_t addIeLen);
#endif

/* Algorithms & Link Monitoring related functions */
/* / Function that handles heartbeat failure */
void lim_handle_heart_beat_failure(tpAniSirGlobal, tpPESession);

/* / Function that triggers link tear down with AP upon HB failure */
void lim_tear_down_link_with_ap(tpAniSirGlobal, uint8_t, tSirMacReasonCodes);

/* / Function that processes Max retries interrupt from TFP */
void limHandleMaxRetriesInterrupt(uint32_t);

/* / Function that processes messages deferred during Learn mode */
void lim_process_deferred_message_queue(tpAniSirGlobal);

/* / Function that defers the messages received */
uint32_t lim_defer_msg(tpAniSirGlobal, tSirMsgQ *);

/* / Function that Switches the Channel and sets the CB Mode */
void lim_set_channel(tpAniSirGlobal pMac, uint8_t channel,
		uint8_t ch_center_freq_seg0, uint8_t ch_center_freq_seg1,
		phy_ch_width ch_width, tPowerdBm maxTxPower,
		uint8_t peSessionId);


/* / Function that completes channel scan */
void lim_complete_mlm_scan(tpAniSirGlobal, tSirResultCodes);

#ifdef ANI_SUPPORT_11H
/* / Function that sends Measurement Report action frame */
tSirRetStatus lim_send_meas_report_frame(tpAniSirGlobal, tpSirMacMeasReqActionFrame,
					 tSirMacAddr, tpPESession psessionEntry);

/* / Function that sends TPC Report action frame */
tSirRetStatus lim_send_tpc_report_frame(tpAniSirGlobal, tpSirMacTpcReqActionFrame,
					tSirMacAddr, tpPESession psessionEntry);
#endif

/* / Function that sends TPC Request action frame */
void lim_send_tpc_request_frame(tpAniSirGlobal, tSirMacAddr,
				tpPESession psessionEntry);

/* Function(s) to handle responses received from HAL */
void lim_process_mlm_add_bss_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ);
void lim_process_mlm_add_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQt,
				 tpPESession psessionEntry);
void lim_process_mlm_del_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ);
void lim_process_mlm_del_bss_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,
				 tpPESession);
void lim_process_sta_mlm_add_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,
				     tpPESession psessionEntry);
void lim_process_sta_mlm_del_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,
				     tpPESession psessionEntry);
void lim_process_sta_mlm_del_bss_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,
				     tpPESession psessionEntry);
void lim_process_mlm_set_sta_key_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ);
void lim_process_mlm_set_bss_key_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ);

/* Function to process WMA_SWITCH_CHANNEL_RSP message */
void lim_process_switch_channel_rsp(tpAniSirGlobal pMac, void *);

void lim_covert_channel_scan_type(tpAniSirGlobal pMac, uint8_t channelNum,
				  bool passiveToActive);
void lim_set_dfs_channel_list(tpAniSirGlobal pMac, uint8_t channelNum,
			      tSirDFSChannelList *dfsChannelList);
void limContinueChannelLearn(tpAniSirGlobal);
/* WLAN_SUSPEND_LINK Related */
uint8_t lim_is_link_suspended(tpAniSirGlobal pMac);
/* end WLAN_SUSPEND_LINK Related */

#ifdef WLAN_FEATURE_11W
/* 11w send SA query request action frame */
tSirRetStatus lim_send_sa_query_request_frame(tpAniSirGlobal pMac, uint8_t *transId,
					      tSirMacAddr peer,
					      tpPESession psessionEntry);
/* 11w SA query request action frame handler */
tSirRetStatus lim_send_sa_query_response_frame(tpAniSirGlobal pMac,
					       uint8_t *transId, tSirMacAddr peer,
					       tpPESession psessionEntry);
#endif

/* Inline functions */

/**
 * lim_post_sme_message()
 *
 ***FUNCTION:
 * This function is called by limProcessMlmMessages(). In this
 * function MLM sub-module invokes MLM ind/cnf primitives.
 *
 ***LOGIC:
 * Initially MLM makes an SME function call to invoke MLM ind/cnf
 * primitive. In future this can be enhanced to 'post' messages to SME.
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param pMac      Pointer to Global MAC structure
 * @param msgType   Indicates the MLM primitive message type
 * @param *pMsgBuf  A pointer to the MLM message buffer
 *
 * @return None
 */
static inline void
lim_post_sme_message(tpAniSirGlobal pMac, uint32_t msgType, uint32_t *pMsgBuf)
{
	tSirMsgQ msg;

	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	msg.type = (uint16_t) msgType;
	msg.bodyptr = pMsgBuf;
	msg.bodyval = 0;
	if (msgType > eWNI_SME_MSG_TYPES_BEGIN)
		lim_process_sme_req_messages(pMac, &msg);
	else
		lim_process_mlm_rsp_messages(pMac, msgType, pMsgBuf);
} /*** end lim_post_sme_message() ***/

/**
 * lim_post_mlm_message()
 *
 ***FUNCTION:
 * This function is called by limProcessSmeMessages(). In this
 * function SME invokes MLME primitives.
 *
 ***PARAMS:
 *
 ***LOGIC:
 * Initially SME makes an MLM function call to invoke MLM primitive.
 * In future this can be enhanced to 'post' messages to MLM.
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param pMac      Pointer to Global MAC structure
 * @param msgType   Indicates the MLM primitive message type
 * @param *pMsgBuf  A pointer to the MLM message buffer
 *
 * @return None
 */
static inline void
lim_post_mlm_message(tpAniSirGlobal pMac, uint32_t msgType, uint32_t *pMsgBuf)
{

	tSirMsgQ msg;
	if (pMsgBuf == NULL) {
		lim_log(pMac, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}
	msg.type = (uint16_t) msgType;
	msg.bodyptr = pMsgBuf;
	msg.bodyval = 0;
	lim_process_mlm_req_messages(pMac, &msg);
} /*** end lim_post_mlm_message() ***/

/**
 * lim_get_current_scan_channel()
 *
 ***FUNCTION:
 * This function is called in various places to get current channel
 * number being scanned.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pMac      Pointer to Global MAC structure
 * @return Channel number
 */
static inline uint8_t lim_get_current_scan_channel(tpAniSirGlobal pMac)
{
	uint8_t *pChanNum =
		pMac->lim.gpLimMlmScanReq->channelList.channelNumber;

	return *(pChanNum + pMac->lim.gLimCurrentScanChannelId);
} /*** end lim_get_current_scan_channel() ***/

/**
 * lim_get_ielen_from_bss_description()
 *
 ***FUNCTION:
 * This function is called in various places to get IE length
 * from tSirBssDescription structure
 * number being scanned.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param     pBssDescr
 * @return    Total IE length
 */

static inline uint16_t
lim_get_ielen_from_bss_description(tpSirBssDescription pBssDescr)
{
	uint16_t ielen;

	if (!pBssDescr)
		return 0;

	/*
	 * Length of BSS desription is without length of
	 * length itself and length of pointer
	 * that holds ieFields
	 *
	 * <------------sizeof(tSirBssDescription)-------------------->
	 * +--------+---------------------------------+---------------+
	 * | length | other fields                    | pointer to IEs|
	 * +--------+---------------------------------+---------------+
	 *                                            ^
	 *                                            ieFields
	 */

	ielen = (uint16_t)(pBssDescr->length + sizeof(pBssDescr->length) -
			   GET_FIELD_OFFSET(tSirBssDescription, ieFields));

	return ielen;
} /*** end lim_get_ielen_from_bss_description() ***/

/**
 * lim_send_beacon_ind()
 *
 ***FUNCTION:
 * This function is called  to send the beacon indication
 * number being scanned.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 */

void lim_send_beacon_ind(tpAniSirGlobal pMac, tpPESession psessionEntry);

void
lim_send_vdev_restart(tpAniSirGlobal pMac, tpPESession psessionEntry,
		      uint8_t sessionId);

void lim_get_wpspbc_sessions(tpAniSirGlobal pMac, uint8_t *addr, uint8_t *uuid_e,
			     eWPSPBCOverlap *overlap, tpPESession psessionEntry);
void limWPSPBCTimeout(tpAniSirGlobal pMac, tpPESession psessionEntry);
void lim_wpspbc_close(tpAniSirGlobal pMac, tpPESession psessionEntry);
void lim_remove_pbc_sessions(tpAniSirGlobal pMac, tSirMacAddr pRemoveMac,
			     tpPESession psessionEntry);

#define LIM_WPS_OVERLAP_TIMER_MS                 10000
void
lim_change_channel_with_callback(tpAniSirGlobal pMac, uint8_t newChannel,
				 CHANGE_CHANNEL_CALLBACK callback,
				 uint32_t *cbdata, tpPESession psessionEntry);

void lim_send_sme_mgmt_frame_ind(tpAniSirGlobal pMac, uint8_t frameType,
				 uint8_t *frame, uint32_t frameLen,
				 uint16_t sessionId, uint32_t rxChan,
				 tpPESession psessionEntry, int8_t rxRssi);
void lim_process_remain_on_chn_timeout(tpAniSirGlobal pMac);
void lim_process_insert_single_shot_noa_timeout(tpAniSirGlobal pMac);
void lim_convert_active_channel_to_passive_channel(tpAniSirGlobal pMac);
void lim_send_p2p_action_frame(tpAniSirGlobal pMac, tpSirMsgQ pMsg);
void lim_abort_remain_on_chan(tpAniSirGlobal pMac, uint8_t sessionId,
	uint32_t scan_id);
tSirRetStatus __lim_process_sme_no_a_update(tpAniSirGlobal pMac, uint32_t *pMsgBuf);
void lim_process_regd_defd_sme_req_after_noa_start(tpAniSirGlobal pMac);

void lim_process_disassoc_ack_timeout(tpAniSirGlobal pMac);
void lim_process_deauth_ack_timeout(tpAniSirGlobal pMac);
CDF_STATUS lim_send_disassoc_cnf(tpAniSirGlobal pMac);
CDF_STATUS lim_send_deauth_cnf(tpAniSirGlobal pMac);
CDF_STATUS lim_disassoc_tx_complete_cnf(tpAniSirGlobal pMac,
					uint32_t txCompleteSuccess);
CDF_STATUS lim_deauth_tx_complete_cnf(tpAniSirGlobal pMac,
				      uint32_t txCompleteSuccess);

#ifdef WLAN_FEATURE_VOWIFI_11R
typedef struct sSetLinkCbackParams {
	void *cbackDataPtr;
} tSetLinkCbackParams;
#endif

void lim_process_rx_scan_event(tpAniSirGlobal mac, void *buf);

int lim_process_remain_on_chnl_req(tpAniSirGlobal pMac, uint32_t *pMsg);
void lim_remain_on_chn_rsp(tpAniSirGlobal pMac, CDF_STATUS status, uint32_t *data);
void lim_send_sme_disassoc_deauth_ntf(tpAniSirGlobal mac_ctx,
				CDF_STATUS status, uint32_t *ctx);

/* / Bit value data structure */
typedef enum sHalBitVal         /* For Bit operations */
{
	eHAL_CLEAR,
	eHAL_SET
} tHalBitVal;

enum {
	eHI_PRI,
	ePROT,
	eDBG
};

#endif /* __LIM_TYPES_H */
