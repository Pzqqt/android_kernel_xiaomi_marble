/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_MGMT_TXRX_UTILS_API_H_
#define _WLAN_MGMT_TXRX_UTILS_API_H_

/**
 * DOC:  wlan_mgmt_txrx_utils_api.h
 *
 * management tx/rx layer public API and structures
 * for umac converged components.
 *
 */

#include "wlan_objmgr_cmn.h"
#include "qdf_nbuf.h"

#ifdef CONFIG_MCL
#define MGMT_DESC_POOL_MAX 64
#else
#define MGMT_DESC_POOL_MAX 512
#endif

#define mgmt_txrx_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmt_txrx_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_MGMT_TXRX, params)

#define mgmttxrx_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmttxrx_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmttxrx_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmttxrx_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_MGMT_TXRX, params)
#define mgmttxrx_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_MGMT_TXRX, params)

/**
 * enum mgmt_subtype - enum of mgmt. subtypes
 * @MGMT_SUBTYPE_ASSOC_REQ:       association request frame
 * @MGMT_SUBTYPE_ASSOC_RESP:      association response frame
 * @MGMT_SUBTYPE_REASSOC_REQ:     reassociation request frame
 * @MGMT_SUBTYPE_REASSOC_RESP:    reassociation response frame
 * @MGMT_SUBTYPE_PROBE_REQ:       probe request frame
 * @MGMT_SUBTYPE_PROBE_RESP:      probe response frame
 * @MGMT_SUBTYPE_BEACON:          beacon frame
 * @MGMT_SUBTYPE_ATIM:            ATIM frame
 * @MGMT_SUBTYPE_DISASSOC:        disassociation frame
 * @MGMT_SUBTYPE_AUTH:            authentication frame
 * @MGMT_SUBTYPE_DEAUTH:          deauthentication frame
 * @MGMT_SUBTYPE_ACTION:          action frame
 * @MGMT_SUBTYPE_ACTION_NO_ACK:   action no ack frame
 */
enum mgmt_subtype {
	MGMT_SUBTYPE_ASSOC_REQ = 0x00,
	MGMT_SUBTYPE_ASSOC_RESP = 0x10,
	MGMT_SUBTYPE_REASSOC_REQ = 0x20,
	MGMT_SUBTYPE_REASSOC_RESP = 0x30,
	MGMT_SUBTYPE_PROBE_REQ = 0x40,
	MGMT_SUBTYPE_PROBE_RESP = 0x50,
	MGMT_SUBTYPE_BEACON = 0x80,
	MGMT_SUBTYPE_ATIM = 0x90,
	MGMT_SUBTYPE_DISASSOC = 0xa0,
	MGMT_SUBTYPE_AUTH = 0xb0,
	MGMT_SUBTYPE_DEAUTH = 0xc0,
	MGMT_SUBTYPE_ACTION = 0xd0,
	MGMT_SUBTYPE_ACTION_NO_ACK = 0xe0,
};

/**
 * enum mgmt_action_category - mgmt. action categories
 * @ACTION_CATEGORY_SPECTRUM_MGMT:  spectrum mgmt. action category
 * @ACTION_CATEGORY_QOS: qos action category
 * @ACTION_CATEGORY_DLS: dls action category
 * @ACTION_CATEGORY_BACK: block ack action category
 * @ACTION_CATEGORY_PUBLIC: public action category
 * @ACTION_CATEGORY_RRM: rrm action category
 * @ACTION_FAST_BSS_TRNST: trnst action category
 * @ACTION_CATEGORY_HT: ht actipon category
 * @ACTION_CATEGORY_SA_QUERY: sa query action category
 * @ACTION_CATEGORY_PROTECTED_DUAL_OF_PUBLIC_ACTION: protected
 *                           public action category
 * @ACTION_CATEGORY_WNM: wnm action category
 * @ACTION_CATEGORY_WNM_UNPROTECTED: wnm protected action category
 * @ACTION_CATEGORY_TDLS: tdls action category
 * @ACTION_CATEGORY_MESH_ACTION: mesh action category
 * @ACTION_CATEGORY_MULTIHOP_ACTION: multihop action category
 * @ACTION_CATEGORY_SELF_PROTECTED: self protected action category
 * @ACTION_CATEGORY_DMG: unprotected dmg action category
 * @ACTION_CATEGORY_WMM: wmm action category
 * @ACTION_CATEGORY_FST: fst action category
 * @ACTION_CATEGORY_UNPROT_DMG: dmg action category
 * @ACTION_CATEGORY_VHT: vht action category
 * @ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED: vendor specific protected
 *                                             action category
 * @ACTION_CATEGORY_VENDOR_SPECIFIC: vendor specific action category
 */
enum mgmt_action_category {
	ACTION_CATEGORY_SPECTRUM_MGMT = 0,
	ACTION_CATEGORY_QOS = 1,
	ACTION_CATEGORY_DLS = 2,
	ACTION_CATEGORY_BACK = 3,
	ACTION_CATEGORY_PUBLIC = 4,
	ACTION_CATEGORY_RRM = 5,
	ACTION_FAST_BSS_TRNST = 6,
	ACTION_CATEGORY_HT = 7,
	ACTION_CATEGORY_SA_QUERY = 8,
	ACTION_CATEGORY_PROTECTED_DUAL_OF_PUBLIC_ACTION = 9,
	ACTION_CATEGORY_WNM = 10,
	ACTION_CATEGORY_WNM_UNPROTECTED = 11,
	ACTION_CATEGORY_TDLS = 12,
	ACTION_CATEGORY_MESH_ACTION = 13,
	ACTION_CATEGORY_MULTIHOP_ACTION = 14,
	ACTION_CATEGORY_SELF_PROTECTED = 15,
	ACTION_CATEGORY_DMG = 16,
	ACTION_CATEGORY_WMM = 17,
	ACTION_CATEGORY_FST = 18,
	ACTION_CATEGORY_UNPROT_DMG = 20,
	ACTION_CATEGORY_VHT = 21,
	ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED = 126,
	ACTION_CATEGORY_VENDOR_SPECIFIC = 127,
};

