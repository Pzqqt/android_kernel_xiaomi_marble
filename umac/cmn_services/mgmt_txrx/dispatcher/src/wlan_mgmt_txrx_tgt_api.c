/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
 *  DOC:    wlan_mgmt_txrx_tgt_api.c
 *  This file contains mgmt txrx public API definitions for
 *  southbound interface.
 */

#include "wlan_mgmt_txrx_tgt_api.h"
#include "wlan_mgmt_txrx_utils_api.h"
#include "../../core/src/wlan_mgmt_txrx_main_i.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_peer_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_mgmt_txrx_rx_reo_tgt_api.h"

/**
 * mgmt_get_spec_mgmt_action_subtype() - gets spec mgmt action subtype
 * @action_code: action code
 *
 * This function returns the subtype for spectrum management action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_spec_mgmt_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case ACTION_SPCT_MSR_REQ:
		frm_type = MGMT_ACTION_MEAS_REQUEST;
		break;
	case ACTION_SPCT_MSR_RPRT:
		frm_type = MGMT_ACTION_MEAS_REPORT;
		break;
	case ACTION_SPCT_TPC_REQ:
		frm_type = MGMT_ACTION_TPC_REQUEST;
		break;
	case ACTION_SPCT_TPC_RPRT:
		frm_type = MGMT_ACTION_TPC_REPORT;
		break;
	case ACTION_SPCT_CHL_SWITCH:
		frm_type = MGMT_ACTION_CHAN_SWITCH;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_qos_action_subtype() - gets qos action subtype
 * @action_code: action code
 *
 * This function returns the subtype for qos action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_qos_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case QOS_ADD_TS_REQ:
		frm_type = MGMT_ACTION_QOS_ADD_TS_REQ;
		break;
	case QOS_ADD_TS_RSP:
		frm_type = MGMT_ACTION_QOS_ADD_TS_RSP;
		break;
	case QOS_DEL_TS_REQ:
		frm_type = MGMT_ACTION_QOS_DEL_TS_REQ;
		break;
	case QOS_SCHEDULE:
		frm_type = MGMT_ACTION_QOS_SCHEDULE;
		break;
	case QOS_MAP_CONFIGURE:
		frm_type = MGMT_ACTION_QOS_MAP_CONFIGURE;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_dls_action_subtype() - gets dls action subtype
 * @action_code: action code
 *
 * This function returns the subtype for dls action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_dls_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case DLS_REQUEST:
		frm_type = MGMT_ACTION_DLS_REQUEST;
		break;
	case DLS_RESPONSE:
		frm_type = MGMT_ACTION_DLS_RESPONSE;
		break;
	case DLS_TEARDOWN:
		frm_type = MGMT_ACTION_DLS_TEARDOWN;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_back_action_subtype() - gets block ack action subtype
 * @action_code: action code
 *
 * This function returns the subtype for block ack action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_back_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case ADDBA_REQUEST:
		frm_type = MGMT_ACTION_BA_ADDBA_REQUEST;
		break;
	case ADDBA_RESPONSE:
		frm_type = MGMT_ACTION_BA_ADDBA_RESPONSE;
		break;
	case DELBA:
		frm_type = MGMT_ACTION_BA_DELBA;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_public_action_subtype() - gets public action subtype
 * @action_code: action code
 *
 * This function returns the subtype for public action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_public_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case PUB_ACTION_2040_BSS_COEXISTENCE:
		frm_type = MGMT_ACTION_2040_BSS_COEXISTENCE;
		break;
	case PUB_ACTION_EXT_CHANNEL_SWITCH_ID:
		frm_type = MGMT_ACTION_EXT_CHANNEL_SWITCH_ID;
		break;
	case PUB_ACTION_VENDOR_SPECIFIC:
		frm_type = MGMT_ACTION_VENDOR_SPECIFIC;
		break;
	case PUB_ACTION_TDLS_DISCRESP:
		frm_type = MGMT_ACTION_TDLS_DISCRESP;
		break;
	case PUB_ACTION_GAS_INITIAL_REQUEST:
		frm_type = MGMT_ACTION_GAS_INITIAL_REQUEST;
		break;
	case PUB_ACTION_GAS_INITIAL_RESPONSE:
		frm_type = MGMT_ACTION_GAS_INITIAL_RESPONSE;
		break;
	case PUB_ACTION_GAS_COMEBACK_REQUEST:
		frm_type = MGMT_ACTION_GAS_COMEBACK_REQUEST;
		break;
	case PUB_ACTION_GAS_COMEBACK_RESPONSE:
		frm_type = MGMT_ACTION_GAS_COMEBACK_RESPONSE;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_rrm_action_subtype() - gets rrm action subtype
 * @action_code: action code
 *
 * This function returns the subtype for rrm action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_rrm_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case RRM_RADIO_MEASURE_REQ:
		frm_type = MGMT_ACTION_RRM_RADIO_MEASURE_REQ;
		break;
	case RRM_RADIO_MEASURE_RPT:
		frm_type = MGMT_ACTION_RRM_RADIO_MEASURE_RPT;
		break;
	case RRM_LINK_MEASUREMENT_REQ:
		frm_type = MGMT_ACTION_RRM_LINK_MEASUREMENT_REQ;
		break;
	case RRM_LINK_MEASUREMENT_RPT:
		frm_type = MGMT_ACTION_RRM_LINK_MEASUREMENT_RPT;
		break;
	case RRM_NEIGHBOR_REQ:
		frm_type = MGMT_ACTION_RRM_NEIGHBOR_REQ;
		break;
	case RRM_NEIGHBOR_RPT:
		frm_type = MGMT_ACTION_RRM_NEIGHBOR_RPT;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

static enum mgmt_frame_type
mgmt_get_ft_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case FT_FAST_BSS_TRNST_REQ:
		frm_type = MGMT_ACTION_FT_REQUEST;
		break;
	case FT_FAST_BSS_TRNST_RES:
		frm_type = MGMT_ACTION_FT_RESPONSE;
		break;
	case FT_FAST_BSS_TRNST_CONFIRM:
		frm_type = MGMT_ACTION_FT_CONFIRM;
		break;
	case FT_FAST_BSS_TRNST_ACK:
		frm_type = MGMT_ACTION_FT_ACK;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_ht_action_subtype() - gets ht action subtype
 * @action_code: action code
 *
 * This function returns the subtype for ht action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_ht_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case HT_ACTION_NOTIFY_CHANWIDTH:
		frm_type = MGMT_ACTION_HT_NOTIFY_CHANWIDTH;
		break;
	case HT_ACTION_SMPS:
		frm_type = MGMT_ACTION_HT_SMPS;
		break;
	case HT_ACTION_PSMP:
		frm_type = MGMT_ACTION_HT_PSMP;
		break;
	case HT_ACTION_PCO_PHASE:
		frm_type = MGMT_ACTION_HT_PCO_PHASE;
		break;
	case HT_ACTION_CSI:
		frm_type = MGMT_ACTION_HT_CSI;
		break;
	case HT_ACTION_NONCOMPRESSED_BF:
		frm_type = MGMT_ACTION_HT_NONCOMPRESSED_BF;
		break;
	case HT_ACTION_COMPRESSED_BF:
		frm_type = MGMT_ACTION_HT_COMPRESSED_BF;
		break;
	case HT_ACTION_ASEL_IDX_FEEDBACK:
		frm_type = MGMT_ACTION_HT_ASEL_IDX_FEEDBACK;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_sa_query_action_subtype() - gets sa query action subtype
 * @action_code: action code
 *
 * This function returns the subtype for sa query action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_sa_query_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case SA_QUERY_REQUEST:
		frm_type = MGMT_ACTION_SA_QUERY_REQUEST;
		break;
	case SA_QUERY_RESPONSE:
		frm_type = MGMT_ACTION_SA_QUERY_RESPONSE;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_pdpa_action_subtype() - gets pdpa action subtype
 * @action_code: action code
 *
 * This function returns the subtype for protected dual public
 * action category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_pdpa_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case PDPA_GAS_INIT_REQ:
		frm_type = MGMT_ACTION_PDPA_GAS_INIT_REQ;
		break;
	case PDPA_GAS_INIT_RSP:
		frm_type = MGMT_ACTION_PDPA_GAS_INIT_RSP;
		break;
	case PDPA_GAS_COMEBACK_REQ:
		frm_type = MGMT_ACTION_PDPA_GAS_COMEBACK_REQ;
		break;
	case PDPA_GAS_COMEBACK_RSP:
		frm_type = MGMT_ACTION_PDPA_GAS_COMEBACK_RSP;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_wnm_action_subtype() - gets wnm action subtype
 * @action_code: action code
 *
 * This function returns the subtype for wnm action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_wnm_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case WNM_BSS_TM_QUERY:
		frm_type = MGMT_ACTION_WNM_BSS_TM_QUERY;
		break;
	case WNM_BSS_TM_REQUEST:
		frm_type = MGMT_ACTION_WNM_BSS_TM_REQUEST;
		break;
	case WNM_BSS_TM_RESPONSE:
		frm_type = MGMT_ACTION_WNM_BSS_TM_RESPONSE;
		break;
	case WNM_NOTIF_REQUEST:
		frm_type = MGMT_ACTION_WNM_NOTIF_REQUEST;
		break;
	case WNM_NOTIF_RESPONSE:
		frm_type = MGMT_ACTION_WNM_NOTIF_RESPONSE;
		break;
	case WNM_FMS_REQ:
		frm_type = MGMT_ACTION_WNM_FMS_REQ;
		break;
	case WNM_FMS_RESP:
		frm_type = MGMT_ACTION_WNM_FMS_RESP;
		break;
	case WNM_TFS_REQ:
		frm_type = MGMT_ACTION_WNM_TFS_REQ;
		break;
	case WNM_TFS_RESP:
		frm_type = MGMT_ACTION_WNM_TFS_RESP;
		break;
	case WNM_TFS_NOTIFY:
		frm_type = MGMT_ACTION_WNM_TFS_NOTIFY;
		break;
	case WNM_SLEEP_REQ:
		frm_type = MGMT_ACTION_WNM_SLEEP_REQ;
		break;
	case WNM_SLEEP_RESP:
		frm_type = MGMT_ACTION_WNM_SLEEP_RESP;
		break;
	case WNM_TIM_REQ:
		frm_type = MGMT_ACTION_WNM_TFS_REQ;
		break;
	case WNM_TIM_RESP:
		frm_type = MGMT_ACTION_WNM_TFS_RESP;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_wnm_action_subtype() - gets tdls action subtype
 * @action_code: action code
 *
 * This function returns the subtype for tdls action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_tdls_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case TDLS_SETUP_REQUEST:
		frm_type = MGMT_ACTION_TDLS_SETUP_REQ;
		break;
	case TDLS_SETUP_RESPONSE:
		frm_type = MGMT_ACTION_TDLS_SETUP_RSP;
		break;
	case TDLS_SETUP_CONFIRM:
		frm_type = MGMT_ACTION_TDLS_SETUP_CNF;
		break;
	case TDLS_TEARDOWN:
		frm_type = MGMT_ACTION_TDLS_TEARDOWN;
		break;
	case TDLS_PEER_TRAFFIC_INDICATION:
		frm_type = MGMT_ACTION_TDLS_PEER_TRAFFIC_IND;
		break;
	case TDLS_CHANNEL_SWITCH_REQUEST:
		frm_type = MGMT_ACTION_TDLS_CH_SWITCH_REQ;
		break;
	case TDLS_CHANNEL_SWITCH_RESPONSE:
		frm_type = MGMT_ACTION_TDLS_CH_SWITCH_RSP;
		break;
	case TDLS_PEER_PSM_REQUEST:
		frm_type = MGMT_ACTION_TDLS_PEER_PSM_REQUEST;
		break;
	case TDLS_PEER_PSM_RESPONSE:
		frm_type = MGMT_ACTION_TDLS_PEER_PSM_RESPONSE;
		break;
	case TDLS_PEER_TRAFFIC_RESPONSE:
		frm_type = MGMT_ACTION_TDLS_PEER_TRAFFIC_RSP;
		break;
	case TDLS_DISCOVERY_REQUEST:
		frm_type = MGMT_ACTION_TDLS_DIS_REQ;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_mesh_action_subtype() - gets mesh action subtype
 * @action_code: action code
 *
 * This function returns the subtype for mesh action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_mesh_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case MESH_ACTION_LINK_METRIC_REPORT:
		frm_type = MGMT_ACTION_MESH_LINK_METRIC_REPORT;
		break;
	case MESH_ACTION_HWMP_PATH_SELECTION:
		frm_type = MGMT_ACTION_MESH_HWMP_PATH_SELECTION;
		break;
	case MESH_ACTION_GATE_ANNOUNCEMENT:
		frm_type = MGMT_ACTION_MESH_GATE_ANNOUNCEMENT;
		break;
	case MESH_ACTION_CONGESTION_CONTROL_NOTIFICATION:
		frm_type = MGMT_ACTION_MESH_CONGESTION_CONTROL_NOTIFICATION;
		break;
	case MESH_ACTION_MCCA_SETUP_REQUEST:
		frm_type = MGMT_ACTION_MESH_MCCA_SETUP_REQUEST;
		break;
	case MESH_ACTION_MCCA_SETUP_REPLY:
		frm_type = MGMT_ACTION_MESH_MCCA_SETUP_REPLY;
		break;
	case MESH_ACTION_MCCA_ADVERTISEMENT_REQUEST:
		frm_type = MGMT_ACTION_MESH_MCCA_ADVERTISEMENT_REQUEST;
		break;
	case MESH_ACTION_MCCA_ADVERTISEMENT:
		frm_type = MGMT_ACTION_MESH_MCCA_ADVERTISEMENT;
		break;
	case MESH_ACTION_MCCA_TEARDOWN:
		frm_type = MGMT_ACTION_MESH_MCCA_TEARDOWN;
		break;
	case MESH_ACTION_TBTT_ADJUSTMENT_REQUEST:
		frm_type = MGMT_ACTION_MESH_TBTT_ADJUSTMENT_REQUEST;
		break;
	case MESH_ACTION_TBTT_ADJUSTMENT_RESPONSE:
		frm_type = MGMT_ACTION_MESH_TBTT_ADJUSTMENT_RESPONSE;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_self_prot_action_subtype() - gets self prot. action subtype
 * @action_code: action code
 *
 * This function returns the subtype for self protected action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_self_prot_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case SP_MESH_PEERING_OPEN:
		frm_type = MGMT_ACTION_SP_MESH_PEERING_OPEN;
		break;
	case SP_MESH_PEERING_CONFIRM:
		frm_type = MGMT_ACTION_SP_MESH_PEERING_CONFIRM;
		break;
	case SP_MESH_PEERING_CLOSE:
		frm_type = MGMT_ACTION_SP_MESH_PEERING_CLOSE;
		break;
	case SP_MGK_INFORM:
		frm_type = MGMT_ACTION_SP_MGK_INFORM;
		break;
	case SP_MGK_ACK:
		frm_type = MGMT_ACTION_SP_MGK_ACK;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_wmm_action_subtype() - gets wmm action subtype
 * @action_code: action code
 *
 * This function returns the subtype for wmm action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_wmm_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case WMM_QOS_SETUP_REQ:
		frm_type = MGMT_ACTION_WMM_QOS_SETUP_REQ;
		break;
	case WMM_QOS_SETUP_RESP:
		frm_type = MGMT_ACTION_WMM_QOS_SETUP_RESP;
		break;
	case WMM_QOS_TEARDOWN:
		frm_type = MGMT_ACTION_WMM_QOS_TEARDOWN;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_vht_action_subtype() - gets vht action subtype
 * @action_code: action code
 *
 * This function returns the subtype for vht action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_vht_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case VHT_ACTION_COMPRESSED_BF:
		frm_type = MGMT_ACTION_VHT_COMPRESSED_BF;
		break;
	case VHT_ACTION_GID_NOTIF:
		frm_type = MGMT_ACTION_VHT_GID_NOTIF;
		break;
	case VHT_ACTION_OPMODE_NOTIF:
		frm_type = MGMT_ACTION_VHT_OPMODE_NOTIF;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_fst_action_subtype() - gets fst action subtype
 * @action_code: action code
 *
 * This function returns the subtype for fst action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_fst_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case FST_SETUP_REQ:
		frm_type = MGMT_ACTION_FST_SETUP_REQ;
		break;
	case FST_SETUP_RSP:
		frm_type = MGMT_ACTION_FST_SETUP_RSP;
		break;
	case FST_TEAR_DOWN:
		frm_type = MGMT_ACTION_FST_TEAR_DOWN;
		break;
	case FST_ACK_REQ:
		frm_type = MGMT_ACTION_FST_ACK_REQ;
		break;
	case FST_ACK_RSP:
		frm_type = MGMT_ACTION_FST_ACK_RSP;
		break;
	case FST_ON_CHANNEL_TUNNEL:
		frm_type = MGMT_ACTION_FST_ON_CHANNEL_TUNNEL;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_get_rvs_action_subtype() - gets rvs action subtype
 * @action_code: action code
 *
 * This function returns the subtype for rvs action
 * category.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_get_rvs_action_subtype(uint8_t action_code)
{
	enum mgmt_frame_type frm_type;

	switch (action_code) {
	case SCS_REQ:
		frm_type = MGMT_ACTION_SCS_REQ;
		break;
	case SCS_RSP:
		frm_type = MGMT_ACTION_SCS_RSP;
		break;
	case GROUP_MEMBERSHIP_REQ:
		frm_type = MGMT_ACTION_GROUP_MEMBERSHIP_REQ;
		break;
	case GROUP_MEMBERSHIP_RSP:
		frm_type = MGMT_ACTION_GROUP_MEMBERSHIP_RSP;
		break;
	case MCSC_REQ:
		frm_type = MGMT_ACTION_MCSC_REQ;
		break;
	case MCSC_RSP:
		frm_type = MGMT_ACTION_MCSC_RSP;
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_txrx_get_action_frm_subtype() - gets action frm subtype
 * @mpdu_data_ptr: pointer to mpdu data
 *
 * This function determines the action category of the frame
 * and calls respective function to get mgmt frame type.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_txrx_get_action_frm_subtype(uint8_t *mpdu_data_ptr)
{
	struct action_frm_hdr *action_hdr =
			(struct action_frm_hdr *)mpdu_data_ptr;
	enum mgmt_frame_type frm_type;

	switch (action_hdr->action_category) {
	case ACTION_CATEGORY_SPECTRUM_MGMT:
		frm_type = mgmt_get_spec_mgmt_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_FAST_BSS_TRNST:
		frm_type = mgmt_get_ft_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_QOS:
		frm_type = mgmt_get_qos_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_DLS:
		frm_type = mgmt_get_dls_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_BACK:
		frm_type = mgmt_get_back_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_PUBLIC:
		frm_type = mgmt_get_public_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_RRM:
		frm_type = mgmt_get_rrm_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_HT:
		frm_type = mgmt_get_ht_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_SA_QUERY:
		frm_type = mgmt_get_sa_query_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_PROTECTED_DUAL_OF_PUBLIC_ACTION:
		frm_type = mgmt_get_pdpa_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_WNM:
		frm_type = mgmt_get_wnm_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_TDLS:
		frm_type = mgmt_get_tdls_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_MESH_ACTION:
		frm_type = mgmt_get_mesh_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_SELF_PROTECTED:
		frm_type = mgmt_get_self_prot_action_subtype(
						action_hdr->action_code);
		break;
	case ACTION_CATEGORY_WMM:
		frm_type = mgmt_get_wmm_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_VHT:
		frm_type = mgmt_get_vht_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_VENDOR_SPECIFIC:
		frm_type = MGMT_ACTION_CATEGORY_VENDOR_SPECIFIC;
		break;
	case ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED:
		frm_type = MGMT_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED;
		break;
	case ACTION_CATEGORY_FST:
		frm_type = mgmt_get_fst_action_subtype(action_hdr->action_code);
		break;
	case ACTION_CATEGORY_RVS:
		frm_type =
			mgmt_get_rvs_action_subtype(action_hdr->action_code);
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

/**
 * mgmt_txrx_get_frm_type() - gets mgmt frm type
 * @mgmt_subtype: mgmt subtype
 * @mpdu_data_ptr: pointer to mpdu data
 *
 * This function returns mgmt frame type of the frame
 * based on the mgmt subtype.
 *
 * Return: mgmt frame type
 */
