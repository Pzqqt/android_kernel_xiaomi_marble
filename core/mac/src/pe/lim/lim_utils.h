/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * This file lim_utils.h contains the utility definitions
 * LIM uses.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef __LIM_UTILS_H
#define __LIM_UTILS_H

#include "sir_api.h"
#include "sir_debug.h"
#include "cfg_api.h"

#include "lim_types.h"
#include "lim_scan_result_utils.h"
#include "lim_timer_utils.h"
#include "lim_trace.h"
typedef enum {
	ONE_BYTE = 1,
	TWO_BYTE = 2
} eSizeOfLenField;

#define LIM_STA_ID_MASK                        0x00FF
#define LIM_AID_MASK                              0xC000
#define LIM_SPECTRUM_MANAGEMENT_BIT_MASK          0x0100
#define LIM_RRM_BIT_MASK                          0x1000
#define LIM_SHORT_PREAMBLE_BIT_MASK               0x0020
#define LIM_IMMEDIATE_BLOCK_ACK_MASK              0x8000
#define LIM_MAX_REASSOC_RETRY_LIMIT            2

/* classifier ID is coded as 0-3: tsid, 4-5:direction */
#define LIM_MAKE_CLSID(tsid, dir) (((tsid) & 0x0F) | (((dir) & 0x03) << 4))

#define VHT_MCS_3x3_MASK    0x30
#define VHT_MCS_2x2_MASK    0x0C

#define CENTER_FREQ_DIFF_160MHz 8
#define CENTER_FREQ_DIFF_80P80MHz 16

#define IS_VHT_NSS_1x1(__mcs_map)	((__mcs_map & 0xFFFC) == 0xFFFC)

#define MGMT_RX_PACKETS_THRESHOLD 200

#ifdef WLAN_FEATURE_11W
typedef union uPmfSaQueryTimerId {
	struct {
		uint8_t sessionId;
		uint16_t peerIdx;
	} fields;
	uint32_t value;
} tPmfSaQueryTimerId, *tpPmfSaQueryTimerId;
#endif

typedef struct last_processed_frame {
	tSirMacAddr sa;
	uint16_t seq_num;
} last_processed_msg;

/* LIM utility functions */
bool lim_is_valid_frame(last_processed_msg *last_processed_frm,
		uint8_t *pRxPacketInfo);
void lim_update_last_processed_frame(last_processed_msg *last_processed_frm,
		uint8_t *pRxPacketInfo);
void limGetBssidFromPkt(tpAniSirGlobal, uint8_t *, uint8_t *, uint32_t *);
char *lim_dot11_reason_str(uint16_t reasonCode);
char *lim_mlm_state_str(tLimMlmStates state);
char *lim_sme_state_str(tLimSmeStates state);
char *lim_msg_str(uint32_t msgType);
char *lim_result_code_str(tSirResultCodes resultCode);
char *lim_dot11_mode_str(tpAniSirGlobal pMac, uint8_t dot11Mode);
void lim_print_mlm_state(tpAniSirGlobal pMac, uint16_t logLevel,
		tLimMlmStates state);
void lim_print_sme_state(tpAniSirGlobal pMac, uint16_t logLevel,
		tLimSmeStates state);
void lim_print_msg_name(tpAniSirGlobal pMac, uint16_t logLevel, uint32_t msgType);

extern tSirRetStatus lim_send_set_max_tx_power_req(tpAniSirGlobal pMac,
		int8_t txPower,
		tpPESession pSessionEntry);
extern uint8_t lim_get_max_tx_power(int8_t regMax, int8_t apTxPower,
		uint8_t iniTxPower);
uint8_t lim_is_addr_bc(tSirMacAddr);
uint8_t lim_is_group_addr(tSirMacAddr);

/* check for type of scan allowed */
uint8_t lim_active_scan_allowed(tpAniSirGlobal, uint8_t);

/* AID pool management functions */
void lim_init_peer_idxpool(tpAniSirGlobal, tpPESession);
uint16_t lim_assign_peer_idx(tpAniSirGlobal, tpPESession);

void lim_enable_overlap11g_protection(tpAniSirGlobal pMac,
		tpUpdateBeaconParams pBeaconParams,
		tpSirMacMgmtHdr pMh,
		tpPESession psessionEntry);
void lim_update_overlap_sta_param(tpAniSirGlobal pMac, tSirMacAddr bssId,
		tpLimProtStaParams pStaParams);
void lim_update_short_preamble(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);
void lim_update_short_slot_time(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);

/*
 * The below 'product' check tobe removed if 'Association' is
 * allowed in IBSS.
 */
void lim_release_peer_idx(tpAniSirGlobal, uint16_t, tpPESession);

void lim_decide_ap_protection(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
		tpUpdateBeaconParams pBeaconParams, tpPESession);
void lim_decide_ap_protection_on_delete(tpAniSirGlobal pMac,
		tpDphHashNode pStaDs,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);

extern tSirRetStatus lim_update_11a_protection(tpAniSirGlobal pMac,
		uint8_t enable,
		uint8_t overlap,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession);
extern tSirRetStatus lim_enable11g_protection(tpAniSirGlobal pMac,
		uint8_t enable,
		uint8_t overlap,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);
extern tSirRetStatus lim_enable_ht_protection_from11g(tpAniSirGlobal pMac,
		uint8_t enable,
		uint8_t overlap,
		tpUpdateBeaconParams
		pBeaconParams,
		tpPESession psessionEntry);
extern tSirRetStatus lim_enable_ht20_protection(tpAniSirGlobal pMac,
		uint8_t enable, uint8_t overlap,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession sessionEntry);