/**
 * enum spectrum_mgmt_actioncode - spectrum mgmt. action frms
 * @ACTION_SPCT_MSR_REQ:  spectrum measurement request frame
 * @ACTION_SPCT_MSR_RPRT: spectrum measurement report frame
 * @ACTION_SPCT_TPC_REQ: spectrum tpc request frame
 * @ACTION_SPCT_TPC_RPRT: spectrum tpc report frame
 * @ACTION_SPCT_CHL_SWITCH: spectrum channel switch frame
 */
enum spectrum_mgmt_actioncode {
	ACTION_SPCT_MSR_REQ,
	ACTION_SPCT_MSR_RPRT,
	ACTION_SPCT_TPC_REQ,
	ACTION_SPCT_TPC_RPRT,
	ACTION_SPCT_CHL_SWITCH,
};

/**
 * enum qos_actioncode - qos action frames
 * @QOS_ADD_TS_REQ:  qos add ts request frame
 * @QOS_ADD_TS_RSP: qos add ts response frame
 * @QOS_DEL_TS_REQ: qos del ts request frame
 * @QOS_SCHEDULE: qos schecule frame
 * @QOS_MAP_CONFIGURE: qos map configure frame
 */
enum qos_actioncode {
	QOS_ADD_TS_REQ,
	QOS_ADD_TS_RSP,
	QOS_DEL_TS_REQ,
	QOS_SCHEDULE,
	QOS_MAP_CONFIGURE,
};

/**
 * enum dls_actioncode - dls action frames
 * @DLS_REQUEST:  dls request frame
 * @DLS_RESPONSE: dls response frame
 * @DLS_TEARDOWN: dls teardown frame
 */
enum dls_actioncode {
	DLS_REQUEST,
	DLS_RESPONSE,
	DLS_TEARDOWN,
};

/**
 * enum block_ack_actioncode - block ack action frames
 * @ADDBA_REQUEST:  add block ack request frame
 * @ADDBA_RESPONSE: add block ack response frame
 * @DELBA: delete block ack frame
 */
enum block_ack_actioncode {
	ADDBA_REQUEST,
	ADDBA_RESPONSE,
	DELBA,
};

/**
 * enum pub_actioncode - public action frames
 * @PUB_ACTION_2040_BSS_COEXISTENCE:  public 20-40 bss coex action frame
 * @PUB_ACTION_EXT_CHANNEL_SWITCH_ID: public ext channel switch id action frame
 * @PUB_ACTION_VENDOR_SPECIFIC: vendor specific public action frame
 * @PUB_ACTION_GAS_INITIAL_REQUEST: GAS initial request action frame
 * @PUB_ACTION_GAS_INITIAL_RESPONSE: GAS initial response action frame
 * @PUB_ACTION_GAS_COMEBACK_REQUEST: GAS comeback request action frame
 * @PUB_ACTION_GAS_COMEBACK_RESPONSE: GAS comeback respose action frame
 * @PUB_ACTION_TDLS_DISCRESP: tdls discovery response public action frame
 */
enum pub_actioncode {
	PUB_ACTION_2040_BSS_COEXISTENCE = 0,
	PUB_ACTION_EXT_CHANNEL_SWITCH_ID = 4,
	PUB_ACTION_VENDOR_SPECIFIC = 9,
	PUB_ACTION_GAS_INITIAL_REQUEST = 10,
	PUB_ACTION_GAS_INITIAL_RESPONSE = 11,
	PUB_ACTION_GAS_COMEBACK_REQUEST = 12,
	PUB_ACTION_GAS_COMEBACK_RESPONSE = 13,
	PUB_ACTION_TDLS_DISCRESP = 14,
};

/**
 * enum rrm_actioncode - rrm action frames
 * @RRM_RADIO_MEASURE_REQ: rrm radio meas. request frame
 * @RRM_RADIO_MEASURE_RPT: rrm radio meas. report frame
 * @RRM_LINK_MEASUREMENT_REQ: rrm link meas. request frmae
 * @RRM_LINK_MEASUREMENT_RPT: rrm link meas. report frame
 * @RRM_NEIGHBOR_REQ: rrm neighbor request frame
 * @RRM_NEIGHBOR_RPT: rrm neighbor report frame
 */
enum rrm_actioncode {
	RRM_RADIO_MEASURE_REQ,
	RRM_RADIO_MEASURE_RPT,
	RRM_LINK_MEASUREMENT_REQ,
	RRM_LINK_MEASUREMENT_RPT,
	RRM_NEIGHBOR_REQ,
	RRM_NEIGHBOR_RPT,
};

/**
 * enum ht_actioncode - ht action frames
 * @HT_ACTION_NOTIFY_CHANWIDTH: ht notify bw action frame
 * @HT_ACTION_SMPS: ht smps action frame
 * @HT_ACTION_PSMP: ht psmp action frame
 * @HT_ACTION_PCO_PHASE: ht pco phase action frame
 * @HT_ACTION_CSI: ht csi action frame
 * @HT_ACTION_NONCOMPRESSED_BF: ht noncompressed bf action frame
 * @HT_ACTION_COMPRESSED_BF: ht compressed bf action frame
 * @HT_ACTION_ASEL_IDX_FEEDBACK: ht asel idx feedback action frame
 */
enum ht_actioncode {
	HT_ACTION_NOTIFY_CHANWIDTH,
	HT_ACTION_SMPS,
	HT_ACTION_PSMP,
	HT_ACTION_PCO_PHASE,
	HT_ACTION_CSI,
	HT_ACTION_NONCOMPRESSED_BF,
	HT_ACTION_COMPRESSED_BF,
	HT_ACTION_ASEL_IDX_FEEDBACK,
};

/**
 * enum sa_query_action - sa query action frames
 * @SA_QUERY_REQUEST: sa query request frame
 * @SA_QUERY_RESPONSE: sa query response frame
 */
