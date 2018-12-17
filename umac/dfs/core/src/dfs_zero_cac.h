/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DOC: This file has Zero CAC DFS APIs.
 */

#ifndef _DFS_ZERO_CAC_H_
#define _DFS_ZERO_CAC_H_

#include "dfs.h"
#include <wlan_dfs_tgt_api.h>

#define VHT160_IEEE_FREQ_DIFF 16
#define OCAC_SUCCESS 0
#define OCAC_RESET 1
#define OCAC_CANCEL 2

/**
 * struct dfs_precac_entry - PreCAC entry.
 * @pe_list:           PreCAC entry.
 * @vht80_freq:        VHT80 freq.
 * @precac_nol_timer:  Per element precac NOL timer.
 * @dfs:               Pointer to wlan_dfs structure.
 */
struct dfs_precac_entry {
	TAILQ_ENTRY(dfs_precac_entry) pe_list;
	uint8_t             vht80_freq;
	qdf_timer_t         precac_nol_timer;
	struct wlan_dfs     *dfs;
};

/**
 * enum precac_chan_state - Enum for PreCAC state of a channel.
 * @PRECAC_ERR:            Invalid preCAC state.
 * @PRECAC_REQUIRED:       preCAC need to be done on the channel.
 * @PRECAC_NOW:            preCAC is running on the channel.
 * @PRECAC_DONE:           preCAC is done and channel is clear.
 * @PRECAC_NOL:            preCAC is done and radar is detected.
 */
enum precac_chan_state {
	PRECAC_ERR      = -1,
	PRECAC_REQUIRED,
	PRECAC_NOW,
	PRECAC_DONE,
	PRECAC_NOL,
};

/**
 * dfs_zero_cac_timer_init() - Initialize zero-cac timers
 * @dfs_soc_obj: Pointer to DFS SOC object structure.
 */
#if !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj);
#else
static inline void
dfs_zero_cac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
}
#endif
/**
 * dfs_print_precaclists() - Print precac list.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_print_precaclists(struct wlan_dfs *dfs);
#else
static inline void dfs_print_precaclists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_reset_precac_lists() - Resets the precac lists.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_reset_precac_lists(struct wlan_dfs *dfs);
#else
static inline void dfs_reset_precac_lists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_reset_precaclists() - Clears and initiakizes precac_required_list,
 *                           precac_done_list and precac_nol_list.
 *
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_reset_precaclists(struct wlan_dfs *dfs);
#else
static inline void dfs_reset_precaclists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_deinit_precac_list() - Clears the precac list.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
void dfs_deinit_precac_list(struct wlan_dfs *dfs);

/**
 * dfs_zero_cac_detach() - Free zero_cac memory.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_detach(struct wlan_dfs *dfs);
#else
static inline void dfs_zero_cac_detach(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_init_precac_list() - Init precac list.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
void dfs_init_precac_list(struct wlan_dfs *dfs);

/**
 * dfs_start_precac_timer() - Start precac timer.
 * @dfs: Pointer to wlan_dfs structure.
 * @precac_chan: Start thr precac timer in this channel.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_start_precac_timer(struct wlan_dfs *dfs,
			    uint8_t precac_chan);
#else
static inline void dfs_start_precac_timer(struct wlan_dfs *dfs,
					  uint8_t precac_chan)
{
}
#endif

/**
 * dfs_cancel_precac_timer() - Cancel the precac timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_cancel_precac_timer(struct wlan_dfs *dfs);
#else
static inline void dfs_cancel_precac_timer(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_zero_cac_attach() - Initialize dfs zerocac variables.
 * @dfs: Pointer to DFS structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_attach(struct wlan_dfs *dfs);
#else
static inline void dfs_zero_cac_attach(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_zero_cac_reset() - Reset Zero cac DFS variables.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_reset(struct wlan_dfs *dfs);
#else
static inline void dfs_zero_cac_reset(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_zero_cac_timer_detach() - Free Zero cac DFS variables.
 * @dfs_soc_obj: Pointer to dfs_soc_priv_obj structure.
 */
#if !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_timer_detach(struct dfs_soc_priv_obj *dfs_soc_obj);
#else
static inline void
dfs_zero_cac_timer_detach(struct dfs_soc_priv_obj *dfs_soc_obj)
{
}
#endif

