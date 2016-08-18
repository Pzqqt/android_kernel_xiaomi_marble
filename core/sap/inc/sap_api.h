/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#ifndef WLAN_QCT_WLANSAP_H
#define WLAN_QCT_WLANSAP_H

/**
 * W L A N   S O F T A P  P A L   L A Y E R
 * E X T E R N A L  A P I
 *
 * DESCRIPTION
 * This file contains the external API exposed by the wlan SAP PAL layer
 *  module.
 */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "cds_api.h"
#include "cds_packet.h"
#include "qdf_types.h"

#include "p2p_api.h"
#include "sme_api.h"
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * defines and enum
 *--------------------------------------------------------------------------*/
#define       MAX_SSID_LEN                 32
#define       MAX_ACL_MAC_ADDRESS          32
#define       AUTO_CHANNEL_SELECT          0
#define       MAX_ASSOC_IND_IE_LEN         255

/* defines for WPS config states */
#define       SAP_WPS_DISABLED             0
#define       SAP_WPS_ENABLED_UNCONFIGURED 1
#define       SAP_WPS_ENABLED_CONFIGURED   2
#define       MAX_NAME_SIZE                64
#define       MAX_TEXT_SIZE                32

#define       MAX_CHANNEL_LIST_LEN         256
#define       QDF_MAX_NO_OF_SAP_MODE       2    /* max # of SAP */
#define       SAP_MAX_NUM_SESSION          5
#define       SAP_MAX_OBSS_STA_CNT         1    /* max # of OBSS STA */
#define       SAP_ACS_WEIGHT_MAX           (4444)

/*--------------------------------------------------------------------------
 * reasonCode taken from 802.11 standard.
 * ------------------------------------------------------------------------*/

typedef enum {
	eSAP_RC_RESERVED0,              /*0 */
	eSAP_RC_UNSPECIFIED,            /*1 */
	eSAP_RC_PREV_AUTH_INVALID,      /*2 */
	eSAP_RC_STA_LEFT_DEAUTH,        /*3 */
	eSAP_RC_INACTIVITY_DISASSOC,    /*4 */
	eSAP_RC_AP_CAPACITY_FULL,       /*5 */
	eSAP_RC_CLS2_FROM_NON_AUTH_STA, /*6 */
	eSAP_RC_CLS3_FROM_NON_AUTH_STA, /*7 */
	eSAP_RC_STA_LEFT_DISASSOC,      /*8 */
	eSAP_RC_STA_NOT_AUTH,           /*9 */
	eSAP_RC_PC_UNACCEPTABLE,        /*10 */
	eSAP_RC_SC_UNACCEPTABLE,        /*11 */
	eSAP_RC_RESERVED1,              /*12 */
	eSAP_RC_INVALID_IE,             /*13 */
	eSAP_RC_MIC_FAIL,               /*14 */
	eSAP_RC_4_WAY_HANDSHAKE_TO,     /*15 */
	eSAP_RC_GO_KEY_HANDSHAKE_TO,    /*16 */
	eSAP_RC_IE_MISMATCH,            /*17 */
	eSAP_RC_INVALID_GRP_CHIPHER,    /*18 */
	eSAP_RC_INVALID_PAIR_CHIPHER,   /*19 */
	eSAP_RC_INVALID_AKMP,           /*20 */
	eSAP_RC_UNSUPPORTED_RSN,        /*21 */
	eSAP_RC_INVALID_RSN,            /*22 */
	eSAP_RC_1X_AUTH_FAILED,         /*23 */
	eSAP_RC_CHIPER_SUITE_REJECTED,  /*24 */
} eSapReasonCode;

typedef enum {
	eSAP_ACCEPT_UNLESS_DENIED = 0,
	eSAP_DENY_UNLESS_ACCEPTED = 1,
	/* this type is added to support accept & deny list at the same time */
	eSAP_SUPPORT_ACCEPT_AND_DENY = 2,
	/*In this mode all MAC addresses are allowed to connect */
	eSAP_ALLOW_ALL = 3,
} eSapMacAddrACL;

typedef enum {
	eSAP_BLACK_LIST = 0,   /* List of mac addresses NOT allowed to assoc */
	eSAP_WHITE_LIST = 1,   /* List of mac addresses allowed to assoc */
} eSapACLType;

typedef enum {
	ADD_STA_TO_ACL = 0,       /* cmd to add STA to access control list */
	DELETE_STA_FROM_ACL = 1,  /* cmd to del STA from access control list */
} eSapACLCmdType;

