/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DOC: This file has the functions related to DFS CAC.
 */

#include "../dfs_channel.h"
#include "../dfs_zero_cac.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"
#include "../dfs_process_radar_found_ind.h"

#define IS_CHANNEL_WEATHER_RADAR(freq) ((freq >= 5600) && (freq <= 5650))
#define ADJACENT_WEATHER_RADAR_CHANNEL   5580
#define CH100_START_FREQ                 5490
#define CH100                            100

/**
 * dfs_cac_valid_timeout() - Timeout function for dfs_cac_valid_timer
 *                           cac_valid bit will be reset in this function.
 */
static os_timer_func(dfs_cac_valid_timeout)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs->dfs_cac_valid = 0;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, ": Timed out!!");
}

/**
 * dfs_clear_cac_started_chan() - Clear dfs cac started channel.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_clear_cac_started_chan(struct wlan_dfs *dfs)
{
	qdf_mem_zero(&dfs->dfs_cac_started_chan,
		     sizeof(dfs->dfs_cac_started_chan));
}

static void dfs_clear_nol_history_for_curchan(struct wlan_dfs *dfs)
{
	struct dfs_channel *chan = dfs->dfs_curchan;
	uint16_t sub_channels[MAX_20MHZ_SUBCHANS];
	uint8_t num_subchs;

	num_subchs = dfs_get_bonding_channel_without_seg_info_for_freq(
				chan, sub_channels);

	if (dfs->dfs_is_stadfs_enabled)
		if (dfs_mlme_is_opmode_sta(dfs->dfs_pdev_obj))
			utils_dfs_reg_update_nol_history_chan_for_freq(
				dfs->dfs_pdev_obj, sub_channels,
				num_subchs, DFS_NOL_HISTORY_RESET);
}

void dfs_process_cac_completion(struct wlan_dfs *dfs)
{
	enum phy_ch_width ch_width = CH_WIDTH_INVALID;
	uint16_t primary_chan_freq = 0, sec_chan_freq = 0;
	struct dfs_channel *dfs_curchan;

	dfs->dfs_cac_timer_running = 0;
	dfs_curchan = dfs->dfs_curchan;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "cac expired, chan %d cur time %d",
		 dfs->dfs_curchan->dfs_ch_freq,
		 (qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000));

	/*
	 * When radar is detected during a CAC we are woken up prematurely to
	 * switch to a new channel. Check the channel to decide how to act.
	 */
	if (WLAN_IS_CHAN_RADAR(dfs, dfs->dfs_curchan)) {
		dfs_mlme_mark_dfs(dfs->dfs_pdev_obj,
				  dfs_curchan->dfs_ch_ieee,
				  dfs_curchan->dfs_ch_freq,
				  dfs_curchan->dfs_ch_mhz_freq_seg2,
				  dfs_curchan->dfs_ch_flags);
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "CAC timer on chan %u (%u MHz) stopped due to radar",
			  dfs_curchan->dfs_ch_ieee,
			  dfs_curchan->dfs_ch_freq);
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "CAC timer on channel %u (%u MHz) expired;"
			  "no radar detected",
			  dfs_curchan->dfs_ch_ieee,
			  dfs_curchan->dfs_ch_freq);

		/* On CAC completion, set the bit 'cac_valid'.
		 * CAC will not be re-done if this bit is reset.
		 * The flag will be reset when dfs_cac_valid_timer
		 * timesout.
		 */
		if (dfs->dfs_cac_valid_time) {
			dfs->dfs_cac_valid = 1;
			qdf_timer_mod(&dfs->dfs_cac_valid_timer,
				      dfs->dfs_cac_valid_time * 1000);
		}

		dfs_find_curchwidth_and_center_chan_for_freq(dfs,
							     &ch_width,
							     &primary_chan_freq,
							     &sec_chan_freq);

		/* ETSI allows the driver to cache the CAC ( Once CAC done,
		 * it can be used in future).
		 * Therefore mark the current channel CAC done.
		 */
		if (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)
			dfs_mark_precac_done_for_freq(dfs,
						      primary_chan_freq,
						      sec_chan_freq,
						      ch_width);
	}

	dfs_clear_cac_started_chan(dfs);

	/* Clear NOL history for current channel on successful CAC completion */
	dfs_clear_nol_history_for_curchan(dfs);
	/* Iterate over the nodes, processing the CAC completion event. */
	dfs_mlme_proc_cac(dfs->dfs_pdev_obj, 0);

	/* Send a CAC timeout, VAP up event to user space */
	dfs_mlme_deliver_event_up_after_cac(dfs->dfs_pdev_obj);

	if (dfs->dfs_defer_precac_channel_change == 1) {
		dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		dfs->dfs_defer_precac_channel_change = 0;
	}
}

