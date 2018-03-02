/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains commnon ieee80211 definitions
 */

#ifndef _WLAN_CMN_IEEE80211_H_
#define _WLAN_CMN_IEEE80211_H_
#include <qdf_types.h>
#include <osdep.h>

#define IEEE80211_CCMP_HEADERLEN    8
#define IEEE80211_CCMP_MICLEN       8
#define IEEE80211_FC1_WEP           0x40
#define WLAN_HDR_IV_LEN            3
#define WLAN_HDR_EXT_IV_BIT        0x20
#define WLAN_HDR_EXT_IV_LEN        4

#define WLAN_SEQ_SEQ_SHIFT 4

#define P2P_WFA_OUI {0x50, 0x6f, 0x9a}
#define P2P_WFA_VER 0x09

#define WSC_OUI 0x0050f204
#define MBO_OCE_OUI 0x506f9a16
#define MBO_OCE_OUI_SIZE 4
#define REDUCED_WAN_METRICS_ATTR 103

/* WCN IE */
/* Microsoft OUI */
#define WCN_OUI 0xf25000
/* WCN */
#define WCN_OUI_TYPE 0x04
#define WME_OUI 0xf25000
#define WME_OUI_TYPE 0x02
#define WME_PARAM_OUI_SUBTYPE 0x01
#define WME_INFO_OUI_SUBTYPE 0x00
 /* Atheros OUI */
#define ATH_OUI 0x7f0300
#define ATH_OUI_TYPE 0x01
/* Atheros Extended Cap Type */
#define ATH_OUI_EXTCAP_TYPE 0x04
/* QCA Bandwidth NSS Mapping Type */
#define ATH_OUI_BW_NSS_MAP_TYPE 0x05
#define SFA_OUI 0x964000
#define SFA_OUI_TYPE 0x14
/* QCA OUI (in little endian) */
#define QCA_OUI 0xf0fd8c
#define QCA_OUI_WHC_TYPE  0x00

/* Temporary vendor specific IE for 11n pre-standard interoperability */
#define VENDOR_HT_OUI       0x00904c
#define VENDOR_HT_CAP_ID    51
#define VENDOR_HT_INFO_ID   52

#define VHT_INTEROP_OUI 0x00904c
#define VHT_INTEROP_TYPE 0x04
#define VHT_INTEROP_OUI_SUBTYPE 0x08
#define VHT_INTEROP_OUI_SUBTYPE_VENDORSPEC 0x18

/* ATH HE OUI ( in little endian) */
#define ATH_HE_OUI                  0x741300
#define ATH_HE_CAP_SUBTYPE          0x01
#define ATH_HE_OP_SUBTYPE           0x02

/* EPR information element flags */
#define ERP_NON_ERP_PRESENT   0x01
#define ERP_USE_PROTECTION    0x02
#define ERP_LONG_PREAMBLE     0x04

#define QCA_OUI_WHC_AP_INFO_SUBTYPE 0x00

#define WLAN_MAX_IE_LEN                255
#define WLAN_RSN_IE_LEN                22

/* Individual element IEs length checks */

#define WLAN_SUPPORTED_RATES_IE_MAX_LEN          8
#define WLAN_DS_PARAM_IE_MAX_LEN                 1
#define WLAN_COUNTRY_IE_MIN_LEN                  3
#define WLAN_QUIET_IE_MAX_LEN                    6
#define WLAN_CSA_IE_MAX_LEN                      3
#define WLAN_XCSA_IE_MAX_LEN                     4
#define WLAN_SECCHANOFF_IE_MAX_LEN               1
#define WLAN_EXT_SUPPORTED_RATES_IE_MAX_LEN      12
#define WLAN_EXTCAP_IE_MAX_LEN                   15
#define WLAN_FILS_INDICATION_IE_MIN_LEN          2
#define WLAN_MOBILITY_DOMAIN_IE_MAX_LEN          3
#define WLAN_OPMODE_IE_MAX_LEN                   1
#define WLAN_IBSSDFS_IE_MIN_LEN                  7

/* HT capability flags */
#define WLAN_HTCAP_C_ADVCODING             0x0001
#define WLAN_HTCAP_C_CHWIDTH40             0x0002
/* Capable of SM Power Save (Static) */
#define WLAN_HTCAP_C_SMPOWERSAVE_STATIC    0x0000
/* Capable of SM Power Save (Dynamic) */
#define WLAN_HTCAP_C_SMPOWERSAVE_DYNAMIC   0x0004
/* Reserved */
#define WLAN_HTCAP_C_SM_RESERVED           0x0008
/* SM enabled, no SM Power Save */
#define WLAN_HTCAP_C_SMPOWERSAVE_DISABLED            0x000c
#define WLAN_HTCAP_C_GREENFIELD            0x0010
#define WLAN_HTCAP_C_SHORTGI20             0x0020
#define WLAN_HTCAP_C_SHORTGI40             0x0040
#define WLAN_HTCAP_C_TXSTBC                0x0080
#define WLAN_HTCAP_C_TXSTBC_S                   7
/* 2 bits */
#define WLAN_HTCAP_C_RXSTBC                0x0300
#define WLAN_HTCAP_C_RXSTBC_S                   8
#define WLAN_HTCAP_C_DELAYEDBLKACK         0x0400
/* 1 = 8K, 0 = 3839B */
#define WLAN_HTCAP_C_MAXAMSDUSIZE          0x0800
#define WLAN_HTCAP_C_DSSSCCK40             0x1000
#define WLAN_HTCAP_C_PSMP                  0x2000
#define WLAN_HTCAP_C_INTOLERANT40          0x4000
#define WLAN_HTCAP_C_LSIGTXOPPROT          0x8000
/* Spatial Multiplexing (SM) capabitlity bitmask */
#define WLAN_HTCAP_C_SM_MASK               0x000c

/* VHT Operation  */
/* 20/40 MHz Operating Channel */
#define WLAN_VHTOP_CHWIDTH_2040          0
/* 80 MHz Operating Channel */
#define WLAN_VHTOP_CHWIDTH_80            1
/* 160 MHz Operating Channel */
#define WLAN_VHTOP_CHWIDTH_160           2
/* 80 + 80 MHz Operating Channel */
#define WLAN_VHTOP_CHWIDTH_80_80         3
/* 160 MHz Operating Channel  (revised signalling) */
#define WLAN_VHTOP_CHWIDTH_REVSIG_160    1
/* 80 + 80 MHz Operating Channel  (revised signalling) */
#define WLAN_VHTOP_CHWIDTH_REVSIG_80_80  1

#define WLAN_RATE_VAL              0x7f

#define WLAN_RV(v)     ((v) & WLAN_RATE_VAL)

#define WLAN_CHAN_IS_5GHZ(chanidx) \
	((chanidx > 30) ? true : false)
#define WLAN_CHAN_IS_2GHZ(chanidx) \
	(((chanidx > 0) && (chanidx < 15)) ? true : false)

/* Check if revised signalling is being used for VHT160 in vhtop */
#define WLAN_IS_REVSIG_VHT160(vhtop) (((vhtop)->vht_op_chwidth == \
	WLAN_VHTOP_CHWIDTH_REVSIG_160) && \
	((vhtop)->vht_op_ch_freq_seg2 != 0) && \
	(abs((vhtop)->vht_op_ch_freq_seg2 - (vhtop)->vht_op_ch_freq_seg1) == 8))

/* Check if revised signalling is being used for VHT80p80 in vhtop */
#define WLAN_IS_REVSIG_VHT80_80(vhtop) (((vhtop)->vht_op_chwidth == \
	WLAN_VHTOP_CHWIDTH_REVSIG_80_80) && \
	((vhtop)->vht_op_ch_freq_seg2 != 0) && \
	(abs((vhtop)->vht_op_ch_freq_seg2 - (vhtop)->vht_op_ch_freq_seg1) > 8))

#define LE_READ_2(p) \
	((uint16_t)\
	((((const uint8_t *)(p))[0]) |\
	(((const uint8_t *)(p))[1] <<  8)))

#define LE_READ_4(p) \
	((uint32_t)\
	((((const uint8_t *)(p))[0]) |\
	(((const uint8_t *)(p))[1] <<  8) |  \
	(((const uint8_t *)(p))[2] << 16) |\
	(((const uint8_t *)(p))[3] << 24)))