enum sa_query_action {
	SA_QUERY_REQUEST,
	SA_QUERY_RESPONSE,
};

/**
 * enum protected_dual_actioncode - protected dual action frames
 * @PDPA_GAS_INIT_REQ: pdpa gas init request frame
 * @PDPA_GAS_INIT_RSP: pdpa gas init response frame
 * @PDPA_GAS_COMEBACK_REQ: pdpa gas comeback request frame
 * @PDPA_GAS_COMEBACK_RSP: pdpa gas comeback response frame
 */
enum protected_dual_actioncode {
	PDPA_GAS_INIT_REQ = 10,
	PDPA_GAS_INIT_RSP = 11,
	PDPA_GAS_COMEBACK_REQ = 12,
	PDPA_GAS_COMEBACK_RSP = 13,
};

/**
 * enum wnm_actioncode - wnm action frames
 * @WNM_BSS_TM_QUERY: wnm bss tm query frame
 * @WNM_BSS_TM_REQUEST: wnm bss tm request frame
 * @WNM_BSS_TM_RESPONSE: wnm bss tm response frame
 * @WNM_FMS_REQ: wnm fms request frame
 * @WNM_FMS_RESP: wnm fms response frame
 * @WNM_TFS_REQ: wnm tfs request frame
 * @WNM_TFS_RESP: wnm tfs response frame
 * @WNM_TFS_NOTIFY: wnm tfs notify frame
 * @WNM_SLEEP_REQ: wnm sleep request frame
 * @WNM_SLEEP_RESP: wnm sleep response frame
 * @WNM_TIM_REQ: wnm Tim broadcast request frame
 * @WNM_TIM_RESP: wnm Tim broadcast response frame
 * @WNM_NOTIF_REQUEST: wnm notify request frame
 * @WNM_NOTIF_RESPONSE: wnm notify response frame
 */
enum wnm_actioncode {
	WNM_BSS_TM_QUERY = 6,
	WNM_BSS_TM_REQUEST = 7,
	WNM_BSS_TM_RESPONSE = 8,
	WNM_FMS_REQ = 9,
	WNM_FMS_RESP = 10,
	WNM_TFS_REQ = 13,
	WNM_TFS_RESP = 14,
	WNM_TFS_NOTIFY = 15,
	WNM_SLEEP_REQ = 16,
	WNM_SLEEP_RESP = 17,
	WNM_TIM_REQ = 18,
	WNM_TIM_RESP = 19,
	WNM_NOTIF_REQUEST = 26,
	WNM_NOTIF_RESPONSE = 27,
};

/**
 * enum tdls_actioncode - tdls action frames
 * @TDLS_SETUP_REQUEST: tdls setup request frame
 * @TDLS_SETUP_RESPONSE: tdls setup response frame
 * @TDLS_SETUP_CONFIRM: tdls setup confirm frame
 * @TDLS_TEARDOWN: tdls teardown frame
 * @TDLS_PEER_TRAFFIC_INDICATION: tdls peer traffic indication frame
 * @TDLS_CHANNEL_SWITCH_REQUEST: tdls channel switch req. frame
 * @TDLS_CHANNEL_SWITCH_RESPONSE: tdls channel switch response frame
 * @TDLS_PEER_PSM_REQUEST: tdls peer psm request frame
 * @TDLS_PEER_PSM_RESPONSE: tdls peer psm response frame
 * @TDLS_PEER_TRAFFIC_RESPONSE: tdls peer traffic response frame
 * @TDLS_DISCOVERY_REQUEST: tdls discovery request frame
 */
enum tdls_actioncode {
	TDLS_SETUP_REQUEST = 0,
	TDLS_SETUP_RESPONSE = 1,
	TDLS_SETUP_CONFIRM = 2,
	TDLS_TEARDOWN = 3,
	TDLS_PEER_TRAFFIC_INDICATION = 4,
	TDLS_CHANNEL_SWITCH_REQUEST = 5,
	TDLS_CHANNEL_SWITCH_RESPONSE = 6,
	TDLS_PEER_PSM_REQUEST = 7,
	TDLS_PEER_PSM_RESPONSE = 8,
	TDLS_PEER_TRAFFIC_RESPONSE = 9,
	TDLS_DISCOVERY_REQUEST = 10,
	TDLS_DISCOVERY_RESPONSE = 14,
};

/**
 * enum mesh_actioncode - mesh action frames
 * @MESH_ACTION_LINK_METRIC_REPORT: mesh link metric report action frame
 * @MESH_ACTION_HWMP_PATH_SELECTION: mesh hwmp path selection action frame
 * @MESH_ACTION_GATE_ANNOUNCEMENT: mesh gate announcement action frame
 * @MESH_ACTION_CONGESTION_CONTROL_NOTIFICATION: mesh congestion control frame
 * @MESH_ACTION_MCCA_SETUP_REQUEST: mesh mcca setup request action frame
 * @MESH_ACTION_MCCA_SETUP_REPLY: mesh mcca setup reply action frame
 * @MESH_ACTION_MCCA_ADVERTISEMENT_REQUEST: mesh mcca advertisement req. frame
 * @MESH_ACTION_MCCA_ADVERTISEMENT: mesh mcca advertisement action frame
 * @MESH_ACTION_MCCA_TEARDOWN: mesh mcca teardown action frame
 * @MESH_ACTION_TBTT_ADJUSTMENT_REQUEST: mesh tbtt adjustment req. frame
 * @MESH_ACTION_TBTT_ADJUSTMENT_RESPONSE: mesh tbtt adjustment rsp. frame
 */
