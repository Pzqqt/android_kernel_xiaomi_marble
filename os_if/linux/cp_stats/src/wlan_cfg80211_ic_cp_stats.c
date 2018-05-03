/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_cfg80211_ic_cp_stats.c
 *
 * This file provide definitions to os_if cp_stats APIs
 */
#include <wlan_cfg80211_ic_cp_stats.h>
#include <wlan_cp_stats_ic_ucfg_api.h>
#include <wlan_cfg80211.h>
#include <qdf_util.h>

int wlan_cfg80211_get_peer_cp_stats(struct wlan_objmgr_peer *peer_obj,
				    struct peer_ic_cp_stats *peer_cp_stats)
{
	QDF_STATUS status;

	if (!peer_obj) {
		cfg80211_err("Invalid input, peer obj NULL");
		return -EINVAL;
	}

	if (!peer_cp_stats) {
		cfg80211_err("Invalid input, peer cp obj is NULL");
		return -EINVAL;
	}

	status = wlan_ucfg_get_peer_cp_stats(peer_obj, peer_cp_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_peer_cp_stats status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}

int wlan_cfg80211_get_vdev_cp_stats(struct wlan_objmgr_vdev *vdev_obj,
				    struct vdev_ic_cp_stats *vdev_cp_stats)
{
	QDF_STATUS status;

	if (!vdev_obj) {
		cfg80211_err("Invalid input, vdev obj is NULL");
		return -EINVAL;
	}

	if (!vdev_cp_stats) {
		cfg80211_err("Invalid input, vdev cp obj is NULL");
		return -EINVAL;
	}

	status = wlan_ucfg_get_vdev_cp_stats(vdev_obj, vdev_cp_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_vdev_cp_stats status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}

int wlan_cfg80211_get_pdev_cp_stats(struct wlan_objmgr_pdev *pdev_obj,
				    struct pdev_ic_cp_stats *pdev_cp_stats)
{
	QDF_STATUS status;

	if (!pdev_obj) {
		cfg80211_err("Invalid input, pdev obj is NULL");
		return -EINVAL;
	}

	if (!pdev_cp_stats) {
		cfg80211_err("Invalid input, pdev cp obj is NULL");
		return -EINVAL;
	}

	status = wlan_ucfg_get_pdev_cp_stats(pdev_obj, pdev_cp_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_pdev_cp_stats status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}

qdf_export_symbol(wlan_cfg80211_get_pdev_cp_stats);

#ifdef WLAN_ATF_ENABLE
int
wlan_cfg80211_get_atf_peer_cp_stats(struct wlan_objmgr_peer *peer_obj,
				    struct atf_peer_cp_stats *atf_cp_stats)
{
	QDF_STATUS status;

	if (!peer_obj) {
		cfg80211_err("Invalid input, peer obj is NULL");
		return -EINVAL;
	}

	if (!atf_cp_stats) {
		cfg80211_err("Invalid input, ATF peer cp obj is NULL!");
		return -EINVAL;
	}

	status = wlan_ucfg_get_atf_peer_cp_stats(peer_obj, atf_cp_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_atf_peer_cp_stats status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}

int wlan_cfg80211_get_atf_peer_cp_stats_from_mac(
		struct wlan_objmgr_vdev *vdev_obj,
		uint8_t *mac,
		struct atf_peer_cp_stats *atf_cp_stats)
{
	QDF_STATUS status;

	if (!vdev_obj) {
		cfg80211_err("Invalid input, vdev obj is NULL");
		return -EINVAL;
	}

	if (!mac) {
		cfg80211_err("Invalid input, peer mac is NULL");
		return -EINVAL;
	}

	if (!atf_cp_stats) {
		cfg80211_err("Invalid input, ATF peer cp stats obj is NULL");
		return -EINVAL;
	}

	status = wlan_ucfg_get_atf_peer_cp_stats_from_mac(vdev_obj, mac,
							  atf_cp_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_cp_stats_from_mac status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}
#endif

int
wlan_cfg80211_get_dcs_pdev_cp_stats(struct wlan_objmgr_pdev *pdev_obj,
				    struct pdev_dcs_chan_stats *dcs_chan_stats)
{
	QDF_STATUS status;

	if (!pdev_obj) {
		cfg80211_err("Invalid input, pdev obj is NULL");
		return -EINVAL;
	}

	if (!dcs_chan_stats) {
		cfg80211_err("Invalid input, dcs chan stats is NULL");
		return -EINVAL;
	}

	status = wlan_ucfg_get_dcs_chan_stats(pdev_obj, dcs_chan_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_cfg80211_get_dcs_pdev_cp_stats status: %d",
			     status);
	}

	return qdf_status_to_os_return(status);
}
