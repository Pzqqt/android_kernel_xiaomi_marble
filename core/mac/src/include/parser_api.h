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

/*
 * This file parser_api.h contains the definitions used
 * for parsing received 802.11 frames
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdarg.h>
#include "sir_mac_prop_exts.h"
#include "dot11f.h"
#include "lim_ft_defs.h"
#include "lim_session.h"

#define COUNTRY_STRING_LENGTH    (3)
#define COUNTRY_INFO_MAX_CHANNEL (84)
#define MAX_SIZE_OF_TRIPLETS_IN_COUNTRY_IE (COUNTRY_STRING_LENGTH * \
						COUNTRY_INFO_MAX_CHANNEL)
#define HIGHEST_24GHZ_CHANNEL_NUM  (14)

#define IS_24G_CH(__chNum) ((__chNum > 0) && (__chNum < 15))
#define IS_5G_CH(__chNum) ((__chNum >= 36) && (__chNum <= 165))
#define IS_2X2_CHAIN(__chain) ((__chain & 0x3) == 0x3)
#define DISABLE_NSS2_MCS 0xC

#define NSS_1x1_MODE 1
#define NSS_2x2_MODE 2
#define MBO_IE_ASSOC_DISALLOWED_SUBATTR_ID 0x04

/* QCN IE definitions */
#define QCN_IE_HDR_LEN     6

#define QCN_IE_VERSION_SUBATTR_ID        1
#define QCN_IE_VERSION_SUBATTR_DATA_LEN  2
#define QCN_IE_VERSION_SUBATTR_LEN       4
#define QCN_IE_VERSION_SUPPORTED    1
#define QCN_IE_SUBVERSION_SUPPORTED 0

#define SIZE_OF_FIXED_PARAM 12
#define SIZE_OF_TAG_PARAM_NUM 1
#define SIZE_OF_TAG_PARAM_LEN 1
#define RSNIEID 0x30
#define RSNIE_CAPABILITY_LEN 2
#define DEFAULT_RSNIE_CAP_VAL 0x00

#define SIZE_MASK 0x7FFF
#define FIXED_MASK 0x8000

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
#define QCOM_VENDOR_IE_MCC_AVOID_CH 0x01

struct sAvoidChannelIE {
	/* following must be 0xDD (221) */
	uint8_t tag_number;
	uint8_t length;
	/* following must be 00-A0-C6 */
	uint8_t oui[3];
	/* following must be 0x01 */
	uint8_t type;
	uint8_t channel;
};
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

typedef struct sSirCountryInformation {
	uint8_t countryString[COUNTRY_STRING_LENGTH];
	uint8_t numIntervals;   /* number of channel intervals */
	struct channelPowerLim {
		uint8_t channelNumber;
		uint8_t numChannel;
		uint8_t maxTransmitPower;
	} channelTransmitPower[COUNTRY_INFO_MAX_CHANNEL];
} tSirCountryInformation, *tpSirCountryInformation;

typedef struct sSirQCNIE {
	bool    is_present;
	uint8_t version;
	uint8_t sub_version;
} tSirQCNIE, *tpSirQCNIE;

#ifdef WLAN_FEATURE_FILS_SK
#define SIR_MAX_IDENTIFIER_CNT 7
#define SIR_CACHE_IDENTIFIER_LEN 2
#define SIR_HESSID_LEN 6
#define SIR_MAX_KEY_CNT 7
#define SIR_MAX_KEY_LEN 48
#define SIR_FILS_IND_ELEM_OFFSET 2
/*
 * struct public_key_identifier: structure for public key identifier
 * present in fils indication element
 * @is_present: if Key info is present
 * @key_cnt:  number of keys present
 * @key_type: type of key used
 * @length: length of key
 * @key: key data
 */
struct public_key_identifier {
	bool is_present;
	uint8_t key_cnt;
	uint8_t key_type;
	uint8_t length;
	uint8_t key[SIR_MAX_KEY_CNT][SIR_MAX_KEY_LEN];
};

/*
 * struct fils_cache_identifier: structure for fils cache identifier
 * present in fils indication element
 * @is_present: if cache identifier is present
 * @identifier: cache identifier
 */
struct fils_cache_identifier {
	bool is_present;
	uint8_t identifier[SIR_CACHE_IDENTIFIER_LEN];
};

/*
 * struct fils_hessid: structure for fils hessid
 * present in fils indication element
 * @is_present: if hessid info is present
 * @hessid: hessid data
 */
struct fils_hessid {
	bool is_present;
	uint8_t hessid[SIR_HESSID_LEN];
};

/*
 * struct fils_realm_identifier: structure for fils_realm_identifier
 * present in fils indication element
 * @is_present: if realm info is present
 * @realm_cnt: realm count
 * @realm: realm data
 */
struct fils_realm_identifier {
	bool is_present;
	uint8_t realm_cnt;
	uint8_t realm[SIR_MAX_REALM_COUNT][SIR_REALM_LEN];
};

/*
 * struct sir_fils_indication: structure for fils indication element
 * @is_present: if indication element is present
 * @is_ip_config_supported: if IP config is supported
 * @is_fils_sk_auth_supported: if fils sk suppprted
 * @is_fils_sk_auth_pfs_supported: if fils sk with pfs supported
 * @is_pk_auth_supported: if fils public key supported
 * @cache_identifier: fils cache idenfier info
 * @hessid: fils hessid info
 * @realm_identifier: fils realm info
 * @key_identifier: fils key identifier info
 */
struct sir_fils_indication {
	bool is_present;
	uint8_t is_ip_config_supported;
	uint8_t is_fils_sk_auth_supported;
	uint8_t is_fils_sk_auth_pfs_supported;
	uint8_t is_pk_auth_supported;
	struct fils_cache_identifier cache_identifier;
	struct fils_hessid hessid;
	struct fils_realm_identifier realm_identifier;
	struct public_key_identifier key_identifier;
};
#endif

