/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#ifndef WLAN_QCT_WMA_H
#define WLAN_QCT_WMA_H

#include "ani_global.h"

#include "wma_api.h"
#include "wma_tgt_cfg.h"
#include "i_cds_packet.h"

#define IS_MCC_SUPPORTED 1
#define IS_FEATURE_SUPPORTED_BY_FW(feat_enum_value) wma_get_fw_wlan_feat_caps(feat_enum_value)

#define IS_ROAM_SCAN_OFFLOAD_FEATURE_ENABLE 1

#define IS_IBSS_HEARTBEAT_OFFLOAD_FEATURE_ENABLE 1

#ifdef FEATURE_WLAN_TDLS
#define IS_ADVANCE_TDLS_ENABLE 0
#endif

#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define WMA_MAX_STA    (41)
#else
#define WMA_MAX_STA    (16)
#endif

#define WMA_NVDownload_Start(x)    ({ QDF_STATUS_SUCCESS; })

#define DPU_FEEDBACK_UNPROTECTED_ERROR 0x0F

#define WMA_GET_RX_MAC_HEADER(pRxMeta) \
	(tpSirMacMgmtHdr)(((t_packetmeta *)pRxMeta)->mpdu_hdr_ptr)

#define WMA_GET_RX_MPDUHEADER3A(pRxMeta) \
	(tpSirMacDataHdr3a)(((t_packetmeta *)pRxMeta)->mpdu_hdr_ptr)

#define WMA_GET_RX_MPDU_HEADER_LEN(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->mpdu_hdr_len)

#define WMA_GET_RX_MPDU_LEN(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->mpdu_len)

#define WMA_GET_RX_PAYLOAD_LEN(pRxMeta)	\
	(((t_packetmeta *)pRxMeta)->mpdu_data_len)

#define WMA_GET_RX_TSF_DELTA(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->tsf_delta)

#define WMA_GET_RX_MAC_RATE_IDX(pRxMeta) 0

#define WMA_GET_RX_MPDU_DATA(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->mpdu_data_ptr)

#define WMA_GET_RX_MPDU_HEADER_OFFSET(pRxMeta) 0

#define WMA_GET_RX_UNKNOWN_UCAST(pRxMeta) 0

#define WMA_GET_RX_CH(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->channel)

#define WMA_IS_RX_BCAST(pRxMeta) 0

#define WMA_GET_RX_FT_DONE(pRxMeta) 0

#define WMA_GET_RX_DPU_FEEDBACK(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->dpuFeedback)

#define WMA_GET_RX_BEACON_SENT(pRxMeta) 0

#define WMA_GET_RX_TSF_LATER(pRxMeta) 0

#define WMA_GET_RX_TIMESTAMP(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->timestamp)

#define WMA_IS_RX_IN_SCAN(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->scan)

#define WMA_GET_OFFLOADSCANLEARN(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->offloadScanLearn)
#define WMA_GET_ROAMCANDIDATEIND(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->roamCandidateInd)
#define WMA_GET_SESSIONID(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->sessionId)
#define WMA_GET_SCAN_SRC(pRxMeta) \
	(((t_packetmeta *)pRxMeta)->scan_src)

#ifdef FEATURE_WLAN_EXTSCAN
#define WMA_IS_EXTSCAN_SCAN_SRC(pRxMeta) \
	((((t_packetmeta *)pRxMeta)->scan_src) == WMI_MGMT_RX_HDR_EXTSCAN)
#define WMA_IS_EPNO_SCAN_SRC(pRxMeta) \
	((((t_packetmeta *)pRxMeta)->scan_src) & WMI_MGMT_RX_HDR_ENLO)
#endif /* FEATURE_WLAN_EXTSCAN */

#define WMA_GET_RX_SNR(pRxMeta)	\
	(((t_packetmeta *)pRxMeta)->snr)

#define WMA_GET_RX_RFBAND(pRxMeta) 0

#define WMA_MAX_TXPOWER_INVALID        127
/* rssi value normalized to noise floor of -96 dBm */
#define WMA_GET_RX_RSSI_NORMALIZED(pRxMeta) \
		       (((t_packetmeta *)pRxMeta)->rssi)

/* raw rssi based on actual noise floor in hardware */
#define WMA_GET_RX_RSSI_RAW(pRxMeta) \
		       (((t_packetmeta *)pRxMeta)->rssi_raw)

/* WMA Messages */
#define WMA_MSG_TYPES_BEGIN            SIR_HAL_MSG_TYPES_BEGIN
#define WMA_ITC_MSG_TYPES_BEGIN        SIR_HAL_ITC_MSG_TYPES_BEGIN
#define WMA_RADAR_DETECTED_IND         SIR_HAL_RADAR_DETECTED_IND

