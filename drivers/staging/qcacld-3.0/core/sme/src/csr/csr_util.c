/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: csr_util.c
 *
 * Implementation supporting routines for CSR.
 */

#include "ani_global.h"

#include "csr_support.h"
#include "csr_inside_api.h"
#include "sme_qos_internal.h"
#include "wma_types.h"
#include "cds_utils.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_serialization_legacy_api.h"
#include "wlan_reg_services_api.h"
#include "wlan_crypto_global_api.h"
#include "wlan_cm_roam_api.h"
#include <../../core/src/wlan_cm_vdev_api.h>
#include <wlan_mlo_mgr_public_structs.h>
#include "wlan_objmgr_vdev_obj.h"

#define CASE_RETURN_STR(n) {\
	case (n): return (# n);\
}

const char *get_e_roam_cmd_status_str(eRoamCmdStatus val)
{
	switch (val) {
		CASE_RETURN_STR(eCSR_ROAM_LOSTLINK);
		CASE_RETURN_STR(eCSR_ROAM_MIC_ERROR_IND);
		CASE_RETURN_STR(eCSR_ROAM_SET_KEY_COMPLETE);
		CASE_RETURN_STR(eCSR_ROAM_INFRA_IND);
		CASE_RETURN_STR(eCSR_ROAM_WPS_PBC_PROBE_REQ_IND);
		CASE_RETURN_STR(eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS);
		CASE_RETURN_STR(eCSR_ROAM_SEND_P2P_STOP_BSS);
		CASE_RETURN_STR(eCSR_ROAM_UNPROT_MGMT_FRAME_IND);
#ifdef FEATURE_WLAN_ESE
		CASE_RETURN_STR(eCSR_ROAM_TSM_IE_IND);
		CASE_RETURN_STR(eCSR_ROAM_ESE_ADJ_AP_REPORT_IND);
		CASE_RETURN_STR(eCSR_ROAM_ESE_BCN_REPORT_IND);
#endif /* FEATURE_WLAN_ESE */
		CASE_RETURN_STR(eCSR_ROAM_DFS_RADAR_IND);
		CASE_RETURN_STR(eCSR_ROAM_SET_CHANNEL_RSP);
		CASE_RETURN_STR(eCSR_ROAM_DFS_CHAN_SW_NOTIFY);
		CASE_RETURN_STR(eCSR_ROAM_EXT_CHG_CHNL_IND);
		CASE_RETURN_STR(eCSR_ROAM_STA_CHANNEL_SWITCH);
		CASE_RETURN_STR(eCSR_ROAM_NDP_STATUS_UPDATE);
		CASE_RETURN_STR(eCSR_ROAM_CHANNEL_COMPLETE_IND);
		CASE_RETURN_STR(eCSR_ROAM_SAE_COMPUTE);
	default:
		return "unknown";
	}
}

const char *get_e_csr_roam_result_str(eCsrRoamResult val)
{
	switch (val) {
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NONE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NOT_ASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_FORCED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DISASSOC_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DEAUTH_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CAP_CHANGED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_LOSTLINK);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_ERROR_UNICAST);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_ERROR_GROUP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_AUTHENTICATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NEW_RSN_BSS);
 #ifdef FEATURE_WLAN_WAPI
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NEW_WAPI_BSS);
 #endif /* FEATURE_WLAN_WAPI */
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_STARTED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_START_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_STOPPED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_DISASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_SEND_ACTION_FAIL);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ADD_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_UPDATE_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DELETE_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED);
		CASE_RETURN_STR
			(eCSR_ROAM_RESULT_TDLS_CONNECTION_TRACKER_NOTIFICATION);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CSA_RESTART_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDI_CREATE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDI_DELETE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_INITIATOR_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_NEW_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_CONFIRM_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_INDICATION);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_SCHED_UPDATE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_RESPONDER_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_END_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_END_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	default:
		return "unknown";
	}
}

const char *csr_phy_mode_str(eCsrPhyMode phy_mode)
{
	switch (phy_mode) {
	case eCSR_DOT11_MODE_abg:
		return "abg";
	case eCSR_DOT11_MODE_11a:
		return "11a";
	case eCSR_DOT11_MODE_11b:
		return "11b";
	case eCSR_DOT11_MODE_11g:
		return "11g";
	case eCSR_DOT11_MODE_11n:
		return "11n";
	case eCSR_DOT11_MODE_11g_ONLY:
		return "11g_only";
	case eCSR_DOT11_MODE_11n_ONLY:
		return "11n_only";
	case eCSR_DOT11_MODE_11b_ONLY:
		return "11b_only";
	case eCSR_DOT11_MODE_11ac:
		return "11ac";
	case eCSR_DOT11_MODE_11ac_ONLY:
		return "11ac_only";
	case eCSR_DOT11_MODE_AUTO:
		return "auto";
	case eCSR_DOT11_MODE_11ax:
		return "11ax";
	case eCSR_DOT11_MODE_11ax_ONLY:
		return "11ax_only";
	case eCSR_DOT11_MODE_11be:
		return "11be";
	case eCSR_DOT11_MODE_11be_ONLY:
		return "11be_only";
	default:
		return "unknown";
	}
}

void csr_purge_pdev_all_ser_cmd_list(struct mac_context *mac_ctx)
{
	wlan_serialization_purge_all_pdev_cmd(mac_ctx->pdev);
}

tListElem *csr_nonscan_active_ll_peek_head(struct mac_context *mac_ctx,
					   bool inter_locked)
{
	struct wlan_serialization_command *cmd;
	tSmeCmd *sme_cmd;

	cmd = wlan_serialization_peek_head_active_cmd_using_psoc(mac_ctx->psoc,
								 false);
	if (!cmd || cmd->source != WLAN_UMAC_COMP_MLME)
		return NULL;

	sme_cmd = cmd->umac_cmd;

	return &sme_cmd->Link;
}

tListElem *csr_nonscan_pending_ll_peek_head(struct mac_context *mac_ctx,
					    bool inter_locked)
{
	struct wlan_serialization_command *cmd;
	tSmeCmd *sme_cmd;

	cmd = wlan_serialization_peek_head_pending_cmd_using_psoc(mac_ctx->psoc,
								  false);
	while (cmd) {
		if (cmd->source == WLAN_UMAC_COMP_MLME) {
			sme_cmd = cmd->umac_cmd;
			return &sme_cmd->Link;
		}
		cmd = wlan_serialization_get_pending_list_next_node_using_psoc(
						mac_ctx->psoc, cmd, false);
	}

	return NULL;
}

