/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: wlan_cm_roam_api.c
 *
 * Implementation for the Common Roaming interfaces.
 */

#include "wlan_cm_roam_api.h"
#include "wlan_vdev_mlme_api.h"
#include "wlan_mlme_main.h"
#include "wlan_policy_mgr_api.h"
#include <wmi_unified_priv.h>
#include <../../core/src/wlan_cm_vdev_api.h>
#include "wlan_crypto_global_api.h"
#include <wlan_cm_api.h>
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "wlan_cm_roam_api.h"
#include "wlan_blm_api.h"
#include <../../core/src/wlan_cm_roam_i.h>
#include "wlan_reg_ucfg_api.h"


/* Support for "Fast roaming" (i.e., ESE, LFR, or 802.11r.) */
#define BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN 15
#define CM_MIN_RSSI 0 /* 0dbm */

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
QDF_STATUS
wlan_cm_enable_roaming_on_connected_sta(struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id)
{
	uint32_t op_ch_freq_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t sta_vdev_id = WLAN_INVALID_VDEV_ID;
	uint32_t count;
	uint32_t idx;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	sta_vdev_id = policy_mgr_get_roam_enabled_sta_session_id(psoc, vdev_id);
	if (sta_vdev_id != WLAN_UMAC_VDEV_ID_MAX)
		return QDF_STATUS_E_FAILURE;

	count = policy_mgr_get_mode_specific_conn_info(psoc,
						       op_ch_freq_list,
						       vdev_id_list,
						       PM_STA_MODE);

	if (!count)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Loop through all connected STA vdevs and roaming will be enabled
	 * on the STA that has a different vdev id from the one passed as
	 * input and has non zero roam trigger value.
	 */
	for (idx = 0; idx < count; idx++) {
		if (vdev_id_list[idx] != vdev_id &&
		    mlme_get_roam_trigger_bitmap(psoc, vdev_id_list[idx])) {
			sta_vdev_id = vdev_id_list[idx];
			break;
		}
	}

	if (sta_vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_FAILURE;

	mlme_debug("ROAM: Enabling roaming on vdev[%d]", sta_vdev_id);

	return cm_roam_state_change(pdev,
				    sta_vdev_id,
				    WLAN_ROAM_RSO_ENABLED,
				    REASON_CTX_INIT);
}

QDF_STATUS wlan_cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
				     uint8_t vdev_id,
				     enum roam_offload_state requested_state,
				     uint8_t reason)
{
	return cm_roam_state_change(pdev, vdev_id, requested_state, reason);
}

QDF_STATUS wlan_cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id, uint8_t rso_command,
				     uint8_t reason)
{
	return cm_roam_send_rso_cmd(psoc, vdev_id, rso_command, reason);
}

void wlan_cm_handle_sta_sta_roaming_enablement(struct wlan_objmgr_psoc *psoc,
					       uint8_t vdev_id)
{
	return cm_handle_sta_sta_roaming_enablement(psoc, vdev_id);
}

QDF_STATUS
wlan_roam_update_cfg(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		     uint8_t reason)
{
	if (!MLME_IS_ROAM_STATE_RSO_ENABLED(psoc, vdev_id)) {
		mlme_debug("Update cfg received while ROAM RSO not started");
		return QDF_STATUS_E_INVAL;
	}

	return cm_roam_send_rso_cmd(psoc, vdev_id, ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				    reason);
}

#endif

char *cm_roam_get_requestor_string(enum wlan_cm_rso_control_requestor requestor)
{
	switch (requestor) {
	case RSO_INVALID_REQUESTOR:
	default:
		return "No requestor";
	case RSO_START_BSS:
		return "SAP start";
	case RSO_CHANNEL_SWITCH:
		return "CSA";
	case RSO_CONNECT_START:
		return "STA connection";
	case RSO_SAP_CHANNEL_CHANGE:
		return "SAP Ch switch";
	case RSO_NDP_CON_ON_NDI:
		return "NDP connection";
	case RSO_SET_PCL:
		return "Set PCL";
	}
}

QDF_STATUS
wlan_cm_rso_set_roam_trigger(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			     struct wlan_roam_triggers *trigger)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = cm_roam_acquire_lock(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;

	status = cm_rso_set_roam_trigger(pdev, vdev_id, trigger);

	cm_roam_release_lock(vdev);

release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

QDF_STATUS wlan_cm_disable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			       enum wlan_cm_rso_control_requestor requestor,
			       uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = cm_roam_acquire_lock(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;

	if (reason == REASON_DRIVER_DISABLED && requestor)
		mlme_set_operations_bitmap(psoc, vdev_id, requestor, false);

	mlme_debug("ROAM_CONFIG: vdev[%d] Disable roaming - requestor:%s",
		   vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STOPPED,
				      REASON_DRIVER_DISABLED);
	cm_roam_release_lock(vdev);

release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

QDF_STATUS wlan_cm_enable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			      enum wlan_cm_rso_control_requestor requestor,
			      uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (reason == REASON_DRIVER_ENABLED && requestor)
		mlme_set_operations_bitmap(psoc, vdev_id, requestor, true);

	status = cm_roam_acquire_lock(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;

	mlme_debug("ROAM_CONFIG: vdev[%d] Enable roaming - requestor:%s",
		   vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_ENABLED,
				      REASON_DRIVER_ENABLED);
	cm_roam_release_lock(vdev);

release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

bool wlan_cm_host_roam_in_progress(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	bool host_roam_in_progress = false;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return host_roam_in_progress;
	}

	if (wlan_cm_is_vdev_roam_preauth_state(vdev) ||
	    wlan_cm_is_vdev_roam_reassoc_state(vdev))
		host_roam_in_progress = true;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return host_roam_in_progress;
}

QDF_STATUS wlan_cm_abort_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_roam_acquire_lock(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;

	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id) ||
	    wlan_cm_host_roam_in_progress(psoc, vdev_id)) {
		cm_roam_release_lock(vdev);
		status = QDF_STATUS_E_BUSY;
		goto release_ref;
	}

	/* RSO stop cmd will be issued with lock held to avoid
	 * any racing conditions with wma/csr layer
	 */
	wlan_cm_disable_rso(pdev, vdev_id, REASON_DRIVER_DISABLED,
			    RSO_INVALID_REQUESTOR);

	cm_roam_release_lock(vdev);
release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

bool wlan_cm_roaming_in_progress(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	bool roaming_in_progress = false;
	struct wlan_objmgr_vdev *vdev;
	enum QDF_OPMODE opmode;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev)
		return roaming_in_progress;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE)
		goto exit;

	roaming_in_progress = wlan_cm_is_vdev_roaming(vdev);

exit:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return roaming_in_progress;
}

QDF_STATUS wlan_cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				 uint8_t reason)
{
	return cm_roam_stop_req(psoc, vdev_id, reason);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
wlan_cm_fw_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	return cm_fw_roam_abort_req(psoc, vdev_id);
}

QDF_STATUS
wlan_cm_roam_extract_btm_response(wmi_unified_t wmi, void *evt_buf,
				  struct roam_btm_response_data *dst,
				  uint8_t idx)
{
	if (wmi->ops->extract_roam_btm_response_stats)
		return wmi->ops->extract_roam_btm_response_stats(wmi, evt_buf,
								 dst, idx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wlan_cm_roam_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				       struct roam_initial_data *dst,
				       uint8_t idx)
{
	if (wmi->ops->extract_roam_initial_info)
		return wmi->ops->extract_roam_initial_info(wmi, evt_buf,
							   dst, idx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wlan_cm_roam_extract_roam_msg_info(wmi_unified_t wmi, void *evt_buf,
				   struct roam_msg_info *dst, uint8_t idx)
{
	if (wmi->ops->extract_roam_msg_info)
		return wmi->ops->extract_roam_msg_info(wmi, evt_buf, dst, idx);

	return QDF_STATUS_E_FAILURE;
}

uint32_t wlan_cm_get_roam_band_value(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev)
{
	struct cm_roam_values_copy config;
	uint32_t band_mask;

	wlan_cm_roam_cfg_get_value(psoc, wlan_vdev_get_id(vdev), ROAM_BAND,
				   &config);

	band_mask = config.uint_value;
	mlme_debug("[ROAM BAND] band mask:%d", band_mask);
	return band_mask;
}

QDF_STATUS
wlan_cm_roam_extract_frame_info(wmi_unified_t wmi, void *evt_buf,
				struct roam_frame_info *dst, uint8_t idx,
				uint8_t num_frames)
{
	if (wmi->ops->extract_roam_msg_info)
		return wmi->ops->extract_roam_frame_info(wmi, evt_buf,
							 dst, idx, num_frames);

	return QDF_STATUS_E_FAILURE;
}

void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id, bool pcl_per_vdev)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err("vdev legacy private object is NULL");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	/* value - true (vdev pcl) false - pdev pcl */
	mlme_priv->cm_roam.pcl_vdev_cmd_active = pcl_per_vdev;
	mlme_debug("CM_ROAM: vdev[%d] SET PCL cmd level - [%s]", vdev_id,
		   pcl_per_vdev ? "VDEV" : "PDEV");
}

bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return false;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err("vdev legacy private object is NULL");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return false;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return mlme_priv->cm_roam.pcl_vdev_cmd_active;
}

bool
wlan_cm_dual_sta_is_freq_allowed(struct wlan_objmgr_psoc *psoc,
				 uint32_t freq,
				 enum QDF_OPMODE opmode)
{
	uint32_t op_ch_freq_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	enum reg_wifi_band band;
	uint32_t count, connected_sta_freq;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct dual_sta_policy *dual_sta_policy;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Check if primary iface is configured. If yes,
	 * then allow further STA connection to all
	 * available bands/channels irrespective of first
	 * STA connection band, which allow driver to
	 * connect with the best available AP present in
	 * environment, so that user can switch to second
	 * connection and mark it as primary.
	 */
	dual_sta_policy = &mlme_obj->cfg.gen.dual_sta_policy;
	if (dual_sta_policy->primary_vdev_id != WLAN_UMAC_VDEV_ID_MAX) {
		mlme_debug("primary iface is configured, vdev_id: %d",
			   dual_sta_policy->primary_vdev_id);
		return true;
	}

	/*
	 * Check if already there is 1 STA connected. If this API is
	 * called for 2nd STA and if dual sta roaming is enabled, then
	 * don't allow the intra band frequencies of the 1st sta for
	 * connection on 2nd STA.
	 */
	count = policy_mgr_get_mode_specific_conn_info(psoc, op_ch_freq_list,
						       vdev_id_list,
						       PM_STA_MODE);
	if (!count || !wlan_mlme_get_dual_sta_roaming_enabled(psoc) ||
	    opmode != QDF_STA_MODE)
		return true;

	connected_sta_freq = op_ch_freq_list[0];
	band = wlan_reg_freq_to_band(connected_sta_freq);
	if ((band == REG_BAND_2G && WLAN_REG_IS_24GHZ_CH_FREQ(freq)) ||
	    (band == REG_BAND_5G && !WLAN_REG_IS_24GHZ_CH_FREQ(freq)))
		return false;

	return true;
}

void
wlan_cm_dual_sta_roam_update_connect_channels(struct wlan_objmgr_psoc *psoc,
					      struct scan_filter *filter)
{
	uint32_t i, num_channels = 0;
	uint32_t *channel_list;
	bool is_ch_allowed;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_mlme_cfg *mlme_cfg;
	struct dual_sta_policy *dual_sta_policy;
	uint32_t buff_len;
	char *chan_buff;
	uint32_t len = 0;
	uint32_t sta_count;

