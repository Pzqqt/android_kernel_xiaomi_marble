/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#define WLAN_UMAC_MAX_COMPONENTS WLAN_UMAC_COMP_ID_MAX

/* Max no. of radios, a pSoc/Device can support */
#ifdef WLAN_MAX_PDEVS
#define WLAN_UMAC_MAX_PDEVS WLAN_MAX_PDEVS
#else
#define WLAN_UMAC_MAX_PDEVS 3
#endif

/* Max no. of VDEV per PSOC */
#ifdef WLAN_PSOC_MAX_VDEVS
#define WLAN_UMAC_PSOC_MAX_VDEVS WLAN_PSOC_MAX_VDEVS
#else
#define WLAN_UMAC_PSOC_MAX_VDEVS 51
#endif

/* Max no. of VDEVs, a PDEV can support */
#ifdef WLAN_PDEV_MAX_VDEVS
#define WLAN_UMAC_PDEV_MAX_VDEVS WLAN_PDEV_MAX_VDEVS
#else
#define WLAN_UMAC_PDEV_MAX_VDEVS 17
#endif

/* Max no. of Peers, a device can support */
#define WLAN_UMAC_PSOC_MAX_PEERS (1536 + WLAN_UMAC_PSOC_MAX_VDEVS)

/* Max no. of Temporary Peers, a pdev can support */
#define WLAN_MAX_PDEV_TEMP_PEERS 128

/* Max no. of Temporary Peers, a psoc can support */
#define WLAN_MAX_PSOC_TEMP_PEERS \
		(WLAN_MAX_PDEV_TEMP_PEERS * WLAN_UMAC_MAX_PDEVS)

/* Max length of a SSID */
#define WLAN_SSID_MAX_LEN 32

#define WLAN_CACHE_ID_LEN 2

/* Max sequence number */
#define WLAN_MAX_SEQ_NUM 4096

/* Max no. of peers for STA vap */
#define WLAN_UMAC_MAX_STA_PEERS 2

/* Max vdev_id */
#define WLAN_UMAC_VDEV_ID_MAX 0xFF

/* TODO: MAX AID */
#define WLAN_UMAC_MAX_AID 2048

/* Invalid pdev_id */
#define WLAN_INVALID_PDEV_ID 0xFFFFFFFF

/* Invalid free descriptor count */
#define WLAN_INVALID_MGMT_DESC_COUNT 0xFFFFFFFF

/* Max fw report pdev id */
#define WLAN_UMAC_MAX_RP_PID 2

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
#define WLAN_MAC_EID_EXT        255

/* VHT capability flags */
/* B0-B1 Maximum MPDU Length */
/* A-MSDU Length 3839 octets */
#define WLAN_VHTCAP_MAX_MPDU_LEN_3839     0x00000000
 /* A-MSDU Length 7991 octets */
#define WLAN_VHTCAP_MAX_MPDU_LEN_7935     0x00000001
/* A-MSDU Length 11454 octets */
#define WLAN_VHTCAP_MAX_MPDU_LEN_11454    0x00000002

/* B2-B3 Supported Channel Width */
/* Does not support 160 or 80+80 */
#define WLAN_VHTCAP_SUP_CHAN_WIDTH_80     0x00000000
/* Supports 160 */
#define WLAN_VHTCAP_SUP_CHAN_WIDTH_160    0x00000004
/* Support both 160 or 80+80 */
#define WLAN_VHTCAP_SUP_CHAN_WIDTH_80_160 0x00000008
/* B2-B3 */
#define WLAN_VHTCAP_SUP_CHAN_WIDTH_S      2
#define WLAN_VHTCAP_SUP_CHAN_WIDTH_MASK   0x0000000C
/* B4 RX LDPC */
#define WLAN_VHTCAP_RX_LDPC             0x00000010
/* B5 Short GI for 80MHz */
#define WLAN_VHTCAP_SHORTGI_80          0x00000020
/* B6 Short GI for 160 and 80+80 MHz */
#define WLAN_VHTCAP_SHORTGI_160         0x00000040
/* B7 Tx STBC */
#define WLAN_VHTCAP_TX_STBC             0x00000080
#define WLAN_VHTCAP_TX_STBC_S           7
/* B8-B10 Rx STBC */
#define WLAN_VHTCAP_RX_STBC             0x00000700
#define WLAN_VHTCAP_RX_STBC_S           8
/* B11 SU Beam former capable */
#define WLAN_VHTCAP_SU_BFORMER          0x00000800
#define WLAN_VHTCAP_SU_BFORMER_S        11
/* B12 SU Beam formee capable */
#define WLAN_VHTCAP_SU_BFORMEE          0x00001000
#define WLAN_VHTCAP_SU_BFORMEE_S        12

