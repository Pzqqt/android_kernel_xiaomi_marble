/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _CFG80211_VEN_CMD_
#define _CFG80211_VEN_CMD_

#ifdef CONFIG_SUPPORT_LIBROXML
#include <ieee80211_external_config.h>
#endif

/*
 * vendor_commands: Structure to maintain vendor command
 * @cmd_value: Corresponding macro for command
 * @cmd_id: Flag to determine if command is set or get
 * @n_args: Number of arguments this command takes
 */
struct vendor_commands {
	char *cmd_name;
	unsigned cmd_value;
	unsigned cmd_id;
	unsigned n_args;
};

#define SET_PARAM 74
#define GET_PARAM 75

#define OL_ATH_PARAM_SHIFT     0x1000
#define OL_SPECIAL_PARAM_SHIFT 0x2000

enum {
	IEEE80211_PARAM_TURBO        = 1,    /* turbo mode */
	IEEE80211_PARAM_MODE        = 2,    /* phy mode (11a, 11b, etc.) */
	IEEE80211_PARAM_AUTHMODE    = 3,    /* authentication mode */
	IEEE80211_PARAM_PROTMODE    = 4,    /* 802.11g protection */
	IEEE80211_PARAM_MCASTCIPHER    = 5,    /* multicast/default cipher */
	IEEE80211_PARAM_MCASTKEYLEN    = 6,    /* multicast key length */
	IEEE80211_PARAM_UCASTCIPHERS    = 7,    /* unicast cipher suites */
	IEEE80211_PARAM_UCASTCIPHER    = 8,    /* unicast cipher */
	IEEE80211_PARAM_UCASTKEYLEN    = 9,    /* unicast key length */
	IEEE80211_PARAM_WPA        = 10,    /* WPA mode (0,1,2) */
	IEEE80211_PARAM_ROAMING        = 12,    /* roaming mode */
	IEEE80211_PARAM_PRIVACY        = 13,    /* privacy invoked */
	IEEE80211_PARAM_COUNTERMEASURES    = 14,    /* WPA/TKIP countermeasures */
	IEEE80211_PARAM_DROPUNENCRYPTED    = 15,    /* discard unencrypted frames */
	IEEE80211_PARAM_DRIVER_CAPS    = 16,    /* driver capabilities */
	IEEE80211_PARAM_MACCMD        = 17,    /* MAC ACL operation */
	IEEE80211_PARAM_WMM        = 18,    /* WMM mode (on, off) */
	IEEE80211_PARAM_HIDESSID    = 19,    /* hide SSID mode (on, off) */
	IEEE80211_PARAM_APBRIDGE    = 20,    /* AP inter-sta bridging */
	IEEE80211_PARAM_KEYMGTALGS    = 21,    /* key management algorithms */
	IEEE80211_PARAM_RSNCAPS        = 22,    /* RSN capabilities */
	IEEE80211_PARAM_INACT        = 23,    /* station inactivity timeout */
	IEEE80211_PARAM_INACT_AUTH    = 24,    /* station auth inact timeout */
	IEEE80211_PARAM_INACT_INIT    = 25,    /* station init inact timeout */
	IEEE80211_PARAM_DTIM_PERIOD    = 28,    /* DTIM period (beacons) */
	IEEE80211_PARAM_BEACON_INTERVAL    = 29,    /* beacon interval (ms) */
	IEEE80211_PARAM_DOTH        = 30,    /* 11.h is on/off */
	IEEE80211_PARAM_PWRTARGET    = 31,    /* Current Channel Pwr Constraint */
	IEEE80211_PARAM_GENREASSOC    = 32,    /* Generate a reassociation request */
	IEEE80211_PARAM_COMPRESSION    = 33,    /* compression */
	IEEE80211_PARAM_FF        = 34,    /* fast frames support */
	IEEE80211_PARAM_XR        = 35,    /* XR support */
	IEEE80211_PARAM_BURST        = 36,    /* burst mode */
	IEEE80211_PARAM_PUREG        = 37,    /* pure 11g (no 11b stations) */
	IEEE80211_PARAM_AR        = 38,    /* AR support */
	IEEE80211_PARAM_WDS        = 39,    /* Enable 4 address processing */
	IEEE80211_PARAM_BGSCAN        = 40,    /* bg scanning (on, off) */
	IEEE80211_PARAM_BGSCAN_IDLE    = 41,    /* bg scan idle threshold */
	IEEE80211_PARAM_BGSCAN_INTERVAL    = 42,    /* bg scan interval */
	IEEE80211_PARAM_MCAST_RATE    = 43,    /* Multicast Tx Rate */
	IEEE80211_PARAM_COVERAGE_CLASS    = 44,    /* coverage class */
	IEEE80211_PARAM_COUNTRY_IE    = 45,    /* enable country IE */
	IEEE80211_PARAM_SCANVALID    = 46,    /* scan cache valid threshold */
	IEEE80211_PARAM_ROAM_RSSI_11A    = 47,    /* rssi threshold in 11a */
	IEEE80211_PARAM_ROAM_RSSI_11B    = 48,    /* rssi threshold in 11b */
	IEEE80211_PARAM_ROAM_RSSI_11G    = 49,    /* rssi threshold in 11g */
	IEEE80211_PARAM_ROAM_RATE_11A    = 50,    /* tx rate threshold in 11a */
	IEEE80211_PARAM_ROAM_RATE_11B    = 51,    /* tx rate threshold in 11b */
	IEEE80211_PARAM_ROAM_RATE_11G    = 52,    /* tx rate threshold in 11g */
	IEEE80211_PARAM_UAPSDINFO    = 53,    /* value for qos info field */
	IEEE80211_PARAM_SLEEP        = 54,    /* force sleep/wake */
	IEEE80211_PARAM_QOSNULL        = 55,    /* force sleep/wake */
	IEEE80211_PARAM_PSPOLL        = 56,    /* force ps-poll generation (sta only) */
	IEEE80211_PARAM_EOSPDROP    = 57,    /* force uapsd EOSP drop (ap only) */
	IEEE80211_PARAM_REGCLASS    = 59,    /* enable regclass ids in country IE */
	IEEE80211_PARAM_CHANBW        = 60,    /* set chan bandwidth preference */
	IEEE80211_PARAM_WMM_AGGRMODE    = 61,    /* set WMM Aggressive Mode */
	IEEE80211_PARAM_SHORTPREAMBLE    = 62,     /* enable/disable short Preamble */
	IEEE80211_PARAM_CWM_MODE    = 64,    /* CWM mode */
	IEEE80211_PARAM_CWM_EXTOFFSET    = 65,    /* CWM extension channel offset */
	IEEE80211_PARAM_CWM_EXTPROTMODE    = 66,    /* CWM extension channel protection mode */
	IEEE80211_PARAM_CWM_EXTPROTSPACING = 67,/* CWM extension channel protection spacing */
	IEEE80211_PARAM_CWM_ENABLE    = 68,/* CWM state machine enabled */
	IEEE80211_PARAM_CWM_EXTBUSYTHRESHOLD = 69,/* CWM extension channel busy threshold */
	IEEE80211_PARAM_CWM_CHWIDTH    = 70,    /* CWM STATE: current channel width */
	IEEE80211_PARAM_SHORT_GI    = 71,    /* half GI */
	IEEE80211_PARAM_FAST_CC        = 72,    /* fast channel change */

	/*
	 * 11n A-MPDU, A-MSDU support
	 */
	IEEE80211_PARAM_AMPDU        = 73,    /* 11n a-mpdu support */
	IEEE80211_PARAM_AMPDU_LIMIT    = 74,    /* a-mpdu length limit */
	IEEE80211_PARAM_AMPDU_DENSITY    = 75,    /* a-mpdu density */
	IEEE80211_PARAM_AMPDU_SUBFRAMES    = 76,    /* a-mpdu subframe limit */
	IEEE80211_PARAM_AMSDU        = 77,    /* a-msdu support */
	IEEE80211_PARAM_AMSDU_LIMIT    = 78,    /* a-msdu length limit */

	IEEE80211_PARAM_COUNTRYCODE    = 79,    /* Get country code */
	IEEE80211_PARAM_TX_CHAINMASK    = 80,    /* Tx chain mask */
	IEEE80211_PARAM_RX_CHAINMASK    = 81,    /* Rx chain mask */
	IEEE80211_PARAM_RTSCTS_RATECODE    = 82,    /* RTS Rate code */
	IEEE80211_PARAM_HT_PROTECTION    = 83,    /* Protect traffic in HT mode */
	IEEE80211_PARAM_RESET_ONCE    = 84,    /* Force a reset */
	IEEE80211_PARAM_SETADDBAOPER    = 85,    /* Set ADDBA mode */
	IEEE80211_PARAM_TX_CHAINMASK_LEGACY = 86, /* Tx chain mask for legacy clients */
	IEEE80211_PARAM_11N_RATE    = 87,    /* Set ADDBA mode */
	IEEE80211_PARAM_11N_RETRIES    = 88,    /* Tx chain mask for legacy clients */
	IEEE80211_PARAM_DBG_LVL        = 89,    /* Debug Level for specific VAP */
	IEEE80211_PARAM_WDS_AUTODETECT    = 90,    /* Configurable Auto Detect/Delba for WDS mode */
	IEEE80211_PARAM_ATH_RADIO    = 91,    /* returns the name of the radio being used */
	IEEE80211_PARAM_IGNORE_11DBEACON = 92,    /* Don't process 11d beacon (on, off) */
	IEEE80211_PARAM_STA_FORWARD    = 93,    /* Enable client 3 addr forwarding */

	/*
	 * Mcast Enhancement support
	 */
	IEEE80211_PARAM_ME          = 94,   /* Set Mcast enhancement option: 0 disable, 1 tunneling, 2 translate  4 to disable snoop feature*/
	IEEE80211_PARAM_PUREN        = 100,    /* pure 11n (no 11bg/11a stations) */
	IEEE80211_PARAM_BASICRATES    = 101,    /* Change Basic Rates */
	IEEE80211_PARAM_NO_EDGE_CH    = 102,    /* Avoid band edge channels */
	IEEE80211_PARAM_WEP_TKIP_HT    = 103,    /* Enable HT rates with WEP/TKIP encryption */
	IEEE80211_PARAM_RADIO        = 104,    /* radio on/off */
	IEEE80211_PARAM_NETWORK_SLEEP    = 105,    /* set network sleep enable/disable */
	IEEE80211_PARAM_DROPUNENC_EAPOL    = 106,

	/*
	 * Unassociated power consumpion improve
	 */
	IEEE80211_PARAM_SLEEP_PRE_SCAN    = 109,
	IEEE80211_PARAM_SCAN_PRE_SLEEP    = 110,

	/* support for wapi: set auth mode and key */
	IEEE80211_PARAM_SETWAPI        = 112,
#if WLAN_SUPPORT_GREEN_AP
	IEEE80211_IOCTL_GREEN_AP_PS_ENABLE = 113,
	IEEE80211_IOCTL_GREEN_AP_PS_TIMEOUT = 114,
#endif
	IEEE80211_PARAM_WPS        = 116,
	IEEE80211_PARAM_RX_RATE        = 117,
	IEEE80211_PARAM_CHEXTOFFSET    = 118,
	IEEE80211_PARAM_CHSCANINIT    = 119,
	IEEE80211_PARAM_MPDU_SPACING    = 120,
	IEEE80211_PARAM_HT40_INTOLERANT    = 121,
	IEEE80211_PARAM_CHWIDTH        = 122,
	IEEE80211_PARAM_EXTAP        = 123,   /* Enable client 3 addr forwarding */
	IEEE80211_PARAM_COEXT_DISABLE    = 124,
	IEEE80211_PARAM_GETIQUECONFIG = 129, /*print out the iQUE config*/
	IEEE80211_PARAM_CCMPSW_ENCDEC = 130,  /* support for ccmp s/w encrypt decrypt */

	/* Support for repeater placement */
	IEEE80211_PARAM_CUSTPROTO_ENABLE = 131,
	IEEE80211_PARAM_GPUTCALC_ENABLE  = 132,
	IEEE80211_PARAM_DEVUP            = 133,
	IEEE80211_PARAM_MACDEV           = 134,
	IEEE80211_PARAM_MACADDR1         = 135,
	IEEE80211_PARAM_MACADDR2         = 136,
	IEEE80211_PARAM_GPUTMODE         = 137,
	IEEE80211_PARAM_TXPROTOMSG       = 138,
	IEEE80211_PARAM_RXPROTOMSG       = 139,
	IEEE80211_PARAM_STATUS           = 140,
	IEEE80211_PARAM_ASSOC            = 141,
	IEEE80211_PARAM_NUMSTAS          = 142,
	IEEE80211_PARAM_STA1ROUTE        = 143,
	IEEE80211_PARAM_STA2ROUTE        = 144,
	IEEE80211_PARAM_STA3ROUTE        = 145,
	IEEE80211_PARAM_STA4ROUTE        = 146,
	IEEE80211_PARAM_PERIODIC_SCAN = 179,
#if ATH_SUPPORT_AP_WDS_COMBO
	IEEE80211_PARAM_NO_BEACON     = 180,  /* No beacon xmit on VAP */
#endif
	IEEE80211_PARAM_VAP_COUNTRY_IE   = 181, /* 802.11d country ie per vap */
	IEEE80211_PARAM_VAP_DOTH         = 182, /* 802.11h per vap */
	IEEE80211_PARAM_STA_QUICKKICKOUT = 183, /* station quick kick out */
	IEEE80211_PARAM_AUTO_ASSOC       = 184,
	IEEE80211_PARAM_RXBUF_LIFETIME   = 185, /* lifetime of reycled rx buffers */
	IEEE80211_PARAM_2G_CSA           = 186, /* 2.4 GHz CSA is on/off */
	IEEE80211_PARAM_WAPIREKEY_USK = 187,
	IEEE80211_PARAM_WAPIREKEY_MSK = 188,
	IEEE80211_PARAM_WAPIREKEY_UPDATE = 189,
#if ATH_SUPPORT_IQUE
	IEEE80211_PARAM_RC_VIVO          = 190, /* Use separate rate control algorithm for VI/VO queues */
#endif
	IEEE80211_PARAM_CLR_APPOPT_IE    = 191,  /* Clear Cached App/OptIE */
	IEEE80211_PARAM_SW_WOW           = 192,   /* wow by sw */
	IEEE80211_PARAM_QUIET_PERIOD    = 193,
	IEEE80211_PARAM_QBSS_LOAD       = 194,
	IEEE80211_PARAM_RRM_CAP         = 195,
	IEEE80211_PARAM_WNM_CAP         = 196,
	IEEE80211_PARAM_ADD_WDS_ADDR    = 197,  /* add wds addr */
#ifdef QCA_PARTNER_PLATFORM
	IEEE80211_PARAM_PLTFRM_PRIVATE = 198, /* platfrom's private ioctl*/
#endif

#if UMAC_SUPPORT_VI_DBG
	/* Support for Video Debug */
	IEEE80211_PARAM_DBG_CFG            = 199,
	IEEE80211_PARAM_DBG_NUM_STREAMS    = 200,
	IEEE80211_PARAM_STREAM_NUM         = 201,
	IEEE80211_PARAM_DBG_NUM_MARKERS    = 202,
	IEEE80211_PARAM_MARKER_NUM         = 203,
	IEEE80211_PARAM_MARKER_OFFSET_SIZE = 204,
	IEEE80211_PARAM_MARKER_MATCH       = 205,
	IEEE80211_PARAM_RXSEQ_OFFSET_SIZE  = 206,
	IEEE80211_PARAM_RX_SEQ_RSHIFT      = 207,
	IEEE80211_PARAM_RX_SEQ_MAX         = 208,
	IEEE80211_PARAM_RX_SEQ_DROP        = 209,
	IEEE80211_PARAM_TIME_OFFSET_SIZE   = 210,
	IEEE80211_PARAM_RESTART            = 211,
	IEEE80211_PARAM_RXDROP_STATUS      = 212,
#endif
#if ATH_SUPPORT_IBSS_DFS
	IEEE80211_PARAM_IBSS_DFS_PARAM     = 225,
#endif
#ifdef ATH_SUPPORT_TxBF
	IEEE80211_PARAM_TXBF_AUTO_CVUPDATE = 240,       /* Auto CV update enable*/
	IEEE80211_PARAM_TXBF_CVUPDATE_PER = 241,        /* per theshold to initial CV update*/
#endif
	IEEE80211_PARAM_MAXSTA              = 242,
	IEEE80211_PARAM_RRM_STATS           = 243,      /* RRM Stats */
	IEEE80211_PARAM_RRM_SLWINDOW        = 244,
	IEEE80211_PARAM_MFP_TEST    = 245,
	IEEE80211_PARAM_SCAN_BAND   = 246,                /* only scan channels of requested band */
#if ATH_SUPPORT_FLOWMAC_MODULE
	IEEE80211_PARAM_FLOWMAC            = 247, /* flowmac enable/disable ath0*/
#endif
	IEEE80211_PARAM_STA_PWR_SET_PSPOLL      = 255,  /* Set ips_use_pspoll flag for STA */
	IEEE80211_PARAM_NO_STOP_DISASSOC        = 256,  /* Do not send disassociation frame on stopping vap */
	IEEE80211_PARAM_CHAN_UTIL_ENAB      = 260,
	IEEE80211_PARAM_CHAN_UTIL           = 261,      /* Get Channel Utilization value (scale: 0 - 255) */
	IEEE80211_PARAM_DBG_LVL_HIGH        = 262, /* Debug Level for specific VAP (upper 32 bits) */
	IEEE80211_PARAM_PROXYARP_CAP        = 263, /* Enable WNM Proxy ARP feature */
	IEEE80211_PARAM_DGAF_DISABLE        = 264, /* Hotspot 2.0 DGAF Disable feature */
	IEEE80211_PARAM_L2TIF_CAP           = 265, /* Hotspot 2.0 L2 Traffic Inspection and Filtering */
	IEEE80211_PARAM_WEATHER_RADAR_CHANNEL = 266, /* weather radar channel selection is bypassed */
	IEEE80211_PARAM_SEND_DEAUTH           = 267,/* for sending deauth while doing interface down*/
	IEEE80211_PARAM_WEP_KEYCACHE          = 268,/* wepkeys mustbe in first fourslots in Keycache*/
#if ATH_SUPPORT_WPA_SUPPLICANT_CHECK_TIME
	IEEE80211_PARAM_REJOINT_ATTEMP_TIME   = 269, /* Set the Rejoint time */
#endif
	IEEE80211_PARAM_WNM_SLEEP           = 270,      /* WNM-Sleep Mode */
	IEEE80211_PARAM_WNM_BSS_CAP         = 271,
	IEEE80211_PARAM_WNM_TFS_CAP         = 272,
	IEEE80211_PARAM_WNM_TIM_CAP         = 273,
	IEEE80211_PARAM_WNM_SLEEP_CAP       = 274,
	IEEE80211_PARAM_WNM_FMS_CAP         = 275,
	IEEE80211_PARAM_RRM_DEBUG           = 276, /* RRM debugging parameter */
	IEEE80211_PARAM_SET_TXPWRADJUST     = 277,
	IEEE80211_PARAM_TXRX_DBG              = 278,    /* show txrx debug info */
	IEEE80211_PARAM_VHT_MCS               = 279,    /* VHT MCS set */
	IEEE80211_PARAM_TXRX_FW_STATS         = 280,    /* single FW stat */
	IEEE80211_PARAM_TXRX_FW_MSTATS        = 281,    /* multiple FW stats */
	IEEE80211_PARAM_NSS                   = 282,    /* Number of Spatial Streams */
	IEEE80211_PARAM_LDPC                  = 283,    /* Support LDPC */
	IEEE80211_PARAM_TX_STBC               = 284,    /* Support TX STBC */
	IEEE80211_PARAM_RX_STBC               = 285,    /* Support RX STBC */
	IEEE80211_PARAM_APONLY                  = 293,
	IEEE80211_PARAM_TXRX_FW_STATS_RESET     = 294,
	IEEE80211_PARAM_TX_PPDU_LOG_CFG         = 295,  /* tx PPDU log cfg params */
	IEEE80211_PARAM_OPMODE_NOTIFY           = 296,  /* Op Mode Notification */
	IEEE80211_PARAM_NOPBN                   = 297, /* don't send push button notification */
	IEEE80211_PARAM_DFS_CACTIMEOUT          = 298, /* override CAC timeout */
	IEEE80211_PARAM_ENABLE_RTSCTS           = 299, /* Enable/disable RTS-CTS */

	IEEE80211_PARAM_MAX_AMPDU               = 300,   /* Set/Get rx AMPDU exponent/shift */
	IEEE80211_PARAM_VHT_MAX_AMPDU           = 301,   /* Set/Get rx VHT AMPDU exponent/shift */
	IEEE80211_PARAM_BCAST_RATE              = 302,   /* Setting Bcast DATA rate */
	IEEE80211_PARAM_PARENT_IFINDEX          = 304,   /* parent net_device ifindex for this VAP */
#if WDS_VENDOR_EXTENSION
	IEEE80211_PARAM_WDS_RX_POLICY           = 305,  /* Set/Get WDS rx filter policy for vendor specific WDS */
#endif
	IEEE80211_PARAM_ENABLE_OL_STATS         = 306,   /*Enables/Disables the
							   stats in the Host and in the FW */
#if WLAN_SUPPORT_GREEN_AP
	IEEE80211_IOCTL_GREEN_AP_ENABLE_PRINT   = 307,  /* Enable/Disable Green-AP debug prints */
#endif
	IEEE80211_PARAM_RC_NUM_RETRIES          = 308,
	IEEE80211_PARAM_GET_ACS                 = 309,/* to get status of acs */
	IEEE80211_PARAM_GET_CAC                 = 310,/* to get status of CAC period */
	IEEE80211_PARAM_EXT_IFACEUP_ACS         = 311,  /* Enable external auto channel selection entity
							   at VAP init time */
	IEEE80211_PARAM_ONETXCHAIN              = 312,  /* force to tx with one chain for legacy client */
	IEEE80211_PARAM_DFSDOMAIN               = 313,  /* Get DFS Domain */
	IEEE80211_PARAM_SCAN_CHAN_EVENT         = 314,  /* Enable delivery of Scan Channel Events during
							   802.11 scans (11ac offload, and IEEE80211_M_HOSTAP
							   mode only). */
	IEEE80211_PARAM_DESIRED_CHANNEL         = 315,  /* Get desired channel corresponding to desired
							   PHY mode */
	IEEE80211_PARAM_DESIRED_PHYMODE         = 316,  /* Get desired PHY mode */
	IEEE80211_PARAM_SEND_ADDITIONAL_IES     = 317,  /* Control sending of additional IEs to host */
	IEEE80211_PARAM_START_ACS_REPORT        = 318,  /* to start acs scan report */
	IEEE80211_PARAM_MIN_DWELL_ACS_REPORT    = 319,  /* min dwell time for  acs scan report */
	IEEE80211_PARAM_MAX_DWELL_ACS_REPORT    = 320,  /* max dwell time for  acs scan report */
	IEEE80211_PARAM_ACS_CH_HOP_LONG_DUR     = 321,  /* channel long duration timer used in acs */
	IEEE80211_PARAM_ACS_CH_HOP_NO_HOP_DUR   = 322,  /* No hopping timer used in acs */
	IEEE80211_PARAM_ACS_CH_HOP_CNT_WIN_DUR  = 323,  /* counter window timer used in acs */
	IEEE80211_PARAM_ACS_CH_HOP_NOISE_TH     = 324,  /* Noise threshold used in acs channel hopping */
	IEEE80211_PARAM_ACS_CH_HOP_CNT_TH       = 325,  /* counter threshold used in acs channel hopping */
	IEEE80211_PARAM_ACS_ENABLE_CH_HOP       = 326,  /* Enable/Disable acs channel hopping */
	IEEE80211_PARAM_SET_CABQ_MAXDUR         = 327,  /* set the max tx percentage for cabq */
	IEEE80211_PARAM_256QAM_2G               = 328,  /* 2.4 GHz 256 QAM support */
	IEEE80211_PARAM_MAX_SCANENTRY           = 330,  /* MAX scan entry */
	IEEE80211_PARAM_SCANENTRY_TIMEOUT       = 331,  /* Scan entry timeout value */
	IEEE80211_PARAM_PURE11AC                = 332,  /* pure 11ac(no 11bg/11a/11n stations) */
#if UMAC_VOW_DEBUG
	IEEE80211_PARAM_VOW_DBG_ENABLE  = 333,  /*Enable VoW debug*/
#endif
	IEEE80211_PARAM_SCAN_MIN_DWELL          = 334,  /* MIN dwell time to be used during scan */
	IEEE80211_PARAM_SCAN_MAX_DWELL          = 335,  /* MAX dwell time to be used during scan */
	IEEE80211_PARAM_BANDWIDTH               = 336,
	IEEE80211_PARAM_FREQ_BAND               = 337,
	IEEE80211_PARAM_EXTCHAN                 = 338,
	IEEE80211_PARAM_MCS                     = 339,
	IEEE80211_PARAM_CHAN_NOISE              = 340,
	IEEE80211_PARAM_VHT_SGIMASK             = 341,   /* Set VHT SGI MASK */
	IEEE80211_PARAM_VHT80_RATEMASK          = 342,   /* Set VHT80 Auto Rate MASK */
#if ATH_PERF_PWR_OFFLOAD
	IEEE80211_PARAM_VAP_TX_ENCAP_TYPE       = 343,
	IEEE80211_PARAM_VAP_RX_DECAP_TYPE       = 344,
#endif /* ATH_PERF_PWR_OFFLOAD */
#if HOST_SW_TSO_SG_ENABLE
	IEEE80211_PARAM_TSO_STATS               = 345, /* Get TSO Stats */
	IEEE80211_PARAM_TSO_STATS_RESET         = 346, /* Reset TSO Stats */
#endif /* HOST_SW_TSO_SG_ENABLE */
#if RX_CHECKSUM_OFFLOAD
	IEEE80211_PARAM_RX_CKSUM_ERR_STATS      = 349, /* Get RX CKSUM Err Stats */
	IEEE80211_PARAM_RX_CKSUM_ERR_RESET      = 350, /* Reset RX CKSUM Err Stats */
#endif /* RX_CHECKSUM_OFFLOAD */

	IEEE80211_PARAM_VHT_STS_CAP             = 351,
	IEEE80211_PARAM_VHT_SOUNDING_DIM        = 352,
	IEEE80211_PARAM_VHT_SUBFEE              = 353,   /* set VHT SU beamformee capability */
	IEEE80211_PARAM_VHT_MUBFEE              = 354,   /* set VHT MU beamformee capability */
	IEEE80211_PARAM_VHT_SUBFER              = 355,   /* set VHT SU beamformer capability */
	IEEE80211_PARAM_VHT_MUBFER              = 356,   /* set VHT MU beamformer capability */
	IEEE80211_PARAM_IMPLICITBF              = 357,
	IEEE80211_PARAM_SEND_WOWPKT             = 358, /* Send Wake-On-Wireless packet */
	IEEE80211_PARAM_STA_FIXED_RATE          = 359, /* set/get fixed rate for associated sta on AP */
	IEEE80211_PARAM_11NG_VHT_INTEROP        = 360,  /* 2.4ng Vht Interop */
#if HOST_SW_SG_ENABLE
	IEEE80211_PARAM_SG_STATS                = 361, /* Get SG Stats */
	IEEE80211_PARAM_SG_STATS_RESET          = 362, /* Reset SG Stats */
#endif /* HOST_SW_SG_ENABLE */
	IEEE80211_PARAM_SPLITMAC                = 363,
	IEEE80211_PARAM_SHORT_SLOT              = 364,   /* Set short slot time */
	IEEE80211_PARAM_SET_ERP                 = 365,   /* Set ERP protection mode  */
	IEEE80211_PARAM_SESSION_TIMEOUT         = 366,   /* STA's session time */
#if ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION
	IEEE80211_PARAM_RAWMODE_SIM_TXAGGR      = 367,   /* Enable/disable raw mode simulation
							    Tx A-MSDU aggregation */
	IEEE80211_PARAM_RAWMODE_PKT_SIM_STATS   = 368,   /* Get Raw mode packet simulation stats. */
	IEEE80211_PARAM_CLR_RAWMODE_PKT_SIM_STATS = 369, /* Clear Raw mode packet simulation stats. */
	IEEE80211_PARAM_RAWMODE_SIM_DEBUG_LEVEL   = 370,   /* Set raw mode simulation debug level */
#endif /* ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION */
	IEEE80211_PARAM_PROXY_STA               = 371,   /* set/get ProxySTA */
	IEEE80211_PARAM_BW_NSS_RATEMASK         = 372,   /* Set ratemask with specific Bandwidth and NSS  */
	IEEE80211_PARAM_RX_SIGNAL_DBM           = 373,  /*get rx signal strength in dBm*/
	IEEE80211_PARAM_VHT_TX_MCSMAP           = 374,   /* Set VHT TX MCS MAP */
	IEEE80211_PARAM_VHT_RX_MCSMAP           = 375,   /* Set VHT RX MCS MAP */
	IEEE80211_PARAM_WNM_SMENTER             = 376,
	IEEE80211_PARAM_WNM_SMEXIT              = 377,
	IEEE80211_PARAM_HC_BSSLOAD              = 378,
	IEEE80211_PARAM_OSEN                    = 379,
#if QCA_AIRTIME_FAIRNESS
	IEEE80211_PARAM_ATF_OPT                 = 380,   /* set airtime feature */
	IEEE80211_PARAM_ATF_PER_UNIT            = 381,
#endif
	IEEE80211_PARAM_TX_MIN_POWER            = 382, /* Get min tx power */
	IEEE80211_PARAM_TX_MAX_POWER            = 383, /* Get max tx power */
	IEEE80211_PARAM_MGMT_RATE               = 384, /* Set mgmt rate, will set mcast/bcast/ucast to same rate*/
	IEEE80211_PARAM_NO_VAP_RESET            = 385, /* Disable the VAP reset in NSS */
	IEEE80211_PARAM_STA_COUNT               = 386, /* TO get number of station associated*/
#if QCA_SUPPORT_SSID_STEERING
	IEEE80211_PARAM_VAP_SSID_CONFIG         = 387, /* Vap configuration  */
#endif
#if ATH_SUPPORT_DSCP_OVERRIDE
	IEEE80211_PARAM_DSCP_MAP_ID             = 388,
	IEEE80211_PARAM_DSCP_TID_MAP            = 389,
#endif
	IEEE80211_PARAM_RX_FILTER_MONITOR       = 390,
	IEEE80211_PARAM_SECOND_CENTER_FREQ      = 391,
	IEEE80211_PARAM_STRICT_BW               = 392,  /* BW restriction in pure 11ac */
	IEEE80211_PARAM_ADD_LOCAL_PEER          = 393,
	IEEE80211_PARAM_SET_MHDR                = 394,
	IEEE80211_PARAM_ALLOW_DATA              = 395,
	IEEE80211_PARAM_SET_MESHDBG             = 396,
	IEEE80211_PARAM_RTT_ENABLE              = 397,
	IEEE80211_PARAM_LCI_ENABLE              = 398,
	IEEE80211_PARAM_VAP_ENHIND              = 399, /* Independent VAP mode for Repeater and AP-STA config */
	IEEE80211_PARAM_VAP_PAUSE_SCAN          = 400, /* Pause VAP mode for scanning */
	IEEE80211_PARAM_EXT_ACS_IN_PROGRESS     = 401, /* Whether external auto channel selection is in
							  progress */
	IEEE80211_PARAM_AMPDU_DENSITY_OVERRIDE  = 402,  /* a-mpdu density override */
	IEEE80211_PARAM_SMART_MESH_CONFIG       = 403,  /* smart MESH configuration */
	IEEE80211_DISABLE_BCN_BW_NSS_MAP        = 404, /* To set & get Bandwidth-NSS mapping in beacon as vendor specific IE*/
	IEEE80211_DISABLE_STA_BWNSS_ADV         = 405, /* To disable all Bandwidth-NSS mapping feature in STA mode*/
	IEEE80211_PARAM_MIXED_MODE              = 406, /* In case of STA, this tells whether the AP we are associated
							  to supports TKIP alongwith AES */
	IEEE80211_PARAM_RX_FILTER_NEIGHBOUR_PEERS_MONITOR = 407,  /* filter out /drop invalid peers packet to upper stack */
#if ATH_DATA_RX_INFO_EN
	IEEE80211_PARAM_RXINFO_PERPKT          = 408,  /* update rx info per pkt */
#endif
	IEEE80211_PARAM_WHC_APINFO_WDS          = 415, /* Whether associated AP supports WDS
							  (as determined from the vendor IE) */
	IEEE80211_PARAM_WHC_APINFO_ROOT_DIST    = 416, /* Distance from the root AP (in hops);
							  only valid if the WDS flag is set
							  based on the param above */
	IEEE80211_PARAM_ATH_SUPPORT_VLAN        = 417,
	IEEE80211_PARAM_CONFIG_ASSOC_WAR_160W   = 418, /* Configure association WAR for 160 MHz width (i.e.
							  160/80+80 MHz modes). Some STAs may have an issue
							  associating with us if we advertise 160/80+80 MHz related
							  capabilities in probe response/association response.
							  Hence this WAR suppresses 160/80+80 MHz related
							  information in probe responses, and association responses
							  for such STAs.
							  Starting from LSB
							  First bit set        = Default WAR behavior (VHT_OP modified)
							  First+second bit set = (VHT_OP+ VHT_CAP modified)
							  No bit set (default) = WAR disabled
							*/
#if DBG_LVL_MAC_FILTERING
	IEEE80211_PARAM_DBG_LVL_MAC             = 419, /* Enable/disable mac based filtering for debug logs */
#endif
#if QCA_AIRTIME_FAIRNESS
	IEEE80211_PARAM_ATF_TXBUF_MAX           = 420,
	IEEE80211_PARAM_ATF_TXBUF_MIN           = 421,
	IEEE80211_PARAM_ATF_TXBUF_SHARE         = 422, /* For ATF UDP */
	IEEE80211_PARAM_ATF_MAX_CLIENT          = 423, /* Support of ATF+non-ATF clients */
	IEEE80211_PARAM_ATF_SSID_GROUP          = 424, /* Support to enable/disable SSID grouping */
#endif
	IEEE80211_PARAM_11N_TX_AMSDU            = 425, /* Enable/Disable HT Tx AMSDU only */
	IEEE80211_PARAM_BSS_CHAN_INFO           = 426,
	IEEE80211_PARAM_LCR_ENABLE              = 427,
	IEEE80211_PARAM_WHC_APINFO_SON          = 428, /* Whether associated AP supports SON mode
							  (as determined from the vendor IE) */
	IEEE80211_PARAM_SON                     = 429, /* Mark/query AP as SON enabled */
	IEEE80211_PARAM_CTSPROT_DTIM_BCN        = 430, /* Enable/Disable CTS2SELF protection for DTIM Beacons */
	IEEE80211_PARAM_RAWMODE_PKT_SIM         = 431, /* Enable/Disable RAWMODE_PKT_SIM*/
	IEEE80211_PARAM_CONFIG_RAW_DWEP_IND     = 432, /* Enable/disable indication to WLAN driver that
							  dynamic WEP is being used in RAW mode. If the indication
							  is enabled and we are in RAW mode, we plumb a dummy key for
							  each of the keys corresponding to WEP cipher
							*/
#if ATH_GEN_RANDOMNESS
	IEEE80211_PARAM_RANDOMGEN_MODE           = 433,
#endif

