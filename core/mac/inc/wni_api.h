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

/*
 * This file wni_api.h contains message definitions exported by
 * Sirius software modules.
 * NOTE: See projects/sirius/include/sir_api.h for structure
 * definitions of the host/FW messages.
 *
 * Author:        Chandra Modumudi
 * Date:          04/11/2002
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#ifndef __WNI_API_H
#define __WNI_API_H

#define SIR_SME_MODULE_ID 0x16

/* / Start of Sirius/Host message types */
#define WNI_HOST_MSG_START             0x1500

enum eWniMsgTypes {
	/* / CFG message types */
	eWNI_CFG_MSG_TYPES_BEGIN = WNI_HOST_MSG_START,
	eWNI_CFG_MSG_TYPES_END = eWNI_CFG_MSG_TYPES_BEGIN + 0xFF,

	/* / SME message types */
	eWNI_SME_MSG_TYPES_BEGIN = eWNI_CFG_MSG_TYPES_END,
	eWNI_SME_SYS_READY_IND,
	eWNI_SME_SCAN_REQ,
	eWNI_SME_SCAN_ABORT_IND,
	eWNI_SME_SCAN_RSP,
	eWNI_SME_JOIN_REQ,
	eWNI_SME_JOIN_RSP,
	eWNI_SME_SETCONTEXT_REQ,
	eWNI_SME_SETCONTEXT_RSP,
	eWNI_SME_REASSOC_REQ,
	eWNI_SME_REASSOC_RSP,
	eWNI_SME_DISASSOC_REQ,
	eWNI_SME_DISASSOC_RSP,
	eWNI_SME_DISASSOC_IND,
	eWNI_SME_DISASSOC_CNF,
	eWNI_SME_DEAUTH_REQ,
	eWNI_SME_DEAUTH_RSP,
	eWNI_SME_DEAUTH_IND,
	eWNI_SME_WM_STATUS_CHANGE_NTF,
	eWNI_SME_IBSS_NEW_PEER_IND,
	eWNI_SME_IBSS_PEER_DEPARTED_IND,
	eWNI_SME_START_BSS_REQ,
	eWNI_SME_START_BSS_RSP,
	eWNI_SME_ASSOC_IND,
	eWNI_SME_ASSOC_CNF,
	eWNI_SME_SWITCH_CHL_IND,
	eWNI_SME_STOP_BSS_REQ,
	eWNI_SME_STOP_BSS_RSP,
	eWNI_SME_NEIGHBOR_BSS_IND,
	eWNI_SME_DEAUTH_CNF,
	eWNI_SME_MIC_FAILURE_IND,
	eWNI_SME_ADDTS_REQ,
	eWNI_SME_ADDTS_RSP,
	eWNI_SME_DELTS_REQ,
	eWNI_SME_DELTS_RSP,
	eWNI_SME_DELTS_IND,
	eWNI_SME_GET_STATISTICS_REQ,
	eWNI_SME_GET_STATISTICS_RSP,
	eWNI_SME_GET_RSSI_REQ,
	eWNI_SME_GET_ASSOC_STAS_REQ,
	eWNI_SME_TKIP_CNTR_MEAS_REQ,
	eWNI_SME_UPDATE_APWPSIE_REQ,
	eWNI_SME_GET_WPSPBC_SESSION_REQ,
	eWNI_SME_WPS_PBC_PROBE_REQ_IND,
	eWNI_SME_SET_APWPARSNIEs_REQ,
	eWNI_SME_UPPER_LAYER_ASSOC_CNF,
	eWNI_SME_HIDE_SSID_REQ,
	eWNI_SME_CHNG_MCC_BEACON_INTERVAL,
	eWNI_SME_REMAIN_ON_CHANNEL_REQ,
	eWNI_SME_REMAIN_ON_CHN_RSP,
	eWNI_SME_REMAIN_ON_CHN_RDY_IND,
	eWNI_SME_SEND_ACTION_FRAME_IND,
	eWNI_SME_ABORT_REMAIN_ON_CHAN_IND,
	eWNI_SME_UPDATE_NOA,
	eWNI_SME_CLEAR_DFS_CHANNEL_LIST,
	eWNI_SME_GET_SNR_REQ,
	/* General Power Save Messages */
	eWNI_PMC_MSG_TYPES_BEGIN,