typedef enum {
	eSAP_START_BSS_EVENT = 0,     /* Event sent when BSS is started */
	eSAP_STOP_BSS_EVENT,          /* Event sent when BSS is stopped */
	eSAP_STA_ASSOC_IND,           /* Indicate assoc req to upper layers */
	/*
	 * Event sent when we have successfully associated a station and
	 * upper layer neeeds to allocate a context
	 */
	eSAP_STA_ASSOC_EVENT,
	/*
	 * Event sent when we have successfully reassociated a station and
	 * upper layer neeeds to allocate a context
	 */
	eSAP_STA_REASSOC_EVENT,
	/*
	 * Event sent when associated a station has disassociated as a
	 * result of various conditions
	 */
	eSAP_STA_DISASSOC_EVENT,
	/* Event sent when user called wlansap_set_key_sta */
	eSAP_STA_SET_KEY_EVENT,
	/* Event sent whenever there is MIC failure detected */
	eSAP_STA_MIC_FAILURE_EVENT,
	/* Event sent when user called wlansap_get_assoc_stations */
	eSAP_ASSOC_STA_CALLBACK_EVENT,
	/* Event send when user call  wlansap_get_wps_session_overlap */
	eSAP_GET_WPSPBC_SESSION_EVENT,
	/* Event send on WPS PBC probe request is received */
	eSAP_WPS_PBC_PROBE_REQ_EVENT,
	eSAP_REMAIN_CHAN_READY,
	eSAP_DISCONNECT_ALL_P2P_CLIENT,
	eSAP_MAC_TRIG_STOP_BSS_EVENT,
	/*
	 * Event send when a STA in neither white list or black list tries to
	 * associate in softap mode
	 */
	eSAP_UNKNOWN_STA_JOIN,
	/* Event send when a new STA is rejected association since softAP
	 * max assoc limit has reached
	 */
	eSAP_MAX_ASSOC_EXCEEDED,
	eSAP_CHANNEL_CHANGE_EVENT,
	eSAP_DFS_CAC_START,
	eSAP_DFS_CAC_INTERRUPTED,
	eSAP_DFS_CAC_END,
	eSAP_DFS_PRE_CAC_END,
	eSAP_DFS_RADAR_DETECT,
	eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC,
	/* Event sent when user need to get the DFS NOL from CNSS */
	eSAP_DFS_NOL_GET,
	/* Event sent when user need to set the DFS NOL to CNSS */
	eSAP_DFS_NOL_SET,
	/* No ch available after DFS RADAR detect */
	eSAP_DFS_NO_AVAILABLE_CHANNEL,
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	eSAP_ACS_SCAN_SUCCESS_EVENT,
#endif
	eSAP_ACS_CHANNEL_SELECTED,
	eSAP_ECSA_CHANGE_CHAN_IND,
} eSapHddEvent;

typedef enum {
	eSAP_OPEN_SYSTEM,
	eSAP_SHARED_KEY,
	eSAP_AUTO_SWITCH
} eSapAuthType;

typedef enum {
	/* Disassociation was internally initated from CORE stack */
	eSAP_MAC_INITATED_DISASSOC = 0x10000,
	/*
	 * Disassociation was internally initated from host by
	 * invoking wlansap_disassoc_sta call
	 */
	eSAP_USR_INITATED_DISASSOC
} eSapDisassocReason;

/*Handle bool over here*/
typedef enum {
	eSAP_FALSE,
	eSAP_TRUE,
} eSapBool;

typedef enum {
	eSAP_DFS_NOL_CLEAR,
	eSAP_DFS_NOL_RANDOMIZE,
} eSapDfsNolType;

/*---------------------------------------------------------------------------
  SAP PAL "status" and "reason" error code defines
  ---------------------------------------------------------------------------*/
typedef enum {
	eSAP_STATUS_SUCCESS,            /* Success.  */
	eSAP_STATUS_FAILURE,            /* General Failure.  */
	/* Channel not selected during intial scan.  */
	eSAP_START_BSS_CHANNEL_NOT_SELECTED,
	eSAP_ERROR_MAC_START_FAIL,     /* Failed to start Infra BSS */
} eSapStatus;

/*---------------------------------------------------------------------------
  SAP PAL "status" and "reason" error code defines
  ---------------------------------------------------------------------------*/
typedef enum {
	eSAP_WPSPBC_OVERLAP_IN120S,  /* Overlap */
	/* no WPS probe request in 120 second */
	eSAP_WPSPBC_NO_WPSPBC_PROBE_REQ_IN120S,
	/* One WPS probe request in 120 second  */
	eSAP_WPSPBC_ONE_WPSPBC_PROBE_REQ_IN120S,
} eWPSPBCOverlap;

/*----------------------------------------------------------------------------
 *  Typedefs
 * -------------------------------------------------------------------------*/
typedef struct sap_StartBssCompleteEvent_s {
	uint8_t status;
	uint8_t operatingChannel;
	uint16_t staId;         /* self StaID */
	uint8_t sessionId;      /* SoftAP SME session ID */
} tSap_StartBssCompleteEvent;

typedef struct sap_StopBssCompleteEvent_s {
	uint8_t status;
} tSap_StopBssCompleteEvent;

typedef struct sap_StationAssocIndication_s {
	struct qdf_mac_addr staMac;
	uint8_t assoId;
	uint8_t staId;
	uint8_t status;
	/* Required for indicating the frames to upper layer */
	uint32_t beaconLength;
	uint8_t *beaconPtr;
	uint32_t assocReqLength;
	uint8_t *assocReqPtr;
	bool fWmmEnabled;
	eCsrAuthType negotiatedAuthType;
	eCsrEncryptionType negotiatedUCEncryptionType;
	eCsrEncryptionType negotiatedMCEncryptionType;
	bool fAuthRequired;
} tSap_StationAssocIndication;

typedef struct sap_StationAssocReassocCompleteEvent_s {
	struct qdf_mac_addr staMac;
	uint8_t staId;
	uint8_t status;
	uint8_t ies[MAX_ASSOC_IND_IE_LEN];
	uint16_t iesLen;
	uint32_t statusCode;
	eSapAuthType SapAuthType;
	bool wmmEnabled;
	/* Required for indicating the frames to upper layer */
	uint32_t beaconLength;
	uint8_t *beaconPtr;
	uint32_t assocReqLength;
	uint8_t *assocReqPtr;
	uint32_t assocRespLength;
	uint8_t *assocRespPtr;
	uint8_t timingMeasCap;
	tSirSmeChanInfo chan_info;
} tSap_StationAssocReassocCompleteEvent;

