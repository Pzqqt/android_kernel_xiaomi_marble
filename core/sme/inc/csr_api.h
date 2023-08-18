/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *   \file csr_api.h
 *
 *   Exports and types for the Common Scan and Roaming Module interfaces.
 */

#ifndef CSRAPI_H__
#define CSRAPI_H__

#include "sir_api.h"
#include "sir_mac_prot_def.h"
#include "csr_link_list.h"
#include "wlan_scan_public_structs.h"
#include "wlan_mlme_public_struct.h"
#include "wlan_mlme_main.h"

#define CSR_INVALID_SCANRESULT_HANDLE       (NULL)

enum csr_akm_type {
	/* never used */
	eCSR_AUTH_TYPE_NONE,
	/* MAC layer authentication types */
	eCSR_AUTH_TYPE_OPEN_SYSTEM,
	eCSR_AUTH_TYPE_SHARED_KEY,
	eCSR_AUTH_TYPE_SAE,
	eCSR_AUTH_TYPE_AUTOSWITCH,

	/* Upper layer authentication types */
	eCSR_AUTH_TYPE_WPA,
	eCSR_AUTH_TYPE_WPA_PSK,
	eCSR_AUTH_TYPE_WPA_NONE,

	eCSR_AUTH_TYPE_RSN,
	eCSR_AUTH_TYPE_RSN_PSK,
	eCSR_AUTH_TYPE_FT_RSN,
	eCSR_AUTH_TYPE_FT_RSN_PSK,
#ifdef FEATURE_WLAN_WAPI
	eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE,
	eCSR_AUTH_TYPE_WAPI_WAI_PSK,
#endif /* FEATURE_WLAN_WAPI */
	eCSR_AUTH_TYPE_CCKM_WPA,
	eCSR_AUTH_TYPE_CCKM_RSN,
	eCSR_AUTH_TYPE_RSN_PSK_SHA256,
	eCSR_AUTH_TYPE_RSN_8021X_SHA256,
	eCSR_AUTH_TYPE_FILS_SHA256,
	eCSR_AUTH_TYPE_FILS_SHA384,
	eCSR_AUTH_TYPE_FT_FILS_SHA256,
	eCSR_AUTH_TYPE_FT_FILS_SHA384,
	eCSR_AUTH_TYPE_DPP_RSN,
	eCSR_AUTH_TYPE_OWE,
	eCSR_AUTH_TYPE_SUITEB_EAP_SHA256,
	eCSR_AUTH_TYPE_SUITEB_EAP_SHA384,
	eCSR_AUTH_TYPE_OSEN,
	eCSR_AUTH_TYPE_FT_SAE,
	eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384,
	eCSR_NUM_OF_SUPPORT_AUTH_TYPE,
	eCSR_AUTH_TYPE_FAILED = 0xff,
	eCSR_AUTH_TYPE_UNKNOWN = eCSR_AUTH_TYPE_FAILED,

};

typedef enum {
	eCSR_ENCRYPT_TYPE_NONE,
	eCSR_ENCRYPT_TYPE_WEP40_STATICKEY,
	eCSR_ENCRYPT_TYPE_WEP104_STATICKEY,
	eCSR_ENCRYPT_TYPE_WEP40,
	eCSR_ENCRYPT_TYPE_WEP104,
	eCSR_ENCRYPT_TYPE_TKIP,
	eCSR_ENCRYPT_TYPE_AES,/* CCMP */
#ifdef FEATURE_WLAN_WAPI
	/* WAPI */
	eCSR_ENCRYPT_TYPE_WPI,
#endif  /* FEATURE_WLAN_WAPI */
	eCSR_ENCRYPT_TYPE_KRK,
	eCSR_ENCRYPT_TYPE_BTK,
	eCSR_ENCRYPT_TYPE_AES_CMAC,
	eCSR_ENCRYPT_TYPE_AES_GMAC_128,
	eCSR_ENCRYPT_TYPE_AES_GMAC_256,
	eCSR_ENCRYPT_TYPE_AES_GCMP,
	eCSR_ENCRYPT_TYPE_AES_GCMP_256,
	eCSR_ENCRYPT_TYPE_ANY,
	eCSR_NUM_OF_ENCRYPT_TYPE = eCSR_ENCRYPT_TYPE_ANY,

	eCSR_ENCRYPT_TYPE_FAILED = 0xff,
	eCSR_ENCRYPT_TYPE_UNKNOWN = eCSR_ENCRYPT_TYPE_FAILED,

} eCsrEncryptionType;

