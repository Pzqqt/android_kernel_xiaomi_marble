/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "wlan_dfs_tgt_api.h"
#include "../dfs_internal.h"
#include "../dfs_filter_init.h"
#include "../dfs_partial_offload_radar.h"

#ifndef WLAN_DFS_STATIC_MEM_ALLOC
/*
 * dfs_alloc_dfs_events() - allocate dfs events buffer
 *
 * Return: events buffer, null on failure.
 */
static inline struct dfs_event *dfs_alloc_dfs_events(void)
{
	return qdf_mem_malloc(sizeof(struct dfs_event) * DFS_MAX_EVENTS);
}

/*
 * dfs_free_dfs_events() - Free events buffer
 * @events: Events buffer pointer
 *
 * Return: None
 */
static inline void dfs_free_dfs_events(struct dfs_event *events)
{
	qdf_mem_free(events);
}

/*
 * dfs_alloc_dfs_pulseline() - allocate buffer for dfs pulses
 *
 * Return: events buffer, null on failure.
 */
static inline struct dfs_pulseline *dfs_alloc_dfs_pulseline(void)
{
	return qdf_mem_malloc(sizeof(struct dfs_pulseline));
}

/*
 * dfs_free_dfs_pulseline() - Free pulse buffer
 * @pulses: Pulses buffer pointer
 *
 * Return: None
 */
static inline void dfs_free_dfs_pulseline(struct dfs_pulseline *pulses)
{
	qdf_mem_free(pulses);
}
#else
/* Static buffers for DFS objects */
static struct dfs_event global_dfs_event[DFS_MAX_EVENTS];
static struct dfs_pulseline global_dfs_pulseline;

static inline struct dfs_event *dfs_alloc_dfs_events(void)
{
	return global_dfs_event;
}

static inline void dfs_free_dfs_events(struct dfs_event *events)
{
}

static inline struct dfs_pulseline *dfs_alloc_dfs_pulseline(void)
{
	return &global_dfs_pulseline;
}

static inline void dfs_free_dfs_pulseline(struct dfs_pulseline *pulses)
{
}
#endif

/*
 * Channel switch announcement (CSA)
 * usenol=1 (default) make CSA and switch to a new channel on radar detect
 * usenol=0, make CSA with next channel same as current on radar detect
 * usenol=2, no CSA and stay on the same channel on radar detect
 */

/**
 * dfs_task() - The timer function to process the radar pulses.
 */
static os_timer_func(dfs_task)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	/* Need to take a lock here since dfs filtering data structures are
	 * freed and re-allocated in dfs_init_radar_filters() during channel
	 * change which may happen in the middle of dfs pulse processing.
	 */
	WLAN_DFS_DATA_STRUCT_LOCK(dfs);
	dfs_process_radarevent(dfs, dfs->dfs_curchan);
	WLAN_DFS_DATA_STRUCT_UNLOCK(dfs);

	dfs->wlan_radar_tasksched = 0;
}

/**
 * dfs_main_task_timer_init() - Initialize dfs task timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
static void dfs_main_task_timer_init(struct wlan_dfs *dfs)
{
	qdf_timer_init(NULL,
			&(dfs->wlan_dfs_task_timer),
			dfs_task,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
}

/**
 * dfs_free_filter() - free memory allocated for dfs ft_filters
 * @radarf: pointer holding ft_filters.
 *
 * Return: None
 */
static void dfs_free_filter(struct dfs_filtertype *radarf)
{
	uint8_t i;

	for (i = 0; i < DFS_MAX_NUM_RADAR_FILTERS; i++) {
		if (radarf->ft_filters[i]) {
			qdf_mem_free(radarf->ft_filters[i]);
			radarf->ft_filters[i] = NULL;
		}
	}
}

/**
 * dfs_alloc_mem_filter() - allocate memory for dfs ft_filters
 * @radarf: pointer holding ft_filters.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dfs_alloc_mem_filter(struct dfs_filtertype *radarf)
{
	uint8_t i;

	for (i = 0; i < DFS_MAX_NUM_RADAR_FILTERS; i++) {
		radarf->ft_filters[i] = qdf_mem_malloc(sizeof(struct
							      dfs_filter));
		if (!radarf->ft_filters[i]) {
			/* Free all the filter if malloc failed */
			dfs_free_filter(radarf);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

