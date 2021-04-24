/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * DOC: csr_neighbor_roam.c
 *
 * Implementation for the simple roaming algorithm for 802.11r Fast
 * transitions and Legacy roaming for Android platform.
 */

#ifndef FEATURE_CM_ENABLE

#include "wma_types.h"
#include "csr_inside_api.h"
#include "sme_qos_internal.h"
#include "sme_inside.h"
#include "host_diag_core_event.h"
#include "host_diag_core_log.h"
#include "csr_api.h"
#include "sme_api.h"
#include "csr_neighbor_roam.h"
#include "mac_trace.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_mlme_api.h"

void csr_neighbor_roam_state_transition(struct mac_context *mac_ctx,
		uint8_t newstate, uint8_t session)
{
	mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState =
		mac_ctx->roam.neighborRoamInfo[session].neighborRoamState;
	mac_ctx->roam.neighborRoamInfo[session].neighborRoamState = newstate;
	sme_nofl_debug("NeighborRoam transition: vdev %d %s --> %s",
		session, csr_neighbor_roam_state_to_string(
		mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState),
		csr_neighbor_roam_state_to_string(newstate));
}

uint8_t *csr_neighbor_roam_state_to_string(uint8_t state)
{
	switch (state) {
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_CLOSED);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_INIT);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_CONNECTED);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE);
	default:
		return "eCSR_NEIGHBOR_ROAM_STATE_UNKNOWN";
	}

}

#ifdef FEATURE_WLAN_LFR_METRICS
/**
 * csr_neighbor_roam_send_lfr_metric_event() - Send event of LFR metric
 * @mac_ctx: Handle returned by mac_open.
 * @session_id: Session information
 * @bssid: BSSID of attempted AP
 * @status: Result of LFR operation
 *
 * LFR metrics - pre-auth completion metric
 * Send the event to supplicant indicating pre-auth result
 *
 * Return: void
 */

void csr_neighbor_roam_send_lfr_metric_event(
				struct mac_context *mac_ctx,
				uint8_t session_id,
				tSirMacAddr bssid,
				eRoamCmdStatus status)
{
	struct csr_roam_info *roam_info;

	roam_info = qdf_mem_malloc(sizeof(struct csr_roam_info));
	if (roam_info) {
		qdf_mem_copy((void *)roam_info->bssid.bytes,
			     (void *)bssid, sizeof(struct qdf_mac_addr));
		csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
			status, 0);
		qdf_mem_free(roam_info);
	}
}
#endif

/**
 * csr_neighbor_roam_reset_connected_state_control_info()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @sessionId : session id
 *
 * This function will reset the neighbor roam control info data structures.
 * This function should be invoked whenever we move to CONNECTED state from
 * any state other than INIT state
 *
 * Return: None
 */
static void csr_neighbor_roam_reset_connected_state_control_info(
							struct mac_context *mac,
							uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct rso_chan_info *chan_lst;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("vdev object is NULL");
		return;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
		return;
	}

	chan_lst = &rso_cfg->roam_scan_freq_lst;
	if (chan_lst->freq_list)
		qdf_mem_free(chan_lst->freq_list);
	chan_lst->freq_list = NULL;
	chan_lst->num_chan = 0;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	csr_neighbor_roam_free_roamable_bss_list(mac,
					&pNeighborRoamInfo->roamableAPList);

	/* Do not free up the preauth done list here */
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	pNeighborRoamInfo->FTRoamInfo.neighborRptPending = false;
	pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;
	pNeighborRoamInfo->FTRoamInfo.preauthRspPending = 0;
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	qdf_mem_zero(&pNeighborRoamInfo->handoffReqInfo,
		     sizeof(tCsrHandoffRequest));
}

static void csr_neighbor_roam_reset_report_scan_state_control_info(
							struct mac_context *mac,
							uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];

	qdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);
