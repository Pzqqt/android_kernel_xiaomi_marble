/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file has the headers of Precac Tree structure feature.
 *
 */

#ifndef _DFS_PRECAC_FOREST_H_
#define _DFS_PRECAC_FOREST_H_
#include <dfs_zero_cac.h>
#include <dfs.h>
#include <wlan_lmac_if_def.h>
#define N_OFFSETS 2

/**
 * struct dfs_channel_bw - Structure to store the information about precac
 * root's primary channel frequency, maximum bandwidth and the center frequency.
 *
 * @dfs_pri_ch_freq:     Primary channel frequency of the root channel.
 * @dfs_center_ch_freq:  Center frequency of the 20/40/80/160 channel.In case of
 *                       the 165MHz channel, it is 5730MHz.
 * @dfs_max_bw:          Maximum bandwidth of the channel available in the
 *                       current channel list.
 */
struct dfs_channel_bw {
	uint16_t dfs_pri_ch_freq;
	uint16_t dfs_center_ch_freq;
	uint16_t dfs_max_bw;
};

/**
 * struct precac_tree_offset_for_different_bw - Bandwidth, tree depth and
 * channel offsets information to build the precac tree.
 * @bandwidth:                Bandwidth of the the root node.
 * @tree_depth:               Tree depth of the precac tree.
 * @initial_and_next_offsets: Offset to root node to find the initial and the
 *                            next channels of the node.
 */
struct precac_tree_offset_for_different_bw {
	int bandwidth;
	int tree_depth;
	int initial_and_next_offsets[TREE_DEPTH_MAX][N_OFFSETS];
};

#define START_INDEX 0
#define STEP_INDEX 1


static const
struct precac_tree_offset_for_different_bw offset20 = {DFS_CHWIDTH_20_VAL,
	TREE_DEPTH_20,
	{
		{0, NEXT_20_CHAN_FREQ_OFFSET}
	}
};

static const
struct precac_tree_offset_for_different_bw offset40 = {DFS_CHWIDTH_40_VAL,
	TREE_DEPTH_40,
	{
		{0, NEXT_40_CHAN_FREQ_OFFSET},
		{-10, NEXT_20_CHAN_FREQ_OFFSET}
	}
};

static const
struct precac_tree_offset_for_different_bw offset80 = {DFS_CHWIDTH_80_VAL,
	TREE_DEPTH_80,
	{
		{0, NEXT_80_CHAN_FREQ_OFFSET},
		{-20, NEXT_40_CHAN_FREQ_OFFSET},
		{-30, NEXT_20_CHAN_FREQ_OFFSET}
	}
};

static const
struct precac_tree_offset_for_different_bw offset160 = {DFS_CHWIDTH_160_VAL,
	TREE_DEPTH_160,
	{
		{INITIAL_160_CHAN_FREQ_OFFSET, NEXT_160_CHAN_FREQ_OFFSET},
		{INITIAL_80_CHAN_FREQ_OFFSET, NEXT_80_CHAN_FREQ_OFFSET},
		{INITIAL_40_CHAN_FREQ_OFFSET, NEXT_40_CHAN_FREQ_OFFSET},
		{INITIAL_20_CHAN_FREQ_OFFSET, NEXT_20_CHAN_FREQ_OFFSET}
	}
};

static const
struct precac_tree_offset_for_different_bw default_offset = {0, 0};

/* Given a bandwidth, find the number of subchannels in that bandwidth */
#define N_SUBCHS_FOR_BANDWIDTH(_bw) ((_bw) / MIN_DFS_SUBCHAN_BW)

#define DFS_160MHZ_SECSEG_CHAN_OFFSET 40

#define VHT80_FREQ_OFFSET 30
/* For any 160MHz channel, a frequency offset of 70MHz would have been enough
 * to include the right edge and left edge channels. But, the restricted 80P80
 * or the 165MHz channel is also assumed to have a 160MHz root ie channel 146,
 * so an offset of 75MHz is chosen.
 */
#define VHT160_FREQ_OFFSET 75

#define IS_WITHIN_RANGE(_A, _B, _C)  \
	(((_A) >= ((_B)-(_C))) && ((_A) <= ((_B)+(_C))))

#define IS_WITHIN_RANGE_STRICT(_A, _B, _C)  \
	(((_A) > ((_B)-(_C))) && ((_A) < ((_B)+(_C))))

#define MAX_PREFIX_CHAR 28

