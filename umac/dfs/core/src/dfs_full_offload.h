/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 *
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
 * DOC: dfs_full_offload.h
 * This file contains full offload specific dfs interfaces
 */

#ifndef _DFS_FULL_OFFLOAD_H_
#define _DFS_FULL_OFFLOAD_H_

#if defined(WLAN_DFS_FULL_OFFLOAD)

/**
 * dfs_fill_emulate_bang_radar_test() - Update dfs unit test arguments and
 * send bangradar command to firmware.
 * @dfs: Pointer to wlan_dfs structure.
 * @segid: Segment Identifier(Primary and Secondary)
 * @is_chirp: Boolean to determine if Chirp or Non Chirp.
 * @freq_offset: Value of frequency offset from centre frequency.
 * @dfs_unit_test: Pointer to Unit test command structure
 *
 * Return: If the event is received return 0.
 */
int dfs_fill_emulate_bang_radar_test(struct wlan_dfs *dfs,
		uint8_t segid, bool is_chirp, int32_t freq_offset,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test);

#else
static inline int dfs_fill_emulate_bang_radar_test(struct wlan_dfs *dfs,
		uint8_t segid, bool is_chirp, int32_t freq_offset,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test)
{
	return 0;
}
#endif
#endif /* _DFS_FULL_OFFLOAD_H_ */