typedef struct sap_StationDisassocCompleteEvent_s {
	struct qdf_mac_addr staMac;
	uint8_t staId;          /* STAID should not be used */
	uint8_t status;
	uint32_t statusCode;
	eSapDisassocReason reason;
} tSap_StationDisassocCompleteEvent;

typedef struct sap_StationSetKeyCompleteEvent_s {
	uint8_t status;
	struct qdf_mac_addr peerMacAddr;
} tSap_StationSetKeyCompleteEvent;

/*struct corresponding to SAP_STA_MIC_FAILURE_EVENT */
typedef struct sap_StationMICFailureEvent_s {
	struct qdf_mac_addr srcMacAddr;    /* address used to compute MIC */
	struct qdf_mac_addr staMac;        /* taMacAddr transmitter address */
	struct qdf_mac_addr dstMacAddr;
	eSapBool multicast;
	uint8_t IV1;            /* first byte of IV */
	uint8_t keyId;          /* second byte of IV */
	uint8_t TSC[SIR_CIPHER_SEQ_CTR_SIZE];           /* sequence number */

} tSap_StationMICFailureEvent;
/*Structure to return MAC address of associated stations */
typedef struct sap_AssocMacAddr_s {
	struct qdf_mac_addr staMac; /* Associated station's MAC address */
	uint8_t assocId;            /* Associated station's Association ID */
	uint8_t staId;              /* Allocated station Id */
	uint8_t ShortGI40Mhz;
	uint8_t ShortGI20Mhz;
	uint8_t Support40Mhz;
	uint32_t requestedMCRate;
	tSirSupportedRates supportedRates;
} tSap_AssocMacAddr, *tpSap_AssocMacAddr;

/*struct corresponding to SAP_ASSOC_STA_CALLBACK_EVENT */
typedef struct sap_AssocStaListEvent_s {
	QDF_MODULE_ID module;
	/* module id that was passed in wlansap_get_assoc_stations API */
	uint8_t noOfAssocSta;           /* Number of associated stations */
	tpSap_AssocMacAddr pAssocStas;
	/*
	 * Pointer to pre allocated memory to obtain list of
	 * associated stations passed in wlansap_get_assoc_stations API
	 */
} tSap_AssocStaListEvent;

typedef struct sap_GetWPSPBCSessionEvent_s {
	uint8_t status;
	/* module id that was passed in wlansap_get_assoc_stations API */
	QDF_MODULE_ID module;
	uint8_t UUID_E[16];             /* Unique identifier of the AP. */
	struct qdf_mac_addr addr;
	eWPSPBCOverlap wpsPBCOverlap;
} tSap_GetWPSPBCSessionEvent;

typedef struct sap_WPSPBCProbeReqEvent_s {
	uint8_t status;
	/* module id that was passed in wlansap_get_assoc_stations API */
	QDF_MODULE_ID module;
	tSirWPSPBCProbeReq WPSPBCProbeReq;
} tSap_WPSPBCProbeReqEvent;

typedef struct sap_ManagementFrameInfo_s {
	uint32_t nFrameLength;
	uint8_t frameType;
	uint32_t rxChan;           /* Channel of where packet is received */
	/*
	 * Point to a buffer contain the beacon, assoc req, assoc rsp frame,
	 * in that order user needs to use nBeaconLength, nAssocReqLength,
	 * nAssocRspLength to desice where each frame starts and ends.
	 */
	uint8_t *pbFrames;
} tSap_ManagementFrameInfo;

typedef struct sap_SendActionCnf_s {
	eSapStatus actionSendSuccess;
} tSap_SendActionCnf;

typedef struct sap_UnknownSTAJoinEvent_s {
	struct qdf_mac_addr macaddr;
} tSap_UnknownSTAJoinEvent;

typedef struct sap_MaxAssocExceededEvent_s {
	struct qdf_mac_addr macaddr;
} tSap_MaxAssocExceededEvent;

typedef struct sap_DfsNolInfo_s {
	uint16_t sDfsList;              /* size of pDfsList in byte */
	void *pDfsList;             /* pointer to pDfsList buffer */
} tSap_DfsNolInfo;

/**
 * sap_acs_ch_selected_s - the structure to hold the selected channels
 * @pri_channel:	   Holds the ACS selected primary channel
 * @sec_channel:	   Holds the ACS selected secondary channel
 *
 * Holds the primary and secondary channel selected by ACS and is
 * used to send it to the HDD.
 */
struct sap_ch_selected_s {
	uint16_t pri_ch;
	uint16_t ht_sec_ch;
	uint16_t vht_seg0_center_ch;
	uint16_t vht_seg1_center_ch;
	uint16_t ch_width;
};

/**
 * sap_roc_ready_ind_s - the structure to hold the scan id
 * @scan_id: scan identifier
 *
 * Holds scan identifier
 */
struct sap_roc_ready_ind_s {
	uint32_t scan_id;
};

/**
 * struct sap_ch_change_ind - channel change indication
 * @new_chan: channel to change
 */
struct sap_ch_change_ind {
	uint16_t new_chan;
};