#define WMA_ADD_STA_REQ                SIR_HAL_ADD_STA_REQ
#define WMA_ADD_STA_RSP                SIR_HAL_ADD_STA_RSP
#define WMA_ADD_STA_SELF_RSP           SIR_HAL_ADD_STA_SELF_RSP
#define WMA_DELETE_STA_REQ             SIR_HAL_DELETE_STA_REQ
#define WMA_DELETE_STA_RSP             SIR_HAL_DELETE_STA_RSP
#define WMA_ADD_BSS_REQ                SIR_HAL_ADD_BSS_REQ
#define WMA_ADD_BSS_RSP                SIR_HAL_ADD_BSS_RSP
#define WMA_DELETE_BSS_REQ             SIR_HAL_DELETE_BSS_REQ
#define WMA_DELETE_BSS_RSP             SIR_HAL_DELETE_BSS_RSP
#define WMA_SEND_BEACON_REQ            SIR_HAL_SEND_BEACON_REQ
#define WMA_SEND_PROBE_RSP_TMPL        SIR_HAL_SEND_PROBE_RSP_TMPL

#define WMA_SET_BSSKEY_REQ             SIR_HAL_SET_BSSKEY_REQ
#define WMA_SET_BSSKEY_RSP             SIR_HAL_SET_BSSKEY_RSP
#define WMA_SET_STAKEY_REQ             SIR_HAL_SET_STAKEY_REQ
#define WMA_SET_STAKEY_RSP             SIR_HAL_SET_STAKEY_RSP
#define WMA_UPDATE_EDCA_PROFILE_IND    SIR_HAL_UPDATE_EDCA_PROFILE_IND

#define WMA_UPDATE_BEACON_IND          SIR_HAL_UPDATE_BEACON_IND
#define WMA_UPDATE_CF_IND              SIR_HAL_UPDATE_CF_IND
#define WMA_CHNL_SWITCH_REQ            SIR_HAL_CHNL_SWITCH_REQ
#define WMA_ADD_TS_REQ                 SIR_HAL_ADD_TS_REQ
#define WMA_DEL_TS_REQ                 SIR_HAL_DEL_TS_REQ

#define WMA_MISSED_BEACON_IND          SIR_HAL_MISSED_BEACON_IND

#define WMA_ENTER_PS_REQ               SIR_HAL_ENTER_PS_REQ
#define WMA_EXIT_PS_REQ                SIR_HAL_EXIT_PS_REQ

#define WMA_CFG_RXP_FILTER_REQ         SIR_HAL_CFG_RXP_FILTER_REQ

#define WMA_SWITCH_CHANNEL_RSP         SIR_HAL_SWITCH_CHANNEL_RSP
#define WMA_P2P_NOA_ATTR_IND           SIR_HAL_P2P_NOA_ATTR_IND
#define WMA_P2P_NOA_START_IND          SIR_HAL_P2P_NOA_START_IND
#define WMA_PWR_SAVE_CFG               SIR_HAL_PWR_SAVE_CFG
#define WMA_REGISTER_PE_CALLBACK       SIR_HAL_REGISTER_PE_CALLBACK

#define WMA_IBSS_STA_ADD               SIR_HAL_IBSS_STA_ADD
#define WMA_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND   SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND
#define WMA_SET_LINK_STATE             SIR_HAL_SET_LINK_STATE
#define WMA_SET_LINK_STATE_RSP         SIR_HAL_SET_LINK_STATE_RSP
#define WMA_SET_STA_BCASTKEY_REQ       SIR_HAL_SET_STA_BCASTKEY_REQ
#define WMA_SET_STA_BCASTKEY_RSP       SIR_HAL_SET_STA_BCASTKEY_RSP
#define WMA_ADD_TS_RSP                 SIR_HAL_ADD_TS_RSP
#define WMA_DPU_MIC_ERROR              SIR_HAL_DPU_MIC_ERROR
#define WMA_TIMER_CHIP_MONITOR_TIMEOUT SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT
#define WMA_TIMER_TRAFFIC_ACTIVITY_REQ SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ
#define WMA_TIMER_ADC_RSSI_STATS       SIR_HAL_TIMER_ADC_RSSI_STATS
#define WMA_TIMER_TRAFFIC_STATS_IND    SIR_HAL_TRAFFIC_STATS_IND

#ifdef WLAN_FEATURE_11W
#define WMA_EXCLUDE_UNENCRYPTED_IND    SIR_HAL_EXCLUDE_UNENCRYPTED_IND
#endif

#ifdef FEATURE_WLAN_ESE
#define WMA_TSM_STATS_REQ              SIR_HAL_TSM_STATS_REQ
#define WMA_TSM_STATS_RSP              SIR_HAL_TSM_STATS_RSP
#endif

#define WMA_HT40_OBSS_SCAN_IND                  SIR_HAL_HT40_OBSS_SCAN_IND

#define WMA_SET_MIMOPS_REQ                      SIR_HAL_SET_MIMOPS_REQ
#define WMA_SET_MIMOPS_RSP                      SIR_HAL_SET_MIMOPS_RSP
#define WMA_SYS_READY_IND                       SIR_HAL_SYS_READY_IND
#define WMA_SET_TX_POWER_REQ                    SIR_HAL_SET_TX_POWER_REQ
#define WMA_SET_TX_POWER_RSP                    SIR_HAL_SET_TX_POWER_RSP
#define WMA_GET_TX_POWER_REQ                    SIR_HAL_GET_TX_POWER_REQ

