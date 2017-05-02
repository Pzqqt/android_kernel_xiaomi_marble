/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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

#ifndef _HALMSGAPI_H_
#define _HALMSGAPI_H_

#include "qdf_types.h"
#include "sir_api.h"
#include "sir_params.h"


/*
 * Validate the OS Type being built
 */

#if defined(ANI_OS_TYPE_ANDROID)        /* ANDROID */

#if defined(ANI_OS_TYPE_QNX)
#error "more than one ANI_OS_TYPE_xxx is defined for this build"
#endif

#elif defined(ANI_OS_TYPE_QNX)        /* QNX */

#if defined(ANI_OS_TYPE_ANDROID)
#error "more than one ANI_OS_TYPE_xxx is defined for this build"
#endif

#elif !defined(ANI_OS_TYPE_ANDROID) && !defined(ANI_OS_TYPE_QNX) /* NONE */
#error "NONE of the ANI_OS_TYPE_xxx are defined for this build"
#endif

/*
 * Validate the compiler
 */
#if (defined(ANI_COMPILER_TYPE_MSVC) && defined(ANI_COMPILER_TYPE_GCC) && \
					defined(ANI_COMPILER_TYPE_RVCT))
#error "more than one ANI_COMPILER_TYPE_xxx is defined for this build"

#elif !(defined(ANI_COMPILER_TYPE_MSVC) || defined(ANI_COMPILER_TYPE_GCC) || \
					defined(ANI_COMPILER_TYPE_RVCT))
#error "NONE of the ANI_COMPILER_TYPE_xxx are defined for this build"

#endif


#define WMA_CONFIG_PARAM_UPDATE_REQ    SIR_CFG_PARAM_UPDATE_IND

#define HAL_NUM_BSSID 2
/* operMode in ADD BSS message */
#define BSS_OPERATIONAL_MODE_AP     0
#define BSS_OPERATIONAL_MODE_STA    1
#define BSS_OPERATIONAL_MODE_IBSS   2
#define BSS_OPERATIONAL_MODE_NDI    3

/* STA entry type in add sta message */
#define STA_ENTRY_SELF              0
#define STA_ENTRY_OTHER             1
#define STA_ENTRY_BSSID             2
/* Special station id for transmitting broadcast frames. */
#define STA_ENTRY_BCAST             3
#define STA_ENTRY_PEER              STA_ENTRY_OTHER
#ifdef FEATURE_WLAN_TDLS
#define STA_ENTRY_TDLS_PEER         4
#endif /* FEATURE_WLAN_TDLS */
#define STA_ENTRY_NDI_PEER          5

#define STA_INVALID_IDX 0xFF

/* invalid channel id. */
#define INVALID_CHANNEL_ID 0

/*
 * From NOVA Mac Arch document
 *  Encryp. mode    The encryption mode
 *  000: Encryption functionality is not enabled
 *  001: Encryption is set to WEP
 *  010: Encryption is set to WEP 104
 *  011: Encryption is set to TKIP
 *  100: Encryption is set to AES
 *  101 - 111: Reserved for future
 */
#define ENC_POLICY_NULL        0
#define ENC_POLICY_WEP40       1
#define ENC_POLICY_WEP104      2
#define ENC_POLICY_TKIP        3
#define ENC_POLICY_AES_CCM     4

/* Max number of bytes required for stations bitmap aligned at 4 bytes boundary
 */
#define HALMSG_NUMBYTES_STATION_BITMAP(x) (((x / 32) + ((x % 32) ? 1 : 0)) * 4)


#define HAL_MAX_SUPP_CHANNELS     128
#define HAL_MAX_SUPP_OPER_CLASSES 32

/**
 * enum eFrameType - frame types
 * @TXRX_FRM_RAW: raw frame
 * @TXRX_FRM_ETH2: ethernet frame
 * @TXRX_FRM_802_3: 802.3 frame
 * @TXRX_FRM_802_11_MGMT: 802.11 mgmt frame
 * @TXRX_FRM_802_11_CTRL: 802.11 control frame
 * @TXRX_FRM_802_11_DATA: 802.11 data frame
 */
typedef enum {
	TXRX_FRM_RAW,
	TXRX_FRM_ETH2,
	TXRX_FRM_802_3,
	TXRX_FRM_802_11_MGMT,
	TXRX_FRM_802_11_CTRL,
	TXRX_FRM_802_11_DATA,
	TXRX_FRM_IGNORED,   /* This frame will be dropped */
	TXRX_FRM_MAX
} eFrameType;

/**
 * enum eFrameTxDir - frame tx direction
 * @ANI_TXDIR_IBSS: IBSS frame
 * @ANI_TXDIR_TODS: frame to DS
 * @ANI_TXDIR_FROMDS: Frame from DS
 * @ANI_TXDIR_WDS: WDS frame
 */
typedef enum {
	ANI_TXDIR_IBSS = 0,
	ANI_TXDIR_TODS,
	ANI_TXDIR_FROMDS,
	ANI_TXDIR_WDS
} eFrameTxDir;

/**
 *struct sAniBeaconStruct - Beacon structure
 * @beaconLength: beacon length
 * @macHdr: mac header for beacon
 */
typedef struct sAniBeaconStruct {
	uint32_t beaconLength;
	tSirMacMgmtHdr macHdr;
} qdf_packed tAniBeaconStruct, *tpAniBeaconStruct;

/**
 * struct sAniProbeRspStruct - probeRsp template structure
 * @macHdr: mac header for probe response
 */
typedef struct sAniProbeRspStruct {
	tSirMacMgmtHdr macHdr;
	/* probeRsp body follows here */
} qdf_packed tAniProbeRspStruct, *tpAniProbeRspStruct;

