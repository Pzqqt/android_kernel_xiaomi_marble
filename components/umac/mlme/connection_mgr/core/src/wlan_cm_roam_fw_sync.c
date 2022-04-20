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
 * DOC: wlan_cm_roam_fw_sync.c
 *
 * Implementation for the FW based roaming sync api interfaces.
 */
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_roam_i.h"
#include "wlan_blm_api.h"
#include "wlan_cm_roam_public_struct.h"
#include "wlan_utility.h"
#include "wlan_scan_api.h"
#include "wlan_crypto_global_api.h"
#include "wlan_cm_tgt_if_tx_api.h"
#include "wlan_cm_vdev_api.h"
#include "wlan_p2p_api.h"
#include "wlan_tdls_api.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "wlan_pkt_capture_ucfg_api.h"
#include "cds_utils.h"
#include "wlan_roam_debug.h"
#include "wlan_mlme_twt_api.h"
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "connection_mgr/core/src/wlan_cm_main.h"
#include "connection_mgr/core/src/wlan_cm_sm.h"
#include <wlan_mlo_mgr_sta.h>
#include "wlan_mlo_mgr_roam.h"

QDF_STATUS cm_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			       void *event, uint32_t event_data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (cm_is_vdev_connecting(vdev) || cm_is_vdev_disconnecting(vdev)) {
		mlme_err("vdev %d Roam sync not handled in conneting/disconneting state",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
		return cm_roam_stop_req(psoc, vdev_id,
					REASON_ROAM_SYNCH_FAILED, NULL, false);
	}

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_SYNC,
				     event_data_len, event);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("EV ROAM SYNC REQ not handled");
		cm_fw_roam_abort_req(psoc, vdev_id);
		cm_roam_stop_req(psoc, vdev_id, REASON_ROAM_SYNCH_FAILED,
				 NULL, false);
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS
cm_fw_send_vdev_roam_event(struct cnx_mgr *cm_ctx, uint16_t data_len,
			   void *data)
{
	QDF_STATUS status;
	wlan_cm_id cm_id;
	struct wlan_objmgr_psoc *psoc;
	struct cm_roam_req *roam_req = NULL;

	roam_req = cm_get_first_roam_command(cm_ctx->vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		cm_id = CM_ID_INVALID;
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	cm_id = roam_req->cm_id;
	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(roam_req->req.vdev_id,
				       roam_req->cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	status = wlan_vdev_mlme_sm_deliver_evt(cm_ctx->vdev,
					       WLAN_VDEV_SM_EV_ROAM,
					       data_len,
					       data);

	if (QDF_IS_STATUS_ERROR(status))
		cm_roam_stop_req(psoc, roam_req->req.vdev_id,
				 REASON_ROAM_SYNCH_FAILED,
				 NULL, false);

error:
	if (QDF_IS_STATUS_ERROR(status))
		cm_abort_fw_roam(cm_ctx, cm_id);

	return status;
}

QDF_STATUS
cm_fw_roam_sync_start_ind(struct wlan_objmgr_vdev *vdev,
			  struct roam_offload_synch_ind *sync_ind)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_pdev *pdev;
	struct qdf_mac_addr connected_bssid;
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;

	pdev = wlan_vdev_get_pdev(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_pdev_get_psoc(pdev);

	if (wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
		if (!MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc,
					sync_ind->roamed_vdev_id))
			status = wlan_cm_roam_state_change(pdev,
					sync_ind->roamed_vdev_id,
					WLAN_ROAM_SYNCH_IN_PROG,
					REASON_ROAM_HANDOFF_DONE);

		status = wlan_cm_roam_state_change(pdev,
					vdev_id,
					WLAN_MLO_ROAM_SYNCH_IN_PROG,
					REASON_ROAM_HANDOFF_DONE);
		return status;
	}

	/*
	 * Get old bssid as, new AP is not updated yet and do cleanup
	 * for old bssid.
	 */
	wlan_mlme_get_bssid_vdev_id(pdev, vdev_id,
				    &connected_bssid);

	/* Update the BLM that the previous profile has disconnected */
	wlan_blm_update_bssid_connect_params(pdev,
					     connected_bssid,
					     BLM_AP_DISCONNECTED);
	if (IS_ROAM_REASON_STA_KICKOUT(sync_ind->roam_reason)) {
		struct reject_ap_info ap_info;

		qdf_mem_zero(&ap_info, sizeof(struct reject_ap_info));
		ap_info.bssid = connected_bssid;
		ap_info.reject_ap_type = DRIVER_AVOID_TYPE;
		ap_info.reject_reason = REASON_STA_KICKOUT;
		ap_info.source = ADDED_BY_DRIVER;
		wlan_blm_add_bssid_to_reject_list(pdev, &ap_info);
	}

	cm_update_scan_mlme_on_roam(vdev, &connected_bssid,
				    SCAN_ENTRY_CON_STATE_NONE);

	if (!MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id))
		status = wlan_cm_roam_state_change(pdev,
						   vdev_id,
						   WLAN_ROAM_SYNCH_IN_PROG,
						   REASON_ROAM_HANDOFF_DONE);

	mlme_init_twt_context(wlan_pdev_get_psoc(pdev), &connected_bssid,
			      TWT_ALL_SESSIONS_DIALOG_ID);

	mlme_cm_osif_roam_sync_ind(vdev);

	return status;
}

