/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: umac/dfs/core/src/dfs_filter_init.h
 * This file contains dfs interfaces
 */

#ifndef _DFS_FILTER_INIT_H_
#define _DFS_FILTER_INIT_H_

/**
 * dfs_main_attach() - Allocates memory for wlan_dfs members.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
int dfs_main_attach(struct wlan_dfs *dfs);
#else
static inline int dfs_main_attach(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

/**
 * dfs_main_detach() - Free dfs variables.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
void dfs_main_detach(struct wlan_dfs *dfs);
#else
static inline void dfs_main_detach(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_start_host_based_bangradar() - Mark as bangradar and start
 * wlan_dfs_task_timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
int dfs_start_host_based_bangradar(struct wlan_dfs *dfs);
#else
static inline int dfs_start_host_based_bangradar(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

/**
 * dfs_main_timer_reset() - Stop dfs timers.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
void dfs_main_timer_reset(struct wlan_dfs *dfs);
#else
static inline void dfs_main_timer_reset(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_main_timer_detach() - Free dfs timers.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
void dfs_main_timer_detach(struct wlan_dfs *dfs);
#else
static inline void dfs_main_timer_detach(struct wlan_dfs *dfs)
{
}
#endif

#endif /* _DFS_FILTER_INIT_H_ */