typedef enum {
	/* 11a/b/g only, no HT, no proprietary */
	eCSR_DOT11_MODE_abg = 0x0001,
	eCSR_DOT11_MODE_11a = 0x0002,
	eCSR_DOT11_MODE_11b = 0x0004,
	eCSR_DOT11_MODE_11g = 0x0008,
	eCSR_DOT11_MODE_11n = 0x0010,
	eCSR_DOT11_MODE_11g_ONLY = 0x0020,
	eCSR_DOT11_MODE_11n_ONLY = 0x0040,
	eCSR_DOT11_MODE_11b_ONLY = 0x0080,
	eCSR_DOT11_MODE_11ac = 0x0100,
	eCSR_DOT11_MODE_11ac_ONLY = 0x0200,
	/*
	 * This is for WIFI test. It is same as eWNIAPI_MAC_PROTOCOL_ALL
	 * It is for CSR internal use
	 */
	eCSR_DOT11_MODE_AUTO = 0x0400,
	eCSR_DOT11_MODE_11ax = 0x0800,
	eCSR_DOT11_MODE_11ax_ONLY = 0x1000,
	eCSR_DOT11_MODE_11be = 0x2000,
	eCSR_DOT11_MODE_11be_ONLY = 0x4000,

	/* specify the number of maximum bits for phyMode */
	eCSR_NUM_PHY_MODE = 18,
} eCsrPhyMode;

#ifdef WLAN_FEATURE_11BE
#define CSR_IS_DOT11_PHY_MODE_11BE(dot11mode) \
	((dot11mode) == eCSR_DOT11_MODE_11be)
#define CSR_IS_DOT11_PHY_MODE_11BE_ONLY(dot11mode) \
	((dot11mode) == eCSR_DOT11_MODE_11be_ONLY)
#else
#define CSR_IS_DOT11_PHY_MODE_11BE(dot11mode) 0
#define CSR_IS_DOT11_PHY_MODE_11BE_ONLY(dot11mode) 0
#endif

/**
 * enum eCsrRoamBssType - BSS type in CSR operations
 * @eCSR_BSS_TYPE_INFRASTRUCTURE: Infrastructure station
 * @eCSR_BSS_TYPE_INFRA_AP: SoftAP
 * @eCSR_BSS_TYPE_NDI: NAN datapath interface
 * @eCSR_BSS_TYPE_ANY: any BSS type
 */
typedef enum {
	eCSR_BSS_TYPE_INFRASTRUCTURE,
	eCSR_BSS_TYPE_INFRA_AP,
	eCSR_BSS_TYPE_NDI,
	eCSR_BSS_TYPE_ANY,
} eCsrRoamBssType;

typedef enum {
	eCSR_SCAN_SUCCESS,
	eCSR_SCAN_FAILURE,
	eCSR_SCAN_ABORT,
	eCSR_SCAN_FOUND_PEER,
} eCsrScanStatus;

typedef enum {
	eCSR_BW_20MHz_VAL = 20,
	eCSR_BW_40MHz_VAL = 40,
	eCSR_BW_80MHz_VAL = 80,
	eCSR_BW_160MHz_VAL = 160
} eCSR_BW_Val;

typedef enum {
	eCSR_INI_SINGLE_CHANNEL_CENTERED = 0,
	eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY = 1,
	eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY = 3,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED = 4,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED = 5,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED = 6,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW = 7,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW = 8,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH = 9,
	eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH = 10,
	eCSR_INI_CHANNEL_BONDING_STATE_MAX = 11
} eIniChanBondState;

#define CSR_RSN_MAX_PMK_LEN         48

typedef struct tagCsrChannelInfo {
	uint8_t numOfChannels;
	uint32_t *freq_list;
} tCsrChannelInfo, *tpCsrChannelInfo;

typedef enum {
	eHIDDEN_SSID_NOT_IN_USE,
	eHIDDEN_SSID_ZERO_LEN,
	eHIDDEN_SSID_ZERO_CONTENTS
} tHiddenssId;

typedef struct tagCsrSSIDInfo {
	tSirMacSSid SSID;
	tHiddenssId ssidHidden;
} tCsrSSIDInfo;

typedef struct tagCsrSSIDs {
	uint32_t numOfSSIDs;
	tCsrSSIDInfo *SSIDList; /* To be allocated for array of SSIDs */
} tCsrSSIDs;

typedef struct tagCsrBSSIDs {
	uint32_t numOfBSSIDs;
	struct qdf_mac_addr *bssid;
} tCsrBSSIDs;

typedef struct tagCsrScanResultInfo {
	/*
	 * Carry the IEs for the current BSSDescription.
	 * A pointer to tDot11fBeaconIEs. Maybe NULL for start BSS.
	 */
	void *pvIes;
	tAniSSID ssId;
	unsigned long timer;           /* timer is variable for hidden SSID timer */
	/*
	 * This member must be the last in the structure because the
	 * end of struct bss_description is an
	 * array with nonknown size at this time */
	struct bss_description BssDescriptor;
} tCsrScanResultInfo;