int dfs_main_attach(struct wlan_dfs *dfs)
{
	int i, n;
	QDF_STATUS status;
	struct wlan_dfs_radar_tab_info radar_info;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return 0;
	}

	/* If ignore_dfs is set to 1 then Radar detection is disabled. */
	if (dfs->dfs_ignore_dfs) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1, "ignoring dfs");
		return 0;
	}

	/*
	 * Zero out radar_info. It's possible that the attach function
	 * won't fetch an initial regulatory configuration; you really
	 * do want to ensure that the contents indicates there aren't
	 * any filters.
	 */
	qdf_mem_zero(&radar_info, sizeof(radar_info));

	lmac_get_caps(dfs->dfs_pdev_obj, &(dfs->dfs_caps));

	dfs_clear_stats(dfs);
	dfs->dfs_event_log_on = 1;
	dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS, "event log enabled by default");

	dfs->dfs_enable = 1;

	/*Verify : Passing NULL to qdf_timer_init().*/
	dfs_main_task_timer_init(dfs);

	dfs_host_wait_timer_init(dfs);

	WLAN_DFSQ_LOCK_CREATE(dfs);
	STAILQ_INIT(&dfs->dfs_radarq);
	WLAN_ARQ_LOCK_CREATE(dfs);
	STAILQ_INIT(&dfs->dfs_arq);
	STAILQ_INIT(&(dfs->dfs_eventq));
	WLAN_DFSEVENTQ_LOCK_CREATE(dfs);
	WLAN_DFS_DATA_STRUCT_LOCK_CREATE(dfs);

	dfs->events = dfs_alloc_dfs_events();
	if (!(dfs->events))
		return 1;

	for (i = 0; i < DFS_MAX_EVENTS; i++)
		STAILQ_INSERT_TAIL(&(dfs->dfs_eventq), &dfs->events[i],
				re_list);

	dfs->pulses = dfs_alloc_dfs_pulseline();
	if (!(dfs->pulses)) {
		dfs_free_dfs_events(dfs->events);
		dfs->events = NULL;
		return 1;
	}

	dfs->pulses->pl_lastelem = DFS_MAX_PULSE_BUFFER_MASK;

	/* Allocate memory for radar filters. */
	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		dfs->dfs_radarf[n] = (struct dfs_filtertype *)
			qdf_mem_malloc(sizeof(struct dfs_filtertype));
		if (!(dfs->dfs_radarf[n]))
			goto bad1;

		qdf_mem_zero(dfs->dfs_radarf[n],
			     sizeof(struct dfs_filtertype));
		status = dfs_alloc_mem_filter(dfs->dfs_radarf[n]);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			dfs_alert(dfs, WLAN_DEBUG_DFS_ALWAYS,
				  "mem alloc for dfs_filter failed");
			goto bad1;
		}
	}

	/* Allocate memory for radar table. */
	dfs->dfs_ftindextable = (int8_t **)qdf_mem_malloc(
			DFS_NUM_FT_IDX_TBL_ROWS*sizeof(int8_t *));
	if (!(dfs->dfs_ftindextable))
		goto bad1;

	for (n = 0; n < DFS_NUM_FT_IDX_TBL_ROWS; n++) {
		dfs->dfs_ftindextable[n] = qdf_mem_malloc(
				DFS_MAX_RADAR_OVERLAP*sizeof(int8_t));
		if (!(dfs->dfs_ftindextable[n]))
			goto bad2;
	}

	dfs->dfs_use_nol = 1;

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
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "Radar Filter Intialization Failed");
		return 1;
	}

	dfs->wlan_dfs_false_rssi_thres = RSSI_POSSIBLY_FALSE;
	dfs->wlan_dfs_peak_mag = SEARCH_FFT_REPORT_PEAK_MAG_THRSH;
	dfs->dfs_phyerr_freq_min     = 0x7fffffff;
	dfs->dfs_phyerr_freq_max     = 0;
	dfs->dfs_phyerr_queued_count = 0;
	dfs->dfs_phyerr_w53_counter  = 0;
	dfs->dfs_pri_multiplier      = 2;
	dfs_get_radars(dfs);

	return 0;