	filter->num_of_channels = 0;
	sta_count = policy_mgr_mode_specific_connection_count(psoc,
							PM_STA_MODE, NULL);

	/* No need to fill freq list, if no other STA is in conencted state */
	if (!sta_count)
		return;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;
	dual_sta_policy = &mlme_obj->cfg.gen.dual_sta_policy;
	mlme_cfg = &mlme_obj->cfg;

	mlme_debug("sta_count %d, primary vdev is %d dual sta roaming enabled %d",
		   sta_count, dual_sta_policy->primary_vdev_id,
		   wlan_mlme_get_dual_sta_roaming_enabled(psoc));

	if (!wlan_mlme_get_dual_sta_roaming_enabled(psoc))
		return;

	/*
	 * Check if primary iface is configured. If yes,
	 * then allow further STA connection to all
	 * available bands/channels irrespective of first
	 * STA connection band, which allow driver to
	 * connect with the best available AP present in
	 * environment, so that user can switch to second
	 * connection and mark it as primary.
	 */
	if (dual_sta_policy->primary_vdev_id != WLAN_UMAC_VDEV_ID_MAX)
		return;

	/*
	 * Get Reg domain valid channels and update to the scan filter
	 * if already 1st sta is in connected state. Don't allow channels
	 * on which the 1st STA is connected.
	 */
	num_channels = mlme_cfg->reg.valid_channel_list_num;
	channel_list = mlme_cfg->reg.valid_channel_freq_list;

	/*
	 * Buffer of (num channl * 5) + 1  to consider the 4 char freq,
	 * 1 space after it for each channel and 1 to end the string
	 * with NULL.
	 */
	buff_len = (num_channels * 5) + 1;
	chan_buff = qdf_mem_malloc(buff_len);
	if (!chan_buff)
		return;

	for (i = 0; i < num_channels; i++) {
		is_ch_allowed =
			wlan_cm_dual_sta_is_freq_allowed(psoc, channel_list[i],
							 QDF_STA_MODE);
		if (!is_ch_allowed)
			continue;

		filter->chan_freq_list[filter->num_of_channels] =
					channel_list[i];
		filter->num_of_channels++;

		len += qdf_scnprintf(chan_buff + len, buff_len - len,
				     "%d ", channel_list[i]);
	}

	if (filter->num_of_channels)
		mlme_debug("Freq list (%d): %s", filter->num_of_channels,
			   chan_buff);

	qdf_mem_free(chan_buff);
}

void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_cm_roam_vendor_btm_params *param)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	qdf_mem_copy(&mlme_obj->cfg.lfr.vendor_btm_param, param,
		     sizeof(struct wlan_cm_roam_vendor_btm_params));
}

void
wlan_cm_roam_get_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_cm_roam_vendor_btm_params *param)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	qdf_mem_copy(param, &mlme_obj->cfg.lfr.vendor_btm_param,
		     sizeof(struct wlan_cm_roam_vendor_btm_params));
}

void wlan_cm_set_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t pmk_len)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev)
		return;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}
	qdf_mem_zero(rso_cfg->psk_pmk, sizeof(rso_cfg->psk_pmk));
	if (psk_pmk)
		qdf_mem_copy(rso_cfg->psk_pmk, psk_pmk, pmk_len);
	rso_cfg->pmk_len = pmk_len;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void wlan_cm_get_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t *pmk_len)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	if (!psk_pmk || !pmk_len)
		return;
	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}
	qdf_mem_copy(psk_pmk, rso_cfg->psk_pmk, rso_cfg->pmk_len);
	*pmk_len = rso_cfg->pmk_len;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void
wlan_cm_roam_get_score_delta_params(struct wlan_objmgr_psoc *psoc,
				    struct wlan_roam_triggers *params)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	params->score_delta_param[IDLE_ROAM_TRIGGER] =
			mlme_obj->cfg.trig_score_delta[IDLE_ROAM_TRIGGER];
	params->score_delta_param[BTM_ROAM_TRIGGER] =
			mlme_obj->cfg.trig_score_delta[BTM_ROAM_TRIGGER];
}

void
wlan_cm_roam_get_min_rssi_params(struct wlan_objmgr_psoc *psoc,
				 struct wlan_roam_triggers *params)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	params->min_rssi_params[DEAUTH_MIN_RSSI] =
			mlme_obj->cfg.trig_min_rssi[DEAUTH_MIN_RSSI];
	params->min_rssi_params[BMISS_MIN_RSSI] =
			mlme_obj->cfg.trig_min_rssi[BMISS_MIN_RSSI];
	params->min_rssi_params[MIN_RSSI_2G_TO_5G_ROAM] =
			mlme_obj->cfg.trig_min_rssi[MIN_RSSI_2G_TO_5G_ROAM];
}
#endif

QDF_STATUS wlan_cm_roam_cfg_get_value(struct wlan_objmgr_psoc *psoc,
				      uint8_t vdev_id,
				      enum roam_cfg_param roam_cfg_type,
				      struct cm_roam_values_copy *dst_config)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct rso_config *rso_cfg;
	struct rso_cfg_params *src_cfg;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	qdf_mem_zero(dst_config, sizeof(*dst_config));
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}
	src_cfg = &rso_cfg->cfg_param;
	switch (roam_cfg_type) {
	case RSSI_CHANGE_THRESHOLD:
		dst_config->int_value = rso_cfg->rescan_rssi_delta;
		break;
	case BEACON_RSSI_WEIGHT:
		dst_config->uint_value = rso_cfg->beacon_rssi_weight;
		break;
	case HI_RSSI_DELAY_BTW_SCANS:
		dst_config->uint_value = rso_cfg->hi_rssi_scan_delay;
		break;
	case EMPTY_SCAN_REFRESH_PERIOD:
		dst_config->uint_value = src_cfg->empty_scan_refresh_period;
		break;
	case SCAN_MIN_CHAN_TIME:
		dst_config->uint_value = src_cfg->min_chan_scan_time;
		break;
	case SCAN_MAX_CHAN_TIME:
		dst_config->uint_value = src_cfg->max_chan_scan_time;
		break;
	case NEIGHBOR_SCAN_PERIOD:
		dst_config->uint_value = src_cfg->neighbor_scan_period;
		break;
	case FULL_ROAM_SCAN_PERIOD:
		dst_config->uint_value = src_cfg->full_roam_scan_period;
		break;
	case ROAM_RSSI_DIFF:
		dst_config->uint_value = src_cfg->roam_rssi_diff;
		break;
	case NEIGHBOUR_LOOKUP_THRESHOLD:
		dst_config->uint_value = src_cfg->neighbor_lookup_threshold;
		break;
	case SCAN_N_PROBE:
		dst_config->uint_value = src_cfg->roam_scan_n_probes;
		break;
	case SCAN_HOME_AWAY:
		dst_config->uint_value = src_cfg->roam_scan_home_away_time;
		break;
	case NEIGHBOUR_SCAN_REFRESH_PERIOD:
		dst_config->uint_value =
				src_cfg->neighbor_results_refresh_period;
		break;
	case ROAM_CONTROL_ENABLE:
		dst_config->bool_value = rso_cfg->roam_control_enable;
		break;
	case UAPSD_MASK:
		dst_config->uint_value = rso_cfg->uapsd_mask;
		break;
	case MOBILITY_DOMAIN:
		dst_config->bool_value = rso_cfg->mdid.mdie_present;
		dst_config->uint_value = rso_cfg->mdid.mobility_domain;
		break;
	case IS_11R_CONNECTION:
		dst_config->bool_value = rso_cfg->is_11r_assoc;
		break;
	case ADAPTIVE_11R_CONNECTION:
		dst_config->bool_value = rso_cfg->is_adaptive_11r_connection;
		break;
	case HS_20_AP:
		dst_config->bool_value = rso_cfg->hs_20_ap;
		break;
	case MBO_OCE_ENABLED_AP:
		dst_config->uint_value = rso_cfg->mbo_oce_enabled_ap;
		break;
	case IS_SINGLE_PMK:
		dst_config->bool_value = rso_cfg->is_single_pmk;
		break;
	case LOST_LINK_RSSI:
		dst_config->int_value = rso_cfg->lost_link_rssi;
		break;
	case ROAM_BAND:
		dst_config->uint_value = rso_cfg->roam_band_bitmask;
		break;
	default:
		mlme_err("Invalid roam config requested:%d", roam_cfg_type);
		status = QDF_STATUS_E_FAILURE;
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

void wlan_cm_set_disable_hi_rssi(struct wlan_objmgr_pdev *pdev,
				 uint8_t vdev_id, bool value)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}

	rso_cfg->disable_hi_rssi = value;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void wlan_cm_set_country_code(struct wlan_objmgr_pdev *pdev,
			      uint8_t vdev_id, uint8_t  *cc)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg || !cc)
		goto release_vdev_ref;

	mlme_debug("Country info from bcn IE:%c%c 0x%x", cc[0], cc[1], cc[2]);

	qdf_mem_copy(rso_cfg->country_code, cc, REG_ALPHA2_LEN + 1);

release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

QDF_STATUS wlan_cm_get_country_code(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id, uint8_t *cc)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg || !cc) {
		status = QDF_STATUS_E_INVAL;
		goto release_vdev_ref;
	}

	qdf_mem_copy(cc, rso_cfg->country_code, REG_ALPHA2_LEN + 1);

release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	return status;
}

#ifdef FEATURE_WLAN_ESE
void wlan_cm_set_ese_assoc(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id, bool value)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}

	rso_cfg->is_ese_assoc = value;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

bool wlan_cm_get_ese_assoc(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id)
{
	static struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	bool ese_assoc;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return false;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return false;
	}

	ese_assoc = rso_cfg->is_ese_assoc;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return ese_assoc;
}
#endif

