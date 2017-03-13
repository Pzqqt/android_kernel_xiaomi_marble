/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
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
 * DOC: This file has the DFS dispatcher API implementation which is exposed
 * to outside of DFS component.
 */
#include "wlan_dfs_utils_api.h"
#include "../../core/src/dfs.h"
#include "../../core/src/dfs_zero_cac.h"

QDF_STATUS utils_nif_dfs_attach(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	nif_dfs_attach(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_nif_dfs_attach);

QDF_STATUS utils_nif_dfs_detach(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	nif_dfs_detach(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_nif_dfs_detach);

QDF_STATUS utils_dfs_cac_valid_reset(struct wlan_objmgr_pdev *pdev,
		uint8_t prevchan_ieee,
		uint32_t prevchan_flags)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_cac_valid_reset(dfs, prevchan_ieee, prevchan_flags);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_cac_valid_reset);

QDF_STATUS utils_dfs_reset_precaclists(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_reset_precaclists(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_reset_precaclists);

QDF_STATUS utils_dfs_cancel_precac_timer(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_cancel_precac_timer(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_cancel_precac_timer);

QDF_STATUS utils_dfs_is_precac_done(struct wlan_objmgr_pdev *pdev,
		bool *is_precac_done)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*is_precac_done = dfs_is_precac_done(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_is_precac_done);

QDF_STATUS utils_dfs_cancel_cac_timer(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_cancel_cac_timer(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_cancel_cac_timer);

QDF_STATUS utils_dfs_start_cac_timer(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_start_cac_timer(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_start_cac_timer);

QDF_STATUS utils_dfs_cac_stop(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_cac_stop(dfs);
	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_cac_stop);

QDF_STATUS utils_dfs_stacac_stop(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_stacac_stop(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_stacac_stop);

QDF_STATUS utils_dfs_random_channel(struct wlan_objmgr_pdev *pdev,
		uint8_t is_select_nondfs,
		uint8_t skip_curchan,
		int *target_channel)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*target_channel = dfs_random_channel(dfs,
			is_select_nondfs,
			skip_curchan);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_random_channel);

QDF_STATUS utils_dfs_get_usenol(struct wlan_objmgr_pdev *pdev, uint16_t *usenol)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*usenol = dfs_get_usenol(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_get_usenol);

QDF_STATUS utils_dfs_radar_disable(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_radar_disable(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_radar_disable);

QDF_STATUS utils_dfs_set_update_nol_flag(struct wlan_objmgr_pdev *pdev,
		bool val)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_set_update_nol_flag(dfs, val);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_set_update_nol_flag);

QDF_STATUS utils_dfs_get_update_nol_flag(struct wlan_objmgr_pdev *pdev,
		bool *nol_flag)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*nol_flag = dfs_get_update_nol_flag(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_get_update_nol_flag);

QDF_STATUS utils_dfs_get_rn_use_nol(struct wlan_objmgr_pdev *pdev,
		int *rn_use_nol)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*rn_use_nol = dfs_get_rn_use_nol(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_get_rn_use_nol);

QDF_STATUS utils_dfs_get_nol_timeout(struct wlan_objmgr_pdev *pdev,
		int *dfs_nol_timeout)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*dfs_nol_timeout = dfs_get_nol_timeout(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_get_nol_timeout);

QDF_STATUS utils_dfs_nol_addchan(struct wlan_objmgr_pdev *pdev,
		uint16_t freq,
		uint32_t dfs_nol_timeout)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_nol_addchan(dfs, freq, dfs_nol_timeout);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_nol_addchan);

QDF_STATUS utils_dfs_nol_update(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_nol_update(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_nol_update);

QDF_STATUS utils_dfs_second_segment_radar_disable(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_second_segment_radar_disable(dfs);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS utils_dfs_is_ignore_dfs(struct wlan_objmgr_pdev *pdev,
		bool *ignore_dfs)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*ignore_dfs = dfs->dfs_ignore_dfs;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_is_ignore_dfs);

QDF_STATUS utils_dfs_is_cac_valid(struct wlan_objmgr_pdev *pdev,
		bool *is_cac_valid)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*is_cac_valid = dfs->dfs_cac_valid;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_is_cac_valid);

QDF_STATUS utils_dfs_is_ignore_cac(struct wlan_objmgr_pdev *pdev,
		bool *ignore_cac)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*ignore_cac = dfs->dfs_ignore_cac;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_is_ignore_cac);

QDF_STATUS utils_dfs_set_cac_timer_running(struct wlan_objmgr_pdev *pdev,
		int val)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs->dfs_cac_timer_running = val;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_set_cac_timer_running);

QDF_STATUS utils_dfs_get_nol_chfreq_and_chwidth(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		uint32_t *nol_chfreq,
		uint32_t *nol_chwidth,
		int index)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_get_nol_chfreq_and_chwidth(nollist, nol_chfreq, nol_chwidth, index);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(utils_dfs_get_nol_chfreq_and_chwidth);

