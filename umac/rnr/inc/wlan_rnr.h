/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_RNR_H_
#define _WLAN_RNR_H_
#include <qdf_atomic.h>
#include <wlan_objmgr_pdev_obj.h>

/**
 * struct rnr_global_info - Global context for RNR
 * @vdev_lower_band_cnt:    5ghz/2ghz vdev count
 * @vdev_6ghz_band_cnt:     6ghz vdev count
 * @pdev_6ghz_ctx:          6Ghz pdev context
 */
struct rnr_global_info {
	qdf_atomic_t vdev_lower_band_cnt;
	qdf_atomic_t vdev_6ghz_band_cnt;
	uint32_t rnr_mbss_idx_map;
	struct wlan_objmgr_pdev *pdev_6ghz_ctx;
};

/**
 * wlan_rnr_lower_band_vdev_inc - Atomic increment of
 *				  global lower band vdev counter
 *
 * API to increment global lower band vdev counter
 *
 * Return:void
 */
void wlan_rnr_lower_band_vdev_inc(void);

/**
 * wlan_rnr_lower_band_vdev_dec - Atomic decrement of
 *				  global lower band vdev counter
 *
 * API to decrement global lower band vdev counter
 *
 * Return:void
 */
void wlan_rnr_lower_band_vdev_dec(void);

/**
 * wlan_rnr_6ghz_vdev_inc - Atomic increment of
 *			    6ghz vdev counter
 *
 * API to increment of 6Ghz vdev counter
 *
 * Return:void
 */
void wlan_rnr_6ghz_vdev_inc(void);

/**
 * wlan_rnr_6ghz_vdev_dec - Atomic decrement of
 *			    6ghz vdev counter
 *
 * API to decrement of 6Ghz vdev counter
 *
 * Return:void
 */
void wlan_rnr_6ghz_vdev_dec(void);

/**
 * wlan_global_6ghz_pdev_set - Store 6Ghz pdev in
 *			       global context
 *
 * API to save 6Ghz pdev in global context for
 * faster access
 *
 * Return:void
 */
void wlan_global_6ghz_pdev_set(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_global_6ghz_pdev_destroy - Delete 6Ghz pdev in
 *				   global context
 *
 * API to delete 6Ghz pdev in global context for
 * faster access
 *
 * Return:void
 */
void wlan_global_6ghz_pdev_destroy(void);

/**
 * wlan_lower_band_ap_cnt_get - Get lower band AP count
 *
 * API to get lower band vdev from global context for
 * faster access
 *
 * Return: int32_t
 */
int32_t wlan_lower_band_ap_cnt_get(void);

/**
 * wlan_rnr_init_cnt - Initialize counters for
 *			6Ghz vdev and lower band vdev
 *
 * API to initialize atomic counters used for 6Ghz vdev
 * and lower band vdev
 *
 * Return: void
 */
void wlan_rnr_init_cnt(void);

/**
 * wlan_gbl_6ghz_pdev_get - Retrieve 6Ghz pdev pointer
 *
 * API to get 6Ghz pdev pointer
 *
 * Return: struct wlan_objmgr_pdev
 */
struct wlan_objmgr_pdev *wlan_gbl_6ghz_pdev_get(void);

/**
 * wlan_rnr_set_bss_idx - Set bit corresponding to bss index
 *
 * API to set bss index bitmap for adding Non Tx APs
 * not included in Mbss IE in the RNR IE
 *
 * Return: void
 */
void wlan_rnr_set_bss_idx(uint32_t bss_idx);

/**
 * wlan_rnr_get_bss_idx - Get bit corresponding to bss index
 *
 * API to Get bss index bitmap for adding Non Tx APs
 * not included in Mbss IE in the RNR IE
 *
 * Return: void
 */
uint32_t wlan_rnr_get_bss_idx(void);

/**
 * wlan_rnr_clear_bss_idx - Clear bits corresponding to bss index map
 *
 * API to clear bss index bitmap for adding Non Tx APs
 * not included in Mbss IE in the RNR IE
 *
 * Return: void
 */
void wlan_rnr_clear_bss_idx(void);

#endif /* End of _WLAN_RNR_H_ */
