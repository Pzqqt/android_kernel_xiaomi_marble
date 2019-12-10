/*
 * Copyright (c) 2013, 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This contains the functionality to process the radar event generated
 * for a pulse. This will group together pulses and call various detection
 * functions to figure out whether a valid radar has been detected.
 */

#include "../dfs.h"
#include "../dfs_zero_cac.h"
#include "../dfs_channel.h"
#include "../dfs_internal.h"
#include "../dfs_process_radar_found_ind.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../dfs_partial_offload_radar.h"

#define FREQ_5500_MHZ  5500
#define FREQ_5500_MHZ       5500

#define DFS_MAX_FREQ_SPREAD            (1375 * 1)
#define DFS_LARGE_PRI_MULTIPLIER       4
#define DFS_W53_DEFAULT_PRI_MULTIPLIER 2
//#define DFS_INVALID_PRI_LIMIT 100  /* should we use 135? */
#define DFS_INVALID_PRI_LIMIT 15  /* should we use 135? */
#define DFS_BIG_SIDX          10000

#define FRAC_PRI_SCORE_ARRAY_SIZE 40

static char debug_dup[33];
static int debug_dup_cnt;

/**
 * dfs_process_pulse_dur() - Process pulse duration.
 * @dfs: Pointer to wlan_dfs structure.
 * @re_dur: Duration.
 *
 * Convert the hardware provided duration to TSF ticks (usecs) taking the clock
 * (fast or normal) into account. Legacy (pre-11n, Owl, Sowl, Howl) operate
 * 5GHz using a 40MHz clock.  Later 11n chips (Merlin, Osprey, etc) operate
 * 5GHz using a 44MHz clock, so the reported pulse durations are different.
 * Peregrine reports the pulse duration in microseconds regardless of the
 * operating mode. (XXX TODO: verify this, obviously.)
 *
 * The hardware returns the duration in a variety of formats,
 * so it's converted from the hardware format to TSF (usec)
 * values here.
 * XXX TODO: this should really be done when the PHY error
 * is processed, rather than way out here..
 *
 *
 * Return: Returns the duration.
 */
static inline uint8_t dfs_process_pulse_dur(struct wlan_dfs *dfs,
		uint8_t re_dur)
{
	/*
	 * Short pulses are sometimes returned as having a duration of 0,
	 * so round those up to 1.
	 * XXX This holds true for BB TLV chips too, right?
	 */
	if (re_dur == 0)
		return 1;

	/*
	 * For BB TLV chips, the hardware always returns microsecond pulse
	 * durations.
	 */
	if (dfs->dfs_caps.wlan_chip_is_bb_tlv)
		return re_dur;

	/*
	 * This is for 11n and legacy chips, which may or may not use the 5GHz
	 * fast clock mode.
	 */
	/* Convert 0.8us durations to TSF ticks (usecs) */
	return (uint8_t)dfs_round((int32_t)((dfs->dur_multiplier)*re_dur));
}

/*
 * dfs_print_radar_events() - Prints the Radar events.
 * @dfs: Pointer to wlan_dfs structure.
 */
static void dfs_print_radar_events(struct wlan_dfs *dfs)
{
	int i;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "#Phyerr=%d, #false detect=%d, #queued=%d",
		 dfs->dfs_phyerr_count, dfs->dfs_phyerr_reject_count,
		dfs->dfs_phyerr_queued_count);

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_phyerr_freq_min=%d, dfs_phyerr_freq_max=%d",
		 dfs->dfs_phyerr_freq_min, dfs->dfs_phyerr_freq_max);

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		"Total radar events detected=%d, entries in the radar queue follows:",
		 dfs->dfs_event_log_count);

	for (i = 0; (i < DFS_EVENT_LOG_SIZE) && (i < dfs->dfs_event_log_count);
			i++) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"ts=%llu diff_ts=%u rssi=%u dur=%u, is_chirp=%d, seg_id=%d, sidx=%d, freq_offset=%d.%dMHz, peak_mag=%d, total_gain=%d, mb_gain=%d, relpwr_db=%d, delta_diff=%d, delta_peak=%d, psidx_diff=%d",
			dfs->radar_log[i].ts, dfs->radar_log[i].diff_ts,
			dfs->radar_log[i].rssi, dfs->radar_log[i].dur,
			dfs->radar_log[i].is_chirp, dfs->radar_log[i].seg_id,
			dfs->radar_log[i].sidx,
			(int)dfs->radar_log[i].freq_offset_khz/1000,
			(int)abs(dfs->radar_log[i].freq_offset_khz)%1000,
			dfs->radar_log[i].peak_mag,
			dfs->radar_log[i].total_gain,
			dfs->radar_log[i].mb_gain,
			dfs->radar_log[i].relpwr_db,
			dfs->radar_log[i].delta_diff,
			dfs->radar_log[i].delta_peak,
			dfs->radar_log[i].psidx_diff);
	}
	dfs->dfs_event_log_count = 0;
	dfs->dfs_phyerr_count = 0;
	dfs->dfs_phyerr_reject_count = 0;
	dfs->dfs_phyerr_queued_count = 0;
	dfs->dfs_phyerr_freq_min = 0x7fffffff;
	dfs->dfs_phyerr_freq_max = 0;
}

/**
 * dfs_get_durmargin() - Find duration margin
 * @rf: Pointer to dfs_filter structure.
 * @durmargin: Duration margin
 */
static inline void dfs_get_durmargin(struct dfs_filter *rf,
				     uint32_t *durmargin)
{
#define DUR_THRESH 10
#define LOW_MARGIN 4
#define HIGH_MARGIN 6

	if (rf->rf_maxdur < DUR_THRESH)
		*durmargin = LOW_MARGIN;
	else
		*durmargin = HIGH_MARGIN;

#undef DUR_THRESH
#undef LOW_MARGIN
#undef HIGH_MARGIN
}

/**
 * dfs_confirm_radar() - This function checks for fractional PRI and jitter in
 * sidx index to determine if the radar is real or not.
 * @dfs: Pointer to dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @ext_chan_flag: ext chan flags.
 */