static enum mgmt_frame_type
mgmt_txrx_get_frm_type(uint8_t mgmt_subtype, uint8_t *mpdu_data_ptr)
{
	enum mgmt_frame_type frm_type;

	switch (mgmt_subtype) {
	case MGMT_SUBTYPE_ASSOC_REQ:
		frm_type = MGMT_ASSOC_REQ;
		break;
	case MGMT_SUBTYPE_ASSOC_RESP:
		frm_type = MGMT_ASSOC_RESP;
		break;
	case MGMT_SUBTYPE_REASSOC_REQ:
		frm_type = MGMT_ASSOC_REQ;
		break;
	case MGMT_SUBTYPE_REASSOC_RESP:
		frm_type = MGMT_REASSOC_RESP;
		break;
	case MGMT_SUBTYPE_PROBE_REQ:
		frm_type = MGMT_PROBE_REQ;
		break;
	case MGMT_SUBTYPE_PROBE_RESP:
		frm_type = MGMT_PROBE_RESP;
		break;
	case MGMT_SUBTYPE_BEACON:
		frm_type = MGMT_BEACON;
		break;
	case MGMT_SUBTYPE_ATIM:
		frm_type = MGMT_ATIM;
		break;
	case MGMT_SUBTYPE_DISASSOC:
		frm_type = MGMT_DISASSOC;
		break;
	case MGMT_SUBTYPE_AUTH:
		frm_type = MGMT_AUTH;
		break;
	case MGMT_SUBTYPE_DEAUTH:
		frm_type = MGMT_DEAUTH;
		break;
	case MGMT_SUBTYPE_ACTION:
	case MGMT_SUBTYPE_ACTION_NO_ACK:
		frm_type = mgmt_txrx_get_action_frm_subtype(mpdu_data_ptr);
		break;
	default:
		frm_type = MGMT_FRM_UNSPECIFIED;
		break;
	}

	return frm_type;
}