/* Messages to support transmit_halt and transmit_resume */
#define WMA_TRANSMISSION_CONTROL_IND            SIR_HAL_TRANSMISSION_CONTROL_IND
#define WMA_BEACON_FILTER_IND          SIR_HAL_BEACON_FILTER_IND

#define WMA_ENABLE_UAPSD_REQ            SIR_HAL_ENABLE_UAPSD_REQ
#define WMA_DISABLE_UAPSD_REQ           SIR_HAL_DISABLE_UAPSD_REQ

/* / PE <-> HAL WOWL messages */
#define WMA_WOW_ADD_PTRN               SIR_HAL_WOW_ADD_PTRN
#define WMA_WOW_DEL_PTRN               SIR_HAL_WOW_DEL_PTRN
#define WMA_WOWL_ENTER_REQ             SIR_HAL_WOWL_ENTER_REQ
#define WMA_WOWL_ENTER_RSP             SIR_HAL_WOWL_ENTER_RSP
#define WMA_WOWL_EXIT_REQ              SIR_HAL_WOWL_EXIT_REQ
#define WMA_WOWL_EXIT_RSP              SIR_HAL_WOWL_EXIT_RSP
/* / PE <-> HAL statistics messages */
#define WMA_GET_STATISTICS_REQ         SIR_HAL_GET_STATISTICS_REQ
#define WMA_GET_STATISTICS_RSP         SIR_HAL_GET_STATISTICS_RSP
#define WMA_SET_KEY_DONE               SIR_HAL_SET_KEY_DONE

/* / PE <-> HAL BTC messages */
#define WMA_BTC_SET_CFG                SIR_HAL_BTC_SET_CFG
#define WMA_HANDLE_FW_MBOX_RSP         SIR_HAL_HANDLE_FW_MBOX_RSP

#ifdef FEATURE_OEM_DATA_SUPPORT
/* PE <-> HAL OEM_DATA RELATED MESSAGES */
#define WMA_START_OEM_DATA_REQ         SIR_HAL_START_OEM_DATA_REQ
#define WMA_START_OEM_DATA_RSP         SIR_HAL_START_OEM_DATA_RSP
#endif

#define WMA_SET_MAX_TX_POWER_REQ       SIR_HAL_SET_MAX_TX_POWER_REQ
#define WMA_SET_MAX_TX_POWER_RSP       SIR_HAL_SET_MAX_TX_POWER_RSP
#define WMA_SET_TX_POWER_REQ           SIR_HAL_SET_TX_POWER_REQ

#define WMA_SET_MAX_TX_POWER_PER_BAND_REQ \
	SIR_HAL_SET_MAX_TX_POWER_PER_BAND_REQ

/* / PE <-> HAL Host Offload message */
#define WMA_SET_HOST_OFFLOAD           SIR_HAL_SET_HOST_OFFLOAD

/* / PE <-> HAL Keep Alive message */
#define WMA_SET_KEEP_ALIVE             SIR_HAL_SET_KEEP_ALIVE

#ifdef WLAN_NS_OFFLOAD
#define WMA_SET_NS_OFFLOAD             SIR_HAL_SET_NS_OFFLOAD
#endif /* WLAN_NS_OFFLOAD */
#define WMA_ADD_STA_SELF_REQ           SIR_HAL_ADD_STA_SELF_REQ
#define WMA_DEL_STA_SELF_REQ           SIR_HAL_DEL_STA_SELF_REQ

#define WMA_SET_P2P_GO_NOA_REQ         SIR_HAL_SET_P2P_GO_NOA_REQ

#ifdef FEATURE_WLAN_TDLS
#define WMA_SET_TDLS_LINK_ESTABLISH_REQ SIR_HAL_TDLS_LINK_ESTABLISH_REQ
#define WMA_SET_TDLS_LINK_ESTABLISH_REQ_RSP SIR_HAL_TDLS_LINK_ESTABLISH_REQ_RSP
#endif

#define WMA_WLAN_SUSPEND_IND           SIR_HAL_WLAN_SUSPEND_IND
#define WMA_WLAN_RESUME_REQ           SIR_HAL_WLAN_RESUME_REQ
#define WMA_MSG_TYPES_END    SIR_HAL_MSG_TYPES_END

#define WMA_RUNTIME_PM_SUSPEND_IND	SIR_HAL_RUNTIME_PM_SUSPEND_IND
#define WMA_RUNTIME_PM_RESUME_IND	SIR_HAL_RUNTIME_PM_RESUME_IND

#define WMA_AGGR_QOS_REQ               SIR_HAL_AGGR_QOS_REQ
#define WMA_AGGR_QOS_RSP               SIR_HAL_AGGR_QOS_RSP

/* FTM CMD MSG */
#define WMA_FTM_CMD_REQ        SIR_PTT_MSG_TYPES_BEGIN
#define WMA_FTM_CMD_RSP        SIR_PTT_MSG_TYPES_END
#define WMA_CSA_OFFLOAD_EVENT  SIR_CSA_OFFLOAD_EVENT

