/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2006, Atheros Communications Inc.
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
 * DOC: This file contains the dfs_attach() and dfs_detach() functions as well
 * as the dfs_control() function which is used to process ioctls related to DFS.
 * For Linux/Mac,  "radartool" is the command line tool that can be used to call
 * various ioctls to set and get radar detection thresholds.
 */

#include "../dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"
#include <dfs_ioctl.h>

/*
 * Channel switch announcement (CSA)
 * usenol=1 (default) make CSA and switch to a new channel on radar detect
 * usenol=0, make CSA with next channel same as current on radar detect
 * usenol=2, no CSA and stay on the same channel on radar detect
 */

static int usenol = 1;

/**
 * dfs_task() - The main function to process the radar pulse.
 *
 * If Radar found, this marks the channel (and the extension channel, if HT40)
 * as having seen a radar event. It marks CHAN_INTERFERENCE and will add it to
 * the local NOL implementation. This is only done for 'usenol=1', as the other
 * two modes don't do radar notification or CAC/CSA/NOL; it just notes there
 * was a radar.
 */
static os_timer_func(dfs_task)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: dfs is NULL\n", __func__);
		return;
	}

	if (dfs_process_radarevent(dfs, dfs->dfs_curchan))
		dfs_radar_found_action(dfs);

	dfs->wlan_radar_tasksched = 0;
}

/**
 * dfs_testtimer_task() - Sends CSA in the current channel.
 *
 * When the user sets usenol to 0 and inject the RADAR, AP does not mark the
 * channel as RADAR and does not add the channel to NOL. It sends the CSA in
 * the current channel.
 */
static os_timer_func(dfs_testtimer_task)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs->wlan_dfstest = 0;

	/*
	 * Flip the channel back to the original channel.
	 * Make sure this is done properly with a CSA.
	 */
	DFS_PRINTK("%s: go back to channel %d\n", __func__,
			dfs->wlan_dfstest_ieeechan);
	dfs_mlme_start_csa(dfs->dfs_pdev_obj, dfs->wlan_dfstest_ieeechan);
}

int dfs_get_debug_info(struct wlan_dfs *dfs, void *data)
{
	if (data)
		*(uint32_t *)data = dfs->dfs_proc_phyerr;

	return (int)dfs->dfs_proc_phyerr;
}

int dfs_create_object(struct wlan_dfs **dfs)
{
	*dfs = (struct wlan_dfs *)qdf_mem_malloc(sizeof(**dfs));
	if (*dfs == NULL) {
		DFS_PRINTK("%s: wlan_dfs allocation failed\n", __func__);
		return 1;
	}

	qdf_mem_zero(*dfs, sizeof(**dfs));

	(*dfs)->dfs_curchan = (struct dfs_ieee80211_channel *)qdf_mem_malloc(
			sizeof(struct dfs_ieee80211_channel));

	if ((*dfs)->dfs_curchan == NULL) {
		DFS_PRINTK("%s: dfs_curchan allocation failed\n", __func__);
		return 1;
	}

	return 0;
}

