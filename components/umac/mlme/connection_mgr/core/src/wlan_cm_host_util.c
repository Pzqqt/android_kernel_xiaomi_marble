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
 * Implements Host roam (LFR2) utils for connection manager
 */

#include "wlan_cm_roam_api.h"
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