/**
 * dfs_is_precac_done() - Is precac done.
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Pointer to dfs_channel for which preCAC done is checked.
 *
 * Return:
 * * True:  If precac is done on channel.
 * * False: If precac is not done on channel.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
bool dfs_is_precac_done(struct wlan_dfs *dfs, struct dfs_channel *chan);
#else
static inline bool dfs_is_precac_done(struct wlan_dfs *dfs,
				      struct dfs_channel *chan)
{
	return false;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_decide_precac_preferred_chan() - Choose operating channel among
 *                                      configured DFS channel and
 *                                      intermediate channel based on
 *                                      precac status of configured
 *                                      DFS channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @pref_chan: Configured DFS channel.
 * @mode: Configured PHY mode.
 *
 * Return: True if intermediate channel needs to configure. False otherwise.
 */
bool
dfs_decide_precac_preferred_chan(struct wlan_dfs *dfs,
				  uint8_t *pref_chan,
				  enum wlan_phymode mode);
#else
static inline void dfs_decide_precac_preferred_chan(struct wlan_dfs *dfs,
						    uint8_t *pref_chan,
						    enum wlan_phymode mode)
{
}
#endif

/**
 * dfs_get_freq_from_precac_required_list() - Get VHT80 freq from
 *                                            precac_required_list.
 * @dfs: Pointer to wlan_dfs structure.
 * @exclude_ieee_freq: Find a VHT80 freqency that is not equal to
 *                     exclude_ieee_freq.
 */
uint8_t dfs_get_freq_from_precac_required_list(struct wlan_dfs *dfs,
		uint8_t exclude_ieee_freq);

/**
 * dfs_override_precac_timeout() - Override the default precac timeout.
 * @dfs: Pointer to wlan_dfs structure.
 * @precac_timeout: Precac timeout value.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
int dfs_override_precac_timeout(struct wlan_dfs *dfs,
		int precac_timeout);
#else
static inline int dfs_override_precac_timeout(struct wlan_dfs *dfs,
		int precac_timeout)
{
	return 0;
}
#endif

/**
 * dfs_get_override_precac_timeout() - Get precac timeout.
 * @dfs: Pointer wlan_dfs structure.
 * @precac_timeout: Get precac timeout value in this variable.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
int dfs_get_override_precac_timeout(struct wlan_dfs *dfs,
		int *precac_timeout);
#else
static inline int dfs_get_override_precac_timeout(struct wlan_dfs *dfs,
		int *precac_timeout)
{
	return 0;
}
#endif

/**
 * dfs_find_vht80_chan_for_precac() - Find VHT80 channel for precac.
 * @dfs: Pointer to wlan_dfs structure.
 * @chan_mode: Channel mode.
 * @ch_freq_seg1: Segment1 channel freq.
 * @cfreq1: cfreq1.
 * @cfreq2: cfreq2.
 * @phy_mode: Precac phymode.
 * @dfs_set_cfreq2: Precac cfreq2
 * @set_agile: Agile mode flag.
 *
 * Zero-CAC-DFS algorithm:-
 * Zero-CAC-DFS algorithm works in stealth mode.
 * 1) When any channel change happens in VHT80 mode the algorithm
 * changes the HW channel mode to VHT80_80/VHT160 mode and adds a
 * new channel in the secondary VHT80 to perform precac and a
 * precac timer is started. However the upper layer/UMAC is unaware
 * of this change.
 * 2) When the precac timer expires without being interrupted by
 * any channel change the secondary VHT80 channel is moved from
 * precac-required-list to precac-done-list.
 * 3) If there is a radar detect at any time in any segment
 * (segment-1 is preimary VHT80 and segment-2 is VHT80)then the
 * channel is searched in both precac-reuired-list and precac-done-list
 * and moved to precac-nol-list.
 * 4) Whenever channel change happens if the new channel is a DFS
 * channel then precac-done-list is searched and if the channel is
 * found in the precac-done-list then the CAC is skipped.
 * 5) The precac expiry timer makes a vedv_restart(channel change
 * with current-upper-layer-channel-mode which is VHT80). In channel
 * change the algorithm tries to pick a new channel from the
 * precac-required list. If none found then channel mode remains same.
 * Which means when all the channels in precac-required-list are
 * exhausted the VHT80_80/VHT160 comes back to VHT80 mode.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile);
#else
static inline void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile)
{
}
#endif

#if defined(QCA_SUPPORT_AGILE_DFS)
/**
 * dfs_find_pdev_for_agile_precac() - Find pdev to select channel for precac.
 * @pdev: Pointer to wlan_objmgr_pdev structure.
 * @cur_precac_dfs_index: current precac index
 */