enum mesh_actioncode {
	MESH_ACTION_LINK_METRIC_REPORT,
	MESH_ACTION_HWMP_PATH_SELECTION,
	MESH_ACTION_GATE_ANNOUNCEMENT,
	MESH_ACTION_CONGESTION_CONTROL_NOTIFICATION,
	MESH_ACTION_MCCA_SETUP_REQUEST,
	MESH_ACTION_MCCA_SETUP_REPLY,
	MESH_ACTION_MCCA_ADVERTISEMENT_REQUEST,
	MESH_ACTION_MCCA_ADVERTISEMENT,
	MESH_ACTION_MCCA_TEARDOWN,
	MESH_ACTION_TBTT_ADJUSTMENT_REQUEST,
	MESH_ACTION_TBTT_ADJUSTMENT_RESPONSE,
};

/**
 * enum self_protected_actioncode - self protected action frames
 * @SP_RESERVED: self protected reserved
 * @SP_MESH_PEERING_OPEN: self protected mesh peering open frame
 * @SP_MESH_PEERING_CONFIRM: self protected mesh peering confirm frame
 * @SP_MESH_PEERING_CLOSE: self protected mesh peering close frame
 * @SP_MGK_INFORM: self protected mgk inform frame
 * @SP_MGK_ACK: self protected mgk ack frame
 */
enum self_protected_actioncode {
	SP_RESERVED,
	SP_MESH_PEERING_OPEN,
	SP_MESH_PEERING_CONFIRM,
	SP_MESH_PEERING_CLOSE,
	SP_MGK_INFORM,
	SP_MGK_ACK,
};

/**
 * enum wmm_actioncode - wmm action frames
 * @WMM_QOS_SETUP_REQ: wmm qos setup request frame
 * @WMM_QOS_SETUP_RESP: q wmm qos setup response frame
 * @WMM_QOS_TEARDOWN:  wmm qos teardown frame
 */
enum wmm_actioncode {
	WMM_QOS_SETUP_REQ,
	WMM_QOS_SETUP_RESP,
	WMM_QOS_TEARDOWN,
};

/**
 * enum fst_actioncode - fst action frames
 * @FST_SETUP_REQ: fst setup request frame
 * @FST_SETUP_RSP: fst setup response frame
 * @FST_TEAR_DOWN: fst qos teardown frame
 * @FST_ACK_REQ:  fst ack frame for request
 * @FST_ACK_RSP:  fst ack frame for response
 * @FST_ON_CHANNEL_TUNNEL:  fst on channel tunnel frame
 */
enum fst_actioncode {
	FST_SETUP_REQ,
	FST_SETUP_RSP,
	FST_TEAR_DOWN,
	FST_ACK_REQ,
	FST_ACK_RSP,
	FST_ON_CHANNEL_TUNNEL,
};

/**
 * enum vht_actioncode - vht action frames
 * @VHT_ACTION_COMPRESSED_BF: vht compressed bf action frame
 * @VHT_ACTION_GID_NOTIF: vht gid notification action frame
 * @VHT_ACTION_OPMODE_NOTIF: vht opmode notification action frame
 */
enum vht_actioncode {
	VHT_ACTION_COMPRESSED_BF,
	VHT_ACTION_GID_NOTIF,
	VHT_ACTION_OPMODE_NOTIF,
};

/**
 * struct action_frm_hdr - action frame header
 * @action_category: action category
 * @action_code: action code
 */
struct action_frm_hdr {
	uint8_t action_category;
	uint8_t action_code;
};

