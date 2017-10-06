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
 * @ext_chan:                Can radar be detected on the extension chan?
 * @combined_rssi:           Can use combined radar RSSI?
 * @use_enhancement:         This flag is used to indicate if radar
 *                           detection scheme should use enhanced chirping
 *                           detection algorithm. This flag also determines
 *                           if certain radar data should be discarded to
 *                           minimize false detection of radar.
 * @strong_signal_diversiry: Strong Signal fast diversity count.
 * @chip_is_bb_tlv:          Chip is BB TLV?
 * @chip_is_over_sampled:    Is Over sampled.
 * @chip_is_ht160:           IS VHT160?
 * @chip_is_false_detect:    Is False detected?
 * @fastdiv_val:             Goes with wlan_strong_signal_diversiry: If we
 *                           have fast diversity capability, read off
 *                           Strong Signal fast diversity count set in the
 *                           ini file, and store so we can restore the
 *                           value when radar is disabled.
 */
void lmac_get_caps(struct wlan_objmgr_pdev *pdev,
		bool *ext_chan,
		bool *combined_rssi,
		bool *use_enhancement,
		bool *strong_signal_diversiry,
		bool *chip_is_bb_tlv,
		bool *chip_is_over_sampled,
		bool *chip_is_ht160,
		bool *chip_is_false_detect,
		uint32_t *fastdiv_val);

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
 * @pe_firpwr:  FIR pwr out threshold.
 * @pe_rrssi:   Radar rssi thresh.
 * @pe_height:  Pulse height thresh.
 * @pe_prssi:   Pulse rssi thresh.
 * @pe_inband:  Inband thresh.
 * @pe_relpwr:  Relative power threshold in 0.5dB steps.
 * @pe_relstep: Pulse Relative step threshold in 0.5dB steps.
 * @pe_maxlen:  Max length of radar sign in 0.8us units.
 * @dfsdomain:  DFS domain.
 */
void lmac_dfs_enable(struct wlan_objmgr_pdev *pdev,
		int *is_fastclk,
		int32_t pe_firpwr,
		int32_t pe_rrssi,
		int32_t pe_height,
		int32_t pe_prssi,
		int32_t pe_inband,
		uint32_t pe_relpwr,
		uint32_t pe_relstep,
		uint32_t pe_maxlen,
		int dfsdomain);

/**
 * lmac_dfs_get_thresholds() - Get thresholds.
 * @pdev: Pointer to PDEV structure.
 * @pe_firpwr:     FIR pwr out threshold.
 * @pe_rrssi:      Radar rssi thresh.
 * @pe_height:     Pulse height thresh.
 * @pe_prssi:      Pulse rssi thresh.
 * @pe_inband:     Inband thresh.
 * @pe_relpwr:     Relative power threshold in 0.5dB steps.
 * @pe_relstep:    Pulse Relative step threshold in 0.5dB steps.
 * @pe_maxlen:     Max length of radar sign in 0.8us units.
 */
void lmac_dfs_get_thresholds(struct wlan_objmgr_pdev *pdev,
	int32_t *pe_firpwr,
	int32_t *pe_rrssi,
	int32_t *pe_height,
	int32_t *pe_prssi,
	int32_t *pe_inband,
	uint32_t *pe_relpwr,
	uint32_t *pe_relstep,
	uint32_t *pe_maxlen);

/**
 * lmac_is_mode_offload() - Check the radio for offload.
 * @pdev: Pointer to PDEV structure.
 */
bool lmac_is_mode_offload(struct wlan_objmgr_pdev *pdev);

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

#endif /* _WLAN_DFS_LMAC_API_H_ */
