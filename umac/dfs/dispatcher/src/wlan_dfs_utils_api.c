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
#include "../../core/src/dfs_random_chan_sel.h"
#ifdef QCA_DFS_USE_POLICY_MANAGER
#include "wlan_policy_mgr_api.h"
#endif
#ifdef QCA_DFS_NOL_PLATFORM_DRV_SUPPORT
#include <pld_common.h>
#endif

struct dfs_nol_info {
	uint16_t num_chans;
	struct dfsreq_nolelem dfs_nol[DFS_MAX_NOL_CHANNEL];
};

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

static void utils_dfs_get_max_phy_mode(struct wlan_objmgr_pdev *pdev,
		uint32_t *phy_mode)
{
	return;
}

static void utils_dfs_get_max_sup_width(struct wlan_objmgr_pdev *pdev,
		uint8_t *ch_width)
{
	return;
}

#ifndef QCA_DFS_USE_POLICY_MANAGER
static void utils_dfs_get_chan_list(struct wlan_objmgr_pdev *pdev,
	struct dfs_ieee80211_channel *chan_list, uint32_t *num_chan)
{
	return;
}
#else
static void utils_dfs_get_chan_list(struct wlan_objmgr_pdev *pdev,
	struct dfs_ieee80211_channel *chan_list, uint32_t *num_chan)
{
	uint8_t pcl_ch[NUM_CHANNELS];
	uint32_t len;
	int i;
	struct wlan_objmgr_psoc *psoc;
	uint32_t conn_count = 0;

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);
	if (!psoc) {
		*num_chan = 0;
		DFS_PRINTK("%s: null psoc\n", __func__);
		return;
	}

	len = QDF_ARRAY_SIZE(pcl_ch);
	conn_count = policy_mgr_mode_specific_connection_count(
			psoc, PM_SAP_MODE, NULL);
	if (0 == conn_count)
		policy_mgr_get_pcl(psoc, PM_SAP_MODE, pcl_ch, &len, NULL, 0);
	else
		policy_mgr_get_pcl_for_existing_conn(psoc, PM_SAP_MODE, pcl_ch,
						     &len, NULL, 0);

	if (*num_chan < len) {
		DFS_PRINTK("%s: Invalid len src=%d, dst=%d\n",
			   __func__, *num_chan, len);
		*num_chan = 0;
		return;
	}

	for (i = 0; i < len; i++) {
		chan_list[i].ic_ieee  = pcl_ch[i];
		chan_list[i].ic_freq  = wlan_reg_chan_to_freq(pdev, pcl_ch[i]);
	}
	*num_chan = i;
	DFS_PRINTK("%s: num channels %d\n", __func__, i);
}
#endif

QDF_STATUS dfs_get_random_channel(
	struct wlan_objmgr_pdev *pdev,
	uint16_t flags,
	struct ch_params *ch_params,
	uint32_t *hw_mode,
	int *target_chan)
{
	uint32_t dfs_reg;
	uint32_t num_chan = NUM_CHANNELS;
	struct wlan_dfs *dfs = NULL;
	struct wlan_objmgr_psoc *psoc;
	struct dfs_ieee80211_channel *chan_list = NULL;
	struct dfs_ieee80211_channel cur_chan;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	*target_chan = 0;
	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);
	if (!psoc) {
		DFS_PRINTK("%s: null psoc\n", __func__);
		goto random_chan_error;
	}

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		DFS_PRINTK("%s: null dfs\n", __func__);
		goto random_chan_error;
	}

	wlan_reg_get_dfs_region(psoc, &dfs_reg);
	chan_list = qdf_mem_malloc(num_chan * sizeof(*chan_list));
	if (!chan_list) {
		DFS_PRINTK("%s: mem alloc failed\n", __func__);
		goto random_chan_error;
	}

	utils_dfs_get_chan_list(pdev, chan_list, &num_chan);
	if (!num_chan) {
		DFS_PRINTK("%s: zero channels\n", __func__);
		goto random_chan_error;
	}

	cur_chan.ic_vhtop_ch_freq_seg1 = ch_params->center_freq_seg0;
	cur_chan.ic_vhtop_ch_freq_seg2 = ch_params->center_freq_seg1;

	if (!ch_params->ch_width)
		utils_dfs_get_max_sup_width(pdev,
				(uint8_t *)&ch_params->ch_width);

	*target_chan = dfs_prepare_random_channel(dfs, chan_list,
		num_chan, flags, (uint8_t *)&ch_params->ch_width,
		&cur_chan, (uint8_t)dfs_reg);

	ch_params->center_freq_seg0 = cur_chan.ic_vhtop_ch_freq_seg1;
	ch_params->center_freq_seg1 = cur_chan.ic_vhtop_ch_freq_seg2;
	if (*target_chan) {
		wlan_reg_set_channel_params(pdev,
			(uint8_t)*target_chan, 0, ch_params);
		utils_dfs_get_max_phy_mode(pdev, hw_mode);
		status = QDF_STATUS_SUCCESS;
	}