typedef struct tagCsrAuthList {
	uint32_t numEntries;
	enum csr_akm_type authType[eCSR_NUM_OF_SUPPORT_AUTH_TYPE];
} tCsrAuthList, *tpCsrAuthList;

typedef struct sCsrChannel_ {
	uint8_t numChannels;
	uint32_t channel_freq_list[CFG_VALID_CHANNEL_LIST_LEN];
} sCsrChannel;

typedef struct sCsrChnPower_ {
	uint32_t first_chan_freq;
	uint8_t numChannels;
	uint8_t maxtxPower;
} sCsrChnPower;

typedef struct tagCsr11dinfo {
	sCsrChannel Channels;
	/* max power channel list */
	sCsrChnPower ChnPower[CFG_VALID_CHANNEL_LIST_LEN];
} tCsr11dinfo;

typedef enum {
	/* CSR is done lostlink roaming and still cannot reconnect */
	eCSR_ROAM_LOSTLINK = 12,
	/*
	 * TKIP MIC error detected, callback gets a pointer
	 * to struct mic_failure_ind
	 */
	eCSR_ROAM_MIC_ERROR_IND = 14,
	eCSR_ROAM_SET_KEY_COMPLETE = 17,
	/* BSS in SoftAP mode status indication */
	eCSR_ROAM_INFRA_IND = 18,
	eCSR_ROAM_WPS_PBC_PROBE_REQ_IND = 19,
	/* Disaconnect all the clients */
	eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS = 31,
	/* Stopbss triggered from SME due to different */
	eCSR_ROAM_SEND_P2P_STOP_BSS = 32,
	/* beacon interval */
	eCSR_ROAM_UNPROT_MGMT_FRAME_IND = 33,

#ifdef FEATURE_WLAN_ESE
	eCSR_ROAM_TSM_IE_IND = 34,
	eCSR_ROAM_ESE_ADJ_AP_REPORT_IND = 36,
	eCSR_ROAM_ESE_BCN_REPORT_IND = 37,
#endif /* FEATURE_WLAN_ESE */

	/* Radar indication from lower layers */
	eCSR_ROAM_DFS_RADAR_IND = 38,
	eCSR_ROAM_SET_CHANNEL_RSP = 39,

	/* Channel sw update notification */
	eCSR_ROAM_DFS_CHAN_SW_NOTIFY = 40,
	eCSR_ROAM_EXT_CHG_CHNL_IND = 41,
	eCSR_ROAM_STA_CHANNEL_SWITCH = 42,
	eCSR_ROAM_NDP_STATUS_UPDATE = 43,
	eCSR_ROAM_CHANNEL_COMPLETE_IND = 47,
	eCSR_ROAM_CAC_COMPLETE_IND = 48,
	eCSR_ROAM_SAE_COMPUTE = 49,
} eRoamCmdStatus;