#ifdef FEATURE_WLAN_SCAN_PNO
/*Requests sent to lower driver*/
#define WMA_SET_PNO_REQ             SIR_HAL_SET_PNO_REQ

#endif /* FEATURE_WLAN_SCAN_PNO */

#ifdef FEATURE_WLAN_ESE
#define WMA_SET_PLM_REQ             SIR_HAL_SET_PLM_REQ
#endif

#define WMA_ROAM_SCAN_OFFLOAD_REQ   SIR_HAL_ROAM_SCAN_OFFLOAD_REQ

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define WMA_ROAM_OFFLOAD_SYNCH_IND  SIR_HAL_ROAM_OFFLOAD_SYNCH_IND
#define WMA_ROAM_OFFLOAD_SYNCH_FAIL SIR_HAL_ROAM_OFFLOAD_SYNCH_FAIL
#endif

#ifdef WLAN_FEATURE_PACKET_FILTERING
#define WMA_8023_MULTICAST_LIST_REQ                     SIR_HAL_8023_MULTICAST_LIST_REQ
#define WMA_RECEIVE_FILTER_SET_FILTER_REQ               SIR_HAL_RECEIVE_FILTER_SET_FILTER_REQ
#define WMA_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ    SIR_HAL_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ
#define WMA_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP    SIR_HAL_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP
#define WMA_RECEIVE_FILTER_CLEAR_FILTER_REQ             SIR_HAL_RECEIVE_FILTER_CLEAR_FILTER_REQ
#endif /* WLAN_FEATURE_PACKET_FILTERING */

#define WMA_DHCP_START_IND              SIR_HAL_DHCP_START_IND
#define WMA_DHCP_STOP_IND               SIR_HAL_DHCP_STOP_IND

#define WMA_TX_FAIL_MONITOR_IND         SIR_HAL_TX_FAIL_MONITOR_IND

#define WMA_HIDDEN_SSID_VDEV_RESTART    SIR_HAL_HIDE_SSID_VDEV_RESTART

#ifdef WLAN_FEATURE_GTK_OFFLOAD
#define WMA_GTK_OFFLOAD_REQ             SIR_HAL_GTK_OFFLOAD_REQ
#define WMA_GTK_OFFLOAD_GETINFO_REQ     SIR_HAL_GTK_OFFLOAD_GETINFO_REQ
#define WMA_GTK_OFFLOAD_GETINFO_RSP     SIR_HAL_GTK_OFFLOAD_GETINFO_RSP
#endif /* WLAN_FEATURE_GTK_OFFLOAD */

#define WMA_SET_TM_LEVEL_REQ       SIR_HAL_SET_TM_LEVEL_REQ

#define WMA_UPDATE_OP_MODE         SIR_HAL_UPDATE_OP_MODE
#define WMA_UPDATE_RX_NSS          SIR_HAL_UPDATE_RX_NSS
#define WMA_UPDATE_MEMBERSHIP      SIR_HAL_UPDATE_MEMBERSHIP
#define WMA_UPDATE_USERPOS         SIR_HAL_UPDATE_USERPOS

#ifdef WLAN_FEATURE_NAN
#define WMA_NAN_REQUEST            SIR_HAL_NAN_REQUEST
#endif

#define WMA_START_SCAN_OFFLOAD_REQ  SIR_HAL_START_SCAN_OFFLOAD_REQ
#define WMA_STOP_SCAN_OFFLOAD_REQ  SIR_HAL_STOP_SCAN_OFFLOAD_REQ
#define WMA_UPDATE_CHAN_LIST_REQ    SIR_HAL_UPDATE_CHAN_LIST_REQ
#define WMA_RX_SCAN_EVENT           SIR_HAL_RX_SCAN_EVENT
#define WMA_IBSS_PEER_INACTIVITY_IND SIR_HAL_IBSS_PEER_INACTIVITY_IND

#define WMA_CLI_SET_CMD             SIR_HAL_CLI_SET_CMD
#ifdef FEATURE_WLAN_SCAN_PNO
#define WMA_SME_SCAN_CACHE_UPDATED  SIR_HAL_SME_SCAN_CACHE_UPDATED
#endif

#ifndef REMOVE_PKT_LOG
#define WMA_PKTLOG_ENABLE_REQ       SIR_HAL_PKTLOG_ENABLE_REQ
#endif

#ifdef FEATURE_WLAN_LPHB
#define WMA_LPHB_CONF_REQ          SIR_HAL_LPHB_CONF_IND
#endif /* FEATURE_WLAN_LPHB */

#ifdef FEATURE_WLAN_CH_AVOID
#define WMA_CH_AVOID_UPDATE_REQ    SIR_HAL_CH_AVOID_UPDATE_REQ
#endif /* FEATURE_WLAN_CH_AVOID */

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
#define WMA_SET_AUTO_SHUTDOWN_TIMER_REQ SIR_HAL_SET_AUTO_SHUTDOWN_TIMER_REQ
#endif