static int dfs_confirm_radar(struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		int ext_chan_flag)
{
	int i = 0;
	int index;
	struct dfs_delayline *dl = &rf->rf_dl;
	struct dfs_delayelem *de;
	uint64_t target_ts = 0;
	struct dfs_pulseline *pl;
	int start_index = 0, current_index, next_index;
	unsigned char scores[FRAC_PRI_SCORE_ARRAY_SIZE];
	uint32_t pri_margin;
	uint64_t this_diff_ts;
	uint32_t search_bin;

	unsigned char max_score = 0;
	int max_score_index = 0;

	uint32_t min_searchdur = 0xFFFFFFFF;
	uint32_t max_searchdur = 0x0;
	uint32_t durmargin = 0;
	uint32_t this_dur;
	uint32_t this_deltadur;

	pl = dfs->pulses;

	OS_MEMZERO(scores, sizeof(scores));
	scores[0] = rf->rf_threshold;

	pri_margin = dfs_get_pri_margin(dfs, ext_chan_flag,
			(rf->rf_patterntype == 1));
	dfs_get_durmargin(rf, &durmargin);


	/*
	 * Look for the entry that matches dl_seq_num_second.
	 * we need the time stamp and diff_ts from there.
	 */

	for (i = 0; i < dl->dl_numelems; i++) {
		index = (dl->dl_firstelem + i) & DFS_MAX_DL_MASK;
		de = &dl->dl_elems[index];
		if (dl->dl_seq_num_second == de->de_seq_num)
			target_ts = de->de_ts - de->de_time;

		if (de->de_dur < min_searchdur)
			min_searchdur = de->de_dur;

		if (de->de_dur > max_searchdur)
			max_searchdur = de->de_dur;
	}

	if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS2) {
		dfs_print_delayline(dfs, &rf->rf_dl);

		/* print pulse line */
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
			"%s: Pulse Line\n", __func__);
		for (i = 0; i < pl->pl_numelems; i++) {
			index =  (pl->pl_firstelem + i) &
				DFS_MAX_PULSE_BUFFER_MASK;
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"Elem %u: ts=%llu dur=%u, seq_num=%d, delta_peak=%d, psidx_diff=%d\n",
					i, pl->pl_elems[index].p_time,
					pl->pl_elems[index].p_dur,
					pl->pl_elems[index].p_seq_num,
					pl->pl_elems[index].p_delta_peak,
					pl->pl_elems[index].p_psidx_diff);
		}
	}

	/*
	 * Walk through the pulse line and find pulse with target_ts.
	 * Then continue until we find entry with seq_number dl_seq_num_stop.
	 */

	for (i = 0; i < pl->pl_numelems; i++) {
		index =  (pl->pl_firstelem + i) & DFS_MAX_PULSE_BUFFER_MASK;
		if (pl->pl_elems[index].p_time == target_ts) {
			dl->dl_seq_num_start = pl->pl_elems[index].p_seq_num;
			start_index = index; /* save for future use */
		}
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS2,
			"%s: target_ts=%llu, dl_seq_num_start=%d, dl_seq_num_second=%d, dl_seq_num_stop=%d\n",
			__func__, target_ts, dl->dl_seq_num_start,
			dl->dl_seq_num_second, dl->dl_seq_num_stop);

	current_index = start_index;
	while (pl->pl_elems[current_index].p_seq_num < dl->dl_seq_num_stop) {
		next_index = (current_index + 1) & DFS_MAX_PULSE_BUFFER_MASK;
		this_diff_ts = pl->pl_elems[next_index].p_time -
			pl->pl_elems[current_index].p_time;

		this_dur =  pl->pl_elems[next_index].p_dur;
		this_deltadur = DFS_MIN(DFS_DIFF(this_dur, min_searchdur),
					DFS_DIFF(this_dur, max_searchdur));

		/* Now update the score for this diff_ts */
		for (i = 1; i < FRAC_PRI_SCORE_ARRAY_SIZE; i++) {
			search_bin = dl->dl_search_pri / (i + 1);

			/*
			 * We do not give score to PRI that is lower then the
			 * limit.
			 */
			if (search_bin < DFS_INVALID_PRI_LIMIT)
				break;

			/*
			 * Increment the score if this_diff_ts belongs to this
			 * search_bin +/- margin.
			 */
			if ((this_diff_ts >= (search_bin - pri_margin)) &&
				(this_diff_ts <= (search_bin + pri_margin)) &&
				(this_deltadur < durmargin)) {
				/*increment score */
				scores[i]++;
			}
		}
		current_index = next_index;
	}

	for (i = 0; i < FRAC_PRI_SCORE_ARRAY_SIZE; i++)
		if (scores[i] > max_score) {
			max_score = scores[i];
			max_score_index = i;
		}

	if (max_score_index != 0) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "Rejecting Radar since Fractional PRI detected: searchpri=%d, threshold=%d, fractional PRI=%d, Fractional PRI score=%d",
			  dl->dl_search_pri, scores[0],
			  dl->dl_search_pri/(max_score_index + 1),
			  max_score);
		return 0;
	}


	/* Check for frequency spread */
	if (dl->dl_min_sidx > pl->pl_elems[start_index].p_sidx)
		dl->dl_min_sidx = pl->pl_elems[start_index].p_sidx;

	if (dl->dl_max_sidx < pl->pl_elems[start_index].p_sidx)
		dl->dl_max_sidx = pl->pl_elems[start_index].p_sidx;

	if ((dl->dl_max_sidx - dl->dl_min_sidx) > rf->rf_sidx_spread) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "Rejecting Radar since frequency spread is too large : min_sidx=%d, max_sidx=%d, rf_sidx_spread=%d",
			  dl->dl_min_sidx, dl->dl_max_sidx,
			  rf->rf_sidx_spread);
		return 0;
	}

	if ((rf->rf_check_delta_peak) &&
			((dl->dl_delta_peak_match_count +
			dl->dl_psidx_diff_match_count - 1) <
			rf->rf_threshold)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "Rejecting Radar since delta peak values are invalid : dl_delta_peak_match_count=%d, dl_psidx_diff_match_count=%d, rf_threshold=%d",
			  dl->dl_delta_peak_match_count,
			  dl->dl_psidx_diff_match_count,
			  rf->rf_threshold);
		return 0;
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS_FALSE_DET, "%s : dl->dl_min_sidx: %d , dl->dl_max_sidx :%d",
		  __func__, dl->dl_min_sidx, dl->dl_max_sidx);

	dfs->dfs_freq_offset = DFS_SIDX_TO_FREQ_OFFSET((dl->dl_min_sidx +
							dl->dl_max_sidx) / 2);
	return 1;
}

/*
 * dfs_reject_on_pri() - Rejecting on individual filter based on min PRI .
 * @dfs: Pointer to wlan_dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @deltaT: deltaT value.
 * @this_ts: Timestamp.
 */