extern tSirRetStatus lim_enable_ht_non_gf_protection(tpAniSirGlobal pMac,
		uint8_t enable, uint8_t overlap,
		tpUpdateBeaconParams
		pBeaconParams, tpPESession);
extern tSirRetStatus lim_enable_ht_rifs_protection(tpAniSirGlobal pMac,
		uint8_t enable, uint8_t overlap,
		tpUpdateBeaconParams
		pBeaconParams,
		tpPESession psessionEntry);
extern tSirRetStatus lim_enable_ht_lsig_txop_protection(tpAniSirGlobal pMac,
		uint8_t enable,
		uint8_t overlap,
		tpUpdateBeaconParams
		pBeaconParams, tpPESession);
extern tSirRetStatus lim_enable_short_preamble(tpAniSirGlobal pMac,
		uint8_t enable,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);
extern tSirRetStatus lim_enable_ht_obss_protection(tpAniSirGlobal pMac,
		uint8_t enable, uint8_t overlap,
		tpUpdateBeaconParams
		pBeaconParams, tpPESession);
void lim_decide_sta_protection(tpAniSirGlobal pMac,
		tpSchBeaconStruct pBeaconStruct,
		tpUpdateBeaconParams pBeaconParams,
		tpPESession psessionEntry);
void lim_decide_sta_protection_on_assoc(tpAniSirGlobal pMac,
		tpSchBeaconStruct pBeaconStruct,
		tpPESession psessionEntry);
void lim_update_sta_run_time_ht_switch_chnl_params(tpAniSirGlobal pMac,
		tDot11fIEHTInfo *pHTInfo,
		uint8_t bssIdx,
		tpPESession psessionEntry);
/* Print MAC address utility function */
void lim_print_mac_addr(tpAniSirGlobal, tSirMacAddr, uint8_t);

/* Deferred Message Queue read/write */
uint8_t lim_write_deferred_msg_q(tpAniSirGlobal pMac,
				 struct scheduler_msg *limMsg);
struct scheduler_msg *lim_read_deferred_msg_q(tpAniSirGlobal pMac);
void lim_handle_defer_msg_error(tpAniSirGlobal pMac,
				struct scheduler_msg *pLimMsg);

/* Deferred Message Queue Reset */
void lim_reset_deferred_msg_q(tpAniSirGlobal pMac);

tSirRetStatus lim_sys_process_mmh_msg_api(tpAniSirGlobal,
					  struct scheduler_msg *, uint8_t);

void lim_handle_update_olbc_cache(tpAniSirGlobal pMac);

uint8_t lim_is_null_ssid(tSirMacSSid *pSsid);

/* 11h Support */
void lim_stop_tx_and_switch_channel(tpAniSirGlobal pMac, uint8_t sessionId);
void lim_process_channel_switch_timeout(tpAniSirGlobal);
tSirRetStatus lim_start_channel_switch(tpAniSirGlobal pMac,
		tpPESession psessionEntry);
void lim_update_channel_switch(tpAniSirGlobal, tpSirProbeRespBeacon,
		tpPESession psessionEntry);
void lim_process_quiet_timeout(tpAniSirGlobal);
void lim_process_quiet_bss_timeout(tpAniSirGlobal);

void lim_start_quiet_timer(tpAniSirGlobal pMac, uint8_t sessionId);
void lim_switch_primary_channel(tpAniSirGlobal, uint8_t, tpPESession);
void lim_switch_primary_secondary_channel(tpAniSirGlobal pMac,
					tpPESession psessionEntry,
					uint8_t newChannel,
					uint8_t ch_center_freq_seg0,
					uint8_t ch_center_freq_seg1,
					enum phy_ch_width ch_width);
void limUpdateStaRunTimeHTSwtichChnlParams(tpAniSirGlobal pMac,
		tDot11fIEHTInfo *pRcvdHTInfo,
		uint8_t bssIdx);
void lim_update_sta_run_time_ht_capability(tpAniSirGlobal pMac,
		tDot11fIEHTCaps *pHTCaps);
void lim_update_sta_run_time_ht_info(struct sAniSirGlobal *pMac,
		tDot11fIEHTInfo *pRcvdHTInfo,
		tpPESession psessionEntry);
void lim_cancel_dot11h_channel_switch(tpAniSirGlobal pMac,
		tpPESession psessionEntry);
void lim_cancel_dot11h_quiet(tpAniSirGlobal pMac, tpPESession psessionEntry);
tAniBool lim_is_channel_valid_for_channel_switch(tpAniSirGlobal pMac,
		uint8_t channel);
void lim_frame_transmission_control(tpAniSirGlobal pMac, tLimQuietTxMode type,
		tLimControlTx mode);
tSirRetStatus lim_restore_pre_channel_switch_state(tpAniSirGlobal pMac,
		tpPESession psessionEntry);
tSirRetStatus lim_restore_pre_quiet_state(tpAniSirGlobal pMac,
		tpPESession psessionEntry);

void lim_prepare_for11h_channel_switch(tpAniSirGlobal pMac,
		tpPESession psessionEntry);
void lim_switch_channel_cback(tpAniSirGlobal pMac, QDF_STATUS status,
		uint32_t *data, tpPESession psessionEntry);

static inline tSirRFBand lim_get_rf_band(uint8_t channel)
{
	if ((channel >= SIR_11A_CHANNEL_BEGIN) &&
			(channel <= SIR_11A_CHANNEL_END))
		return SIR_BAND_5_GHZ;

	if ((channel >= SIR_11B_CHANNEL_BEGIN) &&
			(channel <= SIR_11B_CHANNEL_END))
		return SIR_BAND_2_4_GHZ;

	return SIR_BAND_UNKNOWN;
}