#define WMA_ADD_PERIODIC_TX_PTRN_IND    SIR_HAL_ADD_PERIODIC_TX_PTRN_IND
#define WMA_DEL_PERIODIC_TX_PTRN_IND    SIR_HAL_DEL_PERIODIC_TX_PTRN_IND

#define WMA_TX_POWER_LIMIT          SIR_HAL_SET_TX_POWER_LIMIT

#define WMA_RATE_UPDATE_IND         SIR_HAL_RATE_UPDATE_IND

#define WMA_INIT_THERMAL_INFO_CMD   SIR_HAL_INIT_THERMAL_INFO_CMD
#define WMA_SET_THERMAL_LEVEL       SIR_HAL_SET_THERMAL_LEVEL
#define WMA_RMC_ENABLE_IND          SIR_HAL_RMC_ENABLE_IND
#define WMA_RMC_DISABLE_IND         SIR_HAL_RMC_DISABLE_IND
#define WMA_RMC_ACTION_PERIOD_IND   SIR_HAL_RMC_ACTION_PERIOD_IND

/* IBSS peer info related message */
#define WMA_GET_IBSS_PEER_INFO_REQ  SIR_HAL_IBSS_PEER_INFO_REQ

#define WMA_IBSS_CESIUM_ENABLE_IND  SIR_HAL_IBSS_CESIUM_ENABLE_IND

#ifdef FEATURE_WLAN_TDLS
#define WMA_UPDATE_FW_TDLS_STATE          SIR_HAL_UPDATE_FW_TDLS_STATE
#define WMA_UPDATE_TDLS_PEER_STATE        SIR_HAL_UPDATE_TDLS_PEER_STATE
#define WMA_TDLS_SHOULD_DISCOVER_CMD      SIR_HAL_TDLS_SHOULD_DISCOVER
#define WMA_TDLS_SHOULD_TEARDOWN_CMD      SIR_HAL_TDLS_SHOULD_TEARDOWN
#define WMA_TDLS_PEER_DISCONNECTED_CMD    SIR_HAL_TDLS_PEER_DISCONNECTED
#define WMA_TDLS_SET_OFFCHAN_MODE         SIR_HAL_TDLS_SET_OFFCHAN_MODE
#endif
#define WMA_SET_SAP_INTRABSS_DIS          SIR_HAL_SET_SAP_INTRABSS_DIS

/* Message to Indicate Radar Presence on SAP Channel */
#define WMA_DFS_RADAR_IND           SIR_HAL_DFS_RADAR_IND

/* Message to indicate beacon tx completion after beacon template update
 * beacon offload case
 */
#define WMA_DFS_BEACON_TX_SUCCESS_IND   SIR_HAL_BEACON_TX_SUCCESS_IND
#define WMA_DISASSOC_TX_COMP       SIR_HAL_DISASSOC_TX_COMP
#define WMA_DEAUTH_TX_COMP         SIR_HAL_DEAUTH_TX_COMP
#define WMA_GET_LINK_SPEED         SIR_HAL_GET_LINK_SPEED

#define WMA_MODEM_POWER_STATE_IND SIR_HAL_MODEM_POWER_STATE_IND

#ifdef WLAN_FEATURE_STATS_EXT
#define WMA_STATS_EXT_REQUEST              SIR_HAL_STATS_EXT_REQUEST
#endif

#define WMA_IPA_OFFLOAD_ENABLE_DISABLE        SIR_HAL_IPA_OFFLOAD_ENABLE_DISABLE

#define WMA_GET_TEMPERATURE_REQ     SIR_HAL_GET_TEMPERATURE_REQ
#define WMA_SET_WISA_PARAMS         SIR_HAL_SET_WISA_PARAMS

#ifdef FEATURE_WLAN_EXTSCAN
#define WMA_EXTSCAN_GET_CAPABILITIES_REQ    SIR_HAL_EXTSCAN_GET_CAPABILITIES_REQ
#define WMA_EXTSCAN_START_REQ               SIR_HAL_EXTSCAN_START_REQ
#define WMA_EXTSCAN_STOP_REQ                SIR_HAL_EXTSCAN_STOP_REQ
#define WMA_EXTSCAN_SET_BSSID_HOTLIST_REQ   SIR_HAL_EXTSCAN_SET_BSS_HOTLIST_REQ
#define WMA_EXTSCAN_RESET_BSSID_HOTLIST_REQ SIR_HAL_EXTSCAN_RESET_BSS_HOTLIST_REQ
#define WMA_EXTSCAN_SET_SIGNF_CHANGE_REQ    SIR_HAL_EXTSCAN_SET_SIGNF_CHANGE_REQ
#define WMA_EXTSCAN_RESET_SIGNF_CHANGE_REQ  SIR_HAL_EXTSCAN_RESET_SIGNF_CHANGE_REQ
#define WMA_EXTSCAN_GET_CACHED_RESULTS_REQ  SIR_HAL_EXTSCAN_GET_CACHED_RESULTS_REQ
#define WMA_SET_EPNO_LIST_REQ               SIR_HAL_SET_EPNO_LIST_REQ
#define WMA_SET_PASSPOINT_LIST_REQ          SIR_HAL_SET_PASSPOINT_LIST_REQ
#define WMA_RESET_PASSPOINT_LIST_REQ        SIR_HAL_RESET_PASSPOINT_LIST_REQ
#define WMA_EXTSCAN_SET_SSID_HOTLIST_REQ    SIR_HAL_EXTSCAN_SET_SSID_HOTLIST_REQ

