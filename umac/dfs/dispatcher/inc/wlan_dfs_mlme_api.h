/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * DOC: These APIs are used by DFS core functions to call mlme functions.
 */

#ifndef _WLAN_DFS_MLME_API_H_
#define _WLAN_DFS_MLME_API_H_

#include "wlan_dfs_ucfg_api.h"

extern struct dfs_to_mlme global_dfs_to_mlme;

/**
 * dfs_mlme_start_rcsa() - Send RCSA to RootAP.
 * @pdev: Pointer to DFS pdev object.
 * @wait_for_csa: Wait for CSA from RootAP.
 */
void dfs_mlme_start_rcsa(struct wlan_objmgr_pdev *pdev,
		bool *wait_for_csa);

/**
 * dfs_mlme_mark_dfs() - Mark the channel in the channel list.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @freq: Channel frequency.
 * @vhtop_ch_freq_seg2: VHT80 Cfreq2.
 * @flags: channel flags.
 */
void dfs_mlme_mark_dfs(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee,
			uint16_t freq,
			uint8_t vhtop_ch_freq_seg2,
			uint64_t flags);

/**
 * dfs_mlme_start_csa() - Sends CSA in ieeeChan
 * @pdev: Pointer to DFS pdev object.
 * @ieee_chan: Channel number.
 * @freq: Channel frequency.
 * @cfreq2: HT80 cfreq2.
 * @flags: channel flags.
 */
void dfs_mlme_start_csa(struct wlan_objmgr_pdev *pdev,
		uint8_t ieee_chan,
		uint16_t freq,
		uint8_t cfreq2,
		uint64_t flags);

/**
 * dfs_mlme_proc_cac() - Process the CAC completion event.
 * @pdev: Pointer to DFS pdev object.
 * @vdev_id: vdev id.
 */
void dfs_mlme_proc_cac(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id);

/**
 * dfs_mlme_deliver_event_up_after_cac() - Send a CAC timeout, VAP up event to
 * userspace.
 * @pdev: Pointer to DFS pdev object.
 */
void dfs_mlme_deliver_event_up_after_cac(struct wlan_objmgr_pdev *pdev);

/**
 * dfs_mlme_get_dfs_ch_nchans() - Get number of channels in the channel list
 * @pdev: Pointer to DFS pdev object.
 * @nchans: Pointer to save the channel number.
 */
void dfs_mlme_get_dfs_ch_nchans(struct wlan_objmgr_pdev *pdev, int *nchans);

/**
 * dfs_mlme_get_extchan() - Get extension channel.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center frequency.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 */
QDF_STATUS dfs_mlme_get_extchan(struct wlan_objmgr_pdev *pdev,
		uint16_t *dfs_ch_freq,
		uint64_t *dfs_ch_flags,
		uint16_t *dfs_ch_flagext,
		uint8_t *dfs_ch_ieee,
		uint8_t *dfs_ch_vhtop_ch_freq_seg1,
		uint8_t *dfs_ch_vhtop_ch_freq_seg2);

/**
 * dfs_mlme_set_no_chans_available() - Set no_chans_available flag.
 * @pdev: Pointer to DFS pdev object.
 * @val: Set this value to no_chans_available flag.
 */
void dfs_mlme_set_no_chans_available(struct wlan_objmgr_pdev *pdev,
		int val);

/**
 * dfs_mlme_ieee2mhz() - Get the frequency from channel number.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @flag: Channel flag.
 */
int dfs_mlme_ieee2mhz(struct wlan_objmgr_pdev *pdev,
		int ieee,
		uint64_t flag);

/**
 * dfs_mlme_find_dot11_channel() - Get dot11 channel from ieee, cfreq2 and mode.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @des_cfreq2: cfreq2
 * @mode: Phymode
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center frequency.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 *
 * Return:
 * * QDF_STATUS_SUCCESS  : Channel found.
 * * QDF_STATUS_E_FAILURE: Channel not found.
 */
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
			    uint8_t *dfs_ch_vhtop_ch_freq_seg2);

/**
 * dfs_mlme_get_dfs_ch_channels() - Get channel from channel list.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center frequency.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 * @index: Index into channel list.
 */
