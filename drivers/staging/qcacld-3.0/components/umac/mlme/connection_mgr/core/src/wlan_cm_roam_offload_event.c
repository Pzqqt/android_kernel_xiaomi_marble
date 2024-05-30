/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_roam_offload_event.c
 *
 * Implementation for the FW based roaming events api interfaces.
 */
#include "qdf_status.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_roam_i.h"
#include <wlan_cm_public_struct.h>
#include "wlan_scan_public_structs.h"
#include "wlan_cm_roam_public_struct.h"
#include "wlan_serialization_api.h"
#include "wlan_cm_roam_api.h"
#include <wlan_cfg80211_scan.h>
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "connection_mgr/core/src/wlan_cm_sm.h"
#include "connection_mgr/core/src/wlan_cm_main_api.h"
#include "wlan_roam_debug.h"
#include "wlan_mlo_mgr_roam.h"

#define FW_ROAM_SYNC_TIMEOUT 7000

static QDF_STATUS
cm_fw_roam_ser_cb(struct wlan_serialization_command *cmd,
		  enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;

	if (!cmd) {
		mlme_err("cmd is NULL, reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev = cmd->vdev;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_NULL_VALUE;

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		cm_ctx->active_cm_id = cmd->cmd_id;
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_FMT "Active command timeout",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));

		cm_abort_fw_roam(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		cm_reset_active_cm_id(vdev, cmd->cmd_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

QDF_STATUS cm_abort_fw_roam(struct cnx_mgr *cm_ctx,
			    wlan_cm_id cm_id)
{
	QDF_STATUS status;

	mlme_cm_osif_roam_abort_ind(cm_ctx->vdev);
	status = cm_sm_deliver_event(cm_ctx->vdev,
				     WLAN_CM_SM_EV_ROAM_ABORT,
				     sizeof(wlan_cm_id), &cm_id);

	if (QDF_IS_STATUS_ERROR(status))
		cm_remove_cmd(cm_ctx, &cm_id);

	return status;
}

QDF_STATUS
cm_add_fw_roam_dummy_ser_cb(struct wlan_objmgr_pdev *pdev,
			    struct cnx_mgr *cm_ctx,
			    struct cm_req *cm_req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "unable to get reference",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_ROAM;
	cmd.cmd_id = cm_req->cm_id;
	cmd.cmd_cb = cm_fw_roam_ser_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = true;
	cmd.cmd_timeout_duration = FW_ROAM_SYNC_TIMEOUT;
	cmd.vdev = cm_ctx->vdev;
	cmd.is_blocking = cm_ser_get_blocking_cmd();

	ser_cmd_status = wlan_serialization_request(&cmd);
	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	default:
		mlme_err(CM_PREFIX_FMT "ser cmd status %d",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id), ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_prepare_roam_cmd(struct cnx_mgr *cm_ctx,
			       struct cm_req **roam_req,
			       enum wlan_cm_source source)
{
	struct cm_req *req;

	*roam_req = qdf_mem_malloc(sizeof(**roam_req));
	if (!*roam_req)
		return QDF_STATUS_E_NOMEM;

	req = *roam_req;
	req->roam_req.req.source = source;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_add_fw_roam_cmd_to_list_n_ser(struct cnx_mgr *cm_ctx,
					    struct cm_req *cm_req)
{
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);

	if (!pdev) {
		mlme_err("Failed to find pdev for vdev id %d",
			 wlan_vdev_get_id(cm_ctx->vdev));
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_add_roam_req_to_list(cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_abort_fw_roam(cm_ctx, CM_ID_INVALID);
		cm_free_roam_req_mem(&cm_req->roam_req);
		qdf_mem_free(cm_req);
		return status;
	}

	/**
	 * Skip adding dummy SER command for MLO link vdev. It's expected to add
	 * only for MLO sta in case of MLO connection
	 */
	if (wlan_vdev_mlme_is_mlo_link_vdev(cm_ctx->vdev))
		return status;

	status = cm_add_fw_roam_dummy_ser_cb(pdev, cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_abort_fw_roam(cm_ctx, cm_req->roam_req.cm_id);
		return status;
	}

	return status;
}

QDF_STATUS cm_fw_roam_start_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_START,
				     0, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("EV ROAM START not handled");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS cm_fw_roam_start(struct cnx_mgr *cm_ctx)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	wlan_scan_id scan_id;
	bool abort_host_scan_cap = false;
	wlan_cm_id cm_id;
	struct cm_roam_req *roam_req = NULL;

	roam_req = cm_get_first_roam_command(cm_ctx->vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		cm_id = CM_ID_INVALID;
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	cm_id = roam_req->cm_id;
	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(roam_req->req.vdev_id,
				       roam_req->cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(roam_req->req.vdev_id,
				       roam_req->cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	status = wlan_cm_roam_state_change(pdev,
					   roam_req->req.vdev_id,
					   WLAN_ROAMING_IN_PROG,
					   REASON_ROAM_CANDIDATE_FOUND);

	if (QDF_IS_STATUS_ERROR(status))
		goto error;

	mlme_cm_osif_roam_start_ind(cm_ctx->vdev);
	/*
	 * For emergency deauth roaming, firmware sends ROAM start
	 * instead of ROAM scan start notification as data path queues
	 * will be stopped only during roam start notification.
	 * This is because, for deauth/disassoc triggered roam, the
	 * AP has sent deauth, and packets shouldn't be sent to AP
	 * after that. Since firmware is sending roam start directly
	 * host sends scan abort during roam scan, but in other
	 * triggers, the host receives roam start after candidate
	 * selection and roam scan is complete. So when host sends
	 * roam abort for emergency deauth roam trigger, the firmware
	 * roam scan is also aborted. This results in roaming failure.
	 * So send scan_id as CANCEL_HOST_SCAN_ID to scan module to
	 * abort only host triggered scans.
	 */
	abort_host_scan_cap =
		wlan_mlme_get_host_scan_abort_support(psoc);
	if (abort_host_scan_cap)
		scan_id = CANCEL_HOST_SCAN_ID;
	else
		scan_id = INVAL_SCAN_ID;

	wlan_abort_scan(pdev, INVAL_PDEV_ID,
			roam_req->req.vdev_id,
			scan_id, false);
error:
	if (QDF_IS_STATUS_ERROR(status))
		cm_abort_fw_roam(cm_ctx, cm_id);

	return status;
}

QDF_STATUS cm_fw_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct cm_roam_req *roam_req = NULL;
	wlan_cm_id cm_id = CM_ID_INVALID;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("Failed to find pdev for vdev id %d",
			 vdev_id);
		goto rel_ref;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		goto rel_ref;

	roam_req = cm_get_first_roam_command(vdev);
	if (roam_req)
		cm_id = roam_req->cm_id;

	/* continue even if no roam command is found */

	/*
	 * Switch to RSO enabled state only if the current state is
	 * WLAN_ROAMING_IN_PROG or WLAN_ROAM_SYNCH_IN_PROG.
	 * This API can be called in internal roam aborts also when
	 * RSO state is deinit and cause RSO start to be sent in
	 * disconnected state.
	 */
	if (MLME_IS_ROAMING_IN_PROG(psoc, vdev_id) ||
	    MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id))
		status = wlan_cm_roam_state_change(pdev, vdev_id,
						   WLAN_ROAM_RSO_ENABLED,
						   REASON_ROAM_ABORT);

	cm_abort_fw_roam(cm_ctx, cm_id);
rel_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS
cm_roam_sync_event_handler(struct wlan_objmgr_psoc *psoc,
			   uint8_t *event,
			   uint32_t len,
			   struct roam_offload_synch_ind *sync_ind)
{
	if (!sync_ind) {
		mlme_err("invalid sync_ind");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (sync_ind->hw_mode_trans_present)
		cm_handle_roam_sync_update_hw_mode(
			&sync_ind->hw_mode_trans_ind);

	return mlo_fw_roam_sync_req(psoc, sync_ind->roamed_vdev_id,
				    sync_ind, sizeof(sync_ind));
}

QDF_STATUS
cm_roam_sync_frame_event_handler(struct wlan_objmgr_psoc *psoc,
				 struct roam_synch_frame_ind *frame_ind)
{
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct roam_synch_frame_ind *sync_frame_ind = frame_ind;
	struct roam_synch_frame_ind *roam_synch_frame_ind;
	uint8_t vdev_id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!sync_frame_ind)
		return QDF_STATUS_E_NULL_VALUE;

	vdev_id = sync_frame_ind->vdev_id;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

	roam_synch_frame_ind = &rso_cfg->roam_sync_frame_ind;

	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id)) {
		mlme_err("Ignoring this event as it is unexpected");
		wlan_cm_free_roam_synch_frame_ind(rso_cfg);
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

	if (sync_frame_ind->bcn_probe_rsp_len) {
		roam_synch_frame_ind->bcn_probe_rsp_len =
			sync_frame_ind->bcn_probe_rsp_len;
		roam_synch_frame_ind->is_beacon =
			sync_frame_ind->is_beacon;
		roam_synch_frame_ind->bcn_probe_rsp =
			sync_frame_ind->bcn_probe_rsp;
	}

	if (sync_frame_ind->reassoc_req_len) {
		roam_synch_frame_ind->reassoc_req_len =
				sync_frame_ind->reassoc_req_len;
		roam_synch_frame_ind->reassoc_req =
			sync_frame_ind->reassoc_req;
	}

	if (sync_frame_ind->reassoc_rsp_len) {
		roam_synch_frame_ind->reassoc_rsp_len =
				sync_frame_ind->reassoc_rsp_len;
		roam_synch_frame_ind->reassoc_rsp =
			sync_frame_ind->reassoc_rsp;
	}

err:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	return status;
}

QDF_STATUS cm_roam_sync_event_handler_cb(struct wlan_objmgr_vdev *vdev,
					 uint8_t *event,
					 uint32_t len)
{
	struct roam_offload_synch_ind *sync_ind = NULL;
	struct wlan_objmgr_psoc *psoc = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct rso_config *rso_cfg;
	uint16_t ie_len = 0;
	uint8_t vdev_id;

	sync_ind = (struct roam_offload_synch_ind *)event;

	if (!sync_ind) {
		mlme_err("Roam Sync ind ptr is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!vdev) {
		mlme_err("Vdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("Psoc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_NULL_VALUE;

	wlan_roam_debug_log(sync_ind->roamed_vdev_id, DEBUG_ROAM_SYNCH_IND,
			    DEBUG_INVALID_PEER_ID, sync_ind->bssid.bytes, NULL,
			    0,
			    0);
	DPTRACE(qdf_dp_trace_record_event(QDF_DP_TRACE_EVENT_RECORD,
					  sync_ind->roamed_vdev_id,
					  QDF_TRACE_DEFAULT_PDEV_ID,
					  QDF_PROTO_TYPE_EVENT,
					  QDF_ROAM_SYNCH));

	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, sync_ind->roamed_vdev_id) &&
	    !is_multi_link_roam(sync_ind)) {
		mlme_err("Ignoring RSI since one is already in progress");
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

	if (!QDF_IS_STATUS_SUCCESS(cm_fw_roam_sync_start_ind(vdev,
							     sync_ind))) {
		mlme_err("LFR3: CSR Roam synch cb failed");
		wlan_cm_free_roam_synch_frame_ind(rso_cfg);
		goto err;
	}

	/* 24 byte MAC header and 12 byte to ssid IE */
	if (sync_ind->beaconProbeRespLength >
			(QDF_IEEE80211_3ADDR_HDR_LEN + MAC_B_PR_SSID_OFFSET)) {
		ie_len = sync_ind->beaconProbeRespLength -
			(QDF_IEEE80211_3ADDR_HDR_LEN + MAC_B_PR_SSID_OFFSET);
	} else {
		mlme_err("LFR3: Invalid Beacon Length");
		goto err;
	}

	if (QDF_IS_STATUS_ERROR(cm_roam_pe_sync_callback(sync_ind,
							 vdev_id,
							 ie_len))) {
		mlme_err("LFR3: PE roam synch cb failed");
		status = QDF_STATUS_E_BUSY;
		goto err;
	}

	cm_roam_update_vdev(sync_ind, vdev_id);
	/*
	 * update phy_mode in wma to avoid mismatch in phymode between host and
	 * firmware. The phymode stored in peer->peer_mlme.phymode is
	 * sent to firmware as part of opmode update during either - vht opmode
	 * action frame received or during opmode change detected while
	 * processing beacon. Any mismatch of this value with firmware phymode
	 * results in firmware assert.
	 */
	cm_update_phymode_on_roam(sync_ind->roamed_vdev_id,
				  sync_ind->bssid.bytes,
				  &sync_ind->chan);
	cm_fw_roam_sync_propagation(psoc,
				    vdev_id,
				    sync_ind);

err:
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_mlo_roam_abort_on_link(psoc, sync_ind);
		cm_fw_roam_abort_req(psoc, sync_ind->roamed_vdev_id);
		cm_roam_stop_req(psoc, sync_ind->roamed_vdev_id,
				 REASON_ROAM_SYNCH_FAILED,
				 NULL, false);
	}
	return status;
}

QDF_STATUS
cm_roam_candidate_event_handler(struct wlan_objmgr_psoc *psoc,
				struct roam_scan_candidate_frame *candidate)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct cnx_mgr *cm_ctx;
	uint32_t ie_offset, ie_len;
	uint8_t *ie_ptr = NULL;
	uint8_t *extracted_ie = NULL;
	uint8_t primary_channel, band;
	qdf_freq_t op_freq;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, candidate->vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("pdev object is NULL");
		goto err;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		mlme_err("cm ctx is NULL");
		goto err;
	}

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_MLME, QDF_TRACE_LEVEL_DEBUG,
			   candidate->frame, candidate->frame_length);
	/* Fixed parameters offset */
	ie_offset = sizeof(struct wlan_frame_hdr) + MAC_B_PR_SSID_OFFSET;

	if (candidate->frame_length <= ie_offset) {
		mlme_err("Invalid frame length");
		goto err;
	}

	ie_ptr = candidate->frame + ie_offset;
	ie_len = candidate->frame_length - ie_offset;

	/* For 2.4GHz,5GHz get channel from DS IE */
	extracted_ie = (uint8_t *)wlan_get_ie_ptr_from_eid(WLAN_ELEMID_DSPARMS,
							   ie_ptr, ie_len);
	if (extracted_ie && extracted_ie[0] == WLAN_ELEMID_DSPARMS &&
	    extracted_ie[1] == WLAN_DS_PARAM_IE_MAX_LEN) {
		band = BIT(REG_BAND_2G) | BIT(REG_BAND_5G);
		primary_channel = *(extracted_ie + 2);
		mlme_debug("Extracted primary channel from DS : %d",
			   primary_channel);
		goto update_beacon;
	}

	/* For HT, VHT and non-6GHz HE, get channel from HTINFO IE */
	extracted_ie = (uint8_t *)
			wlan_get_ie_ptr_from_eid(WLAN_ELEMID_HTINFO_ANA,
						 ie_ptr, ie_len);
	if (extracted_ie && extracted_ie[0] == WLAN_ELEMID_HTINFO_ANA &&
	    extracted_ie[1] == sizeof(struct wlan_ie_htinfo_cmn)) {
		band = BIT(REG_BAND_2G) | BIT(REG_BAND_5G);
		primary_channel =
			((struct wlan_ie_htinfo *)extracted_ie)->
						hi_ie.hi_ctrlchannel;
		mlme_debug("Extracted primary channel from HT INFO : %d",
			   primary_channel);
		goto update_beacon;
	}
	/* For 6GHz, get channel from HE OP IE */
	extracted_ie = (uint8_t *)
			wlan_get_ext_ie_ptr_from_ext_id(WLAN_HEOP_OUI_TYPE,
							(uint8_t)
							WLAN_HEOP_OUI_SIZE,
							ie_ptr, ie_len);
	if (extracted_ie && !qdf_mem_cmp(&extracted_ie[2], WLAN_HEOP_OUI_TYPE,
					 WLAN_HEOP_OUI_SIZE) &&
	    extracted_ie[1] <= WLAN_MAX_HEOP_IE_LEN) {
		band = BIT(REG_BAND_6G);
		primary_channel = util_scan_get_6g_oper_channel(extracted_ie);
		mlme_debug("Extracted primary channel from HE OP : %d",
			   primary_channel);
		if (primary_channel)
			goto update_beacon;
	}

	mlme_err("Primary channel was not found in the candidate scan entry");
	goto err;

update_beacon:
	op_freq = wlan_reg_chan_band_to_freq(pdev, primary_channel, band);
	mlme_debug("Roaming candidate frequency : %d", op_freq);
	cm_inform_bcn_probe(cm_ctx, candidate->frame, candidate->frame_length,
			    op_freq,
			    0, /* Real RSSI will be updated by Roam synch ind */
			    cm_ctx->active_cm_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	return QDF_STATUS_SUCCESS;
err:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	return QDF_STATUS_E_FAILURE;
}
