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
 * DOC: This file has the DFS dispatcher API which is exposed to outside of DFS
 * component.
 */

#ifndef _WLAN_DFS_UTILS_API_H_
#define _WLAN_DFS_UTILS_API_H_

#include "wlan_dfs_ucfg_api.h"
#include "wlan_reg_services_api.h"

/* Add channel to nol */
#define DFS_NOL_SET                  1

/* Remove channel from nol */
#define DFS_NOL_RESET                0

/* Max nol channels */
#define DFS_MAX_NOL_CHANNEL         80

/* WLAN 2.4GHz start freq */
#define DFS_24_GHZ_BASE_FREQ     (2407)

/* WLAN 5GHz start freq */
#define DFS_5_GHZ_BASE_FREQ      (5000)

/* WLAN 2.4 GHz channel number 6 */
#define DFS_24_GHZ_CHANNEL_6        (6)

/* WLAN 2.4 GHz channel number 14 */
#define DFS_24_GHZ_CHANNEL_14      (14)

/* WLAN 2.4 GHz channel number 15 */
#define DFS_24_GHZ_CHANNEL_15      (15)

/* WLAN 2.4 GHz channel number 27 */
#define DFS_24_GHZ_CHANNEL_27      (27)

/* WLAN 5GHz channel number 170 */
#define DFS_5_GHZ_CHANNEL_170     (170)

/* WLAN 5MHz channel spacing */
#define DFS_CHAN_SPACING_5MHZ       (5)

/* WLAN 20Hz channel spacing */
#define DFS_CHAN_SPACING_20MHZ     (20)

/* WLAN 2.4GHz channel number 14 freq */
#define DFS_CHAN_14_FREQ         (2484)

/* WLAN 2.4GHz channel number 15 freq */
#define DFS_CHAN_15_FREQ         (2512)

/* WLAN 5GHz channel number 170 freq */
#define DFS_CHAN_170_FREQ        (5852)



extern struct dfs_to_mlme global_dfs_to_mlme;

/**
 * utils_dfs_cac_valid_reset() - Cancels the dfs_cac_valid_timer timer.
 * @pdev: Pointer to DFS pdev object.
 * @prevchan_ieee: Prevchan number.
 * @prevchan_flags: Prevchan flags.
 *
 * Wrapper function for dfs_cac_valid_reset(). This function called from
 * outside of DFS component.
 */

QDF_STATUS utils_dfs_cac_valid_reset(struct wlan_objmgr_pdev *pdev,
		uint8_t prevchan_ieee,
		uint32_t prevchan_flags);

/**
 * utils_dfs_reset() - Reset DFS members.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS utils_dfs_reset(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_reset_precaclists() - Clears and initiakizes precac_required_list,
 *                                 precac_done_list and precac_nol_list.
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for dfs_reset_precaclists(). This function called from
 * outside of DFS component.
 */