#ifdef WLAN_IOT_SIM_SUPPORT
static QDF_STATUS simulation_frame_update(struct wlan_objmgr_psoc *psoc,
					  qdf_nbuf_t buf,
					  struct mgmt_rx_event_params *rx_param)
{
	uint8_t *addr = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	uint8_t pdevid = 0;
	wlan_objmgr_ref_dbgid dbgid;
	struct wlan_lmac_if_rx_ops *rx_ops = NULL;
	struct wlan_objmgr_pdev *pdev;
	struct ieee80211_frame *wh;
	u_int8_t *data;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (rx_ops && rx_ops->iot_sim_rx_ops.iot_sim_cmd_handler) {
		data = (uint8_t *)qdf_nbuf_data(buf);
		wh = (struct ieee80211_frame *)data;
		addr = (uint8_t *)wh->i_addr3;
		pdevid = rx_param->pdev_id;
		dbgid = WLAN_IOT_SIM_ID;
		if (qdf_is_macaddr_broadcast((struct qdf_mac_addr *)addr)) {
			pdev = wlan_objmgr_get_pdev_by_id(psoc, pdevid,
							  dbgid);
			if (pdev) {
				vdev = wlan_objmgr_pdev_get_first_vdev(pdev,
								       dbgid);
				wlan_objmgr_pdev_release_ref(pdev, dbgid);
			}
		} else
			vdev = wlan_objmgr_get_vdev_by_macaddr_from_psoc(psoc,
									 pdevid,
									 addr,
									 dbgid);
		if (vdev) {
			status = rx_ops->iot_sim_rx_ops.
					iot_sim_cmd_handler(vdev, buf,
							    NULL, false,
							    rx_param);
			if (status == QDF_STATUS_E_NULL_VALUE) {
				wlan_objmgr_vdev_release_ref(vdev, dbgid);
				mgmt_txrx_debug("iot_sim:Pkt processed at RX");
				return status;
			}
			wlan_objmgr_vdev_release_ref(vdev, dbgid);
		}
	}
	return status;
}
#else
static QDF_STATUS simulation_frame_update(struct wlan_objmgr_psoc *psoc,
					  qdf_nbuf_t buf,
					  struct mgmt_rx_event_params *rx_param)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * wlan_mgmt_txrx_rx_handler_list_copy() - copies rx handler list
 * @rx_handler: pointer to rx handler list
 * @rx_handler_head: pointer to head of the copies list
 * @rx_handler_tail: pointer to tail of the copies list
 *
 * This function copies the rx handler linked list into a local
 * linked list.
 *
 * Return: QDF_STATUS_SUCCESS in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_rx_handler_list_copy(
			struct mgmt_rx_handler *rx_handler,
			struct mgmt_rx_handler **rx_handler_head,
			struct mgmt_rx_handler **rx_handler_tail)
{
	struct mgmt_rx_handler *rx_handler_node;

