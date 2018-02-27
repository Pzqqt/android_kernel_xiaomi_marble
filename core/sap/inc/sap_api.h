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

#define       MAX_CHANNEL_LIST_LEN         256
#define       QDF_MAX_NO_OF_SAP_MODE       2    /* max # of SAP */
#define       SAP_MAX_NUM_SESSION          5
#define       SAP_MAX_OBSS_STA_CNT         1    /* max # of OBSS STA */
#define       SAP_ACS_WEIGHT_MAX           (26664)

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
	/* Event send on WPS PBC probe request is received */
	eSAP_WPS_PBC_PROBE_REQ_EVENT,
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
	/* No ch available after DFS RADAR detect */
	eSAP_DFS_NO_AVAILABLE_CHANNEL,
	eSAP_STOP_BSS_DUE_TO_NO_CHNL,
	eSAP_ACS_SCAN_SUCCESS_EVENT,
	eSAP_ACS_CHANNEL_SELECTED,
	eSAP_ECSA_CHANGE_CHAN_IND,
	eSAP_DFS_NEXT_CHANNEL_REQ,
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

/*---------------------------------------------------------------------------
  SAP Associated station types
  ---------------------------------------------------------------------------*/
typedef enum {
	eSTA_TYPE_NONE    = 0x00000000,  /* No station type */
	eSTA_TYPE_INFRA   = 0x00000001,  /* legacy station */
	eSTA_TYPE_P2P_CLI = 0x00000002,  /* p2p client */
} eStationType;

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
	uint8_t ecsa_capable;
} tSap_StationAssocIndication;

typedef struct sap_StationAssocReassocCompleteEvent_s {
	struct qdf_mac_addr staMac;
	eStationType staType;
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
	bool ampdu;
	bool sgi_enable;
	bool tx_stbc;
	bool rx_stbc;
	tSirMacHTChannelWidth ch_width;
	enum sir_sme_phy_mode mode;
	uint8_t max_supp_idx;
	uint8_t max_ext_idx;
	uint8_t max_mcs_idx;
	uint8_t rx_mcs_map;
	uint8_t tx_mcs_map;
	uint8_t ecsa_capable;
	tDot11fIEHTCaps ht_caps;
	tDot11fIEVHTCaps vht_caps;
} tSap_StationAssocReassocCompleteEvent;

typedef struct sap_StationDisassocCompleteEvent_s {
	struct qdf_mac_addr staMac;
	uint8_t staId;          /* STAID should not be used */
	uint8_t status;
	uint32_t statusCode;
	uint32_t reason_code;
	eSapDisassocReason reason;
	int rssi;
	int tx_rate;
	int rx_rate;
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
	bool multicast;
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
 * struct sap_acs_scan_complete_event - acs scan complete event
 * @status: status of acs scan
 * @channellist: acs scan channels
 * @num_of_channels: number of channels
 */
struct sap_acs_scan_complete_event {
	uint8_t status;
	uint8_t *channellist;
	uint8_t num_of_channels;
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
		struct sap_acs_scan_complete_event sap_acs_scan_comp;
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
	eCsrPhyMode hw_mode;
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
	uint8_t    pcl_channels[QDF_MAX_NUM_CHAN];
	uint8_t    pcl_channels_weight_list[QDF_MAX_NUM_CHAN];
	uint32_t   pcl_ch_count;
	uint8_t    is_ht_enabled;
	uint8_t    is_vht_enabled;
	/* ACS Algo Output */
	uint8_t    pri_ch;
	uint8_t    ht_sec_ch;
	uint8_t    vht_seg0_center_ch;
	uint8_t    vht_seg1_center_ch;
	uint32_t   band;
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
	struct ch_params ch_params;
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
	enum QDF_OPMODE persona; /* Tells us which persona, GO or AP */
	uint8_t disableDFSChSwitch;
	bool enOverLapCh;
#ifdef WLAN_FEATURE_11W
	bool mfpRequired;
	bool mfpCapable;
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif
	uint32_t auto_channel_select_weight;
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
	uint16_t beacon_tx_rate;
	uint8_t *vendor_ie;
	enum vendor_ie_access_policy vendor_ie_access_policy;
	uint16_t sta_inactivity_timeout;
	uint16_t tx_pkt_fail_cnt_threshold;
	uint8_t short_retry_limit;
	uint8_t long_retry_limit;
	uint8_t ampdu_size;
	tSirMacRateSet supported_rates;
	tSirMacRateSet extended_rates;
	enum sap_acs_dfs_mode acs_dfs_mode;
	struct hdd_channel_info *channel_info;
	uint32_t channel_info_count;
	bool dfs_cac_offload;
	/* beacon count before channel switch */
	uint8_t sap_chanswitch_beacon_cnt;
	uint8_t sap_chanswitch_mode;
	bool chan_switch_hostapd_rate_enabled;
	bool dfs_beacon_tx_enhanced;
	uint16_t reduced_beacon_interval;
} tsap_Config_t;

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
typedef enum {
	eSAP_DO_NEW_ACS_SCAN,
	eSAP_DO_PAR_ACS_SCAN,
	eSAP_SKIP_ACS_SCAN
} tSap_skip_acs_scan;
#endif

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
	struct ch_params new_ch_params;

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
	uint16_t tx_leakage_threshold;
	/* beacon count before channel switch */
	uint8_t sap_ch_switch_beacon_cnt;
	uint8_t sap_ch_switch_mode;
	bool dfs_beacon_tx_enhanced;
	uint16_t reduced_beacon_interval;
} tSapDfsInfo;