static inline bool dfs_reject_on_pri(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint64_t deltaT,
		uint64_t this_ts)
{
	if ((deltaT < rf->rf_minpri) && (deltaT != 0)) {
		/* Second line of PRI filtering. */
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"filterID %d : Rejecting on individual filter min PRI deltaT=%lld rf->rf_minpri=%u",
				rf->rf_pulseid, (uint64_t)deltaT,
				rf->rf_minpri);
		return 1;
	}

	if (rf->rf_ignore_pri_window > 0) {
		if (deltaT < rf->rf_minpri) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"filterID %d : Rejecting on individual filter max PRI deltaT=%lld rf->rf_minpri=%u",
					rf->rf_pulseid, (uint64_t)deltaT,
					rf->rf_minpri);
			/* But update the last time stamp. */
			rf->rf_dl.dl_last_ts = this_ts;
			return 1;
		}
	} else {
		/*
		 * The HW may miss some pulses especially with
		 * high channel loading. This is true for Japan
		 * W53 where channel loaoding is 50%. Also for
		 * ETSI where channel loading is 30% this can
		 * be an issue too. To take care of missing
		 * pulses, we introduce pri_margin multiplie.
		 * This is normally 2 but can be higher for W53.
		 */

		if ((deltaT > (dfs->dfs_pri_multiplier * rf->rf_maxpri)) ||
				(deltaT < rf->rf_minpri)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"filterID %d : Rejecting on individual filter max PRI deltaT=%lld rf->rf_minpri=%u",
					rf->rf_pulseid, (uint64_t) deltaT,
					rf->rf_minpri);
			/* But update the last time stamp. */
			rf->rf_dl.dl_last_ts = this_ts;
			return 1;
		}
	}

	return 0;
}

/**
 * dfs_confirm_radar_check() - Do additioal check to conirm radar except for
 * the staggered, chirp FCC Bin 5, frequency hopping indicated by
 * rf_patterntype == 1.
 * @dfs: Pointer to wlan_dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @ext_chan_event_flag: Extension channel event flag
 * @found: Pointer to radar found flag (return value).
 * @false_radar_found: Pointer to false radar found (return value).
 */

static inline void dfs_confirm_radar_check(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		int ext_chan_event_flag,
		int *found,
		int *false_radar_found)
{
	if (rf->rf_patterntype != 1) {
		*found = dfs_confirm_radar(dfs, rf, ext_chan_event_flag);
		*false_radar_found = (*found == 1) ? 0 : 1;
	}
}

void __dfs_process_radarevent(struct wlan_dfs *dfs,
		struct dfs_filtertype *ft,
		struct dfs_event *re,
		uint64_t this_ts,
		int *found,
		int *false_radar_found)
{
	int p;
	uint64_t deltaT = 0;
	int ext_chan_event_flag = 0;
	struct dfs_filter *rf = NULL;
	int8_t ori_rf_check_delta_peak = 0;

	for (p = 0, *found = 0; (p < ft->ft_numfilters) &&
			(!(*found)) && !(*false_radar_found); p++) {
		rf = ft->ft_filters[p];
		if ((re->re_dur >= rf->rf_mindur) &&
				(re->re_dur <= rf->rf_maxdur)) {
			/* The above check is probably not necessary. */
			deltaT = (this_ts < rf->rf_dl.dl_last_ts) ?
			    (int64_t)((DFS_TSF_WRAP - rf->rf_dl.dl_last_ts) +
				    this_ts + 1) :
			    this_ts - rf->rf_dl.dl_last_ts;

			if (dfs_reject_on_pri(dfs, rf, deltaT, this_ts))
				continue;

			dfs_add_pulse(dfs, rf, re, deltaT, this_ts);

			/*
			 * If this is an extension channel event, flag it for
			 * false alarm reduction.
			 */
			if (re->re_chanindex == dfs->dfs_extchan_radindex)
				ext_chan_event_flag = 1;

			if (rf->rf_patterntype == 2) {
				*found = dfs_staggered_check(dfs, rf,
					(uint32_t) deltaT, re->re_dur);
			} else {
				*found = dfs_bin_check(dfs, rf,
					(uint32_t) deltaT, re->re_dur,
					ext_chan_event_flag);

				if (*found &&
				    (utils_get_dfsdomain(dfs->dfs_pdev_obj) !=
				     DFS_CN_DOMAIN)) {
					ori_rf_check_delta_peak =
						rf->rf_check_delta_peak;
					/*
					 * If FW does not send valid psidx_diff
					 * Do not do chirp check.
					 */
					if (rf->rf_check_delta_peak &&
						(!(re->re_flags &
						DFS_EVENT_VALID_PSIDX_DIFF)))
						rf->rf_check_delta_peak = false;
					dfs_confirm_radar_check(dfs,
							rf, ext_chan_event_flag,
							found,
							false_radar_found);
					rf->rf_check_delta_peak =
						ori_rf_check_delta_peak;
				}
			}

			if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS2)
				if (rf->rf_patterntype !=
						WLAN_DFS_RF_PATTERN_TYPE_1)
					dfs_print_delayline(dfs, &rf->rf_dl);

			rf->rf_dl.dl_last_ts = this_ts;
		}
	}

	if (*found) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Found on channel minDur = %d, filterId = %d",
				ft->ft_mindur,
				rf ?  rf->rf_pulseid : -1);
	}

	return;
}

