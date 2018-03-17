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
 * DOC: target_if_dfs_partial_offload.h
 * This file contains dfs target interface for partial offload.
 */

#ifndef _TARGET_IF_DFS_PARTIAL_OFFLOAD_H_
#define _TARGET_IF_DFS_PARTIAL_OFFLOAD_H_

/**
 * target_if_dfs_reg_phyerr_events() - register phyerror events.
 * @psoc: Pointer to psoc object.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
QDF_STATUS target_if_dfs_reg_phyerr_events(struct wlan_objmgr_psoc *psoc);
#else
static QDF_STATUS
target_if_dfs_reg_phyerr_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * target_if_dfs_get_caps() - get dfs caps.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_caps: Pointer to dfs_caps structure.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
QDF_STATUS target_if_dfs_get_caps(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_caps *dfs_caps);
#else
static QDF_STATUS target_if_dfs_get_caps(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_caps *dfs_caps)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _TARGET_IF_DFS_PARTIAL_OFFLOAD_H_ */