static inline tSirRetStatus
lim_get_mgmt_staid(tpAniSirGlobal pMac, uint16_t *staid,
		tpPESession psessionEntry)
{
	if (LIM_IS_AP_ROLE(psessionEntry))
		*staid = 1;
	else if (LIM_IS_STA_ROLE(psessionEntry))
		*staid = 0;
	else
		return eSIR_FAILURE;

	return eSIR_SUCCESS;
}

static inline uint8_t lim_is_system_in_set_mimops_state(tpAniSirGlobal pMac)
{
	if (pMac->lim.gLimMlmState == eLIM_MLM_WT_SET_MIMOPS_STATE)
		return true;
	return false;
}

static inline uint8_t
is_entering_mimo_ps(tSirMacHTMIMOPowerSaveState curState,
		tSirMacHTMIMOPowerSaveState newState)
{
	if (curState == eSIR_HT_MIMO_PS_NO_LIMIT &&
			(newState == eSIR_HT_MIMO_PS_DYNAMIC
			 || newState == eSIR_HT_MIMO_PS_STATIC))
		return true;
	return false;
}

static inline int lim_select_cb_mode(tDphHashNode *pStaDs,
		tpPESession psessionEntry, uint8_t channel,
		uint8_t chan_bw)
{
	if (pStaDs->mlmStaContext.vhtCapability && chan_bw) {
		if (channel == 36 || channel == 52 || channel == 100 ||
				channel == 116 || channel == 149) {
			return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW - 1;
		} else if (channel == 40 || channel == 56 || channel == 104 ||
				channel == 120 || channel == 153) {
			return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW - 1;
		} else if (channel == 44 || channel == 60 || channel == 108 ||
				channel == 124 || channel == 157) {
			return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH - 1;
		} else if (channel == 48 || channel == 64 || channel == 112 ||
				channel == 128 || channel == 161) {
			return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH - 1;
		} else if (channel == 165) {
			return PHY_SINGLE_CHANNEL_CENTERED;
		}
	} else if (pStaDs->mlmStaContext.htCapability) {
		if (channel == 40 || channel == 48 || channel == 56 ||
			channel == 64 || channel == 104 || channel == 112 ||
			channel == 120 || channel == 128 || channel == 136 ||
			channel == 144 || channel == 153 || channel == 161) {
			return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
		} else if (channel == 36 || channel == 44 || channel == 52 ||
				channel == 60 || channel == 100 ||
				channel == 108 || channel == 116 ||
				channel == 124 || channel == 132 ||
				channel == 140 || channel == 149 ||
				channel == 157) {
			return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
		} else if (channel == 165) {
			return PHY_SINGLE_CHANNEL_CENTERED;
		}
	}
	return PHY_SINGLE_CHANNEL_CENTERED;
}

/* ANI peer station count management and associated actions */
void lim_util_count_sta_add(tpAniSirGlobal pMac, tpDphHashNode pSta,
		tpPESession psessionEntry);
void lim_util_count_sta_del(tpAniSirGlobal pMac, tpDphHashNode pSta,
		tpPESession psessionEntry);

uint8_t lim_get_ht_capability(tpAniSirGlobal, uint32_t, tpPESession);
QDF_STATUS lim_tx_complete(tHalHandle hHal, qdf_nbuf_t buf, bool free);

/**
 * This function will be registered with HAL for callback when TSPEC inactivity
 * timer fires.
 */

void lim_process_del_ts_ind(tpAniSirGlobal pMac, struct scheduler_msg *limMsg);
tSirRetStatus lim_process_hal_ind_messages(tpAniSirGlobal pMac, uint32_t mesgId,
		void *mesgParam);
tSirRetStatus lim_validate_delts_req(tpAniSirGlobal pMac,
		tpSirDeltsReq pDeltsReq,
		tSirMacAddr peerMacAddr,
		tpPESession psessionEntry);

/* callback function registration to HAL for any indication. */
void lim_register_hal_ind_call_back(tpAniSirGlobal pMac);
void lim_pkt_free(tpAniSirGlobal pMac,
		eFrameType frmType, uint8_t *pBD, void *body);

void lim_get_b_dfrom_rx_packet(tpAniSirGlobal pMac, void *body, uint32_t **pBD);

/**
 * utils_power_xy() - calc result of base raised to power
 * @base: Base value
 * @power: Base raised to this Power value
 *
 * Given a base(X) and power(Y), this API will return
 * the result of base raised to power - (X ^ Y)
 *
 * Return: Result of X^Y
 *
 */
static inline uint32_t utils_power_xy(uint16_t base, uint16_t power)
{
	uint32_t result = 1, i;

	for (i = 0; i < power; i++)
		result *= base;

	return result;
}

tSirRetStatus lim_post_sm_state_update(tpAniSirGlobal pMac,
		uint16_t StaIdx,
		tSirMacHTMIMOPowerSaveState MIMOPSState,
		uint8_t *pPeerStaMac, uint8_t sessionId);

void lim_delete_sta_context(tpAniSirGlobal pMac, struct scheduler_msg *limMsg);
void lim_delete_dialogue_token_list(tpAniSirGlobal pMac);
void lim_resset_scan_channel_info(tpAniSirGlobal pMac);
uint8_t lim_get_channel_from_beacon(tpAniSirGlobal pMac,
		tpSchBeaconStruct pBeacon);
