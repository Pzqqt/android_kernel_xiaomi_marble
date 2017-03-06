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
 * DOC: Functions to call mlme functions from DFS component.
 */

#include "wlan_dfs_mlme_api.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "../../core/src/dfs.h"
#include "scheduler_api.h"
#ifdef QCA_MCL_DFS_SUPPORT
#include "wni_api.h"
#endif

void dfs_mlme_channel_mark_radar(struct wlan_objmgr_pdev *pdev,
		uint16_t freq,
		uint8_t vhtop_ch_freq_seg2,
		uint32_t flags)
{
	if (global_dfs_to_mlme.dfs_channel_mark_radar != NULL)
		global_dfs_to_mlme.dfs_channel_mark_radar(pdev,
				freq,
				vhtop_ch_freq_seg2,
				flags);
}

void dfs_mlme_start_rcsa(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.dfs_start_rcsa != NULL)
		global_dfs_to_mlme.dfs_start_rcsa(pdev);
}

#ifndef QCA_MCL_DFS_SUPPORT
void dfs_mlme_mark_dfs(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee,
		uint16_t freq,
		uint8_t vhtop_ch_freq_seg2,
		uint32_t flags)
{
	if (global_dfs_to_mlme.mlme_mark_dfs != NULL)
		global_dfs_to_mlme.mlme_mark_dfs(pdev,
				ieee,
				freq,
				vhtop_ch_freq_seg2,
				flags);
}
#else
static void dfs_send_radar_ind(struct wlan_objmgr_pdev *pdev,
		void *object,
		void *arg)
{
	struct scheduler_msg sme_msg = {0};
	uint8_t vdev_id = wlan_vdev_get_id((struct wlan_objmgr_vdev *)object);

	sme_msg.type = eWNI_SME_DFS_RADAR_FOUND;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = vdev_id;
	scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	DFS_PRINTK("%s: eWNI_SME_DFS_RADAR_FOUND pdev%d posted\n",
		   __func__, vdev_id);
}

void dfs_mlme_mark_dfs(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee,
		uint16_t freq,
		uint8_t vhtop_ch_freq_seg2,
		uint32_t flags)
{
	if (!pdev) {
		DFS_PRINTK("%s: null pdev\n", __func__);
		return;
	}

	wlan_objmgr_pdev_iterate_obj_list(pdev,
				WLAN_VDEV_OP,
				dfs_send_radar_ind,
				NULL, 0, WLAN_DFS_ID);
}
#endif

void dfs_mlme_start_csa(struct wlan_objmgr_pdev *pdev,
		uint8_t ieeeChan)
{
	if (global_dfs_to_mlme.mlme_start_csa != NULL)
		global_dfs_to_mlme.mlme_start_csa(pdev, ieeeChan);
}

#ifndef QCA_MCL_DFS_SUPPORT
void dfs_mlme_proc_cac(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	if (global_dfs_to_mlme.mlme_proc_cac != NULL)
		global_dfs_to_mlme.mlme_proc_cac(pdev);
}
#else
void dfs_mlme_proc_cac(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	struct scheduler_msg sme_msg = {0};

	sme_msg.type = eWNI_SME_DFS_CAC_COMPLETE;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = vdev_id;
	scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	DFS_PRINTK("%s: eWNI_SME_DFS_CAC_COMPLETE vdev%d posted\n",
		   __func__, vdev_id);
}
#endif

void dfs_mlme_deliver_event_up_afrer_cac(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_deliver_event_up_afrer_cac != NULL)
		global_dfs_to_mlme.mlme_deliver_event_up_afrer_cac(
				pdev);
}

void dfs_mlme_get_ic_nchans(struct wlan_objmgr_pdev *pdev,
		int *nchans)
{
	if (global_dfs_to_mlme.mlme_get_ic_nchans != NULL)
		global_dfs_to_mlme.mlme_get_ic_nchans(pdev,
				nchans);
}

void dfs_mlme_get_ic_no_weather_radar_chan(struct wlan_objmgr_pdev *pdev,
		uint8_t *no_wradar)
{
	if (global_dfs_to_mlme.mlme_get_ic_no_weather_radar_chan != NULL)
		global_dfs_to_mlme.mlme_get_ic_no_weather_radar_chan(
				pdev,
				no_wradar);
}

int dfs_mlme_find_alternate_mode_channel(struct wlan_objmgr_pdev *pdev,
		uint32_t alt_chan_mode,
		int chan_count)
{
	int ret_val = 0;

	if (global_dfs_to_mlme.mlme_find_alternate_mode_channel != NULL)
		global_dfs_to_mlme.mlme_find_alternate_mode_channel(
				pdev,
				alt_chan_mode, chan_count, &ret_val);

	return ret_val;
}