/*
 * This struct will be filled in and passed to tpWLAN_SAPEventCB that is
 * provided during wlansap_start_bss call The event id corresponding to
 * structure  in the union is defined in comment next to the structure
 */

typedef struct sap_Event_s {
	eSapHddEvent sapHddEventCode;
	union {
		/*SAP_START_BSS_EVENT */
		tSap_StartBssCompleteEvent sapStartBssCompleteEvent;
		/*SAP_STOP_BSS_EVENT */
		tSap_StopBssCompleteEvent sapStopBssCompleteEvent;
		/*SAP_ASSOC_INDICATION */
		tSap_StationAssocIndication sapAssocIndication;
		/*SAP_STA_ASSOC_EVENT, SAP_STA_REASSOC_EVENT */
		tSap_StationAssocReassocCompleteEvent
				sapStationAssocReassocCompleteEvent;
		/*SAP_STA_DISASSOC_EVENT */
		tSap_StationDisassocCompleteEvent
				sapStationDisassocCompleteEvent;
		/*SAP_STA_SET_KEY_EVENT */
		tSap_StationSetKeyCompleteEvent sapStationSetKeyCompleteEvent;
		/*SAP_STA_MIC_FAILURE_EVENT */
		tSap_StationMICFailureEvent sapStationMICFailureEvent;
		/*SAP_ASSOC_STA_CALLBACK_EVENT */
		tSap_AssocStaListEvent sapAssocStaListEvent;
		/*SAP_GET_WPSPBC_SESSION_EVENT */
		tSap_GetWPSPBCSessionEvent sapGetWPSPBCSessionEvent;
		/*eSAP_WPS_PBC_PROBE_REQ_EVENT */
		tSap_WPSPBCProbeReqEvent sapPBCProbeReqEvent;
		tSap_SendActionCnf sapActionCnf;
		/* eSAP_UNKNOWN_STA_JOIN */
		tSap_UnknownSTAJoinEvent sapUnknownSTAJoin;
		/* eSAP_MAX_ASSOC_EXCEEDED */
		tSap_MaxAssocExceededEvent sapMaxAssocExceeded;
		/*eSAP_DFS_NOL_XXX */
		tSap_DfsNolInfo sapDfsNolInfo;
		struct sap_ch_selected_s sap_ch_selected;
		struct sap_roc_ready_ind_s sap_roc_ind;
		struct sap_ch_change_ind sap_chan_cng_ind;
	} sapevt;
} tSap_Event, *tpSap_Event;

typedef struct sap_SSID {
	uint8_t length;
	uint8_t ssId[MAX_SSID_LEN];
} qdf_packed tSap_SSID_t;

typedef struct sap_SSIDInfo {
	tSap_SSID_t ssid;     /* SSID of the AP */
	/* SSID should/shouldn't be bcast in probe RSP & beacon */
	uint8_t ssidHidden;
} qdf_packed tSap_SSIDInfo_t;

struct sap_acs_cfg {
	/* ACS Algo Input */
	uint8_t    acs_mode;
	uint32_t    hw_mode;
	uint8_t    start_ch;
	uint8_t    end_ch;
	uint8_t    *ch_list;
	uint8_t    ch_list_count;
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	uint8_t    skip_scan_status;
	uint8_t    skip_scan_range1_stch;
	uint8_t    skip_scan_range1_endch;
	uint8_t    skip_scan_range2_stch;
	uint8_t    skip_scan_range2_endch;
#endif

	uint16_t   ch_width;
	uint8_t    pcl_channels[NUM_CHANNELS];
	uint32_t   pcl_ch_count;
	/* ACS Algo Output */
	uint8_t    pri_ch;
	uint8_t    ht_sec_ch;
	uint8_t    vht_seg0_center_ch;
	uint8_t    vht_seg1_center_ch;
};

/*
 * enum vendor_ie_access_policy- access policy
 * @ACCESS_POLICY_NONE: access policy attribute is not valid
 * @ACCESS_POLICY_RESPOND_IF_IE_IS_PRESENT: respond to probe req/assoc req
 *  only if ie is present
 * @ACCESS_POLICY_DONOT_RESPOND_IF_IE_IS_PRESENT: do not respond to probe req/
 *  assoc req if ie is present
*/
enum vendor_ie_access_policy {
	ACCESS_POLICY_NONE,
	ACCESS_POLICY_RESPOND_IF_IE_IS_PRESENT,
	ACCESS_POLICY_DONOT_RESPOND_IF_IE_IS_PRESENT,
};

/*
 * enum sap_acs_dfs_mode- state of DFS mode
 * @ACS_DFS_MODE_NONE: DFS mode attribute is not valid
 * @ACS_DFS_MODE_ENABLE:  DFS mode is enabled
 * @ACS_DFS_MODE_DISABLE: DFS mode is disabled
 * @ACS_DFS_MODE_DEPRIORITIZE: Deprioritize DFS channels in scanning
 */
enum  sap_acs_dfs_mode {
	ACS_DFS_MODE_NONE,
	ACS_DFS_MODE_ENABLE,
	ACS_DFS_MODE_DISABLE,
	ACS_DFS_MODE_DEPRIORITIZE
};