	/* WOWL Messages */
	eWNI_PMC_SMPS_STATE_IND,

	eWNI_SME_RRM_MSG_TYPE_BEGIN,

	eWNI_SME_NEIGHBOR_REPORT_REQ_IND,
	eWNI_SME_NEIGHBOR_REPORT_IND,
	eWNI_SME_BEACON_REPORT_REQ_IND,
	eWNI_SME_BEACON_REPORT_RESP_XMIT_IND,

	eWNI_SME_ADD_STA_SELF_RSP,
	eWNI_SME_DEL_STA_SELF_RSP,

	eWNI_SME_FT_PRE_AUTH_REQ,
	eWNI_SME_FT_PRE_AUTH_RSP,
	eWNI_SME_FT_UPDATE_KEY,
	eWNI_SME_FT_AGGR_QOS_REQ,
	eWNI_SME_FT_AGGR_QOS_RSP,

#if defined FEATURE_WLAN_ESE
	eWNI_SME_ESE_ADJACENT_AP_REPORT,
#endif

	eWNI_SME_REGISTER_MGMT_FRAME_REQ,
#ifdef FEATURE_WLAN_SCAN_PNO
	eWNI_SME_PREF_NETWORK_FOUND_IND,
#endif /* FEATURE_WLAN_SCAN_PNO */

	eWNI_SME_CHANGE_COUNTRY_CODE,
	eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE,
	eWNI_SME_MAX_ASSOC_EXCEEDED,
#ifdef FEATURE_WLAN_TDLS
	eWNI_SME_TDLS_SEND_MGMT_REQ,
	eWNI_SME_TDLS_SEND_MGMT_RSP,
	eWNI_SME_TDLS_ADD_STA_REQ,
	eWNI_SME_TDLS_ADD_STA_RSP,
	eWNI_SME_TDLS_DEL_STA_REQ,
	eWNI_SME_TDLS_DEL_STA_RSP,
	eWNI_SME_TDLS_DEL_STA_IND,
	eWNI_SME_TDLS_DEL_ALL_PEER_IND,
	eWNI_SME_MGMT_FRM_TX_COMPLETION_IND,
	eWNI_SME_TDLS_LINK_ESTABLISH_REQ,
	eWNI_SME_TDLS_LINK_ESTABLISH_RSP,
	eWNI_SME_TDLS_SHOULD_DISCOVER,
	eWNI_SME_TDLS_SHOULD_TEARDOWN,
	eWNI_SME_TDLS_PEER_DISCONNECTED,
	eWNI_SME_TDLS_CONNECTION_TRACKER_NOTIFICATION,
#endif
	/* NOTE: If you are planning to add more mesages, please make sure that */
	/* SIR_LIM_ITC_MSG_TYPES_BEGIN is moved appropriately. It is set as */
	/* SIR_LIM_MSG_TYPES_BEGIN+0xB0 = 12B0 (which means max of 176 messages and */
	/* eWNI_SME_TDLS_DEL_STA_RSP = 175. */
	/* Should fix above issue to enable TDLS_INTERNAL */
	eWNI_SME_SET_BCN_FILTER_REQ,
	eWNI_SME_RESET_AP_CAPS_CHANGED,
#ifdef WLAN_FEATURE_11W
	eWNI_SME_UNPROT_MGMT_FRM_IND,
#endif
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	eWNI_PMC_GTK_OFFLOAD_GETINFO_RSP,
#endif /* WLAN_FEATURE_GTK_OFFLOAD */
	eWNI_SME_CANDIDATE_FOUND_IND,   /*ROAM candidate indication from FW */
	eWNI_SME_HANDOFF_REQ,   /*upper layer requested handoff to driver in STA mode */
	eWNI_SME_ROAM_SCAN_OFFLOAD_RSP, /*Fwd the LFR scan offload rsp from FW to SME */
#ifdef FEATURE_WLAN_LPHB
	eWNI_SME_LPHB_IND,
#endif /* FEATURE_WLAN_LPHB */