#define BE_READ_4(p) \
	((uint32_t)\
	((((const uint8_t *)(p))[0] << 24) |\
	(((const uint8_t *)(p))[1] << 16) |\
	(((const uint8_t *)(p))[2] <<  8) |\
	(((const uint8_t *)(p))[3])))

/**
 * enum ext_chan_offset: extension channel offset
 * @WLAN_HTINFO_EXTOFFSET_NA: no extension channel is present
 * @WLAN_HTINFO_EXTOFFSET_ABOVE: above control channel
 * @WLAN_HTINFO_EXTOFFSET_UNDEF: undefined
 * @WLAN_HTINFO_EXTOFFSET_BELOW: below control channel
 */
enum ext_chan_offset {
	WLAN_HTINFO_EXTOFFSET_NA    = 0,
	WLAN_HTINFO_EXTOFFSET_ABOVE = 1,
	WLAN_HTINFO_EXTOFFSET_UNDEF = 2,
	WLAN_HTINFO_EXTOFFSET_BELOW = 3
};

/**
 * enum element_ie :- Management information element
 * @WLAN_ELEMID_SSID: ssid IE
 * @WLAN_ELEMID_RATES: Rates IE
 * @WLAN_ELEMID_FHPARMS: FH param IE
 * @WLAN_ELEMID_DSPARMS: DS Param IE
 * @WLAN_ELEMID_CFPARMS : CF Param IE
 * @WLAN_ELEMID_TIM: TIM IE
 * @WLAN_ELEMID_IBSSPARMS: Ibss params IE
 * @WLAN_ELEMID_COUNTRY: Country code IE
 * @WLAN_ELEMID_REQINFO: Req Info IE
 * @WLAN_ELEMID_QBSS_LOAD: Qbss load IE
 * @WLAN_ELEMID_TCLAS: TCLAS IE
 * @WLAN_ELEMID_CHALLENGE: Challenge IE
 * @WLAN_ELEMID_PWRCNSTR: Power cn IE
 * @WLAN_ELEMID_PWRCAP: power cap IE
 * @WLAN_ELEMID_TPCREQ: TPC req IE
 * @WLAN_ELEMID_TPCREP: TPC rsp IE
 * @WLAN_ELEMID_SUPPCHAN: Supported channel IE
 * @WLAN_ELEMID_CHANSWITCHANN: Channel switch IE
 * @WLAN_ELEMID_MEASREQ: Measurement request IE
 * @WLAN_ELEMID_MEASREP: Measurement Resp IE
 * @WLAN_ELEMID_QUIET: Quiet IE
 * @WLAN_ELEMID_IBSSDFS: IBSS DFS IE
 * @WLAN_ELEMID_ERP: ERP IE
 * @WLAN_ELEMID_TCLAS_PROCESS: TCLAS process IE
 * @WLAN_ELEMID_HTCAP_ANA: HTT Capability IE
 * @WLAN_ELEMID_RSN: RSN IE
 * @WLAN_ELEMID_XRATES: Extended rate IE
 * @WLAN_ELEMID_HTCAP_VENDOR: HT cap vendor IE
 * @WLAN_ELEMID_HTINFO_VENDOR: HT info vendor IE
 * @WLAN_ELEMID_MOBILITY_DOMAIN: MD IE
 * @WLAN_ELEMID_FT: FT IE
 * @WLAN_ELEMID_TIMEOUT_INTERVAL: Timeout interval IE
 * @WLAN_ELEMID_SUPP_OP_CLASS: OP class IE
 * @WLAN_ELEMID_EXTCHANSWITCHANN: Extended Channel switch IE
 * @WLAN_ELEMID_HTINFO_ANA: HT info IE
 * @WLAN_ELEMID_SECCHANOFFSET: Sec channel Offset IE
 * @WLAN_ELEMID_WAPI: WAPI IE
 * @WLAN_ELEMID_TIME_ADVERTISEMENT: Time IE
 * @WLAN_ELEMID_RRM: Radio resource measurement IE
 * @WLAN_ELEMID_2040_COEXT: 20-40 COext ext IE
 * @WLAN_ELEMID_2040_INTOL:20-40 INT OL IE
 * @WLAN_ELEMID_OBSS_SCAN: OBSS scan IE
 * @WLAN_ELEMID_MMIE: 802.11w Management MIC IE
 * @WLAN_ELEMID_FMS_DESCRIPTOR: 802.11v FMS descriptor IE
 * @WLAN_ELEMID_FMS_REQUEST: 802.11v FMS request IE
 * @WLAN_ELEMID_FMS_RESPONSE: 802.11v FMS response IE
 * @WLAN_ELEMID_BSSMAX_IDLE_PERIOD = 90: BSS MAX IDLE PERIOD
 * @WLAN_ELEMID_TFS_REQUEST: TFS req IE
 * @WLAN_ELEMID_TFS_RESPONSE: TFS resp IE
 * @WLAN_ELEMID_TIM_BCAST_REQUEST: TIM bcast req IE
 * @WLAN_ELEMID_TIM_BCAST_RESPONSE: TIM bcast resp IE
 * @WLAN_ELEMID_INTERWORKING: Interworking IE
 * @WLAN_ELEMID_QOS_MAP: QOS MAP IE
 * @WLAN_ELEMID_XCAPS: Extended capability IE
 * @WLAN_ELEMID_TPC: TPC IE
 * @WLAN_ELEMID_CCKM: CCKM IE
 * @WLAN_ELEMID_VHTCAP: VHT Capabilities
 * @WLAN_ELEMID_VHTOP: VHT Operation
 * @WLAN_ELEMID_EXT_BSS_LOAD: Extended BSS Load
 * @WLAN_ELEMID_WIDE_BAND_CHAN_SWITCH: Wide Band Channel Switch
 * @WLAN_ELEMID_VHT_TX_PWR_ENVLP: VHT Transmit Power Envelope
 * @WLAN_ELEMID_CHAN_SWITCH_WRAP: Channel Switch Wrapper
 * @WLAN_ELEMID_AID: AID
 * @WLAN_ELEMID_QUIET_CHANNEL: Quiet Channel
 * @WLAN_ELEMID_OP_MODE_NOTIFY: Operating Mode Notification
 * @WLAN_ELEMID_VENDOR: vendor private
 * @WLAN_ELEMID_EXTN_ELEM: extended IE
 */
