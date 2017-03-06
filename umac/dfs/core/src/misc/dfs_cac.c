/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"

static int ieee80211_nol_timeout = 30*60; /* 30 minutes */
#define NOL_TIMEOUT (ieee80211_nol_timeout*1000)
#define IS_CHANNEL_WEATHER_RADAR(freq) ((freq >= 5600) && (freq <= 5650))
#define ADJACENT_WEATHER_RADAR_CHANNEL   5580
#define CH100_START_FREQ                 5490
#define CH100                            100

int dfs_override_cac_timeout(struct wlan_dfs *dfs, int cac_timeout)
{
	if (dfs == NULL)
		return -EIO;

	dfs->dfs_cac_timeout_override = cac_timeout;
	DFS_PRINTK("%s: CAC timeout is now %s %d\n", __func__,
		(cac_timeout == -1) ? "default" : "overridden",
		cac_timeout);

	return 0;
}

int dfs_get_override_cac_timeout(struct wlan_dfs *dfs, int *cac_timeout)
{
	if (dfs == NULL)
		return -EIO;

	(*cac_timeout) = dfs->dfs_cac_timeout_override;

	return 0;
}

void nif_dfs_detach(struct wlan_dfs *dfs)
{
	if (!dfs->dfs_enable)
		return;

	dfs->dfs_enable = 0;
	nif_dfs_reset(dfs);
	dfs_deinit_precac_list(dfs);
}

void nif_dfs_reset(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
	dfs->dfs_cac_timeout_override = -1;
	dfs_zero_cac_reset(dfs);
}

void dfs_cac_valid_reset(struct wlan_dfs *dfs,
		uint8_t prevchan_ieee,
		uint32_t prevchan_flags)
{
	if (dfs->dfs_cac_valid_time) {
		if ((prevchan_ieee != dfs->dfs_curchan->ic_ieee) ||
			(prevchan_flags != dfs->dfs_curchan->ic_flags)) {
			DFS_PRINTK(
				"%s : Cancelling timer & clearing cac_valid\n",
				__func__);
			qdf_timer_stop(&dfs->dfs_cac_valid_timer);
			dfs->dfs_cac_valid = 0;
		}
	}
}

uint16_t dfs_get_usenol(struct wlan_dfs *dfs)
{
	return dfs ? (uint16_t)dfs->dfs_rinfo.rn_use_nol : 0;
}

void dfs_set_update_nol_flag(struct wlan_dfs *dfs, bool val)
{
	dfs->update_nol = val;
}

bool dfs_get_update_nol_flag(struct wlan_dfs *dfs)
{
	return dfs->update_nol;
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
	DFS_PRINTK("%s : Timed out!!\n", __func__);
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

	DFS_PRINTK("%s cac expired, chan %d curr time %d\n", __func__,
		dfs->dfs_curchan->ic_freq,
		(qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000));
	/*
	 * When radar is detected during a CAC we are woken up prematurely to
	 * switch to a new channel. Check the channel to decide how to act.
	 */
	if (IEEE80211_IS_CHAN_RADAR(dfs->dfs_curchan)) {
		dfs_mlme_mark_dfs(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->ic_ieee,
				dfs->dfs_curchan->ic_freq,
				dfs->dfs_curchan->ic_vhtop_ch_freq_seg2,
				dfs->dfs_curchan->ic_flags);
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"CAC timer on channel %u (%u MHz) stopped due to radar\n",
			dfs->dfs_curchan->ic_ieee, dfs->dfs_curchan->ic_freq);
	} else {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"CAC timer on channel %u (%u MHz) expired; no radar detected\n",
			dfs->dfs_curchan->ic_ieee, dfs->dfs_curchan->ic_freq);

		/* On CAC completion, set the bit 'cac_valid'.
		 * CAC will not be re-done if this bit is reset.
		 * The flag will be reset when dfs_cac_valid_timer
		 * timesout.
		 */
		if (dfs->dfs_cac_valid_time) {
			dfs->dfs_cac_valid = 1;
			OS_SET_TIMER(&dfs->dfs_cac_valid_timer,
					dfs->dfs_cac_valid_time * 1000);
		}
	}

	/* Iterate over the nodes, processing the CAC completion event. */
	dfs_mlme_proc_cac(dfs->dfs_pdev_obj, 0);

	/* Send a CAC timeout, VAP up event to user space */
	dfs_mlme_deliver_event_up_afrer_cac(dfs->dfs_pdev_obj);

	if (dfs->dfs_defer_precac_channel_change == 1) {
		dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		dfs->dfs_defer_precac_channel_change = 0;
	}
}