	eWNI_SME_IBSS_PEER_INFO_RSP,
	eWNI_SME_GET_TSM_STATS_REQ,
	eWNI_SME_GET_TSM_STATS_RSP,
	eWNI_SME_TSM_IE_IND,

	eWNI_SME_READY_TO_SUSPEND_IND,
#ifdef FEATURE_WLAN_CH_AVOID
	eWNI_SME_CH_AVOID_IND,
#endif /* FEATURE_WLAN_CH_AVOID */
	/* DFS EVENTS */
	eWNI_SME_DFS_RADAR_FOUND,       /* RADAR found indication from DFS */
	eWNI_SME_CHANNEL_CHANGE_REQ,    /* Channel Change Request from SAP */
	eWNI_SME_CHANNEL_CHANGE_RSP,    /* Channel Change Response from WMA */
	eWNI_SME_START_BEACON_REQ,      /* Start Beacon Transmission. */
	eWNI_SME_DFS_BEACON_CHAN_SW_IE_REQ,     /* Transmit CSA IE in beacons */
	eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND,     /* To indicate completion of CSA IE */
	/* update in beacons/probe rsp */
	eWNI_SME_STATS_EXT_EVENT,
	eWNI_SME_LINK_SPEED_IND,        /* Indicate linkspeed response from WMA */
	eWNI_SME_CSA_OFFLOAD_EVENT,
	eWNI_SME_UPDATE_ADDITIONAL_IES, /* indicates Additional IE from hdd to PE */
	eWNI_SME_MODIFY_ADDITIONAL_IES, /* To indicate IE modify from hdd to PE */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	eWNI_SME_AUTO_SHUTDOWN_IND,
#endif
#ifdef QCA_HT_2040_COEX
	eWNI_SME_SET_HT_2040_MODE,
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	eWNI_SME_HO_FAIL_IND,   /* Hand Off Failure Ind from WMA to SME */
#endif
#ifdef WLAN_FEATURE_NAN
	eWNI_SME_NAN_EVENT,
#endif
	eWNI_SME_LINK_STATUS_IND,
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	eWNI_SME_READY_TO_EXTWOW_IND,
#endif
	eWNI_SME_MSG_GET_TEMPERATURE_IND,
	eWNI_SME_SNR_IND,
#ifdef FEATURE_WLAN_EXTSCAN
	eWNI_SME_EXTSCAN_FULL_SCAN_RESULT_IND,
	eWNI_SME_EPNO_NETWORK_FOUND_IND,
#endif
	eWNI_SME_FW_DUMP_IND,
	eWNI_SME_SET_HW_MODE_REQ,
	eWNI_SME_SET_HW_MODE_RESP,
	eWNI_SME_HW_MODE_TRANS_IND,
	eWNI_SME_NSS_UPDATE_REQ,
	eWNI_SME_NSS_UPDATE_RSP,
	eWNI_SME_SCAN_CMD,
	eWNI_SME_OCB_SET_CONFIG_RSP,
	eWNI_SME_OCB_GET_TSF_TIMER_RSP,
	eWNI_SME_DCC_GET_STATS_RSP,
	eWNI_SME_DCC_UPDATE_NDL_RSP,
	eWNI_SME_DCC_STATS_EVENT,
	eWNI_SME_SET_DUAL_MAC_CFG_REQ,
	eWNI_SME_SET_DUAL_MAC_CFG_RESP,
	eWNI_SME_ROC_CMD,
	eWNI_SME_SET_THERMAL_LEVEL_IND,
	eWNI_SME_SET_IE_REQ,
	eWNI_SME_EXT_CHANGE_CHANNEL,
	eWNI_SME_EXT_CHANGE_CHANNEL_IND,
	eWNI_SME_REGISTER_MGMT_FRAME_CB,
	eWNI_SME_HT40_OBSS_SCAN_IND, /* START and UPDATE OBSS SCAN Indication*/
	eWNI_SME_SET_ANTENNA_MODE_REQ,
	eWNI_SME_SET_ANTENNA_MODE_RESP,
	eWNI_SME_TSF_EVENT,
	eWNI_SME_MON_INIT_SESSION,
	eWNI_SME_PDEV_SET_HT_VHT_IE,
	eWNI_SME_SET_VDEV_IES_PER_BAND,
	eWNI_SME_NDP_INITIATOR_REQ,
	eWNI_SME_NDP_INITIATOR_RSP,
	eWNI_SME_NDP_NEW_PEER_IND,
	eWNI_SME_NDP_CONFIRM_IND,
	eWNI_SME_NDP_INDICATION,
	eWNI_SME_NDP_RESPONDER_REQ,
	eWNI_SME_NDP_RESPONDER_RSP,
	eWNI_SME_NDP_END_REQ,
	eWNI_SME_NDP_END_RSP,
	eWNI_SME_NDP_PEER_DEPARTED_IND,
	eWNI_SME_NDP_END_IND,
	eWNI_SME_REGISTER_P2P_ACK_CB,
	eWNI_SME_SEND_DISASSOC_FRAME,
	eWNI_SME_UPDATE_ACCESS_POLICY_VENDOR_IE,
	eWNI_SME_MSG_TYPES_END
};