/**
 * struct tAddStaParams - add sta related parameters
 * @bssId: bssid of sta
 * @assocId: associd
 * @staType: 0 - Self, 1 other/remote, 2 - bssid
 * @staMac: MAC Address of STA
 * @shortPreambleSupported: is short preamble supported or not
 * @listenInterval: Listen interval
 * @wmmEnabled: Support for 11e/WMM
 * @uAPSD: U-APSD Flags: 1b per AC
 * @maxSPLen: Max SP Length
 * @htCapable: 11n HT capable STA
 * @greenFieldCapable: 11n Green Field preamble support
 * @txChannelWidthSet: TX Width Set: 0 - 20 MHz only, 1 - 20/40 MHz
 * @mimoPS: MIMO Power Save
 * @rifsMode: RIFS mode: 0 - NA, 1 - Allowed
 * @lsigTxopProtection: L-SIG TXOP Protection mechanism
 * @us32MaxAmpduDuration: in units of 32 us
 * @maxAmpduSize:  0 : 8k , 1 : 16k, 2 : 32k, 3 : 64k
 * @maxAmpduDensity: 3 : 0~7 : 2^(11nAMPDUdensity -4)
 * @maxAmsduSize: 1 : 3839 bytes, 0 : 7935 bytes
 * @fDsssCckMode40Mhz: DSSS CCK supported 40MHz
 * @fShortGI40Mhz: short GI support for 40Mhz packets
 * @fShortGI20Mhz: short GI support for 20Mhz packets
 * @supportedRates: legacy supported rates
 * @status: QDF status
 * @staIdx: station index
 * @bssIdx: BSSID of BSS to which the station is associated
 * @updateSta: pdate the existing STA entry, if this flag is set
 * @respReqd: A flag to indicate to HAL if the response message is required
 * @rmfEnabled: Robust Management Frame (RMF) enabled/disabled
 * @encryptType: The unicast encryption type in the association
 * @ucUcastSig: Unicast DPU index
 * @ucBcastSig: Broadcast DPU index
 * @sessionId: PE session id
 * @p2pCapableSta: if this is a P2P Capable Sta
 * @csaOffloadEnable: CSA offload enable flag
 * @vhtCapable: is VHT capabale or not
 * @vhtTxChannelWidthSet: VHT channel width
 * @vhtSupportedRxNss: VHT supported RX NSS
 * @vhtTxBFCapable: txbf capable or not
 * @vhtTxMUBformeeCapable: Bformee capable or not
 * @enableVhtpAid: enable VHT AID
 * @enableVhtGid: enable VHT GID
 * @enableAmpduPs: AMPDU power save
 * @enableHtSmps: enable HT SMPS
 * @htSmpsconfig: HT SMPS config
 * @htLdpcCapable: HT LDPC capable
 * @vhtLdpcCapable: VHT LDPC capable
 * @smesessionId: sme session id
 * @wpa_rsn: RSN capable
 * @capab_info: capabality info
 * @ht_caps: HT capabalities
 * @vht_caps: VHT vapabalities
 * @nwType: NW Type
 * @maxTxPower: max tx power
 * @atimIePresent: Peer Atim Info
 * @peerAtimWindowLength: peer ATIM Window length
 * @nss: Return the number of spatial streams supported
 *
 * This structure contains parameter required for
 * add sta request of upper layer.
 */
typedef struct {
	tSirMacAddr bssId;
	uint16_t assocId;
	/* Field to indicate if this is sta entry for itself STA adding entry
	 * for itself or remote (AP adding STA after successful association.
	 * This may or may not be required in production driver.
	 */
	uint8_t staType;
	uint8_t shortPreambleSupported;
	tSirMacAddr staMac;
	uint16_t listenInterval;
	uint8_t wmmEnabled;
	uint8_t uAPSD;
	uint8_t maxSPLen;
	uint8_t htCapable;
	/* 11n Green Field preamble support
	 * 0 - Not supported, 1 - Supported
	 * Add it to RA related fields of sta entry in HAL
	 */
	uint8_t greenFieldCapable;
	uint8_t ch_width;

	tSirMacHTMIMOPowerSaveState mimoPS;
	uint8_t rifsMode;
	/* L-SIG TXOP Protection mechanism
	 * 0 - No Support, 1 - Supported
	 * SG - there is global field.
	 */
	uint8_t lsigTxopProtection;
	uint8_t us32MaxAmpduDuration;
	uint8_t maxAmpduSize;
	uint8_t maxAmpduDensity;
	uint8_t maxAmsduSize;

	/* 11n Parameters */
	/* HT STA should set it to 1 if it is enabled in BSS
	 * HT STA should set it to 0 if AP does not support it.
	 * This indication is sent to HAL and HAL uses this flag
	 * to pickup up appropriate 40Mhz rates.
	 */
	uint8_t fDsssCckMode40Mhz;
	uint8_t fShortGI40Mhz;
	uint8_t fShortGI20Mhz;
	tSirSupportedRates supportedRates;
	/*
	 * Following parameters are for returning status and station index from
	 * HAL to PE via response message. HAL does not read them.
	 */
	/* The return status of SIR_HAL_ADD_STA_REQ is reported here */
	QDF_STATUS status;
	/* Station index; valid only when 'status' field value is
	 * QDF_STATUS_SUCCESS
	 */
	uint8_t staIdx;
	/* BSSID of BSS to which the station is associated.
	 * This should be filled back in by HAL, and sent back to LIM as part of
	 * the response message, so LIM can cache it in the station entry of
	 * hash table. When station is deleted, LIM will make use of this bssIdx
	 * to delete BSS from hal tables and from softmac.
	 */
	uint8_t bssIdx;
	uint8_t updateSta;
	uint8_t respReqd;
	uint8_t rmfEnabled;
	uint32_t encryptType;
	uint8_t ucUcastSig;
	uint8_t ucBcastSig;
	uint8_t sessionId;
	uint8_t p2pCapableSta;
	uint8_t csaOffloadEnable;
	uint8_t vhtCapable;
	uint8_t vhtSupportedRxNss;
	uint8_t vhtTxBFCapable;
	uint8_t enable_su_tx_bformer;
	uint8_t vhtTxMUBformeeCapable;
	uint8_t enableVhtpAid;
	uint8_t enableVhtGid;
	uint8_t enableAmpduPs;
	uint8_t enableHtSmps;
	uint8_t htSmpsconfig;
	bool send_smps_action;
	uint8_t htLdpcCapable;
	uint8_t vhtLdpcCapable;
	uint8_t smesessionId;
	uint8_t wpa_rsn;
	uint16_t capab_info;
	uint16_t ht_caps;
	uint32_t vht_caps;
	tSirNwType nwType;
	int8_t maxTxPower;
	uint8_t atimIePresent;
	uint32_t peerAtimWindowLength;
	uint8_t nonRoamReassoc;
	uint32_t nss;
#ifdef WLAN_FEATURE_11AX
	bool he_capable;
	tDot11fIEvendor_he_cap he_config;
	tDot11fIEvendor_he_op he_op;
#endif
} tAddStaParams, *tpAddStaParams;

/**
 * struct tDeleteStaParams - parameters required for del sta request
 * @staIdx: station index
 * @assocId: association index
 * @status: status
 * @respReqd: is response required
 * @sessionId: PE session id
 * @smesessionId: SME session id
 * @staType: station type
 * @staMac: station mac
 */
typedef struct {
	uint16_t staIdx;
	uint16_t assocId;
	QDF_STATUS status;
	uint8_t respReqd;
	uint8_t sessionId;
	uint8_t smesessionId;
	uint8_t staType;
	tSirMacAddr staMac;
} tDeleteStaParams, *tpDeleteStaParams;