/**
 * enum mgmt_frame_type - enum of mgmt. frames
 * @MGMT_FRM_UNSPECIFIED:           unspecified
 * @MGMT_ASSOC_REQ:                 association request frame
 * @MGMT_ASSOC_RESP:                association response frame
 * @MGMT_REASSOC_REQ:               reassociation request frame
 * @MGMT_REASSOC_RESP:              reassociation response frame
 * @MGMT_PROBE_REQ:                 probe request frame
 * @MGMT_PROBE_RESP:                probe response frame
 * @MGMT_BEACON:                    beacon frame
 * @MGMT_ATIM:                      ATIM frame
 * @MGMT_DISASSOC:                  disassociation frame
 * @MGMT_AUTH:                      authentication frame
 * @MGMT_DEAUTH:                    deauthentication frame
 * @MGMT_ACTION_MEAS_REQUEST:       measure channels request action frame
 * @MGMT_ACTION_MEAS_REPORT:        measure channels response action frame
 * @MGMT_ACTION_TPC_REQUEST:        transmit power control request action frame
 * @MGMT_ACTION_TPC_REPORT:         transmit power control response action frame
 * @MGMT_ACTION_CHAN_SWITCH:        802.11 channel switch announcement frame
 * @MGMT_ACTION_QOS_ADD_TS_REQ:     qos add ts request frame
 * @MGMT_ACTION_QOS_ADD_TS_RSP:     qos add ts response frame
 * @MGMT_ACTION_QOS_DEL_TS_REQ:     qos del ts request frame
 * @MGMT_ACTION_QOS_SCHEDULE:       qos schedule frame
 * @MGMT_ACTION_QOS_MAP_CONFIGURE:  qos map configure frame
 * @MGMT_ACTION_DLS_REQUEST:        DLS request action frame
 * @MGMT_ACTION_DLS_RESPONSE:       DLS response action frame
 * @MGMT_ACTION_DLS_TEARDOWN:       DLS taerdown action frame
 * @MGMT_ACTION_BA_ADDBA_REQUEST:   ADDBA request action frame
 * @MGMT_ACTION_BA_ADDBA_RESPONSE:  ADDBA response action frame
 * @MGMT_ACTION_BA_DELBA:           DELBA action frame
 * @MGMT_ACTION_2040_BSS_COEXISTENCE: 20-40 bss coex action frame
 * @MGMT_ACTION_CATEGORY_VENDOR_SPECIFIC: category vendor spcific action frame
 * @MGMT_ACTION_EXT_CHANNEL_SWITCH_ID: ext channel switch id action frame
 * @MGMT_ACTION_VENDOR_SPECIFIC:    vendor specific action frame
 * @MGMT_ACTION_TDLS_DISCRESP:      TDLS discovery response frame
 * @MGMT_ACTION_RRM_RADIO_MEASURE_REQ: rrm radio meas. req. action frame
 * @MGMT_ACTION_RRM_RADIO_MEASURE_RPT: rrm radio meas. report action frame
 * @MGMT_ACTION_RRM_LINK_MEASUREMENT_REQ: rrm link meas. req. action frame
 * @MGMT_ACTION_RRM_LINK_MEASUREMENT_RPT: rrm link meas. report action frame
 * @MGMT_ACTION_RRM_NEIGHBOR_REQ: rrm neighbor request action frame
 * @MGMT_ACTION_RRM_NEIGHBOR_RPT: rrm neighbor response action frame
 * @MGMT_ACTION_HT_NOTIFY_CHANWIDTH: notify channel width action frame
 * @MGMT_ACTION_HT_SMPS:            spatial multiplexing power save action frame
 * @MGMT_ACTION_HT_PSMP:            psmp action frame
 * @MGMT_ACTION_HT_PCO_PHASE:       pco phase action frame
 * @MGMT_ACTION_HT_CSI:             CSI action frame
 * @MGMT_ACTION_HT_NONCOMPRESSED_BF: non-compressed beamforming action frame
 * @MGMT_ACTION_HT_COMPRESSED_BF:   compressed beamforming action frame
 * @MGMT_ACTION_HT_ASEL_IDX_FEEDBACK: asel idx feedback action frame
 * @MGMT_ACTION_SA_QUERY_REQUEST:   SA query request frame
 * @MGMT_ACTION_SA_QUERY_RESPONSE:  SA query response frame
 * @MGMT_ACTION_PDPA_GAS_INIT_REQ:  pdpa gas init request action frame
 * @MGMT_ACTION_PDPA_GAS_INIT_RSP:  pdpa gas init response frame
 * @MGMT_ACTION_PDPA_GAS_COMEBACK_REQ: pdpa gas comeback req. action frame
 * @MGMT_ACTION_PDPA_GAS_COMEBACK_RSP: pdpa gas comeback rsp. action frame
 * @MGMT_ACTION_WNM_BSS_TM_QUERY:   wnm bss tm query action frame
 * @MGMT_ACTION_WNM_BSS_TM_REQUEST: wnm bss tm request action frame
 * @MGMT_ACTION_WNM_BSS_TM_RESPONSE: wnm bss tm response action frame
 * @MGMT_ACTION_WNM_NOTIF_REQUEST:  wnm notification request action frame
 * @MGMT_ACTION_WNM_NOTIF_RESPONSE: wnm notification response action frame
 * @MGMT_ACTION_WNM_FMS_REQ:    wnm fms request frame
 * @MGMT_ACTION_WNM_FMS_RESP:   wnm fms response frame
 * @MGMT_ACTION_WNM_TFS_REQ:    wnm tfs request frame
 * @MGMT_ACTION_WNM_TFS_RESP:   wnm tfs response frame
 * @MGMT_ACTION_WNM_TFS_NOTIFY: wnm tfs notify frame
 * @MGMT_ACTION_WNM_SLEEP_REQ:  wnm sleep request frame
 * @MGMT_ACTION_WNM_SLEEP_RESP: wnm sleep response frame
 * @MGMT_ACTION_WNM_TIM_REQ:    wnm Tim broadcast request frame
 * @MGMT_ACTION_WNM_TIM_RESP:   wnm Tim broadcast response frame
 * @MGMT_ACTION_TDLS_SETUP_REQ:     tdls setup request action frame
 * @MGMT_ACTION_TDLS_SETUP_RSP:     tdls setup response frame
 * @MGMT_ACTION_TDLS_SETUP_CNF:     tdls setup confirm frame
 * @MGMT_ACTION_TDLS_TEARDOWN:      tdls teardown frame
 * @MGMT_ACTION_TDLS_PEER_TRAFFIC_IND: tdls peer traffic indication frame
 * @MGMT_ACTION_TDLS_CH_SWITCH_REQ: tdls channel switch req. frame
 * @MGMT_ACTION_TDLS_CH_SWITCH_RSP: tdls channel switch response frame
 * @MGMT_ACTION_TDLS_PEER_PSM_REQUEST: tdls peer psm request frame
 * @MGMT_ACTION_TDLS_PEER_PSM_RESPONSE: tdls peer psm response frame
 * @MGMT_ACTION_TDLS_PEER_TRAFFIC_RSP: tdls peer traffic response frame
 * @MGMT_ACTION_TDLS_DIS_REQ:       tdls discovery request frame
 * @MGMT_ACTION_MESH_LINK_METRIC_REPORT: mesh link metric report action frame
 * @MGMT_ACTION_MESH_HWMP_PATH_SELECTION: mesh hwmp path selection action frame
 * @MGMT_ACTION_MESH_GATE_ANNOUNCEMENT: mesh gate announcement action frame
 * @MGMT_ACTION_MESH_CONGESTION_CONTROL_NOTIFICATION: mesh congestion control
 * @MGMT_ACTION_MESH_MCCA_SETUP_REQUEST: mesh mcca setup request action frame
 * @MGMT_ACTION_MESH_MCCA_SETUP_REPLY: mesh mcca setup reply action frame
 * @MGMT_ACTION_MESH_MCCA_ADVERTISEMENT_REQUEST: mesh mcca advertisement req.
 * @MGMT_ACTION_MESH_MCCA_ADVERTISEMENT: mesh mcca advertisement action frame
 * @MGMT_ACTION_MESH_MCCA_TEARDOWN: mesh mcca teardown action fram
 * @MGMT_ACTION_MESH_TBTT_ADJUSTMENT_REQUEST: mesh tbtt adjustment req. frame
 * @MGMT_ACTION_MESH_TBTT_ADJUSTMENT_RESPONSE: mesh tbtt adjustment rsp. frame
 * @MGMT_ACTION_SP_MESH_PEERING_OPEN: self protected mesh peering open frame
 * @MGMT_ACTION_SP_MESH_PEERING_CONFIRM: self protected mesh peering confirm
 * @MGMT_ACTION_SP_MESH_PEERING_CLOSE: self protected mesh peering close frame
 * @MGMT_ACTION_SP_MGK_INFORM:   self protected mgk inform frame
 * @MGMT_ACTION_SP_MGK_ACK:      self protected mgk ack frame
 * @MGMT_ACTION_WMM_QOS_SETUP_REQ: WMM qos setup request action frame
 * @MGMT_ACTION_WMM_QOS_SETUP_RESP: WMM qos setup response action frame
 * @MGMT_ACTION_WMM_QOS_TEARDOWN: WMM qos teardown action frame
 * @MGMT_ACTION_VHT_COMPRESSED_BF: vht compressed bf action frame
 * @MGMT_ACTION_VHT_GID_NOTIF:   vht gid notification action frame
 * @MGMT_ACTION_VHT_OPMODE_NOTIF: vht opmode notification action frame
 * @MGMT_ACTION_GAS_INITIAL_REQUEST: GAS Initial request action frame
 * @MGMT_ACTION_GAS_INITIAL_RESPONSE: GAS Initial response action frame
 * @MGMT_ACTION_GAS_COMEBACK_REQUEST: GAS Comeback request action frame
 * @MGMT_ACTION_GAS_COMEBACK_RESPONSE: GAS Comeback response action frame
 * @MGMT_ACTION_FST_SETUP_REQ: FST setup request frame
 * @MGMT_ACTION_FST_SETUP_RSPA: FST setup response frame
 * @MGMT_ACTION_FST_TEAR_DOWN: FST qos teardown frame
 * @MGMT_ACTION_FST_ACK_REQ: FST ack frame for request
 * @MGMT_ACTION_FST_ACK_RSP: FST ack frame for response
 * @MGMT_ACTION_FST_ON_CHANNEL_TUNNEL: FST on channel tunnel frame
 * @MGMT_FRAME_TYPE_ALL:         mgmt frame type for all type of frames
 * @MGMT_MAX_FRAME_TYPE:         max. mgmt frame types
 */
