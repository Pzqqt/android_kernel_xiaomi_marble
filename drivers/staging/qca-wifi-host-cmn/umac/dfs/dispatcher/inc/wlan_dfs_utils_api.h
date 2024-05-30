/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_vdev_obj.h>

/* Add channel to nol */
#define DFS_NOL_SET                  1

/* Remove channel from nol */
#define DFS_NOL_RESET                0

/* Mark nol-history flag for the channel */
#define DFS_NOL_HISTORY_SET 1

/* Clear nol-history flag from the channel */
#define DFS_NOL_HISTORY_RESET 0

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
 * utils_dfs_cac_valid_reset_for_freq() - Cancels the dfs_cac_valid_timer timer.
 * @pdev: Pointer to DFS pdev object.
 * @prevchan_freq: Prevchan frequency.
 * @prevchan_flags: Prevchan flags.
 *
 * Wrapper function for dfs_cac_valid_reset_for_freq(). This function called
 * from outside of DFS component.
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS utils_dfs_cac_valid_reset_for_freq(struct wlan_objmgr_pdev *pdev,
					      uint16_t prevchan_freq,
					      uint32_t prevchan_flags);
#endif

/**
 * utils_dfs_reset() - Reset DFS members.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS utils_dfs_reset(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_is_freq_in_nol() -  check if given channel in nol list
 * @pdev: Pointer to DFS pdev object
 * @freq: channel frequency
 *
 * check if given channel in nol list.
 *
 * Return: true if channel in nol, false else
 */
bool utils_dfs_is_freq_in_nol(struct wlan_objmgr_pdev *pdev, uint32_t freq);

/**
 * utils_dfs_reset_precaclists() - Clears and initializes precac_list.
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for dfs_reset_precaclists(). This function called from
 * outside of DFS component.
 */