bool csr_nonscan_active_ll_remove_entry(struct mac_context *mac_ctx,
					tListElem *entry, bool inter_locked)
{
	tListElem *head;

	head = csr_nonscan_active_ll_peek_head(mac_ctx, inter_locked);
	if (head == entry)
	return true;

	return false;
}

tListElem *csr_nonscan_pending_ll_next(struct mac_context *mac_ctx,
				       tListElem *entry, bool inter_locked)
{
	tSmeCmd *sme_cmd;
	struct wlan_serialization_command cmd, *tcmd;

	if (!entry)
		return NULL;
	sme_cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
	cmd.cmd_id = sme_cmd->cmd_id;
	cmd.cmd_type = csr_get_cmd_type(sme_cmd);
	cmd.vdev = wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
				mac_ctx->psoc,
				sme_cmd->vdev_id, WLAN_LEGACY_SME_ID);
	tcmd = wlan_serialization_get_pending_list_next_node_using_psoc(
				mac_ctx->psoc, &cmd, false);
	if (cmd.vdev)
		wlan_objmgr_vdev_release_ref(cmd.vdev, WLAN_LEGACY_SME_ID);
	while (tcmd) {
		if (tcmd->source == WLAN_UMAC_COMP_MLME) {
			sme_cmd = tcmd->umac_cmd;
			return &sme_cmd->Link;
		}
		tcmd = wlan_serialization_get_pending_list_next_node_using_psoc(
						mac_ctx->psoc, tcmd, false);
	}

	return NULL;
}

static bool csr_is_conn_state(struct mac_context *mac_ctx, uint32_t session_id,
			      eCsrConnectState state)
{
	QDF_BUG(session_id < WLAN_MAX_VDEVS);
	if (session_id >= WLAN_MAX_VDEVS)
		return false;

	return mac_ctx->roam.roamSession[session_id].connectState == state;
}

bool csr_is_conn_state_connected(struct mac_context *mac, uint32_t sessionId)
{
	return cm_is_vdevid_connected(mac->pdev, sessionId) ||
	       csr_is_conn_state_connected_wds(mac, sessionId);
}

bool csr_is_conn_state_connected_wds(struct mac_context *mac_ctx,
				     uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_WDS_CONNECTED);
}

bool csr_is_conn_state_connected_infra_ap(struct mac_context *mac_ctx,
					  uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED) ||
		csr_is_conn_state(mac_ctx, session_id,
				  eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED);
}

bool csr_is_conn_state_disconnected_wds(struct mac_context *mac_ctx,
					uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_WDS_DISCONNECTED);
}

bool csr_is_conn_state_wds(struct mac_context *mac, uint32_t sessionId)
{
	return csr_is_conn_state_connected_wds(mac, sessionId) ||
	       csr_is_conn_state_disconnected_wds(mac, sessionId);
}

enum mlme_dot11_mode
csr_get_vdev_dot11_mode(struct mac_context *mac,
			enum QDF_OPMODE device_mode,
			enum mlme_dot11_mode curr_dot11_mode)
{
	enum mlme_vdev_dot11_mode vdev_dot11_mode;
	uint8_t dot11_mode_indx;
	uint32_t vdev_type_dot11_mode =
				mac->mlme_cfg->dot11_mode.vdev_type_dot11_mode;
	enum mlme_dot11_mode dot11_mode = curr_dot11_mode;

	sme_debug("curr_dot11_mode %d, vdev_dot11 %08X, dev_mode %d",
		  curr_dot11_mode, vdev_type_dot11_mode, device_mode);

	switch (device_mode) {
	case QDF_STA_MODE:
		dot11_mode_indx = STA_DOT11_MODE_INDX;
		break;
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_DEVICE_MODE:
		dot11_mode_indx = P2P_DEV_DOT11_MODE_INDX;
		break;
	case QDF_TDLS_MODE:
		dot11_mode_indx = TDLS_DOT11_MODE_INDX;
		break;
	case QDF_NAN_DISC_MODE:
		dot11_mode_indx = NAN_DISC_DOT11_MODE_INDX;
		break;
	case QDF_NDI_MODE:
		dot11_mode_indx = NDI_DOT11_MODE_INDX;
		break;
	case QDF_OCB_MODE:
		dot11_mode_indx = OCB_DOT11_MODE_INDX;
		break;
	default:
		return dot11_mode;
	}
	vdev_dot11_mode = QDF_GET_BITS(vdev_type_dot11_mode,
				       dot11_mode_indx, 4);
	if (vdev_dot11_mode == MLME_VDEV_DOT11_MODE_AUTO)
		dot11_mode = curr_dot11_mode;

	if (IS_DOT11_MODE_HT(curr_dot11_mode) &&
	    vdev_dot11_mode == MLME_VDEV_DOT11_MODE_11N)
		dot11_mode = MLME_DOT11_MODE_11N;

	if (IS_DOT11_MODE_VHT(curr_dot11_mode) &&
	    vdev_dot11_mode == MLME_VDEV_DOT11_MODE_11AC)
		dot11_mode = MLME_DOT11_MODE_11AC;

	if (IS_DOT11_MODE_HE(curr_dot11_mode) &&
	    vdev_dot11_mode == MLME_VDEV_DOT11_MODE_11AX)
		dot11_mode = MLME_DOT11_MODE_11AX;
#ifdef WLAN_FEATURE_11BE
	if (IS_DOT11_MODE_EHT(curr_dot11_mode) &&
	    vdev_dot11_mode == MLME_VDEV_DOT11_MODE_11BE)
		dot11_mode = MLME_DOT11_MODE_11BE;
#endif
	sme_debug("INI vdev_dot11_mode %d new dot11_mode %d",
		  vdev_dot11_mode, dot11_mode);

	return dot11_mode;
}

