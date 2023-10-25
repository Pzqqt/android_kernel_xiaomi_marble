/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_tgt_if_tx_api.c
 *
 * Implementation for the Common Roaming interfaces.
 */

#include "wlan_objmgr_psoc_obj.h"
#include "wlan_psoc_mlme_api.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_reg_services_api.h"
#include "wlan_cm_tgt_if_tx_api.h"
#include "wlan_mlme_public_struct.h"
#include "wma.h"

static inline
struct wlan_cm_roam_tx_ops *wlan_cm_roam_get_tx_ops_from_vdev(
				struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlme_psoc_ext_obj *psoc_ext_priv;
	struct wlan_cm_roam_tx_ops *tx_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_legacy_err("psoc object is NULL");
		return NULL;
	}
	psoc_ext_priv = wlan_psoc_mlme_get_ext_hdl(psoc);
	if (!psoc_ext_priv) {
		mlme_legacy_err("psoc legacy private object is NULL");
		return NULL;
	}

	tx_ops = &psoc_ext_priv->rso_tx_ops;

	return tx_ops;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
wlan_cm_roam_send_set_vdev_pcl(struct wlan_objmgr_psoc *psoc,
			       struct set_pcl_req *pcl_req)
{
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct fw_scan_channels *freq_list;
	struct wlan_objmgr_vdev *vdev;
	struct wmi_pcl_chan_weights *weights;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t band_capability;
	uint16_t i;
	bool is_channel_allowed;

	/*
	 * If vdev_id is WLAN_UMAC_VDEV_ID_MAX, then PDEV pcl command
	 * needs to be sent
	 */
	if (!pcl_req || pcl_req->vdev_id == WLAN_UMAC_VDEV_ID_MAX)
		return QDF_STATUS_E_FAILURE;