/**
 * dfs_cal_average_radar_parameters() - Calculate the average radar parameters.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
static void dfs_cal_average_radar_parameters(struct wlan_dfs *dfs)
{
	int i, count = 0;
	u_int32_t total_pri = 0;
	u_int32_t total_duration = 0;
	u_int32_t total_sidx = 0;

	/* Calculating average PRI, Duration, SIDX from
	 * the 2nd pulse, ignoring the 1st pulse (radar_log[0]).
	 * This is because for the first pulse, the diff_ts will be
	 * (0 - current_ts) which will be a huge value.
	 * Average PRI computation will be wrong. FW returns a
	 * failure test result as PRI does not match their expected
	 * value.
	 */

	for (i = 1; (i < DFS_EVENT_LOG_SIZE) && (i < dfs->dfs_event_log_count);
			i++) {
		total_pri +=  dfs->radar_log[i].diff_ts;
		total_duration += dfs->radar_log[i].dur;
		total_sidx +=  dfs->radar_log[i].sidx;
		count++;
	}

	if (count > 0) {
		dfs->dfs_average_pri = total_pri / count;
		dfs->dfs_average_duration = total_duration / count;
		dfs->dfs_average_sidx = total_sidx / count;

		dfs_info(dfs, WLAN_DEBUG_DFS2,
			 "Avg.PRI =%u, Avg.duration =%u Avg.sidx =%u",
			 dfs->dfs_average_pri,
			 dfs->dfs_average_duration,
			 dfs->dfs_average_sidx);
	}
}
#else
static void dfs_cal_average_radar_parameters(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_radarfound_reset_vars() - Reset dfs variables after radar found
 * @dfs: Pointer to wlan_dfs structure.
 * @rs: Pointer to dfs_state.
 * @chan: Current  channel.
 * @seg_id: Segment id.
 */
static inline void dfs_radarfound_reset_vars(
		struct wlan_dfs *dfs,
		struct dfs_state *rs,
		struct dfs_channel *chan,
		uint8_t seg_id)
{
	struct dfs_channel *thischan;

	/*
	 * TODO: Instead of discarding the radar, create a workqueue
	 * if the channel change is happenning through userspace and
	 * process the radar event once the channel change is completed.
	 */

	/* Collect stats */
	dfs->wlan_dfs_stats.num_radar_detects++;
	thischan = &rs->rs_chan;
	if ((seg_id == SEG_ID_SECONDARY) &&
			(dfs_is_precac_timer_running(dfs)))
		dfs->is_radar_during_precac = 1;

	/*
	 * If event log is on then dump the radar event queue on
	 * filter match. This can be used to collect information
	 * on false radar detection.
	 */
	if (dfs->dfs_event_log_on) {
		dfs_cal_average_radar_parameters(dfs);
		dfs_print_radar_events(dfs);
	}

	dfs_reset_radarq(dfs);
	dfs_reset_alldelaylines(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"Primary channel freq = %u flags=0x%x",
			chan->dfs_ch_freq, chan->dfs_ch_flagext);

	if (chan->dfs_ch_freq != thischan->dfs_ch_freq)
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
				"Ext channel freq = %u flags=0x%x",
				thischan->dfs_ch_freq,
				thischan->dfs_ch_flagext);

	dfs->dfs_phyerr_freq_min = 0x7fffffff;
	dfs->dfs_phyerr_freq_max = 0;
	dfs->dfs_phyerr_w53_counter = 0;

	if (seg_id == SEG_ID_SECONDARY) {
		dfs->wlan_dfs_stats.num_seg_two_radar_detects++;
		dfs->is_radar_found_on_secondary_seg = 1;
	}
}

/*
 * dfs_print_radar_found_freq() - Print radar found frequency.
 * @dfs: Pointer to wlan_dfs.
 */
#ifdef CONFIG_CHAN_FREQ_API
static void dfs_print_radar_found_freq(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "bangradar on 2nd segment cfreq = %u",
		  dfs->dfs_precac_secondary_freq_mhz);
}
#else
#ifdef CONFIG_CHAN_NUM_API
static void dfs_print_radar_found_freq(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "bangradar on 2nd segment cfreq = %u",
		  dfs->dfs_precac_secondary_freq);
}
#endif
#endif

/**
 * dfs_handle_bangradar - Handle the case of bangradar
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Current channel.
 * @rs: Pointer to dfs_state.
 * Return: if bangradar then  return 1.  Otherwise, return 0.
 */
static inline int dfs_handle_bangradar(
	struct wlan_dfs *dfs,
	struct dfs_channel *chan,
	struct dfs_state **rs,
	uint8_t *seg_id,
	int *retval)
{

	if (dfs->dfs_bangradar_type) {
		if (dfs->dfs_bangradar_type >= DFS_INVALID_BANGRADAR_TYPE) {
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "Invalid bangradar type");
			return 1;
		}
		/* All bangradars are processed similarly.
		 * arguments for the bangradar are already stored in
		 * respective dfs structures.
		 */

		*rs = &dfs->dfs_radar[dfs->dfs_curchan_radindex];
		if (dfs->dfs_seg_id == SEG_ID_SECONDARY) {
			if (dfs_is_precac_timer_running(dfs) ||
			    WLAN_IS_CHAN_11AC_VHT160(chan) ||
			    WLAN_IS_CHAN_11AC_VHT80_80(chan)) {
				dfs->is_radar_found_on_secondary_seg = 1;
				dfs_print_radar_found_freq(dfs);
			} else {
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					  "No second segment");
				return 1;
			}
		}
		*seg_id = dfs->dfs_seg_id;
		dfs_debug(dfs, WLAN_DEBUG_DFS, "bangradar %d",
			  dfs->dfs_bangradar_type);
		*retval = 1;
		return 1;
	}
	return 0;
}

/**
 * dfs_process_w53_pulses() - Prrocess w53 pulses
 * @dfs: Pointer to wlan_dfs structure.
 *
 * For chips that support frequency information, we can relax PRI
 * restriction if the frequency spread is narrow.
 */
static inline void dfs_process_w53_pulses(
		struct wlan_dfs *dfs)
{
	if ((dfs->dfs_phyerr_freq_max - dfs->dfs_phyerr_freq_min) <
			DFS_MAX_FREQ_SPREAD)
		dfs->dfs_pri_multiplier = DFS_LARGE_PRI_MULTIPLIER;

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"w53_counter=%d, freq_max=%d, freq_min=%d, pri_multiplier=%d",
			 dfs->dfs_phyerr_w53_counter,
			dfs->dfs_phyerr_freq_max, dfs->dfs_phyerr_freq_min,
			dfs->dfs_pri_multiplier);

	dfs->dfs_phyerr_freq_min = 0x7fffffff;
	dfs->dfs_phyerr_freq_max = 0;
}

/**
 * dfs_handle_missing_pulses - Handle the case of missing pulses
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Current channel.
 *
 * The HW may miss some pulses especially with high channel loading.
 * This is true for Japan W53 where channel loaoding is 50%. Also
 * for ETSI where channel loading is 30% this can be an issue too.
 * To take care of missing pulses, we introduce pri_margin multiplie.
 * This is normally 2 but can be higher for W53.
 * Return: If not enough pulses return 0.  Otherwise, return 1.
 */
static inline int dfs_handle_missing_pulses(
		struct wlan_dfs *dfs,
		struct dfs_channel *chan)
{
	if ((dfs->dfsdomain  == DFS_MKK4_DOMAIN) &&
			(dfs->dfs_caps.wlan_chip_is_bb_tlv) &&
			(chan->dfs_ch_freq < FREQ_5500_MHZ)) {
		dfs->dfs_pri_multiplier = DFS_W53_DEFAULT_PRI_MULTIPLIER;
		/*
		 * Do not process W53 pulses unless we have a minimum number
		 * of them.
		 */
		if (dfs->dfs_phyerr_w53_counter >= 5)
			dfs_process_w53_pulses(dfs);
		else
			return 0;
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS1, "pri_multiplier=%d",
			 dfs->dfs_pri_multiplier);