void dfs_mlme_find_any_valid_channel(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode,
		int *ret_val)
{
	if (global_dfs_to_mlme.mlme_find_any_valid_channel != NULL)
		global_dfs_to_mlme.mlme_find_any_valid_channel(
				pdev,
				chan_mode,
				ret_val);
}

void dfs_mlme_get_extchan(struct wlan_objmgr_pdev *pdev,
		uint16_t *ic_freq,
		uint32_t *ic_flags,
		uint16_t *ic_flagext,
		uint8_t *ic_ieee,
		uint8_t *ic_vhtop_ch_freq_seg1,
		uint8_t *ic_vhtop_ch_freq_seg2)
{
	if (global_dfs_to_mlme.mlme_get_extchan != NULL)
		global_dfs_to_mlme.mlme_get_extchan(pdev,
				ic_freq,
				ic_flags,
				ic_flagext,
				ic_ieee,
				ic_vhtop_ch_freq_seg1,
				ic_vhtop_ch_freq_seg2);
}

void dfs_mlme_set_no_chans_available(struct wlan_objmgr_pdev *pdev,
		int val)
{
	if (global_dfs_to_mlme.mlme_set_no_chans_available != NULL)
		global_dfs_to_mlme.mlme_set_no_chans_available(
				pdev,
				val);
}

int dfs_mlme_ieee2mhz(struct wlan_objmgr_pdev *pdev, int ieee, int flag)
{
	int freq = 0;

	if (global_dfs_to_mlme.mlme_ieee2mhz != NULL)
		global_dfs_to_mlme.mlme_ieee2mhz(pdev,
				ieee,
				flag,
				&freq);

	return freq;
}

void dfs_mlme_find_dot11_channel(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee,
		uint8_t des_cfreq2,
		int mode,
		uint16_t *ic_freq,
		uint32_t *ic_flags,
		uint16_t *ic_flagext,
		uint8_t *ic_ieee,
		uint8_t *ic_vhtop_ch_freq_seg1,
		uint8_t *ic_vhtop_ch_freq_seg2)
{
	if (global_dfs_to_mlme.mlme_find_dot11_channel != NULL)
		global_dfs_to_mlme.mlme_find_dot11_channel(pdev,
				ieee,
				des_cfreq2,
				mode,
				ic_freq,
				ic_flags,
				ic_flagext,
				ic_ieee,
				ic_vhtop_ch_freq_seg1,
				ic_vhtop_ch_freq_seg2);
}

void dfs_mlme_get_ic_channels(struct wlan_objmgr_pdev *pdev,
		uint16_t *ic_freq,
		uint32_t *ic_flags,
		uint16_t *ic_flagext,
		uint8_t *ic_ieee,
		uint8_t *ic_vhtop_ch_freq_seg1,
		uint8_t *ic_vhtop_ch_freq_seg2,
		int index)
{
	if (global_dfs_to_mlme.mlme_get_ic_channels != NULL)
		global_dfs_to_mlme.mlme_get_ic_channels(pdev,
				ic_freq,
				ic_flags,
				ic_flagext,
				ic_ieee,
				ic_vhtop_ch_freq_seg1,
				ic_vhtop_ch_freq_seg2,
				index);
}

uint32_t dfs_mlme_ic_flags_ext(struct wlan_objmgr_pdev *pdev)
{
	uint32_t flag_ext = 0;

	if (global_dfs_to_mlme.mlme_ic_flags_ext != NULL)
		global_dfs_to_mlme.mlme_ic_flags_ext(pdev,
				&flag_ext);

	return flag_ext;
}

void dfs_mlme_channel_change_by_precac(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_channel_change_by_precac != NULL)
		global_dfs_to_mlme.mlme_channel_change_by_precac(
				pdev);
}

void dfs_mlme_nol_timeout_notification(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_nol_timeout_notification != NULL)
		global_dfs_to_mlme.mlme_nol_timeout_notification(
				pdev);
}

void dfs_mlme_clist_update(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		int nentries)
{
	if (global_dfs_to_mlme.mlme_clist_update != NULL)
		global_dfs_to_mlme.mlme_clist_update(pdev,
				nollist,
				nentries);
}

int dfs_mlme_get_cac_timeout(struct wlan_objmgr_pdev *pdev,
		uint16_t ic_freq,
		uint8_t ic_vhtop_ch_freq_seg2,
		uint32_t ic_flags)
{
	int cac_timeout = 0;

	if (global_dfs_to_mlme.mlme_get_cac_timeout != NULL)
		global_dfs_to_mlme.mlme_get_cac_timeout(pdev,
				ic_freq,
				ic_vhtop_ch_freq_seg2,
				ic_flags,
				&cac_timeout);

	return cac_timeout;
}
