/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <wlan_reg_ucfg_api.h>
#ifdef QCA_MCL_DFS_SUPPORT
#include "wni_api.h"
#endif

void dfs_mlme_start_rcsa(struct wlan_objmgr_pdev *pdev,
		bool *wait_for_csa)
{
	if (global_dfs_to_mlme.dfs_start_rcsa)
		global_dfs_to_mlme.dfs_start_rcsa(pdev, wait_for_csa);
}

#ifndef QCA_MCL_DFS_SUPPORT
#ifdef CONFIG_CHAN_NUM_API
void dfs_mlme_mark_dfs(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee,
		uint16_t freq,
		uint8_t vhtop_ch_freq_seg2,
		uint64_t flags)
{
	if (global_dfs_to_mlme.mlme_mark_dfs)
		global_dfs_to_mlme.mlme_mark_dfs(pdev,
				ieee,
				freq,
				vhtop_ch_freq_seg2,
				flags);
}
#endif
#ifdef CONFIG_CHAN_FREQ_API
void dfs_mlme_mark_dfs_for_freq(struct wlan_objmgr_pdev *pdev,
				uint8_t ieee,
				uint16_t freq,
				uint16_t vhtop_ch_freq_seg2,
				uint64_t flags)
{
	if (global_dfs_to_mlme.mlme_mark_dfs_for_freq)
	global_dfs_to_mlme.mlme_mark_dfs_for_freq(pdev,
						  ieee,
						  freq,
						  vhtop_ch_freq_seg2,
						  flags);
}
#endif
#else /* Else of ndef MCL_DFS_SUPPORT */
#ifdef CONFIG_CHAN_NUM_API
static void dfs_send_radar_ind(struct wlan_objmgr_pdev *pdev,
		void *object,
		void *arg)
{
	struct scheduler_msg sme_msg = {0};
	uint8_t vdev_id = wlan_vdev_get_id((struct wlan_objmgr_vdev *)object);

	sme_msg.type = eWNI_SME_DFS_RADAR_FOUND;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = vdev_id;
	scheduler_post_message(QDF_MODULE_ID_DFS,
			       QDF_MODULE_ID_SME,
			       QDF_MODULE_ID_SME, &sme_msg);
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "eWNI_SME_DFS_RADAR_FOUND pdev%d posted",
		    vdev_id);
}
#endif

/* dfs_send_radar_ind_for_freq() - Send radar found indication.
 * @pdev: Pointer to wlan_objmgr_pdev.
 * @object: Pointer to wlan_objmgr_vdev.
 * @arg : void pointer to args.
 */
#ifdef CONFIG_CHAN_FREQ_API
static void dfs_send_radar_ind_for_freq(struct wlan_objmgr_pdev *pdev,
					void *object,
					void *arg)
{
	struct scheduler_msg sme_msg = {0};
	uint8_t vdev_id = wlan_vdev_get_id((struct wlan_objmgr_vdev *)object);

	sme_msg.type = eWNI_SME_DFS_RADAR_FOUND;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = vdev_id;
	scheduler_post_message(QDF_MODULE_ID_DFS,
			       QDF_MODULE_ID_SME,
			       QDF_MODULE_ID_SME, &sme_msg);
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "eWNI_SME_DFS_RADAR_FOUND pdev%d posted",
		 vdev_id);
}
#endif

#ifdef CONFIG_CHAN_NUM_API
void dfs_mlme_mark_dfs(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee,
		uint16_t freq,
		uint8_t vhtop_ch_freq_seg2,
		uint64_t flags)
{
	struct wlan_objmgr_vdev *vdev;

	if (!pdev) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return;
	}

	vdev = wlan_pdev_peek_active_first_vdev(pdev, WLAN_DFS_ID);

	if (vdev) {
		dfs_send_radar_ind(pdev, vdev, NULL);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);
	}
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
void dfs_mlme_mark_dfs_for_freq(struct wlan_objmgr_pdev *pdev,
				uint8_t ieee,
				uint16_t freq,
				uint16_t vhtop_ch_freq_seg2,
				uint64_t flags)
{
	struct wlan_objmgr_vdev *vdev;

	if (!pdev) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return;
	}

	vdev = wlan_pdev_peek_active_first_vdev(pdev, WLAN_DFS_ID);

	if (vdev) {
		dfs_send_radar_ind_for_freq(pdev, vdev, NULL);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);
	}
}
#endif
#endif

