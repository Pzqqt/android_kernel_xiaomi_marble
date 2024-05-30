/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: dfs_confirm_radar.h
 * This file contains false radar detection specific dfs interfaces.
 */

#ifndef _DFS_CONFIRM_RADAR_H_
#define _DFS_CONFIRM_RADAR_H_

#include "dfs_internal.h"

#define DFS_INVALID_PRI_LIMIT 100  /* should we use 135? */
/* The time interval between two sucessive pulses
 * in case of w53 chirp type can be as low as 50us.
 * Experimentally the PRI limit was found to be as
 * low as 15us.
 */
#define DFS_INVALID_PRI_LIMIT_MKKN 15
#define FRAC_PRI_SCORE_ARRAY_SIZE 40

/**
 * dfs_is_real_radar() - This function checks for fractional PRI and jitter in
 * sidx index to determine if the radar is real or not.
 * @dfs: Pointer to dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @ext_chan_flag: ext chan flags.
 *
 * Return : true if real RADAR else false.
 */
#ifdef CONFIG_EXT_RADAR_PROCESS
bool dfs_is_real_radar(struct wlan_dfs *dfs,
		       struct dfs_filter *rf,
		       int ext_chan_flag);
#endif /* CONFIG_EXT_RADAR_PROCESS */

/**
 * dfs_get_ext_filter() - This function gets the external radar filter table.
 * @domain: The dfs domain for which the external radar table is needed.
 * @num_radars: Output agrument. Stores the numbers of filters in the filter.
 * table.
 *
 * Return: Pointer to the radar table.
 */
#ifdef CONFIG_EXT_RADAR_PROCESS
struct dfs_pulse *dfs_get_ext_filter(enum DFS_DOMAIN domain,
				     uint8_t *num_radars);
#else
static inline struct dfs_pulse *dfs_get_ext_filter(enum DFS_DOMAIN domain,
						   uint8_t *num_radars)
{
	return NULL;
}
#endif /* CONFIG_EXT_RADAR_PROCESS */
#endif