#ifdef FEATURE_WLAN_ESE
	wlan_cm_set_ese_assoc(mac->pdev, sessionId, false);
	pNeighborRoamInfo->isVOAdmitted = false;
	pNeighborRoamInfo->MinQBssLoadRequired = 0;
#endif

	/* Purge roamable AP list */
	csr_neighbor_roam_free_roamable_bss_list(mac,
					&pNeighborRoamInfo->roamableAPList);
}

/**
 * csr_neighbor_roam_reset_init_state_control_info()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @sessionId : session id
 *
 * This function will reset the neighbor roam control info data structures.
 * This function should be invoked whenever we move to CONNECTED state from
 * INIT state
 *
 * Return: None
 */
static void csr_neighbor_roam_reset_init_state_control_info(struct mac_context *mac,
							    uint8_t sessionId)
{
	csr_neighbor_roam_reset_connected_state_control_info(mac, sessionId);

	/* In addition to the above resets,
	 * we should clear off the curAPBssId/Session ID in the timers
	 */
	csr_neighbor_roam_reset_report_scan_state_control_info(mac, sessionId);
}

QDF_STATUS
csr_neighbor_roam_get_scan_filter_from_profile(struct mac_context *mac,
					       struct scan_filter *filter,
					       uint8_t vdev_id)
{
	tpCsrNeighborRoamControlInfo nbr_roam_info;
	tCsrRoamConnectedProfile *profile;
	uint8_t num_ch = 0;
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct rso_chan_info *chan_lst;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct rso_config_params *rso_usr_cfg;

	mlme_obj = mlme_get_psoc_ext_obj(mac->psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	rso_usr_cfg = &mlme_obj->cfg.lfr.rso_user_config;

	if (!filter)
		return QDF_STATUS_E_FAILURE;
	if (!CSR_IS_SESSION_VALID(mac, vdev_id))
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(filter, sizeof(*filter));
	nbr_roam_info = &mac->roam.neighborRoamInfo[vdev_id];
	profile = &mac->roam.roamSession[vdev_id].connectedProfile;

	/* only for HDD requested handoff fill in the BSSID in the filter */
	if (nbr_roam_info->uOsRequestedHandoff) {
		sme_debug("OS Requested Handoff");
		filter->num_of_bssid = 1;
		qdf_mem_copy(filter->bssid_list[0].bytes,
			     &nbr_roam_info->handoffReqInfo.bssid.bytes,
			     QDF_MAC_ADDR_SIZE);
	}
	sme_debug("No of Allowed SSID List:%d",
		  rso_usr_cfg->num_ssid_allowed_list);

	if (rso_usr_cfg->num_ssid_allowed_list) {
		csr_copy_ssids_from_roam_params(rso_usr_cfg, filter);
	} else {
		filter->num_of_ssid = 1;

		wlan_mlme_get_ssid_vdev_id(mac->pdev, vdev_id,
					   filter->ssid_list[0].ssid,
					   &filter->ssid_list[0].length);
		sme_debug("Filtering for SSID %.*s,length of SSID = %u",
			  filter->ssid_list[0].length,
			  filter->ssid_list[0].ssid,
			  filter->ssid_list[0].length);
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, vdev_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wlan_cm_fill_crypto_filter_from_vdev(vdev, filter);

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
		return QDF_STATUS_E_FAILURE;
	}
	chan_lst = &rso_cfg->roam_scan_freq_lst;
	num_ch = chan_lst->num_chan;
	if (num_ch) {
		filter->num_of_channels = num_ch;
		if (filter->num_of_channels > NUM_CHANNELS)
			filter->num_of_channels = NUM_CHANNELS;
		qdf_mem_copy(filter->chan_freq_list, chan_lst->freq_list,
			     filter->num_of_channels *
			     sizeof(filter->chan_freq_list[0]));
	}

	if (rso_cfg->is_11r_assoc)
		/*
		 * MDIE should be added as a part of profile. This should be
		 * added as a part of filter as well
		 */
		filter->mobility_domain = rso_cfg->mdid.mobility_domain;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
	filter->enable_adaptive_11r =
		wlan_mlme_adaptive_11r_enabled(mac->psoc);
	csr_update_scan_filter_dot11mode(mac, filter);

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_indicate_disconnect() - To indicate disconnect
 * @mac: The handle returned by mac_open
 * @sessionId: CSR session id that got disconnected
 *
 * This function is called by CSR as soon as the station disconnects
 * from the AP. This function does the necessary cleanup of neighbor roam
 * data structures. Neighbor roam state transitions to INIT state whenever
 * this function is called except if the current state is REASSOCIATING
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_neighbor_roam_indicate_disconnect(struct mac_context *mac,
						 uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
			&mac->roam.neighborRoamInfo[sessionId];
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);
	struct wlan_objmgr_vdev *vdev;
	enum QDF_OPMODE opmode;
	struct qdf_mac_addr connected_bssid;

	if (!pSession) {
		sme_err("pSession is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	opmode = wlan_get_opmode_from_vdev_id(mac->pdev, sessionId);
	if (opmode != QDF_STA_MODE) {
		sme_debug("Ignore disconn ind rcvd from nonSTA persona vdev: %d opmode %d",
			  sessionId, opmode);
		return QDF_STATUS_SUCCESS;
	}
	wlan_mlme_get_bssid_vdev_id(mac->pdev, sessionId, &connected_bssid);
	sme_debug("Disconn ind on session %d in state %d from bss :"
			QDF_MAC_ADDR_FMT, sessionId,
			pNeighborRoamInfo->neighborRoamState,
			QDF_MAC_ADDR_REF(connected_bssid.bytes));

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (vdev) {
		csr_update_prev_ap_info(pSession, vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
	}

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
		/*
		 * Stop scan and neighbor refresh timers.
		 * These are indeed not required when we'r in reassoc state.
		 */
		if (!CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(mac, sessionId)) {
			/*
			 * Disconnect indication during Disassoc Handoff
			 * sub-state is received when we are trying to
			 * disconnect with the old AP during roam.
			 * BUT, if receive a disconnect indication outside of
			 * Disassoc Handoff sub-state, then it means that
			 * this is a genuine disconnect and we need to clean up.
			 * Otherwise, we will be stuck in reassoc state which'll
			 * in-turn block scans.
			 */
		csr_neighbor_roam_state_transition(mac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
			pNeighborRoamInfo->uOsRequestedHandoff = 0;
		}
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		csr_neighbor_roam_reset_init_state_control_info(mac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		csr_neighbor_roam_state_transition(mac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
		csr_neighbor_roam_reset_connected_state_control_info(mac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
		/* Stop pre-auth to reassoc interval timer */
		qdf_mc_timer_stop(&pSession->ftSmeContext.
				preAuthReassocIntvlTimer);
		/* fallthrough */
	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
		csr_neighbor_roam_state_transition(mac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
		csr_neighbor_roam_reset_preauth_control_info(mac, sessionId);
		csr_neighbor_roam_reset_report_scan_state_control_info(mac,
				sessionId);
		break;
	default:
		sme_debug("Received disconnect event in state %s",
			mac_trace_get_neighbour_roam_state(
				pNeighborRoamInfo->neighborRoamState));
		sme_debug("Transit to INIT state");
		csr_neighbor_roam_state_transition(mac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
			pNeighborRoamInfo->uOsRequestedHandoff = 0;
		break;
	}

	/*
	 * clear the roaming parameters that are per connection.
	 * For a new connection, they have to be programmed again.
	 */
	if (!csr_neighbor_middle_of_roaming(mac, sessionId)) {
		wlan_roam_reset_roam_params(mac->psoc);
		cm_roam_restore_default_config(mac->pdev, sessionId);
	}

	/*Inform the Firmware to STOP Scanning as the host has a disconnect. */
	if (csr_roam_is_sta_mode(mac, sessionId))
		wlan_cm_roam_state_change(mac->pdev, sessionId,
					  WLAN_ROAM_DEINIT,
					  REASON_DISCONNECTED);

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_info_ctx_init() - Initialize neighbor roam struct
 * @mac: mac context
 * @session_id: Session Id
 *
 * This initializes all the necessary data structures related to the
 * associated AP.
 *
 * Return: QDF status
 */
static void csr_neighbor_roam_info_ctx_init(struct mac_context *mac,
					    uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&mac->roam.neighborRoamInfo[session_id];
	int init_ft_flag = false;

	csr_neighbor_roam_state_transition(mac,
			eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, session_id);
	wlan_mlme_get_bssid_vdev_id(mac->pdev, session_id,
				    &ngbr_roam_info->currAPbssid);
	cm_roam_start_init_on_connect(mac->pdev, session_id);

	/*
	 * Now we can clear the preauthDone that
	 * was saved as we are connected afresh
	 */
	csr_neighbor_roam_free_roamable_bss_list(mac,
		&ngbr_roam_info->FTRoamInfo.preAuthDoneList);
#ifdef FEATURE_WLAN_ESE
	/* Based on the auth scheme tell if we are 11r */
	if (wlan_cm_get_ese_assoc(mac->pdev, session_id)) {
		if (mac->mlme_cfg->lfr.fast_transition_enabled)
			init_ft_flag = true;
	}
#endif
	/* If "FastRoamEnabled" ini is enabled */
	if (csr_roam_is_fast_roam_enabled(mac, session_id))
		init_ft_flag = true;

	if (init_ft_flag) {
		/* Initialize all the data needed for the 11r FT Preauth */
		ngbr_roam_info->FTRoamInfo.currentNeighborRptRetryNum = 0;
		csr_neighbor_roam_purge_preauth_failed_list(mac);
	}

	ngbr_roam_info->uOsRequestedHandoff = 0;
}

/**
* csr_neighbor_roam_indicate_connect()
* @mac: mac context
 * @session_id: Session Id
 * @qdf_status: QDF status
 *
 * This function is called by CSR as soon as the station connects to an AP.
 * This initializes all the necessary data structures related to the
 * associated AP and transitions the state to CONNECTED state
 *
 * Return: QDF status
 */
QDF_STATUS csr_neighbor_roam_indicate_connect(
		struct mac_context *mac, uint8_t session_id,
		QDF_STATUS qdf_status)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&mac->roam.neighborRoamInfo[session_id];
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	struct csr_roam_session *session = &mac->roam.roamSession[session_id];
	struct csr_roam_info *roam_info;
#endif
	enum QDF_OPMODE opmode;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* if session id invalid then we need return failure */
	if (!ngbr_roam_info || !CSR_IS_SESSION_VALID(mac, session_id)
	|| (!mac->roam.roamSession[session_id].pCurRoamProfile)) {
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("Connect ind, vdev id %d in state %s",
		session_id, mac_trace_get_neighbour_roam_state(
			ngbr_roam_info->neighborRoamState));
	opmode = wlan_get_opmode_from_vdev_id(mac->pdev, session_id);

	/* Bail out if this is NOT a STA opmode */
	if (opmode != QDF_STA_MODE) {
		sme_debug("Ignoring Connect ind received from a non STA. vdev: %d, opmode %d",
			  session_id, opmode);
		return QDF_STATUS_SUCCESS;
	}
	/* if a concurrent session is running */
	if ((false ==
		CSR_IS_FASTROAM_IN_CONCURRENCY_INI_FEATURE_ENABLED(mac)) &&
		(csr_is_concurrent_session_running(mac))) {
		sme_err("Ignoring Connect ind. received in multisession %d",
			csr_is_concurrent_session_running(mac));
		return QDF_STATUS_SUCCESS;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(mac->psoc, session_id) &&
	    eSIR_ROAM_AUTH_STATUS_AUTHENTICATED ==
	     session->roam_synch_data->auth_status) {
		sme_debug("LFR3: Authenticated");
		roam_info = qdf_mem_malloc(sizeof(*roam_info));
		if (!roam_info)
			return QDF_STATUS_E_NOMEM;
		wlan_mlme_get_bssid_vdev_id(mac->pdev, session_id,
					    &roam_info->peerMac);
		csr_roam_call_callback(mac, session_id, roam_info, 0,
				       eCSR_ROAM_SET_KEY_COMPLETE,
				       eCSR_ROAM_RESULT_AUTHENTICATED);
		csr_neighbor_roam_reset_init_state_control_info(mac,
								session_id);
		csr_neighbor_roam_info_ctx_init(mac, session_id);
		qdf_mem_free(roam_info);
		return status;
	}
#endif

	switch (ngbr_roam_info->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
		if (QDF_STATUS_SUCCESS != qdf_status) {
			/**
			 * Just transition the state to INIT state.Rest of the
			 * clean up happens when we get next connect indication
			 */
			csr_neighbor_roam_state_transition(mac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, session_id);
			ngbr_roam_info->uOsRequestedHandoff = 0;
			break;
		}
		/* if the status is SUCCESS */
		/* fallthrough */
	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		/* Reset all the data structures here */
		csr_neighbor_roam_reset_init_state_control_info(mac,
			session_id);
		csr_neighbor_roam_info_ctx_init(mac, session_id);
		break;
	default:
		sme_err("Connect evt received in invalid state %s Ignoring",
			mac_trace_get_neighbour_roam_state(
			ngbr_roam_info->neighborRoamState));
		break;
	}
	return status;
}

/*
 * csr_neighbor_roam_init11r_assoc_info -
 * This function initializes 11r related neighbor roam data structures
 *
 * @mac: The handle returned by mac_open.
 *
 * return QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
static QDF_STATUS csr_neighbor_roam_init11r_assoc_info(struct mac_context *mac)
{
	QDF_STATUS status;
	uint8_t i;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	tpCsr11rAssocNeighborInfo pFTRoamInfo = NULL;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		pNeighborRoamInfo = &mac->roam.neighborRoamInfo[i];
		pFTRoamInfo = &pNeighborRoamInfo->FTRoamInfo;

		pFTRoamInfo->neighborReportTimeout =
			CSR_NEIGHBOR_ROAM_REPORT_QUERY_TIMEOUT;
		pFTRoamInfo->neighborRptPending = false;
		pFTRoamInfo->preauthRspPending = false;

		pFTRoamInfo->currentNeighborRptRetryNum = 0;

		status = csr_ll_open(&pFTRoamInfo->preAuthDoneList);
		if (QDF_STATUS_SUCCESS != status) {
			sme_err("LL Open of preauth done AP List failed");
			return QDF_STATUS_E_RESOURCES;
		}
	}
	return status;
}

/*
 * csr_neighbor_roam_init() -
 * This function initializes neighbor roam data structures
 *
 * @mac: The handle returned by mac_open.
 * sessionId: Session identifier
 *
 * Return QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_init(struct mac_context *mac, uint8_t sessionId)
{
	QDF_STATUS status;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];

	pNeighborRoamInfo->neighborRoamState = eCSR_NEIGHBOR_ROAM_STATE_CLOSED;
	pNeighborRoamInfo->prevNeighborRoamState =
		eCSR_NEIGHBOR_ROAM_STATE_CLOSED;

	qdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);

	status = csr_ll_open(&pNeighborRoamInfo->roamableAPList);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("LL Open of roam able AP List failed");
		return QDF_STATUS_E_RESOURCES;
	}

	status = csr_neighbor_roam_init11r_assoc_info(mac);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("LL Open of roam able AP List failed");
		return QDF_STATUS_E_RESOURCES;
	}

	csr_neighbor_roam_state_transition(mac,
			eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
	pNeighborRoamInfo->uOsRequestedHandoff = 0;

	return QDF_STATUS_SUCCESS;
}

/*
 * csr_neighbor_roam_close() -
 * This function closes/frees all the neighbor roam data structures
 *
 * @mac: The handle returned by mac_open.
 * @sessionId: Session identifier
 *
 * Return VOID
 */
void csr_neighbor_roam_close(struct mac_context *mac, uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];

	if (eCSR_NEIGHBOR_ROAM_STATE_CLOSED ==
	    pNeighborRoamInfo->neighborRoamState) {
		sme_warn("Neighbor Roam Algorithm Already Closed");
		return;
	}

	/* Should free up the nodes in the list before closing the
	 * double Linked list
	 */
	csr_neighbor_roam_free_roamable_bss_list(mac,
					&pNeighborRoamInfo->roamableAPList);
	csr_ll_close(&pNeighborRoamInfo->roamableAPList);

	/* Free the profile.. */
	csr_release_profile(mac, &pNeighborRoamInfo->csrNeighborRoamProfile);
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	csr_neighbor_roam_free_roamable_bss_list(mac,
						 &pNeighborRoamInfo->FTRoamInfo.
						 preAuthDoneList);
	csr_ll_close(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList);

	csr_neighbor_roam_state_transition(mac,
		eCSR_NEIGHBOR_ROAM_STATE_CLOSED, sessionId);

}

/*
 * csr_neighbor_middle_of_roaming() -
 * This function returns true if STA is in the middle of roaming states
 *
 * @halHandle: The handle from HDD context.
 * @sessionId: Session identifier
 *
 * Return bool
 */
bool csr_neighbor_middle_of_roaming(struct mac_context *mac, uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];
	bool val = (eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING ==
		    pNeighborRoamInfo->neighborRoamState) ||
		   (eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING ==
		    pNeighborRoamInfo->neighborRoamState) ||
		   (eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE ==
		    pNeighborRoamInfo->neighborRoamState);
	return val;
}

bool
wlan_cm_host_roam_in_progress(struct wlan_objmgr_psoc *psoc,
			      uint8_t vdev_id)
{
	struct csr_roam_session *session;
	struct mac_context *mac_ctx;

	mac_ctx = sme_get_mac_context();
	if (!mac_ctx) {
		sme_err("mac_ctx is NULL");
		return false;
	}

	session = CSR_GET_SESSION(mac_ctx, vdev_id);
	if (!session) {
		sme_err("session is null %d", vdev_id);
		return false;
	}

	return csr_neighbor_middle_of_roaming(mac_ctx, vdev_id);
}

/**
 * csr_neighbor_roam_process_handoff_req - Processes handoff request
 *
 * @mac_ctx  Pointer to mac context
 * @session_id  SME session id
 *
 * This function is called start with the handoff process. First do a
 * SSID scan for the BSSID provided.
 *
 * Return: status
 */
static QDF_STATUS csr_neighbor_roam_process_handoff_req(
			struct mac_context *mac_ctx,
			uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo roam_ctrl_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t roam_id;
	struct csr_roam_profile *profile = NULL;
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);
	uint8_t i = 0;
	uint8_t roamable_ap_count = 0;
	struct scan_filter *scan_filter;
	tScanResultHandle       scan_result;

	if (!session) {
		sme_err("session is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	roam_id = GET_NEXT_ROAM_ID(&mac_ctx->roam);
	profile = qdf_mem_malloc(sizeof(struct csr_roam_profile));
	if (!profile)
		return QDF_STATUS_E_NOMEM;

	status =
		csr_roam_copy_profile(mac_ctx, profile,
				      session->pCurRoamProfile, session_id);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Profile copy failed");
		goto end;
	}
	/* Add the BSSID & Channel */
	profile->BSSIDs.numOfBSSIDs = 1;
	if (!profile->BSSIDs.bssid) {
		profile->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(tSirMacAddr) *
				profile->BSSIDs.numOfBSSIDs);
		if (!profile->BSSIDs.bssid) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
	}

	/* Populate the BSSID from handoff info received from HDD */
	for (i = 0; i < profile->BSSIDs.numOfBSSIDs; i++) {
		qdf_copy_macaddr(&profile->BSSIDs.bssid[i],
				&roam_ctrl_info->handoffReqInfo.bssid);
	}

	profile->ChannelInfo.numOfChannels = 1;
	if (!profile->ChannelInfo.freq_list) {
		profile->ChannelInfo.freq_list =
			qdf_mem_malloc(sizeof(*profile->ChannelInfo.freq_list) *
				       profile->ChannelInfo.numOfChannels);
		if (!profile->ChannelInfo.freq_list) {
			profile->ChannelInfo.numOfChannels = 0;
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
	}

	profile->ChannelInfo.freq_list[0] =
				      roam_ctrl_info->handoffReqInfo.ch_freq;

	/*
	 * For User space connect requests, the scan has already been done.
	 * So, check if the BSS descriptor exists in the scan cache and
	 * proceed with the handoff instead of a redundant scan again.
	 */
	if (roam_ctrl_info->handoffReqInfo.src == CONNECT_CMD_USERSPACE) {
		sme_debug("Connect cmd with bssid within same ESS");
		scan_filter = qdf_mem_malloc(sizeof(*scan_filter));
		if (!scan_filter) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		status = csr_neighbor_roam_get_scan_filter_from_profile(mac_ctx,
								scan_filter,
								session_id);
		sme_debug("Filter creation status: %d", status);
		status = csr_scan_get_result(mac_ctx, scan_filter,
					     &scan_result, true);
		qdf_mem_free(scan_filter);
		csr_neighbor_roam_process_scan_results(mac_ctx, session_id,
							&scan_result);
		roamable_ap_count = csr_ll_count(
					&roam_ctrl_info->roamableAPList);
		sme_debug("roamable_ap_count=%d", roamable_ap_count);
	}
	if (roamable_ap_count) {
		csr_neighbor_roam_trigger_handoff(mac_ctx, session_id);
	} else {
		/* This is temp ifdef will be removed in near future */
		status = csr_scan_for_ssid(mac_ctx, session_id, profile,
					   roam_id, false);
		if (status != QDF_STATUS_SUCCESS)
			sme_err("SSID scan failed");
	}

end:
	if (profile) {
		csr_release_profile(mac_ctx, profile);
		qdf_mem_free(profile);
	}

	return status;
}

/*
 * csr_neighbor_roam_sssid_scan_done() -
 * This function is called once SSID scan is done. If SSID scan failed
 * to find our candidate add an entry to csr scan cache ourself before starting
 * the handoff process

 * @mac: The handle returned by mac_open.
 * @session_id  SME session id
 *
 * Return QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_sssid_scan_done(struct mac_context *mac,
					   uint8_t sessionId, QDF_STATUS status)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS hstatus;

	/* we must be in connected state, if not ignore it */
	if (eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	    pNeighborRoamInfo->neighborRoamState) {
		sme_err("Received in not CONNECTED state. Ignore it");
		return QDF_STATUS_E_FAILURE;
	}
	/* if SSID scan failed to find our candidate add an entry to
	 * csr scan cache ourself
	 */
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Add an entry to csr scan cache");
		hstatus = csr_scan_create_entry_in_scan_cache(mac, sessionId,
				     pNeighborRoamInfo->handoffReqInfo.bssid,
				     pNeighborRoamInfo->handoffReqInfo.ch_freq);
		if (QDF_STATUS_SUCCESS != hstatus) {
			sme_err(
				"csr_scan_create_entry_in_scan_cache failed with status %d",
				hstatus);
			return QDF_STATUS_E_FAILURE;
		}
	}

	/* Now we have completed scanning for the candidate provided by HDD.
	 * Let move on to HO
	 */
	hstatus = csr_neighbor_roam_process_scan_complete(mac, sessionId);

	if (QDF_STATUS_SUCCESS != hstatus) {
		sme_err("Neighbor scan process complete failed with status %d",
			hstatus);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_handoff_req_hdlr - Processes handoff request
 * @mac_ctx  Pointer to mac context
 * @msg  message sent to HDD
 *
 * This function is called by CSR as soon as it gets a handoff request
 * to SME via MC thread
 *
 * Return: status
 */
QDF_STATUS csr_neighbor_roam_handoff_req_hdlr(
			struct mac_context *mac_ctx, void *msg)
{
	tAniHandoffReq *handoff_req = (tAniHandoffReq *) msg;
	uint32_t session_id = handoff_req->sessionId;
	tpCsrNeighborRoamControlInfo roam_ctrl_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* we must be in connected state, if not ignore it */
	if (eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
		roam_ctrl_info->neighborRoamState) {
		sme_err("Received in not CONNECTED state. Ignore it");
		return QDF_STATUS_E_FAILURE;
	}

	/* save the handoff info came from HDD as part of the reassoc req */
	handoff_req = (tAniHandoffReq *) msg;
	if (!handoff_req) {
		sme_err("Received msg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* sanity check */
	if (!qdf_mem_cmp(handoff_req->bssid,
		roam_ctrl_info->currAPbssid.bytes,
		sizeof(tSirMacAddr))) {
		sme_err("Received req has same BSSID as current AP!!");
		return QDF_STATUS_E_FAILURE;
	}
	roam_ctrl_info->handoffReqInfo.ch_freq = handoff_req->ch_freq;
	roam_ctrl_info->handoffReqInfo.src =
		handoff_req->handoff_src;
	qdf_mem_copy(&roam_ctrl_info->handoffReqInfo.bssid.bytes,
			&handoff_req->bssid, QDF_MAC_ADDR_SIZE);
	roam_ctrl_info->uOsRequestedHandoff = 1;

	status = wlan_cm_roam_state_change(mac_ctx->pdev, session_id,
					   WLAN_ROAM_RSO_STOPPED,
					   REASON_OS_REQUESTED_ROAMING_NOW);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("ROAM: RSO stop failed");
		roam_ctrl_info->uOsRequestedHandoff = 0;
	}
	return status;
}

/**
 * csr_neighbor_roam_proceed_with_handoff_req()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function is called by CSR as soon as it gets rsp back for
 * ROAM_SCAN_OFFLOAD_STOP with reason REASON_OS_REQUESTED_ROAMING_NOW
 *
 * Return: QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_proceed_with_handoff_req(struct mac_context *mac,
						      uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	/* we must be in connected state, if not ignore it */
	if ((eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	     pNeighborRoamInfo->neighborRoamState)
	    || (!pNeighborRoamInfo->uOsRequestedHandoff)) {
		sme_err(
			"Received in not CONNECTED state(%d) or uOsRequestedHandoff(%d) is not set. Ignore it",
			pNeighborRoamInfo->neighborRoamState,
			pNeighborRoamInfo->uOsRequestedHandoff);
		status = QDF_STATUS_E_FAILURE;
	} else
		/* Let's go ahead with handoff */
		status = csr_neighbor_roam_process_handoff_req(mac, sessionId);

	if (!QDF_IS_STATUS_SUCCESS(status))
		pNeighborRoamInfo->uOsRequestedHandoff = 0;

	return status;
}

/*
 * csr_neighbor_roam_start_lfr_scan() -
 * This function is called if HDD requested handoff failed for some
 * reason. start the LFR logic at that point.By the time, this function is
 * called, a STOP command has already been issued.

 * @mac: The handle returned by mac_open.
 * @session_id: Session ID
 *
 * Return QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_start_lfr_scan(struct mac_context *mac,
						uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac->roam.neighborRoamInfo[sessionId];
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	/* There is no candidate or We are not roaming Now.
	 * Inform the FW to restart Roam Offload Scan
	 */
	wlan_cm_roam_state_change(mac->pdev, sessionId, WLAN_ROAM_RSO_ENABLED,
				  REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);

	return QDF_STATUS_SUCCESS;



}
#endif