static bool csr_is_conn_state_ap(struct mac_context *mac, uint32_t sessionId)
{
	enum QDF_OPMODE opmode;

	opmode = wlan_get_opmode_vdev_id(mac->pdev, sessionId);
	if (opmode == QDF_SAP_MODE || opmode == QDF_P2P_GO_MODE)
		return true;

	return false;
}

bool csr_is_any_session_in_connect_state(struct mac_context *mac)
{
	uint32_t i;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (CSR_IS_SESSION_VALID(mac, i) &&
		    (cm_is_vdevid_connected(mac->pdev, i) ||
		     csr_is_conn_state_ap(mac, i))) {
			return true;
		}
	}

	return false;
}

qdf_freq_t csr_get_concurrent_operation_freq(struct mac_context *mac_ctx)
{
	uint8_t i = 0;
	qdf_freq_t freq;
	enum QDF_OPMODE op_mode;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		op_mode = wlan_get_opmode_from_vdev_id(mac_ctx->pdev, i);
		/* check only for STA, CLI, GO and SAP */
		if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE &&
		    op_mode != QDF_P2P_GO_MODE && op_mode != QDF_SAP_MODE)
			continue;

		freq = wlan_get_operation_chan_freq_vdev_id(mac_ctx->pdev, i);
		if (!freq)
			continue;

		return freq;
	}

	return 0;
}

uint32_t csr_get_beaconing_concurrent_channel(struct mac_context *mac_ctx,
					     uint8_t vdev_id_to_skip)
{
	struct csr_roam_session *session = NULL;
	uint8_t i = 0;
	enum QDF_OPMODE persona;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (i == vdev_id_to_skip)
			continue;
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;
		session = CSR_GET_SESSION(mac_ctx, i);
		persona = wlan_get_opmode_from_vdev_id(mac_ctx->pdev, i);
		if (((persona == QDF_P2P_GO_MODE) ||
		     (persona == QDF_SAP_MODE)) &&
		     (session->connectState !=
		      eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED))
			return wlan_get_operation_chan_freq_vdev_id(mac_ctx->pdev, i);
	}

	return 0;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH

#define HALF_BW_OF(eCSR_bw_val) ((eCSR_bw_val)/2)

/* calculation of center channel based on V/HT BW and WIFI channel bw=5MHz) */

#define CSR_GET_HT40_PLUS_CCH(och) ((och) + 10)
#define CSR_GET_HT40_MINUS_CCH(och) ((och) - 10)

#define CSR_GET_HT80_PLUS_LL_CCH(och) ((och) + 30)
#define CSR_GET_HT80_PLUS_HL_CCH(och) ((och) + 30)
#define CSR_GET_HT80_MINUS_LH_CCH(och) ((och) - 10)
#define CSR_GET_HT80_MINUS_HH_CCH(och) ((och) - 30)

/**
 * csr_calc_chb_for_sap_phymode() - to calc channel bandwidth for sap phymode
 * @mac_ctx: pointer to mac context
 * @sap_ch: SAP operating channel
 * @sap_phymode: SAP physical mode
 * @sap_cch: concurrency channel
 * @sap_hbw: SAP half bw
 * @chb: channel bandwidth
 *
 * This routine is called to calculate channel bandwidth
 *
 * Return: none
 */
static void csr_calc_chb_for_sap_phymode(struct mac_context *mac_ctx,
		uint32_t *sap_ch, eCsrPhyMode *sap_phymode,
		uint32_t *sap_cch, uint32_t *sap_hbw, uint8_t *chb)
{
	if (*sap_phymode == eCSR_DOT11_MODE_11n ||
			*sap_phymode == eCSR_DOT11_MODE_11n_ONLY) {

		*sap_hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
		if (*chb == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
			*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
		else if (*chb == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
			*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);

	} else if (*sap_phymode == eCSR_DOT11_MODE_11ac ||
		   *sap_phymode == eCSR_DOT11_MODE_11ac_ONLY ||
		   *sap_phymode == eCSR_DOT11_MODE_11ax ||
		   *sap_phymode == eCSR_DOT11_MODE_11ax_ONLY ||
		   CSR_IS_DOT11_PHY_MODE_11BE(*sap_phymode) ||
		   CSR_IS_DOT11_PHY_MODE_11BE_ONLY(*sap_phymode)) {
		/*11AC only 80/40/20 Mhz supported in Rome */
		if (mac_ctx->roam.configParam.nVhtChannelWidth ==
				(WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ + 1)) {
			*sap_hbw = HALF_BW_OF(eCSR_BW_80MHz_VAL);
			if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW - 1))
				*sap_cch = CSR_GET_HT80_PLUS_LL_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW
				     - 1))
				*sap_cch = CSR_GET_HT80_PLUS_HL_CCH(*sap_ch);
			else if (*chb ==
				 (PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT80_MINUS_LH_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT80_MINUS_HH_CCH(*sap_ch);
		} else {
			*sap_hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
			if (*chb == (PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW
					- 1))
				*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW
				     - 1))
				*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);
		}
	}
}

static eCSR_BW_Val csr_get_half_bw(enum phy_ch_width ch_width)
{
	eCSR_BW_Val hw_bw = HALF_BW_OF(eCSR_BW_20MHz_VAL);

	switch (ch_width) {
	case CH_WIDTH_40MHZ:
		hw_bw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
		break;
	case CH_WIDTH_80MHZ:
		hw_bw = HALF_BW_OF(eCSR_BW_80MHz_VAL);
		break;
	case CH_WIDTH_160MHZ:
	case CH_WIDTH_80P80MHZ:
		hw_bw = HALF_BW_OF(eCSR_BW_160MHz_VAL);
		break;
	default:
		break;
	}

	return hw_bw;
}

/**
 * csr_handle_conc_chnl_overlap_for_sap_go - To handle overlap for AP+AP
 * @mac_ctx: pointer to mac context
 * @session: Current session
 * @sap_ch_freq: SAP/GO operating channel frequency
 * @sap_hbw: SAP/GO half bw
 * @sap_cfreq: SAP/GO channel frequency
 * @intf_ch_freq: concurrent SAP/GO operating channel frequency
 * @intf_hbw: concurrent SAP/GO half bw
 * @intf_cfreq: concurrent SAP/GO channel frequency
 * @op_mode: opmode
 *
 * This routine is called to check if one SAP/GO channel is overlapping with
 * other SAP/GO channel
 *
 * Return: none
 */