enum element_ie {
	WLAN_ELEMID_SSID             = 0,
	WLAN_ELEMID_RATES            = 1,
	WLAN_ELEMID_FHPARMS          = 2,
	WLAN_ELEMID_DSPARMS          = 3,
	WLAN_ELEMID_CFPARMS          = 4,
	WLAN_ELEMID_TIM              = 5,
	WLAN_ELEMID_IBSSPARMS        = 6,
	WLAN_ELEMID_COUNTRY          = 7,
	WLAN_ELEMID_REQINFO          = 10,
	WLAN_ELEMID_QBSS_LOAD        = 11,
	WLAN_ELEMID_TCLAS            = 14,
	WLAN_ELEMID_CHALLENGE        = 16,
	/* 17-31 reserved for challenge text extension */
	WLAN_ELEMID_PWRCNSTR         = 32,
	WLAN_ELEMID_PWRCAP           = 33,
	WLAN_ELEMID_TPCREQ           = 34,
	WLAN_ELEMID_TPCREP           = 35,
	WLAN_ELEMID_SUPPCHAN         = 36,
	WLAN_ELEMID_CHANSWITCHANN    = 37,
	WLAN_ELEMID_MEASREQ          = 38,
	WLAN_ELEMID_MEASREP          = 39,
	WLAN_ELEMID_QUIET            = 40,
	WLAN_ELEMID_IBSSDFS          = 41,
	WLAN_ELEMID_ERP              = 42,
	WLAN_ELEMID_TCLAS_PROCESS    = 44,
	WLAN_ELEMID_HTCAP_ANA        = 45,
	WLAN_ELEMID_RSN              = 48,
	WLAN_ELEMID_XRATES           = 50,
	WLAN_ELEMID_HTCAP_VENDOR     = 51,
	WLAN_ELEMID_HTINFO_VENDOR    = 52,
	WLAN_ELEMID_MOBILITY_DOMAIN  = 54,
	WLAN_ELEMID_FT               = 55,
	WLAN_ELEMID_TIMEOUT_INTERVAL = 56,
	WLAN_ELEMID_SUPP_OP_CLASS    = 59,
	WLAN_ELEMID_EXTCHANSWITCHANN = 60,
	WLAN_ELEMID_HTINFO_ANA       = 61,
	WLAN_ELEMID_SECCHANOFFSET    = 62,
	WLAN_ELEMID_WAPI             = 68,
	WLAN_ELEMID_TIME_ADVERTISEMENT = 69,
	WLAN_ELEMID_RRM              = 70,
	WLAN_ELEMID_2040_COEXT       = 72,
	WLAN_ELEMID_2040_INTOL       = 73,
	WLAN_ELEMID_OBSS_SCAN        = 74,
	WLAN_ELEMID_MMIE             = 76,
	WLAN_ELEMID_FMS_DESCRIPTOR   = 86,
	WLAN_ELEMID_FMS_REQUEST      = 87,
	WLAN_ELEMID_FMS_RESPONSE     = 88,
	WLAN_ELEMID_BSSMAX_IDLE_PERIOD = 90,
	WLAN_ELEMID_TFS_REQUEST      = 91,
	WLAN_ELEMID_TFS_RESPONSE     = 92,
	WLAN_ELEMID_TIM_BCAST_REQUEST  = 94,
	WLAN_ELEMID_TIM_BCAST_RESPONSE = 95,
	WLAN_ELEMID_INTERWORKING     = 107,
	WLAN_ELEMID_QOS_MAP          = 110,
	WLAN_ELEMID_XCAPS            = 127,
	WLAN_ELEMID_TPC              = 150,
	WLAN_ELEMID_CCKM             = 156,
	WLAN_ELEMID_VHTCAP           = 191,
	WLAN_ELEMID_VHTOP            = 192,
	WLAN_ELEMID_EXT_BSS_LOAD     = 193,
	WLAN_ELEMID_WIDE_BAND_CHAN_SWITCH = 194,
	WLAN_ELEMID_VHT_TX_PWR_ENVLP = 195,
	WLAN_ELEMID_CHAN_SWITCH_WRAP = 196,
	WLAN_ELEMID_AID              = 197,
	WLAN_ELEMID_QUIET_CHANNEL    = 198,
	WLAN_ELEMID_OP_MODE_NOTIFY   = 199,
	WLAN_ELEMID_VENDOR           = 221,
	WLAN_ELEMID_FILS_INDICATION  = 240,
	WLAN_ELEMID_EXTN_ELEM        = 255,
};

/**
 * enum extn_element_ie :- extended management information element
 * @WLAN_EXTN_ELEMID_HECAP: HE capabilites IE
 * @WLAN_EXTN_ELEMID_HEOP:  HE Operation IE
 * @WLAN_EXTN_ELEMID_SRP:   spatial reuse parameter IE
 */
enum extn_element_ie {
	WLAN_EXTN_ELEMID_HECAP       = 35,
	WLAN_EXTN_ELEMID_HEOP        = 36,
	WLAN_EXTN_ELEMID_SRP         = 39,
	WLAN_EXTN_ELEMID_ESP         = 11,
};

#define WLAN_OUI_SIZE 4
#define WLAN_MAX_CIPHER 6
#define WLAN_RSN_SELECTOR_LEN 4
#define WLAN_WPA_SELECTOR_LEN 4
#define PMKID_LEN 16
#define MAX_PMKID 4

#define WLAN_WPA_OUI 0xf25000
#define WLAN_WPA_OUI_TYPE 0x01
#define WPA_VERSION 1
#define WLAN_WPA_SEL(x) (((x) << 24) | WLAN_WPA_OUI)

#define WLAN_RSN_OUI 0xac0f00
#define WLAN_CCKM_OUI 0x964000
#define WLAN_CCKM_ASE_UNSPEC 0
#define WLAN_WPA_CCKM_AKM 0x00964000
#define WLAN_RSN_CCKM_AKM 0x00964000
#define WLAN_RSN_DPP_AKM 0x029A6F50

#define RSN_VERSION 1
#define WLAN_RSN_SEL(x) (((x) << 24) | WLAN_RSN_OUI)
#define WLAN_CCKM_SEL(x) (((x) << 24) | WLAN_CCKM_OUI)

#define WLAN_CSE_NONE                    0x00
#define WLAN_CSE_WEP40                   0x01
#define WLAN_CSE_TKIP                    0x02
#define WLAN_CSE_RESERVED                0x03
#define WLAN_CSE_CCMP                    0x04
#define WLAN_CSE_WEP104                  0x05
#define WLAN_CSE_AES_CMAC                0x06
#define WLAN_CSE_GCMP_128                0x08
#define WLAN_CSE_GCMP_256                0x09
#define WLAN_CSE_CCMP_256                0x0A
#define WLAN_CSE_BIP_GMAC_128            0x0B
#define WLAN_CSE_BIP_GMAC_256            0x0C
#define WLAN_CSE_BIP_CMAC_256            0x0D

#define WLAN_AKM_IEEE8021X        0x01
#define WLAN_AKM_PSK              0x02
#define WLAN_AKM_FT_IEEE8021X     0x03
#define WLAN_AKM_FT_PSK           0x04
#define WLAN_AKM_SHA256_IEEE8021X 0x05
#define WLAN_AKM_SHA256_PSK       0x06
#define WLAN_AKM_SAE              0x08
#define WLAN_AKM_SUITEB_EAP_SHA256 0x0B
#define WLAN_AKM_SUITEB_EAP_SHA384 0x0C
#define WLAN_AKM_FILS_SHA256      0x0E
#define WLAN_AKM_FILS_SHA384      0x0F
#define WLAN_AKM_FILS_FT_SHA256   0x10
#define WLAN_AKM_FILS_FT_SHA384   0x11
#define WLAN_AKM_OWE              0x12

#define WLAN_ASE_NONE                    0x00
#define WLAN_ASE_8021X_UNSPEC            0x01
#define WLAN_ASE_8021X_PSK               0x02
#define WLAN_ASE_FT_IEEE8021X            0x20
#define WLAN_ASE_FT_PSK                  0x40
#define WLAN_ASE_SHA256_IEEE8021X        0x80
#define WLAN_ASE_SHA256_PSK              0x100
#define WLAN_ASE_WPS                     0x200

#define RSN_CAP_MFP_CAPABLE 0x80
#define RSN_CAP_MFP_REQUIRED 0x40

/**
 * struct wlan_rsn_ie_hdr: rsn ie header
 * @elem_id: RSN element id WLAN_ELEMID_RSN.
 * @len: rsn ie length
 * @version: RSN ver
 */
struct wlan_rsn_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 version[2];
};

#define WLAN_RSN_IE_MIN_LEN                    3

/**
 * struct wlan_rsn_ie: rsn ie info
 * @ver: RSN ver
 * @gp_cipher_suite: group cipher
 * @pwise_cipher_count: number of pw cipher
 * @pwise_cipher_suites:  pair wise cipher list
 * @akm_suite_count: Number of akm suite
 * @akm_suites: akm suites list
 * @cap: RSN capability
 * @pmkid_count: number of PMKID
 * @pmkid: PMKID list
 * @mgmt_cipher_suite: management (11w) cipher suite
 */
struct wlan_rsn_ie {
	uint16_t ver;
	uint32_t gp_cipher_suite;
	uint16_t pwise_cipher_count;
	uint32_t pwise_cipher_suites[WLAN_MAX_CIPHER];
	uint16_t akm_suite_count;
	uint32_t akm_suites[WLAN_MAX_CIPHER];
	uint16_t cap;
	uint16_t pmkid_count;
	uint8_t pmkid[MAX_PMKID][PMKID_LEN];
	uint32_t mgmt_cipher_suite;
};

#define WLAN_WAPI_IE_MIN_LEN            20

/**
 * struct wlan_wpa_ie_hdr: wpa ie header
 * @elem_id: Wpa element id, vender specific.
 * @len: wpa ie length
 * @oui: 24-bit OUI followed by 8-bit OUI type
 * @version: wpa ver
 */
struct wlan_wpa_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 oui[4];
	u8 version[2];
};