void dfs_find_pdev_for_agile_precac(struct wlan_objmgr_pdev *pdev,
				    uint8_t *cur_precac_dfs_index);

/**
 * dfs_prepare_agile_precac_chan() - Send Agile set request for given pdev.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_prepare_agile_precac_chan(struct wlan_dfs *dfs);

/**
 * dfs_process_ocac_complete() - Process Off-Channel CAC complete indication.
 * @pdev :Pointer to wlan_objmgr_pdev structure.
 * @ocac_status: Off channel CAC complete status
 * @center_freq : Center Frequency of O-CAC done indication.
 */
void dfs_process_ocac_complete(struct wlan_objmgr_pdev *pdev,
			       uint32_t ocac_status,
			       uint32_t center_freq);

/**
 * dfs_find_vht80_chan_for_agile_precac() - .
 * @pdev :Pointer to wlan_objmgr_pdev structure.
 * @*ch_freq: Pointer to channel number for agile set request.
 * @ch_freq_seg1 : Current primary beaconing channel number.
 * @ch_freq_seg2 : Current secondary segment channel number.
 */
void dfs_find_vht80_chan_for_agile_precac(struct wlan_dfs *dfs,
					  uint8_t *ch_freq,
					  uint8_t ch_freq_seg1,
					  uint8_t ch_freq_seg2);
/**
 * dfs_agile_precac_start() - Start agile precac.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_agile_precac_start(struct wlan_dfs *dfs);

/**
 * dfs_start_agile_precac_timer() - Start precac timer.
 * @dfs: Pointer to wlan_dfs structure.
 * @precac_chan: Start thr precac timer in this channel.
 * @ocac_status: Status of the off channel CAC.
 */
void dfs_start_agile_precac_timer(struct wlan_dfs *dfs,
				  uint8_t precac_chan,
				  uint8_t ocac_status);
#else
static inline void dfs_find_pdev_for_agile_precac(struct wlan_objmgr_pdev *pdev,
						  uint8_t *cur_precac_dfs_index)
{
}

static inline void dfs_prepare_agile_precac_chan(struct wlan_dfs *dfs)
{
}

static inline void
dfs_process_ocac_complete(struct wlan_objmgr_pdev *pdev,
			  uint32_t ocac_status,
			  uint32_t center_freq)
{
}

static inline void dfs_find_vht80_chan_for_agile_precac(struct wlan_dfs *dfs,
							uint8_t *ch_freq,
							uint8_t ch_freq_seg1,
							uint8_t ch_freq_seg2)
{
}

static inline void dfs_agile_precac_start(struct wlan_dfs *dfs)
{
}

static inline void dfs_start_agile_precac_timer(struct wlan_dfs *dfs,
						uint8_t precac_chan,
						uint8_t ocac_status)
{
}
#endif

#if defined(QCA_SUPPORT_AGILE_DFS) || defined(ATH_SUPPORT_ZERO_CAC_DFS)
/**
 * dfs_agile_soc_obj_init() - Initialize soc obj for agile precac.
 * @dfs: Pointer to wlan_dfs structure.
 * @precac_chan: Start thr precac timer in this channel.
 * @ocac_status: Status of the off channel CAC.
 */
void dfs_agile_soc_obj_init(struct wlan_dfs *dfs,
			    struct wlan_objmgr_psoc *psoc);
#else
static inline void dfs_agile_soc_obj_init(struct wlan_dfs *dfs,
					  struct wlan_objmgr_psoc *psoc)
{
}
#endif