static void csr_handle_conc_chnl_overlap_for_sap_go(
		struct mac_context *mac_ctx,
		struct csr_roam_session *session,
		uint32_t *sap_ch_freq, uint32_t *sap_hbw, uint32_t *sap_cfreq,
		uint32_t *intf_ch_freq, uint32_t *intf_hbw,
		uint32_t *intf_cfreq, enum QDF_OPMODE op_mode)
{
	qdf_freq_t op_chan_freq;
	qdf_freq_t freq_seg_0;
	enum phy_ch_width ch_width;

	wlan_get_op_chan_freq_info_vdev_id(mac_ctx->pdev, session->vdev_id,
					   &op_chan_freq, &freq_seg_0,
					   &ch_width);
	sme_debug("op_chan_freq:%d freq_seg_0:%d ch_width:%d",
		  op_chan_freq, freq_seg_0, ch_width);
	/*
	 * if conc_custom_rule1 is defined then we don't
	 * want p2pgo to follow SAP's channel or SAP to
	 * follow P2PGO's channel.
	 */
	if (0 == mac_ctx->roam.configParam.conc_custom_rule1 &&
		0 == mac_ctx->roam.configParam.conc_custom_rule2) {
		if (*sap_ch_freq == 0) {
			*sap_ch_freq = op_chan_freq;
			*sap_cfreq = freq_seg_0;
			*sap_hbw = csr_get_half_bw(ch_width);
		} else if (*sap_ch_freq != op_chan_freq) {
			*intf_ch_freq = op_chan_freq;
			*intf_cfreq = freq_seg_0;
			*intf_hbw = csr_get_half_bw(ch_width);
		}
	} else if (*sap_ch_freq == 0 && op_mode == QDF_SAP_MODE) {
		*sap_ch_freq = op_chan_freq;
		*sap_cfreq = freq_seg_0;
		*sap_hbw = csr_get_half_bw(ch_width);
	}
}


/**
 * csr_check_concurrent_channel_overlap() - To check concurrent overlap chnls
 * @mac_ctx: Pointer to mac context
 * @sap_ch: SAP channel
 * @sap_phymode: SAP phy mode
 * @cc_switch_mode: concurrent switch mode
 *
 * This routine will be called to check concurrent overlap channels
 *
 * Return: uint16_t
 */
uint16_t csr_check_concurrent_channel_overlap(struct mac_context *mac_ctx,
			uint32_t sap_ch_freq, eCsrPhyMode sap_phymode,
			uint8_t cc_switch_mode, uint8_t vdev_id)
{
	struct csr_roam_session *session = NULL;
	uint8_t i = 0, chb = PHY_SINGLE_CHANNEL_CENTERED;
	uint32_t intf_ch_freq = 0, sap_hbw = 0, intf_hbw = 0, intf_cfreq = 0;
	uint32_t sap_cfreq = 0;
	uint32_t sap_lfreq, sap_hfreq, intf_lfreq, intf_hfreq;
	QDF_STATUS status;
	enum QDF_OPMODE op_mode;
	enum phy_ch_width ch_width;

	if (mac_ctx->roam.configParam.cc_switch_mode ==
			QDF_MCC_TO_SCC_SWITCH_DISABLE)
		return 0;

	if (sap_ch_freq != 0) {
		sap_cfreq = sap_ch_freq;
		sap_hbw = HALF_BW_OF(eCSR_BW_20MHz_VAL);

		if (!WLAN_REG_IS_24GHZ_CH_FREQ(sap_ch_freq))
			chb = mac_ctx->roam.configParam.channelBondingMode5GHz;
		else
			chb = mac_ctx->roam.configParam.channelBondingMode24GHz;

		if (chb)
			csr_calc_chb_for_sap_phymode(mac_ctx, &sap_ch_freq,
						     &sap_phymode, &sap_cfreq,
						     &sap_hbw, &chb);
	}

	sme_debug("sap_ch:%d sap_phymode:%d sap_cch:%d sap_hbw:%d chb:%d",
		  sap_ch_freq, sap_phymode, sap_cfreq, sap_hbw, chb);

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;

		session = CSR_GET_SESSION(mac_ctx, i);
		op_mode = wlan_get_opmode_from_vdev_id(mac_ctx->pdev, i);
		if ((op_mode == QDF_STA_MODE ||
		     op_mode == QDF_P2P_CLIENT_MODE) &&
		    cm_is_vdevid_connected(mac_ctx->pdev, i)) {
			wlan_get_op_chan_freq_info_vdev_id(mac_ctx->pdev,
					   session->vdev_id,
					   &intf_ch_freq, &intf_cfreq,
					   &ch_width);
			intf_hbw = csr_get_half_bw(ch_width);
			sme_debug("%d: intf_ch:%d intf_cfreq:%d intf_hbw:%d ch_width %d",
				  i, intf_ch_freq, intf_cfreq, intf_hbw,
				  ch_width);
		} else if ((op_mode == QDF_P2P_GO_MODE ||
			    op_mode == QDF_SAP_MODE) &&
			   (session->connectState !=
			     eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED)) {

			if (session->ch_switch_in_progress)
				continue;

			csr_handle_conc_chnl_overlap_for_sap_go(mac_ctx,
					session, &sap_ch_freq, &sap_hbw,
					&sap_cfreq, &intf_ch_freq, &intf_hbw,
					&intf_cfreq, op_mode);
		}
		if (intf_ch_freq &&
		    ((intf_ch_freq <= wlan_reg_ch_to_freq(CHAN_ENUM_2484) &&
		     sap_ch_freq <= wlan_reg_ch_to_freq(CHAN_ENUM_2484)) ||
		    (intf_ch_freq > wlan_reg_ch_to_freq(CHAN_ENUM_2484) &&
		     sap_ch_freq > wlan_reg_ch_to_freq(CHAN_ENUM_2484))))
			break;
	}

	sme_debug("intf_ch:%d sap_ch:%d cc_switch_mode:%d, dbs:%d",
		  intf_ch_freq, sap_ch_freq, cc_switch_mode,
		  policy_mgr_is_hw_dbs_capable(mac_ctx->psoc));

	if (intf_ch_freq && sap_ch_freq != intf_ch_freq &&
	    !policy_mgr_is_force_scc(mac_ctx->psoc)) {
		sap_lfreq = sap_cfreq - sap_hbw;
		sap_hfreq = sap_cfreq + sap_hbw;
		intf_lfreq = intf_cfreq - intf_hbw;
		intf_hfreq = intf_cfreq + intf_hbw;

		sme_debug("SAP:  OCH: %03d CCH: %03d BW: %d LF: %d HF: %d INTF: OCH: %03d CF: %d BW: %d LF: %d HF: %d",
			sap_ch_freq, sap_cfreq, sap_hbw * 2,
			sap_lfreq, sap_hfreq, intf_ch_freq,
			intf_cfreq, intf_hbw * 2, intf_lfreq, intf_hfreq);

		if (!(((sap_lfreq > intf_lfreq && sap_lfreq < intf_hfreq) ||
			(sap_hfreq > intf_lfreq && sap_hfreq < intf_hfreq)) ||
			((intf_lfreq > sap_lfreq && intf_lfreq < sap_hfreq) ||
			(intf_hfreq > sap_lfreq && intf_hfreq < sap_hfreq))))
			intf_ch_freq = 0;
	} else if (intf_ch_freq && sap_ch_freq != intf_ch_freq &&
		   (policy_mgr_is_force_scc(mac_ctx->psoc))) {
		policy_mgr_check_scc_sbs_channel(mac_ctx->psoc, &intf_ch_freq,
						 sap_ch_freq, vdev_id,
						 cc_switch_mode);
	} else if ((intf_ch_freq == sap_ch_freq) && (cc_switch_mode ==
				QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL)) {
		if (WLAN_REG_IS_24GHZ_CH_FREQ(intf_ch_freq) ||
		    WLAN_REG_IS_6GHZ_CHAN_FREQ(sap_ch_freq)) {
			status =
				policy_mgr_get_sap_mandatory_channel(
					mac_ctx->psoc, sap_ch_freq,
					&intf_ch_freq, vdev_id);
			if (QDF_IS_STATUS_ERROR(status))
				sme_err("no mandatory channel");
		}
	}

	if (intf_ch_freq == sap_ch_freq)
		intf_ch_freq = 0;

	sme_debug("##Concurrent Channels (%d, %d) %s Interfering", sap_ch_freq,
		  intf_ch_freq,
		  intf_ch_freq == 0 ? "Not" : "Are");

	return intf_ch_freq;
}
#endif