void
cm_update_scan_mlme_on_roam(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *connected_bssid,
			    enum scan_entry_connection_state state)
{
	struct wlan_objmgr_pdev *pdev;
	struct bss_info bss_info;
	struct mlme_info mlme;
	struct wlan_channel *chan;
	QDF_STATUS status;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("failed to find pdev");
		return;
	}

	chan = wlan_vdev_get_active_channel(vdev);
	if (!chan) {
		mlme_err("failed to get active channel");
		return;
	}

	status = wlan_vdev_mlme_get_ssid(vdev, bss_info.ssid.ssid,
					 &bss_info.ssid.length);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("failed to get ssid");
		return;
	}

	mlme.assoc_state = state;
	qdf_copy_macaddr(&bss_info.bssid, connected_bssid);

	bss_info.freq = chan->ch_freq;

	wlan_scan_update_mlme_by_bssinfo(pdev, &bss_info, &mlme);
}

#ifdef WLAN_FEATURE_FILS_SK
static QDF_STATUS
cm_fill_fils_ie(struct wlan_connect_rsp_ies *connect_ies,
		struct roam_offload_synch_ind *roam_synch_data)
{
	struct fils_connect_rsp_params *fils_ie;

	if (!roam_synch_data->hlp_data_len)
		return QDF_STATUS_SUCCESS;

	connect_ies->fils_ie = qdf_mem_malloc(sizeof(*fils_ie));
	if (!connect_ies->fils_ie)
		return QDF_STATUS_E_NOMEM;

	fils_ie = connect_ies->fils_ie;
	cds_copy_hlp_info(&roam_synch_data->dst_mac,
			  &roam_synch_data->src_mac,
			  roam_synch_data->hlp_data_len,
			  roam_synch_data->hlp_data,
			  &fils_ie->dst_mac,
			  &fils_ie->src_mac,
			  &fils_ie->hlp_data_len,
			  fils_ie->hlp_data);

	fils_ie->fils_seq_num = roam_synch_data->next_erp_seq_num;

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
cm_fill_fils_ie(struct wlan_connect_rsp_ies *connect_ies,
		struct roam_offload_synch_ind *roam_synch_data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS
cm_populate_connect_ies(struct roam_offload_synch_ind *roam_synch_data,
			struct cm_vdev_join_rsp *rsp)
{
	struct wlan_connect_rsp_ies *connect_ies;
	uint8_t *bcn_probe_rsp_ptr;
	uint8_t *reassoc_rsp_ptr;
	uint8_t *reassoc_req_ptr;

	connect_ies = &rsp->connect_rsp.connect_ies;

	/* Beacon/Probe Rsp frame */
	if (roam_synch_data->beaconProbeRespLength) {
		connect_ies->bcn_probe_rsp.len =
			roam_synch_data->beaconProbeRespLength;
		bcn_probe_rsp_ptr = (uint8_t *)roam_synch_data +
					roam_synch_data->beaconProbeRespOffset;

		connect_ies->bcn_probe_rsp.ptr =
			qdf_mem_malloc(connect_ies->bcn_probe_rsp.len);
		if (!connect_ies->bcn_probe_rsp.ptr)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_copy(connect_ies->bcn_probe_rsp.ptr, bcn_probe_rsp_ptr,
			     connect_ies->bcn_probe_rsp.len);
	}

	/* ReAssoc Rsp IE data */
	if (roam_synch_data->reassocRespLength >
	    sizeof(struct wlan_frame_hdr)) {
		connect_ies->assoc_rsp.len =
				roam_synch_data->reassocRespLength -
				sizeof(struct wlan_frame_hdr);
		reassoc_rsp_ptr = (uint8_t *)roam_synch_data +
				  roam_synch_data->reassocRespOffset +
				  sizeof(struct wlan_frame_hdr);
		connect_ies->assoc_rsp.ptr =
			qdf_mem_malloc(connect_ies->assoc_rsp.len);
		if (!connect_ies->assoc_rsp.ptr)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(connect_ies->assoc_rsp.ptr, reassoc_rsp_ptr,
			     connect_ies->assoc_rsp.len);
	}

	/* ReAssoc Req IE data */
	if (roam_synch_data->reassoc_req_length >
	    sizeof(struct wlan_frame_hdr)) {
		connect_ies->assoc_req.len =
				roam_synch_data->reassoc_req_length -
				sizeof(struct wlan_frame_hdr);
		reassoc_req_ptr = (uint8_t *)roam_synch_data +
				  roam_synch_data->reassoc_req_offset +
				  sizeof(struct wlan_frame_hdr);
		connect_ies->assoc_req.ptr =
			qdf_mem_malloc(connect_ies->assoc_req.len);
		if (!connect_ies->assoc_req.ptr)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_copy(connect_ies->assoc_req.ptr, reassoc_req_ptr,
			     connect_ies->assoc_req.len);
	}
	rsp->connect_rsp.is_ft = roam_synch_data->is_ft_im_roam;

	cm_fill_fils_ie(connect_ies, roam_synch_data);

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_ESE
static QDF_STATUS
cm_copy_tspec_ie(struct cm_vdev_join_rsp *rsp,
		 struct roam_offload_synch_ind *roam_synch_data)
{
	if (roam_synch_data->tspec_len) {
		rsp->tspec_ie.len = roam_synch_data->tspec_len;
		rsp->tspec_ie.ptr =
			qdf_mem_malloc(rsp->tspec_ie.len);
		if (!rsp->tspec_ie.ptr)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(rsp->tspec_ie.ptr,
			     roam_synch_data->ric_tspec_data +
			     roam_synch_data->ric_data_len,
			     rsp->tspec_ie.len);
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
cm_copy_tspec_ie(struct cm_vdev_join_rsp *rsp,
		 struct roam_offload_synch_ind *roam_synch_data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
static void
cm_fils_update_erp_seq_num(struct wlan_objmgr_vdev *vdev,
			   uint16_t next_erp_seq_num,
			   wlan_cm_id cm_id)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_fils_connection_info *fils_info;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_id));
		return;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(vdev_id, cm_id));
		return;
	}

	fils_info = wlan_cm_get_fils_connection_info(psoc, vdev_id);
	if (!fils_info)
		return;

	/*
	 * update the erp sequence number to the vdev level
	 * FILS cache. This will be sent in the next RSO
	 * command.
	 */
	fils_info->erp_sequence_number = next_erp_seq_num;
}
#else
static inline void
cm_fils_update_erp_seq_num(struct wlan_objmgr_vdev *vdev,
			   uint16_t next_erp_seq_num, wlan_cm_id cm_id)
{}
#endif

static QDF_STATUS
cm_fill_roam_info(struct wlan_objmgr_vdev *vdev,
		  struct roam_offload_synch_ind *roam_synch_data,
		  struct cm_vdev_join_rsp *rsp, wlan_cm_id cm_id)
{
	struct wlan_roam_sync_info *roaming_info;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	rsp->connect_rsp.roaming_info = qdf_mem_malloc(sizeof(*roaming_info));
	if (!rsp->connect_rsp.roaming_info)
			return QDF_STATUS_E_NOMEM;
	rsp->connect_rsp.vdev_id = wlan_vdev_get_id(vdev);
	qdf_copy_macaddr(&rsp->connect_rsp.bssid, &roam_synch_data->bssid);

