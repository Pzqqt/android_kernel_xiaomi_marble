/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: This file init/deint functions for DFS module.
 */

#ifndef _WLAN_DFS_INIT_DEINIT_API_H_
#define _WLAN_DFS_INIT_DEINIT_API_H_

#include "wlan_dfs_ucfg_api.h"

/**
 * wlan_pdev_get_dfs_obj() - Get DFS object from PDEV.
 * @pdev: Pointer to PDEV structure.
 * @id: DFS component ID.
 * @obj: Pointer to DFS object.
 */
struct wlan_dfs *wlan_pdev_get_dfs_obj(struct wlan_objmgr_pdev *pdev);

/**
 * register_dfs_callbacks() - Fill mlme pointers.
 */
void register_dfs_callbacks(void);

/**
 * dfs_init() - Init DFS module
 */
QDF_STATUS dfs_init(void);

/**
 * dfs_deinit() - Deinit DFS module.
 */
QDF_STATUS dfs_deinit(void);

/**
 * wlan_dfs_pdev_obj_create_notification() - DFS pdev object create handler.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS wlan_dfs_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
		void *arg);

/**
 * wlan_dfs_pdev_obj_destroy_notification() - DFS pdev object delete handler.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS wlan_dfs_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
		void *arg);

/**
 * wifi_dfs_psoc_enable() - handles registering dfs event handlers.
 * @psoc: psoc object.
 */
QDF_STATUS wifi_dfs_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_dfs_psoc_disable() - handles deregistering dfs event handlers.
 * @psoc: psoc object.
 */
QDF_STATUS wifi_dfs_psoc_disable(struct wlan_objmgr_psoc *psoc);

#endif /* _WLAN_DFS_INIT_DEINIT_API_H_ */