/* B13-B15 Compressed steering number of beacomformer Antennas supported */
#define WLAN_VHTCAP_BF_MAX_ANT          0x0000E000
#define WLAN_VHTCAP_BF_MAX_ANT_S        13
/* B13-B15 Beamformee STS Capability */
#define WLAN_VHTCAP_STS_CAP_S           13
#define WLAN_VHTCAP_STS_CAP_M           0x7

/* B16-B18 Sounding Dimensions */
#define WLAN_VHTCAP_SOUND_DIM           0x00070000
#define WLAN_VHTCAP_SOUND_DIM_S         16
/* B19 MU Beam Former */
#define WLAN_VHTCAP_MU_BFORMER          0x00080000
#define WLAN_VHTCAP_MU_BFORMER_S        19
/* B20 MU Beam Formee */
#define WLAN_VHTCAP_MU_BFORMEE          0x00100000
#define WLAN_VHTCAP_MU_BFORMEE_S        20
/* B21 VHT TXOP PS */
#define WLAN_VHTCAP_TXOP_PS             0x00200000
/* B22 +HTC-VHT capable */
#define WLAN_VHTCAP_PLUS_HTC_VHT        0x00400000

#define WLAN_VHTCAP_MAX_AMPDU_LEN_FACTOR  13
/* B23-B25 maximum AMPDU Length Exponent */
#define WLAN_VHTCAP_MAX_AMPDU_LEN_EXP   0x03800000
#define WLAN_VHTCAP_MAX_AMPDU_LEN_EXP_S 23
/* B26-B27 VHT Link Adaptation capable */
#define WLAN_VHTCAP_LINK_ADAPT          0x0C000000
/* Rx Antenna Pattern Consistency Supported */
#define WLAN_VHTCAP_RX_ANTENNA_PATTERN  0x10000000
/* Tx Antenna Pattern Consistency Supported */
#define WLAN_VHTCAP_TX_ANTENNA_PATTERN  0x20000000
/* B30-B31 Extended NSS Bandwidth Support */
#define WLAN_VHTCAP_NO_EXT_NSS_BW_SUPPORT  0x00000000
/* B30-B31 Extended NSS Bandwidth Support */
#define WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_1   0x40000000
/* B30-B31 Extended NSS Bandwidth Support */
#define WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_2   0x80000000
/* B30-B31 Extended NSS Bandwidth Support */
#define WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_3   0xC0000000
#define WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_S   30
#define WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_MASK   0xC0000000

#define WLAN_VHTCAP_EXT_NSS_MASK   (WLAN_VHTCAP_SUP_CHAN_WIDTH_MASK |\
					 WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_MASK)
/* VHTCAP combinations of "supported channel width" and "ext nss support"
 * which determine the NSS value supported by STA for <=80 MHz, 160 MHz
 * and 80+80 MHz. The macros to be read as combination of
 * "supported channel width" and "ext nss support" followed by NSS for 80MHz,
 * 160MHz and 80+80MHz defined as a function of Max VHT NSS supported.
 * Ex: WLAN_EXTNSS_MAP_01_80F1_160FDOT5_80P80NONE - To be reas as
 * supported channel width = 0
 * ext nss support = 1
 * NSS value for <=80MHz = max_vht_nss * 1
 * NSS value for 160MHz = max_vht_nss * (.5)
 * NSS value for 80+80MHz = not supported
 */