/**
 * dfs_cac_timeout() - DFS cactimeout function.
 *
 * Sets dfs_cac_timer_running to 0  and dfs_cac_valid_timer.
 */
#ifdef CONFIG_CHAN_FREQ_API
static os_timer_func(dfs_cac_timeout)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);

	if (dfs_is_hw_mode_switch_in_progress(dfs))
		dfs->dfs_defer_params.is_cac_completed = true;
	else
		dfs_process_cac_completion(dfs);
}
#endif

#ifdef QCA_SUPPORT_DFS_CAC
void dfs_cac_timer_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_cac_timeout_override = -1;
	dfs->wlan_dfs_cac_time = WLAN_DFS_WAIT_MS;
	qdf_timer_init(NULL,
			&(dfs->dfs_cac_timer),
			dfs_cac_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_init(NULL,
			&(dfs->dfs_cac_valid_timer),
			dfs_cac_valid_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_cac_timer_reset(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
	dfs_get_override_cac_timeout(dfs,
			&(dfs->dfs_cac_timeout_override));
	dfs_clear_cac_started_chan(dfs);
}

void dfs_cac_timer_detach(struct wlan_dfs *dfs)
{
	qdf_timer_free(&dfs->dfs_cac_timer);

	qdf_timer_free(&dfs->dfs_cac_valid_timer);
	dfs->dfs_cac_valid = 0;
}

int dfs_is_ap_cac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_cac_timer_running;
}

#ifdef CONFIG_CHAN_FREQ_API
void dfs_start_cac_timer(struct wlan_dfs *dfs)
{
	int cac_timeout = 0;
	struct dfs_channel *chan = dfs->dfs_curchan;

	cac_timeout =
	    dfs_mlme_get_cac_timeout_for_freq(dfs->dfs_pdev_obj,
					      chan->dfs_ch_freq,
					      chan->dfs_ch_mhz_freq_seg2,
					      chan->dfs_ch_flags);

	dfs->dfs_cac_started_chan = *chan;

	dfs_deliver_cac_state_events(dfs);
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "chan = %d cfreq2 = %d timeout = %d sec, curr_time = %d sec",
		  chan->dfs_ch_ieee, chan->dfs_ch_vhtop_ch_freq_seg2,
		  cac_timeout,
		  qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000);

	qdf_timer_mod(&dfs->dfs_cac_timer, cac_timeout * 1000);
	dfs->dfs_cac_aborted = 0;
}
#endif

void dfs_cancel_cac_timer(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
	dfs_clear_cac_started_chan(dfs);
}

void dfs_cac_stop(struct wlan_dfs *dfs)
{
	uint32_t phyerr;

	dfs_get_debug_info(dfs, (void *)&phyerr);
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Stopping CAC Timer %d procphyerr 0x%08x",
		 dfs->dfs_curchan->dfs_ch_freq, phyerr);
	qdf_timer_stop(&dfs->dfs_cac_timer);
	if (dfs->dfs_cac_timer_running)
		dfs->dfs_cac_aborted = 1;
	dfs_clear_cac_started_chan(dfs);
	dfs->dfs_cac_timer_running = 0;
}

void dfs_stacac_stop(struct wlan_dfs *dfs)
{
	uint32_t phyerr;

	dfs_get_debug_info(dfs, (void *)&phyerr);
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Stopping STA CAC Timer %d procphyerr 0x%08x",
		 dfs->dfs_curchan->dfs_ch_freq, phyerr);
	dfs_clear_cac_started_chan(dfs);
}