#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
#define WMA_LINK_LAYER_STATS_CLEAR_REQ        SIR_HAL_LL_STATS_CLEAR_REQ
#define WMA_LINK_LAYER_STATS_SET_REQ          SIR_HAL_LL_STATS_SET_REQ
#define WMA_LINK_LAYER_STATS_GET_REQ          SIR_HAL_LL_STATS_GET_REQ
#define WMA_LINK_LAYER_STATS_RESULTS_RSP      SIR_HAL_LL_STATS_RESULTS_RSP
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

#define WMA_LINK_STATUS_GET_REQ SIR_HAL_LINK_STATUS_GET_REQ

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
#define WMA_WLAN_EXT_WOW                      SIR_HAL_CONFIG_EXT_WOW
#define WMA_WLAN_SET_APP_TYPE1_PARAMS         SIR_HAL_CONFIG_APP_TYPE1_PARAMS
#define WMA_WLAN_SET_APP_TYPE2_PARAMS         SIR_HAL_CONFIG_APP_TYPE2_PARAMS
#endif

#define WMA_SET_SCAN_MAC_OUI_REQ              SIR_HAL_SET_SCAN_MAC_OUI_REQ
#define WMA_TSF_GPIO_PIN                      SIR_HAL_TSF_GPIO_PIN_REQ

#ifdef DHCP_SERVER_OFFLOAD
#define WMA_SET_DHCP_SERVER_OFFLOAD_CMD       SIR_HAL_SET_DHCP_SERVER_OFFLOAD
#endif /* DHCP_SERVER_OFFLOAD */

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
#define WMA_LED_FLASHING_REQ   SIR_HAL_LED_FLASHING_REQ
#endif

/* Message posted by wmi when wmi event is received from FW */
#define WMA_PROCESS_FW_EVENT		     SIR_HAL_PROCESS_FW_EVENT

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
#define WMA_UPDATE_Q2Q_IE_IND                 SIR_HAL_UPDATE_Q2Q_IE_IND
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
#define WMA_SET_RSSI_MONITOR_REQ              SIR_HAL_SET_RSSI_MONITOR_REQ

#define WMA_FW_MEM_DUMP_REQ                   SIR_HAL_FW_MEM_DUMP_REQ

#define WMA_OCB_SET_CONFIG_CMD               SIR_HAL_OCB_SET_CONFIG_CMD
#define WMA_OCB_SET_UTC_TIME_CMD             SIR_HAL_OCB_SET_UTC_TIME_CMD
#define WMA_OCB_START_TIMING_ADVERT_CMD      SIR_HAL_OCB_START_TIMING_ADVERT_CMD
#define WMA_OCB_STOP_TIMING_ADVERT_CMD       SIR_HAL_OCB_STOP_TIMING_ADVERT_CMD
#define WMA_OCB_GET_TSF_TIMER_CMD            SIR_HAL_OCB_GET_TSF_TIMER_CMD
#define WMA_DCC_GET_STATS_CMD                SIR_HAL_DCC_GET_STATS_CMD
#define WMA_DCC_CLEAR_STATS_CMD              SIR_HAL_DCC_CLEAR_STATS_CMD
#define WMA_DCC_UPDATE_NDL_CMD               SIR_HAL_DCC_UPDATE_NDL_CMD
#define WMA_SET_IE_INFO                      SIR_HAL_SET_IE_INFO

#define WMA_LRO_CONFIG_CMD                   SIR_HAL_LRO_CONFIG_CMD
#define WMA_GW_PARAM_UPDATE_REQ              SIR_HAL_GATEWAY_PARAM_UPDATE_REQ
#define WMA_SET_EGAP_CONF_PARAMS             SIR_HAL_SET_EGAP_CONF_PARAMS
#define WMA_ADD_BCN_FILTER_CMDID             SIR_HAL_ADD_BCN_FILTER_CMDID
#define WMA_REMOVE_BCN_FILTER_CMDID          SIR_HAL_REMOVE_BCN_FILTER_CMDID

#define WDA_BPF_GET_CAPABILITIES_REQ          SIR_HAL_BPF_GET_CAPABILITIES_REQ
#define WDA_BPF_SET_INSTRUCTIONS_REQ          SIR_HAL_BPF_SET_INSTRUCTIONS_REQ

/* Bit 6 will be used to control BD rate for Management frames */
#define HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME 0x40