int dfs_attach(struct wlan_dfs *dfs)
{
	int i, n;
	struct wlan_dfs_radar_tab_info radar_info;
	bool ext_chan, combined_rssi, use_enhancement, strong_signal_diversiry;
	bool chip_is_bb_tlv, chip_is_over_sampled, chip_is_ht160;
	bool chip_is_false_detect;
	uint32_t fastdiv_val;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: dfs is NULL\n", __func__);
		return 0;
	}

	/* If ignore_dfs is set to 1 then Radar detection is disabled. */
	if (dfs->dfs_ignore_dfs) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: ignoring dfs\n", __func__);
		return 0;
	}

	/*
	 * Zero out radar_info. It's possible that the attach function
	 * won't fetch an initial regulatory configuration; you really
	 * do want to ensure that the contents indicates there aren't
	 * any filters.
	 */
	qdf_mem_zero(&radar_info, sizeof(radar_info));

	lmac_get_caps(dfs->dfs_pdev_obj,
		&ext_chan,
		&combined_rssi,
		&use_enhancement,
		&strong_signal_diversiry,
		&chip_is_bb_tlv,
		&chip_is_over_sampled,
		&chip_is_ht160,
		&chip_is_false_detect,
		&fastdiv_val);

	dfs->dfs_caps.wlan_dfs_ext_chan_ok = ext_chan;
	dfs->dfs_caps.wlan_dfs_combined_rssi_ok = combined_rssi;
	dfs->dfs_caps.wlan_dfs_use_enhancement = use_enhancement;
	dfs->dfs_caps.wlan_strong_signal_diversiry = strong_signal_diversiry;
	dfs->dfs_caps.wlan_chip_is_bb_tlv = chip_is_bb_tlv;
	dfs->dfs_caps.wlan_chip_is_over_sampled = chip_is_over_sampled;
	dfs->dfs_caps.wlan_chip_is_ht160 = chip_is_ht160;
	dfs->dfs_caps.wlan_chip_is_false_detect = chip_is_false_detect;
	dfs->dfs_caps.wlan_fastdiv_val = fastdiv_val;

	dfs_clear_stats(dfs);
	dfs->dfs_event_log_on = 1;
	DFS_PRINTK("%s: event log enabled by default\n", __func__);

	/*Verify : Passing NULL to qdf_timer_init().*/
	qdf_timer_init(NULL,
			&(dfs->wlan_dfs_task_timer),
			dfs_task,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
	qdf_timer_init(NULL,
			&(dfs->wlan_dfstesttimer),
			dfs_testtimer_task,
			(void *)dfs,
			QDF_TIMER_TYPE_WAKE_APPS);
	dfs->wlan_dfs_cac_time = WLAN_DFS_WAIT_MS;
	WLAN_DFSQ_LOCK_INIT(dfs);
	STAILQ_INIT(&dfs->dfs_radarq);
	WLAN_ARQ_LOCK_INIT(dfs);
	STAILQ_INIT(&dfs->dfs_arq);
	STAILQ_INIT(&(dfs->dfs_eventq));
	WLAN_DFSEVENTQ_LOCK_INIT(dfs);

	dfs->events = (struct dfs_event *)qdf_mem_malloc(
			sizeof(struct dfs_event)*DFS_MAX_EVENTS);
	if (dfs->events == NULL) {
		qdf_mem_free(dfs);
		DFS_PRINTK("%s: events allocation failed\n", __func__);
		return 1;
	}
	for (i = 0; i < DFS_MAX_EVENTS; i++)
		STAILQ_INSERT_TAIL(&(dfs->dfs_eventq), &dfs->events[i],
				re_list);

	dfs->pulses = (struct dfs_pulseline *)qdf_mem_malloc(
			sizeof(struct dfs_pulseline));
	if (dfs->pulses == NULL) {
		qdf_mem_free(dfs->events);
		dfs->events = NULL;
		DFS_PRINTK("%s: Pulse buffer allocation failed\n", __func__);
		return 1;
	}

	dfs->pulses->pl_lastelem = DFS_MAX_PULSE_BUFFER_MASK;

	/* Allocate memory for radar filters. */
	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		dfs->dfs_radarf[n] = (struct dfs_filtertype *)
			qdf_mem_malloc(sizeof(struct dfs_filtertype));
		if (dfs->dfs_radarf[n] == NULL) {
			DFS_PRINTK(
					"%s: cannot allocate memory for radar filter types\n",
					__func__);
			goto bad1;
		}
		qdf_mem_zero(dfs->dfs_radarf[n], sizeof(struct dfs_filtertype));
	}

	/* Allocate memory for radar table. */
	dfs->dfs_radartable = (int8_t **)qdf_mem_malloc(256*sizeof(int8_t *));
	if (dfs->dfs_radartable == NULL) {
		DFS_PRINTK(
				"%s: Cannot allocate memory for radar table\n",
				__func__);
		goto bad1;
	}
	for (n = 0; n < 256; n++) {
		dfs->dfs_radartable[n] = qdf_mem_malloc(
				DFS_MAX_RADAR_OVERLAP*sizeof(int8_t));
		if (dfs->dfs_radartable[n] == NULL) {
			DFS_PRINTK(
					"%s: cannot allocate memory for radar table entry\n",
					__func__);
			goto bad2;
		}
	}

	if (usenol == 0)
		DFS_PRINTK("%s: NOL disabled\n", __func__);
	else if (usenol == 2)
		DFS_PRINTK("%s: NOL disabled; no CSA\n", __func__);

	dfs->dfs_rinfo.rn_use_nol = usenol;

	/* Init the cached extension channel busy for false alarm reduction */
	dfs->dfs_rinfo.ext_chan_busy_ts = lmac_get_tsf64(dfs->dfs_pdev_obj);
	dfs->dfs_rinfo.dfs_ext_chan_busy = 0;
	/* Init the Bin5 chirping related data */
	dfs->dfs_rinfo.dfs_bin5_chirp_ts = dfs->dfs_rinfo.ext_chan_busy_ts;
	dfs->dfs_rinfo.dfs_last_bin5_dur = MAX_BIN5_DUR;
	dfs->dfs_b5radars = NULL;

	/*
	 * If dfs_init_radar_filters() fails, we can abort here and
	 * reconfigure when the first valid channel + radar config
	 * is available.
	 */
	if (dfs_init_radar_filters(dfs, &radar_info)) {
		DFS_PRINTK(
				"%s: Radar Filter Intialization Failed\n",
				__func__);
		return 1;
	}

	dfs->wlan_dfs_false_rssi_thres = RSSI_POSSIBLY_FALSE;
	dfs->wlan_dfs_peak_mag = SEARCH_FFT_REPORT_PEAK_MAG_THRSH;
	dfs->dfs_phyerr_freq_min     = 0x7fffffff;
	dfs->dfs_phyerr_freq_max     = 0;
	dfs->dfs_phyerr_queued_count = 0;
	dfs->dfs_phyerr_w53_counter  = 0;
	dfs->dfs_pri_multiplier      = 2;
	dfs->wlan_dfs_nol_timeout = DFS_NOL_TIMEOUT_S;

	return 0;