bool csr_is_all_session_disconnected(struct mac_context *mac)
{
	uint32_t i;
	bool fRc = true;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (CSR_IS_SESSION_VALID(mac, i)
		    && !csr_is_conn_state_disconnected(mac, i)) {
			fRc = false;
			break;
		}
	}

	return fRc;
}

bool csr_is_infra_ap_started(struct mac_context *mac)
{
	uint32_t sessionId;
	bool fRc = false;

	for (sessionId = 0; sessionId < WLAN_MAX_VDEVS; sessionId++) {
		if (CSR_IS_SESSION_VALID(mac, sessionId) &&
				(csr_is_conn_state_connected_infra_ap(mac,
					sessionId))) {
			fRc = true;
			break;
		}
	}

	return fRc;

}

bool csr_is_conn_state_disconnected(struct mac_context *mac, uint8_t vdev_id)
{
	enum QDF_OPMODE opmode;

	opmode = wlan_get_opmode_from_vdev_id(mac->pdev, vdev_id);

	if (opmode == QDF_STA_MODE || opmode == QDF_P2P_CLIENT_MODE)
		return !cm_is_vdevid_connected(mac->pdev, vdev_id);

	return eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED ==
	       mac->roam.roamSession[vdev_id].connectState;
}

bool csr_is11h_supported(struct mac_context *mac)
{
	return mac->mlme_cfg->gen.enabled_11h;
}

bool csr_is_wmm_supported(struct mac_context *mac)
{
	if (WMM_USER_MODE_NO_QOS == mac->roam.configParam.WMMSupportMode)
		return false;
	else
		return true;
}

/* This function will allocate memory for the parsed IEs to the caller.
 * Caller must free the memory after it is done with the data only if
 * this function succeeds
 */
QDF_STATUS csr_get_parsed_bss_description_ies(struct mac_context *mac_ctx,
					      struct bss_description *bss_desc,
					      tDot11fBeaconIEs **ppIEStruct)
{
	return wlan_get_parsed_bss_description_ies(mac_ctx, bss_desc,
						   ppIEStruct);
}

uint32_t csr_get_frag_thresh(struct mac_context *mac_ctx)
{
	return mac_ctx->mlme_cfg->threshold.frag_threshold;
}

uint32_t csr_get_rts_thresh(struct mac_context *mac_ctx)
{
	return mac_ctx->mlme_cfg->threshold.rts_threshold;
}

uint32_t csr_translate_to_wni_cfg_dot11_mode(struct mac_context *mac,
					     enum csr_cfgdot11mode csrDot11Mode)
{
	uint32_t ret;

	switch (csrDot11Mode) {
	case eCSR_CFG_DOT11_MODE_AUTO:
#ifdef WLAN_FEATURE_11BE
		if (IS_FEATURE_11BE_SUPPORTED_BY_FW)
			ret = MLME_DOT11_MODE_11BE;
		else
#endif
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11A:
		ret = MLME_DOT11_MODE_11A;
		break;
	case eCSR_CFG_DOT11_MODE_11B:
		ret = MLME_DOT11_MODE_11B;
		break;
	case eCSR_CFG_DOT11_MODE_11G:
		ret = MLME_DOT11_MODE_11G;
		break;
	case eCSR_CFG_DOT11_MODE_11N:
		ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11G_ONLY:
		ret = MLME_DOT11_MODE_11G_ONLY;
		break;
	case eCSR_CFG_DOT11_MODE_11N_ONLY:
		ret = MLME_DOT11_MODE_11N_ONLY;
		break;
	case eCSR_CFG_DOT11_MODE_11AC_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC_ONLY;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AC:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AX_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AX:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
#ifdef WLAN_FEATURE_11BE
	case eCSR_CFG_DOT11_MODE_11BE_ONLY:
		if (IS_FEATURE_11BE_SUPPORTED_BY_FW)
			ret = MLME_DOT11_MODE_11BE_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11BE:
		if (IS_FEATURE_11BE_SUPPORTED_BY_FW)
			ret = MLME_DOT11_MODE_11BE;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
#endif
	case eCSR_CFG_DOT11_MODE_ABG:
		ret = MLME_DOT11_MODE_ABG;
		break;
	default:
		sme_warn("doesn't expect %d as csrDo11Mode", csrDot11Mode);
		ret = MLME_DOT11_MODE_ALL;
		break;
	}