#define WLAN_EXTNSS_MAP_00_80F1_160NONE_80P80NONE \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80 | WLAN_VHTCAP_NO_EXT_NSS_BW_SUPPORT)
#define WLAN_EXTNSS_MAP_01_80F1_160FDOT5_80P80NONE \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_1)
#define WLAN_EXTNSS_MAP_02_80F1_160FDOT5_80P80FDOT5 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_2)
#define WLAN_EXTNSS_MAP_03_80F1_160FDOT75_80P80FDOT75 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_3)
#define WLAN_EXTNSS_MAP_10_80F1_160F1_80P80NONE \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_160 | WLAN_VHTCAP_NO_EXT_NSS_BW_SUPPORT)
#define WLAN_EXTNSS_MAP_11_80F1_160F1_80P80FDOT5 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_160 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_1)
#define WLAN_EXTNSS_MAP_12_80F1_160F1_80P80FDOT75 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_160 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_2)
#define WLAN_EXTNSS_MAP_13_80F2_160F2_80P80F1 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_160 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_3)
#define WLAN_EXTNSS_MAP_20_80F1_160F1_80P80F1 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80_160 | WLAN_VHTCAP_NO_EXT_NSS_BW_SUPPORT)
#define WLAN_EXTNSS_MAP_23_80F2_160F1_80P80F1 \
	(WLAN_VHTCAP_SUP_CHAN_WIDTH_80_160 | WLAN_VHTCAP_EXT_NSS_BW_SUPPORT_3)


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
 * @WLAN_UMAC_COMP_SA_API:        Smart Antenna API
 * @WLAN_UMAC_COMP_REGULATORY:    REGULATORY
 * @WLAN_UMAC_COMP_CRYPTO:        CRYPTO
 * @WLAN_UMAC_COMP_NAN:           Neighbor Aware Networking
 * @WLAN_UMAC_COMP_DFS:           DFS
 * @WLAN_UMAC_COMP_SPECTRAL:      Spectral
 * @WLAN_UMAC_COMP_OFFCHAN_TXRX:  Offchan TxRx
 * @WLAN_UMAC_COMP_SON:           SON
 * @WLAN_UMAC_COMP_SPECTRAL:      Spectral
 * @WLAN_UMAC_COMP_SPLITMAC:      SplitMAC
 * @WLAN_UMAC_COMP_DISA:          DISA encryption test
 * @WLAN_UMAC_COMP_GREEN_AP:      Green AP
 * @WLAN_UMAC_COMP_FTM:           FTM component
 * @WLAN_UMAC_COMP_FD:            FILS Discovery
 * @WLAN_UMAC_COMP_OCB:           OCB
 * @WLAN_UMAC_COMP_IPA:           IPA
 * @WLAN_UMAC_COMP_CP_STATS:      Control Plane Statistics
 * @WLAN_UMAC_COMP_ACTION_OUI:    ACTION OUI
 * @WLAN_UMAC_COMP_FWOL           FW Offload
 * @WLAN_UMAC_COMP_INTEROP_ISSUES_AP       interop issues ap component
 * @WLAN_UMAC_COMP_BLACKLIST_MGR:      Blacklist mgr component
 * @WLAN_UMAC_COMP_COEX:          Coex config component
 * @WLAN_UMAC_COMP_FTM_TIME_SYNC: WLAN FTM TIMESYNC
 * @WLAN_UMAC_COMP_PKT_CAPTURE:   Packet capture component
 * @WLAN_UMAC_COMP_DCS:           DCS
 * @WLAN_IOT_SIM_COMP:            IOT Simulation component
 * @WLAN_UMAC_COMP_IF_MGR:        Interface manager component
 * @WLAN_UMAC_COMP_GPIO:          GPIO Configuration
 * @WLAN_UMAC_COMP_MLO_MGR:       MLO manager
 * @WLAN_UMAC_COMP_REPEATER:      Repeater component
 * @WLAN_UMAC_COMP_MBSS:          MBSS Framework
 * @WLAN_UMAC_COMP_WIFI_RADAR:    WIFI RADAR component
 * @WLAN_UMAC_COMP_TWT:           Target Wake Time (TWT) Component
 * @WLAN_UMAC_COMP_ID_MAX:        Maximum components in UMAC
 *
 * This id is static.
 * On Adding new component, new id has to be assigned
 */