bad2:
	qdf_mem_free(dfs->dfs_ftindextable);
	dfs->dfs_ftindextable = NULL;
bad1:
	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		if (dfs->dfs_radarf[n]) {
			dfs_free_filter(dfs->dfs_radarf[n]);
			qdf_mem_free(dfs->dfs_radarf[n]);
			dfs->dfs_radarf[n] = NULL;
		}
	}
	if (dfs->pulses) {
		dfs_free_dfs_pulseline(dfs->pulses);
		dfs->pulses = NULL;
	}
	if (dfs->events) {
		dfs_free_dfs_events(dfs->events);
		dfs->events = NULL;
	}

	return 1;
}

void dfs_main_timer_reset(struct wlan_dfs *dfs)
{
	if (dfs->wlan_radar_tasksched) {
		qdf_timer_sync_cancel(&dfs->wlan_dfs_task_timer);
		dfs->wlan_radar_tasksched = 0;
	}
}

void dfs_main_timer_detach(struct wlan_dfs *dfs)
{
	qdf_timer_free(&dfs->wlan_dfs_task_timer);
	dfs->wlan_radar_tasksched = 0;
}

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_host_wait_timer_detach(struct wlan_dfs *dfs)
{
	qdf_timer_free(&dfs->dfs_host_wait_timer);
}
#endif

void dfs_main_detach(struct wlan_dfs *dfs)
{
	int n, empty;

	if (!dfs->dfs_enable) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "Already detached");
		return;
	}

	dfs->dfs_enable = 0;

	dfs_reset_radarq(dfs);
	dfs_reset_alldelaylines(dfs);

	if (dfs->pulses) {
		dfs_free_dfs_pulseline(dfs->pulses);
		dfs->pulses = NULL;
	}

	for (n = 0; n < DFS_MAX_RADAR_TYPES; n++) {
		if (dfs->dfs_radarf[n]) {
			dfs_free_filter(dfs->dfs_radarf[n]);
			qdf_mem_free(dfs->dfs_radarf[n]);
			dfs->dfs_radarf[n] = NULL;
		}
	}

	if (dfs->dfs_ftindextable) {
		for (n = 0; n < DFS_NUM_FT_IDX_TBL_ROWS; n++) {
			if (dfs->dfs_ftindextable[n]) {
				qdf_mem_free(dfs->dfs_ftindextable[n]);
				dfs->dfs_ftindextable[n] = NULL;
			}
		}
		qdf_mem_free(dfs->dfs_ftindextable);
		dfs->dfs_ftindextable = NULL;
		dfs->wlan_dfs_isdfsregdomain = 0;
	}

	if (dfs->dfs_b5radars) {
		qdf_mem_free(dfs->dfs_b5radars);
		dfs->dfs_b5radars = NULL;
	}

	dfs_reset_ar(dfs);

	WLAN_ARQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_arq));
	WLAN_ARQ_UNLOCK(dfs);
	if (!empty)
		dfs_reset_arq(dfs);

	if (dfs->events) {
		dfs_free_dfs_events(dfs->events);
		dfs->events = NULL;
	}

	WLAN_DFS_DATA_STRUCT_LOCK_DESTROY(dfs);
	WLAN_DFSQ_LOCK_DESTROY(dfs);
	WLAN_ARQ_LOCK_DESTROY(dfs);
	WLAN_DFSEVENTQ_LOCK_DESTROY(dfs);
}

int dfs_start_host_based_bangradar(struct wlan_dfs *dfs)
{
	dfs->wlan_radar_tasksched = 1;
	qdf_timer_mod(&dfs->wlan_dfs_task_timer, 0);

	return 0;
}