/* Structure common to Beacons & Probe Responses */
typedef struct sSirProbeRespBeacon {
	tSirMacTimeStamp timeStamp;
	uint16_t beaconInterval;
	tSirMacCapabilityInfo capabilityInfo;

	tSirMacSSid ssId;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;
	tSirMacChanNum channelNumber;
	tSirMacCfParamSet cfParamSet;
	tSirMacTim tim;
	tSirMacEdcaParamSetIE edcaParams;
	tSirMacQosCapabilityIE qosCapability;

	tSirCountryInformation countryInfoParam;
	tSirMacWpaInfo wpa;
	tSirMacRsnInfo rsn;

	tSirMacErpInfo erpIEInfo;

	tSirPropIEStruct propIEinfo;
	tDot11fIEPowerConstraints localPowerConstraint;
	tDot11fIETPCReport tpcReport;
	tDot11fIEChanSwitchAnn channelSwitchIE;
	tDot11fIEsec_chan_offset_ele sec_chan_offset;
	tDot11fIEext_chan_switch_ann ext_chan_switch;
	tDot11fIESuppOperatingClasses supp_operating_classes;
	tSirMacAddr bssid;
	tDot11fIEQuiet quietIE;
	tDot11fIEHTCaps HTCaps;
	tDot11fIEHTInfo HTInfo;
	tDot11fIEP2PProbeRes P2PProbeRes;
	uint8_t mdie[SIR_MDIE_SIZE];
#ifdef FEATURE_WLAN_ESE
	tDot11fIEESETxmitPower eseTxPwr;
	tDot11fIEQBSSLoad QBSSLoad;
#endif
	uint8_t ssidPresent;
	uint8_t suppRatesPresent;
	uint8_t extendedRatesPresent;
	uint8_t supp_operating_class_present;
	uint8_t cfPresent;
	uint8_t dsParamsPresent;
	uint8_t timPresent;

	uint8_t edcaPresent;
	uint8_t qosCapabilityPresent;
	uint8_t wmeEdcaPresent;
	uint8_t wmeInfoPresent;
	uint8_t wsmCapablePresent;

	uint8_t countryInfoPresent;
	uint8_t wpaPresent;
	uint8_t rsnPresent;
	uint8_t erpPresent;
	uint8_t channelSwitchPresent;
	uint8_t sec_chan_offset_present;
	uint8_t ext_chan_switch_present;
	uint8_t quietIEPresent;
	uint8_t tpcReportPresent;
	uint8_t powerConstraintPresent;

	uint8_t mdiePresent;

	tDot11fIEVHTCaps VHTCaps;
	tDot11fIEVHTOperation VHTOperation;
	tDot11fIEVHTExtBssLoad VHTExtBssLoad;
	tDot11fIEExtCap ext_cap;
	tDot11fIEOperatingMode OperatingMode;
	uint8_t WiderBWChanSwitchAnnPresent;
	tDot11fIEWiderBWChanSwitchAnn WiderBWChanSwitchAnn;
	uint8_t Vendor1IEPresent;
	tDot11fIEvendor_vht_ie vendor_vht_ie;
	uint8_t Vendor3IEPresent;
	tDot11fIEhs20vendor_ie hs20vendor_ie;
	tDot11fIEIBSSParams IBSSParams;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tDot11fIEQComVendorIE   AvoidChannelIE;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
#ifdef FEATURE_WLAN_ESE
	uint8_t    is_ese_ver_ie_present;
#endif
	tDot11fIEOBSSScanParameters obss_scanparams;
	bool MBO_IE_present;
	uint8_t MBO_capability;
	bool assoc_disallowed;
	uint8_t assoc_disallowed_reason;
	tSirQCNIE QCN_IE;
	tDot11fIEhe_cap he_cap;
	tDot11fIEhe_op he_op;
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
	tDot11fIEbss_color_change vendor_he_bss_color_change;
#endif
#ifdef WLAN_FEATURE_FILS_SK
	struct sir_fils_indication fils_ind;
#endif
} tSirProbeRespBeacon, *tpSirProbeRespBeacon;

/* probe Request structure */
typedef struct sSirProbeReq {
	tSirMacSSid ssId;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;
	tDot11fIEWscProbeReq probeReqWscIeInfo;
	tDot11fIEHTCaps HTCaps;
	uint8_t ssidPresent;
	uint8_t suppRatesPresent;
	uint8_t extendedRatesPresent;
	uint8_t wscIePresent;
	uint8_t p2pIePresent;
	tDot11fIEVHTCaps VHTCaps;
	tDot11fIEhe_cap he_cap;
} tSirProbeReq, *tpSirProbeReq;

/* / Association Request structure (one day to be replaced by */
/* / tDot11fAssocRequest) */
typedef struct sSirAssocReq {

	tSirMacCapabilityInfo capabilityInfo;
	uint16_t listenInterval;
	tSirMacAddr currentApAddr;      /* only in reassoc frames */
	tSirMacSSid ssId;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;

	tSirAddtsReqInfo addtsReq;
	tSirMacQosCapabilityStaIE qosCapability;

	tSirMacWapiInfo wapi;
	tSirMacWpaInfo wpa;
	tSirMacRsnInfo rsn;
	tSirAddie addIE;

	tSirPropIEStruct propIEinfo;
	tSirMacPowerCapabilityIE powerCapability;
	tSirMacSupportedChannelIE supportedChannels;
	tDot11fIEHTCaps HTCaps;
	tDot11fIEWMMInfoStation WMMInfoStation;
	/* / This is set if the frame is a reassoc request: */
	uint8_t reassocRequest;
	uint8_t ssidPresent;
	uint8_t suppRatesPresent;
	uint8_t extendedRatesPresent;

	uint8_t wmeInfoPresent;
	uint8_t qosCapabilityPresent;
	uint8_t addtsPresent;
	uint8_t wsmCapablePresent;

	uint8_t wapiPresent;
	uint8_t wpaPresent;
	uint8_t rsnPresent;
	uint8_t addIEPresent;

	uint8_t powerCapabilityPresent;
	uint8_t supportedChannelsPresent;
	/* keeping copy of association request received, this is
	   required for indicating the frame to upper layers */
	uint32_t assocReqFrameLength;
	uint8_t *assocReqFrame;
	tDot11fIEVHTCaps VHTCaps;
	tDot11fIEOperatingMode operMode;
	tDot11fIEExtCap ExtCap;
	tDot11fIEvendor_vht_ie vendor_vht_ie;
	tDot11fIEhs20vendor_ie hs20vendor_ie;
	tDot11fIEhe_cap he_cap;
} tSirAssocReq, *tpSirAssocReq;