/* comment inside indicates what roaming callback gets */
typedef enum {
	eCSR_ROAM_RESULT_NONE,
	eCSR_ROAM_RESULT_SUCCESS = eCSR_ROAM_RESULT_NONE,
	/*
	 * If roamStatus is eCSR_ROAM_ASSOCIATION_COMPLETION,
	 * struct csr_roam_info's bss_desc may pass back
	 */
	eCSR_ROAM_RESULT_FAILURE,
	/* Pass back pointer to struct csr_roam_info */
	eCSR_ROAM_RESULT_ASSOCIATED,
	eCSR_ROAM_RESULT_NOT_ASSOCIATED,
	eCSR_ROAM_RESULT_MIC_FAILURE,
	eCSR_ROAM_RESULT_FORCED,
	eCSR_ROAM_RESULT_DISASSOC_IND,
	eCSR_ROAM_RESULT_DEAUTH_IND,
	eCSR_ROAM_RESULT_CAP_CHANGED,
	eCSR_ROAM_RESULT_LOSTLINK,
	eCSR_ROAM_RESULT_MIC_ERROR_UNICAST,
	eCSR_ROAM_RESULT_MIC_ERROR_GROUP,
	eCSR_ROAM_RESULT_AUTHENTICATED,
	eCSR_ROAM_RESULT_NEW_RSN_BSS,
#ifdef FEATURE_WLAN_WAPI
	eCSR_ROAM_RESULT_NEW_WAPI_BSS,
#endif /* FEATURE_WLAN_WAPI */
	/* INFRA started successfully */
	eCSR_ROAM_RESULT_INFRA_STARTED,
	/* INFRA start failed */
	eCSR_ROAM_RESULT_INFRA_START_FAILED,
	/* INFRA stopped */
	eCSR_ROAM_RESULT_INFRA_STOPPED,
	/* A station joining INFRA AP */
	eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND,
	/* A station joined INFRA AP */
	eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF,
	/* INFRA disassociated */
	eCSR_ROAM_RESULT_INFRA_DISASSOCIATED,
	eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND,
	eCSR_ROAM_RESULT_SEND_ACTION_FAIL,
	/* peer rejected assoc because max assoc limit reached */
	eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED,
	/* Assoc rejected due to concurrent session running on a diff channel */
	eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL,
	/* TDLS events */
	eCSR_ROAM_RESULT_ADD_TDLS_PEER,
	eCSR_ROAM_RESULT_UPDATE_TDLS_PEER,
	eCSR_ROAM_RESULT_DELETE_TDLS_PEER,
	eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND,
	eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND,
	eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP,
	eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER,
	eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN,
	eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED,
	eCSR_ROAM_RESULT_TDLS_CONNECTION_TRACKER_NOTIFICATION,
	eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND,
	eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS,
	eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE,
	eCSR_ROAM_RESULT_CSA_RESTART_RSP,
	eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS,
	eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND,

	eCSR_ROAM_RESULT_NDI_CREATE_RSP,
	eCSR_ROAM_RESULT_NDI_DELETE_RSP,
	eCSR_ROAM_RESULT_NDP_INITIATOR_RSP,
	eCSR_ROAM_RESULT_NDP_NEW_PEER_IND,
	eCSR_ROAM_RESULT_NDP_CONFIRM_IND,
	eCSR_ROAM_RESULT_NDP_INDICATION,
	eCSR_ROAM_RESULT_NDP_SCHED_UPDATE_RSP,
	eCSR_ROAM_RESULT_NDP_RESPONDER_RSP,
	eCSR_ROAM_RESULT_NDP_END_RSP,
	eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND,
	eCSR_ROAM_RESULT_NDP_END_IND,
	eCSR_ROAM_RESULT_CAC_END_IND,
	/* If Scan for SSID failed to found proper BSS */
	eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE,
	eCSR_ROAM_RESULT_INVOKE_FAILED,
} eCsrRoamResult;

typedef enum {
	/* Not associated in Infra or participating in an Ad-hoc */
	eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED,
	/* Participating in WDS network in AP/STA mode but not connected yet */
	eCSR_ASSOC_STATE_TYPE_WDS_DISCONNECTED,
	/* Participating in a WDS network and connected peer to peer */
	eCSR_ASSOC_STATE_TYPE_WDS_CONNECTED,
	/* Participating in a Infra network in AP not yet in connected state */
	eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED,
	/* Participating in a Infra network and connected to a peer */
	eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED,
	/* NAN Data interface not started */
	eCSR_CONNECT_STATE_TYPE_NDI_NOT_STARTED,
	/* NAN Data interface started */
	eCSR_CONNECT_STATE_TYPE_NDI_STARTED,
} eCsrConnectState;

typedef enum {
	eCSR_OPERATING_CHANNEL_ALL = 0,
	eCSR_OPERATING_CHANNEL_AUTO = eCSR_OPERATING_CHANNEL_ALL,
	eCSR_OPERATING_CHANNEL_ANY = eCSR_OPERATING_CHANNEL_ALL,
} eOperationChannel;

typedef enum {
	/*
	 * Roaming because HDD requested for reassoc by changing one of the
	 * fields in tCsrRoamModifyProfileFields. OR Roaming because SME
	 * requested for reassoc by changing one of the fields in
	 * tCsrRoamModifyProfileFields.
	 */
	eCsrRoamReasonStaCapabilityChanged,
	/*
	 * Roaming because SME requested for reassoc to a different AP,
	 * as part of inter AP handoff.
	 */
	eCsrRoamReasonBetterAP,
	/*
	 * Roaming because SME requested it as the link is lost - placeholder,
	 * will clean it up once handoff code gets in
	 */
	eCsrRoamReasonSmeIssuedForLostLink,

} eCsrRoamReasonCodes;

/*
 * Following fields might need modification dynamically once STA is up
 * & running & this'd trigger reassoc.
 */
typedef struct tagCsrRoamModifyProfileFields {
	/*
	 * during connect this specifies ACs U-APSD is to be setup
	 * for (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored).
	 * During assoc response this COULD carry confirmation of what
	 * ACs U-APSD got setup for. Later if an APP looking for APSD,
	 * SME-QoS might need to modify this field
	 */
	uint8_t uapsd_mask;
} tCsrRoamModifyProfileFields;