/**
 * struct tSetStaKeyParams - set key params
 * @staIdx: station id
 * @encType: encryption type
 * @wepType: WEP type
 * @defWEPIdx: Default WEP key, valid only for static WEP, must between 0 and 3
 * @key: valid only for non-static WEP encyrptions
 * @singleTidRc: 1=Single TID based Replay Count, 0=Per TID based RC
 * @smesessionId: sme session id
 * @peerMacAddr: peer mac address
 * @status: status
 * @sessionId: session id
 * @sendRsp: send response
 *
 * This is used by PE to configure the key information on a given station.
 * When the secType is WEP40 or WEP104, the defWEPIdx is used to locate
 * a preconfigured key from a BSS the station assoicated with; otherwise
 * a new key descriptor is created based on the key field.
 */
typedef struct {
	uint16_t staIdx;
	tAniEdType encType;
	tAniWepType wepType;
	uint8_t defWEPIdx;
	tSirKeys key[SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS];
	uint8_t singleTidRc;
	uint8_t smesessionId;
	struct qdf_mac_addr peer_macaddr;
	QDF_STATUS status;
	uint8_t sessionId;
	uint8_t sendRsp;
} tSetStaKeyParams, *tpSetStaKeyParams;

/**
 * struct sLimMlmSetKeysReq - set key request parameters
 * @peerMacAddr: peer mac address
 * @sessionId: PE session id
 * @smesessionId: SME session id
 * @aid: association id
 * @edType: Encryption/Decryption type
 * @numKeys: number of keys
 * @key: key data
 */
typedef struct sLimMlmSetKeysReq {
	struct qdf_mac_addr peer_macaddr;
	uint8_t sessionId;      /* Added For BT-AMP Support */
	uint8_t smesessionId;   /* Added for drivers based on wmi interface */
	uint16_t aid;
	tAniEdType edType;      /* Encryption/Decryption type */
	uint8_t numKeys;
	tSirKeys key[SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS];
} tLimMlmSetKeysReq, *tpLimMlmSetKeysReq;

/**
 * struct tAddBssParams - parameters required for add bss params
 * @bssId: MAC Address/BSSID
 * @selfMacAddr: Self Mac Address
 * @bssType: BSS type
 * @operMode: AP - 0; STA - 1;
 * @nwType: network type
 * @shortSlotTimeSupported: is short slot time supported or not
 * @llaCoexist: is 11a coexist or not
 * @llbCoexist: 11b coexist supported or not
 * @llgCoexist: 11g coexist supported or not
 * @ht20Coexist: HT20 coexist supported or not
 * @fLsigTXOPProtectionFullSupport: TXOP protection supported or not
 * @fRIFSMode: RIFS is supported or not
 * @beaconInterval: beacon interval
 * @dtimPeriod: DTIM period
 * @cfParamSet: CF Param Set
 * @rateSet: MAC Rate Set
 * @htCapable: Enable/Disable HT capabilities
 * @obssProtEnabled: Enable/Disable OBSS protection
 * @rmfEnabled: RMF enabled/disabled
 * @htOperMode: HT Operating Mode
 * @HT Operating Mode: Dual CTS Protection: 0 - Unused, 1 - Used
 * @txChannelWidthSet: TX Width Set: 0 - 20 MHz only, 1 - 20/40 MHz
 * @currentOperChannel: Current Operating Channel
 * @currentExtChannel: Current Extension Channel, if applicable
 * @staContext: sta context
 * @status: status
 * @bssIdx: BSS index allocated by HAL
 * @updateBss: update the existing BSS entry, if this flag is set
 * @ssId: Add BSSID info for rxp filter
 * @respReqd: send the response message to LIM only when this flag is set
 * @sessionId: PE session id
 * @txMgmtPower: tx power used for mgmt frames
 * @maxTxPower: max power to be used after applying the power constraint
 * @extSetStaKeyParamValid: Ext Bss Config Msg if set
 * @extSetStaKeyParam: SetStaKeyParams for ext bss msg
 * @ucMaxProbeRespRetryLimit: probe Response Max retries
 * @bHiddenSSIDEn: To Enable Hidden ssid.
 * @bProxyProbeRespEn: To Enable Disable FW Proxy Probe Resp
 * @halPersona: Persona for the BSS can be STA,AP,GO,CLIENT value
 * @bSpectrumMgtEnabled: Spectrum Management Capability, 1:Enabled, 0:Disabled.
 * @vhtCapable: VHT capablity
 * @vhtTxChannelWidthSet: VHT tx channel width
 * @reassocReq: Set only during roaming reassociation
 * @chainMask: chain mask
 * @smpsMode: SMPS mode
 * @dot11_mode: 802.11 mode
 * @he_capable: HE Capability
 * @cac_duration_ms: cac duration in milliseconds
 * @dfs_regdomain: dfs region
 */
typedef struct {
	tSirMacAddr bssId;
#ifdef HAL_SELF_STA_PER_BSS
	tSirMacAddr selfMacAddr;
#endif /* HAL_SELF_STA_PER_BSS */
	tSirBssType bssType;
	uint8_t operMode;
	tSirNwType nwType;
	uint8_t shortSlotTimeSupported;
	uint8_t llaCoexist;
	uint8_t llbCoexist;
	uint8_t llgCoexist;
	uint8_t ht20Coexist;
	uint8_t llnNonGFCoexist;
	uint8_t fLsigTXOPProtectionFullSupport;
	uint8_t fRIFSMode;
	tSirMacBeaconInterval beaconInterval;
	uint8_t dtimPeriod;
	tSirMacCfParamSet cfParamSet;
	tSirMacRateSet rateSet;
	uint8_t htCapable;
	uint8_t obssProtEnabled;
	uint8_t rmfEnabled;
	tSirMacHTOperatingMode htOperMode;
	uint8_t dualCTSProtection;
	uint8_t txChannelWidthSet;
	uint8_t currentOperChannel;
	tAddStaParams staContext;
	QDF_STATUS status;
	uint16_t bssIdx;
	/* HAL should update the existing BSS entry, if this flag is set.
	 * PE will set this flag in case of reassoc, where we want to resue the
	 * the old bssID and still return success.
	 */
	uint8_t updateBss;
	tSirMacSSid ssId;
	uint8_t respReqd;
	uint8_t sessionId;
	int8_t txMgmtPower;
	int8_t maxTxPower;

	uint8_t extSetStaKeyParamValid;
	tSetStaKeyParams extSetStaKeyParam;

	uint8_t ucMaxProbeRespRetryLimit;
	uint8_t bHiddenSSIDEn;
	uint8_t bProxyProbeRespEn;
	uint8_t halPersona;
	uint8_t bSpectrumMgtEnabled;
	uint8_t vhtCapable;
	enum phy_ch_width ch_width;
	uint8_t ch_center_freq_seg0;
	uint8_t ch_center_freq_seg1;
	uint8_t reassocReq;     /* Set only during roaming reassociation */
	uint16_t chainMask;
	uint16_t smpsMode;
	uint8_t dot11_mode;
	uint8_t nonRoamReassoc;
	uint8_t wps_state;
	uint8_t nss;
	uint8_t nss_2g;
	uint8_t nss_5g;
	uint8_t beacon_tx_rate;
	uint32_t tx_aggregation_size;
	uint32_t rx_aggregation_size;
#ifdef WLAN_FEATURE_11AX
	bool he_capable;
	tDot11fIEvendor_he_cap he_config;
	tDot11fIEvendor_he_op he_op;
	uint32_t he_sta_obsspd;
#endif
	uint32_t cac_duration_ms;
	uint32_t dfs_regdomain;
} tAddBssParams, *tpAddBssParams;