	return ret;
}

enum reg_phymode csr_convert_to_reg_phy_mode(eCsrPhyMode csr_phy_mode,
				       qdf_freq_t freq)
{
	if (csr_phy_mode == eCSR_DOT11_MODE_AUTO)
		return REG_PHYMODE_MAX - 1;
#ifdef WLAN_FEATURE_11BE
	else if (CSR_IS_DOT11_PHY_MODE_11BE(csr_phy_mode) ||
		 CSR_IS_DOT11_PHY_MODE_11BE_ONLY(csr_phy_mode))
		return REG_PHYMODE_11BE;
#endif
	else if (csr_phy_mode == eCSR_DOT11_MODE_11ax ||
		 csr_phy_mode == eCSR_DOT11_MODE_11ax_ONLY)
		return REG_PHYMODE_11AX;
	else if (csr_phy_mode == eCSR_DOT11_MODE_11ac ||
		 csr_phy_mode == eCSR_DOT11_MODE_11ac_ONLY)
		return REG_PHYMODE_11AC;
	else if (csr_phy_mode == eCSR_DOT11_MODE_11n ||
		 csr_phy_mode == eCSR_DOT11_MODE_11n_ONLY)
		return REG_PHYMODE_11N;
	else if (csr_phy_mode == eCSR_DOT11_MODE_11a)
		return REG_PHYMODE_11A;
	else if (csr_phy_mode == eCSR_DOT11_MODE_11g ||
		 csr_phy_mode == eCSR_DOT11_MODE_11g_ONLY)
		return REG_PHYMODE_11G;
	else if (csr_phy_mode == eCSR_DOT11_MODE_11b ||
		 csr_phy_mode == eCSR_DOT11_MODE_11b_ONLY)
		return REG_PHYMODE_11B;
	else if (csr_phy_mode == eCSR_DOT11_MODE_abg) {
		if (WLAN_REG_IS_24GHZ_CH_FREQ(freq))
			return REG_PHYMODE_11G;
		else
			return REG_PHYMODE_11A;
	} else {
		sme_err("Invalid eCsrPhyMode");
		return REG_PHYMODE_INVALID;
	}
}

eCsrPhyMode csr_convert_from_reg_phy_mode(enum reg_phymode phymode)
{
	switch (phymode) {
	case REG_PHYMODE_INVALID:
		return eCSR_DOT11_MODE_AUTO;
	case REG_PHYMODE_11B:
		return eCSR_DOT11_MODE_11b;
	case REG_PHYMODE_11G:
		return eCSR_DOT11_MODE_11g;
	case REG_PHYMODE_11A:
		return eCSR_DOT11_MODE_11a;
	case REG_PHYMODE_11N:
		return eCSR_DOT11_MODE_11n;
	case REG_PHYMODE_11AC:
		return eCSR_DOT11_MODE_11ac;
	case REG_PHYMODE_11AX:
		return eCSR_DOT11_MODE_11ax;
#ifdef WLAN_FEATURE_11BE
	case REG_PHYMODE_11BE:
		return eCSR_DOT11_MODE_11be;
#endif
	case REG_PHYMODE_MAX:
		return eCSR_DOT11_MODE_AUTO;
	default:
		return eCSR_DOT11_MODE_AUTO;
	}
}

bool csr_is_auth_type_ese(enum csr_akm_type AuthType)
{
	switch (AuthType) {
	case eCSR_AUTH_TYPE_CCKM_WPA:
	case eCSR_AUTH_TYPE_CCKM_RSN:
		return true;
	default:
		break;
	}
	return false;
}

bool csr_is_pmkid_found_for_peer(struct mac_context *mac,
				 struct csr_roam_session *session,
				 tSirMacAddr peer_mac_addr,
				 uint8_t *pmkid,
				 uint16_t pmkid_count)
{
	uint32_t i;
	uint8_t *session_pmkid;
	struct wlan_crypto_pmksa *pmkid_cache;

	pmkid_cache = qdf_mem_malloc(sizeof(*pmkid_cache));
	if (!pmkid_cache)
		return false;

	qdf_mem_copy(pmkid_cache->bssid.bytes, peer_mac_addr,
		     QDF_MAC_ADDR_SIZE);

	if (!cm_lookup_pmkid_using_bssid(mac->psoc, session->vdev_id,
					 pmkid_cache)) {
		qdf_mem_free(pmkid_cache);
		return false;
	}

	session_pmkid = pmkid_cache->pmkid;
	for (i = 0; i < pmkid_count; i++) {
		if (!qdf_mem_cmp(pmkid + (i * PMKID_LEN),
				 session_pmkid, PMKID_LEN)) {
			qdf_mem_free(pmkid_cache);
			return true;
		}
	}

	sme_debug("PMKID in PmkidCacheInfo doesn't match with PMKIDs of peer");
	qdf_mem_free(pmkid_cache);

	return false;
}

bool csr_is_bssid_match(struct qdf_mac_addr *pProfBssid,
			struct qdf_mac_addr *BssBssid)
{
	bool fMatch = false;
	struct qdf_mac_addr ProfileBssid;

	/* for efficiency of the MAC_ADDRESS functions, move the */
	/* Bssid's into MAC_ADDRESS structs. */
	qdf_mem_copy(&ProfileBssid, pProfBssid, sizeof(struct qdf_mac_addr));

	do {
		/* Give the profile the benefit of the doubt... accept
		 * either all 0 or the real broadcast Bssid (all 0xff)
		 * as broadcast Bssids (meaning to match any Bssids).
		 */
		if (qdf_is_macaddr_zero(&ProfileBssid) ||
		    qdf_is_macaddr_broadcast(&ProfileBssid)) {
			fMatch = true;
			break;
		}

		if (qdf_is_macaddr_equal(BssBssid, &ProfileBssid)) {
			fMatch = true;
			break;
		}

	} while (0);