tSirNwType lim_get_nw_type(tpAniSirGlobal pMac, uint8_t channelNum,
		uint32_t type, tpSchBeaconStruct pBeacon);

void lim_set_tspec_uapsd_mask_per_session(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		tSirMacTSInfo *pTsInfo, uint32_t action);

void lim_handle_heart_beat_timeout_for_session(tpAniSirGlobal pMac,
		tpPESession psessionEntry);

void lim_process_add_sta_rsp(tpAniSirGlobal pMac,
			     struct scheduler_msg *pMsgQ);

void lim_update_beacon(tpAniSirGlobal pMac);

void lim_process_ap_mlm_add_sta_rsp(tpAniSirGlobal pMac,
				    struct scheduler_msg *limMsgQ,
				    tpPESession psessionEntry);
void lim_process_ap_mlm_del_bss_rsp(tpAniSirGlobal pMac,
		struct scheduler_msg *limMsgQ,
		tpPESession psessionEntry);

void lim_process_ap_mlm_del_sta_rsp(tpAniSirGlobal pMac,
		struct scheduler_msg *limMsgQ,
		tpPESession psessionEntry);

tpPESession lim_is_ibss_session_active(tpAniSirGlobal pMac);
tpPESession lim_is_ap_session_active(tpAniSirGlobal pMac);
void lim_handle_heart_beat_failure_timeout(tpAniSirGlobal pMac);

uint8_t *lim_get_ie_ptr_new(tpAniSirGlobal pMac, uint8_t *pIes, int length,
		uint8_t eid, eSizeOfLenField size_of_len_field);

#define limGetWscIEPtr(pMac, ie, ie_len) \
	cfg_get_vendor_ie_ptr_from_oui(pMac, SIR_MAC_WSC_OUI, \
			SIR_MAC_WSC_OUI_SIZE, ie, ie_len)

#define limGetP2pIEPtr(pMac, ie, ie_len) \
	cfg_get_vendor_ie_ptr_from_oui(pMac, SIR_MAC_P2P_OUI, \
			SIR_MAC_P2P_OUI_SIZE, ie, ie_len)

uint8_t lim_get_noa_attr_stream_in_mult_p2p_ies(tpAniSirGlobal pMac,
		uint8_t *noaStream, uint8_t noaLen,
		uint8_t overFlowLen);
uint8_t lim_get_noa_attr_stream(tpAniSirGlobal pMac, uint8_t *pNoaStream,
		tpPESession psessionEntry);

uint8_t lim_build_p2p_ie(tpAniSirGlobal pMac, uint8_t *ie, uint8_t *data,
		uint8_t ie_len);
bool lim_is_noa_insert_reqd(tpAniSirGlobal pMac);
bool lim_isconnected_on_dfs_channel(tpAniSirGlobal mac_ctx,
		uint8_t currentChannel);
uint8_t lim_get_current_operating_channel(tpAniSirGlobal pMac);
uint32_t lim_get_max_rate_flags(tpAniSirGlobal mac_ctx, tpDphHashNode sta_ds);

bool lim_check_vht_op_mode_change(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		uint8_t chanWidth, uint8_t staId,
		uint8_t *peerMac);
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
bool lim_send_he_ie_update(tpAniSirGlobal mac_ctx, tpPESession pe_session);
#endif
bool lim_set_nss_change(tpAniSirGlobal pMac, tpPESession psessionEntry,
		uint8_t rxNss, uint8_t staId, uint8_t *peerMac);
bool lim_check_membership_user_position(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		uint32_t membership, uint32_t userPosition,
		uint8_t staId);

#ifdef FEATURE_WLAN_DIAG_SUPPORT