	return 1;
}

/**
 * dfs_is_radarq_empty - check if radarq is empty
 * @dfs: Pointer to wlan_dfs structure.
 * @empty: Pointer to empty
 */
static inline void dfs_is_radarq_empty(
		struct wlan_dfs *dfs,
		int *empty)
{
	WLAN_DFSQ_LOCK(dfs);
	*empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
	WLAN_DFSQ_UNLOCK(dfs);
}

/**
 * dfs_remove_event_from_radarq - remove event from radarq
 * @dfs: Pointer to wlan_dfs structure.
 * @event: Double pointer to the event structure
 */
static inline void dfs_remove_event_from_radarq(
		struct wlan_dfs *dfs,
		struct dfs_event **event)
{
	WLAN_DFSQ_LOCK(dfs);
	*event = STAILQ_FIRST(&(dfs->dfs_radarq));
	if (*event)
		STAILQ_REMOVE_HEAD(&(dfs->dfs_radarq), re_list);
	WLAN_DFSQ_UNLOCK(dfs);
}

/**
 * dfs_return_event_to_eventq - return event to eventq
 * @dfs: Pointer to wlan_dfs structure.
 * @event: Pointer to the event structure
 */
static inline void dfs_return_event_to_eventq(
		struct wlan_dfs *dfs,
		struct dfs_event *event)
{
	qdf_mem_zero(event, sizeof(struct dfs_event));
	WLAN_DFSEVENTQ_LOCK(dfs);
	STAILQ_INSERT_TAIL(&(dfs->dfs_eventq), event, re_list);
	WLAN_DFSEVENTQ_UNLOCK(dfs);
}

/**
 * dfs_log_event - log dfs event
 * @dfs: Pointer to wlan_dfs structure.
 * @re:  Pointer to dfs_event re
 * @this_ts: Current time stamp 64bit
 * @diff_ts: Difference between 2 timestamps 32bit
 * @index: Index value.
 */
static inline void dfs_log_event(
		struct wlan_dfs *dfs,
		struct dfs_event *re,
		uint64_t this_ts,
		uint32_t diff_ts,
		uint32_t index)
{
	uint8_t i;
	struct dfs_pulseline *pl = dfs->pulses;

	if (dfs->dfs_event_log_on) {
		i = dfs->dfs_event_log_count % DFS_EVENT_LOG_SIZE;
		dfs->radar_log[i].ts = this_ts;
		dfs->radar_log[i].diff_ts = diff_ts;
		dfs->radar_log[i].rssi = (*re).re_rssi;
		dfs->radar_log[i].dur = (*re).re_dur;
		dfs->radar_log[i].seg_id = (*re).re_seg_id;
		dfs->radar_log[i].sidx = (*re).re_sidx;
		dfs->radar_log[i].freq_offset_khz =
			(*re).re_freq_offset_khz;
		dfs->radar_log[i].peak_mag = (*re).re_peak_mag;
		dfs->radar_log[i].total_gain = (*re).re_total_gain;
		dfs->radar_log[i].mb_gain = (*re).re_mb_gain;
		dfs->radar_log[i].relpwr_db = (*re).re_relpwr_db;
		dfs->radar_log[i].delta_diff = (*re).re_delta_diff;
		dfs->radar_log[i].delta_peak = (*re).re_delta_peak;
		dfs->radar_log[i].psidx_diff = (*re).re_psidx_diff;
		dfs->radar_log[i].is_chirp = DFS_EVENT_NOTCHIRP(re) ?
			0 : 1;
		dfs->dfs_event_log_count++;
	}

	dfs->dfs_seq_num++;
	pl->pl_elems[index].p_seq_num = dfs->dfs_seq_num;
}

/**
 * dfs_check_if_nonbin5 - Check if radar, other than bin5, is found
 * @dfs: Pointer to wlan_dfs structure.
 * @re: Pointer to re (radar event)
 * @rs: Double Pointer to rs (radar state)
 * @this_ts: Current time stamp 64bit
 * @diff_ts: Difference between 2 timestamps 32bit
 * @found: Pointer to found. If radar found or not.
 * @retval: Pointer to retval(return value).
 * @false_radar_found: Pointer to false_radar_found(return value).
 */
static inline void dfs_check_if_nonbin5(
	struct wlan_dfs *dfs,
	struct dfs_event *re,
	struct dfs_state **rs,
	uint64_t this_ts,
	uint32_t diff_ts,
	int *found,
	int *retval,
	int *false_radar_found)
{

	uint32_t tabledepth = 0;
	struct dfs_filtertype *ft;
	uint64_t deltaT;

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"  *** chan freq (%d): ts %llu dur %u rssi %u",
			(*rs)->rs_chan.dfs_ch_freq, (uint64_t)this_ts,
			(*re).re_dur, (*re).re_rssi);

	while ((tabledepth < DFS_MAX_RADAR_OVERLAP) &&
			((dfs->dfs_ftindextable[(*re).re_dur])[tabledepth] !=
			 -1) && (!*retval) && !(*false_radar_found)) {
		ft = dfs->dfs_radarf[((dfs->dfs_ftindextable[(*re).re_dur])
				[tabledepth])];
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"  ** RD (%d): ts %x dur %u rssi %u",
				(*rs)->rs_chan.dfs_ch_freq, (*re).re_ts,
				(*re).re_dur, (*re).re_rssi);

		if ((*re).re_rssi < ft->ft_rssithresh &&
				(*re).re_dur > MAX_DUR_FOR_LOW_RSSI) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"Rejecting on rssi rssi=%u thresh=%u",
					 (*re).re_rssi,
					ft->ft_rssithresh);
			tabledepth++;
			continue;
		}
		deltaT = this_ts - ft->ft_last_ts;
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"deltaT = %lld (ts: 0x%llx) (last ts: 0x%llx)",
				(uint64_t)deltaT, (uint64_t)this_ts,
				(uint64_t)ft->ft_last_ts);

		if ((deltaT < ft->ft_minpri) && (deltaT != 0)) {
			/*
			 * This check is for the whole filter type.
			 * Individual filters will check this again.
			 * This is first line of filtering.
			 */
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"Rejecting on pri pri=%lld minpri=%u",
					 (uint64_t)deltaT, ft->ft_minpri);
			tabledepth++;
			continue;
		}

		__dfs_process_radarevent(dfs, ft, re, this_ts, found,
				false_radar_found);

		ft->ft_last_ts = this_ts;
		*retval |= *found;
		tabledepth++;
	}
}