struct csr_roam_profile {
	tCsrSSIDs SSIDs;
	tCsrBSSIDs BSSIDs;
	/* this is bit mask of all the needed phy mode defined in eCsrPhyMode */
	uint32_t phyMode;
	eCsrRoamBssType BSSType;
	tCsrChannelInfo ChannelInfo;
	uint32_t op_freq;
	struct ch_params ch_params;
	/* If this is 0, SME will fill in for caller. */
	uint16_t beaconInterval;
	/*
	 * during connect this specifies ACs U-APSD is to be setup
	 * for (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored).
	 * During assoc resp this'd carry cnf of what ACs U-APSD got setup for
	 */
	uint8_t uapsd_mask;
	uint32_t nRSNReqIELength; /* The byte count in the pRSNReqIE */
	uint8_t *pRSNReqIE;       /* If not null,it's IE byte stream for RSN */
	uint8_t privacy;
	bool fwdWPSPBCProbeReq;
	tAniAuthType csr80211AuthType;
	uint32_t dtimPeriod;
	bool ApUapsdEnable;
	bool protEnabled;
	bool obssProtEnabled;
	bool chan_switch_hostapd_rate_enabled;
	uint16_t cfg_protection;
	uint8_t wps_state;
	enum QDF_OPMODE csrPersona;
	/* addIe params */
	struct add_ie_params add_ie_params;
	uint16_t beacon_tx_rate;
	tSirMacRateSet  supported_rates;
	tSirMacRateSet  extended_rates;
	bool require_h2e;
	uint32_t cac_duration_ms;
	uint32_t dfs_regdomain;
};

struct csr_config_params {
	/* keep this uint32_t. This gets converted to ePhyChannelBondState */
	uint32_t channelBondingMode24GHz;
	uint32_t channelBondingMode5GHz;
	eCsrPhyMode phyMode;
	uint32_t HeartbeatThresh50;
	enum wmm_user_mode WMMSupportMode;
	bool Is11eSupportEnabled;
	bool ProprietaryRatesEnabled;
	/* to set MCC Enable/Disable mode */
	uint8_t fEnableMCCMode;
	bool mcc_rts_cts_prot_enable;
	bool mcc_bcast_prob_resp_enable;
	/*
	 * To allow MCC GO different B.I than STA's.
	 * NOTE: make sure if RIVA firmware can handle this combination before
	 * enabling this at the moment, this flag is provided only to pass
	 * Wi-Fi Cert. 5.1.12
	 */
	uint8_t fAllowMCCGODiffBI;
	tCsr11dinfo Csr11dinfo;
	/*
	 * Customer wants to optimize the scan time. Avoiding scans(passive)
	 * on DFS channels while swipping through both bands can save some time
	 * (apprx 1.3 sec)
	 */
	uint8_t fEnableDFSChnlScan;
	bool send_smps_action;

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif
	bool obssEnabled;
	uint8_t conc_custom_rule1;
	uint8_t conc_custom_rule2;
	uint8_t is_sta_connection_in_5gz_enabled;

	uint8_t max_intf_count;
	uint32_t f_sta_miracast_mcc_rest_time_val;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	bool sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	enum force_1x1_type is_force_1x1;
	bool wep_tkip_in_he;
};

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define DEFAULT_REASSOC_FAILURE_TIMEOUT 1000
#endif

struct csr_roam_info {
	uint32_t nFrameLength;
	uint8_t frameType;
	/*
	 * Point to a buffer contain the beacon, assoc req, assoc rsp frame,
	 * in that order user needs to use nBeaconLength, nAssocReqLength,
	 * nAssocRspLength to desice where each frame starts and ends.
	 */
	uint8_t *pbFrames;
	bool fReassocReq;       /* set to true if for re-association */
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr peerMac;
	tSirResultCodes status_code;
	/* this'd be our own defined or sent from otherBSS(per 802.11spec) */
	uint32_t reasonCode;

	uint8_t disassoc_reason;