/**
 * struct tDeleteBssParams - params required for del bss request
 * @bssIdx: BSSID
 * @status: QDF status
 * @respReqd: response message to LIM only when this flag is set
 * @sessionId: PE session id
 * @bssid: BSSID mac address
 * @smesessionId: sme session id
 */
typedef struct {
	uint8_t bssIdx;
	QDF_STATUS status;
	uint8_t respReqd;
	uint8_t sessionId;
	tSirMacAddr bssid;
	uint8_t smesessionId;
} tDeleteBssParams, *tpDeleteBssParams;

/**
 * struct sSirScanEntry - scan entry
 * @bssIdx: BSSID
 * @activeBSScnt: active BSS count
 */
typedef struct sSirScanEntry {
	uint8_t bssIdx[HAL_NUM_BSSID];
	uint8_t activeBSScnt;
} tSirScanEntry, *ptSirScanEntry;

/**
 * struct tInitScanParams - params required for init scan request
 * @bssid: BSSID
 * @notifyBss: notify BSS
 * @useNoA: use NOA
 * @notifyHost: notify UMAC if set
 * @frameLength: frame length
 * @frameType: frame type
 * @scanDuration: Indicates the scan duration (in ms)
 * @macMgmtHdr: For creation of CTS-to-Self and Data-NULL MAC packets
 * @scanEntry: scan entry
 * @checkLinkTraffic: when this flag is set, HAL should check for
 *                    link traffic prior to scan
 * @status: status
 */
typedef struct {
	tSirMacAddr bssid;
	uint8_t notifyBss;
	uint8_t useNoA;
	uint8_t notifyHost;
	uint8_t frameLength;
	uint8_t frameType;
	uint16_t scanDuration;
	tSirMacMgmtHdr macMgmtHdr;
	tSirScanEntry scanEntry;
	tSirLinkTrafficCheck checkLinkTraffic;
	QDF_STATUS status;
} tInitScanParams, *tpInitScanParams;

typedef enum eDelStaReasonCode {
	HAL_DEL_STA_REASON_CODE_KEEP_ALIVE = 0x1,
	HAL_DEL_STA_REASON_CODE_TIM_BASED = 0x2,
	HAL_DEL_STA_REASON_CODE_RA_BASED = 0x3,
	HAL_DEL_STA_REASON_CODE_UNKNOWN_A2 = 0x4
} tDelStaReasonCode;

typedef enum eSmpsModeValue {
	STATIC_SMPS_MODE = 0x0,
	DYNAMIC_SMPS_MODE = 0x1,
	SMPS_MODE_RESERVED = 0x2,
	SMPS_MODE_DISABLED = 0x3
} tSmpsModeValue;

/**
 * struct tDeleteStaContext - params required for delete sta request
 * @assocId: association id
 * @staId: station id
 * @bssId: mac address
 * @addr2: mac address
 * @reasonCode: reason code
 * @rssi: rssi value during disconnection
 */
typedef struct {
	bool is_tdls;
	uint8_t vdev_id;
	uint16_t assocId;
	uint16_t staId;
	tSirMacAddr bssId;
	tSirMacAddr addr2;
	uint16_t reasonCode;
	int8_t rssi;
} tDeleteStaContext, *tpDeleteStaContext;

/**
 * struct tStartScanParams - params required for start scan request
 * @scanChannel: Indicates the current scan channel
 * @status: return status
 * @startTSF: TSF value
 * @txMgmtPower: TX mgmt power
 */
typedef struct {
	uint8_t scanChannel;
	QDF_STATUS status;
	uint32_t startTSF[2];
	int8_t txMgmtPower;
} tStartScanParams, *tpStartScanParams;

/**
 * struct tEndScanParams - params required for end scan request
 * @scanChannel: Indicates the current scan channel
 * @status: return status
 */
typedef struct {
	uint8_t scanChannel;
	QDF_STATUS status;
} tEndScanParams, *tpEndScanParams;

/**
 * struct tFinishScanParams - params required for finish scan request
 * @bssid: BSSID
 * @currentOperChannel: Current operating channel
 * @cbState: channel bond state
 * @notifyBss: notify BSS flag
 * @notifyHost: notify host flag
 * @frameLength: frame length
 * @frameType: frame type
 * @macMgmtHdr: For creation of CTS-to-Self and Data-NULL MAC packets
 * @scanEntry: scan entry
 * @status: return status
 * Request Type = SIR_HAL_FINISH_SCAN_REQ
 */
typedef struct {
	tSirMacAddr bssid;
	uint8_t currentOperChannel;
	/* If 20/40 MHz is operational, this will indicate the 40 MHz extension
	 * channel in combination with the control channel
	 */
	ePhyChanBondState cbState;
	/* For an STA, indicates if a Data NULL frame needs to be sent
	 * to the AP with FrameControl.PwrMgmt bit set to 0
	 */
	uint8_t notifyBss;
	uint8_t notifyHost;
	uint8_t frameLength;
	uint8_t frameType;
	tSirMacMgmtHdr macMgmtHdr;
	tSirScanEntry scanEntry;
	QDF_STATUS status;
} tFinishScanParams, *tpFinishScanParams;

#ifdef FEATURE_OEM_DATA_SUPPORT

#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 1724
#endif

/**
 * struct tStartOemDataRsp - start OEM Data response
 * @target_rsp: Indicates if the rsp is from Target or WMA generated.
 * @rsp_len: oem data response length
 * @oem_data_rsp: pointer to OEM Data response
 */
typedef struct {
	bool target_rsp;
	uint32_t rsp_len;
	uint8_t *oem_data_rsp;
} tStartOemDataRsp, *tpStartOemDataRsp;
#endif /* FEATURE_OEM_DATA_SUPPORT */