/**
 * dfs_check_each_b5radar() - Check each bin5 radar
 * @dfs: Pointer to wlan_dfs structure.
 * @re:  Pointer to re(radar event).
 * @br: Pointer to dfs_bin5radars structure.
 * @this_ts: Current time stamp 64bit.
 * @diff_ts: Difference between 2 timestamps 32bit.
 * @found: Pointer to found. If radar found or not.
 */
static inline void dfs_check_each_b5radar(
		struct wlan_dfs *dfs,
		struct dfs_event *re,
		struct dfs_bin5radars *br,
		uint64_t this_ts,
		uint32_t diff_ts,
		int *found)
{
	if (dfs_bin5_check_pulse(dfs, re, br)) {
		/*
		 * This is a valid Bin5 pulse, check if it belongs to a
		 * burst.
		 */
		(*re).re_dur = dfs_retain_bin5_burst_pattern(dfs, diff_ts,
				(*re).re_dur);
		/*
		 * Remember our computed duration for the next pulse in the
		 * burst (if needed).
		 */
		dfs->dfs_rinfo.dfs_bin5_chirp_ts = this_ts;
		dfs->dfs_rinfo.dfs_last_bin5_dur = (*re).re_dur;

		if (dfs_bin5_addpulse(dfs, br, re, this_ts))
			*found |= dfs_bin5_check(dfs);
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_PULSE,
				"not a BIN5 pulse (dur=%d)", (*re).re_dur);
	}
}

/**
 * dfs_check_if_bin5() - Check if bin5 radar is found
 * @dfs: Pointer to wlan_dfs structure.
 * @re:  Pointer to re(radar event).
 * @this_ts: Current time stamp 64bit.
 * @diff_ts: Difference between 2 timestamps 32bit.
 * @found: Pointer to found. If radar found or not.
 */
static inline void dfs_check_if_bin5(
	struct wlan_dfs *dfs,
	struct dfs_event *re,
	uint64_t this_ts,
	uint32_t diff_ts,
	int *found)
{
	int p;

	/* BIN5 pulses are FCC and Japan specific. */
	if ((dfs->dfsdomain == DFS_FCC_DOMAIN) ||
			(dfs->dfsdomain == DFS_MKK4_DOMAIN)) {
		for (p = 0; (p < dfs->dfs_rinfo.rn_numbin5radars) && (!*found);
				p++) {
			struct dfs_bin5radars *br;

			br = &(dfs->dfs_b5radars[p]);
			dfs_check_each_b5radar(dfs, re, br, this_ts, diff_ts,
					found);
		}
	}

	if (*found)
		dfs_debug(dfs, WLAN_DEBUG_DFS, "Found bin5 radar");
}

/**
 * dfs_skip_the_event() - Skip the Radar event
 * @dfs: Pointer to wlan_dfs structure.
 * @re: Pointer to re(radar event).
 * @rs: Pointer to dfs_state.
 */
static inline bool dfs_skip_the_event(
	struct wlan_dfs *dfs,
	struct dfs_event *re,
	struct dfs_state **rs)
{
	if ((*re).re_chanindex < DFS_NUM_RADAR_STATES)
		(*rs) = &dfs->dfs_radar[(*re).re_chanindex];
	else
		return 1;

	if ((*rs)->rs_chan.dfs_ch_flagext & CHANNEL_INTERFERENCE)
		return 1;

	return 0;
}

/**
 * dfs_check_ts_wrap() - dfs check for timestamp wrap.
 * @dfs: Pointer to wlan_dfs structure.
 * @re: Pointer to re(radar event).
 * @deltafull_ts: Deltafull ts.
 *
 * Return: Deltafull ts.
 */
static inline uint64_t dfs_check_ts_wrap(
		struct wlan_dfs *dfs,
		struct dfs_event *re,
		uint64_t deltafull_ts)
{
	if (deltafull_ts >
			((uint64_t)((DFS_TSMASK -
					dfs->dfs_rinfo.rn_last_ts) +
				1 + (*re).re_ts)))
		deltafull_ts -=
			(DFS_TSMASK - dfs->dfs_rinfo.rn_last_ts) +
			1 + (*re).re_ts;

	return deltafull_ts;
}

/**
 * dfs_calculate_ts_prefix() - Calculate deltafull ts value.
 * @dfs: Pointer to wlan_dfs structure.
 * @re: Pointer to re(radar event).
 */
static inline void dfs_calculate_ts_prefix(
		struct wlan_dfs *dfs,
		struct dfs_event *re)
{
	uint64_t deltafull_ts;

	if ((*re).re_ts <= dfs->dfs_rinfo.rn_last_ts) {
		dfs->dfs_rinfo.rn_ts_prefix += (((uint64_t) 1) << DFS_TSSHIFT);
		/* Now, see if it's been more than 1 wrap */
		deltafull_ts = (*re).re_full_ts - dfs->dfs_rinfo.rn_lastfull_ts;
		deltafull_ts = dfs_check_ts_wrap(dfs, re, deltafull_ts);
		deltafull_ts >>= DFS_TSSHIFT;

		if (deltafull_ts > 1)
			dfs->dfs_rinfo.rn_ts_prefix +=
				((deltafull_ts - 1) << DFS_TSSHIFT);
	} else {
		deltafull_ts = (*re).re_full_ts -
			dfs->dfs_rinfo.rn_lastfull_ts;
		if (deltafull_ts > (uint64_t) DFS_TSMASK) {
			deltafull_ts >>= DFS_TSSHIFT;
			dfs->dfs_rinfo.rn_ts_prefix +=
				((deltafull_ts - 1) << DFS_TSSHIFT);
		}
	}
}

/**
 * dfs_calculate_timestamps() - Calculate various timestamps
 * @dfs: Pointer to wlan_dfs structure.
 * @re: Pointer to re(radar event)
 * @this_ts : Pointer to  this_ts (this timestamp)
 */

