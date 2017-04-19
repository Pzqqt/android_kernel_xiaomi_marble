/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
  * DOC: This file provides the common definitions for object manager
  */

#ifndef _WLAN_CMN_H_
#define _WLAN_CMN_H_

#include <qdf_types.h>

/* Max no of UMAC components */
#define WLAN_UMAC_MAX_COMPONENTS 25
/* Max no. of radios, a pSoc/Device can support */
#define WLAN_UMAC_MAX_PDEVS 3
/* Max no. of VDEV per PSOC */
#define WLAN_UMAC_PSOC_MAX_VDEVS 51
/* Max no. of VDEVs, a PDEV can support */
#define WLAN_UMAC_PDEV_MAX_VDEVS 17
/* Max no. of Peers, a device can support */
#define WLAN_UMAC_PSOC_MAX_PEERS 1024

/* Max length of a SSID */
#define WLAN_SSID_MAX_LEN 32

/* Max sequence number */
#define WLAN_MAX_SEQ_NUM    4096

/* Max no. of Stations can be associated to VDEV*/
#define WLAN_UMAC_MAX_AP_PEERS WLAN_UMAC_PSOC_MAX_PEERS
/* Max no. of peers for STA vap */
#define WLAN_UMAC_MAX_STA_PEERS 2
/* Max vdev_id */
#define WLAN_UMAC_VDEV_ID_MAX 0xFF

/* 802.11 cap info */
#define WLAN_CAPINFO_ESS               0x0001
#define WLAN_CAPINFO_IBSS              0x0002
#define WLAN_CAPINFO_CF_POLLABLE       0x0004
#define WLAN_CAPINFO_CF_POLLREQ        0x0008
#define WLAN_CAPINFO_PRIVACY           0x0010
#define WLAN_CAPINFO_SHORT_PREAMBLE    0x0020
#define WLAN_CAPINFO_PBCC              0x0040
#define WLAN_CAPINFO_CHNL_AGILITY      0x0080
#define WLAN_CAPINFO_SPECTRUM_MGMT     0x0100
#define WLAN_CAPINFO_QOS               0x0200
#define WLAN_CAPINFO_SHORT_SLOTTIME    0x0400
#define WLAN_CAPINFO_APSD              0x0800
#define WLAN_CAPINFO_RADIOMEAS         0x1000
#define WLAN_CAPINFO_DSSSOFDM          0x2000

/* Allowed time to wait for Object creation  */
#define WLAN_VDEV_CREATE_TIMEOUT_CNT 300
 /* 25 msec */
#define WLAN_VDEV_CREATE_TIMEOUT 25

#define WLAN_PDEV_CREATE_TIMEOUT_CNT 300
 /* 25 msec */
#define WLAN_PDEV_CREATE_TIMEOUT 25

#define WLAN_PSOC_CREATE_TIMEOUT_CNT 300
/* 25 msec */
#define WLAN_PSOC_CREATE_TIMEOUT 25
#define WLAN_24_GHZ_BASE_FREQ   (2407)
#define WLAN_5_GHZ_BASE_FREQ    (5000)
#define WLAN_24_GHZ_CHANNEL_6   (6)
#define WLAN_24_GHZ_CHANNEL_14  (14)
#define WLAN_24_GHZ_CHANNEL_15  (15)
#define WLAN_24_GHZ_CHANNEL_27  (27)
#define WLAN_5_GHZ_CHANNEL_170  (170)
#define WLAN_CHAN_SPACING_5MHZ  (5)
#define WLAN_CHAN_SPACING_20MHZ (20)
#define WLAN_CHAN_14_FREQ       (2484)
#define WLAN_CHAN_15_FREQ       (2512)
#define WLAN_CHAN_170_FREQ      (5852)

#define WLAN_MAC_EID_VENDOR     221

/**
 * enum wlan_umac_comp_id - UMAC component id
 * @WLAN_UMAC_COMP_MLME:          MLME
 * @WLAN_UMAC_COMP_MGMT_TXRX:     MGMT Tx/Rx
 * @WLAN_UMAC_COMP_SERIALIZATION: Serialization
 * @WLAN_UMAC_COMP_SCAN: SCAN -   as scan module uses services provided by
 *                                MLME, MGMT_TXRX and SERIALIZATION, SCAN module
 *                                must be initializes after above modules.
 * @WLAN_UMAC_COMP_PMO:           PMO component
 * @WLAN_UMAC_COMP_P2P:           P2P
 * @WLAN_UMAC_COMP_POLICY_MGR:    Policy Manager
 * @WLAN_UMAC_COMP_CONFIG:        Configuration
 * @WLAN_UMAC_COMP_WIFI_POS:      WIFI Positioning
 * @WLAN_UMAC_COMP_TDLS:          TDLS
 * @WLAN_UMAC_COMP_ATF:           Airtime Fairness
 * @WLAN_UMAC_COMP_REGULATORY:    REGULATORY
 * @WLAN_UMAC_COMP_CRYPTO:        CRYPTO
 * @WLAN_UMAC_COMP_NAN:           Neighbor Aware Networking
 * @WLAN_UMAC_COMP_DFS:           DFS
 * @WLAN_UMAC_COMP_ID_MAX:        Maximum components in UMAC
 * @WLAN_UMAC_COMP_OFFCHAN_TXRX:  Offchan TxRx
 *
 * This id is static.
 * On Adding new component, new id has to be assigned
 */