enum mgmt_frame_type {
	MGMT_FRM_UNSPECIFIED = -1,
	MGMT_ASSOC_REQ,
	MGMT_ASSOC_RESP,
	MGMT_REASSOC_REQ,
	MGMT_REASSOC_RESP,
	MGMT_PROBE_REQ,
	MGMT_PROBE_RESP,
	MGMT_BEACON,
	MGMT_ATIM,
	MGMT_DISASSOC,
	MGMT_AUTH,
	MGMT_DEAUTH,
	MGMT_ACTION_MEAS_REQUEST,
	MGMT_ACTION_MEAS_REPORT,
	MGMT_ACTION_TPC_REQUEST,
	MGMT_ACTION_TPC_REPORT,
	MGMT_ACTION_CHAN_SWITCH,
	MGMT_ACTION_QOS_ADD_TS_REQ,
	MGMT_ACTION_QOS_ADD_TS_RSP,
	MGMT_ACTION_QOS_DEL_TS_REQ,
	MGMT_ACTION_QOS_SCHEDULE,
	MGMT_ACTION_QOS_MAP_CONFIGURE,
	MGMT_ACTION_DLS_REQUEST,
	MGMT_ACTION_DLS_RESPONSE,
	MGMT_ACTION_DLS_TEARDOWN,
	MGMT_ACTION_BA_ADDBA_REQUEST,
	MGMT_ACTION_BA_ADDBA_RESPONSE,
	MGMT_ACTION_BA_DELBA,
	MGMT_ACTION_2040_BSS_COEXISTENCE,
	MGMT_ACTION_CATEGORY_VENDOR_SPECIFIC,
	MGMT_ACTION_EXT_CHANNEL_SWITCH_ID,
	MGMT_ACTION_VENDOR_SPECIFIC,
	MGMT_ACTION_TDLS_DISCRESP,
	MGMT_ACTION_RRM_RADIO_MEASURE_REQ,
	MGMT_ACTION_RRM_RADIO_MEASURE_RPT,
	MGMT_ACTION_RRM_LINK_MEASUREMENT_REQ,
	MGMT_ACTION_RRM_LINK_MEASUREMENT_RPT,
	MGMT_ACTION_RRM_NEIGHBOR_REQ,
	MGMT_ACTION_RRM_NEIGHBOR_RPT,
	MGMT_ACTION_HT_NOTIFY_CHANWIDTH,
	MGMT_ACTION_HT_SMPS,
	MGMT_ACTION_HT_PSMP,
	MGMT_ACTION_HT_PCO_PHASE,
	MGMT_ACTION_HT_CSI,
	MGMT_ACTION_HT_NONCOMPRESSED_BF,
	MGMT_ACTION_HT_COMPRESSED_BF,
	MGMT_ACTION_HT_ASEL_IDX_FEEDBACK,
	MGMT_ACTION_SA_QUERY_REQUEST,
	MGMT_ACTION_SA_QUERY_RESPONSE,
	MGMT_ACTION_PDPA_GAS_INIT_REQ,
	MGMT_ACTION_PDPA_GAS_INIT_RSP,
	MGMT_ACTION_PDPA_GAS_COMEBACK_REQ,
	MGMT_ACTION_PDPA_GAS_COMEBACK_RSP,
	MGMT_ACTION_WNM_BSS_TM_QUERY,
	MGMT_ACTION_WNM_BSS_TM_REQUEST,
	MGMT_ACTION_WNM_BSS_TM_RESPONSE,
	MGMT_ACTION_WNM_NOTIF_REQUEST,
	MGMT_ACTION_WNM_NOTIF_RESPONSE,
	MGMT_ACTION_WNM_FMS_REQ,
	MGMT_ACTION_WNM_FMS_RESP,
	MGMT_ACTION_WNM_TFS_REQ,
	MGMT_ACTION_WNM_TFS_RESP,
	MGMT_ACTION_WNM_TFS_NOTIFY,
	MGMT_ACTION_WNM_SLEEP_REQ,
	MGMT_ACTION_WNM_SLEEP_RESP,
	MGMT_ACTION_WNM_TIM_REQ,
	MGMT_ACTION_WNM_TIM_RESP,
	MGMT_ACTION_TDLS_SETUP_REQ,
	MGMT_ACTION_TDLS_SETUP_RSP,
	MGMT_ACTION_TDLS_SETUP_CNF,
	MGMT_ACTION_TDLS_TEARDOWN,
	MGMT_ACTION_TDLS_PEER_TRAFFIC_IND,
	MGMT_ACTION_TDLS_CH_SWITCH_REQ,
	MGMT_ACTION_TDLS_CH_SWITCH_RSP,
	MGMT_ACTION_TDLS_PEER_PSM_REQUEST,
	MGMT_ACTION_TDLS_PEER_PSM_RESPONSE,
	MGMT_ACTION_TDLS_PEER_TRAFFIC_RSP,
	MGMT_ACTION_TDLS_DIS_REQ,
	MGMT_ACTION_MESH_LINK_METRIC_REPORT,
	MGMT_ACTION_MESH_HWMP_PATH_SELECTION,
	MGMT_ACTION_MESH_GATE_ANNOUNCEMENT,
	MGMT_ACTION_MESH_CONGESTION_CONTROL_NOTIFICATION,
	MGMT_ACTION_MESH_MCCA_SETUP_REQUEST,
	MGMT_ACTION_MESH_MCCA_SETUP_REPLY,
	MGMT_ACTION_MESH_MCCA_ADVERTISEMENT_REQUEST,
	MGMT_ACTION_MESH_MCCA_ADVERTISEMENT,
	MGMT_ACTION_MESH_MCCA_TEARDOWN,
	MGMT_ACTION_MESH_TBTT_ADJUSTMENT_REQUEST,
	MGMT_ACTION_MESH_TBTT_ADJUSTMENT_RESPONSE,
	MGMT_ACTION_SP_MESH_PEERING_OPEN,
	MGMT_ACTION_SP_MESH_PEERING_CONFIRM,
	MGMT_ACTION_SP_MESH_PEERING_CLOSE,
	MGMT_ACTION_SP_MGK_INFORM,
	MGMT_ACTION_SP_MGK_ACK,
	MGMT_ACTION_WMM_QOS_SETUP_REQ,
	MGMT_ACTION_WMM_QOS_SETUP_RESP,
	MGMT_ACTION_WMM_QOS_TEARDOWN,
	MGMT_ACTION_VHT_COMPRESSED_BF,
	MGMT_ACTION_VHT_GID_NOTIF,
	MGMT_ACTION_VHT_OPMODE_NOTIF,
	MGMT_ACTION_GAS_INITIAL_REQUEST,
	MGMT_ACTION_GAS_INITIAL_RESPONSE,
	MGMT_ACTION_GAS_COMEBACK_REQUEST,
	MGMT_ACTION_GAS_COMEBACK_RESPONSE,
	MGMT_ACTION_FST_SETUP_REQ,
	MGMT_ACTION_FST_SETUP_RSP,
	MGMT_ACTION_FST_TEAR_DOWN,
	MGMT_ACTION_FST_ACK_REQ,
	MGMT_ACTION_FST_ACK_RSP,
	MGMT_ACTION_FST_ON_CHANNEL_TUNNEL,
	MGMT_FRAME_TYPE_ALL,
	MGMT_MAX_FRAME_TYPE,
};