	if (!util_scan_is_null_ssid(&roam_synch_data->ssid))
		wlan_vdev_mlme_set_ssid(vdev,
					roam_synch_data->ssid.ssid,
					roam_synch_data->ssid.length);

	status = wlan_vdev_mlme_get_ssid(vdev,
					 rsp->connect_rsp.ssid.ssid,
					 &rsp->connect_rsp.ssid.length);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Failed to get ssid",
			 CM_PREFIX_REF(rsp->connect_rsp.vdev_id, cm_id));
		return QDF_STATUS_E_FAILURE;
	}

	rsp->connect_rsp.is_reassoc = true;
	rsp->connect_rsp.connect_status = QDF_STATUS_SUCCESS;
	rsp->connect_rsp.cm_id = cm_id;
	rsp->connect_rsp.freq = roam_synch_data->chan_freq;
	rsp->nss = roam_synch_data->nss;

	if (roam_synch_data->ric_data_len) {
		rsp->ric_resp_ie.len = roam_synch_data->ric_data_len;
		rsp->ric_resp_ie.ptr =
			qdf_mem_malloc(rsp->ric_resp_ie.len);
		if (!rsp->ric_resp_ie.ptr)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(rsp->ric_resp_ie.ptr,
			     roam_synch_data->ric_tspec_data,
			     rsp->ric_resp_ie.len);
	}
	cm_copy_tspec_ie(rsp, roam_synch_data);

	status = cm_populate_connect_ies(roam_synch_data, rsp);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	roaming_info = rsp->connect_rsp.roaming_info;
	roaming_info->auth_status = roam_synch_data->auth_status;
	roaming_info->kck_len = roam_synch_data->kck_len;
	if (roaming_info->kck_len)
		qdf_mem_copy(roaming_info->kck, roam_synch_data->kck,
			     roam_synch_data->kck_len);
	roaming_info->kek_len = roam_synch_data->kek_len;
	if (roaming_info->kek_len)
		qdf_mem_copy(roaming_info->kek, roam_synch_data->kek,
			     roam_synch_data->kek_len);
	qdf_mem_copy(roaming_info->replay_ctr, roam_synch_data->replay_ctr,
		     REPLAY_CTR_LEN);
	roaming_info->roam_reason =
		roam_synch_data->roam_reason & ROAM_REASON_MASK;
	roaming_info->subnet_change_status =
			CM_GET_SUBNET_STATUS(roaming_info->roam_reason);
	roaming_info->pmk_len = roam_synch_data->pmk_len;
	if (roaming_info->pmk_len)
		qdf_mem_copy(roaming_info->pmk, roam_synch_data->pmk,
			     roaming_info->pmk_len);

	qdf_mem_copy(roaming_info->pmkid, roam_synch_data->pmkid,
		     PMKID_LEN);
	roaming_info->update_erp_next_seq_num =
			roam_synch_data->update_erp_next_seq_num;
	roaming_info->next_erp_seq_num = roam_synch_data->next_erp_seq_num;

	cm_fils_update_erp_seq_num(vdev, roaming_info->next_erp_seq_num, cm_id);

	return status;
}