static QDF_STATUS
cm_roam_update_cfg(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		   uint8_t reason)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = cm_roam_acquire_lock(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;
	if (!MLME_IS_ROAM_STATE_RSO_ENABLED(psoc, vdev_id)) {
		mlme_debug("Update cfg received while ROAM RSO not started");
		cm_roam_release_lock(vdev);
		status = QDF_STATUS_E_INVAL;
		goto release_ref;
	}

	status = cm_roam_send_rso_cmd(psoc, vdev_id,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG, reason);
	cm_roam_release_lock(vdev);

release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

void cm_dump_freq_list(struct rso_chan_info *chan_info)
{
	uint8_t *channel_list;
	uint8_t i = 0, j = 0;
	uint32_t buflen = CFG_VALID_CHANNEL_LIST_LEN * 4;

	channel_list = qdf_mem_malloc(buflen);
	if (!channel_list)
		return;

	if (chan_info->freq_list) {
		for (i = 0; i < chan_info->num_chan; i++) {
			if (j < buflen)
				j += qdf_scnprintf(channel_list + j, buflen - j,
						   "%d ",
						   chan_info->freq_list[i]);
			else
				break;
		}
	}

	mlme_debug("frequency list [%u]: %s", i, channel_list);
	qdf_mem_free(channel_list);
}

static uint8_t
cm_append_pref_chan_list(struct rso_chan_info *chan_info, qdf_freq_t *freq_list,
			 uint8_t num_chan)
{
	uint8_t i = 0, j = 0;

	for (i = 0; i < chan_info->num_chan; i++) {
		for (j = 0; j < num_chan; j++)
			if (chan_info->freq_list[i] == freq_list[j])
				break;

		if (j < num_chan)
			continue;
		if (num_chan == CFG_VALID_CHANNEL_LIST_LEN)
			break;
		freq_list[num_chan++] = chan_info->freq_list[i];
	}

	return num_chan;
}

/**
 * cm_modify_chan_list_based_on_band() - Modify RSO channel list based on band
 * @freq_list: Channel list coming from user space
 * @num_chan: Number of channel present in freq_list buffer
 * @band_bitmap: On basis of this band host modify RSO channel list
 *
 * Return: valid number of channel as per bandmap
 */
static uint8_t
cm_modify_chan_list_based_on_band(qdf_freq_t *freq_list, uint8_t num_chan,
				  uint32_t band_bitmap)
{
	uint8_t i = 0, valid_chan_num = 0;

	if (!(band_bitmap & BIT(REG_BAND_2G))) {
		mlme_debug("disabling 2G");
		for (i = 0; i < num_chan; i++) {
			if (WLAN_REG_IS_24GHZ_CH_FREQ(freq_list[i]))
				freq_list[i] = 0;
		}
	}

	if (!(band_bitmap & BIT(REG_BAND_5G))) {
		mlme_debug("disabling 5G");
		for (i = 0; i < num_chan; i++) {
			if (WLAN_REG_IS_5GHZ_CH_FREQ(freq_list[i]))
				freq_list[i] = 0;
		}
	}

	if (!(band_bitmap & BIT(REG_BAND_6G))) {
		mlme_debug("disabling 6G");
		for (i = 0; i < num_chan; i++) {
			if (WLAN_REG_IS_6GHZ_CHAN_FREQ(freq_list[i]))
				freq_list[i] = 0;
		}
	}

	for (i = 0; i < num_chan; i++) {
		if (freq_list[i])
			freq_list[valid_chan_num++] = freq_list[i];
	}

	return valid_chan_num;
}

static QDF_STATUS cm_create_bg_scan_roam_channel_list(struct rso_chan_info *chan_info,
						const qdf_freq_t *chan_freq_lst,
						const uint8_t num_chan)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i;

	chan_info->freq_list = qdf_mem_malloc(sizeof(qdf_freq_t) * num_chan);
	if (!chan_info->freq_list)
		return QDF_STATUS_E_NOMEM;

	chan_info->num_chan = num_chan;
	for (i = 0; i < num_chan; i++)
		chan_info->freq_list[i] = chan_freq_lst[i];

	return status;
}

/**
 * cm_remove_disabled_channels() - Remove disabled channels as per current
 * connected band
 * @vdev: vdev common object
 * @freq_list: Channel list coming from user space
 * @num_chan: Number of channel present in freq_list buffer
 *
 * Return: Number of channels as per SETBAND mask
 */
static uint32_t cm_remove_disabled_channels(struct wlan_objmgr_vdev *vdev,
					    qdf_freq_t *freq_list,
					    uint8_t num_chan)
{
	struct regulatory_channel *cur_chan_list;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	uint32_t valid_chan_num = 0;
	enum channel_state state;
	uint32_t freq, i, j;
	QDF_STATUS status;
	uint32_t filtered_lst[NUM_CHANNELS] = {0};

	cur_chan_list =
	     qdf_mem_malloc(NUM_CHANNELS * sizeof(struct regulatory_channel));
	if (!cur_chan_list)
		return 0;

	status = wlan_reg_get_current_chan_list(pdev, cur_chan_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(cur_chan_list);
		return 0;
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		freq = cur_chan_list[i].center_freq;
		state = wlan_reg_get_channel_state_for_freq(pdev, freq);
		if (state != CHANNEL_STATE_DISABLE &&
		    state != CHANNEL_STATE_INVALID) {
			for (j = 0; j < num_chan; j++) {
				if (freq == freq_list[j]) {
					filtered_lst[valid_chan_num++] =
								freq_list[j];
					break;
				}
			}
		}
	}

	mlme_debug("[ROAM_BAND]: num channel :%d", valid_chan_num);
	for (i = 0; i < valid_chan_num; i++)
		freq_list[i] = filtered_lst[i];

	qdf_mem_free(cur_chan_list);

	return valid_chan_num;
}

/**
 * cm_update_roam_scan_channel_list() - Update channel list as per RSO chan info
 * band bitmask
 * @psoc: Psoc common object
 * @vdev: vdev common object
 * @rso_cfg: connect config to be used to send info in RSO
 * @vdev_id: vdev id
 * @chan_info: hannel list already sent via RSO
 * @freq_list: Channel list coming from user space
 * @num_chan: Number of channel present in freq_list buffer
 * @update_preferred_chan: Decide whether to update preferred chan list or not
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_update_roam_scan_channel_list(struct wlan_objmgr_psoc *psoc,
				 struct wlan_objmgr_vdev *vdev,
				 struct rso_config *rso_cfg, uint8_t vdev_id,
				 struct rso_chan_info *chan_info,
				 qdf_freq_t *freq_list, uint8_t num_chan,
				 bool update_preferred_chan)
{
	uint16_t pref_chan_cnt = 0;
	uint32_t valid_chan_num = 0;
	struct cm_roam_values_copy config;
	uint32_t current_band;

	if (chan_info->num_chan) {
		mlme_debug("Current channel num: %d", chan_info->num_chan);
		cm_dump_freq_list(chan_info);
	}

	if (update_preferred_chan) {
		pref_chan_cnt = cm_append_pref_chan_list(chan_info, freq_list,
							 num_chan);
		num_chan = pref_chan_cnt;
	}

	num_chan = cm_remove_disabled_channels(vdev, freq_list, num_chan);
	if (!num_chan)
		return QDF_STATUS_E_FAILURE;

	wlan_cm_roam_cfg_get_value(psoc, vdev_id, ROAM_BAND, &config);
	ucfg_reg_get_band(wlan_vdev_get_pdev(vdev), &current_band);
	/* No need to modify channel list if all channel is allowed */
	if (config.uint_value != current_band) {
		valid_chan_num =
			cm_modify_chan_list_based_on_band(freq_list, num_chan,
							  config.uint_value);
		if (!valid_chan_num) {
			mlme_debug("No valid channels left to send to the fw");
			return QDF_STATUS_E_FAILURE;
		}
		num_chan = valid_chan_num;
	}

	cm_flush_roam_channel_list(chan_info);
	cm_create_bg_scan_roam_channel_list(chan_info, freq_list, num_chan);

	mlme_debug("New channel num: %d", num_chan);
	cm_dump_freq_list(chan_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cm_roam_cfg_set_value(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   enum roam_cfg_param roam_cfg_type,
			   struct cm_roam_values_copy *src_config)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct rso_config *rso_cfg;
	struct rso_cfg_params *dst_cfg;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}
	dst_cfg = &rso_cfg->cfg_param;
	mlme_debug("roam_cfg_type %d, uint val %d int val %d bool val %d num chan %d",
		   roam_cfg_type, src_config->uint_value, src_config->int_value,
		   src_config->bool_value, src_config->chan_info.num_chan);
	switch (roam_cfg_type) {
	case RSSI_CHANGE_THRESHOLD:
		rso_cfg->rescan_rssi_delta  = src_config->uint_value;
		break;
	case BEACON_RSSI_WEIGHT:
		rso_cfg->beacon_rssi_weight = src_config->uint_value;
		break;
	case HI_RSSI_DELAY_BTW_SCANS:
		rso_cfg->hi_rssi_scan_delay = src_config->uint_value;
		break;
	case EMPTY_SCAN_REFRESH_PERIOD:
		dst_cfg->empty_scan_refresh_period = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					  REASON_EMPTY_SCAN_REF_PERIOD_CHANGED);
		break;
	case FULL_ROAM_SCAN_PERIOD:
		dst_cfg->full_roam_scan_period = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					  REASON_ROAM_FULL_SCAN_PERIOD_CHANGED);
		break;
	case ENABLE_SCORING_FOR_ROAM:
		dst_cfg->enable_scoring_for_roam = src_config->bool_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_SCORING_CRITERIA_CHANGED);
		break;
	case SCAN_MIN_CHAN_TIME:
		mlme_obj->cfg.lfr.neighbor_scan_min_chan_time =
							src_config->uint_value;
		dst_cfg->min_chan_scan_time = src_config->uint_value;
		break;
	case SCAN_MAX_CHAN_TIME:
		dst_cfg->max_chan_scan_time = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_SCAN_CH_TIME_CHANGED);
		break;
	case NEIGHBOR_SCAN_PERIOD:
		dst_cfg->neighbor_scan_period = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_SCAN_HOME_TIME_CHANGED);
		break;
	case ROAM_CONFIG_ENABLE:
		rso_cfg->roam_control_enable = src_config->bool_value;
		if (!rso_cfg->roam_control_enable)
			break;
		dst_cfg->roam_scan_period_after_inactivity = 0;
		dst_cfg->roam_inactive_data_packet_count = 0;
		dst_cfg->roam_scan_inactivity_time = 0;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_ROAM_CONTROL_CONFIG_ENABLED);
		break;
	case ROAM_PREFERRED_CHAN:
		/*
		 * In RSO update command, the specific channel list is
		 * given priority. So flush the Specific channel list if
		 * preferred channel list is received. Else the channel list
		 * type will be filled as static instead of dynamic.
		 */
		cm_flush_roam_channel_list(&dst_cfg->specific_chan_info);
		status = cm_update_roam_scan_channel_list(psoc, vdev, rso_cfg,
					vdev_id, &dst_cfg->pref_chan_info,
					src_config->chan_info.freq_list,
					src_config->chan_info.num_chan, true);
		if (QDF_IS_STATUS_ERROR(status))
			break;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_CHANNEL_LIST_CHANGED);
		break;
	case ROAM_SPECIFIC_CHAN:
		status = cm_update_roam_scan_channel_list(psoc, vdev, rso_cfg,
					vdev_id, &dst_cfg->specific_chan_info,
					src_config->chan_info.freq_list,
					src_config->chan_info.num_chan,
					false);
		if (QDF_IS_STATUS_ERROR(status))
			break;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_CHANNEL_LIST_CHANGED);
		break;
	case ROAM_RSSI_DIFF:
		dst_cfg->roam_rssi_diff = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_RSSI_DIFF_CHANGED);
		break;
	case NEIGHBOUR_LOOKUP_THRESHOLD:
		dst_cfg->neighbor_lookup_threshold = src_config->uint_value;
		break;
	case SCAN_N_PROBE:
		dst_cfg->roam_scan_n_probes = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_NPROBES_CHANGED);
		break;
	case SCAN_HOME_AWAY:
		dst_cfg->roam_scan_home_away_time = src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled &&
		    src_config->bool_value)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_HOME_AWAY_TIME_CHANGED);
		break;
	case NEIGHBOUR_SCAN_REFRESH_PERIOD:
		dst_cfg->neighbor_results_refresh_period =
						src_config->uint_value;
		mlme_obj->cfg.lfr.neighbor_scan_results_refresh_period =
				src_config->uint_value;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
				REASON_NEIGHBOR_SCAN_REFRESH_PERIOD_CHANGED);
		break;
	case UAPSD_MASK:
		rso_cfg->uapsd_mask = src_config->uint_value;
		break;
	case MOBILITY_DOMAIN:
		rso_cfg->mdid.mdie_present = src_config->bool_value;
		rso_cfg->mdid.mobility_domain = src_config->uint_value;
		break;
	case IS_11R_CONNECTION:
		rso_cfg->is_11r_assoc = src_config->bool_value;
		break;
	case ADAPTIVE_11R_CONNECTION:
		rso_cfg->is_adaptive_11r_connection = src_config->bool_value;
		break;
	case HS_20_AP:
		rso_cfg->hs_20_ap  = src_config->bool_value;
		break;
	case MBO_OCE_ENABLED_AP:
		rso_cfg->mbo_oce_enabled_ap  = src_config->uint_value;
		break;
	case IS_SINGLE_PMK:
		rso_cfg->is_single_pmk = src_config->bool_value;
		break;
	case LOST_LINK_RSSI:
		rso_cfg->lost_link_rssi = src_config->int_value;
		break;
	case ROAM_BAND:
		rso_cfg->roam_band_bitmask = src_config->uint_value;
		mlme_debug("[ROAM BAND] Set roam band:%d",
			   rso_cfg->roam_band_bitmask);
		break;
	default:
		mlme_err("Invalid roam config requested:%d", roam_cfg_type);
		status = QDF_STATUS_E_FAILURE;
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