typedef struct tagSapCtxList {
	uint8_t sessionID;
	void *pSapContext;
	enum QDF_OPMODE sapPersona;
} tSapCtxList, tpSapCtxList;

typedef struct tagSapStruct {
	/* Information Required for SAP DFS Master mode */
	tSapDfsInfo SapDfsInfo;
	tSapCtxList sapCtxList[SAP_MAX_NUM_SESSION];
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	bool sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	bool acs_with_more_param;
	bool enable_dfs_phy_error_logs;
} tSapStruct, *tpSapStruct;

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

/**
 * struct sap_context - per-BSS Context for SAP
 *
 * struct sap_context is used to share per-BSS context between SAP and
 * its clients. A context is generated by sap_create_ctx() and is
 * destroyed by sap_destroy_ctx(). During the lifetime of the BSS the
 * SAP context is passed as the primary parameter to SAP APIs. Note
 * that by design the contents of the structure are opaque to the
 * clients and a SAP context pointer must only be dereferenced by SAP.
 */
struct sap_context;

/**
 * wlansap_roam_callback() - API to get the events for SAP persona
 * @pContext: sap context
 * @pCsrRoamInfo: pointer to SME CSR roam info structure
 * @roamId: roam id being used
 * @roamStatus: status of the event reported by SME to SAP
 * @roamResult: result of the event reported by SME to SAP
 *
 * Any activity like start_bss, stop_bss, and etc for SAP persona
 * happens, SME reports the result of those events to SAP through this
 * callback.
 *
 * Return: QDF_STATUS based on overall result
 */
QDF_STATUS wlansap_roam_callback(void *pContext,
				 struct csr_roam_info *pCsrRoamInfo,
				 uint32_t roamId,
				 eRoamCmdStatus roamStatus,
				 eCsrRoamResult roamResult);

/**
 * sap_create_ctx() - API to create the sap context
 *
 * This API assigns the sap context from global sap context pool
 * stored in gp_sap_ctx[i] array.
 *
 * Return: Pointer to the SAP context, or NULL if a context could not
 * be allocated
 */
struct sap_context *sap_create_ctx(void);

/**
 * sap_destroy_ctx - API to destroy the sap context
 * @sap_ctx: Pointer to the SAP context
 *
 * This API puts back the given sap context to global sap context pool which
 * makes current sap session's sap context invalid.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS sap_destroy_ctx(struct sap_context *sap_ctx);

/**
 * sap_init_ctx - Initialize the sap context
 * @sap_ctx: Pointer to the SAP context
 * @mode: Device mode
 * @addr: MAC address of the SAP
 * @session_id: Pointer to the session id
 *
 * sap_create_ctx() allocates the sap context which is uninitialized.
 * This API needs to be called to properly initialize the sap context
 * which is just created.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: BSS could not be started
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS sap_init_ctx(struct sap_context *sap_ctx,
			 enum QDF_OPMODE mode,
			 uint8_t *addr, uint32_t session_id);

/**
 * sap_deinit_ctx() - De-initialize the sap context
 * @sap_ctx: Pointer to the SAP context
 *
 * When SAP session is about to close, this API needs to be called
 * to de-initialize all the members of sap context structure, so that
 * nobody can accidently start using the sap context.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: BSS could not be stopped
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS sap_deinit_ctx(struct sap_context *sap_ctx);

/**
 * sap_is_auto_channel_select() - is channel AUTO_CHANNEL_SELECT
 * @sapcontext: Pointer to the SAP context
 *
 * Return: true on AUTO_CHANNEL_SELECT, false otherwise
 */