typedef enum {
	eWNI_TDLS_TEARDOWN_REASON_TX,
	eWNI_TDLS_TEARDOWN_REASON_RSSI,
	eWNI_TDLS_TEARDOWN_REASON_SCAN,
	eWNI_TDLS_DISCONNECTED_REASON_PEER_DELETE,
	eWNI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT,
	eWNI_TDLS_TEARDOWN_REASON_BAD_PTR,
	eWNI_TDLS_TEARDOWN_REASON_NO_RESPONSE,
} eWniTdlsTeardownReason;

/**
 * enum ewni_tdls_connection_tracker_notification - connection tracker events
 * @eWNI_TDLS_PEER_ENTER_BUF_STA: TDLS peer enters buff sta
 * @eWNI_TDLS_PEER_EXIT_BUF_STA: TDLS peer exit buff sta
 * @eWNI_TDLS_ENTER_BT_BUSY_MODE: Enter BT busy event
 * @eWNI_TDLS_EXIT_BT_BUSY_MODE: Exit BT busy event
 */
enum ewni_tdls_connection_tracker_notification {
	eWNI_TDLS_PEER_ENTER_BUF_STA,
	eWNI_TDLS_PEER_EXIT_BUF_STA,
	eWNI_TDLS_ENTER_BT_BUSY_MODE,
	eWNI_TDLS_EXIT_BT_BUSY_MODE,
};

#define WNI_CFG_MSG_TYPES_BEGIN        0x1200

/*---------------------------------------------------------------------*/
/* CFG Module Definitions                                              */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/* CFG message definitions                                             */
/*---------------------------------------------------------------------*/
#define WNI_CFG_MSG_HDR_MASK    0xffff0000
#define WNI_CFG_MSG_LEN_MASK    0x0000ffff
#define WNI_CFG_MB_HDR_LEN      4
#define WNI_CFG_MAX_PARAM_NUM   32

/*---------------------------------------------------------------------*/
/* CFG to HDD message types                                            */
/*---------------------------------------------------------------------*/
#define WNI_CFG_PARAM_UPDATE_IND       (WNI_CFG_MSG_TYPES_BEGIN | 0x00)
#define WNI_CFG_DNLD_REQ               (WNI_CFG_MSG_TYPES_BEGIN | 0x01)
#define WNI_CFG_DNLD_CNF               (WNI_CFG_MSG_TYPES_BEGIN | 0x02)
#define WNI_CFG_GET_RSP                (WNI_CFG_MSG_TYPES_BEGIN | 0x03)
#define WNI_CFG_SET_CNF                (WNI_CFG_MSG_TYPES_BEGIN | 0x04)
#define WNI_CFG_GET_ATTRIB_RSP         (WNI_CFG_MSG_TYPES_BEGIN | 0x05)
#define WNI_CFG_ADD_GRP_ADDR_CNF       (WNI_CFG_MSG_TYPES_BEGIN | 0x06)
#define WNI_CFG_DEL_GRP_ADDR_CNF       (WNI_CFG_MSG_TYPES_BEGIN | 0x07)