void wlan_roam_reset_roam_params(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct rso_config_params *rso_usr_cfg;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	rso_usr_cfg = &mlme_obj->cfg.lfr.rso_user_config;

	/*
	 * clear all the whitelist parameters and remaining
	 * needs to be retained across connections.
	 */
	rso_usr_cfg->num_ssid_allowed_list = 0;
	qdf_mem_zero(&rso_usr_cfg->ssid_allowed_list,
		     sizeof(struct wlan_ssid) * MAX_SSID_ALLOWED_LIST);
}

static void cm_rso_chan_to_freq_list(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t *freq_list,
				     const uint8_t *chan_list,
				     uint32_t chan_list_len)
{
	uint32_t count;

	for (count = 0; count < chan_list_len; count++)
		freq_list[count] =
			wlan_reg_legacy_chan_to_freq(pdev, chan_list[count]);
}

#ifdef WLAN_FEATURE_HOST_ROAM
static QDF_STATUS cm_init_reassoc_timer(struct rso_config *rso_cfg)
{
	QDF_STATUS status;

	status = qdf_mc_timer_init(&rso_cfg->reassoc_timer, QDF_TIMER_TYPE_SW,
				   cm_reassoc_timer_callback, &rso_cfg->ctx);

	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("Preauth Reassoc interval Timer allocation failed");

	return status;
}

static void cm_deinit_reassoc_timer(struct rso_config *rso_cfg)
{
	/* check if the timer is running */
	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&rso_cfg->reassoc_timer))
		qdf_mc_timer_stop(&rso_cfg->reassoc_timer);

	qdf_mc_timer_destroy(&rso_cfg->reassoc_timer);
}
#else
static inline QDF_STATUS cm_init_reassoc_timer(struct rso_config *rso_cfg)
{
	return QDF_STATUS_SUCCESS;
}
static inline void cm_deinit_reassoc_timer(struct rso_config *rso_cfg) {}
#endif

QDF_STATUS wlan_cm_rso_config_init(struct wlan_objmgr_vdev *vdev,
				   struct rso_config *rso_cfg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct rso_chan_info *chan_info;
	struct rso_cfg_params *cfg_params;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	status = cm_init_reassoc_timer(rso_cfg);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	qdf_mutex_create(&rso_cfg->cm_rso_lock);
	cfg_params = &rso_cfg->cfg_param;
	cfg_params->max_chan_scan_time =
		mlme_obj->cfg.lfr.neighbor_scan_max_chan_time;
	cfg_params->min_chan_scan_time =
		mlme_obj->cfg.lfr.neighbor_scan_min_chan_time;
	cfg_params->neighbor_lookup_threshold =
		mlme_obj->cfg.lfr.neighbor_lookup_rssi_threshold;
	cfg_params->rssi_thresh_offset_5g =
		mlme_obj->cfg.lfr.rssi_threshold_offset_5g;
	cfg_params->opportunistic_threshold_diff =
		mlme_obj->cfg.lfr.opportunistic_scan_threshold_diff;
	cfg_params->roam_rescan_rssi_diff =
		mlme_obj->cfg.lfr.roam_rescan_rssi_diff;

	cfg_params->roam_bmiss_first_bcn_cnt =
		mlme_obj->cfg.lfr.roam_bmiss_first_bcnt;
	cfg_params->roam_bmiss_final_cnt =
		mlme_obj->cfg.lfr.roam_bmiss_final_bcnt;

	cfg_params->neighbor_scan_period =
		mlme_obj->cfg.lfr.neighbor_scan_timer_period;
	cfg_params->neighbor_scan_min_period =
		mlme_obj->cfg.lfr.neighbor_scan_min_timer_period;
	cfg_params->neighbor_results_refresh_period =
		mlme_obj->cfg.lfr.neighbor_scan_results_refresh_period;
	cfg_params->empty_scan_refresh_period =
		mlme_obj->cfg.lfr.empty_scan_refresh_period;
	cfg_params->full_roam_scan_period =
		mlme_obj->cfg.lfr.roam_full_scan_period;
	cfg_params->enable_scoring_for_roam =
		mlme_obj->cfg.roam_scoring.enable_scoring_for_roam;
	cfg_params->roam_scan_n_probes =
		mlme_obj->cfg.lfr.roam_scan_n_probes;
	cfg_params->roam_scan_home_away_time =
		mlme_obj->cfg.lfr.roam_scan_home_away_time;
	cfg_params->roam_scan_inactivity_time =
		mlme_obj->cfg.lfr.roam_scan_inactivity_time;
	cfg_params->roam_inactive_data_packet_count =
		mlme_obj->cfg.lfr.roam_inactive_data_packet_count;
	cfg_params->roam_scan_period_after_inactivity =
		mlme_obj->cfg.lfr.roam_scan_period_after_inactivity;

	chan_info = &cfg_params->specific_chan_info;
	chan_info->num_chan =
		mlme_obj->cfg.lfr.neighbor_scan_channel_list_num;
	mlme_debug("number of channels: %u", chan_info->num_chan);
	if (chan_info->num_chan) {
		chan_info->freq_list =
			qdf_mem_malloc(sizeof(qdf_freq_t) *
				       chan_info->num_chan);
		if (!chan_info->freq_list) {
			chan_info->num_chan = 0;
			return QDF_STATUS_E_NOMEM;
		}
		/* Update the roam global structure from CFG */
		cm_rso_chan_to_freq_list(pdev, chan_info->freq_list,
			mlme_obj->cfg.lfr.neighbor_scan_channel_list,
			mlme_obj->cfg.lfr.neighbor_scan_channel_list_num);
	} else {
		chan_info->freq_list = NULL;
	}

	cfg_params->hi_rssi_scan_max_count =
		mlme_obj->cfg.lfr.roam_scan_hi_rssi_maxcount;
	cfg_params->hi_rssi_scan_rssi_delta =
		mlme_obj->cfg.lfr.roam_scan_hi_rssi_delta;

	cfg_params->hi_rssi_scan_delay =
		mlme_obj->cfg.lfr.roam_scan_hi_rssi_delay;

	cfg_params->hi_rssi_scan_rssi_ub =
		mlme_obj->cfg.lfr.roam_scan_hi_rssi_ub;
	cfg_params->roam_rssi_diff =
		mlme_obj->cfg.lfr.roam_rssi_diff;
	cfg_params->bg_rssi_threshold =
		mlme_obj->cfg.lfr.bg_rssi_threshold;

	return status;
}

void wlan_cm_rso_config_deinit(struct wlan_objmgr_vdev *vdev,
			       struct rso_config *rso_cfg)
{
	struct rso_cfg_params *cfg_params;

	cfg_params = &rso_cfg->cfg_param;
	if (rso_cfg->assoc_ie.ptr) {
		qdf_mem_free(rso_cfg->assoc_ie.ptr);
		rso_cfg->assoc_ie.ptr = NULL;
		rso_cfg->assoc_ie.len = 0;
	}
	if (rso_cfg->prev_ap_bcn_ie.ptr) {
		qdf_mem_free(rso_cfg->prev_ap_bcn_ie.ptr);
		rso_cfg->prev_ap_bcn_ie.ptr = NULL;
		rso_cfg->prev_ap_bcn_ie.len = 0;
	}
	if (rso_cfg->roam_scan_freq_lst.freq_list)
		qdf_mem_free(rso_cfg->roam_scan_freq_lst.freq_list);
	rso_cfg->roam_scan_freq_lst.freq_list = NULL;
	rso_cfg->roam_scan_freq_lst.num_chan = 0;

	cm_flush_roam_channel_list(&cfg_params->specific_chan_info);
	cm_flush_roam_channel_list(&cfg_params->pref_chan_info);

	qdf_mutex_destroy(&rso_cfg->cm_rso_lock);

	cm_deinit_reassoc_timer(rso_cfg);
}

struct rso_config *wlan_cm_get_rso_config_fl(struct wlan_objmgr_vdev *vdev,
					     const char *func, uint32_t line)

{
	struct cm_ext_obj *cm_ext_obj;
	enum QDF_OPMODE op_mode = wlan_vdev_mlme_get_opmode(vdev);

	/* get only for CLI and STA */
	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return NULL;

	cm_ext_obj = cm_get_ext_hdl_fl(vdev, func, line);
	if (!cm_ext_obj)
		return NULL;

	return &cm_ext_obj->rso_cfg;
}

QDF_STATUS cm_roam_acquire_lock(struct wlan_objmgr_vdev *vdev)
{
	static struct rso_config *rso_cfg;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_INVAL;

	return qdf_mutex_acquire(&rso_cfg->cm_rso_lock);
}

QDF_STATUS cm_roam_release_lock(struct wlan_objmgr_vdev *vdev)
{
	static struct rso_config *rso_cfg;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_INVAL;

	return qdf_mutex_release(&rso_cfg->cm_rso_lock);
}

QDF_STATUS
wlan_cm_roam_invoke(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
		    struct qdf_mac_addr *bssid, qdf_freq_t chan_freq,
		    enum wlan_cm_source source)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("Invalid psoc");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = cm_start_roam_invoke(psoc, vdev, bssid, chan_freq, source);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

bool cm_is_fast_roam_enabled(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return false;

	if (!mlme_obj->cfg.lfr.lfr_enabled)
		return false;

	if (mlme_obj->cfg.lfr.enable_fast_roam_in_concurrency)
		return true;
	/* return true if no concurency */
	if (policy_mgr_get_connection_count(psoc) < 2)
		return true;

	return false;
}

bool cm_is_rsn_or_8021x_sha256_auth_type(struct wlan_objmgr_vdev *vdev)
{
	int32_t akm;

	akm = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		return true;

	return false;
}

#ifdef WLAN_FEATURE_HOST_ROAM
QDF_STATUS wlan_cm_host_roam_start(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct cm_host_roam_start_ind *req;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_ZERO_INIT;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	req = msg->bodyptr;
	status = wlan_cm_roam_invoke(req->pdev, req->vdev_id, &bssid, 0,
				     CM_ROAMING_FW);
	qdf_mem_free(req);

	return status;
}