typedef enum {
	WLAN_PE_DIAG_SCAN_REQ_EVENT = 0,
	WLAN_PE_DIAG_SCAN_ABORT_IND_EVENT,
	WLAN_PE_DIAG_SCAN_RSP_EVENT,
	WLAN_PE_DIAG_JOIN_REQ_EVENT,
	WLAN_PE_DIAG_JOIN_RSP_EVENT,
	WLAN_PE_DIAG_SETCONTEXT_REQ_EVENT,
	WLAN_PE_DIAG_SETCONTEXT_RSP_EVENT,
	WLAN_PE_DIAG_REASSOC_REQ_EVENT,
	WLAN_PE_DIAG_REASSOC_RSP_EVENT,
	WLAN_PE_DIAG_AUTH_REQ_EVENT,
	WLAN_PE_DIAG_AUTH_RSP_EVENT = 10,
	WLAN_PE_DIAG_DISASSOC_REQ_EVENT,
	WLAN_PE_DIAG_DISASSOC_RSP_EVENT,
	WLAN_PE_DIAG_DISASSOC_IND_EVENT,
	WLAN_PE_DIAG_DISASSOC_CNF_EVENT,
	WLAN_PE_DIAG_DEAUTH_REQ_EVENT,
	WLAN_PE_DIAG_DEAUTH_RSP_EVENT,
	WLAN_PE_DIAG_DEAUTH_IND_EVENT,
	WLAN_PE_DIAG_START_BSS_REQ_EVENT,
	WLAN_PE_DIAG_START_BSS_RSP_EVENT,
	WLAN_PE_DIAG_AUTH_IND_EVENT = 20,
	WLAN_PE_DIAG_ASSOC_IND_EVENT,
	WLAN_PE_DIAG_ASSOC_CNF_EVENT,
	WLAN_PE_DIAG_REASSOC_IND_EVENT,
	WLAN_PE_DIAG_SWITCH_CHL_IND_EVENT,
	WLAN_PE_DIAG_SWITCH_CHL_RSP_EVENT,
	WLAN_PE_DIAG_STOP_BSS_REQ_EVENT,
	WLAN_PE_DIAG_STOP_BSS_RSP_EVENT,
	WLAN_PE_DIAG_DEAUTH_CNF_EVENT,
	WLAN_PE_DIAG_ADDTS_REQ_EVENT,
	WLAN_PE_DIAG_ADDTS_RSP_EVENT = 30,
	WLAN_PE_DIAG_DELTS_REQ_EVENT ,
	WLAN_PE_DIAG_DELTS_RSP_EVENT,
	WLAN_PE_DIAG_DELTS_IND_EVENT,
	WLAN_PE_DIAG_ENTER_BMPS_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_BMPS_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_REQ_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_IND_EVENT,
	WLAN_PE_DIAG_ENTER_IMPS_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_IMPS_RSP_EVENT = 40,
	WLAN_PE_DIAG_EXIT_IMPS_REQ_EVENT,
	WLAN_PE_DIAG_EXIT_IMPS_RSP_EVENT,
	WLAN_PE_DIAG_ENTER_UAPSD_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_UAPSD_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_UAPSD_REQ_EVENT ,
	WLAN_PE_DIAG_EXIT_UAPSD_RSP_EVENT,
	WLAN_PE_DIAG_WOWL_ADD_BCAST_PTRN_EVENT,
	WLAN_PE_DIAG_WOWL_DEL_BCAST_PTRN_EVENT,
	WLAN_PE_DIAG_ENTER_WOWL_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_WOWL_RSP_EVENT = 50,
	WLAN_PE_DIAG_EXIT_WOWL_REQ_EVENT,
	WLAN_PE_DIAG_EXIT_WOWL_RSP_EVENT,
	WLAN_PE_DIAG_HAL_ADDBA_REQ_EVENT,
	WLAN_PE_DIAG_HAL_ADDBA_RSP_EVENT,
	WLAN_PE_DIAG_HAL_DELBA_IND_EVENT,
	WLAN_PE_DIAG_HB_FAILURE_TIMEOUT,
	WLAN_PE_DIAG_PRE_AUTH_REQ_EVENT,
	WLAN_PE_DIAG_PRE_AUTH_RSP_EVENT,
	WLAN_PE_DIAG_PREAUTH_DONE,
	WLAN_PE_DIAG_REASSOCIATING = 60,
	WLAN_PE_DIAG_CONNECTED,
	WLAN_PE_DIAG_ASSOC_REQ_EVENT,
	WLAN_PE_DIAG_AUTH_COMP_EVENT,
	WLAN_PE_DIAG_ASSOC_COMP_EVENT,
	WLAN_PE_DIAG_AUTH_START_EVENT,
	WLAN_PE_DIAG_ASSOC_START_EVENT,
	WLAN_PE_DIAG_REASSOC_START_EVENT,
	WLAN_PE_DIAG_ROAM_AUTH_START_EVENT,
	WLAN_PE_DIAG_ROAM_AUTH_COMP_EVENT,
	WLAN_PE_DIAG_ROAM_ASSOC_START_EVENT = 70,
	WLAN_PE_DIAG_ROAM_ASSOC_COMP_EVENT,
	WLAN_PE_DIAG_SCAN_COMPLETE_EVENT,
	WLAN_PE_DIAG_SCAN_RESULT_FOUND_EVENT,
	WLAN_PE_DIAG_ASSOC_TIMEOUT,
	WLAN_PE_DIAG_AUTH_TIMEOUT,
	WLAN_PE_DIAG_DEAUTH_FRAME_EVENT,
	WLAN_PE_DIAG_DISASSOC_FRAME_EVENT,
} WLAN_PE_DIAG_EVENT_TYPE;

void lim_diag_event_report(tpAniSirGlobal pMac, uint16_t eventType,
		tpPESession pSessionEntry, uint16_t status,
		uint16_t reasonCode);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

void pe_set_resume_channel(tpAniSirGlobal pMac, uint16_t channel,
		ePhyChanBondState cbState);

void lim_get_short_slot_from_phy_mode(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		uint32_t phyMode, uint8_t *pShortSlotEnable);

void lim_clean_up_disassoc_deauth_req(tpAniSirGlobal pMac, uint8_t *staMac,
		bool cleanRxPath);

bool lim_check_disassoc_deauth_ack_pending(tpAniSirGlobal pMac,
		uint8_t *staMac);

#ifdef WLAN_FEATURE_11W
void lim_pmf_sa_query_timer_handler(void *pMacGlobal, uint32_t param);
void lim_pmf_comeback_timer_callback(void *context);
void lim_set_protected_bit(tpAniSirGlobal pMac,
	tpPESession psessionEntry,
	tSirMacAddr peer, tpSirMacMgmtHdr pMacHdr);
#else
static inline void lim_set_protected_bit(tpAniSirGlobal pMac,
	tpPESession psessionEntry,
	tSirMacAddr peer, tpSirMacMgmtHdr pMacHdr) {}
#endif /* WLAN_FEATURE_11W */

void lim_set_ht_caps(tpAniSirGlobal p_mac,
		tpPESession p_session_ntry,
		uint8_t *p_ie_start,
		uint32_t num_bytes);

void lim_set_vht_caps(tpAniSirGlobal p_mac,
		tpPESession p_session_entry,
		uint8_t *p_ie_start,
		uint32_t num_bytes);
bool lim_validate_received_frame_a1_addr(tpAniSirGlobal mac_ctx,
		tSirMacAddr a1, tpPESession session);