static inline void  dfs_calculate_timestamps(
	struct wlan_dfs *dfs,
	struct dfs_event *re,
	uint64_t *this_ts)
{
	if (dfs->dfs_rinfo.rn_lastfull_ts == 0) {
		/*
		 * Either not started, or 64-bit rollover exactly to
		 * zero Just prepend zeros to the 15-bit ts.
		 */
		dfs->dfs_rinfo.rn_ts_prefix = 0;
	} else {
		/* WAR 23031- patch duplicate ts on very short pulses.
		 * This pacth has two problems in linux environment.
		 * 1)The time stamp created and hence PRI depends
		 * entirely on the latency. If the latency is high, it
		 * possibly can split two consecutive pulses in the
		 * same burst so far away (the same amount of latency)
		 * that make them look like they are from differenct
		 * bursts. It is observed to happen too often. It sure
		 * makes the detection fail.
		 * 2)Even if the latency is not that bad, it simply
		 * shifts the duplicate timestamps to a new duplicate
		 * timestamp based on how they are processed.
		 * This is not worse but not good either.
		 * Take this pulse as a good one and create a probable
		 * PRI later.
		 */
		if ((*re).re_dur == 0 && (*re).re_ts ==
				dfs->dfs_rinfo.rn_last_unique_ts) {
			debug_dup[debug_dup_cnt++] = '1';
			dfs_debug(dfs, WLAN_DEBUG_DFS1, "deltaT is 0");
		} else {
			dfs->dfs_rinfo.rn_last_unique_ts = (*re).re_ts;
			debug_dup[debug_dup_cnt++] = '0';
		}

		if (debug_dup_cnt >= 32)
			debug_dup_cnt = 0;

		dfs_calculate_ts_prefix(dfs, re);
	}

	/*
	 * At this stage rn_ts_prefix has either been blanked or
	 * calculated, so it's safe to use.
	 */
	*this_ts = dfs->dfs_rinfo.rn_ts_prefix | ((uint64_t) (*re).re_ts);
	dfs->dfs_rinfo.rn_lastfull_ts = (*re).re_full_ts;
	dfs->dfs_rinfo.rn_last_ts = (*re).re_ts;
}

/**
 * dfs_add_to_pulseline - Extract necessary items from dfs_event and
 * add it as pulse in the pulseline
 * @dfs: Pointer to wlan_dfs structure.
 * @re:  Pointer to re(radar event)
 * @this_ts: Pointer to  this_ts (this timestamp)
 * @diff_ts: Diff ts.
 * @index: Pointer to get index value.
 */
static inline void dfs_add_to_pulseline(
	struct wlan_dfs *dfs,
	struct dfs_event *re,
	uint64_t *this_ts,
	uint32_t *test_ts,
	uint32_t *diff_ts,
	uint32_t *index)
{
	struct dfs_pulseline *pl;

	/*
	 * Calculate the start of the radar pulse.
	 *
	 * The TSF is stamped by the MAC upon reception of the event,
	 * which is (typically?) at the end of the event. But the
	 * pattern matching code expects the event timestamps to be at
	 * the start of the event. So to fake it, we subtract the pulse
	 * duration from the given TSF. This is done after the 64-bit
	 * timestamp has been calculated so long pulses correctly
	 * under-wrap the counter.  Ie, if this was done on the 32
	 * (or 15!) bit TSF when the TSF value is closed to 0, it will
	 * underflow to 0xfffffXX, which would mess up the logical "OR"
	 * operation done above.
	 * This isn't valid for Peregrine as the hardware gives us the
	 * actual TSF offset of the radar event, not just the MAC TSF
	 * of the completed receive.
	 *
	 * XXX TODO: ensure that the TLV PHY error processing code will
	 * correctly calculate the TSF to be the start of the radar
	 * pulse.
	 *
	 * XXX TODO TODO: modify the TLV parsing code to subtract the
	 * duration from the TSF, based on the current fast clock value.
	 */
	if ((!dfs->dfs_caps.wlan_chip_is_bb_tlv) && (*re).re_dur != 1)
		*this_ts -= (*re).re_dur;

	pl = dfs->pulses;
	/* Save the pulse parameters in the pulse buffer(pulse line). */
	*index = (pl->pl_lastelem + 1) & DFS_MAX_PULSE_BUFFER_MASK;

	if (pl->pl_numelems == DFS_MAX_PULSE_BUFFER_SIZE)
		pl->pl_firstelem = (pl->pl_firstelem+1) &
			DFS_MAX_PULSE_BUFFER_MASK;
	else
		pl->pl_numelems++;

	pl->pl_lastelem = *index;
	pl->pl_elems[*index].p_time = *this_ts;
	pl->pl_elems[*index].p_dur = (*re).re_dur;
	pl->pl_elems[*index].p_rssi = (*re).re_rssi;
	pl->pl_elems[*index].p_sidx = (*re).re_sidx;
	pl->pl_elems[*index].p_delta_peak = (*re).re_delta_peak;
	pl->pl_elems[*index].p_psidx_diff = (*re).re_psidx_diff;
	*diff_ts = (uint32_t)*this_ts - *test_ts;
	*test_ts = (uint32_t)*this_ts;

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"ts%u %u %u diff %u pl->pl_lastelem.p_time=%llu",
			(uint32_t)*this_ts, (*re).re_dur,
			(*re).re_rssi, *diff_ts,
			(uint64_t)pl->pl_elems[*index].p_time);
}

/**
 * dfs_conditional_clear_delaylines - Clear delay lines to remove  the
 * false pulses.
 * @dfs: Pointer to wlan_dfs structure.
 * @diff_ts: diff between timerstamps.
 * @this_ts: this timestamp value.
 * @re: Pointer to dfs_event structure.
 */
static inline void dfs_conditional_clear_delaylines(
	struct wlan_dfs *dfs,
	uint32_t diff_ts,
	uint64_t this_ts,
	struct dfs_event re)
{
	struct dfs_pulseline *pl = dfs->pulses;
	uint32_t index;

	/* If diff_ts is very small, we might be getting false pulse
	 * detects due to heavy interference. We might be getting
	 * spectral splatter from adjacent channel. In order to prevent
	 * false alarms we clear the delay-lines. This might impact
	 * positive detections under harsh environments, but helps with
	 * false detects.
	 */