enum wlan_umac_comp_id {
	WLAN_UMAC_COMP_MLME               = 0,
	WLAN_UMAC_COMP_MGMT_TXRX          = 1,
	WLAN_UMAC_COMP_SERIALIZATION      = 2,
	WLAN_UMAC_COMP_SCAN               = 3,
	WLAN_UMAC_COMP_PMO                = 4,
	WLAN_UMAC_COMP_P2P                = 5,
	WLAN_UMAC_COMP_POLICY_MGR         = 6,
	WLAN_UMAC_COMP_CONFIG             = 7,
	WLAN_TARGET_IF_COMP_DIRECT_BUF_RX = 8,
	WLAN_UMAC_COMP_WIFI_POS           = 9,
	WLAN_UMAC_COMP_TDLS               = 10,
	WLAN_UMAC_COMP_ATF                = 11,
	WLAN_UMAC_COMP_SA_API             = 12,
	WLAN_UMAC_COMP_REGULATORY         = 13,
	WLAN_UMAC_COMP_CRYPTO             = 14,
	WLAN_UMAC_COMP_NAN                = 15,
	WLAN_UMAC_COMP_DFS                = 16,
	WLAN_UMAC_COMP_OFFCHAN_TXRX       = 17,
	WLAN_UMAC_COMP_SON                = 18,
	WLAN_UMAC_COMP_SPECTRAL           = 19,
	WLAN_UMAC_COMP_SPLITMAC           = 20,
	WLAN_UMAC_COMP_DISA               = 21,
	WLAN_UMAC_COMP_GREEN_AP           = 22,
	WLAN_UMAC_COMP_FTM                = 23,
	WLAN_UMAC_COMP_FD                 = 24,
	WLAN_UMAC_COMP_OCB                = 25,
	WLAN_UMAC_COMP_IPA                = 26,
	WLAN_UMAC_COMP_CP_STATS           = 27,
	WLAN_UMAC_COMP_ACTION_OUI         = 28,
	WLAN_UMAC_COMP_FWOL               = 29,
	WLAN_UMAC_COMP_CFR                = 30,
	WLAN_UMAC_COMP_INTEROP_ISSUES_AP  = 31,
	WLAN_UMAC_COMP_BLACKLIST_MGR      = 32,
	WLAN_UMAC_COMP_COEX               = 33,
	WLAN_UMAC_COMP_FTM_TIME_SYNC      = 34,
	WLAN_UMAC_COMP_PKT_CAPTURE        = 35,
	WLAN_UMAC_COMP_DCS                = 36,
	WLAN_IOT_SIM_COMP                 = 37,
	WLAN_UMAC_COMP_IF_MGR             = 38,
	WLAN_UMAC_COMP_GPIO               = 39,
	WLAN_UMAC_COMP_MLO_MGR            = 40,
	WLAN_UMAC_COMP_REPEATER           = 41,
	WLAN_UMAC_COMP_MBSS               = 42,
	WLAN_UMAC_COMP_WIFI_RADAR         = 43,
	WLAN_UMAC_COMP_TWT                = 44,
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
 * enum wlan_phymode - phy mode
 * @WLAN_PHYMODE_AUTO:           autoselect
 * @WLAN_PHYMODE_11A:            5GHz, OFDM
 * @WLAN_PHYMODE_11B:            2GHz, CCK
 * @WLAN_PHYMODE_11G:            2GHz, OFDM
 * @WLAN_PHYMODE_11G_ONLY:       2GHz only
 * @WLAN_PHYMODE_11NA_HT20:      5Ghz, HT20
 * @WLAN_PHYMODE_11NG_HT20:      2Ghz, HT20
 * @WLAN_PHYMODE_11NA_HT40:      5Ghz, Auto HT40
 * @WLAN_PHYMODE_11NG_HT40PLUS:  2Ghz, HT40 (ext ch +1)
 * @WLAN_PHYMODE_11NG_HT40MINUS: 2Ghz, HT40 (ext ch -1)
 * @WLAN_PHYMODE_11NG_HT40:      2Ghz, Auto HT40
 * @WLAN_PHYMODE_11AC_VHT20:     5Ghz, VHT20
 * @WLAN_PHYMODE_11AC_VHT20_2G:  2Ghz, VHT20
 * @WLAN_PHYMODE_11AC_VHT40:     5Ghz, VHT40
 * @WLAN_PHYMODE_11AC_VHT40PLUS_2G:  2Ghz, VHT40 (ext ch +1)
 * @WLAN_PHYMODE_11AC_VHT40MINUS_2G:  2Ghz, VHT40 (ext ch -1)
 * @WLAN_PHYMODE_11AC_VHT40_2G:  2Ghz, VHT40
 * @WLAN_PHYMODE_11AC_VHT80:     5Ghz, VHT80
 * @WLAN_PHYMODE_11AC_VHT80_2G:  2Ghz, VHT80
 * @WLAN_PHYMODE_11AC_VHT160:    5Ghz, VHT160
 * @WLAN_PHYMODE_11AC_VHT80_80:  5Ghz, VHT80_80
 * @WLAN_PHYMODE_11AXA_HE20:     5GHz, HE20
 * @WLAN_PHYMODE_11AXG_HE20:     2GHz, HE20
 * @WLAN_PHYMODE_11AXA_HE40:     5GHz, HE40
 * @WLAN_PHYMODE_11AXG_HE40PLUS: 2GHz, HE40 (ext ch +1)
 * @WLAN_PHYMODE_11AXG_HE40MINUS:2GHz, HE40 (ext ch -1)
 * @WLAN_PHYMODE_11AXG_HE40:     2GHz, HE40
 * @WLAN_PHYMODE_11AXA_HE80:     5GHz, HE80
 * @WLAN_PHYMODE_11AXG_HE80:     2GHz, HE80
 * @WLAN_PHYMODE_11AXA_HE160:    5GHz, HE160
 * @WLAN_PHYMODE_11AXA_HE80_80:  5GHz, HE80_80
 * @WLAN_PHYMODE_11BEA_EHT20:     5GHz, EHT20
 * @WLAN_PHYMODE_11BEG_EHT20:     2GHz, EHT20
 * @WLAN_PHYMODE_11BEA_EHT40:     5GHz, EHT40
 * @WLAN_PHYMODE_11BEG_EHT40PLUS: 2GHz, EHT40 (ext ch +1)
 * @WLAN_PHYMODE_11BEG_EHT40MINUS:2GHz, EHT40 (ext ch -1)
 * @WLAN_PHYMODE_11BEG_EHT40:     2GHz, EHT40
 * @WLAN_PHYMODE_11BEA_EHT80:     5GHz, EHT80
 * @WLAN_PHYMODE_11BEG_EHT80:     2GHz, EHT80
 * @WLAN_PHYMODE_11BEA_EHT160:    5GHz, EHT160
 * @WLAN_PHYMODE_11BEA_EHT320:    5GHz, EHT320
 * @WLAN_PHYMODE_MAX: Max phymode
 */
enum wlan_phymode {
	WLAN_PHYMODE_AUTO               = 0,
	WLAN_PHYMODE_11A                = 1,
	WLAN_PHYMODE_11B                = 2,
	WLAN_PHYMODE_11G                = 3,
	WLAN_PHYMODE_11G_ONLY           = 4,
	WLAN_PHYMODE_11NA_HT20          = 5,
	WLAN_PHYMODE_11NG_HT20          = 6,
	WLAN_PHYMODE_11NA_HT40          = 7,
	WLAN_PHYMODE_11NG_HT40PLUS      = 8,
	WLAN_PHYMODE_11NG_HT40MINUS     = 9,
	WLAN_PHYMODE_11NG_HT40          = 10,
	WLAN_PHYMODE_11AC_VHT20         = 11,
	WLAN_PHYMODE_11AC_VHT20_2G      = 12,
	WLAN_PHYMODE_11AC_VHT40         = 13,
	WLAN_PHYMODE_11AC_VHT40PLUS_2G  = 14,
	WLAN_PHYMODE_11AC_VHT40MINUS_2G = 15,
	WLAN_PHYMODE_11AC_VHT40_2G      = 16,
	WLAN_PHYMODE_11AC_VHT80         = 17,
	WLAN_PHYMODE_11AC_VHT80_2G      = 18,
	WLAN_PHYMODE_11AC_VHT160        = 19,
	WLAN_PHYMODE_11AC_VHT80_80      = 20,
	WLAN_PHYMODE_11AXA_HE20         = 21,
	WLAN_PHYMODE_11AXG_HE20         = 22,
	WLAN_PHYMODE_11AXA_HE40         = 23,
	WLAN_PHYMODE_11AXG_HE40PLUS     = 24,
	WLAN_PHYMODE_11AXG_HE40MINUS    = 25,
	WLAN_PHYMODE_11AXG_HE40         = 26,
	WLAN_PHYMODE_11AXA_HE80         = 27,
	WLAN_PHYMODE_11AXG_HE80         = 28,
	WLAN_PHYMODE_11AXA_HE160        = 29,
	WLAN_PHYMODE_11AXA_HE80_80      = 30,
#ifdef WLAN_FEATURE_11BE
	WLAN_PHYMODE_11BEA_EHT20        = 31,
	WLAN_PHYMODE_11BEG_EHT20        = 32,
	WLAN_PHYMODE_11BEA_EHT40        = 33,
	WLAN_PHYMODE_11BEG_EHT40PLUS    = 34,
	WLAN_PHYMODE_11BEG_EHT40MINUS   = 35,
	WLAN_PHYMODE_11BEG_EHT40        = 36,
	WLAN_PHYMODE_11BEA_EHT80        = 37,
	WLAN_PHYMODE_11BEG_EHT80        = 38,
	WLAN_PHYMODE_11BEA_EHT160       = 39,
	WLAN_PHYMODE_11BEA_EHT320       = 40,
#endif
	WLAN_PHYMODE_MAX
};

#ifdef WLAN_FEATURE_11BE
#define IS_WLAN_PHYMODE_320MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11BEA_EHT320); })

#define IS_WLAN_PHYMODE_160MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11BEA_EHT160) || \
	((mode) == WLAN_PHYMODE_11AC_VHT80_80) || \
	((mode) == WLAN_PHYMODE_11AC_VHT160)     || \
	((mode) == WLAN_PHYMODE_11AXA_HE80_80)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE160); })