/**
 * struct wlan_wpa_ie: WPA ie info
 * @ver: WPA ver
 * @mc_cipher: multicast cipher
 * @uc_cipher_count: number of unicast cipher
 * @uc_ciphers:  unicast cipher list
 * @auth_suite_count: Number of akm suite
 * @auth_suites: akm suites list
 * @cap: WPA capability
 */
struct wlan_wpa_ie {
	uint16_t ver;
	uint32_t mc_cipher;
	uint16_t uc_cipher_count;
	uint32_t uc_ciphers[WLAN_MAX_CIPHER];
	uint16_t auth_suite_count;
	uint32_t auth_suites[WLAN_MAX_CIPHER];
	uint16_t cap;
};

#define WAPI_VERSION 1
#define WLAN_WAPI_OUI 0x721400

#define WLAN_WAPI_SEL(x) (((x) << 24) | WLAN_WAPI_OUI)

#define WLAN_WAI_CERT_OR_SMS4 0x01
#define WLAN_WAI_PSK 0x02

/**
 * struct wlan_wapi_ie: WAPI ie info
 * @ver: WAPI ver
 * @akm_suite_count: Number of akm suite
 * @akm_suites: akm suites list
 * @uc_cipher_suites:unicast cipher count
 * @uc_cipher_suites: unicast cipher suite
 * @mc_cipher_suite: mc cipher suite
 */
struct wlan_wapi_ie {
	uint16_t ver;
	uint16_t akm_suite_count;
	uint32_t akm_suites[WLAN_MAX_CIPHER];
	uint16_t uc_cipher_count;
	uint32_t uc_cipher_suites[WLAN_MAX_CIPHER];
	uint32_t mc_cipher_suite;
};

/**
 * struct wlan_frame_hdr: generic IEEE 802.11 frames
 * @i_fc: frame control
 * @i_dur: duration field
 * @i_addr1: mac address 1
 * @i_addr2: mac address 2
 * @i_addr3: mac address 3
 * @i_seq: seq info
 */
struct wlan_frame_hdr {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	union {
		struct {
			uint8_t i_addr1[QDF_MAC_ADDR_SIZE];
			uint8_t i_addr2[QDF_MAC_ADDR_SIZE];
			uint8_t i_addr3[QDF_MAC_ADDR_SIZE];
		};
		uint8_t i_addr_all[3 * QDF_MAC_ADDR_SIZE];
	};
	uint8_t i_seq[2];
} qdf_packed;

/* sequence number offset base on begin of mac header */
#define WLAN_SEQ_CTL_OFFSET         22
#define WLAN_LOW_SEQ_NUM_MASK       0x000F
#define WLAN_HIGH_SEQ_NUM_MASK      0x0FF0
#define WLAN_HIGH_SEQ_NUM_OFFSET    4

/**
 * struct wlan_seq_ctl: sequence number control
 * @frag_num: frag number
 * @seq_num_lo: sequence number low byte
 * @seq_num_hi: sequence number high byte
 */
struct wlan_seq_ctl {
	uint8_t frag_num:4;
	uint8_t seq_num_lo:4;
	uint8_t seq_num_hi:8;
} qdf_packed;

/**
 * union wlan_capability : wlan_capability info
 * @value: capability value
 */
union wlan_capability {
	struct caps {
		uint16_t ess:1;
		uint16_t ibss:1;
		uint16_t cf_pollable:1;
		uint16_t cf_poll_request:1;
		uint16_t privacy:1;
		uint16_t short_preamble:1;
		uint16_t pbcc:1;
		uint16_t channel_agility:1;
		uint16_t spectrum_management:1;
		uint16_t qos:1;
		uint16_t short_slot_time:1;
		uint16_t apsd:1;
		uint16_t reserved2:1;
		uint16_t dsss_ofdm:1;
		uint16_t del_block_ack:1;
		uint16_t immed_block_ack:1;
	} wlan_caps;
	uint16_t value;
} qdf_packed;

/**
 * struct ie_header : IE header
 * @ie_id: Element Id
 * @ie_len: IE Length
 */
struct ie_header {
	uint8_t ie_id;
	uint8_t ie_len;
} qdf_packed;

/**
 * struct extn_ie_header : Extension IE header
 * @ie_id: Element Id
 * @ie_len: IE Length
 * @ie_extn_id: extension id
 */
struct extn_ie_header {
	uint8_t ie_id;
	uint8_t ie_len;
	uint8_t ie_extn_id;
} qdf_packed;


/**
 * struct ie_ssid : ssid IE
 * @ssid_id: SSID Element Id
 * @ssid_len: SSID IE Length
 * @ssid: ssid value
 */
struct ie_ssid {
	uint8_t ssid_id;
	uint8_t ssid_len;
	uint8_t ssid[WLAN_SSID_MAX_LEN];
} qdf_packed;

/**
 * struct ds_ie : ds IE
 * @ie: DS Element Id
 * @len: DS IE Length
 * @cur_chan: channel info
 */
struct ds_ie {
	uint8_t ie;
	uint8_t len;
	uint8_t cur_chan;
} qdf_packed;

/**
 * struct erp_ie: ERP IE
 * @ie: ERP Element Id
 * @len: ERP IE Length
 * @value: EP Info
 */
struct erp_ie {
	uint8_t ie;
	uint8_t len;
	uint8_t value;
} qdf_packed;

/**
 * struct htcap_cmn_ie: HT common IE info
 * @hc_cap: HT capabilities
 * @ampdu_param: ampdu params
 * @mcsset: supported MCS set
 * @extcap: extended HT capabilities
 * @txbf_cap: txbf capabilities
 * @antenna: antenna capabilities
 */
struct htcap_cmn_ie {
	uint16_t hc_cap;
	uint8_t ampdu_param;
	uint8_t mcsset[16];
	uint16_t extcap;
	uint32_t txbf_cap;
	uint8_t antenna;
} qdf_packed;

/**
 * struct htcap_ie: HT Capability IE
 * @id: HT IE
 * @len: HT IE LEN
 * @ie: HT cap info
 */
struct htcap_ie {
	uint8_t id;
	uint8_t len;
	struct htcap_cmn_ie ie;
} qdf_packed;

/**
 * struct fils_indication_ie: FILS indication IE element
 * @id: id
 * @len: len
 * @public_key_identifiers_cnt: public key identifiers count
 * @realm_identifiers_cnt: realm identifiers count
 * @is_ip_config_supported: whether ip config is supported in AP
 * @is_cache_id_present: whether cache identifier is present
 * @is_hessid_present: whether hessid is present
 * @is_fils_sk_auth_supported: FILS shared key authentication is supported
 * @is_fils_sk_auth_pfs_supported: FILS shared key auth with PFS is supported
 * @is_pk_auth_supported: FILS public key authentication is supported
 * @reserved: reserved
 * @variable_data: pointer to data depends on initial variables
 */
struct fils_indication_ie {
	uint8_t id;
	uint8_t len;
	uint16_t public_key_identifiers_cnt:3;
	uint16_t realm_identifiers_cnt:3;
	uint16_t is_ip_config_supported:1;
	uint16_t is_cache_id_present:1;
	uint16_t is_hessid_present:1;
	uint16_t is_fils_sk_auth_supported:1;
	uint16_t is_fils_sk_auth_pfs_supported:1;
	uint16_t is_pk_auth_supported:1;
	uint16_t reserved:4;
	uint8_t variable_data[253];
} qdf_packed;

#define WLAN_VENDOR_HT_IE_OFFSET_LEN    4

/**
 * struct wlan_vendor_ie_htcap: vendor private HT Capability IE
 * @id: HT IE
 * @hlen: HT IE len
 * @oui: vendor OUI
 * @oui_type: Oui type
 * @ie: HT cap info
 */
struct wlan_vendor_ie_htcap {
	uint8_t id;
	uint8_t hlen;
	uint8_t oui[3];
	uint8_t oui_type;
	struct htcap_cmn_ie ie;
} qdf_packed;