/* / Association Response structure (one day to be replaced by */
/* / tDot11fAssocRequest) */
typedef struct sSirAssocRsp {

	tSirMacCapabilityInfo capabilityInfo;
	uint16_t aid;
	uint16_t statusCode;
	tSirMacRateSet supportedRates;
	tSirMacRateSet extendedRates;
	tSirPropIEStruct propIEinfo;
	tSirMacEdcaParamSetIE edca;
	tSirAddtsRspInfo addtsRsp;
	tDot11fIEHTCaps HTCaps;
	tDot11fIEHTInfo HTInfo;
	tDot11fIEFTInfo FTInfo;
	uint8_t mdie[SIR_MDIE_SIZE];
	uint8_t num_RICData;
	tDot11fIERICDataDesc RICData[2];

#ifdef FEATURE_WLAN_ESE
	uint8_t num_tspecs;
	tDot11fIEWMMTSPEC TSPECInfo[SIR_ESE_MAX_TSPEC_IES];
	tSirMacESETSMIE tsmIE;
#endif

	uint8_t suppRatesPresent;
	uint8_t extendedRatesPresent;

	uint8_t edcaPresent;
	uint8_t wmeEdcaPresent;
	uint8_t addtsPresent;
	uint8_t wsmCapablePresent;
	uint8_t ftinfoPresent;
	uint8_t mdiePresent;
	uint8_t ricPresent;
#ifdef FEATURE_WLAN_ESE
	uint8_t tspecPresent;
	uint8_t tsmPresent;
#endif
	tDot11fIEVHTCaps VHTCaps;
	tDot11fIEVHTOperation VHTOperation;
	tDot11fIEExtCap ExtCap;
	tSirQosMapSet QosMapSet;
#ifdef WLAN_FEATURE_11W
	tDot11fIETimeoutInterval TimeoutInterval;
#endif
	tDot11fIEvendor_vht_ie vendor_vht_ie;
	tDot11fIEOBSSScanParameters obss_scanparams;
	tDot11fTLVrssi_assoc_rej rssi_assoc_rej;
	tSirQCNIE QCN_IE;
	tDot11fIEhe_cap he_cap;
	tDot11fIEhe_op he_op;
#ifdef WLAN_FEATURE_FILS_SK
	tDot11fIEfils_session fils_session;
	tDot11fIEfils_key_confirmation fils_key_auth;
	tDot11fIEfils_kde fils_kde;
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint16_t hlp_data_len;
	uint8_t hlp_data[FILS_MAX_HLP_DATA_LEN];
#endif
} tSirAssocRsp, *tpSirAssocRsp;

#ifdef FEATURE_WLAN_ESE
/* Structure to hold ESE Beacon report mandatory IEs */
typedef struct sSirEseBcnReportMandatoryIe {
	tSirMacSSid ssId;
	tSirMacRateSet supportedRates;
	tSirMacFHParamSet fhParamSet;
	tSirMacDsParamSetIE dsParamSet;
	tSirMacCfParamSet cfParamSet;
	tSirMacIBSSParams ibssParamSet;
	tSirMacTim tim;
	tSirMacRRMEnabledCap rmEnabledCapabilities;

	uint8_t ssidPresent;
	uint8_t suppRatesPresent;
	uint8_t fhParamPresent;
	uint8_t dsParamsPresent;
	uint8_t cfPresent;
	uint8_t ibssParamPresent;
	uint8_t timPresent;
	uint8_t rrmPresent;
} tSirEseBcnReportMandatoryIe, *tpSirEseBcnReportMandatoryIe;
#endif /* FEATURE_WLAN_ESE */

/**
 * struct s_ext_cap - holds bitfields of extended capability IE
 *
 * s_ext_cap holds bitfields of extended capability IE. In dot11f files
 * extended capability IE information is stored as an array of bytes.
 * This structure is used to encode/decode the byte array present in
 * dot11f IE structure.
 */