#define ANI_CFG_GET_RADIO_STAT_RSP     (WNI_CFG_MSG_TYPES_BEGIN | 0x08)
#define ANI_CFG_GET_PER_STA_STAT_RSP   (WNI_CFG_MSG_TYPES_BEGIN | 0x09)
#define ANI_CFG_GET_AGG_STA_STAT_RSP   (WNI_CFG_MSG_TYPES_BEGIN | 0x0a)
#define ANI_CFG_CLEAR_STAT_RSP         (WNI_CFG_MSG_TYPES_BEGIN | 0x0b)

/*---------------------------------------------------------------------*/
/* CFG to HDD message paramter indices                                 */

/*   The followings are word indices starting from the message body    */

/*   WNI_CFG_xxxx_xxxx_xxxx:         index of parameter                */
/*   WNI_CFG_xxxx_xxxx_NUM:          number of parameters in message   */

/*   WNI_CFG_xxxx_xxxx_LEN:          byte length of message including  */
/*                                   MB header                         */
/*                                                                     */
/*   WNI_CFG_xxxx_xxxx_PARTIAL_LEN:  byte length of message including  */
/*                                   parameters and MB header but      */
/*                                   excluding variable data length    */
/*---------------------------------------------------------------------*/

/* Parameter update indication */
#define WNI_CFG_PARAM_UPDATE_IND_PID   0

#define WNI_CFG_PARAM_UPDATE_IND_NUM   1
#define WNI_CFG_PARAM_UPDATE_IND_LEN   (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_PARAM_UPDATE_IND_NUM << 2))

/* Configuration download request */
#define WNI_CFG_DNLD_REQ_NUM           0
#define WNI_CFG_DNLD_REQ_LEN           WNI_CFG_MB_HDR_LEN

/* Configuration download confirm */
#define WNI_CFG_DNLD_CNF_RES           0

#define WNI_CFG_DNLD_CNF_NUM           1
#define WNI_CFG_DNLD_CNF_LEN           (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_DNLD_CNF_NUM << 2))
/* Get response */
#define WNI_CFG_GET_RSP_RES            0
#define WNI_CFG_GET_RSP_PID            1
#define WNI_CFG_GET_RSP_PLEN           2

#define WNI_CFG_GET_RSP_NUM            3
#define WNI_CFG_GET_RSP_PARTIAL_LEN    (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_GET_RSP_NUM << 2))
/* Set confirm */
#define WNI_CFG_SET_CNF_RES            0
#define WNI_CFG_SET_CNF_PID            1

#define WNI_CFG_SET_CNF_NUM            2
#define WNI_CFG_SET_CNF_LEN            (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_SET_CNF_NUM << 2))
/* Get attribute response */
#define WNI_CFG_GET_ATTRIB_RSP_RES     0
#define WNI_CFG_GET_ATTRIB_RSP_PID     1
#define WNI_CFG_GET_ATTRIB_RSP_TYPE    2
#define WNI_CFG_GET_ATTRIB_RSP_PLEN    3
#define WNI_CFG_GET_ATTRIB_RSP_RW      4

#define WNI_CFG_GET_ATTRIB_RSP_NUM     5
#define WNI_CFG_GET_ATTRIB_RSP_LEN     (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_GET_ATTRIB_RSP_NUM << 2))

/* Add group address confirm */
#define WNI_CFG_ADD_GRP_ADDR_CNF_RES   0

#define WNI_CFG_ADD_GRP_ADDR_CNF_NUM   1
#define WNI_CFG_ADD_GRP_ADDR_CNF_LEN   (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_ADD_GRP_ADDR_CNF_NUM << 2))

/* Delete group address confirm */
#define WNI_CFG_DEL_GRP_ADDR_CNF_RES   0

#define WNI_CFG_DEL_GRP_ADDR_CNF_NUM   1
#define WNI_CFG_DEL_GRP_ADDR_CNF_LEN   (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_DEL_GRP_ADDR_CNF_NUM << 2))

#define IS_CFG_MSG(msg) ((msg & 0xff00) == WNI_CFG_MSG_TYPES_BEGIN)

/* Clear stats types. */
#define ANI_CLEAR_ALL_STATS          0
#define ANI_CLEAR_RX_STATS           1
#define ANI_CLEAR_TX_STATS           2
#define ANI_CLEAR_PER_STA_STATS      3
#define ANI_CLEAR_AGGR_PER_STA_STATS 4
#define ANI_CLEAR_STAT_TYPES_END     5