QDF_STATUS cm_mlme_roam_preauth_fail(struct wlan_objmgr_vdev *vdev,
				     struct wlan_cm_roam_req *req,
				     enum wlan_cm_connect_fail_reason reason)
{
	uint8_t vdev_id, roam_reason;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev || !req) {
		mlme_err("vdev or req is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (reason == CM_NO_CANDIDATE_FOUND)
		roam_reason = REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW;
	else
		roam_reason = REASON_PREAUTH_FAILED_FOR_ALL;

	pdev = wlan_vdev_get_pdev(vdev);
	vdev_id = wlan_vdev_get_id(vdev);

	if (req->source == CM_ROAMING_FW)
		cm_roam_state_change(pdev, vdev_id,
				     ROAM_SCAN_OFFLOAD_RESTART,
				     roam_reason);
	else
		cm_roam_state_change(pdev, vdev_id,
				     ROAM_SCAN_OFFLOAD_START,
				     roam_reason);
	return QDF_STATUS_SUCCESS;
}
#endif

void wlan_cm_fill_crypto_filter_from_vdev(struct wlan_objmgr_vdev *vdev,
					  struct scan_filter *filter)
{
	struct rso_config *rso_cfg;

	filter->authmodeset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_AUTH_MODE);
	filter->mcastcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MCAST_CIPHER);
	filter->ucastcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	filter->key_mgmt =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	filter->mgmtcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MGMT_CIPHER);

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return;

	if (rso_cfg->rsn_cap & WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED)
		filter->pmf_cap = WLAN_PMF_REQUIRED;
	else if (rso_cfg->rsn_cap & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED)
		filter->pmf_cap = WLAN_PMF_CAPABLE;
}

static void cm_dump_occupied_chan_list(struct wlan_chan_list *occupied_ch)
{
	uint8_t idx;
	uint32_t buff_len;
	char *chan_buff;
	uint32_t len = 0;

	buff_len = (occupied_ch->num_chan * 5) + 1;
	chan_buff = qdf_mem_malloc(buff_len);
	if (!chan_buff)
		return;

	for (idx = 0; idx < occupied_ch->num_chan; idx++)
		len += qdf_scnprintf(chan_buff + len, buff_len - len, " %d",
				     occupied_ch->freq_list[idx]);

	mlme_nofl_debug("Occupied chan list[%d]:%s",
			occupied_ch->num_chan, chan_buff);

	qdf_mem_free(chan_buff);
}

/**
 * cm_should_add_to_occupied_channels() - validates bands of active_ch_freq and
 * curr node freq before addition of curr node freq to occupied channels
 *
 * @active_ch_freq: active channel frequency
 * @cur_node_chan_freq: curr channel frequency
 * @dual_sta_roam_active: dual sta roam active
 *
 * Return: True if active_ch_freq and cur_node_chan_freq belongs to same
 * bands else false
 **/
static bool cm_should_add_to_occupied_channels(qdf_freq_t active_ch_freq,
					       qdf_freq_t cur_node_chan_freq,
					       bool dual_sta_roam_active)
{
	/* all channels can be added if dual STA roam is not active */
	if (!dual_sta_roam_active)
		return true;

	/* when dual STA roam is active, channels must be in the same band */
	if (WLAN_REG_IS_24GHZ_CH_FREQ(active_ch_freq) &&
	    WLAN_REG_IS_24GHZ_CH_FREQ(cur_node_chan_freq))
		return true;

	if (!WLAN_REG_IS_24GHZ_CH_FREQ(active_ch_freq) &&
	    !WLAN_REG_IS_24GHZ_CH_FREQ(cur_node_chan_freq))
		return true;

	/* not in same band */
	return false;
}

static QDF_STATUS cm_add_to_freq_list_front(qdf_freq_t *ch_freq_lst,
					    int num_chan, qdf_freq_t chan_freq)
{
	int i = 0;

	/* Check for NULL pointer */
	if (!ch_freq_lst)
		return QDF_STATUS_E_NULL_VALUE;

	/* Make room for the addition.  (Start moving from the back.) */
	for (i = num_chan; i > 0; i--)
		ch_freq_lst[i] = ch_freq_lst[i - 1];

	/* Now add the NEW channel...at the front */
	ch_freq_lst[0] = chan_freq;

	return QDF_STATUS_SUCCESS;
}

/* Add the channel to the occupied channels array */
static void cm_add_to_occupied_channels(qdf_freq_t ch_freq,
					struct rso_config *rso_cfg,
					bool is_init_list)
{
	QDF_STATUS status;
	uint8_t num_occupied_ch = rso_cfg->occupied_chan_lst.num_chan;
	qdf_freq_t *occupied_ch_lst = rso_cfg->occupied_chan_lst.freq_list;

	if (is_init_list)
		rso_cfg->roam_candidate_count++;

	if (wlan_is_channel_present_in_list(occupied_ch_lst,
					    num_occupied_ch, ch_freq))
		return;

	if (num_occupied_ch >= CFG_VALID_CHANNEL_LIST_LEN)
		num_occupied_ch = CFG_VALID_CHANNEL_LIST_LEN - 1;

	status = cm_add_to_freq_list_front(occupied_ch_lst,
					   num_occupied_ch, ch_freq);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		rso_cfg->occupied_chan_lst.num_chan++;
		if (rso_cfg->occupied_chan_lst.num_chan >
		    BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN)
			rso_cfg->occupied_chan_lst.num_chan =
				BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN;
	}
}

void wlan_cm_init_occupied_ch_freq_list(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id)
{
	qdf_list_t *list = NULL;
	qdf_list_node_t *cur_lst = NULL;
	qdf_list_node_t *next_lst = NULL;
	struct scan_cache_node *cur_node = NULL;
	struct scan_filter *filter;
	bool dual_sta_roam_active;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;
	struct rso_config *rso_cfg;
	struct rso_cfg_params *cfg_params;
	struct wlan_ssid ssid;
	qdf_freq_t op_freq, freq;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;
	op_freq = wlan_get_operation_chan_freq(vdev);
	if (!op_freq) {
		mlme_debug("failed to get op freq");
		goto rel_vdev_ref;
	}
	status = wlan_vdev_mlme_get_ssid(vdev, ssid.ssid, &ssid.length);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("failed to find SSID for vdev %d", vdev_id);
		goto rel_vdev_ref;
	}

	cfg_params = &rso_cfg->cfg_param;

	if (cfg_params->specific_chan_info.num_chan) {
		/*
		 * Ini file contains neighbor scan channel list, hence NO need
		 * to build occupied channel list"
		 */
		mlme_debug("Ini contains neighbor scan ch list");
		goto rel_vdev_ref;
	}

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		goto rel_vdev_ref;

	wlan_cm_fill_crypto_filter_from_vdev(vdev, filter);
	filter->num_of_ssid = 1;
	qdf_mem_copy(&filter->ssid_list[0], &ssid, sizeof(ssid));

	/* Empty occupied channels here */
	rso_cfg->occupied_chan_lst.num_chan = 0;
	rso_cfg->roam_candidate_count = 0;

	cm_add_to_occupied_channels(op_freq, rso_cfg, true);
	list = wlan_scan_get_result(pdev, filter);
	qdf_mem_free(filter);
	if (!list || (list && !qdf_list_size(list)))
		goto err;

	dual_sta_roam_active =
			wlan_mlme_get_dual_sta_roaming_enabled(psoc);
	dual_sta_roam_active = dual_sta_roam_active &&
			       policy_mgr_mode_specific_connection_count
				(psoc, PM_STA_MODE, NULL) >= 2;

	qdf_list_peek_front(list, &cur_lst);
	while (cur_lst) {
		cur_node = qdf_container_of(cur_lst, struct scan_cache_node,
					    node);
		freq = cur_node->entry->channel.chan_freq;
		if (cm_should_add_to_occupied_channels(op_freq, freq,
						       dual_sta_roam_active))
			cm_add_to_occupied_channels(freq, rso_cfg, true);

		qdf_list_peek_next(list, cur_lst, &next_lst);
		cur_lst = next_lst;
		next_lst = NULL;
	}
err:
	cm_dump_occupied_chan_list(&rso_cfg->occupied_chan_lst);
	if (list)
		wlan_scan_purge_results(list);
rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS
wlan_cm_update_mlme_fils_info(struct wlan_objmgr_vdev *vdev,
			      struct wlan_fils_con_info *src_fils_info)
{
	struct mlme_legacy_priv *mlme_priv;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct wlan_fils_connection_info *tgt_info;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err("vdev legacy private object is NULL fro vdev %d",
			 vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (!src_fils_info) {
		mlme_debug("FILS: vdev:%d Clear fils info", vdev_id);
		qdf_mem_free(mlme_priv->connect_info.fils_con_info);
		mlme_priv->connect_info.fils_con_info = NULL;
		return QDF_STATUS_SUCCESS;
	}

	if (mlme_priv->connect_info.fils_con_info)
		qdf_mem_free(mlme_priv->connect_info.fils_con_info);

	mlme_priv->connect_info.fils_con_info =
		qdf_mem_malloc(sizeof(struct wlan_fils_connection_info));
	if (!mlme_priv->connect_info.fils_con_info)
		return QDF_STATUS_E_NOMEM;

	tgt_info = mlme_priv->connect_info.fils_con_info;
	mlme_debug("FILS: vdev:%d update fils info", vdev_id);
	tgt_info->is_fils_connection = src_fils_info->is_fils_connection;
	tgt_info->key_nai_length = src_fils_info->username_len;
	qdf_mem_copy(tgt_info->keyname_nai, src_fils_info->username,
		     tgt_info->key_nai_length);

	tgt_info->realm_len = src_fils_info->realm_len;
	qdf_mem_copy(tgt_info->realm, src_fils_info->realm,
		     tgt_info->realm_len);

	tgt_info->r_rk_length = src_fils_info->rrk_len;
	qdf_mem_copy(tgt_info->r_rk, src_fils_info->rrk,
		     tgt_info->r_rk_length);
	tgt_info->erp_sequence_number = src_fils_info->next_seq_num;
	tgt_info->auth_type = src_fils_info->auth_type;

	return QDF_STATUS_SUCCESS;
}

void wlan_cm_update_hlp_info(struct wlan_objmgr_psoc *psoc,
			     const uint8_t *gen_ie, uint16_t len,
			     uint8_t vdev_id, bool flush)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev_id %d", vdev_id);
		return;
	}

	cm_update_hlp_info(vdev, gen_ie, len, flush);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
}

struct wlan_fils_connection_info *wlan_cm_get_fils_connection_info(
				struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;
	struct wlan_fils_connection_info *fils_info;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return NULL;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err("vdev legacy private object is NULL");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return NULL;
	}

	fils_info = mlme_priv->connect_info.fils_con_info;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return fils_info;
}

QDF_STATUS wlan_cm_update_fils_ft(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint8_t *fils_ft,
				  uint8_t fils_ft_len)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err("vdev legacy private object is NULL");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}

	if (!mlme_priv->connect_info.fils_con_info || !fils_ft ||
	    !fils_ft_len ||
	    !mlme_priv->connect_info.fils_con_info->is_fils_connection) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv->connect_info.fils_con_info->fils_ft_len = fils_ft_len;
	qdf_mem_copy(mlme_priv->connect_info.fils_con_info->fils_ft, fils_ft,
		     fils_ft_len);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
wlan_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap)
{
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);

	if (!vdev) {
		mlme_err("vdev%d: vdev object is NULL", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}
	rso_cfg->roam_scan_scheme_bitmap = roam_scan_scheme_bitmap;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_set_roam_band_bitmask(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id,
					 uint32_t roam_band_bitmask)
{
	struct cm_roam_values_copy src_config;

	src_config.uint_value = roam_band_bitmask;
	return wlan_cm_roam_cfg_set_value(psoc, vdev_id, ROAM_BAND,
					  &src_config);
}

QDF_STATUS wlan_cm_set_roam_band_update(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id)
{
	return cm_roam_update_cfg(psoc, vdev_id,
				  REASON_ROAM_CONTROL_CONFIG_ENABLED);
}

uint32_t wlan_cm_get_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	uint32_t roam_scan_scheme_bitmap;
	struct rso_config *rso_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);

	if (!vdev) {
		mlme_err("vdev%d: vdev object is NULL", vdev_id);
		return 0;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return 0;
	}

	roam_scan_scheme_bitmap = rso_cfg->roam_scan_scheme_bitmap;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return roam_scan_scheme_bitmap;
}