#define WLAN_MGMT_TXRX_HOST_MAX_ANTENNA          4
#define WLAN_INVALID_PER_CHAIN_RSSI             0xFF
#define WLAN_INVALID_PER_CHAIN_SNR              0x80
#define WLAN_NOISE_FLOOR_DBM_DEFAULT            -96
/**
 * struct mgmt_rx_event_params - host mgmt header params
 * @channel: channel on which this frame is received
 * @snr: snr information used to call rssi
 * @rssi_ctl[WLAN_MGMT_TXRX_HOST_MAX_ANTENNA]: RSSI of PRI 20MHz for each chain
 * @rate: Rate kbps
 * @phy_mode: rx phy mode
 * @buf_len: length of the frame
 * @status: rx status
 * @flags: information about the management frame e.g. can give a
 *         scan source for a scan result mgmt frame
 * @rssi: combined RSSI, i.e. the sum of the snr + noise floor (dBm units)
 * @tsf_delta: tsf delta
 * @pdev_id: pdev id
 * @rx_params: pointer to other rx params
 *             (win specific, will be removed in phase 4)
 */
struct mgmt_rx_event_params {
	uint32_t    channel;
	uint32_t    snr;
	uint8_t     rssi_ctl[WLAN_MGMT_TXRX_HOST_MAX_ANTENNA];
	uint32_t    rate;
	enum wlan_phymode    phy_mode;
	uint32_t    buf_len;
	QDF_STATUS  status;
	uint32_t    flags;
	int32_t     rssi;
	uint32_t    tsf_delta;
	uint8_t     pdev_id;
	void        *rx_params;
};