static QDF_STATUS cm_process_roam_keys(struct wlan_objmgr_vdev *vdev,
				       struct cm_vdev_join_rsp *rsp,
				       wlan_cm_id cm_id)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_roam_sync_info *roaming_info;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct cm_roam_values_copy config;
	uint8_t mdie_present;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int32_t akm;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj) {
		mlme_err(CM_PREFIX_FMT "Failed to mlme psoc obj",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	roaming_info = rsp->connect_rsp.roaming_info;
	akm = wlan_crypto_get_param(vdev,
				    WLAN_CRYPTO_PARAM_KEY_MGMT);

	/*
	 * Encryption keys for new connection are obtained as follows:
	 * auth_status = CSR_ROAM_AUTH_STATUS_AUTHENTICATED
	 * Open - No keys required.
	 * Static WEP - Firmware copies keys from old AP to new AP.
	 * Fast roaming authentications e.g. PSK, FT, CCKM - firmware
	 *		supplicant obtains them through 4-way handshake.
	 *
	 * auth_status = CSR_ROAM_AUTH_STATUS_CONNECTED
	 * All other authentications - Host supplicant performs EAPOL
	 *	with AP after this point and sends new keys to the driver.
	 *	Driver starts wait_for_key timer for that purpose.
	 * Allow cm_lookup_pmkid_using_bssid() if akm is SAE/OWE since
	 * SAE/OWE roaming uses hybrid model and eapol is offloaded to
	 * supplicant unlike in WPA2 802.1x case, after 8 way handshake
	 * the __wlan_hdd_cfg80211_keymgmt_set_key ->sme_roam_set_psk_pmk()
	 * will get called after roam synch complete to update the
	 * session->psk_pmk, but in SAE/OWE roaming this sequence is not
	 * present and set_pmksa will come before roam synch indication &
	 * eapol. So the session->psk_pmk will be stale in PMKSA cached
	 * SAE/OWE roaming case.
	 */

	if (roaming_info->auth_status == ROAM_AUTH_STATUS_AUTHENTICATED ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE)) {
		struct wlan_crypto_pmksa *pmkid_cache, *pmksa;

		cm_csr_set_ss_none(vdev_id);
		/*
		 * If authStatus is AUTHENTICATED, then we have done successful
		 * 4 way handshake in FW using the cached PMKID.
		 * However, the session->psk_pmk has the PMK of the older AP
		 * as set_key is not received from supplicant.
		 * When any RSO command is sent for the current AP, the older
		 * AP's PMK is sent to the FW which leads to incorrect PMK and
		 * leads to 4 way handshake failure when roaming happens to
		 * this AP again.
		 * Check if a PMK cache exists for the roamed AP and update
		 * it into the session pmk.
		 */
		pmkid_cache = qdf_mem_malloc(sizeof(*pmkid_cache));
		if (!pmkid_cache) {
			status = QDF_STATUS_E_NOMEM;
			mlme_err(CM_PREFIX_FMT "Mem alloc failed",
				 CM_PREFIX_REF(vdev_id, cm_id));
			goto end;
		}
		wlan_vdev_get_bss_peer_mac(vdev, &pmkid_cache->bssid);
		mlme_debug(CM_PREFIX_FMT "Trying to find PMKID for "
			   QDF_MAC_ADDR_FMT " AKM Type:%d",
			   CM_PREFIX_REF(vdev_id, cm_id),
			   QDF_MAC_ADDR_REF(pmkid_cache->bssid.bytes), akm);

		wlan_cm_roam_cfg_get_value(psoc, vdev_id,
					   MOBILITY_DOMAIN, &config);
		mdie_present = config.bool_value;

		if (cm_lookup_pmkid_using_bssid(psoc,
						vdev_id,
						pmkid_cache)) {
			/*
			 * Consider two APs: AP1, AP2
			 * Both APs configured with EAP 802.1x security mode
			 * and OKC is enabled in both APs by default. Initially
			 * DUT successfully associated with AP1, and generated
			 * PMK1 by performing full EAP and added an entry for
			 * AP1 in pmk table. At this stage, pmk table has only
			 * one entry for PMK1 (1. AP1-->PMK1). Now DUT try to
			 * roam to AP2 using PMK1 (as OKC is enabled) but
			 * session timeout happens on AP2 just before 4 way
			 * handshake completion in FW. At this point of time
			 * DUT not in authenticated state. Due to this DUT
			 * performs full EAP with AP2 and generates PMK2. As
			 * there is no previous entry of AP2 (AP2-->PMK1) in pmk
			 * table. When host gets pmk delete command for BSSID of
			 * AP2, the BSSID match fails. Hence host will not
			 * delete pmk entry of AP1 as well.
			 * At this point of time, the PMK table has two entry
			 * 1. AP1-->PMK1 and 2. AP2 --> PMK2.
			 * Ideally, if OKC is enabled then whenever timeout
			 * occurs in a mobility domain, then the driver should
			 * clear all APs cache entries related to that domain
			 * but as the BSSID doesn't exist yet in the driver
			 * cache there is no way of clearing the cache entries,
			 * without disturbing the legacy roaming.
			 * Now security profile for both APs changed to FT-RSN.
			 * DUT first disassociate with AP2 and successfully
			 * associated with AP2 and perform full EAP and
			 * generates PMK3. DUT first deletes PMK entry for AP2
			 * and then adds a new entry for AP2.
			 * At this point of time pmk table has two entry
			 * AP2--> PMK3 and AP1-->PMK1. Now DUT roamed to AP1
			 * using PMK3 but sends stale entry of AP1 (PMK1) to
			 * fw via RSO command. This override PMK for both APs
			 * with PMK1 (as FW uses mlme session PMK for both APs
			 * in case of FT roaming) and next time when FW try to
			 * roam to AP2 using PMK1, AP2 rejects PMK1 (As AP2 is
			 * expecting PMK3) and initiates full EAP with AP2,
			 * which is wrong.
			 * To address this issue update pmk table entry for
			 * roamed AP1 with pmk3 value comes to host via roam
			 * sync indication event. By this host override stale
			 * entry (if any) with the latest valid pmk for that AP
			 * at a point of time.
			 */
			if (roaming_info->pmk_len) {
				pmksa = qdf_mem_malloc(sizeof(*pmksa));
				if (!pmksa) {
					status = QDF_STATUS_E_NOMEM;
					qdf_mem_zero(pmkid_cache,
						     sizeof(*pmkid_cache));
					qdf_mem_free(pmkid_cache);
					goto end;
				}

				/*
				 * This pmksa buffer is to update the
				 * crypto table
				 */
				wlan_vdev_get_bss_peer_mac(vdev, &pmksa->bssid);
				qdf_mem_copy(pmksa->pmkid,
					     roaming_info->pmkid, PMKID_LEN);
				qdf_mem_copy(pmksa->pmk, roaming_info->pmk,
					     roaming_info->pmk_len);
				pmksa->pmk_len = roaming_info->pmk_len;
				status = wlan_crypto_set_del_pmksa(vdev,
								   pmksa, true);
				if (QDF_IS_STATUS_ERROR(status)) {
					qdf_mem_zero(pmksa, sizeof(*pmksa));
					qdf_mem_free(pmksa);
				}

				/* update the pmkid_cache buffer to
				 * update the global session pmk
				 */
				qdf_mem_copy(pmkid_cache->pmkid,
					     roaming_info->pmkid, PMKID_LEN);
				qdf_mem_copy(pmkid_cache->pmk,
					     roaming_info->pmk,
					     roaming_info->pmk_len);
				pmkid_cache->pmk_len = roaming_info->pmk_len;
			}

			wlan_cm_set_psk_pmk(pdev, vdev_id,
					    pmkid_cache->pmk,
					    pmkid_cache->pmk_len);
			mlme_debug(CM_PREFIX_FMT "pmkid found for "
				   QDF_MAC_ADDR_FMT " len %d",
				   CM_PREFIX_REF(vdev_id, cm_id),
				   QDF_MAC_ADDR_REF(pmkid_cache->bssid.bytes),
				   pmkid_cache->pmk_len);
		} else {
			mlme_debug(CM_PREFIX_FMT "PMKID Not found in cache for "
				   QDF_MAC_ADDR_FMT,
				   CM_PREFIX_REF(vdev_id, cm_id),
				   QDF_MAC_ADDR_REF(pmkid_cache->bssid.bytes));
			/*
			 * In FT roam when the CSR lookup fails then the PMK
			 * details from the roam sync indication will be
			 * updated to Session/PMK cache. This will result in
			 * having multiple PMK cache entries for the same MDID,
			 * So do not add the PMKSA cache entry in all the
			 * FT-Roam cases.
			 */
			if (!cm_is_auth_type_11r(mlme_obj, vdev,
						 mdie_present) &&
			    roaming_info->pmk_len) {
				/*
				 * This pmksa buffer is to update the
				 * crypto table
				 */
				pmksa = qdf_mem_malloc(sizeof(*pmksa));
				if (!pmksa) {
					status = QDF_STATUS_E_NOMEM;
					qdf_mem_zero(pmkid_cache,
						     sizeof(*pmkid_cache));
					qdf_mem_free(pmkid_cache);
					goto end;
				}
				wlan_cm_set_psk_pmk(pdev, vdev_id,
						    roaming_info->pmk,
						    roaming_info->pmk_len);
				wlan_vdev_get_bss_peer_mac(vdev,
							   &pmksa->bssid);
				qdf_mem_copy(pmksa->pmkid,
					     roaming_info->pmkid, PMKID_LEN);
				qdf_mem_copy(pmksa->pmk,
					     roaming_info->pmk,
					     roaming_info->pmk_len);
				pmksa->pmk_len = roaming_info->pmk_len;

				status = wlan_crypto_set_del_pmksa(vdev,
								   pmksa,
								   true);
				if (QDF_IS_STATUS_ERROR(status)) {
					qdf_mem_zero(pmksa, sizeof(*pmksa));
					qdf_mem_free(pmksa);
				}
			}
		}
		qdf_mem_zero(pmkid_cache, sizeof(*pmkid_cache));
		qdf_mem_free(pmkid_cache);
	}

	if (roaming_info->auth_status != ROAM_AUTH_STATUS_AUTHENTICATED)
		cm_update_wait_for_key_timer(vdev, vdev_id,
					     WAIT_FOR_KEY_TIMEOUT_PERIOD);
end:
	return status;
}