/**
 * struct tBeaconGenParams - params required for beacon gen request
 * @bssIdx: Identifies the BSSID for which it is time to generate a beacon
 * @bssId: BSSID
 * @numOfSta: Number of stations in power save, who have data pending
 * @numOfStaWithoutData: Number of stations in power save,
 *                       who don't have any data pending
 * @fBroadcastTrafficPending: broadcast traffic pending flag
 * @dtimCount: DTIM count
 * @rsvd: reserved(padding)
 */
typedef struct sBeaconGenParams {
	uint8_t bssIdx;
	tSirMacAddr bssId;
#ifdef FIXME_VOLANS
	uint8_t numOfSta;
	uint8_t numOfStaWithoutData;
	uint8_t fBroadcastTrafficPending;
	uint8_t dtimCount;
#endif /* FIXME_VOLANS */
	uint8_t rsvd[3];
} tBeaconGenParams, *tpBeaconGenParams;

/**
 * struct tSendbeaconParams - send beacon parameters
 * @bssId: BSSID mac address
 * @beacon: beacon data
 * @beaconLength: beacon length of template
 * @timIeOffset: TIM IE offset
 * @p2pIeOffset: P2P IE offset
 */
typedef struct {
	tSirMacAddr bssId;
	uint8_t *beacon;
	uint32_t beaconLength;
	uint32_t timIeOffset;
	uint16_t p2pIeOffset;
} tSendbeaconParams, *tpSendbeaconParams;

/**
 * struct tSendProbeRespParams - send probe response parameters
 * @bssId: BSSID
 * @pProbeRespTemplate: probe response template
 * @probeRespTemplateLen: probe response template length
 * @ucProxyProbeReqValidIEBmap: valid IE bitmap
 */
typedef struct sSendProbeRespParams {
	tSirMacAddr bssId;
	uint8_t *pProbeRespTemplate;
	uint32_t probeRespTemplateLen;
	uint32_t ucProxyProbeReqValidIEBmap[8];
} tSendProbeRespParams, *tpSendProbeRespParams;

/**
 * struct tSetBssKeyParams - BSS key parameters
 * @bssIdx: BSSID index
 * @encType: encryption Type
 * @numKeys: number of keys
 * @key: key data
 * @singleTidRc: 1=Single TID based Replay Count, 0=Per TID based RC
 * @smesessionId: sme session id
 * @status: return status of command
 * @sessionId: PE session id
 */
typedef struct {
	uint8_t bssIdx;
	tAniEdType encType;
	uint8_t numKeys;
	tSirKeys key[SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS];
	uint8_t singleTidRc;
	uint8_t smesessionId;
	QDF_STATUS status;
	uint8_t sessionId;
} tSetBssKeyParams, *tpSetBssKeyParams;

/**
 * struct tUpdateBeaconParams - update beacon request parameters
 * @bssIdx: BSSID index
 * @fShortPreamble: shortPreamble mode
 * @fShortSlotTime: short Slot time
 * @beaconInterval: Beacon Interval
 * @llaCoexist: 11a coexist
 * @llbCoexist: 11b coexist
 * @llgCoexist: 11g coexist
 * @ht20MhzCoexist: HT 20MHz coexist
 * @fLsigTXOPProtectionFullSupport: TXOP protection supported or not
 * @fRIFSMode: RIFS mode
 * @paramChangeBitmap: change bitmap
 * @smeSessionId: SME  session id
 */
typedef struct {
	uint8_t bssIdx;
	uint8_t fShortPreamble;
	uint8_t fShortSlotTime;
	uint16_t beaconInterval;
	uint8_t llaCoexist;
	uint8_t llbCoexist;
	uint8_t llgCoexist;
	uint8_t ht20MhzCoexist;
	uint8_t llnNonGFCoexist;
	uint8_t fLsigTXOPProtectionFullSupport;
	uint8_t fRIFSMode;
	uint16_t paramChangeBitmap;
	uint8_t smeSessionId;
	uint32_t he_ops;
} tUpdateBeaconParams, *tpUpdateBeaconParams;

/**
 * struct tUpdateVHTOpMode - VHT operating mode
 * @opMode: VHT operating mode
 * @staId: station id
 * @smesessionId: SME session id
 * @peer_mac: peer mac address
 */
typedef struct {
	uint16_t opMode;
	uint16_t staId;
	uint16_t smesessionId;
	tSirMacAddr peer_mac;
} tUpdateVHTOpMode, *tpUpdateVHTOpMode;

/**
 * struct tUpdateRxNss - update rx nss parameters
 * @rxNss: rx nss value
 * @staId: station id
 * @smesessionId: sme session id
 * @peer_mac: peer mac address
 */
typedef struct {
	uint16_t rxNss;
	uint16_t staId;
	uint16_t smesessionId;
	tSirMacAddr peer_mac;
} tUpdateRxNss, *tpUpdateRxNss;

/**
 * struct tUpdateMembership - update membership parmaters
 * @membership: membership value
 * @staId: station id
 * @smesessionId: SME session id
 * @peer_mac: peer mac address
 */
typedef struct {
	uint32_t membership;
	uint16_t staId;
	uint16_t smesessionId;
	tSirMacAddr peer_mac;
} tUpdateMembership, *tpUpdateMembership;

/**
 * struct tUpdateUserPos - update user position parmeters
 * @userPos: user position
 * @staId: station id
 * @smesessionId: sme session id
 * @peer_mac: peer mac address
 */
typedef struct {
	uint32_t userPos;
	uint16_t staId;
	uint16_t smesessionId;
	tSirMacAddr peer_mac;
} tUpdateUserPos, *tpUpdateUserPos;

/**
 * struct tUpdateCFParams -CF parameters
 * @bssIdx: BSSID index
 * @cfpCount: CFP count
 * @cfpPeriod: the number of DTIM intervals between the start of CFPs
 */
typedef struct {
	uint8_t bssIdx;
	/*
	 * cfpCount indicates how many DTIMs (including the current frame)
	 * appear before the next CFP start. A CFPCount of 0 indicates that
	 * the current DTIM marks the start of the CFP.
	 */
	uint8_t cfpCount;
	uint8_t cfpPeriod;
} tUpdateCFParams, *tpUpdateCFParams;

/**
 * struct tSwitchChannelParams - switch channel request parameter
 * @channelNumber: channel number
 * @localPowerConstraint: local power constraint
 * @secondaryChannelOffset: scondary channel offset
 * @peSessionId: PE session id
 * @txMgmtPower: TX mgmt power
 * @maxTxPower: max tx power
 * @selfStaMacAddr: self mac address
 * @bssId: bssid
 * @status: QDF status
 * @chainMask: chanin mask
 * @smpsMode: SMPS mode
 * @isDfsChannel: is DFS channel
 * @vhtCapable: VHT capable
 * @dot11_mode: 802.11 mode
 * @cac_duration_ms: cac duration in milliseconds
 * @dfs_regdomain: dfs region
 */
