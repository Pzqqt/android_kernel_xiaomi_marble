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
#include "../dfs_full_offload.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#include "../dfs_etsi_precac.h"
#include "../dfs_partial_offload_radar.h"

/* Disable NOL in FW. */
#define DISABLE_NOL_FW 0

#ifndef WLAN_DFS_STATIC_MEM_ALLOC
/*
 * dfs_alloc_wlan_dfs() - allocate wlan_dfs buffer
 *
 * Return: buffer, null on failure.
 */
static inline struct wlan_dfs *dfs_alloc_wlan_dfs(void)
{
	return qdf_mem_malloc(sizeof(struct wlan_dfs));
}

/*
 * dfs_free_wlan_dfs() - Free wlan_dfs buffer
 * @dfs: wlan_dfs buffer pointer
 *
 * Return: None
 */
static inline void dfs_free_wlan_dfs(struct wlan_dfs *dfs)
{
	qdf_mem_free(dfs);
}

/*
 * dfs_alloc_dfs_curchan() - allocate dfs_channel buffer
 *
 * Return: buffer, null on failure.
 */
static inline struct dfs_channel *dfs_alloc_dfs_curchan(void)
{
	return qdf_mem_malloc(sizeof(struct dfs_channel));
}

/*
 * dfs_free_dfs_curchan() - Free dfs_channel buffer
 * @dfs_curchan: dfs_channel buffer pointer
 *
 * Return: None
 */
static inline void dfs_free_dfs_curchan(struct dfs_channel *dfs_curchan)
{
	qdf_mem_free(dfs_curchan);
}

#else

/* Static buffers for DFS objects */
static struct wlan_dfs global_dfs;
static struct dfs_channel global_dfs_curchan;

static inline struct wlan_dfs *dfs_alloc_wlan_dfs(void)
{
	return &global_dfs;
}

static inline void dfs_free_wlan_dfs(struct wlan_dfs *dfs)
{
}

static inline struct dfs_channel *dfs_alloc_dfs_curchan(void)
{
	return &global_dfs_curchan;
}

static inline void dfs_free_dfs_curchan(struct dfs_channel *dfs_curchan)
{
}
#endif

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
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "go back to channel %d",
			dfs->wlan_dfstest_ieeechan);
	dfs_mlme_start_csa(dfs->dfs_pdev_obj,
			dfs->wlan_dfstest_ieeechan,
			dfs->dfs_curchan->dfs_ch_freq,
			dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
			dfs->dfs_curchan->dfs_ch_flags);
}

int dfs_get_debug_info(struct wlan_dfs *dfs, void *data)
{
	if (data)
		*(uint32_t *)data = dfs->dfs_proc_phyerr;

	return (int)dfs->dfs_proc_phyerr;
}

void dfs_main_task_testtimer_init(struct wlan_dfs *dfs)
{
	qdf_timer_init(NULL,
		&(dfs->wlan_dfstesttimer),
		dfs_testtimer_task, (void *)dfs,
		QDF_TIMER_TYPE_WAKE_APPS);
}

int dfs_create_object(struct wlan_dfs **dfs)
{
	*dfs = dfs_alloc_wlan_dfs();
	if (!(*dfs))
		return 1;

	qdf_mem_zero(*dfs, sizeof(**dfs));

	(*dfs)->dfs_curchan = dfs_alloc_dfs_curchan();
	if (!((*dfs)->dfs_curchan)) {
		dfs_free_wlan_dfs(*dfs);
		return 1;
	}

	return 0;
}