static void
cm_update_scan_db_on_roam_success(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_connect_resp *resp,
				  struct roam_offload_synch_ind *roam_synch_data,
				  wlan_cm_id cm_id)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_inform_bcn_probe(cm_ctx,
			    resp->connect_ies.bcn_probe_rsp.ptr,
			    resp->connect_ies.bcn_probe_rsp.len,
			    resp->freq,
			    roam_synch_data->rssi,
			    cm_id);

	cm_update_scan_mlme_on_roam(vdev, &resp->bssid,
				    SCAN_ENTRY_CON_STATE_ASSOC);
}

QDF_STATUS
cm_fw_roam_sync_propagation(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			    struct roam_offload_synch_ind *roam_synch_data)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;
	struct cm_roam_req *roam_req = NULL;
	struct cm_vdev_join_rsp *rsp = NULL;
	wlan_cm_id cm_id;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_cm_connect_resp *connect_rsp;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		status = QDF_STATUS_E_FAILURE;
		goto rel_ref;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		status = QDF_STATUS_E_FAILURE;
		goto rel_ref;
	}

	roam_req = cm_get_first_roam_command(vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		cm_id = CM_ID_INVALID;
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	wlan_rec_conn_info(vdev_id, DEBUG_CONN_ROAMING,
			   roam_synch_data->bssid.bytes, 0, 0);

	cm_id = roam_req->cm_id;
	rsp = qdf_mem_malloc(sizeof(struct cm_vdev_join_rsp));
	if (!rsp) {
		status = QDF_STATUS_E_NOMEM;
		goto error;
	}
	status = cm_fill_roam_info(vdev, roam_synch_data, rsp, cm_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT " fail to prepare rsp",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto error;
	}

	connect_rsp = &rsp->connect_rsp;
	cm_update_scan_db_on_roam_success(vdev, connect_rsp,
					  roam_synch_data, cm_id);

	status = cm_csr_connect_rsp(vdev, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Roam sync propagation failed, abort roaming");
		goto error;
	}

	cm_process_roam_keys(vdev, rsp, cm_id);
	mlme_cm_osif_connect_complete(vdev, connect_rsp);

	/**
	 * Don't send roam_sync complete for MLO link vdevs.
	 * Send only for legacy STA/MLO STA vdev.
	 */
	if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
		cm_if_mgr_inform_connect_complete(cm_ctx->vdev,
						  connect_rsp->connect_status);
		cm_inform_blm_connect_complete(cm_ctx->vdev, connect_rsp);
		wlan_tdls_notify_sta_connect(vdev_id,
					mlme_get_tdls_chan_switch_prohibited(vdev),
					mlme_get_tdls_prohibited(vdev), vdev);
		wlan_p2p_status_connect(vdev);

		if (!cm_csr_is_ss_wait_for_key(vdev_id)) {
			mlme_debug(CM_PREFIX_FMT "WLAN link up with AP = "
				   QDF_MAC_ADDR_FMT,
				   CM_PREFIX_REF(vdev_id, cm_id),
				   QDF_MAC_ADDR_REF(connect_rsp->bssid.bytes));
			cm_roam_start_init_on_connect(pdev, vdev_id);
		}
		wlan_cm_tgt_send_roam_sync_complete_cmd(psoc, vdev_id);

		mlo_roam_copy_partner_info(connect_rsp, roam_synch_data);
		mlo_roam_update_connected_links(vdev, connect_rsp);
	}
	cm_connect_info(vdev, true, &connect_rsp->bssid, &connect_rsp->ssid,
			connect_rsp->freq);

	status = cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_ROAM_DONE,
					  sizeof(*roam_synch_data),
					  roam_synch_data);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT " fail to post WLAN_CM_SM_EV_ROAM_DONE",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto error;
	}
	mlme_cm_osif_roam_complete(vdev);
	mlme_debug(CM_PREFIX_FMT, CM_PREFIX_REF(vdev_id, cm_id));
	if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev))
		cm_remove_cmd(cm_ctx, &cm_id);
	status = QDF_STATUS_SUCCESS;