	return fMatch;
}

void csr_release_profile(struct mac_context *mac,
			 struct csr_roam_profile *pProfile)
{
	if (pProfile) {
		if (pProfile->BSSIDs.bssid) {
			qdf_mem_free(pProfile->BSSIDs.bssid);
			pProfile->BSSIDs.bssid = NULL;
		}
		if (pProfile->SSIDs.SSIDList) {
			qdf_mem_free(pProfile->SSIDs.SSIDList);
			pProfile->SSIDs.SSIDList = NULL;
		}

		if (pProfile->ChannelInfo.freq_list) {
			qdf_mem_free(pProfile->ChannelInfo.freq_list);
			pProfile->ChannelInfo.freq_list = NULL;
		}
		if (pProfile->pRSNReqIE) {
			qdf_mem_free(pProfile->pRSNReqIE);
			pProfile->pRSNReqIE = NULL;
		}
		qdf_mem_zero(pProfile, sizeof(struct csr_roam_profile));
	}
}

enum bss_type csr_translate_bsstype_to_mac_type(eCsrRoamBssType csrtype)
{
	enum bss_type ret;

	switch (csrtype) {
	case eCSR_BSS_TYPE_INFRASTRUCTURE:
		ret = eSIR_INFRASTRUCTURE_MODE;
		break;
	case eCSR_BSS_TYPE_INFRA_AP:
		ret = eSIR_INFRA_AP_MODE;
		break;
	case eCSR_BSS_TYPE_NDI:
		ret = eSIR_NDI_MODE;
		break;
	case eCSR_BSS_TYPE_ANY:
	default:
		ret = eSIR_AUTO_MODE;
		break;
	}

	return ret;
}

/* This function use the parameters to decide the CFG value. */
/* CSR never sets MLME_DOT11_MODE_ALL to the CFG */
/* So PE should not see MLME_DOT11_MODE_ALL when it gets the CFG value */
enum csr_cfgdot11mode
csr_get_cfg_dot11_mode_from_csr_phy_mode(struct csr_roam_profile *pProfile,
					 eCsrPhyMode phyMode,
					 bool fProprietary)
{
	uint32_t cfgDot11Mode = eCSR_CFG_DOT11_MODE_ABG;

	switch (phyMode) {
	case eCSR_DOT11_MODE_11a:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
		break;
	case eCSR_DOT11_MODE_11b:
	case eCSR_DOT11_MODE_11b_ONLY:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
		break;
	case eCSR_DOT11_MODE_11g:
	case eCSR_DOT11_MODE_11g_ONLY:
		if (pProfile && (CSR_IS_INFRA_AP(pProfile))
		    && (phyMode == eCSR_DOT11_MODE_11g_ONLY))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		break;
	case eCSR_DOT11_MODE_11n:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11n_ONLY:
		if (pProfile && CSR_IS_INFRA_AP(pProfile))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_abg:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_ABG;
		break;
	case eCSR_DOT11_MODE_AUTO:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_AUTO;
		break;

	case eCSR_DOT11_MODE_11ac:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ac_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ax:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ax_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
#ifdef WLAN_FEATURE_11BE
	case eCSR_DOT11_MODE_11be:
		if (IS_FEATURE_11BE_SUPPORTED_BY_FW)
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11BE;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11be_ONLY:
		if (IS_FEATURE_11BE_SUPPORTED_BY_FW)
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11BE_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
#endif
	default:
		/* No need to assign anything here */
		break;
	}

	return cfgDot11Mode;
}

QDF_STATUS csr_get_modify_profile_fields(struct mac_context *mac,
					uint32_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	if (!pModifyProfileFields)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(pModifyProfileFields,
		     &mac->roam.roamSession[sessionId].modifyProfileFields,
		     sizeof(tCsrRoamModifyProfileFields));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_set_modify_profile_fields(struct mac_context *mac,
					uint32_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	if (!pSession) {
		sme_err("Session_id invalid %d", sessionId);
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(&pSession->modifyProfileFields,
		     pModifyProfileFields, sizeof(tCsrRoamModifyProfileFields));

	return QDF_STATUS_SUCCESS;
}

/* no need to acquire lock for this basic function */
uint16_t sme_chn_to_freq(uint8_t chanNum)
{
	int i;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (WLAN_REG_CH_NUM(i) == chanNum)
			return WLAN_REG_CH_TO_FREQ(i);
	}

	return 0;
}

struct lim_channel_status *
csr_get_channel_status(struct mac_context *mac, uint32_t chan_freq)
{
	uint8_t i;
	struct lim_scan_channel_status *channel_status;
	struct lim_channel_status *entry;

	if (!mac->sap.acs_with_more_param)
		return NULL;

	channel_status = &mac->lim.scan_channel_status;
	for (i = 0; i < channel_status->total_channel; i++) {
		entry = &channel_status->channel_status_list[i];
		if (entry->channelfreq == chan_freq)
			return entry;
	}
	sme_err("Channel %d status info not exist", chan_freq);

	return NULL;
}

void csr_clear_channel_status(struct mac_context *mac)
{
	struct lim_scan_channel_status *channel_status;

	if (!mac->sap.acs_with_more_param)
		return;

	channel_status = &mac->lim.scan_channel_status;
	channel_status->total_channel = 0;

	return;
}

/**
 * sme_bsstype_to_string() - converts bss type to string.
 * @bss_type: bss type enum
 *
 * Return: printable string for bss type
 */
const char *sme_bss_type_to_string(const uint8_t bss_type)
{
	switch (bss_type) {
	CASE_RETURN_STRING(eCSR_BSS_TYPE_INFRASTRUCTURE);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_INFRA_AP);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_ANY);
	default:
		return "unknown bss type";
	}
}

/**
 * csr_is_ndi_started() - function to check if NDI is started
 * @mac_ctx: handle to mac context
 * @session_id: session identifier
 *
 * returns: true if NDI is started, false otherwise
 */
bool csr_is_ndi_started(struct mac_context *mac_ctx, uint32_t session_id)
{
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session)
		return false;

	return eCSR_CONNECT_STATE_TYPE_NDI_STARTED == session->connectState;
}