QDF_STATUS
wlan_cm_update_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   uint32_t value, enum roam_fail_params states)
{
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);

	if (!vdev) {
		mlme_err("vdev%d: vdev object is NULL", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_FAILURE;
	}

	switch (states) {
	case ROAM_TRIGGER_REASON:
		rso_cfg->roam_trigger_reason = value;
		break;
	case ROAM_INVOKE_FAIL_REASON:
		rso_cfg->roam_invoke_fail_reason = value;
		break;
	case ROAM_FAIL_REASON:
		rso_cfg->roam_fail_reason = value;
		break;
	default:
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return QDF_STATUS_SUCCESS;
}

uint32_t wlan_cm_get_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				 enum roam_fail_params states)
{
	struct wlan_objmgr_vdev *vdev;
	uint32_t roam_states = 0;
	struct rso_config *rso_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);

	if (!vdev) {
		mlme_err("vdev%d: vdev object is NULL", vdev_id);
		return 0;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return 0;
	}

	switch (states) {
	case ROAM_TRIGGER_REASON:
		roam_states = rso_cfg->roam_trigger_reason;
		break;
	case ROAM_INVOKE_FAIL_REASON:
		roam_states = rso_cfg->roam_invoke_fail_reason;
		break;
	case ROAM_FAIL_REASON:
		roam_states = rso_cfg->roam_fail_reason;
		break;
	default:
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return roam_states;
}
#endif

