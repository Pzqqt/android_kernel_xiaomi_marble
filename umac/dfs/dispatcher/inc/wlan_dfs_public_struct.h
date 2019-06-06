/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_dfs_public_struct.h
 * This file contains DFS data structures
 */

#ifndef __WLAN_DFS_PUBLIC_STRUCT_H_
#define __WLAN_DFS_PUBLIC_STRUCT_H_

/* TODO: This structure has many redundant variables, needs cleanup */
/**
 * struct radar_found_info - radar found info
 * @pdev_id:        pdev id.
 * @detection_mode: 0 indicates RADAR detected, non-zero indicates debug mode.
 * @freq_offset:    frequency offset.
 * @chan_width:     channel width.
 * @detector_id:    detector id for full-offload.
 * @segment_id:     segment id (same as detector_id) for partial-offload.
 * @timestamp:      timestamp (Time when filter match is found in Firmware).
 * @is_chirp:       is chirp or not.
 * @chan_freq:      channel frequency (Primary channel frequency).
 * @radar_freq:     radar frequency (Is it same as '@chan_freq'?).
 * @sidx:           sidx value (same as freq_offset).
 */
struct radar_found_info {
	uint32_t pdev_id;
	uint32_t detection_mode;
	int32_t freq_offset;
	uint32_t chan_width;
	uint32_t detector_id;
	uint32_t segment_id;
	uint32_t timestamp;
	uint32_t is_chirp;
	uint32_t chan_freq;
	uint32_t radar_freq;
	int32_t  sidx;
};

/**
 * struct dfs_acs_info - acs info, ch range
 * @acs_mode: to enable/disable acs 1/0.
 * @channel_list: channel list in acs config
 * @num_of_channel: number of channel in ACS channel list
 */
struct dfs_acs_info {
	uint8_t acs_mode;
	uint8_t *channel_list;
	uint8_t num_of_channel;
};

/**
 * struct radar_event_info - radar event info.
 * @pulse_is_chirp: flag to indicate if this pulse is chirp.
 * @pulse_center_freq: the center frequency of the radar pulse detected, KHz.
 * @pulse_duration: the duaration of the pulse in us.
 * @rssi: RSSI recorded in the ppdu.
 * @pulse_detect_ts: timestamp indicates the time when DFS pulse is detected.
 * @upload_fullts_low: low 32 tsf timestamp get from MAC tsf timer indicates
 *  the time that the radar event uploading to host.
 * @upload_fullts_high: high 32 tsf timestamp get from MAC tsf timer indicates
 *  the time that the radar event uploading to host.
 * @peak_sidx: index of peak magnitude bin (signed)
 * @pdev_id: pdev_id for identifying the MAC.
 * @delta_diff: Delta diff value.
 * @delta_peak: Delta peak value.
 * @psidx_diff: Psidx diff value.
 * @is_psidx_diff_valid: Does fw send valid psidx diff.
 */
struct radar_event_info {
	uint8_t  pulse_is_chirp;
	uint32_t pulse_center_freq;
	uint32_t pulse_duration;
	uint8_t  rssi;
	uint32_t pulse_detect_ts;
	uint32_t upload_fullts_low;
	uint32_t upload_fullts_high;
	int32_t  peak_sidx;
	uint8_t  pdev_id;
	uint8_t  delta_diff;
	int8_t   delta_peak;
	int8_t   psidx_diff;
	int8_t   is_psidx_diff_valid;
};

/**
 * struct dfs_user_config - user configuration required for for DFS.
 * @dfs_is_phyerr_filter_offload: flag to indicate DFS phyerr filtering offload.
 */
struct dfs_user_config {
	bool dfs_is_phyerr_filter_offload;
};

/**
 * struct dfs_radar_found_params - radar found parameters.
 * @pri_min: Minimum PRI of detected radar pulse.
 * @pri_max: Max PRI of detected radar pulse.
 * @duration_min: Min duration of detected pulse in us.
 * @duration_max: Max duration of detected pulse in us.
 * @sidx_min: Min softare index of detected radar pulse.
 * @sidx_max: Max software index of detected radar pulse.
 */
struct dfs_radar_found_params {
	u_int32_t pri_min;
	u_int32_t pri_max;
	u_int32_t duration_min;
	u_int32_t duration_max;
	u_int32_t sidx_min;
	u_int32_t sidx_max;
};
#endif