int dfs_is_ap_cac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_cac_timer_running;
}

void dfs_start_cac_timer(struct wlan_dfs *dfs)
{
	OS_SET_TIMER(&dfs->dfs_cac_timer,
			dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->ic_freq,
				dfs->dfs_curchan->ic_vhtop_ch_freq_seg2,
				dfs->dfs_curchan->ic_flags) * 1000);
}

void dfs_cancel_cac_timer(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_cac_timer);
}

void dfs_cac_stop(struct wlan_dfs *dfs)
{
	uint32_t phyerr;

	dfs_get_debug_info(dfs, (void *)&phyerr);
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
		"%s : Stopping CAC Timer %d procphyerr 0x%08x\n",
		__func__, dfs->dfs_curchan->ic_freq, phyerr);
	qdf_timer_stop(&dfs->dfs_cac_timer);
	dfs->dfs_cac_timer_running = 0;
}

void dfs_stacac_stop(struct wlan_dfs *dfs)
{
	uint32_t phyerr;

	dfs_get_debug_info(dfs, (void *)&phyerr);
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
		"%s : Stopping STA CAC Timer %d procphyerr 0x%08x\n",
		__func__, dfs->dfs_curchan->ic_freq, phyerr);
}

/**
 * dfs_nol_timeout() - NOL timeout function.
 *
 * Clears the IEEE80211_CHAN_DFS_RADAR_FOUND flag for the NOL timeout channel.
 */
static os_timer_func(dfs_nol_timeout)
{
	struct dfs_ieee80211_channel *c = NULL, lc;
	unsigned long oldest, now;
	struct wlan_dfs *dfs = NULL;
	int i;
	int nchans = 0;

	c = &lc;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs_mlme_get_ic_nchans(dfs->dfs_pdev_obj, &nchans);

	now = oldest = qdf_system_ticks();
	for (i = 0; i < nchans; i++) {
		dfs_mlme_get_ic_channels(dfs->dfs_pdev_obj,
				&(c->ic_freq),
				&(c->ic_flags),
				&(c->ic_flagext),
				&(c->ic_ieee),
				&(c->ic_vhtop_ch_freq_seg1),
				&(c->ic_vhtop_ch_freq_seg2),
				i);
		if (IEEE80211_IS_CHAN_RADAR(c)) {
			if (qdf_system_time_after_eq(now,
				dfs->dfs_nol_event[i] + NOL_TIMEOUT)) {
				c->ic_flagext &=
					~IEEE80211_CHAN_DFS_RADAR_FOUND;
				if (c->ic_flags & IEEE80211_CHAN_DFS_RADAR) {
					/*
					 * NB: do this here so we get only one
					 * msg instead of one for every channel
					 * table entry.
					 */
					DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
						"%s : radar on channel %u (%u MHz) cleared after timeout\n",
						__func__,
						c->ic_ieee,
						c->ic_freq);
				}
			} else if (dfs->dfs_nol_event[i] < oldest)
				oldest = dfs->dfs_nol_event[i];
		}
	}
	if (oldest != now) {
		/* Arrange to process next channel up for a status change. */
		OS_SET_TIMER(&dfs->dfs_nol_timer, NOL_TIMEOUT -
				qdf_system_ticks_to_msecs(qdf_system_ticks()));
	}
}