error:
	if (rsp)
		wlan_cm_free_connect_rsp(rsp);

	if (QDF_IS_STATUS_ERROR(status)) {
		cm_roam_stop_req(psoc, vdev_id, REASON_ROAM_SYNCH_FAILED,
				 NULL, false);
		cm_abort_fw_roam(cm_ctx, cm_id);
		mlo_update_connected_links(vdev, 0);
	}
rel_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS cm_fw_roam_complete(struct cnx_mgr *cm_ctx, void *data)
{
	struct roam_offload_synch_ind *roam_synch_data;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id;

	roam_synch_data = (struct roam_offload_synch_ind *)data;
	vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err("Failed to find pdev");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!pdev) {
		mlme_err("Failed to find psoc");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/* Handle one race condition that if candidate is already
	 *selected & FW has gone ahead with roaming or about to go
	 * ahead when set_band comes, it will be complicated for FW
	 * to stop the current roaming. Instead, host will check the
	 * roam sync to make sure the new AP is not on disable freq
	 * or disconnect the AP.
	 */
	if (wlan_reg_is_disable_for_freq(pdev, roam_synch_data->chan_freq)) {
		mlo_disconnect(cm_ctx->vdev, CM_ROAM_DISCONNECT,
			       REASON_OPER_CHANNEL_BAND_CHANGE, NULL);
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/*
	 * Following operations need to be done once roam sync
	 * completion is sent to FW, hence called here:
	 * 1) Firmware has already updated DBS policy. Update connection
	 *	  table in the host driver.
	 * 2) Force SCC switch if needed
	 */
	/* first update connection info from wma interface */
	policy_mgr_update_connection_info(psoc, vdev_id);
	/* then update remaining parameters from roam sync ctx */
	if (roam_synch_data->hw_mode_trans_present)
		policy_mgr_hw_mode_transition_cb(
		roam_synch_data->hw_mode_trans_ind.old_hw_mode_index,
		roam_synch_data->hw_mode_trans_ind.new_hw_mode_index,
		roam_synch_data->hw_mode_trans_ind.num_vdev_mac_entries,
		roam_synch_data->hw_mode_trans_ind.vdev_mac_map,
		0, NULL, psoc);

	cm_check_and_set_sae_single_pmk_cap(psoc, vdev_id);
	cm_csr_send_set_ie(cm_ctx->vdev);

	if (ucfg_pkt_capture_get_pktcap_mode(psoc))
		ucfg_pkt_capture_record_channel(cm_ctx->vdev);

	if (WLAN_REG_IS_24GHZ_CH_FREQ(roam_synch_data->chan_freq)) {
		wlan_cm_set_disable_hi_rssi(pdev,
					    vdev_id, false);
	} else {
		wlan_cm_set_disable_hi_rssi(pdev,
					    vdev_id, true);
		mlme_debug("Disabling HI_RSSI, AP freq=%d rssi %d",
			   roam_synch_data->chan_freq, roam_synch_data->rssi);
	}
	policy_mgr_check_n_start_opportunistic_timer(psoc);

	wlan_cm_handle_sta_sta_roaming_enablement(psoc, vdev_id);

	if (wlan_vdev_mlme_is_mlo_link_vdev(cm_ctx->vdev)) {
		status = wlan_cm_roam_state_change(pdev,
						   vdev_id,
						   WLAN_ROAM_DEINIT,
						   REASON_ROAM_HANDOFF_DONE);
	}

	if (roam_synch_data->auth_status == ROAM_AUTH_STATUS_AUTHENTICATED)
		wlan_cm_roam_state_change(pdev, vdev_id,
					  WLAN_ROAM_RSO_ENABLED,
					  REASON_CONNECT);
	else
		/*
		 * STA is just in associated state here, RSO
		 * enable will be sent once EAP & EAPOL will be done by
		 * user-space and after set key response is received.
		 *
		 * When firmware roaming state is connected, EAP/EAPOL will be
		 * done at the supplicant. If EAP/EAPOL fails and supplicant
		 * sends disconnect, then the RSO state machine sends
		 * deinit directly to firmware without RSO stop with roam
		 * scan mode value 0. So to avoid this move state to RSO
		 * stop.
		 */
		wlan_cm_roam_state_change(pdev, vdev_id,
					  WLAN_ROAM_RSO_STOPPED,
					  REASON_DISCONNECTED);
	policy_mgr_check_concurrent_intf_and_restart_sap(psoc);
end:
	return status;
}

QDF_STATUS cm_fw_roam_invoke_fail(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;
	wlan_cm_id cm_id;
	enum wlan_cm_source source;
	struct cnx_mgr *cm_ctx;
	struct cm_roam_req *roam_req = NULL;
	struct qdf_mac_addr bssid;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	roam_req = cm_get_first_roam_command(vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	cm_id = roam_req->cm_id;
	source = roam_req->req.source;
	bssid = roam_req->req.bssid;

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_INVOKE_FAIL,
				     sizeof(wlan_cm_id), &cm_id);

	if (QDF_IS_STATUS_ERROR(status))
		cm_remove_cmd(cm_ctx, &cm_id);
	/*
	 * If reassoc MAC from user space is broadcast MAC as:
	 * "wpa_cli DRIVER FASTREASSOC ff:ff:ff:ff:ff:ff 0",
	 * user space invoked roaming candidate selection will base on firmware
	 * score algorithm, current connection will be kept if current AP has
	 * highest score. It is requirement from customer which can avoid
	 * ping-pong roaming.
	 */
	if (qdf_is_macaddr_broadcast(&bssid))
		mlme_debug("Keep current connection");
	else if (source == CM_ROAMING_HOST || source == CM_ROAMING_NUD_FAILURE)
		status = mlo_disconnect(vdev, CM_ROAM_DISCONNECT,
					REASON_USER_TRIGGERED_ROAM_FAILURE,
					NULL);

error:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	return status;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
static void cm_ho_fail_diag_event(void)
{
	WLAN_HOST_DIAG_EVENT_DEF(roam_connection,
				 host_event_wlan_status_payload_type);
	qdf_mem_zero(&roam_connection,
		     sizeof(host_event_wlan_status_payload_type));

	roam_connection.eventId = DIAG_WLAN_STATUS_DISCONNECT;
	roam_connection.reason = DIAG_REASON_ROAM_HO_FAIL;
	WLAN_HOST_DIAG_EVENT_REPORT(&roam_connection, EVENT_WLAN_STATUS_V2);
}
#else
static inline void cm_ho_fail_diag_event(void) {}
#endif

static QDF_STATUS cm_handle_ho_fail(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct cm_ho_fail_ind *ind = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct cnx_mgr *cm_ctx;
	wlan_cm_id cm_id = CM_ID_INVALID;
	struct reject_ap_info ap_info;
	struct cm_roam_req *roam_req = NULL;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_objmgr_psoc *psoc;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	ind = msg->bodyptr;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(ind->psoc, ind->vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d : vdev not found", ind->vdev_id);
		qdf_mem_free(ind);
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("pdev object is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("psoc object is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to mlme psoc obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	roam_req = cm_get_first_roam_command(vdev);
	if (roam_req) {
		mlme_debug("Roam req found, get cm id to remove it, before disconnect");
		cm_id = roam_req->cm_id;
	}

	cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_HO_FAIL,
			    sizeof(wlan_cm_id), &cm_id);

	qdf_mem_zero(&ap_info, sizeof(struct reject_ap_info));
	ap_info.bssid = ind->bssid;
	ap_info.reject_ap_type = DRIVER_AVOID_TYPE;
	ap_info.reject_reason = REASON_ROAM_HO_FAILURE;
	ap_info.source = ADDED_BY_DRIVER;
	wlan_blm_add_bssid_to_reject_list(pdev, &ap_info);

	cm_ho_fail_diag_event();
	wlan_roam_debug_log(ind->vdev_id,
			    DEBUG_ROAM_SYNCH_FAIL,
			    DEBUG_INVALID_PEER_ID, NULL, NULL, 0, 0);

	status = mlo_disconnect(vdev, CM_MLME_DISCONNECT,
				REASON_FW_TRIGGERED_ROAM_FAILURE, NULL);

	if (mlme_obj->cfg.gen.fatal_event_trigger)
		cds_flush_logs(WLAN_LOG_TYPE_FATAL,
			       WLAN_LOG_INDICATOR_HOST_DRIVER,
			       WLAN_LOG_REASON_ROAM_HO_FAILURE, false, false);

	if (QDF_IS_STATUS_ERROR(status))
		cm_remove_cmd(cm_ctx, &cm_id);

error:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	qdf_mem_free(ind);

	return QDF_STATUS_SUCCESS;
}

void cm_fw_ho_fail_req(struct wlan_objmgr_psoc *psoc,
		       uint8_t vdev_id, struct qdf_mac_addr bssid)
{
	QDF_STATUS status;
	struct scheduler_msg ind_msg = {0};
	struct cm_ho_fail_ind *ind = NULL;

	ind = qdf_mem_malloc(sizeof(*ind));
	if (!ind)
		return;

	ind->vdev_id = vdev_id;
	ind->psoc = psoc;
	ind->bssid = bssid;

	ind_msg.bodyptr = ind;
	ind_msg.callback = cm_handle_ho_fail;

	status = scheduler_post_message(QDF_MODULE_ID_MLME, QDF_MODULE_ID_OS_IF,
					QDF_MODULE_ID_OS_IF, &ind_msg);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to post HO fail indication on vdev_id %d",
			 vdev_id);
		qdf_mem_free(ind);
		return;
	}
}

QDF_STATUS wlan_cm_free_roam_synch_frame_ind(struct rso_config *rso_cfg)
{
	struct roam_synch_frame_ind *frame_ind;

	if (!rso_cfg)
		return QDF_STATUS_E_FAILURE;

	frame_ind = &rso_cfg->roam_sync_frame_ind;

	if (frame_ind->bcn_probe_rsp) {
		qdf_mem_free(frame_ind->bcn_probe_rsp);
		frame_ind->bcn_probe_rsp_len = 0;
		frame_ind->bcn_probe_rsp = NULL;
	}
	if (frame_ind->reassoc_req) {
		qdf_mem_free(frame_ind->reassoc_req);
		frame_ind->reassoc_req_len = 0;
		frame_ind->reassoc_req = NULL;
	}
	if (frame_ind->reassoc_rsp) {
		qdf_mem_free(frame_ind->reassoc_rsp);
		frame_ind->reassoc_rsp_len = 0;
		frame_ind->reassoc_rsp = NULL;
	}

	return QDF_STATUS_SUCCESS;
}