enum wlan_umac_comp_id {
	WLAN_UMAC_COMP_MLME,
	WLAN_UMAC_COMP_MGMT_TXRX,
	WLAN_UMAC_COMP_SERIALIZATION,
	WLAN_UMAC_COMP_SCAN,
	WLAN_UMAC_COMP_PMO,
	WLAN_UMAC_COMP_P2P,
	WLAN_UMAC_COMP_POLICY_MGR,
	WLAN_UMAC_COMP_CONFIG,
	WLAN_UMAC_COMP_WIFI_POS,
	WLAN_UMAC_COMP_TDLS,
	WLAN_UMAC_COMP_ATF,
	WLAN_UMAC_COMP_REGULATORY,
	WLAN_UMAC_COMP_CRYPTO,
	WLAN_UMAC_COMP_NAN,
	WLAN_UMAC_COMP_DFS,
	WLAN_UMAC_COMP_OFFCHAN_TXRX,
	WLAN_UMAC_COMP_ID_MAX,
};

/**
 *  enum WLAN_DEV_TYPE  - for DA or OL architecture types
 *  @WLAN_DEV_DA:       Direct attach
 *  @WLAN_DEV_OL:       Partial offload
 *  @WLAN_DEV_INVALID:  Invalid dev type
 */
typedef enum {
	WLAN_DEV_DA       = 0,
	WLAN_DEV_OL       = 1,
	WLAN_DEV_INVALID  = 3,
} WLAN_DEV_TYPE;

/**
 *  enum wlan_phymode - phy mode
 *  @WLAN_PHYMODE_AUTO:           autoselect
 *  @WLAN_PHYMODE_11A:            5GHz, OFDM
 *  @WLAN_PHYMODE_11B:            2GHz, CCK
 *  @WLAN_PHYMODE_11G:            2GHz, OFDM
 *  @WLAN_PHYMODE_11NA_HT20:      5Ghz, HT20
 *  @WLAN_PHYMODE_11NG_HT20:      2Ghz, HT20
 *  @WLAN_PHYMODE_11NA_HT40PLUS:  5Ghz, HT40 (ext ch +1)
 *  @WLAN_PHYMODE_11NA_HT40MINUS: 5Ghz, HT40 (ext ch -1)
 *  @WLAN_PHYMODE_11NG_HT40PLUS:  2Ghz, HT40 (ext ch +1)
 *  @WLAN_PHYMODE_11NG_HT40MINUS: 2Ghz, HT40 (ext ch -1)
 *  @WLAN_PHYMODE_11NG_HT40:      2Ghz, Auto HT40
 *  @WLAN_PHYMODE_11NA_HT40:      5Ghz, Auto HT40
 *  @WLAN_PHYMODE_11AC_VHT20:     5Ghz, VHT20
 *  @WLAN_PHYMODE_11AC_VHT40PLUS: 5Ghz, VHT40 (Ext ch +1)
 *  @WLAN_PHYMODE_11AC_VHT40MINUS:5Ghz  VHT40 (Ext ch -1)
 *  @WLAN_PHYMODE_11AC_VHT40:     5Ghz, VHT40
 *  @WLAN_PHYMODE_11AC_VHT80:     5Ghz, VHT80
 *  @WLAN_PHYMODE_11AC_VHT160:    5Ghz, VHT160
 *  @WLAN_PHYMODE_11AC_VHT80_80:  5Ghz, VHT80_80
 */