	if (diff_ts < DFS_INVALID_PRI_LIMIT) {
		dfs->dfs_seq_num = 0;
		dfs_reset_alldelaylines(dfs);
		dfs_reset_radarq(dfs);

		index = (pl->pl_lastelem + 1) & DFS_MAX_PULSE_BUFFER_MASK;
		if (pl->pl_numelems == DFS_MAX_PULSE_BUFFER_SIZE)
			pl->pl_firstelem = (pl->pl_firstelem+1) &
				DFS_MAX_PULSE_BUFFER_MASK;
		else
			pl->pl_numelems++;

		pl->pl_lastelem = index;
		pl->pl_elems[index].p_time = this_ts;
		pl->pl_elems[index].p_dur = re.re_dur;
		pl->pl_elems[index].p_rssi = re.re_rssi;
		pl->pl_elems[index].p_sidx = re.re_sidx;
		pl->pl_elems[index].p_delta_peak = re.re_delta_peak;
		pl->pl_elems[index].p_psidx_diff = re.re_psidx_diff;
		dfs->dfs_seq_num++;
		pl->pl_elems[index].p_seq_num = dfs->dfs_seq_num;
	}
}

/**
 * dfs_process_each_radarevent - remove each event from the dfs radar queue
 * and process it.
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Pointer to DFS current channel.
 * @rs: Pointer to dfs_state structure.
 * @seg_id: segment id.
 * @retval: pointer to retval.
 * @false_radar_found: pointer to false radar found.
 *
 * Return: If radar found then return 1 else return 0.
 */
static inline int dfs_process_each_radarevent(
	struct wlan_dfs *dfs,
	struct dfs_channel *chan,
	struct dfs_state **rs,
	uint8_t *seg_id,
	int *retval,
	int *false_radar_found)
{
	struct dfs_event re, *event;
	int found, empty;
	int events_processed = 0;
	uint64_t this_ts;
	static uint32_t test_ts;
	static uint32_t diff_ts;
	uint32_t index;

	dfs_is_radarq_empty(dfs, &empty);

	while ((!empty) && (!*retval) && !(*false_radar_found) &&
			(events_processed < MAX_EVENTS)) {
		dfs_remove_event_from_radarq(dfs, &event);
		if (!event) {
			empty = 1;
			break;
		}
		events_processed++;
		re = *event;

		dfs_return_event_to_eventq(dfs, event);

		*seg_id = re.re_seg_id;
		found = 0;
		if (dfs_skip_the_event(dfs, &re, rs)) {
			dfs_is_radarq_empty(dfs, &empty);
			continue;
		}

		dfs_calculate_timestamps(dfs, &re, &this_ts);

		re.re_dur = dfs_process_pulse_dur(dfs, re.re_dur);

		dfs_add_to_pulseline(dfs, &re, &this_ts, &test_ts, &diff_ts,
				&index);

		dfs_log_event(dfs, &re, this_ts, diff_ts, index);

		dfs_conditional_clear_delaylines(dfs, diff_ts, this_ts, re);

		found = 0;
		if (events_processed == 1) {
			dfs->dfs_min_sidx = (re).re_sidx;
			dfs->dfs_max_sidx = (re).re_sidx;
		}

		dfs_check_if_bin5(dfs, &re, this_ts, diff_ts, &found);
		if (found) {
			*retval |= found;
			dfs->dfs_freq_offset = DFS_SIDX_TO_FREQ_OFFSET(
				   (dfs->dfs_min_sidx + dfs->dfs_max_sidx) / 2);
			return 1;
		}

		dfs_check_if_nonbin5(dfs, &re, rs, this_ts, diff_ts, &found,
				retval, false_radar_found);

		dfs_is_radarq_empty(dfs, &empty);
	}

	return 0;
}

/**
 * dfs_false_radarfound_reset_vars () - Reset dfs variables after false radar
 *                                      found.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_false_radarfound_reset_vars(
	struct wlan_dfs *dfs)
{
	dfs->dfs_seq_num = 0;
	dfs_reset_radarq(dfs);
	dfs_reset_alldelaylines(dfs);
	dfs->dfs_phyerr_freq_min     = 0x7fffffff;
	dfs->dfs_phyerr_freq_max     = 0;
	dfs->dfs_phyerr_w53_counter  = 0;
	dfs->dfs_event_log_count = 0;
	dfs->dfs_phyerr_count = 0;
	dfs->dfs_phyerr_reject_count = 0;
	dfs->dfs_phyerr_queued_count = 0;
}

void dfs_radarfound_action_generic(struct wlan_dfs *dfs, uint8_t seg_id)
{
	struct radar_found_info *radar_found;

	radar_found = qdf_mem_malloc(sizeof(*radar_found));
	if (!radar_found)
		return;

	qdf_mem_zero(radar_found, sizeof(*radar_found));
	radar_found->segment_id = seg_id;
	dfs->dfs_seg_id = seg_id;
	radar_found->pdev_id =
		wlan_objmgr_pdev_get_pdev_id(dfs->dfs_pdev_obj);

	dfs_process_radar_ind(dfs, radar_found);
	qdf_mem_free(radar_found);
}

void dfs_radar_found_action(struct wlan_dfs *dfs,
			    bool bangradar,
			    uint8_t seg_id)
{
	/* If Host DFS confirmation is supported, save the curchan as
	 * radar found chan, send radar found indication along with
	 * average radar parameters to FW and start the host status
	 * wait timer.
	 */
	if (!bangradar &&
	   (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_FCC_DOMAIN) &&
	   lmac_is_host_dfs_check_support_enabled(dfs->dfs_pdev_obj) &&
	   (dfs->dfs_spoof_test_done ? dfs->dfs_use_nol : 1)) {
		dfs_radarfound_action_fcc(dfs, seg_id);
	} else {
		dfs_radarfound_action_generic(dfs, seg_id);
	}
}

void dfs_process_radarevent(
	struct wlan_dfs *dfs,
	struct dfs_channel *chan)
{
	struct dfs_state *rs = NULL;
	uint8_t   seg_id = 0;
	int retval = 0;
	int false_radar_found = 0;
	bool bangradar = false;

	if (!dfs_radarevent_basic_sanity(dfs, chan))
		return;
	/*
	 * TEST : Simulate radar bang, make sure we add the channel to NOL
	 * (bug 29968)
	 */
	if (dfs_handle_bangradar(dfs, chan, &rs, &seg_id, &retval)) {
		if (retval)
			bangradar = true;
		goto dfsfound;
	}

	if (!dfs_handle_missing_pulses(dfs, chan))
		return;

	dfs_process_each_radarevent(dfs, chan, &rs, &seg_id, &retval,
			&false_radar_found);

dfsfound:
	if (retval) {
		dfs_radarfound_reset_vars(dfs, rs, chan, seg_id);
		dfs_radar_found_action(dfs, bangradar, seg_id);
	}

	if (false_radar_found)
		dfs_false_radarfound_reset_vars(dfs);
}