#ifndef QCA_MCL_DFS_SUPPORT
#ifdef CONFIG_CHAN_NUM_API
void dfs_mlme_start_csa(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee_chan, uint16_t freq,
		uint8_t cfreq2, uint64_t flags)
{
	if (global_dfs_to_mlme.mlme_start_csa)
		global_dfs_to_mlme.mlme_start_csa(pdev, ieee_chan, freq, cfreq2,
				flags);
}
#endif
#ifdef CONFIG_CHAN_FREQ_API
void dfs_mlme_start_csa_for_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t ieee_chan, uint16_t freq,
				 uint16_t cfreq2, uint64_t flags)
{
	if (global_dfs_to_mlme.mlme_start_csa_for_freq)
		global_dfs_to_mlme.mlme_start_csa_for_freq(pdev, ieee_chan,
							   freq, cfreq2, flags);
}
#endif
#else
#ifdef CONFIG_CHAN_NUM_API
void dfs_mlme_start_csa(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee_chan, uint16_t freq,
			uint8_t cfreq2, uint64_t flags)
{
	struct wlan_objmgr_vdev *vdev;

	if (!pdev) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return;
	}

	vdev = wlan_pdev_peek_active_first_vdev(pdev, WLAN_DFS_ID);

	if (vdev) {
		dfs_send_radar_ind(pdev, vdev, NULL);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);
	}
}
#endif
#ifdef CONFIG_CHAN_FREQ_API
void dfs_mlme_start_csa_for_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t ieee_chan, uint16_t freq,
				 uint16_t cfreq2, uint64_t flags)
{
	struct wlan_objmgr_vdev *vdev;

	if (!pdev) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return;
	}

	vdev = wlan_pdev_peek_active_first_vdev(pdev, WLAN_DFS_ID);

	if (vdev) {
		dfs_send_radar_ind(pdev, vdev, NULL);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);
	}
}
#endif
#endif

#ifndef QCA_MCL_DFS_SUPPORT
void dfs_mlme_proc_cac(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	if (global_dfs_to_mlme.mlme_proc_cac)
		global_dfs_to_mlme.mlme_proc_cac(pdev);
}
#else
void dfs_mlme_proc_cac(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id)
{
	struct scheduler_msg sme_msg = {0};

	sme_msg.type = eWNI_SME_DFS_CAC_COMPLETE;
	sme_msg.bodyptr = NULL;
	sme_msg.bodyval = vdev_id;
	scheduler_post_message(QDF_MODULE_ID_DFS,
			       QDF_MODULE_ID_SME,
			       QDF_MODULE_ID_SME, &sme_msg);
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "eWNI_SME_DFS_CAC_COMPLETE vdev%d posted",
		    vdev_id);
}
#endif

void dfs_mlme_deliver_event_up_after_cac(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_deliver_event_up_after_cac)
		global_dfs_to_mlme.mlme_deliver_event_up_after_cac(
				pdev);
}

void dfs_mlme_get_dfs_ch_nchans(struct wlan_objmgr_pdev *pdev,
		int *nchans)
{
	if (global_dfs_to_mlme.mlme_get_dfs_ch_nchans)
		global_dfs_to_mlme.mlme_get_dfs_ch_nchans(pdev,
				nchans);
}

