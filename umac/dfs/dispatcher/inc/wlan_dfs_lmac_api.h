/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: These APIs are used by DFS core functions to call lmac/offload
 * functions.
 */

#ifndef _WLAN_DFS_LMAC_API_H_
#define _WLAN_DFS_LMAC_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>

/**
 * lmac_get_caps() - Get DFS capabilities.
 * @pdev: Pointer to PDEV structure.
 * @dfs_caps: Pointer to dfs_caps structure
 */
void lmac_get_caps(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_caps *dfs_caps);

/**
 * lmac_get_tsf64() - Get tsf64 value.
 * @pdev: Pointer to PDEV structure.
 *
 * Return: tsf64 timer value.
 */
uint64_t lmac_get_tsf64(struct wlan_objmgr_pdev *pdev);

/**
 * lmac_dfs_disable() - Disable DFS.
 * @pdev: Pointer to PDEV structure.
 * @no_cac: no_cac flag.
 */
void lmac_dfs_disable(struct wlan_objmgr_pdev *pdev, int no_cac);

/**
 * lmac_dfs_enable() - Enable DFS.
 * @pdev: Pointer to PDEV structure.
 * @is_fastclk: fastclk value.
 * @param: Pointer to wlan_dfs_phyerr_param structure.
 * @dfsdomain:  DFS domain.
 */
void lmac_dfs_enable(struct wlan_objmgr_pdev *pdev,
		int *is_fastclk,
		struct wlan_dfs_phyerr_param *param,
		int dfsdomain);

/**
 * lmac_dfs_get_thresholds() - Get thresholds.
 * @pdev: Pointer to PDEV structure.
 * @param: Pointer to wlan_dfs_phyerr_param structure.
 */
void lmac_dfs_get_thresholds(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_phyerr_param *param);

/**
 * lmac_get_ah_devid() - Get ah devid.
 * @pdev: Pointer to PDEV structure.
 */
uint16_t lmac_get_ah_devid(struct wlan_objmgr_pdev *pdev);

/**
 * lmac_get_ext_busy() - Get ext_busy.
 * @pdev: Pointer to PDEV structure.
 */
uint32_t lmac_get_ext_busy(struct wlan_objmgr_pdev *pdev);

/**
 * lmac_set_use_cac_prssi() - Set use_cac_prssi value.
 * @pdev: Pointer to PDEV structure.
 */
void lmac_set_use_cac_prssi(struct wlan_objmgr_pdev *pdev);

/**
 * lmac_get_target_type() - Get target type.
 * @pdev: Pointer to PDEV structure.
 */
uint32_t lmac_get_target_type(struct wlan_objmgr_pdev *pdev);

/**
 * lmac_get_phymode_info() - Get phymode info.
 * @pdev: Pointer to PDEV structure.
 */
uint32_t lmac_get_phymode_info(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode);

/**
 * lmac_is_host_dfs_check_support_enabled() - Check if Host DFS confirmation
 * feature is supported.
 * @pdev: Pointer to PDEV structure.
 *
 * Return: true, host dfs check supported, else false.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
bool lmac_is_host_dfs_check_support_enabled(struct wlan_objmgr_pdev *pdev);
#else
static inline bool lmac_is_host_dfs_check_support_enabled(
		struct wlan_objmgr_pdev *pdev)
{
	return false;
}
#endif
#endif /* _WLAN_DFS_LMAC_API_H_ */