	status = ucfg_mlme_get_band_capability(psoc, &band_capability);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	mlme_debug("RSO_CFG: band_capability:%d band_mask:%d for vdev[%d]",
		   band_capability, pcl_req->band_mask, pcl_req->vdev_id);

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, pcl_req->vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("pdev object is NULL");
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_vdev_set_pcl_cmd) {
		mlme_err("send_vdev_set_pcl_cmd is NULL");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	freq_list = qdf_mem_malloc(sizeof(*freq_list));
	if (!freq_list) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	mlme_get_fw_scan_channels(psoc, freq_list->freq,
				  &freq_list->num_channels);

	weights = &pcl_req->chan_weights;
	for (i = 0; i < freq_list->num_channels; i++)
		weights->saved_chan_list[i] = freq_list->freq[i];

	weights->saved_num_chan = freq_list->num_channels;
	if (pcl_req->clear_vdev_pcl)
		weights->saved_num_chan = 0;

	status = policy_mgr_get_valid_chan_weights(
			psoc, (struct policy_mgr_pcl_chan_weights *)weights,
			PM_STA_MODE, vdev);

	qdf_mem_free(freq_list);

	for (i = 0; i < weights->saved_num_chan; i++) {
		weights->weighed_valid_list[i] =
			wma_map_pcl_weights(weights->weighed_valid_list[i]);

		/* Dont allow roaming on 2G when 5G_ONLY configured */
		if ((band_capability == BIT(REG_BAND_5G) ||
		     band_capability == BIT(REG_BAND_6G) ||
		     pcl_req->band_mask == BIT(REG_BAND_5G) ||
		     pcl_req->band_mask == BIT(REG_BAND_6G)) &&
		     WLAN_REG_IS_24GHZ_CH_FREQ(weights->saved_chan_list[i]))
			weights->weighed_valid_list[i] =
				WEIGHT_OF_DISALLOWED_CHANNELS;

		if ((band_capability == BIT(REG_BAND_2G) ||
		     pcl_req->band_mask == BIT(REG_BAND_2G)) &&
		    !WLAN_REG_IS_24GHZ_CH_FREQ(weights->saved_chan_list[i]))
			weights->weighed_valid_list[i] =
				WEIGHT_OF_DISALLOWED_CHANNELS;

		is_channel_allowed =
			policy_mgr_is_sta_chan_valid_for_connect_and_roam(
					pdev, weights->saved_chan_list[i]);
		if (!is_channel_allowed)
			weights->weighed_valid_list[i] =
				WEIGHT_OF_DISALLOWED_CHANNELS;
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Error in creating weighed pcl");
		goto end;
	}

	mlme_debug("RSO_CFG: Dump Vdev PCL weights for vdev[%d]",
		   pcl_req->vdev_id);
	policy_mgr_dump_channel_list(weights->saved_num_chan,
				     weights->saved_chan_list,
				     weights->weighed_valid_list);

	status = roam_tx_ops->send_vdev_set_pcl_cmd(vdev, pcl_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("set vdev PCL failed");

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_rt_stats_config(struct wlan_objmgr_psoc *psoc,
						 struct roam_disable_cfg *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, req->vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_rt_stats_config) {
		mlme_err("vdev %d send_roam_rt_stats_config is NULL",
			 req->vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_rt_stats_config(vdev,
							req->vdev_id, req->cfg);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("vdev %d fail to send roam rt stats config",
			   req->vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_ho_delay_config(struct wlan_objmgr_psoc *psoc,
						 uint8_t vdev_id,
						 uint16_t roam_ho_delay)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_ho_delay_config) {
		mlme_err("vdev %d send_roam_ho_delay_config is NULL", vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_ho_delay_config(vdev, vdev_id,
							roam_ho_delay);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("vdev %d fail to send roam HO delay config",
			   vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS
wlan_cm_tgt_exclude_rm_partial_scan_freq(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id,
					 uint8_t exclude_rm_partial_scan_freq)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_exclude_rm_partial_scan_freq) {
		mlme_err("vdev %d send_exclude_rm_partial_scan_freq is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_exclude_rm_partial_scan_freq(
					vdev, exclude_rm_partial_scan_freq);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("vdev %d fail to exclude roam partial scan freq",
			   vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_full_scan_6ghz_on_disc(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					uint8_t roam_full_scan_6ghz_on_disc)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_full_scan_6ghz_on_disc) {
		mlme_err("vdev %d send_roam_full_scan_6ghz_on_disc is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_full_scan_6ghz_on_disc(
					vdev, roam_full_scan_6ghz_on_disc);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("vdev %d fail to send inclusion of 6 GHz channels",
			   vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS
wlan_cm_tgt_send_roam_scan_offload_rssi_params(
		struct wlan_objmgr_vdev *vdev,
		struct wlan_roam_offload_scan_rssi_params *roam_rssi_params)
{
	QDF_STATUS status;
	uint8_t vdev_id;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;

	vdev_id = wlan_vdev_get_id(vdev);

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_scan_offload_rssi_params) {
		mlme_err("vdev %d send_roam_scan_offload_rssi_params is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_scan_offload_rssi_params(
						vdev, roam_rssi_params);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("vdev %d fail to send roam scan offload RSSI params",
			   vdev_id);

	return status;
}
#endif

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
QDF_STATUS wlan_cm_tgt_send_roam_offload_init(struct wlan_objmgr_psoc *psoc,
					      uint8_t vdev_id, bool is_init)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_roam_offload_init_params init_msg = {0};
	uint32_t disable_4way_hs_offload;
	bool bmiss_skip_full_scan;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_offload_init_req) {
		mlme_err("CM_RSO: vdev%d send_roam_offload_init_req is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	init_msg.vdev_id = vdev_id;
	if (is_init) {
		init_msg.roam_offload_flag = WLAN_ROAM_FW_OFFLOAD_ENABLE |
				 WLAN_ROAM_BMISS_FINAL_SCAN_ENABLE;

		wlan_mlme_get_4way_hs_offload(psoc, &disable_4way_hs_offload);
		if (!disable_4way_hs_offload)
			init_msg.roam_offload_flag |=
				WLAN_ROAM_SKIP_SAE_ROAM_4WAY_HANDSHAKE;
		if (disable_4way_hs_offload &
		    CFG_DISABLE_4WAY_HS_OFFLOAD_DEFAULT)
			init_msg.roam_offload_flag |=
				(WLAN_ROAM_SKIP_EAPOL_4WAY_HANDSHAKE |
				 WLAN_ROAM_SKIP_SAE_ROAM_4WAY_HANDSHAKE);

		wlan_mlme_get_bmiss_skip_full_scan_value(psoc,
							 &bmiss_skip_full_scan);
		if (bmiss_skip_full_scan)
			init_msg.roam_offload_flag |=
				WLAN_ROAM_BMISS_FINAL_SCAN_TYPE;
	}
	mlme_debug("vdev_id:%d, is_init:%d, flag:%d",  vdev_id, is_init,
		   init_msg.roam_offload_flag);

	status = roam_tx_ops->send_roam_offload_init_req(vdev, &init_msg);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev%d fail to send rso init", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_start_req(struct wlan_objmgr_psoc *psoc,
					   uint8_t vdev_id,
					   struct wlan_roam_start_config *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_start_req) {
		mlme_err("CM_RSO: vdev %d send_roam_start_req is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_start_req(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send roam start", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_stop_req(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id,
					 struct wlan_roam_stop_config *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_stop_offload) {
		mlme_err("CM_RSO: vdev %d send_roam_stop_offload is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_stop_offload(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send roam stop", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_update_req(struct wlan_objmgr_psoc *psoc,
					    uint8_t vdev_id,
					    struct wlan_roam_update_config *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_update_config) {
		mlme_err("CM_RSO: vdev %d send_roam_update_config is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_update_config(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send roam update", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_abort_req(struct wlan_objmgr_psoc *psoc,
					   uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_abort) {
		mlme_err("CM_RSO: vdev %d send_roam_abort is NULL", vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_abort(vdev, vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send roam abort", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_per_config(struct wlan_objmgr_psoc *psoc,
					   uint8_t vdev_id,
					   struct wlan_per_roam_config_req *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_per_config) {
		mlme_err("CM_RSO: vdev %d send_roam_per_config is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_per_config(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send per config", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS wlan_cm_tgt_send_roam_triggers(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id,
					  struct wlan_roam_triggers *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_triggers) {
		mlme_err("CM_RSO: vdev %d send_roam_triggers is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_triggers(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("CM_RSO: vdev %d fail to send roamtrigger", vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}
#endif

QDF_STATUS wlan_cm_tgt_send_roam_disable_config(struct wlan_objmgr_psoc *psoc,
						uint8_t vdev_id,
						struct roam_disable_cfg *req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);
	if (!roam_tx_ops || !roam_tx_ops->send_roam_disable_config) {
		mlme_err("CM_RSO: vdev %d send_roam_disable_config is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_disable_config(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("CM_RSO: vdev %d fail to send roam disable config",
			   vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS
wlan_cm_tgt_send_roam_invoke_req(struct wlan_objmgr_psoc *psoc,
				 struct roam_invoke_req *roam_invoke_req)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    roam_invoke_req->vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);

	if (!roam_tx_ops || !roam_tx_ops->send_roam_invoke_cmd) {
		mlme_err("CM_RSO: vdev %d send_roam_invoke_cmd is NULL",
			 roam_invoke_req->vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_invoke_cmd(vdev, roam_invoke_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("CM_RSO: vdev %d fail to send roam invoke cmd",
			   roam_invoke_req->vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}

QDF_STATUS
wlan_cm_tgt_send_roam_sync_complete_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_cm_roam_tx_ops *roam_tx_ops;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	roam_tx_ops = wlan_cm_roam_get_tx_ops_from_vdev(vdev);

	if (!roam_tx_ops || !roam_tx_ops->send_roam_sync_complete_cmd) {
		mlme_err("CM_RSO: vdev %d send_roam_sync_complete_cmd is NULL",
			 vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	status = roam_tx_ops->send_roam_sync_complete_cmd(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("CM_RSO: vdev %d fail to send roam sync complete cmd",
			   vdev_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return status;
}