struct s_ext_cap {
	uint8_t bss_coexist_mgmt_support:1;
	uint8_t reserved1:1;
	uint8_t ext_chan_switch:1;
	uint8_t reserved2:1;
	uint8_t psmp_cap:1;
	uint8_t reserved3:1;
	uint8_t spsmp_cap:1;
	uint8_t event:1;
	uint8_t diagnostics:1;
	uint8_t multi_diagnostics:1;
	uint8_t loc_tracking:1;
	uint8_t fms:1;
	uint8_t proxy_arp_service:1;
	uint8_t co_loc_intf_reporting:1;
	uint8_t civic_loc:1;
	uint8_t geospatial_loc:1;
	uint8_t tfs:1;
	uint8_t wnm_sleep_mode:1;
	uint8_t tim_broadcast:1;
	uint8_t bss_transition:1;
	uint8_t qos_traffic_cap:1;
	uint8_t ac_sta_cnt:1;
	uint8_t multi_bssid:1;
	uint8_t timing_meas:1;
	uint8_t chan_usage:1;
	uint8_t ssid_list:1;
	uint8_t dms:1;
	uint8_t utctsf_offset:1;
	uint8_t tdls_peer_uapsd_buffer_sta:1;
	uint8_t tdls_peer_psm_supp:1;
	uint8_t tdls_channel_switching:1;
	uint8_t interworking_service:1;
	uint8_t qos_map:1;
	uint8_t ebr:1;
	uint8_t sspn_interface:1;
	uint8_t reserved4:1;
	uint8_t msg_cf_cap:1;
	uint8_t tdls_support:1;
	uint8_t tdls_prohibited:1;
	uint8_t tdls_chan_swit_prohibited:1;
	uint8_t reject_unadmitted_traffic:1;
	uint8_t service_interval_granularity:3;
	uint8_t identifier_loc:1;
	uint8_t uapsd_coexistence:1;
	uint8_t wnm_notification:1;
	uint8_t qa_bcapbility:1;
	uint8_t utf8_ssid:1;
	uint8_t qmf_activated:1;
	uint8_t qm_frecon_act:1;
	uint8_t robust_av_streaming:1;
	uint8_t advanced_gcr:1;
	uint8_t mesh_gcr:1;
	uint8_t scs:1;
	uint8_t q_load_report:1;
	uint8_t alternate_edca:1;
	uint8_t unprot_txo_pneg:1;
	uint8_t prot_txo_pneg:1;
	uint8_t reserved6:1;
	uint8_t prot_q_load_report:1;
	uint8_t tdls_wider_bw:1;
	uint8_t oper_mode_notification:1;
	uint8_t max_num_of_msdu_bit1:1;
	uint8_t max_num_of_msdu_bit2:1;
	uint8_t chan_sch_mgmt:1;
	uint8_t geo_db_inband_en_signal:1;
	uint8_t nw_chan_control:1;
	uint8_t white_space_map:1;
	uint8_t chan_avail_query:1;
	uint8_t fine_time_meas_responder:1;
	uint8_t fine_time_meas_initiator:1;
	uint8_t fils_capability:1;
};

uint8_t sirIsPropCapabilityEnabled(struct sAniSirGlobal *pMac, uint32_t bitnum);

#define CFG_GET_INT(nStatus, pMac, nItem, cfg)  do { \
		(nStatus) = wlan_cfg_get_int((pMac), (nItem), &(cfg)); \
		if (eSIR_SUCCESS != (nStatus)) { \
			pe_err("Failed to retrieve nItem from CFG status: %d", (nStatus)); \
			return nStatus; \
		} \
} while (0)

#define CFG_GET_INT_NO_STATUS(nStatus, pMac, nItem, cfg) do { \
		(nStatus) = wlan_cfg_get_int((pMac), (nItem), &(cfg)); \
		if (eSIR_SUCCESS != (nStatus)) { \
			pe_err("Failed to retrieve nItem from CFG status: %d", (nStatus)); \
			return; \
		} \
} while (0)

#define CFG_GET_STR(nStatus, pMac, nItem, cfg, nCfg, nMaxCfg) do { \
		(nCfg) = (nMaxCfg); \
		(nStatus) = wlan_cfg_get_str((pMac), (nItem), (cfg), &(nCfg)); \
		if (eSIR_SUCCESS != (nStatus)) { \
			pe_err("Failed to retrieve nItem from CFG status: %d", (nStatus)); \
			return nStatus; \
		} \
} while (0)

#define CFG_GET_STR_NO_STATUS(nStatus, pMac, nItem, cfg, nCfg, nMaxCfg) do { \
		(nCfg) = (nMaxCfg); \
		(nStatus) = wlan_cfg_get_str((pMac), (nItem), (cfg), &(nCfg)); \
		if (eSIR_SUCCESS != (nStatus)) { \
			pe_err("Failed to retrieve nItem from CFG status: %d", (nStatus)); \
			return; \
		} \
} while (0)

void swap_bit_field16(uint16_t in, uint16_t *out);

/* Currently implemented as "shims" between callers & the new framesc- */
/* generated code: */

tSirRetStatus
sir_convert_probe_req_frame2_struct(struct sAniSirGlobal *pMac,
				uint8_t *frame, uint32_t len,
				tpSirProbeReq probe);

tSirRetStatus
sir_convert_probe_frame2_struct(struct sAniSirGlobal *pMac, uint8_t *frame,
				uint32_t len, tpSirProbeRespBeacon probe);

tSirRetStatus
sir_convert_assoc_req_frame2_struct(struct sAniSirGlobal *pMac,
				uint8_t *frame, uint32_t len,
				tpSirAssocReq assoc);

tSirRetStatus
sir_convert_assoc_resp_frame2_struct(struct sAniSirGlobal *pMac,
				tpPESession session_entry,
				uint8_t *frame, uint32_t len,
				tpSirAssocRsp assoc);

tSirRetStatus
sir_convert_reassoc_req_frame2_struct(struct sAniSirGlobal *pMac,
				uint8_t *frame, uint32_t len,
				tpSirAssocReq assoc);

tSirRetStatus
sir_parse_beacon_ie(struct sAniSirGlobal *pMac,
		tpSirProbeRespBeacon pBeaconStruct,
		uint8_t *pPayload, uint32_t payloadLength);

tSirRetStatus
sir_convert_beacon_frame2_struct(struct sAniSirGlobal *pMac,
				uint8_t *pBeaconFrame,
				tpSirProbeRespBeacon pBeaconStruct);

tSirRetStatus
sir_convert_auth_frame2_struct(struct sAniSirGlobal *pMac,
			uint8_t *frame, uint32_t len,
			tpSirMacAuthFrameBody auth);

tSirRetStatus
sir_convert_addts_req2_struct(struct sAniSirGlobal *pMac,
			uint8_t *frame, uint32_t len,
			tSirAddtsReqInfo *addTs);

tSirRetStatus
sir_convert_addts_rsp2_struct(struct sAniSirGlobal *pMac,
			uint8_t *frame, uint32_t len,
			tSirAddtsRspInfo *addts);

tSirRetStatus
sir_convert_delts_req2_struct(struct sAniSirGlobal *pMac,
			uint8_t *frame, uint32_t len,
			tSirDeltsReqInfo *delTs);