bad2:
	qdf_mem_free(dfs->dfs_radartable);
	dfs->dfs_radartable = NULL;
bad1:
	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		if (dfs->dfs_radarf[n] != NULL) {
			qdf_mem_free(dfs->dfs_radarf[n]);
			dfs->dfs_radarf[n] = NULL;
		}
	}
	if (dfs->pulses) {
		qdf_mem_free(dfs->pulses);
		dfs->pulses = NULL;
	}
	if (dfs->events) {
		qdf_mem_free(dfs->events);
		dfs->events = NULL;
	}

	return 1;
}

void dfs_reset(struct wlan_dfs *dfs)
{
	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
				"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	if (dfs->wlan_radar_tasksched) {
		qdf_timer_stop(&dfs->wlan_dfs_task_timer);
		dfs->wlan_radar_tasksched = 0;
	}

	if (dfs->wlan_dfstest) {
		qdf_timer_stop(&dfs->wlan_dfstesttimer);
		dfs->wlan_dfstest = 0;
	}

	dfs_nol_timer_cleanup(dfs);
	dfs_clear_nolhistory(dfs);
}

void sif_dfs_detach(struct wlan_dfs *dfs)
{
	int n, empty;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: dfs is NULL\n", __func__);
		return;
	}

	if (dfs->dfs_curchan != NULL) {
		OS_FREE(dfs->dfs_curchan);
		dfs->dfs_curchan = NULL;
	}

	if (dfs->wlan_radar_tasksched) {
		qdf_timer_stop(&dfs->wlan_dfs_task_timer);
		dfs->wlan_radar_tasksched = 0;
	}

	if (dfs->wlan_dfstest) {
		qdf_timer_stop(&dfs->wlan_dfstesttimer);
		dfs->wlan_dfstest = 0;
	}

	dfs_reset_radarq(dfs);
	dfs_reset_alldelaylines(dfs);

	if (dfs->pulses != NULL) {
		qdf_mem_free(dfs->pulses);
		dfs->pulses = NULL;
	}

	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		if (dfs->dfs_radarf[n] != NULL) {
			qdf_mem_free(dfs->dfs_radarf[n]);
			dfs->dfs_radarf[n] = NULL;
		}
	}

	if (dfs->dfs_radartable != NULL) {
		for (n = 0; n < 256; n++) {
			if (dfs->dfs_radartable[n] != NULL) {
				qdf_mem_free(dfs->dfs_radartable[n]);
				dfs->dfs_radartable[n] = NULL;
			}
		}
		qdf_mem_free(dfs->dfs_radartable);
		dfs->dfs_radartable = NULL;
		dfs->wlan_dfs_isdfsregdomain = 0;
	}

	if (dfs->dfs_b5radars != NULL) {
		qdf_mem_free(dfs->dfs_b5radars);
		dfs->dfs_b5radars = NULL;
	}

	dfs_reset_ar(dfs);

	WLAN_ARQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_arq));
	WLAN_ARQ_UNLOCK(dfs);
	if (!empty)
		dfs_reset_arq(dfs);

	if (dfs->events != NULL) {
		qdf_mem_free(dfs->events);
		dfs->events = NULL;
	}
	dfs_nol_timer_cleanup(dfs);
}

void dfs_destroy_object(struct wlan_dfs *dfs)
{
	qdf_mem_free(dfs->dfs_curchan);
	qdf_mem_free(dfs);
}

int dfs_radar_disable(struct wlan_dfs *dfs)
{
	dfs->dfs_proc_phyerr &= ~DFS_AR_EN;
	dfs->dfs_proc_phyerr &= ~DFS_RADAR_EN;

	return 0;
}

int dfs_second_segment_radar_disable(struct wlan_dfs *dfs)
{
	dfs->dfs_proc_phyerr &= ~DFS_SECOND_SEGMENT_RADAR_EN;

	return 0;
}