typedef struct sap_Config {
	tSap_SSIDInfo_t SSIDinfo;
	eCsrPhyMode SapHw_mode;         /* Wireless Mode */
	eSapMacAddrACL SapMacaddr_acl;
	struct qdf_mac_addr accept_mac[MAX_ACL_MAC_ADDRESS]; /* MAC filtering */
	bool ieee80211d;      /* Specify if 11D is enabled or disabled */
	bool protEnabled;     /* Specify if protection is enabled or disabled */
	/* Specify if OBSS protection is enabled or disabled */
	bool obssProtEnabled;
	struct qdf_mac_addr deny_mac[MAX_ACL_MAC_ADDRESS];  /* MAC filtering */
	struct qdf_mac_addr self_macaddr;       /* self macaddress or BSSID */
	uint8_t channel;          /* Operation channel */
	uint8_t sec_ch;
	struct ch_params_s ch_params;
	uint32_t ch_width_orig;
	uint8_t max_num_sta;      /* maximum number of STAs in station table */
	uint8_t dtim_period;      /* dtim interval */
	uint8_t num_accept_mac;
	uint8_t num_deny_mac;
	/* Max ie length 255 * 2(WPA+RSN) + 2 bytes(vendor specific ID) * 2 */
	uint8_t RSNWPAReqIE[(SIR_MAC_MAX_IE_LENGTH * 2) + 4];
	/* it is ignored if [0] is 0. */
	uint8_t countryCode[WNI_CFG_COUNTRY_CODE_LEN];
	uint8_t RSNAuthType;
	uint8_t RSNEncryptType;
	uint8_t mcRSNEncryptType;
	eSapAuthType authType;
	bool privacy;
	bool UapsdEnable;
	bool fwdWPSPBCProbeReq;
	/* 0 - disabled, 1 - not configured , 2 - configured */
	uint8_t wps_state;
	uint16_t ht_capab;
	uint16_t RSNWPAReqIELength;     /* The byte count in the pWPAReqIE */
	uint32_t beacon_int;            /* Beacon Interval */
	uint32_t ap_table_max_size;
	uint32_t ap_table_expiration_time;
	uint32_t ht_op_mode_fixed;
	enum tQDF_ADAPTER_MODE persona; /* Tells us which persona, GO or AP */
	uint8_t disableDFSChSwitch;
	bool enOverLapCh;
#ifdef WLAN_FEATURE_11W
	bool mfpRequired;
	bool mfpCapable;
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif
	struct sap_acs_cfg acs_cfg;
	uint16_t probeRespIEsBufferLen;
	/* buffer for addn ies comes from hostapd */
	void *pProbeRespIEsBuffer;
	uint16_t assocRespIEsLen;
	/* buffer for addn ies comes from hostapd */
	void *pAssocRespIEsBuffer;
	uint16_t probeRespBcnIEsLen;
	/* buffer for addn ies comes from hostapd */
	void *pProbeRespBcnIEsBuffer;
	uint8_t sap_dot11mc; /* Specify if 11MC is enabled or disabled*/
	uint8_t beacon_tx_rate;
	uint8_t *vendor_ie;
	enum vendor_ie_access_policy vendor_ie_access_policy;
	uint16_t sta_inactivity_timeout;
	uint16_t tx_pkt_fail_cnt_threshold;
	uint8_t short_retry_limit;
	uint8_t long_retry_limit;
	uint8_t ampdu_size;
	tSirMacRateSet supp_rate_set;
	tSirMacRateSet extended_rate_set;
	enum sap_acs_dfs_mode acs_dfs_mode;
} tsap_Config_t;

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
typedef enum {
	eSAP_DO_NEW_ACS_SCAN,
	eSAP_DO_PAR_ACS_SCAN,
	eSAP_SKIP_ACS_SCAN
} tSap_skip_acs_scan;
#endif

typedef enum {
	eSAP_WPS_PROBE_RSP_IE,
	eSAP_WPS_BEACON_IE,
	eSAP_WPS_ASSOC_RSP_IE
} eSapWPSIE_CODE;

typedef struct sSapName {
	uint8_t num_name;
	uint8_t name[MAX_NAME_SIZE];
} tSapName;

typedef struct sSapText {
	uint8_t num_text;
	uint8_t text[MAX_TEXT_SIZE];
} tSapText;

typedef enum {
	eSAP_DFS_DO_NOT_SKIP_CAC,
	eSAP_DFS_SKIP_CAC
} eSapDfsCACState_t;

typedef enum {
	eSAP_DFS_CHANNEL_USABLE,
	eSAP_DFS_CHANNEL_AVAILABLE,
	eSAP_DFS_CHANNEL_UNAVAILABLE
} eSapDfsChanStatus_t;

typedef struct sSapDfsNolInfo {
	uint8_t dfs_channel_number;
	eSapDfsChanStatus_t radar_status_flag;
	uint64_t radar_found_timestamp;
} tSapDfsNolInfo;