/**
 * struct wlan_ie_htinfo_cmn: ht info comman
 * @hi_ctrlchannel: control channel
 * @hi_extchoff: B0-1 extension channel offset
 * @hi_txchwidth: B2 recommended xmiss width set
 * @hi_rifsmode: rifs mode
 * @hi_ctrlaccess: controlled access only
 * @hi_serviceinterval: B5-7 svc interval granularity
 * @uhi_opmode: B0-1 operating mode
 * @hi_nongfpresent: B2 non greenfield devices present
 * @hi_txburstlimit: B3 transmit burst limit
 * @hi_obssnonhtpresent: B4 OBSS non-HT STA present
 * @hi_reserved0: B5-15 reserved
 * @hi_reserved2: B0-5 reserved
 * @hi_dualbeacon: B6 dual beacon
 * @hi_dualctsprot: B7 dual CTS protection
 * @hi_stbcbeacon: B8 STBC beacon
 * @hi_lsigtxopprot: B9 l-sig txop protection full support
 * @hi_pcoactive: B10 pco active
 * @hi_pcophase: B11 pco phase
 * @hi_reserved1: B12-15 reserved
 * @hi_basicmcsset[16]: basic MCS set
 */
struct wlan_ie_htinfo_cmn {
	uint8_t hi_ctrlchannel;
	uint8_t hi_extchoff:2,
		hi_txchwidth:1,
		hi_rifsmode:1,
		hi_ctrlaccess:1,
		hi_serviceinterval:3;
	uint16_t hi_opmode:2,
		hi_nongfpresent:1,
		hi_txburstlimit:1,
		hi_obssnonhtpresent:1,
		hi_reserved0:11;
	uint16_t hi_reserved2:6,
		hi_dualbeacon:1,
		hi_dualctsprot:1,
		hi_stbcbeacon:1,
		hi_lsigtxopprot:1,
		hi_pcoactive:1,
		hi_pcophase:1,
		hi_reserved1:4;
	uint8_t  hi_basicmcsset[16];
} qdf_packed;

/**
 * struct wlan_ie_htinfo: HT info IE
 * @hi_id: HT info IE
 * @hi_len: HT info IE len
 * @hi_ie: HT info info
 */
struct wlan_ie_htinfo {
	uint8_t hi_id;
	uint8_t hi_len;
	struct wlan_ie_htinfo_cmn  hi_ie;
} qdf_packed;

/**
 * struct wlan_ie_htinfo: vendor private HT info IE
 * @hi_id: HT info IE
 * @hi_len: HT info IE len
 * @hi_oui: vendor OUI
 * @hi_ouitype: Oui type
 * @hi_ie: HT info info
 */
struct wlan_vendor_ie_htinfo {
	uint8_t hi_id;
	uint8_t hi_len;
	uint8_t hi_oui[3];
	uint8_t hi_ouitype;
	struct wlan_ie_htinfo_cmn hi_ie;
} qdf_packed;

#define WLAN_VENDOR_VHTCAP_IE_OFFSET    7
#define WLAN_VENDOR_VHTOP_IE_OFFSET     21

/**
 * struct wlan_ie_vhtcaps - VHT capabilities
 * @elem_id: VHT caps IE
 * @elem_len: VHT caps IE len
 * @max_mpdu_len: MPDU length
 * @supported_channel_widthset: channel width set
 * @ldpc_coding: LDPC coding capability
 * @shortgi80: short GI 80 support
 * @shortgi160and80plus80: short Gi 160 & 80+80 support
 * @tx_stbc; Tx STBC cap
 * @tx_stbc: Rx STBC cap
 * @su_beam_former: SU beam former cap
 * @su_beam_formee: SU beam formee cap
 * @csnof_beamformer_antSup: Antenna support for beamforming
 * @num_soundingdim: Sound dimensions
 * @mu_beam_former: MU beam former cap
 * @mu_beam_formee: MU beam formee cap
 * @vht_txops: TXOP power save
 * @htc_vhtcap: HTC VHT capability
 * @max_ampdu_lenexp: AMPDU length
 * @vht_link_adapt: VHT link adapatation capable
 * @rx_antpattern: Rx Antenna pattern
 * @tx_antpattern: Tx Antenna pattern
 * @rx_mcs_map: RX MCS map
 * @rx_high_sup_data_rate : highest RX supported data rate
 * @tx_mcs_map: TX MCS map
 * @tx_sup_data_rate: highest TX supported data rate
 */
struct wlan_ie_vhtcaps {
	uint8_t elem_id;
	uint8_t elem_len;
	uint32_t max_mpdu_len:2;
	uint32_t supported_channel_widthset:2;
	uint32_t ldpc_coding:1;
	uint32_t shortgi80:1;
	uint32_t shortgi160and80plus80:1;
	uint32_t tx_stbc:1;
	uint32_t rx_stbc:3;
	uint32_t su_beam_former:1;
	uint32_t su_beam_formee:1;
	uint32_t csnof_beamformer_antSup:3;
	uint32_t num_soundingdim:3;
	uint32_t mu_beam_former:1;
	uint32_t mu_beam_formee:1;
	uint32_t vht_txops:1;
	uint32_t htc_vhtcap:1;
	uint32_t max_ampdu_lenexp:3;
	uint32_t vht_link_adapt:2;
	uint32_t rx_antpattern:1;
	uint32_t tx_antpattern:1;
	uint32_t unused:2;
	uint16_t rx_mcs_map;
	uint16_t rx_high_sup_data_rate:13;
	uint16_t reserved2:3;
	uint16_t tx_mcs_map;
	uint16_t tx_sup_data_rate:13;
	uint16_t reserved3:3;
} qdf_packed;

/**
 * struct wlan_ie_vhtop: VHT op IE
 * @elem_id: VHT op IE
 * @elem_len: VHT op IE len
 * @vht_op_chwidth: BSS Operational Channel width
 * @vht_op_ch_freq_seg1: Channel Center frequency
 * @vht_op_ch_freq_seg2: Channel Center frequency for 80+80MHz
 * @vhtop_basic_mcs_set: Basic MCS set
 */
struct wlan_ie_vhtop {
	uint8_t elem_id;
	uint8_t elem_len;
	uint8_t vht_op_chwidth;
	uint8_t vht_op_ch_freq_seg1;
	uint8_t vht_op_ch_freq_seg2;
	uint16_t vhtop_basic_mcs_set;
} qdf_packed;

/**
 * struct wlan_country_ie: country IE
 * @ie: country IE
 * @len: IE len
 * @cc: country code
 */
struct wlan_country_ie {
	uint8_t ie;
	uint8_t len;
	uint8_t cc[3];
} qdf_packed;

/**
 * struct wlan_country_ie: country IE
 * @ie: QBSS IE
 * @len: IE len
 * @station_count: number of station associated
 * @qbss_chan_load: qbss channel load
 * @qbss_load_avail: qbss_load_avail
 */
struct qbss_load_ie {
	uint8_t ie;
	uint8_t len;
	uint16_t station_count;
	uint8_t qbss_chan_load;
	uint16_t qbss_load_avail;
} qdf_packed;

/**
 * struct wlan_bcn_frame: beacon frame fixed params
 * @timestamp: the value of sender's TSFTIMER
 * @beacon_interval: beacon interval
 * @capability: capability
 * @ie: variable IE
 */
struct wlan_bcn_frame {
	uint8_t timestamp[8];
	uint16_t beacon_interval;
	union wlan_capability capability;
	struct ie_header ie;
} qdf_packed;

#define WLAN_TIM_IE_MIN_LENGTH             4

/**
 * struct wlan_tim_ie: tim IE
 * @tim_ie: Time IE
 * @tim_len: TIM IE len
 * @tim_count: dtim count
 * @tim_period: dtim period
 * @tim_bitctl: bitmap control
 * @tim_bitmap: variable length bitmap
 */
struct wlan_tim_ie {
	uint8_t tim_ie;         /* WLAN_ELEMID_TIM */
	uint8_t tim_len;
	uint8_t tim_count;      /* DTIM count */
	uint8_t tim_period;     /* DTIM period */
	uint8_t tim_bitctl;     /* bitmap control */
	uint8_t tim_bitmap[251];  /* variable-length bitmap */
} qdf_packed;

/**
 * struct rsn_mdie: mobility domain IE
 * @rsn_id: RSN IE id
 * @rsn_len: RSN IE len
 * @mobility_domain: mobility domain info
 * @ft_capab: ft capability
 *
 * Reference 9.4.2.47 Mobility Domain element (MDE) of 802.11-2016
 */
struct rsn_mdie {
	uint8_t rsn_id;
	uint8_t rsn_len;
	uint8_t mobility_domain[2];
	uint8_t ft_capab;
} qdf_packed;