QDF_STATUS utils_dfs_reset_precaclists(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_unmark_precac_nol_for_freq() - Clears precac channel marked as NOL.
 * @pdev: Pointer to DFS pdev object.
 * @chan_freq: channel freq to be unmarked as NOL.
 *
 * Return void.
 */
#ifdef CONFIG_CHAN_FREQ_API
void utils_dfs_unmark_precac_nol_for_freq(struct wlan_objmgr_pdev *pdev,
					  uint16_t chan_freq);
#endif

/**
 * utils_dfs_cancel_precac_timer() - Cancel the precac timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for dfs_cancel_precac_timer(). this function called from
 * outside of dfs component.
 */
QDF_STATUS utils_dfs_cancel_precac_timer(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_start_precac_timer() - Start the precac timer.
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for dfs_start_precac_timer(). This function called from
 * outside of dfs component.
 *
 * Return:
 * * QDF_STATUS_E_FAILURE: Failed to start timer.
 * * QDF_STATUS_SUCCESS: Timer started successfully.
 */
QDF_STATUS utils_dfs_start_precac_timer(struct wlan_objmgr_pdev *pdev);

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * utils_dfs_precac_decide_pref_chan_for_freq() - Choose preferred channel
 * @pdev: Pointer to DFS pdev object.
 * @ch_freq: Pointer to channel frequency.
 * @mode: Configured PHY mode.
 *
 * Wrapper function for dfs_decide_precac_preferred_chan_for_freq(). This
 * function called from outside of dfs component.
 *
 * Return: True if intermediate channel needs to configure. False otherwise.
 */
#ifdef CONFIG_CHAN_FREQ_API
bool
utils_dfs_precac_decide_pref_chan_for_freq(struct wlan_objmgr_pdev *pdev,
					   uint16_t *ch_freq,
					   enum wlan_phymode mode);
#endif
#endif

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
/*
 * utils_dfs_radar_disable() - Disables the radar.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: true if Spruce spur WAR is applicable else false.
 *
 * Spur or leakage transmissions is observed in Spruce HW in
 * frequencies from 5260MHz to 5320MHz when one of the following
 * conditions is true,
 * i) The AP is transmitting in 52/56/60/64 in 80MHz mode and then the  AP
 * moves to the adjacent channel 36/44/48 in 80MHz mode and starts
 * transmitting.
 * ii) The AP is transmitting in 36/44/48/52/56/60/64 in 160MHz mode and then
 * the  AP moves to the adjacent channel 36/44/48 in 80MHz mode and starts
 * transmitting.
 *
 * Hence, the spruce spur WAR becomes applicable when,
 * a) the target is Spruce,
 * b) the primary channel is 52/56/60/64, and the home channel width is 80MHz.
 * c) or, the primary channel is 36/44/48/52/56/60/64 and the home channel width
 *    is 160MHz.
 *
 * When the conditions (a) and (b) or (c) is true, random channel selection
 * should make sure to prevent moving to the adjacent channels 36/44/48 in
 * 80MHz mode. Failing to do so will cause spur transmissions in channel 52
 * through 64.
 */
bool utils_dfs_is_spruce_spur_war_applicable(struct wlan_objmgr_pdev *pdev);

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
 * utils_dfs_fetch_nol_ie_info() - Fills the arguments with NOL information
 * needed for sending RCSA.
 * pdev: Pointer to DFS pdev object.
 * nol_ie_bandwidth: Minimum DFS subchannel Bandwidth.
 * nol_ie_startfreq: Radar affected channel list start subchannel's centre
 * frequency.
 * nol_ie_bitmap: Bitmap of radar affected subchannels.
 */
QDF_STATUS utils_dfs_fetch_nol_ie_info(struct wlan_objmgr_pdev *pdev,
				       uint8_t *nol_ie_bandwidth,
				       uint16_t *nol_ie_startfreq,
				       uint8_t *nol_ie_bitmap);

/**
 * utils_dfs_set_rcsa_flags() - Set flags that are required for sending
 * RCSA and NOL IE.
 * pdev: Pointer to DFS pdev object.
 * is_rcsa_ie_sent: Boolean to check if RCSA IE should be sent or not.
 * is_nol_ie_sent: Boolean to check if NOL IE should be sent or not.
 */

QDF_STATUS utils_dfs_set_rcsa_flags(struct wlan_objmgr_pdev *pdev,
				    bool is_rcsa_ie_sent,
				    bool is_nol_ie_sent);

/**
 * utils_dfs_get_rcsa_flags() - Get flags that are required for sending
 * RCSA and NOL IE.
 * pdev: Pointer to DFS pdev object.
 * is_rcsa_ie_sent: Boolean to check if RCSA IE should be sent or not.
 * is_nol_ie_sent: Boolean to check if NOL IE should be sent or not.
 */

QDF_STATUS utils_dfs_get_rcsa_flags(struct wlan_objmgr_pdev *pdev,
				    bool *is_rcsa_ie_sent,
				    bool *is_nol_ie_sent);

/**
 * utils_dfs_process_nol_ie_bitmap() - Update NOL with external radar
 * information.
 * pdev: Pointer to DFS pdev object.
 * nol_ie_bandwidth: Minimum DFS subchannel Bandwidth.
 * nol_ie_startfreq: Radar affected channel list start channel's
 * centre frequency.
 * nol_ie_bitmap: Bitmap of radar affected subchannels.
 *
 * Return: True if NOL IE should be propagated, else false.
 */
bool utils_dfs_process_nol_ie_bitmap(struct wlan_objmgr_pdev *pdev,
				     uint8_t nol_ie_bandwidth,
				     uint16_t nol_ie_startfreq,
				     uint8_t nol_ie_bitmap);

/**
 * utils_dfs_bw_reduce() - Set bw reduce.
 * @pdev: Pointer to DFS pdev object.
 * @bw_reduce: Fill bw_reduce value in this variable.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS utils_dfs_bw_reduce(struct wlan_objmgr_pdev *pdev,
				bool bw_reduce);

/**
 * utils_dfs_is_bw_reduce() - Get bw reduce.
 * @pdev: Pointer to DFS pdev object.
 * @bw_reduce: Pointer to get bw_reduce value.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS utils_dfs_is_bw_reduce(struct wlan_objmgr_pdev *pdev,
				  bool *bw_reduce);
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
 * utils_dfs_get_random_channel_for_freq() - Get random channel.
 * @pdev: Pointer to DFS pdev object.
 * @flags: random channel selection flags.
 * @ch_params: current channel params.
 * @hw_mode: current operating mode.
 * @target_chan: Pointer to target_chan freq.
 * @acs_info: acs range info.
 *
 * wrapper function for get_random_chan(). this
 * function called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
utils_dfs_get_random_channel_for_freq(struct wlan_objmgr_pdev *pdev,
				      uint16_t flags,
				      struct ch_params *ch_params,
				      uint32_t *hw_mode, uint16_t *target_chan,
				      struct dfs_acs_info *acs_info);
#endif

/**
 * utils_dfs_get_vdev_random_channel_for_freq() - Get random channel for vdev
 * @pdev: Pointer to DFS pdev object.
 * @vdev: vdev of the request
 * @flags: random channel selection flags.
 * @ch_params: current channel params.
 * @hw_mode: current operating mode.
 * @target_chan: Pointer to target_chan_freq.
 * @acs_info: acs range info.
 *
 * Get random channel based on vdev interface type. If the vdev is null,
 * the function will get random channel by SAP interface type.
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS utils_dfs_get_vdev_random_channel_for_freq(
	struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
	uint16_t flags, struct ch_params *ch_params, uint32_t *hw_mode,
	uint16_t *target_chan_freq, struct dfs_acs_info *acs_info);
#endif

/**
 * utils_dfs_bw_reduced_channel_for_freq() - Get BW reduced channel.
 * @pdev: Pointer to DFS pdev object.
 * @ch_params: current channel params.
 * @hw_mode: current operating mode.
 * @target_chan: Pointer to target_chan freq.
 *
 * wrapper function for get bw_reduced_channel. this
 * function called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS utils_dfs_bw_reduced_channel_for_freq(struct wlan_objmgr_pdev *pdev,
						 struct ch_params *ch_params,
						 uint32_t *hw_mode,
						 uint16_t *target_chan_freq);
#endif

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
 * utils_is_dfs_chan_for_freq() - is channel dfs.
 * @pdev: pdev handler.
 * @chan_freq: Channel frequency in MHZ.
 *
 * is channel dfs.
 *
 * Return: True if channel dfs, else false.
 */
#ifdef CONFIG_CHAN_FREQ_API
static inline bool utils_is_dfs_chan_for_freq(struct wlan_objmgr_pdev *pdev,
					      uint32_t chan_freq)
{
	return wlan_reg_is_dfs_for_freq(pdev, chan_freq);
}
#endif

/**
 * utils_is_dfs_cfreq2_ch() - is channel dfs cfreq2.
 * @pdev: pdev handler.
 *
 * is channel dfs.
 *
 * Return: True if channel dfs cfreq2, else false.
 */
bool utils_is_dfs_cfreq2_ch(struct wlan_objmgr_pdev *pdev);

/**
 * utils_dfs_reg_update_nol_chan_for_freq() - set nol channel
 *
 * @pdev: pdev ptr
 * @ch_list: freq channel list to be returned
 * @num_ch: number of channels
 * @nol_ch: nol flag
 *
 * Return: void
 */
#ifdef CONFIG_CHAN_FREQ_API
void utils_dfs_reg_update_nol_chan_for_freq(struct wlan_objmgr_pdev *pdev,
					    uint16_t *ch_list,
					    uint8_t num_ch,
					    bool nol_ch);
#endif
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

#ifdef MOBILE_DFS_SUPPORT
/**
 * utils_dfs_mark_leaking_chan_for_freq() - to mark channel leaking in to nol
 * @pdev: Pointer to pdev structure.
 * @ch_width: channel width
 * @temp_ch_lst_sz: the target channel list
 * @temp_ch_lst: the target frequency list
 *
 * This function removes the channels from temp channel list that
 * (if selected as target channel) will cause leakage in one of
 * the NOL channels
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS utils_dfs_mark_leaking_chan_for_freq(struct wlan_objmgr_pdev *pdev,
						enum phy_ch_width ch_width,
						uint8_t temp_ch_lst_sz,
						uint16_t *temp_ch_lst);
#endif

/**
 * utils_dfs_can_ignore_radar_event() - check whether to skip radar event
 * processing
 * @pdev: Pointer to pdev structure.
 *
 * This function will check with policy mgr to process radar event or not based
 * on current concurrency mode and dfs policy.
 *
 * Return: true - ignore radar event processing, otherwise false.
 */
bool utils_dfs_can_ignore_radar_event(struct wlan_objmgr_pdev *pdev);
#else
#ifdef CONFIG_CHAN_FREQ_API
static inline QDF_STATUS utils_dfs_mark_leaking_chan_for_freq
	(struct wlan_objmgr_pdev *pdev,
	enum phy_ch_width ch_width,
	uint8_t temp_ch_lst_sz,
	uint16_t *temp_ch_lst)
{
	return QDF_STATUS_SUCCESS;
}
#endif
static inline bool
utils_dfs_can_ignore_radar_event(struct wlan_objmgr_pdev *pdev)
{
	return false;
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

 * utils_dfs_is_spoof_done() - get spoof check status.
 * @pdev: pdev ptr
 *
 * Return: True if dfs_spoof_test_done is set.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS utils_dfs_is_spoof_check_failed(struct wlan_objmgr_pdev *pdev,
					   bool *is_spoof_check_failed);

bool utils_dfs_is_spoof_done(struct wlan_objmgr_pdev *pdev);
#else
static inline
QDF_STATUS utils_dfs_is_spoof_check_failed(struct wlan_objmgr_pdev *pdev,
					   bool *is_spoof_check_failed)
{
	return QDF_STATUS_SUCCESS;
}

static inline
bool utils_dfs_is_spoof_done(struct wlan_objmgr_pdev *pdev)
{
	return true;
}
#endif

/**
 * dfs_get_num_chans() - Get the number of channels supported by the regulatory.
 *
 * Return: Number of supported channels.
 */
int dfs_get_num_chans(void);

/**
 * utils_dfs_get_chan_list() - Get channel list from regdb.
 * @pdev: Pointer to DFS pdev object.
 * @clist: Pointer to current channel list
 * @num_chan: number of channels in the current channel list.
 */
void utils_dfs_get_chan_list(struct wlan_objmgr_pdev *pdev,
			     void *clist,
			     uint32_t *num_chan);

/**
 * utils_dfs_get_nol_history_chan_list() - Get nol_history channels from regdb
 * component.
 * @pdev: Pointer to pdev structure.
 * @clist: Pointer to channel list.
 * @num_chan: number of channels.
 */
void utils_dfs_get_nol_history_chan_list(struct wlan_objmgr_pdev *pdev,
					 void *clist, uint32_t *num_chan);

/**
 * utils_dfs_reg_update_nol_history_chan_for_freq() - set nol history channel
 *
 * @pdev: pdev ptr
 * @ch_list: freq channel list to be returned
 * @num_ch: number of channels
 * @nol_history_ch: nol history flag
 *
 * Return: void
 */
#ifdef CONFIG_CHAN_FREQ_API
void utils_dfs_reg_update_nol_history_chan_for_freq(struct wlan_objmgr_pdev *,
						    uint16_t *freq_list,
						    uint8_t num_ch,
						    bool nol_history_ch);
#endif

/**
 * utils_dfs_is_cac_required() - Check if CAC is required on the cur_chan.
 * @pdev: pdev ptr
 * @cur_chan: Pointer to current channel of wlan_channel structure.
 * @prev_chan: Pointer to previous channel of wlan_channel structure.
 * @continue_current_cac: If AP can start CAC then this variable indicates
 * whether to continue with the current CAC or restart the CAC. This variable
 * is valid only if this function returns true.
 *
 * Return: true if AP requires CAC or can continue current CAC, else false.
 */
bool utils_dfs_is_cac_required(struct wlan_objmgr_pdev *pdev,
			       struct wlan_channel *cur_chan,
			       struct wlan_channel *prev_chan,
			       bool *continue_current_cac);

/**
 * utils_dfs_is_cac_required_on_dfs_curchan() - Check if CAC is required on the
 * dfs_curchan.
 * @pdev: pdev ptr
 * @continue_current_cac: If AP can start CAC then this variable indicates
 * whether to continue with the current CAC or restart the CAC. This variable
 * is valid only if this function returns true.
 * @is_vap_restart: Flag to indicate if vap is restarted/started.
 * True: VAP restart. False: VAP start
 *
 * This API checks if the dfs_curchan is a subset of the dfs_prevchan.
 * dfs_curchan and dfs_prevchan are updated after start response by
 * dfs_set_current_channel().
 *
 * Return: true if AP requires CAC or can continue current CAC, else false.
 */
bool
utils_dfs_is_cac_required_on_dfs_curchan(struct wlan_objmgr_pdev *pdev,
					 bool *continue_current_cac,
					 bool is_vap_restart);

/** utils_dfs_is_precac_done() - Check if precac has been done in chosen channel
 * @pdev: Pointer to DFS pdev object.
 * @wlan_chan: Pointer to wlan channel object that can be accessed by other
 * components.
 * Wrapper function for dfs_is_precac_done(). This API is called from outside
 * the dfs component.
 *
 * Return:
 * * True :If precac is done on channel.
 * * False:If precac is not done on channel.
 */
bool utils_dfs_is_precac_done(struct wlan_objmgr_pdev *pdev,
			      struct wlan_channel *wlan_chan);
/**
 * utils_dfs_get_disable_radar_marking() - Retrieve the value of disable radar.
 * marking.
 * @pdev: Pointer to DFS pdev object.
 * @dis_radar_marking: pointer to retrieve the value of disable_radar_marking.
 */
#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
QDF_STATUS utils_dfs_get_disable_radar_marking(struct wlan_objmgr_pdev *pdev,
					       bool *disable_radar_marking);
#else
static inline
QDF_STATUS utils_dfs_get_disable_radar_marking(struct wlan_objmgr_pdev *pdev,
					       bool *disable_radar_marking)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * utils_dfs_deliver_event() - Deliver DFS event to userspace.
 * @pdev: Pointer to DFS pdev object
 * @chan: channel radar hit on
 * @event: event being sent
 */
void utils_dfs_deliver_event(struct wlan_objmgr_pdev *pdev, uint16_t freq,
			     enum WLAN_DFS_EVENTS event);

/**
 * utils_dfs_reset_dfs_prevchan() - Reset DFS previous channel structure.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: None.
 */
void utils_dfs_reset_dfs_prevchan(struct wlan_objmgr_pdev *pdev);

#ifdef QCA_SUPPORT_AGILE_DFS
/**
 * utils_dfs_agile_sm_deliver_evt() - API to post events to DFS Agile SM.
 * @pdev: Pointer to DFS pdev object.
 * @event: Event to be posted to DFS AGILE SM.
 *
 * Return: None.
 */
void utils_dfs_agile_sm_deliver_evt(struct wlan_objmgr_pdev *pdev,
				    enum dfs_agile_sm_evt event);
#else
static inline
void utils_dfs_agile_sm_deliver_evt(struct wlan_objmgr_pdev *pdev,
				    enum dfs_agile_sm_evt event)
{
}
#endif/*QCA_SUPPORT_AGILE_DFS*/

#ifdef QCA_SUPPORT_ADFS_RCAC
/**
 * utils_dfs_get_rcac_channel() - Get the completed Rolling CAC channel if
 *                                available.
 * @pdev: Pointer to DFS pdev object.
 * @ch_params: current channel params.
 * @target_chan: Pointer to target_chan freq.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS utils_dfs_get_rcac_channel(struct wlan_objmgr_pdev *pdev,
				      struct ch_params *chan_params,
				      qdf_freq_t *target_chan_freq);
#else
static inline
QDF_STATUS utils_dfs_get_rcac_channel(struct wlan_objmgr_pdev *pdev,
				      struct ch_params *chan_params,
				      qdf_freq_t *target_chan_freq)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_SUPPORT_ADFS_RCAC */

#ifdef ATH_SUPPORT_ZERO_CAC_DFS
/**
 * utils_dfs_precac_status_for_channel() - API to find the preCAC status
 * of the given channel.
 * @pdev: Pointer to DFS pdev object.
 * @deschan: Pointer to desired channel of wlan_channel structure.
 */
enum precac_status_for_chan
utils_dfs_precac_status_for_channel(struct wlan_objmgr_pdev *pdev,
				    struct wlan_channel *deschan);
#else
static inline enum precac_status_for_chan
utils_dfs_precac_status_for_channel(struct wlan_objmgr_pdev *pdev,
				    struct wlan_channel *deschan)
{
	return DFS_INVALID_PRECAC_STATUS;
}
#endif

#if defined(WLAN_DISP_CHAN_INFO)
/**
 * utils_dfs_get_chan_dfs_state() - Get the channel state array of the channels
 * in a radio.
 * @pdev: Pointer to the pdev.
 * @dfs_ch_s: Output channel state array of the dfs channels in the radio.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS utils_dfs_get_chan_dfs_state(struct wlan_objmgr_pdev *pdev,
					enum channel_dfs_state *dfs_ch_s);

/**
 * utils_dfs_update_chan_state_array() - Update the channel state of the dfs
 * channel indicated by the frequency. The dfs event is converted to
 * appropriate dfs state.
 * @pdev: Pointer to the pdev.
 * @freq: Input frequency.
 * @event: Input dfs event.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS utils_dfs_update_chan_state_array(struct wlan_objmgr_pdev *pdev,
					     qdf_freq_t freq,
					     enum WLAN_DFS_EVENTS event);

/**
 * dfs_init_chan_state_array() - Initialize the dfs channel state array.
 *
 * @pdev: Pointer to the pdev.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS dfs_init_chan_state_array(struct wlan_objmgr_pdev *pdev);
#else
static inline
QDF_STATUS utils_dfs_get_chan_dfs_state(struct wlan_objmgr_pdev *pdev,
					enum channel_dfs_state *dfs_ch_s)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS utils_dfs_update_chan_state_array(struct wlan_objmgr_pdev *pdev,
					     uint16_t freq,
					     enum WLAN_DFS_EVENTS event)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS dfs_init_chan_state_array(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_DISP_CHAN_INFO */

/**
 * utils_dfs_radar_enable() - Enables the radar.
 * @pdev: Pointer to DFS pdev object.
 *
 * This is called each time for eCSA occurs but don't change channel due
 * to channel switch disabled in dfs test mode.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS utils_dfs_radar_enable(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_DFS_UTILS_API_H_ */