bool sap_is_auto_channel_select(struct sap_context *sapcontext);

QDF_STATUS wlansap_global_init(void);
QDF_STATUS wlansap_global_deinit(void);
typedef QDF_STATUS (*tpWLAN_SAPEventCB)(tpSap_Event pSapEvent,
					void *pUsrContext);

/**
 * wlansap_is_channel_in_nol_list() - This API checks if channel is
 * in nol list
 * @sap_ctx: SAP context pointer
 * @channelNumber: channel number
 * @chanBondState: channel bonding state
 *
 * Return: True if the channel is in the NOL list, false otherwise
 */
bool wlansap_is_channel_in_nol_list(struct sap_context *sap_ctx,
				    uint8_t channelNumber,
				    ePhyChanBondState chanBondState);

/**
 * wlansap_is_channel_leaking_in_nol() - This API checks if channel is leaking
 * in nol list
 * @sap_ctx: SAP context pointer
 * @channel: channel
 * @chan_bw: channel bandwidth
 *
 * Return: True/False
 */
bool wlansap_is_channel_leaking_in_nol(struct sap_context *sap_ctx,
				       uint8_t channel,
				       uint8_t chan_bw);

/**
 * wlansap_start_bss() - start BSS
 * @sap_ctx: Pointer to the SAP context
 * @pSapEventCallback: Callback function in HDD called by SAP to inform HDD
 *                        about SAP results
 * @pConfig: Pointer to configuration structure passed down from
 *                    HDD(HostApd for Android)
 * @pUsrContext: Parameter that will be passed back in all the SAP callback
 *               events.
 *
 * This api function provides SAP FSM event eWLAN_SAP_PHYSICAL_LINK_CREATE for
 * starting AP BSS
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_start_bss(struct sap_context *sap_ctx,
			     tpWLAN_SAPEventCB pSapEventCallback,
			     tsap_Config_t *pConfig, void *pUsrContext);

/**
 * wlansap_stop_bss() - stop BSS.
 * @sap_ctx: Pointer to SAP context
 *
 * This api function provides SAP FSM event eSAP_HDD_STOP_INFRA_BSS for
 * stopping AP BSS
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_stop_bss(struct sap_context *sap_ctx);

/**
 * wlan_sap_update_next_channel() - Update next channel configured using vendor
 * command in SAP context
 * @sap_ctx: SAP context
 * @channel: channel number
 * @chan_bw: channel width
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_update_next_channel(struct sap_context *sap_ctx,
					uint8_t channel,
					enum phy_ch_width chan_bw);

/**
 * wlan_sap_set_pre_cac_status() - Set the pre cac status
 * @sap_ctx: SAP context
 * @status: Status of pre cac
 * @handle: Global MAC handle
 *
 * Sets the pre cac status in the MAC context and updates the state
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_pre_cac_status(struct sap_context *sap_ctx,
				       bool status, tHalHandle handle);

/**
 * wlan_sap_set_chan_before_pre_cac() - Save the channel before pre cac
 * @sap_ctx: SAP context
 * @chan_before_pre_cac: Channel before pre cac
 *
 * Saves the channel that was in use before pre cac operation
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_chan_before_pre_cac(struct sap_context *sap_ctx,
					    uint8_t chan_before_pre_cac);

/**
 * wlan_sap_set_pre_cac_complete_status() - Sets pre cac complete status
 * @sap_ctx: SAP context
 * @status: Status of pre cac complete
 *
 * Sets the status of pre cac i.e., whether pre cac is complete or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_pre_cac_complete_status(struct sap_context *sap_ctx,
						bool status);

bool wlan_sap_is_pre_cac_active(tHalHandle handle);
QDF_STATUS wlan_sap_get_pre_cac_vdev_id(tHalHandle handle, uint8_t *vdev_id);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * wlansap_check_cc_intf() - Get interfering concurrent channel
 * @sap_ctx: SAP context pointer
 *
 * Determine if a Concurrent Channel is interfering.
 *
 * Return: Channel number of the interfering channel, or 0 if none.
 */
uint16_t wlansap_check_cc_intf(struct sap_context *sap_ctx);
#endif