int dfs_attach(struct wlan_dfs *dfs)
{
	int ret;

	if (!dfs->dfs_is_offload_enabled) {
		ret = dfs_main_attach(dfs);

		/*
		 * For full offload we have a wmi handler registered to process
		 * a radar event from firmware in the event of a radar detect.
		 * So, init of timer, dfs_task is not required for
		 * full-offload. dfs_task timer is called in
		 * dfs_main_timer_init within dfs_main_attach for
		 * partial-offload in the event of radar detect.
		 */
		if (ret) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_main_attach failed");
			return ret;
		}
	}
	dfs_cac_attach(dfs);
	dfs_zero_cac_attach(dfs);
	dfs_etsi_precac_attach(dfs);
	dfs_nol_attach(dfs);

	/*
	 * Init of timer ,dfs_testtimer_task is required by both partial
	 * and full offload, indicating test mode timer initialization for both.
	 */
	dfs_main_task_testtimer_init(dfs);
	return 0;
}

void dfs_stop(struct wlan_dfs *dfs)
{
	dfs_nol_timer_cleanup(dfs);
	dfs_nol_workqueue_cleanup(dfs);
	dfs_clear_nolhistory(dfs);
}

void dfs_task_testtimer_reset(struct wlan_dfs *dfs)
{
	if (dfs->wlan_dfstest) {
		qdf_timer_sync_cancel(&dfs->wlan_dfstesttimer);
		dfs->wlan_dfstest = 0;
	}
}

void dfs_task_testtimer_detach(struct wlan_dfs *dfs)
{
	qdf_timer_free(&dfs->wlan_dfstesttimer);
	dfs->wlan_dfstest = 0;
}

void dfs_reset(struct wlan_dfs *dfs)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	dfs_cac_timer_reset(dfs);
	dfs_zero_cac_reset(dfs);
	if (!dfs->dfs_is_offload_enabled) {
		dfs_main_timer_reset(dfs);
		dfs_host_wait_timer_reset(dfs);
		dfs_false_radarfound_reset_vars(dfs);
	}
	dfs_task_testtimer_reset(dfs);
}

void dfs_timer_detach(struct wlan_dfs *dfs)
{
	dfs_cac_timer_detach(dfs);
	dfs_zero_cac_timer_detach(dfs->dfs_soc_obj);

	if (!dfs->dfs_is_offload_enabled) {
		dfs_main_timer_detach(dfs);
		dfs_host_wait_timer_detach(dfs);
	}

	dfs_task_testtimer_detach(dfs);
	dfs_nol_timer_detach(dfs);
}

void dfs_detach(struct wlan_dfs *dfs)
{
	dfs_timer_detach(dfs);
	if (!dfs->dfs_is_offload_enabled)
		dfs_main_detach(dfs);
	dfs_zero_cac_detach(dfs);
	dfs_nol_detach(dfs);
	dfs_etsi_precac_detach(dfs);
}

#ifndef WLAN_DFS_STATIC_MEM_ALLOC
void dfs_destroy_object(struct wlan_dfs *dfs)
{
	dfs_free_dfs_curchan(dfs->dfs_curchan);
	dfs_free_wlan_dfs(dfs);
}
#else
void dfs_destroy_object(struct wlan_dfs *dfs)
{
}
#endif

/* dfs_set_disable_radar_marking()- Set the flag to mark/unmark a radar flag
 * on NOL channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @disable_radar_marking: Flag to enable/disable marking channel as radar.
 */