random_chan_error:
	qdf_mem_free(chan_list);

	return status;
}
EXPORT_SYMBOL(dfs_get_random_channel);

#ifndef QCA_DFS_NOL_PLATFORM_DRV_SUPPORT
void dfs_init_nol(struct wlan_objmgr_pdev *pdev)
{
}
#else
void dfs_init_nol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;
	struct wlan_objmgr_psoc *psoc;
	qdf_device_t qdf_dev;
	struct dfs_nol_info dfs_nolinfo;
	int len;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);
	if (!dfs || !psoc) {
		DFS_PRINTK("%s: dfs %p, psoc %p\n", __func__, dfs, psoc);
		return;
	}

	qdf_dev = psoc->soc_objmgr.qdf_dev;
	if (!qdf_dev->dev) {
		DFS_PRINTK("%s: null device\n", __func__);
		return;
	}

	qdf_mem_zero(&dfs_nolinfo, sizeof(dfs_nolinfo));
	len = pld_wlan_get_dfs_nol(qdf_dev->dev, (void *)&dfs_nolinfo,
			(uint16_t)sizeof(dfs_nolinfo));
	if (len > 0)
		dfs_set_nol(dfs, dfs_nolinfo.dfs_nol, dfs_nolinfo.num_chans);
	else
		DFS_PRINTK("%s: nol nol in pld\n", __func__);
}
#endif
EXPORT_SYMBOL(dfs_init_nol);

#ifndef QCA_DFS_NOL_PLATFORM_DRV_SUPPORT
void dfs_save_nol(struct wlan_objmgr_pdev *pdev)
{
}
#else
void dfs_save_nol(struct wlan_objmgr_pdev *pdev)
{
	struct dfs_nol_info dfs_nolinfo;
	struct wlan_dfs *dfs = NULL;
	struct wlan_objmgr_psoc *psoc;
	qdf_device_t qdf_dev;
	int num_chans = 0;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		DFS_PRINTK("%s: null dfs\n", __func__);
		return;
	}

	wlan_pdev_obj_lock(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	wlan_pdev_obj_unlock(pdev);
	if (!psoc) {
		DFS_PRINTK("%s: null psoc\n", __func__);
		return;
	}

	qdf_dev = psoc->soc_objmgr.qdf_dev;
	if (!qdf_dev->dev) {
		DFS_PRINTK("%s: null device\n", __func__);
		return;
	}

	qdf_mem_zero(&dfs_nolinfo, sizeof(dfs_nolinfo));
	dfs_get_nol(dfs, dfs_nolinfo.dfs_nol, &num_chans);
	if (num_chans > 0) {

		if (num_chans > DFS_MAX_NOL_CHANNEL)
			dfs_nolinfo.num_chans = DFS_MAX_NOL_CHANNEL;
		else
			dfs_nolinfo.num_chans = num_chans;

		pld_wlan_set_dfs_nol(qdf_dev->dev, (void *)&dfs_nolinfo,
				(uint16_t)sizeof(dfs_nolinfo));
	}
}
#endif
EXPORT_SYMBOL(dfs_save_nol);

void dfs_print_nol_channels(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs = NULL;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		DFS_PRINTK("%s: null dfs\n", __func__);
		return;
	}

	dfs_print_nol(dfs);
}
EXPORT_SYMBOL(dfs_print_nol_channels);

void dfs_clear_nol_channels(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs = NULL;

	dfs = global_dfs_to_mlme.pdev_get_comp_private_obj(pdev);
	if (!dfs) {
		DFS_PRINTK("%s: null dfs\n", __func__);
		return;
	}

	/* First print list */
	dfs_print_nol(dfs);

	/* clear local cache first */
	dfs_nol_timer_cleanup(dfs);

	/*
	 * update platform driver nol list with local cache which is zero,
	 * cleared in above step, so this will clear list in platform driver.
	 */
	dfs_save_nol(pdev);
}
EXPORT_SYMBOL(dfs_clear_nol_channels);

bool utils_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	return wlan_reg_is_dfs_ch(pdev, chan);
}
EXPORT_SYMBOL(utils_is_dfs_ch);

void utils_dfs_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
		uint8_t *ch_list,
		uint8_t num_ch,
		bool nol_ch)
{
	wlan_reg_update_nol_ch(pdev, ch_list, num_ch, nol_ch);
}
EXPORT_SYMBOL(utils_dfs_reg_update_nol_ch);

uint32_t utils_dfs_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			       uint32_t freq)
{
	return wlan_reg_freq_to_chan(pdev, freq);
}
EXPORT_SYMBOL(utils_dfs_freq_to_chan);

uint32_t utils_dfs_chan_to_freq(struct wlan_objmgr_pdev *pdev,
			       uint32_t chan)
{
	return wlan_reg_chan_to_freq(pdev, chan);
}
EXPORT_SYMBOL(utils_dfs_chan_to_freq);
