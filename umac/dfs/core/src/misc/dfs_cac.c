/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
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
#include "../dfs_etsi_precac.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"
#include "../dfs_process_radar_found_ind.h"

#define IS_CHANNEL_WEATHER_RADAR(freq) ((freq >= 5600) && (freq <= 5650))
#define ADJACENT_WEATHER_RADAR_CHANNEL   5580
#define CH100_START_FREQ                 5490
#define CH100                            100

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

void dfs_cac_valid_reset(struct wlan_dfs *dfs,
		uint8_t prevchan_ieee,
		uint32_t prevchan_flags)
{
	if (dfs->dfs_cac_valid_time) {
		if ((prevchan_ieee != dfs->dfs_curchan->dfs_ch_ieee) ||
			(prevchan_flags != dfs->dfs_curchan->dfs_ch_flags)) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"Cancelling timer & clearing cac_valid"
					);
			qdf_timer_stop(&dfs->dfs_cac_valid_timer);
			dfs->dfs_cac_valid = 0;
		}
	}
}

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
 * dfs_cac_timeout() - DFS cactimeout function.
 *
 * Sets dfs_cac_timer_running to 0  and dfs_cac_valid_timer.
 */
static os_timer_func(dfs_cac_timeout)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs->dfs_cac_timer_running = 0;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "cac expired, chan %d curr time %d",
		dfs->dfs_curchan->dfs_ch_freq,
		(qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000));

	/* Once CAC is done, add channel to ETSI precacdone list*/
	if (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)
		dfs_add_to_etsi_precac_done_list(dfs);

	/*
	 * When radar is detected during a CAC we are woken up prematurely to
	 * switch to a new channel. Check the channel to decide how to act.
	 */
	if (WLAN_IS_CHAN_RADAR(dfs->dfs_curchan)) {
		dfs_mlme_mark_dfs(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->dfs_ch_ieee,
				dfs->dfs_curchan->dfs_ch_freq,
				dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
				dfs->dfs_curchan->dfs_ch_flags);
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"CAC timer on channel %u (%u MHz) stopped due to radar",
			dfs->dfs_curchan->dfs_ch_ieee,
			dfs->dfs_curchan->dfs_ch_freq);
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"CAC timer on channel %u (%u MHz) expired; no radar detected",
			dfs->dfs_curchan->dfs_ch_ieee,
			dfs->dfs_curchan->dfs_ch_freq);

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
	}

	/* Iterate over the nodes, processing the CAC completion event. */
	dfs_mlme_proc_cac(dfs->dfs_pdev_obj, 0);

	/* Send a CAC timeout, VAP up event to user space */
	dfs_mlme_deliver_event_up_after_cac(dfs->dfs_pdev_obj);

	if (dfs->dfs_defer_precac_channel_change == 1) {
		dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		dfs->dfs_defer_precac_channel_change = 0;
	}
}

void dfs_cac_timer_init(struct wlan_dfs *dfs)
{
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

void dfs_cac_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_cac_timeout_override = -1;
	dfs->wlan_dfs_cac_time = WLAN_DFS_WAIT_MS;
	dfs_cac_timer_init(dfs);
}

void dfs_cac_timer_reset(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
	dfs_get_override_cac_timeout(dfs,
			&(dfs->dfs_cac_timeout_override));
	qdf_mem_zero(&dfs->dfs_cac_started_chan,
		     sizeof(dfs->dfs_cac_started_chan));

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

void dfs_start_cac_timer(struct wlan_dfs *dfs)
{
	int cac_timeout = 0;
	struct dfs_channel *chan = dfs->dfs_curchan;

	cac_timeout = dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
					       chan->dfs_ch_freq,
					       chan->dfs_ch_vhtop_ch_freq_seg2,
					       chan->dfs_ch_flags);

	dfs->dfs_cac_started_chan = *chan;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "chan = %d cfreq2 = %d timeout = %d sec, curr_time = %d sec",
		  chan->dfs_ch_ieee, chan->dfs_ch_vhtop_ch_freq_seg2,
		  cac_timeout,
		  qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000);

	qdf_timer_mod(&dfs->dfs_cac_timer, cac_timeout * 1000);
	dfs->dfs_cac_aborted = 0;
}