#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
static void dfs_set_disable_radar_marking(struct wlan_dfs *dfs,
					  bool disable_radar_marking)
{
	dfs->dfs_disable_radar_marking = disable_radar_marking;
}
#else
static inline void dfs_set_disable_radar_marking(struct wlan_dfs *dfs,
						 bool disable_radar_marking)
{
}
#endif

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
bool dfs_get_disable_radar_marking(struct wlan_dfs *dfs)
{
	return dfs->dfs_disable_radar_marking;
}
#else
static inline bool dfs_get_disable_radar_marking(struct wlan_dfs *dfs)
{
	return QDF_STATUS_SUCCESS;
}
#endif
int dfs_control(struct wlan_dfs *dfs,
		u_int id,
		void *indata,
		uint32_t insize,
		void *outdata,
		uint32_t *outsize)
{
	struct wlan_dfs_phyerr_param peout;
	struct dfs_ioctl_params *dfsparams;
	struct dfs_bangradar_params *bangradar_params;
	int error = 0;
	uint32_t val = 0;
	struct dfsreq_nolinfo *nol;
	uint32_t *data = NULL;
	int i;
	struct dfs_emulate_bang_radar_test_cmd dfs_unit_test;
	int usenol_pdev_param;

	qdf_mem_zero(&dfs_unit_test, sizeof(dfs_unit_test));

	if (!dfs) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		goto bad;
	}

	switch (id) {
	case DFS_SET_THRESH:
		if (insize < sizeof(struct dfs_ioctl_params) || !indata) {
			dfs_debug(dfs, WLAN_DEBUG_DFS1,
					"insize = %d, expected = %zu bytes, indata = %pK",
					insize,
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
	case DFS_BANGRADAR:
		/*
		 * Handle all types of Bangradar here.
		 * Bangradar arguments:
		 * seg_id      : Segment ID where radar should be injected.
		 * is_chirp    : Is chirp radar or non chirp radar.
		 * freq_offset : Frequency offset from center frequency.
		 *
		 * Type 1 (DFS_BANGRADAR_FOR_ALL_SUBCHANS): To add all subchans.
		 * Type 2 (DFS_BANGRADAR_FOR_ALL_SUBCHANS_OF_SEGID): To add all
		 *               subchans of given segment_id.
		 * Type 3 (DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS): To add specific
		 *               subchans based on the arguments.
		 *
		 * The arguments will already be filled in the indata structure
		 * based on the type.
		 * If an argument is not specified by user, it will be set to
		 * default (0) in the indata already and correspondingly,
		 * the type will change.
		 */
		if (insize < sizeof(struct dfs_bangradar_params) ||
		    !indata) {
			dfs_debug(dfs, WLAN_DEBUG_DFS1,
				  "insize = %d, expected = %zu bytes, indata = %pK",
				  insize,
				  sizeof(struct dfs_bangradar_params),
				  indata);
			error = -EINVAL;
			break;
		}
		bangradar_params = (struct dfs_bangradar_params *)indata;
		if (bangradar_params) {
			if (abs(bangradar_params->freq_offset) >
			    FREQ_OFFSET_BOUNDARY_FOR_80MHZ) {
				dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					 "Frequency Offset out of bound");
				error = -EINVAL;
				break;
			} else if (bangradar_params->seg_id >
				   SEG_ID_SECONDARY) {
				dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					 "Illegal segment ID");
				error = -EINVAL;
				break;
			}
			dfs->dfs_bangradar_type =
				bangradar_params->bangradar_type;
			dfs->dfs_seg_id = bangradar_params->seg_id;
			dfs->dfs_is_chirp = bangradar_params->is_chirp;
			dfs->dfs_freq_offset = bangradar_params->freq_offset;

			if (dfs->dfs_is_offload_enabled) {
				error = dfs_fill_emulate_bang_radar_test
							(dfs, dfs->dfs_seg_id,
							 dfs->dfs_is_chirp,
							 dfs->dfs_freq_offset,
							 &dfs_unit_test);
			} else {
				error = dfs_start_host_based_bangradar(dfs);
			}
		} else {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				 "bangradar_params is NULL");
		}

		break;
	case DFS_GET_THRESH:
		if (!outdata || !outsize ||
				*outsize < sizeof(struct dfs_ioctl_params)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(struct dfs_ioctl_params);
		dfsparams = (struct dfs_ioctl_params *) outdata;

		qdf_mem_zero(&peout, sizeof(struct wlan_dfs_phyerr_param));

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
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"enable detects, ignore_dfs %d",
				dfs->dfs_ignore_dfs ? 1:0);
		break;
	case DFS_ENABLE_DETECT:
		dfs->dfs_proc_phyerr |= DFS_RADAR_EN;
		dfs->dfs_proc_phyerr |= DFS_SECOND_SEGMENT_RADAR_EN;
		dfs->dfs_ignore_dfs = 0;
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS
				, "enable detects, ignore_dfs %d",
				dfs->dfs_ignore_dfs ? 1:0);
		break;
	case DFS_DISABLE_FFT:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"TODO disable FFT val=0x%x", val);
		break;
	case DFS_ENABLE_FFT:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"TODO enable FFT val=0x%x", val);
		break;
	case DFS_SET_DEBUG_LEVEL:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->dfs_debug_mask = *(uint32_t *)indata;

		/* Do not allow user to set the ALWAYS/MAX bit.
		 * It will be used internally  by dfs print macro(s)
		 * to print messages when dfs is NULL.
		 */
		dfs->dfs_debug_mask &= ~(WLAN_DEBUG_DFS_ALWAYS);

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"debug level now = 0x%x", dfs->dfs_debug_mask);
		if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS3) {
			/* Enable debug Radar Event */
			dfs->dfs_event_log_on = 1;
		} else if ((utils_get_dfsdomain(dfs->dfs_pdev_obj) ==
		    DFS_FCC_DOMAIN) &&
		    lmac_is_host_dfs_check_support_enabled(dfs->dfs_pdev_obj)) {
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
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"false RSSI threshold now = 0x%x",
				dfs->wlan_dfs_false_rssi_thres);
		break;
	case DFS_SET_PEAK_MAG:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->wlan_dfs_peak_mag = *(uint32_t *)indata;
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"peak_mag now = 0x%x",
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
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"dfs timeout = %d", dfs->dfs_cac_valid_time);
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

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"ignore cac = 0x%x", dfs->dfs_ignore_cac);
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

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "nol timeout = %d sec",
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
		*((uint32_t *)outdata) = dfs->dfs_use_nol;

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"#Phyerr=%d, #false detect=%d, #queued=%d",
				 dfs->dfs_phyerr_count,
				dfs->dfs_phyerr_reject_count,
				dfs->dfs_phyerr_queued_count);

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"dfs_phyerr_freq_min=%d, dfs_phyerr_freq_max=%d",
				 dfs->dfs_phyerr_freq_min,
				dfs->dfs_phyerr_freq_max);

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Total radar events detected=%d, entries in the radar queue follows:",
				 dfs->dfs_event_log_count);

		for (i = 0; (i < DFS_EVENT_LOG_SIZE) &&
				(i < dfs->dfs_event_log_count); i++) {
#define FREQ_OFFSET1 ((int)dfs->radar_log[i].freq_offset_khz / 1000)
#define FREQ_OFFSET2 ((int)abs(dfs->radar_log[i].freq_offset_khz) % 1000)
			dfs_debug(dfs, WLAN_DEBUG_DFS,
					"ts=%llu diff_ts=%u rssi=%u dur=%u, is_chirp=%d, seg_id=%d, sidx=%d, freq_offset=%d.%dMHz, peak_mag=%d, total_gain=%d, mb_gain=%d, relpwr_db=%d, delta_diff=%d, delta_peak=%d, psidx_diff=%d\n",
					dfs->radar_log[i].ts,
					dfs->radar_log[i].diff_ts,
					dfs->radar_log[i].rssi,
					dfs->radar_log[i].dur,
					dfs->radar_log[i].is_chirp,
					dfs->radar_log[i].seg_id,
					dfs->radar_log[i].sidx,
					FREQ_OFFSET1,
					FREQ_OFFSET2,
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
		break;
	case DFS_SET_USENOL:
		if (insize < sizeof(uint32_t) || !indata) {
			error = -EINVAL;
			break;
		}
		dfs->dfs_use_nol = *(uint32_t *)indata;
		usenol_pdev_param = dfs->dfs_use_nol;
		if (dfs->dfs_is_offload_enabled) {
			if (dfs->dfs_use_nol ==
				USENOL_ENABLE_NOL_HOST_DISABLE_NOL_FW)
				usenol_pdev_param = DISABLE_NOL_FW;
			tgt_dfs_send_usenol_pdev_param(dfs->dfs_pdev_obj,
						       usenol_pdev_param);
		}
		break;
	case DFS_SET_DISABLE_RADAR_MARKING:
		if (dfs->dfs_is_offload_enabled &&
		    (utils_get_dfsdomain(dfs->dfs_pdev_obj) ==
			 DFS_FCC_DOMAIN)) {
			if (insize < sizeof(uint32_t) || !indata) {
				error = -EINVAL;
				break;
			}
			dfs_set_disable_radar_marking(dfs, *(uint8_t *)indata);
		}
		break;
	case DFS_GET_DISABLE_RADAR_MARKING:
		if (!outdata || !outsize || *outsize < sizeof(uint8_t)) {
			error = -EINVAL;
			break;
		}
		if (dfs->dfs_is_offload_enabled) {
			*outsize = sizeof(uint8_t);
			*((uint8_t *)outdata) =
				dfs_get_disable_radar_marking(dfs);
		}
		break;
	case DFS_GET_NOL:
		if (!outdata || !outsize ||
				*outsize < sizeof(struct dfsreq_nolinfo)) {
			error = -EINVAL;
			break;
		}
		*outsize = sizeof(struct dfsreq_nolinfo);
		nol = (struct dfsreq_nolinfo *)outdata;
		DFS_GET_NOL_LOCKED(dfs,
				(struct dfsreq_nolelem *)nol->dfs_nol,
				&nol->dfs_ch_nchans);
		DFS_PRINT_NOL_LOCKED(dfs);
		break;
	case DFS_SET_NOL:
		if (insize < sizeof(struct dfsreq_nolinfo) || !indata) {
			error = -EINVAL;
			break;
		}
		nol = (struct dfsreq_nolinfo *) indata;
		dfs_set_nol(dfs,
				(struct dfsreq_nolelem *)nol->dfs_nol,
				nol->dfs_ch_nchans);
		break;
	case DFS_SHOW_NOL:
		DFS_PRINT_NOL_LOCKED(dfs);
		break;
	case DFS_SHOW_NOLHISTORY:
		dfs_print_nolhistory(dfs);
		break;
	case DFS_SHOW_PRECAC_LISTS:
		dfs_print_precaclists(dfs);
		dfs_print_etsi_precaclists(dfs);
		break;
	case DFS_RESET_PRECAC_LISTS:
		dfs_reset_precac_lists(dfs);
		dfs_reset_etsi_precac_lists(dfs);
		break;
	default:
		error = -EINVAL;
	}

bad:
	return error;
}

void dfs_set_current_channel(struct wlan_dfs *dfs,
		uint16_t dfs_ch_freq,
		uint64_t dfs_ch_flags,
		uint16_t dfs_ch_flagext,
		uint8_t dfs_ch_ieee,
		uint8_t dfs_ch_vhtop_ch_freq_seg1,
		uint8_t dfs_ch_vhtop_ch_freq_seg2)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	dfs->dfs_curchan->dfs_ch_freq = dfs_ch_freq;
	dfs->dfs_curchan->dfs_ch_flags = dfs_ch_flags;
	dfs->dfs_curchan->dfs_ch_flagext = dfs_ch_flagext;
	dfs->dfs_curchan->dfs_ch_ieee = dfs_ch_ieee;
	dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1 = dfs_ch_vhtop_ch_freq_seg1;
	dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2 = dfs_ch_vhtop_ch_freq_seg2;
}

void dfs_update_cur_chan_flags(struct wlan_dfs *dfs,
		uint64_t flags,
		uint16_t flagext)
{
	dfs->dfs_curchan->dfs_ch_flags = flags;
	dfs->dfs_curchan->dfs_ch_flagext = flagext;
}