typedef struct sSapDfsInfo {
	qdf_mc_timer_t sap_dfs_cac_timer;
	uint8_t sap_radar_found_status;
	/*
	 * New channel to move to when a  Radar is
	 * detected on current Channel
	 */
	uint8_t target_channel;
	uint8_t last_radar_found_channel;
	uint8_t ignore_cac;
	eSapDfsCACState_t cac_state;
	uint8_t user_provided_target_channel;

	/*
	 * Requests for Channel Switch Announcement IE
	 * generation and transmission
	 */
	uint8_t csaIERequired;
	uint8_t numCurrentRegDomainDfsChannels;
	tSapDfsNolInfo sapDfsChannelNolList[NUM_5GHZ_CHANNELS];
	uint8_t is_dfs_cac_timer_running;
	/*
	 * New channel width and new channel bonding mode
	 * will only be updated via channel fallback mechanism
	 */
	enum phy_ch_width orig_chanWidth;
	enum phy_ch_width new_chanWidth;
	struct ch_params_s new_ch_params;

	/*
	 * INI param to enable/disable SAP W53
	 * channel operation.
	 */
	uint8_t is_dfs_w53_disabled;

	/*
	 * sap_operating_channel_location holds SAP indoor,
	 * outdoor location information. Currently, if this
	 * param is  set this Indoor/outdoor channel interop
	 * restriction will only be implemented for JAPAN
	 * regulatory domain.
	 *
	 * 0 - Indicates that location unknown
	 * (or) SAP Indoor/outdoor interop is allowed
	 *
	 * 1 - Indicates device is operating on Indoor channels
	 * and SAP cannot pick next random channel from outdoor
	 * list of channels when a radar is found on current operating
	 * DFS channel.
	 *
	 * 2 - Indicates device is operating on Outdoor Channels
	 * and SAP cannot pick next random channel from indoor
	 * list of channels when a radar is found on current
	 * operating DFS channel.
	 */
	uint8_t sap_operating_chan_preferred_location;

	/*
	 * Flag to indicate if DFS test mode is enabled and
	 * channel switch is disabled.
	 */
	uint8_t disable_dfs_ch_switch;
} tSapDfsInfo;

typedef struct tagSapCtxList {
	uint8_t sessionID;
	void *pSapContext;
	enum tQDF_ADAPTER_MODE sapPersona;
} tSapCtxList, tpSapCtxList;

typedef struct tagSapStruct {
	/* Information Required for SAP DFS Master mode */
	tSapDfsInfo SapDfsInfo;
	tSapCtxList sapCtxList[SAP_MAX_NUM_SESSION];
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	bool sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	bool enable_dfs_phy_error_logs;
} tSapStruct, *tpSapStruct;

#define WPS_PROBRSP_VER_PRESENT                          0x00000001
#define WPS_PROBRSP_STATE_PRESENT                        0x00000002
#define WPS_PROBRSP_APSETUPLOCK_PRESENT                  0x00000004
#define WPS_PROBRSP_SELECTEDREGISTRA_PRESENT             0x00000008
#define WPS_PROBRSP_DEVICEPASSWORDID_PRESENT             0x00000010
#define WPS_PROBRSP_SELECTEDREGISTRACFGMETHOD_PRESENT    0x00000020
#define WPS_PROBRSP_RESPONSETYPE_PRESENT                 0x00000040
#define WPS_PROBRSP_UUIDE_PRESENT                        0x00000080
#define WPS_PROBRSP_MANUFACTURE_PRESENT                  0x00000100
#define WPS_PROBRSP_MODELNAME_PRESENT                    0x00000200
#define WPS_PROBRSP_MODELNUMBER_PRESENT                  0x00000400
#define WPS_PROBRSP_SERIALNUMBER_PRESENT                 0x00000800
#define WPS_PROBRSP_PRIMARYDEVICETYPE_PRESENT            0x00001000
#define WPS_PROBRSP_DEVICENAME_PRESENT                   0x00002000
#define WPS_PROBRSP_CONFIGMETHODS_PRESENT                0x00004000
#define WPS_PROBRSP_RF_BANDS_PRESENT                     0x00008000

typedef struct sap_WPSProbeRspIE_s {
	uint32_t FieldPresent;
	uint32_t Version;         /* Version. 0x10 = version 1.0, 0x11 = etc. */
	uint32_t wpsState;        /* 1 = unconfigured, 2 = configured. */
	bool APSetupLocked;       /* Must be included if value is true */
	/* indicates if user has recently activated a reg to add an Enrollee. */
	bool SelectedRegistra;
	uint16_t DevicePasswordID;              /* Device Password ID */
	/* Selected Registrar config method */
	uint16_t SelectedRegistraCfgMethod;
	uint8_t ResponseType;           /* Response type */
	uint8_t UUID_E[16];             /* Unique identifier of the AP. */
	tSapName Manufacture;
	tSapText ModelName;
	tSapText ModelNumber;
	tSapText SerialNumber;
	/* Device Category ID: 1Computer, 2Input Device, ... */
	uint32_t PrimaryDeviceCategory;
	/* Vendor specific OUI for Device Sub Category */
	uint8_t PrimaryDeviceOUI[4];
	/*
	 * Device Sub Category ID: 1-PC,
	 * 2-Server if Device Category is computer
	 */
	uint32_t DeviceSubCategory;
	tSapText DeviceName;
	uint16_t ConfigMethod;  /* Configuaration method */
	uint8_t RFBand;         /* RF bands available on the AP */
} tSap_WPSProbeRspIE;

#define WPS_BEACON_VER_PRESENT                         0x00000001
#define WPS_BEACON_STATE_PRESENT                       0x00000002
#define WPS_BEACON_APSETUPLOCK_PRESENT                 0x00000004
#define WPS_BEACON_SELECTEDREGISTRA_PRESENT            0x00000008
#define WPS_BEACON_DEVICEPASSWORDID_PRESENT            0x00000010
#define WPS_BEACON_SELECTEDREGISTRACFGMETHOD_PRESENT   0x00000020
#define WPS_BEACON_UUIDE_PRESENT                       0x00000080
#define WPS_BEACON_RF_BANDS_PRESENT                    0x00000100