bool csr_is_mcc_channel(struct mac_context *mac_ctx, uint32_t chan_freq)
{
	struct csr_roam_session *session;
	enum QDF_OPMODE oper_mode;
	uint32_t oper_chan_freq = 0;
	uint8_t vdev_id;
	bool hw_dbs_capable, same_band_freqs;

	if (chan_freq == 0)
		return false;

	hw_dbs_capable = policy_mgr_is_hw_dbs_capable(mac_ctx->psoc);
	for (vdev_id = 0; vdev_id < WLAN_MAX_VDEVS; vdev_id++) {
		if (!CSR_IS_SESSION_VALID(mac_ctx, vdev_id))
			continue;

		session = CSR_GET_SESSION(mac_ctx, vdev_id);
		oper_mode =
			wlan_get_opmode_from_vdev_id(mac_ctx->pdev, vdev_id);
		if ((((oper_mode == QDF_STA_MODE) ||
		     (oper_mode == QDF_P2P_CLIENT_MODE)) &&
		    cm_is_vdevid_connected(mac_ctx->pdev, vdev_id)) ||
		    (((oper_mode == QDF_P2P_GO_MODE) ||
		      (oper_mode == QDF_SAP_MODE)) &&
		     (session->connectState !=
		      eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED)))
			oper_chan_freq =
			    wlan_get_operation_chan_freq_vdev_id(mac_ctx->pdev,
								 vdev_id);

		if (!oper_chan_freq)
			continue;
		same_band_freqs = WLAN_REG_IS_SAME_BAND_FREQS(
			chan_freq, oper_chan_freq);

		if (oper_chan_freq && chan_freq != oper_chan_freq &&
		    (!hw_dbs_capable || same_band_freqs))
			return true;
	}

	return false;
}

enum csr_cfgdot11mode csr_phy_mode_to_dot11mode(enum wlan_phymode phy_mode)
{
	switch (phy_mode) {
	case WLAN_PHYMODE_AUTO:
		return eCSR_CFG_DOT11_MODE_AUTO;
	case WLAN_PHYMODE_11A:
		return eCSR_CFG_DOT11_MODE_11A;
	case WLAN_PHYMODE_11B:
		return eCSR_CFG_DOT11_MODE_11B;
	case WLAN_PHYMODE_11G:
		return eCSR_CFG_DOT11_MODE_11G;
	case WLAN_PHYMODE_11G_ONLY:
		return eCSR_CFG_DOT11_MODE_11G_ONLY;
	case WLAN_PHYMODE_11NA_HT20:
	case WLAN_PHYMODE_11NG_HT20:
	case WLAN_PHYMODE_11NA_HT40:
	case WLAN_PHYMODE_11NG_HT40PLUS:
	case WLAN_PHYMODE_11NG_HT40MINUS:
	case WLAN_PHYMODE_11NG_HT40:
		return eCSR_CFG_DOT11_MODE_11N;
	case WLAN_PHYMODE_11AC_VHT20:
	case WLAN_PHYMODE_11AC_VHT20_2G:
	case WLAN_PHYMODE_11AC_VHT40:
	case WLAN_PHYMODE_11AC_VHT40PLUS_2G:
	case WLAN_PHYMODE_11AC_VHT40MINUS_2G:
	case WLAN_PHYMODE_11AC_VHT40_2G:
	case WLAN_PHYMODE_11AC_VHT80:
	case WLAN_PHYMODE_11AC_VHT80_2G:
	case WLAN_PHYMODE_11AC_VHT160:
	case WLAN_PHYMODE_11AC_VHT80_80:
		return eCSR_CFG_DOT11_MODE_11AC;
	case WLAN_PHYMODE_11AXA_HE20:
	case WLAN_PHYMODE_11AXG_HE20:
	case WLAN_PHYMODE_11AXA_HE40:
	case WLAN_PHYMODE_11AXG_HE40PLUS:
	case WLAN_PHYMODE_11AXG_HE40MINUS:
	case WLAN_PHYMODE_11AXG_HE40:
	case WLAN_PHYMODE_11AXA_HE80:
	case WLAN_PHYMODE_11AXG_HE80:
	case WLAN_PHYMODE_11AXA_HE160:
	case WLAN_PHYMODE_11AXA_HE80_80:
		return eCSR_CFG_DOT11_MODE_11AX;
#ifdef WLAN_FEATURE_11BE
	case WLAN_PHYMODE_11BEA_EHT20:
	case WLAN_PHYMODE_11BEG_EHT20:
	case WLAN_PHYMODE_11BEA_EHT40:
	case WLAN_PHYMODE_11BEG_EHT40PLUS:
	case WLAN_PHYMODE_11BEG_EHT40MINUS:
	case WLAN_PHYMODE_11BEG_EHT40:
	case WLAN_PHYMODE_11BEA_EHT80:
	case WLAN_PHYMODE_11BEG_EHT80:
	case WLAN_PHYMODE_11BEA_EHT160:
	case WLAN_PHYMODE_11BEA_EHT320:
		return eCSR_CFG_DOT11_MODE_11BE;
#endif
	default:
		sme_err("invalid phy mode %d", phy_mode);
		return eCSR_CFG_DOT11_MODE_MAX;
	}
}

QDF_STATUS csr_mlme_vdev_disconnect_all_p2p_client_event(uint8_t vdev_id)
{
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_SME);

	if (!mac_ctx)
		return QDF_STATUS_E_FAILURE;

	return csr_roam_call_callback(mac_ctx, vdev_id, NULL, 0,
				      eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS,
				      eCSR_ROAM_RESULT_NONE);
}

QDF_STATUS csr_mlme_vdev_stop_bss(uint8_t vdev_id)
{
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_SME);

	if (!mac_ctx)
		return QDF_STATUS_E_FAILURE;

	return csr_roam_call_callback(mac_ctx, vdev_id, NULL, 0,
				      eCSR_ROAM_SEND_P2P_STOP_BSS,
				      eCSR_ROAM_RESULT_NONE);
}

qdf_freq_t csr_mlme_get_concurrent_operation_freq(void)
{
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_SME);

	if (!mac_ctx)
		return QDF_STATUS_E_FAILURE;

	return csr_get_concurrent_operation_freq(mac_ctx);
}