enum wlan_phymode {
	WLAN_PHYMODE_AUTO             = 0,
	WLAN_PHYMODE_11A              = 1,
	WLAN_PHYMODE_11B              = 2,
	WLAN_PHYMODE_11G              = 3,
	WLAN_PHYMODE_11NA_HT20        = 4,
	WLAN_PHYMODE_11NG_HT20        = 5,
	WLAN_PHYMODE_11NA_HT40PLUS    = 6,
	WLAN_PHYMODE_11NA_HT40MINUS   = 7,
	WLAN_PHYMODE_11NG_HT40PLUS    = 8,
	WLAN_PHYMODE_11NG_HT40MINUS   = 9,
	WLAN_PHYMODE_11NG_HT40        = 10,
	WLAN_PHYMODE_11NA_HT40        = 11,
	WLAN_PHYMODE_11AC_VHT20       = 12,
	WLAN_PHYMODE_11AC_VHT40PLUS   = 13,
	WLAN_PHYMODE_11AC_VHT40MINUS  = 14,
	WLAN_PHYMODE_11AC_VHT40       = 15,
	WLAN_PHYMODE_11AC_VHT80       = 16,
	WLAN_PHYMODE_11AC_VHT160      = 17,
	WLAN_PHYMODE_11AC_VHT80_80    = 18,
};

#define WLAN_PHYMODE_MAX      (WLAN_PHYMODE_11AC_VHT80_80 + 1)

/**
 * enum wlan_peer_type  - peer type
 * @WLAN_PEER_SELF:     for AP mode, SELF PEER or AP PEER are same
 * @WLAN_PEER_AP:       BSS peer for STA mode, Self peer for AP mode
 * @WLAN_PEER_P2P_GO:   BSS peer for P2P CLI mode, Self peer for P2P GO mode
 * @WLAN_PEER_STA:      Self Peer for STA mode, STA peer for AP mode
 * @WLAN_PEER_P2P_CLI:  Self peer for P2P CLI mode, P2P CLI peer for P2P GO mode
 * @WLAN_PEER_TDLS:     TDLS Peer
 * @WLAN_PEER_NAWDS:    NAWDS Peer
 * @WLAN_PEER_STA_TEMP: STA Peer Temp (its host only node)
 * @WLAN_PEER_IBSS:     IBSS Peer
 */
enum wlan_peer_type {
	WLAN_PEER_SELF     = 1,
	WLAN_PEER_AP       = 2,
	WLAN_PEER_P2P_GO   = 3,
	WLAN_PEER_STA      = 4,
	WLAN_PEER_P2P_CLI  = 5,
	WLAN_PEER_TDLS     = 6,
	WLAN_PEER_NAWDS    = 7,
	WLAN_PEER_STA_TEMP = 8,
	WLAN_PEER_IBSS     = 9,
};

/**
 * enum wlan_band - specifies operating channel band
 * @WLAN_BAND_ALL: Any band
 * @WLAN_BAND_2_4_GHZ: 2.4 GHz band
 * @WLAN_BAND_5_GHZ: 5 GHz band
 * @WLAN_BAND_4_9_GHZ: 4.9 GHz band
 */
enum wlan_band {
	WLAN_BAND_ALL,
	WLAN_BAND_2_4_GHZ,
	WLAN_BAND_5_GHZ,
	WLAN_BAND_4_9_GHZ,
};

/**
 * enum wlan_bss_type - type of network
 * @WLAN_TYPE_ANY: Default value
 * @WLAN_TYPE_BSS: Type BSS
 * @WLAN_TYPE_IBSS: Type IBSS
 */
enum wlan_bss_type {
	WLAN_TYPE_ANY,
	WLAN_TYPE_BSS,
	WLAN_TYPE_IBSS,
};

/**
 * enum wlan_pmf_cap: pmf capability
 * @PMF_DISABLED: PMF is disabled
 * @PMF_CAPABLE: PMF is supported
 * @PMF_REQUIRED: PMF is mandatory
 */
enum wlan_pmf_cap {
	WLAN_PMF_DISABLED,
	WLAN_PMF_CAPABLE,
	WLAN_PMF_REQUIRED,
};

/**
 * enum wlan_auth_type - Enumeration of the various Auth types
 * @WLAN_AUTH_TYPE_OPEN_SYSTEM: Open auth type
 * @WLAN_AUTH_TYPE_SHARED_KEY: Shared Key Auth type
 * @WLAN_AUTH_TYPE_AUTOSWITCH: Auto switch Open/Shared
 * @WLAN_AUTH_TYPE_WPA: WPA Enterprise
 * @WLAN_AUTH_TYPE_WPA_PSK: WPA PSK
 * @WLAN_AUTH_TYPE_WPA_NONE: WPA None
 * @WLAN_AUTH_TYPE_RSN: RSN Enterprise
 * @WLAN_AUTH_TYPE_RSN_PSK: RSN PSK
 * @WLAN_AUTH_TYPE_FT_RSN: FT RSN Enterprise
 * @WLAN_AUTH_TYPE_FT_RSN_PSK: FT RSN PSK
 * @WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE: WAPI certificate
 * @WLAN_AUTH_TYPE_WAPI_WAI_PSK: WAPI PSK
 * @WLAN_AUTH_TYPE_CCKM_WPA: CCKM WPA
 * @WLAN_AUTH_TYPE_CCKM_RSN: CCKM RSN
 * @WLAN_AUTH_TYPE_RSN_PSK_SHA256: SHA256 PSK
 * @WLAN_AUTH_TYPE_RSN_8021X_SHA256: SHA256 Enterprise
 * @WLAN_NUM_OF_SUPPORT_AUTH_TYPE: Max no of Auth type
 */