/**
 * wlansap_set_mac_acl() - set MAC list entry in ACL.
 * @sap_ctx: Pointer to the SAP context
 * @pConfig: Pointer to SAP config.
 *
 * This api function provides SAP to set mac list entry in accept list as well
 * as deny list
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_set_mac_acl(struct sap_context *sap_ctx,
			       tsap_Config_t *pConfig);

/**
 * wlansap_disassoc_sta() - initiate disassociation of station.
 * @sap_ctx: Pointer to the SAP context
 * @p_del_sta_params: pointer to station deletion parameters
 *
 * This api function provides for Ap App/HDD initiated disassociation of station
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_disassoc_sta(struct sap_context *sap_ctx,
				struct csr_del_sta_params *p_del_sta_params);

/**
 * wlansap_deauth_sta() - Ap App/HDD initiated deauthentication of station
 * @pStaCtx : Pointer to the SAP context
 * @pDelStaParams : Pointer to parameters of the station to deauthenticate
 *
 * This api function provides for Ap App/HDD initiated deauthentication of
 * station
 *
 * Return: The QDF_STATUS code associated with performing the operation
 */
QDF_STATUS wlansap_deauth_sta(struct sap_context *sap_ctx,
			      struct csr_del_sta_params *pDelStaParams);

/**
 * wlansap_set_channel_change_with_csa() - Set channel change with CSA
 * @sapContext: Pointer to SAP context
 * @targetChannel: Target channel
 * @target_bw: Target bandwidth
 * @strict: if true switch to the requested channel always, fail
 *        otherwise
 *
 * This api function does a channel change to the target channel specified.
 * CSA IE is included in the beacons before doing a channel change.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_set_channel_change_with_csa(struct sap_context *sapContext,
					       uint32_t targetChannel,
					       enum phy_ch_width target_bw,
					       bool strict);

/**
 * wlansap_set_key_sta() - set keys for a stations.
 * @sap_ctx: Pointer to the SAP context
 * @pSetKeyInfo : tCsrRoamSetKey structure for the station
 *
 * This api function provides for Ap App/HDD to set key for a station.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_set_key_sta(struct sap_context *sap_ctx,
			       tCsrRoamSetKey *pSetKeyInfo);

/**
 * wlan_sap_getstation_ie_information() - RSNIE Population
 * @sap_ctx: Pointer to the SAP context
 * @len: Length of @buf
 * @buf: RSNIE IE data
 *
 *  Populate RSN IE from CSR to HDD context
 *
 * Return: QDF_STATUS enumeration
 */

QDF_STATUS wlan_sap_getstation_ie_information(struct sap_context *sap_ctx,
					      uint32_t *len, uint8_t *buf);

/**
 * wlansap_clear_acl() - Clear all ACLs
 * @sap_ctx: Pointer to the SAP context
 *
 * Return: QDF_STATUS. If success the ACLs were cleared, otherwise an
 *    error occurred.
 */
QDF_STATUS wlansap_clear_acl(struct sap_context *sap_ctx);

/**
 * wlansap_get_acl_accept_list() - Get ACL accept list
 * @sap_ctx: Pointer to the SAP context
 * @pAcceptList: Pointer to the buffer to store the ACL accept list
 * @nAcceptList: Pointer to the location to store the number of
 *    entries in the ACL accept list.
 *
 * Return: QDF_STATUS. If success the data was returned, otherwise an
 *    error occurred.
 */
QDF_STATUS wlansap_get_acl_accept_list(struct sap_context *sap_ctx,
				       struct qdf_mac_addr *pAcceptList,
				       uint8_t *nAcceptList);

/**
 * wlansap_get_acl_deny_list() - Get ACL deny list
 * @sap_ctx: Pointer to the SAP context
 * @pDenyList: Pointer to the buffer to store the ACL deny list
 * @nDenyList: Pointer to the location to store the number of
 *    entries in the ACL deny list.
 *
 * Return: QDF_STATUS. If success the data was returned, otherwise an
 *    error occurred.
 */
QDF_STATUS wlansap_get_acl_deny_list(struct sap_context *sap_ctx,
				     struct qdf_mac_addr *pDenyList,
				     uint8_t *nDenyList);