tSirRetStatus
sir_convert_qos_map_configure_frame2_struct(tpAniSirGlobal pMac,
					uint8_t *pFrame, uint32_t nFrame,
					tSirQosMapSet *pQosMapSet);

#ifdef ANI_SUPPORT_11H
tSirRetStatus
sir_convert_tpc_req_frame2_struct(struct sAniSirGlobal *, uint8_t *,
				tpSirMacTpcReqActionFrame, uint32_t);

tSirRetStatus
sir_convert_meas_req_frame2_struct(struct sAniSirGlobal *, uint8_t *,
				tpSirMacMeasReqActionFrame, uint32_t);
#endif

/**
 * \brief Populated a tDot11fFfCapabilities
 *
 * \sa PopulatedDot11fCapabilities2
 *
 *
 * \param pMac Pointer to the global MAC data structure
 *
 * \param pDot11f Address of a tDot11fFfCapabilities to be filled in
 *
 *
 * \note If SIR_MAC_PROP_CAPABILITY_11EQOS is enabled, we'll clear the QOS
 * bit in pDot11f
 *
 *
 */

tSirRetStatus
populate_dot11f_capabilities(tpAniSirGlobal pMac,
			tDot11fFfCapabilities *pDot11f,
			tpPESession psessionEntry);

/**
 * \brief Populated a tDot11fFfCapabilities
 *
 * \sa PopulatedDot11fCapabilities2
 *
 *
 * \param pMac Pointer to the global MAC data structure
 *
 * \param pDot11f Address of a tDot11fFfCapabilities to be filled in
 *
 * \param pSta Pointer to a tDphHashNode representing a peer
 *
 *
 * \note If SIR_MAC_PROP_CAPABILITY_11EQOS is enabled on our peer, we'll
 * clear the QOS bit in pDot11f
 *
 *
 */

struct sDphHashNode;

tSirRetStatus
populate_dot11f_capabilities2(tpAniSirGlobal pMac,
			tDot11fFfCapabilities *pDot11f,
			struct sDphHashNode *pSta,
			tpPESession psessionEntry);

/* / Populate a tDot11fIEChanSwitchAnn */
void
populate_dot11f_chan_switch_ann(tpAniSirGlobal pMac,
				tDot11fIEChanSwitchAnn *pDot11f,
				tpPESession psessionEntry);

void
populate_dot_11_f_ext_chann_switch_ann(tpAniSirGlobal mac_ptr,
				tDot11fIEext_chan_switch_ann *dot_11_ptr,
				tpPESession session_entry);

/* / Populate a tDot11fIEChannelSwitchWrapper */
void
populate_dot11f_chan_switch_wrapper(tpAniSirGlobal pMac,
				tDot11fIEChannelSwitchWrapper *pDot11f,
				tpPESession psessionEntry);

/* / Populate a tDot11fIECountry */
tSirRetStatus
populate_dot11f_country(tpAniSirGlobal pMac,
			tDot11fIECountry *pDot11f, tpPESession psessionEntry);

/* Populated a populate_dot11f_ds_params */
tSirRetStatus
populate_dot11f_ds_params(tpAniSirGlobal pMac,
			tDot11fIEDSParams *pDot11f, uint8_t channel);

/* / Populated a tDot11fIEEDCAParamSet */
void
populate_dot11f_edca_param_set(tpAniSirGlobal pMac,
			tDot11fIEEDCAParamSet *pDot11f,
			tpPESession psessionEntry);

tSirRetStatus
populate_dot11f_erp_info(tpAniSirGlobal pMac,
			tDot11fIEERPInfo *pDot11f, tpPESession psessionEntry);

tSirRetStatus
populate_dot11f_ext_supp_rates(tpAniSirGlobal pMac,
			uint8_t nChannelNum, tDot11fIEExtSuppRates *pDot11f,
			tpPESession psessionEntry);

tSirRetStatus
populate_dot11f_beacon_report(tpAniSirGlobal pMac,
			tDot11fIEMeasurementReport *pDot11f,
			tSirMacBeaconReport *pBeaconReport);

/**
 * \brief Populate a tDot11fIEExtSuppRates
 *
 *
 * \param pMac Pointer to the global MAC data structure
 *
 * \param nChannelNum Channel on which the enclosing frame will be going out
 *
 * \param pDot11f Address of a tDot11fIEExtSuppRates struct to be filled in.
 *
 *
 * This method is a NOP if the channel is greater than 14.
 *
 *
 */

tSirRetStatus
populate_dot11f_ext_supp_rates1(tpAniSirGlobal pMac,
				uint8_t nChannelNum,
				tDot11fIEExtSuppRates *pDot11f);

tSirRetStatus
populate_dot11f_ht_caps(tpAniSirGlobal pMac,
			tpPESession psessionEntry, tDot11fIEHTCaps *pDot11f);

tSirRetStatus
populate_dot11f_ht_info(tpAniSirGlobal pMac,
			tDot11fIEHTInfo *pDot11f, tpPESession psessionEntry);

void populate_dot11f_ibss_params(tpAniSirGlobal pMac,
				tDot11fIEIBSSParams *pDot11f,
				tpPESession psessionEntry);

#ifdef ANI_SUPPORT_11H
tSirRetStatus
populate_dot11f_measurement_report0(tpAniSirGlobal pMac,
				tpSirMacMeasReqActionFrame pReq,
				tDot11fIEMeasurementReport *pDot11f);

/* / Populate a tDot11fIEMeasurementReport when the report type is CCA */
tSirRetStatus
populate_dot11f_measurement_report1(tpAniSirGlobal pMac,
				tpSirMacMeasReqActionFrame pReq,
				tDot11fIEMeasurementReport *pDot11f);

/* / Populate a tDot11fIEMeasurementReport when the report type is RPI Hist */
tSirRetStatus
populate_dot11f_measurement_report2(tpAniSirGlobal pMac,
				tpSirMacMeasReqActionFrame pReq,
				tDot11fIEMeasurementReport *pDot11f);
