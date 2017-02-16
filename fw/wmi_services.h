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

#ifndef _WMI_SERVICES_H_
#define _WMI_SERVICES_H_


#ifdef __cplusplus
extern "C" {
#endif



typedef  enum  {
    WMI_SERVICE_BEACON_OFFLOAD=0,           /* beacon offload */
    WMI_SERVICE_SCAN_OFFLOAD=1,             /* scan offload */
    WMI_SERVICE_ROAM_SCAN_OFFLOAD=2,        /* roam scan offload */
    WMI_SERVICE_BCN_MISS_OFFLOAD=3,         /* beacon miss offload */
    WMI_SERVICE_STA_PWRSAVE=4,              /* fake sleep + basic power save */
    WMI_SERVICE_STA_ADVANCED_PWRSAVE=5,     /* uapsd, pspoll, force sleep */
    WMI_SERVICE_AP_UAPSD=6,                 /* uapsd on AP */
    WMI_SERVICE_AP_DFS=7,                   /* DFS on AP */
    WMI_SERVICE_11AC=8,                     /* supports 11ac */
    WMI_SERVICE_BLOCKACK=9,                 /* Supports triggering ADDBA/DELBA from host*/
    WMI_SERVICE_PHYERR=10,                  /* PHY error */
    WMI_SERVICE_BCN_FILTER=11,              /* Beacon filter support */
    WMI_SERVICE_RTT=12,                     /* RTT (round trip time) support */
    WMI_SERVICE_WOW=13,                     /* WOW Support */
    WMI_SERVICE_RATECTRL_CACHE=14,          /* Rate-control caching */
    WMI_SERVICE_IRAM_TIDS=15,               /* TIDs in IRAM */
    WMI_SERVICE_ARPNS_OFFLOAD=16,           /* ARP NS Offload support for STA vdev */
    WMI_SERVICE_NLO=17,                     /* Network list offload service */
    WMI_SERVICE_GTK_OFFLOAD=18,             /* GTK offload */
    WMI_SERVICE_SCAN_SCH=19,                /* Scan Scheduler Service */
    WMI_SERVICE_CSA_OFFLOAD=20,             /* CSA offload service */
    WMI_SERVICE_CHATTER=21,                 /* Chatter service */
    WMI_SERVICE_COEX_FREQAVOID=22,          /* FW report freq range to avoid */
    WMI_SERVICE_PACKET_POWER_SAVE=23,       /* packet power save service */
    WMI_SERVICE_FORCE_FW_HANG=24,           /* Service to test the firmware recovery mechanism */
    WMI_SERVICE_GPIO=25,                    /* GPIO service */
    WMI_SERVICE_STA_DTIM_PS_MODULATED_DTIM=26, /* Modulated DTIM support */
    WMI_STA_UAPSD_BASIC_AUTO_TRIG=27,       /* Basic version of station UAPSD AC Trigger Generation Method with
                                             * variable tigger periods (service, delay, and suspend intervals) */
    WMI_STA_UAPSD_VAR_AUTO_TRIG=28,         /* Station UAPSD AC Trigger Generation Method with variable
                                             * trigger periods (service, delay, and suspend intervals) */
    WMI_SERVICE_STA_KEEP_ALIVE=29,          /* Serivce to support the STA KEEP ALIVE mechanism */
    WMI_SERVICE_TX_ENCAP=30,                /* Packet type for TX encapsulation */
    WMI_SERVICE_AP_PS_DETECT_OUT_OF_SYNC=31, /* detect out-of-sync sleeping stations */
    WMI_SERVICE_EARLY_RX=32,                /* adaptive early-rx feature */
    WMI_SERVICE_STA_SMPS=33,                /* STA MIMO-PS */
    WMI_SERVICE_FWTEST=34,                  /* Firmware test service */
    WMI_SERVICE_STA_WMMAC=35,               /* STA WMMAC */
    WMI_SERVICE_TDLS=36,                    /* TDLS support */
    WMI_SERVICE_BURST=37,                   /* SIFS spaced burst support */
    WMI_SERVICE_MCC_BCN_INTERVAL_CHANGE=38, /* Dynamic beaocn interval change for SAP/P2p GO in MCC scenario */
    WMI_SERVICE_ADAPTIVE_OCS=39,            /* Service to support adaptive off-channel scheduler */
    WMI_SERVICE_BA_SSN_SUPPORT=40,          /* target will provide Sequence number for the peer/tid combo */
    WMI_SERVICE_FILTER_IPSEC_NATKEEPALIVE=41,
    WMI_SERVICE_WLAN_HB=42,                 /* wlan HB service */
    WMI_SERVICE_LTE_ANT_SHARE_SUPPORT=43,   /* support LTE/WLAN antenna sharing */
    WMI_SERVICE_BATCH_SCAN=44,              /* Service to support batch scan */
    WMI_SERVICE_QPOWER=45,                  /* QPower service */
    WMI_SERVICE_PLMREQ=46,
    WMI_SERVICE_THERMAL_MGMT=47,            /* thermal throttling support */
    WMI_SERVICE_RMC=48,                     /* RMC support */
    WMI_SERVICE_MHF_OFFLOAD=49,             /* multi-hop forwarding offload */
    WMI_SERVICE_COEX_SAR=50,                /* target support SAR tx limit from WMI_PDEV_PARAM_TXPOWER_LIMITxG */
    WMI_SERVICE_BCN_TXRATE_OVERRIDE=51,     /* Will support the bcn/prb rsp rate override */
    WMI_SERVICE_NAN=52,                     /* Neighbor Awareness Network */
    WMI_SERVICE_L1SS_STAT=53,               /* L1SS statistics counter report */
    WMI_SERVICE_ESTIMATE_LINKSPEED=54,      /* Linkspeed Estimation per peer */
    WMI_SERVICE_OBSS_SCAN=55,               /* Service to support OBSS scan */
    WMI_SERVICE_TDLS_OFFCHAN=56,            /* TDLS off channel support */
    WMI_SERVICE_TDLS_UAPSD_BUFFER_STA=57,   /* TDLS UAPSD Buffer STA support */
    WMI_SERVICE_TDLS_UAPSD_SLEEP_STA=58,    /* TDLS UAPSD Sleep STA support */
    WMI_SERVICE_IBSS_PWRSAVE=59,            /* IBSS power save support */
    WMI_SERVICE_LPASS=60,                   /* Service to support LPASS */
    WMI_SERVICE_EXTSCAN=61,                 /* Extended Scans */
    WMI_SERVICE_D0WOW=62,                   /* D0-WOW Support */
    WMI_SERVICE_HSOFFLOAD=63,               /* Hotspot offload feature Support */
    WMI_SERVICE_ROAM_HO_OFFLOAD=64,         /* roam handover offload */
    WMI_SERVICE_RX_FULL_REORDER=65,         /* target-based Rx full reorder */
    WMI_SERVICE_DHCP_OFFLOAD=66,            /* DHCP offload support */
    WMI_SERVICE_STA_RX_IPA_OFFLOAD_SUPPORT=67, /* STA RX DATA offload to IPA support */
    WMI_SERVICE_MDNS_OFFLOAD=68,            /* mDNS responder offload support */
    WMI_SERVICE_SAP_AUTH_OFFLOAD=69,        /* softap auth offload */
    WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT=70, /* Dual Band Simultaneous support */
    WMI_SERVICE_OCB=71,                     /* OCB mode support */
    WMI_SERVICE_AP_ARPNS_OFFLOAD=72,        /* arp offload support for ap mode vdev */
    WMI_SERVICE_PER_BAND_CHAINMASK_SUPPORT=73, /* Per band chainmask support */
    WMI_SERVICE_PACKET_FILTER_OFFLOAD=74,   /* Per vdev packet filters */
    WMI_SERVICE_MGMT_TX_HTT=75,             /* Mgmt Tx via HTT interface */
    WMI_SERVICE_MGMT_TX_WMI=76,             /* Mgmt Tx via WMI interface */
    WMI_SERVICE_EXT_MSG=77,                 /* WMI_SERVICE_READY_EXT msg follows */
    WMI_SERVICE_MAWC=78,                    /* Motion Aided WiFi Connectivity (MAWC)*/
    WMI_SERVICE_PEER_ASSOC_CONF=79,         /* target will send ASSOC_CONF after ASSOC_CMD is processed */
    WMI_SERVICE_EGAP=80,                    /* enhanced green ap support */
    WMI_SERVICE_STA_PMF_OFFLOAD=81,         /* FW supports 11W PMF Offload for STA */
    WMI_SERVICE_UNIFIED_WOW_CAPABILITY=82,  /* FW supports unified D0 and D3 wow */
    WMI_SERVICE_ENHANCED_PROXY_STA=83,      /* Enhanced ProxySTA mode support */
    WMI_SERVICE_ATF=84,                     /* Air Time Fairness support */
    WMI_SERVICE_COEX_GPIO=85,               /* BTCOEX GPIO support */
    WMI_SERVICE_AUX_SPECTRAL_INTF=86,       /* Aux Radio enhancement support for ignoring spectral scan intf from main radios */
    WMI_SERVICE_AUX_CHAN_LOAD_INTF=87,      /* Aux Radio enhancement support for ignoring chan load intf from main radios*/
    WMI_SERVICE_BSS_CHANNEL_INFO_64=88,     /* BSS channel info (freq, noise floor, 64-bit counters) event support */
    WMI_SERVICE_ENTERPRISE_MESH=89,         /* Enterprise MESH Service Support */
    WMI_SERVICE_RESTRT_CHNL_SUPPORT=90,     /* Restricted Channel Support */
    WMI_SERVICE_BPF_OFFLOAD=91,             /* FW supports bpf offload */
    WMI_SERVICE_SYNC_DELETE_CMDS=92,        /* FW sends response event for Peer, Vdev delete commands */
    WMI_SERVICE_SMART_ANTENNA_SW_SUPPORT=93,
    WMI_SERVICE_SMART_ANTENNA_HW_SUPPORT=94,
    WMI_SERVICE_RATECTRL_LIMIT_MAX_MIN_RATES=95, /* allow per-peer tx MCS min/max limits by host */
    WMI_SERVICE_NAN_DATA=96,                /* FW supports NAN data */
    WMI_SERVICE_NAN_RTT=97,                 /* FW supports NAN RTT */
    WMI_SERVICE_11AX=98,                    /* FW supports 802.11ax */
    /* WMI_SERVICE_DEPRECATED_REPLACE
     * FW supports these new WMI commands, to be used rather than
     * deprecated matching commands:
     * - WMI_PDEV_SET_PCL_CMDID          (vs. WMI_SOC_SET_PCL_CMDID)
     * - WMI_PDEV_SET_HW_MODE_CMDID      (vs. WMI_SOC_SET_HW_MODE_CMDID)
     * - WMI_PDEV_SET_MAC_CONFIG_CMDID   (vs. WMI_SOC_SET_DUAL_MAC_CONFIG_CMDID)
     * - WMI_PDEV_SET_ANTENNA_MODE_CMDID (vs. WMI_SOC_SET_ANTENNA_MODE_CMDID)
     * - WMI_VDEV_SET_DSCP_TID_MAP_CMDID (vs. WMI_VDEV_SET_WMM_PARAMS_CMDID)
     */
    WMI_SERVICE_DEPRECATED_REPLACE=99,
    WMI_SERVICE_TDLS_CONN_TRACKER_IN_HOST_MODE=100, /* FW supports a new mode that allows to run connection tracker in host */
    WMI_SERVICE_ENHANCED_MCAST_FILTER=101,  /* FW supports enhanced multicast filtering (of mcast IP inside ucast WLAN) */
    WMI_SERVICE_PERIODIC_CHAN_STAT_SUPPORT=102, /* periodic channel stats service */
    WMI_SERVICE_MESH_11S=103,               /* 11s mesh service support */
    WMI_SERVICE_HALF_RATE_QUARTER_RATE_SUPPORT=104, /* FW+HW supports 10 MHz (half rate) and 5 MHz (quarter rate) channel bandwidth */
    WMI_SERVICE_VDEV_RX_FILTER=105,         /* Support per-vdev specs of which rx frames to filter out */
    WMI_SERVICE_P2P_LISTEN_OFFLOAD_SUPPORT=106,
    WMI_SERVICE_MARK_FIRST_WAKEUP_PACKET=107, /* FW supports marking the first data packet which wakes the host from suspend */
    WMI_SERVICE_MULTIPLE_MCAST_FILTER_SET=108, /* FW supports command that can add/delete multiple mcast filters */
    /* WMI_SERVICE_HOST_MANAGED_RX_REORDER -
     * FW supports host-managed RX reorder.
     * Host managed RX reorder involves RX BA state machine handling
     * on peer/TID basis, REO configuration for HW based reordering/PN
     * check and processing reorder exceptions generated by HW.
     */
    WMI_SERVICE_HOST_MANAGED_RX_REORDER=109,
    /* Specify whether the target supports the following WMI messages for
     * reading / writing its flash memory:
     *     WMI_READ_DATA_FROM_FLASH_CMDID,
     *     WMI_READ_DATA_FROM_FLASH_EVENTID,
     *     WMI_TRANSFER_DATA_TO_FLASH_CMDID,
     *     WMI_TRANSFER_DATA_TO_FLASH_COMPLETE_EVENTID,
     */
    WMI_SERVICE_FLASH_RDWR_SUPPORT=110,
    WMI_SERVICE_WLAN_STATS_REPORT=111, /* support WLAN stats report */
    /* WMI_SERVICE_TX_MSDU_ID_NEW_PARTITION_SUPPORT -
     * FW supports bigger MSDU ID partition which is defined as HTT_TX_IPA_NEW_MSDU_ID_SPACE_BEGIN
     * When both host and FW support new partition, FW uses HTT_TX_IPA_NEW_MSDU_ID_SPACE_BEGIN
     * If host doesn't support, FW falls back to HTT_TX_IPA_MSDU_ID_SPACE_BEGIN
     * Handshaking is done through WMI_INIT and WMI service ready
     */
    WMI_SERVICE_TX_MSDU_ID_NEW_PARTITION_SUPPORT=112, /* support bigger MSDU ID partition */
    WMI_SERVICE_DFS_PHYERR_OFFLOAD=113,
    WMI_SERVICE_RCPI_SUPPORT=114,
    WMI_SERVICE_FW_MEM_DUMP_SUPPORT=115, /* Support FW Memory dump */
    WMI_SERVICE_PEER_STATS_INFO=116, /* support per peer stats info */
    WMI_SERVICE_REGULATORY_DB=117, /* support regulatory database in FW */
    WMI_SERVICE_11D_OFFLOAD=118, /* support 11D scan offload in FW */
    WMI_SERVICE_HW_DATA_FILTERING=119,
    WMI_SERVICE_MULTIPLE_VDEV_RESTART=120, /* Support for single command for multiple vdev restart */
    WMI_SERVICE_PKT_ROUTING=121, /* Support for routing specific data packets to selected destination rings */
    WMI_SERVICE_CHECK_CAL_VERSION=122, /* Support cal version check */


    /******* ADD NEW SERVICES HERE UNTIL ALL VALUES UP TO 128 ARE USED *******/

    WMI_MAX_SERVICE=128, /* max service */

    /* NOTE:
     * The above service flags are delivered in the wmi_service_bitmap field
     * of the WMI_SERVICE_READY_EVENT message.
     * The below service flags are delivered in a WMI_SERVICE_AVAILABLE_EVENT
     * message rather than in the WMI_SERVICE_READY_EVENT message's
     * wmi_service_bitmap field.
     * The WMI_SERVICE_AVAILABLE_EVENT message immediately precedes the
     * WMI_SERVICE_READY_EVENT message.
     */

    /*PUT 1ST EXT SERVICE HERE:*//*WMI_SERVICE_xxxxxxxx=128,*/
    /*PUT 2ND EXT SERVICE HERE:*//*WMI_SERVICE_yyyyyyyy=129,*/

    WMI_MAX_EXT_SERVICE

} WMI_SERVICE;

#define WMI_SERVICE_BM_SIZE   ((WMI_MAX_SERVICE + sizeof(A_UINT32)- 1)/sizeof(A_UINT32))

#define WMI_NUM_EXT_SERVICES (WMI_MAX_EXT_SERVICE - WMI_MAX_SERVICE)
#define WMI_SERVICE_EXT_BM_SIZE32 ((WMI_NUM_EXT_SERVICES + 31) / 32)

#define WMI_SERVICE_ROAM_OFFLOAD WMI_SERVICE_ROAM_SCAN_OFFLOAD /* depreciated the name WMI_SERVICE_ROAM_OFFLOAD, but here to help compiling with old host driver */

/*
 * turn on the WMI service bit corresponding to  the WMI service.
 */
#define WMI_SERVICE_ENABLE(pwmi_svc_bmap,svc_id) \
    ( (pwmi_svc_bmap)[(svc_id)/(sizeof(A_UINT32))] |= \
         (1 << ((svc_id)%(sizeof(A_UINT32)))) )

#define WMI_SERVICE_DISABLE(pwmi_svc_bmap,svc_id) \
    ( (pwmi_svc_bmap)[(svc_id)/(sizeof(A_UINT32))] &=  \
      ( ~(1 << ((svc_id)%(sizeof(A_UINT32)))) ) )

#define WMI_SERVICE_IS_ENABLED(pwmi_svc_bmap,svc_id) \
    ( ((pwmi_svc_bmap)[(svc_id)/(sizeof(A_UINT32))] &  \
       (1 << ((svc_id)%(sizeof(A_UINT32)))) ) != 0)


#define WMI_SERVICE_EXT_ENABLE(pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id) \
    do { \
        if (svc_id < WMI_MAX_SERVICE) { \
            WMI_SERVICE_ENABLE(pwmi_svc_bmap, svc_id); \
        } else { \
            int word = ((svc_id) - WMI_MAX_SERVICE) / 32; \
            int bit = (svc_id) & 0x1f; /* svc_id mod 32 */ \
            (pwmi_svc_ext_bmap)[word] |= (1 << bit); \
        } \
    } while (0)

#define WMI_SERVICE_EXT_DISABLE(pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id) \
    do { \
        if (svc_id < WMI_MAX_SERVICE) { \
            WMI_SERVICE_DISABLE(pwmi_svc_bmap, svc_id); \
        } else { \
            int word = ((svc_id) - WMI_MAX_SERVICE) / 32; \
            int bit = (svc_id) & 0x1f; /* svc_id mod 32 */ \
            (pwmi_svc_ext_bmap)[word] &= ~(1 << bit); \
        } \
    } while (0)

#define WMI_SERVICE_EXT_IS_ENABLED(pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id) \
    /* If the service ID is beyond the known limit, treat it as disabled */ \
    ((svc_id) >= WMI_MAX_EXT_SERVICE ? 0 : \
        /* If service ID is in the non-extension range, use the old check */ \
        (svc_id) < WMI_MAX_SERVICE ? \
            WMI_SERVICE_IS_ENABLED(pwmi_svc_bmap, svc_id) : \
            /* If service ID is in the extended range, check ext_bmap */ \
            (pwmi_svc_ext_bmap)[((svc_id) - WMI_MAX_SERVICE) / 32] >> \
                ((svc_id) & 0x1f))


#ifdef __cplusplus
}
#endif

#endif /*_WMI_SERVICES_H_*/