typedef struct {
	uint8_t channelNumber;
	uint8_t peSessionId;
	int8_t txMgmtPower;
	int8_t maxTxPower;
	tSirMacAddr selfStaMacAddr;
	/* the request has power constraints, this should be applied only to
	 * that session
	 * VO Wifi comment: BSSID is needed to identify which session issued
	 * this request. As the request has power constraints, this should be
	 * applied only to that session
	 * V IMP: Keep bssId field at the end of this msg.
	 * It is used to mantain backward compatbility by way of ignoring if
	 * using new host/old FW or old host/new FW since it is at the end of
	 * this struct
	 */
	tSirMacAddr bssId;
	QDF_STATUS status;
	uint16_t chainMask;
	uint16_t smpsMode;
	uint8_t isDfsChannel;
	uint8_t vhtCapable;
	enum phy_ch_width ch_width;
	uint8_t ch_center_freq_seg0;
	uint8_t ch_center_freq_seg1;
	uint8_t dot11_mode;

	uint8_t restart_on_chan_switch;
	uint8_t nss;
#ifdef WLAN_FEATURE_11AX
	bool he_capable;
#endif
	uint32_t cac_duration_ms;
	uint32_t dfs_regdomain;
} tSwitchChannelParams, *tpSwitchChannelParams;

typedef void (*tpSetLinkStateCallback)(tpAniSirGlobal pMac, void *msgParam,
		bool status);

/**
 * struct tLinkStateParams - link state parameters
 * @bssid: BSSID
 * @selfMacAddr: self mac address
 * @state: link state
 * @callback: callback function pointer
 * @callbackArg: callback argument
 * @session: session context
 */
typedef struct sLinkStateParams {
	/* SIR_HAL_SET_LINK_STATE */
	tSirMacAddr bssid;
	tSirMacAddr selfMacAddr;
	tSirLinkState state;
	tpSetLinkStateCallback callback;
	void *callbackArg;
	int ft;
	void *session;
	bool status;
} tLinkStateParams, *tpLinkStateParams;

/**
 * struct tAddTsParams - ADDTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: QDF status
 * @sessionId: session id
 * @tsm_interval: TSM interval period passed from lim to WMA
 * @setRICparams: RIC parameters
 * @sme_session_id: sme session id
 */
typedef struct {
	uint16_t staIdx;
	uint16_t tspecIdx;
	tSirMacTspecIE tspec;
	QDF_STATUS status;
	uint8_t sessionId;
#ifdef FEATURE_WLAN_ESE
	uint16_t tsm_interval;
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	uint8_t sme_session_id;
} tAddTsParams, *tpAddTsParams;

/**
 * struct tDelTsParams - DELTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC identifier uniquely identifying a TSPEC for a STA in a BSS
 * @bssId: BSSID
 * @sessionId: session id
 * @userPrio: user priority
 * @delTsInfo: DELTS info
 * @setRICparams: RIC parameters
 */
typedef struct {
	uint16_t staIdx;
	uint16_t tspecIdx;
	tSirMacAddr bssId;
	uint8_t sessionId;
	uint8_t userPrio;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	tSirDeltsReqInfo delTsInfo;
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
} tDelTsParams, *tpDelTsParams;


#define HAL_QOS_NUM_TSPEC_MAX 2
#define HAL_QOS_NUM_AC_MAX 4

/**
 * struct tAggrAddTsParams - ADDTS parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: QDF status
 * @sessionId: session id
 */
typedef struct {
	uint16_t staIdx;
	uint16_t tspecIdx;
	tSirMacTspecIE tspec[HAL_QOS_NUM_AC_MAX];
	QDF_STATUS status[HAL_QOS_NUM_AC_MAX];
	uint8_t sessionId;
} tAggrAddTsParams, *tpAggrAddTsParams;


typedef tSirRetStatus (*tHalMsgCallback)(tpAniSirGlobal pMac, uint32_t mesgId,
					 void *mesgParam);

/**
 * struct tEdcaParams - EDCA parameters
 * @bssIdx: BSSID index
 * @acbe: best effort access catagory
 * @acbk: Background access catagory
 * @acvi: video access catagory
 * @acvo: voice access catagory
 */
typedef struct {
	uint16_t bssIdx;
	tSirMacEdcaParamRecord acbe;
	tSirMacEdcaParamRecord acbk;
	tSirMacEdcaParamRecord acvi;
	tSirMacEdcaParamRecord acvo;
} tEdcaParams, *tpEdcaParams;

/**
 * struct tSetMIMOPS - MIMO power save related parameters
 * @staIdx: station index
 * @htMIMOPSState: MIMO Power Save State
 * @status: response status
 * @fsendRsp: send response flag
 * @peerMac: peer mac address
 * @sessionId: session id
 */
typedef struct sSet_MIMOPS {
	uint16_t staIdx;
	tSirMacHTMIMOPowerSaveState htMIMOPSState;
	QDF_STATUS status;
	uint8_t fsendRsp;
	tSirMacAddr peerMac;
	uint8_t sessionId;
} tSetMIMOPS, *tpSetMIMOPS;

/**
 * struct tUapsdParams - Uapsd related parameters
 * @bkDeliveryEnabled: BK delivery enable flag
 * @beDeliveryEnabled: BE delivery enable flag
 * @viDeliveryEnabled: VI delivery enable flag
 * @voDeliveryEnabled: VO delivery enable flag
 * @bkTriggerEnabled: BK trigger enable flag
 * @beTriggerEnabled: BE trigger enable flag
 * @viTriggerEnabled: VI trigger enable flag
 * @voTriggerEnabled: VO trigger enable flag
 * @status: response status
 * @bssIdx: BSSID index
 * Request Type = SIR_HAL_ENTER_UAPSD_REQ
 */
typedef struct sUapsdParams {
	uint8_t bkDeliveryEnabled:1;
	uint8_t beDeliveryEnabled:1;
	uint8_t viDeliveryEnabled:1;
	uint8_t voDeliveryEnabled:1;
	uint8_t bkTriggerEnabled:1;
	uint8_t beTriggerEnabled:1;
	uint8_t viTriggerEnabled:1;
	uint8_t voTriggerEnabled:1;
	QDF_STATUS status;
	uint8_t bssIdx;
} tUapsdParams, *tpUapsdParams;

/**
 * struct tHalIndCB - hal message indication callback
 * @pHalIndCB: hal message indication callabck
 */