#define IS_WLAN_PHYMODE_80MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11BEA_EHT80) || \
	((mode) == WLAN_PHYMODE_11AC_VHT80) || \
	((mode) == WLAN_PHYMODE_11AC_VHT80_2G)     || \
	((mode) == WLAN_PHYMODE_11AXA_HE80)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE80); })

#define IS_WLAN_PHYMODE_40MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11NG_HT40) || \
	((mode) == WLAN_PHYMODE_11NG_HT40PLUS)     || \
	((mode) == WLAN_PHYMODE_11NG_HT40MINUS)  || \
	((mode) == WLAN_PHYMODE_11NA_HT40)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40PLUS_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40MINUS_2G)  || \
	((mode) == WLAN_PHYMODE_11BEA_EHT40)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40PLUS)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40MINUS)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40PLUS)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40MINUS); })

#define IS_WLAN_PHYMODE_EHT(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11BEA_EHT20) || \
	((mode) == WLAN_PHYMODE_11BEG_EHT20)     || \
	((mode) == WLAN_PHYMODE_11BEA_EHT40)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40PLUS)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT40MINUS)  || \
	((mode) == WLAN_PHYMODE_11BEA_EHT80)  || \
	((mode) == WLAN_PHYMODE_11BEG_EHT80)  || \
	((mode) == WLAN_PHYMODE_11BEA_EHT160)  || \
	((mode) == WLAN_PHYMODE_11BEA_EHT320); })