void dfs_cancel_cac_timer(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
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
	dfs->dfs_cac_timer_running = 0;
}

void dfs_stacac_stop(struct wlan_dfs *dfs)
{
	uint32_t phyerr;

	dfs_get_debug_info(dfs, (void *)&phyerr);
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Stopping STA CAC Timer %d procphyerr 0x%08x",
		 dfs->dfs_curchan->dfs_ch_freq, phyerr);
}

bool dfs_is_subset_channel(struct wlan_dfs *dfs,
			   struct dfs_channel *old_chan,
			   struct dfs_channel *new_chan)
{
	uint8_t old_subchans[NUM_CHANNELS_160MHZ];
	uint8_t new_subchans[NUM_CHANNELS_160MHZ];
	uint8_t old_n_chans;
	uint8_t new_n_chans;
	int i = 0, j = 0;
	bool is_found = false;

	if (WLAN_IS_CHAN_11AC_VHT160(old_chan) ||
	    WLAN_IS_CHAN_11AC_VHT80_80(old_chan)) {
		/* If primary segment is NON-DFS */
		if (!WLAN_IS_CHAN_DFS(old_chan))
			old_n_chans = dfs_get_bonding_channels(dfs,
							       old_chan,
							       SEG_ID_SECONDARY,
							       old_subchans);
		else
			old_n_chans = dfs_get_bonding_channels_without_seg_info(
					old_chan, old_subchans);
	} else {
		old_n_chans = dfs_get_bonding_channels_without_seg_info(
				old_chan, old_subchans);
	}

	if (WLAN_IS_CHAN_11AC_VHT160(new_chan) ||
	    WLAN_IS_CHAN_11AC_VHT80_80(new_chan)) {
		/* If primary segment is NON-DFS */
		if (WLAN_IS_CHAN_DFS(new_chan))
			new_n_chans = dfs_get_bonding_channels(
					dfs, new_chan, SEG_ID_SECONDARY,
					new_subchans);
		else
			new_n_chans = dfs_get_bonding_channels_without_seg_info(
					new_chan, new_subchans);
	} else {
		new_n_chans = dfs_get_bonding_channels_without_seg_info(
				new_chan, new_subchans);
	}

	if (new_n_chans > old_n_chans)
		return false;

	for (i = 0; i < new_n_chans; i++) {
		is_found = false;
		for (j = 0; j < old_n_chans; j++) {
			if (new_subchans[i] == old_subchans[j]) {
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

bool dfs_is_curchan_subset_of_cac_started_chan(struct wlan_dfs *dfs)
{
	return dfs_is_subset_channel(dfs, &dfs->dfs_cac_started_chan,
				     dfs->dfs_curchan);
}

void dfs_clear_cac_started_chan(struct wlan_dfs *dfs)
{
	qdf_mem_zero(&dfs->dfs_cac_started_chan,
		     sizeof(dfs->dfs_cac_started_chan));
}

bool dfs_check_for_cac_start(struct wlan_dfs *dfs,
			     bool *continue_current_cac)
{
	if (dfs->dfs_ignore_dfs || dfs->dfs_cac_valid || dfs->dfs_ignore_cac) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Skip CAC, ignore_dfs = %d cac_valid = %d ignore_cac = %d",
			  dfs->dfs_ignore_dfs, dfs->dfs_cac_valid,
			  dfs->dfs_ignore_cac);
		return false;
	}

	if (dfs_is_etsi_precac_done(dfs)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "ETSI PRE-CAC alreay done on this channel %d",
			  dfs->dfs_curchan->dfs_ch_ieee);
		return false;
	}

	/* If the channel has completed PRE-CAC then CAC can be skipped here. */
	if (dfs_is_precac_done(dfs, dfs->dfs_curchan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "PRE-CAC alreay done on this channel %d",
			  dfs->dfs_curchan->dfs_ch_ieee);
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
		if (dfs_is_curchan_subset_of_cac_started_chan(dfs)) {
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
		if (dfs_is_curchan_subset_of_cac_started_chan(dfs)) {
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