	IEEE80211_PARAM_CUSTOM_CHAN_LIST         = 434,
#if UMAC_SUPPORT_ACFG
	IEEE80211_PARAM_DIAG_WARN_THRESHOLD     = 435,
	IEEE80211_PARAM_DIAG_ERR_THRESHOLD      = 436,
#endif
	IEEE80211_PARAM_MBO                           = 437,     /*  Enable MBO */
	IEEE80211_PARAM_MBO_CAP                       = 438,     /*  Enable MBO capability */
	IEEE80211_PARAM_MBO_ASSOC_DISALLOW            = 439,     /*  MBO  reason code for assoc disallow attribute */
	IEEE80211_PARAM_MBO_CELLULAR_PREFERENCE       = 440,     /*  MBO cellular preference */
	IEEE80211_PARAM_MBO_TRANSITION_REASON         = 441,     /*  MBO Tansition reason */
	IEEE80211_PARAM_MBO_ASSOC_RETRY_DELAY         = 442,     /*  MBO  assoc retry delay */
#if ATH_SUPPORT_DSCP_OVERRIDE
	IEEE80211_PARAM_VAP_DSCP_PRIORITY        = 443,  /* VAP Based DSCP - Vap priority */
#endif
	IEEE80211_PARAM_TXRX_VAP_STATS           = 444,
	IEEE80211_PARAM_CONFIG_REV_SIG_160W      = 445, /* Enable/Disable revised signalling for 160/80+80 MHz */
	IEEE80211_PARAM_DISABLE_SELECTIVE_HTMCS_FOR_VAP = 446, /* Enable/Disable selective HT-MCS for this vap. */
	IEEE80211_PARAM_CONFIGURE_SELECTIVE_VHTMCS_FOR_VAP = 447, /* Enable/Disable selective VHT-MCS for this vap. */
	IEEE80211_PARAM_RDG_ENABLE              = 448,
	IEEE80211_PARAM_DFS_SUPPORT             = 449,
	IEEE80211_PARAM_DFS_ENABLE              = 450,
	IEEE80211_PARAM_ACS_SUPPORT             = 451,
	IEEE80211_PARAM_SSID_STATUS             = 452,
	IEEE80211_PARAM_DL_QUEUE_PRIORITY_SUPPORT = 453,
	IEEE80211_PARAM_CLEAR_MIN_MAX_RSSI        = 454,
	IEEE80211_PARAM_CLEAR_QOS            = 455,
#if QCA_AIRTIME_FAIRNESS
	IEEE80211_PARAM_ATF_OVERRIDE_AIRTIME_TPUT = 456, /* Override the airtime estimated */
#endif
#if MESH_MODE_SUPPORT
	IEEE80211_PARAM_MESH_CAPABILITIES      = 457, /* For providing Mesh vap capabilities */
#endif
	IEEE80211_PARAM_CONFIG_ASSOC_DENIAL_NOTIFY = 458,  /* Enable/disable assoc denial notification to userspace */
	IEEE80211_PARAM_ADD_MAC_LIST_SEC = 459, /* To check if the mac address is to added in secondary ACL list */
	IEEE80211_PARAM_GET_MAC_LIST_SEC = 460, /* To get the mac addresses from the secondary ACL list */
	IEEE80211_PARAM_DEL_MAC_LIST_SEC = 461, /* To delete the given mac address from the secondary ACL list */
	IEEE80211_PARAM_MACCMD_SEC = 462, /* To set/get the acl policy of the secondary ACL list */
	IEEE80211_PARAM_UMAC_VERBOSE_LVL           = 463, /* verbose level for UMAC specific debug */
	IEEE80211_PARAM_VAP_TXRX_FW_STATS          = 464, /* Get per VAP MU-MIMO stats */
	IEEE80211_PARAM_VAP_TXRX_FW_STATS_RESET    = 465, /* Reset per VAp MU-MIMO stats */
	IEEE80211_PARAM_PEER_TX_MU_BLACKLIST_COUNT = 466, /* Get number of times a peer has been blacklisted due to sounding failures */
	IEEE80211_PARAM_PEER_TX_COUNT              = 467, /* Get count of MU MIMO tx to a peer */
	IEEE80211_PARAM_PEER_MUMIMO_TX_COUNT_RESET = 468, /* Reset count of MU MIMO tx to a peer */
	IEEE80211_PARAM_PEER_POSITION              = 469, /* Get peer position in MU group */
#if QCA_AIRTIME_FAIRNESS
	IEEE80211_PARAM_ATF_SSID_SCHED_POLICY    = 470, /* support to set per ssid atf sched policy, 0-fair 1-strict */
#endif
	IEEE80211_PARAM_CONNECTION_SM_STATE        = 471, /* Get the current state of the connectionm SM */
#if MESH_MODE_SUPPORT
	IEEE80211_PARAM_CONFIG_MGMT_TX_FOR_MESH    = 472,
	IEEE80211_PARAM_CONFIG_RX_MESH_FILTER      = 473,
#endif
	IEEE80211_PARAM_TRAFFIC_STATS              = 474,   /* Enable/disable the measurement of traffic statistics */
	IEEE80211_PARAM_TRAFFIC_RATE               = 475,   /* set the traffic rate, the rate at which the received signal statistics are be measured */
	IEEE80211_PARAM_TRAFFIC_INTERVAL           = 476,   /* set the traffic interval,the time till which the received signal statistics are to be measured */
	IEEE80211_PARAM_WATERMARK_THRESHOLD        = 477,
	IEEE80211_PARAM_WATERMARK_REACHED          = 478,
	IEEE80211_PARAM_ASSOC_REACHED              = 479,
	IEEE80211_PARAM_DISABLE_SELECTIVE_LEGACY_RATE_FOR_VAP = 480,      /* Enable/Disable selective Legacy Rates for this vap. */
	IEEE80211_PARAM_RTSCTS_RATE                = 481,   /* Set rts and cts rate*/
	IEEE80211_PARAM_REPT_MULTI_SPECIAL         = 482,
	IEEE80211_PARAM_VSP_ENABLE                 = 483,   /* Video Stream Protection */
	IEEE80211_PARAM_ENABLE_VENDOR_IE           = 484,    /* Enable/ disable Vendor ie advertise in Beacon/ proberesponse*/
	IEEE80211_PARAM_WHC_APINFO_SFACTOR         = 485,  /* Set Scaling factor for best uplink selection algorithm */
	IEEE80211_PARAM_WHC_APINFO_BSSID           = 486,  /* Get the best uplink BSSID for scan entries */
	IEEE80211_PARAM_WHC_APINFO_RATE            = 487,  /* Get the current uplink data rate(estimate) */
	IEEE80211_PARAM_CONFIG_MON_DECODER         = 488,  /* Monitor VAP decoder format radiotap/prism */
	IEEE80211_PARAM_DYN_BW_RTS                 = 489,   /* Enable/Disable the dynamic bandwidth RTS */
	IEEE80211_PARAM_CONFIG_MU_CAP_TIMER        = 490,  /* Set/Get timer period in seconds(1 to 300) for de-assoc dedicated client when
							      mu-cap client joins/leaves */
	IEEE80211_PARAM_CONFIG_MU_CAP_WAR          = 491,   /* Enable/Disable Mu Cap WAR function */
#if WLAN_DFS_CHAN_HIDDEN_SSID
	IEEE80211_PARAM_CONFIG_BSSID               = 492,  /* Configure hidden ssid AP's bssid */
#endif
	IEEE80211_PARAM_CONFIG_NSTSCAP_WAR         = 493,  /* Enable/Disable NSTS CAP WAR */
	IEEE80211_PARAM_WHC_APINFO_CAP_BSSID       = 494,   /* get the CAP BSSID from scan entries */
	IEEE80211_PARAM_BEACON_RATE_FOR_VAP        = 495,      /*Configure beacon rate to user provided rate*/
	IEEE80211_PARAM_CHANNEL_SWITCH_MODE        = 496,   /* channel switch mode to be used in CSA and ECSA IE*/
	IEEE80211_PARAM_ENABLE_ECSA_IE             = 497,   /* ECSA IE  enable/disable*/
	IEEE80211_PARAM_ECSA_OPCLASS               = 498,   /* opClass to be announced in ECSA IE */
#if DYNAMIC_BEACON_SUPPORT
	IEEE80211_PARAM_DBEACON_EN                 = 499, /* Enable/disable the dynamic beacon feature */
	IEEE80211_PARAM_DBEACON_RSSI_THR           = 500, /* Set/Get the rssi threshold */
	IEEE80211_PARAM_DBEACON_TIMEOUT            = 501, /* Set/Get the timeout of timer */
#endif
	IEEE80211_PARAM_TXPOW_MGMT                 = 502,   /* set/get the tx power per vap */
	IEEE80211_PARAM_CONFIG_TX_CAPTURE          = 503, /* Configure pkt capture in Tx direction */
#if WLAN_DFS_CHAN_HIDDEN_SSID
	IEEE80211_PARAM_GET_CONFIG_BSSID           = 504, /* get configured hidden ssid AP's bssid */
#endif
	IEEE80211_PARAM_OCE                        = 505,  /* Enable OCE */
	IEEE80211_PARAM_OCE_ASSOC_REJECT           = 506,  /* Enable OCE RSSI-based assoc reject */
	IEEE80211_PARAM_OCE_ASSOC_MIN_RSSI         = 507,  /* Min RSSI for assoc accept */
	IEEE80211_PARAM_OCE_ASSOC_RETRY_DELAY      = 508,  /* Retry delay for subsequent (re-)assoc */
	IEEE80211_PARAM_OCE_WAN_METRICS            = 509,  /* Enable OCE reduced WAN metrics */
	IEEE80211_PARAM_BACKHAUL                   = 510,
	IEEE80211_PARAM_MESH_MCAST                 = 511,
	IEEE80211_PARAM_HE_MCS                     = 512,   /* Set 11ax - HE MCS */

	IEEE80211_PARAM_HE_EXTENDED_RANGE          = 513,   /* set 11ax - HE extended range */
	IEEE80211_PARAM_HE_DCM                     = 514,   /* set 11ax - HE DCM */
	IEEE80211_PARAM_HE_MU_EDCA                 = 515,   /* Set 11ax - HE MU EDCA */
	IEEE80211_PARAM_HE_FRAGMENTATION           = 516,   /* Set 11ax - HE Fragmentation */

	IEEE80211_PARAM_HE_DL_MU_OFDMA             = 517,   /* Set 11ax - HE DL MU OFDMA */
	IEEE80211_PARAM_HE_UL_MU_MIMO              = 518,   /* Set 11ax - HE UL MU MIMO */
	IEEE80211_PARAM_HE_UL_MU_OFDMA             = 519,   /* Set 11ax - HE UL MU OFDMA */
	IEEE80211_PARAM_CONFIG_CATEGORY_VERBOSE    = 521,   /* Configure verbose level for a category */
	IEEE80211_PARAM_TXRX_DP_STATS              = 522,   /* Display stats aggregated at host */
	IEEE80211_PARAM_RX_FILTER_SMART_MONITOR    = 523,   /* Get per vap smart monitor stats */
	IEEE80211_PARAM_WHC_CAP_RSSI               = 524,   /* Set/Get the CAP RSSI threshold for best uplink selection */
	IEEE80211_PARAM_WHC_CURRENT_CAP_RSSI       = 525,   /* Get the current CAP RSSI from scan entrie */
	IEEE80211_PARAM_RX_SMART_MONITOR_RSSI      = 526,   /* Get smart monitor rssi */
	IEEE80211_PARAM_WHC_APINFO_BEST_UPLINK_OTHERBAND_BSSID = 527, /* Get the best otherband uplink BSSID */
	IEEE80211_PARAM_WHC_APINFO_OTHERBAND_UPLINK_BSSID = 528, /* Get the current otherband uplink BSSID from scan entry */
	IEEE80211_PARAM_WHC_APINFO_OTHERBAND_BSSID = 529,   /* Set the otherband BSSID for AP vap */
	IEEE80211_PARAM_WHC_APINFO_UPLINK_RATE     = 530,   /* Get the current uplink rate */
	IEEE80211_PARAM_HE_SU_BFEE                 = 531,   /* Set 11ax - HE SU BFEE */
	IEEE80211_PARAM_HE_SU_BFER                 = 532,   /* Set 11ax - HE SU BFER */
	IEEE80211_PARAM_HE_MU_BFEE                 = 533,   /* Set 11ax - HE MU BFEE */
	IEEE80211_PARAM_HE_MU_BFER                 = 534,   /* Set 11ax - HE MU BFER */
	IEEE80211_PARAM_EXT_NSS_SUPPORT            = 535,   /* EXT NSS Support */
	IEEE80211_PARAM_QOS_ACTION_FRAME_CONFIG    = 536,   /* QOS Action frames config */
	IEEE80211_PARAM_HE_LTF                     = 537,   /* Set 11ax - HE LTF Support */
	IEEE80211_PARAM_DFS_INFO_NOTIFY_APP        = 538,   /* Enable the feature to notify dfs info to app */
	IEEE80211_PARAM_NSSOL_VAP_INSPECT_MODE     = 539,   /* Set Vap inspection mode */
	IEEE80211_PARAM_HE_RTSTHRSHLD              = 540,   /* Set 11ax - HE Rts Threshold */
	IEEE80211_PARAM_RATE_DROPDOWN              = 541,   /* Setting Rate Control Logic */
	IEEE80211_PARAM_DISABLE_CABQ               = 542,   /* Disable multicast buffer when STA is PS */
	IEEE80211_PARAM_CSL_SUPPORT                = 543,   /* CSL Support */
	IEEE80211_PARAM_TIMEOUTIE                  = 544,   /* set/get assoc comeback timeout value */
	IEEE80211_PARAM_PMF_ASSOC                  = 545,   /* enable/disable pmf support */
	IEEE80211_PARAM_ENABLE_FILS                = 546,   /* Enable/disable FILS */
#if ATH_ACS_DEBUG_SUPPORT
	IEEE80211_PARAM_ACS_DEBUG_SUPPORT           = 547,   /* enable acs debug stub for testing */
#endif
#if ATH_ACL_SOFTBLOCKING
	IEEE80211_PARAM_SOFTBLOCK_WAIT_TIME        = 548,   /* set/get wait time in softblcking */
	IEEE80211_PARAM_SOFTBLOCK_ALLOW_TIME       = 549,   /* set/get allow time in softblocking */
#endif
	IEEE80211_PARAM_NR_SHARE_RADIO_FLAG        = 550,   /* Mask to indicate which radio the NR information shares across */
#if QCN_IE
	IEEE80211_PARAM_BCAST_PROBE_RESPONSE_DELAY = 551, /* set/get the delay for holding the broadcast
							     probe response (in ms) */
	IEEE80211_PARAM_BCAST_PROBE_RESPONSE_LATENCY_COMPENSATION = 552, /* set/get latency for the RTT made by the broadcast
									    probe response(in ms) */
	IEEE80211_PARAM_BCAST_PROBE_RESPONSE_STATS = 553, /* Get the broadcast probe response stats */
	IEEE80211_PARAM_BCAST_PROBE_RESPONSE_ENABLE = 554, /* If set, enables the broadcast probe response feature */
	IEEE80211_PARAM_BCAST_PROBE_RESPONSE_STATS_CLEAR = 555, /* Clear the broadcast probe response stats */
	IEEE80211_PARAM_BEACON_LATENCY_COMPENSATION = 556, /* Set/get the beacon latency between driver and firmware */
#endif
	IEEE80211_PARAM_DMS_AMSDU_WAR              = 557,   /* Enable 11v DMS AMSDU WAR */
	IEEE80211_PARAM_TXPOW                      = 558,   /* set/get the control frame tx power per vap */
	IEEE80211_PARAM_BEST_UL_HYST               = 559,
	IEEE80211_PARAM_HE_TX_MCSMAP               = 560,   /* set 11ax - HE TX MCSMAP */
	IEEE80211_PARAM_HE_RX_MCSMAP               = 561,   /* set 11ax - HE RX MCSMAP */
	IEEE80211_PARAM_CONFIG_M_COPY              = 562,   /* Enable/Disable Mirror copy mode */
	IEEE80211_PARAM_BA_BUFFER_SIZE             = 563,   /* Set Block Ack Buffer Size */
	IEEE80211_PARAM_HE_AR_GI_LTF               = 564,   /* Set HE Auto Rate GI LTF combinations */
	IEEE80211_PARAM_NSSOL_VAP_READ_RXPREHDR    = 565,   /* Read Rx pre header content from the packet */
	IEEE80211_PARAM_HE_SOUNDING_MODE           = 566,   /* Select HE/VHT, SU/MU and Trig/Nong-Trig sounding */
	IEEE80211_PARAM_PRB_RATE                   = 570,   /* Set/Get probe-response frame rate */
	IEEE80211_PARAM_RSN_OVERRIDE               = 571,   /* enable/disable rsn override feature */
	IEEE80211_PARAM_MAP                        = 572,   /* multi ap enable */
	IEEE80211_PARAM_MAP_BSS_TYPE               = 573,   /* Multi Ap BSS TYPES */
	/* Enable/Disable HE HT control support */
	IEEE80211_PARAM_HE_HT_CTRL                 = 574,
	IEEE80211_PARAM_OCE_HLP                    = 575,   /* Enable/disable OCE FILS HLP */
	IEEE80211_PARAM_NBR_SCAN_PERIOD            = 576,   /* set/get neighbor AP scan period */
	IEEE80211_PARAM_RNR                        = 577,   /* enable/disable inclusion of RNR IE in Beacon/Probe-Rsp */
	IEEE80211_PARAM_RNR_FD                     = 578,   /* enable/disable inclusion of RNR IE in FILS Discovery */
	IEEE80211_PARAM_RNR_TBTT                   = 579,   /* enable/disable calculation TBTT in RNR IE */
	IEEE80211_PARAM_AP_CHAN_RPT                = 580,   /* enable/disable inclusion of AP Channel Report IE in Beacon/Probe-Rsp */
	IEEE80211_PARAM_MAX_SCAN_TIME_ACS_REPORT   = 581,   /* max scan time for acs scan report */
	IEEE80211_PARAM_MAP_VAP_BEACONING          = 582,   /* multi ap vap teardown */
	IEEE80211_PARAM_ACL_SET_BLOCK_MGMT         = 583,   /* block mgmt from a mac-address */
	IEEE80211_PARAM_ACL_CLR_BLOCK_MGMT         = 584,   /* allow mgmt from a mac-address */
	IEEE80211_PARAM_ACL_GET_BLOCK_MGMT         = 585,   /* get list of mac-addresses */
	IEEE80211_PARAM_WIFI_DOWN_IND              = 586,   /* wifi down indication in 11ax MbSSID case */
#if UMAC_SUPPORT_XBSSLOAD
	IEEE80211_PARAM_XBSS_LOAD                  = 587,   /* Enable/Disable 802.11ac extended BSS load */
#endif
	IEEE80211_PARAM_SIFS_TRIGGER_RATE          = 588,   /* get or set vdev sifs trigger rate */
	IEEE80211_PARAM_LOG_FLUSH_TIMER_PERIOD     = 589,   /* time interval in which wlan log buffers will be pushed to user-space */
	IEEE80211_PARAM_LOG_FLUSH_ONE_TIME         = 590,   /* push the logs to user space one time */
	IEEE80211_PARAM_LOG_DUMP_AT_KERNEL_ENABLE  = 591,   /* enable/disable kernel print call in logging */
	IEEE80211_PARAM_LOG_ENABLE_BSTEERING_RSSI  = 592,   /* enable/disable inst rssi log for son */
	IEEE80211_PARAM_FT_ENABLE                  = 593,   /* Enable/Disable BSS fast transition */
	IEEE80211_PARAM_BCN_STATS_RESET            = 594,   /* send beacon stats reset command */
	IEEE80211_PARAM_WLAN_SER_TEST              = 595,   /* Start unit testing of serialization */
	IEEE80211_PARAM_SIFS_TRIGGER               = 596,   /* get/set sifs trigger interval per vdev */
#if QCA_SUPPORT_SON
	IEEE80211_PARAM_SON_EVENT_BCAST            = 597,   /* enable/disable events broadcast for son */
#endif
	IEEE80211_PARAM_HE_UL_SHORTGI              = 598,   /* Shortgi configuration in UL Trigger */
	IEEE80211_PARAM_HE_UL_LTF                  = 599,   /* LTF configuration in UL Trigger */
	IEEE80211_PARAM_HE_UL_NSS                  = 600,   /* Maximum NSS allowed in UL Trigger */
	IEEE80211_PARAM_HE_UL_PPDU_BW              = 601,   /* Channel width */
	IEEE80211_PARAM_HE_UL_LDPC                 = 602,   /* Enable/Disable LDPC in UL Trigger */
	IEEE80211_PARAM_HE_UL_STBC                 = 603,   /* Enable/Disable STBC in UL Trigger */
	IEEE80211_PARAM_HE_UL_FIXED_RATE           = 604,   /* Control UL fixed rate */
#if WLAN_SER_DEBUG
	IEEE80211_PARAM_WLAN_SER_HISTORY           = 605,   /* Dump serialization queues & history*/
#endif
	IEEE80211_PARAM_DA_WAR_ENABLE              = 606,   /* get/set for destination address setting for WDS */
	/* Enable/disable the advertisement of STA-mode's maximum capabilities instead of it's operating mode in related capability related IEs */
	IEEE80211_PARAM_STA_MAX_CH_CAP             = 607,
	IEEE80211_PARAM_RAWMODE_OPEN_WAR           = 608,   /* enable/disable rawmode open war */
	IEEE80211_PARAM_EXTERNAL_AUTH_STATUS       = 609,   /* To indicate exterbal auth status for SAE */
#if UMAC_SUPPORT_WPA3_STA
	IEEE80211_PARAM_SAE_AUTH_ATTEMPTS          = 610,   /* To set/get sae maximum auth attempts */
#endif
	IEEE80211_PARAM_WHC_SKIP_HYST              = 611,   /* Set/Get Skip hyst for best AP*/
	IEEE80211_PARAM_GET_FREQUENCY              = 612,   /* Get Frequency */
	IEEE80211_PARAM_SON_NUM_VAP                = 613,   /* To get the number of SON enabled Vaps */
	IEEE80211_PARAM_WHC_MIXEDBH_ULRATE         = 614,   /* Get or Set the Uplink rate for SON mixedbackhaul */
	IEEE80211_PARAM_WHC_BACKHAUL_TYPE          = 615,   /* Set the SON Mode and Wifi or Ether backhaul type */
	IEEE80211_PARAM_GET_OPMODE                 = 616,   /* Get operation mode of VAP*/
	IEEE80211_PARAM_HE_BSR_SUPPORT             = 617,   /* HE BSR Support */
	IEEE80211_PARAM_SET_VLAN_TYPE              = 618,   /* enable/disable VLAN configuration on VAP in NSS offload mode */
	IEEE80211_PARAM_DUMP_RA_TABLE              = 619,   /* Dump the RA table used for Multicast Enhancement 6 */
	IEEE80211_PARAM_OBSS_NB_RU_TOLERANCE_TIME  = 620,   /* To set/get OBSS RU26 Intolerance time */
	IEEE80211_PARAM_UNIFORM_RSSI               = 621,   /* Uniform RSSI support */
	IEEE80211_PARAM_CSA_INTEROP_PHY            = 622,
	IEEE80211_PARAM_CSA_INTEROP_BSS            = 623,
	IEEE80211_PARAM_CSA_INTEROP_AUTH           = 624,
	IEEE80211_PARAM_HE_AMSDU_IN_AMPDU_SUPRT    = 625,   /* HE AMSDU in AMPDU support */
	IEEE80211_PARAM_HE_SUBFEE_STS_SUPRT        = 626,   /* HE BFEE STS support */
	IEEE80211_PARAM_HE_4XLTF_800NS_GI_RX_SUPRT = 627,   /* HE 4x LTF + 0.8 us GI rx support */
	IEEE80211_PARAM_HE_1XLTF_800NS_GI_RX_SUPRT = 628,   /* HE 1x LTF + 0.8 us GI rx support */
	IEEE80211_PARAM_HE_MAX_NC_SUPRT            = 629,   /* HE Max-NC support */
	IEEE80211_PARAM_TWT_RESPONDER_SUPRT        = 630,   /* HE TWT responder support */
	IEEE80211_PARAM_CONFIG_CAPTURE_LATENCY_ENABLE  = 631, /* Cap tx latency */
	IEEE80211_PARAM_GET_RU26_TOLERANCE         = 632,   /* Get ru 26 intolerence status */
	IEEE80211_PARAM_WHC_APINFO_UPLINK_SNR      = 633, /* Get uplink SNR */
	IEEE80211_PARAM_DPP_VAP_MODE               = 634, /* Disable/Enable the DPP mode */
#if SM_ENG_HIST_ENABLE
	IEEE80211_PARAM_SM_HISTORY                 = 635,   /* Print all the VDEV SM history */
#endif
	IEEE80211_PARAM_RX_AMSDU                   = 652,   /* Toggle RX AMSDU info */
	IEEE80211_PARAM_MAX_MTU_SIZE               = 653,   /* set max mtu size*/
	IEEE80211_PARAM_HE_6GHZ_BCAST_PROB_RSP     = 654,   /* Enable/Disable 20 TU Probe Resp */
	IEEE80211_PARAM_VHT_MCS_10_11_SUPP         = 660, /* Overall VHT MCS 10/11 Support*/
	IEEE80211_PARAM_VHT_MCS_10_11_NQ2Q_PEER_SUPP = 661, /* Non-Q2Q Peer VHT MCS 10/11 Support*/
	IEEE80211_PARAM_SAE_PWID                     = 662, /* SAE PWD ID present*/
	IEEE80211_PARAM_MCAST_RC_STALE_PERIOD      = 663, /* Multicast RC stale period */
	IEEE80211_PARAM_HE_MULTI_TID_AGGR          = 664,
	IEEE80211_PARAM_HE_MULTI_TID_AGGR_TX       = 665,
	IEEE80211_PARAM_HE_MAX_AMPDU_LEN_EXP       = 666,
	IEEE80211_PARAM_HE_SU_PPDU_1X_LTF_800NS_GI = 667,
	IEEE80211_PARAM_HE_SU_MU_PPDU_4X_LTF_800NS_GI = 668,
	IEEE80211_PARAM_HE_MAX_FRAG_MSDU           = 669,
	IEEE80211_PARAM_HE_MIN_FRAG_SIZE           = 670,
	IEEE80211_PARAM_HE_OMI                     = 671,
	IEEE80211_PARAM_HE_NDP_4X_LTF_3200NS_GI    = 672,
	IEEE80211_PARAM_HE_ER_SU_PPDU_1X_LTF_800NS_GI = 673,
	IEEE80211_PARAM_HE_ER_SU_PPDU_4X_LTF_800NS_GI = 674,
	IEEE80211_PARAM_GET_MAX_RATE               = 675, /* Get Max supported BitRate on Kbps */
	IEEE80211_PARAM_GET_SIGNAL_LEVEL           = 676, /* Get Signal level on dBm */
	IEEE80211_PARAM_DEC_BCN_LOSS               = 677, /*Prioritize less beacon loss over passive scan*/
	IEEE80211_PARAM_ENABLE_MULTI_GROUP_KEY     = 678, /* Enable/Disable multiple group keys */
	IEEE80211_PARAM_MAX_GROUP_KEYS             = 679, /* Maximum number of Group keys */
	IEEE80211_PARAM_RRM_FILTER                 = 680, /* Send RRM packet to application */
	IEEE80211_PARAM_WNM_FILTER                 = 681, /* Send WNM packet to application */
	IEEE80211_PARAM_VENDOR_FRAME_FWD_MASK      = 682, /* subtype bitmask for mgmt frames which are forwarded to application */
	IEEE80211_PARAM_HE_DYNAMIC_MU_EDCA         = 683, /* Enable/Disable dynamic Mu edca selection */
	IEEE80211_PARAM_HE_AR_LDPC                 = 684, /* Enable/Disable Auto-Rate LDPC */
	IEEE80211_PARAM_ENABLE_MCAST_RC            = 685, /* Enable Mcast RC */
	IEEE80211_PARAM_VHT_MCS_12_13_SUPP         = 686, /* Overall HE MCS 12/13 Support*/
	IEEE80211_PARAM_RRM_CAP_IE                 = 687, /* Include RRM Cap IE in Assoc Request for STA mode */
	IEEE80211_PARAM_MAP2_BSTA_VLAN_ID          = 688, /* Multi Ap Primary VLAN Id for STA */
	IEEE80211_PARAM_SET_STATS_UPDATE_PERIOD    = 689, /* Set stats update period on legacy targets */
	IEEE80211_PARAM_VDEV_PEER_PROTOCOL_COUNT        = 690, /* Sets peer-prot count feature enable */
	IEEE80211_PARAM_VDEV_PEER_PROTOCOL_DROP_MASK    = 691, /* Sets peer-prot count feature dropmask */
#if WLAN_SCHED_HISTORY_SIZE
	IEEE80211_PARAM_WLAN_SCHED_HISTORY         = 692, /* Print scheduler history*/
#endif
	IEEE80211_PARAM_HE_DL_MU_OFDMA_BFER        = 693, /* Sets 11ax - HE DL MU OFDMA + TxBF*/
	IEEE80211_PARAM_SEND_PROBE_REQ             = 694, /* Send bcast probe request with current ssid */
	IEEE80211_PARAM_ASSOC_MIN_RSSI             = 695,
	IEEE80211_PARAM_OCE_TX_POWER               = 696, /* Enables tx power to be advertised as OCE attribute in Beacon and  Probe response frame */
	IEEE80211_PARAM_OCE_IP_SUBNET_ID           = 697, /* IP subnet identifier value to be advertised as OCE attribute in Beacon and  Probe response frame */
	IEEE80211_PARAM_OCE_ADD_ESS_RPT            = 698, /* Add ESS Report */
	IEEE80211_PARAM_ENABLE_MSCS                = 699, /* Enable MSCS */
	IEEE80211_PARAM_RSNX_OVERRIDE              = 700,
#if QCA_AIRTIME_FAIRNESS
	IEEE80211_PARAM_ATF_ENABLE_STATS           = 701,
	IEEE80211_PARAM_ATF_STATS_TIMEOUT          = 702,
#endif
	IEEE80211_PARAM_OCE_VERSION_OVERRIDE       = 703, /* Support to override OCE release version to 2*/
#if ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION
	IEEE80211_PARAM_RAWSIM_DEBUG_NUM_ENCAP_FRAMES   = 704, /* Sets the number of encap raw frames to dump when debug enabled */
	IEEE80211_PARAM_RAWSIM_DEBUG_NUM_DECAP_FRAMES   = 705, /* Sets the number of decap raw frames to dump when debug enabled */
#endif /* ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION */
	IEEE80211_PARAM_CURRENT_PP                 = 706, /* P Periodicity */
	IEEE80211_PARAM_NO_ACT_VAPS                = 707, /* Active Vaps */
	IEEE80211_PARAM_TX_VAP                     = 708, /* Current Tx Vap */
	IEEE80211_PARAM_FILS_IS_ENABLE            = 709, /* Fils enable frames*/
	IEEE80211_PARAM_MBSS_TXVDEV                = 710,
	IEEE80211_PARAM_IGMP_ME      = 711, /* Set IGMP Mcast enhancement option: 0 disable, 1 enable */
	IEEE80211_PARAM_HLOS_TID_OVERRIDE          = 712,   /* enable/disable hlos tid override support per vap */
	IEEE80211_PARAM_6G_HE_OP_MIN_RATE          = 713,  /* set HE sta minimum rate for its Tx PPDU in a bss */
	IEEE80211_PARAM_6G_SECURITY_COMP           = 714, /* 6G Security Compliance on/off */
	IEEE80211_PARAM_6G_KEYMGMT_MASK            = 715, /* 6G Key Mgmt Mask Config */
	IEEE80211_PARAM_HE_ER_SU_DISABLE            = 716,
	IEEE80211_PARAM_HE_1024QAM_LT242RU_RX_ENABLE = 717,
	IEEE80211_PARAM_HE_UL_MU_DATA_DIS_RX_SUPP  = 718,
	IEEE80211_PARAM_HE_FULL_BW_UL_MUMIMO       = 719,
	IEEE80211_PARAM_HE_DCM_MAX_CONSTELLATION_RX = 720,
#if WLAN_OBJMGR_REF_ID_TRACE
	IEEE80211_PARAM_VDEV_REF_LEAK_TEST          = 721,
#endif
	IEEE80211_PARAM_DISABLE_INACT_PROBING      = 722, /* Disable FW inactivity Probing behavior */
	IEEE80211_PARAM_PRB_RETRY                  = 723,   /* Set/Get probe-response frame retry limit */
	IEEE80211_PARAM_PEER_AUTHORIZE             = 724, /* Enable port authorization */
	IEEE80211_PARAM_FWD_ACTION_FRAMES_TO_APP   = 725,   /* Forward 11k/v frames to Hostapd instead of handling in driver */
	IEEE80211_PARAM_WNM_STATS                  = 726,   /* WNM Stats */
};