#else

#define IS_WLAN_PHYMODE_160MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11AC_VHT80_80) || \
	((mode) == WLAN_PHYMODE_11AC_VHT160)     || \
	((mode) == WLAN_PHYMODE_11AXA_HE80_80)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE160); })

#define IS_WLAN_PHYMODE_80MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11AC_VHT80) || \
	((mode) == WLAN_PHYMODE_11AC_VHT80_2G)     || \
	((mode) == WLAN_PHYMODE_11AXA_HE80)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE80); })

#define IS_WLAN_PHYMODE_40MHZ(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11NG_HT40) || \
	((mode) == WLAN_PHYMODE_11NG_HT40PLUS)     || \
	((mode) == WLAN_PHYMODE_11NG_HT40MINUS)  || \
	((mode) == WLAN_PHYMODE_11NA_HT40)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40PLUS_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40MINUS_2G)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40PLUS)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40MINUS); })
#endif

#define IS_WLAN_PHYMODE_HT(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11NA_HT20) || \
	((mode) == WLAN_PHYMODE_11NG_HT20)     || \
	((mode) == WLAN_PHYMODE_11NA_HT40)  || \
	((mode) == WLAN_PHYMODE_11NG_HT40PLUS)  || \
	((mode) == WLAN_PHYMODE_11NG_HT40MINUS)  || \
	((mode) == WLAN_PHYMODE_11NG_HT40); })

