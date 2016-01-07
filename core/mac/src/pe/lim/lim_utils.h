/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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

#define MIN_TX_PWR_CAP    8
#define MAX_TX_PWR_CAP    22

#define LIM_STA_ID_MASK                        0x00FF
#define LIM_AID_MASK                              0xC000
#define LIM_SPECTRUM_MANAGEMENT_BIT_MASK          0x0100
#define LIM_RRM_BIT_MASK                          0x1000
#define LIM_SHORT_PREAMBLE_BIT_MASK               0x0020
#define LIM_IMMEDIATE_BLOCK_ACK_MASK              0x8000
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR)
#define LIM_MAX_REASSOC_RETRY_LIMIT            2
#endif

/* classifier ID is coded as 0-3: tsid, 4-5:direction */
#define LIM_MAKE_CLSID(tsid, dir) (((tsid) & 0x0F) | (((dir) & 0x03) << 4))

#define VHT_MCS_3x3_MASK    0x30
#define VHT_MCS_2x2_MASK    0x0C

#ifdef WLAN_FEATURE_11W
typedef union uPmfSaQueryTimerId {
	struct {
		uint8_t sessionId;
		uint16_t peerIdx;
	} fields;
	uint32_t value;
} tPmfSaQueryTimerId, *tpPmfSaQueryTimerId;
#endif

/* LIM utility functions */
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

#if defined FEATURE_WLAN_ESE || defined WLAN_FEATURE_VOWIFI
extern tSirRetStatus lim_send_set_max_tx_power_req(tpAniSirGlobal pMac,
		tPowerdBm txPower,
		tpPESession pSessionEntry);
extern uint8_t lim_get_max_tx_power(tPowerdBm regMax, tPowerdBm apTxPower,
		uint8_t iniTxPower);