/**
 * struct srp_ie: Spatial reuse parameter IE
 * @srp_id: SRP IE id
 * @srp_len: SRP IE len
 * @srp_id_extn: SRP Extension ID
 * @sr_control: sr control
 * @non_srg_obsspd_max_offset: non srg obsspd max offset
 * @srg_obss_pd_min_offset: srg obss pd min offset
 * @srg_obss_pd_max_offset: srg obss pd max offset
 * @srg_bss_color_bitmap: srg bss color bitmap
 * @srg_partial_bssid_bitmap: srg partial bssid bitmap
 */
struct wlan_srp_ie {
	uint8_t srp_id;
	uint8_t srp_len;
	uint8_t srp_id_extn;
	uint8_t sr_control;
	union {
		struct {
			uint8_t non_srg_obsspd_max_offset;
			uint8_t srg_obss_pd_min_offset;
			uint8_t srg_obss_pd_max_offset;
			uint8_t srg_bss_color_bitmap[8];
			uint8_t srg_partial_bssid_bitmap[8];
		} qdf_packed nonsrg_srg_info;
		struct {
			uint8_t non_srg_obsspd_max_offset;
		} qdf_packed nonsrg_info;
		struct {
			uint8_t srg_obss_pd_min_offset;
			uint8_t srg_obss_pd_max_offset;
			uint8_t srg_bss_color_bitmap[8];
			uint8_t srg_partial_bssid_bitmap[8];
		} qdf_packed srg_info;
	};
} qdf_packed;

#define ESP_INFORMATION_LIST_LENGTH 3
#define MAX_ESP_INFORMATION_FIELD 4
/*
 * enum access_category: tells about access category in ESP paramameter
 * @ESP_AC_BK: ESP access category for background
 * @ESP_AC_BE: ESP access category for best effort
 * @ESP_AC_VI: ESP access category for video
 * @ESP_AC_VO: ESP access category for Voice
 */
enum access_category {
	ESP_AC_BK,
	ESP_AC_BE,
	ESP_AC_VI,
	ESP_AC_VO,

};
/*
 * struct wlan_esp_info: structure for Esp information parameter
 * @access_category: access category info
 * @reserved: reserved
 * @data_format: two bits in length and tells about data format
 * i.e. 0 = No aggregation is expected to be performed for MSDUs or MPDUs with
 * the Type subfield equal to Data for the corresponding AC
 * 1 = A-MSDU aggregation is expected to be performed for MSDUs for the
 * corresponding AC, but A-MPDU aggregation is not expected to be performed
 * for MPDUs with the Type subfield equal to Data for the corresponding AC
 * 2 = A-MPDU aggregation is expected to be performed for MPDUs with the Type
 * subfield equal to Data for the corresponding AC, but A-MSDU aggregation is
 * not expected to be performed for MSDUs for the corresponding AC
 * 3 = A-MSDU aggregation is expected to be performed for MSDUs for the
 * corresponding AC and A-MPDU aggregation is expected to be performed for
 * MPDUs with the Type subfield equal to Data for the corresponding AC
 * @ba_window_size: BA Window Size subfield is three bits in length and
 * indicates the size of the Block Ack window that is
 * expected for the corresponding access category
 * @estimated_air_fraction: Estimated Air Time Fraction subfield is 8 bits in
 * length and contains an unsigned integer that represents
 * the predicted percentage of time, linearly scaled with 255 representing
 * 100%, that a new STA joining the
 * BSS will be allocated for PPDUs that contain only
 * MPDUs with the Type
 * subfield equal to Data of the
 * corresponding access category for that STA.
 * @ppdu_duration: Data PPDU Duration Target field
 * is 8 bits in length and is
 * an unsigned integer that indicates the
 * expected target duration of PPDUs that contain only MPDUs with the Type
 * subfield equal to Data for the
 * corresponding access category in units of 50 μs
 */
struct wlan_esp_info {
	uint8_t access_category:2;
	uint8_t reserved:1;
	uint8_t data_format:2;
	uint8_t ba_window_size:3;
	uint8_t estimated_air_fraction;
	uint8_t ppdu_duration;
};

/**
 * struct wlan_esp_ie: struct for ESP information
 * @esp_id: ESP IE id
 * @esp_len: ESP IE len
 * @esp_id_extn: ESP Extension ID
 * @esp_info_AC_BK: ESP information related to BK category
 * @esp_info_AC_BE: ESP information related to BE category
 * @esp_info_AC_VI: ESP information related to VI category
 * @esp_info_AC_VO: ESP information related to VO category
 */
struct wlan_esp_ie {
	uint8_t esp_id;
	uint8_t esp_len;
	uint8_t esp_id_extn;
	struct wlan_esp_info esp_info_AC_BK;
	struct wlan_esp_info esp_info_AC_BE;
	struct wlan_esp_info esp_info_AC_VI;
	struct wlan_esp_info esp_info_AC_VO;
} qdf_packed;

/**
 * struct oce_reduced_wan_metrics: struct for oce wan metrics
 * @downlink_av_cap: Download available capacity
 * @uplink_av_cap: Upload available capacity
 */
struct oce_reduced_wan_metrics {
	uint8_t downlink_av_cap:4;
	uint8_t uplink_av_cap:4;
};

/**
 * is_wpa_oui() - If vendor IE is WPA type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is WPA
 *
 * Return: true if its WPA IE
 */
static inline bool
is_wpa_oui(uint8_t *frm)
{
	return (frm[1] > 3) && (LE_READ_4(frm + 2) ==
		((WLAN_WPA_OUI_TYPE << 24) | WLAN_WPA_OUI));
}

/**
 * is_wps_oui() - If vendor IE is WPS type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is WPS
 *
 * Return: true if its WPS IE
 */
static inline bool
is_wps_oui(const uint8_t *frm)
{
	return frm[1] > 3 && BE_READ_4(frm + 2) == WSC_OUI;
}

/**
 * is_mbo_oce_oui() - If vendor IE is MBO/OCE type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is MBO/OCE
 *
 * Return: true if its MBO/OCE IE
 */
static inline bool
is_mbo_oce_oui(const uint8_t *frm)
{
	return frm[1] > 3 && BE_READ_4(frm + 2) == MBO_OCE_OUI;
}

/**
 * is_wcn_oui() - If vendor IE is WCN type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is WCN
 *
 * Return: true if its WCN IE
 */
static inline bool
is_wcn_oui(uint8_t *frm)
{
	return (frm[1] > 4) && (LE_READ_4(frm + 2) ==
		((WCN_OUI_TYPE << 24) | WCN_OUI));
}

/**
 * is_wme_param() - If vendor IE is WME param type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is WME param
 *
 * Return: true if its WME param IE
 */
static inline bool
is_wme_param(const uint8_t *frm)
{
	return (frm[1] > 5) && (LE_READ_4(frm + 2) ==
			((WME_OUI_TYPE << 24) | WME_OUI)) &&
			(frm[6] == WME_PARAM_OUI_SUBTYPE);
}

/**
 * is_wme_info() - If vendor IE is WME info type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is WME info
 *
 * Return: true if its WME info IE
 */
static inline bool
is_wme_info(const uint8_t *frm)
{
	return (frm[1] > 5) && (LE_READ_4(frm + 2) ==
		((WME_OUI_TYPE << 24) | WME_OUI)) &&
		(frm[6] == WME_INFO_OUI_SUBTYPE);
}

/**
 * is_atheros_oui() - If vendor IE is Atheros type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is Atheros
 *
 * Return: true if its Atheros IE
 */
static inline bool
is_atheros_oui(const uint8_t *frm)
{
	return (frm[1] > 3) && LE_READ_4(frm + 2) ==
		((ATH_OUI_TYPE << 24) | ATH_OUI);
}

/**
 * is_atheros_extcap_oui() - If vendor IE is Atheros ext cap
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is Atheros ext cap
 *
 * Return: true if its Atheros ext cap IE
 */
static inline int
is_atheros_extcap_oui(uint8_t *frm)
{
	return (frm[1] > 3) && (LE_READ_4(frm + 2) ==
		((ATH_OUI_EXTCAP_TYPE << 24) | ATH_OUI));
}

/**
 * is_sfa_oui() - If vendor IE is SFA type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is SFA
 *
 * Return: true if its SFA IE
 */