/**
 * wlansap_set_acl_mode() - Set the SAP ACL mode
 * @sap_ctx: The SAP context pointer
 * @mode: the desired ACL mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_set_acl_mode(struct sap_context *sap_ctx,
				eSapMacAddrACL mode);

/**
 * wlansap_get_acl_mode() - Get the SAP ACL mode
 * @sap_ctx: The SAP context pointer
 * @mode: Pointer where to return the current ACL mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_get_acl_mode(struct sap_context *sap_ctx,
				eSapMacAddrACL *mode);

/**
 * wlansap_modify_acl() - Update ACL entries
 * @sap_ctx: Pointer to the SAP context
 * @peer_sta_mac: peer sta mac to be updated.
 * @list_type: white/Black list type.
 * @cmd: command to be executed on ACL.
 *
 * This function is called when a peer needs to be added or deleted from the
 * white/black ACL
 *
 * Return: Status
 */
QDF_STATUS wlansap_modify_acl(struct sap_context *sap_ctx,
			      uint8_t *peer_sta_mac,
			      eSapACLType list_type, eSapACLCmdType cmd);

/**
 * wlansap_register_mgmt_frame() - register management frame
 * @sap_ctx: Pointer to SAP context
 * @frame_type: frame type that needs to be registered with PE.
 * @match_data: pointer to data which should be matched after @frame_type
 *              is matched.
 * @match_len: Length of the @match_data
 *
 * HDD use this API to register specified type of frame with CORE stack.
 * On receiving such kind of frame CORE stack should pass this frame to HDD
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_register_mgmt_frame(struct sap_context *sap_ctx,
				       uint16_t frame_type,
				       uint8_t *match_data,
				       uint16_t match_len);

/**
 * wlansap_de_register_mgmt_frame() - de register management frame
 * @sap_ctx: Pointer to SAP context
 * @frame_type: frame type that needs to be de-registered with PE.
 * @match_data: pointer to data which should be matched after @frame_type
 *              is matched.
 * @match_len: Length of the @match_data
 *
 * HDD use this API to deregister a previously registered frame
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_de_register_mgmt_frame(struct sap_context *sap_ctx,
					  uint16_t frame_type,
					  uint8_t *match_data,
					  uint16_t match_len);

/**
 * wlansap_channel_change_request() - Send channel change request
 * @sapContext: Pointer to the SAP context
 * @target_channel: Target channel
 *
 * This API is used to send an Indication to SME/PE to change the
 * current operating channel to a different target channel.
 *
 * The Channel change will be issued by SAP under the following
 * scenarios.
 * 1. A radar indication is received  during SAP CAC WAIT STATE and
 *    channel change is required.
 * 2. A radar indication is received during SAP STARTED STATE and
 *    channel change is required.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *   QDF_STATUS_SUCCESS:  Success
 *
 */
QDF_STATUS wlansap_channel_change_request(struct sap_context *sapContext,
					  uint8_t target_channel);

/**
 * wlansap_start_beacon_req() - Send Start Beaconing Request
 * @sap_ctx: Pointer to the SAP context
 *
 * This API is used to send an Indication to SME/PE to start
 * beaconing on the current operating channel.
 *
 * When SAP is started on DFS channel and when ADD BSS RESP is received
 * LIM temporarily holds off Beaconing for SAP to do CAC WAIT. When
 * CAC WAIT is done SAP resumes the Beacon Tx by sending a start beacon
 * request to LIM.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *   QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_start_beacon_req(struct sap_context *sap_ctx);

/**
 * wlansap_dfs_send_csa_ie_request() - Send CSA IE
 * @sap_ctx: Pointer to the SAP context
 *
 * This API is used to send channel switch announcement request to PE
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *    QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_dfs_send_csa_ie_request(struct sap_context *sap_ctx);

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

/**
 * wlan_sap_get_roam_profile() - Returns sap roam profile.
 * @sap_ctx:	Pointer to Sap Context.
 *
 * This function provides the SAP roam profile.
 *
 * Return: SAP RoamProfile
 */
tCsrRoamProfile *wlan_sap_get_roam_profile(struct sap_context *sap_ctx);

/**
 * wlan_sap_get_phymode() - Returns sap phymode.
 * @sap_ctx:	Pointer to Sap Context.
 *
 * This function provides the SAP current phymode.
 *
 * Return: phymode
 */
eCsrPhyMode wlan_sap_get_phymode(struct sap_context *sap_ctx);

/**
 * wlan_sap_get_vht_ch_width() - Returns SAP VHT channel width.
 * @sap_ctx:	Pointer to Sap Context
 *
 * This function provides the SAP current VHT channel with.
 *
 * Return: VHT channel width
 */
uint32_t wlan_sap_get_vht_ch_width(struct sap_context *sap_ctx);