void dfs_mlme_get_dfs_ch_channels(struct wlan_objmgr_pdev *pdev,
				  uint16_t *dfs_ch_freq,
				  uint64_t *dfs_ch_flags,
				  uint16_t *dfs_ch_flagext,
				  uint8_t *dfs_ch_ieee,
				  uint8_t *dfs_ch_vhtop_ch_freq_seg1,
				  uint8_t *dfs_ch_vhtop_ch_freq_seg2,
				  int index);

/**
 * dfs_mlme_dfs_ch_flags_ext() - Get extension channel flags.
 * @pdev: Pointer to DFS pdev object.
 */
uint32_t dfs_mlme_dfs_ch_flags_ext(struct wlan_objmgr_pdev *pdev);

/**
 * dfs_mlme_channel_change_by_precac() - Channel change by PreCAC.
 * @pdev: Pointer to DFS pdev object.
 */
void dfs_mlme_channel_change_by_precac(struct wlan_objmgr_pdev *pdev);

/**
 * dfs_mlme_nol_timeout_notification() - NOL timeout notification to userspace.
 * @pdev: Pointer to DFS pdev object.
 */
void dfs_mlme_nol_timeout_notification(struct wlan_objmgr_pdev *pdev);

/**
 * dfs_mlme_clist_update() - Mark the channel as RADAR.
 * @pdev: Pointer to DFS pdev object.
 * @nollist: Pointer to NOL list.
 * @nentries: Number of channels in the NOL list.
 */
void dfs_mlme_clist_update(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		int nentries);

/**
 * dfs_mlme_get_cac_timeout() - Get cac_timeout.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 * @dfs_ch_flags:               Channel flags.
 */
int dfs_mlme_get_cac_timeout(struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint8_t dfs_ch_vhtop_ch_freq_seg2,
		uint64_t dfs_ch_flags);

/**
 * dfs_mlme_rebuild_chan_list_with_non_dfs_channels() - Rebuild the channel list
 * with only non DFS channels.
 * @pdev: Pointer to DFS pdev object.
 *
 * return: On success return 1 or 0, else failure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
int dfs_mlme_rebuild_chan_list_with_non_dfs_channels(
		struct wlan_objmgr_pdev *pdev);
#else
static inline int dfs_mlme_rebuild_chan_list_with_non_dfs_channels(
		struct wlan_objmgr_pdev *pdev)
{
	return 0;
}
#endif

/**
 * dfs_mlme_restart_vaps_with_non_dfs_chan() - Restart vaps with non DFS
 * channels
 * @pdev: Pointer to DFS pdev object.
 * @no_chans_avail: Indicates if no channel is available.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_mlme_restart_vaps_with_non_dfs_chan(struct wlan_objmgr_pdev *pdev,
					     int no_chans_avail);
#else
static inline
void dfs_mlme_restart_vaps_with_non_dfs_chan(struct wlan_objmgr_pdev *pdev,
					     int no_chans_avail)
{
}
#endif

/**
 * dfs_mlme_check_allowed_prim_chanlist() - Check whether the given channel is
 * present in the primary allowed channel list or not
 * @pdev: Pointer to DFS pdev object.
 * @chan_num: Channel number
 */
#if defined(WLAN_SUPPORT_PRIMARY_ALLOWED_CHAN)
bool dfs_mlme_check_allowed_prim_chanlist(struct wlan_objmgr_pdev *pdev,
					  uint32_t chan_num);

#else
static inline
bool dfs_mlme_check_allowed_prim_chanlist(struct wlan_objmgr_pdev *pdev,
					  uint32_t chan_num)
{
	return true;
}
#endif

/**
 * dfs_mlme_handle_dfs_scan_violation() - Handle scan start failure
 * due to DFS violation (presence of NOL channel in scan channel list).
 * @pdev: Pointer to pdev object.
 */
#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
void dfs_mlme_handle_dfs_scan_violation(struct wlan_objmgr_pdev *pdev);
#else
static inline
void dfs_mlme_handle_dfs_scan_violation(struct wlan_objmgr_pdev *pdev)
{
}
#endif

/**
 * dfs_mlme_is_opmode_sta() - Check if pdev opmode is STA.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: true if pdev opmode is STA, else false.
 */
bool dfs_mlme_is_opmode_sta(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_DFS_MLME_API_H_ */