static inline bool
is_sfa_oui(uint8_t *frm)
{
	return (frm[1] > 4) && (LE_READ_4(frm + 2) ==
		((SFA_OUI_TYPE << 24) | SFA_OUI));
}

/**
 * is_p2p_oui() - If vendor IE is P2P type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is P2P
 *
 * Return: true if its P2P IE
 */
static inline bool
is_p2p_oui(const uint8_t *frm)
{
	const uint8_t wfa_oui[3] = P2P_WFA_OUI;

	return (frm[1] >= 4) &&
		(frm[2] == wfa_oui[0]) &&
		(frm[3] == wfa_oui[1]) &&
		(frm[4] == wfa_oui[2]) &&
		(frm[5] == P2P_WFA_VER);
}

/**
 * is_qca_son_oui() - If vendor IE is QCA WHC type
 * @frm: vendor IE pointer
 * @whc_subtype: subtype
 *
 * API to check if vendor IE is QCA WHC
 *
 * Return: true if its QCA WHC IE
 */
static inline bool
is_qca_son_oui(uint8_t *frm, uint8_t whc_subtype)
{
	return (frm[1] > 4) && (LE_READ_4(frm + 2) ==
		((QCA_OUI_WHC_TYPE << 24) | QCA_OUI)) &&
		(*(frm + 6) == whc_subtype);
}

/**
 * is_ht_cap() - If vendor IE is vendor HT cap type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is vendor HT cap
 *
 * Return: true if its vendor HT cap IE
 */
static inline bool
is_ht_cap(uint8_t *frm)
{
	return (frm[1] > 3) && (BE_READ_4(frm + 2) ==
		((VENDOR_HT_OUI << 8) | VENDOR_HT_CAP_ID));
}

/**
 * is_ht_info() - If vendor IE is vendor HT info type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is vendor HT info
 *
 * Return: true if its vendor HT info IE
 */
static inline bool
is_ht_info(uint8_t *frm)
{
	return (frm[1] > 3) && (BE_READ_4(frm + 2) ==
		((VENDOR_HT_OUI << 8) | VENDOR_HT_INFO_ID));
}

/**
 * is_interop_vht() - If vendor IE is VHT interop
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is VHT interop
 *
 * Return: true if its VHT interop IE
 */
static inline bool
is_interop_vht(uint8_t *frm)
{
	return (frm[1] > 12) && (BE_READ_4(frm + 2) ==
		((VHT_INTEROP_OUI << 8) | VHT_INTEROP_TYPE)) &&
		((*(frm + 6) == VHT_INTEROP_OUI_SUBTYPE) ||
		(*(frm + 6) == VHT_INTEROP_OUI_SUBTYPE_VENDORSPEC));
}

/**
 * is_bwnss_oui() - If vendor IE is BW NSS type
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is BW NSS
 *
 * Return: true if its BW NSS IE
 */
static inline bool
is_bwnss_oui(uint8_t *frm)
{
	return (frm[1] > 3) && (LE_READ_4(frm + 2) ==
		((ATH_OUI_BW_NSS_MAP_TYPE << 24) | ATH_OUI));
}

/**
 * is_he_cap_oui() - If vendor IE is HE CAP OUI
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is HE CAP
 *
 * Return: true if its HE CAP IE
 */
static inline bool
is_he_cap_oui(uint8_t *frm)
{
	return (frm[1] > 4) && (LE_READ_4(frm + 2) ==
		((ATH_HE_CAP_SUBTYPE << 24) | ATH_HE_OUI));
}

/**
 * is_he_op_oui() - If vendor IE is HE OP OUI
 * @frm: vendor IE pointer
 *
 * API to check if vendor IE is HE OP OUT
 *
 * Return: true if its HE OP OUI
 */
static inline bool
is_he_op_oui(uint8_t *frm)
{
	return (frm[1] > 4) && (LE_READ_4(frm + 2) ==
		((ATH_HE_OP_SUBTYPE << 24) | ATH_HE_OUI));
}