typedef struct sap_WPSBeaconIE_s {
	uint32_t FieldPresent;
	uint32_t Version;         /* Version. 0x10 = version 1.0, 0x11 = etc. */
	uint32_t wpsState;        /* 1 = unconfigured, 2 = configured. */
	bool APSetupLocked;       /* Must be included if value is true */
	/* indicates if user has recently activated a reg to add an Enrollee. */
	bool SelectedRegistra;
	uint16_t DevicePasswordID;              /* Device Password ID */
	uint16_t SelectedRegistraCfgMethod;     /* Selected reg config method */
	uint8_t UUID_E[16];     /* Unique identifier of the AP. */
	uint8_t RFBand;         /* RF bands available on the AP */
} tSap_WPSBeaconIE;

#define WPS_ASSOCRSP_VER_PRESENT             0x00000001
#define WPS_ASSOCRSP_RESPONSETYPE_PRESENT    0x00000002

typedef struct sap_WPSAssocRspIE_s {
	uint32_t FieldPresent;
	uint32_t Version;
	uint8_t ResposeType;
} tSap_WPSAssocRspIE;

typedef struct sap_WPSIE_s {
	eSapWPSIE_CODE sapWPSIECode;
	union {
		tSap_WPSProbeRspIE sapWPSProbeRspIE; /* WPS Set Probe Resp IE */
		tSap_WPSBeaconIE sapWPSBeaconIE;     /* WPS Set Beacon IE */
		tSap_WPSAssocRspIE sapWPSAssocRspIE; /* WPS Set Assoc Resp IE */
	} sapwpsie;
} tSap_WPSIE, *tpSap_WPSIE;

#ifdef WLANTL_DEBUG
#define MAX_RATE_INDEX      136
#define MAX_NUM_RSSI        100
#define MAX_RSSI_INTERVAL     5
#endif

typedef struct sap_SoftapStats_s {
	uint32_t txUCFcnt;
	uint32_t txMCFcnt;
	uint32_t txBCFcnt;
	uint32_t txUCBcnt;
	uint32_t txMCBcnt;
	uint32_t txBCBcnt;
	uint32_t rxUCFcnt;
	uint32_t rxMCFcnt;
	uint32_t rxBCFcnt;
	uint32_t rxUCBcnt;
	uint32_t rxMCBcnt;
	uint32_t rxBCBcnt;
	uint32_t rxBcnt;
	uint32_t rxBcntCRCok;
	uint32_t rxRate;
#ifdef WLANTL_DEBUG
	uint32_t pktCounterRateIdx[MAX_RATE_INDEX];
	uint32_t pktCounterRssi[MAX_NUM_RSSI];
#endif
} tSap_SoftapStats, *tpSap_SoftapStats;

#ifdef FEATURE_WLAN_CH_AVOID
/* Store channel safety information */
typedef struct {
	uint16_t channelNumber;
	bool isSafe;
} sapSafeChannelType;
#endif /* FEATURE_WLAN_CH_AVOID */
void sap_cleanup_channel_list(void *sapContext);
void sapCleanupAllChannelList(void);
QDF_STATUS wlansap_set_wps_ie(void *p_cds_gctx, tSap_WPSIE *pWPSIe);
QDF_STATUS wlansap_update_wps_ie(void *p_cds_gctx);
QDF_STATUS wlansap_stop_Wps(void *p_cds_gctx);
QDF_STATUS wlansap_get_wps_state(void *p_cds_gctx, bool *pbWPSState);
void *wlansap_open(void *p_cds_gctx);
QDF_STATUS wlansap_global_init(void);
QDF_STATUS wlansap_global_deinit(void);
QDF_STATUS wlansap_start(void *p_cds_gctx, enum tQDF_ADAPTER_MODE mode,
			 uint8_t *addr, uint32_t *session_id);
QDF_STATUS wlansap_stop(void *p_cds_gctx);
QDF_STATUS wlansap_close(void *p_cds_gctx);
typedef QDF_STATUS (*tpWLAN_SAPEventCB)(tpSap_Event pSapEvent,
					void *pUsrContext);
uint8_t wlansap_get_state(void *p_cds_gctx);

QDF_STATUS wlansap_start_bss(void *p_cds_gctx,
	 tpWLAN_SAPEventCB pSapEventCallback,
	 tsap_Config_t *pConfig, void *pUsrContext);
QDF_STATUS wlan_sap_set_pre_cac_status(void *ctx, bool status,
		tHalHandle handle);
QDF_STATUS wlan_sap_set_chan_before_pre_cac(void *ctx,
		uint8_t chan_before_pre_cac);
QDF_STATUS wlan_sap_set_pre_cac_complete_status(void *ctx, bool status);
bool wlan_sap_is_pre_cac_active(tHalHandle handle);
QDF_STATUS wlan_sap_get_pre_cac_vdev_id(tHalHandle handle, uint8_t *vdev_id);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t wlansap_check_cc_intf(void *Ctx);
#endif
QDF_STATUS wlansap_set_mac_acl(void *p_cds_gctx, tsap_Config_t *pConfig);
QDF_STATUS wlansap_stop_bss(void *p_cds_gctx);
QDF_STATUS wlansap_disassoc_sta(void *p_cds_gctx,
				struct tagCsrDelStaParams *p_del_sta_params);