enum {
	OL_SPECIAL_PARAM_COUNTRY_ID,
	OL_SPECIAL_PARAM_ASF_AMEM_PRINT,
	OL_SPECIAL_DBGLOG_REPORT_SIZE,
	OL_SPECIAL_DBGLOG_TSTAMP_RESOLUTION,
	OL_SPECIAL_DBGLOG_REPORTING_ENABLED,
	OL_SPECIAL_DBGLOG_LOG_LEVEL,
	OL_SPECIAL_DBGLOG_VAP_ENABLE,
	OL_SPECIAL_DBGLOG_VAP_DISABLE,
	OL_SPECIAL_DBGLOG_MODULE_ENABLE,
	OL_SPECIAL_DBGLOG_MODULE_DISABLE,
	OL_SPECIAL_PARAM_DISP_TPC,
	OL_SPECIAL_PARAM_ENABLE_CH_144,
	OL_SPECIAL_PARAM_REGDOMAIN,
	OL_SPECIAL_PARAM_ENABLE_OL_STATS,
	OL_SPECIAL_PARAM_ENABLE_MAC_REQ,
	OL_SPECIAL_PARAM_ENABLE_SHPREAMBLE,
	OL_SPECIAL_PARAM_ENABLE_SHSLOT,
	OL_SPECIAL_PARAM_RADIO_MGMT_RETRY_LIMIT,
	OL_SPECIAL_PARAM_SENS_LEVEL,
	OL_SPECIAL_PARAM_TX_POWER_5G,
	OL_SPECIAL_PARAM_TX_POWER_2G,
	OL_SPECIAL_PARAM_CCA_THRESHOLD,
	OL_SPECIAL_PARAM_WLAN_PROFILE_ID_ENABLE,
	OL_SPECIAL_PARAM_WLAN_PROFILE_TRIGGER,
	OL_SPECIAL_PARAM_ENABLE_CH144_EPPR_OVRD,
	OL_SPECIAL_PARAM_ENABLE_PERPKT_TXSTATS,
	OL_SPECIAL_PARAM_ENABLE_OL_STATSv2,
	OL_SPECIAL_PARAM_ENABLE_OL_STATSv3,
	OL_SPECIAL_PARAM_BSTA_FIXED_IDMASK,
};

enum _ol_ath_param_t {
	OL_ATH_PARAM_TXCHAINMASK               = 1,
	OL_ATH_PARAM_RXCHAINMASK               = 2,
	OL_ATH_PARAM_AMPDU                     = 6,
	OL_ATH_PARAM_AMPDU_LIMIT               = 7,
	OL_ATH_PARAM_AMPDU_SUBFRAMES           = 8,
	OL_ATH_PARAM_TXPOWER_LIMIT2G           = 12,
	OL_ATH_PARAM_TXPOWER_LIMIT5G           = 13,
	OL_ATH_PARAM_LDPC                      = 32,
	OL_ATH_PARAM_VOW_EXT_STATS             = 45,
	OL_ATH_PARAM_DYN_TX_CHAINMASK          = 73,
	OL_ATH_PARAM_BURST_ENABLE              = 77,
	OL_ATH_PARAM_BURST_DUR                 = 78,
	OL_ATH_PARAM_BCN_BURST                 = 80,
	OL_ATH_PARAM_DCS                       = 82,
#if UMAC_SUPPORT_PERIODIC_PERFSTATS
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_ENAB   = 83,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_WIN    = 84,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT        = 85,
	OL_ATH_PARAM_PRDPERFSTAT_PER_ENAB      = 86,
	OL_ATH_PARAM_PRDPERFSTAT_PER_WIN       = 87,
	OL_ATH_PARAM_PRDPERFSTAT_PER           = 88,
#endif
	/* UMAC_SUPPORT_PERIODIC_PERFSTATS */
	OL_ATH_PARAM_TOTAL_PER                 = 89,
	/* set manual rate for rts frame */
	OL_ATH_PARAM_RTS_CTS_RATE              = 92,
	/* co channel interference threshold level */
	OL_ATH_PARAM_DCS_COCH_THR              = 93,
	/* transmit error threshold */
	OL_ATH_PARAM_DCS_TXERR_THR             = 94,
	/* phy error threshold */
	OL_ATH_PARAM_DCS_PHYERR_THR            = 95,
	/*
	 * The IOCTL number is 114, it is made 114, inorder to make the IOCTL
	 * number same as Direct-attach IOCTL.
	 * Please, don't change number. This IOCTL gets the Interface code path
	 * it should be either DIRECT-ATTACH or OFF-LOAD.
	 */
	OL_ATH_PARAM_GET_IF_ID                 = 114,
	/* Enable Acs back Ground Channel selection Scan timer in AP mode*/
	OL_ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN = 118,
	/* ACS scan timer value in Seconds */
	OL_ATH_PARAM_ACS_SCANTIME              = 119,
	/* Negligence Delta RSSI between two channel */
	OL_ATH_PARAM_ACS_RSSIVAR               = 120,
	/* Negligence Delta Channel load between two channel*/
	OL_ATH_PARAM_ACS_CHLOADVAR             = 121,
	/* Enable Limited OBSS check */
	OL_ATH_PARAM_ACS_LIMITEDOBSS           = 122,
	/* Acs control flag for Scan timer */
	OL_ATH_PARAM_ACS_CTRLFLAG              = 123,
	/* Acs Run time Debug level*/
	OL_ATH_PARAM_ACS_DEBUGTRACE            = 124,
	OL_ATH_PARAM_SET_FW_HANG_ID            = 137,
	/* Radio type 1:11ac 0:11abgn */
	OL_ATH_PARAM_RADIO_TYPE                = 138,
	OL_ATH_PARAM_IGMPMLD_OVERRIDE, /* IGMP/MLD packet override */
	OL_ATH_PARAM_IGMPMLD_TID, /* IGMP/MLD packet TID no */
	OL_ATH_PARAM_ARPDHCP_AC_OVERRIDE,
	OL_ATH_PARAM_NON_AGG_SW_RETRY_TH,
	OL_ATH_PARAM_AGG_SW_RETRY_TH,
	/* Dont change this number it as per sync with DA
	   Blocking certian channel from ic channel list */
	OL_ATH_PARAM_DISABLE_DFS   = 144,
	OL_ATH_PARAM_ENABLE_AMSDU  = 145,
	OL_ATH_PARAM_ENABLE_AMPDU  = 146,
	OL_ATH_PARAM_STA_KICKOUT_TH,
	OL_ATH_PARAM_WLAN_PROF_ENABLE,
	OL_ATH_PARAM_LTR_ENABLE,
	OL_ATH_PARAM_LTR_AC_LATENCY_BE = 150,
	OL_ATH_PARAM_LTR_AC_LATENCY_BK,
	OL_ATH_PARAM_LTR_AC_LATENCY_VI,
	OL_ATH_PARAM_LTR_AC_LATENCY_VO,
	OL_ATH_PARAM_LTR_AC_LATENCY_TIMEOUT,
	OL_ATH_PARAM_LTR_TX_ACTIVITY_TIMEOUT = 155,
	OL_ATH_PARAM_LTR_SLEEP_OVERRIDE,
	OL_ATH_PARAM_LTR_RX_OVERRIDE,
	OL_ATH_PARAM_L1SS_ENABLE,
	OL_ATH_PARAM_DSLEEP_ENABLE,
	/* radar error threshold */
	OL_ATH_PARAM_DCS_RADAR_ERR_THR = 160,
	/* Tx channel utilization due to AP's tx and rx */
	OL_ATH_PARAM_DCS_USERMAX_CU_THR,
	/* interference detection threshold */
	OL_ATH_PARAM_DCS_INTR_DETECT_THR,
	/* sampling window, default 10secs */
	OL_ATH_PARAM_DCS_SAMPLE_WINDOW,
	/* debug logs enable/disable */
	OL_ATH_PARAM_DCS_DEBUG,
	OL_ATH_PARAM_ANI_ENABLE = 165,
	OL_ATH_PARAM_ANI_POLL_PERIOD,
	OL_ATH_PARAM_ANI_LISTEN_PERIOD,
	OL_ATH_PARAM_ANI_OFDM_LEVEL,
	OL_ATH_PARAM_ANI_CCK_LEVEL,
	OL_ATH_PARAM_DSCP_TID_MAP = 170,
	OL_ATH_PARAM_TXPOWER_SCALE,
	/* Phy error penalty */
	OL_ATH_PARAM_DCS_PHYERR_PENALTY,
#if ATH_SUPPORT_DSCP_OVERRIDE
	/* set/get TID for sending HMMC packets */
	OL_ATH_PARAM_HMMC_DSCP_TID_MAP,
	/* set/get DSCP mapping override */
	OL_ATH_PARAM_DSCP_OVERRIDE,
	/* set/get HMMC-DSCP mapping override */
	OL_ATH_PARAM_HMMC_DSCP_OVERRIDE = 175,
#endif
#if ATH_RX_LOOPLIMIT_TIMER
	OL_ATH_PARAM_LOOPLIMIT_NUM,
#endif
	OL_ATH_PARAM_ANTENNA_GAIN_2G,
	OL_ATH_PARAM_ANTENNA_GAIN_5G,
	OL_ATH_PARAM_RX_FILTER,
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
	OL_ATH_PARAM_BUFF_THRESH = 180,
	OL_ATH_PARAM_BLK_REPORT_FLOOD,
	OL_ATH_PARAM_DROP_STA_QUERY,
#endif
	OL_ATH_PARAM_QBOOST,
	OL_ATH_PARAM_SIFS_FRMTYPE,
	OL_ATH_PARAM_SIFS_UAPSD = 185,
	OL_ATH_PARAM_FW_RECOVERY_ID,
	OL_ATH_PARAM_RESET_OL_STATS,
	OL_ATH_PARAM_AGGR_BURST,
	/* Number of deauth sent in consecutive rx_peer_invalid */
	OL_ATH_PARAM_DEAUTH_COUNT,
	OL_ATH_PARAM_BLOCK_INTERBSS = 190,
	/* Firmware reset control for Bmiss / timeout / reset */
	OL_ATH_PARAM_FW_DISABLE_RESET,
	OL_ATH_PARAM_MSDU_TTL,
	OL_ATH_PARAM_PPDU_DURATION,
	OL_ATH_PARAM_SET_TXBF_SND_PERIOD,
	OL_ATH_PARAM_ALLOW_PROMISC = 195,
	OL_ATH_PARAM_BURST_MODE,
	OL_ATH_PARAM_DYN_GROUPING,
	OL_ATH_PARAM_DPD_ENABLE,
	OL_ATH_PARAM_DBGLOG_RATELIM,
	/* firmware should intimate us about ps state change for node  */
	OL_ATH_PARAM_PS_STATE_CHANGE = 200,
	OL_ATH_PARAM_MCAST_BCAST_ECHO,
	/* OBSS RSSI threshold for 20/40 coexistence */
	OL_ATH_PARAM_OBSS_RSSI_THRESHOLD,
	/* Link/node RX RSSI threshold  for 20/40 coexistence */
	OL_ATH_PARAM_OBSS_RX_RSSI_THRESHOLD,
#if ATH_CHANNEL_BLOCKING
	OL_ATH_PARAM_ACS_BLOCK_MODE = 205,
#endif
	OL_ATH_PARAM_ACS_TX_POWER_OPTION,
	/*
	 * Default Antenna Polarization MSB 8 bits (24:31) specifying
	 * enable/disable ; LSB 24 bits (0:23) antenna mask value
	 */
	OL_ATH_PARAM_ANT_POLARIZATION,
	/* rate limit mute type error prints */
	OL_ATH_PARAM_PRINT_RATE_LIMIT,
	OL_ATH_PARAM_PDEV_RESET,   /* Reset FW PDEV*/
	/* Do not crash host when target assert happened*/
	OL_ATH_PARAM_FW_DUMP_NO_HOST_CRASH = 210,
	/* Consider OBSS non-erp to change to long slot*/
	OL_ATH_PARAM_CONSIDER_OBSS_NON_ERP_LONG_SLOT = 211,
	OL_ATH_PARAM_STATS_FC,
	OL_ATH_PARAM_QFLUSHINTERVAL,
	OL_ATH_PARAM_TOTAL_Q_SIZE,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE1,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE2,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE3,
	OL_ATH_PARAM_MIN_THRESHOLD,
	OL_ATH_PARAM_MAX_Q_LIMIT,
	OL_ATH_PARAM_MIN_Q_LIMIT,
	OL_ATH_PARAM_CONG_CTRL_TIMER_INTV,
	OL_ATH_PARAM_STATS_TIMER_INTV,
	OL_ATH_PARAM_ROTTING_TIMER_INTV,
	OL_ATH_PARAM_LATENCY_PROFILE,
	OL_ATH_PARAM_HOSTQ_DUMP,
	OL_ATH_PARAM_TIDQ_MAP,
	OL_ATH_PARAM_DBG_ARP_SRC_ADDR, /* ARP DEBUG source address*/
	OL_ATH_PARAM_DBG_ARP_DST_ADDR, /* ARP DEBUG destination address*/
	OL_ATH_PARAM_ARP_DBG_CONF,   /* ARP debug configuration */
	OL_ATH_PARAM_DISABLE_STA_VAP_AMSDU, /* Disable AMSDU for station vap */
#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
	OL_ATH_PARAM_STADFS_ENABLE = 300,    /* STA DFS is enabled or not  */
#endif
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_STRICT_SCHED = 301,
	OL_ATH_PARAM_ATF_GROUP_POLICY = 302,
#endif
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_PRIMARY_RADIO = 303,
	OL_ATH_PARAM_DBDC_ENABLE = 304,
#endif
	OL_ATH_PARAM_TXPOWER_DBSCALE,
	OL_ATH_PARAM_CTL_POWER_SCALE,
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_OBSS_SCHED = 307,
#endif
	OL_ATH_PARAM_PHY_OFDM_ERR = 309,
	OL_ATH_PARAM_PHY_CCK_ERR = 310,
	OL_ATH_PARAM_FCS_ERR = 311,
	OL_ATH_PARAM_CHAN_UTIL = 312,
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_CLIENT_MCAST,
#endif
	OL_ATH_PARAM_EMIWAR_80P80 = 314,
	OL_ATH_PARAM_BATCHMODE = 315,
	OL_ATH_PARAM_PACK_AGGR_DELAY = 316,
#if UMAC_SUPPORT_ACFG
	OL_ATH_PARAM_DIAG_ENABLE = 317,
#endif
#if ATH_SUPPORT_VAP_QOS
	OL_ATH_PARAM_VAP_QOS = 318,
#endif
	OL_ATH_PARAM_CHAN_STATS_TH = 319,
	/* Passive scan is enabled or disabled  */
	OL_ATH_PARAM_PASSIVE_SCAN_ENABLE = 320,
	OL_ATH_MIN_RSSI_ENABLE = 321,
	OL_ATH_MIN_RSSI = 322,
	OL_ATH_PARAM_ACS_2G_ALLCHAN = 323,
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_DELAY_STAVAP_UP = 324,
#endif
	/* It is used to set the channel switch options */
	OL_ATH_PARAM_CHANSWITCH_OPTIONS = 327,
	OL_ATH_BTCOEX_ENABLE        = 328,
	OL_ATH_BTCOEX_WL_PRIORITY   = 329,
	OL_ATH_PARAM_TID_OVERRIDE_QUEUE_MAPPING = 330,
	OL_ATH_PARAM_CAL_VER_CHECK = 331,
	OL_ATH_PARAM_NO_VLAN       = 332,
	OL_ATH_PARAM_CCA_THRESHOLD = 333,
	OL_ATH_PARAM_ATF_LOGGING = 334,
	OL_ATH_PARAM_STRICT_DOTH = 335,
	OL_ATH_PARAM_DISCONNECTION_TIMEOUT   = 336,
	OL_ATH_PARAM_RECONFIGURATION_TIMEOUT = 337,
	OL_ATH_PARAM_CHANNEL_SWITCH_COUNT = 338,
	OL_ATH_PARAM_ALWAYS_PRIMARY = 339,
	OL_ATH_PARAM_FAST_LANE = 340,
	OL_ATH_GET_BTCOEX_DUTY_CYCLE = 341,
	OL_ATH_PARAM_SECONDARY_OFFSET_IE = 342,
	OL_ATH_PARAM_WIDE_BAND_SUB_ELEMENT = 343,
	OL_ATH_PARAM_PREFERRED_UPLINK = 344,
	OL_ATH_PARAM_PRECAC_ENABLE = 345,
	OL_ATH_PARAM_PRECAC_TIMEOUT = 346,
	OL_ATH_COEX_VER_CFG = 347,
	OL_ATH_PARAM_DUMP_TARGET = 348,
	OL_ATH_PARAM_PDEV_TO_REO_DEST = 349,
	OL_ATH_PARAM_DUMP_CHAINMASK_TABLES = 350,
	OL_ATH_PARAM_DUMP_OBJECTS = 351,
	OL_ATH_PARAM_ACS_SRLOADVAR = 352,
	OL_ATH_PARAM_MGMT_RSSI_THRESHOLD = 353,
	OL_ATH_PARAM_EXT_NSS_CAPABLE = 354,
	OL_ATH_PARAM_MGMT_PDEV_STATS_TIMER = 355,
	OL_ATH_PARAM_TXACKTIMEOUT = 356,
	OL_ATH_PARAM_ICM_ACTIVE = 357,
	OL_ATH_PARAM_NOMINAL_NOISEFLOOR = 358,
	OL_ATH_PARAM_CHAN_INFO = 359,
	OL_ATH_PARAM_ACS_RANK = 360,
	OL_ATH_PARAM_TXCHAINSOFT = 361,
	OL_ATH_PARAM_WIDE_BAND_SCAN = 362,
	OL_ATH_PARAM_CCK_TX_ENABLE = 363,
	OL_ATH_PARAM_PAPI_ENABLE = 364,
	OL_ATH_PARAM_ISOLATION = 365,
	OL_ATH_PARAM_MAX_CLIENTS_PER_RADIO = 366,
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
	OL_ATH_PARAM_DFS_HOST_WAIT_TIMEOUT = 367,
#endif
	OL_ATH_PARAM_NF_THRESH = 368,
#ifdef OL_ATH_SMART_LOGGING
	OL_ATH_PARAM_SMARTLOG_ENABLE = 369,
	OL_ATH_PARAM_SMARTLOG_FATAL_EVENT = 370,
	OL_ATH_PARAM_SMARTLOG_SKB_SZ = 371,
	OL_ATH_PARAM_SMARTLOG_P1PINGFAIL = 372,
#endif /* OL_ATH_SMART_LOGGING */
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
	OL_ATH_PARAM_PRECAC_INTER_CHANNEL = 373,
	OL_ATH_PARAM_PRECAC_CHAN_STATE = 374,
#endif
	OL_ATH_PARAM_DBR_RING_STATUS = 375,
#ifdef QCN_ESP_IE
	OL_ATH_PARAM_ESP_PERIODICITY = 376,
	OL_ATH_PARAM_ESP_AIRTIME = 377,
	OL_ATH_PARAM_ESP_PPDU_DURATION = 378,
	OL_ATH_PARAM_ESP_BA_WINDOW = 379,
#endif /* QCN_ESP_IE */

	OL_ATH_PARAM_CBS = 380,
	OL_ATH_PARAM_DCS_SIM = 381,
	OL_ATH_PARAM_CBS_DWELL_SPLIT_TIME = 382,
	OL_ATH_PARAM_CBS_DWELL_REST_TIME = 383,
	OL_ATH_PARAM_CBS_WAIT_TIME = 384,
	OL_ATH_PARAM_CBS_REST_TIME = 385,
	OL_ATH_PARAM_CBS_CSA = 386,
	OL_ATH_PARAM_TWICE_ANTENNA_GAIN = 387,
	OL_ATH_PARAM_ACTIVITY_FACTOR = 388,
	OL_ATH_PARAM_CHAN_AP_RX_UTIL = 389,
	OL_ATH_PARAM_CHAN_FREE  = 390,
	OL_ATH_PARAM_CHAN_AP_TX_UTIL = 391,
	OL_ATH_PARAM_CHAN_OBSS_RX_UTIL = 392,
	OL_ATH_PARAM_CHAN_NON_WIFI = 393,
#if PEER_FLOW_CONTROL
	OL_ATH_PARAM_VIDEO_STATS_FC = 394,
	OL_ATH_PARAM_VIDEO_DELAY_STATS_FC = 395,
#endif
	OL_ATH_PARAM_ENABLE_PEER_RETRY_STATS = 396,
	OL_ATH_PARAM_HE_UL_TRIG_INT = 397,
	OL_ATH_PARAM_DFS_NOL_SUBCHANNEL_MARKING = 398,
	/*
	 * Get the band that is tuned for low, high,
	 * full band freq range or it's 2g
	 */
	OL_ATH_PARAM_BAND_INFO = 399,
	OL_ATH_PARAM_BW_REDUCE = 400,
	/* Enable/disable Spatial Reuse */
	OL_ATH_PARAM_HE_SR = 401,
	OL_ATH_PARAM_HE_UL_PPDU_DURATION = 402,
	OL_ATH_PARAM_HE_UL_RU_ALLOCATION = 403,
	OL_ATH_PARAM_PERIODIC_CFR_CAPTURE = 404,
	OL_ATH_PARAM_FLUSH_PEER_RATE_STATS = 405,
	OL_ATH_PARAM_DCS_RE_ENABLE_TIMER = 406,
	/* Enable/disable Rx lite monitor mode */
	OL_ATH_PARAM_RX_MON_LITE = 407,
	/* wifi down indication used in MBSS feature */
	OL_ATH_PARAM_WIFI_DOWN_IND = 408,
	OL_ATH_PARAM_TX_CAPTURE = 409,
	/* Enable fw dump collectin if wmi disconnects */
	OL_ATH_PARAM_WMI_DIS_DUMP = 410,
	OL_ATH_PARAM_ACS_CHAN_GRADE_ALGO = 411,
	OL_ATH_PARAM_ACS_CHAN_EFFICIENCY_VAR = 412,
	OL_ATH_PARAM_ACS_NEAR_RANGE_WEIGHTAGE = 413,
	OL_ATH_PARAM_ACS_MID_RANGE_WEIGHTAGE = 414,
	OL_ATH_PARAM_ACS_FAR_RANGE_WEIGHTAGE = 415,
	/* Enable/Disable/Set MGMT_TTL in milliseconds. */
	OL_ATH_PARAM_MGMT_TTL = 417,
	/* Enable/Disable/Set PROBE_RESP_TTL in milliseconds */
	OL_ATH_PARAM_PROBE_RESP_TTL = 418,
	/* Set global MU PPDU duration for DL (usec units) */
	OL_ATH_PARAM_MU_PPDU_DURATION = 419,
	/* Set TBTT_CTRL_CFG */
	OL_ATH_PARAM_TBTT_CTRL = 420,
	/* Get baseline radio level channel width */
	OL_ATH_PARAM_RCHWIDTH = 422,
	/* Whether external ACS request is in progress */
	OL_ATH_EXT_ACS_REQUEST_IN_PROGRESS = 423,
	/* set/get hw mode */
	OL_ATH_PARAM_HW_MODE  = 424,
#if DBDC_REPEATER_SUPPORT
	/* same ssid feature global disable */
	OL_ATH_PARAM_SAME_SSID_DISABLE = 425,
#endif
	/* get MBSS enable flag */
	OL_ATH_PARAM_MBSS_EN  = 426,
	/* UNII-1 and UNII-2A channel coexistance */
	OL_ATH_PARAM_CHAN_COEX = 427,
	/* Out of Band Advertisement feature */
	OL_ATH_PARAM_OOB_ENABLE = 428,
	/* set/get opmode-notification timer for hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_OMN_TIMER = 429,
	/* enable opmode-notification when doing hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_OMN_ENABLE = 430,
	/* set primary interface for hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_PRIMARY_IF = 431,
	/* Number of vdevs configured per PSOC */
	OL_ATH_PARAM_GET_PSOC_NUM_VDEVS = 432,
	/* Number of peers configured per PSOC */
	OL_ATH_PARAM_GET_PSOC_NUM_PEERS = 433,
	/* Number of vdevs configured per PDEV */
	OL_ATH_PARAM_GET_PDEV_NUM_VDEVS = 434,
	/* Number of peers configured per PDEV */
	OL_ATH_PARAM_GET_PDEV_NUM_PEERS = 435,
	/* Number of monitor vdevs configured per PDEV */
	OL_ATH_PARAM_GET_PDEV_NUM_MONITOR_VDEVS = 436,
#ifdef CE_TASKLET_DEBUG_ENABLE
	/* Enable/disable CE stats print */
	OL_ATH_PARAM_ENABLE_CE_LATENCY_STATS = 437,
#endif
	/* set operating class table (MBO) value. */
	OL_ATH_PARAM_OPCLASS_TBL = 438,
#ifdef QCA_SUPPORT_ADFS_RCAC
	/* enable/disable rolling cac feature */
	OL_ATH_PARAM_ROLLING_CAC_ENABLE = 439,
	/* configure a frequency for rolling CAC */
	OL_ATH_PARAM_CONFIGURE_RCAC_FREQ = 440,
#endif
#if ATH_SUPPORT_DFS
	/* Prioritize scan over CAC time */
	OL_ATH_SCAN_OVER_CAC = 441,
#endif
	OL_ATH_PARAM_CFR_CAPTURE_STATUS = 442,
	/* Non-inheritance IE enable */
	OL_ATH_PARAM_NON_INHERIT_ENABLE = 443,
	/* Set/Get next frequency for radar */
	OL_ATH_PARAM_NXT_RDR_FREQ = 444,
	/* set the flag for a radio with no backhaul */
	OL_ATH_PARAM_NO_BACKHAUL_RADIO = 445,
	/* MAX_PHY feature flag per PDEV */
	OL_ATH_PARAM_RPT_MAX_PHY = 446,
	/* Enable additional operating triplets in the Country IE */
	OL_ATH_PARAM_ENABLE_ADDITIONAL_TRIPLETS = 447,
#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
	/* Set the primary frequency for the AP to go back to, after NOL */
	OL_ATH_DFS_CHAN_POSTNOL_FREQ = 448,
	/* Set the mode for the AP to go back to, after NOL */
	OL_ATH_DFS_CHAN_POSTNOL_MODE = 449,
	/* Set the secondary center frequency for the post NOL channel. */
	OL_ATH_DFS_CHAN_POSTNOL_CFREQ2 = 450,
#endif
	/* DCS Wideband (5-7GHz) policy */
	OL_ATH_PARAM_DCS_WIDEBAND_POLICY = 451,
#ifdef QCA_CBT_INSTRUMENTATION
	/* Support get_call_map() for CBT */
	OL_ATH_PARAM_FUNC_CALL_MAP = 452,
#endif
	/* Configure punctured band setting */
	OL_ATH_PARAM_PUNCTURED_BAND = 453,
	/* Control frame configuration for MBSSID */
	OL_ATH_PARAM_HE_MBSSID_CTRL_FRAME_CONFIG = 454,
	/* Max users per-PPDU for OFDMA, 16 LSBs for DL and 16 MSBs for UL */
	OL_ATH_PARAM_OFDMA_MAX_USERS = 455,
	/* Max users per-PPDU for MU-MIMO, 16 LSBs for DL and 16 MSBs for UL */
	OL_ATH_PARAM_MUMIMO_MAX_USERS = 456,
	/* ACS pre-CAC only channel selection support */
	OL_ATH_PARAM_ACS_PRECAC_SUPPORT = 457,
	/* MBSSID AUTO MODE TX VDEV derivation */
	OL_ATH_PARAM_MBSS_AUTOMODE = 458,
	/* RNR selective addition */
	OL_ATH_PARAM_RNR_SELECTIVE_ADD = 459,
	/* RNR Unsolicited Probe Response Active Field */
	OL_ATH_PARAM_RNR_UNSOLICITED_PROBE_RESP_ACTIVE = 460,
	/* RNR Member of ESS with 2.4G/5G co-located AP */
	OL_ATH_PARAM_RNR_MEMBER_OF_ESS_24G_5G_CO_LOCATED = 461,
	/* RNR stats */
	OL_ATH_PARAM_RNR_STATS = 462,
	/* NSS WiFi offload status */
	OL_ATH_PARAM_NSS_WIFI_OFFLOAD_STATUS = 463,
	/* enable EMA */
	OL_ATH_PARAM_ENABLE_EMA = 464,
	/* enable Tx Mode Sselect */
	OL_ATH_PARAM_ENABLE_TX_MODE_SELECT = 465,
};