void lim_set_stads_rtt_cap(tpDphHashNode sta_ds, struct s_ext_cap *ext_cap,
			   tpAniSirGlobal mac_ctx);

void lim_check_and_reset_protection_params(tpAniSirGlobal mac_ctx);

QDF_STATUS lim_send_ext_cap_ie(tpAniSirGlobal mac_ctx, uint32_t session_id,
			       tDot11fIEExtCap *extracted_extcap, bool merge);

QDF_STATUS lim_send_ies_per_band(tpAniSirGlobal mac_ctx,
				 tpPESession session, uint8_t vdev_id);

tSirRetStatus lim_strip_extcap_ie(tpAniSirGlobal mac_ctx, uint8_t *addn_ie,
			  uint16_t *addn_ielen, uint8_t *extracted_extcap);
void lim_update_extcap_struct(tpAniSirGlobal mac_ctx, uint8_t *buf,
			      tDot11fIEExtCap *ext_cap);
tSirRetStatus lim_strip_extcap_update_struct(tpAniSirGlobal mac_ctx,
		uint8_t *addn_ie, uint16_t *addn_ielen, tDot11fIEExtCap *dst);
void lim_merge_extcap_struct(tDot11fIEExtCap *dst, tDot11fIEExtCap *src,
		bool add);

/**
 * lim_strip_op_class_update_struct - strip sup op class IE and populate
 *				  the dot11f structure
 * @mac_ctx: global MAC context
 * @addn_ie: Additional IE buffer
 * @addn_ielen: Length of additional IE
 * @dst: Supp operating class IE structure to be updated
 *
 * This function is used to strip supp op class IE from IE buffer and
 * update the passed structure.
 *
 * Return: tSirRetStatus
 */
tSirRetStatus lim_strip_supp_op_class_update_struct(tpAniSirGlobal mac_ctx,
		uint8_t *addn_ie, uint16_t *addn_ielen,
		tDot11fIESuppOperatingClasses *dst);

uint8_t lim_get_80Mhz_center_channel(uint8_t primary_channel);
void lim_update_obss_scanparams(tpPESession session,
			tDot11fIEOBSSScanParameters *scan_params);
void lim_init_obss_params(tpAniSirGlobal mac_ctx, tpPESession session);
#ifdef WLAN_FEATURE_HOST_ROAM
uint32_t lim_create_timers_host_roam(tpAniSirGlobal mac_ctx);
void lim_delete_timers_host_roam(tpAniSirGlobal mac_ctx);
void lim_deactivate_and_change_timer_host_roam(tpAniSirGlobal mac_ctx,
		uint32_t timer_id);
#else
static inline uint32_t lim_create_timers_host_roam(tpAniSirGlobal mac_ctx)
{
	return 0;
}
static inline void lim_delete_timers_host_roam(tpAniSirGlobal mac_ctx)
{}
static inline void lim_deactivate_and_change_timer_host_roam(
		tpAniSirGlobal mac_ctx, uint32_t timer_id)
{}
#endif

bool lim_is_robust_mgmt_action_frame(uint8_t action_category);
uint8_t lim_compute_ext_cap_ie_length(tDot11fIEExtCap *ext_cap);

/**
 * lim_p2p_action_cnf() - callback to indicate Tx completion
 * @mac_ctx: pointer to mac structure
 * @buf: buffer
 * @tx_complete_success: indicates tx success/failure
 * @params: tx completion params
 *
 * function will be invoked on receiving tx completion indication
 *
 * return: success: eHAL_STATUS_SUCCESS failure: eHAL_STATUS_FAILURE
 */
QDF_STATUS lim_p2p_action_cnf(void *mac_ctx, qdf_nbuf_t buf,
			uint32_t tx_complete_success, void *params);
void lim_update_caps_info_for_bss(tpAniSirGlobal mac_ctx,
			uint16_t *caps, uint16_t bss_caps);
void lim_send_set_dtim_period(tpAniSirGlobal mac_ctx, uint8_t dtim_period,
			      tpPESession session);

tSirRetStatus lim_strip_ie(tpAniSirGlobal mac_ctx,
		uint8_t *addn_ie, uint16_t *addn_ielen,
		uint8_t eid, eSizeOfLenField size_of_len_field,
		uint8_t *oui, uint8_t out_len, uint8_t *extracted_ie,
		uint32_t eid_max_len);

#define MCSMAPMASK1x1 0x3
#define MCSMAPMASK2x2 0xC

#ifdef WLAN_FEATURE_11AX

/**
 * lim_intersect_ap_he_caps() - Intersect AP capability with self STA capability
 * @session: pointer to PE session
 * @add_bss: pointer to ADD BSS params
 * @beacon: pointer to beacon
 * @assoc_rsp: pointer to assoc response
 *
 * Return: None
 */
void lim_intersect_ap_he_caps(tpPESession session, tpAddBssParams add_bss,
		tSchBeaconStruct *pBeaconStruct, tpSirAssocRsp assoc_rsp);

/**
 * lim_intersect_sta_he_caps() - Intersect STA capability with SAP capability
 * @assoc_req: pointer to assoc request
 * @session: pointer to PE session
 * @sta_ds: pointer to STA dph hash table entry
 *
 * Return: None
 */
void lim_intersect_sta_he_caps(tpSirAssocReq assoc_req, tpPESession session,
		tpDphHashNode sta_ds);

/**
 * lim_add_he_cap() - Copy HE capability into Add sta params
 * @add_sta_params: pointer to add sta params
 * @assoc_req: pointer to Assoc request
 *
 * Return: None
 */