QDF_STATUS wlansap_deauth_sta(void *p_cds_gctx,
			struct tagCsrDelStaParams *pDelStaParams);
QDF_STATUS wlansap_set_channel_change_with_csa(void *p_cds_gctx,
			uint32_t targetChannel, enum phy_ch_width target_bw);
QDF_STATUS wlansap_set_key_sta(void *p_cds_gctx,
	tCsrRoamSetKey *pSetKeyInfo);
QDF_STATUS wlansap_get_assoc_stations(void *p_cds_gctx,
	 QDF_MODULE_ID module, tpSap_AssocMacAddr pAssocStas);
QDF_STATUS wlansap_remove_wps_session_overlap(void *p_cds_gctx,
	struct qdf_mac_addr pRemoveMac);
QDF_STATUS wlansap_get_wps_session_overlap(void *p_cds_gctx);
QDF_STATUS wlansap_set_counter_measure(void *p_cds_gctx, bool bEnable);
QDF_STATUS wlan_sap_getstation_ie_information(void *p_cds_gctx,
	uint32_t *pLen, uint8_t *pBuf);
QDF_STATUS wlansap_clear_acl(void *p_cds_gctx);
QDF_STATUS wlansap_get_acl_accept_list(void *p_cds_gctx,
	 struct qdf_mac_addr *pAcceptList, uint8_t *nAcceptList);
QDF_STATUS wlansap_get_acl_deny_list(void *pCtx,
	struct qdf_mac_addr *pDenyList, uint8_t *nDenyList);
QDF_STATUS wlansap_set_mode(void *p_cds_gctx, uint32_t mode);
QDF_STATUS wlansap_get_acl_mode(void *p_cds_gctx, eSapMacAddrACL *mode);
QDF_STATUS wlansap_modify_acl(void *p_cds_gctx,
	 uint8_t *pPeerStaMac, eSapACLType listType, eSapACLCmdType cmd);
QDF_STATUS wlansap_set_wparsn_ies
	(void *p_cds_gctx, uint8_t *pWPARSNIEs, uint32_t WPARSNIEsLen);
QDF_STATUS wlansap_send_action
	(void *p_cds_gctx,
	 const uint8_t *pBuf, uint32_t len, uint16_t wait, uint16_t channel_freq);
QDF_STATUS wlansap_remain_on_channel
	(void *p_cds_gctx,
	 uint8_t channel,
	 uint32_t duration, remainOnChanCallback callback, void *pContext,
	 uint32_t *scan_id);
QDF_STATUS wlansap_cancel_remain_on_channel(void *p_cds_gctx,
		uint32_t scan_id);
QDF_STATUS wlansap_register_mgmt_frame
	(void *p_cds_gctx,
	 uint16_t frameType, uint8_t *matchData, uint16_t matchLen);
QDF_STATUS wlansap_de_register_mgmt_frame
	(void *p_cds_gctx,
	 uint16_t frameType, uint8_t *matchData, uint16_t matchLen);
QDF_STATUS wlansap_channel_change_request(void *p_cds_gctx,
		uint8_t tArgetChannel);
QDF_STATUS wlansap_start_beacon_req(void *pSapCtx);
QDF_STATUS wlansap_dfs_send_csa_ie_request(void *pSapCtx);
QDF_STATUS wlansap_get_dfs_ignore_cac(tHalHandle hHal, uint8_t *pIgnore_cac);
QDF_STATUS wlansap_set_dfs_ignore_cac(tHalHandle hHal, uint8_t ignore_cac);
QDF_STATUS wlansap_set_dfs_restrict_japan_w53(tHalHandle hHal,
			uint8_t disable_Dfs_JapanW3);

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
QDF_STATUS
wlan_sap_set_channel_avoidance(tHalHandle hal, bool sap_channel_avoidance);
#endif

QDF_STATUS wlansap_set_dfs_preferred_channel_location(tHalHandle hHal,
		uint8_t dfs_Preferred_Channels_location);
QDF_STATUS wlansap_set_dfs_target_chnl(tHalHandle hHal,
			uint8_t target_channel);
uint32_t wlan_sap_get_vht_ch_width(void *ctx);
void wlan_sap_set_vht_ch_width(void *ctx, uint32_t vht_channel_width);
QDF_STATUS wlansap_update_sap_config_add_ie(tsap_Config_t *pConfig,
		const uint8_t *
		pAdditionIEBuffer,
		uint16_t additionIELength,
		eUpdateIEsType updateType);
QDF_STATUS wlansap_reset_sap_config_add_ie(tsap_Config_t *pConfig,
			eUpdateIEsType updateType);
void wlansap_extend_to_acs_range(uint8_t *startChannelNum,
		uint8_t *endChannelNum,
		uint8_t *bandStartChannel,
		uint8_t *bandEndChannel);
QDF_STATUS wlansap_get_dfs_nol(void *pSapCtx, uint8_t *nol, uint32_t *nol_len);
QDF_STATUS wlansap_set_dfs_nol(void *pSapCtx, eSapDfsNolType conf);
void wlansap_populate_del_sta_params(const uint8_t *mac,
		uint16_t reason_code,
		uint8_t subtype,
		struct tagCsrDelStaParams *pDelStaParams);
QDF_STATUS wlansap_acs_chselect(void *pvos_gctx,
		tpWLAN_SAPEventCB pacs_event_callback,
		tsap_Config_t *pconfig,
		void *pusr_context);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef WLAN_QCT_WLANSAP_H */