	while (rx_handler) {
		rx_handler_node =
				qdf_mem_malloc_atomic(sizeof(*rx_handler_node));
		if (!rx_handler_node) {
			mgmt_txrx_err_rl("Couldn't allocate memory for rx handler node");
			return QDF_STATUS_E_NOMEM;
		}

		rx_handler_node->comp_id = rx_handler->comp_id;
		rx_handler_node->rx_cb = rx_handler->rx_cb;
		rx_handler_node->next = NULL;

		if (!(*rx_handler_head)) {
			*rx_handler_head = rx_handler_node;
			*rx_handler_tail = *rx_handler_head;
		} else {
			(*rx_handler_tail)->next = rx_handler_node;
			*rx_handler_tail = (*rx_handler_tail)->next;
		}
		rx_handler = rx_handler->next;
	}

	return QDF_STATUS_SUCCESS;
}

static bool
mgmt_rx_is_bssid_valid(struct qdf_mac_addr *mac_addr)
{
	if (qdf_is_macaddr_group(mac_addr) ||
	    qdf_is_macaddr_zero(mac_addr))
		return false;

	return true;
}

QDF_STATUS tgt_mgmt_txrx_rx_frame_handler(
			struct wlan_objmgr_psoc *psoc,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params)
{
	struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx;
	struct ieee80211_frame *wh;
	qdf_nbuf_t copy_buf;
	struct wlan_objmgr_peer *peer = NULL;
	uint8_t mgmt_type, mgmt_subtype;
	uint8_t *mac_addr, *mpdu_data_ptr;
	enum mgmt_frame_type frm_type;
	struct mgmt_rx_handler *rx_handler;
	struct mgmt_rx_handler *rx_handler_head = NULL, *rx_handler_tail = NULL;
	u_int8_t *data, *ivp = NULL;
	uint16_t buflen;
	uint16_t len = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool is_from_addr_valid, is_bssid_valid;