typedef struct tHalIndCB {
	tHalMsgCallback pHalIndCB;
} tHalIndCB, *tpHalIndCB;

/**
 * struct sControlTxParams - control tx parameters
 * @stopTx: stop transmission
 * @fCtrlGlobal:  Master flag to stop or resume all transmission
 * @ctrlSta: If this flag is set, staBitmap
 * @ctrlBss: If this flag is set, bssBitmap and beaconBitmap is valid
 * @bssBitmap: bitmap of BSS indices to be stopped for resumed
 * @beaconBitmap: this bitmap contains bitmap of BSS indices to be
 *                stopped for resumed for beacon transmission
 */
typedef struct sControlTxParams {
	bool stopTx;
	uint8_t fCtrlGlobal;
	uint8_t ctrlSta;
	uint8_t ctrlBss;
	/* When ctrlBss is set, this bitmap contains bitmap of BSS indices to be
	 * stopped for resumed for transmission.
	 * This is 32 bit bitmap, not array of bytes.
	 */
	uint32_t bssBitmap;
	/* When ctrlBss is set, this bitmap contains bitmap of BSS indices to be
	 * stopped for resumed for beacon transmission.
	 */
	uint32_t beaconBitmap;
} tTxControlParams, *tpTxControlParams;

/**
 * struct tMaxTxPowerParams - Max Tx Power parameters
 * @bssId: BSSID is needed to identify which session issued this request
 * @selfStaMacAddr: self mac address
 * @power: tx power in dbm
 * @dev_mode: device mode
 * Request Type = SIR_HAL_SET_MAX_TX_POWER_REQ
 */
typedef struct sMaxTxPowerParams {
	struct qdf_mac_addr bssId;
	struct qdf_mac_addr selfStaMacAddr;
	/* In request,
	 * power == MaxTx power to be used.
	 * In response,
	 * power == tx power used for management frames.
	 */
	int8_t power;
	enum tQDF_ADAPTER_MODE dev_mode;
} tMaxTxPowerParams, *tpMaxTxPowerParams;

/**
 * struct tMaxTxPowerPerBandParams - max tx power per band info
 * @bandInfo: band info
 * @power: power in dbm
 */
typedef struct sMaxTxPowerPerBandParams {
	eCsrBand bandInfo;
	int8_t power;
} tMaxTxPowerPerBandParams, *tpMaxTxPowerPerBandParams;

/**
 * struct add_sta_self_params - Add Sta Self params
 * @self_mac_addr: self MAC Address
 * @curr_device_mode: operating device mode
 * @type: Vdev Type
 * @sub_type: Vdev Sub Type
 * @session_id: SME Session ID
 * @nss_2g: vdev nss in 2.4G
 * @nss_5g: vdev nss in 5G
 * @status: response status code
 * @tx_aggregation_size: Tx aggregation size
 * @rx_aggregation_size: Rx aggregation size
 * @enable_bcast_probe_rsp: enable broadcast probe response
 * @fils_max_chan_guard_time: FILS max channel guard time
 * @pkt_err_disconn_th: packet drop threshold
 */
struct add_sta_self_params {
	tSirMacAddr self_mac_addr;
	enum tQDF_ADAPTER_MODE curr_device_mode;
	uint32_t type;
	uint32_t sub_type;
	uint8_t session_id;
	uint8_t nss_2g;
	uint8_t nss_5g;
	uint32_t status;
	uint32_t tx_aggregation_size;
	uint32_t rx_aggregation_size;
	bool enable_bcast_probe_rsp;
	uint8_t fils_max_chan_guard_time;
	uint16_t pkt_err_disconn_th;
};

/**
 * struct set_ie_param - set IE params structure
 * @pdev_id: pdev id
 * @ie_type: IE type
 * @nss: Nss value
 * @ie_len: IE length
 * @ie_ptr: Pointer to IE data
 *
 * Holds the set pdev IE req data.
 */
struct set_ie_param {
	uint8_t pdev_id;
	uint8_t ie_type;
	uint8_t nss;
	uint8_t ie_len;
	uint8_t *ie_ptr;
};

/**
 * struct set_dtim_params - dtim params
 * @session_id: SME Session ID
 * @dtim_period: dtim period
 */
struct set_dtim_params {
	uint8_t session_id;
	uint8_t dtim_period;
};

#define DOT11_HT_IE     1
#define DOT11_VHT_IE    2

#ifdef FEATURE_WLAN_TDLS

#define HAL_TDLS_MAX_SUPP_CHANNELS       128
#define HAL_TDLS_MAX_SUPP_OPER_CLASSES   32

/**
 * struct tTdlsPeerCapParams - TDLS peer capablities parameters
 * @isPeerResponder: is peer responder or not
 * @peerUapsdQueue: peer uapsd queue
 * @peerMaxSp: peer max SP value
 * @peerBuffStaSupport: peer buffer sta supported or not
 * @peerOffChanSupport: peer offchannel support
 * @peerCurrOperClass: peer current operating class
 * @selfCurrOperClass: self current operating class
 * @peerChanLen: peer channel length
 * @peerChan: peer channel list
 * @peerOperClassLen: peer operating class length
 * @peerOperClass: peer operating class
 * @prefOffChanNum: peer offchannel number
 * @prefOffChanBandwidth: peer offchannel bandwidth
 * @opClassForPrefOffChan: operating class for offchannel
 */
typedef struct {
	uint8_t isPeerResponder;
	uint8_t peerUapsdQueue;
	uint8_t peerMaxSp;
	uint8_t peerBuffStaSupport;
	uint8_t peerOffChanSupport;
	uint8_t peerCurrOperClass;
	uint8_t selfCurrOperClass;
	uint8_t peerChanLen;
	tSirUpdateChanParam peerChan[HAL_TDLS_MAX_SUPP_CHANNELS];
	uint8_t peerOperClassLen;
	uint8_t peerOperClass[HAL_TDLS_MAX_SUPP_OPER_CLASSES];
	uint8_t prefOffChanNum;
	uint8_t prefOffChanBandwidth;
	uint8_t opClassForPrefOffChan;
} tTdlsPeerCapParams;

/**
 * struct tTdlsPeerStateParams - TDLS peer state parameters
 * @vdevId: vdev id
 * @peerMacAddr: peer mac address
 * @peerCap: peer capabality
 */
typedef struct sTdlsPeerStateParams {
	uint32_t vdevId;
	tSirMacAddr peerMacAddr;
	uint32_t peerState;
	tTdlsPeerCapParams peerCap;
	bool resp_reqd;
} tTdlsPeerStateParams;