	uint8_t staId;         /* Peer stationId when connected */
	/* false means auth needed from supplicant. true means authenticated */
	bool fAuthRequired;
	uint8_t rsnIELen;
	uint8_t *prsnIE;
	uint8_t wapiIELen;
	uint8_t *pwapiIE;
	uint8_t addIELen;
	uint8_t *paddIE;
	union {
		tSirMicFailureInfo *pMICFailureInfo;
		tSirWPSPBCProbeReq *pWPSPBCProbeReq;
	} u;
	bool wmmEnabledSta;  /* set to true if WMM enabled STA */
#ifdef FEATURE_WLAN_ESE
	struct tsm_ie tsm_ie;
	uint16_t tsmRoamDelay;
	struct ese_bcn_report_rsp *pEseBcnReportRsp;
#endif
	/* Required for indicating the frames to upper layer */
	uint32_t assocReqLength;
	uint8_t *assocReqPtr;
	tSirChanChangeResponse *channelChangeRespEvent;
	/* Timing and fine Timing measurement capability clubbed together */
	uint8_t timingMeasCap;
	struct oem_channel_info chan_info;
	uint32_t target_chan_freq;

#ifdef WLAN_FEATURE_NAN
	union {
		struct ndi_create_rsp ndi_create_params;
		struct ndi_delete_rsp ndi_delete_params;
	} ndp;
#endif
	tDot11fIEHTCaps ht_caps;
	tDot11fIEVHTCaps vht_caps;
	bool he_caps_present;
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
	/* Extended capabilities of STA */
	uint8_t ecsa_capable;
	int rssi;
	int tx_rate;
	int rx_rate;
	tSirMacCapabilityInfo capability_info;
	uint32_t rx_mc_bc_cnt;
	uint32_t rx_retry_cnt;
#ifdef WLAN_FEATURE_SAE
	struct sir_sae_info *sae_info;
#endif
	struct assoc_ind *owe_pending_assoc_ind;
	struct qdf_mac_addr peer_mld;
};

typedef struct sSirSmeAssocIndToUpperLayerCnf {
	uint16_t messageType;   /* eWNI_SME_ASSOC_CNF */
	uint16_t length;
	uint8_t sessionId;
	tSirResultCodes status_code;
	tSirMacAddr bssId;      /* Self BSSID */
	tSirMacAddr peerMacAddr;
	uint16_t aid;
	uint8_t wmmEnabledSta;  /* set to true if WMM enabled STA */
	tSirRSNie rsnIE;        /* RSN IE received from peer */
	tSirWAPIie wapiIE;      /* WAPI IE received from peer */
	tSirAddie addIE;        /* this can be WSC and/or P2P IE */
	uint8_t reassocReq;     /* set to true if reassoc */
	/* Timing and fine Timing measurement capability clubbed together */
	uint8_t timingMeasCap;
	struct oem_channel_info chan_info;
	uint8_t target_channel;
	bool ampdu;
	bool sgi_enable;
	bool tx_stbc;
	tSirMacHTChannelWidth ch_width;
	enum sir_sme_phy_mode mode;
	bool rx_stbc;
	uint8_t max_supp_idx;
	uint8_t max_ext_idx;
	uint8_t max_mcs_idx;
	uint8_t rx_mcs_map;
	uint8_t tx_mcs_map;
	/* Extended capabilities of STA */
	uint8_t              ecsa_capable;

	uint32_t ies_len;
	uint8_t *ies;
	tDot11fIEHTCaps ht_caps;
	tDot11fIEVHTCaps vht_caps;
	tSirMacCapabilityInfo capability_info;
	bool he_caps_present;
#ifdef WLAN_FEATURE_11BE_MLO
	tSirMacAddr peer_mld_addr;
#endif
} tSirSmeAssocIndToUpperLayerCnf, *tpSirSmeAssocIndToUpperLayerCnf;

typedef struct tagCsrSummaryStatsInfo {
	uint32_t snr;
	int8_t rssi;
	uint32_t retry_cnt[4];
	uint32_t multiple_retry_cnt[4];
	uint32_t tx_frm_cnt[4];
	/* uint32_t num_rx_frm_crc_err; same as rx_error_cnt */
	/* uint32_t num_rx_frm_crc_ok; same as rx_frm_cnt */
	uint32_t rx_frm_cnt;
	uint32_t frm_dup_cnt;
	uint32_t fail_cnt[4];
	uint32_t rts_fail_cnt;
	uint32_t ack_fail_cnt;
	uint32_t rts_succ_cnt;
	uint32_t rx_discard_cnt;
	uint32_t rx_error_cnt;
	uint32_t tx_byte_cnt;

} tCsrSummaryStatsInfo;

typedef struct tagCsrGlobalClassAStatsInfo {
	uint8_t tx_nss;
	uint8_t rx_nss;
	uint32_t max_pwr;
	uint32_t tx_rate;
	uint32_t rx_rate;
	/* mcs index for HT20 and HT40 rates */
	uint32_t tx_mcs_index;
	uint32_t rx_mcs_index;
	enum tx_rate_info tx_mcs_rate_flags;
	enum tx_rate_info rx_mcs_rate_flags;
	uint8_t  tx_dcm;
	uint8_t  rx_dcm;
	enum txrate_gi  tx_gi;
	enum txrate_gi  rx_gi;
	/* to diff between HT20 & HT40 rates;short & long guard interval */
	enum tx_rate_info tx_rx_rate_flags;

} tCsrGlobalClassAStatsInfo;