#endif /* ANI_SUPPORT_11H */

/* / Populate a tDot11fIEPowerCaps */
void
populate_dot11f_power_caps(tpAniSirGlobal pMac,
			tDot11fIEPowerCaps *pCaps,
			uint8_t nAssocType, tpPESession psessionEntry);

/* / Populate a tDot11fIEPowerConstraints */
tSirRetStatus
populate_dot11f_power_constraints(tpAniSirGlobal pMac,
				tDot11fIEPowerConstraints *pDot11f);

void
populate_dot11f_qos_caps_ap(tpAniSirGlobal pMac,
			tDot11fIEQOSCapsAp *pDot11f,
			tpPESession psessionEntry);

void
populate_dot11f_qos_caps_station(tpAniSirGlobal pMac, tpPESession session,
				tDot11fIEQOSCapsStation *pDot11f);

tSirRetStatus
populate_dot11f_rsn(tpAniSirGlobal pMac,
		tpSirRSNie pRsnIe, tDot11fIERSN *pDot11f);

tSirRetStatus
populate_dot11f_rsn_opaque(tpAniSirGlobal pMac,
		tpSirRSNie pRsnIe, tDot11fIERSNOpaque *pDot11f);

#if defined(FEATURE_WLAN_WAPI)

tSirRetStatus
populate_dot11f_wapi(tpAniSirGlobal pMac,
		tpSirRSNie pRsnIe, tDot11fIEWAPI *pDot11f);

tSirRetStatus populate_dot11f_wapi_opaque(tpAniSirGlobal pMac,
					tpSirRSNie pRsnIe,
					tDot11fIEWAPIOpaque *pDot11f);

#endif /* defined(FEATURE_WLAN_WAPI) */

/* / Populate a tDot11fIESSID given a tSirMacSSid */
void
populate_dot11f_ssid(tpAniSirGlobal pMac,
		tSirMacSSid *pInternal, tDot11fIESSID *pDot11f);

/* / Populate a tDot11fIESSID from CFG */
tSirRetStatus populate_dot11f_ssid2(tpAniSirGlobal pMac,
				tDot11fIESSID *pDot11f);

/**
 * \brief Populate a tDot11fIESchedule
 *
 * \sa populate_dot11f_wmm_schedule
 *
 *
 * \param pSchedule Address of a tSirMacScheduleIE struct
 *
 * \param pDot11f Address of a tDot11fIESchedule to be filled in
 *
 *
 */

void
populate_dot11f_schedule(tSirMacScheduleIE *pSchedule,
			tDot11fIESchedule *pDot11f);

void
populate_dot11f_supp_channels(tpAniSirGlobal pMac,
			tDot11fIESuppChannels *pDot11f,
			uint8_t nAssocType, tpPESession psessionEntry);

/**
 * \brief Populated a tDot11fIESuppRates
 *
 *
 * \param pMac Pointer to the global MAC data structure
 *
 * \param nChannelNum Channel the enclosing frame will be going out on; see
 * below
 *
 * \param pDot11f Address of a tDot11fIESuppRates struct to be filled in.
 *
 *
 * If nChannelNum is greater than 13, the supported rates will be
 * WNI_CFG_SUPPORTED_RATES_11B.  If it is less than or equal to 13, the
 * supported rates will be WNI_CFG_SUPPORTED_RATES_11A.  If nChannelNum is
 * set to the sentinel value POPULATE_DOT11F_RATES_OPERATIONAL, the struct
 * will be populated with WNI_CFG_OPERATIONAL_RATE_SET.
 *
 *
 */

#define POPULATE_DOT11F_RATES_OPERATIONAL (0xff)

tSirRetStatus
populate_dot11f_supp_rates(tpAniSirGlobal pMac,
			uint8_t nChannelNum,
			tDot11fIESuppRates *pDot11f, tpPESession);

tSirRetStatus
populate_dot11f_rates_tdls(tpAniSirGlobal p_mac,
			tDot11fIESuppRates *p_supp_rates,
			tDot11fIEExtSuppRates *p_ext_supp_rates,
			uint8_t curr_oper_channel);

tSirRetStatus populate_dot11f_tpc_report(tpAniSirGlobal pMac,
					tDot11fIETPCReport *pDot11f,
					tpPESession psessionEntry);

/* / Populate a tDot11FfTSInfo */
void populate_dot11f_ts_info(tSirMacTSInfo *pInfo, tDot11fFfTSInfo *pDot11f);

void populate_dot11f_wmm(tpAniSirGlobal pMac,
			tDot11fIEWMMInfoAp *pInfo,
			tDot11fIEWMMParams *pParams,
			tDot11fIEWMMCaps *pCaps, tpPESession psessionEntry);

void populate_dot11f_wmm_caps(tDot11fIEWMMCaps *pCaps);

#if defined(FEATURE_WLAN_ESE)
/* Fill the ESE version IE */
void populate_dot11f_ese_version(tDot11fIEESEVersion *pESEVersion);
/* Fill the Radio Management Capability */
void populate_dot11f_ese_rad_mgmt_cap(tDot11fIEESERadMgmtCap *pESERadMgmtCap);
/* Fill the CCKM IE */
tSirRetStatus populate_dot11f_ese_cckm_opaque(tpAniSirGlobal pMac,
					tpSirCCKMie pCCKMie,
					tDot11fIEESECckmOpaque *pDot11f);

void populate_dot11_tsrsie(tpAniSirGlobal pMac,
			tSirMacESETSRSIE *pOld,
			tDot11fIEESETrafStrmRateSet *pDot11f,
			uint8_t rate_length);
void populate_dot11f_re_assoc_tspec(tpAniSirGlobal pMac,
				tDot11fReAssocRequest *pReassoc,
				tpPESession psessionEntry);