	if (!buf) {
		mgmt_txrx_err("buffer passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!psoc) {
		mgmt_txrx_err("psoc_ctx passed is NULL");
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_INVAL;
	}

	data = (uint8_t *)qdf_nbuf_data(buf);
	wh = (struct ieee80211_frame *)data;
	buflen = qdf_nbuf_len(buf);

	/**
	 * TO DO (calculate pdev)
	 * Waiting for a new parameter: pdev id to get added in rx event
	 */

	mgmt_type = (wh)->i_fc[0] & IEEE80211_FC0_TYPE_MASK;
	mgmt_subtype = (wh)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;

	if (mgmt_type != IEEE80211_FC0_TYPE_MGT &&
	    mgmt_type != IEEE80211_FC0_TYPE_CTL) {
		mgmt_txrx_err("Rx event doesn't contain a mgmt/ctrl packet, %d",
			      mgmt_type);
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	is_from_addr_valid = mgmt_rx_is_bssid_valid((struct qdf_mac_addr *)
							      wh->i_addr2);
	is_bssid_valid = mgmt_rx_is_bssid_valid((struct qdf_mac_addr *)
							      wh->i_addr3);

	if (!is_from_addr_valid && !is_bssid_valid) {
		mgmt_txrx_debug_rl("from addr "QDF_MAC_ADDR_FMT" bssid addr "QDF_MAC_ADDR_FMT" both not valid, dropping them",
				   QDF_MAC_ADDR_REF(wh->i_addr2),
				   QDF_MAC_ADDR_REF(wh->i_addr3));
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	if (mgmt_type == IEEE80211_FC0_TYPE_MGT &&
	    (mgmt_subtype == MGMT_SUBTYPE_BEACON ||
	     mgmt_subtype == MGMT_SUBTYPE_PROBE_RESP) &&
	    !(is_from_addr_valid && is_bssid_valid)) {
		mgmt_txrx_debug_rl("from addr "QDF_MAC_ADDR_FMT" bssid addr "QDF_MAC_ADDR_FMT" not valid, modifying them",
				   QDF_MAC_ADDR_REF(wh->i_addr2),
				   QDF_MAC_ADDR_REF(wh->i_addr3));
		if (!is_from_addr_valid)
			qdf_mem_copy(wh->i_addr2, wh->i_addr3,
				     QDF_MAC_ADDR_SIZE);
		else
			qdf_mem_copy(wh->i_addr3, wh->i_addr2,
				     QDF_MAC_ADDR_SIZE);
	}

	/* mpdu_data_ptr is pointer to action header */
	mpdu_data_ptr = (uint8_t *)qdf_nbuf_data(buf) +
			sizeof(struct ieee80211_frame);

	if (wh->i_fc[1] & IEEE80211_FC1_ORDER) {
		/* Adjust the offset taking into consideration HT control field
		 * length, in the case when peer sends a frame with HT/VHT/HE
		 * ctrl field in the header(when frame is transmitted in TB
		 * PPDU format).
		 */
		mpdu_data_ptr += IEEE80211_HT_CTRL_LEN;
		len = IEEE80211_HT_CTRL_LEN;
		mgmt_txrx_debug_rl("HT control field present!");
	}

	if ((wh->i_fc[1] & IEEE80211_FC1_WEP) &&
	    !qdf_is_macaddr_group((struct qdf_mac_addr *)wh->i_addr1) &&
	    !qdf_is_macaddr_broadcast((struct qdf_mac_addr *)wh->i_addr1)) {

		if (buflen > (sizeof(struct ieee80211_frame) +
			WLAN_HDR_EXT_IV_LEN))
			ivp = data + sizeof(struct ieee80211_frame) + len;

		/* Set mpdu_data_ptr based on EXT IV bit
		 * if EXT IV bit set, CCMP using PMF 8 bytes of IV is present
		 * else for WEP using PMF, 4 bytes of IV is present
		 */
		if (ivp && (ivp[WLAN_HDR_IV_LEN] & WLAN_HDR_EXT_IV_BIT)) {
			if (buflen <= (sizeof(struct ieee80211_frame)
					+ IEEE80211_CCMP_HEADERLEN)) {
				qdf_nbuf_free(buf);
				return QDF_STATUS_E_FAILURE;
			}
			mpdu_data_ptr += IEEE80211_CCMP_HEADERLEN;
		} else {
			if (buflen <= (sizeof(struct ieee80211_frame)
					+ WLAN_HDR_EXT_IV_LEN)) {
				qdf_nbuf_free(buf);
				return QDF_STATUS_E_FAILURE;
			}
			mpdu_data_ptr += WLAN_HDR_EXT_IV_LEN;
		}
	}

	if (mgmt_type == IEEE80211_FC0_TYPE_MGT) {
		frm_type = mgmt_txrx_get_frm_type(mgmt_subtype, mpdu_data_ptr);
		if (frm_type == MGMT_FRM_UNSPECIFIED) {
			mgmt_txrx_debug_rl(
			"Unspecified mgmt frame type fc: %x %x", wh->i_fc[0],
								wh->i_fc[1]);
			qdf_nbuf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		frm_type = MGMT_CTRL_FRAME;
	}

	if (mgmt_type == IEEE80211_FC0_TYPE_MGT &&
	    !(mgmt_subtype == MGMT_SUBTYPE_BEACON ||
	      mgmt_subtype == MGMT_SUBTYPE_PROBE_RESP ||
	      mgmt_subtype == MGMT_SUBTYPE_PROBE_REQ))
		mgmt_txrx_debug("Rcvd mgmt frame subtype %x (frame type %u) from "QDF_MAC_ADDR_FMT", seq_num = %d, rssi = %d tsf_delta: %u",
				mgmt_subtype, frm_type,
				QDF_MAC_ADDR_REF(wh->i_addr2),
				(le16toh(*(uint16_t *)wh->i_seq) >>
				WLAN_SEQ_SEQ_SHIFT), mgmt_rx_params->rssi,
				mgmt_rx_params->tsf_delta);

	if (simulation_frame_update(psoc, buf, mgmt_rx_params))
		return QDF_STATUS_E_FAILURE;

	mgmt_txrx_psoc_ctx = (struct mgmt_txrx_priv_psoc_context *)
			wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX);

	qdf_spin_lock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
	rx_handler = mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[frm_type];
	if (rx_handler) {
		status = wlan_mgmt_txrx_rx_handler_list_copy(rx_handler,
				&rx_handler_head, &rx_handler_tail);
		if (status != QDF_STATUS_SUCCESS) {
			qdf_spin_unlock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
			qdf_nbuf_free(buf);
			goto rx_handler_mem_free;
		}
	}

	if (mgmt_type == IEEE80211_FC0_TYPE_MGT) {
		rx_handler =
		mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[MGMT_FRAME_TYPE_ALL];
		if (rx_handler) {
			status = wlan_mgmt_txrx_rx_handler_list_copy(
				rx_handler, &rx_handler_head, &rx_handler_tail);
			if (status != QDF_STATUS_SUCCESS) {
				qdf_spin_unlock_bh(
				  &mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
				qdf_nbuf_free(buf);
				goto rx_handler_mem_free;
			}
		}
	}

	if (!rx_handler_head) {
		qdf_spin_unlock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
		mgmt_txrx_debug("No rx callback registered for frm_type: %d",
				frm_type);
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_spin_unlock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);

	mac_addr = (uint8_t *)wh->i_addr2;
	/*
	 * peer can be NULL in following 2 scenarios:
	 * 1. broadcast frame received
	 * 2. operating in monitor mode
	 *
	 * and in both scenarios, the receiver of frame
	 * is expected to do processing accordingly considerng
	 * the fact that peer = NULL can be received and is a valid
	 * scenario.
	 */
	peer = wlan_objmgr_get_peer(psoc, mgmt_rx_params->pdev_id,
				    mac_addr, WLAN_MGMT_SB_ID);
	if (!peer && !qdf_is_macaddr_broadcast(
	    (struct qdf_mac_addr *)wh->i_addr1)) {
		mac_addr = (uint8_t *)wh->i_addr1;
		peer = wlan_objmgr_get_peer(psoc,
					    mgmt_rx_params->pdev_id,
					    mac_addr, WLAN_MGMT_SB_ID);
	}

	rx_handler = rx_handler_head;
	while (rx_handler->next) {
		copy_buf = qdf_nbuf_clone(buf);

		if (!copy_buf) {
			rx_handler = rx_handler->next;
			continue;
		}

		rx_handler->rx_cb(psoc, peer, copy_buf,
					mgmt_rx_params, frm_type);
		rx_handler = rx_handler->next;
	}
	rx_handler->rx_cb(psoc, peer, buf,
				mgmt_rx_params, frm_type);

	if (peer)
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_SB_ID);

rx_handler_mem_free:
	while (rx_handler_head) {
		rx_handler = rx_handler_head;
		rx_handler_head = rx_handler_head->next;
		qdf_mem_free(rx_handler);
	}

	return status;
}

QDF_STATUS tgt_mgmt_txrx_tx_completion_handler(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id, uint32_t status,
			void *tx_compl_params)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	void *cb_context;
	mgmt_tx_download_comp_cb tx_compl_cb;
	mgmt_ota_comp_cb  ota_comp_cb;
	qdf_nbuf_t nbuf;

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Mgmt txrx context empty for pdev %pK", pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (desc_id >= MGMT_DESC_POOL_MAX) {
		mgmt_txrx_err("desc_id:%u is out of bounds", desc_id);
		return QDF_STATUS_E_INVAL;
	}
	mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[desc_id];
	if (!mgmt_desc || !mgmt_desc->in_use) {
		mgmt_txrx_err("Mgmt desc empty for id %d pdev %pK ",
				desc_id, pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}
	tx_compl_cb = mgmt_desc->tx_dwnld_cmpl_cb;
	ota_comp_cb = mgmt_desc->tx_ota_cmpl_cb;
	nbuf = mgmt_desc->nbuf;

	/*
	 *      TO DO
	 * Make the API more generic to handle tx download completion as well
	 * as OTA completion separately.
	 */

	/*
	 * 1. If the tx frame is sent by any UMAC converged component then it
	 *    passes the context as NULL while calling mgmt txrx API for
	 *    sending mgmt frame. If context is NULL, peer will be passed as
	 *    cb_context in completion callbacks.
	 * 2. If the tx frame is sent by legacy MLME then it passes the context
	 *    as its specific context (for ex- mac context in case of MCL) while
	 *    calling mgmt txrx API for sending mgmt frame. This caller specific
	 *    context is passed as cb_context in completion callbacks.
	 */
	if (mgmt_desc->context)
		cb_context = mgmt_desc->context;
	else
		cb_context = (void *)mgmt_desc->peer;

	if (!tx_compl_cb && !ota_comp_cb) {
		qdf_nbuf_free(nbuf);
		goto no_registered_cb;
	}

	if (tx_compl_cb)
		tx_compl_cb(cb_context, nbuf, status);

	if (ota_comp_cb)
		ota_comp_cb(cb_context, nbuf, status, tx_compl_params);

no_registered_cb:
	/*
	 * decrementing the peer ref count that was incremented while
	 * accessing peer in wlan_mgmt_txrx_mgmt_frame_tx
	 */
	wlan_objmgr_peer_release_ref(mgmt_desc->peer, WLAN_MGMT_NB_ID);
	wlan_mgmt_txrx_desc_put(mgmt_txrx_pdev_ctx, desc_id);
	return QDF_STATUS_SUCCESS;
}

qdf_nbuf_t tgt_mgmt_txrx_get_nbuf_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	qdf_nbuf_t buf;

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Mgmt txrx context empty for pdev %pK", pdev);
		goto fail;
	}
	if (desc_id >= MGMT_DESC_POOL_MAX) {
		mgmt_txrx_err("desc_id:%u is out of bounds", desc_id);
		goto fail;
	}

	mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[desc_id];
	if (!mgmt_desc || !mgmt_desc->in_use) {
		mgmt_txrx_err("Mgmt descriptor unavailable for id %d pdev %pK",
				desc_id, pdev);
		goto fail;
	}
	buf = mgmt_desc->nbuf;
	return buf;

fail:
	return NULL;
}

struct wlan_objmgr_peer *
tgt_mgmt_txrx_get_peer_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	struct wlan_objmgr_peer *peer;

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Mgmt txrx context empty for pdev %pK", pdev);
		goto fail;
	}

	mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[desc_id];
	if (!mgmt_desc || !mgmt_desc->in_use) {
		mgmt_txrx_err("Mgmt descriptor unavailable for id %d pdev %pK",
				desc_id, pdev);
		goto fail;
	}

	peer = mgmt_desc->peer;
	return peer;

fail:
	return NULL;
}

uint8_t tgt_mgmt_txrx_get_vdev_id_from_desc_id(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	uint8_t vdev_id;

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Mgmt txrx context empty for pdev %pK", pdev);
		goto fail;
	}
	if (desc_id >= MGMT_DESC_POOL_MAX) {
		mgmt_txrx_err("desc_id:%u is out of bounds", desc_id);
		goto fail;
	}

	mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[desc_id];
	if (!mgmt_desc || !mgmt_desc->in_use) {
		mgmt_txrx_err("Mgmt descriptor unavailable for id %d pdev %pK",
				desc_id, pdev);
		goto fail;
	}

	vdev_id = mgmt_desc->vdev_id;
	return vdev_id;