typedef struct tagCsrGlobalClassDStatsInfo {
	uint32_t tx_uc_frm_cnt;
	uint32_t tx_mc_frm_cnt;
	uint32_t tx_bc_frm_cnt;
	uint32_t rx_uc_frm_cnt;
	uint32_t rx_mc_frm_cnt;
	uint32_t rx_bc_frm_cnt;
	uint32_t tx_uc_byte_cnt[4];
	uint32_t tx_mc_byte_cnt;
	uint32_t tx_bc_byte_cnt;
	uint32_t rx_uc_byte_cnt[4];
	uint32_t rx_mc_byte_cnt;
	uint32_t rx_bc_byte_cnt;
	uint32_t rx_byte_cnt;
	uint32_t num_rx_bytes_crc_ok;
	uint32_t rx_rate;

} tCsrGlobalClassDStatsInfo;

/**
 * struct csr_per_chain_rssi_stats_info - stores chain rssi
 * @rssi: array containing rssi for all chains
 * @peer_mac_addr: peer mac address
 */
struct csr_per_chain_rssi_stats_info {
	int8_t rssi[NUM_CHAINS_MAX];
	tSirMacAddr peer_mac_addr;
};

typedef void *tScanResultHandle;

typedef enum {
	REASSOC = 0,
	FASTREASSOC = 1,
	CONNECT_CMD_USERSPACE = 2,
} handoff_src;

typedef struct tagCsrHandoffRequest {
	struct qdf_mac_addr bssid;
	uint32_t ch_freq;
	uint8_t src;   /* To check if its a REASSOC or a FASTREASSOC IOCTL */
} tCsrHandoffRequest;

#ifdef FEATURE_WLAN_ESE
typedef struct tagCsrEseBeaconReqParams {
	uint16_t measurementToken;
	uint32_t ch_freq;
	uint8_t scanMode;
	uint16_t measurementDuration;
} tCsrEseBeaconReqParams, *tpCsrEseBeaconReqParams;

typedef struct tagCsrEseBeaconReq {
	uint8_t numBcnReqIe;
	tCsrEseBeaconReqParams bcnReq[SIR_ESE_MAX_MEAS_IE_REQS];
} tCsrEseBeaconReq, *tpCsrEseBeaconReq;
#endif /* FEATURE_WLAN_ESE */

struct csr_del_sta_params {
	struct qdf_mac_addr peerMacAddr;
	uint16_t reason_code;
	uint8_t subtype;
};

typedef QDF_STATUS (*csr_roam_complete_cb)(struct wlan_objmgr_psoc *psoc,
					   uint8_t session_id,
					   struct csr_roam_info *param,
					   uint32_t roam_id,
					   eRoamCmdStatus roam_status,
					   eCsrRoamResult roam_result);
typedef QDF_STATUS (*csr_session_open_cb)(uint8_t session_id,
					  QDF_STATUS qdf_status);
typedef QDF_STATUS (*csr_session_close_cb)(uint8_t session_id);

#define CSR_IS_ANY_BSS_TYPE(pProfile) (eCSR_BSS_TYPE_ANY == \
				       (pProfile)->BSSType)
#define CSR_IS_INFRA_AP(pProfile) (eCSR_BSS_TYPE_INFRA_AP ==  \
				   (pProfile)->BSSType)
#ifdef WLAN_FEATURE_NAN
#define CSR_IS_NDI(profile)  (eCSR_BSS_TYPE_NDI == (profile)->BSSType)
#else
#define CSR_IS_NDI(profile)  (false)
#endif

#ifdef WLAN_FEATURE_NAN
#define CSR_IS_CONN_NDI(profile)  (eCSR_BSS_TYPE_NDI == (profile)->BSSType)
#else
#define CSR_IS_CONN_NDI(profile)  (false)
#endif

QDF_STATUS csr_set_channels(struct mac_context *mac,
			    struct csr_config_params *pParam);

/* enum to string conversion for debug output */
const char *get_e_roam_cmd_status_str(eRoamCmdStatus val);
const char *get_e_csr_roam_result_str(eCsrRoamResult val);
const char *csr_phy_mode_str(eCsrPhyMode phy_mode);

#ifdef FEATURE_WLAN_ESE
typedef void (*tCsrTsmStatsCallback)(tAniTrafStrmMetrics tsmMetrics,
				     void *pContext);
#endif /* FEATURE_WLAN_ESE */
typedef void (*tCsrSnrCallback)(int8_t snr, void *pContext);

