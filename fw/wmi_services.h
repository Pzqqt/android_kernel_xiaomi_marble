/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
    WMI_SERVICE_THERMAL_MGMT=47,            /* thermal throttling support,
                                             * compared with
                                             * WMI_SERVICE_THERM_THROT,
                                             * this service only reports
                                             * current temperature to host,
                                             * and host will do the thermal
                                             * throttling. */
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
    WMI_SERVICE_OFFCHAN_TX_WMI=123, /* Support offchan data/mgmt tx over wmi */
    WMI_SERVICE_8SS_TX_BFEE = 124, /* support for 8Ant Bfee */
    WMI_SERVICE_EXTENDED_NSS_SUPPORT=125, /* Extend NSS support for 80Mhz and 160Mhz */
    WMI_SERVICE_ACK_TIMEOUT=126, /* Support TX ack timeout configurations */
    WMI_SERVICE_PDEV_BSS_CHANNEL_INFO_64 = 127, /* BSS channel info (freq, noise floor, rx clear, cycles 64-bit counters) event support */

    WMI_MAX_SERVICE=128, /* max service */

    /* NOTE:
     * The above service flags are delivered in the wmi_service_bitmap field
     * of the WMI_SERVICE_READY_EVENT message.
     * The below service flags are not delivered in the
     * WMI_SERVICE_READY_EVENT message's wmi_service_bitmap field,
     * but instead are delivered in the
     *     fixed_param.wmi_service_segment_bitmap portion
     * of the WMI_SERVICE_AVAILABLE_EVENT message, with
     *     fixed_param.wmi_service_segment_offset
     * set to 128.
     * The WMI_SERVICE_AVAILABLE_EVENT message immediately precedes the
     * WMI_SERVICE_READY_EVENT message.
     */

    WMI_SERVICE_CHAN_LOAD_INFO=128, /* The values in WMI_CHAN_INFO_EVENTID is the difference in cycle counters */
    WMI_SERVICE_TX_PPDU_INFO_STATS_SUPPORT=129, /* support to report tx ppdu info stats via htt events */
    WMI_SERVICE_VDEV_LIMIT_OFFCHAN_SUPPORT=130, /* support to report the offchannel duration limiting capability on connected interface */
    WMI_SERVICE_FILS_SUPPORT=131, /* support for FILS */
    WMI_SERVICE_WLAN_OIC_PING_OFFLOAD=132, /* Support for wlan OIC ping service */
    WMI_SERVICE_WLAN_DHCP_RENEW=133, /* Support for wlan DHCP Renew service */
    WMI_SERVICE_MAWC_SUPPORT = 134, /* Support for MAWC service */
    WMI_SERVICE_VDEV_LATENCY_CONFIG=135, /* support for vdev latency config */
    WMI_SERVICE_PDEV_UPDATE_CTLTABLE_SUPPORT=136, /* support for pdev update ctl table */
    WMI_SERVICE_PKTLOG_SUPPORT_OVER_HTT=137, /* upload pktlog data over HTT communication channel */
    WMI_SERVICE_VDEV_MULTI_GROUP_KEY_SUPPORT=138, /* Support for vdev level multi group key */
    WMI_SERVICE_SCAN_PHYMODE_SUPPORT=139, /* Support for phymode also with channel in offchan WMI_START_SCAN_CMDID */
    WMI_SERVICE_THERM_THROT = 140, /* Support thermal cfg and indication on AP.
                                    * Compared with WMI_SERVICE_THERMAL_MGMT,
                                    * this service is to allow thermal tool to
                                    * configure thermal throttling threshold
                                    * for different levels and pass down duty
                                    * cycle value. FW will also report thermal
                                    * throttling status to host through this
                                    * service. */
    WMI_SERVICE_BCN_OFFLOAD_START_STOP_SUPPORT=141, /* support dynamically enabling / disabling beacon tx offload */
    WMI_SERVICE_WOW_WAKEUP_BY_TIMER_PATTERN=142, /* fw to support waking up host from wow pattern timer */
    WMI_SERVICE_PEER_MAP_UNMAP_V2_SUPPORT=143, /* Support for HTT peer map/unmap V2 */
    WMI_SERVICE_OFFCHAN_DATA_TID_SUPPORT=144, /* Support new tid for offchan data/mgmt tx over wmi */
    WMI_SERVICE_RX_PROMISC_ENABLE_SUPPORT=145, /* Support enabling/disabling rx promiscuous mode as directed by a WMI message from the host */
    WMI_SERVICE_SUPPORT_DIRECT_DMA=146, /* indicates target supports direct DMA,
                                         * host will rely on WMI_DMA_RING_CAPABILITIES to get supported modules */
    WMI_SERVICE_AP_OBSS_DETECTION_OFFLOAD=147, /* Support SAP mode OBSS detection offload */
    WMI_SERVICE_11K_NEIGHBOUR_REPORT_SUPPORT=148, /* Support for 11k neighbor report */
    WMI_SERVICE_LISTEN_INTERVAL_OFFLOAD_SUPPORT=149, /* Support listen interval offload */
    WMI_SERVICE_BSS_COLOR_OFFLOAD=150, /* Support BSS color change for STA, OBSS color collision detection in FW for AP and STA */
    WMI_SERVICE_RUNTIME_DPD_RECAL=151, /* Runtime DPD recalibration support */
    WMI_SERVICE_STA_TWT=152, /* support for TWT (Target Wake Time) of STA */
    WMI_SERVICE_AP_TWT=153, /* support for TWT (Target Wake Time) on AP */
    WMI_SERVICE_GMAC_OFFLOAD_SUPPORT=154, /* Support for GMAC */
    WMI_SERVICE_SPOOF_MAC_SUPPORT=155,  /* support for SERVICE_SPOOF_MAC */
    WMI_SERVICE_PEER_TID_CONFIGS_SUPPORT=156,  /* Support TID specific configurations per peer (ack,aggr,retry,rate) */
    WMI_SERVICE_VDEV_SWRETRY_PER_AC_CONFIG_SUPPORT=157,  /* Support vdev software retries configuration per AC (non aggr retry/aggr retry) */
    WMI_SERVICE_DUAL_BEACON_ON_SINGLE_MAC_SCC_SUPPORT=158, /* Support dual beacon on same channel on single MAC */
    WMI_SERVICE_DUAL_BEACON_ON_SINGLE_MAC_MCC_SUPPORT=159, /* Support dual beacon on different channel on single MAC */
    WMI_SERVICE_MOTION_DET=160, /* support for motion detection config */
    WMI_SERVICE_INFRA_MBSSID=161, /* support infra multi-BSSID feature */
    WMI_SERVICE_OBSS_SPATIAL_REUSE=162, /* support spatial reuse feature */
    WMI_SERVICE_VDEV_DIFFERENT_BEACON_INTERVAL_SUPPORT=163, /* Support different beacon intervals on different VDEVs */
    WMI_SERVICE_NAN_DBS_SUPPORT=164, /* Support DBS for NAN discovery interface */
    WMI_SERVICE_NDI_DBS_SUPPORT=165, /* Support DBS for NAN data interface */
    WMI_SERVICE_NAN_SAP_SUPPORT=166, /* Support SAP Concurrency for NAN Discovery interface */
    WMI_SERVICE_NDI_SAP_SUPPORT=167, /* Support SAP Concurrency for NAN Data interface */
    WMI_SERVICE_CFR_CAPTURE_SUPPORT=168, /* Support to capture uncompressed Channel Frequency Response (CFR) */
    WMI_SERVICE_CFR_CAPTURE_IND_MSG_TYPE_1=169, /* Message type HTT_PEER_CFR_CAPTURE_MSG_TYPE_1 in HTT_T2H_MSG_TYPE_CFR_DUMP_COMPL_IND */
    WMI_SERVICE_ESP_SUPPORT=170, /* Support for Estimated Service Params IE */
    WMI_SERVICE_PEER_CHWIDTH_CHANGE = 171, /* Support for host to update/re-intersect the node capability */
    WMI_SERVICE_WLAN_HPCS_PULSE=172, /* Support for High Precision Clock Synchronization feature */
    WMI_SERVICE_PER_VDEV_CHAINMASK_CONFIG_SUPPORT=173, /* Support for configuring chainmask per VDEV */
    WMI_SERVICE_TX_DATA_MGMT_ACK_RSSI=174, /* ACK RSSI indication to host for host TX data and mgmt frame */
    WMI_SERVICE_NAN_DISABLE_SUPPORT=175, /* indicates firmware is dependent on host to disable NAN incase of concurrencies */
    WMI_SERVICE_NAN_DISABLE_SUPPORT__prototype = WMI_SERVICE_NAN_DISABLE_SUPPORT, /* alias, to clarify that NAN_DISABLE_SUPPORT is for prototype testing purposes */
    WMI_SERVICE_HTT_H2T_NO_HTC_HDR_LEN_IN_MSG_LEN=176, /* indicates FW supports uniformly excluding the HTC header length from the HTT H2T message length */
    WMI_SERVICE_COEX_SUPPORT_UNEQUAL_ISOLATION=177, /* indicates FW supports FDD coex with unequal isolation between BT and each of the WLAN chains */

    /* WMI_SERVICE_HW_DB2DBM_CONVERSION_SUPPORT:
     * Support HW+FW db2dbm conversion for RSSI fields in rx descriptors
     * and host/target messages.
     * If enabled, HW and FW will convert SNR to RSSI by adding noise floor
     * and rssi_offset from BDF to RSSI values that formerly had units of
     * dB w.r.t. noise floor to convert the units to dBm.
     * MAC beacon RSSI average register return a signed value for RSSI,
     * as well as hw descriptors.
     *-------------------------------------------------------------------------
     * The RSSI field of below WMI messages will be changed to dBm units:
     * WMI_MGMT_RX_EVENTID:
     *     wmi_mgmt_rx_hdr.rssi_ctl;
     * WMI_UPDATE_STATS_EVENTID:
     *     wmi_rssi_stats.rssi_avg_beacon;
     *     wmi_rssi_stats.rssi_avg_data;
     *     wmi_snr_info.bcn_snr;
     *     wmi_snr_info.dat_snr;
     *     wmi_vdev_stats.bcn_rssi_history; (NOT USED IN FW)
     *     wmi_peer_stats.peer_rssi;
     * WMI_WOW_WAKEUP_HOST_EVENTID:
     *     wmi_rssi_breach_event_fixed_param.rssi;
     *     wmi_roam_event_fixed_param.rssi;
     * WMI_PEER_STA_KICKOUT_EVENTID:
     *     wmi_peer_sta_kickout_event_fixed_param.rssi;
     * WMI_PASSPOINT_MATCH_EVENTID:
     *     wmi_passpoint_event_hdr.rssi;(NOT USED IN FW)
     * WMI_PEER_INFO_EVENTID:
     *     wmi_peer_info.rssi;
     * WMI_ROAM_SYNCH_EVENTID:
     *     wmi_roam_synch_event_fixed_param.rssi;
     * WMI_ROAM_SCAN_STATS_EVENTID:
     *     wmi_roam_scan_stats_event_fixed_param.rssi;
     *     wmi_pdev_div_rssi_antid_event_id:
     *     wmi_pdev_div_rssi_antid_event_fixed_param.chain_rssi;
     *     wmi_rssi_breach_event_id
     * WMI_INST_RSSI_STATS_EVENTID:
     *     wmi_inst_rssi_stats_resp_fixed_param.iRSSI;
     * RSSI thresholds configured by host
     * WMI_ROAM_SCAN_RSSI_THRESHOLD
     *     roam_scan_rssi_thresh   snr
     *     boost_threshold_5g  snr
     *     penalty_threshold_5g    snr
     *     good_rssi_threshold snr
     *     roam_bg_scan_bad_rssi_thresh    snr
     *     roam_earlystop_thres_min    snr
     *     roam_earlystop_thres_max    snr
     * WMI_ROAM_AP_PROFILE
     *     rssi_abs_thresh snr
     * WMI_ROAM_CONFIGURE_MAWC_CMDID:
     *     best_ap_rssi_threshold  Snr
     *     wmi_ap_profile.rssi_abs_thresh;
     * WMI_ROAM_SCAN_RSSI_THRESHOLD:
     *     wmi_roam_scan_extended_threshold_param.boost_threshold_5g;
     *     wmi_roam_scan_extended_threshold_param.penalty_threshold_5g;
     *     wmi_roam_scan_extended_threshold_param.good_rssi_threshold;
     *     wmi_roam_scan_rssi_threshold_fixed_param.roam_scan_rssi_thresh;
     *     wmi_roam_bg_scan_roaming_param.roam_bg_scan_bad_rssi_thresh;
     * WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID:
     *     wmi_vdev_spectral_configure_cmd_fixed_param.spectral_scan_rssi_rpt_mode;
     *     wmi_vdev_spectral_configure_cmd_fixed_param.spectral_scan_rssi_thr;
     * WMI_RSSI_BREACH_MONITOR_CONFIG_CMDID:
     *     wmi_rssi_breach_monitor_config_fixed_param.low_rssi_breach_threshold;
     *     wmi_rssi_breach_monitor_config_fixed_param.hi_rssi_breach_threshold;
     * WMI_STA_SMPS_PARAM_CMDID:
     *     wmi_sta_smps_param.value of below cmd IDs:
     *         // RSSI threshold to enter Dynamic SMPS mode from inactive mode
     *         WMI_STA_SMPS_PARAM_UPPER_RSSI_THRESH = 0,
     *         // RSSI threshold to enter Stalled-D-SMPS mode from D-SMPS mode
     *         // or to enter D-SMPS mode from Stalled-D-SMPS mode
     *         WMI_STA_SMPS_PARAM_STALL_RSSI_THRESH = 1,
     *         // RSSI threshold to disable SMPS modes
     *         WMI_STA_SMPS_PARAM_LOWER_RSSI_THRESH = 2,
     *         // Upper threshold for beacon-RSSI. Used to reduce RX chainmask.
     *         WMI_STA_SMPS_PARAM_UPPER_BRSSI_THRESH = 3,
     *         // Lower threshold for beacon-RSSI. Used to increase RX chainmask
     *         WMI_STA_SMPS_PARAM_LOWER_BRSSI_THRESH = 4,
     *         // Enable/Disable DTIM 1chRx feature
     *         WMI_STA_SMPS_PARAM_DTIM_1CHRX_ENABLE = 5
     * WMI_TDLS_SET_STATE_CMDID:
     *     wmi_tdls_set_state_cmd_fixed_param.rssi_teardown_threshold;
     *     wmi_tdls_set_state_cmd_fixed_param.rssi_delta;
     *-------------------------------------------------------------------------
     * The RSSI fields of below HTT data type will change to dBm units:
     * PREPACK struct htt_tx_wbm_completion.ack_frame_rssi;
     * PREPACK struct htt_tx_wbm_transmit_status.ack_frame_rssi;
     * htt_ppdu_stats_user_cmpltn_common_tlv.ack_rssi;
     */
    WMI_SERVICE_HW_DB2DBM_CONVERSION_SUPPORT = 178,
    WMI_SERVICE_SUPPORT_EXTEND_ADDRESS=179, /* indicates firmware supports host memory addresses larger than 32 bit */
    WMI_SERVICE_BEACON_RECEPTION_STATS=180, /* Support per vdev beacon stats info */
    WMI_SERVICE_FETCH_TX_PN=181,
    WMI_SERVICE_PEER_UNMAP_RESPONSE_SUPPORT = 182, /* support peer ids unmap response from host */
    WMI_SERVICE_TX_PER_PEER_AMPDU_SIZE = 183, /* indicate FW support per peer TX AMPDU size */
    WMI_SERVICE_BSS_COLOR_SWITCH_COUNT = 184, /* Firmware supports bss-color switch count handling */
    WMI_SERVICE_HTT_PEER_STATS_SUPPORT = 185, /* Supports the feature where FW sends peer stats autonomously to Host via the HTT_T2H PEER_STATS_IND message */
    WMI_SERVICE_UL_RU26_ALLOWED = 186, /* indicates support for RU26 in UL OFDMA */
    WMI_SERVICE_GET_MWS_COEX_STATE = 187, /* FW provides MWS Coex info */
    WMI_SERVICE_GET_MWS_DPWB_STATE = 188, /* FW provides LTE-Coex Dynamic Power Back-off info */
    WMI_SERVICE_GET_MWS_TDM_STATE = 189, /* FW provides LTE-Coex TDM info */
    WMI_SERVICE_GET_MWS_IDRX_STATE = 190, /* FW provides LTE-Coex IDRx info */
    WMI_SERVICE_GET_MWS_ANTENNA_SHARING_STATE = 191, /* FW provides LTE-Coex Antenna sharing info */
    WMI_SERVICE_ENHANCED_TPC_CONFIG_EVENT = 192, /* FW provides enhanced tx power control configuration dump */
    WMI_SERVICE_WLM_STATS_REQUEST = 193, /* FW supports WLAN latency manager stats request */
    WMI_SERVICE_EXT_PEER_TID_CONFIGS_SUPPORT = 194, /* Extended Peer Tid configuration support for QoS related settings */
    WMI_SERVICE_WPA3_FT_SAE_SUPPORT = 195, /* FW roaming support for WPA3_FT_SAE */
    WMI_SERVICE_WPA3_FT_SUITE_B_SUPPORT = 196, /* FW roaming support for WPA3_FT_SUITE_B */
    WMI_SERVICE_VOW_ENABLE=197, /* FW supports a set of features to optimize VoW performance */
    WMI_SERVICE_CFR_CAPTURE_IND_EVT_TYPE_1 = 198, /* support WMI_PEER_CFR_CAPTURE_EVENT msg */
    WMI_SERVICE_BROADCAST_TWT = 199,  /* support of Broadcast TWT (Target Wake Time) for STA/AP */
    WMI_SERVICE_RAP_DETECTION_SUPPORT = 200, /* indicate FW supports rogue AP detection */
    WMI_SERVICE_PS_TDCC = 201, /* FW support tx_duty_cycle_control powersave */
    WMI_SERVICE_THREE_WAY_COEX_CONFIG_LEGACY   = 202, /* BTCOEX Three-way CoEx Config Legacy Feature support */
    WMI_SERVICE_THREE_WAY_COEX_CONFIG_OVERRIDE = 203, /* BTCOEX Three-way CoEx Config Override Feature support */
    WMI_SERVICE_TX_PWR_PER_PEER = 204, /* target supports per-peer tx pwr spec via WMI_PEER_USE_FIXED_PWR */
    WMI_SERVICE_STA_PLUS_STA_SUPPORT = 205, /* indicates target supports STA + STA concurrency */
    WMI_SERVICE_WPA3_FT_FILS = 206,
    WMI_SERVICE_ADAPTIVE_11R_ROAM = 207, /* Indicates FW supports adaptive 11r roaming */
    WMI_SERVICE_CHAN_RF_CHARACTERIZATION_INFO = 208, /* FW provides RF scores for chans in the service ready extension msg */
    WMI_SERVICE_FW_IFACE_COMBINATION_SUPPORT = 209, /* FW sends WMI_IFACE_COMBINATION_IND_EVENT msg immediately after WMI_SERVICE_READY_EXT_EVENT msg */
    WMI_SERVICE_TX_COMPL_TSF64 = 210, /* FW supports 64-bit tx TSF in HTT_T2H TX_COMPL_IND msg */
    WMI_SERVICE_DSM_ROAM_FILTER = 211, /* FW supports data stall AP mitigation while roaming */
    WMI_SERVICE_PACKET_CAPTURE_SUPPORT = 212, /* target supports packet capture Mode (SMART MU) */
    WMI_SERVICE_PER_PEER_HTT_STATS_RESET = 213, /* FW supports HTT per peer stats reset facility */
    WMI_SERVICE_DELETE_ALL_PEER_SUPPORT = 214, /* target supports cmd to delete all peers within a vdev */
    WMI_SERVICE_DYNAMIC_HW_MODE_SWITCH_SUPPORT = 215, /* target supports Dynamic HW mode switch */
    WMI_SERVICE_MSDU_FLOW_OVERRIDE_BY_HOST = 216, /* target supports flow override feature */
    WMI_SERVICE_WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENT= 217, /* target will send WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENT */
    WMI_SERVICE_RX_FSE_SUPPORT = 218, /* target supports flow search through RxOLE FSE hw block */
    WMI_SERVICE_FREQINFO_IN_METADATA = 219, /* FW provides freq_info during spectral scan */
    WMI_SERVICE_EXT2_MSG = 220, /* WMI_SERVICE_READY_EXT2 msg is sent by target */
    WMI_SERVICE_WPA3_SAE_ROAM_SUPPORT = 221, /* Indicates FW supports WPA3 SAE roaming */
    WMI_SERVICE_WPA3_OWE_ROAM_SUPPORT = 222, /* Indicates FW supports WPA3 OWE roaming */
    WMI_SERVICE_AUDIO_AGGR = 223, /* Indicates FW supports audio frame aggregation */
    WMI_SERVICE_6GHZ_SUPPORT = 224, /* Indicates FW supports 6GHZ (scan, connection and so on) */
    /* WMI_SERVICE_QMI_STATS_SUPPORT
     * Indicates FW supports stat request command WMI_REQUEST_LINK_STATS_CMDID
     * and WMI_REQUEST_STATS_CMDID coming from QMI which will be used in
     * runtime PM suspend.
     */
    WMI_SERVICE_QMI_STATS_SUPPORT = 225,
    WMI_SERVICE_CFR_CAPTURE_FILTER_SUPPORT = 226, /* Indicate FW Supports Channel Frequency Response (CFR) via WMI_CFR_CAPTURE_FILTER_CMDID */
    WMI_SERVICE_STA_BSS_MAX_IDLE_TIME = 227, /* Indicate FW supports BSS Max Idle time feature via WMI_VDEV_BSS_MAX_IDLE_TIME_CMDID */
    WMI_SERVICE_BIP_CIPHER_SUPPORT = 228, /* FW supports new BIP_CIPHER suites (WMI_CIPHER_BIP_xxx) */
    WMI_SERVICE_BW_165MHZ_SUPPORT = 229, /* Indicate FW supports bandwidth 165MHz (i.e. 6 GHz in addition to 2.4 and 5) */
    WMI_SERVICE_BW_RESTRICTED_80P80_SUPPORT = WMI_SERVICE_BW_165MHZ_SUPPORT,
    WMI_SERVICE_NAN_NDI_SAP_SAP_SCC_SUPPORT = 230, /* Support SAP + SAP + NAN discovery + NDI concurrency in SCC mode */
    WMI_SERVICE_NAN_VDEV_SUPPORT = 231, /* indicates firmware is dependent on host to create NAN vdev */
    WMI_SERVICE_AUDIO_SYNC_SUPPORT = 232, /* Indicate FW supports Audio sync feature */
    WMI_SERVICE_DUAL_STA_ROAM_SUPPORT = 233, /* Indidate FW support dual STA roaming */
    WMI_SERVICE_PEER_CREATE_CONF = 234, /* Target will send WMI_PEER_CREATE_CONF_EVENTID after WMI_PEER_CREATE_CMDID is processed */
    WMI_SERVICE_MULTIPLE_VDEV_RESTART_RESPONSE_SUPPORT = 235, /* indicates firmware supports Multiple vdev restart response */
    WMI_SERVICE_ROAM_SCAN_CHANNEL_LIST_TO_HOST_SUPPORT = 236, /* Indicates firmware supports sending roam scan channel list to host */
    WMI_SERVICE_PEER_DELETE_NO_PEER_FLUSH_TIDS_CMD = 237, /* Host should not send WMI_PEER_FLUSH_TIDS_CMD as part of peer delete */
    WMI_SERVICE_NSS_RATIO_TO_HOST_SUPPORT = 238, /* Indicates firmware supports sending NSS ratio info to host */
    WMI_SERVICE_WPA3_SUITEB_ROAM_SUPPORT = 239, /* Indicates FW supports WPA3 SUITE B roaming */
    WMI_SERVICE_PERIODIC_FRAME_INJECT_SUPPORT = 240, /* Indicates FW supports periodic frame injection */
    WMI_SERVICE_NDI_NDI_STA_SUPPORT = 241, /* Indicates FW support for STA+NDI+NDI */
    WMI_SERVICE_BW_TRUE_160_SUPPORT = 242, /* Indicates FW supports true 160 BW */
    WMI_SERVICE_HOST_SCAN_STOP_VDEV_ALL_SUPPORT = 243, /* Indicates FW supports scan stop mode WMI_SCN_STOP_HOST_VAP_ALL */
    WMI_SERVICE_BEACON_PROTECTION_SUPPORT = 244, /* Indicates FW supports WPA3 Beacon protection */
    WMI_SERVICE_EMA_AP_SUPPORT = 245, /* FW supports EMA AP feature */
    WMI_SERVICE_PEER_POWER_SAVE_DURATION_SUPPORT = 246, /* Support for adding Power save duration per client */
    WMI_SERVICE_5_DOT_9GHZ_SUPPORT = 247, /* Indicates FW supports new 5.9GHZ (scan, connection and so on) */
    WMI_SERVICE_MU_PREAMBLE_PUNCTURE_SUPPORT = 248, /* Indicates FW supports MU preamble puncture */
    WMI_SERVICE_SRG_SRP_SPATIAL_REUSE_SUPPORT = 249, /* Support for SRG, SRP based spatial reuse support */
    WMI_REQUEST_CTRL_PATH_STATS_REQUEST = 250, /* FW supports control path stats */
    WMI_SERVICE_TPC_STATS_EVENT = 251, /* FW support to dump the TPC tables */
    WMI_SERVICE_NO_INTERBAND_MCC_SUPPORT = 252, /* Indicates FW doesn't support interband MCC */
    WMI_SERVICE_MBSS_PARAM_IN_VDEV_START_SUPPORT = 253, /* FW supports VDEV's MBSS param exchange in VDEV start command */
    WMI_SERVICE_CONFIGURE_ROAM_TRIGGER_PARAM_SUPPORT = 254, /* FW supports ROAM trigger configuration param TLV */
    WMI_SERVICE_CFR_TA_RA_AS_FP_SUPPORT = 255, /* indicates FW support to program CFR TA/RA filtered packets as Filter pass */


    /******* ADD NEW SERVICES UP TO 256 HERE *******/

    WMI_MAX_EXT_SERVICE = 256,

    /* NOTE:
     * The above service flags are delivered in the
     *     fixed_param.wmi_service_segment_bitmap portion
     * of the WMI_SERVICE_AVAILABLE_EVENT message, with
     *     fixed_param.wmi_service_segment_offset
     * set to 128.
     * The below service flags can be delivered in one of two ways:
     * 1.  The target can deliver a 2nd SERVICE_AVAILABLE message, with
     *         fixed_param.wmi_service_segment_offset
     *     set to 256.
     *     (This method is acceptable, but not recommended.)
     * 2.  The target can populate the wmi_service_ext_bitmap[] TLV array
     *     within the WMI_SERVICE_AVAILABLE_EVENT message.
     *     (This method is recommended.)
     */


    /******* ADD NEW SERVICES 256 AND BEYOND HERE *******/

    WMI_SERVICE_CFR_CAPTURE_COUNT_SUPPORT = 256, /* indicates FW support to program CFR capture mode and capture count */
    WMI_SERVICE_OCV_SUPPORT = 257, /* FW supports OCV (Operating Channel Validation) */
    WMI_SERVICE_LL_STATS_PER_CHAN_RX_TX_TIME_SUPPORT = 258, /* Indicates firmware support sending per channel own tx & rx time in radio stats of LL stats. */
    WMI_SERVICE_THERMAL_MULTI_CLIENT_SUPPORT = 259, /* Indicates FW Thermal Mgr will support multiple clients for mitigation */
    WMI_SERVICE_NAN_SEND_NAN_ENABLE_RESPONSE_TO_HOST = 260, /* Indicates FW will include an additional TLV in nan enable response for Host driver to parse */
    WMI_SERVICE_UNIFIED_LL_GET_STA_CMD_SUPPORT = 261, /* Indicates that FW supports handling Link Layer and Get Station stats Commands together (WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID) */
    WMI_SERVICE_FSE_CMEM_ALLOC_SUPPORT = 262, /* Indicates that FW supports non-secure CMEM allocation for FSE table */
    WMI_SERVICE_PASSIVE_SCAN_START_TIME_ENHANCE = 263, /* Indicates FW support to not skip beacon if passive scan dwell time + channel switch delay is lesser than beacon interval */
    WMI_SERVICE_QOS_NULL_FRAME_TX_OVER_WMI = 264, /* Indicates that FW supports tx of QoS null frame downloaded through WMI interface */
    WMI_SERVICE_SCAN_CONFIG_PER_CHANNEL = 265, /* Indicates that FW supports per channel configuration support in the scan start command */
    WMI_SERVICE_CSA_BEACON_TEMPLATE = 266, /* Indicates that FW supports updating CSA count in beacon template */
    WMI_SERVICE_BROADCAST_TWT_REQUESTER = 267, /* Indicates FW supports Broadcast TWT REQUESTER */
    WMI_SERVICE_BROADCAST_TWT_RESPONDER = 268, /* Indicates FW supports Broadcast TWT RESPONDER */
    WMI_SERVICE_TWT_NUDGE = 269, /* Indicates that FW supports TWT Nudge command and event */
    WMI_SERVICE_TWT_STATS = 270, /* Indicates that FW supports TWT Get_stats command and event */
    WMI_SERVICE_TWT_ALL_DIALOG_ID = 271, /* Indicates that FW supports TWT ALL dialog ID(255) for all commands and events, except for TWT add dialog and TWT get stats */
    WMI_SERVICE_SPLIT_AST_SUPPORT = 272, /* Indicate that FW supports SPLIT AST table */
    WMI_SERVICE_SPECTRAL_SCAN_DISABLED = 273, /* Indicates that SKU does not support normal spectral scan capabilities */
    /* WMI_SERVICE_UNIFIED_LL_GET_STA_OVER_QMI_SUPPORT:
     * This service flag indicates that FW can support receiving a
     * WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID command request from the host
     * over the QMI communication link.
     * If the target receives WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID over QMI,
     * any of the event messages sent in response (WMI_UPDATE_STATS_EVENTID,
     * WMI_IFACE_LINK_STATS_EVENTID, WMI_PEER_LINK_STATS_EVENTID,
     * WMI_RADIO_LINK_STATS_EVENTID, WMI_RADIO_TX_POWER_LEVEL_STATS_EVENTID)
     * will be delivered over the QMI communication link.
     * Any such messages delivered over QMI will use the same message format
     * as if they had been delivered over the usual WMI communication link.
     * This service flag indicates QMI is supported for these stats messages
     * in addition to WMI, not instead of WMI - if the host sends the
     * WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID over the usual WMI communication
     * link, the stats event messages sent by the target in response will
     * be sent on the WMI communication link.
     */
    WMI_SERVICE_UNIFIED_LL_GET_STA_OVER_QMI_SUPPORT = 274,
    /*
     * Indicates FW supports EAPOL offload for SAE roaming if PMK of
     * candidate AP is present in FW.
     */
    WMI_SERVICE_SAE_EAPOL_OFFLOAD_SUPPORT = 275,
    /*
     * Indicates FW supports sending events to host during Thermal Throttling
     * every duty cycle on detecting temperature change when dc_per_event is 0.
     */
    WMI_SERVICE_THERM_THROT_TEMP_CHANGE_DYNAMIC_EVENT_SUPPORT = 276,
    WMI_SERVICE_WAPI_CONCURRENCY_SUPPORTED = 277, /* Indicates FW support for WAPI concurrency */
    WMI_SERVICE_SAP_CONNECTED_D3WOW = 278,  /* Indicates FW support for D3WoW for SAP connected case */
    WMI_SERVICE_GO_CONNECTED_D3WOW = 279,   /* Indicates FW support for D3WoW for P2P GO connected case */
    WMI_SERVICE_EXT_TPC_REG_SUPPORT = 280, /* Support for new 6G TPC power limits */
    WMI_SERVICE_REG_CC_EXT_EVENT_SUPPORT = 281, /* Support for Extended REG_CC Event with additional params for 6G */
    WMI_SERVICE_NDI_TXBF_SUPPORT = 282, /* Indicates FW support for Tx beamforming with NDI VDEV */
    WMI_SERVICE_ENABLE_LOWER_6G_EDGE_CH_SUPP = 283, /* Indicates FW support for enabling lower 6 GHz edge channel 5935 */
    WMI_SERVICE_DISABLE_UPPER_6G_EDGE_CH_SUPP = 284, /* Indicates FW support for disabling upper 6 GHz edge channel 7115 */
    WMI_SERVICE_FORCED_DTIM_SUPP = 285, /* Indicates FW supports forced DTIM configuration */
    WMI_SERVICE_DCS_AWGN_INT_SUPPORT = 286, /* Indicates FW supports AWGN Int */
    WMI_SERVICE_IGMP_OFFLOAD_SUPPORT = 287, /* FW supports igmp offload during APPS suspend */
    WMI_SERVICE_11AX_TDLS_SUPPORT = 288, /* Indicates FW supports 11ax TDLS. Host should enable 11ax on TDLS only when FW indicates the support. */
    WMI_SERVICE_11BE = 289, /* Indicates FW supports 11be */
    WMI_SERVICE_BIG_DATA_SUPPORT = 290, /* Indicates FW supports Big Data feature */
    WMI_SERVICE_EAPOL_OVER_NWIFI = 291, /* Indicates FW supports sending eapol frames in native wifi mode even when the vdev is brought up in raw ethernet mode */
    WMI_SERVICE_AMPDU_TX_BUF_SIZE_256_SUPPORT = 292, /* Indicates FW supports MAX 256 MPDUs in A-MPDU instead of 64 */
    WMI_SERVICE_HALPHY_CAL_STATUS = 293, /* Indicates FW supports sending online HALPHY Calibration status to host */
    WMI_SERVICE_HALPHY_CAL_ENABLE_DISABLE_SUPPORT = 294, /* Indicates Calibraton enable/disable support by FW */
    WMI_SERVICE_AFC_SUPPORT = 295, /* Indicates FW supports AFC_CMD,AFC_EVENT */


    WMI_MAX_EXT2_SERVICE

} WMI_SERVICE;