#define wma_tx_frame(hHal, pFrmBuf, frmLen, frmType, txDir, tid, pCompFunc, \
		   pData, txFlag, sessionid, channel_freq) \
	(QDF_STATUS)( wma_tx_packet( \
		      cds_get_context(QDF_MODULE_ID_WMA), \
		      (pFrmBuf), \
		      (frmLen), \
		      (frmType), \
		      (txDir), \
		      (tid), \
		      (pCompFunc), \
		      (pData), \
		      (NULL), \
		      (txFlag), \
		      (sessionid), \
		      (false), \
		      (channel_freq)))

#define wma_tx_frameWithTxComplete(hHal, pFrmBuf, frmLen, frmType, txDir, tid, \
	 pCompFunc, pData, pCBackFnTxComp, txFlag, sessionid, tdlsflag, \
	 channel_freq) \
	(QDF_STATUS)( wma_tx_packet( \
		      cds_get_context(QDF_MODULE_ID_WMA), \
		      (pFrmBuf), \
		      (frmLen), \
		      (frmType), \
		      (txDir), \
		      (tid), \
		      (pCompFunc), \
		      (pData), \
		      (pCBackFnTxComp), \
		      (txFlag), \
		      (sessionid), \
		      (tdlsflag), \
		      (channel_freq)))


#define WMA_SetEnableSSR(enable_ssr) ((void)enable_ssr)

/**
 * struct sUapsd_Params - Powersave Offload Changes
 * @bkDeliveryEnabled: BK delivery enabled flag
 * @beDeliveryEnabled: BE delivery enabled flag
 * @viDeliveryEnabled: VI delivery enabled flag
 * @voDeliveryEnabled: VO delivery enabled flag
 * @bkTriggerEnabled: BK trigger enabled flag
 * @beTriggerEnabled: BE trigger enabled flag
 * @viTriggerEnabled: VI trigger enabled flag
 * @voTriggerEnabled: VO trigger enabled flag
 */
typedef struct sUapsd_Params {
	uint8_t bkDeliveryEnabled:1;
	uint8_t beDeliveryEnabled:1;
	uint8_t viDeliveryEnabled:1;
	uint8_t voDeliveryEnabled:1;
	uint8_t bkTriggerEnabled:1;
	uint8_t beTriggerEnabled:1;
	uint8_t viTriggerEnabled:1;
	uint8_t voTriggerEnabled:1;
	bool enable_ps;
} tUapsd_Params, *tpUapsd_Params;

/**
 * struct sEnablePsParams - Enable PowerSave Params
 * @psSetting: power save setting
 * @uapsdParams: UAPSD Parameters
 * @bssid: mac address
 * @sessionid: sme session id / vdev id
 * @bcnDtimPeriod: beacon DTIM Period
 * @status: success/failure
 */
typedef struct sEnablePsParams {
	tSirAddonPsReq psSetting;
	tUapsd_Params uapsdParams;
	tSirMacAddr bssid;
	uint32_t sessionid;
	uint8_t bcnDtimPeriod;
	uint32_t status;
} tEnablePsParams, *tpEnablePsParams;

/**
 * struct sDisablePsParams - Disable PowerSave Params
 * @psSetting: power save setting
 * @bssid: mac address
 * @sessionid: sme session id / vdev id
 * @status: success/failure
 */
typedef struct sDisablePsParams {
	tSirAddonPsReq psSetting;
	tSirMacAddr bssid;
	uint32_t sessionid;
	uint32_t status;
} tDisablePsParams, *tpDisablePsParams;

/**
 * struct sEnableUapsdParams - Enable Uapsd Params
 * @uapsdParams: UAPSD parameters
 * @bssid: mac address
 * @sessionid: sme session id/ vdev id
 * @status: success/failure
 */
typedef struct sEnableUapsdParams {
	tUapsd_Params uapsdParams;
	tSirMacAddr bssid;
	uint32_t sessionid;
	uint32_t status;
} tEnableUapsdParams, *tpEnableUapsdParams;

/**
 * struct sDisableUapsdParams - Disable Uapsd Params
 * @bssid: mac address
 * @sessionid: sme session id/ vdev id
 * @status: success/failure
 */
typedef struct sDisableUapsdParams {
	tSirMacAddr bssid;
	uint32_t sessionid;
	uint32_t status;
} tDisableUapsdParams, *tpDisableUapsdParams;

typedef void (*pWMATxRxCompFunc)(void *pContext, void *pData,
				 bool bFreeData);

/* callback function for TX complete */
/* parameter 1 - global pMac pointer */
/* parameter 2 - txComplete status : 1- success, 0 - failure. */
typedef QDF_STATUS (*pWMAAckFnTxComp)(tpAniSirGlobal, uint32_t);

typedef void (*wma_txFailIndCallback)(uint8_t *, uint8_t);

/* generic callback for updating parameters from target to UMAC */
typedef void (*wma_tgt_cfg_cb)(void *context, void *param);

/*
 * callback for Indicating Radar to HDD and disable Tx Queues
 * to stop accepting data Tx packets from netif as radar is
 * found on the current operating channel
 */