/**
 * wlan_sap_set_vht_ch_width() - Sets SAP VHT channel width.
 * @sap_ctx:		Pointer to Sap Context
 * @vht_channel_width:	SAP VHT channel width value.
 *
 * This function sets the SAP current VHT channel width.
 *
 * Return: None
 */
void wlan_sap_set_vht_ch_width(struct sap_context *sap_ctx,
			       uint32_t vht_channel_width);

QDF_STATUS wlansap_update_sap_config_add_ie(tsap_Config_t *pConfig,
		const uint8_t *
		pAdditionIEBuffer,
		uint16_t additionIELength,
		eUpdateIEsType updateType);
QDF_STATUS wlansap_reset_sap_config_add_ie(tsap_Config_t *pConfig,
			eUpdateIEsType updateType);
void wlansap_extend_to_acs_range(tHalHandle hal, uint8_t *startChannelNum,
		uint8_t *endChannelNum, uint8_t *bandStartChannel,
		uint8_t *bandEndChannel);

/**
 * wlansap_set_dfs_nol() - Set dfs nol
 * @sap_ctx: SAP context
 * @conf: set type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_set_dfs_nol(struct sap_context *sap_ctx,
			       eSapDfsNolType conf);

/**
 * wlan_sap_set_vendor_acs() - Set vendor specific acs in sap context
 * @sap_context: SAP context
 * @is_vendor_acs: if vendor specific acs is enabled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_vendor_acs(struct sap_context *sap_context,
				   bool is_vendor_acs);

void wlansap_populate_del_sta_params(const uint8_t *mac,
		uint16_t reason_code,
		uint8_t subtype,
		struct csr_del_sta_params *pDelStaParams);

/**
 * wlansap_acs_chselect() - Initiates acs channel selection
 * @sap_context:               Pointer to SAP context structure
 * @pacs_event_callback:       Callback function in hdd called by sap
 *                             to inform hdd about channel selection result
 * @pconfig:                   Pointer to configuration structure
 *                             passed down from hdd
 * @pusr_context:              Parameter that will be passed back in all
 *                             the sap callback events.
 *
 * This function serves as an api for hdd to initiate acs scan pre
 * start bss.
 *
 * Return: The QDF_STATUS code associated with performing the operation.
 */
QDF_STATUS wlansap_acs_chselect(struct sap_context *sap_context,
				tpWLAN_SAPEventCB pacs_event_callback,
				tsap_Config_t *pconfig,
				void *pusr_context);

/**
 * wlansap_get_chan_width() - get sap channel width.
 * @sap_ctx: pointer to the SAP context
 *
 * This function get channel width of sap.
 *
 * Return: sap channel width
 */
uint32_t wlansap_get_chan_width(struct sap_context *sap_ctx);

/**
 * wlansap_set_tx_leakage_threshold() - set sap tx leakage threshold.
 * @hal: HAL pointer
 * @tx_leakage_threshold: sap tx leakage threshold
 *
 * This function set sap tx leakage threshold.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wlansap_set_tx_leakage_threshold(tHalHandle hal,
			uint16_t tx_leakage_threshold);

/*
 * wlansap_set_invalid_session() - set session ID to invalid
 * @sap_ctx: pointer to the SAP context
 *
 * This function sets session ID to invalid
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_set_invalid_session(struct sap_context *sap_ctx);

/**
 * sap_dfs_set_current_channel() - Set current channel params in dfs component
 * @sap_ctx: sap context
 *
 * Set current channel params in dfs component, this info will be used to mark
 * the channels in nol when radar is detected.
 *
 * Return: None
 */
void sap_dfs_set_current_channel(void *sap_ctx);

/**
 * wlansap_cleanup_cac_timer() - Force cleanup DFS CAC timer
 * @sap_ctx: sap context
 *
 * Force cleanup DFS CAC timer when reset all adapters. It will not
 * check concurrency SAP since just called when reset all adapters.
 *
 * Return: None
 */
void wlansap_cleanup_cac_timer(struct sap_context *sap_ctx);

/**
 * wlansap_set_stop_bss_inprogress - sets the stop_bss_in_progress flag
 *
 * @sap_ctx: Pointer to the global SAP ctx
 * @in_progress: the value to be set to the stop_bss_in_progress_flag
 *
 * This function sets the value in in_progress parameter to the
 * stop_bss_in_progress flag in sap_context.
 *
 * Return: None
 */
void wlansap_set_stop_bss_inprogress(struct sap_context *sap_ctx,
					bool in_progress);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef WLAN_QCT_WLANSAP_H */