QDF_STATUS wlan_get_chan_by_bssid_from_rnr(struct wlan_objmgr_vdev *vdev,
					   wlan_cm_id cm_id,
					   struct qdf_mac_addr *link_addr,
					   uint8_t *chan, uint8_t *op_class)
{
	struct reduced_neighbor_report *rnr;
	int i;

	*chan = 0;

	rnr = wlan_cm_get_rnr(vdev, cm_id);

	if (!rnr) {
		mlme_err("no rnr IE is gotten");
		return QDF_STATUS_E_EMPTY;
	}

	for (i = 0; i < MAX_RNR_BSS; i++) {
		if (!rnr->bss_info[i].channel_number)
			continue;
		if (qdf_is_macaddr_equal(link_addr, &rnr->bss_info[i].bssid)) {
			*chan = rnr->bss_info[i].channel_number;
			*op_class = rnr->bss_info[i].operating_class;
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * mlo_rnr_link_id_cmp() - compare given link id with link id in rnr
 * @rnr_bss_info: rnr bss info
 * @link_id: link id
 *
 * Return: true if given link id is the same with link id in rnr
 */
static bool mlo_rnr_link_id_cmp(struct rnr_bss_info *rnr_bss_info,
				uint8_t link_id)
{
	if (rnr_bss_info)
		return link_id == rnr_bss_info->mld_info.link_id;

	return false;
}

QDF_STATUS wlan_get_chan_by_link_id_from_rnr(struct wlan_objmgr_vdev *vdev,
					     wlan_cm_id cm_id,
					     uint8_t link_id,
					     uint8_t *chan, uint8_t *op_class)
{
	struct reduced_neighbor_report *rnr;
	int i;

	*chan = 0;

	rnr = wlan_cm_get_rnr(vdev, cm_id);

	if (!rnr) {
		mlme_err("no rnr IE is gotten");
		return QDF_STATUS_E_EMPTY;
	}

	for (i = 0; i < MAX_RNR_BSS; i++) {
		if (!rnr->bss_info[i].channel_number)
			continue;
		if (mlo_rnr_link_id_cmp(&rnr->bss_info[i], link_id)) {
			*chan = rnr->bss_info[i].channel_number;
			*op_class = rnr->bss_info[i].operating_class;
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS wlan_cm_sta_mlme_vdev_roam_notify(struct vdev_mlme_obj *vdev_mlme,
					     uint16_t data_len, void *data)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	status = cm_roam_sync_event_handler_cb(vdev_mlme->vdev, data, data_len);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("Failed to process roam synch event");
#endif
	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void
cm_handle_roam_offload_events(struct roam_offload_roam_event *roam_event)
{
	switch (roam_event->reason) {
	case ROAM_REASON_HO_FAILED: {
		struct qdf_mac_addr bssid;

		bssid.bytes[0] = roam_event->notif_params >> 0 & 0xFF;
		bssid.bytes[1] = roam_event->notif_params >> 8 & 0xFF;
		bssid.bytes[2] = roam_event->notif_params >> 16 & 0xFF;
		bssid.bytes[3] = roam_event->notif_params >> 24 & 0xFF;
		bssid.bytes[4] = roam_event->notif_params1 >> 0 & 0xFF;
		bssid.bytes[5] = roam_event->notif_params1 >> 8 & 0xFF;
		cm_handle_roam_reason_ho_failed(roam_event->vdev_id, bssid,
						roam_event->hw_mode_trans_ind);
	}
	break;
	case ROAM_REASON_INVALID:
		cm_invalid_roam_reason_handler(roam_event->vdev_id,
					       roam_event->notif);
		break;
	default:
		break;
	}
}

QDF_STATUS
cm_vdev_disconnect_event_handler(struct vdev_disconnect_event_data *data)
{
	return cm_handle_disconnect_reason(data);
}

QDF_STATUS
cm_roam_auth_offload_event_handler(struct auth_offload_event *auth_event)
{
	return cm_handle_auth_offload(auth_event);
}

QDF_STATUS
cm_roam_pmkid_request_handler(struct roam_pmkid_req_event *data)
{
	QDF_STATUS status;

	status = cm_roam_pmkid_req_ind(data->psoc, data->vdev_id, data);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Pmkid request failed");
		qdf_mem_free(data);
		return status;
	}

	return status;
}
#else
static void
cm_handle_roam_offload_events(struct roam_offload_roam_event *roam_event)
{
	mlme_debug("Unhandled roam event with reason 0x%x for vdev_id %u",
		   roam_event->reason, roam_event->vdev_id);
}

QDF_STATUS
cm_vdev_disconnect_event_handler(struct vdev_disconnect_event_data *data)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_roam_pmkid_request_handler(struct roam_pmkid_req_event *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
cm_roam_event_handler(struct roam_offload_roam_event *roam_event)
{
	switch (roam_event->reason) {
	case ROAM_REASON_BTM:
		cm_handle_roam_reason_btm(roam_event->vdev_id);
		break;
	case ROAM_REASON_BMISS:
		cm_handle_roam_reason_bmiss(roam_event->vdev_id,
					    roam_event->rssi);
		break;
	case ROAM_REASON_BETTER_AP:
		cm_handle_roam_reason_better_ap(roam_event->vdev_id,
						roam_event->rssi);
		break;
	case ROAM_REASON_SUITABLE_AP:
		cm_handle_roam_reason_suitable_ap(roam_event->vdev_id,
						  roam_event->rssi);
		break;
	case ROAM_REASON_HO_FAILED:
	case ROAM_REASON_INVALID:
		cm_handle_roam_offload_events(roam_event);
		break;
	case ROAM_REASON_RSO_STATUS:
		cm_rso_cmd_status_event_handler(roam_event->vdev_id,
						roam_event->notif);
		break;
	case ROAM_REASON_INVOKE_ROAM_FAIL:
		cm_handle_roam_reason_invoke_roam_fail(roam_event->vdev_id,
						roam_event->notif_params,
						roam_event->hw_mode_trans_ind);
		break;
	case ROAM_REASON_DEAUTH:
		cm_handle_roam_reason_deauth(roam_event->vdev_id,
					     roam_event->notif_params,
					     roam_event->deauth_disassoc_frame,
					     roam_event->notif_params1);
		break;
	default:
		mlme_debug("Unhandled roam event with reason 0x%x for vdev_id %u",
			   roam_event->reason, roam_event->vdev_id);
		break;
	}

	return QDF_STATUS_SUCCESS;
}

static void
cm_add_bssid_to_reject_list(struct wlan_objmgr_pdev *pdev,
			    struct sir_rssi_disallow_lst *entry)
{
	struct reject_ap_info ap_info;

	qdf_mem_zero(&ap_info, sizeof(struct reject_ap_info));

	ap_info.bssid = entry->bssid;
	ap_info.reject_ap_type = DRIVER_RSSI_REJECT_TYPE;
	ap_info.rssi_reject_params.expected_rssi = entry->expected_rssi;
	ap_info.rssi_reject_params.retry_delay = entry->retry_delay;
	ap_info.reject_reason = entry->reject_reason;
	ap_info.source = entry->source;
	ap_info.rssi_reject_params.received_time = entry->received_time;
	ap_info.rssi_reject_params.original_timeout = entry->original_timeout;
	/* Add this ap info to the rssi reject ap type in blacklist manager */
	wlan_blm_add_bssid_to_reject_list(pdev, &ap_info);
}

QDF_STATUS
cm_btm_blacklist_event_handler(struct wlan_objmgr_psoc *psoc,
			       struct roam_blacklist_event *list)
{
	uint32_t i, pdev_id;
	struct sir_rssi_disallow_lst entry;
	struct roam_blacklist_timeout *blacklist;
	struct wlan_objmgr_pdev *pdev;

	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, list->vdev_id,
						WLAN_MLME_CM_ID);
	if (pdev_id == WLAN_INVALID_PDEV_ID) {
		mlme_err("Invalid pdev id");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_MLME_CM_ID);
	if (!pdev) {
		mlme_err("Invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	mlme_debug("Received Blacklist event from FW num entries %d",
		   list->num_entries);
	blacklist = &list->roam_blacklist[0];
	for (i = 0; i < list->num_entries; i++) {
		qdf_mem_zero(&entry, sizeof(struct sir_rssi_disallow_lst));
		entry.bssid = blacklist->bssid;
		entry.time_during_rejection = blacklist->received_time;
		entry.reject_reason = blacklist->reject_reason;
		entry.source = blacklist->source ? blacklist->source :
						   ADDED_BY_TARGET;
		entry.original_timeout = blacklist->original_timeout;
		entry.received_time = blacklist->received_time;
		/* If timeout = 0 and rssi = 0 ignore the entry */
		if (!blacklist->timeout && !blacklist->rssi) {
			continue;
		} else if (blacklist->timeout) {
			entry.retry_delay = blacklist->timeout;
			/* set 0dbm as expected rssi */
			entry.expected_rssi = CM_MIN_RSSI;
		} else {
			/* blacklist timeout as 0 */
			entry.retry_delay = blacklist->timeout;
			entry.expected_rssi = blacklist->rssi;
		}

		/* Add this bssid to the rssi reject ap type in blacklist mgr */
		cm_add_bssid_to_reject_list(pdev, &entry);
		blacklist++;
	}
	wlan_objmgr_pdev_release_ref(pdev, WLAN_MLME_CM_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_roam_scan_ch_list_event_handler(struct cm_roam_scan_ch_resp *data)
{
	return cm_handle_scan_ch_list_data(data);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_roam_stats_get_trigger_detail_str - Return roam trigger string from the
 * enum roam_trigger_reason
 * @ptr: Pointer to the roam trigger info
 * @buf: Destination buffer to write the reason string
 * @is_full_scan: Is roam scan partial scan or all channels scan
 * @vdev_id: vdev id
 *
 * Return: None
 */
static void
cm_roam_stats_get_trigger_detail_str(struct wmi_roam_trigger_info *ptr,
				     char *buf, bool is_full_scan,
				     uint8_t vdev_id)
{
	uint16_t buf_cons, buf_left = MAX_ROAM_DEBUG_BUF_SIZE;
	char *temp = buf;

	buf_cons = qdf_snprint(temp, buf_left, "Reason: \"%s\" ",
			       mlme_get_roam_trigger_str(ptr->trigger_reason));
	temp += buf_cons;
	buf_left -= buf_cons;

	if (ptr->trigger_sub_reason) {
		buf_cons = qdf_snprint(temp, buf_left, "Sub-Reason: %s",
			      mlme_get_sub_reason_str(ptr->trigger_sub_reason));
		temp += buf_cons;
		buf_left -= buf_cons;
	}

	switch (ptr->trigger_reason) {
	case ROAM_TRIGGER_REASON_PER:
	case ROAM_TRIGGER_REASON_BMISS:
	case ROAM_TRIGGER_REASON_HIGH_RSSI:
	case ROAM_TRIGGER_REASON_MAWC:
	case ROAM_TRIGGER_REASON_DENSE:
	case ROAM_TRIGGER_REASON_BACKGROUND:
	case ROAM_TRIGGER_REASON_IDLE:
	case ROAM_TRIGGER_REASON_FORCED:
	case ROAM_TRIGGER_REASON_UNIT_TEST:
		break;
	case ROAM_TRIGGER_REASON_BTM:
		cm_roam_btm_req_event(&ptr->btm_trig_data, vdev_id);
		buf_cons = qdf_snprint(temp, buf_left,
				       "Req_mode: %d Disassoc_timer: %d",
				       ptr->btm_trig_data.btm_request_mode,
				       ptr->btm_trig_data.disassoc_timer);
		temp += buf_cons;
		buf_left -= buf_cons;

		buf_cons = qdf_snprint(temp, buf_left,
				"validity_interval: %d candidate_list_cnt: %d resp_status: %d, bss_termination_timeout: %d, mbo_assoc_retry_timeout: %d",
				ptr->btm_trig_data.validity_interval,
				ptr->btm_trig_data.candidate_list_count,
				ptr->btm_trig_data.btm_resp_status,
				ptr->btm_trig_data.btm_bss_termination_timeout,
				ptr->btm_trig_data.btm_mbo_assoc_retry_timeout);
		buf_left -= buf_cons;
		temp += buf_cons;
		break;
	case ROAM_TRIGGER_REASON_BSS_LOAD:
		buf_cons = qdf_snprint(temp, buf_left, "CU: %d %% ",
				       ptr->cu_trig_data.cu_load);
		temp += buf_cons;
		buf_left -= buf_cons;
		break;
	case ROAM_TRIGGER_REASON_DEAUTH:
		buf_cons = qdf_snprint(temp, buf_left, "Type: %d Reason: %d ",
				       ptr->deauth_trig_data.type,
				       ptr->deauth_trig_data.reason);
		temp += buf_cons;
		buf_left -= buf_cons;
		break;
	case ROAM_TRIGGER_REASON_LOW_RSSI:
	case ROAM_TRIGGER_REASON_PERIODIC:
		/*
		 * Use ptr->current_rssi get the RSSI of current AP after
		 * roam scan is triggered. This avoids discrepency with the
		 * next rssi threshold value printed in roam scan details.
		 * ptr->rssi_trig_data.threshold gives the rssi threshold
		 * for the Low Rssi/Periodic scan trigger.
		 */
		buf_cons = qdf_snprint(temp, buf_left,
				       "Cur_Rssi threshold:%d Current AP RSSI: %d",
				       ptr->rssi_trig_data.threshold,
				       ptr->current_rssi);
		temp += buf_cons;
		buf_left -= buf_cons;
		break;
	case ROAM_TRIGGER_REASON_WTC_BTM:
		cm_roam_btm_resp_event(ptr, NULL, vdev_id, true);

		if (ptr->wtc_btm_trig_data.wtc_candi_rssi_ext_present) {
			buf_cons = qdf_snprint(temp, buf_left,
				   "Roaming Mode: %d, Trigger Reason: %d, Sub code:%d, wtc mode:%d, wtc scan mode:%d, wtc rssi th:%d, wtc candi rssi th_2g:%d, wtc_candi_rssi_th_5g:%d, wtc_candi_rssi_th_6g:%d",
				   ptr->wtc_btm_trig_data.roaming_mode,
				   ptr->wtc_btm_trig_data.vsie_trigger_reason,
				   ptr->wtc_btm_trig_data.sub_code,
				   ptr->wtc_btm_trig_data.wtc_mode,
				   ptr->wtc_btm_trig_data.wtc_scan_mode,
				   ptr->wtc_btm_trig_data.wtc_rssi_th,
				   ptr->wtc_btm_trig_data.wtc_candi_rssi_th,
				   ptr->wtc_btm_trig_data.wtc_candi_rssi_th_5g,
				   ptr->wtc_btm_trig_data.wtc_candi_rssi_th_6g);
		} else {
			buf_cons = qdf_snprint(temp, buf_left,
				   "Roaming Mode: %d, Trigger Reason: %d, Sub code:%d, wtc mode:%d, wtc scan mode:%d, wtc rssi th:%d, wtc candi rssi th:%d",
				   ptr->wtc_btm_trig_data.roaming_mode,
				   ptr->wtc_btm_trig_data.vsie_trigger_reason,
				   ptr->wtc_btm_trig_data.sub_code,
				   ptr->wtc_btm_trig_data.wtc_mode,
				   ptr->wtc_btm_trig_data.wtc_scan_mode,
				   ptr->wtc_btm_trig_data.wtc_rssi_th,
				   ptr->wtc_btm_trig_data.wtc_candi_rssi_th);
		}

		temp += buf_cons;
		buf_left -= buf_cons;
		break;
	default:
		break;
	}
}

/**
 * cm_roam_stats_print_trigger_info  - Roam trigger related details
 * @data:    Pointer to the roam trigger data
 * @vdev_id: Vdev ID
 *
 * Prints the vdev, roam trigger reason, time of the day at which roaming
 * was triggered.
 *
 * Return: None
 */
static void
cm_roam_stats_print_trigger_info(struct wmi_roam_trigger_info *data,
				 uint8_t vdev_id, bool is_full_scan)
{
	char *buf;
	char time[TIME_STRING_LEN];

	/* Update roam trigger info to userspace */
	cm_roam_trigger_info_event(data, vdev_id, is_full_scan);

	buf = qdf_mem_malloc(MAX_ROAM_DEBUG_BUF_SIZE);
	if (!buf)
		return;

	cm_roam_stats_get_trigger_detail_str(data, buf, is_full_scan, vdev_id);
	mlme_get_converted_timestamp(data->timestamp, time);
	mlme_nofl_info("%s [ROAM_TRIGGER]: VDEV[%d] %s", time, vdev_id, buf);

	qdf_mem_free(buf);
}

/**
 * cm_roam_stats_print_btm_rsp_info - BTM RSP related details
 * @data:    Pointer to the btm rsp data
 * @vdev_id: vdev id
 *
 * Prints the vdev, btm status, target_bssid and vsie reason
 *
 * Return: None
 */
static void
cm_roam_stats_print_btm_rsp_info(struct wmi_roam_trigger_info *trigger_info,
				 struct roam_btm_response_data *data,
				 uint8_t vdev_id, bool is_wtc)
{
	char time[TIME_STRING_LEN];

	mlme_get_converted_timestamp(data->timestamp, time);
	mlme_nofl_info("%s [BTM RSP]:VDEV[%d], Status:%d, VSIE reason:%d, BSSID: "
		       QDF_MAC_ADDR_FMT, time, vdev_id, data->btm_status,
		       data->vsie_reason,
		       QDF_MAC_ADDR_REF(data->target_bssid.bytes));
	cm_roam_btm_resp_event(trigger_info, data, vdev_id, is_wtc);
}

/**
 * cm_roam_stats_print_roam_initial_info - Roaming related initial details
 * @data:    Pointer to the btm rsp data
 * @vdev_id: vdev id
 *
 * Prints the vdev, roam_full_scan_count, channel and rssi
 * utilization threhold and timer
 *
 * Return: None
 */
static void
cm_roam_stats_print_roam_initial_info(struct roam_initial_data *data,
				      uint8_t vdev_id)
{
	mlme_nofl_info("[ROAM INIT INFO]: VDEV[%d], roam_full_scan_count: %d, rssi_th: %d, cu_th: %d, fw_cancel_timer_bitmap: %d",
		       vdev_id, data->roam_full_scan_count, data->rssi_th,
		       data->cu_th, data->fw_cancel_timer_bitmap);
}

/**
 * cm_roam_stats_print_roam_msg_info - Roaming related message details
 * @data:    Pointer to the btm rsp data
 * @vdev_id: vdev id
 *
 * Prints the vdev, msg_id, msg_param1, msg_param2 and timer
 *
 * Return: None
 */
static void cm_roam_stats_print_roam_msg_info(struct roam_msg_info *data,
					      uint8_t vdev_id)
{
	char time[TIME_STRING_LEN];
	static const char msg_id1_str[] = "Roam RSSI TH Reset";

	if (data->msg_id == WMI_ROAM_MSG_RSSI_RECOVERED) {
		mlme_get_converted_timestamp(data->timestamp, time);
		mlme_nofl_info("%s [ROAM MSG INFO]: VDEV[%d] %s, Current rssi: %d dbm, next_rssi_threshold: %d dbm",
			       time, vdev_id, msg_id1_str, data->msg_param1,
			       data->msg_param2);
	}
}

/**
 * cm_stats_log_roam_scan_candidates  - Print roam scan candidate AP info
 * @ap:           Pointer to the candidate AP list
 * @num_entries:  Number of candidate APs
 *
 * Print the RSSI, CU load, Cu score, RSSI score, total score, BSSID
 * and time stamp at which the candidate was found details.
 *
 * Return: None
 */
static void
cm_stats_log_roam_scan_candidates(struct wmi_roam_candidate_info *ap,
				  uint8_t num_entries)
{
	uint16_t i;
	char time[TIME_STRING_LEN], time2[TIME_STRING_LEN];


	mlme_nofl_info("%62s%62s", LINE_STR, LINE_STR);
	mlme_nofl_info("%13s %16s %8s %4s %4s %5s/%3s %3s/%3s %7s %7s %6s %12s %20s",
		       "AP BSSID", "TSTAMP", "CH", "TY", "ETP", "RSSI",
		       "SCR", "CU%", "SCR", "TOT_SCR", "BL_RSN", "BL_SRC",
		       "BL_TSTAMP", "BL_TIMEOUT(ms)");
	mlme_nofl_info("%62s%62s", LINE_STR, LINE_STR);

	if (num_entries > MAX_ROAM_CANDIDATE_AP)
		num_entries = MAX_ROAM_CANDIDATE_AP;

	for (i = 0; i < num_entries; i++) {
		mlme_get_converted_timestamp(ap->timestamp, time);
		mlme_get_converted_timestamp(ap->bl_timestamp, time2);
		mlme_nofl_info(QDF_MAC_ADDR_FMT " %17s %4d %-4s %4d %3d/%-4d %2d/%-4d %5d %7d %7d %17s %9d",
			       QDF_MAC_ADDR_REF(ap->bssid.bytes), time,
			  ap->freq,
			  ((ap->type == 0) ? "C_AP" :
			  ((ap->type == 2) ? "R_AP" : "P_AP")),
			  ap->etp, ap->rssi, ap->rssi_score, ap->cu_load,
			  ap->cu_score, ap->total_score, ap->bl_reason,
			  ap->bl_source, time2, ap->bl_original_timeout);
		/* Update roam candidates info to userspace */
		cm_roam_candidate_info_event(ap, i);
		ap++;
	}
}

/**
 * cm_roam_stats_print_scan_info  - Print the roam scan details and candidate AP
 * details
 * @scan:      Pointer to the received tlv after sanitization
 * @vdev_id:   Vdev ID
 * @trigger:   Roam scan trigger reason
 * @timestamp: Host timestamp in millisecs
 *
 * Prinst the roam scan details with time of the day when the scan was
 * triggered and roam candidate AP with score details
 *
 * Return: None
 */
static void
cm_roam_stats_print_scan_info(struct wmi_roam_scan_data *scan, uint8_t vdev_id,
			      uint32_t trigger, uint32_t timestamp)
{
	uint16_t num_ch = scan->num_chan;
	uint16_t buf_cons = 0, buf_left = ROAM_CHANNEL_BUF_SIZE;
	uint8_t i;
	char *buf, *buf1, *tmp;
	char time[TIME_STRING_LEN];

	/* Update roam scan info to userspace */
	cm_roam_scan_info_event(scan, vdev_id);

	buf = qdf_mem_malloc(ROAM_CHANNEL_BUF_SIZE);
	if (!buf)
		return;

	tmp = buf;
	/* For partial scans, print the channel info */
	if (!scan->type) {
		buf_cons = qdf_snprint(tmp, buf_left, "{");
		buf_left -= buf_cons;
		tmp += buf_cons;

		for (i = 0; i < num_ch; i++) {
			buf_cons = qdf_snprint(tmp, buf_left, "%d ",
					       scan->chan_freq[i]);
			buf_left -= buf_cons;
			tmp += buf_cons;
		}
		buf_cons = qdf_snprint(tmp, buf_left, "}");
		buf_left -= buf_cons;
		tmp += buf_cons;
	}

	buf1 = qdf_mem_malloc(ROAM_FAILURE_BUF_SIZE);
	if (!buf1) {
		qdf_mem_free(buf);
		return;
	}

	if (trigger == ROAM_TRIGGER_REASON_LOW_RSSI ||
	    trigger == ROAM_TRIGGER_REASON_PERIODIC)
		qdf_snprint(buf1, ROAM_FAILURE_BUF_SIZE,
			    "next_rssi_threshold: %d dBm",
			    scan->next_rssi_threshold);

	mlme_get_converted_timestamp(timestamp, time);
	mlme_nofl_info("%s [ROAM_SCAN]: VDEV[%d] Scan_type: %s %s %s",
		       time, vdev_id, mlme_get_roam_scan_type_str(scan->type),
		       buf1, buf);
	cm_stats_log_roam_scan_candidates(scan->ap, scan->num_ap);

	qdf_mem_free(buf);
	qdf_mem_free(buf1);
}

/**
 * cm_roam_stats_print_roam_result()  - Print roam result related info
 * @res:     Roam result strucure pointer
 * @vdev_id: Vdev id
 *
 * Print roam result and failure reason if roaming failed.
 *
 * Return: None
 */
static void
cm_roam_stats_print_roam_result(struct wmi_roam_result *res,
				struct wmi_roam_scan_data *scan_data,
				uint8_t vdev_id)
{
	char *buf;
	char time[TIME_STRING_LEN];

	/* Update roam result info to userspace */
	cm_roam_result_info_event(res, scan_data, vdev_id);

	buf = qdf_mem_malloc(ROAM_FAILURE_BUF_SIZE);
	if (!buf)
		return;

	if (res->status == 1)
		qdf_snprint(buf, ROAM_FAILURE_BUF_SIZE, "Reason: %s",
			    mlme_get_roam_fail_reason_str(res->fail_reason));

	mlme_get_converted_timestamp(res->timestamp, time);
	mlme_nofl_info("%s [ROAM_RESULT]: VDEV[%d] %s %s",
		       time, vdev_id, mlme_get_roam_status_str(res->status),
		       buf);

	qdf_mem_free(buf);
}

/**
 * cm_roam_stats_print_11kv_info  - Print neighbor report/BTM related data
 * @neigh_rpt: Pointer to the extracted TLV structure
 * @vdev_id:   Vdev ID
 *
 * Print BTM/neighbor report info that is sent by firmware after
 * connection/roaming to an AP.
 *
 * Return: none
 */
static void
cm_roam_stats_print_11kv_info(struct wmi_neighbor_report_data *neigh_rpt,
			      uint8_t vdev_id)
{
	char time[TIME_STRING_LEN], time1[TIME_STRING_LEN];
	char *buf, *tmp;
	uint8_t type = neigh_rpt->req_type, i;
	uint16_t buf_left = ROAM_CHANNEL_BUF_SIZE, buf_cons;
	uint8_t num_ch = neigh_rpt->num_freq;

	if (!type)
		return;

	buf = qdf_mem_malloc(ROAM_CHANNEL_BUF_SIZE);
	if (!buf)
		return;

	tmp = buf;
	if (num_ch) {
		buf_cons = qdf_snprint(tmp, buf_left, "{ ");
		buf_left -= buf_cons;
		tmp += buf_cons;

		for (i = 0; i < num_ch; i++) {
			buf_cons = qdf_snprint(tmp, buf_left, "%d ",
					       neigh_rpt->freq[i]);
			buf_left -= buf_cons;
			tmp += buf_cons;
		}

		buf_cons = qdf_snprint(tmp, buf_left, "}");
		buf_left -= buf_cons;
		tmp += buf_cons;
	}

	mlme_get_converted_timestamp(neigh_rpt->req_time, time);
	mlme_nofl_info("%s [%s] VDEV[%d]", time,
		       (type == 1) ? "BTM_QUERY" : "NEIGH_RPT_REQ", vdev_id);

	if (neigh_rpt->resp_time) {
		mlme_get_converted_timestamp(neigh_rpt->resp_time, time1);
		mlme_nofl_info("%s [%s] VDEV[%d] %s", time1,
			       (type == 1) ? "BTM_REQ" : "NEIGH_RPT_RSP",
			       vdev_id,
			       (num_ch > 0) ? buf : "NO Ch update");
	} else {
		mlme_nofl_info("%s No response received from AP",
			       (type == 1) ? "BTM" : "NEIGH_RPT");
	}
	qdf_mem_free(buf);
}

QDF_STATUS
cm_roam_stats_event_handler(struct wlan_objmgr_psoc *psoc,
			    struct roam_stats_event *stats_info)
{
	uint8_t i, rem_tlv = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!stats_info)
		return QDF_STATUS_E_FAILURE;
	for (i = 0; i < stats_info->num_tlv; i++) {
		if (stats_info->trigger[i].present) {
			bool is_full_scan =
				stats_info->scan[i].present &&
				stats_info->scan[i].type;

			cm_roam_stats_print_trigger_info(
						&stats_info->trigger[i],
						stats_info->vdev_id,
						is_full_scan);
		       status = wlan_cm_update_roam_states(psoc,
					stats_info->vdev_id,
					stats_info->trigger[i].trigger_reason,
					ROAM_TRIGGER_REASON);
			if (QDF_IS_STATUS_ERROR(status))
				goto err;
		}

		if (stats_info->scan[i].present &&
		    stats_info->trigger[i].present)
			cm_roam_stats_print_scan_info(&stats_info->scan[i],
					  stats_info->vdev_id,
					  stats_info->trigger[i].trigger_reason,
					  stats_info->trigger[i].timestamp);

		if (stats_info->result[i].present) {
			cm_roam_stats_print_roam_result(&stats_info->result[i],
							&stats_info->scan[i],
							stats_info->vdev_id);
			status = wlan_cm_update_roam_states(psoc,
					      stats_info->vdev_id,
					      stats_info->result[i].fail_reason,
					      ROAM_FAIL_REASON);
			if (QDF_IS_STATUS_ERROR(status))
				goto err;
		}

		/*
		 * Print BTM resp TLV info (wmi_roam_btm_response_info) only
		 * when trigger reason is BTM or WTC_BTM. As for other roam
		 * triggers this TLV contains zeros, so host should not print.
		 */
		if (stats_info->btm_rsp[i].present &&
		    stats_info->trigger[i].present &&
		    (stats_info->trigger[i].trigger_reason ==
		     ROAM_TRIGGER_REASON_BTM ||
		     stats_info->trigger[i].trigger_reason ==
		     ROAM_TRIGGER_REASON_WTC_BTM)) {
			cm_roam_stats_print_btm_rsp_info(
						&stats_info->trigger[i],
						&stats_info->btm_rsp[i],
						stats_info->vdev_id, false);
		}

		if (stats_info->roam_init_info[i].present)
			cm_roam_stats_print_roam_initial_info(
						 &stats_info->roam_init_info[i],
						 stats_info->vdev_id);

		if (stats_info->roam_msg_info &&
		    i < stats_info->num_roam_msg_info &&
		    stats_info->roam_msg_info[i].present) {
			rem_tlv++;
			cm_roam_stats_print_roam_msg_info(
						  &stats_info->roam_msg_info[i],
						  stats_info->vdev_id);
			if (stats_info->data_11kv[i].present)
				cm_roam_stats_print_11kv_info(
						      &stats_info->data_11kv[i],
						      stats_info->vdev_id);
		}
	}

	if (!stats_info->num_tlv) {
		if (stats_info->data_11kv[0].present)
			cm_roam_stats_print_11kv_info(&stats_info->data_11kv[0],
						      stats_info->vdev_id);

		if (stats_info->trigger[0].present)
			cm_roam_stats_print_trigger_info(
						&stats_info->trigger[0],
						stats_info->vdev_id, 1);

		if (stats_info->scan[0].present &&
		    stats_info->trigger[0].present)
			cm_roam_stats_print_scan_info(&stats_info->scan[0],
					  stats_info->vdev_id,
					  stats_info->trigger[0].trigger_reason,
					  stats_info->trigger[0].timestamp);

		if (stats_info->btm_rsp[0].present)
			cm_roam_stats_print_btm_rsp_info(
					&stats_info->trigger[i],
					&stats_info->btm_rsp[0],
					stats_info->vdev_id, 0);
	}
	if (stats_info->roam_msg_info && stats_info->num_roam_msg_info &&
	    stats_info->num_roam_msg_info - rem_tlv) {
		for (i = 0; i < (stats_info->num_roam_msg_info-rem_tlv); i++) {
			if (stats_info->roam_msg_info[rem_tlv + i].present)
				cm_roam_stats_print_roam_msg_info(
					&stats_info->roam_msg_info[rem_tlv + i],
					stats_info->vdev_id);
		}
	}

err:
	if (stats_info->roam_msg_info)
		qdf_mem_free(stats_info->roam_msg_info);
	qdf_mem_free(stats_info);
	return status;
}
#endif

#ifdef WLAN_FEATURE_FIPS
QDF_STATUS cm_roam_pmkid_req_ind(struct wlan_objmgr_psoc *psoc,
				 uint8_t vdev_id,
				 struct roam_pmkid_req_event *src_lst)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct qdf_mac_addr *dst_list;
	uint32_t num_entries, i;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	num_entries = src_lst->num_entries;
	mlme_debug("Num entries %d", num_entries);
	for (i = 0; i < num_entries; i++) {
		dst_list = &src_lst->ap_bssid[i];
		status = mlme_cm_osif_pmksa_candidate_notify(vdev, dst_list,
							     1, false);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err("Number %d Notify failed for " QDF_MAC_ADDR_FMT,
				 i, QDF_MAC_ADDR_REF(dst_list->bytes));
			goto rel_ref;
		}
	}

rel_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}
#endif /* WLAN_FEATURE_FIPS */
