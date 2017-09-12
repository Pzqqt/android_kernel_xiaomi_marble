/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#define VHT160_IEEE_FREQ_DIFF 16

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
	os_timer_t          precac_nol_timer;
	struct wlan_dfs      *dfs;
};

/**
 * dfs_zero_cac_timer_init() - Initialize zero-cac timers
 * @dfs: Pointer to DFS structure.
 */
void dfs_zero_cac_timer_init(struct wlan_dfs *dfs);

/**
 * dfs_print_precaclists() - Print precac list.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_print_precaclists(struct wlan_dfs *dfs);

/**
 * dfs_reset_precac_lists() - Resets the precac lists.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_reset_precac_lists(struct wlan_dfs *dfs);

/**
 * dfs_reset_precaclists() - Clears and initiakizes precac_required_list,
 *                           precac_done_list and precac_nol_list.
 *
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_reset_precaclists(struct wlan_dfs *dfs);

/**
 * dfs_deinit_precac_list() - Clears the precac list.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
void dfs_deinit_precac_list(struct wlan_dfs *dfs);

/**
 * dfs_zero_cac_detach() - Free zero_cac memory.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
void dfs_zero_cac_detach(struct wlan_dfs *dfs);

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
void dfs_start_precac_timer(struct wlan_dfs *dfs,
		uint8_t precac_chan);

/**
 * dfs_cancel_precac_timer() - Cancel the precac timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_cancel_precac_timer(struct wlan_dfs *dfs);

/**
 * dfs_zero_cac_attach() - Initialize dfs zerocac variables.
 * @dfs: Pointer to DFS structure.
 */
void dfs_zero_cac_attach(struct wlan_dfs *dfs);

/**
 * dfs_zero_cac_reset() - Reset Zero cac DFS variables.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_zero_cac_reset(struct wlan_dfs *dfs);

/**
 * dfs_is_precac_done() - Is precac done.
 * @dfs: Pointer to wlan_dfs structure.
 */
bool dfs_is_precac_done(struct wlan_dfs *dfs);

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
int dfs_override_precac_timeout(struct wlan_dfs *dfs,
		int precac_timeout);

/**
 * dfs_get_override_precac_timeout() - Get precac timeout.
 * @dfs: Pointer wlan_dfs structure.
 * @precac_timeout: Get precac timeout value in this variable.
 */
int dfs_get_override_precac_timeout(struct wlan_dfs *dfs,
		int *precac_timeout);

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
void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile);

/**
 * dfs_set_precac_enable() - Set precac enable flag.
 * @dfs: Pointer to wlan_dfs structure.
 * @value: input value for dfs_precac_enable flag.
 */
void dfs_set_precac_enable(struct wlan_dfs *dfs,
		uint32_t value);

/**
 * dfs_get_precac_enable() - Get precac enable flag.
 * @dfs: Pointer to wlan_dfs structure.
 */
uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs);

/**
 * dfs_zero_cac_reset() - Reset Zero cac DFS variables.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_zero_cac_reset(struct wlan_dfs *dfs);

/**
 * dfs_is_ht20_40_80_chan_in_precac_done_list() - Is precac done on a
 *                                                VHT20/40/80 channel.
 *@dfs: Pointer to wlan_dfs structure.
 */
bool dfs_is_ht20_40_80_chan_in_precac_done_list(struct wlan_dfs *dfs);

/**
 * dfs_is_ht80_80_chan_in_precac_done_list() - Is precac done on a VHT80+80
 *                                             channel.
 *@dfs: Pointer to wlan_dfs structure.
 */
bool dfs_is_ht80_80_chan_in_precac_done_list(struct wlan_dfs *dfs);

/**
 * dfs_mark_precac_dfs() - Mark the precac channel as radar.
 * @dfs: Pointer to wlan_dfs structure.
 */
void dfs_mark_precac_dfs(struct wlan_dfs *dfs,
		uint8_t is_radar_found_on_secondary_seg);

/**
 * dfs_is_precac_timer_running() - Check whether precac timer is running.
 * @dfs: Pointer to wlan_dfs structure.
 */
bool dfs_is_precac_timer_running(struct wlan_dfs *dfs);
#endif /* _DFS_ZERO_CAC_H_ */