#ifdef CONFIG_SUPPORT_LIBROXML
struct vendor_commands vap_vendor_cmds[] = {
	{"turbo",               IEEE80211_PARAM_TURBO, SET_PARAM, 1},
	{"get_turbo",           IEEE80211_PARAM_TURBO, GET_PARAM, 0},
	{"authmode",            IEEE80211_PARAM_AUTHMODE, SET_PARAM, 1},
	{"get_authmode",        IEEE80211_PARAM_AUTHMODE, GET_PARAM, 0},
	{"protmode",            IEEE80211_PARAM_PROTMODE, SET_PARAM, 1},
	{"get_protmode",        IEEE80211_PARAM_PROTMODE, GET_PARAM, 0},
	{"mcastcipher",         IEEE80211_PARAM_MCASTCIPHER, SET_PARAM, 1},
	{"get_mcastcipher",     IEEE80211_PARAM_MCASTCIPHER, GET_PARAM, 0},
	{"mcastkeylen",         IEEE80211_PARAM_MCASTKEYLEN, SET_PARAM, 1},
	{"get_mcastkeylen",     IEEE80211_PARAM_MCASTKEYLEN, GET_PARAM, 0},
	{"ucastciphers",        IEEE80211_PARAM_UCASTCIPHERS, SET_PARAM, 1},
	{"get_uciphers",        IEEE80211_PARAM_UCASTCIPHERS, GET_PARAM, 0},
	{"ucastcipher",         IEEE80211_PARAM_UCASTCIPHER, SET_PARAM, 1},
	{"get_ucastcipher",     IEEE80211_PARAM_UCASTCIPHER, GET_PARAM, 0},
	{"ucastkeylen",         IEEE80211_PARAM_UCASTKEYLEN, SET_PARAM, 1},
	{"get_ucastkeylen",     IEEE80211_PARAM_UCASTKEYLEN, GET_PARAM, 0},
	{"wpa",                 IEEE80211_PARAM_WPA, SET_PARAM, 1},
	{"get_wpa",             IEEE80211_PARAM_WPA, GET_PARAM, 0},
	{"privacy",             IEEE80211_PARAM_PRIVACY, SET_PARAM, 1},
	{"get_privacy",         IEEE80211_PARAM_PRIVACY, GET_PARAM, 0},
	{"countermeasures",     IEEE80211_PARAM_COUNTERMEASURES, SET_PARAM, 1},
	{"get_countermeas",     IEEE80211_PARAM_COUNTERMEASURES, GET_PARAM, 0},
	{"dropunencrypted",     IEEE80211_PARAM_DROPUNENCRYPTED, SET_PARAM, 1},
	{"get_dropunencry",     IEEE80211_PARAM_DROPUNENCRYPTED, GET_PARAM, 0},
	{"driver_caps",         IEEE80211_PARAM_DRIVER_CAPS, SET_PARAM, 1},
	{"get_driver_caps",     IEEE80211_PARAM_DRIVER_CAPS, GET_PARAM, 0},
	{"maccmd",              IEEE80211_PARAM_MACCMD, SET_PARAM, 1},
	{"get_maccmd",          IEEE80211_PARAM_MACCMD, GET_PARAM, 0},
	{"wmm",                 IEEE80211_PARAM_WMM, SET_PARAM, 1},
	{"get_wmm",             IEEE80211_PARAM_WMM, GET_PARAM, 0},
	{"hide_ssid",           IEEE80211_PARAM_HIDESSID, SET_PARAM, 1},
	{"get_hide_ssid",       IEEE80211_PARAM_HIDESSID, GET_PARAM, 0},
	{"ap_bridge",           IEEE80211_PARAM_APBRIDGE, SET_PARAM, 1},
	{"get_ap_bridge",       IEEE80211_PARAM_APBRIDGE, GET_PARAM, 0},
	{"keymgtalgs",          IEEE80211_PARAM_KEYMGTALGS, SET_PARAM, 1},
	{"get_keymgtalgs",      IEEE80211_PARAM_KEYMGTALGS, GET_PARAM, 0},
	{"rsncaps",             IEEE80211_PARAM_RSNCAPS, SET_PARAM, 1},
	{"get_rsncaps",         IEEE80211_PARAM_RSNCAPS, GET_PARAM, 0},
	{"inact",               IEEE80211_PARAM_INACT, SET_PARAM, 1},
	{"get_inact",           IEEE80211_PARAM_INACT, GET_PARAM, 0},
	{"inact_auth",          IEEE80211_PARAM_INACT_AUTH, SET_PARAM, 1},
	{"get_inact_auth",      IEEE80211_PARAM_INACT_AUTH, GET_PARAM, 0},
	{"inact_init",          IEEE80211_PARAM_INACT_INIT, SET_PARAM, 1},
	{"get_inact_init",      IEEE80211_PARAM_INACT_INIT, GET_PARAM, 0},
	{"dtim_period",         IEEE80211_PARAM_DTIM_PERIOD, SET_PARAM, 1},
	{"get_dtim_period",     IEEE80211_PARAM_DTIM_PERIOD, GET_PARAM, 0},
	{"bintval",             IEEE80211_PARAM_BEACON_INTERVAL, SET_PARAM, 1},
	{"get_bintval",         IEEE80211_PARAM_BEACON_INTERVAL, GET_PARAM, 0},
	{"doth",                IEEE80211_PARAM_DOTH, SET_PARAM, 1},
	{"get_doth",            IEEE80211_PARAM_DOTH, GET_PARAM, 0},
	{"doth_pwrtgt",         IEEE80211_PARAM_PWRTARGET, SET_PARAM, 1},
	{"get_doth_pwrtgt",     IEEE80211_PARAM_PWRTARGET, GET_PARAM, 0},
	{"doth_reassoc",        IEEE80211_PARAM_GENREASSOC, SET_PARAM, 1},
	{"compression",         IEEE80211_PARAM_COMPRESSION, SET_PARAM, 1},
	{"get_compression",     IEEE80211_PARAM_COMPRESSION, GET_PARAM, 0},
	{"ff",                  IEEE80211_PARAM_FF, SET_PARAM, 1},
	{"get_ff",              IEEE80211_PARAM_FF, GET_PARAM, 0},
	{"burst",               IEEE80211_PARAM_BURST, SET_PARAM, 1},
	{"get_burst",           IEEE80211_PARAM_BURST, GET_PARAM, 0},
	{"pureg",               IEEE80211_PARAM_PUREG, SET_PARAM, 1},
	{"get_pureg",           IEEE80211_PARAM_PUREG, GET_PARAM, 0},
	{"ar",                  IEEE80211_PARAM_AR, SET_PARAM, 1},
	{"get_ar",              IEEE80211_PARAM_AR, GET_PARAM, 0},
	{"wds",                 IEEE80211_PARAM_WDS, SET_PARAM, 1},
	{"get_wds",             IEEE80211_PARAM_WDS, GET_PARAM, 0},
	{"mcast_rate",          IEEE80211_PARAM_MCAST_RATE, SET_PARAM, 1},
	{"get_mcast_rate",      IEEE80211_PARAM_MCAST_RATE, GET_PARAM, 0},
	{"countryie",           IEEE80211_PARAM_COUNTRY_IE, SET_PARAM, 1},
	{"get_countryie",       IEEE80211_PARAM_COUNTRY_IE, GET_PARAM, 0},
	{"scanvalid",           IEEE80211_PARAM_SCANVALID, SET_PARAM, 1},
	{"get_scanvalid",       IEEE80211_PARAM_SCANVALID, GET_PARAM, 0},
	{"uapsd",               IEEE80211_PARAM_UAPSDINFO, SET_PARAM, 1},
	{"get_uapsd",           IEEE80211_PARAM_UAPSDINFO, GET_PARAM, 0},
	{"sleep",               IEEE80211_PARAM_SLEEP, SET_PARAM, 1},
	{"get_sleep",           IEEE80211_PARAM_SLEEP, GET_PARAM, 0},
	{"qosnull",             IEEE80211_PARAM_QOSNULL, SET_PARAM, 1},
	{"pspoll",              IEEE80211_PARAM_PSPOLL, SET_PARAM, 1},
	{"eospdrop",            IEEE80211_PARAM_EOSPDROP, SET_PARAM, 1},
	{"get_eospdrop",        IEEE80211_PARAM_EOSPDROP, GET_PARAM, 0},
	{"chanbw",              IEEE80211_PARAM_CHANBW, SET_PARAM, 1},
	{"get_chanbw",          IEEE80211_PARAM_CHANBW, GET_PARAM, 0},
	{"shpreamble",          IEEE80211_PARAM_SHORTPREAMBLE, SET_PARAM, 1},
	{"get_shpreamble",      IEEE80211_PARAM_SHORTPREAMBLE, GET_PARAM, 0},
	{"extprotmode",         IEEE80211_PARAM_CWM_EXTPROTMODE, SET_PARAM, 1},
	{"get_extprotmode",     IEEE80211_PARAM_CWM_EXTPROTMODE, GET_PARAM, 0},
	{"extprotspac",         IEEE80211_PARAM_CWM_EXTPROTSPACING, SET_PARAM, 1},
	{"get_extprotspac",     IEEE80211_PARAM_CWM_EXTPROTSPACING, GET_PARAM, 0},
	{"cwmenable",           IEEE80211_PARAM_CWM_ENABLE, SET_PARAM, 1},
	{"get_cwmenable",       IEEE80211_PARAM_CWM_ENABLE, GET_PARAM, 0},
	{"extbusythres",        IEEE80211_PARAM_CWM_EXTBUSYTHRESHOLD, SET_PARAM, 1},
	{"g_extbusythres",      IEEE80211_PARAM_CWM_EXTBUSYTHRESHOLD, GET_PARAM, 0},
	{"shortgi",             IEEE80211_PARAM_SHORT_GI, SET_PARAM, 1},
	{"get_shortgi",         IEEE80211_PARAM_SHORT_GI, GET_PARAM, 0},
	{"ampdu",               IEEE80211_PARAM_AMPDU, SET_PARAM, 1},
	{"get_ampdu",           IEEE80211_PARAM_AMPDU, GET_PARAM, 0},
	{"amsdu",               IEEE80211_PARAM_AMSDU, SET_PARAM, 1},
	{"get_amsdu",           IEEE80211_PARAM_AMSDU, GET_PARAM, 0},
	{"get_countrycode",     IEEE80211_PARAM_COUNTRYCODE, GET_PARAM, 0},
	{"reset",               IEEE80211_PARAM_RESET_ONCE, SET_PARAM, 1},
	{"setaddbaoper",        IEEE80211_PARAM_SETADDBAOPER, SET_PARAM, 1},
	{"set11NRates",         IEEE80211_PARAM_11N_RATE, SET_PARAM, 1},
	{"get11NRates",         IEEE80211_PARAM_11N_RATE, GET_PARAM, 0},
	{"set11NRetries",       IEEE80211_PARAM_11N_RETRIES, SET_PARAM, 1},
	{"get11NRetries",       IEEE80211_PARAM_11N_RETRIES, GET_PARAM, 0},
	{"dbgLVL",              IEEE80211_PARAM_DBG_LVL, SET_PARAM, 1},
	{"wdsdetect",           IEEE80211_PARAM_WDS_AUTODETECT, SET_PARAM, 1},
	{"get_wdsdetect",       IEEE80211_PARAM_WDS_AUTODETECT, GET_PARAM, 0},
	{"ignore11d",           IEEE80211_PARAM_IGNORE_11DBEACON, SET_PARAM, 1},
	{"get_ignore11d",       IEEE80211_PARAM_IGNORE_11DBEACON, GET_PARAM, 0},
	{"stafwd",              IEEE80211_PARAM_STA_FORWARD, SET_PARAM, 1},
	{"get_stafwd",          IEEE80211_PARAM_STA_FORWARD, GET_PARAM, 0},
	{"mcastenhance",        IEEE80211_PARAM_ME, SET_PARAM, 1},
	{"g_mcastenhance",      IEEE80211_PARAM_ME, GET_PARAM, 0},
	{"medump_dummy",        95, SET_PARAM, 1},
	{"medump",              95, GET_PARAM, 0},
	{"medebug",             96, SET_PARAM, 1},
	{"get_medebug",         96, GET_PARAM, 0},
	{"me_length",           97, SET_PARAM, 1},
	{"get_me_length",       97, GET_PARAM, 0},
	{"metimer",             98, SET_PARAM, 1},
	{"get_metimer",         98, GET_PARAM, 0},
	{"metimeout",           99, SET_PARAM, 1},
	{"get_metimeout",       99, GET_PARAM, 0},
	{"puren",               IEEE80211_PARAM_PUREN, SET_PARAM, 1},
	{"get_puren",           IEEE80211_PARAM_PUREN, GET_PARAM, 0},
	{"basicrates",          IEEE80211_PARAM_BASICRATES, SET_PARAM, 1},
	{"htweptkip",           IEEE80211_PARAM_WEP_TKIP_HT, SET_PARAM, 1},
	{"get_htweptkip",       IEEE80211_PARAM_WEP_TKIP_HT, GET_PARAM, 0},
	{"powersave",           IEEE80211_PARAM_NETWORK_SLEEP, SET_PARAM, 1},
	{"get_powersave",       IEEE80211_PARAM_NETWORK_SLEEP, GET_PARAM, 0},
	{"chextoffset",         IEEE80211_PARAM_CHEXTOFFSET, SET_PARAM, 1},
	{"chscaninit",          IEEE80211_PARAM_CHSCANINIT, SET_PARAM, 1},
	{"get_chscaninit",      IEEE80211_PARAM_CHSCANINIT, GET_PARAM, 0},
	{"ht40intol",           IEEE80211_PARAM_HT40_INTOLERANT, SET_PARAM, 1},
	{"get_ht40intol",       IEEE80211_PARAM_HT40_INTOLERANT, GET_PARAM, 0},
	{"chwidth",             IEEE80211_PARAM_CHWIDTH, SET_PARAM, 1},
	{"get_chwidth",         IEEE80211_PARAM_CHWIDTH, GET_PARAM, 0},
	{"extap",               IEEE80211_PARAM_EXTAP, SET_PARAM, 1},
	{"get_extap",           IEEE80211_PARAM_EXTAP, GET_PARAM, 0},
	{"disablecoext",        IEEE80211_PARAM_COEXT_DISABLE, SET_PARAM, 1},
	{"g_disablecoext",      IEEE80211_PARAM_COEXT_DISABLE, GET_PARAM, 0},
	{"medropmcast",         125, SET_PARAM, 1},
	{"get_medropmcast",     125, GET_PARAM, 0},
	{"me_showdeny",         126, GET_PARAM, 0},
	{"me_cleardeny",        127, SET_PARAM, 1},
	{"get_iqueconfig",      129, GET_PARAM, 0},
	{"vap_contryie",        IEEE80211_PARAM_VAP_COUNTRY_IE, SET_PARAM, 1},
	{"get_vapcontryie",     IEEE80211_PARAM_VAP_COUNTRY_IE, GET_PARAM, 0},
	{"vap_doth",            IEEE80211_PARAM_VAP_DOTH, SET_PARAM, 1},
	{"get_vap_doth",        IEEE80211_PARAM_VAP_DOTH, GET_PARAM, 0},
	{"sko",                 IEEE80211_PARAM_STA_QUICKKICKOUT, SET_PARAM, 1},
	{"get_sko",             IEEE80211_PARAM_STA_QUICKKICKOUT, GET_PARAM, 0},
	{"autoassoc",           IEEE80211_PARAM_AUTO_ASSOC, SET_PARAM, 1},
	{"get_autoassoc",       IEEE80211_PARAM_AUTO_ASSOC, GET_PARAM, 0},
	{"clrappoptie",         IEEE80211_PARAM_CLR_APPOPT_IE, SET_PARAM, 1},
	{"quiet",               IEEE80211_PARAM_QUIET_PERIOD, SET_PARAM, 1},
	{"get_quiet",           IEEE80211_PARAM_QUIET_PERIOD, GET_PARAM, 0},
	{"qbssload",            IEEE80211_PARAM_QBSS_LOAD, SET_PARAM, 1},
	{"get_qbssload",        IEEE80211_PARAM_QBSS_LOAD, GET_PARAM, 0},
	{"mfptest",             IEEE80211_PARAM_MFP_TEST, SET_PARAM, 1},
	{"get_mfptest",         IEEE80211_PARAM_MFP_TEST, GET_PARAM, 0},
	{"ips_pspoll",          IEEE80211_PARAM_STA_PWR_SET_PSPOLL, SET_PARAM, 1},
	{"get_ips_pspoll",      IEEE80211_PARAM_STA_PWR_SET_PSPOLL, GET_PARAM, 0},
#if UMAC_SUPPORT_CHANUTIL_MEASUREMENT
	{"chutil_enab",         IEEE80211_PARAM_CHAN_UTIL_ENAB, SET_PARAM, 1},
	{"get_chutil_enab",     IEEE80211_PARAM_CHAN_UTIL_ENAB, GET_PARAM, 0},
	{"get_chutil",          IEEE80211_PARAM_CHAN_UTIL, GET_PARAM, 0},
#endif
	{"dbgLVL_high",         IEEE80211_PARAM_DBG_LVL_HIGH, SET_PARAM, 1},
	{"getdbgLVL_high",      IEEE80211_PARAM_DBG_LVL_HIGH, GET_PARAM, 0},
	{"no_wradar",           IEEE80211_PARAM_WEATHER_RADAR_CHANNEL, SET_PARAM, 1},
	{"get_no_wradar",       IEEE80211_PARAM_WEATHER_RADAR_CHANNEL, GET_PARAM, 0},
	{"wepkeycache",         IEEE80211_PARAM_WEP_KEYCACHE, SET_PARAM, 1},
	{"get_wepkeycache",     IEEE80211_PARAM_WEP_KEYCACHE, GET_PARAM, 0},
	{"wnmsleepmode",        IEEE80211_PARAM_WNM_SLEEP, SET_PARAM, 1},
	{"g_wnmsleepmode",      IEEE80211_PARAM_WNM_SLEEP, GET_PARAM, 0},
	{"maxampdu",            IEEE80211_PARAM_MAX_AMPDU, SET_PARAM, 1},
	{"get_maxampdu",        IEEE80211_PARAM_MAX_AMPDU, GET_PARAM, 0},
	{"vhtmaxampdu",         IEEE80211_PARAM_VHT_MAX_AMPDU, SET_PARAM, 1},
	{"get_vhtmaxampdu",     IEEE80211_PARAM_VHT_MAX_AMPDU, GET_PARAM, 0},
	{"get_dfsdomain",       IEEE80211_PARAM_DFSDOMAIN, GET_PARAM, 0},
	{"pure11ac",            IEEE80211_PARAM_PURE11AC, SET_PARAM, 1},
	{"get_pure11ac",        IEEE80211_PARAM_PURE11AC, GET_PARAM, 0},
	{"bandwidth",           IEEE80211_PARAM_BANDWIDTH, SET_PARAM, 1},
	{"get_bandwidth",       IEEE80211_PARAM_BANDWIDTH, GET_PARAM, 0},
	{"freq_band",           IEEE80211_PARAM_FREQ_BAND, SET_PARAM, 1},
	{"get_freq_band",       IEEE80211_PARAM_FREQ_BAND, GET_PARAM, 0},
	{"extchan",             IEEE80211_PARAM_EXTCHAN, SET_PARAM, 1},
	{"get_extchan",         IEEE80211_PARAM_EXTCHAN, GET_PARAM, 0},
	{"get_mcsmode",         IEEE80211_PARAM_MCS, GET_PARAM, 0},
	{"get_channf",          IEEE80211_PARAM_CHAN_NOISE, GET_PARAM, 0},
	{"session",             IEEE80211_PARAM_SESSION_TIMEOUT, SET_PARAM, 1},
	{"get_session",         IEEE80211_PARAM_SESSION_TIMEOUT, GET_PARAM, 0},
	{"wnmsmenter",          IEEE80211_PARAM_WNM_SMENTER, SET_PARAM, 1},
	{"wnmsmexit",           IEEE80211_PARAM_WNM_SMEXIT, SET_PARAM, 1},
	{"hcbssload",           IEEE80211_PARAM_HC_BSSLOAD, SET_PARAM, 1},
	{"get_hcbssload",       IEEE80211_PARAM_HC_BSSLOAD, GET_PARAM, 0},
	{"osen",                IEEE80211_PARAM_OSEN, SET_PARAM, 1},
	{"get_osen",            IEEE80211_PARAM_OSEN, GET_PARAM, 0},
	{"cfreq2",              IEEE80211_PARAM_SECOND_CENTER_FREQ, SET_PARAM, 2},
	{"get_cfreq2",          IEEE80211_PARAM_SECOND_CENTER_FREQ, GET_PARAM, 0},
	{"strictbw",            IEEE80211_PARAM_STRICT_BW, SET_PARAM, 1},
	{"get_strictbw",        IEEE80211_PARAM_STRICT_BW, GET_PARAM, 0},
	{"get_mixedmode",       IEEE80211_PARAM_MIXED_MODE, SET_PARAM, 1},
	{"vlan_tag",            IEEE80211_PARAM_ATH_SUPPORT_VLAN, SET_PARAM, 1},
	{"get_vlan_tag",        IEEE80211_PARAM_ATH_SUPPORT_VLAN, GET_PARAM, 0},
	{"no_HT_TxAmsdu",       IEEE80211_PARAM_11N_TX_AMSDU, SET_PARAM, 1},
	{"g_no_HT_TxAmsdu",     IEEE80211_PARAM_11N_TX_AMSDU, GET_PARAM, 0},
	{"ctsprt_dtmbcn",       IEEE80211_PARAM_CTSPROT_DTIM_BCN, SET_PARAM, 1},
	{"g_ctsprt_dtmbcn",     IEEE80211_PARAM_CTSPROT_DTIM_BCN, GET_PARAM, 0},
#if ATH_SUPPORT_DSCP_OVERRIDE
	{"vappriority",         IEEE80211_PARAM_VAP_DSCP_PRIORITY, SET_PARAM, 1},
	{"get_vappriority",     IEEE80211_PARAM_VAP_DSCP_PRIORITY, GET_PARAM, 0},
#endif
	{"use_custom_chan",     IEEE80211_PARAM_CUSTOM_CHAN_LIST, SET_PARAM, 1},
	{"get_custom_chan",     IEEE80211_PARAM_CUSTOM_CHAN_LIST, GET_PARAM, 0},
	{"mbo",                 IEEE80211_PARAM_MBO, SET_PARAM, 1},
	{"g_mbo",               IEEE80211_PARAM_MBO, GET_PARAM, 0},
	{"mbocap",              IEEE80211_PARAM_MBO_CAP, SET_PARAM, 1},
	{"g_mbocap",            IEEE80211_PARAM_MBO_CAP, GET_PARAM, 0},
	{"mbo_asoc_dis",        IEEE80211_PARAM_MBO_ASSOC_DISALLOW, SET_PARAM, 1},
	{"g_mbo_asoc_dis",      IEEE80211_PARAM_MBO_ASSOC_DISALLOW, GET_PARAM, 0},
	{"mbo_cel_pref",        IEEE80211_PARAM_MBO_CELLULAR_PREFERENCE, SET_PARAM, 1},
	{"g_mbo_cel_pref",      IEEE80211_PARAM_MBO_CELLULAR_PREFERENCE, GET_PARAM, 0},
	{"mbo_trans_rs",        IEEE80211_PARAM_MBO_TRANSITION_REASON, SET_PARAM, 1},
	{"g_mbo_trans_rs",      IEEE80211_PARAM_MBO_TRANSITION_REASON, GET_PARAM, 0},
	{"mbo_asoc_ret",        IEEE80211_PARAM_MBO_ASSOC_RETRY_DELAY, SET_PARAM, 1},
	{"g_mbo_asoc_ret",      IEEE80211_PARAM_MBO_ASSOC_RETRY_DELAY, GET_PARAM, 0},
	{"maccmd_sec",          IEEE80211_PARAM_MACCMD_SEC, SET_PARAM, 1},
	{"get_maccmd_sec",      IEEE80211_PARAM_MACCMD_SEC, GET_PARAM, 0},
	{"vbLVL",               IEEE80211_PARAM_UMAC_VERBOSE_LVL, SET_PARAM, 1},
	{"vsp_enable",          IEEE80211_PARAM_VSP_ENABLE, SET_PARAM, 1},
	{"g_vsp_enable",        IEEE80211_PARAM_VSP_ENABLE, GET_PARAM, 0},
	{"dyn_bw_rts",          IEEE80211_PARAM_DYN_BW_RTS, SET_PARAM, 1},
	{"get_dyn_bw_rts",      IEEE80211_PARAM_DYN_BW_RTS, GET_PARAM, 0},
	{"oce",                 IEEE80211_PARAM_OCE, SET_PARAM, 1},
	{"g_oce",               IEEE80211_PARAM_OCE, GET_PARAM, 0},
	{"oce_asoc_rej",        IEEE80211_PARAM_OCE_ASSOC_REJECT, SET_PARAM, 1},
	{"g_oce_asoc_rej",      IEEE80211_PARAM_OCE_ASSOC_REJECT, GET_PARAM, 0},
	{"oce_asoc_dly",        IEEE80211_PARAM_OCE_ASSOC_RETRY_DELAY, SET_PARAM, 1},
	{"g_oce_asoc_dly",      IEEE80211_PARAM_OCE_ASSOC_RETRY_DELAY, GET_PARAM, 0},
	{"oce_wan_mtr",         IEEE80211_PARAM_OCE_WAN_METRICS, SET_PARAM, 2},
	{"g_oce_wan_mtr",       IEEE80211_PARAM_OCE_WAN_METRICS, GET_PARAM, 0},
	{"prb_rate",            IEEE80211_PARAM_PRB_RATE, SET_PARAM, 1},
	{"g_prb_rate",          IEEE80211_PARAM_PRB_RATE, GET_PARAM, 0},
	{"prb_retry",           IEEE80211_PARAM_PRB_RETRY, SET_PARAM, 1},
	{"g_prb_retry",         IEEE80211_PARAM_PRB_RETRY, GET_PARAM, 0},
	{"oce_hlp",             IEEE80211_PARAM_OCE_HLP, SET_PARAM, 1},
	{"g_oce_hlp",           IEEE80211_PARAM_OCE_HLP, GET_PARAM, 0},
	{"nbr_scan_prd",        IEEE80211_PARAM_NBR_SCAN_PERIOD, SET_PARAM, 1},
	{"g_nbr_scan_prd",      IEEE80211_PARAM_NBR_SCAN_PERIOD, GET_PARAM, 0},
	{"rnr",                 IEEE80211_PARAM_RNR, SET_PARAM, 1},
	{"g_rnr",               IEEE80211_PARAM_RNR, GET_PARAM, 0},
	{"rnr_fd",              IEEE80211_PARAM_RNR_FD, SET_PARAM, 1},
	{"g_rnr_fd",            IEEE80211_PARAM_RNR_FD, GET_PARAM, 0},
	{"rnr_tbtt",            IEEE80211_PARAM_RNR_TBTT, SET_PARAM, 1},
	{"g_rnr_tbtt",          IEEE80211_PARAM_RNR_TBTT, GET_PARAM, 0},
	{"apchanrpt",           IEEE80211_PARAM_AP_CHAN_RPT, SET_PARAM, 1},
	{"g_apchanrpt",         IEEE80211_PARAM_AP_CHAN_RPT, GET_PARAM, 0},
	{"mgmt_rate",           IEEE80211_PARAM_MGMT_RATE, SET_PARAM, 1},
	{"g_mgmt_rate",         IEEE80211_PARAM_MGMT_RATE, GET_PARAM, 0},
	{"rtscts_rate",         IEEE80211_PARAM_RTSCTS_RATE, SET_PARAM, 1},
	{"g_rtscts_rate",       IEEE80211_PARAM_RTSCTS_RATE, GET_PARAM, 0},
	{"rrm",                 IEEE80211_PARAM_RRM_CAP, SET_PARAM, 1},
	{"get_rrm",             IEEE80211_PARAM_RRM_CAP, GET_PARAM, 0},
	{"rrmstats",            IEEE80211_PARAM_RRM_STATS, SET_PARAM, 1},
	{"get_rrmstats",        IEEE80211_PARAM_RRM_STATS, GET_PARAM, 0},
	{"rrmslwin",            IEEE80211_PARAM_RRM_SLWINDOW, SET_PARAM, 1},
	{"get_rrmslwin",        IEEE80211_PARAM_RRM_SLWINDOW, GET_PARAM, 0},
	{"rrmdbg",              IEEE80211_PARAM_RRM_DEBUG, SET_PARAM, 1},
	{"get_rrmdbg",          IEEE80211_PARAM_RRM_DEBUG, GET_PARAM, 0},
	{"rrm_filter",          IEEE80211_PARAM_RRM_FILTER, SET_PARAM, 1},
	{"wnm_bss",             IEEE80211_PARAM_WNM_BSS_CAP, SET_PARAM, 1},
	{"get_wnm_bss",         IEEE80211_PARAM_WNM_BSS_CAP, GET_PARAM, 0},
	{"wnm",                 IEEE80211_PARAM_WNM_CAP, SET_PARAM, 1},
	{"get_wnm",             IEEE80211_PARAM_WNM_CAP, GET_PARAM, 0},
	{"wnm_tfs",             IEEE80211_PARAM_WNM_TFS_CAP, SET_PARAM, 1},
	{"get_wnm_tfs",         IEEE80211_PARAM_WNM_TFS_CAP, GET_PARAM, 0},
	{"wnm_tim",             IEEE80211_PARAM_WNM_TIM_CAP, SET_PARAM, 1},
	{"get_wnm_tim",         IEEE80211_PARAM_WNM_TIM_CAP, GET_PARAM, 0},
	{"wnm_sleep",           IEEE80211_PARAM_WNM_SLEEP_CAP, SET_PARAM, 1},
	{"get_wnm_sleep",       IEEE80211_PARAM_WNM_SLEEP_CAP, GET_PARAM, 0},
	{"wnm_fms",             IEEE80211_PARAM_WNM_FMS_CAP, SET_PARAM, 1},
	{"get_wnm_fms",         IEEE80211_PARAM_WNM_FMS_CAP, GET_PARAM, 0},
	{"wnm_filter",          IEEE80211_PARAM_WNM_FILTER, SET_PARAM, 1},
#if WLAN_SUPPORT_GREEN_AP
	{"ant_ps_on",           IEEE80211_IOCTL_GREEN_AP_PS_ENABLE, SET_PARAM, 1},
	{"get_ant_ps_on",       IEEE80211_IOCTL_GREEN_AP_PS_ENABLE, GET_PARAM, 0},
	{"ps_timeout",          IEEE80211_IOCTL_GREEN_AP_PS_TIMEOUT, SET_PARAM, 1},
	{"get_ps_timeout",      IEEE80211_IOCTL_GREEN_AP_PS_TIMEOUT, GET_PARAM, 0},
#endif
	{"setwapi",             IEEE80211_PARAM_SETWAPI, SET_PARAM, 1},
	{"wps",                 IEEE80211_PARAM_WPS, SET_PARAM, 1},
	{"get_wps",             IEEE80211_PARAM_WPS, GET_PARAM, 0},
	{"ccmpSwSelEn",         IEEE80211_PARAM_CCMPSW_ENCDEC, SET_PARAM, 1},
	{"get_ccmpSwSelEn",     IEEE80211_PARAM_CCMPSW_ENCDEC, GET_PARAM, 0},
	{"periodicScan",        IEEE80211_PARAM_PERIODIC_SCAN, SET_PARAM, 1},
	{"g_periodicScan",      IEEE80211_PARAM_PERIODIC_SCAN, GET_PARAM, 0},
	{"csa2g",               IEEE80211_PARAM_2G_CSA, SET_PARAM, 1},
	{"get_csa2g",           IEEE80211_PARAM_2G_CSA, GET_PARAM, 0},
	{"wapi_rkupkt",         IEEE80211_PARAM_WAPIREKEY_USK, SET_PARAM, 1},
	{"get_wapi_rkupkt",     IEEE80211_PARAM_WAPIREKEY_USK, GET_PARAM, 0},
	{"wapi_rkmpkt",         IEEE80211_PARAM_WAPIREKEY_MSK, SET_PARAM, 1},
	{"get_wapi_rkmpkt",     IEEE80211_PARAM_WAPIREKEY_MSK, GET_PARAM, 0},
	{"wapi_rkupdate",       IEEE80211_PARAM_WAPIREKEY_UPDATE, SET_PARAM, 1},
	{"wdsaddr",             IEEE80211_PARAM_ADD_WDS_ADDR, SET_PARAM, 1},
#if UMAC_SUPPORT_VI_DBG
	{"dbgcfg",              IEEE80211_PARAM_DBG_CFG, SET_PARAM, 1},
	{"getdbgcfg",           IEEE80211_PARAM_DBG_CFG, GET_PARAM, 0},
	{"dbgrestart",          IEEE80211_PARAM_RESTART, SET_PARAM, 1},
	{"getdbgrestart",       IEEE80211_PARAM_RESTART, GET_PARAM, 0},
	{"rxdropstats",         IEEE80211_PARAM_RXDROP_STATUS, SET_PARAM, 1},
	{"getrxdropstats",      IEEE80211_PARAM_RXDROP_STATUS, GET_PARAM, 0},
#endif
#if ATH_SUPPORT_IBSS_DFS
	{"setibssdfsparam",     IEEE80211_PARAM_IBSS_DFS_PARAM, SET_PARAM, 1},
	{"getibssdfsparam",     IEEE80211_PARAM_IBSS_DFS_PARAM, GET_PARAM, 0},
#endif
#ifdef ATH_SUPPORT_TxBF
	{"autocvupdate",        IEEE80211_PARAM_TXBF_AUTO_CVUPDATE, SET_PARAM, 1},
	{"g_autocvupdate",      IEEE80211_PARAM_TXBF_AUTO_CVUPDATE, GET_PARAM, 0},
	{"cvupdateper",         IEEE80211_PARAM_TXBF_CVUPDATE_PER, SET_PARAM, 1},
	{"get_cvupdateper",     IEEE80211_PARAM_TXBF_CVUPDATE_PER, GET_PARAM, 0},
#endif
	{"maxsta",              IEEE80211_PARAM_MAXSTA, SET_PARAM, 1},
	{"get_maxsta",          IEEE80211_PARAM_MAXSTA, GET_PARAM, 0},
	{"scanband",            IEEE80211_PARAM_SCAN_BAND, SET_PARAM, 1},
	{"get_scanband",        IEEE80211_PARAM_SCAN_BAND, GET_PARAM, 0},
	{"no_disassoc",         IEEE80211_PARAM_NO_STOP_DISASSOC, SET_PARAM, 1},
	{"get_no_disassoc",     IEEE80211_PARAM_NO_STOP_DISASSOC, GET_PARAM, 0},
	{"txcorrection",        258, SET_PARAM, 1},
	{"g_txcorrection",      258, GET_PARAM, 0},
	{"proxyarp",            IEEE80211_PARAM_PROXYARP_CAP, SET_PARAM, 1},
	{"get_proxyarp",        IEEE80211_PARAM_PROXYARP_CAP, GET_PARAM, 0},
	{"dgaf_disable",        IEEE80211_PARAM_DGAF_DISABLE, SET_PARAM, 1},
	{"g_dgaf_disable",      IEEE80211_PARAM_DGAF_DISABLE, GET_PARAM, 0},
	{"l2tif",               IEEE80211_PARAM_L2TIF_CAP, SET_PARAM, 1},
	{"get_l2tif",           IEEE80211_PARAM_L2TIF_CAP, GET_PARAM, 0},
	{"senddeauth",          IEEE80211_PARAM_SEND_DEAUTH, SET_PARAM, 1},
	{"get_senddeauth",      IEEE80211_PARAM_SEND_DEAUTH, GET_PARAM, 0},
	{"siwtxpwradjust",      IEEE80211_PARAM_SET_TXPWRADJUST, SET_PARAM, 1},
	{"txrx_dbg",            IEEE80211_PARAM_TXRX_DBG, SET_PARAM, 1},
	{"vhtmcs",              IEEE80211_PARAM_VHT_MCS, SET_PARAM, 1},
	{"get_vhtmcs",          IEEE80211_PARAM_VHT_MCS, GET_PARAM, 0},
	{"txrx_fw_stats",       IEEE80211_PARAM_TXRX_FW_STATS, SET_PARAM, 1},
	{"txrx_fw_mstats",      IEEE80211_PARAM_TXRX_FW_MSTATS, SET_PARAM, 1},
	{"nss",                 IEEE80211_PARAM_NSS, SET_PARAM, 1},
	{"get_nss",             IEEE80211_PARAM_NSS, GET_PARAM, 0},
	{"ldpc",                IEEE80211_PARAM_LDPC, SET_PARAM, 1},
	{"get_ldpc",            IEEE80211_PARAM_LDPC, GET_PARAM, 0},
	{"tx_stbc",             IEEE80211_PARAM_TX_STBC, SET_PARAM, 1},
	{"get_tx_stbc",         IEEE80211_PARAM_TX_STBC, GET_PARAM, 0},
	{"rx_stbc",             IEEE80211_PARAM_RX_STBC, SET_PARAM, 1},
	{"get_rx_stbc",         IEEE80211_PARAM_RX_STBC, GET_PARAM, 0},
	{"aponly",              IEEE80211_PARAM_APONLY, SET_PARAM, 1},
	{"get_aponly",          IEEE80211_PARAM_APONLY, GET_PARAM, 0},
	{"txrx_fw_st_rst",      IEEE80211_PARAM_TXRX_FW_STATS_RESET, SET_PARAM, 1},
	{"tx_ppdu_log_cfg",     IEEE80211_PARAM_TX_PPDU_LOG_CFG, SET_PARAM, 1},
	{"opmode_notify",       IEEE80211_PARAM_OPMODE_NOTIFY, SET_PARAM, 1},
	{"g_opmod_notify",      IEEE80211_PARAM_OPMODE_NOTIFY, GET_PARAM, 0},
	{"nopbn",               IEEE80211_PARAM_NOPBN, SET_PARAM, 1},
	{"get_nopbn",           IEEE80211_PARAM_NOPBN, GET_PARAM, 0},
	{"set_cactimeout",      IEEE80211_PARAM_DFS_CACTIMEOUT, SET_PARAM, 1},
	{"get_cactimeout",      IEEE80211_PARAM_DFS_CACTIMEOUT, GET_PARAM, 0},
	{"enablertscts",        IEEE80211_PARAM_ENABLE_RTSCTS, SET_PARAM, 1},
	{"g_enablertscts",      IEEE80211_PARAM_ENABLE_RTSCTS, GET_PARAM, 0},
	{"bcast_rate",          IEEE80211_PARAM_BCAST_RATE, SET_PARAM, 1},
	{"get_bcast_rate",      IEEE80211_PARAM_BCAST_RATE, GET_PARAM, 0},
	{"get_parent",          IEEE80211_PARAM_PARENT_IFINDEX, GET_PARAM, 0},
	{"enable_ol_stats",     IEEE80211_PARAM_ENABLE_OL_STATS, SET_PARAM, 1},
#if WLAN_SUPPORT_GREEN_AP
	{"gap_dbgprint",        IEEE80211_IOCTL_GREEN_AP_ENABLE_PRINT, SET_PARAM, 1},
	{"gap_g_dbgprint",      IEEE80211_IOCTL_GREEN_AP_ENABLE_PRINT, GET_PARAM, 0},
#endif
	{"rc_retries",          IEEE80211_PARAM_RC_NUM_RETRIES, SET_PARAM, 1},
	{"get_rc_retries",      IEEE80211_PARAM_RC_NUM_RETRIES, GET_PARAM, 0},
	{"get_acs_state",       IEEE80211_PARAM_GET_ACS, GET_PARAM, 0},
	{"get_cac_state",       IEEE80211_PARAM_GET_CAC, GET_PARAM, 0},
	{"set_onetxchain",      IEEE80211_PARAM_ONETXCHAIN, SET_PARAM, 1},
	{"scanchevent",         IEEE80211_PARAM_SCAN_CHAN_EVENT, SET_PARAM, 1},
	{"get_scanchevent",     IEEE80211_PARAM_SCAN_CHAN_EVENT, GET_PARAM, 0},
	{"get_deschan",         IEEE80211_PARAM_DESIRED_CHANNEL, GET_PARAM, 0},
	{"get_desmode",         IEEE80211_PARAM_DESIRED_PHYMODE, GET_PARAM, 0},
	{"send_add_ies",        IEEE80211_PARAM_SEND_ADDITIONAL_IES, SET_PARAM, 1},
	{"g_send_add_ies",      IEEE80211_PARAM_SEND_ADDITIONAL_IES, GET_PARAM, 0},
	{"acsreport",           IEEE80211_PARAM_START_ACS_REPORT, SET_PARAM, 1},
	{"get_acsreport",       IEEE80211_PARAM_START_ACS_REPORT, GET_PARAM, 0},
	{"acsmindwell",         IEEE80211_PARAM_MIN_DWELL_ACS_REPORT, SET_PARAM, 1},
	{"get_acsmindwell",     IEEE80211_PARAM_MIN_DWELL_ACS_REPORT, GET_PARAM, 0},
	{"acsmaxdwell",         IEEE80211_PARAM_MAX_DWELL_ACS_REPORT, SET_PARAM, 1},
	{"get_acsmaxdwell",     IEEE80211_PARAM_MAX_DWELL_ACS_REPORT, GET_PARAM, 0},
	{"ch_long_dur",         IEEE80211_PARAM_ACS_CH_HOP_LONG_DUR, SET_PARAM, 1},
	{"get_ch_long_dur",     IEEE80211_PARAM_ACS_CH_HOP_LONG_DUR, GET_PARAM, 0},
	{"ch_nhop_dur",         IEEE80211_PARAM_ACS_CH_HOP_NO_HOP_DUR, SET_PARAM, 1},
	{"get_ch_nhop_dur",     IEEE80211_PARAM_ACS_CH_HOP_NO_HOP_DUR, GET_PARAM, 0},
	{"ch_cntwn_dur",        IEEE80211_PARAM_ACS_CH_HOP_CNT_WIN_DUR, SET_PARAM, 1},
	{"g_ch_cntwn_dur",      IEEE80211_PARAM_ACS_CH_HOP_CNT_WIN_DUR, GET_PARAM, 0},
	{"ch_noise_th",         IEEE80211_PARAM_ACS_CH_HOP_NOISE_TH, SET_PARAM, 1},
	{"get_ch_noise_th",     IEEE80211_PARAM_ACS_CH_HOP_NOISE_TH, GET_PARAM, 0},
	{"ch_cnt_th",           IEEE80211_PARAM_ACS_CH_HOP_CNT_TH, SET_PARAM, 1},
	{"get_ch_cnt_th",       IEEE80211_PARAM_ACS_CH_HOP_CNT_TH, GET_PARAM, 0},
	{"ch_hop_en",           IEEE80211_PARAM_ACS_ENABLE_CH_HOP, SET_PARAM, 1},
	{"get_ch_hop_en",       IEEE80211_PARAM_ACS_ENABLE_CH_HOP, GET_PARAM, 0},
	{"set_cabq_maxdur",     IEEE80211_PARAM_SET_CABQ_MAXDUR, SET_PARAM, 1},
	{"vht_11ng",            IEEE80211_PARAM_256QAM_2G, SET_PARAM, 1},
	{"get_vht_11ng",        IEEE80211_PARAM_256QAM_2G, GET_PARAM, 0},
	{"maxscanentry",        IEEE80211_PARAM_MAX_SCANENTRY, SET_PARAM, 1},
	{"g_maxscanentry",      IEEE80211_PARAM_MAX_SCANENTRY, GET_PARAM, 0},
	{"scanentryage",        IEEE80211_PARAM_SCANENTRY_TIMEOUT, SET_PARAM, 1},
	{"g_scanentryage",      IEEE80211_PARAM_SCANENTRY_TIMEOUT, GET_PARAM, 0},
#if UMAC_VOW_DEBUG
	{"vow_dbg",             IEEE80211_PARAM_VOW_DBG_ENABLE, SET_PARAM, 1},
	{"get_vow_dbg",         IEEE80211_PARAM_VOW_DBG_ENABLE, GET_PARAM, 0},
#endif
	{"scanmindwell",        IEEE80211_PARAM_SCAN_MIN_DWELL, SET_PARAM, 1},
	{"getscanmindwell",     IEEE80211_PARAM_SCAN_MIN_DWELL, GET_PARAM, 0},
	{"scanmaxdwell",        IEEE80211_PARAM_SCAN_MAX_DWELL, SET_PARAM, 1},
	{"getscanmaxdwell",     IEEE80211_PARAM_SCAN_MAX_DWELL, GET_PARAM, 0},
	{"vht_sgimask",         IEEE80211_PARAM_VHT_SGIMASK, SET_PARAM, 1},
	{"get_vht_sgimask",     IEEE80211_PARAM_VHT_SGIMASK, GET_PARAM, 0},
	{"vht80_rate",          IEEE80211_PARAM_VHT80_RATEMASK, SET_PARAM, 1},
	{"get_vht80_rate",      IEEE80211_PARAM_VHT80_RATEMASK, GET_PARAM, 0},
#if ATH_PERF_PWR_OFFLOAD
	{"encap_type",          IEEE80211_PARAM_VAP_TX_ENCAP_TYPE, SET_PARAM, 1},
	{"get_encap_type",      IEEE80211_PARAM_VAP_TX_ENCAP_TYPE, GET_PARAM, 0},
	{"decap_type",          IEEE80211_PARAM_VAP_RX_DECAP_TYPE, SET_PARAM, 1},
	{"get_decap_type",      IEEE80211_PARAM_VAP_RX_DECAP_TYPE, GET_PARAM, 0},
#endif
#if HOST_SW_TSO_SG_ENABLE
	{"get_tso_stats",       IEEE80211_PARAM_TSO_STATS, GET_PARAM, 0},
	{"rst_tso_stats",       IEEE80211_PARAM_TSO_STATS_RESET, GET_PARAM, 0},
#endif
#if RX_CHECKSUM_OFFLOAD
	{"get_csum_stats",      IEEE80211_PARAM_RX_CKSUM_ERR_STATS, GET_PARAM, 0},
	{"rst_csum_stats",      IEEE80211_PARAM_RX_CKSUM_ERR_RESET, GET_PARAM, 0},
#endif
	{"vhtstscap",           IEEE80211_PARAM_VHT_STS_CAP, SET_PARAM, 1},
	{"get_vhtstscap",       IEEE80211_PARAM_VHT_STS_CAP, GET_PARAM, 0},
	{"vhtsounddim",         IEEE80211_PARAM_VHT_SOUNDING_DIM, SET_PARAM, 1},
	{"get_vhtsounddim",     IEEE80211_PARAM_VHT_SOUNDING_DIM, GET_PARAM, 0},
	{"vhtsubfee",           IEEE80211_PARAM_VHT_SUBFEE, SET_PARAM, 1},
	{"get_vhtsubfee",       IEEE80211_PARAM_VHT_SUBFEE, GET_PARAM, 0},
	{"vhtmubfee",           IEEE80211_PARAM_VHT_MUBFEE, SET_PARAM, 1},
	{"get_vhtmubfee",       IEEE80211_PARAM_VHT_MUBFEE, GET_PARAM, 0},
	{"vhtsubfer",           IEEE80211_PARAM_VHT_SUBFER, SET_PARAM, 1},
	{"get_vhtsubfer",       IEEE80211_PARAM_VHT_SUBFER, GET_PARAM, 0},
	{"vhtmubfer",           IEEE80211_PARAM_VHT_MUBFER, SET_PARAM, 1},
	{"get_vhtmubfer",       IEEE80211_PARAM_VHT_MUBFER, GET_PARAM, 0},
	{"implicitbf",          IEEE80211_PARAM_IMPLICITBF, SET_PARAM, 1},
	{"get_implicitbf",      IEEE80211_PARAM_IMPLICITBF, GET_PARAM, 0},
	{"sta_fixed_rate",      IEEE80211_PARAM_STA_FIXED_RATE, SET_PARAM, 1},
	{"11ngvhtintop",        IEEE80211_PARAM_11NG_VHT_INTEROP, SET_PARAM, 1},
	{"g_11ngvhtintop",      IEEE80211_PARAM_11NG_VHT_INTEROP, GET_PARAM, 0},
#if HOST_SW_SG_ENABLE
	{"get_sg_stats",        IEEE80211_PARAM_SG_STATS, GET_PARAM, 0},
	{"rst_sg_stats",        IEEE80211_PARAM_SG_STATS_RESET, GET_PARAM, 0},
#endif
	{"splitmac",            IEEE80211_PARAM_SPLITMAC, SET_PARAM, 1},
	{"get_splitmac",        IEEE80211_PARAM_SPLITMAC, GET_PARAM, 0},
#if ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION
	{"rawsim_txagr",        IEEE80211_PARAM_RAWMODE_SIM_TXAGGR, SET_PARAM, 1},
	{"g_rawsim_txagr",      IEEE80211_PARAM_RAWMODE_SIM_TXAGGR, GET_PARAM, 0},
	{"rawsim_stats",        IEEE80211_PARAM_RAWMODE_PKT_SIM_STATS, GET_PARAM, 0},
	{"clr_rawsim_stat",     IEEE80211_PARAM_CLR_RAWMODE_PKT_SIM_STATS, SET_PARAM, 1},
	{"rawsim_debug",        IEEE80211_PARAM_RAWMODE_SIM_DEBUG_LEVEL, SET_PARAM, 1},
	{"g_rawsim_debug",      IEEE80211_PARAM_RAWMODE_SIM_DEBUG_LEVEL, GET_PARAM, 0},
#endif
	{"get_proxysta",        IEEE80211_PARAM_PROXY_STA, GET_PARAM, 0},
	{"bw_nss_rate",         IEEE80211_PARAM_BW_NSS_RATEMASK, SET_PARAM, 1},
	{"get_signal_dbm",      IEEE80211_PARAM_RX_SIGNAL_DBM, GET_PARAM, 0},
	{"vht_txmcsmap",        IEEE80211_PARAM_VHT_TX_MCSMAP, SET_PARAM, 1},
	{"g_vht_txmcsmap",      IEEE80211_PARAM_VHT_TX_MCSMAP, GET_PARAM, 0},
	{"vht_rxmcsmap",        IEEE80211_PARAM_VHT_RX_MCSMAP, SET_PARAM, 1},
	{"g_vht_rxmcsmap",      IEEE80211_PARAM_VHT_RX_MCSMAP, GET_PARAM, 0},
#if QCA_AIRTIME_FAIRNESS
	{"commitatf",           IEEE80211_PARAM_ATF_OPT, SET_PARAM, 1},
	{"get_commitatf",       IEEE80211_PARAM_ATF_OPT, GET_PARAM, 0},
	{"perunit",             IEEE80211_PARAM_ATF_PER_UNIT, SET_PARAM, 1},
	{"get_perunit",         IEEE80211_PARAM_ATF_PER_UNIT, GET_PARAM, 0},
	{"enable_atf_stats",    IEEE80211_PARAM_ATF_ENABLE_STATS, SET_PARAM, 1},
	{"g_enable_atf_stats",  IEEE80211_PARAM_ATF_ENABLE_STATS, GET_PARAM, 0},
	{"atf_stats_timeout",   IEEE80211_PARAM_ATF_STATS_TIMEOUT, SET_PARAM, 1},
	{"g_atf_stats_timeout", IEEE80211_PARAM_ATF_STATS_TIMEOUT, GET_PARAM, 0},
#endif
	{"get_minpower",        IEEE80211_PARAM_TX_MIN_POWER, GET_PARAM, 0},
	{"get_maxpower",        IEEE80211_PARAM_TX_MAX_POWER, GET_PARAM, 0},
	{"novap_reset",         IEEE80211_PARAM_NO_VAP_RESET, SET_PARAM, 1},
	{"get_novap_reset",     IEEE80211_PARAM_NO_VAP_RESET, GET_PARAM, 0},
	{"get_sta_count",       IEEE80211_PARAM_STA_COUNT, GET_PARAM, 0},
#if QCA_SUPPORT_SSID_STEERING
	{"ssid_config",         IEEE80211_PARAM_VAP_SSID_CONFIG, SET_PARAM, 1},
	{"get_ssid_config",     IEEE80211_PARAM_VAP_SSID_CONFIG, GET_PARAM, 0},
#endif
#if ATH_SUPPORT_DSCP_OVERRIDE
	{"set_dscp_ovride",     IEEE80211_PARAM_DSCP_MAP_ID, SET_PARAM, 1},
	{"get_dscp_ovride",     IEEE80211_PARAM_DSCP_MAP_ID, GET_PARAM, 0},
	{"s_dscp_tid_map",      IEEE80211_PARAM_DSCP_TID_MAP, SET_PARAM, 2},
	{"g_dscp_tid_map",      IEEE80211_PARAM_DSCP_TID_MAP, GET_PARAM, 0},
#endif
	{"set_monrxfilter",     IEEE80211_PARAM_RX_FILTER_MONITOR, SET_PARAM, 1},
	{"get_monrxfilter",     IEEE80211_PARAM_RX_FILTER_MONITOR, GET_PARAM, 0},
	{"addlocalpeer",        IEEE80211_PARAM_ADD_LOCAL_PEER, SET_PARAM, 2},
	{"setmhdr",             IEEE80211_PARAM_SET_MHDR, SET_PARAM, 1},
	{"allowdata",           IEEE80211_PARAM_ALLOW_DATA, SET_PARAM, 2},
	{"meshdbg",             IEEE80211_PARAM_SET_MESHDBG, SET_PARAM, 1},
	{"enable_rtt",          IEEE80211_PARAM_RTT_ENABLE, SET_PARAM, 1},
	{"enable_lci",          IEEE80211_PARAM_LCI_ENABLE, SET_PARAM, 1},
	{"athnewind",           IEEE80211_PARAM_VAP_ENHIND, SET_PARAM, 1},
	{"get_athnewind",       IEEE80211_PARAM_VAP_ENHIND, GET_PARAM, 0},
	{"pause_scan",          IEEE80211_PARAM_VAP_PAUSE_SCAN, SET_PARAM, 1},
	{"get_pause_scan",      IEEE80211_PARAM_VAP_PAUSE_SCAN, GET_PARAM, 0},
	{"ext_acs_prg",         IEEE80211_PARAM_EXT_ACS_IN_PROGRESS, SET_PARAM, 1},
	{"get_ext_acs_prg",     IEEE80211_PARAM_EXT_ACS_IN_PROGRESS, GET_PARAM, 0},
	{"ampduden_ovrd",       IEEE80211_PARAM_AMPDU_DENSITY_OVERRIDE, SET_PARAM, 1},
	{"g_ampduden_ovrd",     IEEE80211_PARAM_AMPDU_DENSITY_OVERRIDE, GET_PARAM, 0},
	{"smesh_cfg",           IEEE80211_PARAM_SMART_MESH_CONFIG, SET_PARAM, 1},
	{"get_smesh_cfg",       IEEE80211_PARAM_SMART_MESH_CONFIG, GET_PARAM, 0},
	{"bcnbwnssmap",         IEEE80211_DISABLE_BCN_BW_NSS_MAP, SET_PARAM, 1},
	{"get_bcnbwnssmap",     IEEE80211_DISABLE_BCN_BW_NSS_MAP, GET_PARAM, 0},
	{"blbwnssmap",          IEEE80211_DISABLE_STA_BWNSS_ADV, SET_PARAM, 1},
	{"get_blbwnssmap",      IEEE80211_DISABLE_STA_BWNSS_ADV, GET_PARAM, 0},
	{"neighbourfilter",     IEEE80211_PARAM_RX_FILTER_NEIGHBOUR_PEERS_MONITOR,
		SET_PARAM, 1},
	{"get_whc_wds",         IEEE80211_PARAM_WHC_APINFO_WDS, GET_PARAM, 0},
	{"set_whc_dist",        IEEE80211_PARAM_WHC_APINFO_ROOT_DIST, SET_PARAM, 1},
	{"get_whc_dist",        IEEE80211_PARAM_WHC_APINFO_ROOT_DIST, GET_PARAM, 0},
	{"assocwar160",         IEEE80211_PARAM_CONFIG_ASSOC_WAR_160W, SET_PARAM, 1},
	{"get_assocwar160",     IEEE80211_PARAM_CONFIG_ASSOC_WAR_160W, GET_PARAM, 0},
#if QCA_AIRTIME_FAIRNESS
	{"atf_max_buf",         IEEE80211_PARAM_ATF_TXBUF_MAX, SET_PARAM, 1},
	{"g_atf_max_buf",       IEEE80211_PARAM_ATF_TXBUF_MAX, GET_PARAM, 0},
	{"atf_min_buf",         IEEE80211_PARAM_ATF_TXBUF_MIN, SET_PARAM, 1},
	{"g_atf_min_buf",       IEEE80211_PARAM_ATF_TXBUF_MIN, GET_PARAM, 0},
	{"atf_shr_buf",         IEEE80211_PARAM_ATF_TXBUF_SHARE, SET_PARAM, 1},
	{"g_atf_shr_buf",       IEEE80211_PARAM_ATF_TXBUF_SHARE, GET_PARAM, 0},
	{"atfmaxclient",        IEEE80211_PARAM_ATF_MAX_CLIENT, SET_PARAM, 1},
	{"g_atfmaxclient",      IEEE80211_PARAM_ATF_MAX_CLIENT, GET_PARAM, 0},
	{"atfssidgroup",        IEEE80211_PARAM_ATF_SSID_GROUP, SET_PARAM, 1},
	{"g_atfssidgroup",      IEEE80211_PARAM_ATF_SSID_GROUP, GET_PARAM, 0},
#endif
	{"bss_chan_info",       IEEE80211_PARAM_BSS_CHAN_INFO, SET_PARAM, 1},
	{"enable_lcr",          IEEE80211_PARAM_LCR_ENABLE, SET_PARAM, 1},
	{"get_whc_son",         IEEE80211_PARAM_WHC_APINFO_SON, GET_PARAM, 0},
	{"son",                 IEEE80211_PARAM_SON, SET_PARAM, 1},
	{"get_son",             IEEE80211_PARAM_SON, GET_PARAM, 0},
	{"rmode_pktsim",        IEEE80211_PARAM_RAWMODE_PKT_SIM, SET_PARAM, 1},
	{"g_rmode_pktsim",      IEEE80211_PARAM_RAWMODE_PKT_SIM, GET_PARAM, 0},
	{"rawdwepind",          IEEE80211_PARAM_CONFIG_RAW_DWEP_IND, SET_PARAM, 1},
	{"get_rawdwepind",      IEEE80211_PARAM_CONFIG_RAW_DWEP_IND, GET_PARAM, 0},
#if UMAC_SUPPORT_ACFG
	{"set_warn_thres",      IEEE80211_PARAM_DIAG_WARN_THRESHOLD, SET_PARAM, 1},
	{"get_warn_thres",      IEEE80211_PARAM_DIAG_WARN_THRESHOLD, GET_PARAM, 0},
	{"set_err_thres",       IEEE80211_PARAM_DIAG_ERR_THRESHOLD, SET_PARAM, 1},
	{"get_err_thres",       IEEE80211_PARAM_DIAG_ERR_THRESHOLD, GET_PARAM, 0},
#endif
	{"txrx_vap_stats",      IEEE80211_PARAM_TXRX_VAP_STATS, SET_PARAM, 1},
	{"revsig160",           IEEE80211_PARAM_CONFIG_REV_SIG_160W, SET_PARAM, 1},
	{"get_revsig160",       IEEE80211_PARAM_CONFIG_REV_SIG_160W, GET_PARAM, 0},
	{"disable11nmcs",       IEEE80211_PARAM_DISABLE_SELECTIVE_HTMCS_FOR_VAP,
		SET_PARAM, 1},
	{"g_disable11nmcs",     IEEE80211_PARAM_DISABLE_SELECTIVE_HTMCS_FOR_VAP,
		GET_PARAM, 0},
	{"conf_11acmcs",        IEEE80211_PARAM_CONFIGURE_SELECTIVE_VHTMCS_FOR_VAP,
		SET_PARAM, 1},
	{"g_conf_11acmcs",      IEEE80211_PARAM_CONFIGURE_SELECTIVE_VHTMCS_FOR_VAP,
		GET_PARAM, 0},
	{"set_RDG_enable",      IEEE80211_PARAM_RDG_ENABLE, SET_PARAM, 1},
	{"get_RDG_enable",      IEEE80211_PARAM_RDG_ENABLE, GET_PARAM, 0},
	{"get_DFS_support",     IEEE80211_PARAM_DFS_SUPPORT, GET_PARAM, 0},
	{"get_DFS_enable",      IEEE80211_PARAM_DFS_ENABLE, GET_PARAM, 0},
	{"get_ACS_support",     IEEE80211_PARAM_ACS_SUPPORT, GET_PARAM, 0},
	{"get_SSID_status",     IEEE80211_PARAM_SSID_STATUS, GET_PARAM, 0},
	{"get_DL_prisup",       IEEE80211_PARAM_DL_QUEUE_PRIORITY_SUPPORT, GET_PARAM, 0},
	{"clear_mm_rssi",       IEEE80211_PARAM_CLEAR_MIN_MAX_RSSI, GET_PARAM, 0},
	{"clear_qos",           IEEE80211_PARAM_CLEAR_QOS, SET_PARAM, 1},
#if QCA_AIRTIME_FAIRNESS
	{"atf_tput_at",         IEEE80211_PARAM_ATF_OVERRIDE_AIRTIME_TPUT, SET_PARAM, 1},
	{"g_atf_tput_at",       IEEE80211_PARAM_ATF_OVERRIDE_AIRTIME_TPUT, GET_PARAM, 0},
#endif
#if MESH_MODE_SUPPORT
	{"meshcap",             IEEE80211_PARAM_MESH_CAPABILITIES, SET_PARAM, 1},
	{"get_meshcap",         IEEE80211_PARAM_MESH_CAPABILITIES, GET_PARAM, 0},
#endif
	{"acl_notify",          IEEE80211_PARAM_CONFIG_ASSOC_DENIAL_NOTIFY, SET_PARAM, 1},
	{"get_acl_notify",      IEEE80211_PARAM_CONFIG_ASSOC_DENIAL_NOTIFY, GET_PARAM, 0},
	{"vap_txrx_stats",      IEEE80211_PARAM_VAP_TXRX_FW_STATS, SET_PARAM, 1},
	{"vap_txrx_st_rst",     IEEE80211_PARAM_VAP_TXRX_FW_STATS_RESET, SET_PARAM, 1},
	{"mu_blklist_cnt",      IEEE80211_PARAM_PEER_TX_MU_BLACKLIST_COUNT, SET_PARAM, 1},
	{"get_mu_tx_count",     IEEE80211_PARAM_PEER_TX_COUNT, GET_PARAM, 0},
	{"rst_mu_tx_count",     IEEE80211_PARAM_PEER_MUMIMO_TX_COUNT_RESET, SET_PARAM, 1},
	{"get_mu_peer_pos",     IEEE80211_PARAM_PEER_POSITION, SET_PARAM, 1},
#if QCA_AIRTIME_FAIRNESS
	{"atfssidsched",        IEEE80211_PARAM_ATF_SSID_SCHED_POLICY, SET_PARAM, 1},
	{"g_atfssidsched",      IEEE80211_PARAM_ATF_SSID_SCHED_POLICY, GET_PARAM, 0},
#endif
#if MESH_MODE_SUPPORT
	{"conf_meshtx",         IEEE80211_PARAM_CONFIG_MGMT_TX_FOR_MESH, SET_PARAM, 1},
	{"g_conf_meshtx",       IEEE80211_PARAM_CONFIG_MGMT_TX_FOR_MESH, GET_PARAM, 0},
	{"mesh_rxfilter",       IEEE80211_PARAM_CONFIG_RX_MESH_FILTER, SET_PARAM, 1},
#endif
	{"set_traf_stat",       IEEE80211_PARAM_TRAFFIC_STATS, SET_PARAM, 1},
	{"set_traf_rate",       IEEE80211_PARAM_TRAFFIC_RATE, SET_PARAM, 1},
	{"set_traf_int",        IEEE80211_PARAM_TRAFFIC_INTERVAL, SET_PARAM, 1},
	{"set_waterm_th",       IEEE80211_PARAM_WATERMARK_THRESHOLD, SET_PARAM, 1},
	{"get_waterm_th",       IEEE80211_PARAM_WATERMARK_THRESHOLD, GET_PARAM, 0},
	{"get_th_reach",        IEEE80211_PARAM_WATERMARK_REACHED, GET_PARAM, 0},
	{"get_assoc_reach",     IEEE80211_PARAM_ASSOC_REACHED, GET_PARAM, 0},
	{"dis_legacy",
		IEEE80211_PARAM_DISABLE_SELECTIVE_LEGACY_RATE_FOR_VAP, SET_PARAM, 1},
	{"g_dis_legacy",
		IEEE80211_PARAM_DISABLE_SELECTIVE_LEGACY_RATE_FOR_VAP, GET_PARAM, 0},
	{"rept_spl",            IEEE80211_PARAM_REPT_MULTI_SPECIAL, SET_PARAM, 1},
	{"g_rept_spl",          IEEE80211_PARAM_REPT_MULTI_SPECIAL, GET_PARAM, 0},
	{"vie_ena",             IEEE80211_PARAM_ENABLE_VENDOR_IE, SET_PARAM, 1},
	{"g_vie_ena",           IEEE80211_PARAM_ENABLE_VENDOR_IE, GET_PARAM, 0},
	{"set_whc_sfactor",     IEEE80211_PARAM_WHC_APINFO_SFACTOR, SET_PARAM, 1},
	{"get_whc_sfactor",     IEEE80211_PARAM_WHC_APINFO_SFACTOR, GET_PARAM, 0},
	{"get_whc_rate",        IEEE80211_PARAM_WHC_APINFO_RATE, GET_PARAM, 0},
	{"mon_decoder",         IEEE80211_PARAM_CONFIG_MON_DECODER, SET_PARAM, 1},
	{"g_mon_decoder",       IEEE80211_PARAM_CONFIG_MON_DECODER, GET_PARAM, 0},
	{"mudeasoc",            IEEE80211_PARAM_CONFIG_MU_CAP_TIMER, SET_PARAM, 1},
	{"get_mudeasoc",        IEEE80211_PARAM_CONFIG_MU_CAP_TIMER, GET_PARAM, 0},
	{"mucapwar",            IEEE80211_PARAM_CONFIG_MU_CAP_WAR, SET_PARAM, 1},
	{"get_mucapwar",        IEEE80211_PARAM_CONFIG_MU_CAP_WAR, GET_PARAM, 0},
	{"nstswar",             IEEE80211_PARAM_CONFIG_NSTSCAP_WAR, SET_PARAM, 1},
	{"get_nstswar",         IEEE80211_PARAM_CONFIG_NSTSCAP_WAR, GET_PARAM, 0},
	{"set_bcn_rate",        IEEE80211_PARAM_BEACON_RATE_FOR_VAP, SET_PARAM, 1},
	{"get_bcn_rate",        IEEE80211_PARAM_BEACON_RATE_FOR_VAP, GET_PARAM, 0},
	{"csmode",              IEEE80211_PARAM_CHANNEL_SWITCH_MODE, SET_PARAM, 1},
	{"g_csmode",            IEEE80211_PARAM_CHANNEL_SWITCH_MODE, GET_PARAM, 0},
	{"enable_ecsa",         IEEE80211_PARAM_ENABLE_ECSA_IE, SET_PARAM, 1},
	{"g_enable_ecsa",       IEEE80211_PARAM_ENABLE_ECSA_IE, GET_PARAM, 0},
	{"ecsa_opclass",        IEEE80211_PARAM_ECSA_OPCLASS, SET_PARAM, 1},
	{"g_ecsa_opclass",      IEEE80211_PARAM_ECSA_OPCLASS, GET_PARAM, 0},
#if DYNAMIC_BEACON_SUPPORT
	{"dynamicbeacon",       IEEE80211_PARAM_DBEACON_EN, SET_PARAM, 1},
	{"g_dynamicbeacon",     IEEE80211_PARAM_DBEACON_EN, GET_PARAM, 0},
	{"db_rssi_thr",         IEEE80211_PARAM_DBEACON_RSSI_THR, SET_PARAM, 1},
	{"g_db_rssi_thr",       IEEE80211_PARAM_DBEACON_RSSI_THR, GET_PARAM, 0},
	{"db_timeout",          IEEE80211_PARAM_DBEACON_TIMEOUT, SET_PARAM, 1},
	{"g_db_timeout",        IEEE80211_PARAM_DBEACON_TIMEOUT, GET_PARAM, 0},
#endif
	{"s_txpow_mgmt",        IEEE80211_PARAM_TXPOW_MGMT, SET_PARAM, 2},
	{"g_txpow_mgmt",        IEEE80211_PARAM_TXPOW_MGMT, GET_PARAM, 1},
	{"tx_capture",          IEEE80211_PARAM_CONFIG_TX_CAPTURE, SET_PARAM, 1},
	{"g_tx_capture",        IEEE80211_PARAM_CONFIG_TX_CAPTURE, GET_PARAM, 0},
	{"backhaul",            IEEE80211_PARAM_BACKHAUL, SET_PARAM, 1},
	{"get_backhaul",        IEEE80211_PARAM_BACKHAUL, GET_PARAM, 0},
	{"set_mesh_mcast",      IEEE80211_PARAM_MESH_MCAST, SET_PARAM, 1},
	{"get_mesh_mcast",      IEEE80211_PARAM_MESH_MCAST, GET_PARAM, 0},
	{"he_mcs",              IEEE80211_PARAM_HE_MCS, SET_PARAM, 1},
	{"get_he_mcs",          IEEE80211_PARAM_HE_MCS, GET_PARAM, 0},
	{"he_extrange",         IEEE80211_PARAM_HE_EXTENDED_RANGE, SET_PARAM, 1},
	{"get_he_extrange",     IEEE80211_PARAM_HE_EXTENDED_RANGE, GET_PARAM, 0},
	{"he_dcm",              IEEE80211_PARAM_HE_DCM, SET_PARAM, 1},
	{"get_he_dcm",          IEEE80211_PARAM_HE_DCM, GET_PARAM, 0},
	{"he_mu_edca",          IEEE80211_PARAM_HE_MU_EDCA, SET_PARAM, 1},
	{"get_he_mu_edca",      IEEE80211_PARAM_HE_MU_EDCA, GET_PARAM, 0},
	{"he_frag",             IEEE80211_PARAM_HE_FRAGMENTATION, SET_PARAM, 1},
	{"get_he_frag",         IEEE80211_PARAM_HE_FRAGMENTATION, GET_PARAM, 0},
	{"he_dl_ofdma",         IEEE80211_PARAM_HE_DL_MU_OFDMA, SET_PARAM, 1},
	{"get_he_dl_ofdma",     IEEE80211_PARAM_HE_DL_MU_OFDMA, GET_PARAM, 0},
	{"he_ul_mimo",          IEEE80211_PARAM_HE_UL_MU_MIMO, SET_PARAM, 1},
	{"get_he_ul_mimo",      IEEE80211_PARAM_HE_UL_MU_MIMO, GET_PARAM, 0},
	{"he_ul_ofdma",         IEEE80211_PARAM_HE_UL_MU_OFDMA, SET_PARAM, 1},
	{"get_he_ul_ofdma",     IEEE80211_PARAM_HE_UL_MU_OFDMA, GET_PARAM, 0},
	{"qwrap_hk_war",        520, SET_PARAM, 1},
	{"get_qwrap_hk_war",    520, GET_PARAM, 0},
	{"qdf_cv_lvl",          IEEE80211_PARAM_CONFIG_CATEGORY_VERBOSE, SET_PARAM, 1},
	{"g_qdf_cv_lvl",        IEEE80211_PARAM_CONFIG_CATEGORY_VERBOSE, GET_PARAM, 0},
	{"txrx_stats",          IEEE80211_PARAM_TXRX_DP_STATS, SET_PARAM, 1},
	{"caprssi",             IEEE80211_PARAM_WHC_CAP_RSSI, SET_PARAM, 1},
	{"g_caprssi",           IEEE80211_PARAM_WHC_CAP_RSSI, GET_PARAM, 0},
	{"g_curr_caprssi",      IEEE80211_PARAM_WHC_CURRENT_CAP_RSSI, GET_PARAM, 0},
	{"set_whc_ul_rate",     IEEE80211_PARAM_WHC_APINFO_UPLINK_RATE, SET_PARAM, 1},
	{"get_whc_ul_rate",     IEEE80211_PARAM_WHC_APINFO_UPLINK_RATE, GET_PARAM, 0},
	{"he_subfee",           IEEE80211_PARAM_HE_SU_BFEE, SET_PARAM, 1},
	{"get_he_subfee",       IEEE80211_PARAM_HE_SU_BFEE, GET_PARAM, 0},
	{"he_subfer",           IEEE80211_PARAM_HE_SU_BFER, SET_PARAM, 1},
	{"get_he_subfer",       IEEE80211_PARAM_HE_SU_BFER, GET_PARAM, 0},
	{"he_mubfee",           IEEE80211_PARAM_HE_MU_BFEE, SET_PARAM, 1},
	{"get_he_mubfee",       IEEE80211_PARAM_HE_MU_BFEE, GET_PARAM, 0},
	{"he_mubfer",           IEEE80211_PARAM_HE_MU_BFER, SET_PARAM, 1},
	{"get_he_mubfer",       IEEE80211_PARAM_HE_MU_BFER, GET_PARAM, 0},
	{"ext_nss_sup",         IEEE80211_PARAM_EXT_NSS_SUPPORT, SET_PARAM, 1},
	{"g_ext_nss_sup",       IEEE80211_PARAM_EXT_NSS_SUPPORT, GET_PARAM, 0},
	{"set_qosacn_conf",     IEEE80211_PARAM_QOS_ACTION_FRAME_CONFIG, SET_PARAM, 1},
	{"get_qosacn_conf",     IEEE80211_PARAM_QOS_ACTION_FRAME_CONFIG, GET_PARAM, 0},
	{"he_ltf",              IEEE80211_PARAM_HE_LTF, SET_PARAM, 1},
	{"get_he_ltf",          IEEE80211_PARAM_HE_LTF, GET_PARAM, 0},
	{"dfs_notify",          IEEE80211_PARAM_DFS_INFO_NOTIFY_APP, SET_PARAM, 1},
	{"g_dfs_notify",        IEEE80211_PARAM_DFS_INFO_NOTIFY_APP, GET_PARAM, 0},
	{"nssol_inspect",       IEEE80211_PARAM_NSSOL_VAP_INSPECT_MODE, SET_PARAM, 1},
	{"he_rtsthrshld",       IEEE80211_PARAM_HE_RTSTHRSHLD, SET_PARAM, 1},
	{"get_he_rtsthrshld",   IEEE80211_PARAM_HE_RTSTHRSHLD, GET_PARAM, 0},
	{"ratedrop",            IEEE80211_PARAM_RATE_DROPDOWN, SET_PARAM, 1},
	{"get_ratedrop",        IEEE80211_PARAM_RATE_DROPDOWN, GET_PARAM, 0},
	{"disable_cabq",        IEEE80211_PARAM_DISABLE_CABQ, SET_PARAM, 1},
	{"g_disable_cabq",      IEEE80211_PARAM_DISABLE_CABQ, GET_PARAM, 0},
	{"csl",                 IEEE80211_PARAM_CSL_SUPPORT, SET_PARAM, 1},
	{"g_csl",               IEEE80211_PARAM_CSL_SUPPORT, GET_PARAM, 0},
	{"timeoutie",           IEEE80211_PARAM_TIMEOUTIE, SET_PARAM, 1},
	{"g_timeoutie",         IEEE80211_PARAM_TIMEOUTIE, GET_PARAM, 0},
	{"pmf_assoc",           IEEE80211_PARAM_PMF_ASSOC, SET_PARAM, 1},
	{"g_pmf_assoc",         IEEE80211_PARAM_PMF_ASSOC, GET_PARAM, 0},
	{"enable_fils",         IEEE80211_PARAM_ENABLE_FILS, SET_PARAM, 2},
	{"g_enable_fils",       IEEE80211_PARAM_ENABLE_FILS, GET_PARAM, 0},
#if ATH_ACS_DEBUG_SUPPORT
	{"acs_debug_enable",    IEEE80211_PARAM_ACS_DEBUG_SUPPORT, SET_PARAM, 1},
#endif
#if ATH_ACL_SOFTBLOCKING
	{"softblock_wait",      IEEE80211_PARAM_SOFTBLOCK_WAIT_TIME, SET_PARAM, 1},
	{"g_sftblk_wait",       IEEE80211_PARAM_SOFTBLOCK_WAIT_TIME, GET_PARAM, 0},
	{"softblock_allow",     IEEE80211_PARAM_SOFTBLOCK_ALLOW_TIME, SET_PARAM, 1},
	{"g_sftblk_allow",      IEEE80211_PARAM_SOFTBLOCK_ALLOW_TIME, GET_PARAM, 0},
#endif
	{"nrshareflag",         IEEE80211_PARAM_NR_SHARE_RADIO_FLAG, SET_PARAM, 1},
	{"get_nrshareflag",     IEEE80211_PARAM_NR_SHARE_RADIO_FLAG, GET_PARAM, 0},
#if QCN_IE
	{"set_bpr_delay",       IEEE80211_PARAM_BCAST_PROBE_RESPONSE_DELAY, SET_PARAM, 1},
	{"get_bpr_delay",       IEEE80211_PARAM_BCAST_PROBE_RESPONSE_DELAY, GET_PARAM, 0},
	{"set_bpr_latency",
		IEEE80211_PARAM_BCAST_PROBE_RESPONSE_LATENCY_COMPENSATION, SET_PARAM, 1},
	{"get_bpr_latency",
		IEEE80211_PARAM_BCAST_PROBE_RESPONSE_LATENCY_COMPENSATION, GET_PARAM, 0},
	{"get_bpr_stats",       IEEE80211_PARAM_BCAST_PROBE_RESPONSE_STATS, GET_PARAM, 0},
	{"set_bpr_enable",      IEEE80211_PARAM_BCAST_PROBE_RESPONSE_ENABLE, SET_PARAM, 1},
	{"get_bpr_enable",      IEEE80211_PARAM_BCAST_PROBE_RESPONSE_ENABLE, GET_PARAM, 0},
	{"clr_bpr_stats",       IEEE80211_PARAM_BCAST_PROBE_RESPONSE_STATS_CLEAR,
		GET_PARAM, 0},
	{"set_bcn_latency",     IEEE80211_PARAM_BEACON_LATENCY_COMPENSATION, SET_PARAM, 1},
	{"get_bcn_latency",     IEEE80211_PARAM_BEACON_LATENCY_COMPENSATION, GET_PARAM, 0},
#endif
	{"enable_11v_dms",      IEEE80211_PARAM_DMS_AMSDU_WAR, SET_PARAM, 1},
	{"g_enable_11v_dms",    IEEE80211_PARAM_DMS_AMSDU_WAR, GET_PARAM, 0},
	{"s_txpow",             IEEE80211_PARAM_TXPOW, SET_PARAM, 2},
	{"g_txpow",             IEEE80211_PARAM_TXPOW, GET_PARAM, 1},
	{"ul_hyst",             IEEE80211_PARAM_BEST_UL_HYST, SET_PARAM, 1},
	{"g_ul_hyst",           IEEE80211_PARAM_BEST_UL_HYST, GET_PARAM, 0},
	{"he_txmcsmap",         IEEE80211_PARAM_HE_TX_MCSMAP, SET_PARAM, 1},
	{"get_he_txmcsmap",     IEEE80211_PARAM_HE_TX_MCSMAP, GET_PARAM, 0},
	{"he_rxmcsmap",         IEEE80211_PARAM_HE_RX_MCSMAP, SET_PARAM, 1},
	{"get_he_rxmcsmap",     IEEE80211_PARAM_HE_RX_MCSMAP, GET_PARAM, 0},
	{"m_copy",              IEEE80211_PARAM_CONFIG_M_COPY, SET_PARAM, 1},
	{"get_m_copy",          IEEE80211_PARAM_CONFIG_M_COPY, GET_PARAM, 0},
	{"ba_bufsize",          IEEE80211_PARAM_BA_BUFFER_SIZE, SET_PARAM, 1},
	{"get_ba_bufsize",      IEEE80211_PARAM_BA_BUFFER_SIZE, GET_PARAM, 0},
	{"he_ar_gi_ltf",        IEEE80211_PARAM_HE_AR_GI_LTF, SET_PARAM, 1},
	{"get_he_ar_gi_ltf",    IEEE80211_PARAM_HE_AR_GI_LTF, GET_PARAM, 0},
	{"nss_rdprehdr",        IEEE80211_PARAM_NSSOL_VAP_READ_RXPREHDR, SET_PARAM, 1},
	{"g_nss_rdprehdr",      IEEE80211_PARAM_NSSOL_VAP_READ_RXPREHDR, GET_PARAM, 0},
	{"he_sounding_mode",    IEEE80211_PARAM_HE_SOUNDING_MODE, SET_PARAM, 1},
	{"get_he_sounding_mode",    IEEE80211_PARAM_HE_SOUNDING_MODE, GET_PARAM, 0},
	{"rsn_override",        IEEE80211_PARAM_RSN_OVERRIDE, SET_PARAM, 1},
	{"g_rsn_override",      IEEE80211_PARAM_RSN_OVERRIDE, GET_PARAM, 0},
	{"map",                 IEEE80211_PARAM_MAP, SET_PARAM, 1},
	{"get_map",             IEEE80211_PARAM_MAP, GET_PARAM, 0},
	{"MapBSSType",          IEEE80211_PARAM_MAP_BSS_TYPE, SET_PARAM, 1},
	{"get_MapBSSType",      IEEE80211_PARAM_MAP_BSS_TYPE, GET_PARAM, 0},
	{"he_ht_ctrl",          IEEE80211_PARAM_HE_HT_CTRL, SET_PARAM, 1},
	{"get_he_ht_ctrl",      IEEE80211_PARAM_HE_HT_CTRL, GET_PARAM, 0},
	{"acsmaxscantime",      IEEE80211_PARAM_MAX_SCAN_TIME_ACS_REPORT, SET_PARAM, 1},
	{"g_acsmaxscan_t",      IEEE80211_PARAM_MAX_SCAN_TIME_ACS_REPORT, GET_PARAM, 0},
	{"mapset_vapup",        IEEE80211_PARAM_MAP_VAP_BEACONING, SET_PARAM, 1},
	{"mapget_vapup",        IEEE80211_PARAM_MAP_VAP_BEACONING, GET_PARAM, 0},
#if UMAC_SUPPORT_XBSSLOAD
	{"xbssload",            IEEE80211_PARAM_XBSS_LOAD, SET_PARAM, 1},
	{"get_xbssload",        IEEE80211_PARAM_XBSS_LOAD, GET_PARAM, 0},
#endif
	{"sifs_tr_rate",        IEEE80211_PARAM_SIFS_TRIGGER_RATE, SET_PARAM, 1},
	{"g_sifs_tr_rate",      IEEE80211_PARAM_SIFS_TRIGGER_RATE, GET_PARAM, 0},
	{"log_flush_timer_period",      IEEE80211_PARAM_LOG_FLUSH_TIMER_PERIOD,
		SET_PARAM, 1},
	{"log_flush_one_time",          IEEE80211_PARAM_LOG_FLUSH_ONE_TIME, SET_PARAM, 1},
	{"log_dump_at_kernel_enable",   IEEE80211_PARAM_LOG_DUMP_AT_KERNEL_ENABLE,
		SET_PARAM, 1},
	{"bsteerrssi_log",      IEEE80211_PARAM_LOG_ENABLE_BSTEERING_RSSI, SET_PARAM, 1},
	{"ft",                  IEEE80211_PARAM_FT_ENABLE, SET_PARAM, 1},
	{"g_ft",                IEEE80211_PARAM_FT_ENABLE, GET_PARAM, 0},
	{"bcn_stats_clr",       IEEE80211_PARAM_BCN_STATS_RESET, SET_PARAM, 1},
	{"sifs_trigger",        IEEE80211_PARAM_SIFS_TRIGGER, SET_PARAM, 1},
	{"g_sifs_trigger",      IEEE80211_PARAM_SIFS_TRIGGER, GET_PARAM, 0},
#if QCA_SUPPORT_SON
	{"son_event_bcast",     IEEE80211_PARAM_SON_EVENT_BCAST, SET_PARAM, 1},
	{"g_son_event_bcast",   IEEE80211_PARAM_SON_EVENT_BCAST, GET_PARAM, 0},
#endif
	{"he_ul_shortgi",       IEEE80211_PARAM_HE_UL_SHORTGI, SET_PARAM, 1},
	{"get_he_ul_shortgi",   IEEE80211_PARAM_HE_UL_SHORTGI, GET_PARAM, 0},
	{"he_ul_ltf",           IEEE80211_PARAM_HE_UL_LTF, SET_PARAM, 1},
	{"get_he_ul_ltf",       IEEE80211_PARAM_HE_UL_LTF, GET_PARAM, 0},
	{"he_ul_nss",           IEEE80211_PARAM_HE_UL_NSS, SET_PARAM, 1},
	{"get_he_ul_nss",       IEEE80211_PARAM_HE_UL_NSS, GET_PARAM, 0},
	{"he_ul_ppdu_bw",       IEEE80211_PARAM_HE_UL_PPDU_BW, SET_PARAM, 1},
	{"get_he_ul_ppdu_bw",   IEEE80211_PARAM_HE_UL_PPDU_BW, GET_PARAM, 0},
	{"he_ul_ldpc",          IEEE80211_PARAM_HE_UL_LDPC, SET_PARAM, 1},
	{"get_he_ul_ldpc",      IEEE80211_PARAM_HE_UL_LDPC, GET_PARAM, 0},
	{"he_ul_stbc",          IEEE80211_PARAM_HE_UL_STBC, SET_PARAM, 1},
	{"get_he_ul_stbc",      IEEE80211_PARAM_HE_UL_STBC, SET_PARAM, 1},
	{"he_ul_mcs",           IEEE80211_PARAM_HE_UL_FIXED_RATE, SET_PARAM, 1},
	{"get_he_ul_mcs",       IEEE80211_PARAM_HE_UL_FIXED_RATE, GET_PARAM, 0},
#if WLAN_SER_DEBUG
	{"ser_history",         IEEE80211_PARAM_WLAN_SER_HISTORY, SET_PARAM, 2},
#endif
	{"da_wds_war",          IEEE80211_PARAM_DA_WAR_ENABLE, SET_PARAM, 1},
	{"g_da_wds_war",        IEEE80211_PARAM_DA_WAR_ENABLE, GET_PARAM, 0},
	{"advertise_sta_maxcap",    IEEE80211_PARAM_STA_MAX_CH_CAP, SET_PARAM, 1},
	{"g_advertise_sta_maxcap",  IEEE80211_PARAM_STA_MAX_CH_CAP, GET_PARAM, 0},
	{"rawmode_open_war",    IEEE80211_PARAM_RAWMODE_OPEN_WAR, SET_PARAM, 1},
	{"g_rawmode_open_war",  IEEE80211_PARAM_RAWMODE_OPEN_WAR, GET_PARAM, 0},
	{"set_skip_hyst",       IEEE80211_PARAM_WHC_SKIP_HYST, SET_PARAM, 1},
	{"get_skip_hyst",       IEEE80211_PARAM_WHC_SKIP_HYST, GET_PARAM, 0},
	{"whc_mixedbh_ul",      IEEE80211_PARAM_WHC_MIXEDBH_ULRATE, SET_PARAM, 1},
	{"g_whc_mixedbh_ul",    IEEE80211_PARAM_WHC_MIXEDBH_ULRATE, GET_PARAM, 0},
	{"whc_mixedbh_bh_type", IEEE80211_PARAM_WHC_BACKHAUL_TYPE, SET_PARAM, 1},
	{"he_bsr_supp",         IEEE80211_PARAM_HE_BSR_SUPPORT, SET_PARAM, 1},
	{"get_he_bsr_supp",     IEEE80211_PARAM_HE_BSR_SUPPORT, GET_PARAM, 0},
	{"display_me_info",     IEEE80211_PARAM_DUMP_RA_TABLE, GET_PARAM, 0},
	{"set_obss_ru_tolerance_time",  IEEE80211_PARAM_OBSS_NB_RU_TOLERANCE_TIME,
		SET_PARAM, 1},
	{"get_obss_ru_tolerance_time",  IEEE80211_PARAM_OBSS_NB_RU_TOLERANCE_TIME,
		GET_PARAM, 0},
	{"suniformrssi",        IEEE80211_PARAM_UNIFORM_RSSI, SET_PARAM, 1},
	{"guniformrssi",        IEEE80211_PARAM_UNIFORM_RSSI, GET_PARAM, 0},
	{"scsainteropphy",      IEEE80211_PARAM_CSA_INTEROP_PHY, SET_PARAM, 1},
	{"gcsainteropphy",      IEEE80211_PARAM_CSA_INTEROP_PHY, GET_PARAM, 0},
	{"scsainteropbss",      IEEE80211_PARAM_CSA_INTEROP_BSS, SET_PARAM, 1},
	{"gscsainteropbss",     IEEE80211_PARAM_CSA_INTEROP_BSS, GET_PARAM, 0},
	{"scsainteropauth",     IEEE80211_PARAM_CSA_INTEROP_AUTH, SET_PARAM, 1},
	{"gscsainteropauth",    IEEE80211_PARAM_CSA_INTEROP_AUTH, GET_PARAM, 0},
	{"he_amsdu_in_ampdu_supp",      IEEE80211_PARAM_HE_AMSDU_IN_AMPDU_SUPRT,
		SET_PARAM, 1},
	{"get_he_amsdu_in_ampdu_supp",  IEEE80211_PARAM_HE_AMSDU_IN_AMPDU_SUPRT,
		GET_PARAM, 0},
	{"he_bfee_sts_supp",        IEEE80211_PARAM_HE_SUBFEE_STS_SUPRT, SET_PARAM, 2},
	{"he_4xltf_800nsgi_rx",     IEEE80211_PARAM_HE_4XLTF_800NS_GI_RX_SUPRT,
		SET_PARAM, 1},
	{"get_he_4xltf_800nsgi_rx", IEEE80211_PARAM_HE_4XLTF_800NS_GI_RX_SUPRT,
		GET_PARAM, 0},
	{"he_1xltf_800nsgi_rx",     IEEE80211_PARAM_HE_1XLTF_800NS_GI_RX_SUPRT,
		SET_PARAM, 1},
	{"get_he_1xltf_800nsgi_rx", IEEE80211_PARAM_HE_1XLTF_800NS_GI_RX_SUPRT,
		GET_PARAM, 0},
	{"he_max_nc",           IEEE80211_PARAM_HE_MAX_NC_SUPRT, SET_PARAM, 1},
	{"get_he_max_nc",       IEEE80211_PARAM_HE_MAX_NC_SUPRT, GET_PARAM, 0},
	{"twt_responder",       IEEE80211_PARAM_TWT_RESPONDER_SUPRT, SET_PARAM, 1},
	{"get_twt_responder",   IEEE80211_PARAM_TWT_RESPONDER_SUPRT, GET_PARAM, 0},
	{"tx_lat_capture",      IEEE80211_PARAM_CONFIG_CAPTURE_LATENCY_ENABLE,
		SET_PARAM, 1},
	{"g_tx_lat_cptr",       IEEE80211_PARAM_CONFIG_CAPTURE_LATENCY_ENABLE,
		GET_PARAM, 1},
	{"get_ru26_tolerance",  IEEE80211_PARAM_GET_RU26_TOLERANCE, GET_PARAM, 0},
	{"get_whc_ul_snr",      IEEE80211_PARAM_WHC_APINFO_UPLINK_SNR, GET_PARAM, 0},
	{"set_dpp_mode",        IEEE80211_PARAM_DPP_VAP_MODE, SET_PARAM, 1},
	{"get_dpp_mode",        IEEE80211_PARAM_DPP_VAP_MODE, GET_PARAM, 0},
#if SM_ENG_HIST_ENABLE
	{"sm_history",          IEEE80211_PARAM_SM_HISTORY, GET_PARAM, 0},
#endif
	{"s_scan_flush",        646, SET_PARAM, 1},
	{"rx_amsdu_ptid",       IEEE80211_PARAM_RX_AMSDU, SET_PARAM, 2},
	{"get_rx_amsdu",        IEEE80211_PARAM_RX_AMSDU, GET_PARAM, 0},
	{"set_mtu_size",        IEEE80211_PARAM_MAX_MTU_SIZE, SET_PARAM, 1},
	{"get_mtu_size",        IEEE80211_PARAM_MAX_MTU_SIZE, GET_PARAM, 0},
	{"bcast_prbrsp_en",     IEEE80211_PARAM_HE_6GHZ_BCAST_PROB_RSP, SET_PARAM, 2},
	{"get_bcast_prbrsp_en", IEEE80211_PARAM_HE_6GHZ_BCAST_PROB_RSP, GET_PARAM, 0},
	{"vht_mcs_10_11_supp",      IEEE80211_PARAM_VHT_MCS_10_11_SUPP, SET_PARAM, 1},
	{"get_vht_mcs_10_11_supp",  IEEE80211_PARAM_VHT_MCS_10_11_SUPP, GET_PARAM, 0},
	{"vht_mcs_10_11_nq2q_peer_supp",
		IEEE80211_PARAM_VHT_MCS_10_11_NQ2Q_PEER_SUPP, SET_PARAM, 1},
	{"get_vht_mcs_10_11_nq2q_peer_supp",
		IEEE80211_PARAM_VHT_MCS_10_11_NQ2Q_PEER_SUPP, GET_PARAM, 0},
	{"en_sae_pwid",         IEEE80211_PARAM_SAE_PWID, SET_PARAM, 1},
	{"g_en_sae_pwid",       IEEE80211_PARAM_SAE_PWID, GET_PARAM, 0},
	{"mcast_rc_stale_period",   IEEE80211_PARAM_MCAST_RC_STALE_PERIOD, SET_PARAM, 1},
	{"g_mcast_rc_stale_period", IEEE80211_PARAM_MCAST_RC_STALE_PERIOD, GET_PARAM, 0},
	{"he_multi_tid_aggr_rx",        IEEE80211_PARAM_HE_MULTI_TID_AGGR, SET_PARAM, 1},
	{"get_he_multi_tid_aggr_rx",    IEEE80211_PARAM_HE_MULTI_TID_AGGR, GET_PARAM, 0},
	{"he_multi_tid_aggr_tx",        IEEE80211_PARAM_HE_MULTI_TID_AGGR_TX,
		SET_PARAM, 1},
	{"get_he_multi_tid_aggr_tx",    IEEE80211_PARAM_HE_MULTI_TID_AGGR_TX,
		GET_PARAM, 0},
	{"he_max_ampdu_len_exp",        IEEE80211_PARAM_HE_MAX_AMPDU_LEN_EXP,
		SET_PARAM, 1},
	{"get_he_max_ampdu_len_exp",    IEEE80211_PARAM_HE_MAX_AMPDU_LEN_EXP,
		GET_PARAM, 0},
	{"he_su_ppdu_1x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_SU_PPDU_1X_LTF_800NS_GI, SET_PARAM, 1},
	{"get_he_su_ppdu_1x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_SU_PPDU_1X_LTF_800NS_GI, GET_PARAM, 0},
	{"he_su_mu_ppdu_4x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_SU_MU_PPDU_4X_LTF_800NS_GI, SET_PARAM, 1},
	{"get_he_su_mu_ppdu_4x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_SU_MU_PPDU_4X_LTF_800NS_GI, SET_PARAM, 1},
	{"he_max_frag_msdu",        IEEE80211_PARAM_HE_MAX_FRAG_MSDU, SET_PARAM, 1},
	{"get_he_max_frag_msdu",    IEEE80211_PARAM_HE_MAX_FRAG_MSDU, GET_PARAM, 0},
	{"he_min_frag_size",        IEEE80211_PARAM_HE_MIN_FRAG_SIZE, SET_PARAM, 1},
	{"get_he_min_frag_size",    IEEE80211_PARAM_HE_MIN_FRAG_SIZE, GET_PARAM, 0},
	{"he_omi",              IEEE80211_PARAM_HE_OMI, SET_PARAM, 1},
	{"get_he_omi",          IEEE80211_PARAM_HE_OMI, GET_PARAM, 0},
	{"he_ndp_4x_ltf_3200ns_gi",
		IEEE80211_PARAM_HE_NDP_4X_LTF_3200NS_GI, SET_PARAM, 1},
	{"get_he_ndp_4x_ltf_3200ns_gi",
		IEEE80211_PARAM_HE_NDP_4X_LTF_3200NS_GI, GET_PARAM, 0},
	{"he_er_su_ppdu_1x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_ER_SU_PPDU_1X_LTF_800NS_GI, SET_PARAM, 1},
	{"get_he_er_su_ppdu_1x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_ER_SU_PPDU_1X_LTF_800NS_GI, GET_PARAM, 0},
	{"he_er_su_ppdu_4x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_ER_SU_PPDU_4X_LTF_800NS_GI, SET_PARAM, 1},
	{"get_he_er_su_ppdu_4x_ltf_800ns_gi",
		IEEE80211_PARAM_HE_ER_SU_PPDU_4X_LTF_800NS_GI, GET_PARAM, 0},
	{"get_maxrate",         IEEE80211_PARAM_GET_MAX_RATE, GET_PARAM, 0},
	{"get_signal_level",    IEEE80211_PARAM_GET_SIGNAL_LEVEL, GET_PARAM, 0},
	{"dec_bcn_loss",        IEEE80211_PARAM_DEC_BCN_LOSS, SET_PARAM, 1},
	{"multi_group_key",     IEEE80211_PARAM_ENABLE_MULTI_GROUP_KEY, SET_PARAM, 1},
	{"g_multi_group_key",   IEEE80211_PARAM_ENABLE_MULTI_GROUP_KEY, GET_PARAM, 0},
	{"max_group_keys",      IEEE80211_PARAM_MAX_GROUP_KEYS, SET_PARAM, 1},
	{"g_max_group_keys",    IEEE80211_PARAM_MAX_GROUP_KEYS, GET_PARAM, 0},
	{"vendor_fwd_mask",     IEEE80211_PARAM_VENDOR_FRAME_FWD_MASK, SET_PARAM, 1},
	{"g_vendor_fwd_mask",   IEEE80211_PARAM_VENDOR_FRAME_FWD_MASK, GET_PARAM, 0},
	{"he_dynamic_muedca",       IEEE80211_PARAM_HE_DYNAMIC_MU_EDCA, SET_PARAM, 1},
	{"get_he_dynamic_muedca",   IEEE80211_PARAM_HE_DYNAMIC_MU_EDCA, GET_PARAM, 0},
	{"he_ar_ldpc",          IEEE80211_PARAM_HE_AR_LDPC, SET_PARAM, 1},
	{"get_he_ar_ldpc",      IEEE80211_PARAM_HE_AR_LDPC, GET_PARAM, 0},
	{"mcast_rate_control",  IEEE80211_PARAM_ENABLE_MCAST_RC, SET_PARAM, 1},
	{"g_mcast_rate_control",    IEEE80211_PARAM_ENABLE_MCAST_RC, GET_PARAM, 0},
	{"mcs12_13_supp",       IEEE80211_PARAM_VHT_MCS_12_13_SUPP, SET_PARAM, 1},
	{"g_mcs12_13_supp",     IEEE80211_PARAM_VHT_MCS_12_13_SUPP, GET_PARAM, 0},
	{"rrm_capie",           IEEE80211_PARAM_RRM_CAP_IE, SET_PARAM, 1},
	{"g_rrm_capie",         IEEE80211_PARAM_RRM_CAP_IE, GET_PARAM, 0},
	{"map_sta_vlan",        IEEE80211_PARAM_MAP2_BSTA_VLAN_ID, SET_PARAM, 1},
	{"get_map_sta_vlan",    IEEE80211_PARAM_MAP2_BSTA_VLAN_ID, GET_PARAM, 0},
	{"set_vdev_peer_prot_count",    IEEE80211_PARAM_VDEV_PEER_PROTOCOL_COUNT,
		SET_PARAM, 1},
	{"get_vdev_peer_prot_count",    IEEE80211_PARAM_VDEV_PEER_PROTOCOL_COUNT,
		GET_PARAM, 0},
	{"set_vdev_peer_prot_dropmask",
		IEEE80211_PARAM_VDEV_PEER_PROTOCOL_DROP_MASK, SET_PARAM, 1},
	{"get_vdev_peer_prot_dropmask",
		IEEE80211_PARAM_VDEV_PEER_PROTOCOL_DROP_MASK, GET_PARAM, 0},
#if WLAN_SCHED_HISTORY_SIZE
	{"sched_history",       IEEE80211_PARAM_WLAN_SCHED_HISTORY, SET_PARAM, 0},
#endif
	{"he_dlofdma_bf",       IEEE80211_PARAM_HE_DL_MU_OFDMA_BFER, SET_PARAM, 1},
	{"g_he_dlofdma_bf",     IEEE80211_PARAM_HE_DL_MU_OFDMA_BFER, GET_PARAM, 0},
	{"sendprobereq",        IEEE80211_PARAM_SEND_PROBE_REQ, SET_PARAM, 1},
	{"oce_txpower",         IEEE80211_PARAM_OCE_TX_POWER, SET_PARAM, 2},
	{"g_oce_txpower",       IEEE80211_PARAM_OCE_TX_POWER, GET_PARAM, 0},
	{"oce_subnet_id",       IEEE80211_PARAM_OCE_IP_SUBNET_ID, SET_PARAM, 2},
	{"oce_ess_report",      IEEE80211_PARAM_OCE_ADD_ESS_RPT, SET_PARAM, 2},
	{"rsnx_override",       IEEE80211_PARAM_RSNX_OVERRIDE, SET_PARAM, 1},
#if ATH_PERF_PWR_OFFLOAD && QCA_SUPPORT_RAWMODE_PKT_SIMULATION
	{"rsim_en_frmcnt",      IEEE80211_PARAM_RAWSIM_DEBUG_NUM_ENCAP_FRAMES, SET_PARAM, 1},
	{"g_rsim_en_frmcnt",    IEEE80211_PARAM_RAWSIM_DEBUG_NUM_ENCAP_FRAMES, GET_PARAM, 0},
	{"rsim_de_frmcnt",      IEEE80211_PARAM_RAWSIM_DEBUG_NUM_DECAP_FRAMES, SET_PARAM, 1},
	{"g_rsim_de_frmcnt",    IEEE80211_PARAM_RAWSIM_DEBUG_NUM_DECAP_FRAMES, GET_PARAM, 0},
#endif
	{"mscs",                IEEE80211_PARAM_ENABLE_MSCS, SET_PARAM, 1},
	{"g_mscs",              IEEE80211_PARAM_ENABLE_MSCS, GET_PARAM, 0},
	{"mbss_tx_vdev",        IEEE80211_PARAM_MBSS_TXVDEV, SET_PARAM, 1},
	{"g_mbss_tx_vdev",      IEEE80211_PARAM_MBSS_TXVDEV, GET_PARAM, 0},
	{"setiebuf",            35828, SET_PARAM, 1},
	{"getiebuf",            35827, GET_PARAM, 0},
	{"dbgreq",              35832, SET_PARAM, 1},
	{"sendmgmt",            35834, SET_PARAM, 1},
	{"me_adddeny",          35835, SET_PARAM, 1},
	{"hbrparams",           35838, SET_PARAM, 1},
	{"rxtimeout",           35839, SET_PARAM, 1},
	{"oce_version_override ",     IEEE80211_PARAM_OCE_VERSION_OVERRIDE, SET_PARAM, 1},
	{"g_oce_version_override ",   IEEE80211_PARAM_OCE_VERSION_OVERRIDE, GET_PARAM, 0},
	{"igmpmcasten",         IEEE80211_PARAM_IGMP_ME, SET_PARAM, 1},
	{"g_igmpmcasten",       IEEE80211_PARAM_IGMP_ME, GET_PARAM, 0},
	{"hlos_tidoverride ",     IEEE80211_PARAM_HLOS_TID_OVERRIDE, SET_PARAM, 1},
	{"g_hlos_tidoverride ",     IEEE80211_PARAM_HLOS_TID_OVERRIDE, GET_PARAM, 0},
	{"he_6g_min_rate",      IEEE80211_PARAM_6G_HE_OP_MIN_RATE, SET_PARAM, 1},
	{"g_he_6g_min_rate",    IEEE80211_PARAM_6G_HE_OP_MIN_RATE, GET_PARAM, 0},
	{"en_6g_sec_comp",      IEEE80211_PARAM_6G_SECURITY_COMP, SET_PARAM, 1},
	{"g_en_6g_sec_comp",    IEEE80211_PARAM_6G_SECURITY_COMP, GET_PARAM, 0},
	{"keymgmt_mask_6g",     IEEE80211_PARAM_6G_KEYMGMT_MASK, SET_PARAM, 1},
	{"g_keymgmt_mask_6g",   IEEE80211_PARAM_6G_KEYMGMT_MASK, GET_PARAM, 0},
	{"he_er_su_disable",       IEEE80211_PARAM_HE_ER_SU_DISABLE, SET_PARAM, 1},
	{"g_he_er_su_disable",   IEEE80211_PARAM_HE_ER_SU_DISABLE, GET_PARAM, 0},
	{"he_1024qam_lt242ru_rx",       IEEE80211_PARAM_HE_1024QAM_LT242RU_RX_ENABLE, SET_PARAM, 1},
	{"g_he_1024qam_lt242ru_rx",     IEEE80211_PARAM_HE_1024QAM_LT242RU_RX_ENABLE, GET_PARAM, 0},
	{"he_ul_mu_data_dis_rx",        IEEE80211_PARAM_HE_UL_MU_DATA_DIS_RX_SUPP, SET_PARAM, 1},
	{"g_he_ul_mu_data_dis_rx",      IEEE80211_PARAM_HE_UL_MU_DATA_DIS_RX_SUPP, GET_PARAM, 0},
	{"he_full_bw_ulmumimo",         IEEE80211_PARAM_HE_FULL_BW_UL_MUMIMO, SET_PARAM, 1},
	{"g_he_full_bw_ulmumimo",     IEEE80211_PARAM_HE_FULL_BW_UL_MUMIMO, GET_PARAM, 0},
	{"he_dcm_max_const_rx",         IEEE80211_PARAM_HE_DCM_MAX_CONSTELLATION_RX, SET_PARAM, 1},
	{"g_he_dcm_max_const_rx",     IEEE80211_PARAM_HE_DCM_MAX_CONSTELLATION_RX, GET_PARAM, 0},
#if WLAN_OBJMGR_REF_ID_TRACE
	{"enable_ref_leak",         IEEE80211_PARAM_VDEV_REF_LEAK_TEST, SET_PARAM, 1},
#endif
	{"disable_fw_inact_probe",    IEEE80211_PARAM_DISABLE_INACT_PROBING, SET_PARAM, 1},
	{"g_disable_fw_inact_probe",  IEEE80211_PARAM_DISABLE_INACT_PROBING, GET_PARAM, 0},
	{"peer_authorize ",     IEEE80211_PARAM_PEER_AUTHORIZE, SET_PARAM, 1},
	{"g_peer_authorize ",     IEEE80211_PARAM_PEER_AUTHORIZE, GET_PARAM, 0},
	{"fwd_act_app",               IEEE80211_PARAM_FWD_ACTION_FRAMES_TO_APP, SET_PARAM, 1},
	{"g_fwd_act_app",             IEEE80211_PARAM_FWD_ACTION_FRAMES_TO_APP, GET_PARAM, 0},
	{"wnmstats",                  IEEE80211_PARAM_WNM_STATS, SET_PARAM, 1},
	{"get_wnmstats",              IEEE80211_PARAM_WNM_STATS, GET_PARAM, 0},
};

struct vendor_commands radio_vendor_cmds[] = {
	{"DMABcnRespT",         0, SET_PARAM, 1},
	{"txchainmask",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXCHAINMASK, SET_PARAM, 1},
	{"get_txchainmask",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXCHAINMASK, GET_PARAM, 0},
	{"rxchainmask",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RXCHAINMASK, SET_PARAM, 1},
	{"get_rxchainmask",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RXCHAINMASK, GET_PARAM, 0},
	{"AMPDU",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU, SET_PARAM, 1},
	{"getAMPDU",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU, GET_PARAM, 0},
	{"AMPDULim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU_LIMIT, SET_PARAM, 1},
	{"getAMPDULim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU_LIMIT, GET_PARAM, 0},
	{"AMPDUFrames",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU_SUBFRAMES, SET_PARAM, 1},
	{"getAMPDUFrames",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AMPDU_SUBFRAMES, GET_PARAM, 0},
	{"TXPowLim2G",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_LIMIT2G, SET_PARAM, 1},
	{"getTxPowLim2G",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_LIMIT2G, GET_PARAM, 0},
	{"TXPowLim5G",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_LIMIT5G, SET_PARAM, 1},
	{"getTxPowLim5G",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_LIMIT5G, GET_PARAM, 0},
	{"LDPC",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LDPC, SET_PARAM, 1},
	{"getLDPC",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LDPC, GET_PARAM, 0},
	{"vow_extstats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_VOW_EXT_STATS, SET_PARAM, 1},
	{"g_vow_extstats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_VOW_EXT_STATS, GET_PARAM, 0},
	{"dyntxchain",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DYN_TX_CHAINMASK, SET_PARAM, 1},
	{"get_dyntxchain",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DYN_TX_CHAINMASK, GET_PARAM, 0},
	{"burst",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BURST_ENABLE, SET_PARAM, 1},
	{"get_burst",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BURST_ENABLE, GET_PARAM, 0},
	{"burst_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BURST_DUR, SET_PARAM, 1},
	{"get_burst_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BURST_DUR, GET_PARAM, 0},
	{"set_bcnburst",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BCN_BURST, SET_PARAM, 1},
	{"get_bcnburst",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BCN_BURST, GET_PARAM, 0},
	{"dcs_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS, SET_PARAM, 1},
	{"get_dcs_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS, GET_PARAM, 0},
	{"get_total_PER",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_PER, GET_PARAM, 0},
	{"setctsrate",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RTS_CTS_RATE, SET_PARAM, 1},
	{"get_ctsrate",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RTS_CTS_RATE, GET_PARAM, 0},
	{"set_dcs_coch_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_COCH_THR, SET_PARAM, 1},
	{"get_dcs_coch_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_COCH_THR, GET_PARAM, 0},
	{"set_dcs_errth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_TXERR_THR, SET_PARAM, 1},
	{"get_dcs_errth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_TXERR_THR, GET_PARAM, 0},
	{"s_dcs_phyerrth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_PHYERR_THR, SET_PARAM, 1},
	{"g_dcs_phyerrth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_PHYERR_THR, GET_PARAM, 0},
	{"get_if_path",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_IF_ID, GET_PARAM, 0},
	{"acs_bkscanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN, SET_PARAM, 1},
	{"g_acs_bkscanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN, GET_PARAM, 0},
	{"acs_scanintvl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_SCANTIME, SET_PARAM, 1},
	{"g_acsscanintvl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_SCANTIME, GET_PARAM, 0},
	{"acs_rssivar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_RSSIVAR, SET_PARAM, 1},
	{"get_acs_rssivar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_RSSIVAR, GET_PARAM, 0},
	{"acs_chloadvar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHLOADVAR, SET_PARAM, 1},
	{"g_acschloadvar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHLOADVAR, GET_PARAM, 0},
	{"acs_lmtobss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_LIMITEDOBSS, SET_PARAM, 1},
	{"get_acslmtobss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_LIMITEDOBSS, GET_PARAM, 0},
	{"acs_ctrlflags",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CTRLFLAG, SET_PARAM, 1},
	{"getacsctrlflags",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CTRLFLAG, GET_PARAM, 0},
	{"acs_dbgtrace",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_DEBUGTRACE, SET_PARAM, 1},
	{"g_acs_dbgtrace",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_DEBUGTRACE, GET_PARAM, 0},
	{"set_fw_hang",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SET_FW_HANG_ID, SET_PARAM, 1},
	{"get_radio_type",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RADIO_TYPE, GET_PARAM, 0},
	{"sIgmpDscpOvrid",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_IGMPMLD_OVERRIDE, SET_PARAM, 1},
	{"gIgmpDscpOvrid",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_IGMPMLD_OVERRIDE, GET_PARAM, 0},
	{"sIgmpDscpTidMap",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_IGMPMLD_TID, SET_PARAM, 1},
	{"gIgmpDscpTidMap",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_IGMPMLD_TID, GET_PARAM, 0},
	{"arpdhcp_ac",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ARPDHCP_AC_OVERRIDE, SET_PARAM, 1},
	{"g_arpdhcp_ac",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ARPDHCP_AC_OVERRIDE, GET_PARAM, 0},
	{"nonaggr_swretry",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NON_AGG_SW_RETRY_TH, SET_PARAM, 1},
	{"aggr_swretry",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_AGG_SW_RETRY_TH, SET_PARAM, 1},
	{"blockdfslist",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISABLE_DFS, SET_PARAM, 1},
	{"g_blockdfslist",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISABLE_DFS, GET_PARAM, 0},
	{"enable_amsdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_AMSDU, SET_PARAM, 1},
	{"get_amsdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_AMSDU, GET_PARAM, 0},
	{"enable_ampdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_AMPDU, SET_PARAM, 1},
	{"get_ampdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_AMPDU, GET_PARAM, 0},
	{"sta_kickout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STA_KICKOUT_TH, SET_PARAM, 1},
	{"LTR",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_ENABLE, SET_PARAM, 1},
	{"getLTR",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_ENABLE, GET_PARAM, 0},
	{"LTRBE",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_BE, SET_PARAM, 1},
	{"getLTRBE",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_BE, GET_PARAM, 0},
	{"LTRBK",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_BK, SET_PARAM, 1},
	{"getLTRBK",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_BK, GET_PARAM, 0},
	{"LTRVI",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_VI, SET_PARAM, 1},
	{"getLTRVI",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_VI, GET_PARAM, 0},
	{"LTRVO",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_VO, SET_PARAM, 1},
	{"getLTRVO",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_VO, GET_PARAM, 0},
	{"LTRACTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_TIMEOUT, SET_PARAM, 1},
	{"getLTRACTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_AC_LATENCY_TIMEOUT, GET_PARAM, 0},
	{"LTRTxTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_TX_ACTIVITY_TIMEOUT, SET_PARAM, 1},
	{"getLTRTxTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_TX_ACTIVITY_TIMEOUT, GET_PARAM, 0},
	{"LTRSLEEP",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_SLEEP_OVERRIDE, SET_PARAM, 1},
	{"getLTRSLEEP",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_SLEEP_OVERRIDE, GET_PARAM, 0},
	{"LTRRX",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_RX_OVERRIDE, SET_PARAM, 1},
	{"getLTRRX",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LTR_RX_OVERRIDE, GET_PARAM, 0},
	{"L1SS",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_L1SS_ENABLE, SET_PARAM, 1},
	{"getL1SS",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_L1SS_ENABLE, GET_PARAM, 0},
	{"DSLEEP",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DSLEEP_ENABLE, SET_PARAM, 1},
	{"getDSLEEP",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DSLEEP_ENABLE, GET_PARAM, 0},
	{"set_dcs_maxcu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_USERMAX_CU_THR, SET_PARAM, 1},
	{"get_dcs_maxcu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_USERMAX_CU_THR, GET_PARAM, 0},
	{"set_dcs_debug",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_DEBUG, SET_PARAM, 1},
	{"get_dcs_debug",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_DEBUG, GET_PARAM, 0},
	{"ani_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_ENABLE, SET_PARAM, 1},
	{"get_ani_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_ENABLE, GET_PARAM, 0},
	{"ani_poll_len",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_POLL_PERIOD, SET_PARAM, 1},
	{"ani_listen_len",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_LISTEN_PERIOD, SET_PARAM, 1},
	{"ani_ofdm_level",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_OFDM_LEVEL, SET_PARAM, 1},
	{"ani_cck_level",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANI_CCK_LEVEL, SET_PARAM, 1},
	{"dscp_tid_map",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DSCP_TID_MAP, SET_PARAM, 1},
	{"tpscale",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_SCALE, SET_PARAM, 1},
	{"get_tpscale",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_SCALE, GET_PARAM, 0},
#if ATH_SUPPORT_DSCP_OVERRIDE
	{"sHmmcDscpTidMap",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HMMC_DSCP_TID_MAP, SET_PARAM, 1},
	{"gHmmcDscpTidMap",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HMMC_DSCP_TID_MAP, GET_PARAM, 0},
	{"sHmmcDscpOvrid",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HMMC_DSCP_OVERRIDE, SET_PARAM, 1},
	{"gHmmcDscpOvrid",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HMMC_DSCP_OVERRIDE, GET_PARAM, 0},
#endif
	{"antgain_2g",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANTENNA_GAIN_2G, SET_PARAM, 1},
	{"antgain_5g",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANTENNA_GAIN_5G, SET_PARAM, 1},
	{"set_rxfilter",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RX_FILTER, SET_PARAM, 1},
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
	{"set_buff_thresh",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BUFF_THRESH, SET_PARAM, 1},
	{"get_buff_thresh",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BUFF_THRESH, GET_PARAM, 0},
	{"setBlkReportFld",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BLK_REPORT_FLOOD, SET_PARAM, 1},
	{"getBlkReportFld",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BLK_REPORT_FLOOD, GET_PARAM, 0},
	{"setDropSTAQuery",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DROP_STA_QUERY, SET_PARAM, 1},
	{"getDropSTAQuery",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DROP_STA_QUERY, GET_PARAM, 0},
#endif
	{"qboost_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_QBOOST, SET_PARAM, 1},
	{"sifs_frmtype",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SIFS_FRMTYPE, SET_PARAM, 1},
	{"sifs_uapsd",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SIFS_UAPSD, SET_PARAM, 1},
	{"set_fw_recovery",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FW_RECOVERY_ID, SET_PARAM, 1},
	{"get_fw_recovery",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FW_RECOVERY_ID, GET_PARAM, 0},
	{"reset_ol_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RESET_OL_STATS, SET_PARAM, 1},
	{"setdeauthcnt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DEAUTH_COUNT, SET_PARAM, 1},
	{"getdeauthcnt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DEAUTH_COUNT, GET_PARAM, 0},
	{"block_interbss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BLOCK_INTERBSS, SET_PARAM, 1},
	{"g_blk_interbss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BLOCK_INTERBSS, GET_PARAM, 0},
	{"disable_reset",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FW_DISABLE_RESET, SET_PARAM, 1},
	{"msdu_ttl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MSDU_TTL, SET_PARAM, 1},
	{"ppdu_duration",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PPDU_DURATION, SET_PARAM, 1},
	{"txbf_snd_int",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SET_TXBF_SND_PERIOD, SET_PARAM, 1},
	{"get_txbf_snd",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SET_TXBF_SND_PERIOD, GET_PARAM, 0},
	{"promisc",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ALLOW_PROMISC, SET_PARAM, 1},
	{"get_promisc",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ALLOW_PROMISC, GET_PARAM, 0},
	{"burst_mode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BURST_MODE, SET_PARAM, 1},
	{"dyn_grouping",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DYN_GROUPING, SET_PARAM, 1},
	{"g_dyngroup",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DYN_GROUPING, GET_PARAM, 0},
	{"dpd_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DPD_ENABLE, SET_PARAM, 1},
	{"g_dpdenable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DPD_ENABLE, GET_PARAM, 0},
	{"dbglog_ratelim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBGLOG_RATELIM, SET_PARAM, 1},
	{"ps_report",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PS_STATE_CHANGE, SET_PARAM, 1},
	{"get_ps_report",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PS_STATE_CHANGE, GET_PARAM, 0},
	{"mcast_echo",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MCAST_BCAST_ECHO, SET_PARAM, 1},
	{"g_mcast_echo",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MCAST_BCAST_ECHO, GET_PARAM, 0},
	{"obss_rssi_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OBSS_RSSI_THRESHOLD, SET_PARAM, 1},
	{"gobss_rssi_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OBSS_RSSI_THRESHOLD, GET_PARAM, 0},
	{"obss_rxrssi_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OBSS_RX_RSSI_THRESHOLD, SET_PARAM, 1},
	{"gobss_rxrssi_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OBSS_RX_RSSI_THRESHOLD, GET_PARAM, 0},
#if ATH_CHANNEL_BLOCKING
	{"acs_bmode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_BLOCK_MODE, SET_PARAM, 1},
	{"g_acs_bmode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_BLOCK_MODE, GET_PARAM, 0},
#endif
	{"acs_txpwr_opt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_TX_POWER_OPTION, SET_PARAM, 1},
	{"g_acs_txpwr_opt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_TX_POWER_OPTION, GET_PARAM, 0},
	{"antenna_plzn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ANT_POLARIZATION, SET_PARAM, 1},
	{"print_ratelim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRINT_RATE_LIMIT, SET_PARAM, 1},
	{"gprint_ratelim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRINT_RATE_LIMIT, GET_PARAM, 0},
	{"pdev_reset",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PDEV_RESET, SET_PARAM, 1},
	{"set_nohostcrash",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FW_DUMP_NO_HOST_CRASH, SET_PARAM, 1},
	{"get_nohostcrash",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FW_DUMP_NO_HOST_CRASH, GET_PARAM, 0},
	{"obss_longslot",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONSIDER_OBSS_NON_ERP_LONG_SLOT,
		SET_PARAM, 1},
	{"g_obss_longslot",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONSIDER_OBSS_NON_ERP_LONG_SLOT,
		GET_PARAM, 0},
	{"fc_stats_global",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STATS_FC, SET_PARAM, 1},
	{"fc_flush_intv",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_QFLUSHINTERVAL, SET_PARAM, 1},
	{"g_fc_flush_intv",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_QFLUSHINTERVAL, GET_PARAM, 0},
	{"fc_buf_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE, SET_PARAM, 1},
	{"g_fc_buf_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE, GET_PARAM, 0},
	{"fc_buf0_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0, SET_PARAM, 1},
	{"fc_buf1_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE1, SET_PARAM, 1},
	{"fc_buf2_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE2, SET_PARAM, 1},
	{"fc_buf3_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE3, SET_PARAM, 1},
	{"fc_buf_min",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MIN_THRESHOLD, SET_PARAM, 1},
	{"g_fc_buf_min",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MIN_THRESHOLD, GET_PARAM, 0},
	{"fc_q_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MAX_Q_LIMIT, SET_PARAM, 1},
	{"g_fc_q_max",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MAX_Q_LIMIT, GET_PARAM, 0},
	{"fc_q_min",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MIN_Q_LIMIT, SET_PARAM, 1},
	{"g_fc_q_min",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MIN_Q_LIMIT, GET_PARAM, 0},
	{"fc_cong_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONG_CTRL_TIMER_INTV, SET_PARAM, 1},
	{"g_fc_cong_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONG_CTRL_TIMER_INTV, GET_PARAM, 0},
	{"fc_stats_tmr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STATS_TIMER_INTV, SET_PARAM, 1},
	{"g_fc_stats_tmr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STATS_TIMER_INTV, GET_PARAM, 0},
	{"fc_rot_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ROTTING_TIMER_INTV, SET_PARAM, 1},
	{"g_fc_rot_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ROTTING_TIMER_INTV, GET_PARAM, 0},
	{"fc_latency_on",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_LATENCY_PROFILE, SET_PARAM, 1},
	{"fc_hostq_dump",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HOSTQ_DUMP, SET_PARAM, 1},
	{"fc_tidq_map",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TIDQ_MAP, SET_PARAM, 1},
	{"arp_dbg_srcaddr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBG_ARP_SRC_ADDR, SET_PARAM, 1},
	{"g_arp_dbgstats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBG_ARP_SRC_ADDR, GET_PARAM, 0},
	{"arp_dbg_dstaddr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBG_ARP_DST_ADDR, SET_PARAM, 1},
	{"arp_dbg_conf",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ARP_DBG_CONF, SET_PARAM, 1},
	{"g_arp_dbg_conf",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ARP_DBG_CONF, GET_PARAM, 0},
	{"st_ht_noamsdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISABLE_STA_VAP_AMSDU, SET_PARAM, 1},
	{"g_st_ht_noamsdu",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISABLE_STA_VAP_AMSDU, GET_PARAM, 0},
#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
	{"staDFSEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STADFS_ENABLE, SET_PARAM, 1},
	{"get_staDFSEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STADFS_ENABLE, GET_PARAM, 0},
#endif
#if QCA_AIRTIME_FAIRNESS
	{"atfstrictsched",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_STRICT_SCHED, SET_PARAM, 1},
	{"gatfstrictsched",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_STRICT_SCHED, GET_PARAM, 0},
	{"atfgrouppolicy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_GROUP_POLICY, SET_PARAM, 1},
	{"gatfgrouppolicy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_GROUP_POLICY, GET_PARAM, 0},
#endif
#if DBDC_REPEATER_SUPPORT
	{"primaryradio",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRIMARY_RADIO, SET_PARAM, 1},
	{"getprimaryradio",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRIMARY_RADIO, GET_PARAM, 0},
	{"dbdc_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBDC_ENABLE, SET_PARAM, 1},
	{"get_dbdc_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBDC_ENABLE, GET_PARAM, 0},
#endif
	{"tpdbscale",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXPOWER_DBSCALE, SET_PARAM, 1},
	{"ctlpwscale",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CTL_POWER_SCALE, SET_PARAM, 1},
	{"get_ctlpwscale",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CTL_POWER_SCALE, GET_PARAM, 0},
#if QCA_AIRTIME_FAIRNESS
	{"atfobsssched",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_OBSS_SCHED, SET_PARAM, 1},
	{"g_atfobsssched",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_OBSS_SCHED, GET_PARAM, 0},
#endif
	{"g_phycckerr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PHY_CCK_ERR, GET_PARAM, 0},
	{"g_fcserr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FCS_ERR, GET_PARAM, 0},
	{"g_chanutil",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_UTIL, GET_PARAM, 0},
#if DBDC_REPEATER_SUPPORT
	{"client_mcast",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CLIENT_MCAST, SET_PARAM, 1},
	{"getclient_mcast",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CLIENT_MCAST, GET_PARAM, 0},
#endif
	{"get_emiwar80p80",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_EMIWAR_80P80, SET_PARAM, 1},
	{"batch_mode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BATCHMODE, SET_PARAM, 1},
	{"packaggrdelay",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PACK_AGGR_DELAY, SET_PARAM, 1},
#if UMAC_SUPPORT_ACFG
	{"set_diag_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DIAG_ENABLE, SET_PARAM, 1},
	{"get_diag_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DIAG_ENABLE, GET_PARAM, 0},
#endif
	{"chanstats_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_STATS_TH, SET_PARAM, 1},
	{"g_chanstats_th",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_STATS_TH, GET_PARAM, 0},
	{"pas_scanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PASSIVE_SCAN_ENABLE, SET_PARAM, 1},
	{"g_pas_scanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PASSIVE_SCAN_ENABLE, GET_PARAM, 0},
	{"set_min_snr_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_MIN_RSSI_ENABLE, SET_PARAM, 1},
	{"get_min_snr_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_MIN_RSSI_ENABLE, GET_PARAM, 0},
	{"set_min_snr",
		OL_ATH_PARAM_SHIFT | OL_ATH_MIN_RSSI, SET_PARAM, 1},
	{"get_min_snr",
		OL_ATH_PARAM_SHIFT | OL_ATH_MIN_RSSI, GET_PARAM, 0},
	{"acs_2g_allch",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_2G_ALLCHAN, SET_PARAM, 1},
	{"g_acs_2g_allch",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_2G_ALLCHAN, GET_PARAM, 0},
#if DBDC_REPEATER_SUPPORT
	{"delay_stavapup",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DELAY_STAVAP_UP, SET_PARAM, 1},
	{"gdelay_stavapup",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DELAY_STAVAP_UP, GET_PARAM, 0},
#endif
	{"CSwOpts",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHANSWITCH_OPTIONS, SET_PARAM, 1},
	{"get_CSwOpts",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHANSWITCH_OPTIONS, GET_PARAM, 0},
	{"btcoex_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_BTCOEX_ENABLE, SET_PARAM, 1},
	{"g_btcoex_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_BTCOEX_ENABLE, GET_PARAM, 0},
	{"btcoex_wl_pri",
		OL_ATH_PARAM_SHIFT | OL_ATH_BTCOEX_WL_PRIORITY, SET_PARAM, 1},
	{"g_btcoex_wl_pri",
		OL_ATH_PARAM_SHIFT | OL_ATH_BTCOEX_WL_PRIORITY, GET_PARAM, 0},
	{"queue_map",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TID_OVERRIDE_QUEUE_MAPPING, SET_PARAM, 1},
	{"get_queue_map",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TID_OVERRIDE_QUEUE_MAPPING, GET_PARAM, 0},
	{"CalCheck",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CAL_VER_CHECK, SET_PARAM, 1},
	{"get_CalCheck",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CAL_VER_CHECK, GET_PARAM, 0},
	{"no_vlan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NO_VLAN, SET_PARAM, 1},
	{"get_no_vlan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NO_VLAN, GET_PARAM, 0},
	{"cca_threshold",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CCA_THRESHOLD, SET_PARAM, 1},
	{"g_cca_threshold",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CCA_THRESHOLD, GET_PARAM, 0},
	{"atf_log",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_LOGGING, SET_PARAM, 1},
	{"get_atf_log",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ATF_LOGGING, GET_PARAM, 0},
	{"strict_doth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STRICT_DOTH, SET_PARAM, 1},
	{"g_strict_doth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_STRICT_DOTH, GET_PARAM, 0},
	{"discon_time",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISCONNECTION_TIMEOUT, SET_PARAM, 1},
	{"gdiscon_time",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DISCONNECTION_TIMEOUT, GET_PARAM, 0},
	{"reconfig_time",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RECONFIGURATION_TIMEOUT, SET_PARAM, 1},
	{"greconfig_time",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RECONFIGURATION_TIMEOUT, GET_PARAM, 0},
	{"chswitchcnt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHANNEL_SWITCH_COUNT, SET_PARAM, 1},
	{"g_chswitchcnt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHANNEL_SWITCH_COUNT, GET_PARAM, 0},
	{"alwaysprimary",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ALWAYS_PRIMARY, SET_PARAM, 1},
	{"galwaysprimary",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ALWAYS_PRIMARY, GET_PARAM, 0},
	{"fast_lane",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FAST_LANE, SET_PARAM, 1},
	{"get_fast_lane",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FAST_LANE, GET_PARAM, 0},
	{"g_btcoex_dc",
		OL_ATH_PARAM_SHIFT | OL_ATH_GET_BTCOEX_DUTY_CYCLE, GET_PARAM, 0},
	{"sec_offsetie",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SECONDARY_OFFSET_IE, SET_PARAM, 1},
	{"g_sec_offsetie",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SECONDARY_OFFSET_IE, GET_PARAM, 0},
	{"wb_subelem",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WIDE_BAND_SUB_ELEMENT, SET_PARAM, 1},
	{"g_wb_subelem",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WIDE_BAND_SUB_ELEMENT, GET_PARAM, 0},
	{"pref_uplink",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PREFERRED_UPLINK, SET_PARAM, 1},
	{"get_pref_uplink",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PREFERRED_UPLINK, GET_PARAM, 0},
	{"preCACEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_ENABLE, SET_PARAM, 1},
	{"get_preCACEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_ENABLE, GET_PARAM, 0},
	{"pCACTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_TIMEOUT, SET_PARAM, 1},
	{"get_pCACTimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_TIMEOUT, GET_PARAM, 0},
	{"dump_target",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DUMP_TARGET, SET_PARAM, 1},
	{"set_reo_dest",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PDEV_TO_REO_DEST, SET_PARAM, 1},
	{"get_reo_dest",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PDEV_TO_REO_DEST, GET_PARAM, 0},
	{"dump_chmsk_tbl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DUMP_CHAINMASK_TABLES, GET_PARAM, 0},
	{"print_obj_refs",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DUMP_OBJECTS, SET_PARAM, 1},
	{"acs_srloadvar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_SRLOADVAR, SET_PARAM, 1},
	{"g_acssrloadvar",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_SRLOADVAR, GET_PARAM, 0},
	{"ext_nss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_EXT_NSS_CAPABLE, SET_PARAM, 1},
	{"g_ext_nss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_EXT_NSS_CAPABLE, GET_PARAM, 0},
	{"acktimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXACKTIMEOUT, SET_PARAM, 1},
	{"get_acktimeout",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXACKTIMEOUT, GET_PARAM, 0},
	{"set_icm_active",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ICM_ACTIVE, SET_PARAM, 1},
	{"get_icm_active",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ICM_ACTIVE, GET_PARAM, 0},
	{"acs_rank_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_RANK, SET_PARAM, 1},
	{"g_acs_rank_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_RANK, GET_PARAM, 0},
	{"txchainsoft",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXCHAINSOFT, SET_PARAM, 1},
	{"get_txchainsoft",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TXCHAINSOFT, GET_PARAM, 0},
	{"widebw_scan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WIDE_BAND_SCAN, SET_PARAM, 1},
	{"g_widebw_scan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WIDE_BAND_SCAN, GET_PARAM, 0},
	{"cck_tx_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CCK_TX_ENABLE, SET_PARAM, 1},
	{"g_cck_tx_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CCK_TX_ENABLE, GET_PARAM, 0},
	{"set_papi",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PAPI_ENABLE, SET_PARAM, 1},
	{"get_papi",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PAPI_ENABLE, GET_PARAM, 0},
	{"isolation",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ISOLATION, SET_PARAM, 1},
	{"get_isolation",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ISOLATION, GET_PARAM, 0},
	{"max_radio_sta",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MAX_CLIENTS_PER_RADIO, SET_PARAM, 1},
	{"g_max_radio_sta",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MAX_CLIENTS_PER_RADIO, GET_PARAM, 0},
#ifdef OL_ATH_SMART_LOGGING
	{"smartLogEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_ENABLE, SET_PARAM, 1},
	{"get_smartLogEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_ENABLE, GET_PARAM, 0},
	{"smartLogEvent",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_FATAL_EVENT, SET_PARAM, 1},
	{"smartLogSkbsz",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_SKB_SZ, SET_PARAM, 1},
	{"g_smartLogSkbsz",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_SKB_SZ, GET_PARAM, 0},
	{"slPingFailLog",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_P1PINGFAIL, SET_PARAM, 1},
	{"g_slPingFailLog",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SMARTLOG_P1PINGFAIL, GET_PARAM, 0},
#endif
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
	{"interCACChan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_INTER_CHANNEL, SET_PARAM, 1},
	{"getInterCACChan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PRECAC_INTER_CHANNEL, GET_PARAM, 0},
#endif
	{"g_dbr_ring_status",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DBR_RING_STATUS, GET_PARAM, 0},
#ifdef QCN_ESP_IE
	{"esp_period",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_PERIODICITY, SET_PARAM, 1},
	{"g_esp_period",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_PERIODICITY, GET_PARAM, 0},
	{"esp_airtime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_AIRTIME, SET_PARAM, 1},
	{"g_esp_airtime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_AIRTIME, GET_PARAM, 0},
	{"esp_ppdu_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_PPDU_DURATION, SET_PARAM, 1},
	{"g_esp_ppdu_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_PPDU_DURATION, GET_PARAM, 0},
	{"esp_ba_window",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_BA_WINDOW, SET_PARAM, 1},
	{"g_esp_ba_window",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ESP_BA_WINDOW, GET_PARAM, 0},
#endif
	{"cbs_bkscanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS, SET_PARAM, 1},
	{"g_cbs_bkscanen",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS, GET_PARAM, 0},
	{"dcs_sim",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_SIM, SET_PARAM, 1},
	{"cbs_dwellspt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_DWELL_SPLIT_TIME, SET_PARAM, 1},
	{"g_cbs_dwellspt",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_DWELL_SPLIT_TIME, GET_PARAM, 0},
	{"cbs_dwellrest",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_DWELL_REST_TIME, SET_PARAM, 1},
	{"g_cbs_dwellrest",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_DWELL_REST_TIME, GET_PARAM, 0},
	{"cbs_waittime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_WAIT_TIME, SET_PARAM, 1},
	{"g_cbs_waittime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_WAIT_TIME, GET_PARAM, 0},
	{"cbs_resttime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_REST_TIME, SET_PARAM, 1},
	{"g_cbs_resttime",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_REST_TIME, GET_PARAM, 0},
	{"cbs_csa",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_CSA, SET_PARAM, 1},
	{"g_cbs_csa",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CBS_CSA, GET_PARAM, 0},
	{"twice_antgain",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TWICE_ANTENNA_GAIN, SET_PARAM, 1},
	{"g_activity_f",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACTIVITY_FACTOR, GET_PARAM, 0},
	{"g_ch_util_ap_rx",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_AP_RX_UTIL, GET_PARAM, 0},
	{"g_ch_util_free",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_FREE, GET_PARAM, 0},
	{"g_ch_util_ap_tx",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_AP_TX_UTIL, GET_PARAM, 0},
	{"g_ch_util_obss",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_OBSS_RX_UTIL, GET_PARAM, 0},
	{"g_ch_util_nwifi",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_NON_WIFI, GET_PARAM, 0},
#if PEER_FLOW_CONTROL
	{"fc_video_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_VIDEO_STATS_FC, GET_PARAM, 0},
#endif
	{"set_retry_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_PEER_RETRY_STATS, SET_PARAM, 1},
	{"get_retry_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_PEER_RETRY_STATS, GET_PARAM, 0},
	{"he_ul_trig_int",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_UL_TRIG_INT, SET_PARAM, 1},
	{"get_he_ul_trig_int",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_UL_TRIG_INT, GET_PARAM, 0},
	{"mark_subchan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DFS_NOL_SUBCHANNEL_MARKING, SET_PARAM, 1},
	{"g_mark_subchan",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DFS_NOL_SUBCHANNEL_MARKING, GET_PARAM, 0},
	{"get_bandinfo",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BAND_INFO, GET_PARAM, 0},
	{"bw_reduceEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BW_REDUCE, SET_PARAM, 1},
	{"get_bw_reduceEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_BW_REDUCE, GET_PARAM, 0},
	{"get_he_ul_ppdu_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_UL_PPDU_DURATION, GET_PARAM, 0},
	{"he_ru_alloc",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_UL_RU_ALLOCATION, SET_PARAM, 1},
	{"get_he_ru_alloc",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_UL_RU_ALLOCATION, GET_PARAM, 0},
	{"cfr_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PERIODIC_CFR_CAPTURE, SET_PARAM, 1},
	{"get_cfr_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PERIODIC_CFR_CAPTURE, GET_PARAM, 0},
	{"flush_rate_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FLUSH_PEER_RATE_STATS, SET_PARAM, 1},
	{"set_dcs_enable_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_RE_ENABLE_TIMER, SET_PARAM, 1},
	{"get_dcs_enable_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_RE_ENABLE_TIMER, GET_PARAM, 0},
	{"rx_lite_monitor",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RX_MON_LITE, SET_PARAM, 1},
	{"get_rx_lite_monitor",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RX_MON_LITE, GET_PARAM, 0},
	{"wifi_down_ind",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WIFI_DOWN_IND, SET_PARAM, 1},
	{"tx_monitor",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TX_CAPTURE, SET_PARAM, 1},
	{"get_tx_monitor",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TX_CAPTURE, GET_PARAM, 0},
	{"set_wmi_dis_dump",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_WMI_DIS_DUMP, SET_PARAM, 1},
	{"acs_chan_grade_algo",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHAN_GRADE_ALGO, SET_PARAM, 1},
	{"get_acs_chan_grade_algo",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHAN_GRADE_ALGO, GET_PARAM, 0},
	{"set_acs_chan_eff_var",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHAN_EFFICIENCY_VAR, SET_PARAM, 1},
	{"get_acs_chan_eff_var",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_CHAN_EFFICIENCY_VAR, GET_PARAM, 0},
	{"set_acs_near_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_NEAR_RANGE_WEIGHTAGE, SET_PARAM, 1},
	{"get_acs_near_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_NEAR_RANGE_WEIGHTAGE, GET_PARAM, 0},
	{"set_acs_mid_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_MID_RANGE_WEIGHTAGE, SET_PARAM, 1},
	{"get_acs_mid_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_MID_RANGE_WEIGHTAGE, GET_PARAM, 0},
	{"set_acs_far_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_FAR_RANGE_WEIGHTAGE, SET_PARAM, 1},
	{"get_acs_far_range_weightage",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_FAR_RANGE_WEIGHTAGE, GET_PARAM, 0},
	{"set_mgmt_ttl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MGMT_TTL, SET_PARAM, 1},
	{"get_mgmt_ttl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MGMT_TTL, GET_PARAM, 0},
	{"set_prb_rsp_ttl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PROBE_RESP_TTL, SET_PARAM, 1},
	{"get_prb_rsp_ttl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PROBE_RESP_TTL, GET_PARAM, 0},
	{"set_mu_ppdu_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MU_PPDU_DURATION, SET_PARAM, 1},
	{"get_mu_ppdu_dur",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MU_PPDU_DURATION, GET_PARAM, 0},
	{"set_tbtt_ctrl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TBTT_CTRL, SET_PARAM, 1},
	{"get_tbtt_ctrl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_TBTT_CTRL, GET_PARAM, 0},
	{"get_rchwidth",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RCHWIDTH, GET_PARAM, 0},
	{"hw_mode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE, SET_PARAM, 1},
	{"get_hw_mode",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE, GET_PARAM, 0},
#if DBDC_REPEATER_SUPPORT
	{"samessid_disable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_SAME_SSID_DISABLE, SET_PARAM, 1},
#endif
	{"get_mbss_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MBSS_EN, GET_PARAM, 0},
	{"chan_coex_disable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_COEX, SET_PARAM, 1},
	{"g_chan_coex_disable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CHAN_COEX, GET_PARAM, 0},
	{"oob_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OOB_ENABLE, SET_PARAM, 1},
	{"g_oob_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OOB_ENABLE, GET_PARAM, 0},
	{"hw_mode_omn_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE_SWITCH_OMN_TIMER, SET_PARAM, 1},
	{"get_hw_mode_omn_timer",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE_SWITCH_OMN_TIMER, GET_PARAM, 0},
	{"hw_mode_omn_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE_SWITCH_OMN_ENABLE, SET_PARAM, 1},
	{"get_hw_mode_omn_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE_SWITCH_OMN_ENABLE, GET_PARAM, 0},
	{"hw_mode_primary",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HW_MODE_SWITCH_PRIMARY_IF, SET_PARAM, 1},
	{"g_psoc_num_vdevs",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_PSOC_NUM_VDEVS, GET_PARAM, 0},
	{"g_psoc_num_peers",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_PSOC_NUM_PEERS, GET_PARAM, 0},
	{"g_pdev_num_vdevs",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_PDEV_NUM_VDEVS, GET_PARAM, 0},
	{"g_pdev_num_peers",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_PDEV_NUM_PEERS, GET_PARAM, 0},
	{"g_pdev_num_monitor_vdevs",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_GET_PDEV_NUM_MONITOR_VDEVS, GET_PARAM, 0},
	{"set_opclass_tbl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OPCLASS_TBL, SET_PARAM, 1},
	{"get_opclass_tbl",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OPCLASS_TBL, GET_PARAM, 0},
#ifdef CE_TASKLET_DEBUG_ENABLE
	{"ce_debug_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_CE_LATENCY_STATS, SET_PARAM, 1},
#endif
#ifdef QCA_SUPPORT_ADFS_RCAC
	{"rCACEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ROLLING_CAC_ENABLE, SET_PARAM, 1},
	{"get_rCACEn",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ROLLING_CAC_ENABLE, GET_PARAM, 0},
	{"rCAC_freq",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONFIGURE_RCAC_FREQ, SET_PARAM, 1},
	{"get_rCACfreq",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CONFIGURE_RCAC_FREQ, GET_PARAM, 0},
#endif
#if ATH_SUPPORT_DFS
	{"scan_over_cac_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_SCAN_OVER_CAC, SET_PARAM, 1},
#endif
	{"setNxtRadarFreq",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NXT_RDR_FREQ, SET_PARAM, 1},
	{"getNxtRadarFreq",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NXT_RDR_FREQ, GET_PARAM, 0},
	{"rpt_max_phy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RPT_MAX_PHY, SET_PARAM, 1},
	{"g_rpt_max_phy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RPT_MAX_PHY, GET_PARAM, 0},
	{"setCountryID",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_COUNTRY_ID, SET_PARAM, 1},
	{"getCountryID",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_COUNTRY_ID, GET_PARAM, 0},
	{"dl_reportsize",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_REPORT_SIZE, SET_PARAM, 1},
	{"dl_tstamprez",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_TSTAMP_RESOLUTION, SET_PARAM, 1},
	{"dl_reporten",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_REPORTING_ENABLED, SET_PARAM, 1},
	{"dl_loglevel",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_LOG_LEVEL, SET_PARAM, 1},
	{"dl_vapon",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_VAP_ENABLE, SET_PARAM, 1},
	{"dl_vapoff",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_VAP_DISABLE, SET_PARAM, 1},
	{"dl_modon",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_MODULE_ENABLE, SET_PARAM, 1},
	{"dl_modoff",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_DBGLOG_MODULE_DISABLE, SET_PARAM, 1},
	{"disp_tpc",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_DISP_TPC, SET_PARAM, 1},
	{"setCH144",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_CH_144, SET_PARAM, 1},
	{"getCH144",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_CH_144, GET_PARAM, 0},
	{"setRegdomain",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_REGDOMAIN, SET_PARAM, 1},
	{"getRegdomain",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_REGDOMAIN, GET_PARAM, 0},
	{"enable_ol_stats",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_OL_STATS, SET_PARAM, 1},
	{"ol_stats_status",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_OL_STATS, GET_PARAM, 0},
	{"enable_macreq",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_MAC_REQ, SET_PARAM, 1},
	{"sens_level",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_SENS_LEVEL, SET_PARAM, 1},
	{"prof_id_enable",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_WLAN_PROFILE_ID_ENABLE, SET_PARAM, 1},
	{"prof_trigger",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_WLAN_PROFILE_TRIGGER, SET_PARAM, 1},
	{"setCH144EppOvrd",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_CH144_EPPR_OVRD, SET_PARAM, 1},
	{"getCH144EppOvrd",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_CH144_EPPR_OVRD, GET_PARAM, 0},
	{"data_txstats",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_PERPKT_TXSTATS, SET_PARAM, 1},
	{"g_data_txstats",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_PERPKT_TXSTATS, GET_PARAM, 0},
	{"enable_statsv2",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_OL_STATSv2, SET_PARAM, 1},
	{"enable_statsv3",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_ENABLE_OL_STATSv3, SET_PARAM, 1},
	{"bsta_fixed_idmask",
		OL_SPECIAL_PARAM_SHIFT | OL_SPECIAL_PARAM_BSTA_FIXED_IDMASK, SET_PARAM, 1},
	{"setHALparam",         35808, SET_PARAM, 1},
	{"getHALparam",         35809, GET_PARAM, 0},
	{"get_aggr_burst",      35821, GET_PARAM, 0},
	{"get_cfr_capture_status",
	 OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_CFR_CAPTURE_STATUS, GET_PARAM, 0},
	{"non_inherit_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NON_INHERIT_ENABLE, SET_PARAM, 1},
	{"g_non_inherit_enable",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NON_INHERIT_ENABLE, GET_PARAM, 0},
	{"nobckhlradio",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NO_BACKHAUL_RADIO, SET_PARAM, 1},
	{"g_nobckhlradio",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NO_BACKHAUL_RADIO, GET_PARAM, 0},
	{"set_triplets_bw",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_ADDITIONAL_TRIPLETS, SET_PARAM, 1},
	{"get_triplets_bw",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_ADDITIONAL_TRIPLETS, GET_PARAM, 0},
#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
	{"setpostNOLfreq",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_FREQ, SET_PARAM, 1},
	{"getpostNOLfreq",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_FREQ, GET_PARAM, 0},
	{"setpostNOLmode",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_MODE, SET_PARAM, 1},
	{"getpostNOLmode",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_MODE, SET_PARAM, 0},
	{"setpostNOLcfreq2",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_CFREQ2, SET_PARAM, 1},
	{"getpostNOLcfreq2",
		OL_ATH_PARAM_SHIFT | OL_ATH_DFS_CHAN_POSTNOL_CFREQ2, GET_PARAM, 0},
#endif
	{"dcs_wideband_policy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_WIDEBAND_POLICY, SET_PARAM, 1},
	{"get_dcs_wideband_policy",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_DCS_WIDEBAND_POLICY, GET_PARAM, 0},
#ifdef QCA_CBT_INSTRUMENTATION
	{"get_call_map",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_FUNC_CALL_MAP, SET_PARAM, 1},
#endif
	{"puncture_band",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PUNCTURED_BAND, SET_PARAM, 1},
	{"get_puncture_band",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_PUNCTURED_BAND, GET_PARAM, 0},
	{"he_mbssid_ctrl_frame_config",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_MBSSID_CTRL_FRAME_CONFIG, SET_PARAM, 1},
	{"g_he_mbssid_ctrl_frame_config",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_HE_MBSSID_CTRL_FRAME_CONFIG, GET_PARAM, 0},
	{"get_ofdma_usr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_OFDMA_MAX_USERS, GET_PARAM, 0},
	{"get_mumimo_usr",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MUMIMO_MAX_USERS, GET_PARAM, 0},
	{"acs_pcaconly",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_PRECAC_SUPPORT, SET_PARAM, 1},
	{"g_acs_pcaconly",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ACS_PRECAC_SUPPORT, GET_PARAM, 0},
	{"mbss_auto",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MBSS_AUTOMODE, SET_PARAM, 1},
	{"g_mbss_auto",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_MBSS_AUTOMODE, GET_PARAM, 0},
	{"selective_rnr_nontx",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_SELECTIVE_ADD,
		SET_PARAM, 1},
	{"g_selective_rnr_nontx",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_SELECTIVE_ADD,
		GET_PARAM, 0},
	{"rnr_unsolicited_prb_resp_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_UNSOLICITED_PROBE_RESP_ACTIVE, SET_PARAM, 1},
	{"g_rnr_unsolicited_prb_resp_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_UNSOLICITED_PROBE_RESP_ACTIVE, GET_PARAM, 0},
	{"rnr_member_ess_colocated_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_MEMBER_OF_ESS_24G_5G_CO_LOCATED, SET_PARAM, 1},
	{"g_rnr_member_ess_colocated_en",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_MEMBER_OF_ESS_24G_5G_CO_LOCATED, GET_PARAM, 0},
	{"get_rnr_stats",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_RNR_STATS, GET_PARAM, 0},
	{"g_nss_wifi_ol",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_NSS_WIFI_OFFLOAD_STATUS, GET_PARAM, 0},
	{"enable_ema",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_EMA, SET_PARAM, 1},
	{"dur_based_tx_mode_sselect",
		OL_ATH_PARAM_SHIFT | OL_ATH_PARAM_ENABLE_TX_MODE_SELECT, SET_PARAM, 1},
};
#endif
#endif
