/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * struct radar_found_info - radar found info
 * @pdev_id: pdev id.
 * @detection_mode: In-service mode or O-CAC mod.
 * @freq_offset: frequency offset.
 * @chan_width: channel width.
 * @detector_id: detector id.
 * @segment_id: segment id.
 * @timestamp: timestamp.
 * @is_chirp: is chirp or not.
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
};

/**
 * struct dfs_acs_info - acs info, ch range
 * @acs_mode: to enable/disable acs 1/0.
 * @start_ch: start channel number, ignore all channels before.
 * @end_ch: end channel number, ignore all channels after.
 */
struct dfs_acs_info {
	uint8_t acs_mode;
	uint8_t start_ch;
	uint8_t end_ch;
};
#endif
