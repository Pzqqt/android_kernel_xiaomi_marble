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
#include "wlan_dfs_tgt_api.h"
#include "../../core/src/dfs.h"
#include "../../core/src/dfs_zero_cac.h"

QDF_STATUS tgt_dfs_set_current_channel(struct wlan_objmgr_pdev *pdev,
		uint16_t ic_freq,
		uint32_t ic_flags,
		uint16_t ic_flagext,
		uint8_t ic_ieee,
		uint8_t ic_vhtop_ch_freq_seg1,
		uint8_t ic_vhtop_ch_freq_seg2)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_set_current_channel(dfs, ic_freq, ic_flags, ic_flagext, ic_ieee,
			ic_vhtop_ch_freq_seg1, ic_vhtop_ch_freq_seg2);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_set_current_channel);

QDF_STATUS tgt_dfs_reset(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_reset(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_reset);

QDF_STATUS tgt_dfs_radar_enable(struct wlan_objmgr_pdev *pdev,
		int no_cac, uint32_t opmode)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_radar_enable(dfs, no_cac, opmode);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_radar_enable);

QDF_STATUS tgt_dfs_process_phyerr(struct wlan_objmgr_pdev *pdev,
		void *buf,
		uint16_t datalen,
		uint8_t r_rssi,
		uint8_t r_ext_rssi,
		uint32_t r_rs_tstamp,
		uint64_t r_fulltsf)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_process_phyerr(dfs, buf, datalen, r_rssi, r_ext_rssi, r_rs_tstamp,
			r_fulltsf);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_process_phyerr);

QDF_STATUS tgt_dfs_is_precac_timer_running(struct wlan_objmgr_pdev *pdev,
		bool *is_precac_timer_running)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*is_precac_timer_running = dfs_is_precac_timer_running(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_is_precac_timer_running);

QDF_STATUS tgt_dfs_get_radars(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_get_radars(dfs);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_get_radars);

QDF_STATUS tgt_dfs_attach(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	return dfs_attach(dfs);
}
EXPORT_SYMBOL(tgt_dfs_attach);

QDF_STATUS tgt_dfs_destroy_object(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_destroy_object(dfs);
	dfs = NULL;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_destroy_object);

QDF_STATUS tgt_nif_dfs_reset(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	nif_dfs_reset(dfs);
	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_nif_dfs_reset);

QDF_STATUS tgt_sif_dfs_detach(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	sif_dfs_detach(dfs);
	return  QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_dfs_control(struct wlan_objmgr_pdev *pdev,
		u_int id,
		void *indata,
		uint32_t insize,
		void *outdata,
		uint32_t *outsize,
		int *error)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	*error = dfs_control(dfs, id, indata, insize, outdata, outsize);

	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_control);

QDF_STATUS tgt_dfs_find_vht80_chan_for_precac(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile)
{
	struct wlan_dfs *dfs;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (dfs == NULL)
		return  QDF_STATUS_E_FAILURE;

	dfs_find_vht80_chan_for_precac(dfs,
			chan_mode,
			ch_freq_seg1,
			cfreq1,
			cfreq2,
			phy_mode,
			dfs_set_cfreq2,
			set_agile);

	return  QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(tgt_dfs_find_vht80_chan_for_precac);