typedef void (*csr_readyToSuspendCallback)(void *pContext, bool suspended);
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
typedef void (*csr_readyToExtWoWCallback)(void *pContext, bool status);
#endif
typedef void (*csr_link_status_callback)(uint8_t status, void *context);

typedef void (*sme_get_raom_scan_ch_callback)(
				hdd_handle_t hdd_handle,
				struct roam_scan_ch_resp *roam_ch,
				void *context);

#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE) && \
	defined(CONNECTIVITY_PKTLOG)
/**
 * csr_packetdump_timer_stop() - stops packet dump timer
 *
 * This function is used to stop packet dump timer
 *
 * Return: None
 *
 */
void csr_packetdump_timer_stop(void);

/**
 * csr_packetdump_timer_start() - start packet dump timer
 *
 * This function is used to start packet dump timer
 *
 * Return: None
 *
 */
void csr_packetdump_timer_start(void);
#else
static inline void csr_packetdump_timer_stop(void) {}
static inline void csr_packetdump_timer_start(void) {}
#endif

/**
 * csr_get_channel_status() - get chan info via channel number
 * @mac: Pointer to Global MAC structure
 * @chan_freq: channel frequency
 *
 * Return: chan status info
 */
struct lim_channel_status *
csr_get_channel_status(struct mac_context *mac, uint32_t chan_freq);

/**
 * csr_clear_channel_status() - clear chan info
 * @mac: Pointer to Global MAC structure
 *
 * Return: none
 */
void csr_clear_channel_status(struct mac_context *mac);

/**
 * csr_update_owe_info() - Update OWE info
 * @mac: mac context
 * @assoc_ind: assoc ind
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_update_owe_info(struct mac_context *mac,
			       struct assoc_ind *assoc_ind);

typedef void (*csr_ani_callback)(int8_t *ani, void *context);

/*
 * csr_convert_to_reg_phy_mode() - CSR API to convert CSR phymode into
 * regulatory phymode
 * @csr_phy_mode: csr phymode with type eCsrPhyMode
 * @freq: current operating frequency
 *
 * This API is used to convert a phymode from CSR to a phymode from regulatory
 *
 * Return: regulatory phymode that is comparable to input
 */
enum reg_phymode csr_convert_to_reg_phy_mode(eCsrPhyMode csr_phy_mode,
				       qdf_freq_t freq);

/*
 * csr_convert_from_reg_phy_mode() - CSR API to convert regulatory phymode into
 * CSR phymode
 * @reg_phymode: regulatory phymode
 *
 * This API is used to convert a regulatory phymode to a CSR phymode
 *
 * Return: eCSR phymode that is comparable to input
 */
eCsrPhyMode csr_convert_from_reg_phy_mode(enum reg_phymode phymode);

/*
 * csr_update_beacon() - CSR API to update beacon template
 * @mac: mac context
 *
 * This API is used to update beacon template to FW
 *
 * Return: None
 */
void csr_update_beacon(struct mac_context *mac);

/**
 * csr_fill_enc_type() - converts crypto cipher set to csr specific cipher type
 * @cipher_type: output csr cipher type
 * @ cipherset:input cipher set
 *
 * Return: None
 */
void csr_fill_enc_type(eCsrEncryptionType *cipher_type, uint32_t cipherset);

/**
 * csr_fill_auth_type() - auth mode set to csr specific auth type
 * @auth_type: output csr auth type
 * @ authmodeset: authmode set
 * @akm: akm
 * @ucastcipherset: ucastcipherset
 *
 * Return: None
 */
void csr_fill_auth_type(enum csr_akm_type *auth_type,
			uint32_t authmodeset, uint32_t akm,
			uint32_t ucastcipherset);

/**
 * csr_phy_mode_to_dot11mode() - converts phy mode to dot11 mode
 * @phy_mode: wlan phy mode
 *
 * Return: csr_cfgdot11mode
 */
enum csr_cfgdot11mode csr_phy_mode_to_dot11mode(enum wlan_phymode phy_mode);

/*
 * csr_mlme_vdev_disconnect_all_p2p_client_event() - Callback for MLME module
 *	to send a disconnect all P2P event to the SAP event handler
 * @vdev_id: vdev id of SAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_mlme_vdev_disconnect_all_p2p_client_event(uint8_t vdev_id);

/*
 * csr_mlme_vdev_stop_bss() - Callback for MLME module to send a stop BSS event
 *	to the SAP event handler
 * @vdev_id: vdev id of SAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_mlme_vdev_stop_bss(uint8_t vdev_id);

/*
 * csr_mlme_get_concurrent_operation_freq() - Callback for MLME module to
 *	get the concurrent operation frequency
 *
 * Return: concurrent frequency
 */
qdf_freq_t csr_mlme_get_concurrent_operation_freq(void);
#endif