#endif
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
uint8_t lim_write_deferred_msg_q(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
tSirMsgQ *lim_read_deferred_msg_q(tpAniSirGlobal pMac);
void lim_handle_defer_msg_error(tpAniSirGlobal pMac, tpSirMsgQ pLimMsg);

/* Deferred Message Queue Reset */
void lim_reset_deferred_msg_q(tpAniSirGlobal pMac);

tSirRetStatus lim_sys_process_mmh_msg_api(tpAniSirGlobal, tSirMsgQ *, uint8_t);

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
					phy_ch_width ch_width);
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
void lim_switch_channel_cback(tpAniSirGlobal pMac, CDF_STATUS status,
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
void lim_tx_complete(tHalHandle hHal, void *pData, bool free);

/**
 * This function will be registered with HAL for callback when TSPEC inactivity
 * timer fires.
 */

void lim_process_del_ts_ind(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
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

void lim_delete_sta_context(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
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

void lim_process_add_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ pMsgQ);

void lim_update_beacon(tpAniSirGlobal pMac);

void lim_process_ap_mlm_add_sta_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,
		tpPESession psessionEntry);
void lim_process_bt_amp_ap_mlm_del_bss_rsp(tpAniSirGlobal pMac,
		tpSirMsgQ limMsgQ,
		tpPESession psessionEntry);

void lim_process_bt_amp_ap_mlm_del_sta_rsp(tpAniSirGlobal pMac,
		tpSirMsgQ limMsgQ,
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
bool lim_isconnected_on_dfs_channel(uint8_t currentChannel);
uint8_t lim_get_current_operating_channel(tpAniSirGlobal pMac);
uint32_t lim_get_max_rate_flags(tpAniSirGlobal mac_ctx, tpDphHashNode sta_ds);

#ifdef WLAN_FEATURE_11AC
bool lim_check_vht_op_mode_change(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		uint8_t chanWidth, uint8_t staId,
		uint8_t *peerMac);
bool lim_set_nss_change(tpAniSirGlobal pMac, tpPESession psessionEntry,
		uint8_t rxNss, uint8_t staId, uint8_t *peerMac);
bool lim_check_membership_user_position(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		uint32_t membership, uint32_t userPosition,
		uint8_t staId);
#endif

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
	WLAN_PE_DIAG_STOP_BSS_REQ_EVENT,
	WLAN_PE_DIAG_STOP_BSS_RSP_EVENT,
	WLAN_PE_DIAG_DEAUTH_CNF_EVENT,
	WLAN_PE_DIAG_ADDTS_REQ_EVENT,
	WLAN_PE_DIAG_ADDTS_RSP_EVENT,
	WLAN_PE_DIAG_DELTS_REQ_EVENT = 30,
	WLAN_PE_DIAG_DELTS_RSP_EVENT,
	WLAN_PE_DIAG_DELTS_IND_EVENT,
	WLAN_PE_DIAG_ENTER_BMPS_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_BMPS_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_REQ_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_BMPS_IND_EVENT,
	WLAN_PE_DIAG_ENTER_UAPSD_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_UAPSD_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_UAPSD_REQ_EVENT = 40,
	WLAN_PE_DIAG_EXIT_UAPSD_RSP_EVENT,
	WLAN_PE_DIAG_WOWL_ADD_BCAST_PTRN_EVENT,
	WLAN_PE_DIAG_WOWL_DEL_BCAST_PTRN_EVENT,
	WLAN_PE_DIAG_ENTER_WOWL_REQ_EVENT,
	WLAN_PE_DIAG_ENTER_WOWL_RSP_EVENT,
	WLAN_PE_DIAG_EXIT_WOWL_REQ_EVENT,
	WLAN_PE_DIAG_EXIT_WOWL_RSP_EVENT,
	WLAN_PE_DIAG_HB_FAILURE_TIMEOUT,
	WLAN_PE_DIAG_PRE_AUTH_REQ_EVENT,
	WLAN_PE_DIAG_PRE_AUTH_RSP_EVENT = 50,
	WLAN_PE_DIAG_PREAUTH_DONE,
	WLAN_PE_DIAG_REASSOCIATING,
	WLAN_PE_DIAG_CONNECTED,
	WLAN_PE_DIAG_ASSOC_REQ_EVENT,
	WLAN_PE_DIAG_AUTH_COMP_EVENT,
	WLAN_PE_DIAG_ASSOC_COMP_EVENT,
	WLAN_PE_DIAG_AUTH_START_EVENT,
	WLAN_PE_DIAG_ASSOC_START_EVENT,
	WLAN_PE_DIAG_REASSOC_START_EVENT,
	WLAN_PE_DIAG_ROAM_AUTH_START_EVENT = 60,
	WLAN_PE_DIAG_ROAM_AUTH_COMP_EVENT,
	WLAN_PE_DIAG_ROAM_ASSOC_START_EVENT,
	WLAN_PE_DIAG_ROAM_ASSOC_COMP_EVENT,
	RESERVED1, /* = 64 for SCAN_COMPLETE */
	RESERVED2, /* = 65 for SCAN_RES_FOUND */
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
#endif

void lim_set_protected_bit(tpAniSirGlobal pMac,
		tpPESession psessionEntry,
		tSirMacAddr peer, tpSirMacMgmtHdr pMacHdr);

#ifdef WLAN_FEATURE_11W
void lim_pmf_comeback_timer_callback(void *context);
#endif /* WLAN_FEATURE_11W */

void lim_set_ht_caps(tpAniSirGlobal p_mac,
		tpPESession p_session_ntry,
		uint8_t *p_ie_start,
		uint32_t num_bytes);

#ifdef WLAN_FEATURE_11AC
void lim_set_vht_caps(tpAniSirGlobal p_mac,
		tpPESession p_session_entry,
		uint8_t *p_ie_start,
		uint32_t num_bytes);
#endif /* WLAN_FEATURE_11AC */
bool lim_validate_received_frame_a1_addr(tpAniSirGlobal mac_ctx,
		tSirMacAddr a1, tpPESession session);
void lim_set_stads_rtt_cap(tpDphHashNode sta_ds, struct s_ext_cap *ext_cap,
			   tpAniSirGlobal mac_ctx);

void lim_check_and_reset_protection_params(tpAniSirGlobal mac_ctx);

CDF_STATUS lim_send_ext_cap_ie(tpAniSirGlobal mac_ctx, uint32_t session_id,
			       tDot11fIEExtCap *extracted_extcap, bool merge);

tSirRetStatus lim_strip_extcap_ie(tpAniSirGlobal mac_ctx, uint8_t *addn_ie,
			  uint16_t *addn_ielen, uint8_t *extracted_extcap);
void lim_update_extcap_struct(tpAniSirGlobal mac_ctx, uint8_t *buf,
			      tDot11fIEExtCap *ext_cap);
tSirRetStatus lim_strip_extcap_update_struct(tpAniSirGlobal mac_ctx,
		uint8_t *addn_ie, uint16_t *addn_ielen, tDot11fIEExtCap *dst);
void lim_merge_extcap_struct(tDot11fIEExtCap *dst, tDot11fIEExtCap *src);

uint8_t lim_get_80Mhz_center_channel(uint8_t primary_channel);
#endif /* __LIM_UTILS_H */