#define WMI_SERVICE_BM_SIZE   ((WMI_MAX_SERVICE + sizeof(A_UINT32)- 1)/sizeof(A_UINT32))

#define WMI_NUM_EXT_SERVICES (WMI_MAX_EXT_SERVICE - WMI_MAX_SERVICE)
#define WMI_NUM_EXT2_SERVICES (WMI_MAX_EXT2_SERVICE - WMI_MAX_EXT_SERVICE)

/*
 * TEMPORARY WORKAROUND
 * Virtually all branches of the target utilize an incorrect check on
 * the value of WMI_SERVICE_EXT_BM_SIZE32, checking that
 * WMI_SERVICE_EXT_BM_SIZE32 < 4, rather than WMI_SERVICE_EXT_BM_SIZE32 <= 4.
 * Until all target branches are fixed, this check is being disabled
 * by artificially setting WMI_SERVICE_EXT_BM_SIZE32 to 0, rather than
 * the number of 4-byte words required for holding the WMI extension
 * service flags.
 */
#if 0
#define WMI_SERVICE_EXT_BM_SIZE32 ((WMI_NUM_EXT_SERVICES + 31) / 32)
#else
#define WMI_SERVICE_EXT_BM_SIZE32 0
#endif

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
            (((pwmi_svc_ext_bmap)[((svc_id) - WMI_MAX_SERVICE) / 32] >> \
                ((svc_id) & 0x1f)) & 0x1))