void lim_add_he_cap(tpAddStaParams add_sta_params, tpSirAssocReq assoc_req);

/**
 * lim_add_self_he_cap() - Copy HE capability into add sta from PE session
 * @add_sta_params: pointer to add sta params
 * @session: pointer to PE Session
 *
 * Return: None
 */
void lim_add_self_he_cap(tpAddStaParams add_sta_params, tpPESession session);

/**
 * lim_add_bss_he_cap() - Copy HE capability into ADD BSS params
 * @add_bss: pointer to add bss params
 * @assoc_rsp: pointer to assoc response
 *
 * Return: None
 */
void lim_add_bss_he_cap(tpAddBssParams add_bss, tpSirAssocRsp assoc_rsp);

/**
 * lim_add_bss_he_cfg() - Set HE config to BSS params
 * @add_bss: pointer to add bss params
 * @session: Pointer to Session entry struct
 *
 * Return: None
 */
void lim_add_bss_he_cfg(tpAddBssParams add_bss, tpPESession session);

/**
 * lim_copy_bss_he_cap() - Copy HE capability into PE session from start bss
 * @session: pointer to PE session
 * @sme_start_bss_req: pointer to start BSS request
 *
 * Return: None
 */
void lim_copy_bss_he_cap(tpPESession session,
		tpSirSmeStartBssReq sme_start_bss_req);

/**
 * lim_copy_join_req_he_cap() - Copy HE capability to PE session from Join req
 * @session: pointer to PE session
 * @sme_join_req: pointer to SME join request
 *
 * Return: None
 */
void lim_copy_join_req_he_cap(tpPESession session,
			tpSirSmeJoinReq sme_join_req);

/**
 * lim_log_he_op() - Print HE Operation
 * @mac: pointer to MAC context
 * @he_op: pointer to HE Operation
 *
 * Print HE operation stored as dot11f structure
 *
 * Return: None
 */
void lim_log_he_op(tpAniSirGlobal mac, tDot11fIEvendor_he_op *he_ops);

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
/**
 * lim_log_he_bss_color() - Print HE bss color
 * @mac: pointer to MAC context
 * @he_bss_color: pointer to HE bss color
 *
 * Print HE bss color IE
 *
 * Return: None
 */
void lim_log_he_bss_color(tpAniSirGlobal mac,
			tDot11fIEbss_color_change *he_bss_color);
#endif

/**
 * lim_log_he_cap() - Print HE capabilities
 * @mac: pointer to MAC context
 * @he_cap: pointer to HE Capability
 *
 * Received HE capabilities are converted into dot11f structure.
 * This function will print all the HE capabilities as stored
 * in the dot11f structure.
 *
 * Return: None
 */
void lim_log_he_cap(tpAniSirGlobal mac, tDot11fIEvendor_he_cap *he_cap);

/**
 * lim_update_stads_he_caps() - Copy HE capability into STA DPH hash table entry
 * @sta_ds: pointer to sta dph hash table entry
 * @assoc_rsp: pointer to assoc response
 * @session_entry: pointer to PE session
 *
 * Return: None
 */
void lim_update_stads_he_caps(tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
			      tpPESession session_entry);

/**
 * lim_update_usr_he_cap() - Update HE capability based on userspace
 * @mac_ctx: global mac context
 * @session: PE session entry
 *
 * Parse the HE Capability IE and populate the fields to be
 * sent to FW as part of add bss and update PE session.
 */
void lim_update_usr_he_cap(tpAniSirGlobal mac_ctx, tpPESession session);

/**
 * lim_decide_he_op() - Determine HE operation elements
 * @mac_ctx: global mac context
 * @he_ops: pointer to HE operation IE
 * @session: PE session entry
 *
 * Parse the HE Operation IE and populate the fields to be
 * sent to FW as part of add bss.
 */
void lim_decide_he_op(tpAniSirGlobal mac_ctx, tpAddBssParams add_bss,
		tpPESession session);

/**
 * lim_update_sta_he_capable(): Update he_capable in add sta params
 * @mac: pointer to MAC context
 * @add_sta_params: pointer to add sta params
 * @sta_ds: pointer to dph hash table entry
 * @session_entry: pointer to PE session
 *
 * Return: None
 */
void lim_update_sta_he_capable(tpAniSirGlobal mac,
	tpAddStaParams add_sta_params, tpDphHashNode sta_ds,
	tpPESession session_entry);

static inline bool lim_is_session_he_capable(tpPESession session)
{
	return session->he_capable;
}

static inline bool lim_is_sta_he_capable(tpDphHashNode sta_ds)
{
	return sta_ds->mlmStaContext.he_capable;
}

/**
 * lim_update_bss_he_capable(): Update he_capable in add BSS params
 * @mac: pointer to MAC context
 * @add_bss: pointer to add BSS params
 *
 * Return: None
 */
void lim_update_bss_he_capable(tpAniSirGlobal mac, tpAddBssParams add_bss);

/**
 * lim_update_stads_he_capable() - Update he_capable in sta ds context
 * @sta_ds: pointer to sta ds
 * @assoc_req: pointer to assoc request
 *
 * Return: None
 */
void lim_update_stads_he_capable(tpDphHashNode sta_ds, tpSirAssocReq assoc_req);

/**
 * lim_update_session_he_capable(): Update he_capable in PE session
 * @mac: pointer to MAC context
 * @session: pointer to PE session
 *
 * Return: None
 */
void lim_update_session_he_capable(tpAniSirGlobal mac, tpPESession session);

