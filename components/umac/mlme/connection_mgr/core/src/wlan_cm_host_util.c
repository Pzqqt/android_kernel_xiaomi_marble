/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_cm_host_util.c
 *
 * Implements Host roam (LFR2) reassoc specific legacy code for
 * connection manager
 */

#include "wlan_cm_vdev_api.h"
#include "wlan_scan_api.h"
#include "wlan_scan_utils_api.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_roam_debug.h"
#include "wni_api.h"
#include "wlan_logging_sock_svc.h"
#include "connection_mgr/core/src/wlan_cm_roam.h"

/*
 * cm_copy_ssids_from_rso_config_params() - copy SSID from rso_config_params
 * to scan filter
 * @rso_usr_cfg: rso user config
 * @filter: scan filter
 *
 * Return void
 */
static void
cm_copy_ssids_from_rso_config_params(struct rso_config_params *rso_usr_cfg,
				     struct scan_filter *filter)
{
	uint8_t i;
	uint8_t max_ssid;

	if (!rso_usr_cfg->num_ssid_allowed_list)
		return;
	max_ssid = QDF_MIN(WLAN_SCAN_FILTER_NUM_SSID, MAX_SSID_ALLOWED_LIST);

	filter->num_of_ssid = rso_usr_cfg->num_ssid_allowed_list;
	if (filter->num_of_ssid > max_ssid)
		filter->num_of_ssid = max_ssid;
	for  (i = 0; i < filter->num_of_ssid; i++)
		qdf_mem_copy(&filter->ssid_list[i],
			     &rso_usr_cfg->ssid_allowed_list[i],
			     sizeof(struct wlan_ssid));
}

QDF_STATUS cm_update_advance_roam_scan_filter(
		struct wlan_objmgr_vdev *vdev, struct scan_filter *filter)
{
	uint8_t num_ch = 0;
	struct wlan_objmgr_psoc *psoc;
	struct rso_config *rso_cfg;
	struct rso_chan_info *chan_lst;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct rso_config_params *rso_usr_cfg;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_debug("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	rso_usr_cfg = &mlme_obj->cfg.lfr.rso_user_config;

	mlme_debug("No of Allowed SSID List:%d",
		   rso_usr_cfg->num_ssid_allowed_list);

	if (rso_usr_cfg->num_ssid_allowed_list) {
		cm_copy_ssids_from_rso_config_params(rso_usr_cfg, filter);
	} else {
		filter->num_of_ssid = 1;
		wlan_vdev_mlme_get_ssid(vdev, filter->ssid_list[0].ssid,
					&filter->ssid_list[0].length);

		mlme_debug("Filtering for SSID %.*s,length of SSID = %u",
			   filter->ssid_list[0].length,
			   filter->ssid_list[0].ssid,
			   filter->ssid_list[0].length);
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_FAILURE;

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
	filter->enable_adaptive_11r =
		wlan_mlme_adaptive_11r_enabled(psoc);

	if (rso_cfg->rsn_cap & WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED)
		filter->pmf_cap = WLAN_PMF_REQUIRED;
	else if (rso_cfg->rsn_cap & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED)
		filter->pmf_cap = WLAN_PMF_CAPABLE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_reassoc_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_reassoc_req *req)
{
	struct cm_vdev_join_req *join_req;
	struct scheduler_msg msg;
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct rso_config *rso_cfg;

	if (!vdev || !req)
		return QDF_STATUS_E_FAILURE;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "pdev not found",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		return QDF_STATUS_E_INVAL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "psoc not found",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		return QDF_STATUS_E_INVAL;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_NOSUPPORT;

	qdf_mem_zero(&msg, sizeof(msg));
	join_req = qdf_mem_malloc(sizeof(*join_req));
	if (!join_req)
		return QDF_STATUS_E_NOMEM;

	wlan_cm_set_disable_hi_rssi(pdev, req->vdev_id, true);
	mlme_debug(CM_PREFIX_FMT "Disabling HI_RSSI, AP freq=%d, rssi=%d",
		   CM_PREFIX_REF(req->vdev_id, req->cm_id),
		   req->bss->entry->channel.chan_freq,
		   req->bss->entry->rssi_raw);

	if (rso_cfg->assoc_ie.ptr) {
		join_req->assoc_ie.ptr = qdf_mem_malloc(rso_cfg->assoc_ie.len);
		if (!join_req->assoc_ie.ptr)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_copy(join_req->assoc_ie.ptr, rso_cfg->assoc_ie.ptr,
			     rso_cfg->assoc_ie.len);
		join_req->assoc_ie.len = rso_cfg->assoc_ie.len;
	}

	join_req->entry = util_scan_copy_cache_entry(req->bss->entry);
	if (!join_req->entry) {
		mlme_err(CM_PREFIX_FMT "Failed to copy scan entry",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
		return QDF_STATUS_E_NOMEM;
	}
	join_req->vdev_id = req->vdev_id;
	join_req->cm_id = req->cm_id;

	status = cm_csr_handle_join_req(vdev, NULL, join_req, true);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "fail to fill params from legacy",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
		return QDF_STATUS_E_FAILURE;
	}

	wlan_rec_conn_info(req->vdev_id, DEBUG_CONN_CONNECTING,
			   req->bss->entry->bssid.bytes,
			   req->bss->entry->neg_sec_info.key_mgmt,
			   req->bss->entry->channel.chan_freq);

	/* decrement count for self reassoc */
	if (req->self_reassoc)
		policy_mgr_decr_session_set_pcl(psoc,
						wlan_vdev_mlme_get_opmode(vdev),
						req->vdev_id);
	msg.bodyptr = join_req;
	msg.type = CM_REASSOC_REQ;
	msg.flush_callback = cm_flush_join_req;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "msg post fail",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		cm_free_join_req(join_req);
	}

	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		wlan_register_txrx_packetdump(OL_TXRX_PDEV_ID);

	return status;
}

QDF_STATUS cm_handle_roam_start(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_roam_req *req)
{
	if (!vdev || !req) {
		mlme_err("vdev or req is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (req->source == CM_ROAMING_HOST)
		cm_roam_state_change(wlan_vdev_get_pdev(vdev),
				     wlan_vdev_get_id(vdev),
				     WLAN_ROAM_RSO_STOPPED,
				     REASON_OS_REQUESTED_ROAMING_NOW);
	return QDF_STATUS_SUCCESS;
}