#define IS_WLAN_PHYMODE_VHT(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11AC_VHT20) || \
	((mode) == WLAN_PHYMODE_11AC_VHT20_2G)     || \
	((mode) == WLAN_PHYMODE_11AC_VHT40)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40PLUS_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40MINUS_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT40_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT80)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT80_2G)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT160)  || \
	((mode) == WLAN_PHYMODE_11AC_VHT80_80); })

#define IS_WLAN_PHYMODE_HE(_mode) ({typeof(_mode) mode = (_mode); \
	((mode) == WLAN_PHYMODE_11AXA_HE20) || \
	((mode) == WLAN_PHYMODE_11AXG_HE20)     || \
	((mode) == WLAN_PHYMODE_11AXA_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40PLUS)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE40MINUS)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE80)  || \
	((mode) == WLAN_PHYMODE_11AXG_HE80)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE160)  || \
	((mode) == WLAN_PHYMODE_11AXA_HE80_80); })

/**
 * enum phy_ch_width - channel width
 * @CH_WIDTH_20MHZ: 20 mhz width
 * @CH_WIDTH_40MHZ: 40 mhz width
 * @CH_WIDTH_80MHZ: 80 mhz width
 * @CH_WIDTH_160MHZ: 160 mhz width
 * @CH_WIDTH_80P80HZ: 80+80 mhz width
 * @CH_WIDTH_5MHZ: 5 mhz width
 * @CH_WIDTH_10MHZ: 10 mhz width
 * @CH_WIDTH_320MHZ: 320 mhz width
 * @CH_WIDTH_INVALID: invalid width
 * @CH_WIDTH_MAX: max possible width
 */