/**
 * mgmt_tx_download_comp_cb - function pointer for tx download completions.
 * @context: caller component specific context
 * @buf: buffer
 * @free: to free/not free the buffer
 *
 * This is the function pointer to be called on tx download completion
 * if download complete is required.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
typedef QDF_STATUS (*mgmt_tx_download_comp_cb)(void *context,
					 qdf_nbuf_t buf, bool free);

/**
 * mgmt_ota_comp_cb - function pointer for tx ota completions.
 * @context: caller component specific context
 * @buf: buffer
 * @status: tx completion status
 * @tx_compl_params: tx completion params
 *
 * This is the function pointer to be called on tx ota completion.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
typedef QDF_STATUS (*mgmt_ota_comp_cb)(void *context, qdf_nbuf_t buf,
				 uint32_t status, void *tx_compl_params);

/**
 * mgmt_frame_rx_callback - function pointer for receiving mgmt rx frames
 * @psoc: psoc context
 * @peer: peer
 * @buf: buffer
 * @mgmt_rx_params: rx params
 * @frm_type: mgmt rx frame type
 *
 * This is the function pointer to be called on receiving mgmt rx frames.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
typedef QDF_STATUS (*mgmt_frame_rx_callback)(
			struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_peer *peer,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params,
			enum mgmt_frame_type frm_type);

/**
 * mgmt_frame_fill_peer_cb - Function pointer to fill peer in the buf
 * @peer: peer
 * @buf: buffer
 *
 * This is the function pointer to be called during drain to fill the
 * peer into the buf's cb structure.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
typedef QDF_STATUS (*mgmt_frame_fill_peer_cb)(
			struct wlan_objmgr_peer *peer,
			qdf_nbuf_t buf);

/**
 * struct mgmt_txrx_mgmt_frame_cb_info - frm and corresponding rx cb info
 * @frm_type:    mgmt frm type
 * @mgmt_rx_cb:  corresponding rx callback
 */
struct mgmt_txrx_mgmt_frame_cb_info {
	enum mgmt_frame_type frm_type;
	mgmt_frame_rx_callback mgmt_rx_cb;
};


/**
 * wlan_mgmt_txrx_init() - initialize mgmt txrx context.
 *
 * This function initializes the mgmt txrx context,
 * mgmt descriptor pool, etc.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_init(void);

/**
 * wlan_mgmt_txrx_deinit() - deinitialize mgmt txrx context.
 *
 * This function deinitializes the mgmt txrx context,
 * mgmt descriptor pool, etc.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_deinit(void);

/**
 * wlan_mgmt_txrx_mgmt_frame_tx() - transmits mgmt. frame
 * @peer: peer
 * @context: caller component specific context
 * @buf: buffer to be transmitted
 * @comp_cb: download completion cb function
 * @ota_cb: post processing cb function
 * @comp_id: umac component id
 * @mgmt_tx_params: mgmt tx params
 *
 * This function transmits the mgmt. frame to southbound interface.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_mgmt_frame_tx(struct wlan_objmgr_peer *peer,
					void *context,
					qdf_nbuf_t buf,
					mgmt_tx_download_comp_cb tx_comp_cb,
					mgmt_ota_comp_cb tx_ota_comp_cb,
					enum wlan_umac_comp_id comp_id,
					void *mgmt_tx_params);

/**
 * wlan_mgmt_txrx_beacon_frame_tx() - transmits mgmt. beacon
 * @psoc: psoc context
 * @buf: buffer to be transmitted
 * @comp_id: umac component id
 *
 * This function transmits the mgmt. beacon to southbound interface.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_beacon_frame_tx(struct wlan_objmgr_peer *peer,
					  qdf_nbuf_t buf,
					  enum wlan_umac_comp_id comp_id);

#ifdef WLAN_SUPPORT_FILS
/**
 * wlan_mgmt_txrx_fd_action_frame_tx() - transmits mgmt. FD Action frame
 * @vdev: vdev object
 * @buf: buffer to be transmitted
 * @comp_id: umac component id
 *
 * This function transmits the FILS Dicovery Action frame to
 * southbound interface.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_fd_action_frame_tx(struct wlan_objmgr_vdev *vdev,
					     qdf_nbuf_t buf,
					     enum wlan_umac_comp_id comp_id);
#endif /* WLAN_SUPPORT_FILS */

/**
 * wlan_mgmt_txrx_register_rx_cb() - registers the rx cb for mgmt. frames
 * @psoc: psoc context
 * @comp_id: umac component id
 * @frm_cb_info: pointer to array of structure containing frm type and callback
 * @num_entries: num of frames for which cb to be registered
 *
 * This function registers rx callback for mgmt. frames for
 * the corresponding umac component passed in the func.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_register_rx_cb(
			struct wlan_objmgr_psoc *psoc,
			enum wlan_umac_comp_id comp_id,
			struct mgmt_txrx_mgmt_frame_cb_info *frm_cb_info,
			uint8_t num_entries);

/**
 * wlan_mgmt_txrx_vdev_drain() - Function to drain all mgmt packets
 * specific to a vdev
 * @vdev: vdev context
 * @mgmt_fill_peer_cb: callback func to UMAC to fill peer into buf
 * @status: opaque pointer about the status of the pkts passed to UMAC
 *
 * This function drains all mgmt packets of a vdev. This can be used in the
 * event of target going down without sending completions.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_vdev_drain(
			struct wlan_objmgr_vdev *vdev,
			mgmt_frame_fill_peer_cb mgmt_fill_peer_cb,
			void *status);

/**
 * wlan_mgmt_txrx_deregister_rx_cb() - deregisters the rx cb for mgmt. frames
 * @psoc: psoc context
 * @comp_id: umac component id
 * @frm_cb_info: pointer to array of structure containing frm type and callback
 * @num_entries: num of frames for which cb to be deregistered
 *
 * This function deregisters rx callback for mgmt. frames for
 * the corresponding umac component passed in the func.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_deregister_rx_cb(
			struct wlan_objmgr_psoc *psoc,
			enum wlan_umac_comp_id comp_id,
			struct mgmt_txrx_mgmt_frame_cb_info *frm_cb_info,
			uint8_t num_entries);

/**
 * wlan_mgmt_txrx_psoc_open() - mgmt txrx module psoc open API
 * @psoc: psoc context
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_mgmt_txrx_psoc_close() - mgmt txrx module psoc close API
 * @psoc: psoc context
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_mgmt_txrx_pdev_open() - mgmt txrx module pdev open API
 * @pdev: pdev context
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_pdev_open(struct wlan_objmgr_pdev *pdev);


/**
 * wlan_mgmt_txrx_pdev_close() - mgmt txrx module pdev close API
 * @pdev: pdev context
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_pdev_close(struct wlan_objmgr_pdev *pdev);
#endif