/**
 * dfs_set_precac_enable() - Set precac enable flag.
 * @dfs: Pointer to wlan_dfs structure.
 * @value: input value for dfs_precac_enable flag.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_set_precac_enable(struct wlan_dfs *dfs,
		uint32_t value);
#else
static inline void dfs_set_precac_enable(struct wlan_dfs *dfs,
		uint32_t value)
{
}
#endif

/**
 * dfs_get_precac_enable() - Get precac enable flag.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs);
#else
static inline uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_set_precac_intermediate_chan() - Set intermediate chan to be used while
 *                                      doing precac.
 * @dfs: Pointer to wlan_dfs structure.
 * @value: input value for dfs_precac_enable flag.
 *
 * Return:
 * * 0       - Successfully set intermediate channel.
 * * -EINVAL - Invalid channel.
 */
int32_t dfs_set_precac_intermediate_chan(struct wlan_dfs *dfs,
					 uint32_t value);
#else
static inline int32_t dfs_set_precac_intermediate_chan(struct wlan_dfs *dfs,
						       uint32_t value)
{
	return 0;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_get_precac_intermediate_chan() - Get configured precac
 *					intermediate channel.
 * @dfs: Pointer to wlan_dfs structure.
 *
 * Return: Configured intermediate channel number.
 */
uint32_t dfs_get_precac_intermediate_chan(struct wlan_dfs *dfs);
#else
static inline uint32_t dfs_get_intermediate_chan(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_get_precac_chan_state() - Get precac status of a given channel.
 * @dfs:         Pointer to wlan_dfs structure.
 * @precac_chan: Channel number for which precac state need to be checked.
 *
 * Return:
 * * PRECAC_REQUIRED: Precac has not done on precac_chan.
 * * PRECAC_NOW     : Precac is running on precac_chan.
 * * PRECAC_DONE    : precac_chan is in precac done list.
 * * PRECAC_NOL     : precac_chan is in precac NOL list.
 * * PRECAC_ERR     : Invalid precac state.
 */
enum precac_chan_state
dfs_get_precac_chan_state(struct wlan_dfs *dfs, uint8_t precac_chan);
#else
static inline enum precac_chan_state
dfs_get_precac_chan_state(struct wlan_dfs *dfs,
			  uint8_t precac_chan)
{
	return PRECAC_REQUIRED;
}
#endif

/**
 * dfs_zero_cac_reset() - Reset Zero cac DFS variables.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_zero_cac_reset(struct wlan_dfs *dfs);

/**
 * dfs_is_ht20_40_80_chan_in_precac_done_list() - Is precac done on a
 *                                                VHT20/40/80 channel.
 *@dfs: Pointer to wlan_dfs structure.
 *@chan: Pointer to dfs_channel for which preCAC done is checked.
 *
 * Return:
 * * True:  If channel is present in precac-done list.
 * * False: If channel is not present in precac-done list.
 */
bool dfs_is_ht20_40_80_chan_in_precac_done_list(struct wlan_dfs *dfs,
						struct dfs_channel *chan);

/**
 * dfs_is_ht8080_ht160_chan_in_precac_done_list() - Is precac done on
 *                                                  VHT80+80 or VHT160
 *                                                  channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Pointer to dfs_channel for which preCAC done is checked.
 *
 * Return:
 * * True:  If channel is present in precac-done list.
 * * False: If channel is not present in precac-done list.
 */
bool dfs_is_ht8080_ht160_chan_in_precac_done_list(struct wlan_dfs *dfs,
						  struct dfs_channel *chan);

/**
 * dfs_mark_precac_dfs() - Mark the precac channel as radar.
 * @dfs: Pointer to wlan_dfs structure.
 * @is_radar_found_on_secondary_seg: Radar found on secondary seg for Cascade.
 * @detector_id: detector id which found RADAR in HW.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_mark_precac_dfs(struct wlan_dfs *dfs,
		uint8_t is_radar_found_on_secondary_seg,
		uint8_t detector_id);
#else
static inline void dfs_mark_precac_dfs(struct wlan_dfs *dfs,
		uint8_t is_radar_found_on_secondary_seg,
		uint8_t detector_id)
{
}
#endif

/**
 * dfs_is_precac_timer_running() - Check whether precac timer is running.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && !defined(QCA_MCL_DFS_SUPPORT)
bool dfs_is_precac_timer_running(struct wlan_dfs *dfs);
#else
static inline bool dfs_is_precac_timer_running(struct wlan_dfs *dfs)
{
	return false;
}
#endif
#endif /* _DFS_ZERO_CAC_H_ */