void dfs_phyerr_param_copy(struct wlan_dfs_phyerr_param *dst,
		struct wlan_dfs_phyerr_param *src)
{
	memcpy(dst, src, sizeof(*dst));
}

struct dfs_state *dfs_getchanstate(struct wlan_dfs *dfs, uint8_t *index,
		int ext_chan_flag)
{
	struct dfs_state *rs = NULL;
	struct dfs_ieee80211_channel *cmp_ch = NULL;
	int i;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: dfs is NULL\n", __func__);
		return NULL;
	}

	if (ext_chan_flag) {
		dfs_mlme_get_extchan(dfs->dfs_pdev_obj,
				&(cmp_ch->ic_freq),
				&(cmp_ch->ic_flags),
				&(cmp_ch->ic_flagext),
				&(cmp_ch->ic_ieee),
				&(cmp_ch->ic_vhtop_ch_freq_seg1),
				&(cmp_ch->ic_vhtop_ch_freq_seg2));
		if (cmp_ch) {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
					"Extension channel freq = %u flags=0x%x\n",
					cmp_ch->ic_freq, cmp_ch->ic_flagext);
		} else {
			return NULL;
		}

	} else {
		cmp_ch = dfs->dfs_curchan;
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"Primary channel freq = %u flags=0x%x\n",
				cmp_ch->ic_freq, cmp_ch->ic_flagext);
	}
	for (i = 0; i < DFS_NUM_RADAR_STATES; i++) {
		if ((dfs->dfs_radar[i].rs_chan.ic_freq == cmp_ch->ic_freq) &&
			(dfs->dfs_radar[i].rs_chan.ic_flags == cmp_ch->ic_flags)
				) {
			if (index != NULL)
				*index = (uint8_t)i;
			return &(dfs->dfs_radar[i]);
		}
	}
	/* No existing channel found, look for first free channel state entry.*/
	for (i = 0; i < DFS_NUM_RADAR_STATES; i++) {
		if (dfs->dfs_radar[i].rs_chan.ic_freq == 0) {
			rs = &(dfs->dfs_radar[i]);
			/* Found one, set channel info and default thresholds.*/
			rs->rs_chan = *cmp_ch;

			/* Copy the parameters from the default set. */
			dfs_phyerr_param_copy(&rs->rs_param,
					&dfs->dfs_defaultparams);

			if (index != NULL)
				*index = (uint8_t)i;

			return rs;
		}
	}
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
			"%s: No more radar states left.\n", __func__);

	return NULL;
}

void dfs_radar_enable(struct wlan_dfs *dfs, int no_cac, uint32_t opmode)
{
	int is_ext_ch;
	int is_fastclk = 0;
	struct dfs_ieee80211_channel *ext_ch = NULL;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: dfs is NULL\n", __func__);
		return;
	}

	is_ext_ch = IEEE80211_IS_CHAN_11N_HT40(dfs->dfs_curchan);
	lmac_dfs_disable(dfs->dfs_pdev_obj, no_cac);

	/*
	 * In all modes, if the primary is DFS then we have to
	 * enable radar detection. In HT80_80, we can have
	 * primary non-DFS 80MHz with extension 80MHz DFS.
	 */
	if ((IEEE80211_IS_CHAN_DFS(dfs->dfs_curchan) ||
			((IEEE80211_IS_CHAN_11AC_VHT160(dfs->dfs_curchan) ||
		  IEEE80211_IS_CHAN_11AC_VHT80_80(dfs->dfs_curchan)) &&
		 IEEE80211_IS_CHAN_DFS_CFREQ2(dfs->dfs_curchan))) ||
			(dfs_is_precac_timer_running(dfs))) {
		struct dfs_state *rs_pri = NULL, *rs_ext = NULL;
		uint8_t index_pri, index_ext;

		dfs->dfs_proc_phyerr |= DFS_AR_EN;
		dfs->dfs_proc_phyerr |= DFS_RADAR_EN;
		dfs->dfs_proc_phyerr |= DFS_SECOND_SEGMENT_RADAR_EN;

		if (is_ext_ch)
			dfs_mlme_get_extchan(dfs->dfs_pdev_obj,
				&(ext_ch->ic_freq),
				&(ext_ch->ic_flags),
				&(ext_ch->ic_flagext),
				&(ext_ch->ic_ieee),
				&(ext_ch->ic_vhtop_ch_freq_seg1),
				&(ext_ch->ic_vhtop_ch_freq_seg2));

		dfs_reset_alldelaylines(dfs);

		rs_pri = dfs_getchanstate(dfs, &index_pri, 0);
		if (ext_ch)
			rs_ext = dfs_getchanstate(dfs, &index_ext, 1);

		if (rs_pri != NULL && ((ext_ch == NULL) || (rs_ext != NULL))) {
			struct wlan_dfs_phyerr_param pe;

			qdf_mem_set(&pe, '\0', sizeof(pe));

			if (index_pri != dfs->dfs_curchan_radindex)
				dfs_reset_alldelaylines(dfs);

			dfs->dfs_curchan_radindex = (int16_t)index_pri;

			if (rs_ext)
				dfs->dfs_extchan_radindex = (int16_t)index_ext;

			dfs_phyerr_param_copy(&pe, &rs_pri->rs_param);
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS3,
					"%s: firpwr=%d, rssi=%d, height=%d, prssi=%d, inband=%d, relpwr=%d, relstep=%d, maxlen=%d\n",
					__func__, pe.pe_firpwr,
					pe.pe_rrssi, pe.pe_height,
					pe.pe_prssi, pe.pe_inband,
					pe.pe_relpwr, pe.pe_relstep,
					pe.pe_maxlen);

			lmac_dfs_enable(dfs->dfs_pdev_obj, &is_fastclk,
					pe.pe_firpwr,
					pe.pe_rrssi,
					pe.pe_height,
					pe.pe_prssi,
					pe.pe_inband,
					pe.pe_relpwr,
					pe.pe_relstep,
					pe.pe_maxlen,
					dfs->dfsdomain);
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
					"Enabled radar detection on channel %d\n",
					dfs->dfs_curchan->ic_freq);

			dfs->dur_multiplier = is_fastclk ?
				DFS_FAST_CLOCK_MULTIPLIER :
				DFS_NO_FAST_CLOCK_MULTIPLIER;

			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS3,
					"%s: duration multiplier is %d\n",
					__func__, dfs->dur_multiplier);
		} else
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
					"%s: No more radar states left\n",
					__func__);
	}
}