QDF_STATUS utils_dfs_reset_precaclists(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_cancel_precac_timer() - Cancel the precac timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_cancel_precac_timer(). this function called from
 * outside of dfs component.
 */
QDF_STATUS utils_dfs_cancel_precac_timer(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_is_precac_done() - Is precac done.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_is_precac_done(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_is_precac_done(struct wlan_objmgr_pdev *pdev,
		bool *is_precac_done);

/**
 * utils_dfs_cancel_cac_timer() - Cancels the CAC timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_cancel_cac_timer(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_cancel_cac_timer(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_start_cac_timer() - Starts the CAC timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_start_cac_timer(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_start_cac_timer(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_cac_stop() - Clear the AP CAC timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_cac_stop(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_cac_stop(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_stacac_stop() - Clear the STA CAC timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_stacac_stop(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_stacac_stop(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_get_usenol() - Returns use_nol flag.
 * @pdev: Pointer to DFS pdev object.
 * @usenol: Pointer to usenol value.
 *
 * wrapper function for dfs_get_usenol(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_get_usenol(struct wlan_objmgr_pdev *pdev,
		uint16_t *usenol);

/**
 * utils_dfs_radar_disable() - Disables the radar.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for  dfs_radar_disable(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_radar_disable(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_set_update_nol_flag() - Sets update_nol flag.
 * @pdev: Pointer to DFS pdev object.
 * @val: update_nol flag.
 *
 * wrapper function for dfs_set_update_nol_flag(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_set_update_nol_flag(struct wlan_objmgr_pdev *pdev,
		bool val);

/**
 * utils_dfs_get_update_nol_flag() - Returns update_nol flag.
 * @pdev: Pointer to DFS pdev object.
 * @nol_flag: Fill nol_flag in this variable.
 *
 * wrapper function for dfs_get_update_nol_flag(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_get_update_nol_flag(struct wlan_objmgr_pdev *pdev,
		bool *nol_flag);

/**
 * utils_dfs_get_dfs_use_nol() - Get usenol.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_use_nol: Pointer to dfs_use_nol.
 *
 * wrapper function for dfs_get_dfs_use_nol(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_get_dfs_use_nol(struct wlan_objmgr_pdev *pdev,
		int *dfs_use_nol);

/**
 * utils_dfs_get_nol_timeout() - Get NOL timeout.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_nol_timeout: Pointer to dfs_nol_timeout.
 *
 * wrapper function for dfs_get_nol_timeout(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_get_nol_timeout(struct wlan_objmgr_pdev *pdev,
		int *dfs_nol_timeout);

/**
 * utils_dfs_nol_addchan() - Add channel to NOL.
 * @pdev: Pointer to DFS pdev object.
 * @chan: channel t o add NOL.
 * @dfs_nol_timeout: NOL timeout.
 *
 * wrapper function for dfs_nol_addchan(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_nol_addchan(struct wlan_objmgr_pdev *pdev,
		uint16_t freq,
		uint32_t dfs_nol_timeout);

/**
 * utils_dfs_nol_update() - NOL update
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_nol_update(). this
 * function called from outside of dfs component.
 */
QDF_STATUS utils_dfs_nol_update(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_second_segment_radar_disable() - Disables the second segment radar.
 * @pdev: Pointer to DFS pdev object.
 *
 * This is called when AP detects the radar, to (potentially) disable
 * the radar code.
 */
QDF_STATUS utils_dfs_second_segment_radar_disable(
		struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_is_ignore_dfs() - Get Ignore DFS value.
 * @pdev: Pointer to DFS pdev object.
 * @ignore_dfs: Fill ignore_dfs value in this variable.
 */
QDF_STATUS utils_dfs_is_ignore_dfs(struct wlan_objmgr_pdev *pdev,
		bool *ignore_dfs);

/**
 * utils_dfs_is_cac_valid() - Gets the value of is_cac_valid.
 * @pdev: Pointer to DFS pdev object.
 * @is_cac_valid: Fill is_cac_valid in this variable.
 */
QDF_STATUS utils_dfs_is_cac_valid(struct wlan_objmgr_pdev *pdev,
		bool *is_cac_valid);

/**
 * utils_dfs_is_ignore_cac() - Gets the value of is_ignore_cac.
 * @pdev: Pointer to DFS pdev object.
 * @ignore_cac: Fill ignore_cac value in this variable.
 */
QDF_STATUS utils_dfs_is_ignore_cac(struct wlan_objmgr_pdev *pdev,
		bool *ignore_cac);

/**
 * utils_dfs_set_cac_timer_running() - Sets the cac timer running.
 * @pdev: Pointer to DFS pdev object.
 * @val: Set this value to dfs_cac_timer_running variable.
 */
QDF_STATUS utils_dfs_set_cac_timer_running(struct wlan_objmgr_pdev *pdev,
		int val);

/**
 * utils_dfs_get_nol_chfreq_and_chwidth() - Sets the cac timer running.
 * @pdev: Pointer to DFS pdev object.
 * @nollist: Pointer to NOL channel entry.
 * @nol_chfreq: Pointer to save channel frequency.
 * @nol_chwidth: Pointer to save channel width.
 * @index: Index into nol list.
 */
QDF_STATUS utils_dfs_get_nol_chfreq_and_chwidth(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		uint32_t *nol_chfreq,
		uint32_t *nol_chwidth,
		int index);

/**
 * utils_dfs_get_random_channel() - Get random channel.
 * @pdev: Pointer to DFS pdev object.
 * @flags: random channel selection flags.
 * @ch_params: current channel params.
 * @hw_mode: current operating mode.
 * @target_chan: Pointer to target_chan.
 * @acs_info: acs range info.
 *
 * wrapper function for get_random_chan(). this
 * function called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS utils_dfs_get_random_channel(struct wlan_objmgr_pdev *pdev,
		uint16_t flags, struct ch_params *ch_params,
		uint32_t *hw_mode, uint8_t *target_chan,
		struct dfs_acs_info *acs_info);

/**
 * utils_dfs_init_nol() - Initialize nol from platform driver.
 * @pdev: pdev handler.
 *
 * Initialize nol from platform driver.
 *
 * Return: None
 */
#ifdef QCA_DFS_NOL_PLATFORM_DRV_SUPPORT
void utils_dfs_init_nol(struct wlan_objmgr_pdev *pdev);
#else
static inline void utils_dfs_init_nol(struct wlan_objmgr_pdev *pdev)
{
}
#endif
/**
 * utils_dfs_save_nol() - save nol list to platform driver.
 * @pdev: pdev handler.
 *
 * Save nol list to platform driver.
 *
 * Return: None
 */
void utils_dfs_save_nol(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_print_nol_channels() - log nol channels.
 * @pdev: pdev handler.
 *
 * log nol channels.
 *
 * Return: None
 */
#ifdef DFS_COMPONENT_ENABLE
void utils_dfs_print_nol_channels(struct wlan_objmgr_pdev *pdev);
#else
static inline void utils_dfs_print_nol_channels(struct wlan_objmgr_pdev *pdev)
{
}
#endif

/**
 * utils_dfs_clear_nol_channels() - clear nol list.
 * @pdev: pdev handler.
 *
 * log nol channels.
 *
 * Return: None
 */
void utils_dfs_clear_nol_channels(struct wlan_objmgr_pdev *pdev);

/**
 * utils_is_dfs_ch() - is channel dfs.
 * @pdev: pdev handler.
 *
 * is channel dfs.
 *
 * Return: True if channel dfs, else false.
 */
static inline bool utils_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	return wlan_reg_is_dfs_ch(pdev, chan);
}
/**
 * utils_dfs_reg_update_nol_ch() - set nol channel
 *
 * @pdev: pdev ptr
 * @ch_list: channel list to be returned
 * @num_ch: number of channels
 * @nol_ch: nol flag
 *
 * Return: void
 */
void utils_dfs_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
		uint8_t *ch_list,
		uint8_t num_ch,
		bool nol_ch);

/**
 * utils_dfs_freq_to_chan () - convert channel freq to channel number
 * @freq: frequency
 *
 * Return: channel number
 */
uint8_t utils_dfs_freq_to_chan(uint32_t freq);

/**
 * utils_dfs_chan_to_freq () - convert channel number to frequency
 * @chan: channel number
 *
 * Return: frequency
 */
#ifdef DFS_COMPONENT_ENABLE
uint32_t utils_dfs_chan_to_freq(uint8_t chan);
#else
static inline uint32_t utils_dfs_chan_to_freq(uint8_t chan)
{
	return 0;
}
#endif
/**
 * utils_dfs_update_cur_chan_flags() - Update DFS channel flag and flagext.
 * @pdev: Pointer to DFS pdev object.
 * @flags: New channel flags
 * @flagext: New Extended flags
 *
 * Return: QDF_STATUS
 */
QDF_STATUS utils_dfs_update_cur_chan_flags(struct wlan_objmgr_pdev *pdev,
		uint64_t flags,
		uint16_t flagext);

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * utils_dfs_mark_leaking_ch() - to mark channel leaking in to nol
 * @pdev: Pointer to pdev structure.
 * @ch_width: channel width
 * @temp_ch_lst_sz: the target channel list
 * @temp_ch_lst: the target channel list
 *
 * This function removes the channels from temp channel list that
 * (if selected as target channel) will cause leakage in one of
 * the NOL channels
 *
 * Return: QDF_STATUS
 */
QDF_STATUS utils_dfs_mark_leaking_ch(struct wlan_objmgr_pdev *pdev,
	enum phy_ch_width ch_width,
	uint8_t temp_ch_lst_sz,
	uint8_t *temp_ch_lst);
#else
static inline QDF_STATUS utils_dfs_mark_leaking_ch
	(struct wlan_objmgr_pdev *pdev,
	enum phy_ch_width ch_width,
	uint8_t temp_ch_lst_sz,
	uint8_t *temp_ch_lst)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * utils_get_dfsdomain() - Get DFS domain.
 * @pdev: Pointer to PDEV structure.
 *
 * Return: DFS domain.
 */
int utils_get_dfsdomain(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_get_cur_rd() - Get current regdomain.
 * @pdev: pdev ptr
 *
 * Return: Regdomain pair id.
 */
uint16_t utils_dfs_get_cur_rd(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_is_spoof_check_failed() - get spoof check status.
 * @pdev: pdev ptr
 * @is_spoof_check_failed: pointer containing the status.
 *
 * Return: QDF_STATUS.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS utils_dfs_is_spoof_check_failed(struct wlan_objmgr_pdev *pdev,
					   bool *is_spoof_check_failed);
#else
static inline
QDF_STATUS utils_dfs_is_spoof_check_failed(struct wlan_objmgr_pdev *pdev,
					   bool *is_spoof_check_failed)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _WLAN_DFS_UTILS_API_H_ */