#define WMI_SERVICE_EXT2_ENABLE( \
    pwmi_svc_bmap, pwmi_svc_ext_bmap, pwmi_svc_ext2_bmap, svc_id) \
    do { \
        if (svc_id < WMI_MAX_SERVICE) { \
            WMI_SERVICE_ENABLE(pwmi_svc_bmap, svc_id); \
        } else if (svc_id < WMI_MAX_EXT_SERVICE) { \
            WMI_SERVICE_EXT_ENABLE(pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id); \
        } else { \
            int word = ((svc_id) - WMI_MAX_EXT_SERVICE) / 32; \
            int bit = (svc_id) & 0x1f; /* svc_id mod 32 */ \
            (pwmi_svc_ext2_bmap)[word] |= (1 << bit); \
        } \
    } while (0)

#define WMI_SERVICE_EXT2_DISABLE( \
    pwmi_svc_bmap, pwmi_svc_ext_bmap, pwmi_svc_ext2_bmap, svc_id) \
    do { \
        if (svc_id < WMI_MAX_SERVICE) { \
            WMI_SERVICE_DISABLE(pwmi_svc_bmap, svc_id); \
        } else if (svc_id < WMI_MAX_EXT_SERVICE) { \
            WMI_SERVICE_EXT_DISABLE(pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id); \
        } else { \
            int word = ((svc_id) - WMI_MAX_EXT_SERVICE) / 32; \
            int bit = (svc_id) & 0x1f; /* svc_id mod 32 */ \
            (pwmi_svc_ext2_bmap)[word] &= ~(1 << bit); \
        } \
    } while (0)

#define WMI_SERVICE_EXT2_IS_ENABLED( \
    pwmi_svc_bmap, pwmi_svc_ext_bmap, pwmi_svc_ext2_bmap, svc_id) \
    /* If the service ID is beyond the known limit, treat it as disabled */ \
    ((svc_id) >= WMI_MAX_EXT2_SERVICE ? 0 : \
        /* If service ID is in the non-extension range, use the old check */ \
        (svc_id) < WMI_MAX_SERVICE ? \
            WMI_SERVICE_IS_ENABLED(pwmi_svc_bmap, svc_id) : \
            /* If service ID is in the 1st extended range, check ext_bmap */ \
            (svc_id) < WMI_MAX_EXT_SERVICE ? \
                WMI_SERVICE_EXT_IS_ENABLED( \
                    pwmi_svc_bmap, pwmi_svc_ext_bmap, svc_id) : \
                /* \
                 * If service ID is in the 2nd extended range, check ext2_bmap \
                 */ \
                (((pwmi_svc_ext2_bmap)[((svc_id) - WMI_MAX_EXT_SERVICE) / 32] >> \
                ((svc_id) & 0x1f)) & 0x1))

#ifdef __cplusplus
}
#endif

#endif /*_WMI_SERVICES_H_*/