fail:
	return WLAN_UMAC_VDEV_ID_MAX;
}

uint32_t tgt_mgmt_txrx_get_free_desc_pool_count(
			struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	uint32_t free_desc_count = WLAN_INVALID_MGMT_DESC_COUNT;

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Mgmt txrx context empty for pdev %pK", pdev);
		goto fail;
	}

	free_desc_count = qdf_list_size(
		&(mgmt_txrx_pdev_ctx->mgmt_desc_pool.free_list));

fail:
	return free_desc_count;
}

QDF_STATUS
tgt_mgmt_txrx_register_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops;

	mgmt_txrx_tx_ops = wlan_psoc_get_mgmt_txrx_txops(psoc);
	if (!mgmt_txrx_tx_ops) {
		mgmt_txrx_err("txops is null for mgmt txrx module");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (mgmt_txrx_tx_ops->reg_ev_handler)
		return mgmt_txrx_tx_ops->reg_ev_handler(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_mgmt_txrx_unregister_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops;

	mgmt_txrx_tx_ops = wlan_psoc_get_mgmt_txrx_txops(psoc);
	if (!mgmt_txrx_tx_ops) {
		mgmt_txrx_err("txops is null for mgmt txrx module");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (mgmt_txrx_tx_ops->unreg_ev_handler)
		return mgmt_txrx_tx_ops->unreg_ev_handler(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_mgmt_txrx_process_rx_frame(
			struct wlan_objmgr_pdev *pdev,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops;

	mgmt_txrx_tx_ops = wlan_pdev_get_mgmt_txrx_txops(pdev);
	if (!mgmt_txrx_tx_ops) {
		mgmt_txrx_err("txops is null for mgmt txrx module");
		qdf_nbuf_free(buf);
		free_mgmt_rx_event_params(mgmt_rx_params);
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Call the legacy handler to actually process and deliver frames */
	status = mgmt_txrx_tx_ops->rx_frame_legacy_handler(pdev, buf,
							   mgmt_rx_params);
	/**
	 * Free up the mgmt rx params.
	 * nbuf shouldn't be freed here as it is taken care by
	 * rx_frame_legacy_handler.
	 */
	free_mgmt_rx_event_params(mgmt_rx_params);

	return status;
}

QDF_STATUS tgt_mgmt_txrx_rx_frame_entry(
			struct wlan_objmgr_pdev *pdev,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params)
{
	/* Call the MGMT Rx REO handler */
	return tgt_mgmt_rx_reo_frame_handler(pdev, buf, mgmt_rx_params);
}