enum wlan_auth_type {
	WLAN_AUTH_TYPE_OPEN_SYSTEM,
	WLAN_AUTH_TYPE_SHARED_KEY,
	WLAN_AUTH_TYPE_AUTOSWITCH,
	WLAN_AUTH_TYPE_WPA,
	WLAN_AUTH_TYPE_WPA_PSK,
	WLAN_AUTH_TYPE_WPA_NONE,
	WLAN_AUTH_TYPE_RSN,
	WLAN_AUTH_TYPE_RSN_PSK,
	WLAN_AUTH_TYPE_FT_RSN,
	WLAN_AUTH_TYPE_FT_RSN_PSK,
	WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE,
	WLAN_AUTH_TYPE_WAPI_WAI_PSK,
	WLAN_AUTH_TYPE_CCKM_WPA,
	WLAN_AUTH_TYPE_CCKM_RSN,
	WLAN_AUTH_TYPE_RSN_PSK_SHA256,
	WLAN_AUTH_TYPE_RSN_8021X_SHA256,
	WLAN_NUM_OF_SUPPORT_AUTH_TYPE,
};

/**
 * enum wlan_enc_type - Enumeration of the various Enc types
 * @WLAN_ENCRYPT_TYPE_NONE: No encryption
 * @WLAN_ENCRYPT_TYPE_WEP40_STATICKEY: WEP 40 Static key
 * @WLAN_ENCRYPT_TYPE_WEP104_STATICKEY: WEP 104 Static key
 * @WLAN_ENCRYPT_TYPE_WEP40: WEP 40
 * @WLAN_ENCRYPT_TYPE_WEP104: WEP 104
 * @WLAN_ENCRYPT_TYPE_TKIP: TKIP
 * @WLAN_ENCRYPT_TYPE_AES: AES
 * @WLAN_ENCRYPT_TYPE_WPI: WAPI
 * @WLAN_ENCRYPT_TYPE_KRK: KRK
 * @WLAN_ENCRYPT_TYPE_BTK: BTK
 * @WLAN_ENCRYPT_TYPE_AES_CMAC: 11W BIP
 * @WLAN_ENCRYPT_TYPE_ANY: Any
 * @WLAN_NUM_OF_ENCRYPT_TYPE: Max value
 */
enum wlan_enc_type {
	WLAN_ENCRYPT_TYPE_NONE,
	WLAN_ENCRYPT_TYPE_WEP40_STATICKEY,
	WLAN_ENCRYPT_TYPE_WEP104_STATICKEY,
	WLAN_ENCRYPT_TYPE_WEP40,
	WLAN_ENCRYPT_TYPE_WEP104,
	WLAN_ENCRYPT_TYPE_TKIP,
	WLAN_ENCRYPT_TYPE_AES,
	WLAN_ENCRYPT_TYPE_WPI,
	WLAN_ENCRYPT_TYPE_KRK,
	WLAN_ENCRYPT_TYPE_BTK,
	WLAN_ENCRYPT_TYPE_AES_CMAC,
	WLAN_ENCRYPT_TYPE_ANY,
	WLAN_NUM_OF_ENCRYPT_TYPE = WLAN_ENCRYPT_TYPE_ANY,
};

/**
 * struct wlan_ssid - SSID info
 * @length: ssid length of bss excluding null
 * @ssid: ssid character array potentially non null terminated
 */
struct wlan_ssid {
	uint8_t length;
	uint8_t ssid[WLAN_SSID_MAX_LEN];
};

/* MAC address length */
#define WLAN_MACADDR_LEN 6
/* Util API to copy the MAC address */
#define WLAN_ADDR_COPY(dst, src)    qdf_mem_copy(dst, src, WLAN_MACADDR_LEN)
/* Util API to compare the MAC address */
#define WLAN_ADDR_EQ(a1, a2)   qdf_mem_cmp(a1, a2, WLAN_MACADDR_LEN)

#endif /* _WLAN_OBJMGR_CMN_H_*/