tSirRetStatus
sir_beacon_ie_ese_bcn_report(tpAniSirGlobal pMac,
		uint8_t *pPayload, const uint32_t payloadLength,
		uint8_t **outIeBuf, uint32_t *pOutIeLen);

/**
 * ese_populate_wmm_tspec() - Populates TSPEC info for
 * reassoc
 * @source: source structure
 * @dest: destination structure
 *
 * This function copies TSPEC parameters from source
 * structure to destination structure.
 *
 * Return: None
 */
void ese_populate_wmm_tspec(tSirMacTspecIE *source, ese_wmm_tspec_ie *dest);

#endif

void populate_dot11f_wmm_info_ap(tpAniSirGlobal pMac,
				tDot11fIEWMMInfoAp *pInfo,
				tpPESession psessionEntry);

void populate_dot11f_wmm_info_station_per_session(tpAniSirGlobal pMac,
					tpPESession psessionEntry,
					tDot11fIEWMMInfoStation *pInfo);

void populate_dot11f_wmm_params(tpAniSirGlobal pMac,
				tDot11fIEWMMParams *pParams,
				tpPESession psessionEntry);

/**
 * \brief Populate a tDot11fIEWMMSchedule
 *
 * \sa PopulatedDot11fSchedule
 *
 *
 * \param pSchedule Address of a tSirMacScheduleIE struct
 *
 * \param pDot11f Address of a tDot11fIEWMMSchedule to be filled in
 *
 *
 */

void
populate_dot11f_wmm_schedule(tSirMacScheduleIE *pSchedule,
			tDot11fIEWMMSchedule *pDot11f);

tSirRetStatus
populate_dot11f_wpa(tpAniSirGlobal pMac,
		tpSirRSNie pRsnIe, tDot11fIEWPA *pDot11f);

tSirRetStatus
populate_dot11f_wpa_opaque(tpAniSirGlobal pMac,
			tpSirRSNie pRsnIe, tDot11fIEWPAOpaque *pDot11f);

void populate_dot11f_tspec(tSirMacTspecIE *pOld, tDot11fIETSPEC *pDot11f);

void populate_dot11f_wmmtspec(tSirMacTspecIE *pOld, tDot11fIEWMMTSPEC *pDot11f);

tSirRetStatus
populate_dot11f_tclas(tpAniSirGlobal pMac,
		tSirTclasInfo *pOld, tDot11fIETCLAS *pDot11f);

tSirRetStatus
populate_dot11f_wmmtclas(tpAniSirGlobal pMac,
			tSirTclasInfo *pOld, tDot11fIEWMMTCLAS *pDot11f);

tSirRetStatus populate_dot11f_wsc(tpAniSirGlobal pMac,
			tDot11fIEWscBeacon *pDot11f);

tSirRetStatus populate_dot11f_wsc_registrar_info(tpAniSirGlobal pMac,
						tDot11fIEWscBeacon *pDot11f);

tSirRetStatus de_populate_dot11f_wsc_registrar_info(tpAniSirGlobal pMac,
						tDot11fIEWscBeacon *pDot11f);

tSirRetStatus populate_dot11f_probe_res_wpsi_es(tpAniSirGlobal pMac,
						tDot11fIEWscProbeRes *pDot11f,
						tpPESession psessionEntry);
tSirRetStatus populate_dot11f_assoc_res_wpsi_es(tpAniSirGlobal pMac,
						tDot11fIEWscAssocRes *pDot11f,
						tpPESession psessionEntry);
tSirRetStatus populate_dot11f_beacon_wpsi_es(tpAniSirGlobal pMac,
					tDot11fIEWscBeacon *pDot11f,
					tpPESession psessionEntry);

tSirRetStatus populate_dot11f_wsc_in_probe_res(tpAniSirGlobal pMac,
					tDot11fIEWscProbeRes *pDot11f);

tSirRetStatus
populate_dot11f_wsc_registrar_info_in_probe_res(tpAniSirGlobal pMac,
					tDot11fIEWscProbeRes *pDot11f);

tSirRetStatus
de_populate_dot11f_wsc_registrar_info_in_probe_res(tpAniSirGlobal pMac,
						tDot11fIEWscProbeRes *pDot11f);

tSirRetStatus populate_dot11f_assoc_res_wsc_ie(tpAniSirGlobal pMac,
					tDot11fIEWscAssocRes *pDot11f,
					tpSirAssocReq pRcvdAssocReq);

tSirRetStatus populate_dot11_assoc_res_p2p_ie(tpAniSirGlobal pMac,
					tDot11fIEP2PAssocRes *pDot11f,
					tpSirAssocReq pRcvdAssocReq);

tSirRetStatus populate_dot11f_wscInAssocRes(tpAniSirGlobal pMac,
					tDot11fIEWscAssocRes *pDot11f);

tSirRetStatus populate_dot11f_wfatpc(tpAniSirGlobal pMac,
				tDot11fIEWFATPC *pDot11f, uint8_t txPower,
				uint8_t linkMargin);

tSirRetStatus populate_dot11f_rrm_ie(tpAniSirGlobal pMac,
				tDot11fIERRMEnabledCap *pDot11f,
				tpPESession psessionEntry);

void populate_mdie(tpAniSirGlobal pMac,
		tDot11fIEMobilityDomain * pDot11f, uint8_t mdie[]);
void populate_ft_info(tpAniSirGlobal pMac, tDot11fIEFTInfo *pDot11f);

void populate_dot11f_assoc_rsp_rates(tpAniSirGlobal pMac,
				tDot11fIESuppRates *pSupp,
				tDot11fIEExtSuppRates *pExt,
				uint16_t *_11bRates, uint16_t *_11aRates);

int find_ie_location(tpAniSirGlobal pMac, tpSirRSNie pRsnIe, uint8_t EID);

void lim_log_vht_cap(tpAniSirGlobal pMac, tDot11fIEVHTCaps *pDot11f);

tSirRetStatus
populate_dot11f_vht_caps(tpAniSirGlobal pMac, tpPESession psessionEntry,
			tDot11fIEVHTCaps *pDot11f);