/*---------------------------------------------------------------------*/
/* HDD to CFG message types                                            */
/*---------------------------------------------------------------------*/
#define WNI_CFG_DNLD_RSP               (WNI_CFG_MSG_TYPES_BEGIN | 0x80)
#define WNI_CFG_GET_REQ                (WNI_CFG_MSG_TYPES_BEGIN | 0x81)
#define WNI_CFG_SET_REQ                (WNI_CFG_MSG_TYPES_BEGIN | 0x82)
#define WNI_CFG_SET_REQ_NO_RSP         (WNI_CFG_MSG_TYPES_BEGIN | 0x83) /* No RSP for this set */

/* Shall be removed after stats integration */

/*---------------------------------------------------------------------*/
/* HDD to CFG message paramter indices                                 */
/*                                                                     */
/*   The followings are word indices starting from the message body    */
/*                                                                     */
/*   WNI_CFG_xxxx_xxxx_xxxx:         index of parameter                */
/*                                                                     */
/*   WNI_CFG_xxxx_xxxx_NUM:          number of parameters in message   */
/*                                                                     */
/*   WNI_CFG_xxxx_xxxx_LEN:          byte length of message including  */
/*                                   MB header                         */
/*                                                                     */
/*   WNI_CFG_xxxx_xxxx_PARTIAL_LEN:  byte length of message including  */
/*                                   parameters and MB header but      */
/*                                   excluding variable data length    */
/*---------------------------------------------------------------------*/

/* Download response */
#define WNI_CFG_DNLD_RSP_BIN_LEN       0

#define WNI_CFG_DNLD_RSP_NUM           1
#define WNI_CFG_DNLD_RSP_PARTIAL_LEN   (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_DNLD_RSP_NUM << 2))

/* Set parameter request */
#define WNI_CFG_SET_REQ_PID            0
#define WNI_CFG_SET_REQ_PLEN           1

/*---------------------------------------------------------------------*/
/* CFG return values                                                   */
/*---------------------------------------------------------------------*/
#define WNI_CFG_SUCCESS             1
#define WNI_CFG_NOT_READY           2
#define WNI_CFG_INVALID_PID         3
#define WNI_CFG_INVALID_LEN         4
#define WNI_CFG_RO_PARAM            5
#define WNI_CFG_WO_PARAM            6
#define WNI_CFG_INVALID_STAID       7
#define WNI_CFG_OTHER_ERROR         8
#define WNI_CFG_NEED_RESTART        9
#define WNI_CFG_NEED_RELOAD        10

/*---------------------------------------------------------------------*/
/* CFG definitions                                                     */
/*---------------------------------------------------------------------*/

/* Shall be removed after integration of stats. */
/* Get statistic response */
#define WNI_CFG_GET_STAT_RSP_RES       0
#define WNI_CFG_GET_STAT_RSP_PARAMID   1
#define WNI_CFG_GET_STAT_RSP_VALUE     2

#define WNI_CFG_GET_STAT_RSP_NUM       3
#define WNI_CFG_GET_STAT_RSP_LEN       (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_GET_STAT_RSP_NUM << 2))
/* Get per station statistic response */
#define WNI_CFG_GET_PER_STA_STAT_RSP_RES                        0
#define WNI_CFG_GET_PER_STA_STAT_RSP_STAID                      1
#define WNI_CFG_GET_PER_STA_STAT_RSP_FIRST_PARAM                2

/* Per STA statistic structure */
typedef struct sAniCfgPerStaStatStruct {
	unsigned long sentAesBlksUcastHi;
	unsigned long sentAesBlksUcastLo;

	unsigned long recvAesBlksUcastHi;
	unsigned long recvAesBlksUcastLo;

	unsigned long aesFormatErrorUcastCnts;

	unsigned long aesReplaysUcast;

	unsigned long aesDecryptErrUcast;

	unsigned long singleRetryPkts;

	unsigned long failedTxPkts;

	unsigned long ackTimeouts;

	unsigned long multiRetryPkts;

	unsigned long fragTxCntsHi;
	unsigned long fragTxCntsLo;

	unsigned long transmittedPktsHi;
	unsigned long transmittedPktsLo;

	unsigned long phyStatHi;
	unsigned long phyStatLo;
} tCfgPerStaStatStruct, *tpAniCfgPerStaStatStruct;