int dfs_control(struct wlan_dfs *dfs,
		u_int id,
		void *indata,
		uint32_t insize,
		void *outdata,
		uint32_t *outsize)
{
	struct wlan_dfs_phyerr_param peout;
	struct dfs_ioctl_params *dfsparams;
	int error = 0;
	uint32_t val = 0;
	struct dfsreq_nolinfo *nol;
	uint32_t *data = NULL;
	int i;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1, "%s DFS is null\n", __func__);
		/*
		 * Enable/Disable DFS can be done prior to attach,
		 * So handle here.
		 */
		switch (id) {
		case DFS_DISABLE_DETECT:
			dfs->dfs_ignore_dfs = 1;
			DFS_PRINTK(
					"%s enable detects, ignore_dfs %d\n",
					__func__, dfs->dfs_ignore_dfs ? 1:0);
			break;
		case DFS_ENABLE_DETECT:
			dfs->dfs_ignore_dfs = 0;
			DFS_PRINTK(
					"%s enable detects, ignore_dfs %d\n",
					__func__, dfs->dfs_ignore_dfs ? 1:0);
			break;
		default:
			error = -EINVAL;
			break;
		}
		goto bad;
	}

	switch (id) {
	case DFS_SET_THRESH:
		if (insize < sizeof(struct dfs_ioctl_params) || !indata) {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
					"%s: insize = %d, expected = %zu bytes, indata = %p\n",
					__func__, insize,
					sizeof(struct dfs_ioctl_params),
					indata);
			error = -EINVAL;
			break;
		}
		dfsparams = (struct dfs_ioctl_params *)indata;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_FIRPWR,
					dfsparams->dfs_firpwr))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_RRSSI,
					dfsparams->dfs_rrssi))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_HEIGHT,
					dfsparams->dfs_height))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_PRSSI,
					dfsparams->dfs_prssi))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_INBAND,
					dfsparams->dfs_inband))
			error = -EINVAL;

		/* 5413 speicfic. */
		if (!dfs_set_thresholds(dfs, DFS_PARAM_RELPWR,
					dfsparams->dfs_relpwr))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_RELSTEP,
					dfsparams->dfs_relstep))
			error = -EINVAL;
		if (!dfs_set_thresholds(dfs, DFS_PARAM_MAXLEN,
					dfsparams->dfs_maxlen))
			error = -EINVAL;
		break;
	case DFS_GET_THRESH:
		if (!outdata || !outsize ||
				*outsize < sizeof(struct dfs_ioctl_params)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(struct dfs_ioctl_params);
		dfsparams = (struct dfs_ioctl_params *) outdata;

		/* Fetch the DFS thresholds using the internal representation */
		(void) dfs_get_thresholds(dfs, &peout);

		/* Convert them to the dfs IOCTL representation. */
		wlan_dfs_dfsparam_to_ioctlparam(&peout, dfsparams);
		break;
	case DFS_RADARDETECTS:
		if (!outdata || !outsize || *outsize < sizeof(uint32_t)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(uint32_t);
		*((uint32_t *)outdata) = dfs->wlan_dfs_stats.num_radar_detects;
		break;
	case DFS_DISABLE_DETECT:
		dfs->dfs_proc_phyerr &= ~DFS_RADAR_EN;
		dfs->dfs_proc_phyerr &= ~DFS_SECOND_SEGMENT_RADAR_EN;
		dfs->dfs_ignore_dfs = 1;
		DFS_PRINTK("%s enable detects, ignore_dfs %d\n", __func__,
				dfs->dfs_ignore_dfs ? 1:0);
		break;
	case DFS_ENABLE_DETECT:
		dfs->dfs_proc_phyerr |= DFS_RADAR_EN;
		dfs->dfs_proc_phyerr |= DFS_SECOND_SEGMENT_RADAR_EN;
		dfs->dfs_ignore_dfs = 0;
		DFS_PRINTK("%s enable detects, ignore_dfs %d\n", __func__,
				dfs->dfs_ignore_dfs ? 1:0);
		break;
	case DFS_DISABLE_FFT:
		DFS_PRINTK("%s TODO disable FFT val=0x%x\n", __func__, val);
		break;
	case DFS_ENABLE_FFT:
		DFS_PRINTK("%s TODO enable FFT val=0x%x\n", __func__, val);
		break;
	case DFS_SET_DEBUG_LEVEL:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->dfs_debug_mask = *(uint32_t *)indata;
		DFS_PRINTK("%s debug level now = 0x%x\n", __func__,
				dfs->dfs_debug_mask);
		if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS3) {
			/* Enable debug Radar Event */
			dfs->dfs_event_log_on = 1;
		} else {
			dfs->dfs_event_log_on = 0;
		}
		break;
	case DFS_SET_FALSE_RSSI_THRES:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->wlan_dfs_false_rssi_thres = *(uint32_t *)indata;
		DFS_PRINTK("%s false RSSI threshold now = 0x%x\n", __func__,
				dfs->wlan_dfs_false_rssi_thres);
		break;
	case DFS_SET_PEAK_MAG:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->wlan_dfs_peak_mag = *(uint32_t *)indata;
		DFS_PRINTK("%s peak_mag now = 0x%x\n", __func__,
				dfs->wlan_dfs_peak_mag);
		break;
	case DFS_GET_CAC_VALID_TIME:
		if (!outdata || !outsize || *outsize < sizeof(uint32_t)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(uint32_t);
		*((uint32_t *)outdata) = dfs->dfs_cac_valid_time;
		break;
	case DFS_SET_CAC_VALID_TIME:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->dfs_cac_valid_time = *(uint32_t *)indata;
		DFS_PRINTK("%s dfs timeout = %d\n", __func__,
				dfs->dfs_cac_valid_time);
		break;
	case DFS_IGNORE_CAC:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}

		if (*(uint32_t *)indata)
			dfs->dfs_ignore_cac = 1;
		else
			dfs->dfs_ignore_cac = 0;

		DFS_PRINTK("%s ignore cac = 0x%x\n", __func__,
				dfs->dfs_ignore_cac);
		break;
	case DFS_SET_NOL_TIMEOUT:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		if (*(int *)indata)
			dfs->wlan_dfs_nol_timeout = *(int *)indata;
		else
			dfs->wlan_dfs_nol_timeout = DFS_NOL_TIMEOUT_S;

		DFS_PRINTK("%s nol timeout = %d sec\n", __func__,
				dfs->wlan_dfs_nol_timeout);
		break;
	case DFS_MUTE_TIME:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		data = (uint32_t *) indata;
		dfs->wlan_dfstesttime = *data;
		dfs->wlan_dfstesttime *= (1000); /* convert sec into ms */
		break;
	case DFS_GET_USENOL:
		if (!outdata || !outsize || *outsize < sizeof(uint32_t)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(uint32_t);
		*((uint32_t *)outdata) = dfs->dfs_rinfo.rn_use_nol;

		DFS_PRINTK(
				"%s:#Phyerr=%d, #false detect=%d, #queued=%d\n",
				__func__, dfs->dfs_phyerr_count,
				dfs->dfs_phyerr_reject_count,
				dfs->dfs_phyerr_queued_count);

		DFS_PRINTK(
				"%s:dfs_phyerr_freq_min=%d, dfs_phyerr_freq_max=%d\n",
				__func__, dfs->dfs_phyerr_freq_min,
				dfs->dfs_phyerr_freq_max);

		DFS_PRINTK(
				"%s:Total radar events detected=%d, entries in the radar queue follows:\n",
				__func__, dfs->dfs_event_log_count);

		for (i = 0; (i < DFS_EVENT_LOG_SIZE) &&
				(i < dfs->dfs_event_log_count); i++) {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			    "ts=%llu diff_ts=%u rssi=%u dur=%u, is_chirp=%d, seg_id=%d, sidx=%d, freq_offset=%d.%dMHz, peak_mag=%d, total_gain=%d, mb_gain=%d, relpwr_db=%d\n",
			    dfs->radar_log[i].ts,
			    dfs->radar_log[i].diff_ts,
			    dfs->radar_log[i].rssi,
			    dfs->radar_log[i].dur,
			    dfs->radar_log[i].is_chirp,
			    dfs->radar_log[i].seg_id,
			    dfs->radar_log[i].sidx,
			    ((int)dfs->radar_log[i].freq_offset_khz/1000),
			    ((int)abs
			     (dfs->radar_log[i].freq_offset_khz)%1000),
			    dfs->radar_log[i].peak_mag,
			    dfs->radar_log[i].total_gain,
			    dfs->radar_log[i].mb_gain,
			    dfs->radar_log[i].relpwr_db);
		}
		dfs->dfs_event_log_count = 0;
		dfs->dfs_phyerr_count = 0;
		dfs->dfs_phyerr_reject_count = 0;
		dfs->dfs_phyerr_queued_count = 0;
		dfs->dfs_phyerr_freq_min = 0x7fffffff;
		dfs->dfs_phyerr_freq_max = 0;
		break;
	case DFS_SET_USENOL:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->dfs_rinfo.rn_use_nol = *(uint32_t *)indata;
		break;
	case DFS_GET_NOL:
		if (!outdata || !outsize ||
				*outsize < sizeof(struct dfsreq_nolinfo)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(struct dfsreq_nolinfo);
		nol = (struct dfsreq_nolinfo *)outdata;
		dfs_get_nol(dfs,
				(struct dfsreq_nolelem *)nol->dfs_nol,
				&nol->ic_nchans);
		dfs_print_nol(dfs);
		break;
	case DFS_SET_NOL:
		if (insize < sizeof(struct dfsreq_nolinfo) || !indata) {
			error = -EINVAL;
			break;
		}
		nol = (struct dfsreq_nolinfo *) indata;
		dfs_set_nol(dfs,
				(struct dfsreq_nolelem *)nol->dfs_nol,
				nol->ic_nchans);
		break;
	case DFS_SHOW_NOL:
		dfs_print_nol(dfs);
		break;
	case DFS_SHOW_NOLHISTORY:
		dfs_print_nolhistory(dfs);
		break;
	case DFS_BANGRADAR:
		dfs->dfs_bangradar = 1;
		dfs->wlan_radar_tasksched = 1;
		OS_SET_TIMER(&dfs->wlan_dfs_task_timer, 0);
		error = 0;
		break;
	case DFS_SHOW_PRECAC_LISTS:
		dfs_print_precaclists(dfs);
		break;
	case DFS_RESET_PRECAC_LISTS:
		dfs_reset_precac_lists(dfs);
		break;
	case DFS_SECOND_SEGMENT_BANGRADAR:
		dfs->dfs_second_segment_bangradar = 1;
		dfs->wlan_radar_tasksched = 1;
		OS_SET_TIMER(&dfs->wlan_dfs_task_timer, 0);
		error = 0;
		break;
	default:
		error = -EINVAL;
	}

bad:
	return error;
}

