/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

/* Support for "Fast roaming" (i.e., ESE, LFR, or 802.11r.) */
#define BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN 15

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

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	status = cm_rso_set_roam_trigger(pdev, vdev_id, trigger);

	cm_roam_release_lock();

	return status;
}

QDF_STATUS wlan_cm_disable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			       enum wlan_cm_rso_control_requestor requestor,
			       uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	if (reason == REASON_DRIVER_DISABLED && requestor)
		mlme_set_operations_bitmap(psoc, vdev_id, requestor, false);

	mlme_debug("ROAM_CONFIG: vdev[%d] Disable roaming - requestor:%s",
		   vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STOPPED,
				      REASON_DRIVER_DISABLED);
	cm_roam_release_lock();

	return status;
}

QDF_STATUS wlan_cm_enable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			      enum wlan_cm_rso_control_requestor requestor,
			      uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	if (reason == REASON_DRIVER_ENABLED && requestor)
		mlme_set_operations_bitmap(psoc, vdev_id, requestor, true);

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	mlme_debug("ROAM_CONFIG: vdev[%d] Enable roaming - requestor:%s",
		   vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_ENABLED,
				      REASON_DRIVER_ENABLED);
	cm_roam_release_lock();

	return status;
}

#ifdef FEATURE_CM_ENABLE
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
#endif

QDF_STATUS wlan_cm_abort_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id) ||
	    wlan_cm_host_roam_in_progress(psoc, vdev_id)) {
		cm_roam_release_lock();
		return QDF_STATUS_E_BUSY;
	}

	/* RSO stop cmd will be issued with lock held to avoid
	 * any racing conditions with wma/csr layer
	 */
	wlan_cm_disable_rso(pdev, vdev_id, REASON_DRIVER_DISABLED,
			    RSO_INVALID_REQUESTOR);

	cm_roam_release_lock();
	return QDF_STATUS_SUCCESS;
}

bool wlan_cm_roaming_in_progress(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return false;

	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id) ||
	    MLME_IS_ROAMING_IN_PROG(psoc, vdev_id) ||
	    mlme_is_roam_invoke_in_progress(psoc, vdev_id) ||
	    wlan_cm_host_roam_in_progress(psoc, vdev_id)) {
		cm_roam_release_lock();
		return true;
	}

	cm_roam_release_lock();

	return false;
}

QDF_STATUS wlan_cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				 uint8_t reason)
{
	return cm_roam_stop_req(psoc, vdev_id, reason);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
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
	QDF_STATUS status;

	if (!wlan_mlme_get_dual_sta_roaming_enabled(psoc))
		return;

	channel_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	if (!channel_list)
		return;

	/*
	 * Get Reg domain valid channels and update to the scan filter
	 * if already 1st sta is in connected state. Don't allow channels
	 * on which the 1st STA is connected.
	 */
	status = policy_mgr_get_valid_chans(psoc, channel_list,
					    &num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Error in getting valid channels");
		qdf_mem_free(channel_list);
		return;
	}

	filter->num_of_channels = 0;
	for (i = 0; i < num_channels; i++) {
		is_ch_allowed =
			wlan_cm_dual_sta_is_freq_allowed(psoc, channel_list[i],
							 QDF_STA_MODE);
		if (!is_ch_allowed)
			continue;

		filter->chan_freq_list[filter->num_of_channels] =
					channel_list[i];
		filter->num_of_channels++;
	}
	qdf_mem_free(channel_list);
}

void
wlan_cm_roam_disable_vendor_btm(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
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

	/* Set default value of reason code */
	mlme_priv->cm_roam.vendor_btm_param.user_roam_reason =
						DISABLE_VENDOR_BTM_CONFIG;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
}

void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params
									*param)
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

	qdf_mem_copy(&mlme_priv->cm_roam.vendor_btm_param, param,
		     sizeof(struct wlan_cm_roam_vendor_btm_params));
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
}

void
wlan_cm_roam_get_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params *param)
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

	qdf_mem_copy(param, &mlme_priv->cm_roam.vendor_btm_param,
		     sizeof(struct wlan_cm_roam_vendor_btm_params));

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
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

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return status;
	if (!MLME_IS_ROAM_STATE_RSO_ENABLED(psoc, vdev_id)) {
		mlme_debug("Update cfg received while ROAM RSO not started");
		cm_roam_release_lock();
		return QDF_STATUS_E_INVAL;
	}

	status = cm_roam_send_rso_cmd(psoc, vdev_id,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG, reason);
	cm_roam_release_lock();

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
				j += snprintf(channel_list + j, buflen - j,
					      "%d ", chan_info->freq_list[i]);
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
		if (num_chan == ROAM_MAX_CHANNELS)
			break;
		freq_list[num_chan++] = chan_info->freq_list[i];
	}

	return num_chan;
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