/**
 * wlan_parse_rsn_ie() - parse rsn ie
 * @rsn_ie: rsn ie ptr
 * @rsn: out structure for the parsed ie
 *
 * API, function to parse rsn ie, if optional fields are not present use the
 * default values defined by standard.
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS wlan_parse_rsn_ie(uint8_t *rsn_ie,
	struct wlan_rsn_ie *rsn)
{
	uint8_t rsn_ie_len, i;
	uint8_t *ie;
	int rem_len;
	const struct wlan_rsn_ie_hdr *hdr;

	if (!rsn_ie)
		return QDF_STATUS_E_NULL_VALUE;

	ie = rsn_ie;
	rsn_ie_len = ie[1] + 2;

	/*
	 * Check the length once for fixed parts:
	 * element id, len and version. Other, variable-length data,
	 * must be checked separately.
	 */
	if (rsn_ie_len < sizeof(struct wlan_rsn_ie_hdr))
		return QDF_STATUS_E_INVAL;

	hdr = (struct wlan_rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != WLAN_ELEMID_RSN ||
	    LE_READ_2(hdr->version) != RSN_VERSION)
		return QDF_STATUS_E_INVAL;

	/* Set default values for optional field. */
	rsn->gp_cipher_suite = WLAN_RSN_SEL(WLAN_CSE_CCMP);
	rsn->pwise_cipher_count = 1;
	rsn->pwise_cipher_suites[0] = WLAN_RSN_SEL(WLAN_CSE_CCMP);
	rsn->akm_suite_count = 1;
	rsn->akm_suites[0] = WLAN_RSN_SEL(WLAN_AKM_IEEE8021X);

	rsn->ver = LE_READ_2(hdr->version);

	ie = (uint8_t *) (hdr + 1);
	rem_len = rsn_ie_len - sizeof(*hdr);

	/* Check if optional group cipher is present */
	if (rem_len >= WLAN_RSN_SELECTOR_LEN) {
		rsn->gp_cipher_suite  = LE_READ_4(ie);
		ie += WLAN_RSN_SELECTOR_LEN;
		rem_len -= WLAN_RSN_SELECTOR_LEN;
	} else if (rem_len > 0) {
		/* RSN IE is invalid as group cipher is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Check if optional pairwise cipher is present */
	if (rem_len >= 2) {
		rsn->pwise_cipher_count = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
		if (rsn->pwise_cipher_count == 0 ||
		    rsn->pwise_cipher_count > WLAN_MAX_CIPHER ||
		    rsn->pwise_cipher_count > rem_len / WLAN_RSN_SELECTOR_LEN)
			return QDF_STATUS_E_INVAL;
		for (i = 0; i < rsn->pwise_cipher_count; i++) {
			rsn->pwise_cipher_suites[i] = LE_READ_4(ie);
			ie += WLAN_RSN_SELECTOR_LEN;
			rem_len -= WLAN_RSN_SELECTOR_LEN;
		}
	} else if (rem_len == 1) {
		/* RSN IE is invalid as pairwise cipher is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Check if optional akm suite is present */
	if (rem_len >= 2) {
		rsn->akm_suite_count = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
		if (rsn->akm_suite_count == 0 ||
		    rsn->akm_suite_count > WLAN_MAX_CIPHER ||
		    rsn->akm_suite_count > rem_len / WLAN_RSN_SELECTOR_LEN)
			return QDF_STATUS_E_INVAL;
		for (i = 0; i < rsn->akm_suite_count; i++) {
			rsn->akm_suites[i] = LE_READ_4(ie);
			ie += WLAN_RSN_SELECTOR_LEN;
			rem_len -= WLAN_RSN_SELECTOR_LEN;
		}
	} else if (rem_len == 1) {
		/* RSN IE is invalid as akm suite is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Update capabilty if present */
	if (rem_len >= 2) {
		rsn->cap = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
	} else if (rem_len == 1) {
		/* RSN IE is invalid as cap field is truncated */
		return QDF_STATUS_E_INVAL;
	}

	/* Update PMKID if present */
	if (rem_len >= 2) {
		rsn->pmkid_count = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
		if (rsn->pmkid_count > (unsigned int) rem_len / PMKID_LEN) {
			rsn->pmkid_count = 0;
			return QDF_STATUS_E_INVAL;
		}

		qdf_mem_copy(rsn->pmkid, ie,
			rsn->pmkid_count * PMKID_LEN);
		ie += rsn->pmkid_count * PMKID_LEN;
		rem_len -= rsn->pmkid_count * PMKID_LEN;
	} else if (rem_len == 1) {
		/* RSN IE is invalid as pmkid count field is truncated */
		return QDF_STATUS_E_INVAL;
	}

	/* Update mgmt cipher if present */
	if (rem_len >= WLAN_RSN_SELECTOR_LEN) {
		rsn->mgmt_cipher_suite = LE_READ_4(ie);
		ie += WLAN_RSN_SELECTOR_LEN;
		rem_len -= WLAN_RSN_SELECTOR_LEN;
	} else if (rem_len > 0) {
		/* RSN IE is invalid as mgmt cipher is truncated */
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_parse_wpa_ie() - parse wpa ie
 * @wpa_ie: wpa ie ptr
 * @wpa: out structure for the parsed ie
 *
 * API, function to parse wpa ie, if optional fields are not present use the
 * default values defined by standard.
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS wlan_parse_wpa_ie(uint8_t *wpa_ie,
	struct wlan_wpa_ie *wpa)
{
	uint8_t wpa_ie_len, i;
	uint8_t *ie;
	int rem_len;
	struct wlan_wpa_ie_hdr *hdr;

	if (!wpa_ie)
		return QDF_STATUS_E_NULL_VALUE;

	ie = wpa_ie;
	wpa_ie_len = ie[1] + 2;

	/*
	 * Check the length once for fixed parts:
	 * element id, len, oui and version. Other, variable-length data,
	 * must be checked separately.
	 */
	if (wpa_ie_len < sizeof(struct wlan_wpa_ie_hdr))
		return QDF_STATUS_E_INVAL;

	hdr = (struct wlan_wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != WLAN_ELEMID_VENDOR ||
	    !is_wpa_oui(wpa_ie) ||
	    LE_READ_2(hdr->version) != WPA_VERSION)
		return QDF_STATUS_E_INVAL;

	/* Set default values for optional field. */
	wpa->mc_cipher = WLAN_WPA_SEL(WLAN_CSE_TKIP);
	wpa->uc_cipher_count = 1;
	wpa->uc_ciphers[0] = WLAN_WPA_SEL(WLAN_CSE_TKIP);
	wpa->auth_suite_count = 1;
	wpa->auth_suites[0] = WLAN_WPA_SEL(WLAN_ASE_8021X_UNSPEC);

	wpa->ver = LE_READ_2(hdr->version);
	ie = (uint8_t *) (hdr + 1);
	rem_len = wpa_ie_len - sizeof(*hdr);

	/* Check if optional group cipher is present */
	if (rem_len >= WLAN_WPA_SELECTOR_LEN) {
		wpa->mc_cipher = LE_READ_4(ie);
		ie += WLAN_WPA_SELECTOR_LEN;
		rem_len -= WLAN_WPA_SELECTOR_LEN;
	} else if (rem_len > 0) {
		/* WPA IE is invalid as group cipher is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Check if optional pairwise cipher is present */
	if (rem_len >= 2) {
		wpa->uc_cipher_count = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
		if (wpa->uc_cipher_count == 0 ||
		    wpa->uc_cipher_count > WLAN_MAX_CIPHER ||
		    wpa->uc_cipher_count > rem_len / WLAN_WPA_SELECTOR_LEN)
			return QDF_STATUS_E_INVAL;
		for (i = 0; i < wpa->uc_cipher_count; i++) {
			wpa->uc_ciphers[i] = LE_READ_4(ie);
			ie += WLAN_WPA_SELECTOR_LEN;
			rem_len -= WLAN_WPA_SELECTOR_LEN;
		}
	} else if (rem_len == 1) {
		/* WPA IE is invalid as pairwise cipher is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Check if optional akm suite is present */
	if (rem_len >= 2) {
		wpa->auth_suite_count = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
		if (wpa->auth_suite_count == 0 ||
		    wpa->auth_suite_count > WLAN_MAX_CIPHER ||
		    wpa->auth_suite_count > rem_len / WLAN_WPA_SELECTOR_LEN)
			return QDF_STATUS_E_INVAL;
		for (i = 0; i < wpa->auth_suite_count; i++) {
			wpa->auth_suites[i] = LE_READ_4(ie);
			ie += WLAN_WPA_SELECTOR_LEN;
			rem_len -= WLAN_WPA_SELECTOR_LEN;
		}
	} else if (rem_len == 1) {
		/* WPA IE is invalid as akm suite is of invalid length */
		return QDF_STATUS_E_INVAL;
	}

	/* Update capabilty if optional capabilty is present */
	if (rem_len >= 2) {
		wpa->cap = LE_READ_2(ie);
		ie += 2;
		rem_len -= 2;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_parse_wapi_ie() - parse wapi ie
 * @wapi_ie: wpa ie ptr
 * @wapi: out structure for the parsed  IE
 *
 * API, function to parse wapi ie
 *
 * Return: void
 */
static inline void wlan_parse_wapi_ie(uint8_t *wapi_ie,
	struct wlan_wapi_ie *wapi)
{
	uint8_t len, i;
	uint8_t *ie;

	if (!wapi_ie)
		return;

	ie = wapi_ie;
	len = ie[1];
	/*
	 * Check the length once for fixed parts: OUI, type,
	 * version, mcast cipher, and 2 selector counts.
	 * Other, variable-length data, must be checked separately.
	 */
	if (len < 20)
		return;

	ie += 2;

	wapi->ver = LE_READ_2(ie);
	if (wapi->ver != WAPI_VERSION)
		return;

	ie += 2;
	len -= 2;

	/* akm */
	wapi->akm_suite_count = LE_READ_2(ie);

	ie += 2;
	len -= 2;

	if ((wapi->akm_suite_count > WLAN_MAX_CIPHER) ||
		   len < (wapi->akm_suite_count * WLAN_OUI_SIZE))
		return;
	for (i = 0 ; i < wapi->akm_suite_count; i++) {
		wapi->akm_suites[i] = LE_READ_4(ie);
		ie += WLAN_OUI_SIZE;
		len -= WLAN_OUI_SIZE;
	}

	wapi->uc_cipher_count = LE_READ_2(ie);
	ie += 2;
	len -= 2;
	if ((wapi->uc_cipher_count > WLAN_MAX_CIPHER) ||
	   len < (wapi->uc_cipher_count * WLAN_OUI_SIZE + 2))
		return;
	for (i = 0 ; i < wapi->uc_cipher_count; i++) {
		wapi->uc_cipher_suites[i] = LE_READ_4(ie);
		ie += WLAN_OUI_SIZE;
		len -= WLAN_OUI_SIZE;
	}

	if (len >= WLAN_OUI_SIZE)
		wapi->mc_cipher_suite = LE_READ_4(ie);
}

/**
 * wlan_parse_oce_reduced_wan_metrics_ie() - parse oce wan metrics
 * @mbo_oce_ie: MBO/OCE ie ptr
 * @wan_metrics: out structure for the reduced wan metric
 *
 * API, function to parse reduced wan metric
 *
 * Return: true if oce wan metrics is present
 */
static inline bool
wlan_parse_oce_reduced_wan_metrics_ie(uint8_t *mbo_oce_ie,
	struct oce_reduced_wan_metrics *wan_metrics)
{
	uint8_t len, attribute_len, attribute_id;
	uint8_t *ie;

	if (!mbo_oce_ie)
		return false;

	ie = mbo_oce_ie;
	len = ie[1];
	ie += 2;

	if (len <= MBO_OCE_OUI_SIZE)
		return false;

	ie += MBO_OCE_OUI_SIZE;
	len -= MBO_OCE_OUI_SIZE;

	while (len > 2) {
		attribute_id = ie[0];
		attribute_len = ie[1];
		len -= 2;
		if (attribute_len > len)
			return false;

		if (attribute_id == REDUCED_WAN_METRICS_ATTR) {
			wan_metrics->downlink_av_cap = ie[2] & 0xff;
			wan_metrics->uplink_av_cap = ie[2] >> 4;
			return true;
		}

		ie += (attribute_len + 2);
		len -= attribute_len;
	}

	return false;
}

#endif /* _WLAN_CMN_IEEE80211_DEFS_H_ */