int dfs_set_thresholds(struct wlan_dfs *dfs, const uint32_t threshtype,
		const uint32_t value)
{
	int16_t chanindex;
	struct dfs_state *rs;
	struct wlan_dfs_phyerr_param pe;
	int is_fastclk = 0;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: dfs is NULL\n", __func__);
		return 0;
	}

	chanindex = dfs->dfs_curchan_radindex;
	if ((chanindex < 0) || (chanindex >= DFS_NUM_RADAR_STATES)) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: chanindex = %d, DFS_NUM_RADAR_STATES=%d\n",
				__func__, chanindex, DFS_NUM_RADAR_STATES);
		return 0;
	}

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"%s: threshtype=%d, value=%d\n",
			__func__, threshtype, value);

	wlan_dfs_phyerr_init_noval(&pe);

	rs = &(dfs->dfs_radar[chanindex]);
	switch (threshtype) {
	case DFS_PARAM_FIRPWR:
		rs->rs_param.pe_firpwr = (int32_t) value;
		pe.pe_firpwr = value;
		break;
	case DFS_PARAM_RRSSI:
		rs->rs_param.pe_rrssi = value;
		pe.pe_rrssi = value;
		break;
	case DFS_PARAM_HEIGHT:
		rs->rs_param.pe_height = value;
		pe.pe_height = value;
		break;
	case DFS_PARAM_PRSSI:
		rs->rs_param.pe_prssi = value;
		pe.pe_prssi = value;
		break;
	case DFS_PARAM_INBAND:
		rs->rs_param.pe_inband = value;
		pe.pe_inband = value;
		break;
		/* 5413 specific */
	case DFS_PARAM_RELPWR:
		rs->rs_param.pe_relpwr = value;
		pe.pe_relpwr = value;
		break;
	case DFS_PARAM_RELSTEP:
		rs->rs_param.pe_relstep = value;
		pe.pe_relstep = value;
		break;
	case DFS_PARAM_MAXLEN:
		rs->rs_param.pe_maxlen = value;
		pe.pe_maxlen = value;
		break;
	default:
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: unknown threshtype (%d)\n",
				__func__, threshtype);
		break;
	}

	/*
	 * The driver layer dfs_enable routine is tasked with translating
	 * values from the global format to the per-device (HAL, offload)
	 * format.
	 */
	lmac_dfs_enable(dfs->dfs_pdev_obj, &is_fastclk,
			pe.pe_firpwr,
			pe.pe_rrssi,
			pe.pe_height,
			pe.pe_prssi,
			pe.pe_inband,
			pe.pe_relpwr,
			pe.pe_relstep,
			pe.pe_maxlen,
			dfs->dfsdomain);

	return 1;
}