static void cm_flush_roam_channel_list(struct rso_chan_info *channel_info)
{
	/* Free up the memory first (if required) */
	if (channel_info->freq_list) {
		qdf_mem_free(channel_info->freq_list);
		channel_info->freq_list = NULL;
		channel_info->num_chan = 0;
	}
}

static QDF_STATUS
cm_update_roam_scan_channel_list(uint8_t vdev_id,
				 struct rso_chan_info *chan_info,
				 qdf_freq_t *freq_list, uint8_t num_chan,
				 bool update_preferred_chan)
{
	uint16_t pref_chan_cnt = 0;

	if (chan_info->num_chan) {
		mlme_debug("Current channels:");
		cm_dump_freq_list(chan_info);
	}

	if (update_preferred_chan) {
		pref_chan_cnt = cm_append_pref_chan_list(chan_info, freq_list,
							 num_chan);
		num_chan = pref_chan_cnt;
	}
	cm_flush_roam_channel_list(chan_info);
	cm_create_bg_scan_roam_channel_list(chan_info, freq_list, num_chan);

	mlme_debug("New channels:");
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
		if (dst_cfg->specific_chan_info.num_chan) {
			mlme_err("Specific channel list is already configured");
			break;
		}
		status = cm_update_roam_scan_channel_list(vdev_id,
					&dst_cfg->pref_chan_info,
					src_config->chan_info.freq_list,
					src_config->chan_info.num_chan,
					true);
		if (QDF_IS_STATUS_ERROR(status))
			break;
		if (mlme_obj->cfg.lfr.roam_scan_offload_enabled)
			cm_roam_update_cfg(psoc, vdev_id,
					   REASON_CHANNEL_LIST_CHANGED);
		break;
	case ROAM_SPECIFIC_CHAN:
		cm_update_roam_scan_channel_list(vdev_id,
					&dst_cfg->specific_chan_info,
					src_config->chan_info.freq_list,
					src_config->chan_info.num_chan,
					false);
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

QDF_STATUS wlan_cm_rso_config_init(struct wlan_objmgr_vdev *vdev,
				   struct rso_config *rso_cfg)
{
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

	return QDF_STATUS_SUCCESS;
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
	if (rso_cfg->roam_scan_freq_lst.freq_list)
		qdf_mem_free(rso_cfg->roam_scan_freq_lst.freq_list);
	rso_cfg->roam_scan_freq_lst.freq_list = NULL;
	rso_cfg->roam_scan_freq_lst.num_chan = 0;

	cm_flush_roam_channel_list(&cfg_params->specific_chan_info);
	cm_flush_roam_channel_list(&cfg_params->pref_chan_info);
}

#ifdef FEATURE_CM_ENABLE
struct rso_config *wlan_cm_get_rso_config_fl(struct wlan_objmgr_vdev *vdev,
					     const char *func, uint32_t line)

{
	struct cm_ext_obj *cm_ext_obj;

	cm_ext_obj = cm_get_ext_hdl_fl(vdev, func, line);
	if (!cm_ext_obj)
		return NULL;

	return &cm_ext_obj->rso_cfg;
}
#else
struct rso_config *wlan_cm_get_rso_config_fl(struct wlan_objmgr_vdev *vdev,
					     const char *func, uint32_t line)
{
	struct mlme_legacy_priv *mlme_priv;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_nofl_err("%s:%u: vdev %d legacy private object is NULL",
			      func, line, wlan_vdev_get_id(vdev));
		return NULL;
	}

	return &mlme_priv->rso_cfg;
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
#ifndef FEATURE_CM_ENABLE
QDF_STATUS wlan_cm_update_mlme_fils_connection_info(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_fils_connection_info *src_fils_info,
		uint8_t vdev_id)
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

	if (!src_fils_info) {
		mlme_debug("FILS: vdev:%d Clear fils info", vdev_id);
		qdf_mem_free(mlme_priv->connect_info.fils_con_info);
		mlme_priv->connect_info.fils_con_info = NULL;
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_SUCCESS;
	}

	if (mlme_priv->connect_info.fils_con_info)
		qdf_mem_free(mlme_priv->connect_info.fils_con_info);

	mlme_priv->connect_info.fils_con_info =
		qdf_mem_malloc(sizeof(struct wlan_fils_connection_info));
	if (!mlme_priv->connect_info.fils_con_info) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_NOMEM;
	}

	mlme_debug("FILS: vdev:%d update fils info", vdev_id);
	qdf_mem_copy(mlme_priv->connect_info.fils_con_info, src_fils_info,
		     sizeof(struct wlan_fils_connection_info));

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return QDF_STATUS_SUCCESS;
}
#else
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
#endif

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
#endif
