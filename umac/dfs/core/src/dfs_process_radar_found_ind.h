/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: dfs_process_radar_found_ind.h
 * This file provides prototypes of the routines needed for the
 * external components to utilize the services provided by the
 * DFS component.
 */

#ifndef _DFS_PROCESS_RADAR_FOUND_IND_H_
#define _DFS_PROCESS_RADAR_FOUND_IND_H_

/* Number of channel marking offsets */
#define DFS_NUM_FREQ_OFFSET   3

/* Lower channel from 20 Mhz center channel */
#define DFS_20MHZ_LOWER_CHANNEL(_f)    ((_f) - 20)
/* Upper channel from 20 Mhz center channel */
#define DFS_20MHZ_UPPER_CHANNEL(_f)    ((_f) + 20)
/* 1st lower channel from center channel of bandwidth 40/80/160Mhz */
#define DFS_FIRST_LOWER_CHANNEL(_f)    ((_f) - 10)
/* 2nd lower channel from center channel of bandwidth 40/80/160Mhz */
#define DFS_SECOND_LOWER_CHANNEL(_f)   ((_f) - 30)
/* 3rd lower channel from center channel of bandwidth 80/160Mhz */
#define DFS_THIRD_LOWER_CHANNEL(_f)    ((_f) - 50)
/* 1st upper channel from center channel of bandwidth 40/80/160Mhz */
#define DFS_FIRST_UPPER_CHANNEL(_f)    ((_f) + 10)
/* 2nd upper channel from center channel of bandwidth 40/80/160Mhz */
#define DFS_SECOND_UPPER_CHANNEL(_f)   ((_f) + 30)
/* 3rd upper channel from center channel of bandwidth 80/160Mhz */
#define DFS_THIRD_UPPER_CHANNEL(_f)    ((_f) + 50)

/* 20 Mhz freq_offset lower */
#define DFS_20MZ_OFFSET_LOWER    (-10)
/* 20 Mhz freq_offset upper */
#define DFS_20MZ_OFFSET_UPPER     (10)
/* 40/80 Mhz freq_offset first lower */
#define DFS_OFFSET_FIRST_LOWER    (-20)
/* 40/80 Mhz freq_offset second lower */
#define DFS_OFFSET_SECOND_LOWER   (-40)
/* 40/80 Mhz freq_offset first upper */
#define DFS_OFFSET_FIRST_UPPER     (20)
/* 40/80 Mhz freq_offset second upper */
#define DFS_OFFSET_SECOND_UPPER    (40)

/* Frequency offset to sidx */
#define DFS_FREQ_OFFSET_TO_SIDX(_f)  ((32 * (_f)) / 10)
/* Sidx to frequency offset */
#define DFS_SIDX_TO_FREQ_OFFSET(_s)  ((10 * (_s)) / 32)
/* sidx offset boundary */
#define DFS_BOUNDARY_SIDX  32
/* freq offset for chirp */
#define DFS_CHIRP_OFFSET  10
/* second segment freq offset */
#define DFS_160MHZ_SECOND_SEG_OFFSET  40

/* Frequency offset indices */
#define CENTER_CH 0
#define LEFT_CH   1
#define RIGHT_CH  2

/* Next channel number offset's from center channel number */
#define DFS_5GHZ_NEXT_CHAN_OFFSET  2
#define DFS_5GHZ_2ND_CHAN_OFFSET   6
#define DFS_5GHZ_3RD_CHAN_OFFSET  10
#define DFS_5GHZ_4TH_CHAN_OFFSET  14

/* Max number of bonding channels in 160 MHz segment */
#define NUM_CHANNELS_160MHZ 8

/**
 * struct freqs_offsets - frequency and offset information
 * @freq: channel frequency in mhz.
 * @offset: offset from center frequency.
 *
 * Index 0 - Center channel affected by RADAR.
 * Index 1 - Left of Center channel affected by RADAR.
 * Index 2 - Right of Center channel affected by RADAR.
 *
 * This information is needed to find and mark radar infected
 * channels in NOL and regulatory database.
 */
struct freqs_offsets {
	uint32_t freq[DFS_NUM_FREQ_OFFSET];
	int32_t offset[DFS_NUM_FREQ_OFFSET];
};

/**
 * dfs_process_radar_found_indication() - Process radar found indication
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: radar found info.
 *
 * Process radar found indication and update radar effected channel in NOL
 * and regulatory.
 *
 * Return: None
 */
void dfs_process_radar_found_indication(struct wlan_dfs *dfs,
		struct radar_found_info *radar_found);

/**
 * dfs_process_radar_ind() - Process radar indication event
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found_info structure.
 *
 * Wrapper function of dfs_process_radar_found_indication().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dfs_process_radar_ind(struct wlan_dfs *dfs,
		struct radar_found_info *radar_found);

/**
 * dfs_radarfound_action_generic() - The dfs action on radar detection by host
 * for domains other than FCC.
 * @dfs: Pointer to wlan_dfs structure.
 * @seg_id: segment id.
 *
 * Return: None
 */
void dfs_radarfound_action_generic(struct wlan_dfs *dfs, uint8_t seg_id);

/**
 * dfs_get_bonding_channels() - Get bonding channels.
 * @dfs: Pointer to wlan_dfs structure.
 * @curchan: Pointer to dfs_channels to know width and primary channel.
 * @segment_id: Segment id, useful for 80+80/160 MHz operating band.
 * @channels: Pointer to save radar affected channels.
 *
 * Return: Number of channels.
 */
uint8_t dfs_get_bonding_channels(struct wlan_dfs *dfs,
				 struct dfs_channel *curchan,
				 uint32_t segment_id,
				 uint8_t *channels);

/**
 * dfs_get_bonding_channels_without_seg_info() - Get bonding channels in chan
 * @chan: Pointer to dfs_channel structure.
 * @channels: channel array holding list of bonded channels.
 *
 * Return: number of sub channels in the input channel.
 */
uint8_t dfs_get_bonding_channels_without_seg_info(struct dfs_channel *chan,
						  uint8_t *channels);

/**
 * dfs_set_nol_subchannel_marking() - Set or unset NOL subchannel marking.
 * @dfs: Pointer to wlan_dfs structure.
 * @nol_subchannel_marking - Configure NOL subchannel marking.
 *
 * Return: Status of the configuration.
 */
int
dfs_set_nol_subchannel_marking(struct wlan_dfs *dfs,
			       bool nol_subchannel_marking);

/**
 * dfs_get_nol_subchannel_marking() - Get the value of NOL subchannel marking.
 * @dfs: Pointer to wlan_dfs structure.
 * @nol_subchannel_marking - Read and store the value of NOL subchannel marking
 * config.
 *
 * Return: Status of the read.
 */
int
dfs_get_nol_subchannel_marking(struct wlan_dfs *dfs,
			       bool *nol_subchannel_marking);
#endif /*_DFS_PROCESS_RADAR_FOUND_IND_H_ */