int dfs_override_cac_timeout(struct wlan_dfs *dfs, int cac_timeout)
{
	if (!dfs)
		return -EIO;

	dfs->dfs_cac_timeout_override = cac_timeout;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "CAC timeout is now %s %d",
		 (cac_timeout == -1) ? "default" : "overridden",
		 cac_timeout);

	return 0;
}

int dfs_get_override_cac_timeout(struct wlan_dfs *dfs, int *cac_timeout)
{
	if (!dfs)
		return -EIO;

	(*cac_timeout) = dfs->dfs_cac_timeout_override;

	return 0;
}

#ifdef CONFIG_CHAN_FREQ_API
void dfs_cac_valid_reset_for_freq(struct wlan_dfs *dfs,
				  uint16_t prevchan_freq,
				  uint32_t prevchan_flags)
{
	if (dfs->dfs_cac_valid_time) {
		if ((prevchan_freq != dfs->dfs_curchan->dfs_ch_freq) ||
		    (prevchan_flags != dfs->dfs_curchan->dfs_ch_flags)) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Cancelling timer & clearing cac_valid");
			qdf_timer_stop(&dfs->dfs_cac_valid_timer);
			dfs->dfs_cac_valid = 0;
		}
	}
}
#endif
#endif

/*
 * dfs_is_subset_channel_for_freq() - Find out if prev channel and current
 * channel are subsets of each other.
 * @old_subchans_freq: Pointer to previous sub-channels freq.
 * @old_n_chans: Number of previous sub-channels.
 * @new_subchans_freq: Pointer to new sub-channels freq.
 * @new_n_chans:  Number of new sub-channels
 */
