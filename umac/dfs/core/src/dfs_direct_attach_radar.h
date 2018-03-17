/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: dfs_direct_attach_radar.h
 * This file contains direct attach specific dfs interfaces
 */

#ifndef _DFS_DIRECT_ATTACH_RADAR_H_
#define _DFS_DIRECT_ATTACH_RADAR_H_

/**
 * dfs_get_da_radars() - Initialize the RADAR table for DA.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_DIRECT_ATTACH)
void dfs_get_da_radars(struct wlan_dfs *dfs);
#else
static inline void dfs_get_da_radars(struct wlan_dfs *dfs)
{
}
#endif
#endif /* _DFS_DIRECT_ATTACH_RADAR_H_ */