typedef bool (*wma_dfs_radar_indication_cb)(void *context, void *param);

/**
 * struct wma_cli_set_cmd_t - set command parameters
 * @param_id: parameter id
 * @param_value: parameter value
 * @param_sec_value: parameter sec value
 * @param_vdev_id: parameter vdev id
 * @param_vp_dev: is it per vdev/pdev
 */
typedef struct {
	uint32_t param_id;
	uint32_t param_value;
	uint32_t param_sec_value;
	uint32_t param_vdev_id;
	uint32_t param_vp_dev;
} wma_cli_set_cmd_t;

#if defined(QCA_WIFI_FTM)
#define AR6K_TM_DATA_MAX_LEN    5000
struct ar6k_testmode_cmd_data {
	void *data;
	int len;
};
#endif

#ifdef FEATURE_WLAN_TDLS
/**
 * enum WMA_TdlsPeerState - TDLS PEER state
 * @WMA_TDLS_PEER_STATE_PEERING: peer is making connection
 * @WMA_TDLS_PEER_STATE_CONNECTED: peer is connected
 * @WMA_TDLS_PEER_STATE_TEARDOWN: peer is teardown
 * @WMA_TDLS_PEER_ADD_MAC_ADDR: add peer into connection table
 * @WMA_TDLS_PEER_REMOVE_MAC_ADDR: remove peer from connection table
 */
typedef enum {
	WMA_TDLS_PEER_STATE_PEERING,
	WMA_TDLS_PEER_STATE_CONNECTED,
	WMA_TDLS_PEER_STATE_TEARDOWN,
	WMA_TDLS_PEER_ADD_MAC_ADDR,
	WMA_TDLS_PEER_REMOVE_MAC_ADDR,
} WMA_TdlsPeerState;

/**
 * enum wma_tdls_off_chan_mode - modes for WMI_TDLS_SET_OFFCHAN_MODE_CMDID
 * @WMA_TDLS_ENABLE_OFFCHANNEL: enable off channel
 * @WMA_TDLS_DISABLE_OFFCHANNEL: disable off channel
 */
typedef enum {
	WMA_TDLS_ENABLE_OFFCHANNEL,
	WMA_TDLS_DISABLE_OFFCHANNEL
} wma_tdls_off_chan_mode;

#endif /* FEATURE_WLAN_TDLS */

tSirRetStatus wma_post_ctrl_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);

tSirRetStatus u_mac_post_ctrl_msg(void *pSirGlobal, tSirMbMsg *pMb);

QDF_STATUS wma_set_idle_ps_config(void *wma_ptr, uint32_t idle_ps);
QDF_STATUS wma_get_snr(tAniGetSnrReq *psnr_req);

QDF_STATUS
wma_ds_peek_rx_packet_info
	(cds_pkt_t *vosDataBuff, void **ppRxHeader, bool bSwap);


void wma_tx_abort(uint8_t vdev_id);

QDF_STATUS wma_tx_packet(void *pWMA,
			 void *pFrmBuf,
			 uint16_t frmLen,
			 eFrameType frmType,
			 eFrameTxDir txDir,
			 uint8_t tid,
			 pWMATxRxCompFunc pCompFunc,
			 void *pData,
			 pWMAAckFnTxComp pAckTxComp,
			 uint8_t txFlag, uint8_t sessionId, bool tdlsflag,
			 uint16_t channel_freq);

QDF_STATUS wma_open(void *p_cds_context,
		    wma_tgt_cfg_cb pTgtUpdCB,
		    wma_dfs_radar_indication_cb radar_ind_cb,
		    tMacOpenParameters *pMacParams);

typedef QDF_STATUS (*wma_mgmt_frame_rx_callback)(void *p_cds_gctx,
					     void *cds_buff);

QDF_STATUS wma_register_mgmt_frm_client(void *p_cds_gctx,
				wma_mgmt_frame_rx_callback mgmt_rx_cb);

QDF_STATUS wma_de_register_mgmt_frm_client(void *p_cds_gctx);
QDF_STATUS wma_register_roaming_callbacks(void *cds_ctx,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		void (*csr_roam_synch_cb)(tpAniSirGlobal mac,
			roam_offload_synch_ind *roam_synch_data,
			tpSirBssDescription  bss_desc_ptr,
			enum sir_roam_op_code reason),
		QDF_STATUS (*pe_roam_synch_cb)(tpAniSirGlobal mac,
			roam_offload_synch_ind *roam_synch_data,
			tpSirBssDescription  bss_desc_ptr));
#else
static inline QDF_STATUS wma_register_roaming_callbacks(void *cds_ctx,
		void (*csr_roam_synch_cb)(tpAniSirGlobal mac,
			roam_offload_synch_ind *roam_synch_data,
			tpSirBssDescription  bss_desc_ptr,
			enum sir_roam_op_code reason),
		QDF_STATUS (*pe_roam_synch_cb)(tpAniSirGlobal mac,
			roam_offload_synch_ind *roam_synch_data,
			tpSirBssDescription  bss_desc_ptr))
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#endif