int dfs_get_thresholds(struct wlan_dfs *dfs,
		struct wlan_dfs_phyerr_param *param)
{
	qdf_mem_zero(param, sizeof(*param));

	lmac_dfs_get_thresholds(dfs->dfs_pdev_obj,
			&(param->pe_firpwr),
			&(param->pe_rrssi),
			&(param->pe_height),
			&(param->pe_prssi),
			&(param->pe_inband),
			&(param->pe_relpwr),
			&(param->pe_relstep),
			&(param->pe_maxlen));

	return 1;
}

void dfs_getnol(struct wlan_dfs *dfs, void *dfs_nolinfo)
{
	struct dfsreq_nolinfo *nolinfo = (struct dfsreq_nolinfo *)dfs_nolinfo;

	dfs_get_nol(dfs, nolinfo->dfs_nol, &(nolinfo->ic_nchans));
}

void dfs_clear_nolhistory(struct wlan_dfs *dfs)
{
	/* We should have a dfs_clear_nolhistory API from Regdomain. */
	struct dfs_ieee80211_channel *c, lc;
	int i;
	int nchans = 0;

	c = &lc;
	dfs_mlme_get_ic_nchans(dfs->dfs_pdev_obj, &nchans);
	for (i = 0; i < nchans; i++) {
		dfs_mlme_get_ic_channels(dfs->dfs_pdev_obj,
				&(c->ic_freq),
				&(c->ic_flags),
				&(c->ic_flagext),
				&(c->ic_ieee),
				&(c->ic_vhtop_ch_freq_seg1),
				&(c->ic_vhtop_ch_freq_seg2),
				i);
		IEEE80211_CHAN_CLR_HISTORY_RADAR(c);
	}
}

void dfs_set_current_channel(struct wlan_dfs *dfs,
		uint16_t ic_freq,
		uint32_t ic_flags,
		uint16_t ic_flagext,
		uint8_t ic_ieee,
		uint8_t ic_vhtop_ch_freq_seg1,
		uint8_t ic_vhtop_ch_freq_seg2)
{
	if (dfs == NULL) {
		DFS_PRINTK("%s: wlan_dfs is NULL\n", __func__);
		return;
	}

	dfs->dfs_curchan->ic_freq = ic_freq;
	dfs->dfs_curchan->ic_flags = ic_flags;
	dfs->dfs_curchan->ic_flagext = ic_flagext;
	dfs->dfs_curchan->ic_ieee = ic_ieee;
	dfs->dfs_curchan->ic_vhtop_ch_freq_seg1 = ic_vhtop_ch_freq_seg1;
	dfs->dfs_curchan->ic_vhtop_ch_freq_seg2 = ic_vhtop_ch_freq_seg2;
}

u_int dfs_ieee80211_chan2freq(struct dfs_ieee80211_channel *chan)
{
	if (chan == NULL)
		return 0;

	return chan == IEEE80211_CHAN_ANYC ?
		IEEE80211_CHAN_ANY : chan->ic_freq;
}