/**
 * dfs_configure_deschan_for_precac() - API to prioritize user configured
 * channel for preCAC.
 *
 * @dfs: Pointer to DFS of wlan_dfs structure.
 * Return: frequency of type qdf_freq_t if configured, else 0.
 */
qdf_freq_t dfs_configure_deschan_for_precac(struct wlan_dfs *dfs);

/**
 * dfs_is_pcac_required_for_freq() - Find if given frequency is preCAC required.
 * @node: Pointer to the preCAC tree Node in which the frequency is present.
 * @freq: Frequency to be checked.
 *
 * Return: False if the frequency is not fully CAC done or in NOL, else true.
 */
bool dfs_is_pcac_required_for_freq(struct precac_tree_node *node,
				   uint16_t freq);

/**
 * dfs_find_subchannels_for_center_freq() - API to find the subchannels given
 * the center frequencies and ch_width.
 * @pri_center_freq: It is the center of 20/40/80/160Mhz band and for 80+80Mhz
 *                   it is the center of the first 80Mhz band.
 * @sec_center_freq: It is used only for 80+80Mhz and denotes the center
 *                   of the second 80Mhz band.
 * @ch_width: Channel width.
 * @channels: List of subchannels.
 *
 * Return: Number of subchannels.
 */
uint8_t dfs_find_subchannels_for_center_freq(qdf_freq_t pri_center_freq,
					     qdf_freq_t sec_center_freq,
					     enum phy_ch_width ch_width,
					     qdf_freq_t *channels);

/**
 * dfs_find_precac_state_of_node() - Find the preCAC state of the given channel.
 * @channel: Channel whose preCAC state is to be found.
 * @precac_entry: PreCAC entry where the channel exists.
 *
 * Return, enum value of type precac_chan_state.
 */
enum precac_chan_state
dfs_find_precac_state_of_node(qdf_freq_t channel,
			      struct dfs_precac_entry *precac_entry);

/* dfs_mark_adfs_chan_as_cac_done()- Mark the ADFS CAC completed channel as
 *                                   CAC done in the precac tree.
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_mark_adfs_chan_as_cac_done(struct wlan_dfs *dfs);

/* dfs_descend_precac_tree_for_freq() - Descend into the precac BSTree based on
 *                             the channel provided. If the channel is less than
 *                             given node's channel, descend left, else right.
 * @node:    Precac BSTree node.
 * @chan_freq: Channel freq whose node is to be found.
 *
 * Return: the next precac_tree_node (left child or right child of
 * current node).
 */

struct precac_tree_node *
dfs_descend_precac_tree_for_freq(struct precac_tree_node *node,
				 uint16_t chan_freq);

/**
 * dfs_unmark_rcac_done() - Unmark the CAC done channels from the RCAC list.
 * @dfs: Pointer to wlan_dfs object.
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
void dfs_unmark_rcac_done(struct wlan_dfs *dfs);
#else
static inline
void dfs_unmark_rcac_done(struct wlan_dfs *dfs)
{
}
#endif
/**
 * dfs_is_precac_completed_count_non_zero() - API to find if the preCAC
 * completed channels count is zero/non_zero.
 * @dfs: Pointer to DFS object.
 *
 * Return true, if there exists atleast one node/subchannel in the preCAC list
 * that is CAC done, else return false.
 */
bool dfs_is_precac_completed_count_non_zero(struct wlan_dfs *dfs);

/*
 * dfs_fill_adfs_chan_params() - Fill the ADFS FW params.
 * @dfs: Pointer to wlan_dfs.
 * @adfs_param: Pointer to struct dfs_agile_cac_params.
 * @ch_freq: Frequency in MHZ to be programmed to the agile detector.
 */
void dfs_fill_adfs_chan_params(struct wlan_dfs *dfs,
			       struct dfs_agile_cac_params *adfs_param);

/* dfs_agile_precac_cleanup() - Reset parameters of wlan_dfs.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_agile_precac_cleanup(struct wlan_dfs *dfs);

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_precac_check_home_chan_change() - Change the home channel
 *                                       after precac is done.
 *
 * @dfs: Pointer to dfs handler.
 *
 * If precac is done on the home channel, then return true, else false.
 *
 * Return: true if precac done on home channel, else false.
 */
bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs);
#else
static inline bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs)
{
	return false;
}
#endif
#endif /* _DFS_PRECAC_FOREST_H_ */