#define WNI_CFG_GET_PER_STA_STAT_RSP_NUM                       23
#define WNI_CFG_GET_PER_STA_STAT_RSP_LEN    (WNI_CFG_MB_HDR_LEN + \
					     (WNI_CFG_GET_PER_STA_STAT_RSP_NUM << 2))

/* Shall be removed after integrating stats. */
#define WNI_CFG_GET_STAT_RSP           (WNI_CFG_MSG_TYPES_BEGIN | 0x08)
#define WNI_CFG_GET_PER_STA_STAT_RSP   (WNI_CFG_MSG_TYPES_BEGIN | 0x09)
#define WNI_CFG_GET_AGG_STA_STAT_RSP   (WNI_CFG_MSG_TYPES_BEGIN | 0x0a)
#define WNI_CFG_GET_TX_RATE_CTR_RSP    (WNI_CFG_MSG_TYPES_BEGIN | 0x0b)

#define WNI_CFG_GET_AGG_STA_STAT_RSP_NUM    21
#define WNI_CFG_GET_AGG_STA_STAT_RSP_LEN    (WNI_CFG_MB_HDR_LEN + \
					     (WNI_CFG_GET_AGG_STA_STAT_RSP_NUM << 2))
#define WNI_CFG_GET_AGG_STA_STAT_RSP_RES 0

/* Get TX rate based stats */
#define WNI_CFG_GET_TX_RATE_CTR_RSP_RES                        0

typedef struct sAniCfgTxRateCtrs {
/* add the rate counters here */
	unsigned long TxFrames_1Mbps;
	unsigned long TxFrames_2Mbps;
	unsigned long TxFrames_5_5Mbps;
	unsigned long TxFrames_6Mbps;
	unsigned long TxFrames_9Mbps;
	unsigned long TxFrames_11Mbps;
	unsigned long TxFrames_12Mbps;
	unsigned long TxFrames_18Mbps;
	unsigned long TxFrames_24Mbps;
	unsigned long TxFrames_36Mbps;
	unsigned long TxFrames_48Mbps;
	unsigned long TxFrames_54Mbps;
	unsigned long TxFrames_72Mbps;
	unsigned long TxFrames_96Mbps;
	unsigned long TxFrames_108Mbps;

} tAniCfgTxRateCtrs, *tpAniCfgTxRateCtrs;

#define WNI_CFG_GET_STAT_REQ           (WNI_CFG_MSG_TYPES_BEGIN | 0x86)
#define WNI_CFG_GET_PER_STA_STAT_REQ   (WNI_CFG_MSG_TYPES_BEGIN | 0x87)
#define WNI_CFG_GET_AGG_STA_STAT_REQ   (WNI_CFG_MSG_TYPES_BEGIN | 0x88)
#define WNI_CFG_GET_TX_RATE_CTR_REQ    (WNI_CFG_MSG_TYPES_BEGIN | 0x89)

/* Get statistic request */
#define WNI_CFG_GET_STAT_REQ_PARAMID   0

#define WNI_CFG_GET_STAT_REQ_NUM       1
#define WNI_CFG_GET_STAT_REQ_LEN       (WNI_CFG_MB_HDR_LEN + \
					(WNI_CFG_GET_STAT_REQ_NUM << 2))

/* Get per station statistic request */
#define WNI_CFG_GET_PER_STA_STAT_REQ_STAID 0

#define WNI_CFG_GET_PER_STA_STAT_REQ_NUM   1
#define WNI_CFG_GET_PER_STA_STAT_REQ_LEN   (WNI_CFG_MB_HDR_LEN + \
					    (WNI_CFG_GET_PER_STA_STAT_REQ_NUM << 2))

#define DYNAMIC_CFG_TYPE_SELECTED_REGISTRAR   (0)
#define DYNAMIC_CFG_TYPE_WPS_STATE            (1)

#endif /* __WNI_API_H */