enum phy_ch_width {
	CH_WIDTH_20MHZ = 0,
	CH_WIDTH_40MHZ,
	CH_WIDTH_80MHZ,
	CH_WIDTH_160MHZ,
	CH_WIDTH_80P80MHZ,
	CH_WIDTH_5MHZ,
	CH_WIDTH_10MHZ,
#ifdef WLAN_FEATURE_11BE
	CH_WIDTH_320MHZ,
#endif
	CH_WIDTH_INVALID,
	CH_WIDTH_MAX
};

/**
 * enum wifi_traffic_ac - access category type
 * @WIFI_AC_VO: Voice AC
 * @WIFI_AC_VI: Video AC
 * @WIFI_AC_BE: Best effort AC
 * @WIFI_AC_BK: Background AC
 * @WIFI_AC_MAX: MAX access category
 */
enum wifi_traffic_ac {
	WIFI_AC_VO = 0,
	WIFI_AC_VI = 1,
	WIFI_AC_BE = 2,
	WIFI_AC_BK = 3,
	WIFI_AC_MAX = 4,
};

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
 * @WLAN_PEER_NDP:      NDP Peer
 * @WLAN_PEER_MLO_TEMP: MLO Peer Temp (host only node)
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
	WLAN_PEER_NDP      = 10,
	WLAN_PEER_MLO_TEMP = 11,
};

/**
 * enum wlan_band - specifies operating channel band
 * @WLAN_BAND_ALL: Any band
 * @WLAN_BAND_2_4_GHZ: 2.4 GHz band
 * @WLAN_BAND_5_GHZ: 5 GHz band
 * @WLAN_BAND_4_9_GHZ: 4.9 GHz band
 * @WLAN_BAND_NUM_MAX: Max num band
 */
enum wlan_band {
	WLAN_BAND_ALL,
	WLAN_BAND_2_4_GHZ,
	WLAN_BAND_5_GHZ,
	WLAN_BAND_4_9_GHZ,
	WLAN_BAND_NUM_MAX,
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
 * struct wlan_ssid - SSID info
 * @length: ssid length of bss excluding null
 * @ssid: ssid character array potentially non null terminated
 */
struct wlan_ssid {
	uint8_t length;
	uint8_t ssid[WLAN_SSID_MAX_LEN];
};

/* Util API to copy the MAC address */
#define WLAN_ADDR_COPY(dst, src)    qdf_mem_copy(dst, src, QDF_MAC_ADDR_SIZE)
/* Util API to compare the MAC address */
#define WLAN_ADDR_EQ(a1, a2)   qdf_mem_cmp(a1, a2, QDF_MAC_ADDR_SIZE)

#define PSOC_SERVICE_BM_SIZE ((128 + sizeof(uint32_t) - 1) / sizeof(uint32_t))
#define PSOC_HOST_MAX_NUM_SS (8)
#define PSOC_HOST_MAX_PHY_SIZE (3)
#define PSOC_HOST_MAX_MAC_SIZE (2)
#define PSOC_MAX_HW_MODE (3)
#define PSOC_MAX_MAC_PHY_CAP (5)
#define PSOC_MAX_PHY_REG_CAP (3)
#define PSOC_MAX_CHAINMASK_TABLES (5)

#ifdef WLAN_FEATURE_11BE
#define PSOC_HOST_MAX_EHT_MAC_SIZE 1
#define PSOC_HOST_MAX_EHT_PHY_SIZE 2
#define PSOC_HOST_EHT_MCS_NSS_MAP_2G_SIZE 2
#define PSOC_HOST_EHT_MCS_NSS_MAP_5G_SIZE 4
#endif

#endif /* _WLAN_OBJMGR_CMN_H_*/