/**
 * lim_update_chan_he_capable(): Update he_capable in chan switch params
 * @mac: pointer to MAC context
 * @chan: pointer to channel switch params
 *
 * Return: None
 */
void lim_update_chan_he_capable(tpAniSirGlobal mac, tpSwitchChannelParams chan);

/**
 * lim_set_he_caps() - update HE caps to be sent to FW as part of scan IE
 * @mac: pointer to MAC
 * @session: pointer to PE session
 * @ie_start: pointer to start of IE buffer
 * @num_bytes: length of IE buffer
 *
 * Return: None
 */
void lim_set_he_caps(tpAniSirGlobal mac, tpPESession session,
		     uint8_t *ie_start, uint32_t num_bytes);

/**
 * lim_send_he_caps_ie() - gets HE capability and send to firmware via wma
 * @mac_ctx: global mac context
 * @session: pe session. This can be NULL. In that case self cap will be sent
 * @vdev_id: vdev for which IE is targeted
 *
 * This function gets HE capability and send to firmware via wma
 *
 * Return: QDF_STATUS
 */
QDF_STATUS lim_send_he_caps_ie(tpAniSirGlobal mac_ctx, tpPESession session,
			       uint8_t vdev_id);

/**
 * lim_populate_he_mcs_set - function to populate HE mcs rate set
 * @mac_ctx: pointer to global mac structure
 * @rates: pointer to supported rate set
 * @peer_vht_caps: pointer to peer HE capabilities
 * @session_entry: pe session entry
 *
 * Populates HE mcs rate set based on peer and self capabilities
 *
 * Return: QDF_STATUS
 */
QDF_STATUS lim_populate_he_mcs_set(tpAniSirGlobal mac_ctx,
		tpSirSupportedRates rates, tDot11fIEvendor_he_cap *peer_he_caps,
		tpPESession session_entry, uint8_t nss);

#else
static inline void lim_add_he_cap(tpAddStaParams add_sta_params,
				  tpSirAssocReq assoc_req)
{
}

static inline void lim_add_self_he_cap(tpAddStaParams add_sta_params,
				       tpPESession session)
{
}

static inline void lim_add_bss_he_cap(tpAddBssParams add_bss,
				      tpSirAssocRsp assoc_rsp)
{
	return;
}

static inline void lim_add_bss_he_cfg(tpAddBssParams add_bss,
					 tpPESession session)
{
}

static inline void lim_intersect_ap_he_caps(tpPESession session,
		tpAddBssParams add_bss,	tSchBeaconStruct *pBeaconStruct,
		tpSirAssocRsp assoc_rsp)
{
	return;
}

static inline void lim_intersect_sta_he_caps(tpSirAssocReq assoc_req,
		tpPESession session, tpDphHashNode sta_ds)
{
}

static inline void lim_update_stads_he_caps(tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
		tpPESession session_entry)
{
	return;
}

static inline void lim_update_usr_he_cap(tpAniSirGlobal mac_ctx,
			tpPESession session)
{
}

static inline void lim_decide_he_op(tpAniSirGlobal mac_ctx,
			tpAddBssParams add_bss, tpPESession session)
{
}

static inline void lim_copy_bss_he_cap(tpPESession session,
				tpSirSmeStartBssReq sme_start_bss_req)
{
}

static inline void lim_copy_join_req_he_cap(tpPESession session,
			tpSirSmeJoinReq sme_join_req)
{
}

static inline void lim_log_he_op(tpAniSirGlobal mac,
	tDot11fIEvendor_he_op *he_ops)
{
}

static inline void lim_log_he_cap(tpAniSirGlobal mac,
	tDot11fIEvendor_he_cap *he_cap)
{
}

static inline void lim_update_sta_he_capable(tpAniSirGlobal mac,
			tpAddStaParams add_sta_params,
			tpDphHashNode sta_ds, tpPESession session_entry)
{
}

static inline bool lim_is_session_he_capable(tpPESession session)
{
	return false;
}

static inline bool lim_is_sta_he_capable(tpDphHashNode sta_ds)
{
	return false;
}

static inline void lim_update_bss_he_capable(tpAniSirGlobal mac,
			tpAddBssParams add_bss)
{
}

static inline void lim_update_stads_he_capable(tpDphHashNode sta_ds,
		tpSirAssocReq assoc_req)
{
}

static inline void lim_update_session_he_capable(tpAniSirGlobal mac,
			tpPESession session)
{
}

static inline void lim_update_chan_he_capable(tpAniSirGlobal mac,
		tpSwitchChannelParams chan)
{
}

static inline void lim_set_he_caps(tpAniSirGlobal mac, tpPESession session,
				   uint8_t *ie_start, uint32_t num_bytes)
{
}

static inline QDF_STATUS lim_send_he_caps_ie(tpAniSirGlobal mac_ctx,
					     tpPESession session,
					     uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS lim_populate_he_mcs_set(tpAniSirGlobal mac_ctx,
				tpSirSupportedRates rates,
				tDot11fIEvendor_he_cap *peer_he_caps,
				tpPESession session_entry, uint8_t nss)
{
	return QDF_STATUS_SUCCESS;
}

#endif

/**
 * lim_decrement_pending_mgmt_count: Decrement mgmt frame count
 * @mac_ctx: Pointer to global MAC structure
 *
 * This function is used to decrement pe mgmt count once frame
 * removed from queue
 *
 * Return: None
 */
void lim_decrement_pending_mgmt_count(tpAniSirGlobal mac_ctx);
#endif /* __LIM_UTILS_H */