#ifdef CONFIG_CHAN_FREQ_API
static bool
dfs_is_subset_channel_for_freq(uint16_t *old_subchans_freq,
			       uint8_t old_n_chans,
			       uint16_t *new_subchans_freq,
			       uint8_t new_n_chans)
{
	bool is_found;
	int i, j;

	if (!new_n_chans)
		return true;

	if (new_n_chans > old_n_chans)
		return false;

	for (i = 0; i < new_n_chans; i++) {
		is_found = false;
		for (j = 0; j < old_n_chans; j++) {
			if (new_subchans_freq[i] == old_subchans_freq[j]) {
				is_found = true;
				break;
			}
		}

		/* If new_subchans[i] is not found in old_subchans, then,
		 * new_chan is not subset of old_chan.
		 */
		if (!is_found)
			break;
	}

	return is_found;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
uint8_t
dfs_find_dfs_sub_channels_for_freq(struct wlan_dfs *dfs,
				   struct dfs_channel *chan,
				   uint16_t *subchan_arr)
{
	if (WLAN_IS_CHAN_MODE_160(chan) || WLAN_IS_CHAN_MODE_80_80(chan)) {
		if (WLAN_IS_CHAN_DFS(chan) && WLAN_IS_CHAN_DFS_CFREQ2(chan))
			return dfs_get_bonding_channel_without_seg_info_for_freq
				(chan, subchan_arr);
		if (WLAN_IS_CHAN_DFS(chan))
			return dfs_get_bonding_channels_for_freq(dfs,
								 chan,
								 SEG_ID_PRIMARY,
								 DETECTOR_ID_0,
								 subchan_arr);
		if (WLAN_IS_CHAN_DFS_CFREQ2(chan))
			return dfs_get_bonding_channels_for_freq
				(dfs, chan, SEG_ID_SECONDARY,
				 DETECTOR_ID_0, subchan_arr);
		/* All channels in 160/80_80 BW are non DFS, return 0
		 * as number of subchannels
		 */
		return 0;
	} else if (WLAN_IS_CHAN_DFS(chan)) {
		return dfs_get_bonding_channel_without_seg_info_for_freq
			(chan, subchan_arr);
	}
	/* All channels are non DFS, return 0 as number of subchannels*/
	return 0;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
bool
dfs_is_new_chan_subset_of_old_chan(struct wlan_dfs *dfs,
				   struct dfs_channel *new_chan,
				   struct dfs_channel *old_chan)
{
	uint16_t new_subchans[MAX_20MHZ_SUBCHANS];
	uint16_t old_subchans[MAX_20MHZ_SUBCHANS];
	uint8_t n_new_subchans = 0;
	uint8_t n_old_subchans = 0;

	/* Given channel is the old channel. i.e. The channel which
	 * should have the new channel as subset.
	 */
	n_old_subchans = dfs_find_dfs_sub_channels_for_freq(dfs, old_chan,
							    old_subchans);
	/* cur_chan is the new channel to be check if subset of old channel */
	n_new_subchans = dfs_find_dfs_sub_channels_for_freq(dfs, new_chan,
							    new_subchans);

	return dfs_is_subset_channel_for_freq(old_subchans,
					      n_old_subchans,
					      new_subchans,
					      n_new_subchans);
}
#endif

#ifdef QCA_SUPPORT_DFS_CAC
bool dfs_is_cac_required(struct wlan_dfs *dfs,
			 struct dfs_channel *cur_chan,
			 struct dfs_channel *prev_chan,
			 bool *continue_current_cac,
			 bool is_vap_restart)
{
	struct dfs_channel *cac_started_chan = &dfs->dfs_cac_started_chan;

	if (!WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(cur_chan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS, "Skip CAC on non-DFS channel");
		return false;
	}

	if (dfs->dfs_ignore_dfs || dfs->dfs_cac_valid || dfs->dfs_ignore_cac) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Skip CAC, ignore_dfs = %d cac_valid = %d ignore_cac = %d",
			  dfs->dfs_ignore_dfs, dfs->dfs_cac_valid,
			  dfs->dfs_ignore_cac);
		return false;
	}

	/* If the channel has completed PRE-CAC then CAC can be skipped here. */
	if (dfs_is_precac_done(dfs, cur_chan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "PRE-CAC alreay done on this channel %d",
			  cur_chan->dfs_ch_ieee);
		return false;
	}

	if (dfs_is_ap_cac_timer_running(dfs)) {
		/* Check if we should continue the existing CAC or
		 * cancel the existing CAC.
		 * For example: - if an existing VAP(0) is already in
		 * DFS wait state (which means the radio(wifi) is
		 * running the CAC) and it is in channel A and another
		 * VAP(1) comes up in the same channel then instead of
		 * cancelling the CAC we can let the CAC continue.
		 */
		if (dfs_is_new_chan_subset_of_old_chan(dfs,
						       cur_chan,
						       cac_started_chan)) {
			*continue_current_cac = true;
		} else {
			/* New CAC is needed, cancel the running CAC
			 * timer.
			 * 1) When AP is in DFS_WAIT state and it is in
			 *    channel A and user restarts the AP vap in
			 *    channel B, then cancel the running CAC in
			 *    channel A and start new CAC in channel B.
			 *
			 * 2) When AP detects the RADAR during CAC in
			 *    channel A, it cancels the running CAC and
			 *    tries to find channel B with the reduced
			 *    bandwidth with of channel A.
			 *    In this case, since the CAC is aborted by
			 *    the RADAR, AP should start the CAC again.
			 */
			dfs_cancel_cac_timer(dfs);
		}
	} else { /* CAC timer is not running. */
		/* If channel change happens via VAP DOWN/UP on subset channels,
		 * (eg: from 52 HT80 to 64 HT80) CAC done information
		 * (of 52 HT80) based on subset logic
		 * (as 52 and 64 HT80 are subsets of each other)
		 * is not expected to be preserved as VAP has come up
		 * from DOWN state. Hence do not skip CAC on 64 HT80.
		 * is_vap_restart flag is used as an identifer to indicate if
		 * vap has come up from a DOWN state or UP state (vap restart).
		 */
		if (!is_vap_restart) {
			dfs_debug(dfs, WLAN_DEBUG_DFS, "CAC is needed");
			return true;
		}
		if (dfs_is_new_chan_subset_of_old_chan(dfs,
						       cur_chan,
						       prev_chan)) {
			/* AP bandwidth reduce case:
			 * When AP detects the RADAR in in-service monitoring
			 * mode in channel A, it cancels the running CAC and
			 * tries to find the channel B with the reduced
			 * bandwidth of channel A.
			 * If the new channel B is subset of the channel A
			 * then AP skips the CAC.
			 */
			if (!dfs->dfs_cac_aborted) {
				dfs_debug(dfs, WLAN_DEBUG_DFS, "Skip CAC");
				return false;
			}
		}
	}

	return true;
}
#endif