#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS dfs_mlme_get_extchan(struct wlan_objmgr_pdev *pdev,
		uint16_t *dfs_ch_freq,
		uint64_t *dfs_ch_flags,
		uint16_t *dfs_ch_flagext,
		uint8_t *dfs_ch_ieee,
		uint8_t *dfs_ch_vhtop_ch_freq_seg1,
		uint8_t *dfs_ch_vhtop_ch_freq_seg2)
{
	if (global_dfs_to_mlme.mlme_get_extchan)
		return global_dfs_to_mlme.mlme_get_extchan(pdev,
				dfs_ch_freq,
				dfs_ch_flags,
				dfs_ch_flagext,
				dfs_ch_ieee,
				dfs_ch_vhtop_ch_freq_seg1,
				dfs_ch_vhtop_ch_freq_seg2);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS dfs_mlme_get_extchan_for_freq(struct wlan_objmgr_pdev *pdev,
					 uint16_t *dfs_chan_freq,
					 uint64_t *dfs_chan_flags,
					 uint16_t *dfs_chan_flagext,
					 uint8_t *dfs_chan_ieee,
					 uint8_t *dfs_chan_vhtop_ch_freq_seg1,
					 uint8_t *dfs_chan_vhtop_ch_freq_seg2,
					 uint16_t *dfs_chan_mhz_freq_seg1,
					 uint16_t *dfs_chan_mhz_freq_seg2)
{
	if (global_dfs_to_mlme.mlme_get_extchan_for_freq)
		return global_dfs_to_mlme.mlme_get_extchan_for_freq(pdev,
				dfs_chan_freq,
				dfs_chan_flags,
				dfs_chan_flagext,
				dfs_chan_ieee,
				dfs_chan_vhtop_ch_freq_seg1,
				dfs_chan_vhtop_ch_freq_seg2,
				dfs_chan_mhz_freq_seg1,
				dfs_chan_mhz_freq_seg2);

	return QDF_STATUS_E_FAILURE;
}
#endif

void dfs_mlme_set_no_chans_available(struct wlan_objmgr_pdev *pdev,
		int val)
{
	if (global_dfs_to_mlme.mlme_set_no_chans_available)
		global_dfs_to_mlme.mlme_set_no_chans_available(
				pdev,
				val);
}

int dfs_mlme_ieee2mhz(struct wlan_objmgr_pdev *pdev, int ieee, uint64_t flag)
{
	int freq = 0;

	if (global_dfs_to_mlme.mlme_ieee2mhz)
		global_dfs_to_mlme.mlme_ieee2mhz(pdev,
				ieee,
				flag,
				&freq);

	return freq;
}

#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS
dfs_mlme_find_dot11_channel(struct wlan_objmgr_pdev *pdev,
			    uint8_t ieee,
			    uint8_t des_cfreq2,
			    int mode,
			    uint16_t *dfs_ch_freq,
			    uint64_t *dfs_ch_flags,
			    uint16_t *dfs_ch_flagext,
			    uint8_t *dfs_ch_ieee,
			    uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			    uint8_t *dfs_ch_vhtop_ch_freq_seg2)
{
	if (global_dfs_to_mlme.mlme_find_dot11_channel)
		return global_dfs_to_mlme.mlme_find_dot11_channel(pdev,
								  ieee,
								  des_cfreq2,
								  mode,
								  dfs_ch_freq,
								  dfs_ch_flags,
								  dfs_ch_flagext,
								  dfs_ch_ieee,
								  dfs_ch_vhtop_ch_freq_seg1,
								  dfs_ch_vhtop_ch_freq_seg2);
	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
dfs_mlme_find_dot11_chan_for_freq(struct wlan_objmgr_pdev *pdev,
				  uint16_t freq,
				  uint16_t des_cfreq2,
				  int mode,
				  uint16_t *dfs_chan_freq,
				  uint64_t *dfs_chan_flag,
				  uint16_t *dfs_flagext,
				  uint8_t *dfs_chan_ieee,
				  uint8_t *dfs_cfreq1,
				  uint8_t *dfs_cfreq2,
				  uint16_t *cfreq1_mhz,
				  uint16_t *cfreq2_mhz)
{
	if (global_dfs_to_mlme.mlme_find_dot11_chan_for_freq)
	return global_dfs_to_mlme.mlme_find_dot11_chan_for_freq(pdev,
								freq,
								des_cfreq2,
								mode,
								dfs_chan_freq,
								dfs_chan_flag,
								dfs_flagext,
								dfs_chan_ieee,
								dfs_cfreq1,
								dfs_cfreq2,
								cfreq1_mhz,
								cfreq2_mhz);
	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef CONFIG_CHAN_NUM_API
void dfs_mlme_get_dfs_ch_channels(struct wlan_objmgr_pdev *pdev,
		uint16_t *dfs_ch_freq,
		uint64_t *dfs_ch_flags,
		uint16_t *dfs_ch_flagext,
		uint8_t *dfs_ch_ieee,
		uint8_t *dfs_ch_vhtop_ch_freq_seg1,
		uint8_t *dfs_ch_vhtop_ch_freq_seg2,
		int index)
{
	if (global_dfs_to_mlme.mlme_get_dfs_ch_channels)
		global_dfs_to_mlme.mlme_get_dfs_ch_channels(pdev,
				dfs_ch_freq,
				dfs_ch_flags,
				dfs_ch_flagext,
				dfs_ch_ieee,
				dfs_ch_vhtop_ch_freq_seg1,
				dfs_ch_vhtop_ch_freq_seg2,
				index);
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
void dfs_mlme_get_dfs_channels_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t *dfs_chan_freq,
					uint64_t *dfs_chan_flags,
					uint16_t *dfs_chan_flagext,
					uint8_t *dfs_chan_ieee,
					uint8_t *dfs_chan_vhtop_freq_seg1,
					uint8_t *dfs_chan_vhtop_freq_seg2,
					uint16_t *dfs_ch_mhz_freq_seg1,
					uint16_t *dfs_ch_mhz_freq_seg2,
					int index)
{
	if (global_dfs_to_mlme.mlme_get_dfs_channels_for_freq)
		global_dfs_to_mlme.mlme_get_dfs_channels_for_freq(pdev,
				dfs_chan_freq,
				dfs_chan_flags,
				dfs_chan_flagext,
				dfs_chan_ieee,
				dfs_chan_vhtop_freq_seg1,
				dfs_chan_vhtop_freq_seg2,
				dfs_ch_mhz_freq_seg1,
				dfs_ch_mhz_freq_seg2,
				index);
}
#endif

uint32_t dfs_mlme_dfs_ch_flags_ext(struct wlan_objmgr_pdev *pdev)
{
	uint16_t flag_ext = 0;

	if (global_dfs_to_mlme.mlme_dfs_ch_flags_ext)
		global_dfs_to_mlme.mlme_dfs_ch_flags_ext(pdev,
				&flag_ext);

	return flag_ext;
}

void dfs_mlme_channel_change_by_precac(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_channel_change_by_precac)
		global_dfs_to_mlme.mlme_channel_change_by_precac(
				pdev);
}

void dfs_mlme_nol_timeout_notification(struct wlan_objmgr_pdev *pdev)
{
	if (global_dfs_to_mlme.mlme_nol_timeout_notification)
		global_dfs_to_mlme.mlme_nol_timeout_notification(
				pdev);
}

void dfs_mlme_clist_update(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		int nentries)
{
	if (global_dfs_to_mlme.mlme_clist_update)
		global_dfs_to_mlme.mlme_clist_update(pdev,
				nollist,
				nentries);
}

#ifdef CONFIG_CHAN_NUM_API
int dfs_mlme_get_cac_timeout(struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint8_t dfs_ch_vhtop_ch_freq_seg2,
		uint64_t dfs_ch_flags)
{
	int cac_timeout = 0;

	if (global_dfs_to_mlme.mlme_get_cac_timeout)
		global_dfs_to_mlme.mlme_get_cac_timeout(pdev,
				dfs_ch_freq,
				dfs_ch_vhtop_ch_freq_seg2,
				dfs_ch_flags,
				&cac_timeout);

	return cac_timeout;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
int dfs_mlme_get_cac_timeout_for_freq(struct wlan_objmgr_pdev *pdev,
				      uint16_t dfs_chan_freq,
				      uint16_t dfs_cfreq2,
				      uint64_t dfs_ch_flags)
{
	int cac_timeout = 0;

	if (global_dfs_to_mlme.mlme_get_cac_timeout_for_freq)
		global_dfs_to_mlme.mlme_get_cac_timeout_for_freq(pdev,
								 dfs_chan_freq,
								 dfs_cfreq2,
								 dfs_ch_flags,
								 &cac_timeout);

	return cac_timeout;
}
#endif

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
int dfs_mlme_rebuild_chan_list_with_non_dfs_channels(
		struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_rebuild_chan_list_with_non_dfs_channels)
		return 1;

	return global_dfs_to_mlme.mlme_rebuild_chan_list_with_non_dfs_channels(
			pdev);
}

void dfs_mlme_restart_vaps_with_non_dfs_chan(struct wlan_objmgr_pdev *pdev,
					     int no_chans_avail)
{
	if (!global_dfs_to_mlme.mlme_restart_vaps_with_non_dfs_chan)
		return;

	global_dfs_to_mlme.mlme_restart_vaps_with_non_dfs_chan(pdev,
							       no_chans_avail);
}
#endif

#if defined(WLAN_SUPPORT_PRIMARY_ALLOWED_CHAN)
bool dfs_mlme_check_allowed_prim_chanlist(struct wlan_objmgr_pdev *pdev,
					  uint32_t chan_freq)
{
	if (!global_dfs_to_mlme.mlme_check_allowed_prim_chanlist)
		return true;

	return global_dfs_to_mlme.mlme_check_allowed_prim_chanlist(pdev,
								   chan_freq);
}

#endif

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
void dfs_mlme_handle_dfs_scan_violation(struct wlan_objmgr_pdev *pdev)
{
	bool dfs_enable = 0;

	/*Disable all DFS channels in master channel list and ic channel list */
	ucfg_reg_enable_dfs_channels(pdev, dfs_enable);

	/* send the updated channel list to FW */
	global_dfs_to_mlme.mlme_update_scan_channel_list(pdev);
}
#endif

bool dfs_mlme_is_opmode_sta(struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_is_opmode_sta)
		return false;

	return global_dfs_to_mlme.mlme_is_opmode_sta(pdev);
}

void dfs_mlme_acquire_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_acquire_radar_mode_switch_lock)
		return;

	global_dfs_to_mlme.mlme_acquire_radar_mode_switch_lock(pdev);
}

void dfs_mlme_release_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_release_radar_mode_switch_lock)
		return;

	global_dfs_to_mlme.mlme_release_radar_mode_switch_lock(pdev);
}