void nif_dfs_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_enable = 1;
	dfs->dfs_cac_timeout_override = -1;
	dfs_zero_cac_attach(dfs);
	qdf_timer_init(NULL,
			&(dfs->dfs_cac_timer),
			dfs_cac_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_init(NULL,
			&(dfs->dfs_nol_timer),
			dfs_nol_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_init(NULL,
			&(dfs->dfs_cac_valid_timer),
			dfs_cac_valid_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
}

int dfs_random_channel(struct wlan_dfs *dfs,
		uint8_t is_select_nondfs,
		uint8_t skip_curchan)
{
	int chanStart, n = 0;
	uint32_t curChanFlags = 0, chan_flags, chan_flagext = 0;
	int numGChannels = 0;
	int numAChannels = 0;
	int j = 0;
	int ht160_count = 0;
	int ht80_80_count = 0;
	int ht80_count = 0;
	int ht40plus_count = 0;
	int ht40minus_count = 0;
	int ht20_count = 0;
	int use_lower_5g_only = 0;
	int use_upper_5g_only = 0;

	/*
	 * IR: 107025 -- Random channel selction not correct instead
	 * of uint8_t available_chan_idx[IEEE80211_CHAN_MAX+1] use
	 * int *available_chan_idx and dynamically allocate it
	 * storing int charStart in byte array available_chan_idx[]
	 * is reason for random channel selection failure when
	 * number of max channel (IEEE80211_CHAN_MAX) is more than 255.
	 */
	int *available_chan_idx;
	int available_chan_count = 0;
	int ret_val = -1;
	uint32_t alt_chan_mode = 0;
	int chan_count = 0;
	struct dfs_ieee80211_channel *c = NULL, lc;
	int nchans = 0;
	uint8_t no_wradar = 0;

	c = &lc;

	available_chan_idx = qdf_mem_malloc(
			(IEEE80211_CHAN_MAX + 1) * sizeof(int));

	if (available_chan_idx == NULL) {
		DFS_PRINTK("%s: cannot allocate memory\n", __func__);
		return ret_val;
	}

	/*
	 * FR 27305: In Japan domain, if current channel is below
	 * channel 100 then find a new channel that is below 100.
	 * Similarly if the current channel is 100 or higher then
	 * pick a channel that is 100 or higher.
	 */
	if (lmac_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_MKK4_DOMAIN) {
		if (IEEE80211_IS_CHAN_11AC_VHT80_80(dfs->dfs_curchan)) {
			/* No action required for now. */
			use_lower_5g_only = 0;
			use_upper_5g_only = 0;
			DFS_PRINTK(
				"%s -- MMK4 domain, HT80_80, no restriction on using upper or lower 5G channel\n",
				__func__);
		} else if (IEEE80211_IS_CHAN_11AC_VHT160(dfs->dfs_curchan)) {
			/* No action required for now. */
			use_lower_5g_only = 0;
			use_upper_5g_only = 0;
			DFS_PRINTK(
				"%s -- MMK4 domain, HT160, will look for HT160. if can't find no restriction on using upper or lower 5G channel\n",
				__func__);
		} else {
			if (dfs->dfs_curchan->ic_freq < CH100_START_FREQ) {
				use_lower_5g_only = 1;
				use_upper_5g_only = 0;
				DFS_PRINTK(
					"%s -- MMK4 domain, search for lower 5G (less than 5490 MHz) channels\n",
					__func__);
			} else {
				use_lower_5g_only = 0;
				use_upper_5g_only = 1;
				DFS_PRINTK(
					"%s -- MMK4 domain, search for upper 5G (more than 5490 MHz) channels\n",
					__func__);
			}
		}
	}

	/*
	 * Pick a random channel.
	 * Find how many G channels are present in the channel list.
	 * Assuming all G channels are present at the beginning of the
	 * list, followed by all A channels
	 */
	dfs_mlme_get_ic_nchans(dfs->dfs_pdev_obj, &nchans);
	for (j = 0; j < nchans; j++) {
		dfs_mlme_get_ic_channels(dfs->dfs_pdev_obj,
				&(c->ic_freq),
				&(c->ic_flags),
				&(c->ic_flagext),
				&(c->ic_ieee),
				&(c->ic_vhtop_ch_freq_seg1),
				&(c->ic_vhtop_ch_freq_seg2),
				j);
		chan_flags = c->ic_flags;
		if (chan_flags & IEEE80211_CHAN_2GHZ) {
			numGChannels++;
			continue;
		} else
			break;
	}

	numAChannels = (nchans - numGChannels);
	chanStart = numGChannels;

	curChanFlags = (dfs->dfs_curchan->ic_flags) & IEEE80211_CHAN_ALL;
	if (dfs_mlme_ic_flags_ext(dfs->dfs_pdev_obj) &
			IEEE80211_FEXT_BLKDFSCHAN)
		curChanFlags &= ~IEEE80211_CHAN_DFS;

	for (n = 0; n < nchans; chanStart++, n++) {
		if (chanStart == nchans)
			chanStart = 0;
		dfs_mlme_get_ic_channels(dfs->dfs_pdev_obj,
				&(c->ic_freq),
				&(c->ic_flags),
				&(c->ic_flagext),
				&(c->ic_ieee),
				&(c->ic_vhtop_ch_freq_seg1),
				&(c->ic_vhtop_ch_freq_seg2),
				chanStart);
		chan_flags = c->ic_flags;
		chan_flagext = c->ic_flagext;

		if (skip_curchan) {
			/* Skip curchan when choosing apriori random channel.
			 */
			if (c->ic_freq == dfs->dfs_curchan->ic_freq)
				continue;
		}

		/* These channels have CAC of 10 minutes so skipping these. */
		dfs_mlme_get_ic_no_weather_radar_chan(dfs->dfs_pdev_obj,
				&no_wradar);
		if (no_wradar) {
			/*
			 * We should also avoid this channel in HT40 mode as
			 * extension channel will be on 5600.
			 */
			uint32_t freq = 0;

			freq = dfs_ieee80211_chan2freq(c);

			if (((IS_CHANNEL_WEATHER_RADAR(freq)) ||
					((IEEE80211_CHAN_11NA_HT40PLUS &
					  chan_flags)
			 && (freq == ADJACENT_WEATHER_RADAR_CHANNEL)
			)) && (DFS_ETSI_DOMAIN ==
			lmac_get_dfsdomain(dfs->dfs_pdev_obj)))
				continue;
		}
#undef ADJACENT_WEATHER_RADAR_CHANNEL

		/*
		 * 1) Skip static turbo channel as it will require STA to be
		 *    in static turbo to work.
		 * 2) Skip channel which's marked with radar detction.
		 * 3) WAR: We allow user to config not to use any DFS channel.
		 * When we pick a channel, skip excluded 11D channels.
		 * See bug 3124.
		 */
		if ((chan_flags & IEEE80211_CHAN_STURBO) ||
			(chan_flags & IEEE80211_CHAN_DFS_RADAR) ||
			(chan_flagext & IEEE80211_CHAN_11D_EXCLUDED) ||
			(chan_flagext & IEEE80211_CHAN_DFS &&
			 dfs_mlme_ic_flags_ext(dfs->dfs_pdev_obj) &
			 IEEE80211_FEXT_BLKDFSCHAN) ||
			(chan_flagext & IEEE80211_CHAN_DFS && is_select_nondfs))
			continue;

		/*
		 * FR 27305: In Japan domain, if current channel is below
		 * channel 100 then find a new channel that is below 100.
		 * Similarly if the current channel is 100 or higher then
		 * pick a channel that is 100 or higher.
		 */
		if (use_lower_5g_only) {
			if (IEEE80211_IS_CHAN_11AC_VHT80_80(c)) {
				if ((c->ic_freq > CH100_START_FREQ) ||
					(c->ic_vhtop_ch_freq_seg2 > CH100)) {
					/* Skip this channel. */
					continue;
				}
			} else {
				if (c->ic_freq > CH100_START_FREQ) {
					/* Skip this channel. */
					continue;
				}
			}
		}

		if (use_upper_5g_only) {
			if (IEEE80211_IS_CHAN_11AC_VHT80_80(c)) {
				if ((c->ic_freq < CH100_START_FREQ) ||
					(c->ic_vhtop_ch_freq_seg2 < CH100)) {
					/* Skip this channel. */
					continue;
				}
			} else {
				if (c->ic_freq < CH100_START_FREQ) {
					/* Skip this channel. */
					continue;
				}
			}
		}

		/*
		 * Keep a count of VHT160, VHT80_80 and VHT80 channels
		 * so that we can move from VHT160 to VHT80_80 to VHT80
		 * if we cannot find a channel in current mode.
		 */
		if (chan_flags & IEEE80211_CHAN_VHT20)
			ht20_count++;
		else if (chan_flags & IEEE80211_CHAN_VHT40PLUS)
			ht40plus_count++;
		else if (chan_flags & IEEE80211_CHAN_VHT40MINUS)
			ht40minus_count++;
		else if (chan_flags & IEEE80211_CHAN_VHT80)
			ht80_count++;
		else if (chan_flags & IEEE80211_CHAN_VHT80_80)
			ht80_80_count++;
		else if (chan_flags & IEEE80211_CHAN_VHT160)
			ht160_count++;

		if ((chan_flags & IEEE80211_CHAN_ALL) == curChanFlags) {
			available_chan_idx[available_chan_count++] = chanStart;
			if (available_chan_count >= IEEE80211_CHAN_MAX + 1)
				break;
		}
	}

	if (available_chan_count) {
		uint32_t random_byte = 0;

		get_random_bytes(&random_byte, 1);
		j = (random_byte + qdf_system_ticks()) % available_chan_count;
		chanStart = (available_chan_idx[j]);
		ret_val = chanStart;
	} else {
		DFS_PRINTK(
			"%s: Cannot find a channel, looking for channel in other mode. ht80_count=%d, ht80_80_count=%d, ht160_count=%d\n",
			__func__, ht80_count,
			ht80_80_count, ht160_count);
		/*
		 * We need to handle HT160/HT80_80 in a special way HT160
		 * has only two channels available. We will try to change
		 * to HT80_80 if we cannot find any 160 MHz contiguous
		 * channel. If there is no HT80_80 channel then we will
		 * look for HT80 channel. Also we will change HT80_80 to
		 * HT80 in case we can't find a HT80_80 channel. This can
		 * happen in some design with two 5G radios where one
		 * radio operates in channel 36 through 64. The same could
		 * be done for other 11AC modes but we have plenty of HT80,
		 * HT40 and HT20 channels. The following code can also
		 * be enhanced to switch automatically to a wider channel
		 * whenever one is present.
		 */
		if (ht160_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT160;
			chan_count    = ht160_count;
		} else if (ht80_80_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT80_80;
			chan_count    = ht80_80_count;
		} else if (ht80_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT80;
			chan_count    = ht80_count;
		} else if (ht40plus_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT40PLUS;
			chan_count    = ht40plus_count;
		} else if (ht40minus_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT40MINUS;
			chan_count    = ht40minus_count;
		} else if (ht20_count > 0) {
			alt_chan_mode = IEEE80211_CHAN_VHT20;
			chan_count    = ht20_count;
		}
		if (chan_count) {
			ret_val = dfs_mlme_find_alternate_mode_channel
				(dfs->dfs_pdev_obj, alt_chan_mode, chan_count);
			if (ret_val == -1) {
				/* Last attempt to get a valid channel. */
				DFS_PRINTK(
					"%s: Cannot find a channel. Forcing to first available HT20 channel\n",
					__func__);
				dfs_mlme_find_any_valid_channel
					(dfs->dfs_pdev_obj,
					 IEEE80211_CHAN_VHT20, &ret_val);
			}
		} else {
			/*
			 * When all the DFS channels are in NOL and there
			 * is no DFS channel, chan_count is 0 and observed
			 * 'Division by zero in kernel'.
			 */
			ret_val = -1;
		}
	}
	qdf_mem_free(available_chan_idx);

	return ret_val;
}