/**
 * struct tdls_chan_switch_params - channel switch parameter structure
 * @vdev_id: vdev ID
 * @peer_mac_addr: Peer mac address
 * @tdls_off_ch_bw_offset: Target off-channel bandwitdh offset
 * @tdls_off_ch: Target Off Channel
 * @oper_class: Operating class for target channel
 * @is_responder: Responder or initiator
 */
typedef struct tdls_chan_switch_params_struct {
	uint32_t    vdev_id;
	tSirMacAddr peer_mac_addr;
	uint16_t    tdls_off_ch_bw_offset;
	uint8_t     tdls_off_ch;
	uint8_t     tdls_sw_mode;
	uint8_t     oper_class;
	uint8_t     is_responder;
} tdls_chan_switch_params;

#endif /* FEATURE_WLAN_TDLS */

/**
 * struct tAbortScanParams - Abort scan parameters
 * @SessionId: PE session id
 * @scan_id: Scan ID used for original scan request
 * @scan_requestor_id: Scan requesting entity
 */
typedef struct sAbortScanParams {
	uint8_t SessionId;
	uint32_t scan_id;
	uint32_t scan_requestor_id;
} tAbortScanParams, *tpAbortScanParams;

/**
 * struct del_sta_self_params - Del Sta Self params
 * @session_id: SME Session ID
 * @status: response status code
 * @sme_callback: callback to be called from WMA to SME
 * @sme_ctx: pointer to context provided by SME
 */
struct del_sta_self_params {
	tSirMacAddr self_mac_addr;
	uint8_t session_id;
	uint32_t status;
	QDF_STATUS (*sme_callback) (void *sme_ctx);
	void *sme_ctx;
};

/**
 * struct del_sta_self_rsp_params - Del Sta Self response params
 * @self_sta_param: sta params
 * @generate_rsp: generate response to upper layers
 */
struct del_sta_self_rsp_params {
	struct del_sta_self_params *self_sta_param;
	uint8_t generate_rsp;
};

/**
 * struct tP2pPsParams - P2P powersave related params
 * @opp_ps: opportunistic power save
 * @ctWindow: CT window
 * @count: count
 * @duration: duration
 * @interval: interval
 * @single_noa_duration: single shot noa duration
 * @psSelection: power save selection
 * @sessionId: session id
 */
typedef struct sP2pPsParams {
	uint8_t opp_ps;
	uint32_t ctWindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t psSelection;
	uint8_t sessionId;
} tP2pPsParams, *tpP2pPsParams;

/**
 * struct tTdlsLinkEstablishParams - TDLS Link establish parameters
 * @staIdx: station index
 * @isResponder: responder flag
 * @uapsdQueues: uapsd queue
 * @maxSp: max SP period
 * @isBufsta: is station flag
 * @isOffChannelSupported: offchannel supported or not
 * @peerCurrOperClass: peer current operating class
 * @selfCurrOperClass: self current operating class
 * @validChannelsLen: valid channel length
 * @validChannels: valid channels
 * @validOperClassesLen: valid operating class length
 * @validOperClasses: valid operating class
 * @status: return status of command
 */
typedef struct sTdlsLinkEstablishParams {
	uint16_t staIdx;
	uint8_t isResponder;
	uint8_t uapsdQueues;
	uint8_t maxSp;
	uint8_t isBufsta;
	uint8_t isOffChannelSupported;
	uint8_t peerCurrOperClass;
	uint8_t selfCurrOperClass;
	uint8_t validChannelsLen;
	uint8_t validChannels[HAL_MAX_SUPP_CHANNELS];
	uint8_t validOperClassesLen;
	uint8_t validOperClasses[HAL_MAX_SUPP_OPER_CLASSES];
	uint32_t status;
} tTdlsLinkEstablishParams, *tpTdlsLinkEstablishParams;

/**
 * struct tHalHiddenSsidVdevRestart - hidden ssid vdev restart params
 * @ssidHidden: is hidden ssid or not
 * @sessionId: session id
 */
typedef struct tHalHiddenSsidVdevRestart {
	uint8_t ssidHidden;
	uint8_t sessionId;
} tHalHiddenSsidVdevRestart, *tpHalHiddenSsidVdevRestart;


extern void sys_process_mmh_msg(tpAniSirGlobal pMac,
				struct scheduler_msg *pMsg);

/**
 * struct tBeaconFilterMsg - Beacon Filtering data structure
 * @capabilityInfo: capability info
 * @capabilityMask: capabality mask
 * @beaconInterval: beacon interval
 * @ieNum: IE number
 * @reserved: reserved
 */
typedef struct sBeaconFilterMsg {
	uint16_t capabilityInfo;
	uint16_t capabilityMask;
	uint16_t beaconInterval;
	uint16_t ieNum;
	uint8_t bssIdx;
	uint8_t reserved;
} qdf_packed tBeaconFilterMsg, *tpBeaconFilterMsg;

/**
 * struct tEidByteInfo - Eid byte info
 * @offset: offset
 * @value: value
 * @bitMask: BIT mask
 * @ref: refrence
 */
typedef struct sEidByteInfo {
	uint8_t offset;
	uint8_t value;
	uint8_t bitMask;
	uint8_t ref;
} qdf_packed tEidByteInfo, *tpEidByteInfo;

/**
 * struct tBeaconFilterIe - beacon filter IE
 * @elementId: element IE
 * @checkIePresence: check IE presence
 * @byte: Eid byte info
 */
typedef struct sBeaconFilterIe {
	uint8_t elementId;
	uint8_t checkIePresence;
	tEidByteInfo byte;
} qdf_packed tBeaconFilterIe, *tpBeaconFilterIe;

/**
 * struct tDisableIntraBssFwd - intra bss forward parameters
 * @sessionId: session id
 * @disableintrabssfwd: disable intra bss forward flag
 */
typedef struct sDisableIntraBssFwd {
	uint16_t sessionId;
	bool disableintrabssfwd;
} qdf_packed tDisableIntraBssFwd, *tpDisableIntraBssFwd;

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * struct tStatsExtRequest - ext stats request
 * @vdev_id: vdev id
 * @request_data_len: request data length
 * @request_data: request data
 */
typedef struct sStatsExtRequest {
	uint32_t vdev_id;
	uint32_t request_data_len;
	uint8_t request_data[];
} tStatsExtRequest, *tpStatsExtRequest;
#endif /* WLAN_FEATURE_STATS_EXT */

#ifdef WLAN_FEATURE_NAN
/**
 * struct tNanRequest - NAN request params
 * @request_data_len: request data length
 * @request_data: request data
 */
typedef struct sNanRequest {
	uint16_t request_data_len;
	uint8_t request_data[];
} tNanRequest, *tpNanRequest;
#endif /* WLAN_FEATURE_NAN */

#endif /* _HALMSGAPI_H_ */