tSirRetStatus
populate_dot11f_vht_operation(tpAniSirGlobal pMac,
			tpPESession psessionEntry,
			tDot11fIEVHTOperation *pDot11f);

tSirRetStatus
populate_dot11f_vht_ext_bss_load(tpAniSirGlobal pMac,
				tDot11fIEVHTExtBssLoad *pDot11f);

tSirRetStatus
populate_dot11f_ext_cap(tpAniSirGlobal pMac, bool isVHTEnabled,
			tDot11fIEExtCap *pDot11f, tpPESession psessionEntry);

void populate_dot11f_qcn_ie(tDot11fIEQCN_IE *pDot11f);

#ifdef WLAN_FEATURE_FILS_SK
/**
 * populate_dot11f_fils_params() - Populate FILS IE to frame
 * @mac_ctx: global mac context
 * @frm: Assoc request frame
 * @pe_session: PE session
 *
 * This API is used to populate FILS IE to Association request
 *
 * Return: None
 */
void populate_dot11f_fils_params(tpAniSirGlobal mac_ctx,
				 tDot11fAssocRequest * frm,
				 tpPESession pe_session);
#else
static inline void populate_dot11f_fils_params(tpAniSirGlobal mac_ctx,
				 tDot11fAssocRequest *frm,
				 tpPESession pe_session)
{ }
#endif

tSirRetStatus
populate_dot11f_operating_mode(tpAniSirGlobal pMac,
			tDot11fIEOperatingMode *pDot11f,
			tpPESession psessionEntry);

void
populate_dot11f_wider_bw_chan_switch_ann(tpAniSirGlobal pMac,
					tDot11fIEWiderBWChanSwitchAnn *pDot11f,
					tpPESession psessionEntry);

void populate_dot11f_timeout_interval(tpAniSirGlobal pMac,
				tDot11fIETimeoutInterval *pDot11f,
				uint8_t type, uint32_t value);

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/* Populate a tDot11fIEQComVendorIE */
void
populate_dot11f_avoid_channel_ie(tpAniSirGlobal mac_ctx,
				tDot11fIEQComVendorIE *dot11f,
				tpPESession session_entry);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

tSirRetStatus populate_dot11f_timing_advert_frame(tpAniSirGlobal pMac,
	tDot11fTimingAdvertisementFrame *frame);
void populate_dot11_supp_operating_classes(tpAniSirGlobal mac_ptr,
	tDot11fIESuppOperatingClasses *dot_11_ptr, tpPESession session_entry);

tSirRetStatus
sir_validate_and_rectify_ies(tpAniSirGlobal mac_ctx,
				uint8_t *mgmt_frame,
				uint32_t frame_bytes,
				uint32_t *missing_rsn_bytes);
/**
 * sir_copy_caps_info() - Copy Caps info from tDot11fFfCapabilities to
 *                        beacon/probe response structure.
 * @mac_ctx: MAC Context
 * @caps: tDot11fFfCapabilities structure
 * @pProbeResp: beacon/probe response structure
 *
 * Copy the caps info to beacon/probe response structure
 *
 * Return: None
 */
void sir_copy_caps_info(tpAniSirGlobal mac_ctx, tDot11fFfCapabilities caps,
			tpSirProbeRespBeacon pProbeResp);

#ifdef WLAN_FEATURE_FILS_SK
/**
 * update_fils_data: update fils params from beacon/probe response
 * @fils_ind: pointer to sir_fils_indication
 * @fils_indication: pointer to tDot11fIEfils_indication
 *
 * Return: None
 */
void update_fils_data(struct sir_fils_indication *fils_ind,
				 tDot11fIEfils_indication * fils_indication);
#endif
#ifdef WLAN_FEATURE_11AX
QDF_STATUS populate_dot11f_he_caps(tpAniSirGlobal, tpPESession,
				   tDot11fIEhe_cap *);
QDF_STATUS populate_dot11f_he_operation(tpAniSirGlobal, tpPESession,
					tDot11fIEhe_op *);
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
QDF_STATUS populate_dot11f_he_bss_color_change(tpAniSirGlobal mac_ctx,
				tpPESession session,
				tDot11fIEbss_color_change *bss_color);
#else
static inline QDF_STATUS populate_dot11f_he_bss_color_change(
				tpAniSirGlobal mac_ctx,
				tpPESession session,
				tDot11fIEbss_color_change *bss_color)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#else
static inline QDF_STATUS populate_dot11f_he_caps(tpAniSirGlobal mac_ctx,
			tpPESession session, tDot11fIEhe_cap *he_cap)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS populate_dot11f_he_operation(tpAniSirGlobal mac_ctx,
			tpPESession session, tDot11fIEhe_op *he_op)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS populate_dot11f_he_bss_color_change(
				tpAniSirGlobal mac_ctx,
				tpPESession session,
				tDot11fIEbss_color_change *bss_color)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * sir_unpack_beacon_ie: wrapper to unpack beacon and update def RSN params
 * if optional fields are not present.
 * @mac_ctx: mac context
 * @buf: beacon buffer pointer
 * @buf_len: beacon buffer length
 * @frame: outframe frame structure
 * @append_ie: flag to indicate if the frame need to be appended from buf
 *
 * Return: parse status
 */
uint32_t sir_unpack_beacon_ie(tpAniSirGlobal mac_ctx, uint8_t *buf,
				       uint32_t buf_len,
				       tDot11fBeaconIEs *frame, bool append_ie);

/**
 * lim_truncate_ppet: truncates ppet of trailling zeros
 * @ppet: ppet to truncate
 * max_len: max length of ppet
 *
 * Return: new length after truncation
 */
static inline uint32_t lim_truncate_ppet(uint8_t *ppet, uint32_t max_len)
{
	while (max_len) {
		if (ppet[max_len - 1])
			break;
		max_len--;
	}
	return max_len;
}
#endif /* __PARSE_H__ */
